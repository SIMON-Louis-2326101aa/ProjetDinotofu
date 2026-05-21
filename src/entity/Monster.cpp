// EN: Monster.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Monster.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/Monster.hpp"

#include <iostream>

Monster::Monster()
    : Entity(),
      level(1),
      race(Race::Unknown),
      invocation(false),
      elite(false),
      hiddenStats(false),
      // EN: evolved declares or implements a focused behavior used by this module.
      // FR: evolved déclare ou implémente un comportement précis utilisé par ce module.
      evolved(false)
{
}

Monster::Monster(
    const std::string& name,
    const std::string& type,
    Race race,
    int level,
    int maxHp,
    int minDamage,
    int maxDamage,
    int criticalDamage,
    int healingPotionCount,
    int damagePotionCount,
    bool invocation,
    bool elite,
    bool hiddenStats,
    bool evolved
)
    : Entity(
          name,
          type,
          maxHp,
          minDamage,
          maxDamage,
          criticalDamage,
          healingPotionCount,
          damagePotionCount
      ),
      level(level),
      race(race),
      invocation(invocation),
      elite(elite),
      hiddenStats(hiddenStats),
      // EN: evolved declares or implements a focused behavior used by this module.
      // FR: evolved déclare ou implémente un comportement précis utilisé par ce module.
      evolved(evolved)
{
}

// EN: getLevel declares or implements a focused behavior used by this module.
// FR: getLevel déclare ou implémente un comportement précis utilisé par ce module.
int Monster::getLevel() const
{
    return level;
}

// EN: getRace declares or implements a focused behavior used by this module.
// FR: getRace déclare ou implémente un comportement précis utilisé par ce module.
Race Monster::getRace() const
{
    return race;
}

std::string Monster::getRaceText() const
{
    return raceVersTexte(race);
}

// EN: isInvocation declares or implements a focused behavior used by this module.
// FR: isInvocation déclare ou implémente un comportement précis utilisé par ce module.
bool Monster::isInvocation() const
{
    return invocation;
}

// EN: isElite declares or implements a focused behavior used by this module.
// FR: isElite déclare ou implémente un comportement précis utilisé par ce module.
bool Monster::isElite() const
{
    return elite;
}

// EN: isEvolved declares or implements a focused behavior used by this module.
// FR: isEvolved déclare ou implémente un comportement précis utilisé par ce module.
bool Monster::isEvolved() const
{
    return evolved;
}

// EN: areStatsVisible declares or implements a focused behavior used by this module.
// FR: areStatsVisible déclare ou implémente un comportement précis utilisé par ce module.
bool Monster::areStatsVisible() const
{
    return !hiddenStats;
}

// EN: revealStats declares or implements a focused behavior used by this module.
// FR: revealStats déclare ou implémente un comportement précis utilisé par ce module.
void Monster::revealStats()
{
    hiddenStats = false;
}

// EN: displayStats declares or implements a focused behavior used by this module.
// FR: displayStats déclare ou implémente un comportement précis utilisé par ce module.
void Monster::displayStats() const
{
    if (!areStatsVisible())
    {
        std::cout << "========== DONNÉES BROUILLÉES ==========" << std::endl;
        std::cout << "Nom : " << name << std::endl;
        std::cout << "Race : " << getRaceText() << std::endl;
        std::cout << "Type : " << type << std::endl;
        std::cout << "Niveau : ???" << std::endl;
        std::cout << "PV : ???" << std::endl;
        std::cout << "Dégâts : ???" << std::endl;
        std::cout << "Critique : ???" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;
        return;
    }

    std::cout << "========== STATS MONSTRE ==========" << std::endl;
    std::cout << "Nom : " << name << std::endl;
    std::cout << "Race : " << getRaceText() << std::endl;
    std::cout << "Type : " << type << std::endl;
    std::cout << "Niveau : " << level << std::endl;
    std::cout << "PV : " << hp << "/" << maxHp << std::endl;
    std::cout << "Dégâts : " << minDamage << " - " << maxDamage << std::endl;
    std::cout << "Critique : " << criticalDamage << std::endl;

    if (invocation)
    {
        std::cout << "Statut : Invocation" << std::endl;
    }
    else if (elite && evolved)
    {
        std::cout << "Statut : Élite évoluée" << std::endl;
    }
    else if (elite)
    {
        std::cout << "Statut : Élite" << std::endl;
    }
    else if (evolved)
    {
        std::cout << "Statut : Créature évoluée" << std::endl;
    }
    else
    {
        std::cout << "Statut : Monstre standard" << std::endl;
    }

    std::cout << "===================================" << std::endl;
    std::cout << std::endl;
}
