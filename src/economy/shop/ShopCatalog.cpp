// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Provides preview shop inventories until the real shop economy is connected.
// Français : Fournit des inventaires de boutique de prévisualisation jusqu'au branchement réel de l'économie.

#include "economy/shop/ShopCatalog.hpp"

std::vector<ShopType> ShopCatalog::getAvailableShopTypes()
{
    return {
        ShopType::MonsterMaterial,
        ShopType::Material,
        ShopType::Plant,
        ShopType::Armor,
        ShopType::Weapon,
        ShopType::Consumable,
        ShopType::Library
    };
}

ShopInventory ShopCatalog::createPreviewShop(ShopType type)
{
    ShopInventory shop(type, shopTypeToText(type));

    switch (type)
    {
        case ShopType::MonsterMaterial:
            shop.addItem(ShopItem("goblin_ear", "Oreille de gobelin", "Matériau commun récupéré sur certains gobelins.", ShopItemCategory::MonsterMaterial, 12, 4, 5));
            shop.addItem(ShopItem("wolf_fang", "Croc de loup", "Matériau utile pour de petites améliorations d'armes.", ShopItemCategory::MonsterMaterial, 18, 6, 4));
            break;

        case ShopType::Material:
            shop.addItem(ShopItem("rusted_metal_fragment", "Fragment de métal rouillé", "Reste d'équipement brisé, encore utile pour des réparations basiques.", ShopItemCategory::Material, 10, 3, 8));
            shop.addItem(ShopItem("worn_leather_piece", "Morceau de cuir abîmé", "Matière simple pour armures légères.", ShopItemCategory::Material, 14, 5, 6));
            break;

        case ShopType::Plant:
            shop.addItem(ShopItem("mountain_blue_flower", "Fleur bleue de montagne", "Une fleur rare et calme, souvent liée aux remèdes et aux secrets anciens.", ShopItemCategory::Plant, 35, 10, 2));
            shop.addItem(ShopItem("bitter_healing_leaf", "Feuille amère de soin", "Plante commune utilisée dans les potions simples.", ShopItemCategory::Plant, 15, 5, 6));
            break;

        case ShopType::Armor:
            shop.addItem(ShopItem("worn_leather_armor", "Armure en cuir usée", "Une protection légère qui a déjà vécu.", ShopItemCategory::Armor, 70, 22, 1));
            break;

        case ShopType::Weapon:
            shop.addItem(ShopItem("rusty_sword", "Épée rouillée", "Une arme modeste, mais mieux que les mains nues.", ShopItemCategory::Weapon, 60, 18, 1));
            break;

        case ShopType::Consumable:
            shop.addItem(ShopItem("basic_healing_potion", "Potion de soin", "Restaure une quantité simple de PV.", ShopItemCategory::Consumable, 25, 8, 5));
            shop.addItem(ShopItem("basic_damage_potion", "Potion de rage", "Renforce une attaque offensive.", ShopItemCategory::Consumable, 30, 10, 3));
            break;

        case ShopType::Library:
            shop.addItem(ShopItem("common_goblin_notes", "Notes communes sur les gobelins", "Débloquera plus tard des informations de bestiaire sur les gobelins.", ShopItemCategory::Information, 40, 0, 1, true));
            shop.addItem(ShopItem("basic_magic_manual", "Manuel de magie basique", "Prépare la future progression magique et les sorts de départ.", ShopItemCategory::Book, 80, 10, 1));
            break;

        case ShopType::Unknown:
        default:
            break;
    }

    return shop;
}

std::vector<ShopInventory> ShopCatalog::createAllPreviewShops()
{
    std::vector<ShopInventory> shops;

    for (ShopType type : getAvailableShopTypes())
    {
        shops.push_back(createPreviewShop(type));
    }

    return shops;
}
