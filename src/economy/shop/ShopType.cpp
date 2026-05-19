// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Converts shop types into French player-facing labels.
// Français : Convertit les types de boutiques en libellés français affichables au joueur.

#include "economy/shop/ShopType.hpp"

const char* shopTypeToText(ShopType type)
{
    switch (type)
    {
        case ShopType::MonsterMaterial:
            return "Boutique de monstres";

        case ShopType::Material:
            return "Boutique de matériaux";

        case ShopType::Plant:
            return "Herboristerie";

        case ShopType::Armor:
            return "Armurerie défensive";

        case ShopType::Weapon:
            return "Forge d'armes";

        case ShopType::Consumable:
            return "Boutique de consommables";

        case ShopType::Library:
            return "Bibliothèque";

        case ShopType::Unknown:
        default:
            return "Boutique inconnue";
    }
}
