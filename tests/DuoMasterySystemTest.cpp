#include "combat/ally/DuoMasterySystem.hpp"
#include "entity/Player.hpp"
#include <cassert>

int main()
{
    assert(DuoMasterySystem::pairKey("Boros", "Aline") == "Aline|Boros");
    assert(DuoMasterySystem::pairKey("Aline", "Boros") == "Aline|Boros");
    assert(DuoMasterySystem::masteryTier(0) == 0);
    assert(DuoMasterySystem::masteryTier(3) == 1);
    assert(DuoMasterySystem::masteryTier(7) == 2);
    assert(DuoMasterySystem::masteryTier(12) == 3);
    assert(DuoMasterySystem::masteryLabel(3) == "signature commune");

    assert(DuoMasterySystem::roleForJob("Gardien lourd") == DuoCombatRole::Tank);
    assert(DuoMasterySystem::roleForJob("Soigneur de campagne") == DuoCombatRole::Support);
    assert(DuoMasterySystem::roleForJob("Archer") == DuoCombatRole::Ranged);

    const DuoTechniquePlan wall = DuoMasterySystem::techniqueForJobs("Gardien", "Protecteur");
    assert(wall.name == "Mur en mouvement");
    assert(wall.playerGuardPercent > 0);
    assert(wall.powerPercent < 100);

    const DuoTechniquePlan relay = DuoMasterySystem::techniqueForJobs("Soigneur", "Support");
    assert(relay.name == "Relais vital");
    assert(relay.playerHealPercent > 0);

    const DuoTechniquePlan crossfire = DuoMasterySystem::techniqueForJobs("Archer", "Tireur");
    assert(crossfire.name == "Feu croisé");
    assert(crossfire.powerPercent > 100);

    assert(DuoMasterySystem::coordinationChance(0, 0, 0, false) == 62);
    assert(DuoMasterySystem::coordinationChance(99, 99, 99, true) == 95);

    Player player;
    const std::string key = DuoMasterySystem::pairKey("Aline", "Boros");
    player.recordCanonicalEvent("techniques_combinees_alliees", key + ":Feu croisé", "test", 7);
    assert(DuoMasterySystem::experience(player, key) == 7);
    assert(DuoMasterySystem::masteryTier(DuoMasterySystem::experience(player, key)) == 2);
    return 0;
}
