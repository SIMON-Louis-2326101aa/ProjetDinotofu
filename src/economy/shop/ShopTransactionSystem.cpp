// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Implements shop purchases and safe resale rules.
// Français : Implémente les achats de boutique et les règles de revente sécurisée.

#include "economy/shop/ShopTransactionSystem.hpp"

#include "economy/shop/ShopPriceRules.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"

#include <iostream>

namespace
{
    bool isMaterialShop(ShopType type)
    {
        return type == ShopType::MonsterMaterial
            || type == ShopType::Material
            || type == ShopType::Plant
            || type == ShopType::Library;
    }
}

bool ShopTransactionSystem::canBeBoughtNow(const ShopItem& item)
{
    const std::string id = item.getId();

    return id == "basic_healing_potion"
        || id == "basic_damage_potion"
        || id == "rusty_sword"
        || id == "worn_leather_armor"
        || id == "goblin_ear"
        || id == "wolf_fang"
        || id == "rusted_metal_fragment"
        || id == "worn_leather_piece"
        || id == "mountain_blue_flower"
        || id == "bitter_healing_leaf"
        || id == "common_goblin_notes"
        || id == "common_wolf_notes"
        || id == "basic_plant_manual"
        || id == "basic_magic_manual";
}

bool ShopTransactionSystem::buyItem(
    Player& player,
    const ShopItem& item,
    int finalPrice
)
{
    if (!canBeBoughtNow(item))
    {
        displayUnsupportedPurchaseMessage(item);
        return false;
    }

    if (!player.getInventory().spendGold(finalPrice))
    {
        std::cout << "Tu n'as pas assez d'or pour acheter "
                  << item.getName()
                  << "."
                  << std::endl;
        std::cout << "Or disponible : "
                  << player.getInventory().getGold()
                  << " pièces."
                  << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (item.getId() == "basic_healing_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createBasicHealingPotion());
    }
    else if (item.getId() == "basic_damage_potion")
    {
        player.getInventory().addConsumable(ConsumableCatalog::createBasicDamagePotion());
    }
    else if (item.getId() == "rusty_sword")
    {
        player.getInventory().addWeapon(WeaponCatalog::createRustySword());
    }
    else if (item.getId() == "worn_leather_armor")
    {
        player.getInventory().addArmor(ArmorCatalog::createWornLeatherArmor());
    }
    else
    {
        player.getInventory().addMaterial(MaterialCatalog::createById(item.getId(), 1));
    }

    std::cout << "Achat réussi : " << item.getName() << "." << std::endl;
    std::cout << "Or restant : "
              << player.getInventory().getGold()
              << " pièces."
              << std::endl;

    if (item.isCommonInformation())
    {
        BestiaryRuntimeProgress::unlockCommonInformation(item.getId());
        std::cout << "Renseignement ajouté au bestiaire pour cette session." << std::endl;
    }

    std::cout << std::endl;

    return true;
}

int ShopTransactionSystem::getSellableEntryCount(
    const Player& player,
    ShopType shopType
)
{
    if (shopType == ShopType::Weapon)
    {
        return player.getInventory().getWeaponCount();
    }

    if (shopType == ShopType::Armor)
    {
        return player.getInventory().getArmorCount();
    }

    if (shopType == ShopType::Consumable)
    {
        return player.getInventory().getConsumableCount();
    }

    if (isMaterialShop(shopType))
    {
        return static_cast<int>(player.getInventory().getMaterials().size());
    }

    return 0;
}

bool ShopTransactionSystem::canShopBuyInventoryEntry(
    const Player& player,
    ShopType shopType,
    int index
)
{
    if (shopType == ShopType::Weapon)
    {
        return player.getInventory().hasWeapon(index)
            && index != player.getEquippedWeaponIndex()
            && player.getInventory().getWeapon(index).getName() != "Mains nues";
    }

    if (shopType == ShopType::Armor)
    {
        return player.getInventory().hasArmor(index)
            && index != player.getEquippedArmorIndex()
            && player.getInventory().getArmor(index).getName() != "Tenue simple";
    }

    if (shopType == ShopType::Consumable)
    {
        return player.getInventory().hasConsumable(index);
    }

    if (isMaterialShop(shopType))
    {
        return player.getInventory().hasMaterial(index);
    }

    return false;
}

int ShopTransactionSystem::getSellPriceForEntry(
    const Player& player,
    ShopType shopType,
    int index
)
{
    int basePrice = 0;

    if (shopType == ShopType::Weapon && player.getInventory().hasWeapon(index))
    {
        basePrice = player.getInventory().getWeapon(index).getValue() / 3;
    }
    else if (shopType == ShopType::Armor && player.getInventory().hasArmor(index))
    {
        basePrice = player.getInventory().getArmor(index).getValue() / 3;
    }
    else if (shopType == ShopType::Consumable && player.getInventory().hasConsumable(index))
    {
        basePrice = player.getInventory().getConsumable(index).getValue() / 3;
    }
    else if (isMaterialShop(shopType) && player.getInventory().hasMaterial(index))
    {
        basePrice = player.getInventory().getMaterial(index).getValue();
    }

    if (basePrice < 1)
    {
        basePrice = 1;
    }

    return ShopPriceRules::applySellModifier(basePrice, player.getRaceText());
}

bool ShopTransactionSystem::sellInventoryEntry(
    Player& player,
    ShopType shopType,
    int index,
    int finalSellPrice
)
{
    if (!canShopBuyInventoryEntry(player, shopType, index))
    {
        std::cout << "Impossible de vendre cette entrée." << std::endl;
        std::cout << "L'équipement porté, les objets de base et les entrées invalides sont protégés." << std::endl;
        std::cout << std::endl;
        return false;
    }

    std::string soldName = "Objet";
    bool removed = false;

    if (shopType == ShopType::Weapon)
    {
        soldName = player.getInventory().getWeapon(index).getName();
        removed = player.getInventory().removeWeapon(index);
    }
    else if (shopType == ShopType::Armor)
    {
        soldName = player.getInventory().getArmor(index).getName();
        removed = player.getInventory().removeArmor(index);
    }
    else if (shopType == ShopType::Consumable)
    {
        soldName = player.getInventory().getConsumable(index).getName();
        removed = player.getInventory().removeConsumable(index);
    }
    else if (isMaterialShop(shopType))
    {
        soldName = player.getInventory().getMaterial(index).getName();
        removed = player.getInventory().removeMaterialQuantity(index, 1);
    }

    if (!removed)
    {
        std::cout << "La vente a échoué. Rien n'a été perdu." << std::endl;
        std::cout << std::endl;
        return false;
    }

    player.getInventory().earnGold(finalSellPrice);

    std::cout << "Vente réussie : " << soldName << "." << std::endl;
    std::cout << "+" << finalSellPrice << " pièces d'or." << std::endl;
    std::cout << "Or actuel : " << player.getInventory().getGold() << " pièces." << std::endl;
    std::cout << std::endl;

    return true;
}

void ShopTransactionSystem::displaySellableEntries(
    const Player& player,
    ShopType shopType
)
{
    int count = getSellableEntryCount(player, shopType);

    if (count <= 0)
    {
        std::cout << "Rien à vendre ici pour le moment." << std::endl;
        return;
    }

    for (int i = 0; i < count; ++i)
    {
        std::cout << i << " : ";

        if (shopType == ShopType::Weapon)
        {
            std::cout << player.getInventory().getWeapon(i).getName();
        }
        else if (shopType == ShopType::Armor)
        {
            std::cout << player.getInventory().getArmor(i).getName();
        }
        else if (shopType == ShopType::Consumable)
        {
            std::cout << player.getInventory().getConsumable(i).getName();
        }
        else if (isMaterialShop(shopType))
        {
            Material material = player.getInventory().getMaterial(i);
            std::cout << material.getName() << " x" << material.getQuantity();
        }

        if (!canShopBuyInventoryEntry(player, shopType, i))
        {
            std::cout << " | Protégé";
        }
        else
        {
            std::cout << " | Revente : "
                      << getSellPriceForEntry(player, shopType, i)
                      << " or";
        }

        std::cout << std::endl;
    }
}

void ShopTransactionSystem::displayUnsupportedPurchaseMessage(const ShopItem& item)
{
    std::cout << item.getName() << " existe dans la boutique," << std::endl;
    std::cout << "mais son stockage réel n'est pas encore branché dans l'inventaire." << std::endl;
    std::cout << "Aucun or n'a été dépensé." << std::endl;
    std::cout << std::endl;
}
