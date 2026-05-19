// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ITEM_INVENTORY_HPP
#define INCLUDE_ITEM_INVENTORY_HPP

#include <vector>

#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/consumable/ConsumableType.hpp"
#include "item/material/Material.hpp"

class Inventory
{
private:
    std::vector<Weapon> weapons;
    std::vector<Armor> armors;
    std::vector<Consumable> consumables;
    std::vector<Material> materials;

    int or_;

public:
    Inventory();

    int getGold() const;
    void setGold(int amount);
    void earnGold(int amount);
    bool spendGold(int amount);

    int getWeaponCount() const;
    int getArmorCount() const;
    int getConsumableCount() const;
    int getMaterialCount() const;
    int countConsumables(ConsumableType type) const;

    const std::vector<Weapon>& getWeapons() const;
    const std::vector<Armor>& getArmors() const;
    const std::vector<Consumable>& getConsumables() const;
    const std::vector<Material>& getMaterials() const;

    void addWeapon(const Weapon& weapon);
    void addArmor(const Armor& armor);
    void addConsumable(const Consumable& consumable);
    void addMaterial(const Material& material);

    bool hasWeapon(int index) const;
    bool hasArmor(int index) const;
    bool hasConsumable(int index) const;
    bool hasMaterial(int index) const;

    Weapon getWeapon(int index) const;
    Weapon* getMutableWeapon(int index);

    Armor getArmor(int index) const;
    Armor* getMutableArmor(int index);

    Consumable getConsumable(int index) const;
    Material getMaterial(int index) const;
    Material* getMutableMaterial(int index);

    int findFirstConsumable(ConsumableType type) const;
    bool useFirstConsumable(ConsumableType type, Consumable& usedConsumable);

    bool removeWeapon(int index);
    bool removeArmor(int index);
    bool removeConsumable(int index);
    bool removeMaterialQuantity(int index, int quantity = 1);
    void clearAll();

    void displayWeapons() const;
    void displayArmors() const;
    void displayConsumables() const;
    void displayMaterials() const;
    void display() const;

    void displayWeaponList() const;
    void displayArmorList() const;
    void displayConsumableList() const;
    void displayMaterialList() const;
    void displaySummary() const;

    void inspectWeapon(int index) const;
    void inspectArmor(int index) const;
    void inspectConsumable(int index) const;
    void inspectMaterial(int index) const;
};

#endif
