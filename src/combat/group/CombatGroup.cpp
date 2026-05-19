// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Stores a visible combat group made of active entity and summon slots.

#include "combat/group/CombatGroup.hpp"

#include <algorithm>

CombatGroup::CombatGroup()
{
    maxMainSlots = 3;
    maxSummonSlots = 2;
}

CombatGroup::CombatGroup(int maxMainSlots, int maxSummonSlots)
{
    this->maxMainSlots = maxMainSlots;
    this->maxSummonSlots = maxSummonSlots;
}

bool CombatGroup::addSlot(const CombatUnitSlot& slot)
{
    int mainSlotCount = 0;
    int summonSlotCount = 0;

    for (const CombatUnitSlot& existingSlot : slots)
    {
        if (existingSlot.getKind() == CombatUnitKind::Summon)
        {
            summonSlotCount++;
        }
        else if (!existingSlot.isEmpty())
        {
            mainSlotCount++;
        }
    }

    if (slot.getKind() == CombatUnitKind::Summon)
    {
        if (summonSlotCount >= maxSummonSlots)
        {
            return false;
        }
    }
    else if (!slot.isEmpty())
    {
        if (mainSlotCount >= maxMainSlots)
        {
            return false;
        }
    }

    slots.push_back(slot);
    return true;
}

void CombatGroup::removeDeadOrExpiredSlots()
{
    slots.erase(
        std::remove_if(
            slots.begin(),
            slots.end(),
            [](const CombatUnitSlot& slot)
            {
                return !slot.isEmpty() && !slot.isAlive();
            }
        ),
        slots.end()
    );
}

int CombatGroup::getSlotCount() const
{
    return static_cast<int>(slots.size());
}

int CombatGroup::getTargetableSlotCount() const
{
    int count = 0;

    for (const CombatUnitSlot& slot : slots)
    {
        if (slot.isTargetable())
        {
            count++;
        }
    }

    return count;
}

CombatUnitSlot* CombatGroup::getSlot(int index)
{
    if (index < 0 || index >= static_cast<int>(slots.size()))
    {
        return nullptr;
    }

    return &slots[index];
}

const CombatUnitSlot* CombatGroup::getSlot(int index) const
{
    if (index < 0 || index >= static_cast<int>(slots.size()))
    {
        return nullptr;
    }

    return &slots[index];
}

std::vector<CombatUnitSlot*> CombatGroup::getTargetableSlots()
{
    std::vector<CombatUnitSlot*> targetableSlots;

    for (CombatUnitSlot& slot : slots)
    {
        if (slot.isTargetable())
        {
            targetableSlots.push_back(&slot);
        }
    }

    return targetableSlots;
}

const std::vector<CombatUnitSlot>& CombatGroup::getSlots() const
{
    return slots;
}
