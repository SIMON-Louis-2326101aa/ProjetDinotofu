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
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
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
#include <cstddef>
#include <sstream>
#include <ostream>
#include <random>
#include <string>
#include <vector>


namespace
{

    std::string yesNoText(bool value, const std::string& yesText, const std::string& noText)
    {
        return value ? yesText : noText;
    }

    void showInventoryNotice(const std::string& title, const std::string& screenId, const std::vector<std::string>& lines)
    {
        MessageScreen::show(title, screenId, lines);
    }


    class InventoryNoticeStream
    {
    public:
        template <typename T>
        InventoryNoticeStream& operator<<(const T& value)
        {
            current << value;
            return *this;
        }

        using StreamManipulator = std::ostream& (*)(std::ostream&);

        InventoryNoticeStream& operator<<(StreamManipulator manipulator)
        {
            if (manipulator == static_cast<StreamManipulator>(std::endl<char, std::char_traits<char>>))
            {
                flushLine();
            }
            else
            {
                manipulator(current);
            }
            return *this;
        }

        void flushPending()
        {
            std::string line = current.str();
            if (!line.empty())
            {
                flushLine();
            }
            showPending();
        }

    private:
        std::ostringstream current;
        std::vector<std::string> lines;

        void flushLine()
        {
            std::string line = current.str();
            while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
            {
                line.pop_back();
            }

            current.str(std::string());
            current.clear();

            if (line.empty())
            {
                showPending();
                return;
            }

            lines.push_back(line);
        }

        void showPending()
        {
            if (lines.empty())
            {
                return;
            }

            MessageScreen::show("INVENTAIRE", "inventory.notice.stream", lines, false);
            lines.clear();
        }
    };

    InventoryNoticeStream inventoryNotice;

    void showWeaponInspectionScreen(const Weapon& weapon)
    {
        std::vector<std::string> lines;
        lines.push_back("Nom : " + weapon.getName());
        lines.push_back("Description : " + weapon.getDescription());
        lines.push_back("Bonus dégâts : +" + std::to_string(weapon.getMinDamageBonus()) + " à +" + std::to_string(weapon.getMaxDamageBonus()));
        lines.push_back("Bonus critique : +" + std::to_string(weapon.getCriticalBonus()));
        lines.push_back("Durabilité : " + InventoryUtils::weaponDurabilityText(weapon));
        lines.push_back("État : " + yesNoText(weapon.isBroken(), "Cassée, ses bonus ne s'appliquent plus.", "Utilisable"));
        lines.push_back("Valeur : " + std::to_string(weapon.getValue()) + " pièces");
        MessageScreen::show("INSPECTION - ARME", "inventory.weapon.inspect.details", lines);
    }

    void showArmorInspectionScreen(const Armor& armor)
    {
        std::vector<std::string> lines;
        lines.push_back("Nom : " + armor.getName());
        lines.push_back("Description : " + armor.getDescription());
        lines.push_back("Bonus PV max : +" + std::to_string(armor.getMaxHpBonus()));
        lines.push_back("Réduction dégâts : " + std::to_string(armor.getDamageReduction()));
        lines.push_back("Durabilité : " + InventoryUtils::armorDurabilityText(armor));
        lines.push_back("État : " + yesNoText(armor.isBroken(), "Cassée, ses bonus ne s'appliquent plus.", "Utilisable"));
        lines.push_back("Valeur : " + std::to_string(armor.getValue()) + " pièces");
        MessageScreen::show("INSPECTION - ARMURE", "inventory.armor.inspect.details", lines);
    }

    void showConsumableInspectionScreen(const Consumable& consumable)
    {
        MessageScreen::show(
            "INSPECTION - CONSOMMABLE",
            "inventory.consumable.inspect.details",
            {
                "Nom : " + consumable.getName(),
                "Description : " + consumable.getDescription(),
                "Type : " + InventoryUtils::consumableTypeToText(consumable.getType()),
                "Puissance : " + std::to_string(consumable.getPower()),
                "Valeur : " + std::to_string(consumable.getValue()) + " pièces"
            }
        );
    }

    void showMaterialInspectionScreen(const Material& material)
    {
        std::vector<std::string> lines;
        lines.push_back("Nom : " + material.getName());
        lines.push_back("Description : " + material.getDescription());
        lines.push_back("Catégorie : " + material.getCategory());
        lines.push_back("Quantité : " + std::to_string(material.getQuantity()));
        lines.push_back("Qualité : " + material.getQualityLabel());
        lines.push_back("Valeur : " + std::to_string(material.getValue()) + " pièces/unité");

        if (material.hasSpecialQuality())
        {
            lines.push_back("Note : cette qualité peut influencer le craft, le prix ou certaines expérimentations.");
        }

        MessageScreen::show("INSPECTION - ENTRÉE", "inventory.material.inspect.details", lines);
    }

    void showEquipWeaponResultScreen(const Player& player, const Weapon& weapon, bool success)
    {
        if (!success)
        {
            MessageScreen::show(
                "ARME NON ÉQUIPÉE",
                "inventory.weapon.equip.failed",
                {"Impossible d'équiper cette arme.", "Elle a peut-être été déplacée, retirée ou rendue indisponible."}
            );
            return;
        }

        std::vector<std::string> lines;
        lines.push_back(player.getName() + " équipe : " + weapon.getName() + ".");
        lines.push_back("Durabilité : " + InventoryUtils::weaponDurabilityText(weapon));
        lines.push_back(weapon.isBroken()
            ? "Attention : cette arme est cassée, elle ne donnera aucun bonus."
            : "La prise en main est bonne. Cette arme est prête au combat.");
        MessageScreen::show("ARME ÉQUIPÉE", "inventory.weapon.equip.result", lines);
    }

    void showEquipArmorResultScreen(const Player& player, const Armor& armor, bool success)
    {
        if (!success)
        {
            MessageScreen::show(
                "ARMURE NON ÉQUIPÉE",
                "inventory.armor.equip.failed",
                {"Impossible d'équiper cette armure.", "Elle a peut-être été déplacée, retirée ou rendue indisponible."}
            );
            return;
        }

        std::vector<std::string> lines;
        lines.push_back(player.getName() + " équipe : " + armor.getName() + ".");
        lines.push_back("Durabilité : " + InventoryUtils::armorDurabilityText(armor));
        lines.push_back(armor.isBroken()
            ? "Attention : cette armure est cassée, elle ne donnera aucun bonus."
            : "Ses protections sont maintenant actives.");
        lines.push_back(player.getName() + " possède maintenant " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()) + " PV.");
        MessageScreen::show("ARMURE ÉQUIPÉE", "inventory.armor.equip.result", lines);
    }

    void showHealingConsumableResultScreen(const Player& player, const Consumable& consumable, int hpBefore)
    {
        MessageScreen::show(
            "CONSOMMABLE UTILISÉ",
            "inventory.consumable.use.heal.result",
            {
                player.getName() + " utilise : " + consumable.getName() + ".",
                "Soin théorique : " + std::to_string(consumable.getPower()) + " PV.",
                "PV avant : " + std::to_string(hpBefore) + "/" + std::to_string(player.getMaxHp()),
                "PV après : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()),
                player.hasInfiniteConsumables() ? "Code actif : le consommable n'est pas retiré." : "Consommable retiré de l'inventaire."
            }
        );
    }

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

    std::string materialStockText(Player& player, const std::string& id, int needed)
    {
        int owned = player.getInventory().countMaterialById(id);
        int qualityPoints = player.getInventory().countMaterialQualityPointsById(id);
        int requiredPoints = needed * 2;

        std::ostringstream line;
        line << MaterialCatalog::createById(id, 1).getName()
             << " x" << needed
             << " | possédé : " << owned
             << " | équiv. qualité normale : " << qualityPoints / 2
             << " | points : " << qualityPoints << "/" << requiredPoints;

        if (qualityPoints < requiredPoints)
        {
            line << " | manquant";
        }

        return line.str();
    }

    std::vector<std::string> buildRepairMaterialCostLines(Player& player, bool armorRepair, int thresholdPercent)
    {
        std::vector<std::string> lines;

        if (armorRepair)
        {
            lines.push_back(materialStockText(player, "worn_leather_piece", thresholdPercent >= 75 ? 2 : 1));

            if (thresholdPercent >= 75)
            {
                lines.push_back(materialStockText(player, "beast_hide", 1));
            }
        }
        else
        {
            lines.push_back(materialStockText(player, "rusted_metal_fragment", thresholdPercent >= 75 ? 2 : 1));
        }

        if (thresholdPercent >= 50)
        {
            lines.push_back(materialStockText(player, "slime_residue", 1));
        }

        if (thresholdPercent >= 75)
        {
            lines.push_back(materialStockText(player, "arcane_dust", 1));
        }

        if (thresholdPercent >= 95)
        {
            lines.push_back(materialStockText(player, "draconic_scale_fragment", 1));
        }

        return lines;
    }

    std::string repairKitStatusText(const std::string& kitId)
    {
        if (isUsedRepairKitId(kitId))
        {
            return "Entamé [" + std::to_string(repairKitCurrentDurability(kitId)) + "/" + std::to_string(repairKitMaxDurability(kitId)) + "]";
        }

        return "Intact [" + std::to_string(repairKitMaxDurability(kitId)) + "/" + std::to_string(repairKitMaxDurability(kitId)) + "]";
    }

    std::string repairKitWearResultText(const std::string& kitId, bool kitDurabilitySaved)
    {
        if (kitDurabilitySaved)
        {
            return "Spécial Forgeron : l'usure du kit est économisée cette fois-ci.";
        }

        int remainingDurability = repairKitCurrentDurability(kitId) - 1;

        if (remainingDurability > 0)
        {
            return "Le kit perd 1 durabilité. Il passe à " + std::to_string(remainingDurability) + "/" + std::to_string(repairKitMaxDurability(kitId)) + ".";
        }

        return "Le kit perd sa dernière durabilité et disparaît.";
    }

    void showCraftResultScreen(const std::string& recipeName, int crafted, int requested)
    {
        std::vector<std::string> lines;
        lines.push_back("Schéma : " + recipeName);
        lines.push_back("Fabrication(s) réussie(s) : " + std::to_string(crafted) + "/" + std::to_string(requested) + ".");

        if (crafted <= 0)
        {
            lines.push_back("Rien n'a été produit. Les composants ont été vérifiés avant l'essai ou la fabrication s'est arrêtée immédiatement.");
        }
        else
        {
            lines.push_back("Les composants consommés et les objets obtenus sont enregistrés dans l'inventaire.");
            lines.push_back("Le journal d'expérimentation garde la trace de cette fabrication.");
        }

        MessageScreen::show("RÉSUMÉ CRAFT", "inventory.craft.result", lines);
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
            inventoryNotice << "Spécial Alchimiste : tu récupères/économises " << catalystName << " pendant la préparation." << std::endl;
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

    // EN: chooseRepairKit declares or implements a focused behavior used by this module.
    // FR: chooseRepairKit déclare ou implémente un comportement précis utilisé par ce module.
    int chooseRepairKit(Player& player)
    {
        std::vector<RepairKitChoice> choices = getRepairKitChoices(player);

        if (choices.empty())
        {
            MessageScreen::show(
                "AUCUN KIT",
                "inventory.repair.kit.empty",
                {
                    "Tu n'as aucun kit de réparation disponible.",
                    "Les kits restent visibles avec les consommables, mais la réparation se lance depuis une arme ou une armure."
                }
            );
            return -1;
        }

        MenuScreen screen("CHOIX DU KIT", "inventory.repair.kit.choice");
        screen.addLine("Sélectionne le kit à utiliser pour cette réparation.");
        screen.addBackOption("Annuler", "inventory.repair.kit.cancel");

        for (int i = 0; i < static_cast<int>(choices.size()); ++i)
        {
            const RepairKitChoice& choice = choices[i];
            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "material";
            itemData.section = "Réparation";
            itemData.actionType = "repair_kit";
            itemData.name = choice.label;
            itemData.quantity = std::to_string(choice.quantity);
            itemData.detail = "Seuil de réparation : " + std::to_string(choice.thresholdPercent) + "%";
            itemData.status = repairKitStatusText(choice.id);
            itemData.progress = "Durabilité kit : " + std::to_string(choice.currentDurability) + "/" + std::to_string(choice.maxDurability);

            screen.addOption(
                i + 1,
                choice.label + " x" + std::to_string(choice.quantity),
                repairKitStatusText(choice.id) + " | seuil : " + std::to_string(choice.thresholdPercent) + "%",
                true,
                "inventory.repair.kit.select",
                itemData
            );
        }

        int selected = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un kit affiché, ou 0 pour annuler.");
        Console::clear();

        if (selected == 0)
        {
            return -1;
        }

        if (selected < 1 || selected > static_cast<int>(choices.size()))
        {
            MessageScreen::show(
                "KIT INVALIDE",
                "inventory.repair.kit.invalid",
                {"Ce numéro ne correspond à aucun kit affiché.", "La réparation est annulée pour éviter de consommer le mauvais outil."}
            );
            return -1;
        }

        return selected - 1;
    }


    // EN: confirmRepairCost declares or implements a focused behavior used by this module.
    // FR: confirmRepairCost déclare ou implémente un comportement précis utilisé par ce module.
    bool confirmRepairCost(Player& player, const std::string& itemName, bool armorRepair, const RepairKitChoice& kitChoice)
    {
        MenuScreen screen("CONFIRMATION RÉPARATION", "inventory.repair.confirm");
        screen.addLine("Équipement : " + itemName);
        screen.addLine("Kit : " + kitChoice.label + " | " + repairKitStatusText(kitChoice.id));
        screen.addLine("Seuil permis : " + std::to_string(kitChoice.thresholdPercent) + "% de la durabilité maximale.");
        screen.addLine("Matériaux nécessaires :");

        for (const std::string& line : buildRepairMaterialCostLines(player, armorRepair, kitChoice.thresholdPercent))
        {
            screen.addLine("- " + line);
        }

        if (!hasRepairMaterials(player, armorRepair, kitChoice.thresholdPercent))
        {
            screen.addLine("Matériaux insuffisants pour cette réparation.");
            screen.addBackOption("Retour", "inventory.repair.confirm.back");
            TerminalInterface::askMenuChoice(screen, 0, 0, "Entre 0 pour revenir.");
            Console::clear();
            return false;
        }

        screen.addBackOption("Annuler", "inventory.repair.cancel");
        screen.addOption(1, "Continuer et réparer", "Le kit et les composants indiqués seront utilisés.", true, "inventory.repair.accept");

        int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une option affichée.");
        Console::clear();
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

    // EN: repairSelectedWeapon declares or implements a focused behavior used by this module.
    // FR: repairSelectedWeapon déclare ou implémente un comportement précis utilisé par ce module.
    bool repairSelectedWeapon(Player& player, int weaponIndex)
    {
        Weapon* weapon = player.getInventory().getMutableWeapon(weaponIndex);

        if (weapon == nullptr)
        {
            MessageScreen::show("ARME INTROUVABLE", "inventory.weapon.repair.missing", {"Cette arme n'existe plus.", "La réparation est annulée."});
            return false;
        }

        if (weapon->isIndestructible())
        {
            MessageScreen::show("RÉPARATION INUTILE", "inventory.weapon.repair.indestructible", {"Cette arme est indestructible.", "Elle n'a pas besoin de réparation."});
            return false;
        }

        if (weapon->getDurability() >= weapon->getMaxDurability())
        {
            MessageScreen::show("ARME INTACTE", "inventory.weapon.repair.full", {"Cette arme est déjà en parfait état.", "Aucun kit n'est consommé."});
            return false;
        }

        std::vector<RepairKitChoice> choices = getRepairKitChoices(player);
        int kitChoiceIndex = chooseRepairKit(player);

        if (kitChoiceIndex < 0)
        {
            return false;
        }

        RepairKitChoice kitChoice = choices[kitChoiceIndex];
        int durabilityBefore = weapon->getDurability();
        int cap = repairedDurabilityCap(weapon->getMaxDurability(), kitChoice.thresholdPercent);

        if (weapon->getDurability() >= cap)
        {
            MessageScreen::show(
                "KIT TROP FAIBLE",
                "inventory.weapon.repair.cap_blocked",
                {
                    "Arme : " + weapon->getName(),
                    "Durabilité actuelle : " + std::to_string(weapon->getDurability()) + "/" + std::to_string(weapon->getMaxDurability()),
                    "Seuil du kit : " + std::to_string(cap) + "/" + std::to_string(weapon->getMaxDurability()),
                    "Ce kit ne peut pas améliorer davantage cette arme."
                }
            );
            return false;
        }

        if (!confirmRepairCost(player, weapon->getName(), false, kitChoice))
        {
            return false;
        }

        std::string kitStatusBefore = repairKitStatusText(kitChoice.id);
        std::vector<std::string> consumedLines = buildRepairMaterialCostLines(player, false, kitChoice.thresholdPercent);
        consumeRepairMaterials(player, false, kitChoice.thresholdPercent);
        bool kitDurabilitySaved = false;
        applyRepairKitWear(player, kitChoice.id, kitDurabilitySaved);

        int amountToRepair = cap - weapon->getDurability();
        weapon->repair(amountToRepair);

        std::vector<std::string> lines;
        lines.push_back("Arme : " + weapon->getName());
        lines.push_back("Kit utilisé : " + kitChoice.label + " | état avant usage : " + kitStatusBefore);
        lines.push_back("Durabilité avant : " + std::to_string(durabilityBefore) + "/" + std::to_string(weapon->getMaxDurability()));
        lines.push_back("Durabilité après : " + std::to_string(weapon->getDurability()) + "/" + std::to_string(weapon->getMaxDurability()));
        lines.push_back("Matériaux consommés :");
        for (const std::string& line : consumedLines)
        {
            lines.push_back("- " + line);
        }
        lines.push_back(repairKitWearResultText(kitChoice.id, kitDurabilitySaved));

        MessageScreen::show("ARME RÉPARÉE", "inventory.weapon.repair.result", lines);
        return false;
    }


    // EN: repairSelectedArmor declares or implements a focused behavior used by this module.
    // FR: repairSelectedArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool repairSelectedArmor(Player& player, int armorIndex)
    {
        Armor* armor = player.getInventory().getMutableArmor(armorIndex);

        if (armor == nullptr)
        {
            MessageScreen::show("ARMURE INTROUVABLE", "inventory.armor.repair.missing", {"Cette armure n'existe plus.", "La réparation est annulée."});
            return false;
        }

        if (armor->isIndestructible())
        {
            MessageScreen::show("RÉPARATION INUTILE", "inventory.armor.repair.indestructible", {"Cette armure est indestructible.", "Elle n'a pas besoin de réparation."});
            return false;
        }

        if (armor->getDurability() >= armor->getMaxDurability())
        {
            MessageScreen::show("ARMURE INTACTE", "inventory.armor.repair.full", {"Cette armure est déjà en parfait état.", "Aucun kit n'est consommé."});
            return false;
        }

        std::vector<RepairKitChoice> choices = getRepairKitChoices(player);
        int kitChoiceIndex = chooseRepairKit(player);

        if (kitChoiceIndex < 0)
        {
            return false;
        }

        RepairKitChoice kitChoice = choices[kitChoiceIndex];
        int durabilityBefore = armor->getDurability();
        int cap = repairedDurabilityCap(armor->getMaxDurability(), kitChoice.thresholdPercent);

        if (armor->getDurability() >= cap)
        {
            MessageScreen::show(
                "KIT TROP FAIBLE",
                "inventory.armor.repair.cap_blocked",
                {
                    "Armure : " + armor->getName(),
                    "Durabilité actuelle : " + std::to_string(armor->getDurability()) + "/" + std::to_string(armor->getMaxDurability()),
                    "Seuil du kit : " + std::to_string(cap) + "/" + std::to_string(armor->getMaxDurability()),
                    "Ce kit ne peut pas améliorer davantage cette armure."
                }
            );
            return false;
        }

        if (!confirmRepairCost(player, armor->getName(), true, kitChoice))
        {
            return false;
        }

        std::string kitStatusBefore = repairKitStatusText(kitChoice.id);
        std::vector<std::string> consumedLines = buildRepairMaterialCostLines(player, true, kitChoice.thresholdPercent);
        consumeRepairMaterials(player, true, kitChoice.thresholdPercent);
        bool kitDurabilitySaved = false;
        applyRepairKitWear(player, kitChoice.id, kitDurabilitySaved);

        int amountToRepair = cap - armor->getDurability();
        armor->repair(amountToRepair);

        std::vector<std::string> lines;
        lines.push_back("Armure : " + armor->getName());
        lines.push_back("Kit utilisé : " + kitChoice.label + " | état avant usage : " + kitStatusBefore);
        lines.push_back("Durabilité avant : " + std::to_string(durabilityBefore) + "/" + std::to_string(armor->getMaxDurability()));
        lines.push_back("Durabilité après : " + std::to_string(armor->getDurability()) + "/" + std::to_string(armor->getMaxDurability()));
        lines.push_back("Matériaux consommés :");
        for (const std::string& line : consumedLines)
        {
            lines.push_back("- " + line);
        }
        lines.push_back(repairKitWearResultText(kitChoice.id, kitDurabilitySaved));

        MessageScreen::show("ARMURE RÉPARÉE", "inventory.armor.repair.result", lines);
        return false;
    }


    // EN: displayMaterialUtility declares or implements a focused behavior used by this module.
    // FR: displayMaterialUtility déclare ou implémente un comportement précis utilisé par ce module.
    void displayMaterialUtility(const Material& material)
    {
        std::vector<std::string> lines;
        lines.push_back(material.getName() + " | " + material.getCategory());

        if (material.getCategory() != "Livre" && material.getCategory() != "Renseignement" && material.getCategory() != "Outil")
        {
            lines.push_back("Qualité : " + material.getQualityLabel());
            lines.push_back("Craft : faible/impur compte moins, pur/haute qualité compte plus.");
        }

        if (isRepairKitId(material.getId()))
        {
            lines.push_back("Usage : réparation autonome jusqu'à environ " + std::to_string(repairThresholdPercentForKit(material.getId())) + "% de durabilité.");
            lines.push_back("Durabilité par kit : " + std::to_string(repairKitCurrentDurability(material.getId())) + "/" + std::to_string(repairKitMaxDurability(material.getId())));
            lines.push_back("Kits empilés ici : " + std::to_string(material.getQuantity()));
            lines.push_back("La réparation se lance depuis l'arme ou l'armure, pas directement depuis le kit.");
        }
        else if (material.getId() == "rusted_metal_fragment" || material.getId() == "worn_leather_piece")
        {
            lines.push_back("Usages connus : craft basique, réparation et amélioration d'équipement commun.");
        }
        else if (material.getId() == "wolf_fang" || material.getId() == "goblin_ear" || material.getId() == "cracked_bone")
        {
            lines.push_back("Usages connus : trophées, recettes de monstres, contrats de guilde et artisanat spécialisé.");
        }
        else if (material.getId() == "arcane_dust")
        {
            lines.push_back("Usages connus : enchantements, catalyseurs de sorts et équipements magiques.");
        }
        else if (material.getId() == "slime_residue")
        {
            lines.push_back("Usages connus : colle, pièges, potions et réparations de fortune.");
        }
        else if (material.getCategory() == "Plante")
        {
            lines.push_back("Usages connus : potions, remèdes, quêtes botaniques et expériences.");
        }
        else if (material.getCategory() == "Livre" || material.getCategory() == "Renseignement")
        {
            lines.push_back("Usages connus : lecture, recoupement d'archives et progression du bestiaire.");
        }
        else if (material.getId() == "battle_torn_badge")
        {
            lines.push_back("Usages connus : guildes, contrats, réputation et rencontres d'aventuriers.");
        }
        else if (material.getId() == "beast_hide")
        {
            lines.push_back("Usage supposé : armures, réparations épaisses, équipements lourds et survie.");
        }
        else if (material.getId() == "shadow_thread")
        {
            lines.push_back("Usages connus : ombres de Hazak, objets d'assassin, nécromancie et améliorations furtives.");
        }
        else if (material.getId() == "kitsune_ember")
        {
            lines.push_back("Usages connus : flammes kitsune, invocations d'Aoi, potions avancées et enchantements feu.");
        }
        else if (material.getId() == "draconic_scale_fragment")
        {
            lines.push_back("Usages connus : protections rares, armures lourdes et crafts semi-dragons.");
        }
        else if (material.getId() == "unstable_core")
        {
            lines.push_back("Usage supposé : alchimie risquée, expériences de Fail, invocations instables et objets explosifs.");
        }
        else if (material.getId() == "fitoria_feather")
        {
            lines.push_back("Usage supposé : bénédictions, soins rares, équipements de lumière et reliques angéliques.");
        }
        else if (material.getId() == "zelef_demon_blood")
        {
            lines.push_back("Usages connus : alchimie dangereuse, armes démoniaques, malédictions et contrats sombres.");
        }
        else if (material.getId() == "atlas_broken_plate")
        {
            lines.push_back("Usages connus : armures lourdes, réparation extrême, reliques défensives et forge de haut niveau.");
        }
        else if (material.getId() == "precision_harvest_tools")
        {
            lines.push_back("Usages connus : bonus passif de récupération propre tant que l'outil est conservé.");
        }
        else if (material.getId() == "preservation_vials")
        {
            lines.push_back("Usages connus : bonus passif de conservation des composants fragiles.");
        }
        else if (material.getId() == "clean_harvest_manual")
        {
            lines.push_back("Usages connus : technique passive de récolte propre après lecture.");
        }
        else if (material.getId() == "monster_dissection_guide")
        {
            lines.push_back("Usages connus : technique passive pour récupérer des composants de monstres.");
        }
        else
        {
            lines.push_back("Usage supposé : craft, réparation ou expérimentation artisanale.");
        }

        MessageScreen::show("UTILITÉ PRÉVUE", "inventory.material.utility", lines);
    }


    // EN: readMaterialIfPossible declares or implements a focused behavior used by this module.
    // FR: readMaterialIfPossible déclare ou implémente un comportement précis utilisé par ce module.
    bool readMaterialIfPossible(const Material& material)
    {
        if (material.getCategory() != "Livre" && material.getCategory() != "Renseignement")
        {
            inventoryNotice << "Tu manipules " << material.getName() << ", mais tu ne trouves pas comment l'utiliser sans risque." << std::endl;
            inventoryNotice << "[rien ne se passe]" << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        BestiaryRuntimeProgress::unlockCommonInformation(material.getId());

        inventoryNotice << "Tu prends le temps de lire : " << material.getName() << "." << std::endl;
        if (material.getId() == "clean_harvest_manual" || material.getId() == "monster_dissection_guide")
        {
            inventoryNotice << "Technique passive comprise : ses effets restent actifs tant que l'apprentissage est conservé dans l'inventaire." << std::endl;
        }
        inventoryNotice << "Les informations compatibles sont ajoutées ou confirmées dans le bestiaire de session." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Matériaux exceptionnels majoritaires : la fabrication ajoute une faible particularité à l'objet créé." << std::endl;
            inventoryNotice << "Chance appliquée : " << chance << "% grâce à la qualité et au métier." << std::endl;
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
            inventoryNotice << "Matériaux exceptionnels majoritaires : l'arme gagne une faible particularité." << std::endl;
            inventoryNotice << "Chance appliquée : " << chance << "% grâce à la qualité et au métier." << std::endl;
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
            inventoryNotice << "Matériaux exceptionnels majoritaires : l'armure gagne une faible particularité." << std::endl;
            inventoryNotice << "Chance appliquée : " << chance << "% grâce à la qualité et au métier." << std::endl;
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
                inventoryNotice << "Il manque des composants pour : " << recipeName << "." << std::endl << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 2 Feuilles amères de soin et 1 Résidu de slime." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"bitter_healing_leaf", 2}, {"slime_residue", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "bitter_healing_leaf", 2, exceptionalMajority);
        consumeRecipeIngredient(player, "slime_residue", 1, exceptionalMajority);
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createBasicHealingPotion(), exceptionalMajority);
        maybeRestoreAlchemistCatalyst(player, "slime_residue", "le résidu de slime");

        inventoryNotice << "Tu écrases les feuilles, stabilises le mélange avec le résidu de slime," << std::endl;
        inventoryNotice << "et obtiens une Potion de soin." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 1 Fleur bleue de montagne et 1 Poussière arcanique." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"mountain_blue_flower", 1}, {"arcane_dust", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "mountain_blue_flower", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 1, exceptionalMajority);
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createReinforcedHealingPotion(), exceptionalMajority);
        maybeRestoreAlchemistCatalyst(player, "arcane_dust", "la poussière arcanique");

        inventoryNotice << "La fleur bleue absorbe la poussière arcanique sans se briser." << std::endl;
        inventoryNotice << "Tu obtiens une Potion de soin renforcée." << std::endl;
        inventoryNotice << std::endl;
        return true;
    }


    bool craftAntidotePotion(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"slime_residue", 1}, {"bitter_healing_leaf", 1}}, "Antidote simple")) return false;
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createAntidotePotion(), false);
        maybeRestoreAlchemistCatalyst(player, "slime_residue", "le résidu de slime");
        inventoryNotice << "Tu filtres le résidu et obtiens un Antidote simple contre les poisons faibles." << std::endl << std::endl;
        return true;
    }

    bool craftBurnSalvePotion(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"bitter_healing_leaf", 1}, {"slime_residue", 1}, {"arcane_dust", 1}}, "Baume anti-brûlure")) return false;
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createBurnSalvePotion(), false);
        maybeRestoreAlchemistCatalyst(player, "arcane_dust", "la poussière arcanique");
        inventoryNotice << "Tu obtiens un Baume anti-brûlure, assez stable pour stopper une brûlure faible." << std::endl << std::endl;
        return true;
    }

    bool craftFrostResistancePotion(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"mountain_blue_flower", 1}, {"slime_residue", 1}}, "Potion tiède anti-givre")) return false;
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createFrostResistancePotion(), false);
        maybeRestoreAlchemistCatalyst(player, "slime_residue", "le résidu de slime");
        inventoryNotice << "Tu obtiens une Potion tiède anti-givre, utile contre les ralentissements froids." << std::endl << std::endl;
        return true;
    }

    bool craftShockResistancePotion(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"rusted_metal_fragment", 1}, {"arcane_dust", 1}, {"slime_residue", 1}}, "Potion isolante")) return false;
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createShockResistancePotion(), false);
        maybeRestoreAlchemistCatalyst(player, "arcane_dust", "la poussière arcanique");
        inventoryNotice << "Tu obtiens une Potion isolante, pensée pour couper les décharges avant le prochain geste raté." << std::endl << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 1 Croc de loup et 1 Poussière arcanique." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"wolf_fang", 1}, {"arcane_dust", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "wolf_fang", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 1, exceptionalMajority);
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createBasicDamagePotion(), exceptionalMajority);
        maybeRestoreAlchemistCatalyst(player, "arcane_dust", "la poussière arcanique");

        inventoryNotice << "Le croc garde une agressivité étrange une fois broyé." << std::endl;
        inventoryNotice << "Tu obtiens une Potion de rage." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 2 Crocs de loup, 1 Os fissuré et 1 Poussière arcanique." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"wolf_fang", 2}, {"cracked_bone", 1}, {"arcane_dust", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "wolf_fang", 2, exceptionalMajority);
        consumeRecipeIngredient(player, "cracked_bone", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 1, exceptionalMajority);
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createReinforcedDamagePotion(), exceptionalMajority);
        maybeRestoreAlchemistCatalyst(player, "arcane_dust", "la poussière arcanique");

        inventoryNotice << "L'os fissuré absorbe la rage du croc et la poussière arcanique la stabilise à peine." << std::endl;
        inventoryNotice << "Tu obtiens une Potion de rage supérieure." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 1 Braise kitsune, 1 Fleur bleue de montagne et 1 Résidu de slime." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"kitsune_ember", 1}, {"mountain_blue_flower", 1}, {"slime_residue", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "kitsune_ember", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "mountain_blue_flower", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "slime_residue", 1, exceptionalMajority);
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createReinforcedHealingPotion(), exceptionalMajority);
        maybeRestoreAlchemistCatalyst(player, "slime_residue", "le résidu de slime");

        inventoryNotice << "La braise kitsune réchauffe la fleur sans la brûler." << std::endl;
        inventoryNotice << "Tu obtiens une Potion de soin renforcée." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 1 Noyau instable, 1 Croc de loup et 1 Poussière arcanique." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        std::vector<RecipeIngredient> ingredients = {{"unstable_core", 1}, {"wolf_fang", 1}, {"arcane_dust", 1}};
        bool exceptionalMajority = recipeUsesExceptionalMajority(player, ingredients);
        consumeRecipeIngredient(player, "unstable_core", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "wolf_fang", 1, exceptionalMajority);
        consumeRecipeIngredient(player, "arcane_dust", 1, exceptionalMajority);
        addCraftedConsumableWithExceptionalChance(player, ConsumableCatalog::createReinforcedDamagePotion(), exceptionalMajority);
        maybeRestoreAlchemistCatalyst(player, "arcane_dust", "la poussière arcanique");

        inventoryNotice << "Le noyau instable pulse une dernière fois avant d'être scellé." << std::endl;
        inventoryNotice << "Tu obtiens une Potion de rage supérieure." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 2 Fragments de métal rouillé, 1 Morceau de cuir abîmé et 1 Résidu de slime." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "rusted_metal_fragment", 2);
        consumeRecipeIngredient(player, "worn_leather_piece", 1);
        consumeRecipeIngredient(player, "slime_residue", 1);
        player.getInventory().addMaterial(MaterialCatalog::createWeakRepairKit());

        inventoryNotice << "Tu relies le cuir, coinces les fragments métalliques et utilises le slime comme liant." << std::endl;
        inventoryNotice << "Tu obtiens un Kit de réparation faible. Sa durabilité est limitée." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 1 Kit faible intact, 3 Fragments de métal rouillé, 1 Morceau de cuir abîmé et 1 Poussière arcanique." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "weak_repair_kit", 1);
        consumeRecipeIngredient(player, "rusted_metal_fragment", 3);
        consumeRecipeIngredient(player, "worn_leather_piece", 1);
        consumeRecipeIngredient(player, "arcane_dust", 1);
        player.getInventory().addMaterial(MaterialCatalog::createMediumRepairKit());

        inventoryNotice << "La poussière arcanique stabilise les plaques de réparation." << std::endl;
        inventoryNotice << "Tu obtiens un Kit de réparation moyen." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 1 Kit moyen intact, 5 Fragments de métal rouillé, 3 Morceaux de cuir abîmé et 2 Poussières arcaniques." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "medium_repair_kit", 1);
        consumeRecipeIngredient(player, "rusted_metal_fragment", 5);
        consumeRecipeIngredient(player, "worn_leather_piece", 3);
        consumeRecipeIngredient(player, "arcane_dust", 2);
        player.getInventory().addMaterial(MaterialCatalog::createBigRepairKit());

        inventoryNotice << "Tu renforces les attaches et solidifies les outils." << std::endl;
        inventoryNotice << "Tu obtiens un Gros kit de réparation." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 1 Gros kit intact, 8 Fragments de métal rouillé, 5 Morceaux de cuir abîmé, 4 Poussières arcaniques et 3 Résidus de slime." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "big_repair_kit", 1);
        consumeRecipeIngredient(player, "rusted_metal_fragment", 8);
        consumeRecipeIngredient(player, "worn_leather_piece", 5);
        consumeRecipeIngredient(player, "arcane_dust", 4);
        consumeRecipeIngredient(player, "slime_residue", 3);
        player.getInventory().addMaterial(MaterialCatalog::createTinkererCompleteRepairKit());

        inventoryNotice << "Tu assembles une vraie trousse presque complète." << std::endl;
        inventoryNotice << "Tu obtiens un Kit complet du bricoleur." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 1 Peau de bête robuste, 2 Morceaux de cuir abîmé et 1 Résidu de slime." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "beast_hide", 1);
        consumeRecipeIngredient(player, "worn_leather_piece", 2);
        consumeRecipeIngredient(player, "slime_residue", 1);
        player.getInventory().addMaterial(MaterialCatalog::createMediumRepairKit());

        inventoryNotice << "Tu transformes la peau robuste en plaques de rafistolage renforcées." << std::endl;
        inventoryNotice << "Tu obtiens un Kit de réparation moyen." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 4 Fragments de métal rouillé, 1 Morceau de cuir abîmé et 1 Poussière arcanique." << std::endl;
            inventoryNotice << std::endl;
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

        inventoryNotice << "Tu ajustes les fragments et obtiens une Lame de récupération." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 2 Peaux de bête robustes, 3 Morceaux de cuir abîmé et 1 Résidu de slime." << std::endl;
            inventoryNotice << std::endl;
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

        inventoryNotice << "Tu tends la peau, renforces les coutures et obtiens une Armure de chasseur rafistolée." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 2 Fils d'ombre, 3 Fragments de métal rouillé et 1 Poussière arcanique." << std::endl;
            inventoryNotice << std::endl;
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
        inventoryNotice << "Tu tends le fil d'ombre autour de la lame et obtiens une Dague cousue d'ombre." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 3 Crocs de loup, 1 Peau de bête robuste et 2 Morceaux de cuir abîmé." << std::endl;
            inventoryNotice << std::endl;
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
        inventoryNotice << "Tu tends le cuir et fixes les crocs : l'Arc recourbé aux crocs est prêt." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 1 Noyau instable, 3 Poussières arcaniques et 1 Morceau de cuir abîmé." << std::endl;
            inventoryNotice << std::endl;
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
        inventoryNotice << "Le noyau pulse au bout du bâton. C'est probablement stable. Probablement." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 2 Fragments d'écaille draconique, 2 Peaux de bête robustes et 2 Poussières arcaniques." << std::endl;
            inventoryNotice << std::endl;
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
        inventoryNotice << "Tu fixes les écailles sur une base robuste : l'armure encaisse déjà mieux." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 2 Braises kitsune, 1 Fleur bleue de montagne et 2 Poussières arcaniques." << std::endl;
            inventoryNotice << std::endl;
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
        inventoryNotice << "Les braises se calment autour du tissu : la Robe aux braises kitsune est terminée." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 2 Oreilles de gobelin, 2 Crocs de loup et 2 Fragments de métal rouillé." << std::endl;
            inventoryNotice << std::endl;
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
        inventoryNotice << "Tu ajustes les crocs contre la lame : la Dague dentelée de traque est prête." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 1 Noyau instable, 2 Résidus de slime, 2 Poussières arcaniques et 1 Morceau de cuir abîmé." << std::endl;
            inventoryNotice << std::endl;
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
        inventoryNotice << "Le noyau cesse de vibrer dans les coutures : la Robe stabilisée du laboratoire est terminée." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 1 Insigne abîmé, 6 Fragments de métal rouillé, 1 Peau de bête robuste et 2 Os fissurés." << std::endl;
            inventoryNotice << std::endl;
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
        inventoryNotice << "Tu serres les plaques autour du manche : le Marteau lesté d'arène est prêt." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 3 Fragments de métal rouillé, 2 Morceaux de cuir abîmé et 1 Poussière arcanique." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "rusted_metal_fragment", 3);
        consumeRecipeIngredient(player, "worn_leather_piece", 2);
        consumeRecipeIngredient(player, "arcane_dust", 1);
        player.getInventory().addMaterial(MaterialCatalog::createPrecisionHarvestTools());

        inventoryNotice << "Tu assembles des outils fins pour récupérer les ressources plus proprement." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 2 Résidus de slime, 1 Poussière arcanique et 1 Fleur bleue de montagne." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "slime_residue", 2);
        consumeRecipeIngredient(player, "arcane_dust", 1);
        consumeRecipeIngredient(player, "mountain_blue_flower", 1);
        player.getInventory().addMaterial(MaterialCatalog::createPreservationVials());

        inventoryNotice << "Tu obtiens des Fioles de conservation pour préserver les composants fragiles." << std::endl;
        inventoryNotice << std::endl;
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
            inventoryNotice << "Recette incomplète : il faut 2 Oreilles de gobelin, 2 Crocs de loup et 1 Insigne abîmé d'aventurier." << std::endl;
            inventoryNotice << std::endl;
            return false;
        }

        consumeRecipeIngredient(player, "goblin_ear", 2);
        consumeRecipeIngredient(player, "wolf_fang", 2);
        consumeRecipeIngredient(player, "battle_torn_badge", 1);
        player.getInventory().addMaterial(MaterialCatalog::createMonsterDissectionGuide());

        inventoryNotice << "Tu compares plusieurs restes de combat et rédiges une vraie méthode de dissection." << std::endl;
        inventoryNotice << std::endl;
        return true;
    }



    bool craftBarbedArrows(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"wolf_fang", 1}, {"worn_leather_piece", 1}}, "Flèches barbelées")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("barbed_arrows", 8));
        MaterialExperimentLog::recordCraft("Flèches barbelées", 8);
        inventoryNotice << "Tu fabriques 8 flèches barbelées. Elles peuvent maintenant provoquer du saignement sur une attaque réussie." << std::endl << std::endl;
        return true;
    }

    bool craftPiercingBolts(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"rusted_metal_fragment", 2}, {"arcane_dust", 1}}, "Carreaux perforants")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("piercing_bolts", 6));
        MaterialExperimentLog::recordCraft("Carreaux perforants", 6);
        inventoryNotice << "Tu fabriques 6 carreaux perforants. C'est cher, mais pensé pour les cibles solides." << std::endl << std::endl;
        return true;
    }

    bool craftBalancedThrowingKnives(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"rusted_metal_fragment", 2}, {"worn_leather_piece", 1}}, "Couteaux de lancer équilibrés")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("balanced_throwing_knives", 5));
        MaterialExperimentLog::recordCraft("Couteaux de lancer équilibrés", 5);
        inventoryNotice << "Tu fabriques 5 couteaux de lancer équilibrés. Les classes mobiles devraient mieux les rentabiliser." << std::endl << std::endl;
        return true;
    }

    bool craftAshArrows(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"training_arrows", 6}, {"arcane_dust", 1}}, "Flèches de cendre")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("ash_arrows", 6));
        MaterialExperimentLog::recordCraft("Flèches de cendre", 6);
        inventoryNotice << "Tu transformes 6 flèches en flèches de cendre. Elles peuvent maintenant accrocher une brûlure faible sur une attaque réussie." << std::endl << std::endl;
        return true;
    }

    bool craftFrozenBolts(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"training_bolts", 5}, {"mountain_blue_flower", 1}}, "Carreaux givrés")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("frozen_bolts", 5));
        MaterialExperimentLog::recordCraft("Carreaux givrés", 5);
        inventoryNotice << "Tu fabriques 5 carreaux givrés. Ils peuvent maintenant ralentir une cible avec du givre." << std::endl << std::endl;
        return true;
    }

    bool craftConductiveKnives(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"training_throwing_knives", 5}, {"rusted_metal_fragment", 2}, {"arcane_dust", 1}}, "Couteaux conducteurs")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("conductive_knives", 5));
        MaterialExperimentLog::recordCraft("Couteaux conducteurs", 5);
        inventoryNotice << "Tu fabriques 5 couteaux conducteurs. Ils peuvent maintenant infliger un choc, surtout contre l'équipement métallique." << std::endl << std::endl;
        return true;
    }

    bool craftVenomArrows(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"training_arrows", 6}, {"slime_residue", 1}, {"goblin_ear", 1}}, "Flèches enduites de venin")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("venom_arrows", 6));
        MaterialExperimentLog::recordCraft("Flèches enduites de venin", 6);
        inventoryNotice << "Tu fabriques 6 flèches enduites de venin. Elles sont sales, pas nobles, mais efficaces si le tir blesse vraiment." << std::endl << std::endl;
        return true;
    }

    bool craftShockBolts(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"training_bolts", 5}, {"rusted_metal_fragment", 3}, {"arcane_dust", 2}}, "Carreaux à pointe conductrice")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("shock_bolts", 5));
        MaterialExperimentLog::recordCraft("Carreaux à pointe conductrice", 5);
        inventoryNotice << "Tu fabriques 5 carreaux conducteurs. Très utile contre les ennemis équipés de métal." << std::endl << std::endl;
        return true;
    }

    bool craftSmokeKnives(Player& player)
    {
        if (!consumeRecipeIngredients(player, {{"training_throwing_knives", 5}, {"slime_residue", 1}, {"arcane_dust", 1}}, "Couteaux fumigènes")) return false;
        player.getInventory().addMaterial(MaterialCatalog::createById("smoke_knives", 5));
        MaterialExperimentLog::recordCraft("Couteaux fumigènes", 5);
        inventoryNotice << "Tu fabriques 5 couteaux fumigènes. Ce n'est pas magique : le projectile éclate une petite poudre qui gêne l'ennemi." << std::endl << std::endl;
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

    std::vector<std::string> buildRecipeIngredientLines(Player& player, const CraftRecipe& recipe)
    {
        std::vector<std::string> lines;

        for (const RecipeIngredient& ingredient : recipe.ingredients)
        {
            int requiredPoints = ingredient.quantity * 2;
            int ownedPoints = player.getInventory().countMaterialQualityPointsById(ingredient.id);
            int ownedEquivalent = ownedPoints / 2;

            std::ostringstream line;
            line << "- " << materialNameById(ingredient.id)
                 << " x" << ingredient.quantity
                 << " (possédé : " << player.getInventory().countMaterialById(ingredient.id)
                 << ", équiv. qualité normale : " << ownedEquivalent
                 << ", points : " << ownedPoints << "/" << requiredPoints
                 << ")";

            lines.push_back(line.str());
        }

        return lines;
    }


    std::vector<CraftRecipe> buildCraftRecipes()
    {
        std::vector<CraftRecipe> recipes;

        // EN: recipes.push_back declares or implements a focused behavior used by this module.
        // FR: recipes.push_back déclare ou implémente un comportement précis utilisé par ce module.
        recipes.push_back({"Potion de soin", "Potion", {{"bitter_healing_leaf", 2}, {"slime_residue", 1}}, false, false, craftBasicHealingPotion});
        recipes.push_back({"Potion de soin renforcée", "Potion avancée", {{"mountain_blue_flower", 1}, {"arcane_dust", 1}}, true, false, craftReinforcedHealingPotion});
        recipes.push_back({"Antidote simple", "Potion anti-statut", {{"slime_residue", 1}, {"bitter_healing_leaf", 1}}, false, false, craftAntidotePotion});
        recipes.push_back({"Baume anti-brûlure", "Potion anti-statut", {{"bitter_healing_leaf", 1}, {"slime_residue", 1}, {"arcane_dust", 1}}, true, false, craftBurnSalvePotion});
        recipes.push_back({"Potion tiède anti-givre", "Potion anti-statut", {{"mountain_blue_flower", 1}, {"slime_residue", 1}}, false, false, craftFrostResistancePotion});
        recipes.push_back({"Potion isolante", "Potion anti-statut", {{"rusted_metal_fragment", 1}, {"arcane_dust", 1}, {"slime_residue", 1}}, true, false, craftShockResistancePotion});
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
        recipes.push_back({"Flèches enduites de venin x6", "Munition toxique", {{"training_arrows", 6}, {"slime_residue", 1}, {"goblin_ear", 1}}, false, false, craftVenomArrows});
        recipes.push_back({"Carreaux à pointe conductrice x5", "Munition électrique", {{"training_bolts", 5}, {"rusted_metal_fragment", 3}, {"arcane_dust", 2}}, false, true, craftShockBolts});
        recipes.push_back({"Couteaux fumigènes x5", "Munition utilitaire", {{"training_throwing_knives", 5}, {"slime_residue", 1}, {"arcane_dust", 1}}, false, false, craftSmokeKnives});

        return recipes;
    }


    bool runMaterialCraftAttempt(Player& player, const std::string& recipeName, const std::string& screenId, const std::function<bool(Player&)>& craftAction)
    {
        bool success = false;
        {
            success = craftAction(player);
        }

        MessageScreen::show(
            success ? "EXPÉRIENCE RÉUSSIE" : "EXPÉRIENCE IMPOSSIBLE",
            screenId,
            {
                "Essai : " + recipeName,
                success
                    ? "Le résultat est ajouté à l'inventaire ou confirmé dans le registre correspondant."
                    : "Les composants nécessaires ne sont pas réunis, ou la préparation demande une meilleure maîtrise.",
                "Les détails de composants restent consultables dans le menu Craft / schémas."
            }
        );

        return success;
    }

    // EN: useMaterialIfPossible declares or implements a focused behavior used by this module.
    // FR: useMaterialIfPossible déclare ou implémente un comportement précis utilisé par ce module.
    bool useMaterialIfPossible(Player& player, const Material& material)
    {
        if (material.getCategory() == "Livre" || material.getCategory() == "Renseignement")
        {
            bool read = false;
            {
                read = readMaterialIfPossible(material);
            }

            MessageScreen::show(
                "LECTURE",
                "inventory.material.read.result",
                {
                    "Entrée : " + material.getName(),
                    read ? "Lecture utile confirmée." : "Le contenu est parcouru, puis archivé mentalement.",
                    "Les informations compatibles sont ajoutées ou confirmées dans le bestiaire de session."
                }
            );
            return read;
        }

        if (isRepairKitId(material.getId()))
        {
            MessageScreen::show(
                "KIT DE RÉPARATION",
                "inventory.material.repair_kit.info",
                {
                    "Ce kit est prêt, mais la réparation se lance depuis l'arme ou l'armure à réparer.",
                    "Chemin conseillé : Inventaire > armes/armures > sélectionner l'équipement > Réparer.",
                    "Durabilité du kit : " + std::to_string(repairKitCurrentDurability(material.getId())) + "/" + std::to_string(repairKitMaxDurability(material.getId()))
                }
            );
            return false;
        }

        if (material.getId() == "bitter_healing_leaf" || material.getId() == "slime_residue")
        {
            MessageScreen::show(
                "PRÉPARATION DE FORTUNE",
                "inventory.material.quick_alchemy.info",
                {
                    isAlchemist(player)
                        ? "Spécial Alchimiste : tu stabilises naturellement la préparation."
                        : "Tu tentes une recette de fortune. Un vrai Alchimiste ferait ça plus naturellement.",
                    "Essai proposé : Potion de soin."
                }
            );
            return runMaterialCraftAttempt(player, "Potion de soin", "inventory.material.quick_alchemy.result", craftBasicHealingPotion);
        }

        if (material.getId() == "mountain_blue_flower")
        {
            if (!isAlchemist(player))
            {
                MessageScreen::show(
                    "RECETTE INSTABLE",
                    "inventory.material.alchemist_locked",
                    {
                        "Cette fleur demande une vraie main d'Alchimiste pour devenir une potion stable.",
                        "[◘ recette avancée réservée à l'Alchimiste]"
                    }
                );
                return false;
            }

            return runMaterialCraftAttempt(player, "Potion de soin renforcée", "inventory.material.mountain_flower.result", craftReinforcedHealingPotion);
        }

        if (material.getId() == "wolf_fang" || material.getId() == "arcane_dust" || material.getId() == "cracked_bone")
        {
            MenuScreen screen("EXPÉRIENCE DE COMPOSANT", "inventory.material.monster_component.choice");
            screen.addLine("Composant : " + material.getName());
            screen.addBackOption("Annuler", "inventory.material.experiment.cancel");
            screen.addOption(1, "Potion de rage simple", "Essai agressif basique.", true, "inventory.material.craft.damage");
            screen.addOption(2, "Potion de soin renforcée", isAlchemist(player) ? "Préparation alchimique avancée." : "[◘ réservé à l'Alchimiste]", isAlchemist(player), "inventory.material.craft.heal_plus");
            screen.addOption(3, "Potion de rage supérieure", isAlchemist(player) ? "Préparation alchimique avancée." : "[◘ réservé à l'Alchimiste]", isAlchemist(player), "inventory.material.craft.damage_plus");
            bool canKit = isBlacksmith(player) || material.getId() == "arcane_dust";
            screen.addOption(4, "Améliorer / renforcer un kit", canKit ? "Ouvre le choix du kit à fabriquer." : "[◘ connaissances de forge insuffisantes]", canKit, "inventory.material.craft.kit_branch");

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une expérience affichée.");
            Console::clear();

            if (choice == 1) return runMaterialCraftAttempt(player, "Potion de rage simple", "inventory.material.damage_potion.result", craftDamagePotion);
            if (choice == 2) return runMaterialCraftAttempt(player, "Potion de soin renforcée", "inventory.material.reinforced_heal.result", craftReinforcedHealingPotion);
            if (choice == 3) return runMaterialCraftAttempt(player, "Potion de rage supérieure", "inventory.material.reinforced_damage.result", craftReinforcedDamagePotion);
            if (choice == 4)
            {
                MenuScreen kitScreen("RENFORCER UN KIT", "inventory.material.kit.choice");
                kitScreen.addBackOption("Annuler", "inventory.material.kit.cancel");
                kitScreen.addOption(1, "Kit moyen (+50%)", "Nécessite un kit faible intact et des composants de forge.", true, "inventory.material.kit.medium");
                kitScreen.addOption(2, "Gros kit (+75%)", "Nécessite un kit moyen intact et plus de matériaux.", true, "inventory.material.kit.big");
                kitScreen.addOption(3, "Kit complet du bricoleur (+95%)", "Nécessite un gros kit intact et beaucoup de composants.", true, "inventory.material.kit.complete");
                int kitChoice = TerminalInterface::askMenuChoiceFromOptions(kitScreen, "Choisis un kit à fabriquer.");
                Console::clear();
                if (kitChoice == 1) return runMaterialCraftAttempt(player, "Kit de réparation moyen", "inventory.material.kit.medium.result", craftMediumRepairKit);
                if (kitChoice == 2) return runMaterialCraftAttempt(player, "Gros kit de réparation", "inventory.material.kit.big.result", craftBigRepairKit);
                if (kitChoice == 3) return runMaterialCraftAttempt(player, "Kit complet du bricoleur", "inventory.material.kit.complete.result", craftTinkererCompleteRepairKit);
            }
            return false;
        }

        if (material.getId() == "kitsune_ember" || material.getId() == "unstable_core")
        {
            if (!isAlchemist(player))
            {
                MessageScreen::show(
                    "COMPOSANT INSTABLE",
                    "inventory.material.unstable.locked",
                    {
                        "Ce composant est trop instable sans vraie maîtrise d'Alchimiste.",
                        "Garde-le pour une recette avancée ou un artisan plus compétent."
                    }
                );
                return false;
            }

            MenuScreen screen("ALCHIMIE AVANCÉE", "inventory.material.advanced_alchemy.choice");
            screen.addLine("Composant : " + material.getName());
            screen.addBackOption("Annuler", "inventory.material.advanced_alchemy.cancel");
            screen.addOption(1, "Potion de soin renforcée à la braise kitsune", "Préparation chaude et fragile.", true, "inventory.material.kitsune_heal");
            screen.addOption(2, "Potion de rage supérieure au noyau instable", "Préparation dangereuse mais puissante.", true, "inventory.material.unstable_rage");
            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une expérience affichée.");
            Console::clear();
            if (choice == 1) return runMaterialCraftAttempt(player, "Potion kitsune renforcée", "inventory.material.kitsune_heal.result", craftKitsuneHealingPotion);
            if (choice == 2) return runMaterialCraftAttempt(player, "Potion de rage au noyau instable", "inventory.material.unstable_rage.result", craftUnstableRagePotion);
            return false;
        }

        if (material.getId() == "beast_hide")
        {
            MenuScreen screen("TRAVAIL DE PEAU ROBUSTE", "inventory.material.beast_hide.choice");
            screen.addBackOption("Annuler", "inventory.material.beast_hide.cancel");
            screen.addOption(1, "Kit moyen par rafistolage renforcé", "Transforme la peau en plaques de réparation.", true, "inventory.material.beast_hide.kit");
            screen.addOption(2, "Armure de chasseur rafistolée", "Fabrique une protection souple.", true, "inventory.material.beast_hide.armor");
            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une expérience affichée.");
            Console::clear();
            if (choice == 1) return runMaterialCraftAttempt(player, "Kit moyen renforcé", "inventory.material.beast_hide.kit.result", craftFieldArmorPatch);
            if (choice == 2) return runMaterialCraftAttempt(player, "Armure de chasseur rafistolée", "inventory.material.beast_hide.armor.result", craftHunterLeatherArmor);
            return false;
        }

        if (material.getId() == "rusted_metal_fragment" || material.getId() == "worn_leather_piece")
        {
            MenuScreen screen("ARTISANAT DE BASE", "inventory.material.basic_craft.choice");
            screen.addLine("Composant : " + material.getName());
            screen.addBackOption("Annuler", "inventory.material.basic_craft.cancel");
            screen.addOption(1, "Kit de réparation faible", "Outil basique pour réparer jusqu'à 25%.", true, "inventory.material.basic_craft.kit");
            screen.addOption(2, "Lame de récupération", "Arme bricolée mais fiable.", true, "inventory.material.basic_craft.weapon");
            screen.addOption(3, "Armure de chasseur rafistolée", "Protection simple de survie.", true, "inventory.material.basic_craft.armor");
            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une expérience affichée.");
            Console::clear();
            if (choice == 1) return runMaterialCraftAttempt(player, "Kit de réparation faible", "inventory.material.basic_craft.kit.result", craftRepairKit);
            if (choice == 2) return runMaterialCraftAttempt(player, "Lame de récupération", "inventory.material.basic_craft.weapon.result", craftRecoveryBlade);
            if (choice == 3) return runMaterialCraftAttempt(player, "Armure de chasseur rafistolée", "inventory.material.basic_craft.armor.result", craftHunterLeatherArmor);
            return false;
        }

        MessageScreen::show(
            "UTILISATION INCERTAINE",
            "inventory.material.use.unknown",
            {
                "Tu manipules " + material.getName() + ", mais tu ne trouves pas comment l'utiliser sans risque.",
                "[rien ne se passe]"
            }
        );
        return false;
    }

}
// EN: openWeapons declares or implements a focused behavior used by this module.
// FR: openWeapons déclare ou implémente un comportement précis utilisé par ce module.
bool InventorySelection::openWeapons(Player& player)
{
    if (player.getInventory().getWeaponCount() <= 0)
    {
        showInventoryNotice(
            "ARMES",
            "inventory.weapons.empty",
            {"Tu n'as aucune arme dans ton inventaire.", "Les armes obtenues en combat, boutique ou craft apparaîtront ici."}
        );
        return false;
    }

    constexpr std::size_t itemsPerPage = 10;
    std::size_t pageIndex = 0;

    while (true)
    {
        const std::size_t totalItems = static_cast<std::size_t>(player.getInventory().getWeaponCount());
        const std::size_t totalPages = PagedMenu::pageCount(totalItems, itemsPerPage);

        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, pageIndex, itemsPerPage);

        MenuScreen screen("ARMES", "inventory.weapons.page");
        screen.addSubtitle(PagedMenu::pageInfoText(pageIndex, totalPages, totalItems));
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalItems));

        for (std::size_t i = first; i < last; ++i)
        {
            Weapon weapon = player.getInventory().getWeapon(static_cast<int>(i));
            std::ostringstream label;
            label << weapon.getName()
                  << " | Durabilité : " << InventoryUtils::weaponDurabilityText(weapon);

            if (weapon.isBroken())
            {
                label << " | Cassée";
            }

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "weapon";
            itemData.section = "Armes";
            itemData.actionType = "select";
            itemData.name = weapon.getName();
            itemData.detail = "Dégâts : " + std::to_string(weapon.getMinDamageBonus()) + "-" + std::to_string(weapon.getMaxDamageBonus())
                + " | Critique : +" + std::to_string(weapon.getCriticalBonus());
            itemData.status = weapon.isBroken() ? "Cassée" : "Utilisable";
            itemData.progress = "Durabilité : " + InventoryUtils::weaponDurabilityText(weapon);
            itemData.price = std::to_string(weapon.getValue()) + " or";
            itemData.important = weapon.isBroken();

            screen.addOption(
                static_cast<int>(i - first + 1),
                label.str(),
                "",
                true,
                "inventory.weapon.select",
                itemData
            );
        }

        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);
        screen.addFooterLine("Sélectionne une arme affichée.");

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix invalide. Choisis une entrée visible, une page disponible ou 0 pour revenir."
        );
        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (choice < 1 || choice > visibleCount)
        {
            showInventoryNotice("CHOIX REFUSÉ", "inventory.weapons.invalid_choice", {"Ce numéro ne correspond à aucune arme affichée.", "Change de page ou choisis une entrée visible."});
            continue;
        }

        int index = static_cast<int>(first) + choice - 1;

        if (!player.getInventory().hasWeapon(index))
        {
            showInventoryNotice("ARME INTROUVABLE", "inventory.weapons.missing", {"Cette arme n'existe plus dans ton inventaire.", "Le registre se mettra à jour au prochain affichage."});
            continue;
        }

        Weapon weapon = player.getInventory().getWeapon(index);
        MenuScreen selectedWeaponScreen = InventoryDisplay::buildSelectedWeaponScreen(weapon);
        int action = TerminalInterface::askMenuChoiceFromOptions(
            selectedWeaponScreen,
            "Choix invalide. Choisis une action visible pour cette arme."
        );

        Console::clear();

        if (action == 1)
        {
            showWeaponInspectionScreen(weapon);
            continue;
        }

        if (action == 3)
        {
            BestiaryMenu::displayObjectEntry(weapon.getName());
            continue;
        }

        if (action == 4)
        {
            repairSelectedWeapon(player, index);
            continue;
        }

        if (action == 2)
        {
            bool equipped = player.equipWeapon(index);
            Weapon equippedWeapon = equipped ? player.getEquippedWeapon() : weapon;
            showEquipWeaponResultScreen(player, equippedWeapon, equipped);
            continue;
        }

        return false;
    }
}

// EN: openArmors declares or implements a focused behavior used by this module.
// FR: openArmors déclare ou implémente un comportement précis utilisé par ce module.
bool InventorySelection::openArmors(Player& player)
{
    if (player.getInventory().getArmorCount() <= 0)
    {
        showInventoryNotice(
            "ARMURES",
            "inventory.armors.empty",
            {"Tu n'as aucune armure dans ton inventaire.", "Les protections obtenues en combat, boutique ou craft apparaîtront ici."}
        );
        return false;
    }

    constexpr std::size_t itemsPerPage = 10;
    std::size_t pageIndex = 0;

    while (true)
    {
        const std::size_t totalItems = static_cast<std::size_t>(player.getInventory().getArmorCount());
        const std::size_t totalPages = PagedMenu::pageCount(totalItems, itemsPerPage);

        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, pageIndex, itemsPerPage);

        MenuScreen screen("ARMURES", "inventory.armors.page");
        screen.addSubtitle(PagedMenu::pageInfoText(pageIndex, totalPages, totalItems));
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalItems));

        for (std::size_t i = first; i < last; ++i)
        {
            Armor armor = player.getInventory().getArmor(static_cast<int>(i));
            std::ostringstream label;
            label << armor.getName()
                  << " | Durabilité : " << InventoryUtils::armorDurabilityText(armor);

            if (armor.isBroken())
            {
                label << " | Cassée";
            }

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "armor";
            itemData.section = "Armures";
            itemData.actionType = "select";
            itemData.name = armor.getName();
            itemData.detail = "PV : +" + std::to_string(armor.getMaxHpBonus()) + " | Réduction : " + std::to_string(armor.getDamageReduction());
            itemData.status = armor.isBroken() ? "Cassée" : "Utilisable";
            itemData.progress = "Durabilité : " + InventoryUtils::armorDurabilityText(armor);
            itemData.price = std::to_string(armor.getValue()) + " or";
            itemData.important = armor.isBroken();

            screen.addOption(
                static_cast<int>(i - first + 1),
                label.str(),
                "",
                true,
                "inventory.armor.select",
                itemData
            );
        }

        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);
        screen.addFooterLine("Sélectionne une armure affichée.");

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix invalide. Choisis une entrée visible, une page disponible ou 0 pour revenir."
        );
        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (choice < 1 || choice > visibleCount)
        {
            showInventoryNotice("CHOIX REFUSÉ", "inventory.armors.invalid_choice", {"Ce numéro ne correspond à aucune armure affichée.", "Change de page ou choisis une entrée visible."});
            continue;
        }

        int index = static_cast<int>(first) + choice - 1;

        if (!player.getInventory().hasArmor(index))
        {
            showInventoryNotice("ARMURE INTROUVABLE", "inventory.armors.missing", {"Cette armure n'existe plus dans ton inventaire.", "Le registre se mettra à jour au prochain affichage."});
            continue;
        }

        Armor armor = player.getInventory().getArmor(index);
        MenuScreen selectedArmorScreen = InventoryDisplay::buildSelectedArmorScreen(armor);
        int action = TerminalInterface::askMenuChoiceFromOptions(
            selectedArmorScreen,
            "Choix invalide. Choisis une action visible pour cette armure."
        );

        Console::clear();

        if (action == 1)
        {
            showArmorInspectionScreen(armor);
            continue;
        }

        if (action == 3)
        {
            BestiaryMenu::displayObjectEntry(armor.getName());
            continue;
        }

        if (action == 4)
        {
            repairSelectedArmor(player, index);
            continue;
        }

        if (action == 2)
        {
            bool equipped = player.equipArmor(index);
            Armor equippedArmor = equipped ? player.getEquippedArmor() : armor;
            showEquipArmorResultScreen(player, equippedArmor, equipped);
            continue;
        }

        return false;
    }
}

// EN: openConsumables declares or implements a focused behavior used by this module.
// FR: openConsumables déclare ou implémente un comportement précis utilisé par ce module.
bool InventorySelection::openConsumables(Player& player)
{
    bool hasClassicConsumables = player.getInventory().getConsumableCount() > 0;
    bool hasRepairKits = hasAnyRepairKit(player);

    if (!hasClassicConsumables && !hasRepairKits)
    {
        showInventoryNotice(
            "CONSOMMABLES",
            "inventory.consumables.empty",
            {"Tu n'as aucun consommable dans ton inventaire.", "Les potions, objets rapides et préparations apparaîtront ici."}
        );
        return false;
    }

    constexpr std::size_t itemsPerPage = 10;
    std::size_t pageIndex = 0;

    while (true)
    {
        std::vector<ConsumableGroup> groups = InventoryUtils::groupConsumables(player);
        hasRepairKits = hasAnyRepairKit(player);
        const std::size_t totalItems = groups.size();
        const std::size_t totalPages = PagedMenu::pageCount(totalItems, itemsPerPage);

        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, pageIndex, itemsPerPage);

        MenuScreen screen("CONSOMMABLES", "inventory.consumables.page");
        screen.addSubtitle(PagedMenu::pageInfoText(pageIndex, totalPages, totalItems));
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalItems));

        for (std::size_t i = first; i < last; ++i)
        {
            const ConsumableGroup& group = groups[i];
            std::ostringstream label;
            label << group.name
                  << " x" << group.amount
                  << " | " << InventoryUtils::consumableTypeToText(group.type)
                  << " | Puissance : " << group.power;

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "consumable";
            itemData.section = "Consommables";
            itemData.actionType = "select";
            itemData.name = group.name;
            itemData.quantity = std::to_string(group.amount);
            itemData.detail = InventoryUtils::consumableTypeToText(group.type);
            itemData.progress = "Puissance : " + std::to_string(group.power);
            itemData.important = group.amount <= 1;

            screen.addOption(
                static_cast<int>(i - first + 1),
                label.str(),
                "",
                true,
                "inventory.consumable.select",
                itemData
            );
        }

        if (hasRepairKits)
        {
            screen.addFooterLine("Kits de réparation : visibles avec les consommables, utilisables depuis l'équipement à réparer.");
        }

        if (groups.empty())
        {
            screen.addLine("Aucun consommable utilisable directement ici.");
            screen.addLine(hasRepairKits
                ? "Les kits se lancent depuis l'équipement à réparer."
                : "La section est maintenant vide.");
            screen.addBackOption();
            TerminalInterface::askMenuChoiceFromOptions(screen, "Entre 0 pour revenir.");
            Console::clear();
            return false;
        }

        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);
        screen.addFooterLine("Sélectionne un consommable affiché.");

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix invalide. Choisis une entrée visible, une page disponible ou 0 pour revenir."
        );
        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (choice < 1 || choice > visibleCount)
        {
            showInventoryNotice("CHOIX REFUSÉ", "inventory.consumables.invalid_choice", {"Ce numéro ne correspond à aucun consommable affiché.", "Change de page ou choisis une entrée visible."});
            continue;
        }

        int index = groups[first + static_cast<std::size_t>(choice - 1)].firstIndex;

        if (!player.getInventory().hasConsumable(index))
        {
            showInventoryNotice("CONSOMMABLE INTROUVABLE", "inventory.consumables.missing", {"Ce consommable n'existe plus dans ton inventaire.", "Le registre se mettra à jour au prochain affichage."});
            continue;
        }

        Consumable consumable = player.getInventory().getConsumable(index);

        MenuScreen selectedConsumableScreen = InventoryDisplay::buildSelectedConsumableScreen(consumable);
        int action = TerminalInterface::askMenuChoiceFromOptions(
            selectedConsumableScreen,
            "Choix invalide. Choisis une action visible pour ce consommable."
        );

        Console::clear();

        if (action == 1)
        {
            showConsumableInspectionScreen(consumable);
            continue;
        }

        if (action == 3)
        {
            BestiaryMenu::displayObjectEntry(consumable.getName());
            continue;
        }

        if (action == 2)
        {
            if (consumable.getType() != ConsumableType::Healing)
            {
                showInventoryNotice(
                    "UTILISATION BLOQUÉE",
                    "inventory.consumable.use.locked",
                    {"Ce consommable demande une cible ou un effet spécial.", "Utilise plutôt l'option Potions du menu de combat."}
                );
                continue;
            }

            int hpBefore = player.getHp();
            player.heal(consumable.getPower());
            ThreatSystem::markSelfHealingAction(player);

            if (!player.hasInfiniteConsumables())
            {
                player.getInventory().removeConsumable(index);
            }

            showHealingConsumableResultScreen(player, consumable, hpBefore);
            return true;
        }

        return false;
    }
}

// EN: openMaterials declares or implements a focused behavior used by this module.
// FR: openMaterials déclare ou implémente un comportement précis utilisé par ce module.
bool InventorySelection::openMaterials(Player& player)
{
    if (player.getInventory().getMaterials().empty())
    {
        showInventoryNotice(
            "MATÉRIAUX / PLANTES / INFOS",
            "inventory.materials.empty",
            {"Tu n'as aucun matériau, plante ou renseignement dans ton inventaire.", "Les loots, les boutiques et la bibliothèque pourront remplir cette partie."}
        );
        return false;
    }

    constexpr std::size_t itemsPerPage = 12;
    std::size_t pageIndex = 0;

    while (true)
    {
        const std::vector<Material>& materials = player.getInventory().getMaterials();
        const std::size_t totalItems = materials.size();
        const std::size_t totalPages = PagedMenu::pageCount(totalItems, itemsPerPage);

        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, pageIndex, itemsPerPage);

        MenuScreen screen("MATÉRIAUX / PLANTES / INFOS", "inventory.materials.page");
        screen.addSubtitle(PagedMenu::pageInfoText(pageIndex, totalPages, totalItems));
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalItems));

        for (std::size_t i = first; i < last; ++i)
        {
            Material material = player.getInventory().getMaterial(static_cast<int>(i));
            std::ostringstream label;
            label << material.getName() << " x" << material.getQuantity();

            if (material.hasSpecialQuality())
            {
                label << " | " << material.getQualityLabel();
            }

            label << " | " << material.getCategory();

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "material";
            itemData.section = "Matériaux";
            itemData.actionType = "select";
            itemData.name = material.getName();
            itemData.quantity = std::to_string(material.getQuantity());
            itemData.detail = material.getCategory();
            itemData.status = material.hasSpecialQuality() ? material.getQualityLabel() : "Qualité normale";
            itemData.price = std::to_string(material.getValue()) + " or/unité";
            itemData.important = material.hasSpecialQuality();

            screen.addOption(
                static_cast<int>(i - first + 1),
                label.str(),
                "",
                true,
                "inventory.material.select",
                itemData
            );
        }

        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);
        screen.addFooterLine("Sélectionne une entrée affichée.");

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix invalide. Choisis une entrée visible, une page disponible ou 0 pour revenir."
        );
        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (choice < 1 || choice > visibleCount)
        {
            showInventoryNotice("CHOIX REFUSÉ", "inventory.materials.invalid_choice", {"Ce numéro ne correspond à aucune entrée affichée.", "Change de page ou choisis une entrée visible."});
            continue;
        }

        int index = static_cast<int>(first) + choice - 1;

        if (!player.getInventory().hasMaterial(index))
        {
            showInventoryNotice("ENTRÉE INTROUVABLE", "inventory.materials.missing", {"Cette entrée n'existe plus dans ton inventaire.", "Le registre se mettra à jour au prochain affichage."});
            continue;
        }

        Material material = player.getInventory().getMaterial(index);
        MenuScreen selectedMaterialScreen = InventoryDisplay::buildSelectedMaterialScreen(material);
        int action = TerminalInterface::askMenuChoiceFromOptions(
            selectedMaterialScreen,
            "Choix invalide. Choisis une action visible pour cette entrée."
        );

        Console::clear();

        if (action == 1)
        {
            showMaterialInspectionScreen(material);
            continue;
        }

        if (action == 2)
        {
            BestiaryMenu::displayObjectEntry(material.getName());
            continue;
        }

        if (action == 3)
        {
            displayMaterialUtility(material);
            continue;
        }

        if (action == 4)
        {
            useMaterialIfPossible(player, material);
            continue;
        }

        return false;
    }
}
// EN: openCraft declares or implements a focused behavior used by this module.
// FR: openCraft déclare ou implémente un comportement précis utilisé par ce module.
bool InventorySelection::openCraft(Player& player)
{
    std::size_t pageIndex = 0;
    constexpr std::size_t recipesPerPage = 8;

    while (true)
    {
        std::vector<CraftRecipe> recipes = buildCraftRecipes();
        const std::size_t totalItems = recipes.size();
        const std::size_t totalPages = PagedMenu::pageCount(totalItems, recipesPerPage);

        if (recipes.empty())
        {
            MenuScreen emptyScreen("CRAFT / SCHÉMAS", "inventory.craft.empty");
            emptyScreen.addLine("Aucun schéma de craft n'est disponible.");
            TerminalInterface::renderMenuScreen(emptyScreen, false);
            Console::waitForEnter();
            Console::clear();
            return false;
        }

        if (pageIndex >= totalPages)
        {
            pageIndex = totalPages - 1;
        }

        const std::size_t first = PagedMenu::firstIndex(pageIndex, recipesPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, pageIndex, recipesPerPage);

        MenuScreen screen("CRAFT / SCHÉMAS", "inventory.craft.page");
        screen.addSubtitle(PagedMenu::pageInfoText(pageIndex, totalPages, totalItems));
        screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalItems));

        for (std::size_t i = first; i < last; ++i)
        {
            const CraftRecipe& recipe = recipes[i];
            int maxCrafts = maxCraftsForRecipe(player, recipe);

            std::ostringstream label;
            label << recipe.name << " | " << recipe.category << " | max : " << maxCrafts;

            std::string hint;
            if (recipe.alchemistOnly && !isAlchemist(player))
            {
                hint = "Réservé à l'Alchimiste, mais consultable pour préparer les composants.";
            }
            else if (recipe.blacksmithHint && isBlacksmith(player))
            {
                hint = "Maîtrise Forgeron : tu comprends mieux cette fabrication.";
            }
            else if (maxCrafts <= 0)
            {
                hint = "Composants insuffisants pour l'instant.";
            }

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "material";
            itemData.section = "Craft";
            itemData.actionType = "select";
            itemData.name = recipe.name;
            itemData.detail = recipe.category;
            itemData.status = hint;
            itemData.maxQuantity = std::to_string(maxCrafts);
            itemData.progress = maxCrafts > 0 ? "Fabricable" : "Composants insuffisants";
            itemData.important = maxCrafts <= 0;

            screen.addOption(
                static_cast<int>(i - first + 1),
                label.str(),
                hint,
                true,
                "inventory.craft.recipe.select",
                itemData
            );
        }

        PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);
        screen.addFooterLine("Les max utilisent l'équivalence de qualité : faible/impur compte moins, pur/haute qualité/exceptionnel compte plus.");

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Sélectionne un schéma affiché, une page, ou 0 pour revenir."
        );
        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 98 && pageIndex > 0)
        {
            --pageIndex;
            continue;
        }

        if (choice == 99 && pageIndex + 1 < totalPages)
        {
            ++pageIndex;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (choice < 1 || choice > visibleCount)
        {
            MessageScreen::show(
                "SCHÉMA INTROUVABLE",
                "inventory.craft.invalid_choice",
                {"Ce numéro ne correspond à aucun schéma sur cette page.", "Change de page ou choisis une entrée visible."}
            );
            continue;
        }

        CraftRecipe recipe = recipes[first + static_cast<std::size_t>(choice - 1)];
        int maxCrafts = maxCraftsForRecipe(player, recipe);

        MenuScreen detailScreen("SCHÉMA DE CRAFT", "inventory.craft.detail");
        detailScreen.setDisplayOnlyInput("Détail temporaire avant la quantité ou la confirmation de craft.");
        detailScreen.addLine("Schéma : " + recipe.name);
        detailScreen.addLine("Catégorie : " + recipe.category);
        detailScreen.addLine("Nombre maximum possible : " + std::to_string(maxCrafts));

        if (recipe.alchemistOnly && !isAlchemist(player))
        {
            detailScreen.addLine("Condition : réservé à l'Alchimiste.");
        }

        if (recipe.blacksmithHint && isBlacksmith(player))
        {
            detailScreen.addLine("Bonus métier : le Forgeron comprend mieux cette fabrication.");
        }

        detailScreen.addLine("Matériaux nécessaires pour 1 craft :");
        for (const std::string& line : buildRecipeIngredientLines(player, recipe))
        {
            detailScreen.addLine(line);
        }

        TerminalInterface::renderMenuScreen(detailScreen, false);

        if (maxCrafts <= 0)
        {
            MessageScreen::show(
                "CRAFT IMPOSSIBLE",
                "inventory.craft.blocked",
                {
                    "Schéma : " + recipe.name,
                    "Tu ne peux pas fabriquer ce schéma pour le moment.",
                    "Les lignes de composants indiquent ce qui manque."
                }
            );
            continue;
        }

        int quantityToCraft = 1;

        if (maxCrafts > 1)
        {
            MenuScreen quantityScreen("QUANTITÉ DE CRAFT", "inventory.craft.quantity");
            quantityScreen.addLine("Schéma : " + recipe.name);
            quantityScreen.addLine("Maximum possible : " + std::to_string(maxCrafts));
            quantityScreen.addBackOption("Annuler");
            quantityScreen.addOption(1, "Choisir une quantité", "Entre ensuite un nombre entre 1 et " + std::to_string(maxCrafts) + ".", true, "inventory.craft.quantity.custom");

            int quantityChoice = TerminalInterface::askMenuChoiceFromOptions(quantityScreen, "Choisis une option affichée.");
            Console::clear();

            if (quantityChoice == 0)
            {
                continue;
            }

            quantityToCraft = MessageScreen::askQuantity(
                "QUANTITÉ À FABRIQUER",
                "inventory.craft.quantity.input",
                {
                    "Schéma : " + recipe.name,
                    "Maximum possible : " + std::to_string(maxCrafts),
                    "0 : Annuler"
                },
                0,
                maxCrafts,
                "Quantité invalide."
            );

            if (quantityToCraft == 0)
            {
                Console::clear();
                continue;
            }
        }

        MenuScreen confirmScreen("CONFIRMATION CRAFT", "inventory.craft.confirm");
        confirmScreen.addLine("Schéma : " + recipe.name);
        confirmScreen.addLine("Quantité : " + std::to_string(quantityToCraft));
        confirmScreen.addBackOption("Annuler");
        confirmScreen.addOption(1, "Confirmer la fabrication", "Les composants seront consommés.", true, "inventory.craft.confirm.accept");

        int confirm = TerminalInterface::askMenuChoiceFromOptions(confirmScreen, "Choisis une option affichée.");
        Console::clear();

        if (confirm == 0)
        {
            continue;
        }

        int crafted = 0;

        for (int i = 0; i < quantityToCraft; ++i)
        {
            bool success = false;
            {
                success = recipe.craft(player);
            }

            if (!success)
            {
                break;
            }

            ++crafted;
        }

        if (crafted > 0)
        {
            MaterialExperimentLog::recordCraft(recipe.name, crafted);
        }

        showCraftResultScreen(recipe.name, crafted, quantityToCraft);
    }
}

