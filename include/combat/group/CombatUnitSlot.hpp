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

    int getSlotIndex() const;
    CombatSide getSide() const;
    CombatUnitKind getKind() const;

    bool isEmpty() const;
    bool isAlive() const;
    bool isTargetable() const;

    std::string getDisplayName() const;
    Entity* getEntity();
    Summon* getSummon();
};

#endif
