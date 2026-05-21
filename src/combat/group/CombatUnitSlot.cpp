// EN: CombatUnitSlot.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatUnitSlot.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Represents one visible active slot in future group fights: fighter, ally, enemy, summon, or boss.

#include "combat/group/CombatUnitSlot.hpp"

#include "combat/summon/Summon.hpp"
#include "entity/Entity.hpp"

// EN: CombatUnitSlot declares or implements a focused behavior used by this module.
// FR: CombatUnitSlot déclare ou implémente un comportement précis utilisé par ce module.
CombatUnitSlot::CombatUnitSlot()
{
    slotIndex = -1;
    side = CombatSide::Neutral;
    kind = CombatUnitKind::Empty;
    entity = nullptr;
    summon = nullptr;
}

CombatUnitSlot CombatUnitSlot::createEntitySlot(
    int slotIndex,
    CombatSide side,
    CombatUnitKind kind,
    Entity& entity
)
{
    CombatUnitSlot slot;

    slot.slotIndex = slotIndex;
    slot.side = side;
    slot.kind = kind;
    slot.entity = &entity;
    slot.summon = nullptr;

    return slot;
}

CombatUnitSlot CombatUnitSlot::createSummonSlot(
    int slotIndex,
    CombatSide side,
    Summon& summon
)
{
    CombatUnitSlot slot;

    slot.slotIndex = slotIndex;
    slot.side = side;
    slot.kind = CombatUnitKind::Summon;
    slot.entity = nullptr;
    slot.summon = &summon;

    return slot;
}

// EN: getSlotIndex declares or implements a focused behavior used by this module.
// FR: getSlotIndex déclare ou implémente un comportement précis utilisé par ce module.
int CombatUnitSlot::getSlotIndex() const
{
    return slotIndex;
}

// EN: getSide declares or implements a focused behavior used by this module.
// FR: getSide déclare ou implémente un comportement précis utilisé par ce module.
CombatSide CombatUnitSlot::getSide() const
{
    return side;
}

// EN: getKind declares or implements a focused behavior used by this module.
// FR: getKind déclare ou implémente un comportement précis utilisé par ce module.
CombatUnitKind CombatUnitSlot::getKind() const
{
    return kind;
}

// EN: isEmpty declares or implements a focused behavior used by this module.
// FR: isEmpty déclare ou implémente un comportement précis utilisé par ce module.
bool CombatUnitSlot::isEmpty() const
{
    return kind == CombatUnitKind::Empty || (entity == nullptr && summon == nullptr);
}

// EN: isAlive declares or implements a focused behavior used by this module.
// FR: isAlive déclare ou implémente un comportement précis utilisé par ce module.
bool CombatUnitSlot::isAlive() const
{
    if (entity != nullptr)
    {
        return !entity->isDead();
    }

    if (summon != nullptr)
    {
        return !summon->isDead() && !summon->isExpired();
    }

    return false;
}

// EN: isTargetable declares or implements a focused behavior used by this module.
// FR: isTargetable déclare ou implémente un comportement précis utilisé par ce module.
bool CombatUnitSlot::isTargetable() const
{
    return !isEmpty() && isAlive();
}

std::string CombatUnitSlot::getDisplayName() const
{
    if (entity != nullptr)
    {
        return entity->getName();
    }

    if (summon != nullptr)
    {
        return summon->getName();
    }

    return "Emplacement vide";
}

// EN: getEntity declares or implements a focused behavior used by this module.
// FR: getEntity déclare ou implémente un comportement précis utilisé par ce module.
Entity* CombatUnitSlot::getEntity()
{
    return entity;
}

// EN: getSummon declares or implements a focused behavior used by this module.
// FR: getSummon déclare ou implémente un comportement précis utilisé par ce module.
Summon* CombatUnitSlot::getSummon()
{
    return summon;
}
