// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "item/Inventory.hpp"

#include <iostream>

Inventory::Inventory()
{
    or_ = 0;
}

int Inventory::getGold() const
{
    return or_;
}

void Inventory::earnGold(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    or_ += amount;
}

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

int Inventory::getWeaponCount() const
{
    return static_cast<int>(weapons.size());
}

int Inventory::getArmorCount() const
{
    return static_cast<int>(armors.size());
}

int Inventory::getConsumableCount() const
{
    return static_cast<int>(consumables.size());
}

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

const std::vector<Weapon>& Inventory::getWeapons() const
{
    return weapons;
}

const std::vector<Armor>& Inventory::getArmors() const
{
    return armors;
}

const std::vector<Consumable>& Inventory::getConsumables() const
{
    return consumables;
}

void Inventory::addWeapon(const Weapon& weapon)
{
    weapons.push_back(weapon);
}

void Inventory::addArmor(const Armor& armor)
{
    armors.push_back(armor);
}

void Inventory::addConsumable(const Consumable& consumable)
{
    consumables.push_back(consumable);
}

bool Inventory::hasWeapon(int index) const
{
    return index >= 0 && index < static_cast<int>(weapons.size());
}

bool Inventory::hasArmor(int index) const
{
    return index >= 0 && index < static_cast<int>(armors.size());
}

bool Inventory::hasConsumable(int index) const
{
    return index >= 0 && index < static_cast<int>(consumables.size());
}

Weapon Inventory::getWeapon(int index) const
{
    if (!hasWeapon(index))
    {
        return Weapon();
    }

    return weapons[index];
}

Weapon* Inventory::getMutableWeapon(int index)
{
    if (!hasWeapon(index))
    {
        return nullptr;
    }

    return &weapons[index];
}

Armor Inventory::getArmor(int index) const
{
    if (!hasArmor(index))
    {
        return Armor();
    }

    return armors[index];
}

Armor* Inventory::getMutableArmor(int index)
{
    if (!hasArmor(index))
    {
        return nullptr;
    }

    return &armors[index];
}

Consumable Inventory::getConsumable(int index) const
{
    if (!hasConsumable(index))
    {
        return Consumable();
    }

    return consumables[index];
}

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

bool Inventory::removeConsumable(int index)
{
    if (!hasConsumable(index))
    {
        return false;
    }

    consumables.erase(consumables.begin() + index);
    return true;
}

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

void Inventory::displayConsumableList() const
{
    std::cout << "===== CONSOMMABLES =====" << std::endl;

    if (consumables.empty())
    {
        std::cout << "Aucun consommable dans l'inventaire." << std::endl;
        std::cout << std::endl;
        return;
    }

    for (int i = 0; i < static_cast<int>(consumables.size()); i++)
    {
        std::cout << "[" << i << "] " << consumables[i].getName() << std::endl;
    }

    std::cout << std::endl;
}

void Inventory::displaySummary() const
{
    std::cout << "================ INVENTAIRE ================" << std::endl;
    std::cout << "Or : " << or_ << " pièces" << std::endl;
    std::cout << "Armes : " << getWeaponCount() << std::endl;
    std::cout << "Armures : " << getArmorCount() << std::endl;
    std::cout << "Consommables : " << getConsumableCount() << std::endl;
    std::cout << "Potions de soin : " << countConsumables(ConsumableType::Healing) << std::endl;
    std::cout << "Potions de rage : " << countConsumables(ConsumableType::Damage) << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << std::endl;
}

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

void Inventory::displayWeapons() const
{
    displayWeaponList();
}

void Inventory::displayArmors() const
{
    displayArmorList();
}

void Inventory::displayConsumables() const
{
    displayConsumableList();
}

void Inventory::display() const
{
    displaySummary();

    displayWeaponList();
    displayArmorList();
    displayConsumableList();
}