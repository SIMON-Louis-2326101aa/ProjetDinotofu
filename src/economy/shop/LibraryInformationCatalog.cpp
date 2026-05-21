// EN: LibraryInformationCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: LibraryInformationCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Provides early purchasable information offers for common bestiary entries.
// Français : Fournit les premiers renseignements achetables pour les entrées communes du bestiaire.

#include "economy/shop/LibraryInformationCatalog.hpp"

std::vector<ShopItem> LibraryInformationCatalog::createCommonInformationOffers()
{
    return {
        ShopItem(
            "common_goblin_notes",
            "Notes communes sur les gobelins",
            "Prépare le futur achat d'informations sur les gobelins communs.",
            ShopItemCategory::Information,
            40,
            0,
            1,
            true
        ),
        ShopItem(
            "common_wolf_notes",
            "Notes communes sur les loups",
            "Prépare le futur achat d'informations sur les bêtes communes.",
            ShopItemCategory::Information,
            35,
            0,
            1,
            true
        ),
        ShopItem(
            "basic_plant_manual",
            "Petit guide des plantes communes",
            "Prépare les futures informations sur les plantes basiques.",
            ShopItemCategory::Information,
            45,
            0,
            1,
            true
        )
    };
}
