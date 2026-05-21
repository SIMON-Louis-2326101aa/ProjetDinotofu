// EN: CombatGroup.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatGroup.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Stores a visible combat group made of active entity and summon slots.

#include "combat/group/CombatGroup.hpp"

#include <algorithm>

// EN: CombatGroup declares or implements a focused behavior used by this module.
// FR: CombatGroup déclare ou implémente un comportement précis utilisé par ce module.
CombatGroup::CombatGroup()
{
    maxMainSlots = 3;
    maxSummonSlots = 2;
}

// EN: CombatGroup declares or implements a focused behavior used by this module.
// FR: CombatGroup déclare ou implémente un comportement précis utilisé par ce module.
CombatGroup::CombatGroup(int maxMainSlots, int maxSummonSlots)
{
    this->maxMainSlots = maxMainSlots;
    this->maxSummonSlots = maxSummonSlots;
}

// EN: addSlot declares or implements a focused behavior used by this module.
// FR: addSlot déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: removeDeadOrExpiredSlots declares or implements a focused behavior used by this module.
// FR: removeDeadOrExpiredSlots déclare ou implémente un comportement précis utilisé par ce module.
void CombatGroup::removeDeadOrExpiredSlots()
{
    slots.erase(
        std::remove_if(
            slots.begin(),
            slots.end(),
            // EN: [] declares or implements a focused behavior used by this module.
            // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
            [](const CombatUnitSlot& slot)
            {
                return !slot.isEmpty() && !slot.isAlive();
            }
        ),
        slots.end()
    );
}

// EN: getSlotCount declares or implements a focused behavior used by this module.
// FR: getSlotCount déclare ou implémente un comportement précis utilisé par ce module.
int CombatGroup::getSlotCount() const
{
    return static_cast<int>(slots.size());
}

// EN: getTargetableSlotCount declares or implements a focused behavior used by this module.
// FR: getTargetableSlotCount déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: getSlot declares or implements a focused behavior used by this module.
// FR: getSlot déclare ou implémente un comportement précis utilisé par ce module.
CombatUnitSlot* CombatGroup::getSlot(int index)
{
    if (index < 0 || index >= static_cast<int>(slots.size()))
    {
        return nullptr;
    }

    return &slots[index];
}

// EN: getSlot declares or implements a focused behavior used by this module.
// FR: getSlot déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: getSlots declares or implements a focused behavior used by this module.
// FR: getSlots déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<CombatUnitSlot>& CombatGroup::getSlots() const
{
    return slots;
}
