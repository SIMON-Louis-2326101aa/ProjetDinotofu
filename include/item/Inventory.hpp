// EN: Inventory.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Inventory.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ITEM_INVENTORY_HPP
#define INCLUDE_ITEM_INVENTORY_HPP

#include <string>
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

    long long totalCopper_;

public:
    // EN: Inventory declares or implements a focused behavior used by this module.
    // FR: Inventory déclare ou implémente un comportement précis utilisé par ce module.
    Inventory();

    // EN: getGold declares or implements a focused behavior used by this module.
    // FR: getGold déclare ou implémente un comportement précis utilisé par ce module.
    int getGold() const;
    long long getTotalCopper() const;
    // EN: setGold declares or implements a focused behavior used by this module.
    // FR: setGold déclare ou implémente un comportement précis utilisé par ce module.
    void setGold(int amount);
    void setTotalCopper(long long amount);
    // EN: earnGold declares or implements a focused behavior used by this module.
    // FR: earnGold déclare ou implémente un comportement précis utilisé par ce module.
    void earnGold(int amount);
    void earnCopper(long long amount);
    // EN: spendGold declares or implements a focused behavior used by this module.
    // FR: spendGold déclare ou implémente un comportement précis utilisé par ce module.
    bool spendGold(int amount);
    bool spendCopper(long long amount);
    std::string getWalletLine() const;
    std::string getWalletTotalLine() const;

    // EN: getWeaponCount declares or implements a focused behavior used by this module.
    // FR: getWeaponCount déclare ou implémente un comportement précis utilisé par ce module.
    int getWeaponCount() const;
    // EN: getArmorCount declares or implements a focused behavior used by this module.
    // FR: getArmorCount déclare ou implémente un comportement précis utilisé par ce module.
    int getArmorCount() const;
    // EN: getConsumableCount declares or implements a focused behavior used by this module.
    // FR: getConsumableCount déclare ou implémente un comportement précis utilisé par ce module.
    int getConsumableCount() const;
    // EN: getMaterialCount declares or implements a focused behavior used by this module.
    // FR: getMaterialCount déclare ou implémente un comportement précis utilisé par ce module.
    int getMaterialCount() const;
    // EN: countConsumables declares or implements a focused behavior used by this module.
    // FR: countConsumables déclare ou implémente un comportement précis utilisé par ce module.
    int countConsumables(ConsumableType type) const;
    // EN: findMaterialIndexById declares or implements a focused behavior used by this module.
    // FR: findMaterialIndexById déclare ou implémente un comportement précis utilisé par ce module.
    int findMaterialIndexById(const std::string& id) const;
    // EN: countMaterialById declares or implements a focused behavior used by this module.
    // FR: countMaterialById déclare ou implémente un comportement précis utilisé par ce module.
    int countMaterialById(const std::string& id) const;
    // EN: countMaterialQualityPointsById declares or implements a focused behavior used by this module.
    // FR: countMaterialQualityPointsById déclare ou implémente un comportement précis utilisé par ce module.
    int countMaterialQualityPointsById(const std::string& id) const;

    // EN: getWeapons declares or implements a focused behavior used by this module.
    // FR: getWeapons déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<Weapon>& getWeapons() const;
    // EN: getArmors declares or implements a focused behavior used by this module.
    // FR: getArmors déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<Armor>& getArmors() const;
    // EN: getConsumables declares or implements a focused behavior used by this module.
    // FR: getConsumables déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<Consumable>& getConsumables() const;
    // EN: getMaterials declares or implements a focused behavior used by this module.
    // FR: getMaterials déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<Material>& getMaterials() const;

    // EN: addWeapon declares or implements a focused behavior used by this module.
    // FR: addWeapon déclare ou implémente un comportement précis utilisé par ce module.
    void addWeapon(const Weapon& weapon);
    // EN: addArmor declares or implements a focused behavior used by this module.
    // FR: addArmor déclare ou implémente un comportement précis utilisé par ce module.
    void addArmor(const Armor& armor);
    // EN: addConsumable declares or implements a focused behavior used by this module.
    // FR: addConsumable déclare ou implémente un comportement précis utilisé par ce module.
    void addConsumable(const Consumable& consumable);
    // EN: addMaterial declares or implements a focused behavior used by this module.
    // FR: addMaterial déclare ou implémente un comportement précis utilisé par ce module.
    void addMaterial(const Material& material);

    // EN: hasWeapon declares or implements a focused behavior used by this module.
    // FR: hasWeapon déclare ou implémente un comportement précis utilisé par ce module.
    bool hasWeapon(int index) const;
    // EN: hasArmor declares or implements a focused behavior used by this module.
    // FR: hasArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool hasArmor(int index) const;
    // EN: hasConsumable declares or implements a focused behavior used by this module.
    // FR: hasConsumable déclare ou implémente un comportement précis utilisé par ce module.
    bool hasConsumable(int index) const;
    // EN: hasMaterial declares or implements a focused behavior used by this module.
    // FR: hasMaterial déclare ou implémente un comportement précis utilisé par ce module.
    bool hasMaterial(int index) const;

    // EN: getWeapon declares or implements a focused behavior used by this module.
    // FR: getWeapon déclare ou implémente un comportement précis utilisé par ce module.
    Weapon getWeapon(int index) const;
    // EN: getMutableWeapon declares or implements a focused behavior used by this module.
    // FR: getMutableWeapon déclare ou implémente un comportement précis utilisé par ce module.
    Weapon* getMutableWeapon(int index);

    // EN: getArmor declares or implements a focused behavior used by this module.
    // FR: getArmor déclare ou implémente un comportement précis utilisé par ce module.
    Armor getArmor(int index) const;
    // EN: getMutableArmor declares or implements a focused behavior used by this module.
    // FR: getMutableArmor déclare ou implémente un comportement précis utilisé par ce module.
    Armor* getMutableArmor(int index);

    // EN: getConsumable declares or implements a focused behavior used by this module.
    // FR: getConsumable déclare ou implémente un comportement précis utilisé par ce module.
    Consumable getConsumable(int index) const;
    // EN: getMaterial declares or implements a focused behavior used by this module.
    // FR: getMaterial déclare ou implémente un comportement précis utilisé par ce module.
    Material getMaterial(int index) const;
    // EN: getMutableMaterial declares or implements a focused behavior used by this module.
    // FR: getMutableMaterial déclare ou implémente un comportement précis utilisé par ce module.
    Material* getMutableMaterial(int index);

    // EN: findFirstConsumable declares or implements a focused behavior used by this module.
    // FR: findFirstConsumable déclare ou implémente un comportement précis utilisé par ce module.
    int findFirstConsumable(ConsumableType type) const;
    // EN: useFirstConsumable declares or implements a focused behavior used by this module.
    // FR: useFirstConsumable déclare ou implémente un comportement précis utilisé par ce module.
    bool useFirstConsumable(ConsumableType type, Consumable& usedConsumable);

    // EN: removeWeapon declares or implements a focused behavior used by this module.
    // FR: removeWeapon déclare ou implémente un comportement précis utilisé par ce module.
    bool removeWeapon(int index);
    // EN: removeArmor declares or implements a focused behavior used by this module.
    // FR: removeArmor déclare ou implémente un comportement précis utilisé par ce module.
    bool removeArmor(int index);
    // EN: removeConsumable declares or implements a focused behavior used by this module.
    // FR: removeConsumable déclare ou implémente un comportement précis utilisé par ce module.
    bool removeConsumable(int index);
    bool removeMaterialQuantity(int index, int quantity = 1);
    bool removeMaterialQuantityById(const std::string& id, int quantity = 1);
    bool removeMaterialQuantityByIdFlexible(const std::string& id, int normalQualityQuantity = 1);
    // EN: clearAll declares or implements a focused behavior used by this module.
    // FR: clearAll déclare ou implémente un comportement précis utilisé par ce module.
    void clearAll();

    // EN: displayWeapons declares or implements a focused behavior used by this module.
    // FR: displayWeapons déclare ou implémente un comportement précis utilisé par ce module.
    void displayWeapons() const;
    // EN: displayArmors declares or implements a focused behavior used by this module.
    // FR: displayArmors déclare ou implémente un comportement précis utilisé par ce module.
    void displayArmors() const;
    // EN: displayConsumables declares or implements a focused behavior used by this module.
    // FR: displayConsumables déclare ou implémente un comportement précis utilisé par ce module.
    void displayConsumables() const;
    // EN: displayMaterials declares or implements a focused behavior used by this module.
    // FR: displayMaterials déclare ou implémente un comportement précis utilisé par ce module.
    void displayMaterials() const;
    void display() const;

    // EN: displayWeaponList declares or implements a focused behavior used by this module.
    // FR: displayWeaponList déclare ou implémente un comportement précis utilisé par ce module.
    void displayWeaponList() const;
    // EN: displayArmorList declares or implements a focused behavior used by this module.
    // FR: displayArmorList déclare ou implémente un comportement précis utilisé par ce module.
    void displayArmorList() const;
    // EN: displayConsumableList declares or implements a focused behavior used by this module.
    // FR: displayConsumableList déclare ou implémente un comportement précis utilisé par ce module.
    void displayConsumableList() const;
    // EN: displayMaterialList declares or implements a focused behavior used by this module.
    // FR: displayMaterialList déclare ou implémente un comportement précis utilisé par ce module.
    void displayMaterialList() const;
    // EN: displaySummary declares or implements a focused behavior used by this module.
    // FR: displaySummary déclare ou implémente un comportement précis utilisé par ce module.
    void displaySummary() const;

    // EN: inspectWeapon declares or implements a focused behavior used by this module.
    // FR: inspectWeapon déclare ou implémente un comportement précis utilisé par ce module.
    void inspectWeapon(int index) const;
    // EN: inspectArmor declares or implements a focused behavior used by this module.
    // FR: inspectArmor déclare ou implémente un comportement précis utilisé par ce module.
    void inspectArmor(int index) const;
    // EN: inspectConsumable declares or implements a focused behavior used by this module.
    // FR: inspectConsumable déclare ou implémente un comportement précis utilisé par ce module.
    void inspectConsumable(int index) const;
    // EN: inspectMaterial declares or implements a focused behavior used by this module.
    // FR: inspectMaterial déclare ou implémente un comportement précis utilisé par ce module.
    void inspectMaterial(int index) const;
};

#endif
