// EN: Central rules for physical materials usable by material-based magic.
// FR: Règles centrales des matériaux physiques utilisables par la magie à composants.
#include "combat/MagicMaterialRules.hpp"

#include "item/Inventory.hpp"
#include "item/material/Material.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <unordered_set>

namespace
{
    std::string normalizeElement(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    const std::vector<std::string>& allKnownMagicMaterialIds()
    {
        static const std::vector<std::string> ids = {
            "arcane_dust",
            "runic_iron_shard",
            "moonlit_salt",
            "echoing_resin",
            "kitsune_ember",
            "rare_fire_rune_core",
            "rare_cold_rune_core",
            "cold_iron_nail",
            "sun_dried_clay",
            "holy_water_vial",
            "blue_mist_reed",
            "fitoria_feather",
            "unstable_core",
            "elemental_fusion_core"
        };
        return ids;
    }
}

namespace MagicMaterialRules
{
    const std::vector<std::string>& orderedChannelingMaterialIds()
    {
        static const std::vector<std::string> ids = {
            "arcane_dust",
            "runic_iron_shard",
            "moonlit_salt",
            "echoing_resin"
        };
        return ids;
    }

    bool isChannelingMaterialId(const std::string& id)
    {
        const auto& ids = orderedChannelingMaterialIds();
        return std::find(ids.begin(), ids.end(), id) != ids.end();
    }

    bool isKnownMagicMaterialId(const std::string& id)
    {
        const auto& ids = allKnownMagicMaterialIds();
        return std::find(ids.begin(), ids.end(), id) != ids.end();
    }

    bool isMineralFuelId(const std::string& id)
    {
        return id == "runic_iron_shard"
            || id == "cold_iron_nail"
            || id == "rare_fire_rune_core"
            || id == "rare_cold_rune_core"
            || id == "unstable_core"
            || id == "elemental_fusion_core";
    }

    std::string displayName(const std::string& id)
    {
        return MaterialCatalog::createById(id, 1).getName();
    }

    std::string qualityLabel(const std::string& id, const std::string& quality)
    {
        if (quality == "dissolved") return "dissous";
        return MaterialCatalog::createById(id, 1, quality).getQualityLabel();
    }

    std::string elementDisplayName(const std::string& elementKey)
    {
        const std::string element = normalizeElement(elementKey);
        if (element == "fire" || element == "feu") return "Feu";
        if (element == "cold" || element == "frost" || element == "ice" || element == "froid" || element == "givre" || element == "glace") return "Froid / glace";
        if (element == "earth" || element == "terre") return "Terre";
        if (element == "water" || element == "eau") return "Eau";
        if (element == "wind" || element == "air" || element == "vent") return "Vent";
        if (element == "lightning" || element == "electric" || element == "electro" || element == "foudre" || element == "choc") return "Foudre";
        if (element == "arcane" || element == "arcanique") return "Arcanique";
        return elementKey.empty() ? "Générique" : elementKey;
    }

    const std::vector<std::string>& elementalMaterialIds(const std::string& elementKey)
    {
        static const std::vector<std::string> none;
        static const std::vector<std::string> fire = {"kitsune_ember", "rare_fire_rune_core", "elemental_fusion_core"};
        static const std::vector<std::string> cold = {"rare_cold_rune_core", "cold_iron_nail", "moonlit_salt", "elemental_fusion_core"};
        static const std::vector<std::string> earth = {"runic_iron_shard", "sun_dried_clay", "elemental_fusion_core"};
        static const std::vector<std::string> water = {"holy_water_vial", "blue_mist_reed", "elemental_fusion_core"};
        static const std::vector<std::string> wind = {"echoing_resin", "fitoria_feather", "elemental_fusion_core"};
        static const std::vector<std::string> lightning = {"runic_iron_shard", "unstable_core", "elemental_fusion_core"};
        static const std::vector<std::string> arcane = {"arcane_dust", "runic_iron_shard", "moonlit_salt", "echoing_resin", "unstable_core", "elemental_fusion_core"};

        const std::string element = normalizeElement(elementKey);
        if (element == "fire" || element == "feu") return fire;
        if (element == "cold" || element == "frost" || element == "ice" || element == "froid" || element == "givre" || element == "glace") return cold;
        if (element == "earth" || element == "terre") return earth;
        if (element == "water" || element == "eau") return water;
        if (element == "wind" || element == "air" || element == "vent") return wind;
        if (element == "lightning" || element == "electric" || element == "electro" || element == "foudre" || element == "choc") return lightning;
        if (element == "arcane" || element == "arcanique") return arcane;
        return none;
    }

    bool materialSupportsElement(const std::string& id, const std::string& elementKey)
    {
        const auto& ids = elementalMaterialIds(elementKey);
        return std::find(ids.begin(), ids.end(), id) != ids.end();
    }

    std::vector<std::string> materialElementLabels(const std::string& id)
    {
        std::vector<std::string> labels;
        const std::vector<std::string> keys = {"feu", "froid", "terre", "eau", "vent", "foudre", "arcanique"};
        for (const std::string& key : keys)
        {
            if (materialSupportsElement(id, key))
            {
                labels.push_back(elementDisplayName(key));
            }
        }
        return labels;
    }

    std::vector<std::string> availableMaterialIds(const Inventory& inventory, const std::string& elementKey)
    {
        std::vector<std::string> available;
        std::unordered_set<std::string> alreadyAdded;

        auto appendIfAvailable = [&](const std::string& id) {
            if (alreadyAdded.insert(id).second && inventory.countMaterialById(id) > 0)
            {
                available.push_back(id);
            }
        };

        if (!elementKey.empty())
        {
            for (const std::string& id : elementalMaterialIds(elementKey))
            {
                appendIfAvailable(id);
            }
        }

        for (const std::string& id : orderedChannelingMaterialIds())
        {
            appendIfAvailable(id);
        }

        return available;
    }

    std::string findFirstAvailableMaterialId(const Inventory& inventory)
    {
        const std::vector<std::string> available = availableMaterialIds(inventory);
        return available.empty() ? "" : available.front();
    }

    std::string findFirstAvailableMaterialId(const Inventory& inventory, const std::string& elementKey)
    {
        const std::vector<std::string> available = availableMaterialIds(inventory, elementKey);
        return available.empty() ? "" : available.front();
    }

    int countAvailableMaterials(const Inventory& inventory)
    {
        int total = 0;
        for (const std::string& id : allKnownMagicMaterialIds())
        {
            total += inventory.countMaterialById(id);
        }
        return total;
    }

    std::string nextMaterialQuality(const Inventory& inventory, const std::string& id)
    {
        const Material* selected = nullptr;
        int selectedWeight = 999;
        for (const Material& material : inventory.getMaterials())
        {
            if (material.getId() != id || material.getQuantity() <= 0)
            {
                continue;
            }
            const int weight = material.getQualityCraftWeight();
            if (weight < selectedWeight)
            {
                selected = &material;
                selectedWeight = weight;
            }
        }
        return selected == nullptr ? "" : selected->getQuality();
    }

    std::vector<std::string> availableQualityVariants(const Inventory& inventory, const std::string& id)
    {
        std::vector<std::string> qualities;
        for (const Material& material : inventory.getMaterials())
        {
            if (material.getId() != id || material.getQuantity() <= 0)
            {
                continue;
            }
            if (std::find(qualities.begin(), qualities.end(), material.getQuality()) == qualities.end())
            {
                qualities.push_back(material.getQuality());
            }
        }
        std::sort(qualities.begin(), qualities.end(), [&](const std::string& left, const std::string& right) {
            const int leftWeight = MaterialCatalog::createById(id, 1, left).getQualityCraftWeight();
            const int rightWeight = MaterialCatalog::createById(id, 1, right).getQualityCraftWeight();
            if (leftWeight != rightWeight)
            {
                return leftWeight > rightWeight;
            }
            return left < right;
        });
        return qualities;
    }

    std::string formatMaterialUsePreview(const Inventory& inventory, const std::string& id, bool degradeInsteadOfConsume)
    {
        const int quantity = inventory.countMaterialById(id);
        const std::string quality = nextMaterialQuality(inventory, id);
        std::string line = displayName(id) + " x" + std::to_string(quantity);
        if (!quality.empty())
        {
            line += " | qualité utilisée : " + qualityLabel(id, quality);
        }
        line += degradeInsteadOfConsume
            ? " | effet : qualité -1 palier (dissolution au minimum)"
            : " | effet : 1 unité consommée";
        return line;
    }

    std::string formatMaterialUsePreviewForQuality(const Inventory& inventory, const std::string& id, const std::string& quality, bool degradeInsteadOfConsume)
    {
        const int quantity = inventory.countMaterialByIdAndQuality(id, quality);
        std::string line = displayName(id) + " [" + qualityLabel(id, quality) + "] x" + std::to_string(quantity);
        line += degradeInsteadOfConsume
            ? " | effet : cette unité perd 1 palier de qualité (dissolution au minimum)"
            : " | effet : 1 unité de cette qualité est consommée";
        return line;
    }

    int materialRarityTier(const std::string& id)
    {
        static const std::unordered_map<std::string, int> tiers = {
            {"arcane_dust", 1}, {"moonlit_salt", 1}, {"sun_dried_clay", 1},
            {"holy_water_vial", 1}, {"blue_mist_reed", 1},
            {"runic_iron_shard", 2}, {"echoing_resin", 2}, {"cold_iron_nail", 2},
            {"kitsune_ember", 3}, {"fitoria_feather", 3},
            {"rare_fire_rune_core", 4}, {"rare_cold_rune_core", 4}, {"unstable_core", 4},
            {"elemental_fusion_core", 5}
        };
        const auto it = tiers.find(id);
        return it == tiers.end() ? 1 : std::clamp(it->second, 1, 5);
    }

    std::string materialRarityLabel(const std::string& id)
    {
        switch (materialRarityTier(id))
        {
            case 5: return "relique";
            case 4: return "très rare";
            case 3: return "rare";
            case 2: return "peu commun";
            default: return "commun";
        }
    }

    MaterialChannelingProfile channelingProfileForQuality(const std::string& id, const std::string& quality, bool sacrificeMaterial)
    {
        MaterialChannelingProfile profile;
        profile.rarityTier = materialRarityTier(id);
        profile.qualityWeight = quality.empty()
            ? 2
            : MaterialCatalog::createById(id, 1, quality).getQualityCraftWeight();
        profile.qualityWeight = std::clamp(profile.qualityWeight, 1, 4);

        if (sacrificeMaterial)
        {
            // Burst mode: less efficient over time, but all remaining quality of one physical unit
            // is converted into an immediate magical surge. Rare and pristine supports matter more.
            profile.manaPercent = std::clamp(90 - profile.qualityWeight * 3 - profile.rarityTier * 2, 65, 82);
            profile.stabilityBonus = std::clamp(8 + profile.qualityWeight * 3 + profile.rarityTier * 2, 12, 30);
            profile.powerBoostPercent = std::clamp(15 + profile.qualityWeight * 8 + profile.rarityTier * 5, 25, 75);
            return profile;
        }

        profile.manaPercent = std::clamp(52 - profile.qualityWeight * 3 - (profile.rarityTier - 1), 34, 50);
        profile.stabilityBonus = std::clamp(3 + profile.qualityWeight * 2 + profile.rarityTier, 6, 18);
        profile.powerBoostPercent = std::clamp((profile.qualityWeight - 1) * 3 + (profile.rarityTier - 1) * 2, 0, 20);
        return profile;
    }

    MaterialChannelingProfile channelingProfile(const Inventory& inventory, const std::string& id, bool sacrificeMaterial)
    {
        return channelingProfileForQuality(id, nextMaterialQuality(inventory, id), sacrificeMaterial);
    }

    MaterialChannelingProfile recipeChannelingProfileForSelection(const std::vector<SpellMaterialSelectedRequirement>& selection)
    {
        MaterialChannelingProfile result;
        if (selection.empty())
        {
            return result;
        }

        int manaTotal = 0;
        int stabilityTotal = 0;
        int powerTotal = 0;
        int rarityTotal = 0;
        int qualityTotal = 0;
        int counted = 0;
        for (const SpellMaterialSelectedRequirement& selected : selection)
        {
            if (selected.materialId.empty() || selected.quality.empty() || selected.quantity <= 0)
            {
                continue;
            }
            const MaterialChannelingProfile part = channelingProfileForQuality(selected.materialId, selected.quality, false);
            for (int i = 0; i < selected.quantity; ++i)
            {
                manaTotal += part.manaPercent;
                stabilityTotal += part.stabilityBonus;
                powerTotal += part.powerBoostPercent;
                rarityTotal += part.rarityTier;
                qualityTotal += part.qualityWeight;
                ++counted;
            }
        }

        if (counted <= 0)
        {
            return result;
        }

        result.manaPercent = std::clamp(manaTotal / counted - 3, 32, 48);
        result.stabilityBonus = std::clamp(stabilityTotal / counted + 3, 8, 22);
        result.powerBoostPercent = std::clamp(powerTotal / counted + 5, 5, 25);
        result.rarityTier = std::clamp(rarityTotal / counted, 1, 5);
        result.qualityWeight = std::clamp(qualityTotal / counted, 1, 4);
        return result;
    }

    MaterialChannelingProfile recipeChannelingProfile(const Inventory& inventory, const std::string& spellName)
    {
        const SpellMaterialRecipe* recipe = findSpellMaterialRecipe(spellName);
        if (recipe == nullptr)
        {
            return {};
        }

        std::vector<SpellMaterialSelectedRequirement> selection;
        for (const SpellMaterialRequirement& requirement : recipe->requirements)
        {
            const std::string quality = nextMaterialQuality(inventory, requirement.materialId);
            if (quality.empty())
            {
                continue;
            }
            selection.push_back({requirement.materialId, quality, requirement.quantity, requirement.degradeInsteadOfConsume});
        }
        return recipeChannelingProfileForSelection(selection);
    }

    std::string formatChannelingProfilePreview(const Inventory& inventory, const std::string& id, bool sacrificeMaterial)
    {
        const MaterialChannelingProfile profile = channelingProfile(inventory, id, sacrificeMaterial);
        std::string text = "rareté : " + materialRarityLabel(id)
            + " | mana : " + std::to_string(profile.manaPercent) + "% du coût de base"
            + " | stabilité : +" + std::to_string(profile.stabilityBonus)
            + " | puissance : +" + std::to_string(profile.powerBoostPercent) + "%";
        if (sacrificeMaterial)
        {
            text += " | l'unité entière sera détruite";
        }
        return text;
    }

    std::string formatChannelingProfilePreviewForQuality(const std::string& id, const std::string& quality, bool sacrificeMaterial)
    {
        const MaterialChannelingProfile profile = channelingProfileForQuality(id, quality, sacrificeMaterial);
        std::string text = "qualité : " + qualityLabel(id, quality)
            + " | rareté : " + materialRarityLabel(id)
            + " | mana : " + std::to_string(profile.manaPercent) + "% du coût de base"
            + " | stabilité : +" + std::to_string(profile.stabilityBonus)
            + " | puissance : +" + std::to_string(profile.powerBoostPercent) + "%";
        if (sacrificeMaterial)
        {
            text += " | l'unité entière sera détruite";
        }
        return text;
    }

    std::string formatAvailableMaterialStocks(const Inventory& inventory)
    {
        std::string details;
        for (const std::string& id : allKnownMagicMaterialIds())
        {
            const int quantity = inventory.countMaterialById(id);
            if (quantity <= 0)
            {
                continue;
            }

            if (!details.empty())
            {
                details += ", ";
            }
            details += displayName(id) + " x" + std::to_string(quantity);
            const std::string quality = nextMaterialQuality(inventory, id);
            if (!quality.empty())
            {
                details += " [" + qualityLabel(id, quality) + "]";
            }
            if (isMineralFuelId(id))
            {
                details += " (carburant minéral)";
            }
        }
        return details;
    }

    const std::vector<SpellMaterialRecipe>& spellMaterialRecipes()
    {
        static const std::vector<SpellMaterialRecipe> recipes = {
            {
                "Aiguille de givre",
                {
                    {"moonlit_salt", 1, false},
                    {"cold_iron_nail", 1, true}
                }
            },
            {
                "Surcharge contrôlée",
                {
                    {"arcane_dust", 1, false},
                    {"runic_iron_shard", 1, true}
                }
            }
        };
        return recipes;
    }

    const SpellMaterialRecipe* findSpellMaterialRecipe(const std::string& spellName)
    {
        const auto& recipes = spellMaterialRecipes();
        const auto it = std::find_if(recipes.begin(), recipes.end(), [&](const SpellMaterialRecipe& recipe) {
            return recipe.spellName == spellName;
        });
        return it == recipes.end() ? nullptr : &(*it);
    }

    bool hasSpellMaterialRecipeIngredients(const Inventory& inventory, const std::string& spellName)
    {
        const SpellMaterialRecipe* recipe = findSpellMaterialRecipe(spellName);
        if (recipe == nullptr)
        {
            return false;
        }

        for (const SpellMaterialRequirement& requirement : recipe->requirements)
        {
            if (inventory.countMaterialById(requirement.materialId) < requirement.quantity)
            {
                return false;
            }
        }
        return true;
    }

    std::vector<std::string> missingSpellMaterialIngredients(const Inventory& inventory, const std::string& spellName)
    {
        std::vector<std::string> missing;
        const SpellMaterialRecipe* recipe = findSpellMaterialRecipe(spellName);
        if (recipe == nullptr)
        {
            return missing;
        }

        for (const SpellMaterialRequirement& requirement : recipe->requirements)
        {
            const int available = inventory.countMaterialById(requirement.materialId);
            if (available < requirement.quantity)
            {
                missing.push_back(
                    displayName(requirement.materialId)
                    + " x" + std::to_string(requirement.quantity - available)
                );
            }
        }
        return missing;
    }

    std::vector<std::string> formatSpellMaterialRecipeSelection(const std::vector<SpellMaterialSelectedRequirement>& selection, const std::string& spellName)
    {
        std::vector<std::string> lines;
        if (selection.empty())
        {
            return lines;
        }

        lines.push_back("Recette : " + spellName + ".");
        for (const SpellMaterialSelectedRequirement& selected : selection)
        {
            std::string line = "- " + displayName(selected.materialId) + " x" + std::to_string(selected.quantity);
            if (!selected.quality.empty())
            {
                line += " [" + qualityLabel(selected.materialId, selected.quality) + "]";
            }
            line += selected.degradeInsteadOfConsume ? " -> qualité dégradée" : " -> sacrifié";
            lines.push_back(line);
        }
        return lines;
    }

    std::vector<std::string> formatSpellMaterialRecipe(const Inventory& inventory, const std::string& spellName)
    {
        std::vector<std::string> lines;
        const SpellMaterialRecipe* recipe = findSpellMaterialRecipe(spellName);
        if (recipe == nullptr)
        {
            return lines;
        }

        lines.push_back("Recette : " + recipe->spellName + ".");
        for (const SpellMaterialRequirement& requirement : recipe->requirements)
        {
            std::string line = "- " + displayName(requirement.materialId) + " x" + std::to_string(requirement.quantity);
            const std::string quality = nextMaterialQuality(inventory, requirement.materialId);
            if (!quality.empty())
            {
                line += " [prochaine qualité : " + qualityLabel(requirement.materialId, quality) + "]";
            }
            line += requirement.degradeInsteadOfConsume ? " -> qualité dégradée" : " -> sacrifié";
            if (inventory.countMaterialById(requirement.materialId) < requirement.quantity)
            {
                line += " [MANQUANT]";
            }
            lines.push_back(line);
        }
        return lines;
    }

    bool canConsumeSpellMaterialRecipeSelection(const Inventory& inventory, const std::vector<SpellMaterialSelectedRequirement>& selection)
    {
        if (selection.empty())
        {
            return false;
        }

        std::unordered_map<std::string, int> requiredByStack;
        for (const SpellMaterialSelectedRequirement& selected : selection)
        {
            if (selected.materialId.empty() || selected.quality.empty() || selected.quantity <= 0)
            {
                return false;
            }
            requiredByStack[selected.materialId + "\n" + selected.quality] += selected.quantity;
        }

        for (const auto& [stackKey, quantity] : requiredByStack)
        {
            const std::size_t split = stackKey.find('\n');
            const std::string materialId = stackKey.substr(0, split);
            const std::string quality = split == std::string::npos ? "" : stackKey.substr(split + 1);
            if (inventory.countMaterialByIdAndQuality(materialId, quality) < quantity)
            {
                return false;
            }
        }
        return true;
    }

    bool consumeSpellMaterialRecipeSelection(Inventory& inventory, const std::vector<SpellMaterialSelectedRequirement>& selection, std::vector<std::string>& resultLines)
    {
        resultLines.clear();
        if (!canConsumeSpellMaterialRecipeSelection(inventory, selection))
        {
            return false;
        }

        for (const SpellMaterialSelectedRequirement& selected : selection)
        {
            for (int i = 0; i < selected.quantity; ++i)
            {
                if (selected.degradeInsteadOfConsume)
                {
                    std::string previousQuality;
                    std::string resultingQuality;
                    bool dissolved = false;
                    if (!inventory.degradeMaterialQualityByIdAndQuality(selected.materialId, selected.quality, previousQuality, resultingQuality, dissolved))
                    {
                        return false;
                    }
                    if (dissolved)
                    {
                        resultLines.push_back(displayName(selected.materialId) + " [" + qualityLabel(selected.materialId, previousQuality) + "] se dissout après avoir alimenté la recette.");
                    }
                    else
                    {
                        resultLines.push_back(
                            displayName(selected.materialId) + " : "
                            + qualityLabel(selected.materialId, previousQuality) + " -> "
                            + qualityLabel(selected.materialId, resultingQuality) + "."
                        );
                    }
                }
                else
                {
                    if (!inventory.removeMaterialQuantityByIdAndQuality(selected.materialId, selected.quality, 1))
                    {
                        return false;
                    }
                    resultLines.push_back(displayName(selected.materialId) + " [" + qualityLabel(selected.materialId, selected.quality) + "] est sacrifié par la recette.");
                }
            }
        }
        return true;
    }

    bool consumeSpellMaterialRecipe(Inventory& inventory, const std::string& spellName, std::vector<std::string>& resultLines)
    {
        const SpellMaterialRecipe* recipe = findSpellMaterialRecipe(spellName);
        if (recipe == nullptr || !hasSpellMaterialRecipeIngredients(inventory, spellName))
        {
            resultLines.clear();
            return false;
        }

        std::vector<SpellMaterialSelectedRequirement> selection;
        for (const SpellMaterialRequirement& requirement : recipe->requirements)
        {
            const std::string quality = nextMaterialQuality(inventory, requirement.materialId);
            if (quality.empty())
            {
                resultLines.clear();
                return false;
            }
            selection.push_back({requirement.materialId, quality, requirement.quantity, requirement.degradeInsteadOfConsume});
        }
        return consumeSpellMaterialRecipeSelection(inventory, selection, resultLines);
    }

}
