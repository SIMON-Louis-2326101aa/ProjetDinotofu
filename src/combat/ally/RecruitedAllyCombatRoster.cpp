#include "combat/ally/RecruitedAllyCombatRoster.hpp"
#include "story/HestiaCompanionSystem.hpp"

#include "entity/Player.hpp"
#include "entity/player/PlayerHistoryTypes.hpp"
#include "combat/ally/RecruitedAllyConditionSystem.hpp"
#include "progression/Level.hpp"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace
{
    int journalCount(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key)
            {
                return record.count;
            }
        }
        return 0;
    }

    std::string journalLabel(const Player& player, const std::string& category, const std::string& key, const std::string& fallback = "")
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key)
            {
                return record.label.empty() ? fallback : record.label;
            }
        }
        return fallback;
    }

    std::string stringField(const std::string& label, const std::string& fieldName, const std::string& fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t pos = label.find(marker);
        if (pos == std::string::npos) return fallback;
        const std::size_t start = pos + marker.size();
        std::size_t end = label.find(" | ", start);
        if (end == std::string::npos) end = label.size();
        if (end <= start) return fallback;
        return label.substr(start, end - start);
    }

    int intField(const std::string& label, const std::string& fieldName, int fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t pos = label.find(marker);
        if (pos == std::string::npos) return fallback;
        std::size_t start = pos + marker.size();
        std::size_t end = start;
        if (end < label.size() && label[end] == '-') ++end;
        while (end < label.size() && std::isdigit(static_cast<unsigned char>(label[end]))) ++end;
        if (end <= start) return fallback;
        try { return std::stoi(label.substr(start, end - start)); }
        catch (...) { return fallback; }
    }

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

    int initialHealthPercent(const std::string& recruitName)
    {
        return 70 + static_cast<int>(stableHash(recruitName + ":initial_hp_percent") % 31u);
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

    bool isInInfirmary(const Player& player, const std::string& name)
    {
        return !name.empty()
            && journalCount(player, "recrues_infirmerie_sejours", name)
                > journalCount(player, "recrues_infirmerie_recuperees", name);
    }

    bool isAwaitingTransfer(const Player& player, const std::string& name)
    {
        return !name.empty()
            && journalCount(player, "recrues_a_evacuer", name)
                > journalCount(player, "recrues_evacuees_infirmerie", name);
    }

    bool manualOrderEnabled(const Player& player)
    {
        const std::string mode = journalLabel(player, "ordre_manuel_recrues", "mode", "auto");
        return mode.find("manuel") != std::string::npos || mode.find("manual") != std::string::npos;
    }

    int manualOrderIndex(const Player& player, const std::string& name)
    {
        if (!manualOrderEnabled(player)) return 10000;
        for (int slot = 1; slot <= 12; ++slot)
        {
            if (journalLabel(player, "ordre_manuel_recrues", "slot_" + std::to_string(slot), "") == name)
            {
                return slot;
            }
        }
        return 10000;
    }

    RecruitedAllyCombatSupport fromRecord(const Player& player, const PlayerJournalRecord& record)
    {
        RecruitedAllyCombatSupport ally;
        ally.name = record.key.empty() ? record.label : record.key;
        ally.race = stringField(record.label, "race", "Inconnue");
        ally.job = stringField(record.label, "profil", "profil libre");
        ally.trait = stringField(record.label, "trait", "trait non relu");
        ally.motivation = stringField(record.label, "motivation", "contrat");
        ally.recruitedLevel = std::max(1, intField(record.label, "niveau_recrutement", std::max(1, player.getLevel() - 2)));
        const int gained = std::min(18, std::max(0, record.count - 1) * 2 + static_cast<int>(stableHash(ally.name + ":combat_growth") % 5u));
        const int retainedEstimatedLevel = std::max(ally.recruitedLevel, std::min(100, ally.recruitedLevel + gained));
        int experienceEstimatedLevel = ally.recruitedLevel;
        int remainingRecruitXp = std::max(0, journalCount(player, "xp_recrues", ally.name));
        while (experienceEstimatedLevel < 100)
        {
            const int required = Level::getExperienceRequiredForNextLevel(experienceEstimatedLevel);
            if (required <= 0 || remainingRecruitXp < required) break;
            remainingRecruitXp -= required;
            ++experienceEstimatedLevel;
        }
        ally.estimatedLevel = std::max(retainedEstimatedLevel, experienceEstimatedLevel);
        ally.originRankIndex = std::clamp(intField(record.label, "rang_origine_index", rankIndexFromLevel(ally.recruitedLevel)), 0, 6);

        int promotionCount = 0;
        const std::string prefix = ally.name + ":";
        for (const PlayerJournalRecord& promotion : player.getCanonicalJournalRecords())
        {
            if (promotion.category == "rangs_recrues_obtenus" && promotion.key.rfind(prefix, 0) == 0 && promotion.count > 0)
            {
                promotionCount += promotion.count;
            }
        }
        ally.currentRankIndex = std::max(ally.originRankIndex, rankIndexFromLevel(ally.estimatedLevel));
        ally.currentRankIndex = std::clamp(ally.currentRankIndex + promotionCount, ally.originRankIndex, 6);
        const int baseShare = std::clamp(intField(record.label, "part_base", 12 + ally.currentRankIndex * 4), 6, 40);
        const int earlyDiscount = std::max(0, ally.currentRankIndex - ally.originRankIndex) * 2;
        const int storedDiscount = intField(record.label, "remise_fidelite", std::max(0, 7 - ally.originRankIndex));
        ally.requestedShare = std::clamp(baseShare - std::clamp(storedDiscount + earlyDiscount, 0, 14), 6, 40);
        ally.maxHp = std::max(18, 34 + ally.estimatedLevel * 3 + ally.currentRankIndex * 15);

        std::string profile = ally.job + " " + ally.trait;
        std::transform(profile.begin(), profile.end(), profile.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        if (profile.find("gardien") != std::string::npos || profile.find("tank") != std::string::npos) ally.maxHp = ally.maxHp * 120 / 100;
        if (profile.find("soigneur") != std::string::npos || profile.find("mage d'appui") != std::string::npos) ally.maxHp = ally.maxHp * 95 / 100;

        const int defaultPotions = std::clamp(1 + ally.currentRankIndex / 2 + (profile.find("intendant") != std::string::npos ? 1 : 0), 1, 4);
        const std::string vitalsLabel = journalLabel(player, "pv_recrues_persistants", ally.name, "");
        ally.currentHp = std::clamp(intField(vitalsLabel, "hp", std::max(1, ally.maxHp * initialHealthPercent(ally.name) / 100)), 0, ally.maxHp);
        ally.healingPotionCharges = std::clamp(intField(vitalsLabel, "potions", defaultPotions), 0, 5);

        const std::string equipmentLabel = journalLabel(player, "equipement_recrues", ally.name, "");
        const int naturalQuality = std::clamp(1 + ally.currentRankIndex + ally.estimatedLevel / 28, 1, 9);
        ally.weaponQuality = std::clamp(intField(equipmentLabel, "arme", naturalQuality), 1, 10);
        ally.armorQuality = std::clamp(intField(equipmentLabel, "armure", std::max(1, naturalQuality - 1)), 1, 10);
        ally.supportKitQuality = std::clamp(intField(equipmentLabel, "kit", profile.find("soigneur") != std::string::npos ? 2 : 1), 0, 10);
        RecruitedAllyConditionSystem::applyToCombatSupport(player, ally);
        return ally;
    }

    bool isActuallyRecruited(const Player& player, const PlayerJournalRecord& record, const std::string& name)
    {
        // Anciennes sauvegardes : avant profil_seulement=1, un profil retenu était déjà un membre réel.
        const bool profileOnly = intField(record.label, "profil_seulement", 0) != 0;
        return !profileOnly || journalCount(player, "recrues_engagees", name) > 0;
    }

    struct RosterCandidate
    {
        RecruitedAllyCombatSupport ally;
        bool equipped = false;
    };

    bool rosterOrderBefore(const Player& player, const RosterCandidate& a, const RosterCandidate& b)
    {
        const int manualA = manualOrderIndex(player, a.ally.name);
        const int manualB = manualOrderIndex(player, b.ally.name);
        if (manualA != manualB) return manualA < manualB;
        if (a.ally.currentRankIndex != b.ally.currentRankIndex) return a.ally.currentRankIndex > b.ally.currentRankIndex;
        if (a.ally.estimatedLevel != b.ally.estimatedLevel) return a.ally.estimatedLevel > b.ally.estimatedLevel;
        return a.ally.name < b.ally.name;
    }

    std::vector<RosterCandidate> collectAvailableTeamCandidates(const Player& player)
    {
        std::vector<RosterCandidate> candidates;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "pnj_recrutables_retenus" || record.count <= 0) continue;
            const std::string name = record.key.empty() ? record.label : record.key;
            if (name.empty()
                || journalCount(player, "recrues_renvoyees", name) > 0
                || journalCount(player, "recrues_departies", name) > 0) continue;
            if (!isActuallyRecruited(player, record, name)) continue;
            if (isInInfirmary(player, name) || isAwaitingTransfer(player, name)) continue;

            RosterCandidate candidate;
            candidate.ally = fromRecord(player, record);
            candidate.equipped = (journalCount(player, "recrues_equipees_toggle", name) % 2) == 1;
            candidates.push_back(candidate);
        }

        std::stable_sort(candidates.begin(), candidates.end(), [&](const RosterCandidate& a, const RosterCandidate& b) {
            if (a.equipped != b.equipped) return a.equipped > b.equipped;
            return rosterOrderBefore(player, a, b);
        });

        // Sécurité de compatibilité : une vieille sauvegarde peut théoriquement contenir davantage de membres.
        // On n'efface personne du journal, mais jamais plus de 5 recrues ne voyagent avec le joueur à la fois.
        if (candidates.size() > static_cast<std::size_t>(RecruitedAllyCombatRoster::MaxRecruitedTeamMembers))
        {
            candidates.resize(static_cast<std::size_t>(RecruitedAllyCombatRoster::MaxRecruitedTeamMembers));
        }
        return candidates;
    }

}

namespace RecruitedAllyCombatRoster
{
    int normalizeShares(std::vector<RecruitedAllyCombatSupport>& allies)
    {
        std::vector<std::size_t> eligible;
        for (std::size_t i = 0; i < allies.size(); ++i)
        {
            allies[i].normalizedShare = 0;
            if (!allies[i].temporaryNoReward) eligible.push_back(i);
        }
        if (eligible.empty()) return 100;

        const int groupSize = static_cast<int>(eligible.size()) + 1;
        int playerShare = groupSize <= 1 ? 100 : std::clamp(100 / groupSize + 10, 30, 65);
        int maxCap = groupSize >= 5 ? 22 : groupSize == 4 ? 25 : groupSize == 3 ? 30 : groupSize == 2 ? 35 : 0;
        const int perAllyCap = std::min(maxCap, std::max(6, playerShare - 1));
        const int available = std::max(0, 100 - playerShare);
        int totalWeight = 0;
        std::vector<int> weights;
        for (std::size_t index : eligible)
        {
            const auto& ally = allies[index];
            const int weight = std::clamp(ally.requestedShare + ally.currentRankIndex * 3 + ally.estimatedLevel / 8, 5, perAllyCap * 2);
            weights.push_back(weight);
            totalWeight += weight;
        }
        int assigned = 0;
        for (std::size_t i = 0; i < eligible.size(); ++i)
        {
            int share = totalWeight <= 0 ? available / static_cast<int>(eligible.size()) : (weights[i] * available) / totalWeight;
            share = std::clamp(share, eligible.size() <= 1 ? 0 : 5, perAllyCap);
            allies[eligible[i]].normalizedShare = share;
            assigned += share;
        }
        int safety = 0;
        while (assigned < available && safety++ < 200)
        {
            bool changed = false;
            for (std::size_t index : eligible)
            {
                if (assigned >= available) break;
                auto& ally = allies[index];
                if (ally.normalizedShare < perAllyCap)
                {
                    ++ally.normalizedShare;
                    ++assigned;
                    changed = true;
                }
            }
            if (!changed) break;
        }
        playerShare = 100 - assigned;
        auto biggest = std::max_element(eligible.begin(), eligible.end(), [&](std::size_t a, std::size_t b) {
            return allies[a].normalizedShare < allies[b].normalizedShare;
        });
        if (biggest != eligible.end() && playerShare <= allies[*biggest].normalizedShare)
        {
            int needed = allies[*biggest].normalizedShare + 1 - playerShare;
            for (std::size_t index : eligible)
            {
                auto& ally = allies[index];
                if (needed <= 0) break;
                if (ally.normalizedShare > 5)
                {
                    const int taken = std::min(needed, ally.normalizedShare - 5);
                    ally.normalizedShare -= taken;
                    playerShare += taken;
                    needed -= taken;
                }
            }
        }
        return std::clamp(playerShare, 0, 100);
    }

    std::vector<RecruitedAllyCombatSupport> collectActive(const Player& player)
    {
        std::vector<RecruitedAllyCombatSupport> active;
        if (HestiaCompanionSystem::isActive(player))
        {
            // Hestia is a special companion, not a salaried recruit. She still occupies one physical
            // active slot so keeping her around never creates a free third combatant.
            active.push_back(HestiaCompanionSystem::toCombatSupport(player));
        }

        for (const RosterCandidate& candidate : collectAvailableTeamCandidates(player))
        {
            if (!candidate.equipped) continue;
            active.push_back(candidate.ally);
            if (active.size() >= static_cast<std::size_t>(MaxActiveRecruitsWithPlayer)) break;
        }
        normalizeShares(active);
        return active;
    }

    std::vector<RecruitedAllyCombatSupport> collectReserve(const Player& player)
    {
        const std::vector<RosterCandidate> candidates = collectAvailableTeamCandidates(player);
        std::vector<std::string> activeNames;
        const int specialSlots = HestiaCompanionSystem::isActive(player) ? 1 : 0;
        const int recruitSlots = std::max(0, MaxActiveRecruitsWithPlayer - specialSlots);
        for (const RosterCandidate& candidate : candidates)
        {
            if (!candidate.equipped) continue;
            activeNames.push_back(candidate.ally.name);
            if (static_cast<int>(activeNames.size()) >= recruitSlots) break;
        }

        std::vector<RecruitedAllyCombatSupport> reserve;
        for (const RosterCandidate& candidate : candidates)
        {
            if (std::find(activeNames.begin(), activeNames.end(), candidate.ally.name) != activeNames.end()) continue;
            reserve.push_back(candidate.ally);
        }
        return reserve;
    }

    bool jobContains(const RecruitedAllyCombatSupport& ally, const std::string& token)
    {
        std::string text = ally.job + " " + ally.trait;
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return text.find(token) != std::string::npos;
    }

    int maturityScore(const RecruitedAllyCombatSupport& ally)
    {
        return std::clamp(ally.currentRankIndex * 16 + ally.estimatedLevel / 4 + ally.weaponQuality + ally.armorQuality + ally.supportKitQuality, 0, 140);
    }

    int maturityTier(const RecruitedAllyCombatSupport& ally)
    {
        const int score = maturityScore(ally);
        if (score >= 92) return 5;
        if (score >= 72) return 4;
        if (score >= 52) return 3;
        if (score >= 34) return 2;
        if (score >= 18) return 1;
        return 0;
    }

    std::string maturityName(const RecruitedAllyCombatSupport& ally)
    {
        switch (maturityTier(ally))
        {
            case 0: return "débutant";
            case 1: return "apprenti";
            case 2: return "habitué";
            case 3: return "aguerri";
            case 4: return "vétéran";
            default: return "maître de terrain";
        }
    }
    int relationshipScore(const Player& player, const std::string& recruitName)
    {
        int score = 0;
        score += std::min(40, journalCount(player, "participation_recrues", recruitName) * 4);
        score += std::min(24, journalCount(player, "quetes_communes_recrues", recruitName) * 8);
        score += std::min(12, journalCount(player, "semaines_actives_recrues", recruitName) * 2);
        if (journalCount(player, "quetes_personnelles_recrues_terminees", recruitName) > 0) score += 24;
        if (journalCount(player, "soins_relation_recrues", recruitName) > 0) score += 6;
        if (journalCount(player, "sauvetages_relation_recrues", recruitName) > 0) score += 10;
        score -= std::min(36, journalCount(player, "salaires_recrues_impayes", recruitName) * 12);
        return std::clamp(score, 0, 100);
    }

    void markReserveEntryDelay(RecruitedAllyCombatSupport& ally)
    {
        ally.enteredFromReserveThisPhase = true;
    }

    bool consumeReserveEntryDelay(RecruitedAllyCombatSupport& ally)
    {
        if (!ally.enteredFromReserveThisPhase) return false;
        ally.enteredFromReserveThisPhase = false;
        return true;
    }

    void recordPostKoObservation(RecruitedAllyCombatSupport& ally, int round, bool fromReserve)
    {
        if (ally.currentHp <= 0 || round <= 0) return;
        if (ally.postKoFirstObservedRound <= 0) ally.postKoFirstObservedRound = round;
        ally.postKoLastObservedRound = std::max(ally.postKoLastObservedRound, round);
        if (fromReserve)
        {
            ally.postKoObservedFromReserve = true;
            ++ally.postKoObservedRoundsFromReserve;
        }
    }

    void recordPostKoObservedDefeats(RecruitedAllyCombatSupport& ally, int defeatedCount)
    {
        if (ally.currentHp <= 0 || defeatedCount <= 0 || ally.postKoFirstObservedRound <= 0) return;
        ally.postKoObservedDefeats += defeatedCount;
    }

    void recordPostKoFinalOutcome(RecruitedAllyCombatSupport& ally)
    {
        if (ally.currentHp <= 0 || ally.postKoFirstObservedRound <= 0) return;
        ally.postKoObservedFinalOutcome = true;
        ally.postKoAvailableForDebrief = true;
    }

    bool hasPostKoObservation(const RecruitedAllyCombatSupport& ally)
    {
        return ally.postKoFirstObservedRound > 0
            && ally.postKoLastObservedRound >= ally.postKoFirstObservedRound;
    }

}
