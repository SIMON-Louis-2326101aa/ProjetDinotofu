#include "combat/ally/RecruitedAllyBossSupport.hpp"
#include "combat/ally/RecruitedAllyActionPlanner.hpp"
#include "combat/ally/RecruitedAllyInitiativeSystem.hpp"
#include "combat/ally/RecruitedAllyConditionSystem.hpp"
#include "combat/group/EmployerTeamRosterSystem.hpp"
#include "combat/group/CombatReserveSpilloverSystem.hpp"
#include "combat/group/CombatLeaderRotationSystem.hpp"
#include "story/HestiaCompanionSystem.hpp"

#include "entity/Boss.hpp"
#include "entity/Player.hpp"
#include "core/Random.hpp"

#include <algorithm>
#include <sstream>

namespace
{
    void persistOne(Player& player, const RecruitedAllyCombatSupport& ally, const std::string& status)
    {
        if (ally.name.empty() || ally.temporaryNoReward) return;
        if (ally.teamOrigin == AllyTeamOrigin::EmployerTeam)
        {
            EmployerTeamRosterSystem::setMemberVitals(
                player,
                ally.persistentMemberId,
                ally.currentHp,
                ally.currentHp > 0,
                status.empty() ? "operationnel" : status
            );
            return;
        }
        std::ostringstream label;
        label << ally.name
              << " | hp=" << std::clamp(ally.currentHp, 0, std::max(1, ally.maxHp))
              << " | max=" << std::max(1, ally.maxHp)
              << " | potions=" << std::clamp(ally.healingPotionCharges, 0, 9)
              << " | statut=" << (status.empty() ? "stable" : status)
              << " | jour=" << player.getWorldDaysElapsed();
        player.recordCanonicalEvent("pv_recrues_persistants", ally.name, label.str(), 1);
    }

    void markFallen(Player& player, const RecruitedAllyCombatSupport& ally, const Boss& boss, std::vector<std::string>& lines)
    {
        if (ally.persistentMemberId == "special:hestia")
        {
            lines.push_back("- Hestia s'effondre sous la pression de " + boss.getName() + ". Elle reste une compagne spéciale : aucun faux séjour de recrue salariée n'est créé.");
            return;
        }
        if (ally.teamOrigin == AllyTeamOrigin::EmployerTeam)
        {
            const int readyDay = player.getWorldDaysElapsed() + 3;
            EmployerTeamRosterSystem::setMemberVitals(
                player, ally.persistentMemberId, 0, false,
                "ko_boss_employeur | ready_day=" + std::to_string(readyDay));
            player.recordCanonicalEvent(
                "team_employer_members_fallen",
                ally.persistentMemberId,
                ally.name + " tombe face au boss " + boss.getName() + " | retour_estime_jour=" + std::to_string(readyDay),
                1
            );
            lines.push_back("- " + ally.name + " tombe à 0 PV face à " + boss.getName() + ". Son organisation devra gérer son évacuation.");
            return;
        }
        persistOne(player, ally, "ko_a_evacuer");
        std::ostringstream label;
        label << ally.name
              << " | gravite=grave"
              << " | ville=" << player.getCurrentCityId()
              << " | jour_chute=" << player.getWorldDaysElapsed()
              << " | repos_base=3"
              << " | sortie_pct=35"
              << " | origine=boss:" << boss.getBossId();
        player.recordCanonicalEvent("recrues_a_evacuer", ally.name, label.str(), 1);
        player.recordCanonicalEvent("recrues_tombees_combat", ally.name, ally.name + " tombe face au boss " + boss.getName(), 1);
        lines.push_back("- " + ally.name + " tombe à 0 PV face à " + boss.getName() + ". Elle devra être évacuée vers l'infirmerie.");
    }

    void mergeParticipant(std::vector<RecruitedAllyCombatSupport>& participants, const RecruitedAllyCombatSupport& ally)
    {
        if (ally.name.empty() || (ally.turnsTaken <= 0 && !RecruitedAllyCombatRoster::hasPostKoObservation(ally))) return;
        auto existing = std::find_if(participants.begin(), participants.end(), [&](const auto& current) {
            return current.name == ally.name;
        });
        if (existing == participants.end())
        {
            participants.push_back(ally);
            return;
        }
        if (ally.turnsTaken >= existing->turnsTaken)
        {
            *existing = ally;
        }
    }
    int healthPercent(const RecruitedAllyCombatSupport& ally)
    {
        return ally.maxHp > 0 ? std::clamp(ally.currentHp * 100 / ally.maxHp, 0, 100) : 0;
    }

    RecruitedAllyCombatSupport* mostWoundedAlly(
        std::vector<RecruitedAllyCombatSupport>& allies,
        const std::string& exceptName)
    {
        RecruitedAllyCombatSupport* result = nullptr;
        for (auto& current : allies)
        {
            if (current.name == exceptName || current.currentHp <= 0) continue;
            if (result == nullptr || healthPercent(current) < healthPercent(*result)) result = &current;
        }
        return result;
    }

    void executeBossPlannedAction(
        Player& player,
        Boss& boss,
        std::vector<RecruitedAllyCombatSupport>& allies,
        RecruitedAllyCombatSupport& ally,
        Random& random,
        bool leaderPresent,
        bool allowBossDefeat,
        std::vector<std::string>& lines)
    {
        if (ally.persistentMemberId == "special:hestia")
        {
            if (ally.turnsTaken == 1) HestiaCompanionSystem::recordCombatParticipation(player);
            const HestiaCompanionProfile hestiaProfile = HestiaCompanionSystem::profile(player);
            const bool magicalAction = HestiaCompanionSystem::looksMagicOriented(hestiaProfile);

            HestiaAwakeningCombatResult awakening = HestiaCompanionSystem::tryAwakening(player, random, magicalAction);
            if (awakening.triggered && !boss.isDead())
            {
                int damage = std::max(1, awakening.bonusDamage);
                if (!allowBossDefeat) damage = std::min(damage, std::max(0, boss.getHp() - 1));
                const int before = boss.getHp();
                if (damage > 0) boss.takeDamage(damage);
                const int effective = std::max(0, before - boss.getHp());
                ally.damageDealt += effective;
                if (boss.isDead()) ++ally.finishBlows;
                lines.push_back("- Hestia libère une magie lumineuse impossible à expliquer : " + boss.getName() + " subit " + std::to_string(effective) + " dégâts.");
                lines.insert(lines.end(), awakening.lines.begin(), awakening.lines.end());
                if (awakening.returnsToHeavens)
                {
                    ally.currentHp = 0;
                    lines.push_back("- Hestia retrouve tous ses souvenirs et sa matérialisation quitte le combat pour retourner aux cieux.");
                }
                return;
            }

            const HestiaCombatMishap mishap = HestiaCompanionSystem::rollMishap(player, random, magicalAction);
            if (mishap.occurred)
            {
                lines.push_back("- " + mishap.line);
                if (mishap.mayHitLeader && player.getHp() > 1 && mishap.maxFriendlyDamagePercent > 0)
                {
                    const int requested = std::max(1, player.getMaxHp() * mishap.maxFriendlyDamagePercent / 100);
                    const int damage = std::min(requested, std::max(0, player.getHp() - 1));
                    if (damage > 0)
                    {
                        player.takeDamage(damage);
                        lines.push_back("  L'erreur touche le joueur : -" + std::to_string(damage) + " PV, sans pouvoir l'achever directement.");
                    }
                }
                else if (mishap.mayHitAlly && mishap.maxFriendlyDamagePercent > 0)
                {
                    RecruitedAllyCombatSupport* friendly = mostWoundedAlly(allies, ally.name);
                    if (friendly != nullptr && friendly->currentHp > 1)
                    {
                        const int requested = std::max(1, friendly->maxHp * mishap.maxFriendlyDamagePercent / 100);
                        const int damage = std::min(requested, std::max(0, friendly->currentHp - 1));
                        friendly->currentHp -= damage;
                        lines.push_back("  " + friendly->name + " encaisse accidentellement " + std::to_string(damage) + " dégâts, sans pouvoir être mis KO par cette maladresse.");
                    }
                }
                return;
            }
        }

        RecruitedAllyCombatSupport* wounded = mostWoundedAlly(allies, ally.name);
        RecruitedAllyActionContext context;
        context.hasEnemy = !boss.isDead();
        context.enemyCount = boss.isDead() ? 0 : 1;
        context.bossContext = true;
        context.leaderPresent = leaderPresent && !player.isDead();
        context.leaderHpPercent = player.getMaxHp() > 0 ? std::clamp(player.getHp() * 100 / player.getMaxHp(), 0, 100) : 0;
        context.leaderCanBeHealed = context.leaderPresent;
        context.woundedAllyAvailable = wounded != nullptr;
        context.woundedAllyHpPercent = wounded != nullptr ? healthPercent(*wounded) : 100;
        context.techniqueAvailable = ally.activeSkillCooldown <= 0;
        context.techniqueAllowed = true;
        context.dangerPercent = boss.isUltimateActive() ? 90 : 65;
        context.relationshipScore = RecruitedAllyCombatRoster::relationshipScore(player, ally.name);

        const RecruitedAllyActionDecision decision = RecruitedAllyActionPlanner::decide(ally, context, random);

        if (decision.type == RecruitedAllyActionType::SelfHeal)
        {
            const int amount = std::max(5, ally.maxHp / 3);
            const int before = ally.currentHp;
            ally.currentHp = std::min(ally.maxHp, ally.currentHp + amount);
            --ally.healingPotionCharges;
            ally.healingDone += std::max(0, ally.currentHp - before);
            ++ally.supportActions;
            lines.push_back("- " + ally.name + " se stabilise : +" + std::to_string(std::max(0, ally.currentHp - before)) + " PV.");
            return;
        }

        if (decision.type == RecruitedAllyActionType::HealLeader && context.leaderPresent)
        {
            const int heal = std::max(2, 4 + ally.estimatedLevel / 12 + ally.currentRankIndex * 2 + ally.supportKitQuality * 2);
            const int before = player.getHp();
            player.heal(heal);
            const int effective = std::max(0, player.getHp() - before);
            ally.healingDone += effective;
            ++ally.supportActions;
            lines.push_back("- " + ally.name + " soutient " + player.getName() + " : +" + std::to_string(effective) + " PV.");
            return;
        }

        if (decision.type == RecruitedAllyActionType::HealAlly && wounded != nullptr)
        {
            const int heal = std::max(3, 4 + ally.estimatedLevel / 12 + ally.currentRankIndex * 2 + ally.supportKitQuality * 2);
            const int before = wounded->currentHp;
            wounded->currentHp = std::min(wounded->maxHp, wounded->currentHp + heal);
            const int effective = std::max(0, wounded->currentHp - before);
            ally.healingDone += effective;
            ++ally.supportActions;
            lines.push_back("- " + ally.name + " stabilise " + wounded->name + " : +" + std::to_string(effective) + " PV.");
            return;
        }

        int damage = 4 + ally.estimatedLevel / 3 + ally.currentRankIndex * 3 + ally.weaponQuality * 2;
        if (RecruitedAllyActionPlanner::isGuardProfile(ally)) damage = damage * 88 / 100;
        if (RecruitedAllyCombatRoster::jobContains(ally, "archer") || RecruitedAllyCombatRoster::jobContains(ally, "assassin")) damage += 3;

        if (decision.type == RecruitedAllyActionType::Technique)
        {
            damage = damage * (RecruitedAllyActionPlanner::isGuardProfile(ally) ? 110 : (RecruitedAllyActionPlanner::isSupportProfile(ally) ? 105 : 140)) / 100;
            ally.activeSkillCooldown = std::max(2, 4 - std::min(2, ally.currentRankIndex / 2));
            if (RecruitedAllyActionPlanner::isGuardProfile(ally))
            {
                ally.protectedThisRound = true;
                ++ally.supportActions;
            }
            if (RecruitedAllyActionPlanner::isSupportProfile(ally)) ++ally.supportActions;
        }
        else if (decision.type == RecruitedAllyActionType::Guard)
        {
            ally.protectedThisRound = true;
            ++ally.supportActions;
            damage = damage * 72 / 100;
        }

        damage = std::max(2, damage * random.between(86, 112) / 100);
        if (!allowBossDefeat) damage = std::min(damage, std::max(0, boss.getHp() - 1));
        if (damage <= 0) return;

        const int before = boss.getHp();
        boss.takeDamage(damage);
        const int effective = std::max(0, before - boss.getHp());
        ally.damageDealt += effective;
        if (boss.isDead()) ++ally.finishBlows;
        lines.push_back("- " + ally.name + " utilise " + RecruitedAllyActionPlanner::actionTypeLabel(decision.type)
            + " contre " + boss.getName() + " : " + std::to_string(effective) + " dégâts.");
    }

}

auto recruitedAllyBossPresenceLines(const std::vector<RecruitedAllyCombatSupport>& allies) -> std::vector<std::string>
{
    std::vector<std::string> lines;
    if (allies.empty()) return lines;
    const bool employerTeam = std::any_of(allies.begin(), allies.end(), [](const RecruitedAllyCombatSupport& ally) {
        return ally.teamOrigin == AllyTeamOrigin::EmployerTeam;
    });
    lines.push_back(employerTeam
        ? "Escouade employeuse engagée dans ce boss : " + std::to_string(allies.size()) + " collègue(s) actif(s) avec le joueur."
        : "Recrues équipées engagées dans ce boss : " + std::to_string(allies.size()) + "/2.");
    for (const auto& ally : allies)
    {
        lines.push_back(
            "- " + ally.name + " | " + ally.job
            + " | maturité " + RecruitedAllyCombatRoster::maturityName(ally)
            + " | PV " + std::to_string(ally.currentHp) + "/" + std::to_string(ally.maxHp)
            + (employerTeam ? " | salarié par l'organisation" : " | part " + std::to_string(ally.normalizedShare) + "%")
        );
    }
    lines.push_back("Les recrues aident réellement, mais elles peuvent être blessées ou tomber. Les mécaniques uniques du boss restent prioritaires.");
    return lines;
}

bool tryEmployerBossLeaderRotation(
    Player& player,
    std::vector<RecruitedAllyCombatSupport>& allies,
    std::vector<RecruitedAllyCombatSupport>& reserveAllies,
    Random& random,
    std::vector<std::string>& lines)
{
    if (reserveAllies.empty()) return false;
    auto leaderIt = std::find_if(allies.begin(), allies.end(), [](const auto& ally) {
        return ally.teamOrigin == AllyTeamOrigin::EmployerTeam && ally.teamLeader && ally.currentHp > 0;
    });
    if (leaderIt == allies.end()) return false;

    auto outgoingIt = allies.end();
    int lowHp = 0;
    int critical = 0;
    for (auto it = allies.begin(); it != allies.end(); ++it)
    {
        if (it->teamLeader || it->currentHp <= 0) continue;
        const int hp = healthPercent(*it);
        if (hp <= 45) ++lowHp;
        if (hp <= 24) ++critical;
        if (outgoingIt == allies.end() || hp < healthPercent(*outgoingIt)) outgoingIt = it;
    }
    if (outgoingIt == allies.end() || lowHp <= 0) return false;

    auto incomingIt = std::max_element(reserveAllies.begin(), reserveAllies.end(), [](const auto& a, const auto& b) {
        return healthPercent(a) < healthPercent(b);
    });
    if (incomingIt == reserveAllies.end() || incomingIt->currentHp <= 0) return false;

    int leaderLoyalty = 65;
    int outgoingLoyalty = 60;
    for (const EmployerTeamMember& member : EmployerTeamRosterSystem::rosterFor(player))
    {
        if (member.memberId == leaderIt->persistentMemberId) leaderLoyalty = member.loyalty;
        if (member.memberId == outgoingIt->persistentMemberId) outgoingLoyalty = member.loyalty;
    }

    CombatLeaderRotationContext context;
    context.leaderPresent = true;
    context.activeCount = static_cast<int>(allies.size());
    context.reserveCount = static_cast<int>(reserveAllies.size());
    context.lowHpActiveCount = lowHp;
    context.criticalHpActiveCount = critical;
    context.leaderIntelligence = std::clamp(48 + leaderIt->estimatedLevel / 2 + leaderIt->currentRankIndex * 5, 35, 95);
    context.groupSolidarity = std::clamp((leaderLoyalty + outgoingLoyalty) / 2, 20, 95);
    context.roundNumber = std::max(1, leaderIt->turnsTaken + 1);
    const CombatLeaderRotationDecision decision = CombatLeaderRotationSystem::decide(context, random);
    if (!decision.attempt) return false;

    const int refusalHp = std::min(healthPercent(*outgoingIt), healthPercent(*incomingIt));
    const std::string stubbornText = outgoingIt->trait + " " + outgoingIt->job + " " + incomingIt->trait + " " + incomingIt->job;
    const bool stubborn = stubbornText.find("berser") != std::string::npos || stubbornText.find("fanati") != std::string::npos;
    if (CombatLeaderRotationSystem::refusesRotation(refusalHp, outgoingLoyalty, stubborn, random))
    {
        lines.push_back("- " + leaderIt->name + " tente une rotation face au boss, mais le changement est refusé. Son tour reste disponible ; il ne retentera pas immédiatement.");
        return false;
    }

    const std::string outgoingName = outgoingIt->name;
    const std::string incomingName = incomingIt->name;
    std::swap(*outgoingIt, *incomingIt);
    RecruitedAllyCombatRoster::markReserveEntryDelay(*outgoingIt);
    incomingIt->enteredFromReserveThisPhase = false;
    ++leaderIt->turnsTaken;
    if (leaderIt->activeSkillCooldown > 0) --leaderIt->activeSkillCooldown;
    player.recordCanonicalEvent(
        "rotations_tactiques_employeur_boss",
        leaderIt->persistentMemberId,
        leaderIt->name + " sacrifie son tour de chef face au boss pour sortir " + outgoingName + " et faire entrer " + incomingName + ".",
        1);
    lines.push_back("- " + leaderIt->name + " sacrifie son tour : " + outgoingName + " recule et " + incomingName + " entre depuis la réserve.");
    lines.push_back("  La rotation protège un membre blessé sans attendre sa chute ; aucune phase du boss n'est sautée.");
    return true;
}

bool tryEmployerBossLeaderRecallPlayer(
    Player& player,
    std::vector<RecruitedAllyCombatSupport>& allies,
    std::vector<RecruitedAllyCombatSupport>& reserveAllies,
    Random& random,
    int roundNumber,
    std::vector<std::string>& lines)
{
    if (allies.size() <= static_cast<std::size_t>(RecruitedAllyCombatRoster::MaxActiveRecruitsWithPlayer))
    {
        return false;
    }

    auto leaderIt = std::find_if(allies.begin(), allies.end(), [](const auto& ally) {
        return ally.teamOrigin == AllyTeamOrigin::EmployerTeam && ally.teamLeader && ally.currentHp > 0;
    });
    if (leaderIt == allies.end()) return false;

    std::size_t outgoingIndex = allies.size();
    int lowHp = 0;
    int critical = 0;
    for (std::size_t index = 0; index < allies.size(); ++index)
    {
        const auto& ally = allies[index];
        if (ally.teamLeader || ally.currentHp <= 0) continue;
        const int hp = healthPercent(ally);
        if (hp <= 45) ++lowHp;
        if (hp <= 24) ++critical;
        if (outgoingIndex == allies.size() || hp < healthPercent(allies[outgoingIndex]))
        {
            outgoingIndex = index;
        }
    }
    if (outgoingIndex >= allies.size()) return false;

    int leaderLoyalty = 65;
    int outgoingLoyalty = 60;
    for (const EmployerTeamMember& member : EmployerTeamRosterSystem::rosterFor(player))
    {
        if (member.memberId == leaderIt->persistentMemberId) leaderLoyalty = member.loyalty;
        if (member.memberId == allies[outgoingIndex].persistentMemberId) outgoingLoyalty = member.loyalty;
    }

    CombatLeaderRotationContext context;
    context.leaderPresent = true;
    context.activeCount = static_cast<int>(allies.size());
    context.reserveCount = static_cast<int>(reserveAllies.size()) + 1; // Le joueur attend lui aussi en réserve.
    context.lowHpActiveCount = lowHp;
    context.criticalHpActiveCount = critical;
    context.playerWaiting = true;
    context.leaderIntelligence = std::clamp(48 + leaderIt->estimatedLevel / 2 + leaderIt->currentRankIndex * 5, 35, 95);
    context.groupSolidarity = std::clamp((leaderLoyalty + outgoingLoyalty) / 2, 20, 95);
    context.roundNumber = std::max(1, roundNumber);
    const CombatLeaderRotationDecision decision = CombatLeaderRotationSystem::decide(context, random);
    if (!decision.attempt) return false;

    const RecruitedAllyCombatSupport outgoing = allies[outgoingIndex];
    const bool stubborn = outgoing.trait.find("berser") != std::string::npos
        || outgoing.trait.find("fanati") != std::string::npos
        || outgoing.job.find("berser") != std::string::npos
        || outgoing.job.find("fanati") != std::string::npos;
    if (CombatLeaderRotationSystem::refusesRotation(healthPercent(outgoing), outgoingLoyalty, stubborn, random))
    {
        lines.push_back("- " + leaderIt->name + " veut te rappeler depuis la réserve, mais " + outgoing.name
            + " refuse de quitter la ligne. Le chef conserve son tour normal.");
        return false;
    }

    const std::string leaderName = leaderIt->name;
    const std::string leaderId = leaderIt->persistentMemberId;
    ++leaderIt->turnsTaken;
    if (leaderIt->activeSkillCooldown > 0) --leaderIt->activeSkillCooldown;

    RecruitedAllyCombatSupport movedToReserve = outgoing;
    movedToReserve.enteredFromReserveThisPhase = false;
    reserveAllies.push_back(movedToReserve);
    allies.erase(allies.begin() + static_cast<std::ptrdiff_t>(outgoingIndex));

    player.recordCanonicalEvent(
        "team_employer_reserve_entries",
        "boss_rotation_jour_" + std::to_string(player.getWorldDaysElapsed()),
        leaderName + " sacrifie son tour face au boss pour rappeler le joueur depuis la liste d'attente et faire reculer "
            + outgoing.name + ".",
        1);
    player.recordCanonicalEvent(
        "rotations_tactiques_employeur_boss",
        leaderId.empty() ? leaderName : leaderId,
        leaderName + " ouvre une place au joueur depuis la réserve face au boss.",
        1);

    lines.push_back("- " + leaderName + " sacrifie son tour et rappelle le joueur depuis la réserve.");
    lines.push_back("- " + outgoing.name + " recule : la troisième place principale est rendue au joueur.");
    lines.push_back("  Le rappel n'accorde aucune action gratuite : le joueur attendra la prochaine phase de son équipe avant d'agir.");
    return true;
}

void playRecruitedAlliesAgainstBoss(
    Player& player,
    Boss& boss,
    std::vector<RecruitedAllyCombatSupport>& allies,
    Random& random,
    std::vector<std::string>& lines,
    RecruitedAllyBossTurnPhase phase
)
{
    const std::vector<int> allyInitiativeOrder = RecruitedAllyInitiativeSystem::orderedActiveIndices(allies);
    for (int allyIndex : allyInitiativeOrder)
    {
        auto& ally = allies[static_cast<std::size_t>(allyIndex)];
        if (phase == RecruitedAllyBossTurnPhase::LeaderOnly && !ally.teamLeader) continue;
        if (phase == RecruitedAllyBossTurnPhase::NonLeaderOnly && ally.teamLeader) continue;
        if (ally.currentHp <= 0 || boss.isDead()) continue;
        if (RecruitedAllyCombatRoster::consumeReserveEntryDelay(ally))
        {
            lines.push_back("- " + ally.name + " vient d'entrer depuis la réserve : aucun tour gratuit n'est accordé pendant cette phase.");
            continue;
        }
        ++ally.turnsTaken;
        if (ally.activeSkillCooldown > 0) --ally.activeSkillCooldown;
        ally.protectedThisRound = false;
        executeBossPlannedAction(player, boss, allies, ally, random, true, true, lines);
    }
}

void applyBossPressureToRecruitedAllies(
    Player& player,
    const Boss& boss,
    std::vector<RecruitedAllyCombatSupport>& allies,
    std::vector<RecruitedAllyCombatSupport>& reserveAllies,
    Random& random,
    std::vector<std::string>& lines
)
{
    std::vector<std::size_t> alive;
    for (std::size_t i = 0; i < allies.size(); ++i)
    {
        if (allies[i].currentHp > 0) alive.push_back(i);
    }
    if (alive.empty()) return;

    // Les boss gardent leur attaque normale sur le joueur. Cette pression représente uniquement
    // les zones, éclats et déplacements dangereux du combat de boss ; elle reste donc bornée.
    if (random.between(1, 100) > 45) return;

    auto& ally = allies[alive[static_cast<std::size_t>(random.between(0, static_cast<int>(alive.size()) - 1))]];
    const int averageBossDamage = std::max(1, (boss.getMinDamage() + boss.getMaxDamage()) / 2);
    const int mitigation = 3 + ally.armorQuality * 2 + ally.currentRankIndex;
    const int pressure = std::max(2, averageBossDamage * random.between(24, 38) / 100 - mitigation);
    ally.currentHp = std::max(0, ally.currentHp - pressure);
    lines.push_back("- Pression de zone : " + ally.name + " subit " + std::to_string(pressure) + " dégâts (" + std::to_string(ally.currentHp) + "/" + std::to_string(ally.maxHp) + " PV).");
    if (ally.currentHp <= 0)
    {
        markFallen(player, ally, boss, lines);
    }

    // Seules les zones réellement assez larges atteignent la file : une pression locale ne traverse pas magiquement la formation.
    CombatReserveEffectReach reach = CombatReserveEffectReach::None;
    if (boss.isUltimateActive())
        reach = CombatReserveEffectReach::Massive;
    else if (averageBossDamage >= 34)
        reach = CombatReserveEffectReach::Wide;
    else if (averageBossDamage >= 22 && random.between(1, 100) <= 35)
        reach = CombatReserveEffectReach::Edge;

    if (reach != CombatReserveEffectReach::None && !reserveAllies.empty())
    {
        const CombatReserveSpillover spill = CombatReserveSpilloverSystem::roll(pressure, reach, random);
        const int maxTargets = std::min(
            static_cast<int>(reserveAllies.size()),
            CombatReserveSpilloverSystem::maxAffectedReserveTargets(reach));
        int touched = 0;
        for (int index = 0; index < maxTargets; ++index)
        {
            auto& reserve = reserveAllies[static_cast<std::size_t>(index)];
            if (reserve.currentHp <= 0) continue;
            reserve.currentHp = std::max(0, reserve.currentHp - spill.amount);
            ++touched;
            lines.push_back("  Réserve exposée : " + reserve.name + " subit " + std::to_string(spill.amount)
                + " dégâts indirects (" + std::to_string(spill.percent) + " % de la zone).");
            if (reserve.currentHp <= 0)
            {
                markFallen(player, reserve, boss, lines);
            }
        }
        if (touched > 0)
        {
            lines.push_back("  La zone est assez large pour déborder vers " + std::to_string(touched)
                + " membre(s) de la file, sans leur appliquer la pleine puissance.");
        }
    }
}

CombatReward splitBossRewardWithRecruitedAllies(
    Player& player,
    const CombatReward& reward,
    std::vector<RecruitedAllyCombatSupport>& allies,
    std::vector<std::string>& lines
)
{
    if (allies.empty()) return reward;
    const bool employerTeam = std::any_of(allies.begin(), allies.end(), [](const RecruitedAllyCombatSupport& ally) {
        return ally.teamOrigin == AllyTeamOrigin::EmployerTeam;
    });
    if (employerTeam)
    {
        lines.push_back("Les collègues de l'équipe employeuse sont payés par leur organisation : aucune part n'est prélevée sur ta récompense de boss.");
        for (auto& ally : allies)
        {
            if (ally.turnsTaken > 0)
            {
                const int activityScore = ally.damageDealt + ally.healingDone + ally.supportActions * 8 + ally.finishBlows * 15 + ally.turnsTaken * 3;
                EmployerTeamRosterSystem::recordMemberCombatProgress(
                    player, ally.persistentMemberId, std::max(1, activityScore / 12));
            }
            persistOne(player, ally, ally.currentHp > 0 ? "boss_employeur" : "ko_boss_employeur");
            lines.push_back("- " + ally.name + " : dégâts " + std::to_string(ally.damageDealt)
                + ", soutien " + std::to_string(ally.healingDone)
                + ", tours " + std::to_string(ally.turnsTaken) + ".");
        }
        return reward;
    }
    const int playerShare = RecruitedAllyCombatRoster::normalizeShares(allies);
    const int weekIndex = std::max(0, player.getWorldDaysElapsed() / 7);
    lines.push_back("Part économique du chef après participation des recrues : " + std::to_string(playerShare) + "%.");
    for (auto& ally : allies)
    {
        if (ally.turnsTaken <= 0)
        {
            persistOne(player, ally, ally.currentHp > 0 ? "boss_sans_action" : "ko_a_evacuer");
            continue;
        }
        const int economyUnits = std::max(0, reward.getEconomyUnits() * ally.normalizedShare / 100);
        const int activityScore = ally.damageDealt + ally.healingDone + ally.supportActions * 8 + ally.finishBlows * 15 + ally.turnsTaken * 3;
        const int xp = std::max(0, reward.getExperience() * std::max(5, activityScore) / std::max(100, activityScore + player.getLevel() * 8));
        player.recordCanonicalEvent("recrues_equipees_semaines", ally.name + ":semaine_" + std::to_string(weekIndex), ally.name + " a été équipée cette semaine", 1);
        if (economyUnits > 0)
            player.recordCanonicalEvent("parts_or_recrues", ally.name + ":semaine_" + std::to_string(weekIndex), ally.name + " reçoit une part de boss réel", economyUnits);
        if (xp > 0)
            player.recordCanonicalEvent("xp_recrues", ally.name, ally.name + " gagne de l'expérience sur un boss réel", xp);
        player.recordCanonicalEvent("participation_recrues", ally.name, ally.name + " participe à un boss réel", ally.turnsTaken);
        RecruitedAllyConditionSystem::recordCombatStrain(player, ally, true);
        persistOne(player, ally, ally.currentHp > 0 ? "boss_recompense" : "ko_a_evacuer");
        lines.push_back("- " + ally.name + " : part " + std::to_string(ally.normalizedShare) + "%, " + std::to_string(xp) + " XP, dégâts " + std::to_string(ally.damageDealt) + ", soutien " + std::to_string(ally.healingDone) + ".");
    }
    return reward.getModified(100, playerShare);
}

void persistBossRecruitedAllyVitals(Player& player, const std::vector<RecruitedAllyCombatSupport>& allies, const std::string& status)
{
    for (const auto& ally : allies) persistOne(player, ally, status);
}

RecruitedAllyBossPostKoOutcome simulateRecruitedAlliesAfterPlayerDownAgainstBoss(
    Player& player,
    Boss& boss,
    std::vector<RecruitedAllyCombatSupport>& allies,
    std::vector<RecruitedAllyCombatSupport>& reserveAllies,
    Random& random,
    bool allowBossDefeat,
    int roundOffset
)
{
    RecruitedAllyBossPostKoOutcome outcome;

    auto refill = [&]() {
        while (allies.size() < static_cast<std::size_t>(RecruitedAllyCombatRoster::MaxActiveRecruitsWithoutPlayer)
            && !reserveAllies.empty())
        {
            if (reserveAllies.front().currentHp <= 0)
            {
                mergeParticipant(outcome.participants, reserveAllies.front());
                reserveAllies.erase(reserveAllies.begin());
                continue;
            }
            allies.push_back(reserveAllies.front());
            reserveAllies.erase(reserveAllies.begin());
            ++outcome.reserveEntries;
        }
    };

    refill();
    if (allies.empty()) return outcome;

    const int maxRounds = std::clamp(3 + static_cast<int>(allies.size()) + static_cast<int>(reserveAllies.size()) / 2, 4, 8);
    for (int round = 1; round <= maxRounds && !allies.empty() && !boss.isDead(); ++round)
    {
        outcome.simulatedRounds = round;
        const int observedRound = roundOffset + round;
        const bool bossAliveAtRoundStart = !boss.isDead();
        for (RecruitedAllyCombatSupport& ally : allies)
        {
            RecruitedAllyCombatRoster::recordPostKoObservation(ally, observedRound, false);
        }
        // Boss arenas keep the reserve close enough to watch unless a future encounter explicitly
        // models an obstructed/remote waiting area. Watching is not the same as participating.
        for (RecruitedAllyCombatSupport& ally : reserveAllies)
        {
            RecruitedAllyCombatRoster::recordPostKoObservation(ally, observedRound, true);
        }
        if (boss.shouldUnlockUltimate()) boss.unlockUltimate();
        if (boss.canUseUltimate()) boss.activateUltimate();
        const bool ultimatePressure = boss.isUltimateActive();

        for (auto& ally : allies)
        {
            if (ally.currentHp <= 0 || boss.isDead()) continue;
            ++ally.turnsTaken;
            if (ally.activeSkillCooldown > 0) --ally.activeSkillCooldown;
            ally.protectedThisRound = false;
            std::vector<std::string> hiddenActionLines;
            executeBossPlannedAction(player, boss, allies, ally, random, false, allowBossDefeat, hiddenActionLines);
        }

        if (bossAliveAtRoundStart && boss.isDead())
        {
            for (RecruitedAllyCombatSupport& ally : allies)
            {
                RecruitedAllyCombatRoster::recordPostKoObservedDefeats(ally, 1);
            }
            for (RecruitedAllyCombatSupport& ally : reserveAllies)
            {
                RecruitedAllyCombatRoster::recordPostKoObservedDefeats(ally, 1);
            }
        }

        if (boss.isDead()) break;

        const int pressureTargets = ultimatePressure ? std::min(2, static_cast<int>(allies.size())) : 1;
        for (int pressureIndex = 0; pressureIndex < pressureTargets && !allies.empty(); ++pressureIndex)
        {
            if (!ultimatePressure && random.between(1, 100) > 62) continue;
            const int victimIndex = random.between(0, static_cast<int>(allies.size()) - 1);
            RecruitedAllyCombatSupport& victim = allies[static_cast<std::size_t>(victimIndex)];
            const int averageBossDamage = std::max(1, (boss.getMinDamage() + boss.getMaxDamage()) / 2);
            const int mitigation = 3 + victim.armorQuality * 2 + victim.currentRankIndex;
            const int percent = ultimatePressure ? random.between(52, 72) : random.between(30, 48);
            const int pressure = std::max(2, averageBossDamage * percent / 100 - mitigation);
            victim.currentHp = std::max(0, victim.currentHp - pressure);
        }

        for (auto it = allies.begin(); it != allies.end(); )
        {
            if (it->currentHp > 0)
            {
                ++it;
                continue;
            }
            std::vector<std::string> hiddenLines;
            it->postKoAvailableForDebrief = false;
            markFallen(player, *it, boss, hiddenLines);
            mergeParticipant(outcome.participants, *it);
            ++outcome.alliesFallen;
            it = allies.erase(it);
        }
        refill();

        if (ultimatePressure)
        {
            boss.reduceUltimate();
            if (!boss.isUltimateActive()) boss.resetUltimateCooldown();
        }
        else
        {
            boss.reduceUltimateCooldown();
        }
    }


    for (RecruitedAllyCombatSupport& ally : allies)
    {
        RecruitedAllyCombatRoster::recordPostKoFinalOutcome(ally);
    }
    for (RecruitedAllyCombatSupport& ally : reserveAllies)
    {
        RecruitedAllyCombatRoster::recordPostKoFinalOutcome(ally);
    }

    for (const auto& ally : allies) mergeParticipant(outcome.participants, ally);
    for (const auto& ally : reserveAllies) mergeParticipant(outcome.participants, ally);

    outcome.bossDefeated = boss.isDead();
    outcome.playerRecovered = !allies.empty() || std::any_of(reserveAllies.begin(), reserveAllies.end(), [](const auto& ally) { return ally.currentHp > 0; });
    for (const auto& ally : allies) persistOne(player, ally, "boss_post_ko");
    return outcome;
}
