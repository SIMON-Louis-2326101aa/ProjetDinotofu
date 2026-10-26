#include "class_system/ClassCatalog.hpp"
#include "combat/CombatActions.hpp"
#include "combat/MagicMaterialRules.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <cassert>
#include <string>
#include <vector>

int main()
{
    Player material("Rune", ClassCatalog::createClassByName("Runiste"));
    Player catalyst("Mira", ClassCatalog::createClassByName("Mage"));
    Player pure("Sera", ClassCatalog::createClassByName("Sorcier"));
    Player druid("Iria", ClassCatalog::createClassByName("Druide"));
    Player electromancer("Volt", ClassCatalog::createClassByName("Électromancien"));
    Player mirageCaster("Mirae", ClassCatalog::createClassByName("Miragien"));
    Player priest("Aster", ClassCatalog::createClassByName("Prêtre"));
    Player necromancer("Nox", ClassCatalog::createClassByName("Nécromancien"));
    Player pactist("Vey", ClassCatalog::createClassByName("Pactisant"));
    Player warrior("Bram", ClassCatalog::createClassByName("Guerrier"));

    const auto& magicMaterialIds = MagicMaterialRules::orderedChannelingMaterialIds();
    assert(magicMaterialIds.size() == 4);
    assert(magicMaterialIds[0] == "arcane_dust");
    assert(magicMaterialIds[1] == "runic_iron_shard");
    assert(magicMaterialIds[2] == "moonlit_salt");
    assert(magicMaterialIds[3] == "echoing_resin");
    assert(MagicMaterialRules::displayName("arcane_dust") == "Poussière arcanique");
    assert(MagicMaterialRules::displayName("runic_iron_shard") == "Éclat de fer runique");
    assert(MagicMaterialRules::displayName("moonlit_salt") == "Sel lunaire");
    assert(MagicMaterialRules::displayName("echoing_resin") == "Résine d'écho");
    assert(MagicMaterialRules::isMineralFuelId("runic_iron_shard"));
    assert(MagicMaterialRules::isMineralFuelId("cold_iron_nail"));
    assert(!MagicMaterialRules::isMineralFuelId("arcane_dust"));
    assert(MagicMaterialRules::materialSupportsElement("kitsune_ember", "feu"));
    assert(MagicMaterialRules::materialSupportsElement("rare_cold_rune_core", "givre"));
    assert(MagicMaterialRules::materialSupportsElement("runic_iron_shard", "terre"));
    assert(MagicMaterialRules::materialSupportsElement("holy_water_vial", "eau"));
    assert(MagicMaterialRules::materialSupportsElement("echoing_resin", "vent"));
    assert(MagicMaterialRules::materialSupportsElement("arcane_dust", "arcanique"));
    assert(MagicMaterialRules::materialSupportsElement("runic_iron_shard", "foudre"));
    assert(MagicMaterialRules::materialSupportsElement("unstable_core", "electro"));
    assert(MagicMaterialRules::elementDisplayName("foudre") == "Foudre");
    assert(MagicMaterialRules::materialRarityTier("arcane_dust") == 1);
    assert(MagicMaterialRules::materialRarityTier("runic_iron_shard") == 2);
    assert(MagicMaterialRules::materialRarityTier("unstable_core") == 4);
    assert(MagicMaterialRules::materialRarityTier("elemental_fusion_core") == 5);

    Player profileRunist("RuneProfil", ClassCatalog::createClassByName("Runiste"));
    profileRunist.getInventory().addMaterial(MaterialCatalog::createRunicIronShard(1));
    auto normalRunicProfile = MagicMaterialRules::channelingProfile(profileRunist.getInventory(), "runic_iron_shard", false);
    auto sacrificeRunicProfile = MagicMaterialRules::channelingProfile(profileRunist.getInventory(), "runic_iron_shard", true);
    assert(normalRunicProfile.manaPercent == 45);
    assert(normalRunicProfile.stabilityBonus == 9);
    assert(normalRunicProfile.powerBoostPercent == 5);
    assert(sacrificeRunicProfile.manaPercent == 80);
    assert(sacrificeRunicProfile.stabilityBonus == 18);
    assert(sacrificeRunicProfile.powerBoostPercent == 41);
    assert(sacrificeRunicProfile.manaPercent > normalRunicProfile.manaPercent);
    assert(sacrificeRunicProfile.powerBoostPercent > normalRunicProfile.powerBoostPercent);

    Player exceptionalFuel("RuneException", ClassCatalog::createClassByName("Runiste"));
    exceptionalFuel.getInventory().addMaterial(MaterialCatalog::createById("elemental_fusion_core", 1, "exceptional"));
    auto exceptionalSacrifice = MagicMaterialRules::channelingProfile(exceptionalFuel.getInventory(), "elemental_fusion_core", true);
    assert(exceptionalSacrifice.rarityTier == 5);
    assert(exceptionalSacrifice.qualityWeight == 4);
    assert(exceptionalSacrifice.powerBoostPercent >= 70);

    // Balance invariants: normal channeling must remain the long-term efficient path.
    // Sacrifice is stronger in one turn, but deliberately spends more mana per effective output and destroys the unit.
    const std::vector<std::string> balanceMaterialIds = {
        "arcane_dust", "runic_iron_shard", "kitsune_ember", "unstable_core", "elemental_fusion_core"
    };
    const std::vector<std::string> balanceQualities = {"impure", "normal", "pure", "exceptional"};
    for (const std::string& materialId : balanceMaterialIds)
    {
        int previousNormalMana = 999;
        int previousNormalPower = -1;
        int previousSacrificeMana = 999;
        int previousSacrificePower = -1;
        for (const std::string& quality : balanceQualities)
        {
            const auto normalProfile = MagicMaterialRules::channelingProfileForQuality(materialId, quality, false);
            const auto burstProfile = MagicMaterialRules::channelingProfileForQuality(materialId, quality, true);
            assert(normalProfile.manaPercent >= 34 && normalProfile.manaPercent <= 50);
            assert(normalProfile.stabilityBonus >= 6 && normalProfile.stabilityBonus <= 18);
            assert(normalProfile.powerBoostPercent >= 0 && normalProfile.powerBoostPercent <= 20);
            assert(burstProfile.manaPercent >= 65 && burstProfile.manaPercent <= 82);
            assert(burstProfile.stabilityBonus >= 12 && burstProfile.stabilityBonus <= 30);
            assert(burstProfile.powerBoostPercent >= 25 && burstProfile.powerBoostPercent <= 75);
            assert(burstProfile.manaPercent > normalProfile.manaPercent);
            assert(burstProfile.powerBoostPercent > normalProfile.powerBoostPercent);

            const long long normalEfficiencyNumerator = 100LL + normalProfile.powerBoostPercent;
            const long long burstEfficiencyNumerator = 100LL + burstProfile.powerBoostPercent;
            assert(burstEfficiencyNumerator * normalProfile.manaPercent
                < normalEfficiencyNumerator * burstProfile.manaPercent);

            assert(normalProfile.manaPercent <= previousNormalMana);
            assert(normalProfile.powerBoostPercent >= previousNormalPower);
            assert(burstProfile.manaPercent <= previousSacrificeMana);
            assert(burstProfile.powerBoostPercent >= previousSacrificePower);
            previousNormalMana = normalProfile.manaPercent;
            previousNormalPower = normalProfile.powerBoostPercent;
            previousSacrificeMana = burstProfile.manaPercent;
            previousSacrificePower = burstProfile.powerBoostPercent;
        }
    }

    // Sacrifice matérial must let the player target the exact quality stack instead of silently burning the cheapest one.
    Player mixedQualityFuel("RuneQualites", ClassCatalog::createClassByName("Runiste"));
    mixedQualityFuel.getInventory().addMaterial(MaterialCatalog::createById("runic_iron_shard", 2, "normal"));
    mixedQualityFuel.getInventory().addMaterial(MaterialCatalog::createById("runic_iron_shard", 1, "exceptional"));
    const auto qualityVariants = MagicMaterialRules::availableQualityVariants(mixedQualityFuel.getInventory(), "runic_iron_shard");
    assert(qualityVariants.size() == 2);
    assert(qualityVariants.front() == "exceptional");
    assert(mixedQualityFuel.getInventory().countMaterialByIdAndQuality("runic_iron_shard", "normal") == 2);
    assert(mixedQualityFuel.getInventory().countMaterialByIdAndQuality("runic_iron_shard", "exceptional") == 1);
    const auto normalSacrificeExact = MagicMaterialRules::channelingProfileForQuality("runic_iron_shard", "normal", true);
    const auto exceptionalSacrificeExact = MagicMaterialRules::channelingProfileForQuality("runic_iron_shard", "exceptional", true);
    assert(exceptionalSacrificeExact.powerBoostPercent > normalSacrificeExact.powerBoostPercent);
    assert(exceptionalSacrificeExact.stabilityBonus > normalSacrificeExact.stabilityBonus);
    assert(exceptionalSacrificeExact.manaPercent < normalSacrificeExact.manaPercent);
    assert(mixedQualityFuel.getInventory().removeMaterialQuantityByIdAndQuality("runic_iron_shard", "exceptional", 1));
    assert(mixedQualityFuel.getInventory().countMaterialByIdAndQuality("runic_iron_shard", "exceptional") == 0);
    assert(mixedQualityFuel.getInventory().countMaterialByIdAndQuality("runic_iron_shard", "normal") == 2);

    // Normal channeling can also target an exact quality stack; higher quality is never selected silently.
    Player exactNormalFuel("RuneCanalQualite", ClassCatalog::createClassByName("Runiste"));
    exactNormalFuel.getInventory().addMaterial(MaterialCatalog::createById("runic_iron_shard", 1, "normal"));
    exactNormalFuel.getInventory().addMaterial(MaterialCatalog::createById("runic_iron_shard", 1, "exceptional"));
    const std::string exactPreview = MagicMaterialRules::formatMaterialUsePreviewForQuality(
        exactNormalFuel.getInventory(), "runic_iron_shard", "exceptional", true
    );
    assert(exactPreview.find("Exceptionnel") != std::string::npos);
    std::string previousQuality;
    std::string resultingQuality;
    bool dissolvedExact = false;
    assert(exactNormalFuel.getInventory().degradeMaterialQualityByIdAndQuality(
        "runic_iron_shard", "exceptional", previousQuality, resultingQuality, dissolvedExact
    ));
    assert(previousQuality == "exceptional");
    assert(resultingQuality == "pure");
    assert(!dissolvedExact);
    assert(exactNormalFuel.getInventory().countMaterialByIdAndQuality("runic_iron_shard", "normal") == 1);
    assert(exactNormalFuel.getInventory().countMaterialByIdAndQuality("runic_iron_shard", "pure") == 1);
    const auto* frostRecipe = MagicMaterialRules::findSpellMaterialRecipe("Aiguille de givre");
    assert(frostRecipe != nullptr);
    assert(frostRecipe->requirements.size() == 2);
    assert(frostRecipe->requirements[0].materialId == "moonlit_salt");
    assert(!frostRecipe->requirements[0].degradeInsteadOfConsume);
    assert(frostRecipe->requirements[1].materialId == "cold_iron_nail");
    assert(frostRecipe->requirements[1].degradeInsteadOfConsume);

    assert(material.getMagicChannelingStyleLabel() == "Minerais / matériaux");
    assert(catalyst.getMagicChannelingStyleLabel() == "Baguette / catalyseur");
    assert(pure.getMagicChannelingStyleLabel() == "Incantation pure");

    // Hybrid/support casters must not silently fall back to the tiny non-caster reserve.
    assert(druid.getMagicChannelingStyleLabel() == "Baguette / catalyseur");
    assert(electromancer.getMagicChannelingStyleLabel() == "Baguette / catalyseur");
    assert(mirageCaster.getMagicChannelingStyleLabel() == "Baguette / catalyseur");
    assert(priest.getMagicChannelingStyleLabel() == "Baguette / catalyseur");
    assert(necromancer.getMagicChannelingStyleLabel() == "Incantation pure");
    assert(pactist.getMagicChannelingStyleLabel() == "Incantation pure");
    assert(warrior.getMagicChannelingStyleLabel() == "Réserve mineure");

    assert(material.getMagicRecoveryAdjustment() == 0);
    assert(material.getMagicRecoveryLabel() == "standard");
    assert(material.getMagicSupportStatusLine().find("aucun composant adapté") != std::string::npos);
    assert(pure.getMagicRecoveryAdjustment() == 1);
    assert(pure.getMagicRecoveryLabel() == "lent");
    assert(pure.getMagicSupportStatusLine().find("aucun support obligatoire") != std::string::npos);
    assert(catalyst.getMagicRecoveryAdjustment() == 1); // no catalyst equipped yet -> slow fallback
    assert(catalyst.getMagicRecoveryLabel() == "lent");
    assert(catalyst.getMagicSupportStatusLine().find("aucun catalyseur équipé") != std::string::npos);
    catalyst.getInventory().addWeapon(WeaponCatalog::createSingingResinStaff());
    catalyst.equipWeapon(0);
    assert(catalyst.getMagicRecoveryAdjustment() == -1);
    assert(catalyst.getMagicRecoveryLabel() == "rapide");
    assert(catalyst.getMagicSupportStatusLine().find("catalyseur fonctionnel") != std::string::npos);

    // The three intended reservoir profiles stay distinct at equal progression.
    assert(material.getMaxMana() < catalyst.getMaxMana());
    assert(catalyst.getMaxMana() < pure.getMaxMana());
    assert(warrior.getMaxMana() < material.getMaxMana());

    const int levelOneMaterialMana = material.getMaxMana();
    material.setLoadedProgress(10, 0, material.getHp());
    assert(material.getMaxMana() > levelOneMaterialMana);
    material.restoreManaToFull();
    assert(material.getMana() == material.getMaxMana());

    const int beforeSpend = catalyst.getMana();
    assert(catalyst.spendMana(10));
    assert(catalyst.getMana() == beforeSpend - 10);
    catalyst.regenerateManaForCombatTurn();
    assert(catalyst.getMana() > beforeSpend - 10);
    assert(catalyst.getMana() <= catalyst.getMaxMana());

    // High-level pools must not turn fixed-cost spells into free mana engines.
    // Natural combat regeneration keeps helping, but is capped at 5 mana per turn.
    Player highLevelCatalyst("Mira255", ClassCatalog::createClassByName("Mage"));
    highLevelCatalyst.setLoadedProgress(255, 0, highLevelCatalyst.getHp());
    highLevelCatalyst.restoreManaToFull();
    const int highLevelManaBeforeSpend = highLevelCatalyst.getMana();
    assert(highLevelCatalyst.spendMana(20));
    highLevelCatalyst.regenerateManaForCombatTurn();
    assert(highLevelCatalyst.getMana() == highLevelManaBeforeSpend - 15);
    assert(highLevelCatalyst.getMaxMana() > 500);

    catalyst.applyManaExhaustion(2);
    assert(catalyst.getManaExhaustionTurns() == 2);
    assert(catalyst.consumeManaExhaustionTurn());
    assert(catalyst.getManaExhaustionTurns() == 1);
    assert(catalyst.consumeManaExhaustionTurn());
    assert(catalyst.getManaExhaustionTurns() == 0);
    assert(!catalyst.consumeManaExhaustionTurn());

    Random random;

    // An impossible cast must not eat a prepared material.
    Player materialNoMana("RuneSec", ClassCatalog::createClassByName("Runiste"));
    materialNoMana.getInventory().addMaterial(MaterialCatalog::createArcaneDust(1));
    materialNoMana.getInventory().addMaterial(MaterialCatalog::createRunicIronShard(2));
    assert(MagicMaterialRules::countAvailableMaterials(materialNoMana.getInventory()) == 3);
    assert(MagicMaterialRules::findFirstAvailableMaterialId(materialNoMana.getInventory()) == "arcane_dust");
    assert(MagicMaterialRules::formatAvailableMaterialStocks(materialNoMana.getInventory()).find("Éclat de fer runique x2") != std::string::npos);
    assert(materialNoMana.getMagicSupportStatusLine().find("Poussière arcanique x1") != std::string::npos);
    assert(MagicMaterialRules::nextMaterialQuality(materialNoMana.getInventory(), "runic_iron_shard") == "normal");
    assert(MagicMaterialRules::formatMaterialUsePreview(materialNoMana.getInventory(), "runic_iron_shard", true).find("qualité -1 palier") != std::string::npos);
    materialNoMana.setLoadedManaState(1, 0);
    const int materialBeforeFailedCast = materialNoMana.getInventory().countMaterialById("arcane_dust");
    assert(!CombatActions::prepareMagicTechnique(materialNoMana, "Écho canalisé", random, 18));
    assert(materialNoMana.getInventory().countMaterialById("arcane_dust") == materialBeforeFailedCast);
    assert(materialNoMana.getMana() == 1);

    // A functional catalyst is only worn once enough mana exists to actually start channeling.
    Player catalystNoMana("MiraSec", ClassCatalog::createClassByName("Mage"));
    catalystNoMana.getInventory().addWeapon(WeaponCatalog::createSingingResinStaff());
    catalystNoMana.equipWeapon(0);
    catalystNoMana.setLoadedManaState(1, 0);
    const int durabilityBeforeFailedCast = catalystNoMana.getEquippedWeapon().getDurability();
    assert(!CombatActions::prepareMagicTechnique(catalystNoMana, "Écho canalisé", random, 18));
    assert(catalystNoMana.getEquippedWeapon().getDurability() == durabilityBeforeFailedCast);
    assert(catalystNoMana.getMana() == 1);

    // Broken catalysts no longer keep the normal 25% mana discount. They fall back to direct incantation.
    Player brokenCatalyst("MiraBrisee", ClassCatalog::createClassByName("Mage"));
    brokenCatalyst.getInventory().addWeapon(WeaponCatalog::createSingingResinStaff());
    brokenCatalyst.equipWeapon(0);
    Weapon* brokenStaff = brokenCatalyst.getInventory().getMutableWeapon(brokenCatalyst.getEquippedWeaponIndex());
    assert(brokenStaff != nullptr);
    brokenStaff->loseDurability(brokenStaff->getDurability());
    assert(brokenStaff->isBroken());
    assert(brokenCatalyst.getMagicRecoveryLabel() == "lent");
    assert(brokenCatalyst.getMagicSupportStatusLine().find("brisé") != std::string::npos);
    brokenCatalyst.setLoadedManaState(18, 0);
    assert(!CombatActions::prepareMagicTechnique(brokenCatalyst, "Écho canalisé", random, 18));
    assert(brokenCatalyst.getMana() == 18); // fallback costs 21 mana (120% of 18), so the cast cannot start.

    // Elemental supports are prioritized for their matching spell family, while generic channeling stays available.
    Player elementalRunist("RuneElement", ClassCatalog::createClassByName("Runiste"));
    elementalRunist.getInventory().addMaterial(MaterialCatalog::createArcaneDust(1));
    elementalRunist.getInventory().addMaterial(MaterialCatalog::createKitsuneEmber(1));
    const auto fireSupports = MagicMaterialRules::availableMaterialIds(elementalRunist.getInventory(), "feu");
    assert(!fireSupports.empty());
    assert(fireSupports.front() == "kitsune_ember");
    assert(MagicMaterialRules::findFirstAvailableMaterialId(elementalRunist.getInventory(), "feu") == "kitsune_ember");
    assert(elementalRunist.getMagicSupportStatusLine().find("Braise kitsune") != std::string::npos);
    assert(MagicMaterialRules::isKnownMagicMaterialId("kitsune_ember"));
    const auto kitsuneElements = MagicMaterialRules::materialElementLabels("kitsune_ember");
    assert(!kitsuneElements.empty());
    assert(kitsuneElements.front() == "Feu");

    // Known recipes remain inspectable even while incomplete, so the UI can teach what is missing.
    Player incompleteRecipeRunist("RuneRecetteIncomplete", ClassCatalog::createClassByName("Runiste"));
    incompleteRecipeRunist.getInventory().addMaterial(MaterialCatalog::createMoonlitSalt(1));
    assert(!MagicMaterialRules::hasSpellMaterialRecipeIngredients(incompleteRecipeRunist.getInventory(), "Aiguille de givre"));
    const auto missingFrostIngredients = MagicMaterialRules::missingSpellMaterialIngredients(incompleteRecipeRunist.getInventory(), "Aiguille de givre");
    assert(missingFrostIngredients.size() == 1);
    assert(missingFrostIngredients[0].find("Clou de fer froid x1") != std::string::npos);
    const auto incompleteRecipePreview = MagicMaterialRules::formatSpellMaterialRecipe(incompleteRecipeRunist.getInventory(), "Aiguille de givre");
    assert(incompleteRecipePreview.size() == 3);
    assert(incompleteRecipePreview[2].find("[MANQUANT]") != std::string::npos);

    // Multi-material spell recipes are explicit and transactionally consume/degrade their ingredients.
    Player recipeRunist("RuneRecette", ClassCatalog::createClassByName("Runiste"));
    recipeRunist.getInventory().addMaterial(MaterialCatalog::createMoonlitSalt(1));
    recipeRunist.getInventory().addMaterial(MaterialCatalog::createColdIronNail(1));
    assert(MagicMaterialRules::hasSpellMaterialRecipeIngredients(recipeRunist.getInventory(), "Aiguille de givre"));
    const auto recipePreview = MagicMaterialRules::formatSpellMaterialRecipe(recipeRunist.getInventory(), "Aiguille de givre");
    assert(recipePreview.size() == 3);
    assert(recipePreview[1].find("sacrifié") != std::string::npos);
    assert(recipePreview[2].find("qualité dégradée") != std::string::npos);
    std::vector<std::string> recipeResultLines;
    assert(MagicMaterialRules::consumeSpellMaterialRecipe(recipeRunist.getInventory(), "Aiguille de givre", recipeResultLines));
    assert(recipeRunist.getInventory().countMaterialById("moonlit_salt") == 0);
    assert(recipeRunist.getInventory().countMaterialById("cold_iron_nail") == 1);
    assert(MagicMaterialRules::nextMaterialQuality(recipeRunist.getInventory(), "cold_iron_nail") == "impure");
    assert(!recipeResultLines.empty());

    // A recipe can lock exact qualities for every ingredient instead of silently using another stack.
    Player exactRecipeRunist("RuneRecetteQualite", ClassCatalog::createClassByName("Runiste"));
    Material normalSalt = MaterialCatalog::createMoonlitSalt(1);
    normalSalt.setQuality("normal");
    Material exceptionalSalt = MaterialCatalog::createMoonlitSalt(1);
    exceptionalSalt.setQuality("exceptional");
    Material normalNail = MaterialCatalog::createColdIronNail(1);
    normalNail.setQuality("normal");
    Material exceptionalNail = MaterialCatalog::createColdIronNail(1);
    exceptionalNail.setQuality("exceptional");
    exactRecipeRunist.getInventory().addMaterial(normalSalt);
    exactRecipeRunist.getInventory().addMaterial(exceptionalSalt);
    exactRecipeRunist.getInventory().addMaterial(normalNail);
    exactRecipeRunist.getInventory().addMaterial(exceptionalNail);
    std::vector<MagicMaterialRules::SpellMaterialSelectedRequirement> exactRecipe = {
        {"moonlit_salt", "exceptional", 1, false},
        {"cold_iron_nail", "exceptional", 1, true}
    };
    assert(MagicMaterialRules::canConsumeSpellMaterialRecipeSelection(exactRecipeRunist.getInventory(), exactRecipe));
    const auto exactProfile = MagicMaterialRules::recipeChannelingProfileForSelection(exactRecipe);
    assert(exactProfile.powerBoostPercent > 5);
    std::vector<std::string> exactRecipeLines;
    assert(MagicMaterialRules::consumeSpellMaterialRecipeSelection(exactRecipeRunist.getInventory(), exactRecipe, exactRecipeLines));
    assert(exactRecipeRunist.getInventory().countMaterialByIdAndQuality("moonlit_salt", "normal") == 1);
    assert(exactRecipeRunist.getInventory().countMaterialByIdAndQuality("moonlit_salt", "exceptional") == 0);
    assert(exactRecipeRunist.getInventory().countMaterialByIdAndQuality("cold_iron_nail", "normal") == 1);
    assert(exactRecipeRunist.getInventory().countMaterialByIdAndQuality("cold_iron_nail", "pure") == 1);

    // Mineral fuel degrades one quality step per real channeling use, then dissolves at the lowest quality.
    Player mineralFuel("RuneFer", ClassCatalog::createClassByName("Runiste"));
    mineralFuel.getInventory().addMaterial(MaterialCatalog::createRunicIronShard(1));
    mineralFuel.restoreManaToFull();
    CombatActions::prepareMagicTechnique(mineralFuel, "Écho canalisé", random, 18);
    assert(mineralFuel.getInventory().countMaterialById("runic_iron_shard") == 1);
    bool foundImpureFuel = false;
    for (const Material& item : mineralFuel.getInventory().getMaterials())
    {
        if (item.getId() == "runic_iron_shard")
        {
            foundImpureFuel = item.getQuality() == "impure" || item.getQuality() == "low";
        }
    }
    assert(foundImpureFuel);
    mineralFuel.restoreManaToFull();
    CombatActions::prepareMagicTechnique(mineralFuel, "Écho canalisé", random, 18);
    assert(mineralFuel.getInventory().countMaterialById("runic_iron_shard") == 0);

    // Once a cast really starts, the selected support is consumed/worn even if the stability roll later fails.
    Player materialAttempt("RuneGo", ClassCatalog::createClassByName("Runiste"));
    materialAttempt.getInventory().addMaterial(MaterialCatalog::createArcaneDust(1));
    materialAttempt.restoreManaToFull();
    CombatActions::prepareMagicTechnique(materialAttempt, "Écho canalisé", random, 18);
    assert(materialAttempt.getInventory().countMaterialById("arcane_dust") == 0);

    Player catalystAttempt("MiraGo", ClassCatalog::createClassByName("Mage"));
    catalystAttempt.getInventory().addWeapon(WeaponCatalog::createSingingResinStaff());
    catalystAttempt.equipWeapon(0);
    catalystAttempt.restoreManaToFull();
    const int durabilityBeforeAttempt = catalystAttempt.getEquippedWeapon().getDurability();
    CombatActions::prepareMagicTechnique(catalystAttempt, "Écho canalisé", random, 18);
    assert(catalystAttempt.getEquippedWeapon().getDurability() == durabilityBeforeAttempt - 1);

    return 0;
}
