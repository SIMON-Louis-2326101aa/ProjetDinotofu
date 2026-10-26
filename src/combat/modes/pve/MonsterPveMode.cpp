// EN: MonsterPveMode.cpp briefly defines this Dinotofu module and its responsibilities.
#include "combat/death/PlayerDeathResolver.hpp"
// FR: MonsterPveMode.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/modes/pve/MonsterPveMode.hpp"

#include "combat/ally/RecruitedAllyCombatRoster.hpp"
#include "combat/ally/RecruitedAllyAftermathSystem.hpp"
#include "combat/ally/RecruitedAllyInitiativeSystem.hpp"
#include "combat/ally/RecruitedAllyConditionSystem.hpp"
#include "story/HestiaCompanionSystem.hpp"
#include "combat/ally/RecruitedAllyOrderSafetySystem.hpp"
#include "combat/ally/RecruitedAllyActionPlanner.hpp"
#include "combat/death/LethalTeamRescueSystem.hpp"
#include "combat/death/LethalSpiritResistanceSystem.hpp"
#include "combat/modes/pve/PveWaveNarrativeSupport.hpp"

#include "combat/EnemyCombatQueue.hpp"
#include "combat/dialogue/EncounterDialogueSystem.hpp"
#include "combat/encounter/AdventurerGroupEncounter.hpp"
#include "combat/summon/SummonCombatSystem.hpp"
#include "combat/system/WaveCombatSystem.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "combat/system/ElementalAffinitySystem.hpp"
#include "combat/profile/MonsterBehaviorProfile.hpp"
#include "combat/flavor/MonsterFlavorCatalog.hpp"
#include "combat/rival/RivalEmergenceSystem.hpp"
#include "combat/reward/CombatReward.hpp"
#include "combat/reward/CombatRewardSystem.hpp"
#include "combat/loot/LootGenerator.hpp"
#include "combat/ally/DuoMasterySystem.hpp"
#include "combat/group/CombatGroupBuilder.hpp"
#include "combat/group/TeamLeadershipSystem.hpp"
#include "combat/group/TeamEmploymentSystem.hpp"
#include "combat/group/TeamEmploymentMissionSystem.hpp"
#include "combat/group/EmployerTeamRosterSystem.hpp"
#include "combat/group/EmployerTeamDisciplineSystem.hpp"
#include "combat/group/CombatLeaderRotationSystem.hpp"
#include "combat/group/CombatIndividualSwitchSystem.hpp"
#include "combat/group/CombatReserveSpilloverSystem.hpp"
#include "combat/group/EmployerTacticalOrderSystem.hpp"
#include "combat/group/CombatUnitKind.hpp"
#include "combat/group/InitiativeSystem.hpp"
#include "combat/group/TurnOrder.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"
#include "combat/TurnManager.hpp"
#include "interface/menu/potions/CombatPotionUtils.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/CombatDisplay.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/model/MenuScreen.hpp"
#include "economy/Money.hpp"

#include "combat/turn/wave/PlayerWaveCombatTurn.hpp"
#include "combat/turn/wave/MonsterWaveCombatTurn.hpp"

#include "progression/DifficultyRules.hpp"
#include "progression/death/DeathPenaltyResult.hpp"
#include "progression/death/DeathPenaltySystem.hpp"
#include "progression/blessing/BlessingSystem.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "core/Console.hpp"
#include "character/SpecialCharacterDialogueCatalog.hpp"
#include "character/relationship/SpecialCharacterGroupDialogueCatalog.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <numeric>
#include <cctype>
#include <map>
#include <set>
#include <sstream>

namespace
{



    // EN: recordWaveEncountersInBestiary declares or implements a focused behavior used by this module.
    // FR: recordWaveEncountersInBestiary déclare ou implémente un comportement précis utilisé par ce module.







    // EN: displaySpecialDefeatDialogues declares or implements a focused behavior used by this module.
    // FR: displaySpecialDefeatDialogues déclare ou implémente un comportement précis utilisé par ce module.

    // EN: displaySpecialVictoryDialogues declares or implements a focused behavior used by this module.
    // FR: displaySpecialVictoryDialogues déclare ou implémente un comportement précis utilisé par ce module.


    // EN: countDefeatedEvolvedMonsters declares or implements a focused behavior used by this module.
    // FR: countDefeatedEvolvedMonsters déclare ou implémente un comportement précis utilisé par ce module.
    int countDefeatedEvolvedMonsters(const EnemyCombatQueue& wave)
    {
        int total = 0;

        for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
        {
            if (wave.getDefeatedEnemy(i).isEvolved())
            {
                ++total;
            }
        }

        return total;
    }

    int countLivingEscapeBlockers(
        const Player& player,
        const std::vector<RecruitedAllyCombatSupport>& activeAllies,
        const std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        const std::vector<Summon>& summons)
    {
        int total = player.isDead() ? 0 : 1;
        for (const auto& ally : activeAllies)
        {
            if (ally.currentHp > 0) ++total;
        }
        // The waiting line is physically close to the battlefield in these modes. It can observe
        // and it can also close an escape route even if those members are not currently taking turns.
        for (const auto& ally : reserveAllies)
        {
            if (ally.currentHp > 0) ++total;
        }
        for (const Summon& summon : summons)
        {
            if (!summon.isDead() && !summon.isExpired()) ++total;
        }
        return std::max(1, total);
    }

    bool bobMauriceProtectionQuestActive(const Player& player)
    {
        for (const Quest& quest : player.getQuestLog().getQuests())
        {
            if (quest.id.rfind("bob_maurice_protection_", 0) == 0
                && quest.accepted
                && !quest.completed
                && !quest.turnedIn
                && !quest.failed)
            {
                return true;
            }
        }
        return false;
    }

    void playBobMauriceAlliedTurn(Player& player, EnemyCombatQueue& wave, Random& random)
    {
        if (!bobMauriceProtectionQuestActive(player) || !wave.hasActiveEnemies())
        {
            return;
        }

        std::vector<std::string> lines;

        Monster& bobTarget = wave.getActiveEnemy(0);
        const std::string bobTargetName = bobTarget.getName();
        bobTarget.takeDamage(1);
        lines.push_back("Bob : Hannnn... hummm... hammmm.");
        lines.push_back("Maurice : « Mon collègue Bob a dit qu'il vient d'infliger exactement 1 dégât à " + bobTargetName + ". Il insiste sur le mot exactement. »");
        wave.removeDeadAndReplace();

        if (wave.hasActiveEnemies())
        {
            Monster& mauriceTarget = wave.getActiveEnemy(0);
            const std::string mauriceTargetName = mauriceTarget.getName();
            mauriceTarget.takeDamage(1);
            lines.push_back("Maurice : Huuuhhhhh... hannn... hummm.");
            lines.push_back("Bob : « Maurice demande que son dégât contre " + mauriceTargetName + " soit inscrit séparément. Il en a fait 1 aussi. »");
            wave.removeDeadAndReplace();
        }
        else
        {
            lines.push_back("Maurice : Huuuhhhhh...");
            lines.push_back("Bob : « Maurice demande s'il peut garder son dégât pour plus tard. La réponse est probablement non. »");
        }

        const int effectRoll = random.between(1, 8);
        if (effectRoll == 1)
        {
            const int healed = std::max(2, player.getMaxHp() / 20);
            player.heal(healed);
            lines.push_back("Bob lance une petite fiole correctement étiquetée : tu récupères " + std::to_string(healed) + " PV.");
        }
        else if (effectRoll == 2)
        {
            player.applyElementalWard(2, 15);
            lines.push_back("Maurice renverse une poudre protectrice au bon endroit : protection élémentaire légère pendant 2 tours.");
        }
        else if (effectRoll == 3 && wave.hasActiveEnemies())
        {
            wave.getActiveEnemy(0).applyPoison(2, 1);
            lines.push_back("Une bouteille mal fermée roule sous un ennemi : poison léger pendant 2 tours.");
        }
        else if (effectRoll == 4)
        {
            player.takeDamage(1);
            lines.push_back("Bob te lance une potion. Maurice oublie de préciser qu'elle est encore dans sa bouteille : tu subis 1 dégât.");
        }
        else if (effectRoll == 5 && wave.hasActiveEnemies())
        {
            const int healed = std::max(2, wave.getActiveEnemy(0).getMaxHp() / 25);
            wave.getActiveEnemy(0).heal(healed);
            lines.push_back("Maurice soigne accidentellement l'ennemi actif de " + std::to_string(healed) + " PV. Bob prétend que c'était un test de loyauté.");
        }
        else if (effectRoll == 6)
        {
            player.applyWeakening(1, 10);
            lines.push_back("Une fumée commerciale te pique les yeux : affaiblissement léger pendant 1 tour.");
        }
        else if (effectRoll == 7)
        {
            player.applyRegeneration(2, 2);
            lines.push_back("Le tonique de Maurice fonctionne contre toute attente : régénération légère pendant 2 tours.");
        }
        else
        {
            lines.push_back("Les deux agitent une caisse vide avec conviction. Aucun effet mesurable, mais ils semblent satisfaits.");
        }

        MessageScreen::show(
            "TOUR DE BOB ET MAURICE",
            "combat.pve.bob_maurice.allied_turn",
            lines,
            false
        );
    }

    bool defeatedWaveContainedElite(const EnemyCombatQueue& wave)
    {
        for (int i = 0; i < wave.getDefeatedEnemyCount(); ++i)
        {
            if (wave.getDefeatedEnemy(i).isElite() || wave.getDefeatedEnemy(i).isEvolved())
            {
                return true;
            }
        }
        return false;
    }

    // EN: recordWaveKillsInBestiary declares or implements a focused behavior used by this module.
    // FR: recordWaveKillsInBestiary déclare ou implémente un comportement précis utilisé par ce module.


    std::string returningRivalDisplayName(const PlayerRivalRecord& rival)
    {
        if (rival.notoriety >= 14 || rival.returns >= 4) return rival.enemyName + " — Némésis";
        if (rival.notoriety >= 10 || rival.returns >= 3) return rival.enemyName + " — Fléau familier";
        if (rival.notoriety >= 6 || rival.returns >= 2) return rival.enemyName + " — Ombre revenue";
        return rival.enemyName + " — Rancune tenace";
    }

    bool maybeAppendReturningRival(Player& player, EnemyCombatQueue& wave, Random& random)
    {
        std::vector<std::string> candidates;
        for (const PlayerRivalRecord& rival : player.getRivalRecords())
        {
            if (!rival.alive || rival.lastSeenDay >= player.getWorldDaysElapsed()) continue;
            const bool localTrace = rival.lastKnownLocationId.empty() || rival.lastKnownLocationId == player.getCurrentCityId();
            const int chance = RivalEmergenceSystem::returnChancePercent(
                localTrace,
                player.hasPassiveSkill("church_oath_rivals"),
                player.hasPassiveSkill("church_oath_memory"),
                rival.returns,
                player.getWorldDaysElapsed() - rival.lastSeenDay
            );
            if (random.between(1, 100) <= chance) candidates.push_back(rival.rivalId);
        }
        if (candidates.empty()) return false;

        const std::string rivalId = candidates[static_cast<std::size_t>(random.between(0, static_cast<int>(candidates.size()) - 1))];
        const PlayerRivalRecord* rival = player.findRival(rivalId);
        if (rival == nullptr || !rival->alive) return false;

        player.recordRivalReturn(rivalId, player.getCurrentCityId());
        rival = player.findRival(rivalId);
        if (rival == nullptr) return false;

        int hp = std::max(8, rival->baseMaxHp + rival->returns * std::max(4, rival->baseMaxHp / 5) + rival->wounds * 2);
        int attack = std::max(2, rival->baseAttack + rival->returns * 2);
        if (rival->temperament == "endurci") hp += std::max(3, rival->baseMaxHp / 8);
        else if (rival->temperament == "calculateur") attack += 2;
        else if (rival->temperament == "traqueur") attack += 1 + rival->returns / 2;
        else if (rival->temperament == "rancunier") attack += std::min(3, rival->wounds);
        Monster returning(
            returningRivalDisplayName(*rival),
            rival->enemyFamily.empty() ? "Rival" : rival->enemyFamily,
            Race::Unknown,
            rival->currentLevel,
            hp,
            std::max(1, attack - 2),
            attack,
            attack + 4,
            rival->returns >= 2 ? 1 : 0,
            rival->returns >= 3 ? 1 : 0,
            false,
            rival->returns >= 2,
            false,
            rival->returns >= 3
        );
        returning.setRivalId(rivalId);
        wave.addWaitingEnemy(returning);

        MessageScreen::show(
            "TRACE FAMILIÈRE",
            "combat.pve.rival.return",
            {
                "Une présence déjà connue suit la rencontre au lieu d'apparaître comme un ennemi anonyme.",
                returning.getName() + " | retour " + std::to_string(rival->returns) + " | niveau " + std::to_string(rival->currentLevel) + ".",
                "Dernière trace connue : " + (rival->lastKnownLocationId.empty() ? std::string("inconnue") : rival->lastKnownLocationId) + ".",
                "Tempérament observé : " + (rival->temperament.empty() ? std::string("survivant prudent") : rival->temperament) + ".",
                rival->visibleMark.empty() ? "Aucune marque physique certaine n'a encore été reconnue." : "Marque reconnue : " + rival->visibleMark + ".",
                "Ce retour existe parce que cet individu a survécu auparavant. Sa mort, elle, restera définitive."
            },
            false
        );
        return true;
    }


    void displayWaveCombatSnapshot(
        const Player& player,
        const EnemyCombatQueue& wave,
        const std::vector<Summon>& summons,
        const std::string& title,
        const std::string& phase,
        int turnNumber,
        bool waitAndClear = false
    )
    {
        GuiCombatStateSnapshot snapshot = CombatDisplay::buildWaveSnapshot(
            player,
            wave,
            summons,
            title,
            phase,
            turnNumber
        );

        if (!player.isDead())
        {
            snapshot.currentActorName = player.getName();
        }

        CombatDisplay::displayCombatState(snapshot, waitAndClear);
    }

    void showCombatPhaseGate(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        MessageScreen::show(title, screenId, lines, false);
        Console::pauseSeconds(1);
    }


    void showPostCombatRouteScreen(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        MenuScreen screen(title, screenId);
        screen.addSubtitle("Bilan de route de combat");

        for (const std::string& line : lines)
        {
            screen.addLine(line);
        }

        screen.setDisplayOnlyInput("Résumé affiché sans saisie directe.");
        TerminalInterface::renderMenuScreen(screen, false);
    }


    struct TemporaryAdventurerSupport
    {
        bool active = false;
        std::string groupName;
        int remainingTurns = 0;
        int initiativeSlot = 0;
    };

    std::string rareCombatAidGroupName(Random& random)
    {
        const std::vector<std::string> groups = {
            "Les Lanternes de Prunigil",
            "Les Chasseurs du Croc Tordu",
            "Deux Lames et un Chariot",
            "L'Ordo de Pierre",
            "Les Éclats d'Azur"
        };
        return groups[static_cast<std::size_t>(random.between(0, static_cast<int>(groups.size()) - 1))];
    }

    int localJournalCount(const Player& player, const std::string& category, const std::string& key)
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

    std::string extractLabelStringField(const std::string& label, const std::string& fieldName, const std::string& fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t pos = label.find(marker);
        if (pos == std::string::npos)
        {
            return fallback;
        }
        const std::size_t start = pos + marker.size();
        std::size_t end = label.find(" | ", start);
        if (end == std::string::npos)
        {
            end = label.size();
        }
        if (end <= start)
        {
            return fallback;
        }
        return label.substr(start, end - start);
    }

    int extractLabelIntField(const std::string& label, const std::string& fieldName, int fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t pos = label.find(marker);
        if (pos == std::string::npos)
        {
            return fallback;
        }
        std::size_t start = pos + marker.size();
        std::size_t end = start;
        if (end < label.size() && label[end] == '-')
        {
            ++end;
        }
        while (end < label.size() && std::isdigit(static_cast<unsigned char>(label[end])))
        {
            ++end;
        }
        if (end <= start)
        {
            return fallback;
        }
        try
        {
            return std::stoi(label.substr(start, end - start));
        }
        catch (...)
        {
            return fallback;
        }
    }

    unsigned int stableAllyHash(const std::string& seed)
    {
        unsigned int hash = 2166136261u;
        for (unsigned char c : seed)
        {
            hash ^= c;
            hash *= 16777619u;
        }
        return hash;
    }

    int recruitedAllyRespawnHealthPercent(DifficultyMode difficulty)
    {
        return std::clamp(DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty), 1, 100);
    }

    std::string recruitedAllyRankName(int index)
    {
        static const std::vector<std::string> ranks = {"F", "E", "D", "C", "B", "A", "S"};
        index = std::clamp(index, 0, static_cast<int>(ranks.size()) - 1);
        return ranks[static_cast<std::size_t>(index)];
    }

    struct RecruitedAllyUniqueOrder
    {
        bool healOneTurn = false;
        bool guardOneTurn = false;
        bool forceTechniqueOneTurn = false;
        bool hasPriorityTarget = false;
        std::string priorityTargetName;
        bool switchRequested = false;
        std::string switchInName;
    };

    struct RecruitedAllyOrderState
    {
        bool groupHealOneTurn = false;
        bool groupGuardOneTurn = false;
        bool groupTechniqueOneTurn = false;
        bool groupCombinedTechniqueOneTurn = false;
        // Transient combat-only recovery. It decreases only after a real enemy response.
        // Value 2 means: one complete intervening round without the same duo chain.
        int combinedTechniqueRecoveryEnemyPhases = 0;
        bool groupSpreadTargetsOneTurn = false;
        bool groupPriorityTarget = false;
        std::string groupPriorityTargetName;
        std::map<std::string, RecruitedAllyUniqueOrder> uniqueOrders;
    };

    std::string lowerCopy(std::string text)
    {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return text;
    }


    EmployerTacticalOrder issueEmployerTacticalOrder(
        Player& player,
        EnemyCombatQueue& wave,
        const std::vector<RecruitedAllyCombatSupport>& activeAllies,
        const std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        Random& random,
        const std::string& contextId)
    {
        EmployerTacticalOrderContext context;
        context.activeEnemyCount = wave.getActiveEnemyCount();
        context.reserveAvailable = !reserveAllies.empty();
        context.professionalTrust = EmployerTeamDisciplineSystem::stateFor(player).professionalTrust;
        for (const auto& ally : activeAllies)
        {
            if (ally.currentHp <= 0) continue;
            const int hp = ally.maxHp > 0 ? std::clamp(ally.currentHp * 100 / ally.maxHp, 0, 100) : 100;
            if (hp <= 45) ++context.lowHpAllies;
            if (hp <= 24) ++context.criticalHpAllies;
            if (RecruitedAllyActionPlanner::isSupportProfile(ally)) context.supportMemberActive = true;
            if (ally.teamLeader)
            {
                context.leaderIntelligence = std::clamp(48 + ally.estimatedLevel / 2 + ally.currentRankIndex * 5, 35, 95);
            }
        }

        EmployerTacticalOrder order = EmployerTacticalOrderSystem::decide(context, random);
        if (order.kind == EmployerTacticalOrderKind::FocusFire && wave.hasActiveEnemies())
        {
            order.targetName = wave.getActiveEnemy(0).getName();
            order.targetId = wave.getActiveEnemy(0).getCombatInstanceId();
            order.instruction += " Cible désignée : " + order.targetName + ".";
        }
        if (order.kind == EmployerTacticalOrderKind::None)
        {
            return order;
        }

        player.recordCanonicalEvent(
            "team_employer_tactical_orders_issued",
            contextId + ":" + order.code + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
            TeamLeadershipSystem::leaderDisplayName(player) + " | " + order.title + " | " + order.instruction,
            1
        );
        MessageScreen::show(
            "ORDRE DU CHEF — " + order.title,
            "combat.pve.employer_team.tactical_order." + order.code,
            {
                TeamLeadershipSystem::leaderDisplayName(player) + " donne une consigne pour cet engagement.",
                "« " + order.instruction + " »",
                "Raison visible : " + order.rationale + ".",
                "Tu restes libre de ton propre personnage. Une désobéissance ne sera sanctionnée que si elle peut être constatée et jugée de façon crédible."
            },
            false
        );
        return order;
    }

    void applyEmployerTacticalOrderToAllies(
        const EmployerTacticalOrder& order,
        EnemyCombatQueue& wave,
        RecruitedAllyOrderState& orders)
    {
        switch (order.kind)
        {
            case EmployerTacticalOrderKind::ProtectSupport:
            case EmployerTacticalOrderKind::HoldLine:
            case EmployerTacticalOrderKind::ExtractWounded:
                orders.groupGuardOneTurn = true;
                break;
            case EmployerTacticalOrderKind::FocusFire:
            {
                orders.groupPriorityTarget = false;
                orders.groupPriorityTargetName.clear();
                for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
                {
                    if (wave.getActiveEnemy(index).getCombatInstanceId() == order.targetId)
                    {
                        orders.groupPriorityTarget = true;
                        orders.groupPriorityTargetName = order.targetName;
                        break;
                    }
                }
                break;
            }
            case EmployerTacticalOrderKind::ControlledBreakthrough:
                orders.groupTechniqueOneTurn = true;
                break;
            case EmployerTacticalOrderKind::None:
            default:
                break;
        }
    }

    bool employerLeaderCanObservePlayer(
        const std::vector<RecruitedAllyCombatSupport>& activeAllies)
    {
        return std::any_of(activeAllies.begin(), activeAllies.end(), [](const RecruitedAllyCombatSupport& ally) {
            return ally.teamOrigin == AllyTeamOrigin::EmployerTeam
                && ally.teamLeader
                && ally.currentHp > 0;
        });
    }

    int employerColleagueWitnessCount(
        const std::vector<RecruitedAllyCombatSupport>& activeAllies,
        const std::vector<RecruitedAllyCombatSupport>& reserveAllies)
    {
        auto countIn = [](const std::vector<RecruitedAllyCombatSupport>& allies) {
            return static_cast<int>(std::count_if(allies.begin(), allies.end(), [](const RecruitedAllyCombatSupport& ally) {
                return ally.teamOrigin == AllyTeamOrigin::EmployerTeam
                    && !ally.teamLeader
                    && ally.currentHp > 0;
            }));
        };
        return countIn(activeAllies) + countIn(reserveAllies);
    }

    void observeEmployerPlayerAction(
        Player& player,
        const EmployerTacticalOrder& order,
        EmployerOrderCombatReview& review,
        const std::vector<RecruitedAllyCombatSupport>& activeAllies,
        const std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        const EnemyCombatQueue& wave,
        const PlayerWaveObservedAction& observedAction,
        bool escapeSucceeded,
        const std::string& combatContextId,
        int turnNumber)
    {
        if (order.kind == EmployerTacticalOrderKind::None || order.code.empty()) return;

        EmployerObservedOrderActionContext context;
        context.orderKind = order.kind;
        context.orderCode = order.code;
        context.actionKind = observedAction.actionKind;
        context.actionTargetName = observedAction.targetKnown ? observedAction.targetName : std::string();
        context.actionTargetId = observedAction.targetKnown ? observedAction.targetId : std::string();
        context.orderedTargetName = order.targetName;
        context.orderedTargetId = order.targetId;
        context.targetForcedByCombatRules = observedAction.targetForcedByCombatRules;
        context.orderedTargetStillActive = false;
        if (!order.targetName.empty())
        {
            for (int index = 0; index < wave.getActiveEnemyCount(); ++index)
            {
                if (wave.getActiveEnemy(index).getCombatInstanceId() == order.targetId)
                {
                    context.orderedTargetStillActive = true;
                    break;
                }
            }
        }
        context.leaderPresent = employerLeaderCanObservePlayer(activeAllies);
        context.witnessCount = employerColleagueWitnessCount(activeAllies, reserveAllies);
        context.playerHpPercent = player.getMaxHp() > 0
            ? std::clamp(player.getHp() * 100 / player.getMaxHp(), 0, 100)
            : 0;
        context.escapeSucceeded = escapeSucceeded;
        context.immediateSurvivalThreat = context.playerHpPercent <= 20;

        EmployerTeamDisciplineSystem::appendObservedOrderAction(
            player, review, context, combatContextId, turnNumber);
    }

    void finalizeEmployerOrderReview(
        Player& player,
        const EmployerTacticalOrder& order,
        const EmployerOrderCombatReview& review,
        const std::string& combatContextId,
        bool displayReaction)
    {
        if (order.kind == EmployerTacticalOrderKind::None || order.code.empty()) return;
        if (review.observedActions <= 0 && review.unobservedActions <= 0) return;

        const EmployerOrderReviewOutcome outcome = EmployerTeamDisciplineSystem::finalizeOrderReview(
            player, review, order.code, combatContextId);

        if (!displayReaction) return;

        std::vector<std::string> lines;
        lines.push_back("Ordre initial : " + order.title + ".");
        lines.push_back("Actions réellement observées : " + std::to_string(review.observedActions)
            + " ; non observées : " + std::to_string(review.unobservedActions) + ".");
        if (review.directLeaderObservations > 0)
        {
            lines.push_back("Le chef a vu directement " + std::to_string(review.directLeaderObservations) + " action(s).");
        }
        if (review.reportedObservations > 0)
        {
            lines.push_back(std::to_string(review.reportedObservations) + " action(s) peuvent être rapportées par des collègues présents.");
        }
        lines.push_back("Conclusion : " + EmployerTeamDisciplineSystem::orderObservationVerdictLabel(outcome.verdict) + ".");
        lines.push_back(outcome.reaction);

        MessageScreen::show(
            "RETOUR DU CHEF — " + order.title,
            "combat.pve.employer_team.order_review." + combatContextId + "." + order.code,
            lines,
            false
        );
    }

    bool employerTeamMissionIsActive(const Player& player)
    {
        return TeamEmploymentSystem::hasActiveContract(player)
            && TeamEmploymentMissionSystem::hasOpenEmployerMission(player);
    }

    bool usesEmployerTeam(const std::vector<RecruitedAllyCombatSupport>& allies)
    {
        return std::any_of(allies.begin(), allies.end(), [](const RecruitedAllyCombatSupport& ally) {
            return ally.teamOrigin == AllyTeamOrigin::EmployerTeam;
        });
    }

    std::vector<RecruitedAllyCombatSupport> collectActiveRecruitedAllies(const Player& player)
    {
        if (employerTeamMissionIsActive(player))
        {
            std::vector<RecruitedAllyCombatSupport> result;
            const EmployerTeamFormation formation = EmployerTeamRosterSystem::formationFor(player, true);
            for (const EmployerTeamMember& member : formation.activeMembers)
            {
                result.push_back(EmployerTeamRosterSystem::toCombatSupport(member));
            }
            return result;
        }
        return RecruitedAllyCombatRoster::collectActive(player);
    }

    std::vector<RecruitedAllyCombatSupport> collectReserveRecruitedAllies(const Player& player)
    {
        if (employerTeamMissionIsActive(player))
        {
            std::vector<RecruitedAllyCombatSupport> result;
            const EmployerTeamFormation formation = EmployerTeamRosterSystem::formationFor(player, true);
            for (const EmployerTeamMember& member : formation.reserveMembers)
            {
                result.push_back(EmployerTeamRosterSystem::toCombatSupport(member));
            }
            return result;
        }
        return RecruitedAllyCombatRoster::collectReserve(player);
    }

    bool recruitedAllyJobContains(const RecruitedAllyCombatSupport& ally, const std::string& token)
    {
        std::string text = ally.job + " " + ally.trait;
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return text.find(token) != std::string::npos;
    }


    int recruitedAllyMaturityScore(const RecruitedAllyCombatSupport& ally)
    {
        return std::clamp(ally.currentRankIndex * 16 + ally.estimatedLevel / 4 + ally.weaponQuality + ally.armorQuality + ally.supportKitQuality, 0, 140);
    }

    int recruitedAllyMaturityTier(const RecruitedAllyCombatSupport& ally)
    {
        const int score = recruitedAllyMaturityScore(ally);
        if (score >= 92) return 5;
        if (score >= 72) return 4;
        if (score >= 52) return 3;
        if (score >= 34) return 2;
        if (score >= 18) return 1;
        return 0;
    }

    std::string recruitedAllyMaturityName(const RecruitedAllyCombatSupport& ally)
    {
        switch (recruitedAllyMaturityTier(ally))
        {
            case 0: return "débutant";
            case 1: return "apprenti";
            case 2: return "habitué";
            case 3: return "aguerri";
            case 4: return "vétéran";
            default: return "maître de terrain";
        }
    }

    bool recruitedAllyIsStillLearning(const RecruitedAllyCombatSupport& ally)
    {
        return ally.currentRankIndex <= 1 || recruitedAllyMaturityTier(ally) <= 1;
    }

    bool recruitedAllyHasAdvancedReading(const RecruitedAllyCombatSupport& ally)
    {
        return ally.currentRankIndex >= 2 && recruitedAllyMaturityTier(ally) >= 2;
    }

    int recruitedAllyOrderTechniqueChance(const RecruitedAllyCombatSupport& ally, bool forced)
    {
        const int base = forced ? 56 : 28;
        const int rankBonus = ally.currentRankIndex * (forced ? 7 : 8);
        const int levelBonus = ally.estimatedLevel / (forced ? 10 : 12);
        return std::clamp(base + rankBonus + levelBonus, forced ? 58 : 25, forced ? 96 : 86);
    }

    int recruitedAllyNaturalTechniqueChance(const RecruitedAllyCombatSupport& ally, EnemyCombatQueue& wave)
    {
        int chance = 7 + ally.currentRankIndex * 4 + ally.estimatedLevel / 20 + recruitedAllyMaturityTier(ally) * 4;
        if (recruitedAllyJobContains(ally, "roublard") || recruitedAllyJobContains(ally, "assassin") || recruitedAllyJobContains(ally, "brigand"))
        {
            chance += 11;
        }
        else if ((recruitedAllyJobContains(ally, "archer") || recruitedAllyJobContains(ally, "mage") || recruitedAllyJobContains(ally, "lancier")) && wave.getActiveEnemyCount() >= 2)
        {
            chance += 9;
        }
        else if (recruitedAllyJobContains(ally, "soigneur") || recruitedAllyJobContains(ally, "mage d'appui"))
        {
            chance += 6;
        }
        if (recruitedAllyIsStillLearning(ally))
        {
            chance = chance * 55 / 100;
        }
        if (recruitedAllyMaturityTier(ally) >= 4)
        {
            chance += 5;
        }
        return std::clamp(chance, 3, 56);
    }


    std::string recruitedAllyHealthLine(const RecruitedAllyCombatSupport& ally)
    {
        return "PV " + std::to_string(std::max(0, ally.currentHp)) + "/" + std::to_string(std::max(1, ally.maxHp))
            + " | potions " + std::to_string(std::max(0, ally.healingPotionCharges))
            + " | arme " + std::to_string(std::max(1, ally.weaponQuality))
            + " | armure " + std::to_string(std::max(1, ally.armorQuality))
            + " | maturité " + recruitedAllyMaturityName(ally) + " (" + std::to_string(recruitedAllyMaturityScore(ally)) + "/140)"
            + " | technique CD " + std::to_string(std::max(0, ally.activeSkillCooldown));
    }

    bool recruitedAllyShouldUseTechnique(const RecruitedAllyCombatSupport& ally, EnemyCombatQueue& wave, Random& random, bool hasDirectOrder, bool forceTechnique)
    {
        if (ally.activeSkillCooldown > 0 || !wave.hasActiveEnemies())
        {
            return false;
        }
        if (forceTechnique)
        {
            return random.between(1, 100) <= recruitedAllyOrderTechniqueChance(ally, true);
        }
        if (hasDirectOrder)
        {
            return random.between(1, 100) <= recruitedAllyOrderTechniqueChance(ally, false);
        }
        return random.between(1, 100) <= recruitedAllyNaturalTechniqueChance(ally, wave);
    }

    void recordRecruitedAllyHitContribution(RecruitedAllyCombatSupport& ally, int damage, bool killedTarget)
    {
        ally.damageDealt += std::max(0, damage);
        if (killedTarget)
        {
            ++ally.finishBlows;
        }
    }

    void recordRecruitedAllySupportContribution(RecruitedAllyCombatSupport& ally, int amount = 1)
    {
        ally.supportActions += std::max(1, amount);
    }

    std::string recruitedAllyTechniqueGesture(const RecruitedAllyCombatSupport& ally, const std::string& defaultGesture)
    {
        const std::string profile = lowerCopy(ally.name + " " + ally.race + " " + ally.job + " " + ally.trait);
        if (profile.find("assassin") != std::string::npos || profile.find("roublard") != std::string::npos || profile.find("brigand") != std::string::npos)
        {
            if (profile.find("elfe") != std::string::npos)
            {
                return ally.name + " disparaît dans un pas trop silencieux pour être honnête";
            }
            return ally.name + " baisse l'épaule, montre une fausse ouverture et passe sous la garde";
        }
        if (profile.find("archer") != std::string::npos || profile.find("tireur") != std::string::npos)
        {
            return ally.name + " cale sa respiration, puis coupe la ligne ennemie avec un tir sec";
        }
        if (profile.find("lancier") != std::string::npos)
        {
            return ally.name + " avance d'un demi-pas et force les ennemis à respecter la pointe";
        }
        if (profile.find("gardien") != std::string::npos || profile.find("tank") != std::string::npos || profile.find("protecteur") != std::string::npos)
        {
            return ally.name + " plante sa garde devant la ligne et transforme l'avancée en mur";
        }
        if (profile.find("soigneur") != std::string::npos || profile.find("mage d'appui") != std::string::npos || profile.find("intendant") != std::string::npos)
        {
            return ally.name + " pose un repère calme au milieu du chaos et désigne la faille";
        }
        if (profile.find("mage") != std::string::npos || profile.find("sorcier") != std::string::npos)
        {
            return ally.name + " serre les doigts autour d'une formule courte et laisse l'air se tendre";
        }
        if (profile.find("kitsune") != std::string::npos)
        {
            return ally.name + " laisse une flamme courte suivre son mouvement";
        }
        if (profile.find("dragon") != std::string::npos || profile.find("dracon") != std::string::npos)
        {
            return ally.name + " fait vibrer sa force ancienne dans l'impact";
        }
        return defaultGesture;
    }

    std::string recruitedAllyReadinessLine(const RecruitedAllyCombatSupport& ally, const Player& player)
    {
        const std::string profile = lowerCopy(ally.name + " " + ally.race + " " + ally.job + " " + ally.trait);
        const int allyHpPercent = ally.maxHp > 0 ? ally.currentHp * 100 / ally.maxHp : 0;
        const int playerHpPercent = player.getMaxHp() > 0 ? player.getHp() * 100 / player.getMaxHp() : 100;
        if (recruitedAllyMaturityTier(ally) <= 0)
        {
            return ally.name + " reste proche de la ligne et attend surtout de comprendre ce que tu vas ordonner";
        }
        if (recruitedAllyMaturityTier(ally) == 1 && playerHpPercent > 35)
        {
            return ally.name + " connaît son rôle, mais garde encore plus les yeux sur toi que sur tout le champ de bataille";
        }
        if (allyHpPercent <= 30)
        {
            return ally.name + " arrive en serrant les dents : elle peut encore agir, mais pas encaisser n'importe quoi";
        }
        if (playerHpPercent <= 35 && (profile.find("soigneur") != std::string::npos || profile.find("mage d'appui") != std::string::npos || profile.find("gardien") != std::string::npos))
        {
            return ally.name + " garde déjà un oeil sur ton état avant même que la ligne bouge";
        }
        if (profile.find("assassin") != std::string::npos || profile.find("roublard") != std::string::npos)
        {
            return ally.name + " prend une place légèrement décalée, assez loin pour chercher un angle mort";
        }
        if (profile.find("archer") != std::string::npos || profile.find("tireur") != std::string::npos)
        {
            return ally.name + " vérifie les distances comme si chaque pas ennemi comptait déjà";
        }
        if (profile.find("gardien") != std::string::npos || profile.find("tank") != std::string::npos || profile.find("protecteur") != std::string::npos)
        {
            return ally.name + " se place naturellement entre le danger et le reste du groupe";
        }
        if (profile.find("mage") != std::string::npos || profile.find("chaman") != std::string::npos || profile.find("sorcier") != std::string::npos)
        {
            return ally.name + " attend le premier désordre pour poser son rythme magique";
        }
        return ally.name + " rejoint la ligne sans grand discours, mais avec une intention claire";
    }

    std::string recruitedAllyPresenceLine(const RecruitedAllyCombatSupport& ally, const Player& player, EnemyCombatQueue& wave)
    {
        const std::string profile = lowerCopy(ally.name + " " + ally.race + " " + ally.job + " " + ally.trait);
        const int allyHpPercent = ally.maxHp > 0 ? ally.currentHp * 100 / ally.maxHp : 0;
        const int playerHpPercent = player.getMaxHp() > 0 ? player.getHp() * 100 / player.getMaxHp() : 100;

        if (recruitedAllyMaturityTier(ally) <= 0)
        {
            return ally.name + " suit la cadence avec prudence, sans encore lire toute la scène";
        }
        if (recruitedAllyMaturityTier(ally) == 1 && ally.turnsTaken % 2 == 1)
        {
            return ally.name + " hésite une demi-seconde, puis se replace sur une consigne simple";
        }
        if (allyHpPercent <= 30)
        {
            return ally.name + " garde un bras près de ses côtes, mais reste dans la ligne tant que ses jambes tiennent";
        }
        if (playerHpPercent <= 35 && (profile.find("soigneur") != std::string::npos || profile.find("mage d'appui") != std::string::npos || profile.find("intendant") != std::string::npos))
        {
            return ally.name + " surveille ton souffle plus que la cible et prépare déjà une couverture";
        }
        if (playerHpPercent <= 35 && (profile.find("gardien") != std::string::npos || profile.find("tank") != std::string::npos || profile.find("protecteur") != std::string::npos))
        {
            return ally.name + " se décale d'un pas pour couper l'angle le plus dangereux vers toi";
        }
        if (wave.getActiveEnemyCount() >= 3 && (profile.find("chef") != std::string::npos || profile.find("capitaine") != std::string::npos || profile.find("stratège") != std::string::npos || profile.find("stratege") != std::string::npos))
        {
            return ally.name + " compte les lignes ennemies avant de choisir qui doit vraiment tomber";
        }
        if (profile.find("assassin") != std::string::npos || profile.find("roublard") != std::string::npos || profile.find("brigand") != std::string::npos)
        {
            return ally.name + " ne fixe pas la cible principale, seulement l'espace où elle va finir par se tromper";
        }
        if (profile.find("archer") != std::string::npos || profile.find("tireur") != std::string::npos || profile.find("éclaireur") != std::string::npos || profile.find("eclaireur") != std::string::npos)
        {
            return ally.name + " suit les épaules ennemies et attend une respiration trop longue";
        }
        if (profile.find("kitsune") != std::string::npos)
        {
            return ally.name + " laisse passer un sourire nerveux, comme si la prochaine feinte était déjà prête";
        }
        if (profile.find("orc") != std::string::npos || profile.find("barbare") != std::string::npos || profile.find("berserker") != std::string::npos)
        {
            return ally.name + " serre sa prise et cherche le moment où la ligne ennemie fera enfin du bruit";
        }
        if (profile.find("mage") != std::string::npos || profile.find("sorcier") != std::string::npos || profile.find("chaman") != std::string::npos)
        {
            return ally.name + " écoute le combat comme une formule qui n'a pas encore choisi sa fin";
        }
        return ally.name + " relit rapidement la distance avant de reprendre son rôle dans la ligne";
    }

    std::string recruitedAllyTacticalCommentLine(const RecruitedAllyCombatSupport& ally, const Player& player, EnemyCombatQueue& wave, bool hasActiveOrder)
    {
        if (!wave.hasActiveEnemies())
        {
            return "";
        }

        const std::string profile = lowerCopy(ally.name + " " + ally.race + " " + ally.job + " " + ally.trait);
        const int playerHpPercent = player.getMaxHp() > 0 ? player.getHp() * 100 / player.getMaxHp() : 100;
        const Monster& frontEnemy = wave.getActiveEnemy(0);
        const MonsterBehaviorProfile enemyProfile = MonsterBehaviorProfileCatalog::build(frontEnemy);
        const std::string enemyArchetype = lowerCopy(enemyProfile.archetype);
        const int enemyHpPercent = frontEnemy.getMaxHp() > 0 ? frontEnemy.getHp() * 100 / frontEnemy.getMaxHp() : 100;

        if (recruitedAllyMaturityTier(ally) <= 0 && !hasActiveOrder)
        {
            return ally.turnsTaken % 3 == 0 ? ally.name + " attend une indication claire avant de tenter quelque chose de trop technique" : "";
        }
        if (recruitedAllyMaturityTier(ally) == 1 && !hasActiveOrder && ally.turnsTaken % 2 == 1)
        {
            return "";
        }
        if (recruitedAllyMaturityTier(ally) <= 1 && hasActiveOrder)
        {
            return ally.name + " suit l'ordre donné, même si sa lecture du terrain reste encore limitée";
        }
        if (!recruitedAllyHasAdvancedReading(ally) && enemyArchetype.find("soigneur") == std::string::npos && enemyArchetype.find("rameuteur") == std::string::npos)
        {
            return "";
        }

        if (playerHpPercent <= 35 && (profile.find("soigneur") != std::string::npos || profile.find("mage d'appui") != std::string::npos || profile.find("intendant") != std::string::npos))
        {
            return ally.name + " garde sa main libre, comme si le prochain mauvais souffle du chef était déjà prévu";
        }
        if (playerHpPercent <= 35 && (profile.find("gardien") != std::string::npos || profile.find("protecteur") != std::string::npos || profile.find("tank") != std::string::npos))
        {
            return ally.name + " resserre la ligne vers toi au lieu de chercher un duel héroïque";
        }
        if (enemyArchetype.find("rameuteur") != std::string::npos)
        {
            if (profile.find("assassin") != std::string::npos || profile.find("roublard") != std::string::npos)
            {
                return ally.name + " fixe surtout la gorge du rameuteur : couper le signal vaut plus qu'un joli coup";
            }
            return ally.name + " garde le signal ennemi dans son champ de vision pour éviter une mauvaise surprise";
        }
        if (enemyArchetype.find("soigneur") != std::string::npos || enemyArchetype.find("transfuseur") != std::string::npos)
        {
            return ally.name + " cherche la main qui soigne, pas seulement le corps le plus proche";
        }
        if (enemyArchetype.find("porte-bouclier") != std::string::npos || enemyArchetype.find("garde runique") != std::string::npos)
        {
            if (profile.find("lancier") != std::string::npos || profile.find("archer") != std::string::npos || profile.find("mage") != std::string::npos)
            {
                return ally.name + " lit les bords de la garde et attend la jointure plutôt que le centre";
            }
            return ally.name + " évite de gaspiller son impact sur la partie la plus solide";
        }
        if (enemyArchetype.find("slime") != std::string::npos || enemyArchetype.find("fongique") != std::string::npos || enemyArchetype.find("champignon") != std::string::npos)
        {
            return ally.name + " surveille surtout le sol, parce que la cible attaque presque autant avec l'espace qu'avec son corps";
        }
        if (enemyArchetype.find("illusionniste") != std::string::npos || enemyArchetype.find("piégeur") != std::string::npos)
        {
            return ally.name + " ralentit volontairement son geste pour ne pas répondre à la première fausse ouverture";
        }
        if (wave.getActiveEnemyCount() >= 3 && (profile.find("chef") != std::string::npos || profile.find("capitaine") != std::string::npos || profile.find("stratège") != std::string::npos || profile.find("stratege") != std::string::npos))
        {
            return ally.name + " découpe la mêlée en petites priorités au lieu de regarder le groupe comme une masse";
        }
        if (enemyHpPercent <= 30 && (profile.find("orc") != std::string::npos || profile.find("barbare") != std::string::npos || profile.find("berserker") != std::string::npos))
        {
            return ally.name + " sent que la cible peut tomber et serre sa prise avec beaucoup trop d'envie";
        }
        if (profile.find("fée") != std::string::npos || profile.find("fee") != std::string::npos)
        {
            return ally.name + " laisse ses appuis légers, prêt à punir le moindre angle trop lourd";
        }
        if (profile.find("dragon") != std::string::npos || profile.find("dracon") != std::string::npos)
        {
            return ally.name + " respire plus lentement, comme si son sang refusait de paniquer devant la ligne";
        }

        return "";
    }

    std::string recruitedAllyTargetResultLine(const RecruitedAllyCombatSupport& ally, const Monster& target, bool killed)
    {
        const std::string profile = lowerCopy(ally.name + " " + ally.race + " " + ally.job + " " + ally.trait);
        const MonsterBehaviorProfile targetProfile = MonsterBehaviorProfileCatalog::build(target);
        if (recruitedAllyIsStillLearning(ally))
        {
            if (killed)
            {
                return "La cible tombe, mais la recrue semble surtout surprise d'avoir trouvé le bon angle.";
            }
            return "La recrue touche, sans encore comprendre tout ce que la cible vient de révéler.";
        }
        if (killed)
        {
            if (profile.find("assassin") != std::string::npos || profile.find("roublard") != std::string::npos)
            {
                return "La cible tombe sans avoir eu le temps de refermer son angle.";
            }
            if (profile.find("gardien") != std::string::npos || profile.find("tank") != std::string::npos)
            {
                return "La ligne alliée avance d'un cran derrière ce choc.";
            }
            return "La cible quitte la ligne ennemie et laisse un vide exploitable.";
        }
        if (target.getMaxHp() > 0 && target.getHp() * 100 <= target.getMaxHp() * 35)
        {
            return "La cible reste debout, mais " + targetProfile.archetype + " montre une vraie fissure.";
        }
        if (recruitedAllyMaturityTier(ally) >= 4 && !targetProfile.counterplayLine.empty())
        {
            return "Lecture vétérane : " + targetProfile.counterplayLine;
        }
        if (!targetProfile.durabilityLine.empty())
        {
            return "Lecture de cible : " + targetProfile.durabilityLine;
        }
        return "Le coup force surtout la cible à révéler un peu plus son rythme.";
    }

    int chooseRecruitedAllyTargetIndexWithOrders(
        RecruitedAllyCombatSupport& ally,
        EnemyCombatQueue& wave,
        Random& random,
        RecruitedAllyOrderState& orders
    );


    int recruitedAllyTechniqueImpactBonus(const RecruitedAllyCombatSupport& ally, bool hasDirectOrder, bool forceTechnique)
    {
        int bonus = recruitedAllyMaturityTier(ally) * 3 + ally.currentRankIndex + ally.weaponQuality / 3;
        if (hasDirectOrder)
        {
            bonus += 3;
        }
        if (forceTechnique)
        {
            bonus += recruitedAllyMaturityTier(ally) <= 1 ? 2 : 4;
        }
        return std::clamp(bonus, 0, 24);
    }

    int recruitedAllyTechniqueCooldown(const RecruitedAllyCombatSupport& ally, int baseCooldown, bool hasDirectOrder, bool forceTechnique)
    {
        int reduction = 0;
        if (recruitedAllyMaturityTier(ally) >= 3) ++reduction;
        if (recruitedAllyMaturityTier(ally) >= 5) ++reduction;
        if (hasDirectOrder && recruitedAllyMaturityTier(ally) >= 2) ++reduction;
        if (forceTechnique && recruitedAllyMaturityTier(ally) <= 1) reduction = std::max(0, reduction - 1);
        return std::max(1, baseCooldown - reduction);
    }

    std::string recruitedAllyTechniqueMaturityLine(const RecruitedAllyCombatSupport& ally, bool hasDirectOrder)
    {
        if (recruitedAllyMaturityTier(ally) >= 4)
        {
            return "  Lecture alliée : " + ally.name + " transforme son expérience en vrai tempo de combat, pas seulement en dégâts. Sa technique doit changer le tour.";
        }
        if (recruitedAllyMaturityTier(ally) >= 2)
        {
            return "  Lecture alliée : " + ally.name + " comprend assez la scène pour choisir un effet utile.";
        }
        if (hasDirectOrder)
        {
            return "  Ordre clair : la recrue reste limitée, mais l'ordre évite une partie de l'hésitation.";
        }
        return "  Lecture alliée : recrue encore basse maturité, technique volontairement simple.";
    }

    bool executeRecruitedAllyActiveTechnique(
        RecruitedAllyCombatSupport& ally,
        EnemyCombatQueue& wave,
        Random& random,
        RecruitedAllyOrderState& orders,
        std::vector<std::string>& lines,
        bool plannerSelectedTechnique = false
    )
    {
        RecruitedAllyUniqueOrder& techniqueOrder = orders.uniqueOrders[ally.name];
        const bool forceTechnique = techniqueOrder.forceTechniqueOneTurn || orders.groupTechniqueOneTurn;
        const bool hasDirectOrder = techniqueOrder.hasPriorityTarget || orders.groupPriorityTarget || forceTechnique;
        if (!plannerSelectedTechnique && !recruitedAllyShouldUseTechnique(ally, wave, random, hasDirectOrder, forceTechnique))
        {
            return false;
        }

        const int techniqueImpactBonus = recruitedAllyTechniqueImpactBonus(ally, hasDirectOrder, forceTechnique);
        const int baseDamage = std::max(3, ally.estimatedLevel / 2 + ally.currentRankIndex * 4 + ally.weaponQuality * 2 + techniqueImpactBonus + random.between(2, 7));

        if (recruitedAllyIsStillLearning(ally) && !hasDirectOrder && random.between(1, 100) <= 25)
        {
            lines.push_back("- " + ally.name + " commence une technique, voit trop d'informations d'un coup, puis revient à une attaque simple. Une consigne claire l'aurait probablement stabilisée.");
            return false;
        }

        if (recruitedAllyIsStillLearning(ally))
        {
            const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, orders);
            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const int damage = std::max(2, baseDamage * (forceTechnique ? 68 : (hasDirectOrder ? 58 : 50)) / 100 + random.between(0, 2));
            target.takeDamage(damage);
            const bool killed = target.isDead();
            wave.removeDeadAndReplace();
            recordRecruitedAllyHitContribution(ally, damage, killed);
            ally.activeSkillCooldown = recruitedAllyTechniqueCooldown(ally, forceTechnique ? 3 : (hasDirectOrder ? 3 : 4), hasDirectOrder, forceTechnique);
            lines.push_back("- " + ally.name + (forceTechnique ? " applique l'ordre avec une technique encore hésitante" : (hasDirectOrder ? " transforme la consigne en technique simple" : " tente une technique simple")) + " : " + targetName + " subit " + std::to_string(damage) + " dégâts. Maîtrise " + recruitedAllyMaturityName(ally) + ", CD " + std::to_string(ally.activeSkillCooldown) + " tours.");
            lines.push_back(recruitedAllyTechniqueMaturityLine(ally, hasDirectOrder));
            return true;
        }

        if (recruitedAllyJobContains(ally, "gardien") || recruitedAllyJobContains(ally, "tank"))
        {
            const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, orders);
            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const int damage = std::max(2, baseDamage * 50 / 100 + ally.armorQuality);
            target.takeDamage(damage);
            target.applyWeakening(2, 10 + ally.currentRankIndex * 2);
            const bool killed = target.isDead();
            wave.removeDeadAndReplace();
            recordRecruitedAllyHitContribution(ally, damage, killed);
            recordRecruitedAllySupportContribution(ally, 2);
            ally.protectedThisRound = true;
            ally.activeSkillCooldown = recruitedAllyTechniqueCooldown(ally, 3, hasDirectOrder, forceTechnique);
            lines.push_back("- " + recruitedAllyTechniqueGesture(ally, ally.name + " plante sa garde devant la ligne") + " : " + targetName + " subit " + std::to_string(damage) + " dégâts et perd de la force. Couverture alliée active. CD " + std::to_string(ally.activeSkillCooldown) + " tours.");
            lines.push_back(recruitedAllyTechniqueMaturityLine(ally, hasDirectOrder));
            return true;
        }

        if (recruitedAllyJobContains(ally, "roublard") || recruitedAllyJobContains(ally, "assassin") || recruitedAllyJobContains(ally, "brigand"))
        {
            const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, orders);
            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            int totalDamage = 0;
            const int hitCount = 3 + (ally.currentRankIndex >= 4 ? 1 : 0);
            for (int hit = 0; hit < hitCount && !target.isDead(); ++hit)
            {
                const int damage = std::max(2, baseDamage * (hit == 0 ? 60 : 42) / 100 + random.between(0, 3));
                target.takeDamage(damage);
                totalDamage += damage;
            }
            ElementalAffinitySystem::applyPoison(target, 2, std::max(1, 1 + ally.currentRankIndex / 2));
            ElementalAffinitySystem::applyBleeding(target, 2, std::max(1, 1 + ally.weaponQuality / 4));
            const bool killed = target.isDead();
            wave.removeDeadAndReplace();
            recordRecruitedAllyHitContribution(ally, totalDamage, killed);
            ally.activeSkillCooldown = recruitedAllyTechniqueCooldown(ally, 4, hasDirectOrder, forceTechnique);
            lines.push_back("- " + recruitedAllyTechniqueGesture(ally, ally.name + " glisse dans l'angle mort") + " : " + targetName + " subit " + std::to_string(hitCount) + " coups, " + std::to_string(totalDamage) + " dégâts, poison et saignement. CD " + std::to_string(ally.activeSkillCooldown) + " tours.");
            lines.push_back(recruitedAllyTechniqueMaturityLine(ally, hasDirectOrder));
            return true;
        }

        if ((recruitedAllyJobContains(ally, "archer") || recruitedAllyJobContains(ally, "mage") || recruitedAllyJobContains(ally, "lancier")) && wave.getActiveEnemyCount() >= 2)
        {
            const int maxHits = std::min(wave.getActiveEnemyCount(), 2 + (ally.currentRankIndex >= 3 ? 1 : 0));
            int totalDamage = 0;
            std::vector<std::string> touched;
            for (int i = 0; i < maxHits && wave.hasActiveEnemies(); ++i)
            {
                const int targetIndex = std::min(i, wave.getActiveEnemyCount() - 1);
                Monster& target = wave.getActiveEnemy(targetIndex);
                const std::string targetName = target.getName();
                const int damage = std::max(2, baseDamage * 70 / 100 + random.between(0, 3));
                target.takeDamage(damage);
                totalDamage += damage;
                const bool killed = target.isDead();
                if (killed) ++ally.finishBlows;
                touched.push_back(targetName + " (" + std::to_string(damage) + ")");
                wave.removeDeadAndReplace();
            }
            ally.damageDealt += totalDamage;
            ally.activeSkillCooldown = recruitedAllyTechniqueCooldown(ally, 3, hasDirectOrder, forceTechnique);
            lines.push_back("- " + recruitedAllyTechniqueGesture(ally, ally.name + " coupe plusieurs lignes ennemies") + " : " + std::to_string(totalDamage) + " dégâts répartis sur " + std::to_string(static_cast<int>(touched.size())) + " cible(s). CD " + std::to_string(ally.activeSkillCooldown) + " tours.");
            lines.push_back(recruitedAllyTechniqueMaturityLine(ally, hasDirectOrder));
            return true;
        }

        if (recruitedAllyJobContains(ally, "soigneur") || recruitedAllyJobContains(ally, "mage d'appui") || recruitedAllyJobContains(ally, "intendant"))
        {
            const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, orders);
            Monster& target = wave.getActiveEnemy(targetIndex);
            const std::string targetName = target.getName();
            const int damage = std::max(2, baseDamage * 55 / 100);
            target.takeDamage(damage);
            target.applyVulnerability(2, 15 + std::min(8, ally.supportKitQuality));
            const bool killed = target.isDead();
            wave.removeDeadAndReplace();
            recordRecruitedAllyHitContribution(ally, damage, killed);
            const int supportValue = std::max(1, ally.currentRankIndex + 1 + ally.supportKitQuality / 2);
            ally.healingDone += supportValue;
            recordRecruitedAllySupportContribution(ally, 2);
            ally.activeSkillCooldown = recruitedAllyTechniqueCooldown(ally, 4, hasDirectOrder, forceTechnique);
            lines.push_back("- " + recruitedAllyTechniqueGesture(ally, ally.name + " désigne une faille") + " : " + targetName + " subit " + std::to_string(damage) + " dégâts et une vulnérabilité 2 tours. CD " + std::to_string(ally.activeSkillCooldown) + " tours.");
            lines.push_back(recruitedAllyTechniqueMaturityLine(ally, hasDirectOrder));
            return true;
        }

        const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, orders);
        Monster& target = wave.getActiveEnemy(targetIndex);
        const std::string targetName = target.getName();
        int totalDamage = 0;
        const int hitCount = 2;
        for (int hit = 0; hit < hitCount && !target.isDead(); ++hit)
        {
            const int damage = std::max(2, baseDamage * (hit == 0 ? 75 : 55) / 100 + random.between(0, 3));
            target.takeDamage(damage);
            totalDamage += damage;
        }
        const bool killed = target.isDead();
        wave.removeDeadAndReplace();
        recordRecruitedAllyHitContribution(ally, totalDamage, killed);
        ally.activeSkillCooldown = recruitedAllyTechniqueCooldown(ally, 3, hasDirectOrder, forceTechnique);
        lines.push_back("- " + recruitedAllyTechniqueGesture(ally, ally.name + " cherche son propre rythme") + " : " + targetName + " subit " + std::to_string(hitCount) + " frappe(s), " + std::to_string(totalDamage) + " dégâts. CD " + std::to_string(ally.activeSkillCooldown) + " tours.");
        lines.push_back(recruitedAllyTechniqueMaturityLine(ally, hasDirectOrder));
        return true;
    }

    int chooseRecruitedAllyTargetIndex(const RecruitedAllyCombatSupport& ally, EnemyCombatQueue& wave, Random& random)
    {
        const int activeCount = wave.getActiveEnemyCount();
        if (activeCount <= 1)
        {
            return 0;
        }
        if (recruitedAllyIsStillLearning(ally))
        {
            return 0;
        }
        if (recruitedAllyMaturityTier(ally) == 2 && random.between(1, 100) <= 45)
        {
            return random.between(0, activeCount - 1);
        }
        if (recruitedAllyJobContains(ally, "roublard") || recruitedAllyJobContains(ally, "archer"))
        {
            return random.between(0, activeCount - 1);
        }
        int bestIndex = 0;
        int bestHp = wave.getActiveEnemy(0).getHp();
        for (int i = 1; i < activeCount; ++i)
        {
            const int hp = wave.getActiveEnemy(i).getHp();
            if (hp < bestHp)
            {
                bestHp = hp;
                bestIndex = i;
            }
        }
        return bestIndex;
    }


    int findActiveEnemyIndexByName(EnemyCombatQueue& wave, const std::string& targetName)
    {
        if (targetName.empty())
        {
            return -1;
        }
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            if (wave.getActiveEnemy(i).getName() == targetName)
            {
                return i;
            }
        }
        return -1;
    }

    std::string selectActiveEnemyNameForOrder(EnemyCombatQueue& wave, const std::string& title, const std::string& id)
    {
        if (!wave.hasActiveEnemies())
        {
            MessageScreen::show(title, id + ".empty", {"Aucun ennemi actif à prioriser pour le moment."}, false);
            return "";
        }

        MenuScreen targetScreen(title, id);
        targetScreen.addLine("La priorité reste active jusqu'à la mort/disparition de cette cible.");
        targetScreen.addBackOption("Retour", id + ".back");
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            const Monster& enemy = wave.getActiveEnemy(i);
            targetScreen.addOption(
                i + 1,
                enemy.getName(),
                "PV " + std::to_string(enemy.getHp()) + "/" + std::to_string(enemy.getMaxHp()) + " | niv. " + std::to_string(enemy.getLevel()),
                true,
                id + ".target"
            );
        }
        const int choice = TerminalInterface::askMenuChoiceFromOptions(targetScreen, "Choisis une cible active.");
        Console::clear();
        if (choice <= 0 || choice > wave.getActiveEnemyCount())
        {
            return "";
        }
        return wave.getActiveEnemy(choice - 1).getName();
    }

    int chooseRecruitedAllyTargetIndexWithOrders(
        RecruitedAllyCombatSupport& ally,
        EnemyCombatQueue& wave,
        Random& random,
        RecruitedAllyOrderState& orders
    )
    {
        RecruitedAllyUniqueOrder& unique = orders.uniqueOrders[ally.name];
        if (unique.hasPriorityTarget)
        {
            const int index = findActiveEnemyIndexByName(wave, unique.priorityTargetName);
            if (index >= 0)
            {
                return index;
            }
            unique.hasPriorityTarget = false;
            unique.priorityTargetName.clear();
        }

        if (orders.groupPriorityTarget)
        {
            const int index = findActiveEnemyIndexByName(wave, orders.groupPriorityTargetName);
            if (index >= 0)
            {
                return index;
            }
            orders.groupPriorityTarget = false;
            orders.groupPriorityTargetName.clear();
        }

        if (orders.groupSpreadTargetsOneTurn && wave.getActiveEnemyCount() > 1)
        {
            return static_cast<int>(stableAllyHash(ally.name + ":spread:" + std::to_string(ally.turnsTaken)) % static_cast<unsigned>(wave.getActiveEnemyCount()));
        }

        return chooseRecruitedAllyTargetIndex(ally, wave, random);
    }

    bool executeRequestedAllySwitch(
        Player& player,
        RecruitedAllyCombatSupport& ally,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        RecruitedAllyUniqueOrder& unique,
        std::vector<std::string>& lines
    )
    {
        if (!unique.switchRequested || unique.switchInName.empty())
        {
            return false;
        }

        auto reserveIt = std::find_if(reserveAllies.begin(), reserveAllies.end(), [&](const RecruitedAllyCombatSupport& reserve) {
            return reserve.name == unique.switchInName;
        });
        if (reserveIt == reserveAllies.end())
        {
            lines.push_back("- " + ally.name + " devait switch, mais la réserve demandée n'est plus disponible.");
            unique.switchRequested = false;
            unique.switchInName.clear();
            return true;
        }

        const std::string outgoingName = ally.name;
        const std::string incomingName = reserveIt->name;
        RecruitedAllyCombatSupport incoming = *reserveIt;
        reserveAllies.erase(reserveIt);
        reserveAllies.push_back(ally);

        player.recordCanonicalEvent("switch_recrues_combat", outgoingName + "->" + incomingName, "Switch tactique temporaire en combat", 1);

        lines.push_back("- " + outgoingName + " applique le switch temporaire demandé avec " + incomingName + ". La place hors combat ne change pas.");
        ally = incoming;
        unique.switchRequested = false;
        unique.switchInName.clear();
        return true;
    }

    void clearOneTurnTeamOrders(RecruitedAllyOrderState& orders)
    {
        orders.groupHealOneTurn = false;
        orders.groupGuardOneTurn = false;
        orders.groupTechniqueOneTurn = false;
        orders.groupCombinedTechniqueOneTurn = false;
        orders.groupSpreadTargetsOneTurn = false;
        for (auto& entry : orders.uniqueOrders)
        {
            entry.second.healOneTurn = false;
            entry.second.guardOneTurn = false;
            entry.second.forceTechniqueOneTurn = false;
        }
    }

    void advanceRecruitedAllyRecoveryAfterEnemyResponse(RecruitedAllyOrderState& orders)
    {
        if (!DuoMasterySystem::combinedTechniqueReady(orders.combinedTechniqueRecoveryEnemyPhases))
        {
            orders.combinedTechniqueRecoveryEnemyPhases = DuoMasterySystem::advanceCombinedTechniqueRecovery(
                orders.combinedTechniqueRecoveryEnemyPhases);
        }
    }

    void openUniqueAllyOrderMenu(
        Player& player,
        EnemyCombatQueue& wave,
        std::vector<RecruitedAllyCombatSupport>& activeAllies,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        RecruitedAllyOrderState& orders
    )
    {
        (void)player;
        if (activeAllies.empty())
        {
            MessageScreen::show("CONSIGNE CIBLÉE", "combat.team_orders.unique.empty", {"Aucune recrue active à commander."}, false);
            return;
        }

        MenuScreen allyScreen("CONSIGNE CIBLÉE", "combat.team_orders.unique.select");
        allyScreen.addLine("Choisis une recrue précise. Donner une consigne ne consomme pas le tour du joueur.");
        allyScreen.addLine("Une recrue faible rang lit mal le combat seule, mais un ordre clair augmente ses chances d'agir correctement.");
        allyScreen.addLine("Switch existe seulement ici, jamais dans les consignes de groupe, et se déclenche au tour de la recrue concernée.");
        allyScreen.addBackOption("Retour", "combat.team_orders.unique.back");
        for (std::size_t i = 0; i < activeAllies.size(); ++i)
        {
            const RecruitedAllyCombatSupport& ally = activeAllies[i];
            allyScreen.addOption(
                static_cast<int>(i + 1),
                ally.name,
                ally.job + " | rang " + recruitedAllyRankName(ally.currentRankIndex) + " | niv. " + std::to_string(ally.estimatedLevel),
                true,
                "combat.team_orders.unique.ally"
            );
        }
        const int allyChoice = TerminalInterface::askMenuChoiceFromOptions(allyScreen, "Choisis une recrue.");
        Console::clear();
        if (allyChoice <= 0 || allyChoice > static_cast<int>(activeAllies.size()))
        {
            return;
        }

        RecruitedAllyCombatSupport& ally = activeAllies[static_cast<std::size_t>(allyChoice - 1)];
        RecruitedAllyUniqueOrder& unique = orders.uniqueOrders[ally.name];
        MenuScreen orderScreen("CONSIGNE CIBLÉE — " + ally.name, "combat.team_orders.unique.action");
        orderScreen.addLine("Consignes de soin/protection : actives 1 tour, à redire chaque fois.");
        orderScreen.addLine("Maturité : " + recruitedAllyMaturityName(ally) + " | les ordres aident surtout les recrues peu expérimentées.");
        orderScreen.addLine("Priorité de cible : reste active jusqu'à la mort/disparition de la cible.");
        orderScreen.addBackOption("Retour", "combat.team_orders.unique.action.back");
        orderScreen.addOption(1, "Demander du soin", "La recrue tente de soigner/couvrir le chef à son prochain tour. Consigne 1 tour.", true, "combat.team_orders.unique.heal");
        orderScreen.addOption(2, "Prioriser une cible", "La recrue visera cette cible jusqu'à sa mort/disparition.", wave.hasActiveEnemies(), "combat.team_orders.unique.focus");
        orderScreen.addOption(3, "Switch", "Échange cette recrue avec une recrue en réserve. Déclenché au tour de la recrue.", !reserveAllies.empty(), "combat.team_orders.unique.switch");
        orderScreen.addOption(4, "Protéger le chef", "Posture prudente pendant 1 tour : un peu moins de dégâts, un peu plus de couverture.", true, "combat.team_orders.unique.guard");
        orderScreen.addOption(5, "Forcer une technique", "Si sa technique est prête, la recrue l'utilisera à son prochain tour. Consigne 1 tour.", ally.activeSkillCooldown <= 0, "combat.team_orders.unique.technique");
        orderScreen.addOption(6, "Annuler la consigne ciblée", "Retire soin/protection/switch/priorité/technique pour cette recrue.", true, "combat.team_orders.unique.clear");
        const int orderChoice = TerminalInterface::askMenuChoiceFromOptions(orderScreen, "Choisis la consigne ciblée.");
        Console::clear();

        if (orderChoice == 1)
        {
            unique.healOneTurn = true;
            MessageScreen::show("CONSIGNE DONNÉE", "combat.team_orders.unique.heal.done", {ally.name + " tentera de soigner/couvrir au prochain tour.", "Cette consigne expire après le tour allié."}, false);
        }
        else if (orderChoice == 2)
        {
            const std::string targetName = selectActiveEnemyNameForOrder(wave, "PRIORITÉ UNIQUE", "combat.team_orders.unique.focus.target");
            if (!targetName.empty())
            {
                unique.hasPriorityTarget = true;
                unique.priorityTargetName = targetName;
                MessageScreen::show("PRIORITÉ ENREGISTRÉE", "combat.team_orders.unique.focus.done", {ally.name + " priorisera " + targetName + " jusqu'à sa mort/disparition."}, false);
            }
        }
        else if (orderChoice == 3)
        {
            MenuScreen reserveScreen("SWITCH — RÉSERVE", "combat.team_orders.unique.switch.reserve");
            reserveScreen.addLine("Le switch consommera le tour de " + ally.name + ", pas le tour du joueur.");
            reserveScreen.addBackOption("Retour", "combat.team_orders.unique.switch.back");
            for (std::size_t i = 0; i < reserveAllies.size(); ++i)
            {
                const RecruitedAllyCombatSupport& reserve = reserveAllies[i];
                reserveScreen.addOption(
                    static_cast<int>(i + 1),
                    reserve.name,
                    reserve.job + " | rang " + recruitedAllyRankName(reserve.currentRankIndex) + " | niv. " + std::to_string(reserve.estimatedLevel),
                    true,
                    "combat.team_orders.unique.switch.target"
                );
            }
            const int reserveChoice = TerminalInterface::askMenuChoiceFromOptions(reserveScreen, "Choisis la recrue en réserve.");
            Console::clear();
            if (reserveChoice > 0 && reserveChoice <= static_cast<int>(reserveAllies.size()))
            {
                unique.switchRequested = true;
                unique.switchInName = reserveAllies[static_cast<std::size_t>(reserveChoice - 1)].name;
                MessageScreen::show("SWITCH PLANIFIÉ", "combat.team_orders.unique.switch.done", {ally.name + " switchera avec " + unique.switchInName + " pendant son propre tour."}, false);
            }
        }
        else if (orderChoice == 4)
        {
            unique.guardOneTurn = true;
            MessageScreen::show("CONSIGNE DONNÉE", "combat.team_orders.unique.guard.done", {ally.name + " jouera plus prudemment pendant 1 tour."}, false);
        }
        else if (orderChoice == 5)
        {
            unique.forceTechniqueOneTurn = true;
            MessageScreen::show("TECHNIQUE DEMANDÉE", "combat.team_orders.unique.technique.done", {ally.name + " utilisera sa technique active au prochain tour si la situation le permet.", "Maturité " + recruitedAllyMaturityName(ally) + " : l'ordre aide, mais ne transforme pas une recrue en vétéran.", "Cette consigne expire après le tour allié."}, false);
        }
        else if (orderChoice == 6)
        {
            orders.uniqueOrders.erase(ally.name);
            MessageScreen::show("CONSIGNE ANNULÉE", "combat.team_orders.unique.clear.done", {"Les consignes ciblées de " + ally.name + " sont retirées."}, false);
        }
    }

    void openGroupAllyOrderMenu(EnemyCombatQueue& wave, RecruitedAllyOrderState& orders)
    {
        MenuScreen screen("CONSIGNE DE GROUPE", "combat.team_orders.group");
        screen.addLine("Consignes de soin/protection : actives 1 tour, à redire chaque fois.");
        screen.addLine("Les recrues faibles réagissent mieux avec une consigne directe qu'avec une consigne de groupe trop vague.");
        screen.addLine("Priorité de cible : reste active jusqu'à la mort/disparition de la cible.");
        screen.addLine("Switch indisponible ici : le switch est uniquement une consigne ciblée.");
        screen.addBackOption("Retour", "combat.team_orders.group.back");
        screen.addOption(1, "Demander un soin de groupe", "Un soutien capable déploiera un vrai soin de zone. La réserve n'en reçoit qu'un écho de 5 à 15 % selon la portée.", true, "combat.team_orders.group.heal");
        screen.addOption(2, "Prioriser une cible", "Toute l'équipe active visera cette cible jusqu'à sa mort/disparition.", wave.hasActiveEnemies(), "combat.team_orders.group.focus");
        screen.addOption(3, "Tenir la ligne", "Consigne prudente 1 tour : moins d'agression, plus de couverture.", true, "combat.team_orders.group.guard");
        screen.addOption(4, "Percée coordonnée", "Les recrues dont la technique est prête tenteront chacune leur technique ce tour. Consigne 1 tour.", true, "combat.team_orders.group.technique");
        const bool combinedTechniqueRecovered = DuoMasterySystem::combinedTechniqueReady(orders.combinedTechniqueRecoveryEnemyPhases);
        screen.addOption(5, "Technique combinée",
            combinedTechniqueRecovered
                ? "Les deux recrues actives dépensent leurs deux tours dans une seule technique de duo si leurs techniques sont prêtes."
                : "Le duo doit encore laisser passer " + std::to_string(orders.combinedTechniqueRecoveryEnemyPhases) + " riposte(s) ennemie(s) avant une nouvelle technique combinée.",
            combinedTechniqueRecovered, "combat.team_orders.group.combo");
        screen.addOption(6, "Répartir les cibles", "L'équipe évite de tous taper la même cible ce tour, utile contre plusieurs ennemis.", wave.getActiveEnemyCount() >= 2, "combat.team_orders.group.spread");
        screen.addOption(7, "Annuler la priorité de groupe", "Retire la cible prioritaire globale.", orders.groupPriorityTarget, "combat.team_orders.group.clear_focus");
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis la consigne de groupe.");
        Console::clear();
        if (choice == 1)
        {
            orders.groupHealOneTurn = true;
            MessageScreen::show("CONSIGNE DE GROUPE", "combat.team_orders.group.heal.done", {"Un soutien disponible tentera un vrai soin de groupe ce tour.", "La ligne reçoit l'effet principal ; la réserve ne peut recevoir que 5 à 15 % selon la portée.", "Cette consigne expire après le tour allié."}, false);
        }
        else if (choice == 2)
        {
            const std::string targetName = selectActiveEnemyNameForOrder(wave, "PRIORITÉ DE GROUPE", "combat.team_orders.group.focus.target");
            if (!targetName.empty())
            {
                orders.groupPriorityTarget = true;
                orders.groupPriorityTargetName = targetName;
                MessageScreen::show("PRIORITÉ DE GROUPE", "combat.team_orders.group.focus.done", {"Toute l'équipe priorisera " + targetName + " jusqu'à sa mort/disparition."}, false);
            }
        }
        else if (choice == 3)
        {
            orders.groupGuardOneTurn = true;
            MessageScreen::show("CONSIGNE DE GROUPE", "combat.team_orders.group.guard.done", {"L'équipe tiendra la ligne pendant 1 tour."}, false);
        }
        else if (choice == 4)
        {
            orders.groupTechniqueOneTurn = true;
            MessageScreen::show("PERCÉE COORDONNÉE", "combat.team_orders.group.technique.done", {"Les recrues dont la technique est prête tenteront une action active ce tour.", "Cette consigne expire après le tour allié."}, false);
        }
        else if (choice == 5)
        {
            if (DuoMasterySystem::combinedTechniqueReady(orders.combinedTechniqueRecoveryEnemyPhases))
            {
                orders.groupCombinedTechniqueOneTurn = true;
                MessageScreen::show("TECHNIQUE COMBINÉE", "combat.team_orders.group.combo.done", {"Les deux recrues actives tenteront une vraie technique de duo.", "Le duo consomme leurs deux tours alliés et demande leurs deux techniques prêtes.", "Après une tentative réelle, une fenêtre de récupération empêche de chaîner A -> B -> A -> B avant de laisser le combat respirer."}, false);
            }
        }
        else if (choice == 6)
        {
            orders.groupSpreadTargetsOneTurn = true;
            MessageScreen::show("CIBLES RÉPARTIES", "combat.team_orders.group.spread.done", {"L'équipe répartira ses cibles pendant 1 tour au lieu de tunnel automatiquement.", "Cette consigne est utile contre plusieurs ennemis, moins contre un boss seul."}, false);
        }
        else if (choice == 7)
        {
            orders.groupPriorityTarget = false;
            orders.groupPriorityTargetName.clear();
            MessageScreen::show("PRIORITÉ ANNULÉE", "combat.team_orders.group.clear_focus.done", {"La priorité de groupe est retirée."}, false);
        }
    }


    int recruitedAllyHpPercent(const RecruitedAllyCombatSupport& ally)
    {
        return ally.maxHp > 0 ? std::clamp(ally.currentHp * 100 / ally.maxHp, 0, 100) : 100;
    }

    int employerMemberLoyalty(const Player& player, const RecruitedAllyCombatSupport& ally)
    {
        if (player.getCanonicalJournalCategoryTotal("cheat_trustmebro") > 0) return 100;
        if (ally.teamOrigin != AllyTeamOrigin::EmployerTeam) return 70;
        for (const EmployerTeamMember& member : EmployerTeamRosterSystem::rosterFor(player))
        {
            if (member.memberId == ally.persistentMemberId || member.name == ally.name)
            {
                return member.loyalty;
            }
        }
        return 55;
    }

    bool allyLooksStubbornForRotation(const RecruitedAllyCombatSupport& ally)
    {
        const std::string profile = lowerCopy(ally.trait + " " + ally.job + " " + ally.motivation);
        return profile.find("orgueill") != std::string::npos
            || profile.find("têtu") != std::string::npos
            || profile.find("tetu") != std::string::npos
            || profile.find("berser") != std::string::npos
            || profile.find("fanati") != std::string::npos;
    }

    bool openLeaderRotationMenu(
        Player& player,
        std::vector<RecruitedAllyCombatSupport>& activeAllies,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        Random& random,
        bool& rotationLockedForTurn)
    {
        if (rotationLockedForTurn)
        {
            MessageScreen::show(
                "ROTATION DÉJÀ TENTÉE",
                "combat.team_rotation.locked",
                {"Une rotation a déjà été refusée pendant ce tour.", "Le chef garde son action normale, mais ne peut pas répéter l'ordre jusqu'à son prochain tour."},
                false
            );
            return false;
        }
        if (activeAllies.empty() || reserveAllies.empty())
        {
            MessageScreen::show("ROTATION DU CHEF", "combat.team_rotation.unavailable", {"Il faut au moins un membre actif et un membre en réserve."}, false);
            return false;
        }

        const int maxSwaps = std::min({2, static_cast<int>(activeAllies.size()), static_cast<int>(reserveAllies.size())});
        MenuScreen countScreen("ROTATION DU CHEF", "combat.team_rotation.count");
        countScreen.addLine("La rotation échange 1 ou 2 membres entre la ligne et la réserve.");
        countScreen.addLine("Une rotation réussie consomme TON tour de chef. Une rotation refusée ne consomme pas ton tour, mais bloque toute nouvelle rotation jusqu'au prochain tour.");
        countScreen.addBackOption("Retour", "combat.team_rotation.back");
        countScreen.addOption(1, "Échanger 1 membre", "Remplace un membre actif par un membre en réserve.", true, "combat.team_rotation.one");
        countScreen.addOption(2, "Échanger 2 membres", "Réorganise deux places principales en une seule action de chef.", maxSwaps >= 2, "combat.team_rotation.two");
        const int requested = TerminalInterface::askMenuChoiceFromOptions(countScreen, "Choisis le nombre d'échanges.");
        Console::clear();
        if (requested <= 0 || requested > maxSwaps) return false;

        std::vector<std::string> outgoingNames;
        std::vector<std::string> incomingNames;
        std::set<std::string> usedOutgoing;
        std::set<std::string> usedIncoming;
        for (int step = 0; step < requested; ++step)
        {
            MenuScreen outScreen("ROTATION — SORTIE", "combat.team_rotation.out");
            outScreen.addLine("Choisis le membre actif qui quitte temporairement la ligne.");
            outScreen.addBackOption("Annuler toute la rotation", "combat.team_rotation.cancel");
            int option = 1;
            std::vector<std::size_t> outMap;
            for (std::size_t i = 0; i < activeAllies.size(); ++i)
            {
                if (usedOutgoing.count(activeAllies[i].name) > 0) continue;
                outMap.push_back(i);
                outScreen.addOption(option++, activeAllies[i].name,
                    activeAllies[i].job + " | PV " + std::to_string(activeAllies[i].currentHp) + "/" + std::to_string(activeAllies[i].maxHp), true,
                    "combat.team_rotation.out.member");
            }
            const int outChoice = TerminalInterface::askMenuChoiceFromOptions(outScreen, "Choisis le membre à retirer.");
            Console::clear();
            if (outChoice <= 0 || outChoice > static_cast<int>(outMap.size())) return false;
            const RecruitedAllyCombatSupport& outgoing = activeAllies[outMap[static_cast<std::size_t>(outChoice - 1)]];

            MenuScreen inScreen("ROTATION — ENTRÉE", "combat.team_rotation.in");
            inScreen.addLine("Choisis le membre de réserve appelé sur la ligne.");
            inScreen.addBackOption("Annuler toute la rotation", "combat.team_rotation.cancel");
            option = 1;
            std::vector<std::size_t> inMap;
            for (std::size_t i = 0; i < reserveAllies.size(); ++i)
            {
                if (usedIncoming.count(reserveAllies[i].name) > 0) continue;
                inMap.push_back(i);
                inScreen.addOption(option++, reserveAllies[i].name,
                    reserveAllies[i].job + " | PV " + std::to_string(reserveAllies[i].currentHp) + "/" + std::to_string(reserveAllies[i].maxHp), true,
                    "combat.team_rotation.in.member");
            }
            const int inChoice = TerminalInterface::askMenuChoiceFromOptions(inScreen, "Choisis le membre à faire entrer.");
            Console::clear();
            if (inChoice <= 0 || inChoice > static_cast<int>(inMap.size())) return false;
            const RecruitedAllyCombatSupport& incoming = reserveAllies[inMap[static_cast<std::size_t>(inChoice - 1)]];

            outgoingNames.push_back(outgoing.name);
            incomingNames.push_back(incoming.name);
            usedOutgoing.insert(outgoing.name);
            usedIncoming.insert(incoming.name);
        }

        // On vérifie tous les refus AVANT de muter la formation : une rotation de deux membres reste atomique.
        for (int step = 0; step < requested; ++step)
        {
            auto outIt = std::find_if(activeAllies.begin(), activeAllies.end(), [&](const auto& a) { return a.name == outgoingNames[step]; });
            auto inIt = std::find_if(reserveAllies.begin(), reserveAllies.end(), [&](const auto& a) { return a.name == incomingNames[step]; });
            if (outIt == activeAllies.end() || inIt == reserveAllies.end()) return false;
            const int refusalHp = std::min(recruitedAllyHpPercent(*outIt), recruitedAllyHpPercent(*inIt));
            const int loyalty = std::min(employerMemberLoyalty(player, *outIt), employerMemberLoyalty(player, *inIt));
            const bool stubborn = allyLooksStubbornForRotation(*outIt) || allyLooksStubbornForRotation(*inIt);
            if (CombatLeaderRotationSystem::refusesRotation(refusalHp, loyalty, stubborn, random))
            {
                rotationLockedForTurn = true;
                MessageScreen::show(
                    "ROTATION REFUSÉE",
                    "combat.team_rotation.refused",
                    {
                        (refusalHp == recruitedAllyHpPercent(*outIt) ? outIt->name : inIt->name) + " refuse brièvement le changement alors que son état est très mauvais.",
                        "Ton tour de chef n'est PAS consommé.",
                        "En revanche, l'action Rotation est verrouillée jusqu'à ton prochain tour : impossible de spammer l'ordre jusqu'à obtenir un oui."
                    },
                    false
                );
                return false;
            }
        }

        std::vector<std::string> lines;
        for (int step = 0; step < requested; ++step)
        {
            auto outIt = std::find_if(activeAllies.begin(), activeAllies.end(), [&](const auto& a) { return a.name == outgoingNames[step]; });
            auto inIt = std::find_if(reserveAllies.begin(), reserveAllies.end(), [&](const auto& a) { return a.name == incomingNames[step]; });
            if (outIt == activeAllies.end() || inIt == reserveAllies.end()) continue;
            std::swap(*outIt, *inIt);
            RecruitedAllyCombatRoster::markReserveEntryDelay(*outIt);
            inIt->enteredFromReserveThisPhase = false;
            lines.push_back("- " + outgoingNames[step] + " recule ; " + incomingNames[step] + " prend sa place. Son entrée n'accorde aucun tour gratuit pendant cette phase.");
        }
        lines.push_back("La rotation est réussie : le tour du chef est consommé.");
        player.recordCanonicalEvent("rotations_tactiques_alliees", "chef:" + player.getName(),
            player.getName() + " a consommé son tour pour " + std::to_string(requested) + " rotation(s) tactique(s).", requested);
        MessageScreen::show("ROTATION DU CHEF", "combat.team_rotation.success", lines, false);
        return true;
    }

    enum class EmployerLeaderRotationOutcome
    {
        NoAction,
        TurnConsumed,
        PlayerCalledIn
    };

    EmployerLeaderRotationOutcome maybeEmployerLeaderUsesRotation(
        Player& player,
        std::vector<RecruitedAllyCombatSupport>& activeAllies,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        Random& random,
        int roundNumber,
        bool playerWaiting)
    {
        auto leaderIt = std::find_if(activeAllies.begin(), activeAllies.end(), [](const RecruitedAllyCombatSupport& ally) {
            return ally.teamLeader && ally.currentHp > 0;
        });
        if (leaderIt == activeAllies.end() || (reserveAllies.empty() && !playerWaiting)) return EmployerLeaderRotationOutcome::NoAction;

        int lowHp = 0;
        int critical = 0;
        for (const auto& ally : activeAllies)
        {
            if (ally.teamLeader || ally.currentHp <= 0) continue;
            const int hp = recruitedAllyHpPercent(ally);
            if (hp <= 45) ++lowHp;
            if (hp <= 24) ++critical;
        }

        CombatLeaderRotationContext context;
        context.leaderPresent = true;
        context.activeCount = static_cast<int>(activeAllies.size());
        context.reserveCount = static_cast<int>(reserveAllies.size()) + (playerWaiting ? 1 : 0);
        context.lowHpActiveCount = lowHp;
        context.criticalHpActiveCount = critical;
        context.playerWaiting = playerWaiting;
        context.roundNumber = roundNumber;
        context.leaderIntelligence = std::clamp(48 + leaderIt->estimatedLevel / 2 + leaderIt->currentRankIndex * 5, 35, 95);
        context.groupSolidarity = std::clamp((employerMemberLoyalty(player, *leaderIt) + 55) / 2, 20, 95);
        const CombatLeaderRotationDecision decision = CombatLeaderRotationSystem::decide(context, random);
        if (!decision.attempt) return EmployerLeaderRotationOutcome::NoAction;

        auto outgoingIt = std::min_element(activeAllies.begin(), activeAllies.end(), [](const auto& a, const auto& b) {
            if (a.teamLeader != b.teamLeader) return !a.teamLeader;
            if (a.teamLeader && b.teamLeader) return false;
            return static_cast<long long>(a.currentHp) * std::max(1, b.maxHp)
                < static_cast<long long>(b.currentHp) * std::max(1, a.maxHp);
        });
        if (outgoingIt == activeAllies.end() || outgoingIt->teamLeader) return EmployerLeaderRotationOutcome::NoAction;

        if (playerWaiting)
        {
            ++leaderIt->turnsTaken;
            if (leaderIt->activeSkillCooldown > 0) --leaderIt->activeSkillCooldown;
            const std::string outgoingName = outgoingIt->name;
            reserveAllies.push_back(*outgoingIt);
            activeAllies.erase(outgoingIt);
            player.recordCanonicalEvent("team_employer_reserve_entries",
                "rotation_jour_" + std::to_string(player.getWorldDaysElapsed()),
                "Le chef a consommé son tour pour rappeler le joueur depuis la liste d'attente et faire reculer " + outgoingName + ".", 1);
            MessageScreen::show(
                "ROTATION DU CHEF",
                "combat.pve.employer_team.player_reserve.rotation_call",
                {
                    TeamLeadershipSystem::leaderDisplayName(player) + " renonce à son action pour modifier la formation.",
                    outgoingName + " recule vers la réserve et le chef te fait entrer sur la ligne.",
                    "Aucun membre n'avait besoin de mourir : l'entrée vient d'une vraie décision tactique."
                },
                false
            );
            return EmployerLeaderRotationOutcome::PlayerCalledIn;
        }

        const int wanted = std::min({decision.swapCount, static_cast<int>(reserveAllies.size()), static_cast<int>(activeAllies.size()) - 1});
        if (wanted <= 0) return EmployerLeaderRotationOutcome::NoAction;

        std::vector<std::pair<std::string, std::string>> plan;
        std::vector<std::size_t> outgoingIndices;
        for (std::size_t i = 0; i < activeAllies.size(); ++i)
        {
            if (!activeAllies[i].teamLeader) outgoingIndices.push_back(i);
        }
        std::sort(outgoingIndices.begin(), outgoingIndices.end(), [&](std::size_t a, std::size_t b) {
            return recruitedAllyHpPercent(activeAllies[a]) < recruitedAllyHpPercent(activeAllies[b]);
        });
        for (int step = 0; step < wanted; ++step)
        {
            const auto& outgoing = activeAllies[outgoingIndices[static_cast<std::size_t>(step)]];
            const auto& incoming = reserveAllies[static_cast<std::size_t>(step)];
            const int refusalHp = std::min(recruitedAllyHpPercent(outgoing), recruitedAllyHpPercent(incoming));
            const int loyalty = std::min(employerMemberLoyalty(player, outgoing), employerMemberLoyalty(player, incoming));
            if (CombatLeaderRotationSystem::refusesRotation(refusalHp, loyalty,
                    allyLooksStubbornForRotation(outgoing) || allyLooksStubbornForRotation(incoming), random))
            {
                MessageScreen::show(
                    "ROTATION REFUSÉE",
                    "combat.pve.employer_team.rotation_refused",
                    {
                        TeamLeadershipSystem::leaderDisplayName(player) + " tente de modifier la ligne, mais un membre très mal en point refuse le changement.",
                        "Le chef garde donc son tour normal, mais ne retentera pas une rotation pendant ce même tour."
                    }, false);
                return EmployerLeaderRotationOutcome::NoAction;
            }
            plan.push_back({outgoing.name, incoming.name});
        }

        std::vector<std::string> lines = {TeamLeadershipSystem::leaderDisplayName(player) + " sacrifie son tour pour réorganiser l'escouade."};
        for (const auto& rotation : plan)
        {
            const std::string outName = rotation.first;
            const std::string inName = rotation.second;
            auto outIt = std::find_if(activeAllies.begin(), activeAllies.end(), [&](const auto& a) { return a.name == outName; });
            auto inIt = std::find_if(reserveAllies.begin(), reserveAllies.end(), [&](const auto& a) { return a.name == inName; });
            if (outIt == activeAllies.end() || inIt == reserveAllies.end()) continue;
            std::swap(*outIt, *inIt);
            RecruitedAllyCombatRoster::markReserveEntryDelay(*outIt);
            inIt->enteredFromReserveThisPhase = false;
            lines.push_back("- " + outName + " recule ; " + inName + " entre sans action immédiate gratuite.");
        }
        ++leaderIt->turnsTaken;
        if (leaderIt->activeSkillCooldown > 0) --leaderIt->activeSkillCooldown;
        player.recordCanonicalEvent("rotations_tactiques_employeur", TeamLeadershipSystem::leaderDisplayName(player),
            "Le chef employeur a consommé son tour pour réorganiser " + std::to_string(plan.size()) + " place(s).", static_cast<int>(plan.size()));
        MessageScreen::show("ROTATION DE L'ESCOUADE", "combat.pve.employer_team.rotation", lines, false);
        return EmployerLeaderRotationOutcome::TurnConsumed;
    }

    bool openTeamOrdersMenu(
        Player& player,
        EnemyCombatQueue& wave,
        std::vector<RecruitedAllyCombatSupport>& activeAllies,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        RecruitedAllyOrderState& orders,
        Random& random,
        bool& leaderRotationLockedForTurn
    )
    {
        const TeamLeadershipContext leadership = TeamLeadershipSystem::contextFor(player);
        if (!leadership.canIssueTacticalOrders)
        {
            std::vector<std::string> blockedLines;
            blockedLines.push_back("Tu es membre de cette équipe, pas son chef actuel.");
            blockedLines.push_back("Chef reconnu : " + TeamLeadershipSystem::leaderDisplayName(player) + ".");
            if (!leadership.organizationName.empty())
            {
                blockedLines.push_back("Organisation : " + leadership.organizationName + ".");
            }
            blockedLines.push_back("Les consignes tactiques globales dépendent du chef. Ton propre tour reste sous ton contrôle.");
            MessageScreen::show("CONSIGNES DU CHEF", "combat.team_orders.external_leader", blockedLines, false);
            return false;
        }

        if (activeAllies.empty())
        {
            MessageScreen::show("CONSIGNES D'ÉQUIPE", "combat.team_orders.empty", {"Aucune recrue équipée ne peut recevoir de consigne."}, false);
            return false;
        }

        MenuScreen screen("CONSIGNES D'ÉQUIPE", "combat.team_orders");
        screen.addLine("Donner une consigne classique ne consomme pas le tour du joueur.");
        screen.addLine("Rotation du chef : échange 1 ou 2 membres avec la réserve et consomme le tour seulement si elle réussit.");
        screen.addLine("Une recrue faible rang lit moins bien le combat, utilise moins de techniques et suit mieux une consigne claire qu'un instinct flou.");
        screen.addLine("Consigne ciblée : vise une recrue précise. Switch disponible uniquement ici.");
        screen.addLine("Consigne de groupe : consigne globale. Soin/protection/technique/répartition = 1 tour ; priorité cible = jusqu'à mort/disparition.");
        if (orders.groupPriorityTarget)
        {
            screen.addLine("Priorité de groupe actuelle : " + orders.groupPriorityTargetName + ".");
        }
        screen.addBackOption("Retour", "combat.team_orders.back");
        screen.addOption(1, "Consigne ciblée", "Demander du soin, prioriser une cible, protéger le chef ou switch une recrue précise.", true, "combat.team_orders.unique");
        screen.addOption(2, "Consigne de groupe", "Soin global, cible prioritaire, tenir la ligne, percée coordonnée ou cibles réparties. Pas de switch ici.", true, "combat.team_orders.group");
        screen.addOption(3, "Voir l'état des consignes", "Récapitulatif des priorités et consignes 1 tour.", true, "combat.team_orders.status");
        screen.addOption(4, "Rotation du chef", "Échanger 1 ou 2 membres actifs avec la réserve. Une réussite consomme ton tour de chef.", !reserveAllies.empty() && !activeAllies.empty() && !leaderRotationLockedForTurn, "combat.team_orders.leader_rotation");
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un type de consigne.");
        Console::clear();
        if (choice == 1)
        {
            openUniqueAllyOrderMenu(player, wave, activeAllies, reserveAllies, orders);
        }
        else if (choice == 2)
        {
            openGroupAllyOrderMenu(wave, orders);
        }
        else if (choice == 3)
        {
            std::vector<std::string> lines;
            lines.push_back("Actifs : " + std::to_string(activeAllies.size()) + " | réserve : " + std::to_string(reserveAllies.size()) + ".");
            lines.push_back("Soin groupe 1 tour : " + std::string(orders.groupHealOneTurn ? "oui" : "non") + ".");
            lines.push_back("Tenir ligne 1 tour : " + std::string(orders.groupGuardOneTurn ? "oui" : "non") + ".");
            lines.push_back("Percée coordonnée 1 tour : " + std::string(orders.groupTechniqueOneTurn ? "oui" : "non") + ".");
            lines.push_back("Technique combinée 1 tour : " + std::string(orders.groupCombinedTechniqueOneTurn ? "oui" : "non") + ".");
            lines.push_back("Récupération technique combinée : "
                + (!DuoMasterySystem::combinedTechniqueReady(orders.combinedTechniqueRecoveryEnemyPhases)
                    ? std::to_string(orders.combinedTechniqueRecoveryEnemyPhases) + " riposte(s) ennemie(s) restante(s)"
                    : std::string("prête")) + ".");
            lines.push_back("Répartition cibles 1 tour : " + std::string(orders.groupSpreadTargetsOneTurn ? "oui" : "non") + ".");
            lines.push_back("Priorité groupe : " + std::string(orders.groupPriorityTarget ? orders.groupPriorityTargetName : "aucune") + ".");
            for (const auto& entry : orders.uniqueOrders)
            {
                const RecruitedAllyUniqueOrder& order = entry.second;
                lines.push_back("- " + entry.first + " : soin=" + std::string(order.healOneTurn ? "oui" : "non")
                    + " | protection=" + std::string(order.guardOneTurn ? "oui" : "non")
                    + " | technique=" + std::string(order.forceTechniqueOneTurn ? "oui" : "non")
                    + " | priorité=" + std::string(order.hasPriorityTarget ? order.priorityTargetName : "aucune")
                    + " | switch=" + std::string(order.switchRequested ? order.switchInName : "non") + ".");
            }
            MessageScreen::show("ÉTAT DES CONSIGNES", "combat.team_orders.status", lines, false);
        }
        else if (choice == 4)
        {
            return openLeaderRotationMenu(player, activeAllies, reserveAllies, random, leaderRotationLockedForTurn);
        }
        return false;
    }

    void displayRecruitedAllyCombatStart(const Player& player, const std::vector<RecruitedAllyCombatSupport>& allies)
    {
        if (allies.empty())
        {
            return;
        }
        const bool employerTeam = usesEmployerTeam(allies);
        std::vector<std::string> lines;
        lines.push_back(employerTeam
            ? "L'escouade employeuse entre dans le combat : 3 places principales partagées, dont une occupée par le joueur."
            : "Les recrues équipées entrent maintenant dans le vrai combat PvE, en soutien simple joueur + 2 alliés max.");
        lines.push_back(employerTeam
            ? "Le chef opérationnel agit toujours en premier. Les autres collègues suivent ensuite la formation de l'équipe."
            : "Une recrue bas rang observe moins de choses, utilise moins de techniques et progresse surtout avec des ordres clairs.");
        const TeamLeadershipContext leadership = TeamLeadershipSystem::contextFor(player);
        lines.push_back("Chef actuel : " + TeamLeadershipSystem::leaderDisplayName(player) + ".");
        lines.push_back(leadership.playerIsLeader
            ? "Placement : le joueur dirige, puis chaîne manuelle si elle existe, sinon fallback rang/niveau."
            : "Placement : tu es membre du groupe ; l'ordre général dépend du chef externe et non de tes menus personnels.");
        for (const RecruitedAllyCombatSupport& ally : allies)
        {
            lines.push_back("- " + std::string(ally.teamLeader ? "[CHEF] " : "") + ally.name + " | " + ally.job + " | rang " + recruitedAllyRankName(ally.currentRankIndex) + " | niv. " + std::to_string(ally.estimatedLevel) + " | " + recruitedAllyHealthLine(ally)
                + (employerTeam ? " | salarié par l'organisation." : " | part combat " + std::to_string(ally.normalizedShare) + "%."));
            lines.push_back("  Présence : " + recruitedAllyReadinessLine(ally, player) + ".");
        }
        MessageScreen::show(
            employerTeam ? "ESCOUADE EMPLOYEUSE" : "RECRUES ÉQUIPÉES",
            employerTeam ? "combat.pve.employer_team.start" : "combat.pve.recruited_allies.start",
            lines,
            false
        );
    }

    bool executeRecruitedAllyCombinedTechnique(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        std::vector<RecruitedAllyCombatSupport>& allies,
        RecruitedAllyOrderState& orders,
        std::vector<std::string>& lines
    )
    {
        if (!orders.groupCombinedTechniqueOneTurn || allies.size() < 2 || !wave.hasActiveEnemies())
        {
            return false;
        }
        if (!DuoMasterySystem::combinedTechniqueReady(orders.combinedTechniqueRecoveryEnemyPhases))
        {
            lines.push_back("Technique combinée différée : le duo doit encore laisser passer "
                + std::to_string(orders.combinedTechniqueRecoveryEnemyPhases) + " riposte(s) ennemie(s).");
            return false;
        }

        RecruitedAllyCombatSupport& first = allies[0];
        RecruitedAllyCombatSupport& second = allies[1];
        if (first.enteredFromReserveThisPhase || second.enteredFromReserveThisPhase)
        {
            lines.push_back("Technique combinée différée : une recrue vient d'entrer depuis la réserve et doit d'abord attendre sa vraie fenêtre d'initiative.");
            return false;
        }
        if (first.currentHp <= 0 || second.currentHp <= 0)
        {
            lines.push_back("Technique combinée annulée : les deux recrues doivent être encore debout.");
            return false;
        }
        if (first.activeSkillCooldown > 0 || second.activeSkillCooldown > 0)
        {
            lines.push_back("Technique combinée impossible : les deux techniques personnelles doivent être prêtes.");
            return false;
        }

        ++first.turnsTaken;
        ++second.turnsTaken;

        const std::string pairKey = DuoMasterySystem::pairKey(first.name, second.name);
        const int duoExperience = DuoMasterySystem::experience(player, pairKey);
        const int duoMastery = DuoMasterySystem::masteryTier(duoExperience);
        const DuoBondProfile bondBefore = DuoMasterySystem::bondProfile(player, first.name, second.name);
        const int maturity = recruitedAllyMaturityTier(first) + recruitedAllyMaturityTier(second);
        const int coordinationChance = DuoMasterySystem::coordinationChance(
            maturity,
            first.currentRankIndex + second.currentRankIndex,
            duoExperience,
            bondBefore.relationshipScore,
            player.hasPassiveSkill("church_oath_bonds")
        );
        const int coordinationRoll = random.between(1, 100);
        const bool fullCoordination = coordinationRoll <= coordinationChance;
        const bool partialCoordination = !fullCoordination
            && coordinationRoll <= std::min(100, coordinationChance + 14 + duoMastery * 3 + bondBefore.bondTier * 2);

        if (!fullCoordination && !partialCoordination)
        {
            first.activeSkillCooldown = 1;
            second.activeSkillCooldown = 1;
            orders.combinedTechniqueRecoveryEnemyPhases = std::max(
                orders.combinedTechniqueRecoveryEnemyPhases,
                DuoMasterySystem::combinedTechniqueRecoveryAfterAttempt());
            player.recordCanonicalEvent(
                "techniques_combinees_ratees",
                pairKey,
                first.name + " + " + second.name + " : coordination rompue",
                1
            );
            lines.push_back("TECHNIQUE COMBINÉE ROMPUE : " + first.name + " et " + second.name + " engagent bien leurs deux tours, mais leurs rythmes se croisent trop tôt.");
            lines.push_back("- Maîtrise : " + DuoMasterySystem::masteryLabel(duoMastery)
                + " | lien : " + DuoMasterySystem::bondLabel(bondBefore.bondTier)
                + " | coordination " + std::to_string(coordinationChance) + "%.");
            lines.push_back("- Une paire novice peut réellement hésiter ou rater son timing. Les échecs vécus apprennent un peu, mais ne se farment pas comme une victoire.");
            return true;
        }

        DuoTechniquePlan duoPlan = DuoMasterySystem::techniqueForJobs(first.job, second.job);
        std::string techniqueName = duoPlan.name;
        if (duoMastery >= 3)
        {
            techniqueName = "Signature commune — " + techniqueName;
        }
        int powerPercent = duoPlan.powerPercent + duoMastery * 4;
        int vulnerability = duoPlan.vulnerabilityPercent;
        int weakening = duoPlan.weakening;
        int healPercent = duoPlan.playerHealPercent;
        int guardPercent = duoPlan.playerGuardPercent;

        if (partialCoordination)
        {
            powerPercent = std::max(45, powerPercent * 55 / 100);
            vulnerability = 0;
            weakening = 0;
            healPercent /= 2;
            guardPercent /= 2;
        }

        Monster& target = wave.getActiveEnemy(0);
        const std::string targetName = target.getName();
        int baseDamage = std::max(6,
            first.estimatedLevel / 2 + second.estimatedLevel / 2
            + first.weaponQuality + second.weaponQuality
            + first.currentRankIndex * 3 + second.currentRankIndex * 3
            + maturity * 2 + random.between(4, 11));
        if (player.hasPassiveSkill("church_oath_bonds"))
        {
            baseDamage += 2;
        }
        const int damage = std::max(3, baseDamage * powerPercent / 100);
        target.takeDamage(damage);
        if (!target.isDead() && vulnerability > 0) target.applyVulnerability(2, vulnerability);
        if (!target.isDead() && weakening > 0) target.applyWeakening(2, weakening);
        const int duoHeal = healPercent > 0
            ? std::max(1, player.getMaxHp() * healPercent / 100)
            : 0;
        if (duoHeal > 0) player.heal(duoHeal);
        if (guardPercent > 0) player.applyGuardBoost(2, guardPercent);
        const bool killed = target.isDead();
        wave.removeDeadAndReplace();

        first.damageDealt += damage / 2;
        second.damageDealt += damage - damage / 2;
        if (killed)
        {
            ++first.finishBlows;
            ++second.finishBlows;
        }
        const int masteryCooldownReduction = duoMastery >= 2 ? 1 : 0;
        const int partialPenalty = partialCoordination ? 1 : 0;
        first.activeSkillCooldown = std::max(2, 4 + partialPenalty - recruitedAllyMaturityTier(first) / 2 - masteryCooldownReduction);
        second.activeSkillCooldown = std::max(2, 4 + partialPenalty - recruitedAllyMaturityTier(second) / 2 - masteryCooldownReduction);
        orders.combinedTechniqueRecoveryEnemyPhases = std::max(
                orders.combinedTechniqueRecoveryEnemyPhases,
                DuoMasterySystem::combinedTechniqueRecoveryAfterAttempt());

        const std::string category = partialCoordination ? "techniques_combinees_partielles" : "techniques_combinees_alliees";
        player.recordCanonicalEvent(
            category,
            pairKey + ":" + duoPlan.name,
            first.name + " + " + second.name + " : " + duoPlan.name + (partialCoordination ? " (exécution partielle)" : ""),
            1
        );
        DuoMasterySystem::recordCoordinatedOrderDay(player, first.name, second.name, "combat_pve");
        player.recordHistoricalEvent(
            partialCoordination ? "ally_combo_partial" : "ally_combo",
            "duo:" + pairKey,
            first.name + " et " + second.name + (partialCoordination ? " rattrapent partiellement " : " exécutent ") + duoPlan.name + " sur " + targetName + "."
        );

        const int newExperience = DuoMasterySystem::experience(player, pairKey);
        const int newMastery = DuoMasterySystem::masteryTier(newExperience);
        if (newMastery > duoMastery)
        {
            player.recordHistoricalEvent(
                "ally_combo_mastery",
                "duo:" + pairKey,
                first.name + " et " + second.name + " atteignent le palier "
                    + DuoMasterySystem::masteryLabel(newMastery) + "."
            );
        }
        const DuoBondProfile bondAfter = DuoMasterySystem::bondProfile(player, first.name, second.name);

        lines.push_back(std::string(partialCoordination ? "TECHNIQUE COMBINÉE RATTRAPÉE — " : "TECHNIQUE COMBINÉE — ")
            + techniqueName + " : " + first.name + " et " + second.name + " consomment leurs deux tours alliés.");
        lines.push_back("- Maîtrise : " + DuoMasterySystem::masteryLabel(newMastery)
            + " | expérience commune " + std::to_string(newExperience)
            + " | lien " + DuoMasterySystem::bondLabel(bondAfter.bondTier)
            + " (" + std::to_string(bondAfter.bondScore) + "/100)." );
        if (partialCoordination)
        {
            lines.push_back("- Exécution partielle : la paire sauve son mouvement, mais perd les effets tactiques principaux et une partie de la puissance.");
        }
        lines.push_back("- Cible : " + targetName + " | dégâts : " + std::to_string(damage) + (killed ? " | cible vaincue." : "."));
        if (vulnerability > 0 && !killed) lines.push_back("- Effet duo : vulnérabilité " + std::to_string(vulnerability) + "% pendant 2 tours.");
        if (weakening > 0 && !killed) lines.push_back("- Effet duo : affaiblissement " + std::to_string(weakening) + " pendant 2 tours.");
        if (duoHeal > 0) lines.push_back("- Effet duo : soin du chef +" + std::to_string(duoHeal) + " PV.");
        if (guardPercent > 0) lines.push_back("- Effet duo : garde du chef +" + std::to_string(guardPercent) + "% pendant 2 tours.");
        lines.push_back("- Récupération : " + first.name + " CD " + std::to_string(first.activeSkillCooldown) + " | " + second.name + " CD " + std::to_string(second.activeSkillCooldown) + ".");
        return true;
    }

    enum class AllyTurnPhase
    {
        All,
        LeaderOnly,
        NonLeaderOnly
    };

    int healthiestReserveIndex(const std::vector<RecruitedAllyCombatSupport>& reserveAllies)
    {
        if (reserveAllies.empty()) return -1;
        int best = 0;
        for (int i = 1; i < static_cast<int>(reserveAllies.size()); ++i)
        {
            if (recruitedAllyHpPercent(reserveAllies[static_cast<std::size_t>(i)])
                > recruitedAllyHpPercent(reserveAllies[static_cast<std::size_t>(best)]))
            {
                best = i;
            }
        }
        return best;
    }

    int allyFearPercentForSwitch(const RecruitedAllyCombatSupport& ally, int enemyCount)
    {
        const std::string profile = lowerCopy(ally.trait + " " + ally.job + " " + ally.motivation);
        int fear = std::max(0, 55 - recruitedAllyHpPercent(ally));
        fear += std::max(0, enemyCount - 1) * 7;
        if (profile.find("peure") != std::string::npos || profile.find("prudent") != std::string::npos) fear += 14;
        if (profile.find("berser") != std::string::npos || profile.find("fanati") != std::string::npos) fear -= 14;
        return std::clamp(fear, 0, 100);
    }

    bool tryIndividualAllySwitch(
        Player& player,
        EnemyCombatQueue& wave,
        RecruitedAllyCombatSupport& ally,
        std::vector<RecruitedAllyCombatSupport>& allActiveAllies,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        Random& random,
        std::vector<std::string>& lines)
    {
        if (reserveAllies.empty() || ally.currentHp <= 0 || ally.teamLeader) return false;

        const bool employerTeam = ally.teamOrigin == AllyTeamOrigin::EmployerTeam;
        const bool personalLeaderPresent = !employerTeam && !player.isDead();
        auto employerLeaderIt = std::find_if(allActiveAllies.begin(), allActiveAllies.end(), [](const auto& candidate) {
            return candidate.teamLeader && candidate.currentHp > 0;
        });
        const bool employerLeaderPresent = employerTeam && employerLeaderIt != allActiveAllies.end();
        const bool leaderPresent = personalLeaderPresent || employerLeaderPresent;

        const int hp = recruitedAllyHpPercent(ally);
        CombatIndividualSwitchContext context;
        context.reserveAvailable = true;
        context.leaderPresent = leaderPresent;
        context.memberIsLeader = false;
        context.hpPercent = hp;
        context.fearPercent = allyFearPercentForSwitch(ally, wave.getActiveEnemyCount());
        context.dangerPercent = std::clamp(35 + wave.getActiveEnemyCount() * 12 + std::max(0, 45 - hp), 0, 100);
        context.intelligence = std::clamp(35 + ally.estimatedLevel / 2 + ally.currentRankIndex * 7, 10, 95);
        context.discipline = std::clamp(35 + RecruitedAllyCombatRoster::maturityTier(ally) * 11, 20, 95);
        context.loyaltyOrCohesion = employerTeam
            ? employerMemberLoyalty(player, ally)
            : std::clamp(45 + RecruitedAllyCombatRoster::relationshipScore(player, ally.name) / 2, 35, 95);
        context.stubborn = allyLooksStubbornForRotation(ally);
        context.survivalUrgent = hp <= 16;
        context.tacticalOpportunity = hp <= 38 && healthiestReserveIndex(reserveAllies) >= 0
            && recruitedAllyHpPercent(reserveAllies[static_cast<std::size_t>(healthiestReserveIndex(reserveAllies))]) >= hp + 20;

        const CombatIndividualSwitchDecision decision = CombatIndividualSwitchSystem::decide(context, random);
        if (!decision.wantsSwitch) return false;

        const int reserveIndex = healthiestReserveIndex(reserveAllies);
        if (reserveIndex < 0) return false;
        RecruitedAllyCombatSupport& incoming = reserveAllies[static_cast<std::size_t>(reserveIndex)];

        bool authorized = decision.actsWithoutPermission || !leaderPresent;
        bool bypassed = decision.actsWithoutPermission && leaderPresent;

        if (decision.asksPermission && personalLeaderPresent)
        {
            MenuScreen permission("DEMANDE DE SWITCH", "combat.ally.individual_switch.permission");
            permission.addLine(ally.name + " demande l'autorisation de quitter la ligne.");
            permission.addLine("Remplaçant proposé : " + incoming.name + " | " + std::to_string(recruitedAllyHpPercent(incoming)) + "% PV.");
            permission.addLine("État de " + ally.name + " : " + std::to_string(hp) + "% PV. Accepter consommera SON tour, pas le tien.");
            permission.addBackOption("Refuser", "combat.ally.individual_switch.deny");
            permission.addOption(1, "Autoriser", "Valider le changement demandé.", true, "combat.ally.individual_switch.allow");
            const int choice = TerminalInterface::askMenuChoiceFromOptions(permission, "Autoriser ce switch ?");
            Console::clear();
            authorized = choice == 1;
            if (!authorized)
            {
                lines.push_back("- " + ally.name + " demande à sortir, mais le chef refuse. La recrue reste en ligne et peut encore agir.");
                player.recordCanonicalEvent("switchs_individuels_refuses", ally.name, "Demande de switch refusée par le chef", 1);
                return false;
            }
        }
        else if (decision.asksPermission && employerLeaderPresent)
        {
            CombatSwitchAuthorizationContext auth;
            auth.leaderIntelligence = std::clamp(48 + employerLeaderIt->estimatedLevel / 2 + employerLeaderIt->currentRankIndex * 5, 35, 95);
            auth.leaderSolidarity = employerMemberLoyalty(player, *employerLeaderIt);
            auth.outgoingHpPercent = hp;
            auth.incomingHpPercent = recruitedAllyHpPercent(incoming);
            auth.tacticalNeedPercent = context.dangerPercent;
            auth.outgoingCritical = hp <= 20;
            auth.incomingFresher = auth.incomingHpPercent >= hp + 15;
            authorized = CombatIndividualSwitchSystem::leaderAuthorizes(auth, random);
            if (!authorized)
            {
                lines.push_back("- " + ally.name + " demande à sortir ; " + employerLeaderIt->name + " refuse et maintient la formation.");
                return false;
            }
        }

        if (!authorized) return false;

        const std::string outgoingName = ally.name;
        const std::string incomingName = incoming.name;
        std::swap(ally, incoming);
        player.recordCanonicalEvent(
            bypassed ? "switchs_individuels_sans_ordre" : "switchs_individuels_autorises",
            outgoingName + "->" + incomingName,
            bypassed ? "Switch individuel sans attendre l'autorisation du chef" : "Switch individuel autorisé",
            1);
        if (bypassed && employerTeam)
        {
            EmployerUnauthorizedSwitchContext switchReview;
            switchReview.actorName = outgoingName;
            switchReview.leaderPresent = employerLeaderPresent;
            switchReview.witnessCount = std::max(0, static_cast<int>(allActiveAllies.size()) - 2)
                + static_cast<int>(reserveAllies.size());
            switchReview.outgoingHpPercent = hp;
            switchReview.incomingHpPercent = recruitedAllyHpPercent(ally);
            switchReview.fearPercent = context.fearPercent;
            switchReview.survivalUrgent = context.survivalUrgent;
            switchReview.tacticalOpportunity = context.tacticalOpportunity;
            switchReview.switchSucceeded = true;
            const EmployerObservedOrderActionAssessment review =
                EmployerTeamDisciplineSystem::assessUnauthorizedIndividualSwitch(switchReview);
            player.recordCanonicalEvent(
                "team_employer_unauthorized_switch_reviews",
                outgoingName + "->" + incomingName + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
                "verdict=" + EmployerTeamDisciplineSystem::orderObservationVerdictLabel(review.verdict)
                    + " | raison=" + review.reason,
                1);
            lines.push_back("- Relecture du chef : "
                + EmployerTeamDisciplineSystem::orderObservationVerdictLabel(review.verdict)
                + " — " + review.reason + ".");
        }
        if (bypassed)
        {
            lines.push_back("- " + outgoingName + " quitte la ligne SANS attendre l'autorisation (" + decision.reason + ") ; " + incomingName + " entre. Son tour est consommé.");
        }
        else
        {
            lines.push_back("- " + outgoingName + " obtient l'autorisation de switcher avec " + incomingName + ". Son tour est consommé.");
        }
        return true;
    }

    int applyReserveGroupHealSpillover(
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        int sourceHeal,
        CombatReserveEffectReach reach,
        Random& random,
        std::vector<std::string>& lines)
    {
        if (reserveAllies.empty() || sourceHeal <= 0 || reach == CombatReserveEffectReach::None) return 0;
        const CombatReserveSpillover spill = CombatReserveSpilloverSystem::roll(sourceHeal, reach, random);
        int total = 0;
        int touched = 0;
        for (auto& reserve : reserveAllies)
        {
            if (reserve.currentHp <= 0 || reserve.currentHp >= reserve.maxHp) continue;
            const int before = reserve.currentHp;
            reserve.currentHp = std::min(reserve.maxHp, reserve.currentHp + spill.amount);
            const int effective = std::max(0, reserve.currentHp - before);
            if (effective <= 0) continue;
            total += effective;
            ++touched;
        }
        if (touched > 0)
        {
            lines.push_back("  Débordement vers la réserve : " + std::to_string(spill.percent)
                + "% du soin de zone atteint " + std::to_string(touched) + " membre(s) en attente ("
                + CombatReserveSpilloverSystem::reachLabel(reach) + ").");
        }
        return total;
    }

    void playRecruitedAllyCombatTurns(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        std::vector<RecruitedAllyCombatSupport>& allies,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        RecruitedAllyOrderState& orders,
        AllyTurnPhase phase = AllyTurnPhase::All
    )
    {
        if (allies.empty() || !wave.hasActiveEnemies())
        {
            clearOneTurnTeamOrders(orders);
            return;
        }

        const bool employerTeam = usesEmployerTeam(allies);
        std::vector<std::string> lines;
        if (employerTeam)
        {
            if (phase == AllyTurnPhase::LeaderOnly)
                lines.push_back("Le chef de l'escouade employeuse ouvre le round avant le joueur.");
            else
                lines.push_back("Les autres collègues actifs de l'escouade agissent selon la formation du chef.");
        }
        else
        {
            lines.push_back("Les recrues équipées agissent après le joueur et les invocations, avant la riposte ennemie.");
            lines.push_back("Les consignes de soin/protection/technique/répartition expirent après ce tour allié. Les priorités de cible restent jusqu'à mort/disparition.");
        }

        if (employerTeam && phase == AllyTurnPhase::LeaderOnly)
        {
            const EmployerLeaderRotationOutcome rotation = maybeEmployerLeaderUsesRotation(
                player, allies, reserveAllies, random, 1, false);
            if (rotation == EmployerLeaderRotationOutcome::TurnConsumed)
            {
                return;
            }
        }

        if (phase == AllyTurnPhase::All && executeRecruitedAllyCombinedTechnique(player, wave, random, allies, orders, lines))
        {
            clearOneTurnTeamOrders(orders);
            MessageScreen::show("TOUR DES RECRUES ÉQUIPÉES", "combat.pve.recruited_allies.combo", lines, false);
            return;
        }

        bool acted = false;
        const std::vector<int> allyInitiativeOrder = RecruitedAllyInitiativeSystem::orderedActiveIndices(allies);
        for (int allyIndex : allyInitiativeOrder)
        {
            RecruitedAllyCombatSupport& ally = allies[static_cast<std::size_t>(allyIndex)];
            if (phase == AllyTurnPhase::LeaderOnly && !ally.teamLeader) continue;
            if (phase == AllyTurnPhase::NonLeaderOnly && ally.teamLeader) continue;
            if (!wave.hasActiveEnemies())
            {
                break;
            }
            if (RecruitedAllyCombatRoster::consumeReserveEntryDelay(ally))
            {
                acted = true;
                lines.push_back("- " + ally.name + " vient d'entrer depuis la réserve : aucune action gratuite pendant cette phase ; sa prochaine fenêtre normale reste intacte.");
                continue;
            }
            acted = true;
            ++ally.turnsTaken;
            if (ally.activeSkillCooldown > 0)
            {
                --ally.activeSkillCooldown;
            }
            ally.protectedThisRound = false;

            if (ally.persistentMemberId == "special:hestia")
            {
                if (ally.turnsTaken == 1) HestiaCompanionSystem::recordCombatParticipation(player);
                const HestiaCompanionProfile hestiaProfile = HestiaCompanionSystem::profile(player);
                const bool magicalAction = HestiaCompanionSystem::looksMagicOriented(hestiaProfile);

                HestiaAwakeningCombatResult awakening = HestiaCompanionSystem::tryAwakening(player, random, magicalAction);
                if (awakening.triggered && wave.hasActiveEnemies())
                {
                    Monster& target = wave.getActiveEnemy(0);
                    const std::string targetName = target.getName();
                    const int beforeHp = target.getHp();
                    target.takeDamage(std::max(1, awakening.bonusDamage));
                    const int effective = std::max(0, beforeHp - target.getHp());
                    const bool killed = target.isDead();
                    recordRecruitedAllyHitContribution(ally, effective, killed);
                    wave.removeDeadAndReplace();
                    lines.push_back("- Hestia déclenche une magie lumineuse qu'elle ne savait pas posséder : " + targetName + " subit " + std::to_string(effective) + " dégâts.");
                    lines.insert(lines.end(), awakening.lines.begin(), awakening.lines.end());
                    if (awakening.returnsToHeavens)
                    {
                        ally.currentHp = 0;
                        lines.push_back("- La présence d'Hestia se dissout après le retour complet de ses souvenirs. Ce n'est pas un KO : elle quitte réellement le monde matériel.");
                    }
                    continue;
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
                            lines.push_back("  Dégâts alliés accidentels : -" + std::to_string(damage) + " PV au joueur. Les maladresses d'Hestia ne peuvent pas achever directement le joueur.");
                        }
                    }
                    else if (mishap.mayHitAlly && mishap.maxFriendlyDamagePercent > 0)
                    {
                        auto friendly = std::find_if(allies.begin(), allies.end(), [&](const RecruitedAllyCombatSupport& candidate) {
                            return candidate.name != ally.name && candidate.currentHp > 1;
                        });
                        if (friendly != allies.end())
                        {
                            const int requested = std::max(1, friendly->maxHp * mishap.maxFriendlyDamagePercent / 100);
                            const int damage = std::min(requested, std::max(0, friendly->currentHp - 1));
                            friendly->currentHp -= damage;
                            lines.push_back("  " + friendly->name + " encaisse accidentellement " + std::to_string(damage) + " dégâts. Hestia ne peut pas mettre un allié KO par maladresse pure.");
                        }
                    }
                    continue;
                }
            }
            RecruitedAllyUniqueOrder& unique = orders.uniqueOrders[ally.name];
            const bool hasActiveOrder = unique.healOneTurn
                || unique.guardOneTurn
                || unique.forceTechniqueOneTurn
                || unique.hasPriorityTarget
                || unique.switchRequested
                || orders.groupHealOneTurn
                || orders.groupGuardOneTurn
                || orders.groupTechniqueOneTurn
                || orders.groupCombinedTechniqueOneTurn
                || orders.groupSpreadTargetsOneTurn
                || orders.groupPriorityTarget;
            lines.push_back("- Présence alliée : " + recruitedAllyPresenceLine(ally, player, wave) + ".");
            const std::string tacticalComment = recruitedAllyTacticalCommentLine(ally, player, wave, hasActiveOrder);
            if (!tacticalComment.empty())
            {
                lines.push_back("  Lecture alliée : " + tacticalComment + ".");
            }

            if (executeRequestedAllySwitch(player, ally, reserveAllies, unique, lines))
            {
                continue;
            }

            // Un membre peut aussi demander de lui-même à quitter la ligne. Normalement le chef
            // autorise/refuse ; peur, instinct ou jugement immédiat peuvent exceptionnellement court-circuiter la demande.
            if (tryIndividualAllySwitch(player, wave, ally, allies, reserveAllies, random, lines))
            {
                continue;
            }

            // La consigne "soin de groupe" devient un vrai soin de zone : un seul soutien la résout,
            // puis 5-15 % du soin déborde vers la réserve selon la portée réelle de son effet.
            if (orders.groupHealOneTurn && RecruitedAllyActionPlanner::isSupportProfile(ally))
            {
                const int pulse = std::max(4, 4 + ally.estimatedLevel / 15 + ally.currentRankIndex * 2 + ally.supportKitQuality);
                int effectiveTotal = 0;
                if (!player.isDead() && player.getHp() < player.getMaxHp())
                {
                    const int before = player.getHp();
                    player.heal(pulse);
                    effectiveTotal += std::max(0, player.getHp() - before);
                }
                for (auto& targetAlly : allies)
                {
                    if (targetAlly.currentHp <= 0 || targetAlly.currentHp >= targetAlly.maxHp) continue;
                    const int before = targetAlly.currentHp;
                    targetAlly.currentHp = std::min(targetAlly.maxHp, targetAlly.currentHp + pulse);
                    effectiveTotal += std::max(0, targetAlly.currentHp - before);
                }

                CombatReserveEffectReach reach = CombatReserveEffectReach::Edge;
                const std::string healerProfile = lowerCopy(ally.job + " " + ally.trait);
                if (ally.currentRankIndex >= 5 && (healerProfile.find("prêtre") != std::string::npos
                    || healerProfile.find("pretre") != std::string::npos
                    || healerProfile.find("lumomancien") != std::string::npos))
                    reach = CombatReserveEffectReach::Massive;
                else if (ally.currentRankIndex >= 3 || ally.supportKitQuality >= 5)
                    reach = CombatReserveEffectReach::Wide;

                effectiveTotal += applyReserveGroupHealSpillover(reserveAllies, pulse, reach, random, lines);
                ally.healingDone += effectiveTotal;
                recordRecruitedAllySupportContribution(ally, 3);
                orders.groupHealOneTurn = false;
                lines.push_back("- " + ally.name + " déploie un soin de groupe : puissance de base +" + std::to_string(pulse)
                    + " PV sur la ligne, avec un débordement borné vers la réserve.");
                continue;
            }

            RecruitedAllyOrderState effectiveOrders = orders;
            RecruitedAllyUniqueOrder& effectiveUnique = effectiveOrders.uniqueOrders[ally.name];
            bool orderedHeal = effectiveUnique.healOneTurn || effectiveOrders.groupHealOneTurn;
            bool orderedGuard = effectiveUnique.guardOneTurn || effectiveOrders.groupGuardOneTurn;
            bool orderedTechnique = effectiveUnique.forceTechniqueOneTurn || effectiveOrders.groupTechniqueOneTurn;
            bool directTechniqueContext = effectiveUnique.hasPriorityTarget || effectiveOrders.groupPriorityTarget || orderedTechnique;

            const int allyHpPercent = ally.maxHp > 0 ? std::clamp(ally.currentHp * 100 / ally.maxHp, 0, 100) : 0;
            RecruitedAllyOrderSafetyContext safetyContext;
            safetyContext.hpPercent = allyHpPercent;
            safetyContext.readinessPercent = ally.readinessPercent;
            safetyContext.moralePercent = ally.moralePercent;
            safetyContext.enemyCount = wave.getActiveEnemyCount();
            safetyContext.relationshipScore = employerTeam ? 0 : RecruitedAllyCombatRoster::relationshipScore(player, ally.name);
            safetyContext.bossContext = false;
            safetyContext.aggressiveForcedOrder = orderedTechnique || effectiveUnique.hasPriorityTarget || effectiveOrders.groupPriorityTarget;
            const RecruitedAllyOrderSafetyDecision safety = RecruitedAllyOrderSafetySystem::evaluate(safetyContext, random);
            if (safety.refused)
            {
                effectiveUnique.forceTechniqueOneTurn = false;
                effectiveUnique.hasPriorityTarget = false;
                effectiveOrders.groupTechniqueOneTurn = false;
                effectiveOrders.groupPriorityTarget = false;
                orderedTechnique = false;
                directTechniqueContext = false;
                lines.push_back("- " + ally.name + " refuse l'ordre offensif immédiat : " + safety.reason + ". Elle reste dans le combat et choisit une action de survie crédible au lieu de perdre son tour.");
                if (!player.isDead())
                {
                    player.recordCanonicalEvent(
                        "recrues_refus_ordres_dangereux",
                        ally.name,
                        "danger=" + std::to_string(safety.dangerScore) + " | hp=" + std::to_string(allyHpPercent) + " | ennemis=" + std::to_string(wave.getActiveEnemyCount()),
                        1);
                }
            }

            RecruitedAllyCombatSupport* woundedAlly = nullptr;
            for (auto& candidate : allies)
            {
                if (candidate.name == ally.name || candidate.currentHp <= 0) continue;
                if (woundedAlly == nullptr
                    || static_cast<long long>(candidate.currentHp) * std::max(1, woundedAlly->maxHp)
                        < static_cast<long long>(woundedAlly->currentHp) * std::max(1, candidate.maxHp))
                {
                    woundedAlly = &candidate;
                }
            }

            RecruitedAllyActionContext actionContext;
            actionContext.hasEnemy = wave.hasActiveEnemies();
            actionContext.enemyCount = wave.getActiveEnemyCount();
            actionContext.bossContext = false;
            actionContext.leaderPresent = !employerTeam && !player.isDead();
            actionContext.leaderHpPercent = (!employerTeam && player.getMaxHp() > 0) ? std::clamp(player.getHp() * 100 / player.getMaxHp(), 0, 100) : 0;
            actionContext.leaderCanBeHealed = !employerTeam && !player.isDead();
            actionContext.woundedAllyAvailable = woundedAlly != nullptr;
            actionContext.woundedAllyHpPercent = woundedAlly != nullptr && woundedAlly->maxHp > 0
                ? std::clamp(woundedAlly->currentHp * 100 / woundedAlly->maxHp, 0, 100)
                : 100;
            actionContext.orderedHeal = orderedHeal;
            actionContext.orderedGuard = orderedGuard;
            actionContext.orderedTechnique = orderedTechnique;
            actionContext.hasDirectOrder = directTechniqueContext;
            actionContext.techniqueAvailable = ally.activeSkillCooldown <= 0;
            actionContext.techniqueAllowed = true;
            actionContext.relationshipScore = employerTeam ? 0 : RecruitedAllyCombatRoster::relationshipScore(player, ally.name);

            const RecruitedAllyActionDecision action = RecruitedAllyActionPlanner::decide(ally, actionContext, random);

            if (action.type == RecruitedAllyActionType::SelfHeal)
            {
                const int selfHeal = std::max(5, ally.maxHp / 3);
                const int before = ally.currentHp;
                ally.currentHp = std::min(ally.maxHp, ally.currentHp + selfHeal);
                --ally.healingPotionCharges;
                const int effective = std::max(0, ally.currentHp - before);
                ally.healingDone += effective;
                recordRecruitedAllySupportContribution(ally, 1);
                lines.push_back("- " + ally.name + " boit une potion personnelle : +" + std::to_string(effective) + " PV (" + recruitedAllyHealthLine(ally) + ").");
                continue;
            }

            if (action.type == RecruitedAllyActionType::HealLeader && player.getHp() < player.getMaxHp())
            {
                const int healAmount = std::max(3, player.getMaxHp() / 18 + ally.currentRankIndex * 2 + ally.supportKitQuality + random.between(0, 4) + (orderedHeal ? 2 : 0));
                const int before = player.getHp();
                player.heal(healAmount);
                const int effective = std::max(0, player.getHp() - before);
                ally.healingDone += effective;
                recordRecruitedAllySupportContribution(ally, 2);
                lines.push_back("- " + ally.name + (orderedHeal ? " suit la consigne de soin : +" : " couvre le chef : +") + std::to_string(effective) + " PV.");
                continue;
            }

            if (action.type == RecruitedAllyActionType::HealAlly && woundedAlly != nullptr)
            {
                const int healAmount = std::max(3, woundedAlly->maxHp / 10 + ally.currentRankIndex * 2 + ally.supportKitQuality + random.between(0, 3));
                const int before = woundedAlly->currentHp;
                woundedAlly->currentHp = std::min(woundedAlly->maxHp, woundedAlly->currentHp + healAmount);
                const int effective = std::max(0, woundedAlly->currentHp - before);
                ally.healingDone += effective;
                recordRecruitedAllySupportContribution(ally, 2);
                lines.push_back("- " + ally.name + " soigne " + woundedAlly->name + " : +" + std::to_string(effective) + " PV.");
                continue;
            }

            if (action.type == RecruitedAllyActionType::Technique
                && executeRecruitedAllyActiveTechnique(ally, wave, random, effectiveOrders, lines, true))
            {
                continue;
            }

            if (action.type == RecruitedAllyActionType::Hold)
            {
                lines.push_back("- " + ally.name + " conserve sa position : " + action.reason + ".");
                continue;
            }

            const bool plannedGuard = action.type == RecruitedAllyActionType::Guard;
            const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, effectiveOrders);
            Monster& target = wave.getActiveEnemy(targetIndex);
            const int playerLevelContribution = employerTeam ? 0 : player.getLevel() / 5;
            int damage = std::max(3, ally.estimatedLevel / 2 + ally.currentRankIndex * 5 + ally.weaponQuality * 2 + playerLevelContribution + random.between(2, 10));
            if (recruitedAllyJobContains(ally, "gardien"))
            {
                damage = std::max(2, damage - 2);
            }
            if (recruitedAllyIsStillLearning(ally))
            {
                damage = std::max(2, damage * 78 / 100);
            }
            else if (recruitedAllyMaturityTier(ally) == 2)
            {
                damage = std::max(2, damage * 92 / 100);
            }
            if (plannedGuard)
            {
                ally.protectedThisRound = true;
                damage = std::max(2, damage - 3);
                const int guardHeal = std::max(1, ally.currentRankIndex + 1 + ally.supportKitQuality / 3);
                if (player.getHp() < player.getMaxHp())
                {
                    player.heal(guardHeal);
                    ally.healingDone += guardHeal;
                }
                recordRecruitedAllySupportContribution(ally, 1);
                lines.push_back("- " + ally.name + " tient la ligne : couverture légère du chef" + (player.getHp() < player.getMaxHp() ? "" : "") + ".");
            }
            if (recruitedAllyJobContains(ally, "roublard") && target.getHp() * 2 < target.getMaxHp())
            {
                damage += std::max(2, ally.currentRankIndex + 3);
            }
            const std::string targetName = target.getName();
            target.takeDamage(damage);
            const bool killed = target.isDead();
            const std::string targetResultLine = recruitedAllyTargetResultLine(ally, target, killed);
            wave.removeDeadAndReplace();
            recordRecruitedAllyHitContribution(ally, damage, killed);

            const bool hasUniqueFocus = effectiveOrders.uniqueOrders[ally.name].hasPriorityTarget;
            const bool hasGroupFocus = effectiveOrders.groupPriorityTarget;
            std::string orderHint;
            if (hasUniqueFocus || hasGroupFocus)
            {
                orderHint = " (priorité suivie).";
            }
            else if (effectiveOrders.groupSpreadTargetsOneTurn)
            {
                orderHint = " (cibles réparties).";
            }
            else if (recruitedAllyJobContains(ally, "archer") || recruitedAllyJobContains(ally, "roublard"))
            {
                orderHint = " (instinct/cible parfois aléatoire).";
            }
            else
            {
                orderHint = ".";
            }
            lines.push_back("- " + ally.name + " attaque " + targetName + " : " + std::to_string(damage) + " dégâts" + orderHint + " " + targetResultLine);
        }
        if (phase != AllyTurnPhase::LeaderOnly)
        {
            clearOneTurnTeamOrders(orders);
        }
        if (acted)
        {
            MessageScreen::show(
                employerTeam ? "TOUR DE L'ESCOUADE EMPLOYEUSE" : "TOUR DES RECRUES ÉQUIPÉES",
                employerTeam ? "combat.pve.employer_team.turn" : "combat.pve.recruited_allies.turn",
                lines,
                false
            );
        }
    }

    void persistRecruitedAllyVitals(Player& player, const RecruitedAllyCombatSupport& ally, const std::string& status)
    {
        if (ally.name.empty() || ally.temporaryNoReward)
        {
            return;
        }
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

    void persistRecruitedAllyVitals(Player& player, const std::vector<RecruitedAllyCombatSupport>& allies, const std::string& status)
    {
        for (const RecruitedAllyCombatSupport& ally : allies)
        {
            persistRecruitedAllyVitals(player, ally, status);
        }
    }

    void sendRecruitedAllyNameToInfirmary(
        Player& player,
        const std::string& recruitName,
        int recoveryDays,
        const std::string& severity,
        int exitHealthPercent,
        const std::string& origin,
        std::vector<std::string>& lines
    )
    {
        if (recruitName.empty())
        {
            return;
        }
        const int readyDay = player.getWorldDaysElapsed() + std::max(1, recoveryDays);
        std::ostringstream label;
        label << recruitName
              << " | pret_jour=" << readyDay
              << " | gravite=" << severity
              << " | ville=" << player.getCurrentCityId()
              << " | sortie_pct=" << std::clamp(exitHealthPercent, 1, 100)
              << " | origine=" << origin;
        player.recordCanonicalEvent("recrues_infirmerie_sejours", recruitName, label.str(), 1);
        player.recordCanonicalEvent("recrues_evacuees_infirmerie", recruitName, recruitName + " amené à l'infirmerie", 1);
        lines.push_back("- " + recruitName + " est pris en charge par l'infirmerie. Retour possible jour " + std::to_string(readyDay) + ", au seuil de PV imposé par la difficulté.");
    }

    void markRecruitedAllyAwaitingInfirmaryTransfer(
        Player& player,
        const RecruitedAllyCombatSupport& ally,
        int recoveryDaysHint,
        const std::string& severity,
        DifficultyMode difficulty,
        std::vector<std::string>& lines
    )
    {
        if (ally.teamOrigin == AllyTeamOrigin::EmployerTeam)
        {
            const int readyDay = player.getWorldDaysElapsed() + std::max(1, recoveryDaysHint);
            EmployerTeamRosterSystem::setMemberVitals(
                player,
                ally.persistentMemberId,
                0,
                false,
                "ko_escouade | ready_day=" + std::to_string(readyDay) + " | gravite=" + severity
            );
            player.recordCanonicalEvent(
                "team_employer_roster_history",
                ally.persistentMemberId + ":ko:day_" + std::to_string(player.getWorldDaysElapsed()),
                ally.name + " tombe au combat pour l'équipe employeuse | retour estimé jour=" + std::to_string(readyDay),
                1
            );
            lines.push_back("- " + ally.name + " tombe à 0 PV. L'escouade employeuse le retire de la ligne ; retour estimé jour " + std::to_string(readyDay) + ".");
            return;
        }
        persistRecruitedAllyVitals(player, ally, "ko_a_evacuer");
        std::ostringstream label;
        label << ally.name
              << " | gravite=" << severity
              << " | ville=" << player.getCurrentCityId()
              << " | jour_chute=" << player.getWorldDaysElapsed()
              << " | repos_base=" << std::max(1, recoveryDaysHint)
              << " | sortie_pct=" << recruitedAllyRespawnHealthPercent(difficulty);
        player.recordCanonicalEvent("recrues_a_evacuer", ally.name, label.str(), 1);
        player.recordCanonicalEvent("recrues_tombees_combat", ally.name, ally.name + " tombe au combat et attend une évacuation", 1);
        lines.push_back("- " + ally.name + " tombe à 0 PV. Elle n'est pas téléportée : il faudra l'amener à l'infirmerie avant qu'elle redevienne opérationnelle.");
    }

    void transferPendingFallenRecruitsToInfirmary(
        Player& player,
        DifficultyMode difficulty,
        Random& random,
        const std::string& contextId,
        std::vector<std::string>& lines
    )
    {
        int moved = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "recrues_a_evacuer" || record.count <= 0)
            {
                continue;
            }
            const std::string recruitName = record.key.empty() ? record.label : record.key;
            if (recruitName.empty())
            {
                continue;
            }
            if (localJournalCount(player, "recrues_evacuees_infirmerie", recruitName) >= record.count)
            {
                continue;
            }
            const std::string severity = extractLabelStringField(record.label, "gravite", "surveillance");
            int recoveryDays = extractLabelIntField(record.label, "repos_base", severity == "grave" ? random.between(2, 5) : random.between(1, 2));
            if (contextId.find("joueur_ko") != std::string::npos || contextId.find("player") != std::string::npos)
            {
                recoveryDays = std::max(recoveryDays, severity == "grave" ? 3 : 1);
            }
            const int exitPercent = extractLabelIntField(record.label, "sortie_pct", recruitedAllyRespawnHealthPercent(difficulty));
            sendRecruitedAllyNameToInfirmary(player, recruitName, recoveryDays, severity, exitPercent, contextId, lines);
            ++moved;
        }
        if (moved > 0)
        {
            lines.push_back("Les recrues tombées suivent le transfert vers l'infirmerie : personne ne redevient disponible sans soin ni récupération.");
        }
    }

    void resolveRecruitedAllyInjuriesAfterEnemyTurn(
        Player& player,
        EnemyCombatQueue& wave,
        std::vector<RecruitedAllyCombatSupport>& allies,
        RecruitedAllyOrderState& orders,
        Random& random,
        DifficultyMode difficulty,
        int playerHpBeforeEnemyTurn
    )
    {
        if (allies.empty())
        {
            return;
        }

        const int hpLost = std::max(0, playerHpBeforeEnemyTurn - player.getHp());
        int baseRisk = 2 + std::min(12, hpLost / std::max(1, player.getMaxHp() / 12));
        if (player.isDead())
        {
            baseRisk += 18;
        }
        if (baseRisk <= 2 && random.between(1, 100) > 5)
        {
            return;
        }

        std::vector<std::string> lines;
        for (auto it = allies.begin(); it != allies.end();)
        {
            RecruitedAllyCombatSupport& ally = *it;
            if (ally.turnsTaken <= 0)
            {
                ++it;
                continue;
            }

            int risk = baseRisk + std::max(0, ally.turnsTaken - 1) * 2;
            if (recruitedAllyJobContains(ally, "gardien") || recruitedAllyJobContains(ally, "tank"))
            {
                risk += 3;
            }
            if (recruitedAllyJobContains(ally, "soigneur") || recruitedAllyJobContains(ally, "mage d'appui"))
            {
                risk += 2;
            }
            risk = std::clamp(risk, 1, 35);
            if (random.between(1, 100) > risk)
            {
                ++it;
                continue;
            }

            const int enemyPressure = wave.hasActiveEnemies()
                ? std::max(1, wave.getActiveEnemy(0).getLevel())
                : std::max(1, player.getLevel());
            int damage = std::max(3, enemyPressure / 2 + random.between(3, 10) + hpLost / std::max(4, player.getMaxHp() / 8));
            if (orders.groupGuardOneTurn || orders.uniqueOrders[ally.name].guardOneTurn || ally.protectedThisRound)
            {
                damage = std::max(1, damage * 60 / 100);
            }
            damage = std::max(1, damage - std::max(0, ally.armorQuality / 2));
            if (recruitedAllyJobContains(ally, "gardien") || recruitedAllyJobContains(ally, "tank"))
            {
                damage = std::max(1, damage * 85 / 100);
            }
            ally.currentHp -= damage;
            lines.push_back("- " + ally.name + " encaisse " + std::to_string(damage) + " dégâts collatéraux (" + recruitedAllyHealthLine(ally) + ").");
            if (ally.currentHp > 0)
            {
                ++it;
                continue;
            }

            const bool severe = player.isDead() || hpLost > player.getMaxHp() / 3 || damage > ally.maxHp / 3 || random.between(1, 100) <= 25;
            const int recoveryDays = severe ? random.between(2, 4) : 1;
            const std::string severity = severe ? "grave" : "surveillance";
            markRecruitedAllyAwaitingInfirmaryTransfer(player, ally, recoveryDays, severity, difficulty, lines);
            orders.uniqueOrders.erase(ally.name);
            it = allies.erase(it);
        }

        if (!lines.empty())
        {
            lines.push_back("Une recrue tombée reste hors de la ligne tant que l'infirmerie ne l'a pas remise sur pied.");
            MessageScreen::show("INFIRMERIE DES RECRUES", "combat.pve.recruited_allies.infirmary", lines, false);
        }
    }


    int estimatePlayerInfirmaryCostCopper(const Player& player, DifficultyMode difficulty, bool rescuedByTeam)
    {
        int base = 28 + player.getLevel() * 7;
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                base = base * 70 / 100;
                break;
            case DifficultyMode::Normal:
                break;
            case DifficultyMode::Hard:
                base = base * 125 / 100;
                break;
            case DifficultyMode::Nightmare:
                base = base * 145 / 100;
                break;
            case DifficultyMode::Lethal:
                base = base * 160 / 100;
                break;
        }
        if (rescuedByTeam)
        {
            base = base * 75 / 100;
        }
        return std::max(15, base);
    }

    long long recruitedAllyWeeklyGoldPartsTotal(const Player& player)
    {
        const std::string currentWeekSuffix = ":semaine_" + std::to_string(std::max(0, player.getWorldDaysElapsed() / 7));
        long long total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "parts_or_recrues")
            {
                continue;
            }
            if (record.key.find(currentWeekSuffix) != std::string::npos)
            {
                total += std::max(0, record.count);
            }
        }
        return total;
    }

    bool tryLethalTeamRescueAndDisplay(
        Player& player,
        const std::vector<RecruitedAllyCombatSupport>& activeAllies,
        const std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        Random& random,
        const std::string& contextId)
    {
        const LethalTeamRescueResult rescue = LethalTeamRescueSystem::tryRecruitedTeamRescue(
            player, activeAllies, reserveAllies, random, contextId);
        if (!rescue.teamPresent) return false;

        std::vector<std::string> lines;
        lines.push_back("La chute est normalement définitive. Le combat/extraction est résolu : seuls les alliés encore debout peuvent maintenant intervenir.");
        if (rescue.hasCapableRescuer)
        {
            lines.push_back(rescue.rescuerName + " possède réellement les compétences nécessaires pour tenter un soin/réveil d'urgence.");
        }
        else
        {
            lines.push_back(rescue.rescuerName + " n'est pas un spécialiste du soin : il ne peut qu'essayer de te stabiliser, te transporter ou trouver du secours.");
        }
        lines.push_back("Chance de survie de cette tentative : " + std::to_string(rescue.survivalChancePercent) + "%.");
        lines.push_back("Jet : " + std::to_string(rescue.roll) + "/100.");
        if (rescue.survived)
        {
            lines.push_back("La mort définitive est évitée de justesse. Tu reviens avec "
                + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + " PV.");
            lines.push_back("Cette survie vient de l'équipe présente, pas d'une résurrection gratuite du mode de difficulté.");
        }
        else
        {
            lines.push_back("La tentative échoue. Les règles de mort définitive reprennent immédiatement leur cours.");
        }
        MessageScreen::show("SAUVETAGE LÉTAL D'ÉQUIPE", "combat.pve.lethal.team_rescue." + contextId, lines, false);
        return rescue.survived;
    }

    bool tryFirstLethalSpiritResistanceAndDisplay(
        Player& player,
        Random& random,
        const std::string& contextId)
    {
        const LethalSpiritResistanceResult spirit = LethalSpiritResistanceSystem::tryFirstLethalResistance(
            player, random, contextId.c_str());
        if (!spirit.consumed) return false;
        DeathPenaltySystem::displayLethalSpiritResistance(spirit);
        return spirit.survived;
    }

    struct RecruitedAllyPostKoOutcome
    {
        bool teamFinishedFight = false;
        bool teamRecoveredPlayer = false;
        int simulatedRounds = 0;
        int defeatedWhileUnconscious = 0;
        int enemiesRemaining = 0;
        std::vector<RecruitedAllyCombatSupport> participants;
    };

    void mergePostKoParticipant(
        std::vector<RecruitedAllyCombatSupport>& participants,
        const RecruitedAllyCombatSupport& ally)
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

        // Les objets déplacés entre ligne et réserve conservent déjà leurs compteurs cumulés.
        // On garde donc l'état le plus avancé au lieu d'additionner deux fois les mêmes tours.
        if (ally.turnsTaken >= existing->turnsTaken)
        {
            *existing = ally;
        }
    }

    int settlePostKoGroupEconomy(
        Player& player,
        const CombatReward& reward,
        std::vector<RecruitedAllyCombatSupport>& survivingAllies,
        const std::string& contextId
    )
    {
        if (usesEmployerTeam(survivingAllies))
        {
            for (RecruitedAllyCombatSupport& ally : survivingAllies)
            {
                if (ally.turnsTaken <= 0) continue;
                const int activityScore = ally.damageDealt + ally.healingDone + ally.supportActions * 8 + ally.finishBlows * 15 + ally.turnsTaken * 3;
                EmployerTeamRosterSystem::recordMemberCombatProgress(
                    player, ally.persistentMemberId, std::max(1, activityScore / 12));
                persistRecruitedAllyVitals(player, ally, "post_ko_employeur");
            }

            const int owed = std::max(0, reward.getEconomyUnits());
            if (owed > 0)
            {
                player.recordCanonicalEvent(
                    "parts_joueur_a_remettre_equipe_employeuse",
                    contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
                    "Part économique du joueur conservée par l'escouade employeuse pendant son inconscience.",
                    owed
                );
            }
            return owed;
        }

        std::vector<RecruitedAllyCombatSupport> contributors;
        for (const RecruitedAllyCombatSupport& ally : survivingAllies)
        {
            if (ally.turnsTaken > 0) contributors.push_back(ally);
        }
        const int playerShare = RecruitedAllyCombatRoster::normalizeShares(contributors);
        for (RecruitedAllyCombatSupport& ally : survivingAllies)
        {
            if (ally.turnsTaken <= 0)
            {
                ally.normalizedShare = 0;
                continue;
            }
            const auto contributor = std::find_if(contributors.begin(), contributors.end(), [&](const auto& current) {
                return (!ally.persistentMemberId.empty() && ally.persistentMemberId == current.persistentMemberId)
                    || ally.name == current.name;
            });
            ally.normalizedShare = contributor == contributors.end() ? 0 : contributor->normalizedShare;
        }

        const int weekIndex = std::max(0, player.getWorldDaysElapsed() / 7);
        for (RecruitedAllyCombatSupport& ally : survivingAllies)
        {
            if (ally.turnsTaken <= 0) continue;
            const int economyUnits = std::max(0, reward.getEconomyUnits() * ally.normalizedShare / 100);
            const int activityScore = ally.damageDealt + ally.healingDone + ally.supportActions * 8 + ally.finishBlows * 15 + ally.turnsTaken * 3;
            const int xp = std::max(0, reward.getExperience() * std::max(5, activityScore) / std::max(100, activityScore + player.getLevel() * 8));
            if (economyUnits > 0)
            {
                player.recordCanonicalEvent("recrues_equipees_semaines", ally.name + ":semaine_" + std::to_string(weekIndex), ally.name + " a été équipé cette semaine", 1);
                player.recordCanonicalEvent("parts_or_recrues", ally.name + ":semaine_" + std::to_string(weekIndex), ally.name + " reçoit sa part après la chute du chef", economyUnits);
            }
            if (xp > 0)
            {
                player.recordCanonicalEvent("xp_recrues", ally.name, ally.name + " gagne de l'expérience pendant la simulation post-KO", xp);
            }
            player.recordCanonicalEvent("participation_recrues", ally.name, ally.name + " continue le combat après la chute du chef", std::max(1, ally.turnsTaken));
            persistRecruitedAllyVitals(player, ally, "post_ko_recompense");
        }

        const int owed = std::max(0, reward.getEconomyUnits() * playerShare / 100);
        if (owed > 0)
        {
            player.recordCanonicalEvent(
                "parts_joueur_a_remettre_recrues",
                contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
                "Part économique due au joueur après sa chute ; elle doit être remise au réveil.",
                owed
            );
        }
        return owed;
    }

    void resolvePlayerInfirmaryTransferAfterNonLethalDeath(
        Player& player,
        DifficultyMode difficulty,
        Random& random,
        bool rescuedByTeam,
        const RecruitedAllyPostKoOutcome& outcome,
        int owedEconomyUnits,
        const std::vector<RecruitedAllyCombatSupport>& remainingAllies,
        const std::string& contextId
    )
    {
        const int costCopper = estimatePlayerInfirmaryCostCopper(player, difficulty, rescuedByTeam);
        int recoveryDays = outcome.teamFinishedFight
            ? random.between(1, 2)
            : random.between(2, 4);
        if (!rescuedByTeam)
        {
            recoveryDays += random.between(0, 1);
        }
        if (difficulty == DifficultyMode::Hard || difficulty == DifficultyMode::Nightmare)
        {
            recoveryDays += 1;
        }
        recoveryDays = std::clamp(recoveryDays, 1, 6);
        const int startDay = player.getWorldDaysElapsed();
        const int readyDay = startDay + std::max(1, recoveryDays);
        std::vector<std::string> lines;

        if (rescuedByTeam)
        {
            const bool employerTeam = usesEmployerTeam(remainingAllies);
            const long long teamCopper = employerTeam ? 0 : recruitedAllyWeeklyGoldPartsTotal(player);
            const bool coveredByTeam = employerTeam || teamCopper >= costCopper || !remainingAllies.empty();
            player.recordCanonicalEvent(
                "infirmerie_joueur_transferts",
                contextId + ":equipe",
                std::string(employerTeam ? "L'escouade employeuse ramène son membre à l'infirmerie" : "Le groupe ramène le chef à l'infirmerie")
                    + " | pret_jour=" + std::to_string(readyDay),
                1
            );
            player.recordCanonicalEvent(
                coveredByTeam ? "frais_infirmerie_couverts_par_groupe" : "dettes_infirmerie_groupe",
                contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
                coveredByTeam
                    ? "Le groupe couvre le transport et les soins du chef."
                    : "Le groupe n'a pas assez de réserve : dette d'infirmerie notée.",
                costCopper
            );

            lines.push_back((employerTeam ? "L'escouade pour laquelle tu travailles" : "Ton équipe")
                + std::string(" te récupère après ta chute et te ramène à l'infirmerie de ") + player.getCurrentCityId() + ".");
            lines.push_back(outcome.teamFinishedFight
                ? (employerTeam ? "Tes collègues ont réussi à finir le combat avant le transport." : "Les recrues ont réussi à finir le combat avant le transport.")
                : (employerTeam ? "Tes collègues ont surtout sécurisé ton extraction avant de décrocher." : "Les recrues ont surtout sécurisé ton extraction avant de décrocher."));
            const RecruitedAllyAftermathDecision aftermath = RecruitedAllyAftermathSystem::decide(
                player, remainingAllies, random, contextId);
            const int deliveredShare = RecruitedAllyAftermathSystem::deliverPlayerEconomyShare(
                player, owedEconomyUnits, aftermath, contextId);

            if (!aftermath.memberName.empty())
            {
                if (aftermath.givesDetailedReport)
                {
                    const RecruitedAllyAftermathKnowledgeReport knowledge = RecruitedAllyAftermathSystem::buildKnowledgeReport(
                        aftermath,
                        remainingAllies,
                        outcome.simulatedRounds,
                        outcome.teamFinishedFight,
                        outcome.defeatedWhileUnconscious,
                        outcome.enemiesRemaining
                    );
                    lines.push_back(aftermath.memberName + " reste près de toi au réveil et te fait un compte rendu.");
                    lines.insert(lines.end(), knowledge.lines.begin(), knowledge.lines.end());
                    player.recordCanonicalEvent(
                        employerTeam ? "team_employer_after_action_reports" : "rapports_combat_recrues_racontes",
                        contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
                        aftermath.memberName + " raconte la phase post-KO"
                            + " | direct=" + std::to_string(knowledge.directFirstRound) + "-" + std::to_string(knowledge.directLastRound)
                            + " | source_seconde_main=" + (knowledge.secondHandSourceName.empty() ? std::string("aucune") : knowledge.secondHandSourceName)
                            + " | lacune=" + std::string(knowledge.hasUnknownGap ? "oui" : "non"),
                        1
                    );
                }
                else
                {
                    lines.push_back(aftermath.memberName + " ne te fait pas de compte rendu détaillé. " + aftermath.refusalReason);
                }

                if (owedEconomyUnits > 0)
                {
                    if (aftermath.stealsPlayerShare)
                    {
                        lines.push_back("Part économique : mauvaise nouvelle, " + aftermath.memberName + " s'est cassé avec la part qui devait te revenir : "
                            + Money::formatEconomyUnits(owedEconomyUnits) + ".");
                    }
                    else
                    {
                        lines.push_back("Part économique : " + aftermath.memberName + " te remet ce qui te revient de droit : "
                            + Money::formatEconomyUnits(deliveredShare) + ".");
                    }
                }
            }
            else if (owedEconomyUnits > 0)
            {
                // Aucun survivant de l'équipe n'est en état de faire le débrief : aucune remise
                // artificielle n'est inventée ici. La trace reste due et pourra être traitée plus tard.
                lines.push_back("Aucun coéquipier n'est en état de te remettre immédiatement la part notée comme due.");
            }
            lines.push_back(coveredByTeam
                ? (employerTeam
                    ? "Coût joueur : 0. L'équipe employeuse prend en charge le transport et les soins de terrain pour ce retour."
                    : "Coût joueur : 0. Le groupe couvre les frais avec ses parts/réserves.")
                : "Coût joueur immédiat : 0, mais une dette de groupe est notée : " + Money::formatCopper(costCopper) + ".");
        }
        else
        {
            const bool paid = player.getInventory().spendCopper(costCopper);
            player.recordCanonicalEvent(
                "infirmerie_joueur_transferts",
                contextId + ":secours_lambda",
                "Un groupe lambda ramène le joueur à l'infirmerie | pret_jour=" + std::to_string(readyDay),
                1
            );
            player.recordCanonicalEvent(
                paid ? "frais_infirmerie_payes" : "dettes_infirmerie_joueur",
                contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
                paid ? "Facture d'infirmerie payée après sauvetage lambda." : "Facture d'infirmerie impayée après sauvetage lambda.",
                costCopper
            );

            lines.push_back("Tu étais seul : un groupe lambda te retrouve et te transporte à l'infirmerie de " + player.getCurrentCityId() + ".");
            lines.push_back(paid
                ? "Facture payée : " + Money::formatCopper(costCopper) + "."
                : "Tu n'avais pas assez d'argent : dette d'infirmerie notée pour " + Money::formatCopper(costCopper) + ".");
        }

        transferPendingFallenRecruitsToInfirmary(player, difficulty, random, contextId + ":joueur_ko", lines);
        persistRecruitedAllyVitals(player, remainingAllies, "chef_infirmerie");

        lines.push_back("Repos estimé : " + std::to_string(recoveryDays) + " jour(s). Retour opérationnel prévu jour " + std::to_string(readyDay) + ".");
        lines.push_back("Le monde avance réellement de " + std::to_string(recoveryDays) + " jour(s) pendant les soins.");
        lines.push_back(usesEmployerTeam(remainingAllies)
            ? "Sous contrat, l'escouade employeuse prend en charge le retour de son membre ; cela ne transforme pas le joueur en chef de cette équipe."
            : "En solo, le secours d'un inconnu coûte cher ; en équipe, le groupe amortit le retour du chef, sauf manque de réserve.");
        MessageScreen::show("INFIRMERIE DU JOUEUR", "combat.pve.player.infirmary." + contextId, lines, false);
        player.advanceWorldDays(recoveryDays);
    }

    RecruitedAllyPostKoOutcome simulateRecruitedAlliesAfterPlayerDown(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        DifficultyMode difficulty,
        std::vector<RecruitedAllyCombatSupport>& allies,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        RecruitedAllyOrderState& orders,
        const std::string& contextId
    )
    {
        RecruitedAllyPostKoOutcome outcome;
        if (!wave.hasEnemiesLeft())
        {
            outcome.teamFinishedFight = true;
            outcome.teamRecoveredPlayer = !allies.empty() || !reserveAllies.empty();
            return outcome;
        }

        while (allies.size() < static_cast<std::size_t>(RecruitedAllyCombatRoster::MaxActiveRecruitsWithoutPlayer)
            && !reserveAllies.empty())
        {
            allies.push_back(reserveAllies.front());
            reserveAllies.erase(reserveAllies.begin());
        }
        if (allies.empty())
        {
            outcome.enemiesRemaining = wave.getTotalRemainingEnemyCount();
            return outcome;
        }

        MessageScreen::show(
            "CHEF À TERRE",
            "combat.pve.player.down.team_continues." + contextId,
            {
                "Ta vision décroche avant la suite du combat.",
                "Ton équipe peut continuer à agir et faire entrer des membres de réserve, mais tu n'en observes plus le déroulé tour par tour.",
                "Si quelqu'un te ramène vivant, un survivant pourra éventuellement te résumer ce qui s'est passé à ton réveil."
            },
            false
        );

        const int defeatedBeforeSimulation = wave.getDefeatedEnemyCount();

        int rounds = 0;
        const int maxRounds = std::clamp(3 + static_cast<int>(allies.size()) + static_cast<int>(reserveAllies.size()) / 2, 3, 7);
        while (wave.hasEnemiesLeft() && !allies.empty() && rounds < maxRounds)
        {
            ++rounds;
            const int defeatedAtRoundStart = wave.getDefeatedEnemyCount();
            for (RecruitedAllyCombatSupport& ally : allies)
            {
                RecruitedAllyCombatRoster::recordPostKoObservation(ally, rounds, false);
            }
            // In standard PvE the waiting line is physically near the fight: a reserve member can
            // witness the scene without taking a turn, like someone watching from the edge/stands.
            for (RecruitedAllyCombatSupport& ally : reserveAllies)
            {
                RecruitedAllyCombatRoster::recordPostKoObservation(ally, rounds, true);
            }
            std::vector<std::string> lines;
            lines.push_back("Round automatique des recrues " + std::to_string(rounds) + "/" + std::to_string(maxRounds) + ".");

            for (auto it = allies.begin(); it != allies.end() && wave.hasEnemiesLeft(); )
            {
                RecruitedAllyCombatSupport& ally = *it;
                ++ally.turnsTaken;
                if (ally.activeSkillCooldown > 0)
                {
                    --ally.activeSkillCooldown;
                }
                RecruitedAllyUniqueOrder& unique = orders.uniqueOrders[ally.name];
                if (executeRequestedAllySwitch(player, ally, reserveAllies, unique, lines))
                {
                    ++it;
                    continue;
                }

                RecruitedAllyCombatSupport* woundedAlly = nullptr;
                for (auto& candidate : allies)
                {
                    if (candidate.name == ally.name || candidate.currentHp <= 0) continue;
                    if (woundedAlly == nullptr
                        || static_cast<long long>(candidate.currentHp) * std::max(1, woundedAlly->maxHp)
                            < static_cast<long long>(woundedAlly->currentHp) * std::max(1, candidate.maxHp))
                    {
                        woundedAlly = &candidate;
                    }
                }

                const bool orderedHeal = unique.healOneTurn || orders.groupHealOneTurn;
                const bool orderedGuard = unique.guardOneTurn || orders.groupGuardOneTurn;
                const bool orderedTechnique = unique.forceTechniqueOneTurn || orders.groupTechniqueOneTurn;
                RecruitedAllyActionContext actionContext;
                actionContext.hasEnemy = wave.hasActiveEnemies();
                actionContext.enemyCount = wave.getActiveEnemyCount();
                actionContext.bossContext = false;
                actionContext.leaderPresent = false;
                actionContext.leaderHpPercent = 0;
                actionContext.leaderCanBeHealed = false;
                actionContext.woundedAllyAvailable = woundedAlly != nullptr;
                actionContext.woundedAllyHpPercent = woundedAlly != nullptr && woundedAlly->maxHp > 0
                    ? std::clamp(woundedAlly->currentHp * 100 / woundedAlly->maxHp, 0, 100)
                    : 100;
                actionContext.orderedHeal = orderedHeal;
                actionContext.orderedGuard = orderedGuard;
                actionContext.orderedTechnique = orderedTechnique;
                actionContext.hasDirectOrder = unique.hasPriorityTarget || orders.groupPriorityTarget || orderedTechnique;
                actionContext.techniqueAvailable = ally.activeSkillCooldown <= 0;
                actionContext.techniqueAllowed = true;
                actionContext.relationshipScore = RecruitedAllyCombatRoster::relationshipScore(player, ally.name);

                const RecruitedAllyActionDecision action = RecruitedAllyActionPlanner::decide(ally, actionContext, random);

                if (action.type == RecruitedAllyActionType::SelfHeal)
                {
                    const int selfHeal = std::max(5, ally.maxHp / 3);
                    const int before = ally.currentHp;
                    ally.currentHp = std::min(ally.maxHp, ally.currentHp + selfHeal);
                    --ally.healingPotionCharges;
                    const int effective = std::max(0, ally.currentHp - before);
                    ally.healingDone += effective;
                    ++ally.supportActions;
                    lines.push_back("- " + ally.name + " se stabilise avant de continuer : +" + std::to_string(effective) + " PV.");
                    ++it;
                    continue;
                }

                if (action.type == RecruitedAllyActionType::HealAlly && woundedAlly != nullptr)
                {
                    const int heal = std::max(3, woundedAlly->maxHp / 10 + ally.currentRankIndex * 2 + ally.supportKitQuality + random.between(0, 3));
                    const int before = woundedAlly->currentHp;
                    woundedAlly->currentHp = std::min(woundedAlly->maxHp, woundedAlly->currentHp + heal);
                    const int effective = std::max(0, woundedAlly->currentHp - before);
                    ally.healingDone += effective;
                    ++ally.supportActions;
                    lines.push_back("- " + ally.name + " maintient " + woundedAlly->name + " dans le combat : +" + std::to_string(effective) + " PV.");
                    ++it;
                    continue;
                }

                if (action.type == RecruitedAllyActionType::Technique
                    && executeRecruitedAllyActiveTechnique(ally, wave, random, orders, lines, true))
                {
                    ++it;
                    continue;
                }

                if (action.type == RecruitedAllyActionType::Hold)
                {
                    ++it;
                    continue;
                }

                const bool plannedGuard = action.type == RecruitedAllyActionType::Guard;
                ally.protectedThisRound = plannedGuard;
                const int targetIndex = chooseRecruitedAllyTargetIndexWithOrders(ally, wave, random, orders);
                Monster& target = wave.getActiveEnemy(targetIndex);
                int damage = std::max(4, ally.estimatedLevel / 2 + ally.currentRankIndex * 6 + random.between(3, 13));
                if (plannedGuard)
                {
                    damage = std::max(2, damage * 75 / 100);
                    ++ally.supportActions;
                }
                if (recruitedAllyJobContains(ally, "roublard") || recruitedAllyJobContains(ally, "assassin"))
                {
                    damage += random.between(2, 7);
                }
                if (recruitedAllyJobContains(ally, "soigneur") || recruitedAllyJobContains(ally, "mage d'appui"))
                {
                    damage = std::max(2, damage - 3);
                    ally.healingDone += std::max(2, ally.currentRankIndex + 2);
                }
                const std::string targetName = target.getName();
                target.takeDamage(damage);
                wave.removeDeadAndReplace();
                ally.damageDealt += damage;
                lines.push_back("- " + ally.name + " garde la ligne et frappe " + targetName + " : " + std::to_string(damage) + " dégâts.");
                ++it;
            }

            const int defeatedThisRound = std::max(0, wave.getDefeatedEnemyCount() - defeatedAtRoundStart);
            for (RecruitedAllyCombatSupport& ally : allies)
            {
                RecruitedAllyCombatRoster::recordPostKoObservedDefeats(ally, defeatedThisRound);
            }
            for (RecruitedAllyCombatSupport& ally : reserveAllies)
            {
                RecruitedAllyCombatRoster::recordPostKoObservedDefeats(ally, defeatedThisRound);
            }

            if (wave.hasEnemiesLeft() && !allies.empty())
            {
                int injuryRisk = 14 + wave.getActiveEnemyCount() * 6 + std::max(0, rounds - 1) * 4;
                if (random.between(1, 100) <= injuryRisk)
                {
                    const int victimIndex = random.between(0, static_cast<int>(allies.size()) - 1);
                    RecruitedAllyCombatSupport victim = allies[static_cast<std::size_t>(victimIndex)];
                    victim.postKoAvailableForDebrief = false;
                    const bool severe = random.between(1, 100) <= 35 || rounds >= maxRounds - 1;
                    markRecruitedAllyAwaitingInfirmaryTransfer(player, victim, severe ? random.between(2, 5) : 1, severe ? "grave" : "surveillance", difficulty, lines);
                    mergePostKoParticipant(outcome.participants, victim);
                    orders.uniqueOrders.erase(victim.name);
                    allies.erase(allies.begin() + victimIndex);
                }
            }

            while (allies.size() < static_cast<std::size_t>(RecruitedAllyCombatRoster::MaxActiveRecruitsWithoutPlayer)
                && !reserveAllies.empty())
            {
                RecruitedAllyCombatSupport incoming = reserveAllies.front();
                reserveAllies.erase(reserveAllies.begin());
                allies.push_back(incoming);
                lines.push_back("- " + incoming.name + " quitte la réserve et occupe une place libérée après la chute du chef.");
            }

            // Le chef est inconscient : le moteur simule réellement ce round, mais n'affiche pas
            // les actions détaillées au joueur. `lines` reste utile aux sous-systèmes internes.
        }

        outcome.teamFinishedFight = !wave.hasEnemiesLeft();
        outcome.teamRecoveredPlayer = !allies.empty() || std::any_of(reserveAllies.begin(), reserveAllies.end(), [](const auto& ally) { return ally.currentHp > 0; });
        outcome.simulatedRounds = rounds;
        const int defeatedAfterSimulation = wave.getDefeatedEnemyCount();
        outcome.defeatedWhileUnconscious = std::max(0, defeatedAfterSimulation - defeatedBeforeSimulation);
        outcome.enemiesRemaining = wave.getTotalRemainingEnemyCount();

        // Active fighters and nearby reserve witnesses can both know the ending if they were still
        // present and conscious. Observation never implies that the reserve member participated.
        for (RecruitedAllyCombatSupport& ally : allies)
        {
            RecruitedAllyCombatRoster::recordPostKoFinalOutcome(ally);
        }
        for (RecruitedAllyCombatSupport& ally : reserveAllies)
        {
            RecruitedAllyCombatRoster::recordPostKoFinalOutcome(ally);
        }
        for (const auto& ally : allies) mergePostKoParticipant(outcome.participants, ally);
        for (const auto& ally : reserveAllies) mergePostKoParticipant(outcome.participants, ally);
        const std::string reportKey = contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed());
        const bool employerTeam = usesEmployerTeam(outcome.participants) || usesEmployerTeam(allies);
        player.recordCanonicalEvent(
            employerTeam
                ? (outcome.teamFinishedFight ? "team_employer_post_ko_victories" : "team_employer_post_ko_extractions")
                : (outcome.teamFinishedFight ? "combats_finies_par_recrues" : "extractions_par_recrues"),
            reportKey,
            employerTeam
                ? (outcome.teamFinishedFight ? "L'escouade employeuse finit le combat après la chute du joueur." : "L'escouade employeuse extrait le joueur sans finir le combat.")
                : (outcome.teamFinishedFight ? "Les recrues finissent le combat après la chute du chef." : "Les recrues extraient le chef sans finir proprement le combat."),
            1
        );
        player.recordCanonicalEvent(
            employerTeam ? "team_employer_after_action_reports" : "rapports_combat_recrues",
            reportKey,
            "rounds=" + std::to_string(rounds)
                + " | resultat=" + std::string(outcome.teamFinishedFight ? "victoire_equipe" : "extraction")
                + " | vaincus_apres_chute=" + std::to_string(outcome.defeatedWhileUnconscious)
                + " | ennemis_restants=" + std::to_string(outcome.enemiesRemaining),
            1
        );
        if (outcome.teamFinishedFight)
        {
            player.recordCanonicalEvent(
                "team_victories_without_personal_clear",
                reportKey,
                "Victoire obtenue par l'équipe pendant l'inconscience du joueur ; aucun clear personnel gratuit.",
                1
            );
        }
        return outcome;
    }

    enum class EmployerReserveObservationOutcome
    {
        PlayerCalledIn,
        TeamWonBeforePlayerEntry
    };

    void persistEmployerReserveParticipants(
        Player& player,
        std::vector<RecruitedAllyCombatSupport>& allies,
        const std::string& status)
    {
        for (RecruitedAllyCombatSupport& ally : allies)
        {
            if (ally.turnsTaken > 0)
            {
                const int activityScore = ally.damageDealt + ally.healingDone
                    + ally.supportActions * 8 + ally.finishBlows * 15 + ally.turnsTaken * 3;
                EmployerTeamRosterSystem::recordMemberCombatProgress(
                    player,
                    ally.persistentMemberId,
                    std::max(1, activityScore / 12)
                );
            }
            persistRecruitedAllyVitals(player, ally, status);
        }
    }

    EmployerReserveObservationOutcome resolveEmployerReserveObservation(
        Player& player,
        EnemyCombatQueue& wave,
        Random& random,
        DifficultyMode difficulty,
        std::vector<RecruitedAllyCombatSupport>& allies,
        std::vector<RecruitedAllyCombatSupport>& reserveAllies,
        RecruitedAllyOrderState& orders,
        const EmployerTacticalOrder& employerOrder,
        const std::string& contextId)
    {
        MessageScreen::show(
            "LISTE D'ATTENTE DU CHEF",
            "combat.pve.employer_team.player_reserve." + contextId,
            {
                "Le chef ne t'accorde pas une place principale pour l'ouverture de ce combat.",
                "Tu restes avec la réserve : tu vois le combat normalement, mais tu n'agis pas tant qu'il ne te fait pas entrer.",
                "Si une place se libère ou si la situation s'enlise, le chef peut te rappeler sur la ligne."
            },
            false
        );

        const int maxObservedRounds = 3;
        for (int round = 1; round <= maxObservedRounds && wave.hasEnemiesLeft() && !allies.empty(); ++round)
        {
            MessageScreen::show(
                "COMBAT OBSERVÉ — RÉSERVE",
                "combat.pve.employer_team.player_reserve.round." + contextId,
                {
                    "Round observé : " + std::to_string(round) + "/" + std::to_string(maxObservedRounds) + ".",
                    "Tu es encore hors des trois places principales. Le chef ouvre toujours l'action de son équipe."
                },
                false
            );

            applyEmployerTacticalOrderToAllies(employerOrder, wave, orders);
            const EmployerLeaderRotationOutcome rotation = maybeEmployerLeaderUsesRotation(
                player, allies, reserveAllies, random, round, true);
            if (rotation == EmployerLeaderRotationOutcome::PlayerCalledIn)
            {
                EmployerTeamDisciplineSystem::consumeReserveBattle(player);
                player.recordCanonicalEvent(
                    "team_employer_reserve_entries",
                    contextId + ":rotation_round_" + std::to_string(round),
                    "Le joueur entre depuis la liste d'attente après une rotation volontaire du chef.",
                    1
                );
                return EmployerReserveObservationOutcome::PlayerCalledIn;
            }
            if (rotation != EmployerLeaderRotationOutcome::TurnConsumed)
            {
                playRecruitedAllyCombatTurns(
                    player, wave, random, allies, reserveAllies, orders, AllyTurnPhase::LeaderOnly);
            }
            if (wave.hasEnemiesLeft())
            {
                playRecruitedAllyCombatTurns(
                    player, wave, random, allies, reserveAllies, orders, AllyTurnPhase::NonLeaderOnly);
            }

            if (!wave.hasEnemiesLeft())
            {
                EmployerTeamDisciplineSystem::consumeReserveBattle(player);
                TeamEmploymentMissionSystem::progressServiceCombat(player, 1);
                persistEmployerReserveParticipants(player, allies, "combat_observe_reserve");
                persistEmployerReserveParticipants(player, reserveAllies, "reserve_employeur");
                player.recordCanonicalEvent(
                    "team_employer_reserve_observations",
                    contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
                    "Le joueur est resté en liste d'attente pendant toute la victoire de l'escouade.",
                    1
                );
                MessageScreen::show(
                    "VICTOIRE DE L'ESCOUADE",
                    "combat.pve.employer_team.player_reserve.team_win." + contextId,
                    {
                        "L'équipe termine le combat avant que le chef ait besoin de te faire entrer.",
                        "Tu as accompli ton affectation de réserve, mais ce n'est pas une victoire personnelle : aucune XP ni butin de combat ne t'est attribué artificiellement.",
                        "Ton salaire contractuel reste séparé de cette récompense de terrain."
                    },
                    false
                );
                return EmployerReserveObservationOutcome::TeamWonBeforePlayerEntry;
            }

            const int injuryRisk = std::clamp(16 + wave.getActiveEnemyCount() * 7 + round * 5, 20, 58);
            if (random.between(1, 100) <= injuryRisk)
            {
                const int victimIndex = random.between(0, static_cast<int>(allies.size()) - 1);
                RecruitedAllyCombatSupport victim = allies[static_cast<std::size_t>(victimIndex)];
                std::vector<std::string> injuryLines;
                markRecruitedAllyAwaitingInfirmaryTransfer(
                    player,
                    victim,
                    random.between(1, 3),
                    "blessure_de_service",
                    difficulty,
                    injuryLines
                );
                allies.erase(allies.begin() + victimIndex);
                if (!injuryLines.empty())
                {
                    injuryLines.push_back("Une place principale vient de se libérer : le chef te rappelle sur la ligne.");
                    MessageScreen::show(
                        "PLACE LIBÉRÉE",
                        "combat.pve.employer_team.player_reserve.slot_open." + contextId,
                        injuryLines,
                        false
                    );
                }
                EmployerTeamDisciplineSystem::consumeReserveBattle(player);
                player.recordCanonicalEvent(
                    "team_employer_reserve_entries",
                    contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
                    "Le joueur entre depuis la liste d'attente après la chute d'un collègue.",
                    1
                );
                return EmployerReserveObservationOutcome::PlayerCalledIn;
            }
        }

        // Anti-frustration : si personne n'est tombé mais que le combat dure, le chef finit par faire entrer le joueur.
        if (allies.size() > static_cast<std::size_t>(RecruitedAllyCombatRoster::MaxActiveRecruitsWithPlayer))
        {
            auto moveIt = std::find_if(allies.rbegin(), allies.rend(), [](const RecruitedAllyCombatSupport& ally) {
                return !ally.teamLeader;
            });
            if (moveIt != allies.rend())
            {
                reserveAllies.insert(reserveAllies.begin(), *moveIt);
                allies.erase(std::next(moveIt).base());
            }
        }
        EmployerTeamDisciplineSystem::consumeReserveBattle(player);
        player.recordCanonicalEvent(
            "team_employer_reserve_entries",
            contextId + ":jour_" + std::to_string(player.getWorldDaysElapsed()),
            "Après plusieurs rounds observés, le chef rappelle le joueur depuis la liste d'attente.",
            1
        );
        MessageScreen::show(
            "RAPPEL SUR LA LIGNE",
            "combat.pve.employer_team.player_reserve.called_in." + contextId,
            {
                "Le combat s'éternise.",
                TeamLeadershipSystem::leaderDisplayName(player) + " te fait signe d'entrer : une place principale t'est rendue.",
                "La sanction de réserve est considérée comme purgée pour ce combat."
            },
            false
        );
        return EmployerReserveObservationOutcome::PlayerCalledIn;
    }

    CombatReward splitCombatRewardWithRecruitedAllies(Player& player, const CombatReward& reward, std::vector<RecruitedAllyCombatSupport>& allies, const std::string& contextId)
    {
        if (allies.empty())
        {
            return reward;
        }
        if (usesEmployerTeam(allies))
        {
            std::vector<std::string> lines;
            lines.push_back("Les collègues de l'équipe employeuse sont payés par leur organisation : aucune part n'est prélevée sur ta récompense personnelle.");
            for (RecruitedAllyCombatSupport& ally : allies)
            {
                if (ally.turnsTaken <= 0) continue;
                const int activityScore = ally.damageDealt + ally.healingDone + ally.supportActions * 8 + ally.finishBlows * 15 + ally.turnsTaken * 3;
                EmployerTeamRosterSystem::recordMemberCombatProgress(
                    player,
                    ally.persistentMemberId,
                    std::max(1, activityScore / 12)
                );
                persistRecruitedAllyVitals(player, ally, "combat_employeur");
                lines.push_back("- " + ally.name + " : dégâts " + std::to_string(ally.damageDealt)
                    + ", soutien " + std::to_string(ally.healingDone)
                    + ", actions utiles " + std::to_string(ally.supportActions) + ".");
            }
            MessageScreen::show("PARTICIPATION DE L'ESCOUADE", "combat.pve.employer_team.reward." + contextId, lines, false);
            return reward;
        }
        std::vector<std::string> duoExperienceLines;
        for (std::size_t i = 0; i < allies.size(); ++i)
        {
            if (allies[i].turnsTaken <= 0) continue;
            for (std::size_t j = i + 1; j < allies.size(); ++j)
            {
                if (allies[j].turnsTaken <= 0) continue;
                if (DuoMasterySystem::recordSharedCombatDay(player, allies[i].name, allies[j].name, contextId))
                {
                    duoExperienceLines.push_back("- Expérience de duo : " + allies[i].name + " et " + allies[j].name + " enregistrent une vraie journée de combat partagée.");
                }
            }
        }

        if (reward.getEconomyUnits() <= 0)
        {
            for (const RecruitedAllyCombatSupport& ally : allies)
                RecruitedAllyConditionSystem::recordCombatStrain(player, ally, false);
            persistRecruitedAllyVitals(player, allies, "combat_sans_or");
            return reward;
        }

        const int playerShare = RecruitedAllyCombatRoster::normalizeShares(allies);
        const int weekIndex = std::max(0, player.getWorldDaysElapsed() / 7);
        std::vector<std::string> lines;
        lines.push_back("Récompense de groupe : " + Money::formatEconomyUnits(reward.getEconomyUnits()) + " et " + std::to_string(reward.getExperience()) + " XP.");
        lines.push_back("Part du chef : " + std::to_string(playerShare) + "%. Le joueur garde la plus grosse part, même avec plusieurs alliés.");
        lines.insert(lines.end(), duoExperienceLines.begin(), duoExperienceLines.end());
        for (const RecruitedAllyCombatSupport& ally : allies)
        {
            if (ally.temporaryNoReward)
            {
                lines.push_back("- " + ally.name + " : soutien municipal temporaire, aucune part de récompense prélevée.");
                continue;
            }
            const int economyUnits = std::max(0, (reward.getEconomyUnits() * ally.normalizedShare) / 100);
            const int activityScore = ally.damageDealt + ally.healingDone + ally.supportActions * 8 + ally.finishBlows * 15 + ally.turnsTaken * 3;
            const int xp = std::max(0, (reward.getExperience() * std::max(5, activityScore)) / std::max(100, activityScore + player.getLevel() * 8));
            if (economyUnits > 0)
            {
                player.recordCanonicalEvent("recrues_equipees_semaines", ally.name + ":semaine_" + std::to_string(weekIndex), ally.name + " a été équipé cette semaine", 1);
                player.recordCanonicalEvent("parts_or_recrues", ally.name + ":semaine_" + std::to_string(weekIndex), ally.name + " reçoit une part de combat réel", economyUnits);
            }
            if (xp > 0)
            {
                player.recordCanonicalEvent("xp_recrues", ally.name, ally.name + " gagne de l'expérience de combat réel", xp);
            }
            player.recordCanonicalEvent("participation_recrues", ally.name, ally.name + " participe au combat réel", std::max(1, ally.turnsTaken));
            RecruitedAllyConditionSystem::recordCombatStrain(player, ally, false);
            persistRecruitedAllyVitals(player, ally, "combat_recompense");
            lines.push_back("- " + ally.name + " : " + Money::formatEconomyUnits(economyUnits) + " (" + std::to_string(ally.normalizedShare) + "%), " + std::to_string(xp) + " XP, dégâts " + std::to_string(ally.damageDealt) + ", soutien " + std::to_string(ally.healingDone) + ", actions utiles " + std::to_string(ally.supportActions) + ", finitions " + std::to_string(ally.finishBlows) + ".");
        }
        lines.push_back("Le salaire hebdo pourra baisser seulement si la recrue était équipée ET si ces parts couvrent son salaire.");
        MessageScreen::show("PARTAGE AVEC RECRUES", "combat.pve.recruited_allies.reward." + contextId, lines, false);
        return reward.getModified(100, playerShare);
    }

    TemporaryAdventurerSupport maybeTriggerRareAdventurerCombatAid(Player& player, EnemyCombatQueue& wave, Random& random)
    {
        TemporaryAdventurerSupport support;
        if (!wave.hasActiveEnemies())
        {
            return support;
        }

        const int roll = random.between(1, 1000);
        if (roll > 18)
        {
            return support;
        }

        support.active = true;
        support.groupName = rareCombatAidGroupName(random);
        support.remainingTurns = random.between(1, 2);
        support.initiativeSlot = 2;

        const int targetIndex = random.between(0, wave.getActiveEnemyCount() - 1);
        Monster& target = wave.getActiveEnemy(targetIndex);
        const int aidDamage = std::max(4, player.getLevel() * 2 + random.between(2, 6));
        target.takeDamage(aidDamage);
        wave.removeDeadAndReplace();

        const bool healHelp = player.getHp() < player.getMaxHp();
        const int healAmount = healHelp ? std::max(3, player.getMaxHp() / 14) : 0;
        if (healAmount > 0)
        {
            player.heal(healAmount);
        }

        player.recordCanonicalEvent("aides_rares_groupes_combat", support.groupName, support.groupName + " intervient pendant un combat PvE");
        player.recordCanonicalEvent("alliés_temporaires_combat", support.groupName, support.groupName + " agit brièvement avec initiative limitée");
        MessageScreen::show(
            "AIDE RARE D'AVENTURIERS",
            "combat.pve.rare_adventurer_aid",
            {
                "Événement très rare : " + support.groupName + " passent près du combat.",
                "Ils rejoignent temporairement la file alliée : initiative limitée après le joueur et les invocations, avant les ennemis.",
                "Impact d'arrivée : " + std::to_string(aidDamage) + " dégâts infligés à une cible active." + (healAmount > 0 ? " Soin de secours : +" + std::to_string(healAmount) + " PV." : ""),
                "Durée : " + std::to_string(support.remainingTurns) + " tour(s) allié(s). C'est volontairement rare pour ne pas voler le combat au joueur."
            },
            false
        );
        return support;
    }

    void playTemporaryAdventurerSupportTurn(Player& player, EnemyCombatQueue& wave, Random& random, TemporaryAdventurerSupport& support)
    {
        if (!support.active || support.remainingTurns <= 0 || !wave.hasActiveEnemies())
        {
            return;
        }

        const int targetIndex = random.between(0, wave.getActiveEnemyCount() - 1);
        Monster& target = wave.getActiveEnemy(targetIndex);
        const int damage = std::max(3, player.getLevel() * 2 + random.between(1, 7));
        target.takeDamage(damage);
        wave.removeDeadAndReplace();

        int healAmount = 0;
        if (player.getHp() * 3 < player.getMaxHp() * 2 && random.between(1, 100) <= 35)
        {
            healAmount = std::max(2, player.getMaxHp() / 18);
            player.heal(healAmount);
        }

        --support.remainingTurns;
        player.recordCanonicalEvent("tours_allies_temporaires", support.groupName, support.groupName + " prend un tour allié temporaire");
        MessageScreen::show(
            "TOUR ALLIÉ TEMPORAIRE",
            "combat.pve.temporary_adventurer_turn",
            {
                support.groupName + " agit dans une fenêtre d'initiative alliée limitée.",
                "Dégâts infligés : " + std::to_string(damage) + "." + (healAmount > 0 ? " Soin rapide : +" + std::to_string(healAmount) + " PV." : ""),
                support.remainingTurns > 0 ? "Ils restent encore un instant dans la mêlée." : "Ils quittent le combat avant de devenir ton équipe permanente."
            },
            false
        );

        if (support.remainingTurns <= 0)
        {
            support.active = false;
        }
    }

    void maybeTriggerRareScavengerAfterCombat(Player& player, Random& random, const std::string& contextId)
    {
        const int roll = random.between(1, 1000);
        if (roll > 10)
        {
            return;
        }

        const std::vector<std::string> scavengers = {
            "deux novices trop confiants",
            "un ramasse-miette solitaire",
            "une petite bande opportuniste",
            "un faux secouriste beaucoup trop intéressé"
        };
        const std::string group = scavengers[static_cast<std::size_t>(random.between(0, static_cast<int>(scavengers.size()) - 1))];
        const bool vulnerable = player.getHp() * 4 < player.getMaxHp();
        int stolenCopper = 0;
        if (vulnerable)
        {
            stolenCopper = std::max(4, std::min(45, player.getLevel() * 5 + random.between(0, 12)));
            if (!player.getInventory().spendCopper(stolenCopper))
            {
                stolenCopper = 0;
            }
        }

        player.recordCanonicalEvent("tentatives_ramasse_miettes", contextId, group + " tente de profiter de la fin d'un combat");
        MessageScreen::show(
            "RAMASSE-MIETTES APRÈS COMBAT",
            "combat.pve.rare_scavenger",
            {
                "Événement très rare : " + group + " attend la fin du combat pour tenter de profiter de l'état du joueur.",
                vulnerable
                    ? "Tu es assez amoché pour qu'ils osent approcher."
                    : "Tu tiens encore assez debout : ils comprennent vite que ce n'est pas le bon cadavre à dépouiller.",
                stolenCopper > 0
                    ? "Perte légère : " + Money::formatCopper(stolenCopper) + " arrachés dans la confusion."
                    : "Perte : aucune. Ils reculent avant que ça devienne une vraie mauvaise idée.",
                "Note : l'événement reste volontairement rarissime pour rendre le monde vivant sans spammer le joueur."
            },
            false
        );
    }

}

void MonsterPveMode::run(
    Player& player,
    Random& random,
    DifficultyMode difficulty,
    DeathRuleMode deathRule
)
{
    player.resetEscapeOpeningProgress();
    Console::clear();

    MenuScreen encounterScreen("RENCONTRE PVE", "combat.pve.encounter_type");
    encounterScreen.addLine("Choisis le type de rencontre PvE.");
    encounterScreen.addOption(
        1,
        "Vague de monstres",
        "Une file d'ennemis classiques, avec maximum trois ennemis actifs.",
        true,
        "pve.encounter.wave"
    );
    encounterScreen.addOption(
        2,
        "Groupe d'aventuriers aléatoire",
        "Humains, semi-humains ou groupe spécial. Normalement pas un combat à mort, sauf cas dangereux.",
        true,
        "pve.encounter.adventurers"
    );

    int encounterChoice = TerminalInterface::askMenuChoiceFromOptions(
        encounterScreen,
        "Veuillez entrer 1 ou 2."
    );

    EnemyCombatQueue wave;

    if (encounterChoice == 2)
    {
        AdventurerGroupEncounter::displayGroupEncounterIntroduction();
        wave = AdventurerGroupEncounter::createRandomGroupForPlayer(player, random);
    }
    else
    {
        WaveCombatSystem::displayWaveIntroduction();
        wave = WaveCombatSystem::createWaveForPlayer(player, random, difficulty);
        maybeAppendReturningRival(player, wave, random);
    }

    WaveCombatSystem::displayFrontLineArrival(wave);
    PveWaveNarrativeSupport::recordWaveEncountersInBestiary(wave);
    PveWaveNarrativeSupport::recordWaveEncountersInJournal(player, wave);
    EncounterDialogueSystem::display(player, wave, random, "pve.encounter");
    TemporaryAdventurerSupport temporaryAdventurerSupport = maybeTriggerRareAdventurerCombatAid(player, wave, random);

    CombatGroup enemyFrontPreview = CombatGroupBuilder::buildSideFromWave(
        wave,
        CombatSide::EnemySide
    );

    CombatGroupBuilder::displayGroup(
        enemyFrontPreview,
        "LIGNE ENNEMIE ACTIVE"
    );

    std::vector<Summon> playerSummons = SummonCombatSystem::createInitialSummonsFor(player);
    SummonCombatSystem::displaySummonArrival(player, playerSummons);

    CombatGroup playerGroupPreview = CombatGroupBuilder::buildSideFromEntityAndSummons(
        player,
        playerSummons,
        CombatSide::PlayerSide,
        CombatUnitKind::MainFighter
    );

    CombatGroupBuilder::displayGroup(
        playerGroupPreview,
        "GROUPE DU JOUEUR"
    );

    std::vector<RecruitedAllyCombatSupport> recruitedAllies = collectActiveRecruitedAllies(player);
    std::vector<RecruitedAllyCombatSupport> reserveRecruitedAllies = collectReserveRecruitedAllies(player);
    RecruitedAllyOrderState recruitedAllyOrders;
    const bool employerMissionTeam = usesEmployerTeam(recruitedAllies);
    EmployerTacticalOrder employerTacticalOrder;
    EmployerOrderCombatReview employerOrderReview;
    if (employerMissionTeam)
    {
        employerTacticalOrder = issueEmployerTacticalOrder(
            player, wave, recruitedAllies, reserveRecruitedAllies, random, "pve_standard");
        applyEmployerTacticalOrderToAllies(employerTacticalOrder, wave, recruitedAllyOrders);
    }
    displayRecruitedAllyCombatStart(player, recruitedAllies);

    CombatRoleActionSystem::displayRoleIdentity(player);

    SummonControlMode playerSummonControlMode =
        SummonCombatSystem::askPlayerSummonControlMode(player, playerSummons);

    bool escapeSucceeded = false;
    int initialPlayerHp = player.getHp();
    int combatTurnCount = 0;
    player.beginChallengeCombatTracking();

    const bool playerStartsEmployerCombatInReserve = employerMissionTeam
        && EmployerTeamDisciplineSystem::playerStartsInReserve(player);
    if (playerStartsEmployerCombatInReserve)
    {
        const EmployerReserveObservationOutcome reserveOutcome = resolveEmployerReserveObservation(
            player, wave, random, difficulty, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders, employerTacticalOrder, "pve_standard");
        if (reserveOutcome == EmployerReserveObservationOutcome::TeamWonBeforePlayerEntry)
        {
            player.finishChallengeCombatTracking(false, false, defeatedWaveContainedElite(wave), wave.getDefeatedEnemyCount());
            PveWaveNarrativeSupport::displaySpecialDefeatDialogues(wave);
            return;
        }
    }

    while (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
    {
        if (employerMissionTeam && wave.hasActiveEnemies())
        {
            applyEmployerTacticalOrderToAllies(employerTacticalOrder, wave, recruitedAllyOrders);
            playRecruitedAllyCombatTurns(
                player, wave, random, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders, AllyTurnPhase::LeaderOnly);
        }

        bool playerTurnFinished = false;
        bool leaderRotationLockedForTurn = false;

        while (!playerTurnFinished
            // EN: !player.isDead declares or implements a focused behavior used by this module.
            // FR: !player.isDead déclare ou implémente un comportement précis utilisé par ce module.
            && !player.isDead()
            // EN: wave.hasEnemiesLeft declares or implements a focused behavior used by this module.
            // FR: wave.hasEnemiesLeft déclare ou implémente un comportement précis utilisé par ce module.
            && wave.hasEnemiesLeft()
            && !escapeSucceeded)
        {
            displayWaveCombatSnapshot(
                player,
                wave,
                playerSummons,
                "ÉTAT DU COMBAT",
                "Tour du joueur",
                combatTurnCount + 1
            );

            auto openOrders = [&]() -> bool {
                return openTeamOrdersMenu(player, wave, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders, random, leaderRotationLockedForTurn);
            };

            auto observeAction = [&](const PlayerWaveObservedAction& observedAction) {
                if (!employerMissionTeam) return;
                observeEmployerPlayerAction(
                    player, employerTacticalOrder, employerOrderReview,
                    recruitedAllies, reserveRecruitedAllies, wave, observedAction, escapeSucceeded,
                    "pve_standard", combatTurnCount + 1);
            };

            playerTurnFinished = PlayerWaveCombatTurn::play(
                player,
                wave,
                random,
                escapeSucceeded,
                difficulty,
                !recruitedAllies.empty(),
                openOrders,
                observeAction
            );

            if (playerTurnFinished)
            {
                ++combatTurnCount;
                player.reduceClassSkillCooldown();
            }

            if (!playerTurnFinished && !escapeSucceeded)
            {
                MessageScreen::show(
                    "ACTION NON CONSOMMÉE",
                    "combat.pve.turn.not_consumed",
                    {"Ton tour n'est pas encore consommé."},
                    true
                );
            }
        }

        if (playerTurnFinished && !escapeSucceeded && !player.isDead())
        {
            showCombatPhaseGate(
                "FIN DU TOUR JOUEUR",
                "combat.pve.phase.player_resolved",
                {"Action du joueur résolue.", "Le souffle du combat reprend, sans casser le rythme."}
            );
        }

        if (!player.isDead()
            // EN: wave.hasEnemiesLeft declares or implements a focused behavior used by this module.
            // FR: wave.hasEnemiesLeft déclare ou implémente un comportement précis utilisé par ce module.
            && wave.hasEnemiesLeft()
            && !escapeSucceeded
            // EN: hasActiveSummons declares or implements a focused behavior used by this module.
            // FR: hasActiveSummons déclare ou implémente un comportement précis utilisé par ce module.
            && SummonCombatSystem::hasActiveSummons(playerSummons))
        {
            showCombatPhaseGate(
                "TOUR DES INVOCATIONS ALLIÉES",
                "combat.pve.phase.player_summons",
                {"Les invocations alliées encore liées agissent séparément du joueur."}
            );
            SummonCombatSystem::playPlayerSummonTurnsAgainstWave(
                playerSummons,
                wave,
                random,
                playerSummonControlMode
            );
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            playBobMauriceAlliedTurn(player, wave, random);
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            playTemporaryAdventurerSupportTurn(player, wave, random, temporaryAdventurerSupport);
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            playRecruitedAllyCombatTurns(
                player,
                wave,
                random,
                recruitedAllies,
                reserveRecruitedAllies,
                recruitedAllyOrders,
                employerMissionTeam ? AllyTurnPhase::NonLeaderOnly : AllyTurnPhase::All
            );
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            showCombatPhaseGate(
                "TOUR DES ENNEMIS VIVANTS",
                "combat.pve.phase.enemies",
                {"Les ennemis actifs agissent maintenant, un groupe après l'autre."}
            );
            const int playerHpBeforeEnemyTurn = player.getHp();
            MonsterWaveCombatTurn::playMonsterTurns(
                player,
                wave,
                playerSummons,
                random,
                countLivingEscapeBlockers(player, recruitedAllies, reserveRecruitedAllies, playerSummons)
            );
            resolveRecruitedAllyInjuriesAfterEnemyTurn(player, wave, recruitedAllies, recruitedAllyOrders, random, difficulty, playerHpBeforeEnemyTurn);
            advanceRecruitedAllyRecoveryAfterEnemyResponse(recruitedAllyOrders);

            displayWaveCombatSnapshot(
                player,
                wave,
                playerSummons,
                "ÉTAT DU COMBAT",
                "Après la riposte ennemie",
                combatTurnCount,
                true
            );
        }
    }

    if (escapeSucceeded)
    {
        player.finishChallengeCombatTracking(false, false, false, wave.getDefeatedEnemyCount());
        showPostCombatRouteScreen(
            "FUITE RÉUSSIE",
            "combat.pve.escape.success",
            {
                "Résultat : fuite validée",
                "Route : combat PvE",
                "Tours joués : " + std::to_string(combatTurnCount),
                "Ennemis vaincus : " + std::to_string(wave.getDefeatedEnemyCount()),
                "Récompense : partielle, basée seulement sur ce qui s'est réellement passé",
                "Conséquence : aucune mort enregistrée"
            }
        );

        CombatReward reward = CombatRewardSystem::calculatePlayerEscapeReward(
            wave,
            difficulty
        );

        reward = splitCombatRewardWithRecruitedAllies(player, reward, recruitedAllies, "escape");

        CombatRewardSystem::displayPartialReward(
            reward,
            "Fuite réussie : les récompenses sont calculées selon la difficulté, les ennemis vaincus, et les ennemis encore en vie déjà blessés."
        );

        CombatRewardSystem::giveRewardToPlayer(player, reward);
        player.recordEscape();
        player.recordEnemyKills(wave.getDefeatedEnemyCount());
        PveWaveNarrativeSupport::recordWaveKillsInBestiary(wave);
        PveWaveNarrativeSupport::recordWaveKillsInJournal(player, wave);
        if (employerMissionTeam)
        {
            finalizeEmployerOrderReview(player, employerTacticalOrder, employerOrderReview, "pve_standard_escape", true);
        }

        return;
    }

    if (player.isDead())
    {
        player.finishChallengeCombatTracking(false, false, false, wave.getDefeatedEnemyCount());
        if (employerMissionTeam)
        {
            finalizeEmployerOrderReview(player, employerTacticalOrder, employerOrderReview, "pve_standard_defeat", false);
        }
        showPostCombatRouteScreen(
            "DÉFAITE",
            "combat.pve.defeat",
            {
                "Résultat : défaite",
                "Route : combat PvE",
                "Personnage : " + player.getName(),
                "Tours joués : " + std::to_string(combatTurnCount),
                "Ennemis vaincus avant chute : " + std::to_string(wave.getDefeatedEnemyCount()),
                "Mort : enregistrée selon la difficulté",
                "Action suivante : application de la règle de mort"
            }
        );

        player.recordDefeat();

        const PlayerDeathImmediateResolution immediateDeath = PlayerDeathResolver::resolveImmediate(player, "pve_standard");
        if (immediateDeath.handled)
        {
            showPostCombatRouteScreen("SACRIFICE D'HESTIA", "combat.pve.hestia_sacrifice", immediateDeath.lines);
            return;
        }

        if (DifficultyRules::isPermanentDeath(difficulty, deathRule))
        {
            if (!recruitedAllies.empty() || !reserveRecruitedAllies.empty())
            {
                // En mort définitive, on ne fige pas les sauveteurs au moment exact de la chute :
                // l'équipe doit d'abord survivre à la fin du combat ou réussir l'extraction.
                // Un soigneur tombé pendant cette simulation ne peut donc plus fournir son bonus.
                simulateRecruitedAlliesAfterPlayerDown(
                    player, wave, random, difficulty, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders,
                    "pve_standard_lethal");
            }
            if (tryLethalTeamRescueAndDisplay(player, recruitedAllies, reserveRecruitedAllies, random, "pve_standard"))
            {
                return;
            }
            if (tryFirstLethalSpiritResistanceAndDisplay(player, random, "pve_standard"))
            {
                return;
            }
            if (BlessingSystem::tryTriggerLethalSurvival(player))
            {
                DeathPenaltySystem::displayLethalSurvivalAnomaly();
                return;
            }
            player.recordDeath();
            DeathPenaltySystem::displayLethalDeathCorruption();
            return;
        }

        const CombatReward rewardAtPlayerDown = CombatRewardSystem::calculatePlayerEscapeReward(wave, difficulty);
        const bool hadTeamAtPlayerDown = !recruitedAllies.empty() || !reserveRecruitedAllies.empty();
        RecruitedAllyPostKoOutcome postKoOutcome;
        if (hadTeamAtPlayerDown)
        {
            postKoOutcome = simulateRecruitedAlliesAfterPlayerDown(
                player, wave, random, difficulty, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders, "pve_standard");
        }
        const bool rescuedByTeam = postKoOutcome.teamRecoveredPlayer;
        const CombatReward postKoGroupReward = postKoOutcome.teamFinishedFight
            ? CombatRewardSystem::calculateWaveReward(wave, difficulty)
            : rewardAtPlayerDown;
        const int owedEconomyUnits = rescuedByTeam
            ? settlePostKoGroupEconomy(player, postKoGroupReward, postKoOutcome.participants, "pve_standard")
            : 0;
        const RecruitedAllyLootTransportPlan postKoLootPlan = (rescuedByTeam && postKoOutcome.teamFinishedFight)
            ? RecruitedAllyAftermathSystem::buildLootTransportPlan(
                postKoOutcome.participants, wave.getDefeatedEnemyCount(), true)
            : RecruitedAllyLootTransportPlan{};

        player.recordDeath();
        DeathPenaltyResult deathPenalty = DeathPenaltySystem::applyNonLethalDeathPenalty(
            player,
            difficulty,
            random
        );

        DeathPenaltySystem::displayNonLethalDeathPenalty(deathPenalty);
        resolvePlayerInfirmaryTransferAfterNonLethalDeath(
            player,
            difficulty,
            random,
            rescuedByTeam,
            postKoOutcome,
            owedEconomyUnits,
            postKoOutcome.participants,
            "pve_standard"
        );
        if (postKoOutcome.teamFinishedFight)
        {
            LootGenerator::givePostKoRecoveredWaveLoot(
                player, wave, random, difficulty,
                postKoLootPlan.availableLootCarryUnits,
                "pve_standard",
                "secteur de combat autour de " + player.getCurrentCityId());
        }

        PveWaveNarrativeSupport::displaySpecialVictoryDialogues(wave);

        player.reviveWithHealthPercentage(
            DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty)
        );

        showPostCombatRouteScreen(
            "RETOUR À LA VIE",
            "combat.pve.revive",
            {
                "Résultat : personnage restauré",
                "Personnage : " + player.getName(),
                "PV après retour : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()),
                "Difficulté : mort non définitive",
                "Conséquence : pénalité de mort déjà appliquée",
                "Note : tu as survécu, mais la mort a laissé sa trace"
            }
        );

        return;
    }

    player.finishChallengeCombatTracking(
        true,
        false,
        defeatedWaveContainedElite(wave),
        wave.getDefeatedEnemyCount()
    );

    PveWaveNarrativeSupport::displaySpecialDefeatDialogues(wave);

    showPostCombatRouteScreen(
        "VICTOIRE PVE",
        "combat.pve.victory",
        {
            "Résultat : victoire",
            "Route : combat PvE",
            "Personnage : " + player.getName(),
            "Tours joués : " + std::to_string(combatTurnCount),
            "Ennemis vaincus : " + std::to_string(wave.getDefeatedEnemyCount()),
            "Action suivante : récompenses complètes puis butin"
        }
    );

    CombatReward reward = CombatRewardSystem::calculateWaveReward(
        wave,
        difficulty,
        player,
        initialPlayerHp,
        combatTurnCount,
        random
    );

    reward = splitCombatRewardWithRecruitedAllies(player, reward, recruitedAllies, "victory");
    CombatRewardSystem::displayReward(reward);
    CombatRewardSystem::giveRewardToPlayer(player, reward);
    player.recordVictory();
    player.recordEnemyKills(wave.getDefeatedEnemyCount());
    PveWaveNarrativeSupport::recordWaveKillsInBestiary(wave);
    PveWaveNarrativeSupport::recordWaveKillsInJournal(player, wave);
    LootGenerator::giveDefeatedWaveLoot(player, wave, random, difficulty);
    maybeTriggerRareScavengerAfterCombat(player, random, "pve_standard");
    if (employerMissionTeam)
    {
        finalizeEmployerOrderReview(player, employerTacticalOrder, employerOrderReview, "pve_standard_victory", true);
    }

    int evolvedKilled = countDefeatedEvolvedMonsters(wave);
    if (evolvedKilled > 0)
    {
        int updated = player.getQuestLog().progressCombatQuestsByFamily(evolvedKilled, "Créature évoluée");
        if (updated > 0)
        {
            MessageScreen::show(
                "PROGRESSION BESTIAIRE",
                "combat.pve.evolved_quest.progress",
                {"Le bestiaire et les quêtes liées aux créatures évoluées progressent."},
                false
            );
        }
    }
}


bool MonsterPveMode::runExplorationWave(
    Player& player,
    Random& random,
    DifficultyMode difficulty,
    DeathRuleMode deathRule,
    const std::vector<Monster>& monsters,
    const std::string& title,
    bool friendlyTrial,
    bool municipalGuardSupport,
    int authorityRewardPercent,
    int authorizedLootPercent
)
{
    player.resetEscapeOpeningProgress();
    Console::clear();

    EnemyCombatQueue wave;
    for (const Monster& monster : monsters)
    {
        wave.addWaitingEnemy(monster);
    }
    wave.initializeFrontLine();

    MessageScreen::show(
        "ÉVÉNEMENT D'EXPLORATION",
        "exploration.wave.intro",
        {
            title,
            "La rencontre se referme autour de toi : il faut tenir la ligne."
        },
        false
    );

    WaveCombatSystem::displayFrontLineArrival(wave);
    PveWaveNarrativeSupport::recordWaveEncountersInBestiary(wave);
    PveWaveNarrativeSupport::recordWaveEncountersInJournal(player, wave);
    EncounterDialogueSystem::display(player, wave, random, "exploration.wave");

    CombatGroup enemyFrontPreview = CombatGroupBuilder::buildSideFromWave(
        wave,
        CombatSide::EnemySide
    );

    CombatGroupBuilder::displayGroup(
        enemyFrontPreview,
        "LIGNE ENNEMIE ACTIVE"
    );

    std::vector<Summon> playerSummons = SummonCombatSystem::createInitialSummonsFor(player);
    SummonCombatSystem::displaySummonArrival(player, playerSummons);

    CombatGroup playerGroupPreview = CombatGroupBuilder::buildSideFromEntityAndSummons(
        player,
        playerSummons,
        CombatSide::PlayerSide,
        CombatUnitKind::MainFighter
    );

    CombatGroupBuilder::displayGroup(
        playerGroupPreview,
        "GROUPE DU JOUEUR"
    );

    std::vector<RecruitedAllyCombatSupport> recruitedAllies = collectActiveRecruitedAllies(player);
    std::vector<RecruitedAllyCombatSupport> reserveRecruitedAllies = collectReserveRecruitedAllies(player);
    bool municipalGuardJoined = false;
    if (municipalGuardSupport
        && recruitedAllies.size() + reserveRecruitedAllies.size() < static_cast<std::size_t>(RecruitedAllyCombatRoster::MaxRecruitedTeamMembers))
    {
        RecruitedAllyCombatSupport guard;
        guard.name = "Garde municipal";
        guard.race = "Humain";
        guard.job = "gardien municipal";
        guard.trait = "discipliné, protège le coffre sans demander de part";
        guard.motivation = "défense de la ville";
        guard.recruitedLevel = std::max(1, player.getLevel());
        guard.estimatedLevel = std::max(1, player.getLevel());
        guard.currentRankIndex = std::clamp(player.getLevel() / 18, 0, 5);
        guard.requestedShare = 0;
        guard.maxHp = std::max(80, 70 + player.getLevel() * 5);
        guard.currentHp = guard.maxHp;
        guard.healingPotionCharges = 1;
        guard.weaponQuality = std::clamp(2 + player.getLevel() / 20, 2, 8);
        guard.armorQuality = std::clamp(2 + player.getLevel() / 18, 2, 8);
        guard.temporaryNoReward = true;
        if (recruitedAllies.size() < static_cast<std::size_t>(RecruitedAllyCombatRoster::MaxActiveRecruitsWithPlayer)) recruitedAllies.push_back(guard);
        else reserveRecruitedAllies.push_back(guard);
        municipalGuardJoined = true;
    }
    RecruitedAllyOrderState recruitedAllyOrders;
    const bool employerMissionTeam = usesEmployerTeam(recruitedAllies);
    EmployerTacticalOrder employerTacticalOrder;
    EmployerOrderCombatReview employerOrderReview;
    if (employerMissionTeam)
    {
        employerTacticalOrder = issueEmployerTacticalOrder(
            player, wave, recruitedAllies, reserveRecruitedAllies, random, "exploration_wave");
        applyEmployerTacticalOrderToAllies(employerTacticalOrder, wave, recruitedAllyOrders);
    }
    displayRecruitedAllyCombatStart(player, recruitedAllies);

    CombatRoleActionSystem::displayRoleIdentity(player);

    SummonControlMode playerSummonControlMode =
        SummonCombatSystem::askPlayerSummonControlMode(player, playerSummons);

    bool escapeSucceeded = false;
    int initialPlayerHp = player.getHp();
    int combatTurnCount = 0;
    player.beginChallengeCombatTracking();

    const bool playerStartsEmployerExplorationInReserve = employerMissionTeam
        && EmployerTeamDisciplineSystem::playerStartsInReserve(player);
    if (playerStartsEmployerExplorationInReserve)
    {
        const EmployerReserveObservationOutcome reserveOutcome = resolveEmployerReserveObservation(
            player, wave, random, difficulty, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders, employerTacticalOrder, "exploration_wave");
        if (reserveOutcome == EmployerReserveObservationOutcome::TeamWonBeforePlayerEntry)
        {
            player.finishChallengeCombatTracking(false, false, defeatedWaveContainedElite(wave), wave.getDefeatedEnemyCount());
            PveWaveNarrativeSupport::displaySpecialDefeatDialogues(wave);
            return true;
        }
    }

    while (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
    {
        if (employerMissionTeam && wave.hasActiveEnemies())
        {
            applyEmployerTacticalOrderToAllies(employerTacticalOrder, wave, recruitedAllyOrders);
            playRecruitedAllyCombatTurns(
                player, wave, random, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders, AllyTurnPhase::LeaderOnly);
        }

        bool playerTurnFinished = false;
        bool leaderRotationLockedForTurn = false;

        while (!playerTurnFinished
            && !player.isDead()
            && wave.hasEnemiesLeft()
            && !escapeSucceeded)
        {
            displayWaveCombatSnapshot(
                player,
                wave,
                playerSummons,
                "ÉTAT DE L'ÉVÉNEMENT",
                "Tour du joueur",
                combatTurnCount + 1
            );

            auto openOrders = [&]() -> bool {
                return openTeamOrdersMenu(player, wave, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders, random, leaderRotationLockedForTurn);
            };

            auto observeAction = [&](const PlayerWaveObservedAction& observedAction) {
                if (!employerMissionTeam) return;
                observeEmployerPlayerAction(
                    player, employerTacticalOrder, employerOrderReview,
                    recruitedAllies, reserveRecruitedAllies, wave, observedAction, escapeSucceeded,
                    "exploration_wave", combatTurnCount + 1);
            };

            playerTurnFinished = PlayerWaveCombatTurn::play(
                player,
                wave,
                random,
                escapeSucceeded,
                difficulty,
                !recruitedAllies.empty(),
                openOrders,
                observeAction
            );

            if (playerTurnFinished)
            {
                ++combatTurnCount;
                player.reduceClassSkillCooldown();
            }

            if (!playerTurnFinished && !escapeSucceeded)
            {
                MessageScreen::show(
                    "ACTION NON CONSOMMÉE",
                    "exploration.wave.turn.not_consumed",
                    {"Ton tour n'est pas encore consommé."},
                    true
                );
            }
        }

        if (playerTurnFinished && !escapeSucceeded && !player.isDead())
        {
            showCombatPhaseGate(
                "FIN DU TOUR JOUEUR",
                "exploration.wave.phase.player_resolved",
                {"Action du joueur résolue.", "Le souffle du combat reprend, sans casser le rythme."}
            );
        }

        if (!player.isDead()
            && wave.hasEnemiesLeft()
            && !escapeSucceeded
            && SummonCombatSystem::hasActiveSummons(playerSummons))
        {
            showCombatPhaseGate(
                "TOUR DES INVOCATIONS ALLIÉES",
                "exploration.wave.phase.player_summons",
                {"Les invocations alliées encore liées agissent avant les ennemis de l'événement."}
            );
            SummonCombatSystem::playPlayerSummonTurnsAgainstWave(
                playerSummons,
                wave,
                random,
                playerSummonControlMode
            );
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            playBobMauriceAlliedTurn(player, wave, random);
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            playRecruitedAllyCombatTurns(
                player,
                wave,
                random,
                recruitedAllies,
                reserveRecruitedAllies,
                recruitedAllyOrders,
                employerMissionTeam ? AllyTurnPhase::NonLeaderOnly : AllyTurnPhase::All
            );
        }

        if (!player.isDead() && wave.hasEnemiesLeft() && !escapeSucceeded)
        {
            showCombatPhaseGate(
                "TOUR DES ENNEMIS VIVANTS",
                "exploration.wave.phase.enemies",
                {"Les ennemis actifs de l'événement agissent maintenant."}
            );
            const int playerHpBeforeEnemyTurn = player.getHp();
            MonsterWaveCombatTurn::playMonsterTurns(
                player,
                wave,
                playerSummons,
                random,
                countLivingEscapeBlockers(player, recruitedAllies, reserveRecruitedAllies, playerSummons)
            );
            resolveRecruitedAllyInjuriesAfterEnemyTurn(player, wave, recruitedAllies, recruitedAllyOrders, random, difficulty, playerHpBeforeEnemyTurn);
            advanceRecruitedAllyRecoveryAfterEnemyResponse(recruitedAllyOrders);

            displayWaveCombatSnapshot(
                player,
                wave,
                playerSummons,
                "ÉTAT DE L'ÉVÉNEMENT",
                "Après la riposte ennemie",
                combatTurnCount,
                true
            );
        }
    }

    if (escapeSucceeded)
    {
        player.finishChallengeCombatTracking(false, false, false, wave.getDefeatedEnemyCount());
        if (employerMissionTeam)
        {
            finalizeEmployerOrderReview(player, employerTacticalOrder, employerOrderReview, "exploration_wave_escape", !friendlyTrial);
        }
        if (friendlyTrial)
        {
            showPostCombatRouteScreen(
                "ÉPREUVE ABANDONNÉE",
                "exploration.wave.friendly_trial.escape",
                {
                    "Résultat : défi amical abandonné",
                    "Aucune mort, pénalité ou statistique réelle n'est enregistrée.",
                    "Les objets des caisses restent dans le sous-inventaire temporaire et disparaissent avec l'épreuve."
                }
            );
            return false;
        }
        showPostCombatRouteScreen(
            "FUITE D'EXPLORATION",
            "exploration.wave.escape.success",
            {
                "Résultat : fuite d'événement",
                "Route : exploration",
                "Tours joués : " + std::to_string(combatTurnCount),
                "Ennemis vaincus : " + std::to_string(wave.getDefeatedEnemyCount()),
                "Récompense : partielle, actions réelles uniquement",
                "Conséquence : exploration interrompue proprement"
            }
        );

        CombatReward reward = CombatRewardSystem::calculatePlayerEscapeReward(
            wave,
            difficulty
        );

        reward = splitCombatRewardWithRecruitedAllies(player, reward, recruitedAllies, "exploration_escape");

        CombatRewardSystem::displayPartialReward(
            reward,
            "Fuite d'exploration : seules les actions réelles comptent."
        );

        CombatRewardSystem::giveRewardToPlayer(player, reward);
        player.recordEscape();
        player.recordEnemyKills(wave.getDefeatedEnemyCount());
        PveWaveNarrativeSupport::recordWaveKillsInBestiary(wave);
        PveWaveNarrativeSupport::recordWaveKillsInJournal(player, wave);
        return false;
    }

    if (player.isDead())
    {
        player.finishChallengeCombatTracking(false, false, false, wave.getDefeatedEnemyCount());
        if (employerMissionTeam)
        {
            finalizeEmployerOrderReview(player, employerTacticalOrder, employerOrderReview, "exploration_wave_defeat", false);
        }
        if (friendlyTrial)
        {
            showPostCombatRouteScreen(
                "DÉFAITE AMICALE",
                "exploration.wave.friendly_trial.defeat",
                {
                    "Résultat : personnage mis hors combat pendant l'épreuve",
                    "Cette chute n'est pas une mort et ne modifie ni les statistiques, ni l'équipement réel, ni les règles létales.",
                    "Le combat s'arrête avant toute pénalité."
                }
            );
            return false;
        }
        showPostCombatRouteScreen(
            "DÉFAITE D'EXPLORATION",
            "exploration.wave.defeat",
            {
                "Résultat : défaite d'événement",
                "Route : exploration",
                "Personnage : " + player.getName(),
                "Tours joués : " + std::to_string(combatTurnCount),
                "Ennemis vaincus avant chute : " + std::to_string(wave.getDefeatedEnemyCount()),
                "Mort : enregistrée selon la difficulté",
                "Action suivante : application de la règle de mort"
            }
        );

        player.recordDefeat();

        const PlayerDeathImmediateResolution immediateDeath = PlayerDeathResolver::resolveImmediate(player, "exploration_wave");
        if (immediateDeath.handled)
        {
            showPostCombatRouteScreen("SACRIFICE D'HESTIA", "exploration.wave.hestia_sacrifice", immediateDeath.lines);
            return false;
        }

        if (DifficultyRules::isPermanentDeath(difficulty, deathRule))
        {
            if (!recruitedAllies.empty() || !reserveRecruitedAllies.empty())
            {
                simulateRecruitedAlliesAfterPlayerDown(
                    player, wave, random, difficulty, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders,
                    "exploration_wave_lethal");
            }
            if (tryLethalTeamRescueAndDisplay(player, recruitedAllies, reserveRecruitedAllies, random, "exploration_wave"))
            {
                return false;
            }
            if (tryFirstLethalSpiritResistanceAndDisplay(player, random, "exploration_wave"))
            {
                return false;
            }
            if (BlessingSystem::tryTriggerLethalSurvival(player))
            {
                DeathPenaltySystem::displayLethalSurvivalAnomaly();
                return false;
            }
            player.recordDeath();
            DeathPenaltySystem::displayLethalDeathCorruption();
            return false;
        }

        const CombatReward rewardAtPlayerDown = CombatRewardSystem::calculatePlayerEscapeReward(wave, difficulty);
        const bool hadTeamAtPlayerDown = !recruitedAllies.empty() || !reserveRecruitedAllies.empty();
        RecruitedAllyPostKoOutcome postKoOutcome;
        if (hadTeamAtPlayerDown)
        {
            postKoOutcome = simulateRecruitedAlliesAfterPlayerDown(
                player, wave, random, difficulty, recruitedAllies, reserveRecruitedAllies, recruitedAllyOrders, "exploration_wave");
        }
        const bool rescuedByTeam = postKoOutcome.teamRecoveredPlayer;
        const CombatReward postKoGroupReward = postKoOutcome.teamFinishedFight
            ? CombatRewardSystem::calculateWaveReward(wave, difficulty)
            : rewardAtPlayerDown;
        const int owedEconomyUnits = rescuedByTeam
            ? settlePostKoGroupEconomy(player, postKoGroupReward, postKoOutcome.participants, "exploration_wave")
            : 0;
        const RecruitedAllyLootTransportPlan postKoLootPlan = (rescuedByTeam && postKoOutcome.teamFinishedFight)
            ? RecruitedAllyAftermathSystem::buildLootTransportPlan(
                postKoOutcome.participants, wave.getDefeatedEnemyCount(), true)
            : RecruitedAllyLootTransportPlan{};

        player.recordDeath();
        DeathPenaltyResult deathPenalty = DeathPenaltySystem::applyNonLethalDeathPenalty(
            player,
            difficulty,
            random
        );

        DeathPenaltySystem::displayNonLethalDeathPenalty(deathPenalty);
        resolvePlayerInfirmaryTransferAfterNonLethalDeath(
            player,
            difficulty,
            random,
            rescuedByTeam,
            postKoOutcome,
            owedEconomyUnits,
            postKoOutcome.participants,
            "exploration_wave"
        );
        if (postKoOutcome.teamFinishedFight)
        {
            LootGenerator::givePostKoRecoveredWaveLoot(
                player, wave, random, difficulty,
                postKoLootPlan.availableLootCarryUnits,
                "exploration_wave",
                title);
        }
        PveWaveNarrativeSupport::displaySpecialVictoryDialogues(wave);

        player.reviveWithHealthPercentage(
            DifficultyRules::getNonLethalRespawnHealthPercentage(difficulty)
        );

        showPostCombatRouteScreen(
            "RETOUR À LA VIE",
            "exploration.wave.revive",
            {
                "Résultat : personnage restauré",
                "Route : exploration",
                "Personnage : " + player.getName(),
                "PV après retour : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()),
                "Difficulté : mort non définitive",
                "Conséquence : pénalité de mort déjà appliquée"
            }
        );
        return false;
    }

    if (friendlyTrial)
    {
        player.finishChallengeCombatTracking(false, false, false, wave.getDefeatedEnemyCount());
        PveWaveNarrativeSupport::displaySpecialDefeatDialogues(wave);
        showPostCombatRouteScreen(
            "ÉPREUVE AMICALE RÉUSSIE",
            "exploration.wave.friendly_trial.victory",
            {
                "Résultat : mini-boss vaincu avec le contenu des caisses",
                "Aucun butin standard n'est généré dans le sous-inventaire temporaire.",
                "Bob et Maurice calculent maintenant leur récompense à leur manière."
            }
        );
        if (employerMissionTeam)
        {
            finalizeEmployerOrderReview(player, employerTacticalOrder, employerOrderReview, "exploration_wave_friendly_victory", false);
        }
        return true;
    }

    player.finishChallengeCombatTracking(
        true,
        false,
        defeatedWaveContainedElite(wave),
        wave.getDefeatedEnemyCount()
    );

    PveWaveNarrativeSupport::displaySpecialDefeatDialogues(wave);

    showPostCombatRouteScreen(
        "ÉVÉNEMENT TERMINÉ",
        "exploration.wave.victory",
        {
            "Résultat : événement terminé",
            "Route : exploration",
            "Personnage : " + player.getName(),
            "Tours joués : " + std::to_string(combatTurnCount),
            "Ennemis vaincus : " + std::to_string(wave.getDefeatedEnemyCount()),
            "Action suivante : récompenses d'exploration"
        }
    );

    CombatReward reward = CombatRewardSystem::calculateWaveReward(
        wave,
        difficulty,
        player,
        initialPlayerHp,
        combatTurnCount,
        random
    );

    reward = splitCombatRewardWithRecruitedAllies(player, reward, recruitedAllies, "exploration_victory");
    authorityRewardPercent = std::clamp(authorityRewardPercent, 0, 100);
    reward = reward.getPercentage(authorityRewardPercent);
    if (municipalGuardJoined)
    {
        // A municipal guard never takes loot or money, but real assistance slightly reduces personal combat XP.
        reward = reward.getModified(90, 100);
    }
    CombatRewardSystem::displayReward(reward);
    CombatRewardSystem::giveRewardToPlayer(player, reward);
    player.recordVictory();
    player.recordEnemyKills(wave.getDefeatedEnemyCount());
    PveWaveNarrativeSupport::recordWaveKillsInBestiary(wave);
    PveWaveNarrativeSupport::recordWaveKillsInJournal(player, wave);
    LootGenerator::giveDefeatedWaveLoot(player, wave, random, difficulty, authorizedLootPercent);
    maybeTriggerRareScavengerAfterCombat(player, random, "exploration_wave");
    if (employerMissionTeam)
    {
        finalizeEmployerOrderReview(player, employerTacticalOrder, employerOrderReview, "exploration_wave_victory", true);
    }

    int evolvedKilled = countDefeatedEvolvedMonsters(wave);
    if (evolvedKilled > 0)
    {
        int updated = player.getQuestLog().progressCombatQuestsByFamily(evolvedKilled, "Créature évoluée");
        if (updated > 0)
        {
            MessageScreen::show(
                "PROGRESSION BESTIAIRE",
                "exploration.wave.evolved_quest.progress",
                {"Le bestiaire et les quêtes liées aux créatures évoluées progressent."},
                false
            );
        }
    }

    return true;
}
