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
            "Rassemble des observations simples sur les gobelins communs.",
            ShopItemCategory::Information,
            40,
            0,
            1,
            true
        ),
        ShopItem(
            "common_wolf_notes",
            "Notes communes sur les loups",
            "Rassemble des observations simples sur les bêtes communes.",
            ShopItemCategory::Information,
            35,
            0,
            1,
            true
        ),
        ShopItem(
            "basic_plant_manual",
            "Petit guide des plantes communes",
            "Rassemble des observations simples sur les plantes basiques.",
            ShopItemCategory::Information,
            45,
            0,
            1,
            true
        ),
        ShopItem(
            "class_identity_manual",
            "Manuel des styles de classe",
            "Explique pourquoi deux classes ne doivent pas se jouer pareil : posture, arme, risque et rôle.",
            ShopItemCategory::Information,
            85,
            0,
            1,
            true
        ),
        ShopItem(
            "biome_field_notes",
            "Carnet de terrain des biomes",
            "Ajoute des notes sur les zones, les évolutions de niveau et les rencontres locales.",
            ShopItemCategory::Information,
            95,
            0,
            1,
            true
        )
,
        ShopItem(
            "slime_color_codex",
            "Codex des couleurs de slimes",
            "Rassemble les règles de base des couleurs de slimes et de leurs comportements.",
            ShopItemCategory::Information,
            110,
            0,
            1,
            true
        ),
        ShopItem(
            "monster_family_evolution_notes",
            "Dossier des familles de monstres",
            "Explique les variantes, évolutions et rôles crédibles de plusieurs familles de monstres.",
            ShopItemCategory::Information,
            125,
            0,
            1,
            true
        ),
        ShopItem(
            "weapon_training_notes",
            "Notes d'entraînement aux techniques",
            "Décrit comment les techniques naissent du niveau, de l'arme et de l'expérimentation.",
            ShopItemCategory::Information,
            135,
            0,
            1,
            true
        )    };
}
