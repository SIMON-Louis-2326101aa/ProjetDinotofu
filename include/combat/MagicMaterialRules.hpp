// EN: Central rules for physical materials usable by material-based magic.
// FR: Règles centrales des matériaux physiques utilisables par la magie à composants.
#ifndef INCLUDE_COMBAT_MAGICMATERIALRULES_HPP
#define INCLUDE_COMBAT_MAGICMATERIALRULES_HPP

#include <string>
#include <vector>

class Inventory;

namespace MagicMaterialRules
{
    struct MaterialChannelingProfile
    {
        int manaPercent = 45;
        int stabilityBonus = 0;
        int powerBoostPercent = 0;
        int rarityTier = 1;
        int qualityWeight = 2;
    };

    struct SpellMaterialRequirement
    {
        std::string materialId;
        int quantity;
        bool degradeInsteadOfConsume;
    };

    struct SpellMaterialRecipe
    {
        std::string spellName;
        std::vector<SpellMaterialRequirement> requirements;
    };

    struct SpellMaterialSelectedRequirement
    {
        std::string materialId;
        std::string quality;
        int quantity = 1;
        bool degradeInsteadOfConsume = false;
    };

    const std::vector<std::string>& orderedChannelingMaterialIds();
    bool isChannelingMaterialId(const std::string& id);
    bool isKnownMagicMaterialId(const std::string& id);
    bool isMineralFuelId(const std::string& id);
    std::string displayName(const std::string& id);
    std::string qualityLabel(const std::string& id, const std::string& quality);
    std::string elementDisplayName(const std::string& elementKey);

    std::string findFirstAvailableMaterialId(const Inventory& inventory);
    std::string findFirstAvailableMaterialId(const Inventory& inventory, const std::string& elementKey);
    std::vector<std::string> availableMaterialIds(const Inventory& inventory, const std::string& elementKey = "");
    int countAvailableMaterials(const Inventory& inventory);
    std::string formatAvailableMaterialStocks(const Inventory& inventory);
    std::string nextMaterialQuality(const Inventory& inventory, const std::string& id);
    std::vector<std::string> availableQualityVariants(const Inventory& inventory, const std::string& id);
    std::string formatMaterialUsePreview(const Inventory& inventory, const std::string& id, bool degradeInsteadOfConsume);
    std::string formatMaterialUsePreviewForQuality(const Inventory& inventory, const std::string& id, const std::string& quality, bool degradeInsteadOfConsume);

    int materialRarityTier(const std::string& id);
    std::string materialRarityLabel(const std::string& id);
    MaterialChannelingProfile channelingProfile(const Inventory& inventory, const std::string& id, bool sacrificeMaterial = false);
    MaterialChannelingProfile channelingProfileForQuality(const std::string& id, const std::string& quality, bool sacrificeMaterial = false);
    MaterialChannelingProfile recipeChannelingProfile(const Inventory& inventory, const std::string& spellName);
    MaterialChannelingProfile recipeChannelingProfileForSelection(const std::vector<SpellMaterialSelectedRequirement>& selection);
    std::string formatChannelingProfilePreview(const Inventory& inventory, const std::string& id, bool sacrificeMaterial = false);
    std::string formatChannelingProfilePreviewForQuality(const std::string& id, const std::string& quality, bool sacrificeMaterial = false);

    const std::vector<std::string>& elementalMaterialIds(const std::string& elementKey);
    bool materialSupportsElement(const std::string& id, const std::string& elementKey);
    std::vector<std::string> materialElementLabels(const std::string& id);

    const std::vector<SpellMaterialRecipe>& spellMaterialRecipes();
    const SpellMaterialRecipe* findSpellMaterialRecipe(const std::string& spellName);
    bool hasSpellMaterialRecipeIngredients(const Inventory& inventory, const std::string& spellName);
    std::vector<std::string> missingSpellMaterialIngredients(const Inventory& inventory, const std::string& spellName);
    std::vector<std::string> formatSpellMaterialRecipe(const Inventory& inventory, const std::string& spellName);
    std::vector<std::string> formatSpellMaterialRecipeSelection(const std::vector<SpellMaterialSelectedRequirement>& selection, const std::string& spellName);
    bool canConsumeSpellMaterialRecipeSelection(const Inventory& inventory, const std::vector<SpellMaterialSelectedRequirement>& selection);
    bool consumeSpellMaterialRecipeSelection(Inventory& inventory, const std::vector<SpellMaterialSelectedRequirement>& selection, std::vector<std::string>& resultLines);
    bool consumeSpellMaterialRecipe(Inventory& inventory, const std::string& spellName, std::vector<std::string>& resultLines);
}

#endif
