// EN: EnemyCombatQueue.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: EnemyCombatQueue.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_ENEMYCOMBATQUEUE_HPP
#define INCLUDE_COMBAT_ENEMYCOMBATQUEUE_HPP

#include "entity/Monster.hpp"

#include <string>
#include <vector>

class EnemyCombatQueue
{
public:
    static constexpr int MAX_ACTIVE_ENEMIES = 3;

    // EN: EnemyCombatQueue declares or implements a focused behavior used by this module.
    // FR: EnemyCombatQueue déclare ou implémente un comportement précis utilisé par ce module.
    EnemyCombatQueue();

    // EN: addWaitingEnemy declares or implements a focused behavior used by this module.
    // FR: addWaitingEnemy déclare ou implémente un comportement précis utilisé par ce module.
    void addWaitingEnemy(const Monster& monster);
    // EN: initializeFrontLine declares or implements a focused behavior used by this module.
    // FR: initializeFrontLine déclare ou implémente un comportement précis utilisé par ce module.
    void initializeFrontLine();

    // EN: hasEnemiesLeft declares or implements a focused behavior used by this module.
    // FR: hasEnemiesLeft déclare ou implémente un comportement précis utilisé par ce module.
    bool hasEnemiesLeft() const;
    // EN: hasActiveEnemies declares or implements a focused behavior used by this module.
    // FR: hasActiveEnemies déclare ou implémente un comportement précis utilisé par ce module.
    bool hasActiveEnemies() const;
    // EN: hasWaitingEnemies declares or implements a focused behavior used by this module.
    // FR: hasWaitingEnemies déclare ou implémente un comportement précis utilisé par ce module.
    bool hasWaitingEnemies() const;

    // EN: getActiveEnemyCount declares or implements a focused behavior used by this module.
    // FR: getActiveEnemyCount déclare ou implémente un comportement précis utilisé par ce module.
    int getActiveEnemyCount() const;
    // EN: getWaitingEnemyCount declares or implements a focused behavior used by this module.
    // FR: getWaitingEnemyCount déclare ou implémente un comportement précis utilisé par ce module.
    int getWaitingEnemyCount() const;
    // EN: getTotalRemainingEnemyCount declares or implements a focused behavior used by this module.
    // FR: getTotalRemainingEnemyCount déclare ou implémente un comportement précis utilisé par ce module.
    int getTotalRemainingEnemyCount() const;

    // EN: getDefeatedEnemyCount declares or implements a focused behavior used by this module.
    // FR: getDefeatedEnemyCount déclare ou implémente un comportement précis utilisé par ce module.
    int getDefeatedEnemyCount() const;
    // EN: getEscapedEnemyCount declares or implements a focused behavior used by this module.
    // FR: getEscapedEnemyCount déclare ou implémente un comportement précis utilisé par ce module.
    int getEscapedEnemyCount() const;
    // EN: getDamagedAliveEnemyCount declares or implements a focused behavior used by this module.
    // FR: getDamagedAliveEnemyCount déclare ou implémente un comportement précis utilisé par ce module.
    int getDamagedAliveEnemyCount() const;

    // EN: isActiveIndexValid declares or implements a focused behavior used by this module.
    // FR: isActiveIndexValid déclare ou implémente un comportement précis utilisé par ce module.
    bool isActiveIndexValid(int index) const;

    // EN: getActiveEnemy declares or implements a focused behavior used by this module.
    // FR: getActiveEnemy déclare ou implémente un comportement précis utilisé par ce module.
    Monster& getActiveEnemy(int index);
    const Monster& getActiveEnemy(int index) const;
    // EN: getWaitingEnemy declares or implements a focused behavior used by this module.
    // FR: getWaitingEnemy déclare ou implémente un comportement précis utilisé par ce module.
    const Monster& getWaitingEnemy(int index) const;

    // EN: getDefeatedEnemy declares or implements a focused behavior used by this module.
    // FR: getDefeatedEnemy déclare ou implémente un comportement précis utilisé par ce module.
    const Monster& getDefeatedEnemy(int index) const;
    // EN: getEscapedEnemy declares or implements a focused behavior used by this module.
    // FR: getEscapedEnemy déclare ou implémente un comportement précis utilisé par ce module.
    const Monster& getEscapedEnemy(int index) const;
    // EN: getDamagedAliveEnemy declares or implements a focused behavior used by this module.
    // FR: getDamagedAliveEnemy déclare ou implémente un comportement précis utilisé par ce module.
    const Monster& getDamagedAliveEnemy(int index) const;

    // EN: removeActiveEnemy declares or implements a focused behavior used by this module.
    // FR: removeActiveEnemy déclare ou implémente un comportement précis utilisé par ce module.
    void removeActiveEnemy(int index);
    // EN: removeActiveEnemyAsEscaped declares or implements a focused behavior used by this module.
    // FR: removeActiveEnemyAsEscaped déclare ou implémente un comportement précis utilisé par ce module.
    void removeActiveEnemyAsEscaped(int index);
    // EN: removeDeadAndReplace declares or implements a focused behavior used by this module.
    // FR: removeDeadAndReplace déclare ou implémente un comportement précis utilisé par ce module.
    void removeDeadAndReplace();

    // EN: displayActiveEnemies declares or implements a focused behavior used by this module.
    // FR: displayActiveEnemies déclare ou implémente un comportement précis utilisé par ce module.
    std::vector<std::string> getActiveEnemyDisplayLines() const;
    std::vector<std::string> getQueueSummaryLines() const;
    void displayActiveEnemies() const;
    // EN: displayQueueSummary declares or implements a focused behavior used by this module.
    // FR: displayQueueSummary déclare ou implémente un comportement précis utilisé par ce module.
    void displayQueueSummary() const;

private:
    std::vector<Monster> activeEnemies;
    std::vector<Monster> waitingEnemies;
    std::vector<Monster> defeatedEnemies;
    std::vector<Monster> escapedEnemies;

    // EN: canAddActiveEnemy declares or implements a focused behavior used by this module.
    // FR: canAddActiveEnemy déclare ou implémente un comportement précis utilisé par ce module.
    bool canAddActiveEnemy() const;
    // EN: bringNextEnemyIn declares or implements a focused behavior used by this module.
    // FR: bringNextEnemyIn déclare ou implémente un comportement précis utilisé par ce module.
    void bringNextEnemyIn();

    // EN: isDamagedAndAlive declares or implements a focused behavior used by this module.
    // FR: isDamagedAndAlive déclare ou implémente un comportement précis utilisé par ce module.
    bool isDamagedAndAlive(const Monster& monster) const;
};

#endif
