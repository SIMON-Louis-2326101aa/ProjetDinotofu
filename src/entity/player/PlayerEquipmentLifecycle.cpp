// EN: Split from Player.cpp to keep Player responsibilities maintainable.
// FR: Extrait de Player.cpp afin de garder des responsabilités Player maintenables.
#include "entity/Player.hpp"
#include "progression/language/LanguageSystem.hpp"
#include "economy/EconomyBalance.hpp"
#include "core/VersionInfo.hpp"
#include "economy/Money.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/Level.hpp"
#include "progression/TitleCatalog.hpp"
#include "character/RaceCatalog.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "item/equipment/EquipmentWeightRules.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <random>
#include <sstream>
#include <tuple>
#include <vector>

namespace
{

    bool isDistanceStarterClass(const std::string& className)
    {
        std::string normalized = className;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        return normalized.find("archer") != std::string::npos
            || normalized.find("rôdeur") != std::string::npos
            || normalized.find("rodeur") != std::string::npos
            || normalized.find("arbal") != std::string::npos
            || normalized.find("chasseur") != std::string::npos
            || normalized.find("lanceur de dagues") != std::string::npos
            || normalized.find("tireur") != std::string::npos
            || normalized.find("artificier") != std::string::npos
            || normalized.find("javelinier") != std::string::npos
            || normalized.find("trappeur") != std::string::npos
            || normalized.find("guetteur") != std::string::npos
            || normalized.find("messager arm") != std::string::npos;
    }

    bool usesStarterAmmunition(const std::string& className)
    {
        std::string normalized = className;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        return normalized.find("archer") != std::string::npos
            || normalized.find("rôdeur") != std::string::npos
            || normalized.find("rodeur") != std::string::npos
            || normalized.find("arbal") != std::string::npos
            || normalized.find("chasseur") != std::string::npos
            || normalized.find("lanceur de dagues") != std::string::npos
            || normalized.find("tireur") != std::string::npos
            || normalized.find("artificier") != std::string::npos
            || normalized.find("javelinier") != std::string::npos
            || normalized.find("trappeur") != std::string::npos
            || normalized.find("guetteur") != std::string::npos
            || normalized.find("messager arm") != std::string::npos;
    }

    bool inventoryHasWeaponNamed(const Inventory& inventory, const std::string& name)
    {
        for (const Weapon& weapon : inventory.getWeapons())
        {
            if (weapon.getName() == name) return true;
        }
        return false;
    }

    bool inventoryHasArmorNamed(const Inventory& inventory, const std::string& name)
    {
        for (const Armor& armor : inventory.getArmors())
        {
            if (armor.getName() == name) return true;
        }
        return false;
    }

    bool inventoryHasMaterialId(const Inventory& inventory, const std::string& id)
    {
        return inventory.countMaterialById(id) > 0;
    }
}

int Player::getEquippedWeaponIndex() const
{
    return equippedWeaponIndex;
}

// EN: hasEquippedWeapon declares or implements a focused behavior used by this module.
// FR: hasEquippedWeapon déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasEquippedWeapon() const
{
    return inventory.hasWeapon(equippedWeaponIndex);
}

// EN: getEquippedWeapon declares or implements a focused behavior used by this module.
// FR: getEquippedWeapon déclare ou implémente un comportement précis utilisé par ce module.
Weapon Player::getEquippedWeapon() const
{
    if (!hasEquippedWeapon())
    {
        return Weapon();
    }

    return inventory.getWeapon(equippedWeaponIndex);
}

// EN: equipWeapon declares or implements a focused behavior used by this module.
// FR: equipWeapon déclare ou implémente un comportement précis utilisé par ce module.
bool Player::equipWeapon(int index)
{
    if (!inventory.hasWeapon(index))
    {
        return false;
    }

    equippedWeaponIndex = index;
    return true;
}

// EN: unequipWeapon declares or implements a focused behavior used by this module.
// FR: unequipWeapon déclare ou implémente un comportement précis utilisé par ce module.
void Player::unequipWeapon()
{
    equippedWeaponIndex = -1;
}

// EN: getEquippedArmorIndex declares or implements a focused behavior used by this module.
// FR: getEquippedArmorIndex déclare ou implémente un comportement précis utilisé par ce module.
int Player::getEquippedArmorIndex() const
{
    return equippedArmorIndex;
}

// EN: hasEquippedArmor declares or implements a focused behavior used by this module.
// FR: hasEquippedArmor déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasEquippedArmor() const
{
    return inventory.hasArmor(equippedArmorIndex);
}

// EN: getEquippedArmor declares or implements a focused behavior used by this module.
// FR: getEquippedArmor déclare ou implémente un comportement précis utilisé par ce module.
Armor Player::getEquippedArmor() const
{
    if (!hasEquippedArmor())
    {
        return Armor();
    }

    return inventory.getArmor(equippedArmorIndex);
}

// EN: equipArmor declares or implements a focused behavior used by this module.
// FR: equipArmor déclare ou implémente un comportement précis utilisé par ce module.
bool Player::equipArmor(int index)
{
    if (!inventory.hasArmor(index))
    {
        return false;
    }

    const Armor candidateArmor = inventory.getArmor(index);
    std::string armorProbe = candidateArmor.getName() + " " + candidateArmor.getDescription();
    std::transform(armorProbe.begin(), armorProbe.end(), armorProbe.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (armorProbe.find("non ajust") != std::string::npos
        || armorProbe.find("mauvaise taille") != std::string::npos)
    {
        return false;
    }

    int oldMaxHealthBonus = getEquippedArmorMaxHpBonus();

    equippedArmorIndex = index;

    int newMaxHealthBonus = getEquippedArmorMaxHpBonus();
    int maxHealthDifference = newMaxHealthBonus - oldMaxHealthBonus;

    maxHp += maxHealthDifference;
    hp += maxHealthDifference;

    if (maxHp < 1)
    {
        maxHp = 1;
    }

    if (hp < 1)
    {
        hp = 1;
    }

    if (hp > maxHp)
    {
        hp = maxHp;
    }

    return true;
}

// EN: unequipArmor declares or implements a focused behavior used by this module.
// FR: unequipArmor déclare ou implémente un comportement précis utilisé par ce module.
void Player::unequipArmor()
{
    int oldMaxHealthBonus = getEquippedArmorMaxHpBonus();

    equippedArmorIndex = -1;

    maxHp -= oldMaxHealthBonus;

    if (maxHp < 1)
    {
        maxHp = 1;
    }

    if (hp > maxHp)
    {
        hp = maxHp;
    }
}

// EN: initializeStarterInventory declares or implements a focused behavior used by this module.
// FR: initializeStarterInventory déclare ou implémente un comportement précis utilisé par ce module.
void Player::initializeStarterInventory()
{
    initializeStarterInventory(DifficultyMode::Normal);
}

// EN: initializeStarterInventory declares or implements a focused behavior used by this module.
// FR: initializeStarterInventory déclare ou implémente un comportement précis utilisé par ce module.
void Player::initializeStarterInventory(DifficultyMode difficulty)
{
    inventory.addWeapon(WeaponCatalog::createBareHands());
    recordStarterKitEntry("Arme de base : Mains nues");

    Weapon classStarterWeapon = WeaponCatalog::createStarterWeaponForClass(type);
    inventory.addWeapon(classStarterWeapon);
    recordStarterKitEntry("Arme de classe : " + classStarterWeapon.getName());

    if (isDistanceStarterClass(type))
    {
        Weapon emergencyKnife = WeaponCatalog::createEmergencyWoodKnife();
        inventory.addWeapon(emergencyKnife);
        recordStarterKitEntry("Défense urgente : " + emergencyKnife.getName());
        MessageScreen::show("KIT DE DÉPART", "player.starter_kit.emergency_weapon", {"Équipement de secours : classe à distance détectée, petit couteau de bois ajouté."}, false);
    }

    if (usesStarterAmmunition(type))
    {
        if (type.find("Arbal") != std::string::npos || type.find("arbal") != std::string::npos)
        {
            inventory.addMaterial(MaterialCatalog::createById("training_bolts", 14));
            recordStarterKitEntry("Munitions de départ : carreaux d'entraînement x14");
            MessageScreen::show("MUNITIONS DE DÉPART", "player.starter_kit.bolts", {"Munitions de départ : carreaux d'entraînement x14."}, false);
        }
        else if (type.find("dagues") != std::string::npos || type.find("Dagues") != std::string::npos)
        {
            inventory.addMaterial(MaterialCatalog::createById("training_throwing_knives", 10));
            recordStarterKitEntry("Munitions de départ : couteaux de lancer émoussés x10");
            MessageScreen::show("MUNITIONS DE DÉPART", "player.starter_kit.knives", {"Munitions de départ : couteaux de lancer émoussés x10."}, false);
        }
        else
        {
            inventory.addMaterial(MaterialCatalog::createById("training_arrows", 18));
            recordStarterKitEntry("Munitions de départ : flèches d'entraînement x18");
            MessageScreen::show("MUNITIONS DE DÉPART", "player.starter_kit.arrows", {"Munitions de départ : flèches d'entraînement x18."}, false);
        }

        MessageScreen::show("RECETTES FUTURES", "player.starter_kit.special_ammo_note", {"Les recettes de munitions spéciales se découvrent par exploration, expérience et expérimentation."}, false);
    }

    Weapon* starterWeapon = inventory.getMutableWeapon(1);

    if (starterWeapon != nullptr)
    {
        starterWeapon->loseDurability(
            DifficultyRules::getStarterWeaponDurabilityLoss(difficulty)
        );
    }

    equipWeapon(1);

    Armor simpleOutfit = ArmorCatalog::createSimpleOutfit();
    inventory.addArmor(simpleOutfit);
    recordStarterKitEntry("Tenue de base : " + simpleOutfit.getName());

    Armor classStarterArmor = ArmorCatalog::createStarterArmorForClass(type);
    inventory.addArmor(classStarterArmor);
    recordStarterKitEntry("Protection de classe : " + classStarterArmor.getName());

    Armor* starterArmor = inventory.getMutableArmor(1);

    if (starterArmor != nullptr)
    {
        starterArmor->loseDurability(
            DifficultyRules::getStarterArmorDurabilityLoss(difficulty)
        );
    }

    equipArmor(1);

    int starterHealingPotions =
        DifficultyRules::getStarterHealingPotionCount(
            healingPotionCount,
            difficulty
        );

    int starterDamagePotions =
        DifficultyRules::getStarterDamagePotionCount(
            damagePotionCount,
            difficulty
        );

    for (int i = 0; i < starterHealingPotions; i++)
    {
        inventory.addConsumable(ConsumableCatalog::createBasicHealingPotion());
    }
    if (starterHealingPotions > 0)
    {
        recordStarterKitEntry("Potions de soin de départ x" + std::to_string(starterHealingPotions));
    }

    for (int i = 0; i < starterDamagePotions; i++)
    {
        inventory.addConsumable(ConsumableCatalog::createBasicDamagePotion());
    }
    if (starterDamagePotions > 0)
    {
        recordStarterKitEntry("Potions de rage de départ x" + std::to_string(starterDamagePotions));
    }

    int starterGold = DifficultyRules::getStarterGold(difficulty);
    inventory.earnGold(starterGold);
    if (starterGold > 0)
    {
        recordStarterKitEntry("Or de départ : " + std::to_string(starterGold));
    }
}


std::vector<std::string> Player::applyHeavyVersionAdaptation(DifficultyMode difficulty)
{
    (void)difficulty;

    std::vector<std::string> changes;

    if (starterKitLog.empty())
    {
        starterKitLog.push_back("Kit de départ original inconnu : sauvegarde créée avant le journal de départ.");
        changes.push_back("Journal du kit de départ initialisé en mode sauvegarde ancienne.");
    }

    if (!inventoryHasWeaponNamed(inventory, "Mains nues"))
    {
        inventory.addWeapon(WeaponCatalog::createBareHands());
        changes.push_back("Mains nues restaurées comme base de secours.");
    }

    if (isDistanceStarterClass(type) && !inventoryHasWeaponNamed(inventory, "Couteau de bois d'urgence"))
    {
        inventory.addWeapon(WeaponCatalog::createEmergencyWoodKnife());
        changes.push_back("Couteau de bois d'urgence ajouté pour classe à distance.");
    }

    if (usesStarterAmmunition(type))
    {
        if ((type.find("Arbal") != std::string::npos || type.find("arbal") != std::string::npos)
            && !inventoryHasMaterialId(inventory, "training_bolts"))
        {
            inventory.addMaterial(MaterialCatalog::createById("training_bolts", 8));
            changes.push_back("Carreaux d'entraînement x8 ajoutés pour adaptation.");
        }
        else if ((type.find("dagues") != std::string::npos || type.find("Dagues") != std::string::npos)
            && !inventoryHasMaterialId(inventory, "training_throwing_knives"))
        {
            inventory.addMaterial(MaterialCatalog::createById("training_throwing_knives", 6));
            changes.push_back("Couteaux de lancer émoussés x6 ajoutés pour adaptation.");
        }
        else if (!inventoryHasMaterialId(inventory, "training_arrows"))
        {
            inventory.addMaterial(MaterialCatalog::createById("training_arrows", 10));
            changes.push_back("Flèches d'entraînement x10 ajoutées pour adaptation.");
        }
    }

    Armor expectedArmor = ArmorCatalog::createStarterArmorForClass(type);
    if (inventory.getArmorCount() == 0)
    {
        inventory.addArmor(ArmorCatalog::createSimpleOutfit());
        inventory.addArmor(expectedArmor);
        equipArmor(1);
        changes.push_back("Protection de classe ajoutée car aucune armure n'était présente.");
    }
    else if (equippedArmorIndex < 0 && !inventoryHasArmorNamed(inventory, expectedArmor.getName()))
    {
        inventory.addArmor(expectedArmor);
        changes.push_back("Protection de classe ajoutée sans remplacer l'équipement existant.");
    }

    if (inventory.getWeaponCount() <= 1)
    {
        Weapon expectedWeapon = WeaponCatalog::createStarterWeaponForClass(type);
        if (!inventoryHasWeaponNamed(inventory, expectedWeapon.getName()))
        {
            inventory.addWeapon(expectedWeapon);
            changes.push_back("Arme de classe basique ajoutée car l'arsenal était presque vide.");
        }
    }

    if (inventory.countConsumables(ConsumableType::Healing) == 0)
    {
        inventory.addConsumable(ConsumableCatalog::createBasicHealingPotion());
        changes.push_back("Potion de soin ajoutée pour éviter une ancienne sauvegarde sans sécurité.");
    }

    if (changes.empty())
    {
        changes.push_back("Aucun objet ajouté : l'équipement actuel semblait déjà compatible.");
    }

    for (const std::string& change : changes)
    {
        starterKitLog.push_back("Adaptation V" + VersionInfo::currentVersion() + " : " + change);
    }

    markAdaptedToCurrentVersion();
    return changes;
}

// EN: destroyEquippedWeapon declares or implements a focused behavior used by this module.
// FR: destroyEquippedWeapon déclare ou implémente un comportement précis utilisé par ce module.
bool Player::destroyEquippedWeapon()
{
    if (!hasEquippedWeapon())
    {
        return false;
    }

    int index = equippedWeaponIndex;
    equippedWeaponIndex = -1;

    return inventory.removeWeapon(index);
}

// EN: destroyEquippedArmor declares or implements a focused behavior used by this module.
// FR: destroyEquippedArmor déclare ou implémente un comportement précis utilisé par ce module.
bool Player::destroyEquippedArmor()
{
    if (!hasEquippedArmor())
    {
        return false;
    }

    Armor armor = getEquippedArmor();

    if (armor.getName() == "Tenue simple")
    {
        return false;
    }

    int index = equippedArmorIndex;
    unequipArmor();

    return inventory.removeArmor(index);
}

// EN: gainExperience declares or implements a focused behavior used by this module.
// FR: gainExperience déclare ou implémente un comportement précis utilisé par ce module.
