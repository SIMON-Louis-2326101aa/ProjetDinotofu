#include <cassert>
#include <string>

#include "class_system/ClassCatalog.hpp"
#include "combat/anatomy/MonsterAnatomySystem.hpp"
#include "core/Random.hpp"
#include "entity/Monster.hpp"
#include "entity/Player.hpp"

namespace
{
    Monster makeMonster(const std::string& name, const std::string& type, Race race, int level = 8)
    {
        return Monster(name, type, race, level, 180, 8, 15, 22, 0, 0, false, true, false, true);
    }
}

int main()
{
    Player player("Anatomie", ClassCatalog::createClassByName("Guerrier"));
    Random random(13);

    Monster explosive(
        "Squelette suicidaire explosif",
        "Mort-vivant explosif toxique et hallucinogène — crâne piégé",
        Race::MortVivant,
        8,
        80,
        5,
        9,
        12,
        0,
        0,
        false,
        true,
        false,
        true
    );

    const MonsterWeakPointProfile explosiveProfile = MonsterAnatomySystem::weakPointProfile(explosive);
    assert(explosiveProfile.zoneId == "trapped_skull");
    assert(explosiveProfile.reactiveHazard);
    assert(explosiveProfile.selfDestructiveHazard);
    assert(explosiveProfile.hint.find("crâne") != std::string::npos);

    const int hpBefore = player.getHp();
    const MonsterWeakPointHitResult explosiveResult = MonsterAnatomySystem::resolveWeakPointReaction(explosive, player, random);
    assert(explosiveResult.triggered);
    assert(explosiveResult.targetDestroyedByReaction);
    assert(explosive.isDead());
    assert(player.getHp() < hpBefore);
    assert(player.hasPoison());
    assert(player.hasBurning());
    assert(player.hasIllusion());

    Monster normalSkeleton("Squelette normal", "Mort-vivant", Race::MortVivant, 4, 30, 3, 5, 8, 0, 0);
    const MonsterWeakPointProfile normalProfile = MonsterAnatomySystem::weakPointProfile(normalSkeleton);
    assert(!normalProfile.reactiveHazard);
    assert(normalProfile.zoneId == "joints");
    const MonsterWeakPointHitResult normalResult = MonsterAnatomySystem::resolveWeakPointReaction(normalSkeleton, player, random);
    assert(normalResult.triggered);
    assert(normalSkeleton.hasNextHitVulnerability());

    Monster construction = makeMonster("Golem d'atelier", "Construction usée", Race::Construction, 7);
    construction.startDefensePosture(25, 10, "carapace mécanique");
    const MonsterWeakPointProfile constructionProfile = MonsterAnatomySystem::weakPointProfile(construction);
    assert(constructionProfile.zoneId == "mechanical_joint");
    assert(constructionProfile.revealTargetStats);
    assert(constructionProfile.breakDefensePosture);
    const MonsterWeakPointHitResult constructionResult = MonsterAnatomySystem::resolveWeakPointReaction(construction, player, random);
    assert(constructionResult.triggered);
    assert(construction.areStatsVisible());
    assert(construction.hasWeakening());
    assert(construction.hasNextHitVulnerability());
    assert(!construction.isInDefensePosture());

    Monster armoredConstruction = makeMonster("Armure cabossée", "Construction hantée faible", Race::Construction, 4);
    const MonsterWeakPointProfile armoredConstructionProfile = MonsterAnatomySystem::weakPointProfile(armoredConstruction);
    assert(armoredConstructionProfile.zoneId == "plate_joint");
    assert(armoredConstructionProfile.nextHitVulnerabilityPercent > constructionProfile.nextHitVulnerabilityPercent);

    Monster dragon = makeMonster("Dragon mineur", "Jeune menace draconique", Race::Dragon, 8);
    const MonsterWeakPointProfile dragonProfile = MonsterAnatomySystem::weakPointProfile(dragon);
    assert(dragonProfile.zoneId == "wing_root");
    MonsterAnatomySystem::resolveWeakPointReaction(dragon, player, random);
    assert(dragon.hasWeakening());
    assert(dragon.hasNextHitVulnerability());

    Monster draconide = makeMonster("Draconide à écailles grises", "Draconide jeune", Race::Draconide, 3);
    const MonsterWeakPointProfile draconideProfile = MonsterAnatomySystem::weakPointProfile(draconide);
    assert(draconideProfile.zoneId == "scale_seam");
    MonsterAnatomySystem::resolveWeakPointReaction(draconide, player, random);
    assert(draconide.hasNextHitVulnerability());

    Monster plant = makeMonster("Écorceuse ancienne", "Plante qui râpe les armures", Race::Plante, 7);
    plant.startDefensePosture(20, 0, "racines verrouillées");
    const MonsterWeakPointProfile plantProfile = MonsterAnatomySystem::weakPointProfile(plant);
    assert(plantProfile.zoneId == "root_collar");
    MonsterAnatomySystem::resolveWeakPointReaction(plant, player, random);
    assert(plant.hasWeakening());
    assert(!plant.isInDefensePosture());

    Monster armoredInsect = makeMonster("Araignée cuirassée", "Insectoïde protégé", Race::Insectoide, 6);
    const MonsterWeakPointProfile armoredInsectProfile = MonsterAnatomySystem::weakPointProfile(armoredInsect);
    assert(armoredInsectProfile.zoneId == "carapace_joint");
    assert(armoredInsectProfile.nextHitVulnerabilityPercent == 10);

    Monster armoredBeast = makeMonster("Escargot cuirassé de fossé", "Bête lente à coquille épaisse", Race::Bete, 1);
    const MonsterWeakPointProfile armoredBeastProfile = MonsterAnatomySystem::weakPointProfile(armoredBeast);
    assert(armoredBeastProfile.zoneId == "shell_hinge");
    assert(armoredBeastProfile.zoneLabel.find("coquille") != std::string::npos);

    Monster elemental = makeMonster("Élémentaire instable", "Énergie condensée", Race::Elementaire, 5);
    const MonsterWeakPointProfile elementalProfile = MonsterAnatomySystem::weakPointProfile(elemental);
    assert(elementalProfile.zoneId == "focus_core");
    MonsterAnatomySystem::resolveWeakPointReaction(elemental, player, random);
    assert(elemental.areStatsVisible());
    assert(elemental.hasVulnerability());

    Monster spirit = makeMonster("Spectre de serment brisé", "Esprit vengeur", Race::Esprit, 4);
    const MonsterWeakPointProfile spiritProfile = MonsterAnatomySystem::weakPointProfile(spirit);
    assert(spiritProfile.zoneId == "spiritual_anchor");
    MonsterAnatomySystem::resolveWeakPointReaction(spirit, player, random);
    assert(spirit.areStatsVisible());
    assert(spirit.hasVulnerability());

    Player anomalyPlayer("Contrecoup", ClassCatalog::createClassByName("Guerrier"));
    Monster anomaly = makeMonster("Anomalie arcanique instable", "Erreur magique", Race::AnomalieArcanique, 4);
    const MonsterWeakPointProfile anomalyProfile = MonsterAnatomySystem::weakPointProfile(anomaly);
    assert(anomalyProfile.zoneId == "unstable_knot");
    assert(anomalyProfile.reactiveHazard);
    assert(!anomalyProfile.selfDestructiveHazard);
    const int anomalyHpBefore = anomalyPlayer.getHp();
    const MonsterWeakPointHitResult anomalyResult = MonsterAnatomySystem::resolveWeakPointReaction(anomaly, anomalyPlayer, random);
    assert(anomalyResult.triggered);
    assert(!anomalyResult.targetDestroyedByReaction);
    assert(anomalyPlayer.getHp() < anomalyHpBefore);
    assert(anomalyPlayer.hasIllusion());
    assert(anomaly.hasVulnerability());

    Monster humanoid = makeMonster("Bandit perdu", "Humain opportuniste", Race::Humain, 4);
    const MonsterWeakPointProfile humanoidProfile = MonsterAnatomySystem::weakPointProfile(humanoid);
    assert(humanoidProfile.zoneId == "generic");
    const MonsterWeakPointHitResult humanoidResult = MonsterAnatomySystem::resolveWeakPointReaction(humanoid, player, random);
    assert(!humanoidResult.triggered);

    return 0;
}
