// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Represents one visible active slot in future group fights: fighter, ally, enemy, summon, or boss.

#include "combat/group/CombatUnitSlot.hpp"

#include "combat/summon/Summon.hpp"
#include "entity/Entity.hpp"

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

int CombatUnitSlot::getSlotIndex() const
{
    return slotIndex;
}

CombatSide CombatUnitSlot::getSide() const
{
    return side;
}

CombatUnitKind CombatUnitSlot::getKind() const
{
    return kind;
}

bool CombatUnitSlot::isEmpty() const
{
    return kind == CombatUnitKind::Empty || (entity == nullptr && summon == nullptr);
}

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

Entity* CombatUnitSlot::getEntity()
{
    return entity;
}

Summon* CombatUnitSlot::getSummon()
{
    return summon;
}
