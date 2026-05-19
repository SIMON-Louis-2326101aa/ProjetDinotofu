// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Defines item categories that can be sold or bought in shops.
// Français : Définit les catégories d'objets pouvant être vendus ou achetés en boutique.

#ifndef INCLUDE_ECONOMY_SHOP_SHOPITEMCATEGORY_HPP
#define INCLUDE_ECONOMY_SHOP_SHOPITEMCATEGORY_HPP

enum class ShopItemCategory
{
    Weapon,
    Armor,
    Consumable,
    MonsterMaterial,
    Material,
    Plant,
    Book,
    Spell,
    Information,
    Unknown
};

const char* shopItemCategoryToText(ShopItemCategory category);

#endif
