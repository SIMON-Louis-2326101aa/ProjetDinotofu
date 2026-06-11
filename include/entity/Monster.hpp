// EN: Monster.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Monster.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ENTITY_MONSTER_HPP
#define INCLUDE_ENTITY_MONSTER_HPP

#include "entity/Entity.hpp"
#include "entity/Race.hpp"

#include <cstddef>
#include <string>
#include <vector>

class Monster : public Entity
{
private:
    int level;
    Race race;

    bool invocation;
    bool elite;
    bool hiddenStats;
    bool evolved;

    bool splitsOnDeath;
    int splitMinCount;
    int splitMaxCount;
    std::string splitChildName;
    std::vector<std::string> splitStageNames;
    std::size_t splitStageIndex;
    bool finalSplitChildrenAreInvocations;

public:
    // EN: Monster declares or implements a focused behavior used by this module.
    // FR: Monster déclare ou implémente un comportement précis utilisé par ce module.
    Monster();

    Monster(
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
        bool invocation = false,
        bool elite = false,
        bool hiddenStats = false,
        bool evolved = false
    );

    // EN: getLevel declares or implements a focused behavior used by this module.
    // FR: getLevel déclare ou implémente un comportement précis utilisé par ce module.
    int getLevel() const;
    // EN: getRace declares or implements a focused behavior used by this module.
    // FR: getRace déclare ou implémente un comportement précis utilisé par ce module.
    Race getRace() const;
    std::string getRaceText() const;

    // EN: isInvocation declares or implements a focused behavior used by this module.
    // FR: isInvocation déclare ou implémente un comportement précis utilisé par ce module.
    bool isInvocation() const;
    // EN: isElite declares or implements a focused behavior used by this module.
    // FR: isElite déclare ou implémente un comportement précis utilisé par ce module.
    bool isElite() const;
    // EN: isEvolved declares or implements a focused behavior used by this module.
    // FR: isEvolved déclare ou implémente un comportement précis utilisé par ce module.
    bool isEvolved() const;

    bool doesSplitOnDeath() const;
    int getSplitMinCount() const;
    int getSplitMaxCount() const;
    std::string getSplitChildName() const;
    void configureSplitOnDeath(int minCount, int maxCount, const std::string& childName);
    void configureSplitTree(
        int childrenPerStage,
        const std::vector<std::string>& stageNames,
        bool finalChildrenAreInvocations = true
    );
    void copySplitBehaviorFrom(const Monster& source);
    int getSplitStagesRemaining() const;
    bool splitChildrenWillBeInvocations() const;
    Monster createSplitChild(int childIndex) const;

    // EN: areStatsVisible declares or implements a focused behavior used by this module.
    // FR: areStatsVisible déclare ou implémente un comportement précis utilisé par ce module.
    bool areStatsVisible() const override;
    // EN: revealStats declares or implements a focused behavior used by this module.
    // FR: revealStats déclare ou implémente un comportement précis utilisé par ce module.
    void revealStats();

    std::vector<std::string> toDisplayLines() const;

    // EN: displayStats declares or implements a focused behavior used by this module.
    // FR: displayStats déclare ou implémente un comportement précis utilisé par ce module.
    void displayStats() const override;
};

#endif
