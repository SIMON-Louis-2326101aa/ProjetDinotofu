// EN: InventorySelection.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: InventorySelection.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/inventory/InventorySelection.hpp"

#include "combat/threat/ThreatSystem.hpp"

#include "core/Console.hpp"

#include "interface/menu/inventory/InventoryDisplay.hpp"
#include "interface/menu/inventory/InventoryUtils.hpp"
#include "interface/menu/common/MenuFrame.hpp"
#include "interface/menu/progression/BestiaryMenu.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "progression/material/MaterialExperimentLog.hpp"

#include "item/Inventory.hpp"
#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/consumable/ConsumableType.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <vector>


namespace
{
    bool hasRecipeIngredients(Player& player, const std::string& id, int quantity);
    bool consumeRecipeIngredient(Player& player, const std::string& id, int quantity);

    std::string normalizeText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });

        return value;
    }

    // EN: playerClassIs declares or implements a focused behavior used by this module.
    // FR: playerClassIs déclare ou implémente un comportement précis utilisé par ce module.
    bool playerClassIs(const Player& player, const std::string& expectedClass)
    {
        return normalizeText(player.getType()) == normalizeText(expectedClass);
    }

    // EN: isAlchemist declares or implements a focused behavior used by this module.
    // FR: isAlchemist déclare ou implémente un comportement précis utilisé par ce module.
    bool isAlchemist(const Player& player)
    {
        return playerClassIs(player, "Alchimiste");
    }

    // EN: isBlacksmith declares or implements a focused behavior used by this module.
    // FR: isBlacksmith déclare ou implémente un comportement précis utilisé par ce module.
    bool isBlacksmith(const Player& player)
    {
        return playerClassIs(player, "Forgeron");
    }

    // EN: isArtificer declares or implements a focused behavior used by this module.
    // FR: isArtificer déclare ou implémente un comportement précis utilisé par ce module.
    bool isArtificer(const Player& player)
    {
        return playerClassIs(player, "Artificier");
    }

    // EN: exceptionalCraftChanceFor declares or implements a focused behavior used by this module.
    // FR: exceptionalCraftChanceFor déclare ou implémente un comportement précis utilisé par ce module.
    int exceptionalCraftChanceFor(const Player& player, const std::string& craftKind)
    {
        int chance = 12;

        if (craftKind == "consumable" && isAlchemist(player))
        {
            chance += 10;
        }
        else if ((craftKind == "weapon" || craftKind == "armor") && isBlacksmith(player))
        {
            chance += 10;
        }
        else if (craftKind == "tool" && isArtificer(player))
        {
            chance += 10;
        }

        if (isArtificer(player) && craftKind != "consumable")
        {
            chance += 4;
        }

        return std::min(35, chance);
    }

    // EN: startsWith declares or implements a focused behavior used by this module.
    // FR: startsWith déclare ou implémente un comportement précis utilisé par ce module.
    bool startsWith(const std::string& value, const std::string& prefix)
    {
        return value.rfind(prefix, 0) == 0;
    }

    // EN: isRepairKitId declares or implements a focused behavior used by this module.
    // FR: isRepairKitId déclare ou implémente un comportement précis utilisé par ce module.
    bool isRepairKitId(const std::string& kitId)
    {
        return kitId == "weak_repair_kit"
            || kitId == "medium_repair_kit"
            || kitId == "big_repair_kit"
            || kitId == "tinkerer_complete_repair_kit"
            || kitId == "small_repair_kit"
            || kitId == "reinforced_repair_kit"
            || startsWith(kitId, "weak_repair_kit_used_")
            || startsWith(kitId, "medium_repair_kit_used_")
            || startsWith(kitId, "big_repair_kit_used_")
            || startsWith(kitId, "tinkerer_complete_repair_kit_used_");
    }

    std::string repairKitBaseId(const std::string& kitId)
    {
        if (kitId == "small_repair_kit") return "weak_repair_kit";
        if (kitId == "reinforced_repair_kit") return "medium_repair_kit";
        if (startsWith(kitId, "weak_repair_kit")) return "weak_repair_kit";
        if (startsWith(kitId, "medium_repair_kit")) return "medium_repair_kit";
        if (startsWith(kitId, "big_repair_kit")) return "big_repair_kit";
        if (startsWith(kitId, "tinkerer_complete_repair_kit")) return "tinkerer_complete_repair_kit";
        return kitId;
    }

    // EN: isUsedRepairKitId declares or implements a focused behavior used by this module.
    // FR: isUsedRepairKitId déclare ou implémente un comportement précis utilisé par ce module.
    bool isUsedRepairKitId(const std::string& kitId)
    {
        return startsWith(kitId, "weak_repair_kit_used_")
            || startsWith(kitId, "medium_repair_kit_used_")
            || startsWith(kitId, "big_repair_kit_used_")
            || startsWith(kitId, "tinkerer_complete_repair_kit_used_");
    }

    // EN: repairKitMaxDurability declares or implements a focused behavior used by this module.
    // FR: repairKitMaxDurability déclare ou implémente un comportement précis utilisé par ce module.
    int repairKitMaxDurability(const std::string& kitId)
    {
        std::string baseId = repairKitBaseId(kitId);
        if (baseId == "medium_repair_kit") return 3;
        if (baseId == "big_repair_kit") return 4;
        if (baseId == "tinkerer_complete_repair_kit") return 5;
        return 2;
    }

    // EN: repairKitCurrentDurability declares or implements a focused behavior used by this module.
    // FR: repairKitCurrentDurability déclare ou implémente un comportement précis utilisé par ce module.
    int repairKitCurrentDurability(const std::string& kitId)
    {
        if (!isUsedRepairKitId(kitId))
        {
            return repairKitMaxDurability(kitId);
        }

        std::size_t pos = kitId.find_last_of('_');

        if (pos == std::string::npos || pos + 1 >= kitId.size())
        {
            return 1;
        }

        try
        {
            return std::max(1, std::stoi(kitId.substr(pos + 1)));
        }
        catch (...)
        {
            return 1;
        }
    }

    std::string repairKitUsedId(const std::string& kitId, int remainingDurability)
    {
        return repairKitBaseId(kitId) + "_used_" + std::to_string(remainingDurability);
    }

    // EN: repairThresholdPercentForKit declares or implements a focused behavior used by this module.
    // FR: repairThresholdPercentForKit déclare ou implémente un comportement précis utilisé par ce module.
    int repairThresholdPercentForKit(const std::string& kitId)
    {
        std::string baseId = repairKitBaseId(kitId);
        if (baseId == "medium_repair_kit") return 50;
        if (baseId == "big_repair_kit") return 75;
        if (baseId == "tinkerer_complete_repair_kit") return 95;
        return 25;
    }

    std::string repairKitLabel(const std::string& kitId)
    {
        std::string baseId = repairKitBaseId(kitId);
        if (baseId == "medium_repair_kit") return "Kit de réparation moyen";
        if (baseId == "big_repair_kit") return "Gros kit de réparation";
        if (baseId == "tinkerer_complete_repair_kit") return "Kit complet du bricoleur";
        return "Kit de réparation faible";
    }


    // EN: rollPercent declares or implements a focused behavior used by this module.
    // FR: rollPercent déclare ou implémente un comportement précis utilisé par ce module.
    bool rollPercent(int percent)
    {
        // EN: generator declares or implements a focused behavior used by this module.
        // FR: generator déclare ou implémente un comportement précis utilisé par ce module.
        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(1, 100);
        return distribution(generator) <= percent;
    }

    // EN: blacksmithSaveKitDurabilityChance declares or implements a focused behavior used by this module.
    // FR: blacksmithSaveKitDurabilityChance déclare ou implémente un comportement précis utilisé par ce module.
    int blacksmithSaveKitDurabilityChance(const Player& player)
    {
        return isBlacksmith(player) ? 60 : 0;
    }

    // EN: alchemistSaveCatalystChance declares or implements a focused behavior used by this module.
    // FR: alchemistSaveCatalystChance déclare ou implémente un comportement précis utilisé par ce module.
    int alchemistSaveCatalystChance(const Player& player)
    {
        return isAlchemist(player) ? 60 : 0;
    }

    // EN: maybeRestoreAlchemistCatalyst declares or implements a focused behavior used by this module.
    // FR: maybeRestoreAlchemistCatalyst déclare ou implémente un comportement précis utilisé par ce module.
    void maybeRestoreAlchemistCatalyst(Player& player, const std::string& catalystId, const std::string& catalystName)
    {
        int saveChance = alchemistSaveCatalystChance(player);

        if (saveChance > 0 && rollPercent(saveChance))
        {
            player.getInventory().addMaterial(MaterialCatalog::createById(catalystId, 1));
            std::cout << "Spécial Alchimiste : tu récupères/économises " << catalystName << " pendant la préparation." << std::endl;
        }
    }

    // EN: consumeRepairKitDurability declares or implements a focused behavior used by this module.
    // FR: consumeRepairKitDurability déclare ou implémente un comportement précis utilisé par ce module.
    bool consumeRepairKitDurability(Player& player, const std::string& kitId)
    {
        if (!player.getInventory().removeMaterialQuantityById(kitId, 1))
        {
            return false;
        }

        int remainingDurability = repairKitCurrentDurability(kitId) - 1;

        if (remainingDurability > 0)
        {
            player.getInventory().addMaterial(MaterialCatalog::createById(repairKitUsedId(kitId, remainingDurability), 1));
        }

        return true;
    }

    // EN: repairedDurabilityCap declares or implements a focused behavior used by this module.
    // FR: repairedDurabilityCap déclare ou implémente un comportement précis utilisé par ce module.
    int repairedDurabilityCap(int maxDurability, int thresholdPercent)
    {
        if (maxDurability <= 0)
        {
            return maxDurability;
        }

        int cap = (maxDurability * thresholdPercent) / 100;

        if (cap < 1)
        {
            cap = 1;
        }

        return cap;
    }

    // EN: hasRepairMaterials declares or implements a focused behavior used by this module.
    // FR: hasRepairMaterials déclare ou implémente un comportement précis utilisé par ce module.
    bool hasRepairMaterials(Player& player, bool armorRepair, int thresholdPercent)
    {
        if (armorRepair)
        {
            int leatherNeeded = thresholdPercent >= 75 ? 2 : 1;

            if (!hasRecipeIngredients(player, "worn_leather_piece", leatherNeeded))
            {
                return false;
            }

            if (thresholdPercent >= 75 && !hasRecipeIngredients(player, "beast_hide", 1))
            {
                return false;
            }
        }
        else
        {
            int metalNeeded = thresholdPercent >= 75 ? 2 : 1;

            if (!hasRecipeIngredients(player, "rusted_metal_fragment", metalNeeded))
            {
                return false;
            }
        }

        if (thresholdPercent >= 50 && !hasRecipeIngredients(player, "slime_residue", 1))
        {
            return false;
        }

        if (thresholdPercent >= 75 && !hasRecipeIngredients(player, "arcane_dust", 1))
        {
            return false;
        }

        if (thresholdPercent >= 95 && !hasRecipeIngredients(player, "draconic_scale_fragment", 1))
        {
            return false;
        }

        return true;
    }

    // EN: displayRepairMaterialCost declares or implements a focused behavior used by this module.
    // FR: displayRepairMaterialCost déclare ou implémente un comportement précis utilisé par ce module.
    void displayRepairMaterialCost(bool armorRepair, int thresholdPercent)
    {
        if (armorRepair)
        {
            std::cout << (thresholdPercent >= 75 ? "2 Morceaux de cuir abîmé" : "1 Morceau de cuir abîmé");

            if (thresholdPercent >= 75)
            {
                std::cout << " + 1 Peau de bête robuste";
            }
        }
        else
        {
            std::cout << (thresholdPercent >= 75 ? "2 Fragments de métal rouillé" : "1 Fragment de métal rouillé");
        }

        if (thresholdPercent >= 50)
        {
            std::cout << " + 1 Résidu de slime";
        }

        if (thresholdPercent >= 75)
        {
            std::cout << " + 1 Poussière arcanique";
        }

        if (thresholdPercent >= 95)
        {
            std::cout << " + 1 Fragment d'écaille draconique";
        }
    }


    // EN: displayRepairMaterialLine declares or implements a focused behavior used by this module.
    // FR: displayRepairMaterialLine déclare ou implémente un comportement précis utilisé par ce module.
    void displayRepairMaterialLine(Player& player, const std::string& id, const std::string& name, int needed)
    {
        std::cout << "- " << name << " x" << needed
                  << " (possédé : " << player.getInventory().countMaterialById(id)
                  << ", équiv. qualité normale : "
                  << player.getInventory().countMaterialQualityPointsById(id) / 2
                  << ")" << std::endl;
    }

    // EN: displayRepairMaterialCostDetailed declares or implements a focused behavior used by this module.
    // FR: displayRepairMaterialCostDetailed déclare ou implémente un comportement précis utilisé par ce module.
    void displayRepairMaterialCostDetailed(Player& player, bool armorRepair, int thresholdPercent)
    {
        if (armorRepair)
        {
            displayRepairMaterialLine(player, "worn_leather_piece", "Morceau de cuir abîmé", thresholdPercent >= 75 ? 2 : 1);

            if (thresholdPercent >= 75)
            {
                displayRepairMaterialLine(player, "beast_hide", "Peau de bête robuste", 1);
            }
        }
        else
        {
            displayRepairMaterialLine(player, "rusted_metal_fragment", "Fragment de métal rouillé", thresholdPercent >= 75 ? 2 : 1);
        }

        if (thresholdPercent >= 50)
        {
            displayRepairMaterialLine(player, "slime_residue", "Résidu de slime", 1);
        }

        if (thresholdPercent >= 75)
        {
            displayRepairMaterialLine(player, "arcane_dust", "Poussière arcanique", 1);
        }

        if (thresholdPercent >= 95)
        {
            displayRepairMaterialLine(player, "draconic_scale_fragment", "Fragment d'écaille draconique", 1);
        }
    }

    // EN: consumeRepairMaterials declares or implements a focused behavior used by this module.
    // FR: consumeRepairMaterials déclare ou implémente un comportement précis utilisé par ce module.
    void consumeRepairMaterials(Player& player, bool armorRepair, int thresholdPercent)
    {
        if (armorRepair)
        {
            consumeRecipeIngredient(player, "worn_leather_piece", thresholdPercent >= 75 ? 2 : 1);

            if (thresholdPercent >= 75)
            {
                consumeRecipeIngredient(player, "beast_hide", 1);
            }
        }
        else
        {
            consumeRecipeIngredient(player, "rusted_metal_fragment", thresholdPercent >= 75 ? 2 : 1);
        }

        if (thresholdPercent >= 50)
        {
            consumeRecipeIngredient(player, "slime_residue", 1);
        }

        if (thresholdPercent >= 75)
        {
            consumeRecipeIngredient(player, "arcane_dust", 1);
        }

        if (thresholdPercent >= 95)
        {
            consumeRecipeIngredient(player, "draconic_scale_fragment", 1);
        }
    }

    struct RepairKitChoice
    {
        std::string id;
        std::string label;
        int quantity;
        int currentDurability;
        int maxDurability;
        int thresholdPercent;
    };

    std::vector<RepairKitChoice> getRepairKitChoices(const Player& player)
    {
        std::vector<RepairKitChoice> choices;

        for (const Material& material : player.getInventory().getMaterials())
        {
            if (!isRepairKitId(material.getId()) || material.getQuantity() <= 0)
            {
                continue;
            }

            RepairKitChoice choice;
            choice.id = material.getId();
            choice.label = repairKitLabel(material.getId());
            choice.quantity = material.getQuantity();
            choice.currentDurability = repairKitCurrentDurability(material.getId());
            choice.maxDurability = repairKitMaxDurability(material.getId());
            choice.thresholdPercent = repairThresholdPercentForKit(material.getId());
            choices.push_back(choice);
        }

        return choices;
    }


    // EN: hasAnyRepairKit declares or implements a focused behavior used by this module.
    // FR: hasAnyRepairKit déclare ou implémente un comportement précis utilisé par ce module.
    bool hasAnyRepairKit(const Player& player)
    {
        return !getRepairKitChoices(player).empty();
    }

    // EN: displayRepairKitsAsConsumables declares or implements a focused behavior used by this module.
    // FR: displayRepairKitsAsConsumables déclare ou implémente un comportement précis utilisé par ce module.
    void displayRepairKitsAsConsumables(const Player& player)
    {
        std::vector<RepairKitChoice> kits = getRepairKitChoices(player);

        if (kits.empty())
        {
            return;
        }

        std::cout << std::endl;
        std::cout << "--- Kits de réparation ---" << std::endl;

        for (const RepairKitChoice& kit : kits)
        {
            std::cout << "- " << kit.label;

            if (isUsedRepairKitId(kit.id))
            {
                std::cout << " entamé [" << kit.currentDurability << "/" << kit.maxDurability << "]";
            }
            else
            {
                std::cout << " intact [" << kit.maxDurability << "/" << kit.maxDurability << "]";
            }

            std::cout << " x" << kit.quantity
                      << " | réparation max : +" << kit.thresholdPercent << "%"
                      << std::endl;
        }

        std::cout << "Pour les utiliser : sélectionne une arme ou une armure, puis choisis Réparer." << std::endl;
    }

    // EN: chooseRepairKit declares or implements a focused behavior used by this module.
    // FR: chooseRepairKit déclare ou implémente un comportement précis utilisé par ce module.
    int chooseRepairKit(Player& player)
    {
        std::vector<RepairKitChoice> choices = getRepairKitChoices(player);

        if (choices.empty())
        {
            std::cout << "Tu n'as aucun kit de réparation disponible." << std::endl;
            std::cout << "Les kits restent listés avec les consommables, mais la réparation se lance depuis une arme ou une armure." << std::endl;
            std::cout << std::endl;
            return -1;
        }

        std::cout << "========== CHOIX DU KIT ==========" << std::endl;
        std::cout << "0 : Annuler" << std::endl;

        for (int i = 0; i < static_cast<int>(choices.size()); ++i)
        {
            std::cout << i + 1 << " : " << choices[i].label;

            if (isUsedRepairKitId(choices[i].id))
            {
                std::cout << " entamé [" << choices[i].currentDurability << "/" << choices[i].maxDurability << "]";
            }
            else
            {
                std::cout << " intact [" << choices[i].maxDurability << "/" << choices[i].maxDurability << "]";
            }

            std::cout << " x" << choices[i].quantity
                      << " | seuil : +" << choices[i].thresholdPercent << "%"
                      << std::endl;
        }

        std::cout << "==================================" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, static_cast<int>(choices.size()), "Choix invalide.");
        std::cout << std::endl;

        if (choice == 0)
        {
            return -1;
        }

        return choice - 1;
    }

    // EN: confirmRepairCost declares or implements a focused behavior used by this module.
    // FR: confirmRepairCost déclare ou implémente un comportement précis utilisé par ce module.
    bool confirmRepairCost(Player& player, const std::string& itemName, bool armorRepair, const RepairKitChoice& kitChoice)
    {
        std::cout << "========== RÉPARATION ==========" << std::endl;
        std::cout << "Équipement : " << itemName << std::endl;
        std::cout << "Kit : " << kitChoice.label << " [" << kitChoice.currentDurability << "/" << kitChoice.maxDurability << "]" << std::endl;
        std::cout << "Seuil permis : +" << kitChoice.thresholdPercent << "% de durabilité maximale." << std::endl;
        std::cout << std::endl;
        std::cout << "Matériaux nécessaires :" << std::endl;
        displayRepairMaterialCostDetailed(player, armorRepair, kitChoice.thresholdPercent);
        std::cout << std::endl;

        if (!hasRepairMaterials(player, armorRepair, kitChoice.thresholdPercent))
        {
            std::cout << "Matériaux insuffisants pour cette réparation." << std::endl;
            std::cout << "================================" << std::endl;
            std::cout << std::endl;
            return false;
        }

        std::cout << "1 : Continuer et réparer" << std::endl;
        std::cout << "0 : Annuler" << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, 1, "Choix invalide.");
        std::cout << std::endl;

        return choice == 1;
    }

    // EN: applyRepairKitWear declares or implements a focused behavior used by this module.
    // FR: applyRepairKitWear déclare ou implémente un comportement précis utilisé par ce module.
    void applyRepairKitWear(Player& player, const std::string& kitId, bool& kitDurabilitySaved)
    {
        kitDurabilitySaved = false;
        int saveChance = blacksmithSaveKitDurabilityChance(player);

        if (saveChance > 0 && rollPercent(saveChance))
        {
            kitDurabilitySaved = true;
            return;
        }

        consumeRepairKitDurability(player, kitId);
    }

    // EN: displayRepairKitWearResult declares or implements a focused behavior used by this module.
    // FR: displayRepairKitWearResult déclare ou implémente un comportement précis utilisé par ce module.
    void displayRepairKitWearResult(const std::string& kitId, bool kitDurabilitySaved)
    {
        if (kitDurabilitySaved)
        {
            std::cout << "Spécial Forgeron : tu économises l'usure du kit cette fois-ci." << std::endl;
            return;
        }

        int remainingDurability = repairKitCurrentDurability(kitId) - 1;

        if (remainingDurability > 0)
        {
            std::cout << "Le kit perd 1 durabilité. Il passe à " << remainingDurability << "/" << repairKitMaxDurability(kitId) << "." << std::endl;
        }
        else
        {
            std::cout << "Le kit perd sa dernière durabilité et disparaît." << std::endl;
        }
    }

    // EN: repairSelectedWeapon declares or implements a focused behavior used by this module.
    // FR: repairSelectedWeapon déclare ou implémente un comportement précis utilisé par ce module.
    bool repairSelectedWeapon(Player& player, int weaponIndex)
    {
        Weapon* weapon = player.getInventory().getMutableWeapon(weaponIndex);

        if (weapon == nullptr)
        {
            std::cout << "Cette arme n'existe plus." << std::endl << std::endl;
            return false;
        }

        if (weapon->isIndestructible())
        {
            std::cout << "Cette arme est indestructible. Elle n'a pas besoin de réparation." << std::endl << std::endl;
            return false;
        }

        if (weapon->getDurability() >= weapon->getMaxDurability())
        {
            std::cout << "Cette arme est déjà en parfait état." << std::endl << std::endl;
            return false;
        }

        std::vector<RepairKitChoice> choices = getRepairKitChoices(player);
        int kitChoiceIndex = chooseRepairKit(player);

        if (kitChoiceIndex < 0)
        {
            return false;
        }

        RepairKitChoice kitChoice = choices[kitChoiceIndex];
        int cap = repairedDurabilityCap(weapon->getMaxDurability(), kitChoice.thresholdPercent);

        if (weapon->getDurability() >= cap)
        {
            std::cout << "Ce kit ne peut pas améliorer davantage cette arme." << std::endl;
            std::cout << "Seuil du kit : " << cap << "/" << weapon->getMaxDurability() << "." << std::endl;
            std::cout << std::endl;
            return false;
        }

        if (!confirmRepairCost(player, weapon->getName(), false, kitChoice))
        {
            return false;
        }

        consumeRepairMaterials(player, false, kitChoice.thresholdPercent);
        bool kitDurabilitySaved = false;
        applyRepairKitWear(player, kitChoice.id, kitDurabilitySaved);

        int amountToRepair = cap - weapon->getDurability();
        weapon->repair(amountToRepair);

        std::cout << "Tu répares " << weapon->getName() << "." << std::endl;
        std::cout << "Matériaux consommés : ";
        displayRepairMaterialCost(false, kitChoice.thresholdPercent);
        std::cout << "." << std::endl;
        displayRepairKitWearResult(kitChoice.id, kitDurabilitySaved);
        std::cout << "Durabilité actuelle : " << weapon->getDurability() << "/" << weapon->getMaxDurability() << std::endl;
        std::cout << std::endl;
        return false;
    }

    // EN: repairSelectedArmor declares or implements a focused behavior used by this module.
    // FR: repairSelectedArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool repairSelectedArmor(Player& player, int armorIndex)
    {
        Armor* armor = player.getInventory().getMutableArmor(armorIndex);

        if (armor == nullptr)
        {
            std::cout << "Cette armure n'existe plus." << std::endl << std::endl;
            return false;
        }

        if (armor->isIndestructible())
        {
            std::cout << "Cette armure est indestructible. Elle n'a pas besoin de réparation." << std::endl << std::endl;
            return false;
        }

        if (armor->getDurability() >= armor->getMaxDurability())
        {
            std::cout << "Cette armure est déjà en parfait état." << std::endl << std::endl;
            return false;
        }

        std::vector<RepairKitChoice> choices = getRepairKitChoices(player);
        int kitChoiceIndex = chooseRepairKit(player);

        if (kitChoiceIndex < 0)
        {
            return false;
        }

        RepairKitChoice kitChoice = choices[kitChoiceIndex];
        int cap = repairedDurabilityCap(armor->getMaxDurability(), kitChoice.thresholdPercent);

        if (armor->getDurability() >= cap)
        {
            std::cout << "Ce kit ne peut pas améliorer davantage cette armure." << std::endl;
            std::cout << "Seuil du kit : " << cap << "/" << armor->getMaxDurability() << "." << std::endl;
            std::cout << std::endl;
            return false;
        }

        if (!confirmRepairCost(player, armor->getName(), true, kitChoice))
        {
            return false;
        }

        consumeRepairMaterials(player, true, kitChoice.thresholdPercent);
        bool kitDurabilitySaved = false;
        applyRepairKitWear(player, kitChoice.id, kitDurabilitySaved);

        int amountToRepair = cap - armor->getDurability();
        armor->repair(amountToRepair);

        std::cout << "Tu répares " << armor->getName() << "." << std::endl;
        std::cout << "Matériaux consommés : ";
        displayRepairMaterialCost(true, kitChoice.thresholdPercent);
        std::cout << "." << std::endl;
        displayRepairKitWearResult(kitChoice.id, kitDurabilitySaved);
        std::cout << "Durabilité actuelle : " << armor->getDurability() << "/" << armor->getMaxDurability() << std::endl;
        std::cout << std::endl;
        return false;
    }

    // EN: displayMaterialUtility declares or implements a focused behavior used by this module.
    // FR: displayMaterialUtility déclare ou implémente un comportement précis utilisé par ce module.
    void displayMaterialUtility(const Material& material)
    {
        std::cout << "========== UTILITÉ PRÉVUE ==========" << std::endl;
        std::cout << material.getName() << " | " << material.getCategory() << std::endl;
        if (material.getCategory() != "Livre" && material.getCategory() != "Renseignement" && material.getCategory() != "Outil")
        {
            std::cout << "Qualité : " << material.getQualityLabel() << std::endl;
            std::cout << "Craft : faible/impur compte moins, pur/haute qualité compte plus." << std::endl;
        }

        if (isRepairKitId(material.getId()))
        {
            std::cout << "Usage : autorise une réparation autonome jusqu'à environ +" << repairThresholdPercentForKit(material.getId()) << "% de durabilité." << std::endl;
            if (isUsedRepairKitId(material.getId()))
            {
                std::cout << "Kits entamés empilés ici : " << material.getQuantity() << std::endl;
                std::cout << "Durabilité par kit : " << repairKitCurrentDurability(material.getId()) << "/" << repairKitMaxDurability(material.getId()) << std::endl;
            }
            else
            {
                std::cout << "Kits intacts empilés ici : " << material.getQuantity() << std::endl;
                std::cout << "Durabilité par kit : " << repairKitMaxDurability(material.getId()) << "/" << repairKitMaxDurability(material.getId()) << std::endl;
            }
            std::cout << "Après chaque réparation, le kit perd normalement 1 durabilité. Les matériaux restent consommés." << std::endl;
        }
        else if (material.getId() == "rusted_metal_fragment" || material.getId() == "worn_leather_piece")
        {
            std::cout << "Usage prévu : craft basique, réparation et amélioration d'équipement commun." << std::endl;
        }
        else if (material.getId() == "wolf_fang" || material.getId() == "goblin_ear" || material.getId() == "cracked_bone")
        {
            std::cout << "Usage prévu : trophées, recettes de monstres, contrats de guilde et artisanat spécialisé." << std::endl;
        }
        else if (material.getId() == "arcane_dust")
        {
            std::cout << "Usage prévu : enchantements, catalyseurs de sorts et équipements magiques." << std::endl;
        }
        else if (material.getId() == "slime_residue")
        {
            std::cout << "Usage prévu : colle, pièges, potions et réparations de fortune." << std::endl;
        }
        else if (material.getCategory() == "Plante")
        {
            std::cout << "Usage prévu : potions, remèdes, quêtes botaniques et expériences." << std::endl;
        }
        else if (material.getCategory() == "Livre" || material.getCategory() == "Renseignement")
        {
            std::cout << "Usage prévu : lecture pour débloquer ou renforcer une entrée de bestiaire." << std::endl;
        }
        else if (material.getId() == "cracked_bone")
        {
            std::cout << "Usage prévu : nécromancie, rage supérieure, invocations sombres et futurs crafts d'os." << std::endl;
        }
        else if (material.getId() == "battle_torn_badge")
        {
            std::cout << "Usage prévu : guildes, contrats, réputation et rencontres d'aventuriers." << std::endl;
        }
        else if (material.getId() == "beast_hide")
        {
            std::cout << "Usage prévu : armures, réparations épaisses, futurs crafts de tanks et équipements de survie." << std::endl;
        }
        else if (material.getId() == "shadow_thread")
        {
            std::cout << "Usage prévu : ombres de Hazak, objets d'assassin, nécromancie et améliorations furtives." << std::endl;
        }
        else if (material.getId() == "kitsune_ember")
        {
            std::cout << "Usage prévu : flammes kitsune, invocations d'Aoi, potions avancées et enchantements feu." << std::endl;
        }
        else if (material.getId() == "draconic_scale_fragment")
        {
            std::cout << "Usage prévu : protections rares, armures lourdes et crafts semi-dragons." << std::endl;
        }
        else if (material.getId() == "unstable_core")
        {
            std::cout << "Usage prévu : alchimie risquée, expériences de Fail, invocations instables et futurs objets explosifs." << std::endl;
        }
        else if (material.getId() == "fitoria_feather")
        {
            std::cout << "Usage prévu : bénédictions, soins rares, équipements de lumière et futures reliques angéliques." << std::endl;
        }
        else if (material.getId() == "zelef_demon_blood")
        {
            std::cout << "Usage prévu : alchimie dangereuse, armes démoniaques, malédictions et contrats sombres." << std::endl;
        }
        else if (material.getId() == "atlas_broken_plate")
        {
            std::cout << "Usage prévu : armures lourdes, réparation extrême, reliques défensives et forge de haut niveau." << std::endl;
        }
        else if (material.getId() == "precision_harvest_tools")
        {
            std::cout << "Usage prévu : bonus passif de récupération. Tant que tu les possèdes, tu récupères plus proprement certains matériaux." << std::endl;
        }
        else if (material.getId() == "preservation_vials")
        {
            std::cout << "Usage prévu : bonus passif de conservation. Réduit les chances d'abîmer liquides, braises, résidus et composants instables." << std::endl;
        }
        else if (material.getId() == "clean_harvest_manual")
        {
            std::cout << "Usage prévu : technique passive. La lecture confirme l'apprentissage, et posséder le manuel augmente la récupération propre." << std::endl;
        }
        else if (material.getId() == "monster_dissection_guide")
        {
            std::cout << "Usage prévu : technique passive. Aide à récupérer les composants de monstres sans trop les dégrader." << std::endl;
        }
        else
        {
            std::cout << "Usage prévu : système de craft/réparation futur." << std::endl;
        }

        std::cout << "====================================" << std::endl;
        std::cout << std::endl;
    }

    // EN: readMaterialIfPossible declares or implements a focused behavior used by this module.
    // FR: readMaterialIfPossible déclare ou implémente un comportement précis utilisé par ce module.
    bool readMaterialIfPossible(const Material& material)
    {
        if (material.getCategory() != "Livre" && material.getCategory() != "Renseignement")
        {
            std::cout << "Tu manipules " << material.getName() << ", mais son vrai système d'utilisation n'est pas encore stabilisé." << std::endl;
            std::cout << "[action en cours de développement]" << std::endl;
            std::cout << std::endl;
            return false;
        }

        BestiaryRuntimeProgress::unlockCommonInformation(material.getId());

        std::cout << "Tu prends le temps de lire : " << material.getName() << "." << std::endl;
        if (material.getId() == "clean_harvest_manual" || material.getId() == "monster_dissection_guide")
        {
            std::cout << "Technique passive comprise : ses effets restent actifs tant que l'apprentissage est conservé dans l'inventaire." << std::endl;
        }
        std::cout << "Les informations compatibles sont ajoutées ou confirmées dans le bestiaire de session." << std::endl;
        std::cout << std::endl;
        return false;
    }



    struct RecipeIngredient
    {
        std::string id;
        int quantity;
    };

    // EN: countExceptionalQualityPointsForId declares or implements a focused behavior used by this module.
    // FR: countExceptionalQualityPointsForId déclare ou implémente un comportement précis utilisé par ce module.
    int countExceptionalQualityPointsForId(Player& player, const std::string& id)
    {
        int total = 0;

        for (const Material& material : player.getInventory().getMaterials())
        {
            if (material.getId() == id && material.getQuality() == "exceptional")
            {
                total += material.getQuantity() * material.getQualityCraftWeight();
            }
        }

        return total;
    }

    // EN: recipeUsesExceptionalMajority declares or implements a focused behavior used by this module.
    // FR: recipeUsesExceptionalMajority déclare ou implémente un comportement précis utilisé par ce module.
    bool recipeUsesExceptionalMajority(Player& player, const std::vector<RecipeIngredient>& ingredients)
    {
        int requiredNormalQualityPoints = 0;
        int availableExceptionalPoints = 0;

        for (const RecipeIngredient& ingredient : ingredients)
        {
            requiredNormalQualityPoints += ingredient.quantity * 2;
            availableExceptionalPoints += std::min(
                countExceptionalQualityPointsForId(player, ingredient.id),
                ingredient.quantity * 2
            );
        }

        return requiredNormalQualityPoints > 0
            && availableExceptionalPoints * 2 > requiredNormalQualityPoints;
    }

    // EN: removeExceptionalMaterialPointsFirst declares or implements a focused behavior used by this module.
    // FR: removeExceptionalMaterialPointsFirst déclare ou implémente un comportement précis utilisé par ce module.
    bool removeExceptionalMaterialPointsFirst(Player& player, const std::string& id, int& requiredPoints)
    {
        for (int i = 0; i < player.getInventory().getMaterialCount() && requiredPoints > 0; ++i)
        {
            if (!player.getInventory().hasMaterial(i))
            {
                continue;
            }

            Material material = player.getInventory().getMaterial(i);

            if (material.getId() != id || material.getQuality() != "exceptional")
            {
                continue;
            }

            int weight = material.getQualityCraftWeight();
            int toRemove = std::min(material.getQuantity(), (requiredPoints + weight - 1) / weight);

            if (toRemove <= 0)
            {
                continue;
            }

            player.getInventory().removeMaterialQuantity(i, toRemove);
            requiredPoints -= toRemove * weight;
            --i;
        }

        return requiredPoints <= 0;
    }

    // EN: consumeRecipeIngredient declares or implements a focused behavior used by this module.
    // FR: consumeRecipeIngredient déclare ou implémente un comportement précis utilisé par ce module.
    bool consumeRecipeIngredient(Player& player, const std::string& id, int quantity, bool preferExceptional)
    {
        if (quantity <= 0)
        {
            return true;
        }

        int requiredPoints = quantity * 2;

        if (player.getInventory().countMaterialQualityPointsById(id) < requiredPoints)
        {
            return false;
        }

        if (preferExceptional)
        {
            removeExceptionalMaterialPointsFirst(player, id, requiredPoints);
        }

        if (requiredPoints <= 0)
        {
            return true;
        }

        int remainingNormalQualityQuantity = (requiredPoints + 1) / 2;
        return player.getInventory().removeMaterialQuantityByIdFlexible(id, remainingNormalQualityQuantity);
    }

    // EN: createExceptionalCraftedConsumable declares or implements a focused behavior used by this module.
    // FR: createExceptionalCraftedConsumable déclare ou implémente un comportement précis utilisé par ce module.
    Consumable createExceptionalCraftedConsumable(const Consumable& base)
    {
        if (base.isHealing())
        {
            return Consumable(
                base.getName() + " — particularité stable",
                base.getDescription() + " Une faible particularité issue de matériaux exceptionnels augmente légèrement son effet.",
                base.getValue() + 18,
                ConsumableType::Healing,
                base.getPower() + 8
            );
        }

        if (base.isDamage())
        {
            return Consumable(
                base.getName() + " — particularité agressive",
                base.getDescription() + " Une faible particularité issue de matériaux exceptionnels augmente légèrement son effet.",
                base.getValue() + 20,
                ConsumableType::Damage,
                base.getPower() + 6
            );
        }

        return base;
    }

    // EN: addCraftedConsumableWithExceptionalChance declares or implements a focused behavior used by this module.
    // FR: addCraftedConsumableWithExceptionalChance déclare ou implémente un comportement précis utilisé par ce module.
    void addCraftedConsumableWithExceptionalChance(Player& player, const Consumable& base, bool exceptionalMajority)
    {
        int chance = exceptionalCraftChanceFor(player, "consumable");

        if (exceptionalMajority && rollPercent(chance))
        {
            Consumable special = createExceptionalCraftedConsumable(base);
            player.getInventory().addConsumable(special);
            std::cout << "Matériaux exceptionnels majoritaires : la fabrication ajoute une faible particularité à l'objet créé." << std::endl;
            std::cout << "Chance appliquée : " << chance << "% grâce à la qualité et au métier." << std::endl;
            return;
        }

        player.getInventory().addConsumable(base);
    }

    // EN: createExceptionalCraftedWeapon declares or implements a focused behavior used by this module.
    // FR: createExceptionalCraftedWeapon déclare ou implémente un comportement précis utilisé par ce module.
    Weapon createExceptionalCraftedWeapon(const Weapon& base)
    {
        return Weapon(
            base.getName() + " — particularité précise",
            base.getDescription() + " Une faible particularité née de matériaux exceptionnels rend l'équilibre légèrement meilleur.",
            base.getValue() + 35,
            base.getType(),
            base.getMinDamageBonus() + 1,
            base.getMaxDamageBonus() + 2,
            base.getCriticalBonus() + 1,
            base.getMaxDurability() < 0 ? base.getMaxDurability() : base.getMaxDurability() + 8
        );
    }

    // EN: createExceptionalCraftedArmor declares or implements a focused behavior used by this module.
    // FR: createExceptionalCraftedArmor déclare ou implémente un comportement précis utilisé par ce module.
    Armor createExceptionalCraftedArmor(const Armor& base)
    {
        return Armor(
            base.getName() + " — particularité renforcée",
            base.getDescription() + " Une faible particularité née de matériaux exceptionnels stabilise mieux les protections.",
            base.getValue() + 40,
            base.getType(),
            base.getMaxHpBonus() + 6,
            base.getDamageReduction() + 1,
            base.getMaxDurability() < 0 ? base.getMaxDurability() : base.getMaxDurability() + 10
        );
    }

    // EN: addCraftedWeaponWithExceptionalChance declares or implements a focused behavior used by this module.
    // FR: addCraftedWeaponWithExceptionalChance déclare ou implémente un comportement précis utilisé par ce module.
    void addCraftedWeaponWithExceptionalChance(Player& player, const Weapon& base, bool exceptionalMajority)
    {
        int chance = exceptionalCraftChanceFor(player, "weapon");

        if (exceptionalMajority && rollPercent(chance))
        {
            player.getInventory().addWeapon(createExceptionalCraftedWeapon(base));
            std::cout << "Matériaux exceptionnels majoritaires : l'arme gagne une faible particularité." << std::endl;
            std::cout << "Chance appliquée : " << chance << "% grâce à la qualité et au métier." << std::endl;
            return;
        }

        player.getInventory().addWeapon(base);
    }

    // EN: addCraftedArmorWithExceptionalChance declares or implements a focused behavior used by this module.
    // FR: addCraftedArmorWithExceptionalChance déclare ou implémente un comportement précis utilisé par ce module.
    void addCraftedArmorWithExceptionalChance(Player& player, const Armor& base, bool exceptionalMajority)
    {
        int chance = exceptionalCraftChanceFor(player, "armor");

        if (exceptionalMajority && rollPercent(chance))
        {
            player.getInventory().addArmor(createExceptionalCraftedArmor(base));
            std::cout << "Matériaux exceptionnels majoritaires : l'armure gagne une faible particularité." << std::endl;
            std::cout << "Chance appliquée : " << chance << "% grâce à la qualité et au métier." << std::endl;
            return;
        }

        player.getInventory().addArmor(base);
    }

    // EN: hasRecipeIngredients declares or implements a focused behavior used by this module.
    // FR: hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
    bool hasRecipeIngredients(Player& player, const std::string& id, int quantity)
    {
        return player.getInventory().countMaterialQualityPointsById(id) >= quantity * 2;
    }

    // EN: consumeRecipeIngredient declares or implements a focused behavior used by this module.
    // FR: consumeRecipeIngredient déclare ou implémente un comportement précis utilisé par ce module.
    bool consumeRecipeIngredient(Player& player, const std::string& id, int quantity)
    {
        return consumeRecipeIngredient(player, id, quantity, false);
    }

    bool consumeRecipeIngredients(Player& player, const std::vector<RecipeIngredient>& ingredients, const std::string& recipeName)
    {
        for (const RecipeIngredient& ingredient : ingredients)
        {
            if (!hasRecipeIngredients(player, ingredient.id, ingredient.quantity))
            {
                std::cout << "Il manque des composants pour : " << recipeName << "." << std::endl << std::endl;
                return false;
            }
        }

        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        for (const RecipeIngredient& ingredient : ingredients)
        {
            consumeRecipeIngredient(player, ingredient.id, ingredient.quantity, exceptionalMajority);
        }
        return true;
    }

    // EN: craftBasicHealingPotion declares or implements a focused behavior used by this module.
    // FR: craftBasicHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
    bool craftBasicHealingPotion(Player& player)
    {
        if (!hasRecipeIngredients(player, "bitter_healing_leaf", 2)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "slime_residue", 1))
        {
            std::cout << "Recette incomplète : il faut 2 Feuilles amères de soin et 1 Résidu de slime." << std::endl;
            std::cout << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"bitter_healing_leaf", 2}, {"slime_residue", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "bitter_healing_leaf", 2, exceptionalMajority);
        consumeRecipeIngredient(player, "slime_residue", 1, exceptionalMajority);
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createBasicHealingPotion(), exceptionalMajority);
        maybeRestoreAlchemistCatalyst(player, "slime_residue", "le résidu de slime");

        std::cout << "Tu écrases les feuilles, stabilises le mélange avec le résidu de slime," << std::endl;
        std::cout << "et obtiens une Potion de soin." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftReinforcedHealingPotion declares or implements a focused behavior used by this module.
    // FR: craftReinforcedHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
    bool craftReinforcedHealingPotion(Player& player)
    {
        if (!hasRecipeIngredients(player, "mountain_blue_flower", 1)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "arcane_dust", 1))
        {
            std::cout << "Recette incomplète : il faut 1 Fleur bleue de montagne et 1 Poussière arcanique." << std::endl;
            std::cout << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"mountain_blue_flower", 1}, {"arcane_dust", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "mountain_blue_flower", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 1, exceptionalMajority);
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createReinforcedHealingPotion(), exceptionalMajority);
        maybeRestoreAlchemistCatalyst(player, "arcane_dust", "la poussière arcanique");

        std::cout << "La fleur bleue absorbe la poussière arcanique sans se briser." << std::endl;
        std::cout << "Tu obtiens une Potion de soin renforcée." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftDamagePotion declares or implements a focused behavior used by this module.
    // FR: craftDamagePotion déclare ou implémente un comportement précis utilisé par ce module.
    bool craftDamagePotion(Player& player)
    {
        if (!hasRecipeIngredients(player, "wolf_fang", 1)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "arcane_dust", 1))
        {
            std::cout << "Recette incomplète : il faut 1 Croc de loup et 1 Poussière arcanique." << std::endl;
            std::cout << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"wolf_fang", 1}, {"arcane_dust", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "wolf_fang", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 1, exceptionalMajority);
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createBasicDamagePotion(), exceptionalMajority);
        maybeRestoreAlchemistCatalyst(player, "arcane_dust", "la poussière arcanique");

        std::cout << "Le croc garde une agressivité étrange une fois broyé." << std::endl;
        std::cout << "Tu obtiens une Potion de rage." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftReinforcedDamagePotion declares or implements a focused behavior used by this module.
    // FR: craftReinforcedDamagePotion déclare ou implémente un comportement précis utilisé par ce module.
    bool craftReinforcedDamagePotion(Player& player)
    {
        if (!hasRecipeIngredients(player, "wolf_fang", 2)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "cracked_bone", 1)
            || !hasRecipeIngredients(player, "arcane_dust", 1))
        {
            std::cout << "Recette incomplète : il faut 2 Crocs de loup, 1 Os fissuré et 1 Poussière arcanique." << std::endl;
            std::cout << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"wolf_fang", 2}, {"cracked_bone", 1}, {"arcane_dust", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "wolf_fang", 2, exceptionalMajority);
        consumeRecipeIngredient(player, "cracked_bone", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 1, exceptionalMajority);
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createReinforcedDamagePotion(), exceptionalMajority);
        maybeRestoreAlchemistCatalyst(player, "arcane_dust", "la poussière arcanique");

        std::cout << "L'os fissuré absorbe la rage du croc et la poussière arcanique la stabilise à peine." << std::endl;
        std::cout << "Tu obtiens une Potion de rage supérieure." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftKitsuneHealingPotion declares or implements a focused behavior used by this module.
    // FR: craftKitsuneHealingPotion déclare ou implémente un comportement précis utilisé par ce module.
    bool craftKitsuneHealingPotion(Player& player)
    {
        if (!hasRecipeIngredients(player, "kitsune_ember", 1)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "mountain_blue_flower", 1)
            || !hasRecipeIngredients(player, "slime_residue", 1))
        {
            std::cout << "Recette incomplète : il faut 1 Braise kitsune, 1 Fleur bleue de montagne et 1 Résidu de slime." << std::endl;
            std::cout << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"kitsune_ember", 1}, {"mountain_blue_flower", 1}, {"slime_residue", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "kitsune_ember", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "mountain_blue_flower", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "slime_residue", 1, exceptionalMajority);
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createReinforcedHealingPotion(), exceptionalMajority);
        maybeRestoreAlchemistCatalyst(player, "slime_residue", "le résidu de slime");

        std::cout << "La braise kitsune réchauffe la fleur sans la brûler." << std::endl;
        std::cout << "Tu obtiens une Potion de soin renforcée." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftUnstableRagePotion declares or implements a focused behavior used by this module.
    // FR: craftUnstableRagePotion déclare ou implémente un comportement précis utilisé par ce module.
    bool craftUnstableRagePotion(Player& player)
    {
        if (!hasRecipeIngredients(player, "unstable_core", 1)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "wolf_fang", 1)
            || !hasRecipeIngredients(player, "arcane_dust", 1))
        {
            std::cout << "Recette incomplète : il faut 1 Noyau instable, 1 Croc de loup et 1 Poussière arcanique." << std::endl;
            std::cout << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"unstable_core", 1}, {"wolf_fang", 1}, {"arcane_dust", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "unstable_core", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "wolf_fang", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 1, exceptionalMajority);
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createReinforcedDamagePotion(), exceptionalMajority);
        maybeRestoreAlchemistCatalyst(player, "arcane_dust", "la poussière arcanique");

        std::cout << "Le noyau instable pulse une dernière fois avant d'être scellé." << std::endl;
        std::cout << "Tu obtiens une Potion de rage supérieure." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftRepairKit declares or implements a focused behavior used by this module.
    // FR: craftRepairKit déclare ou implémente un comportement précis utilisé par ce module.
    bool craftRepairKit(Player& player)
    {
        if (!hasRecipeIngredients(player, "rusted_metal_fragment", 2)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "worn_leather_piece", 1)
            || !hasRecipeIngredients(player, "slime_residue", 1))
        {
            std::cout << "Recette incomplète : il faut 2 Fragments de métal rouillé, 1 Morceau de cuir abîmé et 1 Résidu de slime." << std::endl;
            std::cout << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "rusted_metal_fragment", 2);
        consumeRecipeIngredient(player, "worn_leather_piece", 1);
        consumeRecipeIngredient(player, "slime_residue", 1);
        player.getInventory().addMaterial(MaterialCatalog::createWeakRepairKit());

        std::cout << "Tu relies le cuir, coinces les fragments métalliques et utilises le slime comme liant." << std::endl;
        std::cout << "Tu obtiens un Kit de réparation faible. Sa durabilité est limitée." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftMediumRepairKit declares or implements a focused behavior used by this module.
    // FR: craftMediumRepairKit déclare ou implémente un comportement précis utilisé par ce module.
    bool craftMediumRepairKit(Player& player)
    {
        if (!hasRecipeIngredients(player, "weak_repair_kit", 1)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "rusted_metal_fragment", 3)
            || !hasRecipeIngredients(player, "worn_leather_piece", 1)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "arcane_dust", 1))
        {
            std::cout << "Recette incomplète : il faut 1 Kit faible intact, 3 Fragments de métal rouillé, 1 Morceau de cuir abîmé et 1 Poussière arcanique." << std::endl;
            std::cout << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "weak_repair_kit", 1);
        consumeRecipeIngredient(player, "rusted_metal_fragment", 3);
        consumeRecipeIngredient(player, "worn_leather_piece", 1);
        consumeRecipeIngredient(player, "arcane_dust", 1);
        player.getInventory().addMaterial(MaterialCatalog::createMediumRepairKit());

        std::cout << "La poussière arcanique stabilise les plaques de réparation." << std::endl;
        std::cout << "Tu obtiens un Kit de réparation moyen." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftBigRepairKit declares or implements a focused behavior used by this module.
    // FR: craftBigRepairKit déclare ou implémente un comportement précis utilisé par ce module.
    bool craftBigRepairKit(Player& player)
    {
        if (!hasRecipeIngredients(player, "medium_repair_kit", 1)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "rusted_metal_fragment", 5)
            || !hasRecipeIngredients(player, "worn_leather_piece", 3)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "arcane_dust", 2))
        {
            std::cout << "Recette incomplète : il faut 1 Kit moyen intact, 5 Fragments de métal rouillé, 3 Morceaux de cuir abîmé et 2 Poussières arcaniques." << std::endl;
            std::cout << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "medium_repair_kit", 1);
        consumeRecipeIngredient(player, "rusted_metal_fragment", 5);
        consumeRecipeIngredient(player, "worn_leather_piece", 3);
        consumeRecipeIngredient(player, "arcane_dust", 2);
        player.getInventory().addMaterial(MaterialCatalog::createBigRepairKit());

        std::cout << "Tu renforces les attaches et solidifies les outils." << std::endl;
        std::cout << "Tu obtiens un Gros kit de réparation." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftTinkererCompleteRepairKit declares or implements a focused behavior used by this module.
    // FR: craftTinkererCompleteRepairKit déclare ou implémente un comportement précis utilisé par ce module.
    bool craftTinkererCompleteRepairKit(Player& player)
    {
        if (!hasRecipeIngredients(player, "big_repair_kit", 1)
            || !hasRecipeIngredients(player, "rusted_metal_fragment", 8)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "worn_leather_piece", 5)
            || !hasRecipeIngredients(player, "arcane_dust", 4)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "slime_residue", 3))
        {
            std::cout << "Recette incomplète : il faut 1 Gros kit intact, 8 Fragments de métal rouillé, 5 Morceaux de cuir abîmé, 4 Poussières arcaniques et 3 Résidus de slime." << std::endl;
            std::cout << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "big_repair_kit", 1);
        consumeRecipeIngredient(player, "rusted_metal_fragment", 8);
        consumeRecipeIngredient(player, "worn_leather_piece", 5);
        consumeRecipeIngredient(player, "arcane_dust", 4);
        consumeRecipeIngredient(player, "slime_residue", 3);
        player.getInventory().addMaterial(MaterialCatalog::createTinkererCompleteRepairKit());

        std::cout << "Tu assembles une vraie trousse presque complète." << std::endl;
        std::cout << "Tu obtiens un Kit complet du bricoleur." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftFieldArmorPatch declares or implements a focused behavior used by this module.
    // FR: craftFieldArmorPatch déclare ou implémente un comportement précis utilisé par ce module.
    bool craftFieldArmorPatch(Player& player)
    {
        if (!hasRecipeIngredients(player, "beast_hide", 1)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "worn_leather_piece", 2)
            || !hasRecipeIngredients(player, "slime_residue", 1))
        {
            std::cout << "Recette incomplète : il faut 1 Peau de bête robuste, 2 Morceaux de cuir abîmé et 1 Résidu de slime." << std::endl;
            std::cout << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "beast_hide", 1);
        consumeRecipeIngredient(player, "worn_leather_piece", 2);
        consumeRecipeIngredient(player, "slime_residue", 1);
        player.getInventory().addMaterial(MaterialCatalog::createMediumRepairKit());

        std::cout << "Tu transformes la peau robuste en plaques de rafistolage renforcées." << std::endl;
        std::cout << "Tu obtiens un Kit de réparation moyen." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftRecoveryBlade declares or implements a focused behavior used by this module.
    // FR: craftRecoveryBlade déclare ou implémente un comportement précis utilisé par ce module.
    bool craftRecoveryBlade(Player& player)
    {
        if (!hasRecipeIngredients(player, "rusted_metal_fragment", 4)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "worn_leather_piece", 1)
            || !hasRecipeIngredients(player, "arcane_dust", 1))
        {
            std::cout << "Recette incomplète : il faut 4 Fragments de métal rouillé, 1 Morceau de cuir abîmé et 1 Poussière arcanique." << std::endl;
            std::cout << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"rusted_metal_fragment", 4}, {"worn_leather_piece", 1}, {"arcane_dust", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);

        consumeRecipeIngredient(player, "rusted_metal_fragment", 4, exceptionalMajority);
        consumeRecipeIngredient(player, "worn_leather_piece", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 1, exceptionalMajority);

        Weapon blade(
            "Lame de récupération",
            "Une lame assemblée avec des matériaux récupérés. Pas noble, mais plus fiable qu'une rouille abandonnée.",
            65,
            WeaponType::Sword,
            4,
            8,
            5,
            115
        );

        addCraftedWeaponWithExceptionalChance(player, blade, exceptionalMajority);

        std::cout << "Tu ajustes les fragments et obtiens une Lame de récupération." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftHunterLeatherArmor declares or implements a focused behavior used by this module.
    // FR: craftHunterLeatherArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool craftHunterLeatherArmor(Player& player)
    {
        if (!hasRecipeIngredients(player, "beast_hide", 2)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "worn_leather_piece", 3)
            || !hasRecipeIngredients(player, "slime_residue", 1))
        {
            std::cout << "Recette incomplète : il faut 2 Peaux de bête robustes, 3 Morceaux de cuir abîmé et 1 Résidu de slime." << std::endl;
            std::cout << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"beast_hide", 2}, {"worn_leather_piece", 3}, {"slime_residue", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);

        consumeRecipeIngredient(player, "beast_hide", 2, exceptionalMajority);
        consumeRecipeIngredient(player, "worn_leather_piece", 3, exceptionalMajority);
        consumeRecipeIngredient(player, "slime_residue", 1, exceptionalMajority);

        Armor armor(
            "Armure de chasseur rafistolée",
            "Une armure souple faite pour survivre aux mauvaises rencontres plutôt que briller dans une forge royale.",
            90,
            ArmorType::Leather,
            38,
            4,
            125
        );

        addCraftedArmorWithExceptionalChance(player, armor, exceptionalMajority);

        std::cout << "Tu tends la peau, renforces les coutures et obtiens une Armure de chasseur rafistolée." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftShadowThreadDagger declares or implements a focused behavior used by this module.
    // FR: craftShadowThreadDagger déclare ou implémente un comportement précis utilisé par ce module.
    bool craftShadowThreadDagger(Player& player)
    {
        std::vector<RecipeIngredient> ingredients = {{"shadow_thread", 2}, {"rusted_metal_fragment", 3}, {"arcane_dust", 1}};

        if (!hasRecipeIngredients(player, "shadow_thread", 2)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "rusted_metal_fragment", 3)
            || !hasRecipeIngredients(player, "arcane_dust", 1))
        {
            std::cout << "Recette incomplète : il faut 2 Fils d'ombre, 3 Fragments de métal rouillé et 1 Poussière arcanique." << std::endl;
            std::cout << std::endl;
            return false;
        }

        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "shadow_thread", 2, exceptionalMajority);
        consumeRecipeIngredient(player, "rusted_metal_fragment", 3, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 1, exceptionalMajority);

        Weapon dagger(
            "Dague cousue d'ombre",
            "Une lame courte retenue par un fil sombre. Elle préfère les gestes précis aux grands coups héroïques.",
            155,
            WeaponType::Dagger,
            6,
            11,
            10,
            105
        );

        addCraftedWeaponWithExceptionalChance(player, dagger, exceptionalMajority);
        std::cout << "Tu tends le fil d'ombre autour de la lame et obtiens une Dague cousue d'ombre." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftFangRecurveBow declares or implements a focused behavior used by this module.
    // FR: craftFangRecurveBow déclare ou implémente un comportement précis utilisé par ce module.
    bool craftFangRecurveBow(Player& player)
    {
        std::vector<RecipeIngredient> ingredients = {{"wolf_fang", 3}, {"beast_hide", 1}, {"worn_leather_piece", 2}};

        if (!hasRecipeIngredients(player, "wolf_fang", 3)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "beast_hide", 1)
            || !hasRecipeIngredients(player, "worn_leather_piece", 2))
        {
            std::cout << "Recette incomplète : il faut 3 Crocs de loup, 1 Peau de bête robuste et 2 Morceaux de cuir abîmé." << std::endl;
            std::cout << std::endl;
            return false;
        }

        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "wolf_fang", 3, exceptionalMajority);
        consumeRecipeIngredient(player, "beast_hide", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "worn_leather_piece", 2, exceptionalMajority);

        Weapon bow(
            "Arc recourbé aux crocs",
            "Un arc léger renforcé par des crocs polis. Il frappe moins fort qu'une hache, mais il reste propre et précis.",
            145,
            WeaponType::Bow,
            5,
            10,
            7,
            115
        );

        addCraftedWeaponWithExceptionalChance(player, bow, exceptionalMajority);
        std::cout << "Tu tends le cuir et fixes les crocs : l'Arc recourbé aux crocs est prêt." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftUnstableStaff declares or implements a focused behavior used by this module.
    // FR: craftUnstableStaff déclare ou implémente un comportement précis utilisé par ce module.
    bool craftUnstableStaff(Player& player)
    {
        std::vector<RecipeIngredient> ingredients = {{"unstable_core", 1}, {"arcane_dust", 3}, {"worn_leather_piece", 1}};

        if (!hasRecipeIngredients(player, "unstable_core", 1)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "arcane_dust", 3)
            || !hasRecipeIngredients(player, "worn_leather_piece", 1))
        {
            std::cout << "Recette incomplète : il faut 1 Noyau instable, 3 Poussières arcaniques et 1 Morceau de cuir abîmé." << std::endl;
            std::cout << std::endl;
            return false;
        }

        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "unstable_core", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 3, exceptionalMajority);
        consumeRecipeIngredient(player, "worn_leather_piece", 1, exceptionalMajority);

        Weapon staff(
            "Bâton au noyau instable",
            "Un catalyseur dangereux. Il n'est pas totalement fiable, mais il donne envie aux mages de tenter des choses stupides.",
            210,
            WeaponType::Staff,
            7,
            14,
            6,
            95
        );

        addCraftedWeaponWithExceptionalChance(player, staff, exceptionalMajority);
        std::cout << "Le noyau pulse au bout du bâton. C'est probablement stable. Probablement." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftDraconicPatchArmor declares or implements a focused behavior used by this module.
    // FR: craftDraconicPatchArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool craftDraconicPatchArmor(Player& player)
    {
        std::vector<RecipeIngredient> ingredients = {{"draconic_scale_fragment", 2}, {"beast_hide", 2}, {"arcane_dust", 2}};

        if (!hasRecipeIngredients(player, "draconic_scale_fragment", 2)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "beast_hide", 2)
            || !hasRecipeIngredients(player, "arcane_dust", 2))
        {
            std::cout << "Recette incomplète : il faut 2 Fragments d'écaille draconique, 2 Peaux de bête robustes et 2 Poussières arcaniques." << std::endl;
            std::cout << std::endl;
            return false;
        }

        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "draconic_scale_fragment", 2, exceptionalMajority);
        consumeRecipeIngredient(player, "beast_hide", 2, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 2, exceptionalMajority);

        Armor armor(
            "Armure rapiécée d'écailles",
            "Une protection lourde et irrégulière. Chaque écaille semble refuser de céder au premier choc.",
            245,
            ArmorType::Plate,
            62,
            8,
            150
        );

        addCraftedArmorWithExceptionalChance(player, armor, exceptionalMajority);
        std::cout << "Tu fixes les écailles sur une base robuste : l'armure encaisse déjà mieux." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftKitsuneEmberRobe declares or implements a focused behavior used by this module.
    // FR: craftKitsuneEmberRobe déclare ou implémente un comportement précis utilisé par ce module.
    bool craftKitsuneEmberRobe(Player& player)
    {
        std::vector<RecipeIngredient> ingredients = {{"kitsune_ember", 2}, {"mountain_blue_flower", 1}, {"arcane_dust", 2}};

        if (!hasRecipeIngredients(player, "kitsune_ember", 2)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "mountain_blue_flower", 1)
            || !hasRecipeIngredients(player, "arcane_dust", 2))
        {
            std::cout << "Recette incomplète : il faut 2 Braises kitsune, 1 Fleur bleue de montagne et 2 Poussières arcaniques." << std::endl;
            std::cout << std::endl;
            return false;
        }

        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "kitsune_ember", 2, exceptionalMajority);
        consumeRecipeIngredient(player, "mountain_blue_flower", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 2, exceptionalMajority);

        Armor robe(
            "Robe aux braises kitsune",
            "Un vêtement magique plus protecteur qu'il n'en a l'air. Les braises refusent de s'éteindre complètement.",
            230,
            ArmorType::Magical,
            48,
            6,
            110
        );

        addCraftedArmorWithExceptionalChance(player, robe, exceptionalMajority);
        std::cout << "Les braises se calment autour du tissu : la Robe aux braises kitsune est terminée." << std::endl;
        std::cout << std::endl;
        return true;
    }


    // EN: craftSerratedMonsterDagger creates an aggressive dagger from monster components.
    // FR: craftSerratedMonsterDagger fabrique une dague agressive avec des composants de monstre.
    bool craftSerratedMonsterDagger(Player& player)
    {
        std::vector<RecipeIngredient> ingredients = {{"goblin_ear", 2}, {"wolf_fang", 2}, {"rusted_metal_fragment", 2}};

        if (!hasRecipeIngredients(player, "goblin_ear", 2)
            || !hasRecipeIngredients(player, "wolf_fang", 2)
            || !hasRecipeIngredients(player, "rusted_metal_fragment", 2))
        {
            std::cout << "Recette incomplète : il faut 2 Oreilles de gobelin, 2 Crocs de loup et 2 Fragments de métal rouillé." << std::endl;
            std::cout << std::endl;
            return false;
        }

        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "goblin_ear", 2, exceptionalMajority);
        consumeRecipeIngredient(player, "wolf_fang", 2, exceptionalMajority);
        consumeRecipeIngredient(player, "rusted_metal_fragment", 2, exceptionalMajority);

        Weapon dagger(
            "Dague dentelée de traque",
            "Une dague courte faite pour les enchaînements rapides. Les crocs mal polis accrochent la cible.",
            125,
            WeaponType::Dagger,
            5,
            10,
            12,
            100
        );

        addCraftedWeaponWithExceptionalChance(player, dagger, exceptionalMajority);
        std::cout << "Tu ajustes les crocs contre la lame : la Dague dentelée de traque est prête." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftStabilizedMageRobe creates a safer magical armor from unstable materials.
    // FR: craftStabilizedMageRobe crée une armure magique plus sûre à partir de matériaux instables.
    bool craftStabilizedMageRobe(Player& player)
    {
        std::vector<RecipeIngredient> ingredients = {{"unstable_core", 1}, {"slime_residue", 2}, {"arcane_dust", 2}, {"worn_leather_piece", 1}};

        if (!hasRecipeIngredients(player, "unstable_core", 1)
            || !hasRecipeIngredients(player, "slime_residue", 2)
            || !hasRecipeIngredients(player, "arcane_dust", 2)
            || !hasRecipeIngredients(player, "worn_leather_piece", 1))
        {
            std::cout << "Recette incomplète : il faut 1 Noyau instable, 2 Résidus de slime, 2 Poussières arcaniques et 1 Morceau de cuir abîmé." << std::endl;
            std::cout << std::endl;
            return false;
        }

        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "unstable_core", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "slime_residue", 2, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 2, exceptionalMajority);
        consumeRecipeIngredient(player, "worn_leather_piece", 1, exceptionalMajority);

        Armor robe(
            "Robe stabilisée du laboratoire",
            "Une robe magique dont les coutures absorbent les ratés d'énergie avant qu'ils ne deviennent trop visibles.",
            260,
            ArmorType::Magical,
            55,
            7,
            115
        );

        addCraftedArmorWithExceptionalChance(player, robe, exceptionalMajority);
        std::cout << "Le noyau cesse de vibrer dans les coutures : la Robe stabilisée du laboratoire est terminée." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftWeightedOrcHammer creates a heavy weapon for slower physical classes.
    // FR: craftWeightedOrcHammer fabrique une arme lourde adaptée aux classes physiques lentes.
    bool craftWeightedOrcHammer(Player& player)
    {
        std::vector<RecipeIngredient> ingredients = {{"battle_torn_badge", 1}, {"rusted_metal_fragment", 6}, {"beast_hide", 1}, {"cracked_bone", 2}};

        if (!hasRecipeIngredients(player, "battle_torn_badge", 1)
            || !hasRecipeIngredients(player, "rusted_metal_fragment", 6)
            || !hasRecipeIngredients(player, "beast_hide", 1)
            || !hasRecipeIngredients(player, "cracked_bone", 2))
        {
            std::cout << "Recette incomplète : il faut 1 Insigne abîmé, 6 Fragments de métal rouillé, 1 Peau de bête robuste et 2 Os fissurés." << std::endl;
            std::cout << std::endl;
            return false;
        }

        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "battle_torn_badge", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "rusted_metal_fragment", 6, exceptionalMajority);
        consumeRecipeIngredient(player, "beast_hide", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "cracked_bone", 2, exceptionalMajority);

        Weapon hammer(
            "Marteau lesté d'arène",
            "Un marteau lent, malpoli et efficace. Il récompense les combattants qui acceptent de frapper moins souvent mais plus fort.",
            235,
            WeaponType::Hammer,
            9,
            18,
            3,
            135
        );

        addCraftedWeaponWithExceptionalChance(player, hammer, exceptionalMajority);
        std::cout << "Tu serres les plaques autour du manche : le Marteau lesté d'arène est prêt." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftPrecisionHarvestTools declares or implements a focused behavior used by this module.
    // FR: craftPrecisionHarvestTools déclare ou implémente un comportement précis utilisé par ce module.
    bool craftPrecisionHarvestTools(Player& player)
    {
        if (!hasRecipeIngredients(player, "rusted_metal_fragment", 3)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "worn_leather_piece", 2)
            || !hasRecipeIngredients(player, "arcane_dust", 1))
        {
            std::cout << "Recette incomplète : il faut 3 Fragments de métal rouillé, 2 Morceaux de cuir abîmé et 1 Poussière arcanique." << std::endl;
            std::cout << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "rusted_metal_fragment", 3);
        consumeRecipeIngredient(player, "worn_leather_piece", 2);
        consumeRecipeIngredient(player, "arcane_dust", 1);
        player.getInventory().addMaterial(MaterialCatalog::createPrecisionHarvestTools());

        std::cout << "Tu assembles des outils fins pour récupérer les ressources plus proprement." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftPreservationVials declares or implements a focused behavior used by this module.
    // FR: craftPreservationVials déclare ou implémente un comportement précis utilisé par ce module.
    bool craftPreservationVials(Player& player)
    {
        if (!hasRecipeIngredients(player, "slime_residue", 2)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "arcane_dust", 1)
            || !hasRecipeIngredients(player, "mountain_blue_flower", 1))
        {
            std::cout << "Recette incomplète : il faut 2 Résidus de slime, 1 Poussière arcanique et 1 Fleur bleue de montagne." << std::endl;
            std::cout << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "slime_residue", 2);
        consumeRecipeIngredient(player, "arcane_dust", 1);
        consumeRecipeIngredient(player, "mountain_blue_flower", 1);
        player.getInventory().addMaterial(MaterialCatalog::createPreservationVials());

        std::cout << "Tu obtiens des Fioles de conservation pour préserver les composants fragiles." << std::endl;
        std::cout << std::endl;
        return true;
    }

    // EN: craftMonsterDissectionGuide declares or implements a focused behavior used by this module.
    // FR: craftMonsterDissectionGuide déclare ou implémente un comportement précis utilisé par ce module.
    bool craftMonsterDissectionGuide(Player& player)
    {
        if (!hasRecipeIngredients(player, "goblin_ear", 2)
            // EN: !hasRecipeIngredients declares or implements a focused behavior used by this module.
            // FR: !hasRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
            || !hasRecipeIngredients(player, "wolf_fang", 2)
            || !hasRecipeIngredients(player, "battle_torn_badge", 1))
        {
            std::cout << "Recette incomplète : il faut 2 Oreilles de gobelin, 2 Crocs de loup et 1 Insigne abîmé d'aventurier." << std::endl;
            std::cout << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "goblin_ear", 2);
        consumeRecipeIngredient(player, "wolf_fang", 2);
        consumeRecipeIngredient(player, "battle_torn_badge", 1);
        player.getInventory().addMaterial(MaterialCatalog::createMonsterDissectionGuide());

        std::cout << "Tu compares plusieurs restes de combat et rédiges une vraie méthode de dissection." << std::endl;
        std::cout << std::endl;
        return true;
    }



    bool craftBarbedArrows(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"wolf_fang", 1}, {"worn_leather_piece", 1}}, "Flèches barbelées")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("barbed_arrows", 8));
        MaterialExperimentLog::recordCraft("Flèches barbelées", 8);
        std::cout << "Tu fabriques 8 flèches barbelées. Les effets spéciaux complets viendront avec le système de munitions avancées." << std::endl << std::endl;
        return true;
    }

    bool craftPiercingBolts(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"rusted_metal_fragment", 2}, {"arcane_dust", 1}}, "Carreaux perforants")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("piercing_bolts", 6));
        MaterialExperimentLog::recordCraft("Carreaux perforants", 6);
        std::cout << "Tu fabriques 6 carreaux perforants. C'est cher, mais pensé pour les cibles solides." << std::endl << std::endl;
        return true;
    }

    bool craftBalancedThrowingKnives(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"rusted_metal_fragment", 2}, {"worn_leather_piece", 1}}, "Couteaux de lancer équilibrés")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("balanced_throwing_knives", 5));
        MaterialExperimentLog::recordCraft("Couteaux de lancer équilibrés", 5);
        std::cout << "Tu fabriques 5 couteaux de lancer équilibrés. Les classes mobiles devraient mieux les rentabiliser." << std::endl << std::endl;
        return true;
    }

    bool craftAshArrows(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"training_arrows", 6}, {"arcane_dust", 1}}, "Flèches de cendre")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("ash_arrows", 6));
        MaterialExperimentLog::recordCraft("Flèches de cendre", 6);
        std::cout << "Tu transformes 6 flèches en flèches de cendre. Effet élémentaire complet prévu avec la tâche statuts." << std::endl << std::endl;
        return true;
    }

    bool craftFrozenBolts(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"training_bolts", 5}, {"mountain_blue_flower", 1}}, "Carreaux givrés")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("frozen_bolts", 5));
        MaterialExperimentLog::recordCraft("Carreaux givrés", 5);
        std::cout << "Tu fabriques 5 carreaux givrés. Pour l'instant, ils existent surtout comme munition élémentaire rare." << std::endl << std::endl;
        return true;
    }

    bool craftConductiveKnives(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"training_throwing_knives", 5}, {"rusted_metal_fragment", 2}, {"arcane_dust", 1}}, "Couteaux conducteurs")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("conductive_knives", 5));
        MaterialExperimentLog::recordCraft("Couteaux conducteurs", 5);
        std::cout << "Tu fabriques 5 couteaux conducteurs. Le futur système électrique les rendra beaucoup plus intéressants." << std::endl << std::endl;
        return true;
    }

    struct CraftRecipe
    {
        std::string name;
        std::string category;
        std::vector<RecipeIngredient> ingredients;
        bool alchemistOnly;
        bool blacksmithHint;
        std::function<bool(Player&)> craft;
    };

    std::string materialNameById(const std::string& id)
    {
        return MaterialCatalog::createById(id, 1).getName();
    }

    // EN: maxCraftsForRecipe declares or implements a focused behavior used by this module.
    // FR: maxCraftsForRecipe déclare ou implémente un comportement précis utilisé par ce module.
    int maxCraftsForRecipe(Player& player, const CraftRecipe& recipe)
    {
        if (recipe.alchemistOnly && !isAlchemist(player))
        {
            return 0;
        }

        if (recipe.ingredients.empty())
        {
            return 0;
        }

        int maxCrafts = 999999;

        for (const RecipeIngredient& ingredient : recipe.ingredients)
        {
            int requiredPoints = ingredient.quantity * 2;

            if (requiredPoints <= 0)
            {
                continue;
            }

            int possible = player.getInventory().countMaterialQualityPointsById(ingredient.id) / requiredPoints;
            maxCrafts = std::min(maxCrafts, possible);
        }

        if (maxCrafts == 999999)
        {
            return 0;
        }

        return std::max(0, maxCrafts);
    }

    // EN: displayRecipeIngredients declares or implements a focused behavior used by this module.
    // FR: displayRecipeIngredients déclare ou implémente un comportement précis utilisé par ce module.
    void displayRecipeIngredients(Player& player, const CraftRecipe& recipe)
    {
        for (const RecipeIngredient& ingredient : recipe.ingredients)
        {
            int requiredPoints = ingredient.quantity * 2;
            int ownedPoints = player.getInventory().countMaterialQualityPointsById(ingredient.id);
            int ownedEquivalent = ownedPoints / 2;

            std::cout << "- " << materialNameById(ingredient.id)
                      << " x" << ingredient.quantity
                      << " (possédé : " << player.getInventory().countMaterialById(ingredient.id)
                      << ", équiv. qualité normale : " << ownedEquivalent
                      << ", points : " << ownedPoints << "/" << requiredPoints
                      << ")" << std::endl;
        }
    }

    std::vector<CraftRecipe> buildCraftRecipes()
    {
        std::vector<CraftRecipe> recipes;

        // EN: recipes.push_back declares or implements a focused behavior used by this module.
        // FR: recipes.push_back déclare ou implémente un comportement précis utilisé par ce module.
        recipes.push_back({"Potion de soin", "Potion", {{"bitter_healing_leaf", 2}, {"slime_residue", 1}}, false, false, craftBasicHealingPotion});
        recipes.push_back({"Potion de soin renforcée", "Potion avancée", {{"mountain_blue_flower", 1}, {"arcane_dust", 1}}, true, false, craftReinforcedHealingPotion});
        // EN: recipes.push_back declares or implements a focused behavior used by this module.
        // FR: recipes.push_back déclare ou implémente un comportement précis utilisé par ce module.
        recipes.push_back({"Potion de rage", "Potion", {{"wolf_fang", 1}, {"arcane_dust", 1}}, false, false, craftDamagePotion});
        recipes.push_back({"Potion de rage supérieure", "Potion avancée", {{"wolf_fang", 2}, {"cracked_bone", 1}, {"arcane_dust", 1}}, true, false, craftReinforcedDamagePotion});
        // EN: recipes.push_back declares or implements a focused behavior used by this module.
        // FR: recipes.push_back déclare ou implémente un comportement précis utilisé par ce module.
        recipes.push_back({"Potion kitsune de soin", "Potion avancée", {{"kitsune_ember", 1}, {"mountain_blue_flower", 1}, {"slime_residue", 1}}, true, false, craftKitsuneHealingPotion});
        recipes.push_back({"Potion de rage instable", "Potion avancée", {{"unstable_core", 1}, {"wolf_fang", 2}, {"arcane_dust", 1}}, true, false, craftUnstableRagePotion});

        // EN: recipes.push_back declares or implements a focused behavior used by this module.
        // FR: recipes.push_back déclare ou implémente un comportement précis utilisé par ce module.
        recipes.push_back({"Kit de réparation faible", "Outil", {{"rusted_metal_fragment", 2}, {"worn_leather_piece", 1}, {"slime_residue", 1}}, false, true, craftRepairKit});
        recipes.push_back({"Kit de réparation moyen", "Outil", {{"weak_repair_kit", 1}, {"rusted_metal_fragment", 3}, {"worn_leather_piece", 1}, {"arcane_dust", 1}}, false, true, craftMediumRepairKit});
        // EN: recipes.push_back declares or implements a focused behavior used by this module.
        // FR: recipes.push_back déclare ou implémente un comportement précis utilisé par ce module.
        recipes.push_back({"Gros kit de réparation", "Outil", {{"medium_repair_kit", 1}, {"rusted_metal_fragment", 5}, {"worn_leather_piece", 3}, {"arcane_dust", 2}}, false, true, craftBigRepairKit});
        recipes.push_back({"Kit complet du bricoleur", "Outil", {{"big_repair_kit", 1}, {"rusted_metal_fragment", 8}, {"worn_leather_piece", 5}, {"arcane_dust", 4}, {"slime_residue", 3}}, false, true, craftTinkererCompleteRepairKit});
        // EN: recipes.push_back declares or implements a focused behavior used by this module.
        // FR: recipes.push_back déclare ou implémente un comportement précis utilisé par ce module.
        recipes.push_back({"Kit moyen par rafistolage renforcé", "Outil", {{"beast_hide", 1}, {"worn_leather_piece", 2}, {"slime_residue", 1}}, false, true, craftFieldArmorPatch});

        // EN: recipes.push_back declares or implements a focused behavior used by this module.
        // FR: recipes.push_back déclare ou implémente un comportement précis utilisé par ce module.
        recipes.push_back({"Lame de récupération", "Arme", {{"rusted_metal_fragment", 4}, {"worn_leather_piece", 1}, {"arcane_dust", 1}}, false, true, craftRecoveryBlade});
        recipes.push_back({"Dague cousue d'ombre", "Arme rare", {{"shadow_thread", 2}, {"rusted_metal_fragment", 3}, {"arcane_dust", 1}}, false, true, craftShadowThreadDagger});
        // EN: recipes.push_back declares or implements a focused behavior used by this module.
        // FR: recipes.push_back déclare ou implémente un comportement précis utilisé par ce module.
        recipes.push_back({"Arc recourbé aux crocs", "Arme", {{"wolf_fang", 3}, {"beast_hide", 1}, {"worn_leather_piece", 2}}, false, false, craftFangRecurveBow});
        recipes.push_back({"Bâton au noyau instable", "Arme magique", {{"unstable_core", 1}, {"arcane_dust", 3}, {"worn_leather_piece", 1}}, false, false, craftUnstableStaff});
        // EN: recipes.push_back declares or implements a focused behavior used by this module.
        // FR: recipes.push_back déclare ou implémente un comportement précis utilisé par ce module.
        recipes.push_back({"Armure de chasseur rafistolée", "Armure", {{"beast_hide", 2}, {"worn_leather_piece", 3}, {"slime_residue", 1}}, false, true, craftHunterLeatherArmor});
        recipes.push_back({"Armure rapiécée d'écailles", "Armure rare", {{"draconic_scale_fragment", 2}, {"beast_hide", 2}, {"arcane_dust", 2}}, false, true, craftDraconicPatchArmor});
        // EN: recipes.push_back declares or implements a focused behavior used by this module.
        // FR: recipes.push_back déclare ou implémente un comportement précis utilisé par ce module.
        recipes.push_back({"Robe aux braises kitsune", "Armure magique", {{"kitsune_ember", 2}, {"mountain_blue_flower", 1}, {"arcane_dust", 2}}, false, false, craftKitsuneEmberRobe});
        recipes.push_back({"Dague dentelée de traque", "Arme rare", {{"goblin_ear", 2}, {"wolf_fang", 2}, {"rusted_metal_fragment", 2}}, false, true, craftSerratedMonsterDagger});
        recipes.push_back({"Robe stabilisée du laboratoire", "Armure magique", {{"unstable_core", 1}, {"slime_residue", 2}, {"arcane_dust", 2}, {"worn_leather_piece", 1}}, false, false, craftStabilizedMageRobe});
        recipes.push_back({"Marteau lesté d'arène", "Arme lourde", {{"battle_torn_badge", 1}, {"rusted_metal_fragment", 6}, {"beast_hide", 1}, {"cracked_bone", 2}}, false, true, craftWeightedOrcHammer});

        // EN: recipes.push_back declares or implements a focused behavior used by this module.
        // FR: recipes.push_back déclare ou implémente un comportement précis utilisé par ce module.
        recipes.push_back({"Outils de récupération précise", "Outil passif", {{"rusted_metal_fragment", 3}, {"worn_leather_piece", 2}, {"arcane_dust", 1}}, false, false, craftPrecisionHarvestTools});
        recipes.push_back({"Fioles de conservation", "Outil passif", {{"slime_residue", 2}, {"arcane_dust", 1}, {"mountain_blue_flower", 1}}, false, false, craftPreservationVials});
        // EN: recipes.push_back declares or implements a focused behavior used by this module.
        // FR: recipes.push_back déclare ou implémente un comportement précis utilisé par ce module.
        recipes.push_back({"Technique passive : dissection de monstre", "Livre / technique", {{"goblin_ear", 2}, {"wolf_fang", 2}, {"battle_torn_badge", 1}}, false, false, craftMonsterDissectionGuide});
        recipes.push_back({"Flèches barbelées x8", "Munition", {{"wolf_fang", 1}, {"worn_leather_piece", 1}}, false, false, craftBarbedArrows});
        recipes.push_back({"Carreaux perforants x6", "Munition avancée", {{"rusted_metal_fragment", 2}, {"arcane_dust", 1}}, false, true, craftPiercingBolts});
        recipes.push_back({"Couteaux de lancer équilibrés x5", "Munition", {{"rusted_metal_fragment", 2}, {"worn_leather_piece", 1}}, false, true, craftBalancedThrowingKnives});
        recipes.push_back({"Flèches de cendre x6", "Munition élémentaire", {{"training_arrows", 6}, {"arcane_dust", 1}}, false, false, craftAshArrows});
        recipes.push_back({"Carreaux givrés x5", "Munition élémentaire", {{"training_bolts", 5}, {"mountain_blue_flower", 1}}, false, false, craftFrozenBolts});
        recipes.push_back({"Couteaux conducteurs x5", "Munition élémentaire", {{"training_throwing_knives", 5}, {"rusted_metal_fragment", 2}, {"arcane_dust", 1}}, false, true, craftConductiveKnives});

        return recipes;
    }


    // EN: useMaterialIfPossible declares or implements a focused behavior used by this module.
    // FR: useMaterialIfPossible déclare ou implémente un comportement précis utilisé par ce module.
    bool useMaterialIfPossible(Player& player, const Material& material)
    {
        if (material.getCategory() == "Livre" || material.getCategory() == "Renseignement")
        {
            return readMaterialIfPossible(material);
        }

        if (isRepairKitId(material.getId()))
        {
            std::cout << "Ce kit est prêt, mais la réparation se lance maintenant depuis l'arme ou l'armure à réparer." << std::endl;
            std::cout << "Inventaire > armes/armures > sélectionner l'équipement > Réparer." << std::endl;
            std::cout << std::endl;
            return false;
        }

        if (material.getId() == "bitter_healing_leaf" || material.getId() == "slime_residue")
        {
            if (isAlchemist(player))
            {
                std::cout << "Spécial Alchimiste : tu stabilises la préparation sans vendeur." << std::endl;
                std::cout << std::endl;
            }
            else
            {
                std::cout << "Tu tentes une recette de fortune. Un vrai Alchimiste ferait ça plus naturellement." << std::endl;
                std::cout << std::endl;
            }

            return craftBasicHealingPotion(player);
        }

        if (material.getId() == "mountain_blue_flower")
        {
            if (!isAlchemist(player))
            {
                std::cout << "Cette fleur demande une vraie main d'Alchimiste pour devenir une potion stable." << std::endl;
                std::cout << "[recette avancée réservée à l'Alchimiste pour l'instant]" << std::endl;
                std::cout << std::endl;
                return false;
            }

            return craftReinforcedHealingPotion(player);
        }

        if (material.getId() == "wolf_fang" || material.getId() == "arcane_dust" || material.getId() == "cracked_bone")
        {
            std::cout << "Choisis l'expérience à tenter avec ce composant." << std::endl;
            std::cout << "0 : Annuler" << std::endl;
            std::cout << "1 : Potion de rage simple" << std::endl;

            int maxChoice = 1;

            if (isAlchemist(player))
            {
                std::cout << "2 : Potion de soin renforcée" << std::endl;
                std::cout << "3 : Potion de rage supérieure" << std::endl;
                maxChoice = 3;
            }
            else
            {
                std::cout << "2 : Recettes avancées [réservées à l'Alchimiste pour l'instant]" << std::endl;
                maxChoice = 2;
            }

            if (isBlacksmith(player) || material.getId() == "arcane_dust")
            {
                std::cout << "4 : Améliorer / renforcer un kit de réparation" << std::endl;
                maxChoice = 4;
            }

            std::cout << "> ";

            int choice = Console::askNumberBetween(0, maxChoice, "Choix invalide.");
            std::cout << std::endl;

            if (choice == 1) return craftDamagePotion(player);

            if (choice == 2 && !isAlchemist(player))
            {
                std::cout << "Tu sens que le mélange pourrait mal tourner." << std::endl;
                std::cout << "Il faudra une recette apprise ou la classe Alchimiste pour stabiliser ça." << std::endl;
                std::cout << std::endl;
                return false;
            }

            if (choice == 2) return craftReinforcedHealingPotion(player);
            if (choice == 3) return craftReinforcedDamagePotion(player);
            if (choice == 4)
            {
                std::cout << "0 : Annuler" << std::endl;
                std::cout << "1 : Kit moyen (+50%)" << std::endl;
                std::cout << "2 : Gros kit (+75%)" << std::endl;
                std::cout << "3 : Kit complet du bricoleur (+95%)" << std::endl;
                std::cout << "> ";
                int kitChoice = Console::askNumberBetween(0, 3, "Choix invalide.");
                std::cout << std::endl;
                if (kitChoice == 1) return craftMediumRepairKit(player);
                if (kitChoice == 2) return craftBigRepairKit(player);
                if (kitChoice == 3) return craftTinkererCompleteRepairKit(player);
                return false;
            }
            return false;
        }

        if (material.getId() == "kitsune_ember" || material.getId() == "unstable_core")
        {
            if (!isAlchemist(player))
            {
                std::cout << "Ce composant est trop instable sans vraie maîtrise d'Alchimiste." << std::endl;
                std::cout << "Tu pourrais le garder pour une recette avancée plus tard." << std::endl;
                std::cout << std::endl;
                return false;
            }

            std::cout << "Choisis l'expérience alchimique avancée." << std::endl;
            std::cout << "0 : Annuler" << std::endl;
            std::cout << "1 : Potion de soin renforcée à la braise kitsune" << std::endl;
            std::cout << "2 : Potion de rage supérieure au noyau instable" << std::endl;
            std::cout << "> ";

            int choice = Console::askNumberBetween(0, 2, "Choix invalide.");
            std::cout << std::endl;

            if (choice == 1) return craftKitsuneHealingPotion(player);
            if (choice == 2) return craftUnstableRagePotion(player);
            return false;
        }

        if (material.getId() == "beast_hide")
        {
            std::cout << "Choisis l'expérience à tenter avec cette peau robuste." << std::endl;
            std::cout << "0 : Annuler" << std::endl;
            std::cout << "1 : Fabriquer un kit moyen par rafistolage renforcé" << std::endl;
            std::cout << "2 : Fabriquer une Armure de chasseur rafistolée" << std::endl;
            std::cout << "> ";
            int choice = Console::askNumberBetween(0, 2, "Choix invalide.");
            std::cout << std::endl;
            if (choice == 1) return craftFieldArmorPatch(player);
            if (choice == 2) return craftHunterLeatherArmor(player);
            return false;
        }

        if (material.getId() == "rusted_metal_fragment"
            || material.getId() == "worn_leather_piece")
        {
            std::cout << "Choisis l'expérience à tenter avec ce composant." << std::endl;
            std::cout << "0 : Annuler" << std::endl;
            std::cout << "1 : Fabriquer un Kit de réparation faible" << std::endl;
            std::cout << "2 : Fabriquer une Lame de récupération" << std::endl;
            std::cout << "3 : Fabriquer une Armure de chasseur rafistolée" << std::endl;
            std::cout << "> ";

            int choice = Console::askNumberBetween(0, 3, "Choix invalide.");
            std::cout << std::endl;

            if (choice == 1) return craftRepairKit(player);
            if (choice == 2) return craftRecoveryBlade(player);
            if (choice == 3) return craftHunterLeatherArmor(player);
            return false;
        }

        std::cout << "Tu manipules " << material.getName() << ", mais son vrai système d'utilisation n'est pas encore stabilisé." << std::endl;
        std::cout << "[action en cours de développement]" << std::endl;
        std::cout << std::endl;
        return false;
    }
}
// EN: openWeapons declares or implements a focused behavior used by this module.
// FR: openWeapons déclare ou implémente un comportement précis utilisé par ce module.
bool InventorySelection::openWeapons(Player& player)
{
    if (player.getInventory().getWeaponCount() <= 0)
    {
        std::cout << "Tu n'as aucune arme dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    player.getInventory().displayWeaponList();

    std::cout << "Sélectionne une arme, ou entre -1 pour revenir." << std::endl;
    std::cout << "> ";

    int index = Console::askNumberBetween(
        -1,
        player.getInventory().getWeaponCount() - 1,
        "Choix invalide. Entre un numéro d'arme valide, ou -1 pour revenir."
    );

    Console::clear();

    if (index == -1)
    {
        return false;
    }

    if (!player.getInventory().hasWeapon(index))
    {
        std::cout << "Cette arme n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Weapon weapon = player.getInventory().getWeapon(index);
    InventoryDisplay::displaySelectedWeapon(weapon);

    int action = Console::askNumberBetween(
        0,
        4,
        "Choix invalide. Entre 0, 1, 2, 3 ou 4."
    );

    Console::clear();

    if (action == 1)
    {
        player.getInventory().inspectWeapon(index);
        return false;
    }

    if (action == 3)
    {
        BestiaryMenu::displayObjectEntry(weapon.getName());
        return false;
    }

    if (action == 4)
    {
        return repairSelectedWeapon(player, index);
    }

    if (action == 2)
    {
        if (player.equipWeapon(index))
        {
            Weapon equippedWeapon = player.getEquippedWeapon();

            std::cout << player.getName() << " équipe : " << equippedWeapon.getName() << "." << std::endl;

            if (equippedWeapon.isBroken())
            {
                std::cout << "Attention : cette arme est cassée, elle ne donnera aucun bonus." << std::endl;
            }
            else
            {
                std::cout << "La prise en main est bonne. Cette arme est prête au combat." << std::endl;
            }

            std::cout << std::endl;
        }
        else
        {
            std::cout << "Impossible d'équiper cette arme." << std::endl;
            std::cout << std::endl;
        }
    }

    return false;
}

// EN: openArmors declares or implements a focused behavior used by this module.
// FR: openArmors déclare ou implémente un comportement précis utilisé par ce module.
bool InventorySelection::openArmors(Player& player)
{
    if (player.getInventory().getArmorCount() <= 0)
    {
        std::cout << "Tu n'as aucune armure dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    player.getInventory().displayArmorList();

    std::cout << "Sélectionne une armure, ou entre -1 pour revenir." << std::endl;
    std::cout << "> ";

    int index = Console::askNumberBetween(
        -1,
        player.getInventory().getArmorCount() - 1,
        "Choix invalide. Entre un numéro d'armure valide, ou -1 pour revenir."
    );

    Console::clear();

    if (index == -1)
    {
        return false;
    }

    if (!player.getInventory().hasArmor(index))
    {
        std::cout << "Cette armure n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Armor armor = player.getInventory().getArmor(index);
    InventoryDisplay::displaySelectedArmor(armor);

    int action = Console::askNumberBetween(
        0,
        4,
        "Choix invalide. Entre 0, 1, 2, 3 ou 4."
    );

    Console::clear();

    if (action == 1)
    {
        player.getInventory().inspectArmor(index);
        return false;
    }

    if (action == 3)
    {
        BestiaryMenu::displayObjectEntry(armor.getName());
        return false;
    }

    if (action == 4)
    {
        return repairSelectedArmor(player, index);
    }

    if (action == 2)
    {
        if (player.equipArmor(index))
        {
            Armor equippedArmor = player.getEquippedArmor();

            std::cout << player.getName() << " équipe : " << equippedArmor.getName() << "." << std::endl;

            if (equippedArmor.isBroken())
            {
                std::cout << "Attention : cette armure est cassée, elle ne donnera aucun bonus." << std::endl;
            }
            else
            {
                std::cout << "Ses protections sont maintenant actives." << std::endl;
            }

            std::cout << player.getName() << " possède maintenant "
                      << player.getHp()
                      << "/"
                      << player.getMaxHp()
                      << " PV."
                      << std::endl;
            std::cout << std::endl;
        }
        else
        {
            std::cout << "Impossible d'équiper cette armure." << std::endl;
            std::cout << std::endl;
        }
    }

    return false;
}

// EN: openConsumables declares or implements a focused behavior used by this module.
// FR: openConsumables déclare ou implémente un comportement précis utilisé par ce module.
bool InventorySelection::openConsumables(Player& player)
{
    bool hasClassicConsumables = player.getInventory().getConsumableCount() > 0;
    bool hasRepairKits = hasAnyRepairKit(player);

    if (!hasClassicConsumables && !hasRepairKits)
    {
        std::cout << "Tu n'as aucun consommable dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    std::vector<ConsumableGroup> groups = InventoryUtils::groupConsumables(player);

    std::cout << "============ CONSOMMABLES ============" << std::endl;

    for (int i = 0; i < static_cast<int>(groups.size()); ++i)
    {
        const ConsumableGroup& group = groups[i];

        std::cout << i + 1
                  << " : "
                  << group.name
                  << " x"
                  << group.amount
                  << " | "
                  << InventoryUtils::consumableTypeToText(group.type)
                  << " | Puissance : "
                  << group.power
                  << std::endl;
    }

    displayRepairKitsAsConsumables(player);

    std::cout << "=======================================" << std::endl;

    if (groups.empty())
    {
        std::cout << "Aucun consommable utilisable directement ici." << std::endl;
        std::cout << "Les kits se lancent depuis l'équipement à réparer." << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "> ";
        Console::askNumberBetween(0, 0, "Entre 0 pour revenir.");
        Console::clear();
        return false;
    }

    std::cout << "Sélectionne un consommable, ou entre 0 pour revenir." << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(
        0,
        static_cast<int>(groups.size()),
        "Choix invalide. Sélectionne un consommable affiché, ou 0 pour revenir."
    );

    Console::clear();

    if (choice == 0)
    {
        return false;
    }

    int index = groups[choice - 1].firstIndex;

    if (!player.getInventory().hasConsumable(index))
    {
        std::cout << "Ce consommable n'existe plus dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Consumable consumable = player.getInventory().getConsumable(index);

    InventoryDisplay::displaySelectedConsumable(consumable);

    int action = Console::askNumberBetween(
        0,
        3,
        "Choix invalide. Entre 0, 1, 2 ou 3."
    );

    Console::clear();

    if (action == 1)
    {
        player.getInventory().inspectConsumable(index);
        return false;
    }

    if (action == 3)
    {
        BestiaryMenu::displayObjectEntry(consumable.getName());
        return false;
    }

    if (action == 2)
    {
        if (consumable.getType() != ConsumableType::Healing)
        {
            std::cout << "Ce consommable demande une cible ou un effet spécial." << std::endl;
            std::cout << "Utilise plutôt l'option Potions du menu de combat." << std::endl;
            std::cout << std::endl;
            return false;
        }

        player.heal(consumable.getPower());
        ThreatSystem::markSelfHealingAction(player);

        if (!player.hasInfiniteConsumables())
        {
            player.getInventory().removeConsumable(index);
        }

        std::cout << player.getName() << " utilise : " << consumable.getName() << "." << std::endl;
        std::cout << "Ses blessures se referment, et il récupère "
                  << consumable.getPower()
                  << " PV."
                  << std::endl;
        std::cout << player.getName() << " possède maintenant "
                  << player.getHp()
                  << "/"
                  << player.getMaxHp()
                  << " PV."
                  << std::endl;
        std::cout << std::endl;

        return true;
    }

    return false;
}

// EN: openMaterials declares or implements a focused behavior used by this module.
// FR: openMaterials déclare ou implémente un comportement précis utilisé par ce module.
bool InventorySelection::openMaterials(Player& player)
{
    if (player.getInventory().getMaterials().empty())
    {
        std::cout << "Tu n'as aucun matériau, plante ou renseignement dans ton inventaire." << std::endl;
        std::cout << "Les loots, les boutiques et la bibliothèque pourront remplir cette partie." << std::endl;
        std::cout << std::endl;
        return false;
    }

    player.getInventory().displayMaterialList();

    std::cout << "Sélectionne une entrée, ou entre -1 pour revenir." << std::endl;
    std::cout << "> ";

    int index = Console::askNumberBetween(
        -1,
        static_cast<int>(player.getInventory().getMaterials().size()) - 1,
        "Choix invalide. Entre un numéro valide, ou -1 pour revenir."
    );

    Console::clear();

    if (index == -1)
    {
        return false;
    }

    if (!player.getInventory().hasMaterial(index))
    {
        std::cout << "Cette entrée n'existe pas dans ton inventaire." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Material material = player.getInventory().getMaterial(index);
    InventoryDisplay::displaySelectedMaterial(material);

    int action = Console::askNumberBetween(
        0,
        4,
        "Choix invalide. Entre 0, 1, 2, 3 ou 4."
    );

    Console::clear();

    if (action == 1)
    {
        player.getInventory().inspectMaterial(index);
        return false;
    }

    if (action == 2)
    {
        BestiaryMenu::displayObjectEntry(material.getName());
        return false;
    }

    if (action == 3)
    {
        displayMaterialUtility(material);
        return false;
    }

    if (action == 4)
    {
        return useMaterialIfPossible(player, material);
    }

    return false;
}
// EN: openCraft declares or implements a focused behavior used by this module.
// FR: openCraft déclare ou implémente un comportement précis utilisé par ce module.
bool InventorySelection::openCraft(Player& player)
{
    int page = 0;
    const int recipesPerPage = 8;

    while (true)
    {
        std::vector<CraftRecipe> recipes = buildCraftRecipes();
        int pageCount = static_cast<int>((recipes.size() + recipesPerPage - 1) / recipesPerPage);

        if (pageCount <= 0)
        {
            std::cout << "Aucun schéma de craft n'est disponible." << std::endl;
            std::cout << std::endl;
            return false;
        }

        if (page < 0) page = 0;
        if (page >= pageCount) page = pageCount - 1;

        int firstIndex = page * recipesPerPage;
        int lastIndex = std::min(firstIndex + recipesPerPage, static_cast<int>(recipes.size()));

        MenuFrame::title("CRAFT / SCHÉMAS");
        std::cout << "Page " << page + 1 << "/" << pageCount << std::endl;
        MenuFrame::separator();
        MenuFrame::backOption("Retour");

        for (int i = firstIndex; i < lastIndex; ++i)
        {
            int visibleChoice = i - firstIndex + 1;
            int maxCrafts = maxCraftsForRecipe(player, recipes[i]);

            std::cout << visibleChoice << " : " << recipes[i].name
                      << " | " << recipes[i].category
                      << " | max : " << maxCrafts;

            if (recipes[i].alchemistOnly && !isAlchemist(player))
            {
                std::cout << " | réservé Alchimiste";
            }
            else if (recipes[i].blacksmithHint && isBlacksmith(player))
            {
                std::cout << " | maîtrise Forgeron";
            }

            std::cout << std::endl;
        }

        if (pageCount > 1)
        {
            MenuFrame::separator();
            std::cout << "9 : Page suivante" << std::endl;
            std::cout << "-1 : Page précédente" << std::endl;
        }

        MenuFrame::end();
        std::cout << "Les max utilisent l'équivalence de qualité : faible/impur compte moins, pur/haute qualité/exceptionnel compte plus." << std::endl;
        MenuFrame::prompt();

        int maxVisibleChoice = lastIndex - firstIndex;
        int choice = Console::askNumberBetween(
            pageCount > 1 ? -1 : 0,
            pageCount > 1 ? 9 : maxVisibleChoice,
            "Choix invalide. Sélectionne un schéma affiché, une page, ou 0 pour revenir."
        );

        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (pageCount > 1 && choice == 9)
        {
            page = (page + 1) % pageCount;
            continue;
        }

        if (pageCount > 1 && choice == -1)
        {
            page = (page - 1 + pageCount) % pageCount;
            continue;
        }

        if (choice < 1 || choice > maxVisibleChoice)
        {
            std::cout << "Ce numéro ne correspond à aucun schéma sur cette page." << std::endl;
            std::cout << std::endl;
            continue;
        }

        CraftRecipe recipe = recipes[firstIndex + choice - 1];
        int maxCrafts = maxCraftsForRecipe(player, recipe);

        MenuFrame::title("SCHÉMA DE CRAFT");
        std::cout << "Schéma : " << recipe.name << std::endl;
        std::cout << "Catégorie : " << recipe.category << std::endl;
        std::cout << "Nombre maximum possible : " << maxCrafts << std::endl;

        if (recipe.alchemistOnly && !isAlchemist(player))
        {
            std::cout << "Condition : réservé à l'Alchimiste pour l'instant." << std::endl;
        }

        if (recipe.blacksmithHint && isBlacksmith(player))
        {
            std::cout << "Bonus métier : le Forgeron comprend mieux cette fabrication." << std::endl;
        }

        std::cout << std::endl;
        std::cout << "Matériaux nécessaires pour 1 craft :" << std::endl;
        displayRecipeIngredients(player, recipe);
        MenuFrame::end();
        std::cout << std::endl;

        if (maxCrafts <= 0)
        {
            std::cout << "Tu ne peux pas fabriquer ce schéma pour le moment." << std::endl;
            std::cout << std::endl;
            continue;
        }

        int quantityToCraft = 1;

        if (maxCrafts > 1)
        {
            std::cout << "Combien de fois veux-tu fabriquer ce schéma ?" << std::endl;
            std::cout << "0 : Annuler" << std::endl;
            std::cout << "1-" << maxCrafts << " : Quantité à fabriquer" << std::endl;
            std::cout << "> ";

            quantityToCraft = Console::askNumberBetween(0, maxCrafts, "Quantité invalide.");
            std::cout << std::endl;

            if (quantityToCraft == 0)
            {
                continue;
            }
        }

        std::cout << "Confirmer la fabrication : " << recipe.name << " x" << quantityToCraft << " ?" << std::endl;
        std::cout << "1 : Confirmer" << std::endl;
        std::cout << "0 : Annuler" << std::endl;
        std::cout << "> ";

        int confirm = Console::askNumberBetween(0, 1, "Choix invalide.");
        std::cout << std::endl;

        if (confirm == 0)
        {
            continue;
        }

        int crafted = 0;

        for (int i = 0; i < quantityToCraft; ++i)
        {
            if (!recipe.craft(player))
            {
                break;
            }

            ++crafted;
        }

        if (crafted > 0)
        {
            MaterialExperimentLog::recordCraft(recipe.name, crafted);
        }

        std::cout << "Résumé craft : " << crafted << "/" << quantityToCraft << " fabrication(s) réussie(s)." << std::endl;
        std::cout << std::endl;
    }
}
