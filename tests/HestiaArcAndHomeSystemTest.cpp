#include <cassert>
#include <string>

#include "class_system/ClassCatalog.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "story/HestiaArcSystem.hpp"
#include "story/HestiaSoulArtifactSystem.hpp"
#include "world/home/PlayerHomeSystem.hpp"

int main()
{
    Player player("Maison", ClassCatalog::createClassByName("Guerrier"));
    Random random(1337);

    auto rentPlan = PlayerHomeSystem::planPurchase(player, "valebrume", false);
    assert(rentPlan.valid && rentPlan.recurringRent > 0);
    player.getInventory().earnCopper(50000);
    const long long walletBeforeHome = player.getInventory().getTotalCopper();
    assert(PlayerHomeSystem::registerPurchase(player, "valebrume", false, 4));
    assert(player.getInventory().getTotalCopper() == walletBeforeHome - rentPlan.upfrontCost);
    assert(player.getHomeState().owned);
    assert(PlayerHomeSystem::servicesActive(player, 10));
    assert(PlayerHomeSystem::isRentDue(player, 11));
    assert(!PlayerHomeSystem::servicesActive(player, 11));
    const long long walletBeforeRent = player.getInventory().getTotalCopper();
    assert(PlayerHomeSystem::payRent(player, 100)); // no accumulated arrears: one current payment restarts 7 days from now
    assert(player.getInventory().getTotalCopper() == walletBeforeRent - rentPlan.recurringRent);
    assert(player.getHomeState().nextRentDueDay == 107);
    assert(PlayerHomeSystem::servicesActive(player, 100));
    assert(!PlayerHomeSystem::planPurchase(player, "rocheveille", true).valid); // one city only

    assert(HestiaArcSystem::completeMainStoryRescue(player, "player_home", true));
    assert(player.getHestiaStoryState().rescued);
    assert(player.getHestiaStoryState().residence == "player_home");
    assert(HestiaArcSystem::combatAptitudeScore(player, "magie lumineuse") == -4); // readaptation first

    assert(PlayerHomeSystem::registerNpcSpouse(player, "Partenaire PNJ", true));
    auto residents = PlayerHomeSystem::describeResidents(player);
    bool hestiaCohabits = false;
    for (const auto& line : residents) if (line.find("Hestia vit ici aussi") != std::string::npos) hestiaCohabits = true;
    assert(hestiaCohabits);

    const int initialTrust = player.getHestiaStoryState().trust;
    assert(HestiaArcSystem::wellbeingLabel(player, 9999).find("tranquillement") != std::string::npos);
    assert(player.getHestiaStoryState().trust == initialTrust); // absence never decays trust or creates upkeep
    auto visitLines = HestiaArcSystem::visit(player, 5, random);
    assert(!visitLines.empty());
    assert(player.getHestiaStoryState().trust > initialTrust);

    auto goblins = HestiaArcSystem::rescueScenario(false);
    auto skeletons = HestiaArcSystem::rescueScenario(true);
    assert(goblins.eliteThreat.find("ninja") != std::string::npos);
    assert(skeletons.eliteThreat.find("explosif") != std::string::npos);
    assert(skeletons.warnings.front().find("crâne") != std::string::npos);

    assert(HestiaArcSystem::setAwakeningStage(player, 1));
    assert(HestiaArcSystem::combatAptitudeScore(player, "magie divine") > HestiaArcSystem::combatAptitudeScore(player, "épée"));
    assert(!HestiaArcSystem::canSacrificeForPlayer(player, true)); // at home, not active
    assert(HestiaArcSystem::moveResidence(player, "active_party", true));
    assert(HestiaArcSystem::canSacrificeForPlayer(player, true));
    assert(HestiaArcSystem::consumeSacrificeForPlayer(player, true));
    assert(!player.getHestiaStoryState().materialized);
    assert(!HestiaArcSystem::consumeSacrificeForPlayer(player, true));

    Player artifactBearer("Artefact", ClassCatalog::createClassByName("Guerrier"));
    assert(HestiaSoulArtifactSystem::grant(artifactBearer, "épée"));
    assert(HestiaSoulArtifactSystem::restingForm(artifactBearer).find("gant") != std::string::npos);
    assert(HestiaSoulArtifactSystem::damage(artifactBearer) == 3);
    assert(HestiaSoulArtifactSystem::killsRemaining(artifactBearer) == 1);
    auto ignoredKill = HestiaSoulArtifactSystem::recordKillingBlow(artifactBearer, "gobelin", false);
    assert(!ignoredKill.counted);
    auto firstSoul = HestiaSoulArtifactSystem::recordKillingBlow(artifactBearer, "gobelin", true);
    assert(firstSoul.counted && firstSoul.levelUp && firstSoul.level == 2);
    assert(firstSoul.damage == 6 && firstSoul.killsRemaining == 2);

    // Une même famille finit définitivement par ne plus nourrir l'artefact.
    for (int i = 1; i < HestiaSoulArtifactSystem::SoulsPerFamilyLimit; ++i)
        HestiaSoulArtifactSystem::recordKillingBlow(artifactBearer, "gobelin", true);
    auto saturated = HestiaSoulArtifactSystem::recordKillingBlow(artifactBearer, "gobelin", true);
    assert(!saturated.counted);
    assert(saturated.reason.find("trop familière") != std::string::npos);

    // Monter à 16 avec des essences variées débloque une deuxième forme choisie par le joueur.
    int uniqueSoul = 0;
    while (artifactBearer.getHestiaArtifactState().level < 16)
    {
        HestiaSoulArtifactSystem::recordKillingBlow(
            artifactBearer, "essence_unique_" + std::to_string(++uniqueSoul), true);
    }
    assert(artifactBearer.getHestiaArtifactState().secondFormUnlocked);
    assert(HestiaSoulArtifactSystem::chooseSecondForm(artifactBearer, "arc"));
    assert(!HestiaSoulArtifactSystem::chooseSecondForm(artifactBearer, "faux"));
    assert(HestiaSoulArtifactSystem::switchToSecondary(artifactBearer));
    assert(HestiaSoulArtifactSystem::activeWeaponForm(artifactBearer) == "arc");
    assert(HestiaSoulArtifactSystem::switchToPrimary(artifactBearer));
    assert(HestiaSoulArtifactSystem::activeWeaponForm(artifactBearer) == "épée");

    return 0;
}
