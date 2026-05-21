// EN: Inventory.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Inventory.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/Inventory.hpp"

#include "progression/material/MaterialKnowledgeProgress.hpp"

#include <algorithm>
#include <iostream>
#include <string>


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
    std::cout << "===== ARMES =====" << std::endl;

    if (weapons.empty())
    {
        std::cout << "Aucune arme dans l'inventaire." << std::endl;
        std::cout << std::endl;
        return;
    }

    for (int i = 0; i < static_cast<int>(weapons.size()); i++)
    {
        std::cout << "[" << i << "] " << weapons[i].getName();

        if (!weapons[i].isIndestructible())
        {
            std::cout << " (" << weapons[i].getDurability() << "/" << weapons[i].getMaxDurability() << ")";
        }

        if (weapons[i].isBroken())
        {
            std::cout << " - Cassée";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

// EN: displayArmorList declares or implements a focused behavior used by this module.
// FR: displayArmorList déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayArmorList() const
{
    std::cout << "===== ARMURES =====" << std::endl;

    if (armors.empty())
    {
        std::cout << "Aucune armure dans l'inventaire." << std::endl;
        std::cout << std::endl;
        return;
    }

    for (int i = 0; i < static_cast<int>(armors.size()); i++)
    {
        std::cout << "[" << i << "] " << armors[i].getName();

        if (!armors[i].isIndestructible())
        {
            std::cout << " (" << armors[i].getDurability() << "/" << armors[i].getMaxDurability() << ")";
        }

        if (armors[i].isBroken())
        {
            std::cout << " - Cassée";
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

// EN: displayConsumableList declares or implements a focused behavior used by this module.
// FR: displayConsumableList déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayConsumableList() const
{
    std::cout << "===== CONSOMMABLES =====" << std::endl;

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
        std::cout << "Aucun consommable dans l'inventaire." << std::endl;
        std::cout << std::endl;
        return;
    }

    for (int i = 0; i < static_cast<int>(consumables.size()); i++)
    {
        std::cout << "[" << i << "] " << consumables[i].getName() << std::endl;
    }

    if (hasRepairKit)
    {
        std::cout << std::endl;
        std::cout << "--- Kits de réparation ---" << std::endl;

        for (const Material& material : materials)
        {
            if (material.getCategory() != "Outil" || !isRepairKitInventoryId(material.getId()))
            {
                continue;
            }

            std::cout << "- " << material.getName();

            if (isUsedRepairKitInventoryId(material.getId()))
            {
                std::cout << " x" << material.getQuantity()
                          << " | dura " << usedRepairKitInventoryDurability(material.getId())
                          << "/" << maxRepairKitInventoryDurability(material.getId());
            }
            else
            {
                std::cout << " x" << material.getQuantity()
                          << " | intact " << maxRepairKitInventoryDurability(material.getId())
                          << "/" << maxRepairKitInventoryDurability(material.getId());
            }

            std::cout << std::endl;
        }

        std::cout << "Astuce : sélectionne une arme ou une armure pour lancer une réparation." << std::endl;
    }

    std::cout << std::endl;
}

// EN: displayMaterialList declares or implements a focused behavior used by this module.
// FR: displayMaterialList déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displayMaterialList() const
{
    std::cout << "===== MATÉRIAUX / PLANTES / INFOS =====" << std::endl;

    if (materials.empty())
    {
        std::cout << "Aucun matériau, plante ou renseignement stocké." << std::endl;
        std::cout << std::endl;
        return;
    }

    for (int i = 0; i < static_cast<int>(materials.size()); i++)
    {
        std::cout << "[" << i << "] "
                  << materials[i].getName();

        if (materials[i].getCategory() == "Outil" && isRepairKitInventoryId(materials[i].getId()))
        {
            if (isUsedRepairKitInventoryId(materials[i].getId()))
            {
                std::cout << " x" << materials[i].getQuantity()
                          << " | dura " << usedRepairKitInventoryDurability(materials[i].getId())
                          << "/" << maxRepairKitInventoryDurability(materials[i].getId());
            }
            else
            {
                std::cout << " x" << materials[i].getQuantity()
                          << " | intact " << maxRepairKitInventoryDurability(materials[i].getId())
                          << "/" << maxRepairKitInventoryDurability(materials[i].getId());
            }
        }
        else
        {
            std::cout << " x" << materials[i].getQuantity();
            if (materials[i].hasSpecialQuality())
            {
                std::cout << " | " << materials[i].getQualityLabel();
            }
        }

        std::cout << " | " << materials[i].getCategory()
                  << std::endl;
    }

    std::cout << std::endl;
}

// EN: displaySummary declares or implements a focused behavior used by this module.
// FR: displaySummary déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::displaySummary() const
{
    std::cout << "================ INVENTAIRE ================" << std::endl;
    std::cout << "Or : " << or_ << " pièces" << std::endl;
    std::cout << "Armes : " << getWeaponCount() << std::endl;
    std::cout << "Armures : " << getArmorCount() << std::endl;
    std::cout << "Consommables : " << getConsumableCount() << std::endl;
    std::cout << "Matériaux / plantes / infos : " << getMaterialCount() << std::endl;
    std::cout << "Potions de soin : " << countConsumables(ConsumableType::Healing) << std::endl;
    std::cout << "Potions de rage : " << countConsumables(ConsumableType::Damage) << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << std::endl;
}

// EN: inspectWeapon declares or implements a focused behavior used by this module.
// FR: inspectWeapon déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::inspectWeapon(int index) const
{
    if (!hasWeapon(index))
    {
        std::cout << "Cette arme n'existe pas." << std::endl;
        std::cout << std::endl;
        return;
    }

    const Weapon& weapon = weapons[index];

    std::cout << "========== ARME ==========" << std::endl;
    std::cout << "Nom : " << weapon.getName() << std::endl;
    std::cout << "Description : " << weapon.getDescription() << std::endl;
    std::cout << "Bonus dégâts : +" << weapon.getMinDamageBonus()
              << " à +" << weapon.getMaxDamageBonus() << std::endl;
    std::cout << "Bonus critique : +" << weapon.getCriticalBonus() << std::endl;

    if (weapon.isIndestructible())
    {
        std::cout << "Durabilité : Indestructible" << std::endl;
    }
    else
    {
        std::cout << "Durabilité : " << weapon.getDurability()
                  << "/" << weapon.getMaxDurability() << std::endl;
    }

    std::cout << "État : " << (weapon.isBroken() ? "Cassée" : "Utilisable") << std::endl;
    std::cout << "Valeur : " << weapon.getValue() << " pièces" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << std::endl;
}

// EN: inspectArmor declares or implements a focused behavior used by this module.
// FR: inspectArmor déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::inspectArmor(int index) const
{
    if (!hasArmor(index))
    {
        std::cout << "Cette armure n'existe pas." << std::endl;
        std::cout << std::endl;
        return;
    }

    const Armor& armor = armors[index];

    std::cout << "========== ARMURE ==========" << std::endl;
    std::cout << "Nom : " << armor.getName() << std::endl;
    std::cout << "Description : " << armor.getDescription() << std::endl;
    std::cout << "Bonus PV max : +" << armor.getMaxHpBonus() << std::endl;
    std::cout << "Réduction dégâts : " << armor.getDamageReduction() << std::endl;

    if (armor.isIndestructible())
    {
        std::cout << "Durabilité : Indestructible" << std::endl;
    }
    else
    {
        std::cout << "Durabilité : " << armor.getDurability()
                  << "/" << armor.getMaxDurability() << std::endl;
    }

    std::cout << "État : " << (armor.isBroken() ? "Cassée" : "Utilisable") << std::endl;
    std::cout << "Valeur : " << armor.getValue() << " pièces" << std::endl;
    std::cout << "============================" << std::endl;
    std::cout << std::endl;
}

// EN: inspectConsumable declares or implements a focused behavior used by this module.
// FR: inspectConsumable déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::inspectConsumable(int index) const
{
    if (!hasConsumable(index))
    {
        std::cout << "Ce consommable n'existe pas." << std::endl;
        std::cout << std::endl;
        return;
    }

    const Consumable& consumable = consumables[index];

    std::cout << "======= CONSOMMABLE =======" << std::endl;
    std::cout << "Nom : " << consumable.getName() << std::endl;
    std::cout << "Description : " << consumable.getDescription() << std::endl;
    std::cout << "Puissance : " << consumable.getPower() << std::endl;
    std::cout << "Valeur : " << consumable.getValue() << " pièces" << std::endl;
    std::cout << "===========================" << std::endl;
    std::cout << std::endl;
}

// EN: inspectMaterial declares or implements a focused behavior used by this module.
// FR: inspectMaterial déclare ou implémente un comportement précis utilisé par ce module.
void Inventory::inspectMaterial(int index) const
{
    if (!hasMaterial(index))
    {
        std::cout << "Cette entrée n'existe pas." << std::endl;
        std::cout << std::endl;
        return;
    }

    materials[index].display();
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
