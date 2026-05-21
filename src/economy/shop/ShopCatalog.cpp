// EN: ShopCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ShopCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Provides preview shop inventories until the real shop economy is connected.
// Français : Fournit des inventaires de boutique de prévisualisation jusqu'au branchement réel de l'économie.

#include "economy/shop/ShopCatalog.hpp"
#include "economy/shop/ShopRotationSystem.hpp"

std::vector<ShopType> ShopCatalog::getAvailableShopTypes()
{
    return {
        ShopType::MonsterMaterial,
        ShopType::Material,
        ShopType::Plant,
        ShopType::Armor,
        ShopType::Weapon,
        ShopType::Consumable,
        ShopType::Library,
        ShopType::Blacksmith,
        ShopType::Alchemist
    };
}

// EN: createPreviewShop declares or implements a focused behavior used by this module.
// FR: createPreviewShop déclare ou implémente un comportement précis utilisé par ce module.
ShopInventory ShopCatalog::createPreviewShop(ShopType type)
{
    ShopInventory shop(type, shopTypeToText(type));
    int rotation = ShopRotationSystem::getRotationIndex();

    switch (type)
    {
        case ShopType::MonsterMaterial:
            shop.addItem(ShopItem("goblin_ear", "Oreille de gobelin", "Matériau commun récupéré sur certains gobelins.", ShopItemCategory::MonsterMaterial, 12, 4, 3 + rotation % 4));
            shop.addItem(ShopItem("wolf_fang", "Croc de loup", "Matériau utile pour de petites améliorations d'armes.", ShopItemCategory::MonsterMaterial, 18, 6, 2 + rotation % 3));
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("cracked_bone", "Os fissuré", "Matériau sombre lié aux morts-vivants et à la future nécromancie.", ShopItemCategory::MonsterMaterial, 22, 7, 2));
            }
            if (rotation % 3 == 2)
            {
                shop.addItem(ShopItem("slime_residue", "Résidu de slime", "Matière collante utile pour potions, pièges et réparations de fortune.", ShopItemCategory::MonsterMaterial, 11, 4, 3));
            }
            if (rotation % 4 == 0)
            {
                shop.addItem(ShopItem("battle_torn_badge", "Insigne abîmé d'aventurier", "Trophée utile pour les futures guildes et contrats.", ShopItemCategory::MonsterMaterial, 34, 15, 1));
            }
            if (rotation % 5 == 2)
            {
                shop.addItem(ShopItem("beast_hide", "Peau de bête robuste", "Matériau plus solide pour armures, réparation épaisse et futur craft tank.", ShopItemCategory::MonsterMaterial, 44, 16, 1));
            }
            if (rotation % 6 == 3)
            {
                shop.addItem(ShopItem("shadow_thread", "Fil d'ombre", "Composant sombre très rare, lié aux assassins et aux futures ombres.", ShopItemCategory::MonsterMaterial, 95, 32, 1));
            }
            if (rotation % 5 == 4)
            {
                shop.addItem(ShopItem("monster_dissection_guide", "Technique passive : dissection de monstre", "Apprentissage rare vendu par un spécialiste des composants de monstres.", ShopItemCategory::Information, 180, 0, 1, true));
            }
            break;

        case ShopType::Material:
            shop.addItem(ShopItem("rusted_metal_fragment", "Fragment de métal rouillé", "Reste d'équipement brisé, encore utile pour des réparations basiques.", ShopItemCategory::Material, 10, 3, 5 + rotation % 5));
            shop.addItem(ShopItem("worn_leather_piece", "Morceau de cuir abîmé", "Matière simple pour armures légères.", ShopItemCategory::Material, 14, 5, 4 + rotation % 4));
            if (rotation % 2 == 1)
            {
                shop.addItem(ShopItem("weak_repair_kit", "Kit de réparation faible", "Outil fragile : +25% max, durabilité faible.", ShopItemCategory::Material, 50, 18, 1));
            }
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("slime_residue", "Résidu de slime", "Liant utile pour fabriquer des kits de réparation.", ShopItemCategory::Material, 13, 4, 2));
            }
            if (rotation % 5 == 0)
            {
                shop.addItem(ShopItem("medium_repair_kit", "Kit de réparation moyen", "Outil correct : +50% max, durabilité limitée.", ShopItemCategory::Material, 130, 35, 1));
            }
            if (rotation % 7 == 0)
            {
                shop.addItem(ShopItem("big_repair_kit", "Gros kit de réparation", "Outil cher : +75% max, durabilité correcte.", ShopItemCategory::Material, 275, 90, 1));
            }
            if (rotation % 11 == 0)
            {
                shop.addItem(ShopItem("tinkerer_complete_repair_kit", "Kit complet du bricoleur", "Outil très cher : +95% max, presque une forge portable.", ShopItemCategory::Material, 520, 160, 1));
            }
            if (rotation % 4 == 1)
            {
                shop.addItem(ShopItem("precision_harvest_tools", "Outils de récupération précise", "Outils coûteux qui améliorent légèrement la qualité des ressources récupérées.", ShopItemCategory::Material, 210, 95, 1));
            }
            break;

        case ShopType::Plant:
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("mountain_blue_flower", "Fleur bleue de montagne", "Une fleur rare et calme, souvent liée aux remèdes et aux secrets anciens.", ShopItemCategory::Plant, 35, 10, 1 + rotation % 2));
            }
            shop.addItem(ShopItem("bitter_healing_leaf", "Feuille amère de soin", "Plante commune utilisée dans les potions simples.", ShopItemCategory::Plant, 15, 5, 4 + rotation % 4));
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("clean_harvest_manual", "Technique passive : récolte propre", "Petit manuel qui aide à récupérer plantes et matériaux sans les abîmer.", ShopItemCategory::Information, 120, 0, 1, true));
            }
            break;

        case ShopType::Armor:
            shop.addItem(ShopItem("worn_leather_armor", "Armure en cuir usée", "Une protection légère qui a déjà vécu.", ShopItemCategory::Armor, 70, 22, 1));
            break;

        case ShopType::Weapon:
            shop.addItem(ShopItem("rusty_sword", "Épée rouillée", "Une arme modeste, mais mieux que les mains nues.", ShopItemCategory::Weapon, 60, 18, 1));
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("arcane_dust", "Poussière arcanique", "Composant magique acheté ici quand le forgeron expérimente avec l'enchantement.", ShopItemCategory::Material, 38, 12, 1));
            }
            if (rotation % 8 == 1)
            {
                shop.addItem(ShopItem("draconic_scale_fragment", "Fragment d'écaille draconique", "Matériau très rare pour protections futures.", ShopItemCategory::Material, 180, 45, 1));
            }
            break;

        case ShopType::Consumable:
            shop.addItem(ShopItem("basic_healing_potion", "Potion de soin", "Restaure une quantité simple de PV.", ShopItemCategory::Consumable, 25, 8, 5));
            shop.addItem(ShopItem("basic_damage_potion", "Potion de rage", "Renforce une attaque offensive.", ShopItemCategory::Consumable, 30, 10, 3));
            if (rotation % 4 == 2)
            {
                shop.addItem(ShopItem("unstable_core", "Noyau instable", "Composant dangereux pour alchimie et invocations futures.", ShopItemCategory::Material, 120, 40, 1));
            }
            if (rotation % 5 == 0)
            {
                shop.addItem(ShopItem("preservation_vials", "Fioles de conservation", "Outil de terrain pour mieux préserver liquides, braises et résidus.", ShopItemCategory::Material, 240, 110, 1));
            }
            break;

        case ShopType::Library:
            shop.addItem(ShopItem("common_goblin_notes", "Notes communes sur les gobelins", "Débloque des informations de bestiaire sur les gobelins.", ShopItemCategory::Information, 40, 0, 1, true));
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("common_wolf_notes", "Notes communes sur les loups", "Débloque des informations de bestiaire sur les bêtes communes.", ShopItemCategory::Information, 35, 0, 1, true));
            }
            else
            {
                shop.addItem(ShopItem("basic_plant_manual", "Petit guide des plantes communes", "Débloque des informations de bestiaire sur les plantes basiques.", ShopItemCategory::Information, 45, 0, 1, true));
            }
            shop.addItem(ShopItem("basic_magic_manual", "Manuel de magie basique", "Prépare la future progression magique et les sorts de départ.", ShopItemCategory::Book, 80, 10, 1, true));
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("special_adventurer_notes", "Notes sur les groupes spéciaux", "Débloque des notes de bestiaire sur les groupes de personnages spéciaux.", ShopItemCategory::Information, 95, 0, 1, true));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("summoning_notes", "Manuel d'invocations instables", "Ajoute des notes sur les invocations, les slots et les ombres futures.", ShopItemCategory::Book, 90, 12, 1, true));
            }
            if (rotation % 3 == 2)
            {
                shop.addItem(ShopItem("boss_identity_scrap", "Fragment d'identité de boss", "Rappelle la règle lore : un boss doit révéler son nom lui-même.", ShopItemCategory::Information, 120, 0, 1, true));
            }
            if (rotation % 4 == 0)
            {
                shop.addItem(ShopItem("potion_recipe_page", "Page de recettes d'alchimie", "Débloque des rappels de recettes de potions dans le bestiaire.", ShopItemCategory::Book, 65, 14, 1, true));
            }
            if (rotation % 4 == 1)
            {
                shop.addItem(ShopItem("repair_recipe_page", "Page de réparation de fortune", "Explique comment assembler un kit de réparation simple.", ShopItemCategory::Book, 60, 14, 1, true));
            }
            if (rotation % 4 == 2)
            {
                shop.addItem(ShopItem("advanced_monster_notes", "Notes avancées sur les monstres", "Ajoute des détails sur les loots et comportements dangereux.", ShopItemCategory::Information, 85, 0, 1, true));
            }
            if (rotation % 4 == 3)
            {
                shop.addItem(ShopItem("necromancy_warning", "Avertissement nécromantique", "Prépare les futures ombres de Hazak sans révéler tout le système.", ShopItemCategory::Information, 100, 0, 1, true));
            }
            if (rotation % 5 == 0)
            {
                shop.addItem(ShopItem("monster_dissection_guide", "Technique passive : dissection de monstre", "Guide de terrain pour récupérer les composants sans les massacrer.", ShopItemCategory::Information, 165, 0, 1, true));
            }
            if (rotation % 5 == 1)
            {
                shop.addItem(ShopItem("clean_harvest_manual", "Technique passive : récolte propre", "Méthode simple pour récolter avant de découper n'importe comment.", ShopItemCategory::Information, 115, 0, 1, true));
            }
            break;


        case ShopType::Blacksmith:
            shop.addItem(ShopItem("rusted_metal_fragment", "Fragment de métal rouillé", "Le forgeron revend des restes de métal pour les réparations d'urgence.", ShopItemCategory::Material, 14, 4, 6 + rotation % 4));
            shop.addItem(ShopItem("worn_leather_piece", "Morceau de cuir abîmé", "Utile pour rafistoler les armures légères.", ShopItemCategory::Material, 18, 5, 4 + rotation % 3));
            shop.addItem(ShopItem("weak_repair_kit", "Kit de réparation faible", "Outil fragile : +25% max, durabilité faible.", ShopItemCategory::Material, 65, 18, 2));
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("medium_repair_kit", "Kit de réparation moyen", "Outil correct : +50% max, durabilité limitée.", ShopItemCategory::Material, 155, 35, 1));
            }
            if (rotation % 4 == 0)
            {
                shop.addItem(ShopItem("big_repair_kit", "Gros kit de réparation", "Outil cher : +75% max, demandé par les aventuriers prudents.", ShopItemCategory::Material, 320, 90, 1));
            }
            if (rotation % 6 == 0)
            {
                shop.addItem(ShopItem("draconic_scale_fragment", "Fragment d'écaille draconique", "Matériau très solide, rare chez un forgeron classique.", ShopItemCategory::Material, 210, 45, 1));
            }
            if (rotation % 5 == 1)
            {
                shop.addItem(ShopItem("precision_harvest_tools", "Outils de récupération précise", "Outils chers mais utiles pour abîmer moins de composants.", ShopItemCategory::Material, 225, 95, 1));
            }
            break;

        case ShopType::Alchemist:
            shop.addItem(ShopItem("basic_healing_potion", "Potion de soin", "Restaure une quantité simple de PV.", ShopItemCategory::Consumable, 28, 8, 4));
            shop.addItem(ShopItem("bitter_healing_leaf", "Feuille amère de soin", "Base de potions simples.", ShopItemCategory::Plant, 17, 5, 5 + rotation % 3));
            shop.addItem(ShopItem("slime_residue", "Résidu de slime", "Liant instable pour potions et kits.", ShopItemCategory::MonsterMaterial, 15, 4, 3));
            if (rotation % 2 == 1)
            {
                shop.addItem(ShopItem("mountain_blue_flower", "Fleur bleue de montagne", "Plante rare pour soin renforcé.", ShopItemCategory::Plant, 42, 10, 1));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("arcane_dust", "Poussière arcanique", "Catalyseur pour recettes avancées.", ShopItemCategory::Material, 45, 12, 2));
            }
            if (rotation % 5 == 2)
            {
                shop.addItem(ShopItem("unstable_core", "Noyau instable", "Composant dangereux, réservé aux expérimentations coûteuses.", ShopItemCategory::Material, 145, 40, 1));
            }
            if (rotation % 6 == 3)
            {
                shop.addItem(ShopItem("kitsune_ember", "Braise kitsune", "Braise magique rare pour alchimie avancée.", ShopItemCategory::Material, 165, 42, 1));
            }
            if (rotation % 4 == 2)
            {
                shop.addItem(ShopItem("preservation_vials", "Fioles de conservation", "Outil cher pour limiter les pertes sur les composants fragiles.", ShopItemCategory::Material, 260, 110, 1));
            }
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
