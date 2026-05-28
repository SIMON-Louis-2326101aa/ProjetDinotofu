// EN: EnemyCombatQueue.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: EnemyCombatQueue.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/EnemyCombatQueue.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <sstream>

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
            const std::string enemyName = activeEnemies[i].getName();

            MessageScreen::show(
                "LIGNE ENNEMIE",
                "combat.wave.enemy_removed",
                { enemyName + " disparaît de la première ligne." },
                false
            );

            removeActiveEnemy(i);
            continue;
        }

        ++i;
    }
}

// EN: getActiveEnemyDisplayLines returns active enemies as reusable interface lines.
// FR: getActiveEnemyDisplayLines renvoie les ennemis actifs sous forme de lignes réutilisables par l'interface.
std::vector<std::string> EnemyCombatQueue::getActiveEnemyDisplayLines() const
{
    std::vector<std::string> lines;

    if (activeEnemies.empty())
    {
        lines.push_back("Aucun ennemi actif.");
        return lines;
    }

    for (int i = 0; i < static_cast<int>(activeEnemies.size()); ++i)
    {
        const Monster& monster = activeEnemies[i];
        std::ostringstream line;
        line << (i + 1)
             << " : "
             << monster.getName()
             << " | "
             << monster.getHp()
             << "/"
             << monster.getMaxHp()
             << " PV";

        if (monster.isEvolved())
        {
            line << " | variation évoluée";
        }
        else if (monster.isElite())
        {
            line << " | élite";
        }

        if (monster.isProvoking())
        {
            line << " | provocation";
        }

        if (monster.hasHealingThreat())
        {
            line << " | soigneur marqué";
        }

        lines.push_back(line.str());
    }

    return lines;
}

// EN: getQueueSummaryLines returns wave information without forcing terminal rendering.
// FR: getQueueSummaryLines renvoie les informations de vague sans imposer le rendu terminal.
std::vector<std::string> EnemyCombatQueue::getQueueSummaryLines() const
{
    return {
        "Ennemis actifs : " + std::to_string(getActiveEnemyCount()),
        "Ennemis en attente : " + std::to_string(getWaitingEnemyCount()),
        "Ennemis blessés encore en vie : " + std::to_string(getDamagedAliveEnemyCount()),
        "Ennemis vaincus : " + std::to_string(getDefeatedEnemyCount()),
        "Ennemis en fuite : " + std::to_string(getEscapedEnemyCount()),
        "Total restant : " + std::to_string(getTotalRemainingEnemyCount())
    };
}

// EN: displayActiveEnemies declares or implements a focused behavior used by this module.
// FR: displayActiveEnemies déclare ou implémente un comportement précis utilisé par ce module.
void EnemyCombatQueue::displayActiveEnemies() const
{
    MessageScreen::show(
        "ENNEMIS ACTIFS",
        "combat.wave.active_enemies",
        getActiveEnemyDisplayLines(),
        false
    );
}

// EN: displayQueueSummary declares or implements a focused behavior used by this module.
// FR: displayQueueSummary déclare ou implémente un comportement précis utilisé par ce module.
void EnemyCombatQueue::displayQueueSummary() const
{
    MessageScreen::show(
        "ÉTAT DE LA VAGUE",
        "combat.wave.summary",
        getQueueSummaryLines(),
        false
    );
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

    std::string line = next.getName();

    if (next.isEvolved())
    {
        line += " [variation évoluée]";
    }

    line += " entre dans la première ligne.";

    MessageScreen::show(
        "RENFORT ENNEMI",
        "combat.wave.enemy_reinforcement",
        { line },
        false
    );
}

// EN: isDamagedAndAlive declares or implements a focused behavior used by this module.
// FR: isDamagedAndAlive déclare ou implémente un comportement précis utilisé par ce module.
bool EnemyCombatQueue::isDamagedAndAlive(const Monster& monster) const
{
    return !monster.isDead() && monster.getHp() < monster.getMaxHp();
}
