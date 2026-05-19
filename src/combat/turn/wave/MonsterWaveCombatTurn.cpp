// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/wave/MonsterWaveCombatTurn.hpp"

#include "combat/TurnManager.hpp"
#include "combat/system/EscapeSystem.hpp"
#include "combat/summon/SummonCombatSystem.hpp"
#include "combat/threat/ThreatSystem.hpp"
#include "combat/role/CombatRoleActionSystem.hpp"

#include "core/Console.hpp"

#include "entity/Monster.hpp"

#include <iostream>

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

        std::cout << "Tour de " << monster.getName() << std::endl;
        std::cout << std::endl;

        Console::pauseSeconds(1);

        CombatRoleActionSystem::tryActivateAutomaticRoleReaction(player, random);

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

        std::cout << "Tour de " << monster.getName() << std::endl;
        std::cout << std::endl;

        Console::pauseSeconds(1);

        CombatRoleActionSystem::tryActivateAutomaticRoleReaction(player, random);

        bool attackedSummon = false;

        if (ThreatSystem::shouldForceTargetMainEntity(player, monster.getName()))
        {
            ThreatSystem::notifyForcedTarget(player, monster.getName());
        }
        else if (SummonCombatSystem::hasTargetableSummons(playerSummons)
            && random.between(1, 100) <= 30)
        {
            int summonIndex = SummonCombatSystem::chooseRandomTargetableSummonIndex(
                playerSummons,
                random
            );

            if (summonIndex >= 0)
            {
                std::cout << monster.getName()
                          << " se jette sur une invocation au lieu de viser directement "
                          << player.getName()
                          << "."
                          << std::endl;
                std::cout << std::endl;

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
