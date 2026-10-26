#include "item/inventory/CarryLoadSystem.hpp"

#include "entity/Player.hpp"
#include "item/Inventory.hpp"
#include "item/equipment/EquipmentWeightRules.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <map>
#include <string>

namespace
{
    std::string lowerLoadText(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    int raceCapacityPercent(CharacterRace race)
    {
        switch (race)
        {
            case CharacterRace::Fairy: return -20;
            case CharacterRace::Gnome:
            case CharacterRace::Halfling: return -10;
            case CharacterRace::Elf:
            case CharacterRace::DarkElf:
            case CharacterRace::Kitsune:
            case CharacterRace::SemiFox:
            case CharacterRace::SemiCat:
            case CharacterRace::SemiBird: return -5;
            case CharacterRace::Dwarf: return 5;
            case CharacterRace::SemiWolf:
            case CharacterRace::SemiDog:
            case CharacterRace::SemiLizard:
            case CharacterRace::Demon: return 5;
            case CharacterRace::HalfDragon: return 10;
            case CharacterRace::Orc: return 15;
            default: return 0;
        }
    }

    int raceHitboxPercent(CharacterRace race)
    {
        switch (race)
        {
            case CharacterRace::Fairy: return -8;
            case CharacterRace::Gnome:
            case CharacterRace::Halfling: return -5;
            case CharacterRace::Elf:
            case CharacterRace::DarkElf:
            case CharacterRace::Kitsune:
            case CharacterRace::SemiFox:
            case CharacterRace::SemiCat:
            case CharacterRace::SemiBird: return -2;
            case CharacterRace::Dwarf: return 2;
            case CharacterRace::HalfDragon: return 4;
            case CharacterRace::Orc: return 5;
            default: return 0;
        }
    }

    int morphologyCapacityPercent(const Player& player)
    {
        const std::string morphology = player.getBodyMorphology();
        if (morphology == "Fine") return -10;
        if (morphology == "Robuste") return 10;
        return 0;
    }

    int morphologyHitboxPercent(const Player& player)
    {
        const std::string morphology = player.getBodyMorphology();
        if (morphology == "Fine") return -3;
        if (morphology == "Robuste") return 3;
        return 0;
    }

    bool isPureMageClass(const Player& player)
    {
        const std::string text = lowerLoadText(player.getType());
        // Bibliomancer is the one object-centric magic archetype exempted from the pure-mage carrying penalty.
        if (text.find("bibliomancien") != std::string::npos) return false;
        if (text.find("mage-lame") != std::string::npos) return false;
        return text.find("mage") != std::string::npos
            || text.find("magicien") != std::string::npos
            || text.find("mancien") != std::string::npos
            || text.find("arcaniste") != std::string::npos
            || text.find("ensorceleur") != std::string::npos
            || text.find("sorcier") != std::string::npos
            || text.find("occultiste") != std::string::npos
            || text.find("miragien") != std::string::npos
            || text.find("démoniste") != std::string::npos
            || text.find("demoniste") != std::string::npos;
    }

    bool isDocumentLikeMaterial(const Material& material)
    {
        const std::string text = lowerLoadText(material.getCategory() + " " + material.getName());
        return text.find("renseignement") != std::string::npos
            || text.find("information") != std::string::npos
            || text.find("notice") != std::string::npos
            || text.find("document") != std::string::npos
            || text.find("livre") != std::string::npos
            || text.find("carte") != std::string::npos;
    }

    bool isDenseMaterial(const Material& material)
    {
        const std::string text = lowerLoadText(material.getCategory() + " " + material.getName());
        return text.find("miner") != std::string::npos
            || text.find("métal") != std::string::npos
            || text.find("metal") != std::string::npos
            || text.find("pierre") != std::string::npos
            || text.find("cristal") != std::string::npos
            || text.find("carapace") != std::string::npos
            || text.rfind("os ", 0) == 0
            || text.find(" os") != std::string::npos
            || text.find("ossement") != std::string::npos
            || text.find("osseux") != std::string::npos;
    }

    bool isLightMaterial(const Material& material)
    {
        const std::string text = lowerLoadText(material.getCategory() + " " + material.getName());
        return text.find("plante") != std::string::npos
            || text.find("herbe") != std::string::npos
            || text.find("poudre") != std::string::npos
            || text.find("plume") != std::string::npos
            || text.find("pétale") != std::string::npos
            || text.find("petale") != std::string::npos
            || text.rfind("fil ", 0) == 0
            || text.find(" fil") != std::string::npos;
    }
}

int CarryLoadSystem::weaponLoad(const Weapon& weapon)
{
    switch (EquipmentWeightRules::getWeaponWeightClass(weapon))
    {
        case EquipmentWeightClass::Light: return 3;
        case EquipmentWeightClass::Heavy: return 8;
        case EquipmentWeightClass::Medium:
        default: return 5;
    }
}

int CarryLoadSystem::armorLoad(const Armor& armor)
{
    switch (EquipmentWeightRules::getArmorWeightClass(armor))
    {
        case EquipmentWeightClass::Light: return 8;
        case EquipmentWeightClass::Heavy: return 18;
        case EquipmentWeightClass::Medium:
        default: return 12;
    }
}

int CarryLoadSystem::materialLoad(const Material& material)
{
    const int quantity = std::max(1, material.getQuantity());
    if (Inventory::isAmmunitionMaterialId(material.getId()))
    {
        return std::max(1, (quantity + 19) / 20);
    }
    if (isDocumentLikeMaterial(material)) return std::max(1, (quantity + 19) / 20);
    if (isDenseMaterial(material)) return std::max(2, ((quantity + 4) / 5) * 2);
    if (isLightMaterial(material)) return std::max(1, (quantity + 19) / 20);
    return std::max(1, (quantity + 9) / 10);
}

int CarryLoadSystem::coinLoad(const Inventory& inventory)
{
    const CoinBreakdown coins = inventory.getCoinStacks();
    const long long physicalPieces = std::max(0LL, coins.platinum)
        + std::max(0LL, coins.gold)
        + std::max(0LL, coins.electrum)
        + std::max(0LL, coins.iron)
        + std::max(0LL, coins.copper);
    if (physicalPieces <= 0) return 0;
    const long long load = (physicalPieces + 99) / 100;
    return static_cast<int>(std::min<long long>(load, std::numeric_limits<int>::max()));
}

int CarryLoadSystem::inventoryLoad(const Inventory& inventory, int equippedWeaponIndex, int equippedArmorIndex)
{
    int total = coinLoad(inventory);

    for (int i = 0; i < inventory.getWeaponCount(); ++i)
    {
        if (i == equippedWeaponIndex) continue;
        total += weaponLoad(inventory.getWeapon(i));
    }
    for (int i = 0; i < inventory.getArmorCount(); ++i)
    {
        if (i == equippedArmorIndex) continue;
        total += armorLoad(inventory.getArmor(i));
    }

    // Five identical consumables use one bulk unit.
    std::map<std::string, int> consumableCounts;
    for (const Consumable& consumable : inventory.getConsumables())
    {
        ++consumableCounts[consumable.getName()];
    }
    for (const auto& [name, count] : consumableCounts)
    {
        (void)name;
        total += std::max(1, (count + 4) / 5);
    }

    for (const Material& material : inventory.getMaterials())
    {
        total += materialLoad(material);
    }

    return std::max(0, total);
}

int CarryLoadSystem::carriedLoad(const Player& player)
{
    return inventoryLoad(player.getInventory(), player.getEquippedWeaponIndex(), player.getEquippedArmorIndex());
}

int CarryLoadSystem::backpackBaseCapacity(int tier)
{
    switch (std::clamp(tier, 0, 4))
    {
        case 1: return 50;
        case 2: return 60;
        case 3: return 70;
        case 4: return 80;
        case 0:
        default: return 40;
    }
}

int CarryLoadSystem::capacity(const Player& player)
{
    const int base = backpackBaseCapacity(player.getBackpackTier());
    int percent = 100 + raceCapacityPercent(player.getRace()) + morphologyCapacityPercent(player);
    if (isPureMageClass(player)) percent -= 10;
    percent = std::clamp(percent, 55, 150);

    const int strength = player.getAttributes().getStrength();
    const int strengthBonus = std::clamp((strength - 10) / 3, -2, 6);
    return std::max(20, (base * percent + 50) / 100 + strengthBonus);
}

CarryLoadSummary CarryLoadSystem::evaluate(const Player& player)
{
    CarryLoadSummary summary;
    summary.used = carriedLoad(player);
    summary.capacity = capacity(player);
    summary.percent = summary.capacity <= 0 ? 0 : (summary.used * 100) / summary.capacity;

    if (summary.percent < 85)
    {
        summary.label = "Confortable";
    }
    else if (summary.percent <= 100)
    {
        summary.label = "Chargé";
    }
    else if (summary.percent <= 125)
    {
        summary.label = "Surchargé";
        summary.escapeModifierPercent = -10;
        summary.meleeAccuracyModifierPercent = -5;
        summary.physicalProtectionPercent = 2;
        summary.explorationTimeModifierPercent = 10;
    }
    else if (summary.percent < 200)
    {
        summary.label = "Très surchargé";
        summary.escapeModifierPercent = -20;
        summary.meleeAccuracyModifierPercent = -10;
        summary.physicalProtectionPercent = 4;
        summary.explorationTimeModifierPercent = 25;
    }
    else
    {
        // At 200%+ the decisive consequence is logistical: the character can no longer
        // start travel/exploration until they lighten the load. Combat penalties do not
        // escalate into a third arbitrary tier beyond the agreed Very Overloaded state.
        summary.label = "Immobilisé par la charge";
        summary.escapeModifierPercent = -20;
        summary.meleeAccuracyModifierPercent = -10;
        summary.physicalProtectionPercent = 4;
        summary.explorationTimeModifierPercent = 25;
        summary.movementBlocked = true;
    }
    return summary;
}

int CarryLoadSystem::escapeModifierPercent(const Player& player) { return evaluate(player).escapeModifierPercent; }
int CarryLoadSystem::meleeAccuracyModifierPercent(const Player& player) { return evaluate(player).meleeAccuracyModifierPercent; }
int CarryLoadSystem::physicalProtectionPercent(const Player& player) { return evaluate(player).physicalProtectionPercent; }
int CarryLoadSystem::actionTimeModifierPercent(const Player& player) { return evaluate(player).explorationTimeModifierPercent; }

int CarryLoadSystem::adjustedActionTimeUnits(const Player& player, int baseUnits)
{
    baseUnits = std::max(0, baseUnits);
    if (baseUnits == 0) return 0;
    const int modifier = std::max(0, actionTimeModifierPercent(player));
    return std::max(1, (baseUnits * (100 + modifier) + 99) / 100);
}

bool CarryLoadSystem::movementBlocked(const Player& player) { return evaluate(player).movementBlocked; }

int CarryLoadSystem::incomingAccuracyModifierPercent(const Player& player)
{
    return std::clamp(raceHitboxPercent(player.getRace()) + morphologyHitboxPercent(player), -12, 12);
}

std::string CarryLoadSystem::morphologyLabel(const Player& player)
{
    return player.getBodyMorphology();
}
