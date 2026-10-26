#include "combat/group/EmployerTeamRosterSystem.hpp"

#include "combat/group/TeamEmploymentSystem.hpp"
#include "combat/group/EmployerTeamDisciplineSystem.hpp"
#include "entity/Player.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <vector>

namespace
{
    constexpr const char* RosterCategory = "team_employer_roster_state";
    constexpr const char* VitalsCategory = "team_employer_roster_vitals";
    constexpr const char* ProgressCategory = "team_employer_roster_progress";
    constexpr const char* HistoryCategory = "team_employer_roster_history";

    unsigned int stableHash(const std::string& seed)
    {
        unsigned int hash = 2166136261u;
        for (unsigned char c : seed)
        {
            hash ^= c;
            hash *= 16777619u;
        }
        return hash;
    }

    int rankIndexFromLevel(int level)
    {
        if (level >= 70) return 6;
        if (level >= 52) return 5;
        if (level >= 38) return 4;
        if (level >= 26) return 3;
        if (level >= 15) return 2;
        if (level >= 8) return 1;
        return 0;
    }

    std::string field(const std::string& label, const std::string& name, const std::string& fallback = "")
    {
        const std::string marker = name + "=";
        const std::size_t pos = label.find(marker);
        if (pos == std::string::npos) return fallback;
        const std::size_t start = pos + marker.size();
        std::size_t end = label.find(" | ", start);
        if (end == std::string::npos) end = label.size();
        if (end <= start) return fallback;
        return label.substr(start, end - start);
    }

    int intField(const std::string& label, const std::string& name, int fallback)
    {
        try
        {
            return std::stoi(field(label, name, std::to_string(fallback)));
        }
        catch (...)
        {
            return fallback;
        }
    }

    bool boolField(const std::string& label, const std::string& name, bool fallback)
    {
        const std::string value = field(label, name, fallback ? "1" : "0");
        return value == "1" || value == "true" || value == "yes";
    }

    std::string contractMemberKey(const TeamEmploymentContract& contract, const std::string& memberId)
    {
        return contract.contractId + ":" + memberId;
    }

    const PlayerJournalRecord* findRecord(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key)
            {
                return &record;
            }
        }
        return nullptr;
    }

    std::string memberLabel(const EmployerTeamMember& member)
    {
        return "nom=" + member.name
            + " | race=" + member.race
            + " | profil=" + member.job
            + " | trait=" + member.trait
            + " | niveau=" + std::to_string(member.level)
            + " | rang=" + std::to_string(member.rankIndex)
            + " | max_hp=" + std::to_string(member.maxHp)
            + " | arme=" + std::to_string(member.weaponQuality)
            + " | armure=" + std::to_string(member.armorQuality)
            + " | kit=" + std::to_string(member.supportKitQuality)
            + " | humeur=" + std::to_string(member.mood)
            + " | loyaute=" + std::to_string(member.loyalty)
            + " | chef=" + std::string(member.leader ? "1" : "0");
    }

    std::string vitalsLabel(int currentHp, bool available, const std::string& status)
    {
        return "hp=" + std::to_string(std::max(0, currentHp))
            + " | disponible=" + std::string(available ? "1" : "0")
            + " | statut=" + (status.empty() ? std::string("operationnel") : status);
    }

    EmployerTeamMember parseMember(
        const Player& player,
        const TeamEmploymentContract& contract,
        const PlayerJournalRecord& record)
    {
        EmployerTeamMember member;
        const std::size_t separator = record.key.find(':');
        member.memberId = separator == std::string::npos ? record.key : record.key.substr(separator + 1);
        member.name = field(record.label, "nom", member.memberId);
        member.race = field(record.label, "race", "Humain");
        member.job = field(record.label, "profil", "Combattant");
        member.trait = field(record.label, "trait", "Professionnel");
        member.level = std::max(1, intField(record.label, "niveau", 1));
        member.rankIndex = std::clamp(intField(record.label, "rang", rankIndexFromLevel(member.level)), 0, 6);
        member.maxHp = std::max(18, intField(record.label, "max_hp", 34 + member.level * 3 + member.rankIndex * 15));
        member.weaponQuality = std::clamp(intField(record.label, "arme", 1 + member.rankIndex), 1, 10);
        member.armorQuality = std::clamp(intField(record.label, "armure", std::max(1, member.weaponQuality - 1)), 1, 10);
        member.supportKitQuality = std::clamp(intField(record.label, "kit", 0), 0, 10);
        member.mood = std::clamp(intField(record.label, "humeur", 50), 0, 100);
        member.loyalty = std::clamp(intField(record.label, "loyaute", 50), 0, 100);
        member.leader = boolField(record.label, "chef", false);

        const std::string key = contractMemberKey(contract, member.memberId);
        const PlayerJournalRecord* vitals = findRecord(player, VitalsCategory, key);
        if (vitals != nullptr)
        {
            member.currentHp = std::clamp(intField(vitals->label, "hp", member.maxHp), 0, member.maxHp);
            member.available = boolField(vitals->label, "disponible", true) && member.currentHp > 0;
            const int readyDay = intField(vitals->label, "ready_day", -1);
            if (!member.available && readyDay >= 0 && player.getWorldDaysElapsed() >= readyDay)
            {
                member.currentHp = std::max(1, member.maxHp * 45 / 100);
                member.available = true;
            }
        }
        else
        {
            member.currentHp = member.maxHp;
            member.available = true;
        }

        const PlayerJournalRecord* progress = findRecord(player, ProgressCategory, key);
        if (progress != nullptr && progress->count > 0)
        {
            const int gained = std::min(12, progress->count / 6);
            member.level = std::min(100, member.level + gained);
            member.rankIndex = std::max(member.rankIndex, rankIndexFromLevel(member.level));
        }
        return member;
    }

    std::string normalizedToken(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return value;
    }

    bool supportProfile(const std::string& profile)
    {
        const std::string value = normalizedToken(profile);
        return value.find("soigneur") != std::string::npos
            || value.find("médecin") != std::string::npos
            || value.find("mage d'appui") != std::string::npos
            || value.find("clerc") != std::string::npos
            || value.find("prêtre") != std::string::npos;
    }
}

bool EmployerTeamRosterSystem::ensureRoster(Player& player)
{
    const TeamEmploymentContract contract = TeamEmploymentSystem::contractFor(player);
    if (!contract.active || contract.contractId.empty())
    {
        return false;
    }

    const std::string prefix = contract.contractId + ":";
    for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
    {
        if (record.category == RosterCategory && record.key.rfind(prefix, 0) == 0)
        {
            return false;
        }
    }

    static const std::array<const char*, 16> names = {
        "Aelis", "Boran", "Celia", "Doran", "Eren", "Fara", "Galen", "Hilda",
        "Ilyan", "Jora", "Kael", "Lysa", "Marek", "Nessa", "Orin", "Pyria"
    };
    static const std::array<const char*, 8> jobs = {
        "Gardien", "Soigneur", "Archer", "Éclaireur", "Mage d'appui", "Lancier", "Roublard", "Combattant"
    };
    static const std::array<const char*, 8> traits = {
        "Calme", "Prudent", "Loyal", "Direct", "Observateur", "Patient", "Ambitieux", "Solidaire"
    };

    const unsigned int seed = stableHash(contract.contractId + ":" + contract.organizationId + ":roster");
    const int npcCount = 3 + static_cast<int>(seed % 3u); // Avant recrutement : 3 à 5 PNJ ; avec le joueur : 4 à 6 membres au total.
    const int playerLevel = std::max(1, player.getLevel());

    for (int index = 0; index < npcCount; ++index)
    {
        EmployerTeamMember member;
        member.memberId = index == 0 ? "leader" : "member_" + std::to_string(index);
        member.leader = index == 0;
        member.name = member.leader ? contract.leaderName : names[(seed / 7u + static_cast<unsigned int>(index) * 3u) % names.size()];
        member.race = "Humain";
        member.job = member.leader ? "Chef de terrain" : jobs[(seed / 11u + static_cast<unsigned int>(index) * 5u) % jobs.size()];
        member.trait = member.leader ? "Responsable" : traits[(seed / 13u + static_cast<unsigned int>(index) * 7u) % traits.size()];

        const int levelOffset = static_cast<int>((seed >> (index % 12)) % 7u) - 3;
        member.level = std::max(1, std::min(100, playerLevel + levelOffset + (member.leader ? 3 : 0)));
        member.rankIndex = rankIndexFromLevel(member.level);
        const int naturalQuality = std::clamp(1 + member.rankIndex + member.level / 30, 1, 9);
        member.weaponQuality = std::clamp(naturalQuality + (member.leader ? 1 : 0), 1, 10);
        member.armorQuality = std::clamp(naturalQuality - (supportProfile(member.job) ? 1 : 0), 1, 10);
        member.supportKitQuality = supportProfile(member.job) ? std::clamp(2 + member.rankIndex / 2, 2, 7) : (member.leader ? 2 : 1);
        member.maxHp = std::max(20, 36 + member.level * 3 + member.rankIndex * 14 + (member.job == "Gardien" ? 24 : 0));
        member.currentHp = member.maxHp;
        member.mood = std::clamp(48 + static_cast<int>(stableHash(contract.contractId + member.memberId + ":mood") % 31u), 0, 100);
        member.loyalty = std::clamp(52 + static_cast<int>(stableHash(contract.contractId + member.memberId + ":loyalty") % 29u) + (member.leader ? 8 : 0), 0, 100);

        const std::string key = contractMemberKey(contract, member.memberId);
        player.recordCanonicalEvent(RosterCategory, key, memberLabel(member), 1);
        player.recordCanonicalEvent(VitalsCategory, key, vitalsLabel(member.currentHp, true, "operationnel"), 1);
    }

    player.recordCanonicalEvent(
        HistoryCategory,
        contract.contractId + ":creation",
        (contract.organizationName.empty() ? std::string("Équipe employeuse") : contract.organizationName)
            + " | roster persistant créé | PNJ=" + std::to_string(npcCount)
            + " | chef=" + contract.leaderName,
        1
    );
    return true;
}

std::vector<EmployerTeamMember> EmployerTeamRosterSystem::rosterFor(const Player& player)
{
    const TeamEmploymentContract contract = TeamEmploymentSystem::contractFor(player);
    if (!contract.active || contract.contractId.empty())
    {
        return {};
    }

    const std::string prefix = contract.contractId + ":";
    std::vector<EmployerTeamMember> roster;
    for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
    {
        if (record.category != RosterCategory || record.key.rfind(prefix, 0) != 0)
        {
            continue;
        }
        roster.push_back(parseMember(player, contract, record));
    }

    std::stable_sort(roster.begin(), roster.end(), [](const EmployerTeamMember& a, const EmployerTeamMember& b) {
        if (a.leader != b.leader) return a.leader; // Règle dure : le chef reste la première référence de formation.
        if (a.rankIndex != b.rankIndex) return a.rankIndex > b.rankIndex;
        if (a.level != b.level) return a.level > b.level;
        return a.name < b.name;
    });

    if (roster.size() > static_cast<std::size_t>(MaxNpcMembersWhilePlayerEmployed))
    {
        roster.resize(MaxNpcMembersWhilePlayerEmployed);
    }
    return roster;
}

EmployerTeamFormation EmployerTeamRosterSystem::formationFor(const Player& player, bool playerPresent)
{
    EmployerTeamFormation formation;
    formation.playerPresent = playerPresent;
    formation.playerInReserve = playerPresent && EmployerTeamDisciplineSystem::playerStartsInReserve(player);
    const std::vector<EmployerTeamMember> roster = rosterFor(player);
    const int npcActiveSlots = (playerPresent && !formation.playerInReserve) ? 2 : 3;

    for (const EmployerTeamMember& member : roster)
    {
        if (!member.available || member.currentHp <= 0)
        {
            formation.unavailableMembers.push_back(member);
            continue;
        }
        if (static_cast<int>(formation.activeMembers.size()) < npcActiveSlots)
        {
            formation.activeMembers.push_back(member);
        }
        else
        {
            formation.reserveMembers.push_back(member);
        }
    }
    return formation;
}


std::vector<std::string> EmployerTeamRosterSystem::openingTurnOrder(const Player& player, bool playerPresent)
{
    const EmployerTeamFormation formation = formationFor(player, playerPresent);
    std::vector<std::string> order;

    auto leaderIt = std::find_if(formation.activeMembers.begin(), formation.activeMembers.end(), [](const EmployerTeamMember& member) {
        return member.leader && member.available && member.currentHp > 0;
    });
    if (leaderIt != formation.activeMembers.end())
    {
        order.push_back(leaderIt->name);
    }

    if (playerPresent && !formation.playerInReserve)
    {
        order.push_back(player.getName());
    }

    for (const EmployerTeamMember& member : formation.activeMembers)
    {
        if (leaderIt != formation.activeMembers.end() && member.memberId == leaderIt->memberId) continue;
        order.push_back(member.name);
    }
    return order;
}

RecruitedAllyCombatSupport EmployerTeamRosterSystem::toCombatSupport(const EmployerTeamMember& member)
{
    RecruitedAllyCombatSupport ally;
    ally.name = member.name;
    ally.race = member.race;
    ally.job = member.job;
    ally.trait = member.trait;
    ally.motivation = member.leader ? "diriger son escouade" : "remplir son contrat d'équipe";
    ally.recruitedLevel = member.level;
    ally.estimatedLevel = member.level;
    ally.originRankIndex = member.rankIndex;
    ally.currentRankIndex = member.rankIndex;
    ally.requestedShare = 0; // Collègue salarié par l'employeur : aucune part prélevée sur le joueur.
    ally.normalizedShare = 0;
    ally.maxHp = member.maxHp;
    ally.currentHp = member.currentHp;
    ally.healingPotionCharges = std::clamp(1 + member.rankIndex / 2, 1, 4);
    ally.weaponQuality = member.weaponQuality;
    ally.armorQuality = member.armorQuality;
    ally.supportKitQuality = member.supportKitQuality;
    ally.teamOrigin = AllyTeamOrigin::EmployerTeam;
    ally.persistentMemberId = member.memberId;
    ally.teamLeader = member.leader;
    return ally;
}

bool EmployerTeamRosterSystem::setMemberVitals(
    Player& player,
    const std::string& memberId,
    int currentHp,
    bool available,
    const std::string& status)
{
    const TeamEmploymentContract contract = TeamEmploymentSystem::contractFor(player);
    if (!contract.active || memberId.empty()) return false;

    const auto roster = rosterFor(player);
    const auto it = std::find_if(roster.begin(), roster.end(), [&](const EmployerTeamMember& member) {
        return member.memberId == memberId;
    });
    if (it == roster.end()) return false;

    const int hp = std::clamp(currentHp, 0, it->maxHp);
    player.recordCanonicalEvent(
        VitalsCategory,
        contractMemberKey(contract, memberId),
        vitalsLabel(hp, available && hp > 0, status),
        1
    );
    return true;
}

bool EmployerTeamRosterSystem::recordMemberCombatProgress(Player& player, const std::string& memberId, int participationPoints)
{
    const TeamEmploymentContract contract = TeamEmploymentSystem::contractFor(player);
    if (!contract.active || memberId.empty() || participationPoints <= 0) return false;

    const auto roster = rosterFor(player);
    const auto it = std::find_if(roster.begin(), roster.end(), [&](const EmployerTeamMember& member) {
        return member.memberId == memberId;
    });
    if (it == roster.end()) return false;

    player.recordCanonicalEvent(
        ProgressCategory,
        contractMemberKey(contract, memberId),
        it->name + " | progression de terrain avec " + (contract.organizationName.empty() ? std::string("l'équipe") : contract.organizationName),
        participationPoints
    );
    return true;
}

std::string EmployerTeamRosterSystem::statusLabel(const EmployerTeamMember& member)
{
    if (!member.available || member.currentHp <= 0) return "indisponible";
    if (member.leader) return "chef — opérationnel";
    return "opérationnel";
}
