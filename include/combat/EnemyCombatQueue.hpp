// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_ENEMYCOMBATQUEUE_HPP
#define INCLUDE_COMBAT_ENEMYCOMBATQUEUE_HPP

#include "entity/Monster.hpp"

#include <vector>

class EnemyCombatQueue
{
public:
    static constexpr int MAX_ACTIVE_ENEMIES = 3;

    EnemyCombatQueue();

    void addWaitingEnemy(const Monster& monster);
    void initializeFrontLine();

    bool hasEnemiesLeft() const;
    bool hasActiveEnemies() const;
    bool hasWaitingEnemies() const;

    int getActiveEnemyCount() const;
    int getWaitingEnemyCount() const;
    int getTotalRemainingEnemyCount() const;

    bool isActiveIndexValid(int index) const;

    Monster& getActiveEnemy(int index);
    const Monster& getActiveEnemy(int index) const;

    void removeActiveEnemy(int index);
    void removeDeadAndReplace();

    void displayActiveEnemies() const;
    void displayQueueSummary() const;

private:
    std::vector<Monster> activeEnemies;
    std::vector<Monster> waitingEnemies;

    bool canAddActiveEnemy() const;
    void bringNextEnemyIn();
};

#endif
