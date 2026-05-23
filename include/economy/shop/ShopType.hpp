// EN: ShopType.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopType.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Defines every planned shop family used by the economy system.
// Français : Définit chaque famille de boutique prévue pour le système économique.

#ifndef INCLUDE_ECONOMY_SHOP_SHOPTYPE_HPP
#define INCLUDE_ECONOMY_SHOP_SHOPTYPE_HPP

enum class ShopType
{
    MonsterMaterial,
    Material,
    Plant,
    Armor,
    Weapon,
    Consumable,
    Library,
    Blacksmith,
    Alchemist,
    BlackMarket,
    Unknown
};

// EN: shopTypeToText declares or implements a focused behavior used by this module.
// FR: shopTypeToText déclare ou implémente un comportement précis utilisé par ce module.
const char* shopTypeToText(ShopType type);

#endif
