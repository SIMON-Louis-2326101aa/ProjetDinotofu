// EN: CombatGroup.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatGroup.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    // EN: CombatGroup declares or implements a focused behavior used by this module.
    // FR: CombatGroup déclare ou implémente un comportement précis utilisé par ce module.
    CombatGroup();
    CombatGroup(int maxMainSlots, int maxSummonSlots);

    // EN: addSlot declares or implements a focused behavior used by this module.
    // FR: addSlot déclare ou implémente un comportement précis utilisé par ce module.
    bool addSlot(const CombatUnitSlot& slot);
    // EN: removeDeadOrExpiredSlots declares or implements a focused behavior used by this module.
    // FR: removeDeadOrExpiredSlots déclare ou implémente un comportement précis utilisé par ce module.
    void removeDeadOrExpiredSlots();

    // EN: getSlotCount declares or implements a focused behavior used by this module.
    // FR: getSlotCount déclare ou implémente un comportement précis utilisé par ce module.
    int getSlotCount() const;
    // EN: getTargetableSlotCount declares or implements a focused behavior used by this module.
    // FR: getTargetableSlotCount déclare ou implémente un comportement précis utilisé par ce module.
    int getTargetableSlotCount() const;

    // EN: getSlot declares or implements a focused behavior used by this module.
    // FR: getSlot déclare ou implémente un comportement précis utilisé par ce module.
    CombatUnitSlot* getSlot(int index);
    const CombatUnitSlot* getSlot(int index) const;

    std::vector<CombatUnitSlot*> getTargetableSlots();
    // EN: getSlots declares or implements a focused behavior used by this module.
    // FR: getSlots déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<CombatUnitSlot>& getSlots() const;
};

#endif
