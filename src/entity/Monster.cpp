// EN: Monster.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Monster.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/Monster.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <vector>
#include <string>

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
std::vector<std::string> Monster::toDisplayLines() const
{
    std::vector<std::string> lines;

    if (!areStatsVisible())
    {
        lines = {
            "========== DONNÉES BROUILLÉES ==========" ,
            "Nom : " + name,
            "Race : " + getRaceText(),
            "Type : " + type,
            "Niveau : ???",
            "PV : ???",
            "Dégâts : ???",
            "Critique : ???",
            "========================================"
        };

        return lines;
    }

    lines = {
        "========== STATS MONSTRE ==========" ,
        "Nom : " + name,
        "Race : " + getRaceText(),
        "Type : " + type,
        "Niveau : " + std::to_string(level),
        "PV : " + std::to_string(hp) + "/" + std::to_string(maxHp),
        "Dégâts : " + std::to_string(minDamage) + " - " + std::to_string(maxDamage),
        "Critique : " + std::to_string(criticalDamage)
    };

    if (invocation)
    {
        lines.push_back("Statut : Invocation");
    }
    else if (elite && evolved)
    {
        lines.push_back("Statut : Élite évoluée");
    }
    else if (elite)
    {
        lines.push_back("Statut : Élite");
    }
    else if (evolved)
    {
        lines.push_back("Statut : Créature évoluée");
    }
    else
    {
        lines.push_back("Statut : Monstre standard");
    }

    lines.push_back("===================================");

    return lines;
}


// EN: displayStats declares or implements a focused behavior used by this module.
// FR: displayStats déclare ou implémente un comportement précis utilisé par ce module.
void Monster::displayStats() const
{
    MessageScreen::show(
        "MONSTRE",
        areStatsVisible() ? "monster.stats" : "monster.stats.hidden",
        toDisplayLines(),
        false
    );
}
