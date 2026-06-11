// EN: Monster.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Monster.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/Monster.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <algorithm>
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
      evolved(false),
      splitsOnDeath(false),
      splitMinCount(0),
      splitMaxCount(0),
      splitChildName(""),
      splitStageNames(),
      splitStageIndex(0),
      finalSplitChildrenAreInvocations(false)
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
      evolved(evolved),
      splitsOnDeath(false),
      splitMinCount(0),
      splitMaxCount(0),
      splitChildName(""),
      splitStageNames(),
      splitStageIndex(0),
      finalSplitChildrenAreInvocations(false)
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

bool Monster::doesSplitOnDeath() const
{
    return splitsOnDeath && splitMinCount > 0 && splitMaxCount >= splitMinCount;
}

int Monster::getSplitMinCount() const
{
    return splitMinCount;
}

int Monster::getSplitMaxCount() const
{
    return splitMaxCount;
}

std::string Monster::getSplitChildName() const
{
    return splitChildName;
}

void Monster::configureSplitOnDeath(int minCount, int maxCount, const std::string& childName)
{
    if (minCount < 1 || maxCount < minCount || childName.empty())
    {
        splitsOnDeath = false;
        splitMinCount = 0;
        splitMaxCount = 0;
        splitChildName.clear();
        splitStageNames.clear();
        splitStageIndex = 0;
        finalSplitChildrenAreInvocations = false;
        return;
    }

    splitsOnDeath = true;
    splitMinCount = minCount;
    splitMaxCount = maxCount;
    splitChildName = childName;
    splitStageNames.clear();
    splitStageIndex = 0;
    finalSplitChildrenAreInvocations = false;
}

void Monster::configureSplitTree(
    int childrenPerStage,
    const std::vector<std::string>& stageNames,
    bool finalChildrenAreInvocations
)
{
    if (childrenPerStage < 1 || stageNames.empty())
    {
        configureSplitOnDeath(0, 0, "");
        return;
    }

    splitsOnDeath = true;
    splitMinCount = childrenPerStage;
    splitMaxCount = childrenPerStage;
    splitStageNames = stageNames;
    splitStageIndex = 0;
    splitChildName = splitStageNames.front();
    finalSplitChildrenAreInvocations = finalChildrenAreInvocations;
}

void Monster::copySplitBehaviorFrom(const Monster& source)
{
    splitsOnDeath = source.splitsOnDeath;
    splitMinCount = source.splitMinCount;
    splitMaxCount = source.splitMaxCount;
    splitChildName = source.splitChildName;
    splitStageNames = source.splitStageNames;
    splitStageIndex = source.splitStageIndex;
    finalSplitChildrenAreInvocations = source.finalSplitChildrenAreInvocations;
}

int Monster::getSplitStagesRemaining() const
{
    if (splitStageNames.empty() || splitStageIndex >= splitStageNames.size())
    {
        return doesSplitOnDeath() ? 1 : 0;
    }

    return static_cast<int>(splitStageNames.size() - splitStageIndex);
}

bool Monster::splitChildrenWillBeInvocations() const
{
    return finalSplitChildrenAreInvocations
        && !splitStageNames.empty()
        && splitStageIndex + 1 >= splitStageNames.size();
}

Monster Monster::createSplitChild(int childIndex) const
{
    const bool hierarchicalSplit = !splitStageNames.empty() && splitStageIndex < splitStageNames.size();
    const bool finalStage = hierarchicalSplit && splitStageIndex + 1 >= splitStageNames.size();
    const bool childIsInvocation = finalStage && finalSplitChildrenAreInvocations;
    const std::string childBaseName = hierarchicalSplit ? splitStageNames[splitStageIndex] : splitChildName;

    const int childLevel = std::max(1, level - (hierarchicalSplit ? 1 : 2));
    const int hpPercent = hierarchicalSplit ? 42 : 16;
    const int damagePercent = hierarchicalSplit ? (childIsInvocation ? 24 : 38) : 35;
    const int criticalPercent = hierarchicalSplit ? (childIsInvocation ? 28 : 42) : 40;
    const int childMaxHp = std::max(childIsInvocation ? 8 : 12, maxHp * hpPercent / 100);
    const int childMinDamage = std::max(1, minDamage * damagePercent / 100);
    const int childMaxDamage = std::max(childMinDamage, maxDamage * damagePercent / 100);
    const int childCriticalDamage = std::max(childMaxDamage, criticalDamage * criticalPercent / 100);

    Monster child(
        childBaseName + " " + std::to_string(std::max(1, childIndex)),
        childIsInvocation
            ? "Invocation gélatineuse née du dernier noyau"
            : (hierarchicalSplit ? "Sous-forme d'un slime à divisions successives" : "Petit slime issu d'une fusion"),
        childIsInvocation ? Race::Esprit : Race::Slime,
        childLevel,
        childMaxHp,
        childMinDamage,
        childMaxDamage,
        childCriticalDamage,
        0,
        0,
        childIsInvocation,
        false,
        false,
        false
    );

    if (hierarchicalSplit && !finalStage)
    {
        child.splitsOnDeath = true;
        child.splitMinCount = splitMinCount;
        child.splitMaxCount = splitMaxCount;
        child.splitStageNames = splitStageNames;
        child.splitStageIndex = splitStageIndex + 1;
        child.splitChildName = child.splitStageNames[child.splitStageIndex];
        child.finalSplitChildrenAreInvocations = finalSplitChildrenAreInvocations;
    }

    return child;
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

    if (doesSplitOnDeath())
    {
        if (!splitStageNames.empty())
        {
            lines.push_back(
                "Mort spéciale : division en "
                + std::to_string(splitMinCount)
                + " sous-formes | étages restants : "
                + std::to_string(getSplitStagesRemaining())
                + (splitChildrenWillBeInvocations()
                    ? " | prochaine forme : invocation"
                    : "")
            );
        }
        else
        {
            lines.push_back(
                "Mort spéciale : se sépare en "
                + std::to_string(splitMinCount)
                + " à "
                + std::to_string(splitMaxCount)
                + " petits slimes."
            );
        }
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
