// EN: MonsterWaveCombatTurn.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: MonsterWaveCombatTurn.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/wave/MonsterWaveCombatTurn.hpp"

#include "combat/TurnManager.hpp"
#include "combat/ai/CombatAI.hpp"
#include "combat/system/EscapeSystem.hpp"
#include "combat/summon/SummonCombatSystem.hpp"
#include "combat/threat/ThreatSystem.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"
#include "combat/role/CombatRoleSystem.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include "entity/Monster.hpp"

#include <string>
#include <vector>

namespace
{
    // EN: calculateHpPercentage declares or implements a focused behavior used by this module.
    // FR: calculateHpPercentage déclare ou implémente un comportement précis utilisé par ce module.
    int calculateHpPercentage(const Entity& entity)
    {
        if (entity.getMaxHp() <= 0)
        {
            return 0;
        }

        return entity.getHp() * 100 / entity.getMaxHp();
    }

    // EN: findMostInjuredAllyIndex declares or implements a focused behavior used by this module.
    // FR: findMostInjuredAllyIndex déclare ou implémente un comportement précis utilisé par ce module.
    void showWaveTurnNotice(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        MessageScreen::show(title, screenId, lines, false);
    }

    int findMostInjuredAllyIndex(EnemyCombatQueue& wave, int healerIndex)
    {
        int bestIndex = -1;
        int bestPercentage = 101;

        for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
        {
            if (i == healerIndex)
            {
                continue;
            }

            Monster& ally = wave.getActiveEnemy(i);

            if (ally.isDead() || ally.getHp() >= ally.getMaxHp())
            {
                continue;
            }

            int percentage = calculateHpPercentage(ally);

            if (percentage < bestPercentage)
            {
                bestPercentage = percentage;
                bestIndex = i;
            }
        }

        return bestIndex;
    }

    bool tryHealerSupportAction(
        Monster& healer,
        EnemyCombatQueue& wave,
        int healerIndex,
        Random& random
    )
    {
        if (!CombatRoleSystem::isHealer(healer))
        {
            return false;
        }

        int allyIndex = findMostInjuredAllyIndex(wave, healerIndex);

        if (allyIndex < 0)
        {
            return false;
        }

        Monster& ally = wave.getActiveEnemy(allyIndex);
        int allyHpPercent = calculateHpPercentage(ally);
        int chance = allyHpPercent <= 35 ? 85 : 45;

        if (random.between(1, 100) > chance)
        {
            return false;
        }

        int healAmount = random.between(28, 44);
        ally.heal(healAmount);
        ThreatSystem::markAllyHealingAction(healer, ally);

        showWaveTurnNotice(
            "SOUTIEN DE VAGUE",
            "wave.monster.support.heal",
            {
                healer.getName() + " protège " + ally.getName() + " avec un soin rapide.",
                "Soin : +" + std::to_string(healAmount) + " PV.",
                ally.getName() + " possède maintenant " + std::to_string(ally.getHp()) + "/" + std::to_string(ally.getMaxHp()) + " PV."
            }
        );
        return true;
    }

    bool tryTankProtectionAction(
        Monster& tank,
        EnemyCombatQueue& wave,
        int tankIndex,
        Random& random
    )
    {
        int allyIndex = findMostInjuredAllyIndex(wave, tankIndex);

        if (allyIndex < 0)
        {
            return false;
        }

        return CombatRoleActionSystem::tryActivateAllyProtection(
            tank,
            wave.getActiveEnemy(allyIndex),
            random
        );
    }

    bool trySupportRecoveryAction(
        Monster& support,
        EnemyCombatQueue& wave,
        int supportIndex,
        Random& random
    )
    {
        int allyIndex = findMostInjuredAllyIndex(wave, supportIndex);

        if (allyIndex < 0)
        {
            return false;
        }

        return CombatRoleActionSystem::tryActivateSupportRecovery(
            support,
            wave.getActiveEnemy(allyIndex),
            random
        );
    }
}

void MonsterWaveCombatTurn::playMonsterTurns(
    Player& player,
    EnemyCombatQueue& wave,
    Random& random
)
{
    int i = 0;

    while (i < wave.getActiveEnemyCount())
    {
        if (player.isDead())
        {
            return;
        }

        if (!wave.isActiveIndexValid(i))
        {
            ++i;
            continue;
        }

        Monster& monster = wave.getActiveEnemy(i);

        if (monster.isDead())
        {
            ++i;
            continue;
        }

        if (EscapeSystem::monsterAttemptsEscape(monster, random))
        {
            wave.removeActiveEnemyAsEscaped(i);
            continue;
        }

        showWaveTurnNotice(
            "TOUR DE VAGUE",
            "wave.monster.turn.start",
            {"Tour de " + monster.getName() + "."}
        );

        Console::pauseSeconds(1);

        CombatRoleActionSystem::tryActivateAutomaticRoleReaction(player, random);

        if (tryTankProtectionAction(monster, wave, i, random)
            // EN: tryHealerSupportAction declares or implements a focused behavior used by this module.
            // FR: tryHealerSupportAction déclare ou implémente un comportement précis utilisé par ce module.
            || tryHealerSupportAction(monster, wave, i, random)
            // EN: trySupportRecoveryAction declares or implements a focused behavior used by this module.
            // FR: trySupportRecoveryAction déclare ou implémente un comportement précis utilisé par ce module.
            || trySupportRecoveryAction(monster, wave, i, random))
        {
            Console::pauseSeconds(1);
            ++i;
            continue;
        }

        TurnManager::executeAttack(
            monster,
            player,
            random
        );

        ThreatSystem::consumeForcedTargetIfNeeded(player);

        Console::pauseSeconds(1);

        ++i;
    }

    wave.removeDeadAndReplace();
}

void MonsterWaveCombatTurn::playMonsterTurns(
    Player& player,
    EnemyCombatQueue& wave,
    std::vector<Summon>& playerSummons,
    Random& random
)
{
    int i = 0;

    while (i < wave.getActiveEnemyCount())
    {
        if (player.isDead())
        {
            return;
        }

        if (!wave.isActiveIndexValid(i))
        {
            ++i;
            continue;
        }

        Monster& monster = wave.getActiveEnemy(i);

        if (monster.isDead())
        {
            ++i;
            continue;
        }

        if (EscapeSystem::monsterAttemptsEscape(monster, random))
        {
            wave.removeActiveEnemyAsEscaped(i);
            continue;
        }

        showWaveTurnNotice(
            "TOUR DE VAGUE",
            "wave.monster.turn.start",
            {"Tour de " + monster.getName() + "."}
        );

        Console::pauseSeconds(1);

        CombatRoleActionSystem::tryActivateAutomaticRoleReaction(player, random);

        if (tryTankProtectionAction(monster, wave, i, random)
            // EN: tryHealerSupportAction declares or implements a focused behavior used by this module.
            // FR: tryHealerSupportAction déclare ou implémente un comportement précis utilisé par ce module.
            || tryHealerSupportAction(monster, wave, i, random)
            // EN: trySupportRecoveryAction declares or implements a focused behavior used by this module.
            // FR: trySupportRecoveryAction déclare ou implémente un comportement précis utilisé par ce module.
            || trySupportRecoveryAction(monster, wave, i, random))
        {
            Console::pauseSeconds(1);
            ++i;
            continue;
        }

        bool attackedSummon = false;

        if (ThreatSystem::shouldForceTargetMainEntity(player, monster.getName()))
        {
            ThreatSystem::notifyForcedTarget(player, monster.getName());
        }
        else if (SummonCombatSystem::hasTargetableSummons(playerSummons)
            && random.between(1, 100) <= CombatAI::getSummonTargetPriorityChance(monster))
        {
            int summonIndex = SummonCombatSystem::chooseStrategicTargetableSummonIndex(
                playerSummons,
                monster,
                random
            );

            if (summonIndex >= 0)
            {
                showWaveTurnNotice(
                    "CIBLE CHANGÉE",
                    "wave.monster.target.summon",
                    {
                        monster.getName() + " se jette sur une invocation.",
                        player.getName() + " n'est pas visé directement pendant cette action."
                    }
                );

                SummonCombatSystem::entityAttacksSummon(
                    monster,
                    playerSummons[summonIndex],
                    random
                );

                SummonCombatSystem::removeInactiveSummons(playerSummons);
                attackedSummon = true;
            }
        }

        if (!attackedSummon)
        {
            TurnManager::executeAttack(
                monster,
                player,
                random
            );

            ThreatSystem::consumeForcedTargetIfNeeded(player);
        }

        Console::pauseSeconds(1);

        ++i;
    }

    wave.removeDeadAndReplace();
}
