// EN: EnemyCombatQueue.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: EnemyCombatQueue.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/EnemyCombatQueue.hpp"

#include <iostream>

// EN: EnemyCombatQueue declares or implements a focused behavior used by this module.
// FR: EnemyCombatQueue déclare ou implémente un comportement précis utilisé par ce module.
EnemyCombatQueue::EnemyCombatQueue()
{
}

// EN: addWaitingEnemy declares or implements a focused behavior used by this module.
// FR: addWaitingEnemy déclare ou implémente un comportement précis utilisé par ce module.
void EnemyCombatQueue::addWaitingEnemy(const Monster& monster)
{
    waitingEnemies.push_back(monster);
}

// EN: initializeFrontLine declares or implements a focused behavior used by this module.
// FR: initializeFrontLine déclare ou implémente un comportement précis utilisé par ce module.
void EnemyCombatQueue::initializeFrontLine()
{
    while (canAddActiveEnemy() && !waitingEnemies.empty())
    {
        bringNextEnemyIn();
    }
}

// EN: hasEnemiesLeft declares or implements a focused behavior used by this module.
// FR: hasEnemiesLeft déclare ou implémente un comportement précis utilisé par ce module.
bool EnemyCombatQueue::hasEnemiesLeft() const
{
    return !activeEnemies.empty() || !waitingEnemies.empty();
}

// EN: hasActiveEnemies declares or implements a focused behavior used by this module.
// FR: hasActiveEnemies déclare ou implémente un comportement précis utilisé par ce module.
bool EnemyCombatQueue::hasActiveEnemies() const
{
    return !activeEnemies.empty();
}

// EN: hasWaitingEnemies declares or implements a focused behavior used by this module.
// FR: hasWaitingEnemies déclare ou implémente un comportement précis utilisé par ce module.
bool EnemyCombatQueue::hasWaitingEnemies() const
{
    return !waitingEnemies.empty();
}

// EN: getActiveEnemyCount declares or implements a focused behavior used by this module.
// FR: getActiveEnemyCount déclare ou implémente un comportement précis utilisé par ce module.
int EnemyCombatQueue::getActiveEnemyCount() const
{
    return static_cast<int>(activeEnemies.size());
}

// EN: getWaitingEnemyCount declares or implements a focused behavior used by this module.
// FR: getWaitingEnemyCount déclare ou implémente un comportement précis utilisé par ce module.
int EnemyCombatQueue::getWaitingEnemyCount() const
{
    return static_cast<int>(waitingEnemies.size());
}

// EN: getTotalRemainingEnemyCount declares or implements a focused behavior used by this module.
// FR: getTotalRemainingEnemyCount déclare ou implémente un comportement précis utilisé par ce module.
int EnemyCombatQueue::getTotalRemainingEnemyCount() const
{
    return getActiveEnemyCount() + getWaitingEnemyCount();
}

// EN: getDefeatedEnemyCount declares or implements a focused behavior used by this module.
// FR: getDefeatedEnemyCount déclare ou implémente un comportement précis utilisé par ce module.
int EnemyCombatQueue::getDefeatedEnemyCount() const
{
    return static_cast<int>(defeatedEnemies.size());
}

// EN: getEscapedEnemyCount declares or implements a focused behavior used by this module.
// FR: getEscapedEnemyCount déclare ou implémente un comportement précis utilisé par ce module.
int EnemyCombatQueue::getEscapedEnemyCount() const
{
    return static_cast<int>(escapedEnemies.size());
}

// EN: getDamagedAliveEnemyCount declares or implements a focused behavior used by this module.
// FR: getDamagedAliveEnemyCount déclare ou implémente un comportement précis utilisé par ce module.
int EnemyCombatQueue::getDamagedAliveEnemyCount() const
{
    int total = 0;

    for (const Monster& monster : activeEnemies)
    {
        if (isDamagedAndAlive(monster))
        {
            ++total;
        }
    }

    return total;
}

// EN: isActiveIndexValid declares or implements a focused behavior used by this module.
// FR: isActiveIndexValid déclare ou implémente un comportement précis utilisé par ce module.
bool EnemyCombatQueue::isActiveIndexValid(int index) const
{
    return index >= 0 && index < getActiveEnemyCount();
}

// EN: getActiveEnemy declares or implements a focused behavior used by this module.
// FR: getActiveEnemy déclare ou implémente un comportement précis utilisé par ce module.
Monster& EnemyCombatQueue::getActiveEnemy(int index)
{
    return activeEnemies[index];
}

// EN: getActiveEnemy declares or implements a focused behavior used by this module.
// FR: getActiveEnemy déclare ou implémente un comportement précis utilisé par ce module.
const Monster& EnemyCombatQueue::getActiveEnemy(int index) const
{
    return activeEnemies[index];
}

// EN: getWaitingEnemy declares or implements a focused behavior used by this module.
// FR: getWaitingEnemy déclare ou implémente un comportement précis utilisé par ce module.
const Monster& EnemyCombatQueue::getWaitingEnemy(int index) const
{
    return waitingEnemies[index];
}

// EN: getDefeatedEnemy declares or implements a focused behavior used by this module.
// FR: getDefeatedEnemy déclare ou implémente un comportement précis utilisé par ce module.
const Monster& EnemyCombatQueue::getDefeatedEnemy(int index) const
{
    return defeatedEnemies[index];
}

// EN: getEscapedEnemy declares or implements a focused behavior used by this module.
// FR: getEscapedEnemy déclare ou implémente un comportement précis utilisé par ce module.
const Monster& EnemyCombatQueue::getEscapedEnemy(int index) const
{
    return escapedEnemies[index];
}

// EN: getDamagedAliveEnemy declares or implements a focused behavior used by this module.
// FR: getDamagedAliveEnemy déclare ou implémente un comportement précis utilisé par ce module.
const Monster& EnemyCombatQueue::getDamagedAliveEnemy(int index) const
{
    int found = 0;

    for (const Monster& monster : activeEnemies)
    {
        if (isDamagedAndAlive(monster))
        {
            if (found == index)
            {
                return monster;
            }

            ++found;
        }
    }

    return activeEnemies.front();
}

// EN: removeActiveEnemy declares or implements a focused behavior used by this module.
// FR: removeActiveEnemy déclare ou implémente un comportement précis utilisé par ce module.
void EnemyCombatQueue::removeActiveEnemy(int index)
{
    if (!isActiveIndexValid(index))
    {
        return;
    }

    if (activeEnemies[index].isDead())
    {
        defeatedEnemies.push_back(activeEnemies[index]);
    }

    activeEnemies.erase(activeEnemies.begin() + index);

    if (canAddActiveEnemy() && !waitingEnemies.empty())
    {
        bringNextEnemyIn();
    }
}

// EN: removeActiveEnemyAsEscaped declares or implements a focused behavior used by this module.
// FR: removeActiveEnemyAsEscaped déclare ou implémente un comportement précis utilisé par ce module.
void EnemyCombatQueue::removeActiveEnemyAsEscaped(int index)
{
    if (!isActiveIndexValid(index))
    {
        return;
    }

    escapedEnemies.push_back(activeEnemies[index]);
    activeEnemies.erase(activeEnemies.begin() + index);

    if (canAddActiveEnemy() && !waitingEnemies.empty())
    {
        bringNextEnemyIn();
    }
}

// EN: removeDeadAndReplace declares or implements a focused behavior used by this module.
// FR: removeDeadAndReplace déclare ou implémente un comportement précis utilisé par ce module.
void EnemyCombatQueue::removeDeadAndReplace()
{
    int i = 0;

    while (i < getActiveEnemyCount())
    {
        if (activeEnemies[i].isDead())
        {
            std::cout << activeEnemies[i].getName()
                      << " disparaît de la première ligne."
                      << std::endl;

            removeActiveEnemy(i);
            continue;
        }

        ++i;
    }
}

// EN: displayActiveEnemies declares or implements a focused behavior used by this module.
// FR: displayActiveEnemies déclare ou implémente un comportement précis utilisé par ce module.
void EnemyCombatQueue::displayActiveEnemies() const
{
    std::cout << "========== ENNEMIS ACTIFS ==========" << std::endl;

    if (activeEnemies.empty())
    {
        std::cout << "Aucun ennemi actif." << std::endl;
    }
    else
    {
        for (int i = 0; i < static_cast<int>(activeEnemies.size()); ++i)
        {
            const Monster& monster = activeEnemies[i];

            std::cout << i + 1
                      << " : "
                      << monster.getName()
                      << " | "
                      << monster.getHp()
                      << "/"
                      << monster.getMaxHp()
                      << " PV";

            if (monster.isEvolved())
            {
                std::cout << " | variation évoluée";
            }
            else if (monster.isElite())
            {
                std::cout << " | élite";
            }

            std::cout << std::endl;
        }
    }

    std::cout << "====================================" << std::endl;
    std::cout << std::endl;
}

// EN: displayQueueSummary declares or implements a focused behavior used by this module.
// FR: displayQueueSummary déclare ou implémente un comportement précis utilisé par ce module.
void EnemyCombatQueue::displayQueueSummary() const
{
    std::cout << "========== ÉTAT DE LA VAGUE ==========" << std::endl;
    std::cout << "Ennemis actifs : " << getActiveEnemyCount() << std::endl;
    std::cout << "Ennemis en attente : " << getWaitingEnemyCount() << std::endl;
    std::cout << "Ennemis blessés encore en vie : " << getDamagedAliveEnemyCount() << std::endl;
    std::cout << "Ennemis vaincus : " << getDefeatedEnemyCount() << std::endl;
    std::cout << "Ennemis en fuite : " << getEscapedEnemyCount() << std::endl;
    std::cout << "Total restant : " << getTotalRemainingEnemyCount() << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << std::endl;
}

// EN: canAddActiveEnemy declares or implements a focused behavior used by this module.
// FR: canAddActiveEnemy déclare ou implémente un comportement précis utilisé par ce module.
bool EnemyCombatQueue::canAddActiveEnemy() const
{
    return getActiveEnemyCount() < MAX_ACTIVE_ENEMIES;
}

// EN: bringNextEnemyIn declares or implements a focused behavior used by this module.
// FR: bringNextEnemyIn déclare ou implémente un comportement précis utilisé par ce module.
void EnemyCombatQueue::bringNextEnemyIn()
{
    if (waitingEnemies.empty())
    {
        return;
    }

    Monster next = waitingEnemies.front();
    waitingEnemies.erase(waitingEnemies.begin());

    activeEnemies.push_back(next);

    std::cout << next.getName();

    if (next.isEvolved())
    {
        std::cout << " [variation évoluée]";
    }

    std::cout << " entre dans la première ligne."
              << std::endl;
    std::cout << std::endl;
}

// EN: isDamagedAndAlive declares or implements a focused behavior used by this module.
// FR: isDamagedAndAlive déclare ou implémente un comportement précis utilisé par ce module.
bool EnemyCombatQueue::isDamagedAndAlive(const Monster& monster) const
{
    return !monster.isDead() && monster.getHp() < monster.getMaxHp();
}
