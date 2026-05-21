// EN: ShopItemCategory.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopItemCategory.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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

// EN: shopItemCategoryToText declares or implements a focused behavior used by this module.
// FR: shopItemCategoryToText déclare ou implémente un comportement précis utilisé par ce module.
const char* shopItemCategoryToText(ShopItemCategory category);

#endif
