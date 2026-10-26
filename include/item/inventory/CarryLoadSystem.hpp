#ifndef INCLUDE_ITEM_INVENTORY_CARRYLOADSYSTEM_HPP
#define INCLUDE_ITEM_INVENTORY_CARRYLOADSYSTEM_HPP

#include <string>

class Player;
class Inventory;
class Weapon;
class Armor;
class Material;

struct CarryLoadSummary
{
    int used = 0;
    int capacity = 1;
    int percent = 0;
    int escapeModifierPercent = 0;
    int meleeAccuracyModifierPercent = 0;
    int physicalProtectionPercent = 0;
    int explorationTimeModifierPercent = 0;
    bool movementBlocked = false;
    std::string label = "Confortable";
};

class CarryLoadSystem
{
public:
    // Coarse bulk units: never kilograms and never inventory Tetris.
    static int weaponLoad(const Weapon& weapon);
    static int armorLoad(const Armor& armor);
    static int materialLoad(const Material& material);
    static int coinLoad(const Inventory& inventory);
    static int inventoryLoad(const Inventory& inventory, int equippedWeaponIndex = -1, int equippedArmorIndex = -1);
    static int carriedLoad(const Player& player);

    // Backpack progression is deliberately readable: 40 / 50 / 60 / 70 / 80 before body/class modifiers.
    static int backpackBaseCapacity(int tier);
    static int capacity(const Player& player);
    static CarryLoadSummary evaluate(const Player& player);

    static int escapeModifierPercent(const Player& player);
    static int meleeAccuracyModifierPercent(const Player& player);
    static int physicalProtectionPercent(const Player& player);
    static int actionTimeModifierPercent(const Player& player);
    static int adjustedActionTimeUnits(const Player& player, int baseUnits);
    static bool movementBlocked(const Player& player);

    // Positive = easier to hit, negative = harder to hit. Race base + chosen morphology.
    static int incomingAccuracyModifierPercent(const Player& player);
    static std::string morphologyLabel(const Player& player);
};

#endif
