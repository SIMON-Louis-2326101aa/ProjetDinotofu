// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/wave/MonsterWaveCombatTurn.hpp"

#include "combat/TurnManager.hpp"
#include "combat/system/EscapeSystem.hpp"

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
            wave.removeActiveEnemy(i);
            continue;
        }

        std::cout << "Tour de " << monster.getName() << std::endl;
        std::cout << std::endl;

        Console::pauseSeconds(1);

        TurnManager::executeAttack(
            monster,
            player,
            random
        );

        Console::pauseSeconds(1);

        ++i;
    }

    wave.removeDeadAndReplace();
}