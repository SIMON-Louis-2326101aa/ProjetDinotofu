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

    assert(DuoMasterySystem::coordinationChance(0, 0, 0, 0, false) == 42);
    assert(DuoMasterySystem::coordinationChance(99, 99, 99, 100, true) == 95);

    // V3.51.44 : impossible de chaîner une technique combinée à chaque fenêtre alliée.
    int recovery = DuoMasterySystem::combinedTechniqueRecoveryAfterAttempt();
    assert(recovery == 2);
    assert(!DuoMasterySystem::combinedTechniqueReady(recovery));
    recovery = DuoMasterySystem::advanceCombinedTechniqueRecovery(recovery);
    assert(recovery == 1);
    assert(!DuoMasterySystem::combinedTechniqueReady(recovery));
    recovery = DuoMasterySystem::advanceCombinedTechniqueRecovery(recovery);
    assert(recovery == 0);
    assert(DuoMasterySystem::combinedTechniqueReady(recovery));
    assert(DuoMasterySystem::advanceCombinedTechniqueRecovery(0) == 0);

    Player player;
    const std::string key = DuoMasterySystem::pairKey("Aline", "Boros");
    player.recordCanonicalEvent("techniques_combinees_alliees", key + ":Feu croisé", "test", 7);
    assert(DuoMasterySystem::experience(player, key) == 7);
    assert(DuoMasterySystem::masteryTier(DuoMasterySystem::experience(player, key)) == 2);

    // Une seule trace de combat/ordre par journée : pas de farm en répétant 50 petits combats le même jour.
    assert(DuoMasterySystem::recordSharedCombatDay(player, "Aline", "Boros", "test_pve"));
    assert(!DuoMasterySystem::recordSharedCombatDay(player, "Boros", "Aline", "autre_combat_meme_jour"));
    assert(DuoMasterySystem::experience(player, key) == 8);
    assert(DuoMasterySystem::recordCoordinatedOrderDay(player, "Aline", "Boros", "test_pve"));
    assert(!DuoMasterySystem::recordCoordinatedOrderDay(player, "Boros", "Aline", "autre_ordre_meme_jour"));
    assert(DuoMasterySystem::experience(player, key) == 9);

    // Les faits relationnels vécus comptent dans la fiabilité sans remplacer l'expérience commune.
    player.recordCanonicalEvent("participation_recrues", "Aline", "combat réel", 5);
    player.recordCanonicalEvent("participation_recrues", "Boros", "combat réel", 5);
    player.recordCanonicalEvent("sauvetages_relation_recrues", "Aline", "sauvetage", 1);
    player.recordCanonicalEvent("quetes_personnelles_recrues_terminees", "Boros", "quête perso", 1);
    const DuoBondProfile bond = DuoMasterySystem::bondProfile(player, "Aline", "Boros");
    assert(bond.successfulCombos == 7);
    assert(bond.sharedCombatDays == 1);
    assert(bond.coordinatedOrderDays == 1);
    assert(bond.relationshipScore > 0);
    assert(bond.bondScore > 0);
    assert(DuoMasterySystem::bondLabel(bond.bondTier).empty() == false);

    return 0;
}
