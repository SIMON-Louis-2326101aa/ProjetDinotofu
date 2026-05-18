// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/EnemyCombatQueue.hpp"

#include <iostream>

EnemyCombatQueue::EnemyCombatQueue()
{
}

void EnemyCombatQueue::addWaitingEnemy(const Monster& monster)
{
    waitingEnemies.push_back(monster);
}

void EnemyCombatQueue::initializeFrontLine()
{
    while (canAddActiveEnemy() && !waitingEnemies.empty())
    {
        bringNextEnemyIn();
    }
}

bool EnemyCombatQueue::hasEnemiesLeft() const
{
    return !activeEnemies.empty() || !waitingEnemies.empty();
}

bool EnemyCombatQueue::hasActiveEnemies() const
{
    return !activeEnemies.empty();
}

bool EnemyCombatQueue::hasWaitingEnemies() const
{
    return !waitingEnemies.empty();
}

int EnemyCombatQueue::getActiveEnemyCount() const
{
    return static_cast<int>(activeEnemies.size());
}

int EnemyCombatQueue::getWaitingEnemyCount() const
{
    return static_cast<int>(waitingEnemies.size());
}

int EnemyCombatQueue::getTotalRemainingEnemyCount() const
{
    return getActiveEnemyCount() + getWaitingEnemyCount();
}

int EnemyCombatQueue::getDefeatedEnemyCount() const
{
    return static_cast<int>(defeatedEnemies.size());
}

int EnemyCombatQueue::getEscapedEnemyCount() const
{
    return static_cast<int>(escapedEnemies.size());
}

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

bool EnemyCombatQueue::isActiveIndexValid(int index) const
{
    return index >= 0 && index < getActiveEnemyCount();
}

Monster& EnemyCombatQueue::getActiveEnemy(int index)
{
    return activeEnemies[index];
}

const Monster& EnemyCombatQueue::getActiveEnemy(int index) const
{
    return activeEnemies[index];
}

const Monster& EnemyCombatQueue::getDefeatedEnemy(int index) const
{
    return defeatedEnemies[index];
}

const Monster& EnemyCombatQueue::getEscapedEnemy(int index) const
{
    return escapedEnemies[index];
}

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
                      << " PV"
                      << std::endl;
        }
    }

    std::cout << "====================================" << std::endl;
    std::cout << std::endl;
}

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

bool EnemyCombatQueue::canAddActiveEnemy() const
{
    return getActiveEnemyCount() < MAX_ACTIVE_ENEMIES;
}

void EnemyCombatQueue::bringNextEnemyIn()
{
    if (waitingEnemies.empty())
    {
        return;
    }

    Monster next = waitingEnemies.front();
    waitingEnemies.erase(waitingEnemies.begin());

    activeEnemies.push_back(next);

    std::cout << next.getName()
              << " entre dans la première ligne."
              << std::endl;
    std::cout << std::endl;
}

bool EnemyCombatQueue::isDamagedAndAlive(const Monster& monster) const
{
    return !monster.isDead() && monster.getHp() < monster.getMaxHp();
}
