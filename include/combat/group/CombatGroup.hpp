// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Stores a visible combat group made of active entity and summon slots.

#ifndef INCLUDE_COMBAT_GROUP_COMBATGROUP_HPP
#define INCLUDE_COMBAT_GROUP_COMBATGROUP_HPP

#include "combat/group/CombatUnitSlot.hpp"

#include <vector>

class CombatGroup
{
private:
    std::vector<CombatUnitSlot> slots;
    int maxMainSlots;
    int maxSummonSlots;

public:
    CombatGroup();
    CombatGroup(int maxMainSlots, int maxSummonSlots);

    bool addSlot(const CombatUnitSlot& slot);
    void removeDeadOrExpiredSlots();

    int getSlotCount() const;
    int getTargetableSlotCount() const;

    CombatUnitSlot* getSlot(int index);
    const CombatUnitSlot* getSlot(int index) const;

    std::vector<CombatUnitSlot*> getTargetableSlots();
    const std::vector<CombatUnitSlot>& getSlots() const;
};

#endif
