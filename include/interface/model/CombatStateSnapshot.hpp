// EN: CombatStateSnapshot.hpp defines a GUI-ready combat state snapshot.
// FR: CombatStateSnapshot.hpp définit un instantané de combat prêt pour la future IG.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_INTERFACE_MODEL_COMBATSTATESNAPSHOT_HPP
#define INCLUDE_INTERFACE_MODEL_COMBATSTATESNAPSHOT_HPP

#include "combat/group/CombatGroup.hpp"
#include "combat/summon/Summon.hpp"
#include "entity/Entity.hpp"

#include <string>
#include <vector>

struct GuiCombatUnitSnapshot
{
    std::string unitId;
    std::string name;
    std::string type;
    std::string side;
    std::string kind;
    int hp = 0;
    int maxHp = 0;
    int minDamage = 0;
    int maxDamage = 0;
    int criticalDamage = 0;
    int healingPotionCount = 0;
    int damagePotionCount = 0;
    bool dead = false;
    bool targetable = false;
    bool healingThreatMarked = false;
    bool provoking = false;
    bool defensePostureActive = false;
    std::string defensePostureLabel;
    std::vector<std::string> statusLabels;
};

struct GuiCombatStateSnapshot
{
    std::string screenId;
    std::string title;
    std::string phase;
    int turnNumber = 0;
    std::vector<GuiCombatUnitSnapshot> playerUnits;
    std::vector<GuiCombatUnitSnapshot> enemyUnits;
    std::vector<GuiCombatUnitSnapshot> neutralUnits;
    std::vector<std::string> logLines;
};

class CombatStateSnapshot
{
public:
    static GuiCombatUnitSnapshot fromEntity(
        const Entity& entity,
        const std::string& side,
        const std::string& kind,
        bool targetable = true
    );

    static GuiCombatUnitSnapshot fromSummon(
        const Summon& summon,
        const std::string& side,
        bool targetable = true
    );

    static GuiCombatStateSnapshot fromDuel(
        const Entity& playerSideEntity,
        const Entity& enemySideEntity,
        const std::string& title,
        const std::string& phase,
        int turnNumber = 0
    );

    static GuiCombatStateSnapshot fromGroups(
        const CombatGroup& playerGroup,
        const CombatGroup& enemyGroup,
        const std::string& title,
        const std::string& phase,
        int turnNumber = 0
    );

    static std::vector<std::string> toDisplayLines(const GuiCombatStateSnapshot& snapshot);
    static std::string healthText(const GuiCombatUnitSnapshot& unit);

private:
    static std::vector<std::string> statusLabelsForEntity(const Entity& entity);
    static std::string sideToText(CombatSide side);
    static std::string kindToText(CombatUnitKind kind);
    static void addUnitToSnapshot(GuiCombatStateSnapshot& snapshot, const GuiCombatUnitSnapshot& unit);
};

#endif
