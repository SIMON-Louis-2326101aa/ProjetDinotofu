// EN: CombatUnitSlot.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatUnitSlot.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Represents one visible active slot in future group fights: fighter, ally, enemy, summon, or boss.

#ifndef INCLUDE_COMBAT_GROUP_COMBATUNITSLOT_HPP
#define INCLUDE_COMBAT_GROUP_COMBATUNITSLOT_HPP

#include "combat/group/CombatSide.hpp"
#include "combat/group/CombatUnitKind.hpp"

#include <string>

class Entity;
class Summon;

class CombatUnitSlot
{
private:
    int slotIndex;
    CombatSide side;
    CombatUnitKind kind;
    Entity* entity;
    Summon* summon;

public:
    // EN: CombatUnitSlot declares or implements a focused behavior used by this module.
    // FR: CombatUnitSlot déclare ou implémente un comportement précis utilisé par ce module.
    CombatUnitSlot();

    static CombatUnitSlot createEntitySlot(
        int slotIndex,
        CombatSide side,
        CombatUnitKind kind,
        Entity& entity
    );

    static CombatUnitSlot createSummonSlot(
        int slotIndex,
        CombatSide side,
        Summon& summon
    );

    // EN: getSlotIndex declares or implements a focused behavior used by this module.
    // FR: getSlotIndex déclare ou implémente un comportement précis utilisé par ce module.
    int getSlotIndex() const;
    // EN: getSide declares or implements a focused behavior used by this module.
    // FR: getSide déclare ou implémente un comportement précis utilisé par ce module.
    CombatSide getSide() const;
    // EN: getKind declares or implements a focused behavior used by this module.
    // FR: getKind déclare ou implémente un comportement précis utilisé par ce module.
    CombatUnitKind getKind() const;

    // EN: isEmpty declares or implements a focused behavior used by this module.
    // FR: isEmpty déclare ou implémente un comportement précis utilisé par ce module.
    bool isEmpty() const;
    // EN: isAlive declares or implements a focused behavior used by this module.
    // FR: isAlive déclare ou implémente un comportement précis utilisé par ce module.
    bool isAlive() const;
    // EN: isTargetable declares or implements a focused behavior used by this module.
    // FR: isTargetable déclare ou implémente un comportement précis utilisé par ce module.
    bool isTargetable() const;

    std::string getDisplayName() const;
    // EN: getEntity declares or implements a focused behavior used by this module.
    // FR: getEntity déclare ou implémente un comportement précis utilisé par ce module.
    Entity* getEntity();
    const Entity* getEntity() const;
    // EN: getSummon declares or implements a focused behavior used by this module.
    // FR: getSummon déclare ou implémente un comportement précis utilisé par ce module.
    Summon* getSummon();
    const Summon* getSummon() const;
};

#endif
