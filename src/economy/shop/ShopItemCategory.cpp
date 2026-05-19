// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Converts shop item categories into French player-facing labels.
// Français : Convertit les catégories d'articles en libellés français affichables au joueur.

#include "economy/shop/ShopItemCategory.hpp"

const char* shopItemCategoryToText(ShopItemCategory category)
{
    switch (category)
    {
        case ShopItemCategory::Weapon:
            return "Arme";

        case ShopItemCategory::Armor:
            return "Armure";

        case ShopItemCategory::Consumable:
            return "Consommable";

        case ShopItemCategory::MonsterMaterial:
            return "Matériau de monstre";

        case ShopItemCategory::Material:
            return "Matériau";

        case ShopItemCategory::Plant:
            return "Plante";

        case ShopItemCategory::Book:
            return "Livre";

        case ShopItemCategory::Spell:
            return "Magie";

        case ShopItemCategory::Information:
            return "Renseignement";

        case ShopItemCategory::Unknown:
        default:
            return "Inconnu";
    }
}
