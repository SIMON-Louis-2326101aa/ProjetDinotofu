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
        ),
        ShopItem(
            "special_identity_rumors",
            "Dossier de rumeurs spéciales",
            "Confirme quelques personnages spéciaux sans ouvrir toutes leurs fiches gratuitement.",
            ShopItemCategory::Information,
            145,
            0,
            1,
            true
        ),
        ShopItem(
            "legend_child_tales",
            "Contes et légendes pour enfant",
            "Ajoute un premier rayonnage de légendes optionnelles, dont l’origine des Bras cassés.",
            ShopItemCategory::Information,
            75,
            0,
            1,
            true
        ),
        ShopItem(
            "legend_trigger_notes",
            "Notes de conteur : légendes rares",
            "Explique comment certaines légendes peuvent apparaître par PNJ, bibliothèque ou salle sans devenir obligatoires.",
            ShopItemCategory::Information,
            90,
            0,
            1,
            true
        ),
        ShopItem(
            "legend_storyteller_routes",
            "Carnet des conteurs itinérants",
            "Ajoute des rumeurs de bibliothèque et de taverne sur la manière dont le lore peut apparaître sans bloquer l'action.",
            ShopItemCategory::Information,
            105,
            0,
            1,
            true
        ),
        ShopItem(
            "curse_counter_rites_notes",
            "Notes sur les rites anti-malédiction",
            "Ouvre des archives sur les objets sources, serments, contre-légendes et patients maudits sans révéler d'effets exacts.",
            ShopItemCategory::Information,
            135,
            0,
            1,
            true
        ),
        ShopItem("language_goblin_primer", "Initiation : Gobelin", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en gobelin.", ShopItemCategory::Information, 55, 0, 1, true),
        ShopItem("language_goblin_course", "Cours avancé : Gobelin", "Dossier plus complet : conversation, demandes de quête et formulations courantes en gobelin.", ShopItemCategory::Information, 120, 0, 1, true),
        ShopItem("language_orc_primer", "Initiation : Orc", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en orc.", ShopItemCategory::Information, 65, 0, 1, true),
        ShopItem("language_orc_course", "Cours avancé : Orc", "Dossier plus complet : conversation, demandes de quête et formulations courantes en orc.", ShopItemCategory::Information, 135, 0, 1, true),
        ShopItem("language_infernal_primer", "Initiation : Infernal", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en infernal.", ShopItemCategory::Information, 90, 0, 1, true),
        ShopItem("language_infernal_course", "Cours avancé : Infernal", "Dossier plus complet : conversation, demandes de quête et formulations courantes en infernal.", ShopItemCategory::Information, 185, 0, 1, true),
        ShopItem("language_draconic_primer", "Initiation : Draconique", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en draconique.", ShopItemCategory::Information, 100, 0, 1, true),
        ShopItem("language_draconic_course", "Cours avancé : Draconique", "Dossier plus complet : conversation, demandes de quête et formulations courantes en draconique.", ShopItemCategory::Information, 210, 0, 1, true),
        ShopItem("language_elven_primer", "Initiation : Elfique", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en elfique.", ShopItemCategory::Information, 70, 0, 1, true),
        ShopItem("language_elven_course", "Cours avancé : Elfique", "Dossier plus complet : conversation, demandes de quête et formulations courantes en elfique.", ShopItemCategory::Information, 145, 0, 1, true),
        ShopItem("language_dark_elven_primer", "Initiation : Elfique noir", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en elfique noir.", ShopItemCategory::Information, 85, 0, 1, true),
        ShopItem("language_dark_elven_course", "Cours avancé : Elfique noir", "Dossier plus complet : conversation, demandes de quête et formulations courantes en elfique noir.", ShopItemCategory::Information, 170, 0, 1, true),
        ShopItem("language_celestial_primer", "Initiation : Céleste", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en céleste.", ShopItemCategory::Information, 95, 0, 1, true),
        ShopItem("language_celestial_course", "Cours avancé : Céleste", "Dossier plus complet : conversation, demandes de quête et formulations courantes en céleste.", ShopItemCategory::Information, 195, 0, 1, true),
        ShopItem("language_fey_primer", "Initiation : Féerique", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en féerique.", ShopItemCategory::Information, 80, 0, 1, true),
        ShopItem("language_fey_course", "Cours avancé : Féerique", "Dossier plus complet : conversation, demandes de quête et formulations courantes en féerique.", ShopItemCategory::Information, 165, 0, 1, true),
        ShopItem("language_kitsune_primer", "Initiation : Kitsune", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en kitsune.", ShopItemCategory::Information, 80, 0, 1, true),
        ShopItem("language_kitsune_course", "Cours avancé : Kitsune", "Dossier plus complet : conversation, demandes de quête et formulations courantes en kitsune.", ShopItemCategory::Information, 165, 0, 1, true),
        ShopItem("language_dwarven_primer", "Initiation : Nain", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en nain.", ShopItemCategory::Information, 65, 0, 1, true),
        ShopItem("language_dwarven_course", "Cours avancé : Nain", "Dossier plus complet : conversation, demandes de quête et formulations courantes en nain.", ShopItemCategory::Information, 135, 0, 1, true),
        ShopItem("language_gnomish_primer", "Initiation : Gnome", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en gnome.", ShopItemCategory::Information, 60, 0, 1, true),
        ShopItem("language_gnomish_course", "Cours avancé : Gnome", "Dossier plus complet : conversation, demandes de quête et formulations courantes en gnome.", ShopItemCategory::Information, 125, 0, 1, true),
        ShopItem("language_halfling_primer", "Initiation : Halfelin", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en halfelin.", ShopItemCategory::Information, 55, 0, 1, true),
        ShopItem("language_halfling_course", "Cours avancé : Halfelin", "Dossier plus complet : conversation, demandes de quête et formulations courantes en halfelin.", ShopItemCategory::Information, 115, 0, 1, true),
        ShopItem("language_vampiric_primer", "Initiation : Vampirique", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en vampirique.", ShopItemCategory::Information, 95, 0, 1, true),
        ShopItem("language_vampiric_course", "Cours avancé : Vampirique", "Dossier plus complet : conversation, demandes de quête et formulations courantes en vampirique.", ShopItemCategory::Information, 195, 0, 1, true),
        ShopItem("language_spirit_primer", "Initiation : Spirituel", "Manuel de bibliothèque : alphabet, salutations, avertissements et vocabulaire de terrain en spirituel.", ShopItemCategory::Information, 100, 0, 1, true),
        ShopItem("language_spirit_course", "Cours avancé : Spirituel", "Dossier plus complet : conversation, demandes de quête et formulations courantes en spirituel.", ShopItemCategory::Information, 205, 0, 1, true),
        ShopItem("language_anomaly_notation", "Notation des anomalies", "Feuillets dangereux : permettent seulement de reconnaître quelques motifs anormaux, jamais de parler réellement cette non-langue.", ShopItemCategory::Information, 240, 0, 1, true)
    };
}
