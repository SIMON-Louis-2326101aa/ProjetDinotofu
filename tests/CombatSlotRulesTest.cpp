#include "combat/group/CombatGroup.hpp"
#include "combat/group/CombatUnitSlot.hpp"
#include "combat/unit/CombatSlotRules.hpp"
#include "combat/summon/Summon.hpp"
#include "entity/Entity.hpp"

#include <cassert>

int main()
{
    static_assert(CombatSlotRules::SharedActiveSlots == 3);
    static_assert(CombatSlotRules::SummonOnlyBonusSlots == 2);
    static_assert(CombatSlotRules::MaxVisibleActiveSlots == 5);

    assert(CombatSlotRules::maxSummonsForNonSummonCount(0) == 5);
    assert(CombatSlotRules::maxSummonsForNonSummonCount(1) == 4);
    assert(CombatSlotRules::maxSummonsForNonSummonCount(2) == 3);
    assert(CombatSlotRules::maxSummonsForNonSummonCount(3) == 2);

    Entity player("Chef", "Joueur", 100, 5, 10, 15, 0, 0);
    Entity ally1("Aline", "Allié", 80, 4, 8, 12, 0, 0);
    Entity ally2("Boros", "Allié", 90, 5, 9, 13, 0, 0);
    Summon s1("I1", "Chef", 20, 1, 3, 3);
    Summon s2("I2", "Chef", 20, 1, 3, 3);
    Summon s3("I3", "Chef", 20, 1, 3, 3);
    Summon s4("I4", "Chef", 20, 1, 3, 3);
    Summon s5("I5", "Chef", 20, 1, 3, 3);

    CombatGroup oneFighter;
    assert(oneFighter.addSlot(CombatUnitSlot::createEntitySlot(0, CombatSide::PlayerSide, CombatUnitKind::MainFighter, player)));
    assert(oneFighter.addSlot(CombatUnitSlot::createSummonSlot(1, CombatSide::PlayerSide, s1)));
    assert(oneFighter.addSlot(CombatUnitSlot::createSummonSlot(2, CombatSide::PlayerSide, s2)));
    assert(oneFighter.addSlot(CombatUnitSlot::createSummonSlot(3, CombatSide::PlayerSide, s3)));
    assert(oneFighter.addSlot(CombatUnitSlot::createSummonSlot(4, CombatSide::PlayerSide, s4)));
    assert(!oneFighter.addSlot(CombatUnitSlot::createSummonSlot(5, CombatSide::PlayerSide, s5)));
    assert(oneFighter.getSlotCount() == 5);

    CombatGroup fullMain;
    assert(fullMain.addSlot(CombatUnitSlot::createEntitySlot(0, CombatSide::PlayerSide, CombatUnitKind::MainFighter, player)));
    assert(fullMain.addSlot(CombatUnitSlot::createEntitySlot(1, CombatSide::PlayerSide, CombatUnitKind::Ally, ally1)));
    assert(fullMain.addSlot(CombatUnitSlot::createEntitySlot(2, CombatSide::PlayerSide, CombatUnitKind::Ally, ally2)));
    assert(fullMain.addSlot(CombatUnitSlot::createSummonSlot(3, CombatSide::PlayerSide, s1)));
    assert(fullMain.addSlot(CombatUnitSlot::createSummonSlot(4, CombatSide::PlayerSide, s2)));
    assert(!fullMain.addSlot(CombatUnitSlot::createSummonSlot(5, CombatSide::PlayerSide, s3)));
    assert(fullMain.getSlotCount() == 5);

    return 0;
}
