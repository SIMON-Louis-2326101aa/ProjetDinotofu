// EN: Inventory.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Inventory.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/Inventory.hpp"

#include "progression/material/MaterialKnowledgeProgress.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <algorithm>
#include <string>
#include <vector>


namespace
{
    // EN: startsWith declares or implements a focused behavior used by this module.
    // FR: startsWith déclare ou implémente un comportement précis utilisé par ce module.
    bool startsWith(const std::string& value, const std::string& prefix)
    {
        return value.rfind(prefix, 0) == 0;
    }

    // EN: isRepairKitInventoryId declares or implements a focused behavior used by this module.
    // FR: isRepairKitInventoryId déclare ou implémente un comportement précis utilisé par ce module.
    bool isRepairKitInventoryId(const std::string& id)
    {
        return id == "weak_repair_kit"
            || id == "medium_repair_kit"
            || id == "big_repair_kit"
            || id == "tinkerer_complete_repair_kit"
            || startsWith(id, "weak_repair_kit_used_")
            || startsWith(id, "medium_repair_kit_used_")
            || startsWith(id, "big_repair_kit_used_")
            || startsWith(id, "tinkerer_complete_repair_kit_used_");
    }

    // EN: isUsedRepairKitInventoryId declares or implements a focused behavior used by this module.
    // FR: isUsedRepairKitInventoryId déclare ou implémente un comportement précis utilisé par ce module.
    bool isUsedRepairKitInventoryId(const std::string& id)
    {
        return startsWith(id, "weak_repair_kit_used_")
            || startsWith(id, "medium_repair_kit_used_")
            || startsWith(id, "big_repair_kit_used_")
            || startsWith(id, "tinkerer_complete_repair_kit_used_");
    }

    // EN: maxRepairKitInventoryDurability declares or implements a focused behavior used by this module.
    // FR: maxRepairKitInventoryDurability déclare ou implémente un comportement précis utilisé par ce module.
    int maxRepairKitInventoryDurability(const std::string& id)
    {
        if (startsWith(id, "medium_repair_kit")) return 3;
        if (startsWith(id, "big_repair_kit")) return 4;
        if (startsWith(id, "tinkerer_complete_repair_kit")) return 5;
        return 2;
    }

    // EN: usedRepairKitInventoryDurability declares or implements a focused behavior used by this module.
    // FR: usedRepairKitInventoryDurability déclare ou implémente un comportement précis utilisé par ce module.
    int usedRepairKitInventoryDurability(const std::string& id)
    {
        std::size_t pos = id.find_last_of('_');

        if (pos == std::string::npos || pos + 1 >= id.size())
        {
            return 1;
        }

        try
        {
            return std::stoi(id.substr(pos + 1));
        }
        catch (...)
        {
            return 1;
        }
    }

    struct ConsumableInventoryStack
    {
        int firstIndex;
        int amount;
        std::string name;
        ConsumableType type;
        int power;
    };

    std::string formatConsumableInventoryStackLabel(const std::string& name, int amount)
    {
        if (amount < 1)
        {
            amount = 1;
        }

        return name + " (*" + std::to_string(amount) + ")";
    }

    std::vector<ConsumableInventoryStack> groupConsumablesForInventory(const std::vector<Consumable>& consumables)
    {
        std::vector<ConsumableInventoryStack> groups;

        for (int i = 0; i < static_cast<int>(consumables.size()); ++i)
        {
            const Consumable& consumable = consumables[i];
            bool found = false;

            for (ConsumableInventoryStack& group : groups)
            {
                if (group.name == consumable.getName()
                    && group.type == consumable.getType()
                    && group.power == consumable.getPower())
                {
                    group.amount++;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                ConsumableInventoryStack group;
                group.firstIndex = i;
                group.amount = 1;
                group.name = consumable.getName();
                group.type = consumable.getType();
                group.power = consumable.getPower();
                groups.push_back(group);
            }
        }

        return groups;
    }

    void showInventoryScreen(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines,
        bool waitAndClear = false
    )
    {
        if (!lines.empty())
        {
            MessageScreen::show(title, screenId, lines, waitAndClear);
        }
    }

    std::string formatRepairKitInventoryStatus(const Material& material)
    {
        if (isUsedRepairKitInventoryId(material.getId()))
        {
            return " x" + std::to_string(material.getQuantity())
                + " | dura "
                + std::to_string(usedRepairKitInventoryDurability(material.getId()))
                + "/"
                + std::to_string(maxRepairKitInventoryDurability(material.getId()));
        }

        return " x" + std::to_string(material.getQuantity())
            + " | intact "
            + std::to_string(maxRepairKitInventoryDurability(material.getId()))
            + "/"
            + std::to_string(maxRepairKitInventoryDurability(material.getId()));
    }

}

// EN: Inventory declares or implements a focused behavior used by this module.
// FR: Inventory déclare ou implémente un comportement précis utilisé par ce module.
Inventory::Inventory()
{
    or_ = 0;
}

// EN: getGold declares or implements a focused behavior used by this module.
// FR: getGold déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::getGold() const
{
    return or_;
}

// EN: setGold declares or implements a focused behavior used by this module.
// FR: setGold déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::setGold(int amount)
{
    if (amount < 0)
    {
        amount = 0;
    }

    or_ = amount;
}

// EN: earnGold declares or implements a focused behavior used by this module.
// FR: earnGold déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::earnGold(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    or_ += amount;
}

// EN: spendGold declares or implements a focused behavior used by this module.
// FR: spendGold déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::spendGold(int amount)
{
    if (amount <= 0)
    {
        return true;
    }

    if (or_ < amount)
    {
        return false;
    }

    or_ -= amount;
    return true;
}

// EN: getWeaponCount declares or implements a focused behavior used by this module.
// FR: getWeaponCount déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::getWeaponCount() const
{
    return static_cast<int>(weapons.size());
}

// EN: getArmorCount declares or implements a focused behavior used by this module.
// FR: getArmorCount déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::getArmorCount() const
{
    return static_cast<int>(armors.size());
}

// EN: getConsumableCount declares or implements a focused behavior used by this module.
// FR: getConsumableCount déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::getConsumableCount() const
{
    return static_cast<int>(consumables.size());
}

// EN: getMaterialCount declares or implements a focused behavior used by this module.
// FR: getMaterialCount déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::getMaterialCount() const
{
    int total = 0;

    for (const Material& material : materials)
    {
        total += material.getQuantity();
    }

    return total;
}

// EN: countConsumables declares or implements a focused behavior used by this module.
// FR: countConsumables déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::countConsumables(ConsumableType type) const
{
    int total = 0;

    for (const Consumable& consumable : consumables)
    {
        if (consumable.getType() == type)
        {
            total++;
        }
    }

    return total;
}

// EN: findMaterialIndexById declares or implements a focused behavior used by this module.
// FR: findMaterialIndexById déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::findMaterialIndexById(const std::string& id) const
{
    for (int i = 0; i < static_cast<int>(materials.size()); ++i)
    {
        if (materials[i].getId() == id)
        {
            return i;
        }
    }

    return -1;
}

// EN: countMaterialById declares or implements a focused behavior used by this module.
// FR: countMaterialById déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::countMaterialById(const std::string& id) const
{
    int total = 0;

    for (const Material& material : materials)
    {
        if (material.getId() == id)
        {
            total += material.getQuantity();
        }
    }

    return total;
}

// EN: countMaterialQualityPointsById declares or implements a focused behavior used by this module.
// FR: countMaterialQualityPointsById déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::countMaterialQualityPointsById(const std::string& id) const
{
    int total = 0;

    for (const Material& material : materials)
    {
        if (material.getId() == id)
        {
            total += material.getQuantity() * material.getQualityCraftWeight();
        }
    }

    return total;
}

// EN: getWeapons declares or implements a focused behavior used by this module.
// FR: getWeapons déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<Weapon>& Inventory::getWeapons() const
{
    return weapons;
}

// EN: getArmors declares or implements a focused behavior used by this module.
// FR: getArmors déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<Armor>& Inventory::getArmors() const
{
    return armors;
}

// EN: getConsumables declares or implements a focused behavior used by this module.
// FR: getConsumables déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<Consumable>& Inventory::getConsumables() const
{
    return consumables;
}

// EN: getMaterials declares or implements a focused behavior used by this module.
// FR: getMaterials déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<Material>& Inventory::getMaterials() const
{
    return materials;
}

// EN: addWeapon declares or implements a focused behavior used by this module.
// FR: addWeapon déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::addWeapon(const Weapon& weapon)
{
    weapons.push_back(weapon);
}

// EN: addArmor declares or implements a focused behavior used by this module.
// FR: addArmor déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::addArmor(const Armor& armor)
{
    armors.push_back(armor);
}

// EN: addConsumable declares or implements a focused behavior used by this module.
// FR: addConsumable déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::addConsumable(const Consumable& consumable)
{
    consumables.push_back(consumable);
}

// EN: addMaterial declares or implements a focused behavior used by this module.
// FR: addMaterial déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::addMaterial(const Material& material)
{
    if (material.getQuantity() <= 0)
    {
        return;
    }

    MaterialKnowledgeProgress::recordDiscovery(material);

    for (Material& existingMaterial : materials)
    {
        if (existingMaterial.getId() == material.getId()
            && existingMaterial.getQuality() == material.getQuality())
        {
            existingMaterial.addQuantity(material.getQuantity());
            return;
        }
    }

    materials.push_back(material);
}

// EN: hasWeapon declares or implements a focused behavior used by this module.
// FR: hasWeapon déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::hasWeapon(int index) const
{
    return index >= 0 && index < static_cast<int>(weapons.size());
}

// EN: hasArmor declares or implements a focused behavior used by this module.
// FR: hasArmor déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::hasArmor(int index) const
{
    return index >= 0 && index < static_cast<int>(armors.size());
}

// EN: hasConsumable declares or implements a focused behavior used by this module.
// FR: hasConsumable déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::hasConsumable(int index) const
{
    return index >= 0 && index < static_cast<int>(consumables.size());
}

// EN: hasMaterial declares or implements a focused behavior used by this module.
// FR: hasMaterial déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::hasMaterial(int index) const
{
    return index >= 0 && index < static_cast<int>(materials.size());
}

// EN: getWeapon declares or implements a focused behavior used by this module.
// FR: getWeapon déclare ou implémente un comportement précis utilisé par ce module.
Weapon Inventory::getWeapon(int index) const
{
    if (!hasWeapon(index))
    {
        return Weapon();
    }

    return weapons[index];
}

// EN: getMutableWeapon declares or implements a focused behavior used by this module.
// FR: getMutableWeapon déclare ou implémente un comportement précis utilisé par ce module.
Weapon* Inventory::getMutableWeapon(int index)
{
    if (!hasWeapon(index))
    {
        return nullptr;
    }

    return &weapons[index];
}

// EN: getArmor declares or implements a focused behavior used by this module.
// FR: getArmor déclare ou implémente un comportement précis utilisé par ce module.
Armor Inventory::getArmor(int index) const
{
    if (!hasArmor(index))
    {
        return Armor();
    }

    return armors[index];
}

// EN: getMutableArmor declares or implements a focused behavior used by this module.
// FR: getMutableArmor déclare ou implémente un comportement précis utilisé par ce module.
Armor* Inventory::getMutableArmor(int index)
{
    if (!hasArmor(index))
    {
        return nullptr;
    }

    return &armors[index];
}

// EN: getConsumable declares or implements a focused behavior used by this module.
// FR: getConsumable déclare ou implémente un comportement précis utilisé par ce module.
Consumable Inventory::getConsumable(int index) const
{
    if (!hasConsumable(index))
    {
        return Consumable();
    }

    return consumables[index];
}

// EN: getMaterial declares or implements a focused behavior used by this module.
// FR: getMaterial déclare ou implémente un comportement précis utilisé par ce module.
Material Inventory::getMaterial(int index) const
{
    if (!hasMaterial(index))
    {
        return Material();
    }

    return materials[index];
}

// EN: getMutableMaterial declares or implements a focused behavior used by this module.
// FR: getMutableMaterial déclare ou implémente un comportement précis utilisé par ce module.
Material* Inventory::getMutableMaterial(int index)
{
    if (!hasMaterial(index))
    {
        return nullptr;
    }

    return &materials[index];
}

// EN: findFirstConsumable declares or implements a focused behavior used by this module.
// FR: findFirstConsumable déclare ou implémente un comportement précis utilisé par ce module.
int Inventory::findFirstConsumable(ConsumableType type) const
{
    for (int i = 0; i < static_cast<int>(consumables.size()); i++)
    {
        if (consumables[i].getType() == type)
        {
            return i;
        }
    }

    return -1;
}

// EN: useFirstConsumable declares or implements a focused behavior used by this module.
// FR: useFirstConsumable déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::useFirstConsumable(ConsumableType type, Consumable& usedConsumable)
{
    int index = findFirstConsumable(type);

    if (index == -1)
    {
        return false;
    }

    usedConsumable = consumables[index];
    consumables.erase(consumables.begin() + index);

    return true;
}

// EN: removeWeapon declares or implements a focused behavior used by this module.
// FR: removeWeapon déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::removeWeapon(int index)
{
    if (!hasWeapon(index))
    {
        return false;
    }

    weapons.erase(weapons.begin() + index);
    return true;
}

// EN: removeArmor declares or implements a focused behavior used by this module.
// FR: removeArmor déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::removeArmor(int index)
{
    if (!hasArmor(index))
    {
        return false;
    }

    armors.erase(armors.begin() + index);
    return true;
}

// EN: removeConsumable declares or implements a focused behavior used by this module.
// FR: removeConsumable déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::removeConsumable(int index)
{
    if (!hasConsumable(index))
    {
        return false;
    }

    consumables.erase(consumables.begin() + index);
    return true;
}

// EN: removeMaterialQuantity declares or implements a focused behavior used by this module.
// FR: removeMaterialQuantity déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::removeMaterialQuantity(int index, int quantity)
{
    if (!hasMaterial(index) || quantity <= 0)
    {
        return false;
    }

    if (!materials[index].removeQuantity(quantity))
    {
        return false;
    }

    if (materials[index].getQuantity() <= 0)
    {
        materials.erase(materials.begin() + index);
    }

    return true;
}

// EN: removeMaterialQuantityById declares or implements a focused behavior used by this module.
// FR: removeMaterialQuantityById déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::removeMaterialQuantityById(const std::string& id, int quantity)
{
    if (quantity <= 0)
    {
        return true;
    }

    if (countMaterialById(id) < quantity)
    {
        return false;
    }

    int remaining = quantity;

    for (int pass = 0; pass < 4 && remaining > 0; ++pass)
    {
        for (int i = 0; i < static_cast<int>(materials.size()) && remaining > 0; ++i)
        {
            if (materials[i].getId() != id)
            {
                continue;
            }

            bool passMatches = (pass == 0 && materials[i].getQualityCraftWeight() == 1)
                || (pass == 1 && materials[i].getQualityCraftWeight() == 2)
                || (pass == 2 && materials[i].getQualityCraftWeight() == 3)
                || (pass == 3 && materials[i].getQualityCraftWeight() == 4);

            if (!passMatches)
            {
                continue;
            }

            int removed = std::min(materials[i].getQuantity(), remaining);
            materials[i].removeQuantity(removed);
            remaining -= removed;

            if (materials[i].getQuantity() <= 0)
            {
                materials.erase(materials.begin() + i);
                --i;
            }
        }
    }

    return remaining <= 0;
}

// EN: removeMaterialQuantityByIdFlexible declares or implements a focused behavior used by this module.
// FR: removeMaterialQuantityByIdFlexible déclare ou implémente un comportement précis utilisé par ce module.
bool Inventory::removeMaterialQuantityByIdFlexible(const std::string& id, int normalQualityQuantity)
{
    if (normalQualityQuantity <= 0)
    {
        return true;
    }

    int requiredPoints = normalQualityQuantity * 2;

    if (countMaterialQualityPointsById(id) < requiredPoints)
    {
        return false;
    }

    for (int pass = 0; pass < 4 && requiredPoints > 0; ++pass)
    {
        for (int i = 0; i < static_cast<int>(materials.size()) && requiredPoints > 0; ++i)
        {
            if (materials[i].getId() != id)
            {
                continue;
            }

            bool passMatches = (pass == 0 && materials[i].getQualityCraftWeight() == 1)
                || (pass == 1 && materials[i].getQualityCraftWeight() == 2)
                || (pass == 2 && materials[i].getQualityCraftWeight() == 3)
                || (pass == 3 && materials[i].getQualityCraftWeight() == 4);

            if (!passMatches)
            {
                continue;
            }

            int weight = materials[i].getQualityCraftWeight();
            int toRemove = std::min(materials[i].getQuantity(), (requiredPoints + weight - 1) / weight);
            materials[i].removeQuantity(toRemove);
            requiredPoints -= toRemove * weight;

            if (materials[i].getQuantity() <= 0)
            {
                materials.erase(materials.begin() + i);
                --i;
            }
        }
    }

    return true;
}

// EN: clearAll declares or implements a focused behavior used by this module.
// FR: clearAll déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::clearAll()
{
    weapons.clear();
    armors.clear();
    consumables.clear();
    materials.clear();
    or_ = 0;
}

// EN: displayWeaponList declares or implements a focused behavior used by this module.
// FR: displayWeaponList déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayWeaponList() const
{
    std::vector<std::string> lines;

    if (weapons.empty())
    {
        lines.push_back("Aucune arme dans l'inventaire.");
        showInventoryScreen("ARMES", "inventory.weapons", lines, false);
        return;
    }

    for (int i = 0; i < static_cast<int>(weapons.size()); i++)
    {
        std::string line = "[" + std::to_string(i) + "] " + weapons[i].getName();

        if (!weapons[i].isIndestructible())
        {
            line += " (" + std::to_string(weapons[i].getDurability()) + "/" + std::to_string(weapons[i].getMaxDurability()) + ")";
        }

        if (weapons[i].isBroken())
        {
            line += " - Cassée";
        }

        lines.push_back(line);
    }

    showInventoryScreen("ARMES", "inventory.weapons", lines, false);
}


// EN: displayArmorList declares or implements a focused behavior used by this module.
// FR: displayArmorList déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayArmorList() const
{
    std::vector<std::string> lines;

    if (armors.empty())
    {
        lines.push_back("Aucune armure dans l'inventaire.");
        showInventoryScreen("ARMURES", "inventory.armors", lines, false);
        return;
    }

    for (int i = 0; i < static_cast<int>(armors.size()); i++)
    {
        std::string line = "[" + std::to_string(i) + "] " + armors[i].getName();

        if (!armors[i].isIndestructible())
        {
            line += " (" + std::to_string(armors[i].getDurability()) + "/" + std::to_string(armors[i].getMaxDurability()) + ")";
        }

        if (armors[i].isBroken())
        {
            line += " - Cassée";
        }

        lines.push_back(line);
    }

    showInventoryScreen("ARMURES", "inventory.armors", lines, false);
}


// EN: displayConsumableList declares or implements a focused behavior used by this module.
// FR: displayConsumableList déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayConsumableList() const
{
    std::vector<std::string> lines;
    bool hasRepairKit = false;

    for (const Material& material : materials)
    {
        if (material.getCategory() == "Outil" && isRepairKitInventoryId(material.getId()))
        {
            hasRepairKit = true;
            break;
        }
    }

    if (consumables.empty() && !hasRepairKit)
    {
        lines.push_back("Aucun consommable dans l'inventaire.");
        showInventoryScreen("CONSOMMABLES", "inventory.consumables", lines, false);
        return;
    }

    std::vector<ConsumableInventoryStack> consumableGroups = groupConsumablesForInventory(consumables);

    for (int i = 0; i < static_cast<int>(consumableGroups.size()); i++)
    {
        const ConsumableInventoryStack& group = consumableGroups[i];
        lines.push_back(
            "[" + std::to_string(group.firstIndex) + "] "
            + formatConsumableInventoryStackLabel(group.name, group.amount)
            + " | Puissance : " + std::to_string(group.power)
        );
    }

    if (hasRepairKit)
    {
        if (!lines.empty())
        {
            lines.push_back("");
        }

        lines.push_back("Kits de réparation :");

        for (const Material& material : materials)
        {
            if (material.getCategory() != "Outil" || !isRepairKitInventoryId(material.getId()))
            {
                continue;
            }

            lines.push_back("- " + material.getName() + formatRepairKitInventoryStatus(material));
        }

        lines.push_back("Astuce : sélectionne une arme ou une armure pour lancer une réparation.");
    }

    showInventoryScreen("CONSOMMABLES", "inventory.consumables", lines, false);
}


// EN: displayMaterialList declares or implements a focused behavior used by this module.
// FR: displayMaterialList déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayMaterialList() const
{
    std::vector<std::string> lines;

    if (materials.empty())
    {
        lines.push_back("Aucun matériau, plante ou renseignement stocké.");
        showInventoryScreen("MATÉRIAUX / PLANTES / INFOS", "inventory.materials", lines, false);
        return;
    }

    for (int i = 0; i < static_cast<int>(materials.size()); i++)
    {
        std::string line = "[" + std::to_string(i) + "] " + materials[i].getName();

        if (materials[i].getCategory() == "Outil" && isRepairKitInventoryId(materials[i].getId()))
        {
            line += formatRepairKitInventoryStatus(materials[i]);
        }
        else
        {
            line += " x" + std::to_string(materials[i].getQuantity());
            if (materials[i].hasSpecialQuality())
            {
                line += " | " + materials[i].getQualityLabel();
            }
        }

        line += " | " + materials[i].getCategory();
        lines.push_back(line);
    }

    showInventoryScreen("MATÉRIAUX / PLANTES / INFOS", "inventory.materials", lines, false);
}


// EN: displaySummary declares or implements a focused behavior used by this module.
// FR: displaySummary déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displaySummary() const
{
    showInventoryScreen(
        "INVENTAIRE",
        "inventory.summary",
        {
            "Or : " + std::to_string(or_) + " pièces",
            "Armes : " + std::to_string(getWeaponCount()),
            "Armures : " + std::to_string(getArmorCount()),
            "Consommables : " + std::to_string(getConsumableCount()),
            "Matériaux / plantes / infos : " + std::to_string(getMaterialCount()),
            "Potions de soin : " + std::to_string(countConsumables(ConsumableType::Healing)),
            "Potions de rage : " + std::to_string(countConsumables(ConsumableType::Damage))
        },
        false
    );
}


// EN: inspectWeapon declares or implements a focused behavior used by this module.
// FR: inspectWeapon déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::inspectWeapon(int index) const
{
    if (!hasWeapon(index))
    {
        MessageScreen::show("ARME INTROUVABLE", "inventory.weapon.invalid", {"Cette arme n'existe pas."}, false);
        return;
    }

    const Weapon& weapon = weapons[index];
    std::vector<std::string> lines;
    lines.push_back("Nom : " + weapon.getName());
    lines.push_back("Description : " + weapon.getDescription());
    lines.push_back(
        "Bonus dégâts : +"
        + std::to_string(weapon.getMinDamageBonus())
        + " à +"
        + std::to_string(weapon.getMaxDamageBonus())
    );
    lines.push_back("Bonus critique : +" + std::to_string(weapon.getCriticalBonus()));

    if (weapon.isIndestructible())
    {
        lines.push_back("Durabilité : Indestructible");
    }
    else
    {
        lines.push_back("Durabilité : " + std::to_string(weapon.getDurability()) + "/" + std::to_string(weapon.getMaxDurability()));
    }

    lines.push_back(std::string("État : ") + (weapon.isBroken() ? "Cassée" : "Utilisable"));
    lines.push_back("Valeur : " + std::to_string(weapon.getValue()) + " pièces");
    showInventoryScreen("ARME", "inventory.weapon.inspect", lines, false);
}


// EN: inspectArmor declares or implements a focused behavior used by this module.
// FR: inspectArmor déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::inspectArmor(int index) const
{
    if (!hasArmor(index))
    {
        MessageScreen::show("ARMURE INTROUVABLE", "inventory.armor.invalid", {"Cette armure n'existe pas."}, false);
        return;
    }

    const Armor& armor = armors[index];
    std::vector<std::string> lines;
    lines.push_back("Nom : " + armor.getName());
    lines.push_back("Description : " + armor.getDescription());
    lines.push_back("Bonus PV max : +" + std::to_string(armor.getMaxHpBonus()));
    lines.push_back("Réduction dégâts : " + std::to_string(armor.getDamageReduction()));

    if (armor.isIndestructible())
    {
        lines.push_back("Durabilité : Indestructible");
    }
    else
    {
        lines.push_back("Durabilité : " + std::to_string(armor.getDurability()) + "/" + std::to_string(armor.getMaxDurability()));
    }

    lines.push_back(std::string("État : ") + (armor.isBroken() ? "Cassée" : "Utilisable"));
    lines.push_back("Valeur : " + std::to_string(armor.getValue()) + " pièces");
    showInventoryScreen("ARMURE", "inventory.armor.inspect", lines, false);
}


// EN: inspectConsumable declares or implements a focused behavior used by this module.
// FR: inspectConsumable déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::inspectConsumable(int index) const
{
    if (!hasConsumable(index))
    {
        MessageScreen::show("CONSOMMABLE INTROUVABLE", "inventory.consumable.invalid", {"Ce consommable n'existe pas."}, false);
        return;
    }

    const Consumable& consumable = consumables[index];
    showInventoryScreen(
        "CONSOMMABLE",
        "inventory.consumable.inspect",
        {
            "Nom : " + consumable.getName(),
            "Description : " + consumable.getDescription(),
            "Puissance : " + std::to_string(consumable.getPower()),
            "Valeur : " + std::to_string(consumable.getValue()) + " pièces"
        },
        false
    );
}


// EN: inspectMaterial declares or implements a focused behavior used by this module.
// FR: inspectMaterial déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::inspectMaterial(int index) const
{
    if (!hasMaterial(index))
    {
        MessageScreen::show("ENTRÉE INTROUVABLE", "inventory.material.invalid", {"Cette entrée n'existe pas."}, false);
        return;
    }

    const Material& material = materials[index];
    std::vector<std::string> lines;
    lines.push_back("Nom : " + material.getName());
    lines.push_back("Catégorie : " + material.getCategory());

    if (material.getCategory() != "Livre" && material.getCategory() != "Renseignement" && material.getCategory() != "Outil")
    {
        lines.push_back("Qualité : " + material.getQualityLabel());
    }

    if (material.getCategory() == "Outil" && isRepairKitInventoryId(material.getId()))
    {
        if (isUsedRepairKitInventoryId(material.getId()))
        {
            lines.push_back("Nombre : " + std::to_string(material.getQuantity()) + " kit(s) entamé(s)");
            lines.push_back(
                "Durabilité par kit : "
                + std::to_string(usedRepairKitInventoryDurability(material.getId()))
                + "/"
                + std::to_string(maxRepairKitInventoryDurability(material.getId()))
            );
        }
        else
        {
            lines.push_back("Nombre : " + std::to_string(material.getQuantity()) + " kit(s) intact(s)");
            lines.push_back(
                "Durabilité par kit : "
                + std::to_string(maxRepairKitInventoryDurability(material.getId()))
                + "/"
                + std::to_string(maxRepairKitInventoryDurability(material.getId()))
            );
        }
    }
    else
    {
        lines.push_back("Quantité : " + std::to_string(material.getQuantity()));
    }

    lines.push_back("Description : " + material.getDescription());

    std::string valueLine = "Valeur unitaire : "
        + std::to_string(material.getValue() * material.getQualityPricePercent() / 100)
        + " pièces";
    if (material.hasSpecialQuality())
    {
        valueLine += " (base " + std::to_string(material.getValue()) + ")";
    }
    lines.push_back(valueLine);

    if (material.getCategory() == "Matériau de monstre")
    {
        lines.push_back("Utilité connue : revente, artisanat, trophées et recettes liées aux monstres.");
    }
    else if (material.getCategory() == "Matériau")
    {
        lines.push_back("Utilité connue : réparation, amélioration et fabrication d'équipement.");
    }
    else if (material.getCategory() == "Plante")
    {
        lines.push_back("Utilité connue : potions, remèdes, quêtes et secrets botaniques.");
    }
    else if (material.getCategory() == "Outil")
    {
        lines.push_back("Utilité connue : réparation autonome. Un kit intact reste empilé ; un kit entamé garde la marque de son usure.");
    }
    else if (material.getCategory() == "Renseignement" || material.getCategory() == "Livre")
    {
        lines.push_back("Utilité connue : débloquer ou compléter des informations du bestiaire.");
    }

    showInventoryScreen("MATÉRIAU / INFO", "inventory.material.inspect", lines, false);
}


// EN: displayWeapons declares or implements a focused behavior used by this module.
// FR: displayWeapons déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayWeapons() const
{
    displayWeaponList();
}

// EN: displayArmors declares or implements a focused behavior used by this module.
// FR: displayArmors déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayArmors() const
{
    displayArmorList();
}

// EN: displayConsumables declares or implements a focused behavior used by this module.
// FR: displayConsumables déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayConsumables() const
{
    displayConsumableList();
}

// EN: displayMaterials declares or implements a focused behavior used by this module.
// FR: displayMaterials déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayMaterials() const
{
    displayMaterialList();
}

// EN: display declares or implements a focused behavior used by this module.
// FR: display déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::display() const
{
    displaySummary();

    displayWeaponList();
    displayArmorList();
    displayConsumableList();
    displayMaterialList();
}
