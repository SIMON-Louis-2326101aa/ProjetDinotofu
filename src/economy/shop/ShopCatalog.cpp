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
        ShopType::Alchemist,
        ShopType::BlackMarket
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
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("worn_leather_piece", "Renforts de cuir usé", "Pas une vraie armure, mais parfait pour réparer ou renforcer du cuir.", ShopItemCategory::Material, 24, 5, 3));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("rusted_metal_fragment", "Plaques rouillées de récupération", "Plaques imparfaites pour bricoler une protection métallique.", ShopItemCategory::Material, 28, 3, 4));
            }
            if (rotation % 4 == 2)
            {
                shop.addItem(ShopItem("weak_repair_kit", "Kit de réparation d'armurier", "Petit kit gardé pour les clients qui cassent tout trop vite.", ShopItemCategory::Material, 58, 18, 1));
            }
            break;

        case ShopType::Weapon:
            shop.addItem(ShopItem("rusty_sword", "Épée rouillée", "Une arme modeste, mais mieux que les mains nues.", ShopItemCategory::Weapon, 60, 18, 1));
            shop.addItem(ShopItem("training_bow", "Arc d'entraînement", "Arme à distance simple. Les tirs demandent maintenant des munitions compatibles.", ShopItemCategory::Weapon, 85, 24, 1));
            if (rotation % 5 == 0)
            {
                shop.addItem(ShopItem("training_crossbow", "Arbalète d'occasion", "Une arbalète simple, pas luxueuse, mais légale.", ShopItemCategory::Weapon, 118, 30, 1));
            }
            if (rotation % 5 == 3)
            {
                shop.addItem(ShopItem("training_throwing_bandolier", "Bandoulière de lancer usée", "Matériel de jet de seconde main, encore utilisable.", ShopItemCategory::Weapon, 102, 26, 1));
            }
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("training_spear", "Lance d'entraînement", "Lance simple pour garder l'ennemi à distance sans prétendre être un héros.", ShopItemCategory::Weapon, 78, 22, 1));
                shop.addItem(ShopItem("training_dagger", "Dague d'entraînement", "Petite lame correcte pour classes rapides et défense de secours.", ShopItemCategory::Weapon, 68, 20, 1));
            }
            else
            {
                shop.addItem(ShopItem("training_crossbow", "Arbalète d'entraînement", "Arme à distance lente mais stable, compatible avec les carreaux.", ShopItemCategory::Weapon, 105, 30, 1));
                shop.addItem(ShopItem("training_throwing_bandolier", "Bandoulière de lancer", "Équipement de jet pour utiliser des couteaux de lancer.", ShopItemCategory::Weapon, 92, 26, 1));
            }
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("training_staff", "Bâton d'apprenti", "Bâton simple pour mages, voyageurs et combats prudents.", ShopItemCategory::Weapon, 72, 18, 1));
            }
            if (rotation % 4 == 2)
            {
                shop.addItem(ShopItem("heavy_training_axe", "Hache lourde émoussée", "Arme lourde de débutant, lente mais cohérente pour les briseurs.", ShopItemCategory::Weapon, 115, 32, 1));
            }
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("training_arrows", "Flèches d'entraînement", "Munitions basiques pour arcs.", ShopItemCategory::Material, 12, 2, 12));
            }
            else
            {
                shop.addItem(ShopItem("training_bolts", "Carreaux d'entraînement", "Munitions basiques pour arbalètes.", ShopItemCategory::Material, 14, 2, 10));
            }
            if (rotation % 3 == 2)
            {
                shop.addItem(ShopItem("training_throwing_knives", "Couteaux de lancer émoussés", "Munitions de jet à courte distance.", ShopItemCategory::Material, 18, 3, 6));
            }
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
            shop.addItem(ShopItem("minor_healing_potion", "Petite potion de soin", "Restaure peu de PV, mais coûte peu.", ShopItemCategory::Consumable, 12, 4, 4 + rotation % 3));
            shop.addItem(ShopItem("basic_healing_potion", "Potion de soin", "Restaure une quantité simple de PV.", ShopItemCategory::Consumable, 25, 8, 3 + rotation % 3));
            shop.addItem(ShopItem("basic_damage_potion", "Potion de rage", "Renforce une attaque offensive.", ShopItemCategory::Consumable, 30, 10, 2 + rotation % 2));
            shop.addItem(ShopItem("antidote_potion", "Antidote simple", "Petit stock permanent : les slimes violets et marais ne préviennent pas.", ShopItemCategory::Consumable, 42, 12, 1 + rotation % 2));
            if (rotation % 2 == 1)
            {
                shop.addItem(ShopItem("antidote_potion", "Antidote simple", "Potion utile contre les poisons faibles des marais et slimes violets.", ShopItemCategory::Consumable, 40, 12, 2));
            }
            if (rotation % 3 == 2)
            {
                shop.addItem(ShopItem("burn_salve_potion", "Baume anti-brûlure", "Soin de terrain contre les petites brûlures et attaques de feu.", ShopItemCategory::Consumable, 42, 12, 2));
            }
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("defensive_potion", "Potion défensive", "Buff défensif préparé pour le futur sous-menu Potions.", ShopItemCategory::Consumable, 45, 15, 2));
            }
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("minor_damage_potion", "Petite potion de rage", "Petit boost offensif de secours.", ShopItemCategory::Consumable, 15, 5, 2));
            }
            if (rotation % 4 == 1)
            {
                shop.addItem(ShopItem("reinforced_healing_potion", "Potion de soin renforcée", "Soin plus fiable, stock limité.", ShopItemCategory::Consumable, 60, 18, 1));
            }
            if (rotation % 4 == 0)
            {
                shop.addItem(ShopItem("training_arrows", "Flèches d'entraînement", "Lot de flèches pour ne pas tomber à sec.", ShopItemCategory::Material, 10, 2, 10));
            }
            if (rotation % 4 == 3)
            {
                shop.addItem(ShopItem("training_bolts", "Carreaux d'entraînement", "Petit lot de carreaux pour arbalète.", ShopItemCategory::Material, 12, 2, 8));
            }
            if (rotation % 5 == 3)
            {
                shop.addItem(ShopItem("precision_potion", "Potion de précision", "Buff futur pour tirs, dagues et gestes minutieux.", ShopItemCategory::Consumable, 70, 20, 1));
            }
            if (rotation % 6 == 4)
            {
                shop.addItem(ShopItem("barbed_arrows", "Flèches barbelées", "Munitions spéciales rares en boutique normale.", ShopItemCategory::Material, 45, 5, 3));
            }
            if (rotation % 6 == 5)
            {
                shop.addItem(ShopItem("piercing_bolts", "Carreaux perforants", "Munitions spéciales pour tirs lourds.", ShopItemCategory::Material, 52, 6, 2));
            }
            if (rotation % 7 == 2)
            {
                shop.addItem(ShopItem("ash_arrows", "Flèches de cendre", "Petit lot élémentaire, cher pour une boutique normale.", ShopItemCategory::Material, 72, 8, 2));
            }
            if (rotation % 7 == 4)
            {
                shop.addItem(ShopItem("frozen_bolts", "Carreaux givrés", "Munitions froides rarement proposées hors montagne.", ShopItemCategory::Material, 82, 9, 2));
            }
            if (rotation % 6 == 1)
            {
                shop.addItem(ShopItem("balanced_throwing_knives", "Couteaux de lancer équilibrés", "Petit lot utile aux profils mobiles qui veulent une option plus propre que les couteaux de base.", ShopItemCategory::Material, 64, 6, 3));
            }
            if (rotation % 8 == 3)
            {
                shop.addItem(ShopItem("venom_arrows", "Flèches enduites de venin", "Petit lot toxique très limité : légalement discutable, mais pas encore marché noir.", ShopItemCategory::Material, 96, 10, 2));
            }
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
            if (rotation % 5 == 2)
            {
                shop.addItem(ShopItem("advanced_monster_notes", "Dossier : couleurs de slimes", "Ajoute des rappels sur les variantes de slimes et leurs statuts possibles.", ShopItemCategory::Information, 90, 0, 1, true));
            }
            if (rotation % 5 == 3)
            {
                shop.addItem(ShopItem("advanced_monster_notes", "Dossier : soigneurs crédibles", "Explique pourquoi seuls certains profils peuvent soigner leurs alliés.", ShopItemCategory::Information, 88, 0, 1, true));
            }
            if (rotation % 6 == 0)
            {
                shop.addItem(ShopItem("slime_color_codex", "Codex des couleurs de slimes", "Règles de terrain sur les couleurs de slimes, surtout dans les mares gélatineuses.", ShopItemCategory::Information, 110, 0, 1, true));
            }
            if (rotation % 6 == 2)
            {
                shop.addItem(ShopItem("monster_family_evolution_notes", "Dossier des familles de monstres", "Variantes, évolutions, rôles et comportements crédibles par famille.", ShopItemCategory::Information, 125, 0, 1, true));
            }
            if (rotation % 6 == 4)
            {
                shop.addItem(ShopItem("weapon_training_notes", "Notes d'entraînement aux techniques", "Première couche de compétences de classe/arme débloquées par niveau.", ShopItemCategory::Information, 135, 0, 1, true));
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
            shop.addItem(ShopItem("minor_healing_potion", "Petite potion de soin", "Petit soin courant préparé sur place.", ShopItemCategory::Consumable, 13, 4, 5));
            shop.addItem(ShopItem("basic_healing_potion", "Potion de soin", "Restaure une quantité simple de PV.", ShopItemCategory::Consumable, 28, 8, 3));
            shop.addItem(ShopItem("basic_damage_potion", "Potion de rage", "Boost offensif simple, vendu par alchimiste prudent.", ShopItemCategory::Consumable, 34, 10, 2));
            shop.addItem(ShopItem("antidote_potion", "Antidote simple", "Réponse classique aux poisons faibles.", ShopItemCategory::Consumable, 42, 12, 3));
            shop.addItem(ShopItem("burn_salve_potion", "Baume anti-brûlure", "Le minimum vital quand un slime rouge ou une flèche de cendre passe par là.", ShopItemCategory::Consumable, 44, 12, 2));
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("burn_salve_potion", "Baume anti-brûlure", "Utile après une mauvaise rencontre avec du feu ou un slime rouge.", ShopItemCategory::Consumable, 44, 12, 2));
            }
            if (rotation % 3 == 2)
            {
                shop.addItem(ShopItem("frost_resistance_potion", "Potion tiède anti-givre", "Protection expérimentale contre le froid.", ShopItemCategory::Consumable, 82, 22, 1));
            }
            if (rotation % 4 == 3)
            {
                shop.addItem(ShopItem("shock_resistance_potion", "Potion isolante", "Protection chère contre les décharges, surtout avec équipement métallique.", ShopItemCategory::Consumable, 98, 24, 1));
            }
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("reinforced_healing_potion", "Potion de soin renforcée", "Potion plus chère mais beaucoup plus sûre.", ShopItemCategory::Consumable, 65, 18, 2));
            }
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("greater_healing_potion", "Potion de soin supérieure", "Soin avancé rarement en stock normal.", ShopItemCategory::Consumable, 115, 35, 1));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("weakening_debuff_potion", "Fiole d'affaiblissement", "Debuff offensif préparé pour les futurs lancers de potion.", ShopItemCategory::Consumable, 85, 25, 1));
            }
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

        case ShopType::BlackMarket:
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("experimental_damage_potion", "Potion de rage expérimentale", "Potion illégale, puissante et instable. Évidemment hors de prix.", ShopItemCategory::Consumable, 520, 90, 1));
                shop.addItem(ShopItem("smoke_escape_vial", "Fiole de fumée de secours", "Outil de fuite instable vendu sans garantie, parfait pour gens désespérés.", ShopItemCategory::Consumable, 310, 40, 1));
            }
            else
            {
                shop.addItem(ShopItem("major_healing_potion", "Potion de soin majeure non déclarée", "Soin très rare vendu sans provenance claire.", ShopItemCategory::Consumable, 440, 80, 1));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("greater_defensive_potion", "Potion défensive supérieure expérimentale", "Buff défensif rare, cher, et probablement pas légal.", ShopItemCategory::Consumable, 360, 65, 1));
            }
            if (rotation % 3 == 2)
            {
                shop.addItem(ShopItem("balanced_throwing_knives", "Couteaux de lancer équilibrés volés", "Munitions de jet spéciales, vendues trop cher et sans facture.", ShopItemCategory::Material, 165, 6, 3));
            }
            if (rotation % 4 == 0)
            {
                shop.addItem(ShopItem("barbed_arrows", "Flèches barbelées interdites", "Munitions spéciales de chasse vendues sous le manteau.", ShopItemCategory::Material, 130, 5, 5));
            }
            if (rotation % 4 == 2)
            {
                shop.addItem(ShopItem("piercing_bolts", "Carreaux perforants militaires", "Munitions lourdes difficiles à acheter légalement.", ShopItemCategory::Material, 155, 6, 4));
            }
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("unstable_core", "Lot interdit : noyau instable", "Matériau dangereux, vendu sous le manteau. Très cher, parfois d'une qualité impossible à garantir.", ShopItemCategory::Material, 420, 40, 1));
                shop.addItem(ShopItem("shadow_thread", "Fil d'ombre sans origine", "Composant sombre illégal. Les assassins et occultistes savent pourquoi il vaut ce prix.", ShopItemCategory::MonsterMaterial, 360, 32, 1));
            }
            else
            {
                shop.addItem(ShopItem("kitsune_ember", "Braise kitsune scellée", "Braise rare conservée dans un sceau douteux. Puissante, instable, et probablement volée.", ShopItemCategory::Material, 390, 42, 1));
                shop.addItem(ShopItem("draconic_scale_fragment", "Écaille draconique non déclarée", "Fragment très solide vendu trop cher par quelqu'un qui refuse de donner son nom.", ShopItemCategory::Material, 520, 45, 1));
            }
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("precision_harvest_tools", "Outils de prélèvement interdits", "Outils expérimentaux pour récupérer mieux, mais personne ne garantit leur origine.", ShopItemCategory::Material, 480, 95, 1));
            }
            if (rotation % 4 == 1)
            {
                shop.addItem(ShopItem("preservation_vials", "Fioles noires de conservation", "Fioles chères, parfaites pour préserver des composants qui ne devraient pas être transportés.", ShopItemCategory::Material, 560, 110, 1));
            }
            if (rotation % 5 == 2)
            {
                shop.addItem(ShopItem("anomaly_glitch_fragment", "Fragment glitché expérimental", "Fragment impossible à vendre légalement. Utilisable comme matériau, mais son comportement reste douteux.", ShopItemCategory::Material, 850, 0, 1));
            }
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("ash_arrows", "Flèches de cendre non déclarées", "Munitions élémentaires expérimentales. Trop chères, mais introuvables légalement en stock fiable.", ShopItemCategory::Material, 260, 8, 6));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("frozen_bolts", "Carreaux givrés de contrebande", "Carreaux traités par alchimie froide. Instables, chers, mais tentants.", ShopItemCategory::Material, 285, 9, 5));
            }
            if (rotation % 3 == 2)
            {
                shop.addItem(ShopItem("conductive_knives", "Couteaux conducteurs interdits", "Projectiles métalliques préparés pour futures réactions électriques.", ShopItemCategory::Material, 300, 9, 5));
            }
            if (rotation % 5 == 1)
            {
                shop.addItem(ShopItem("venom_arrows", "Flèches empoisonnées sans étiquette", "Munitions toxiques vendues trop cher par quelqu'un qui sourit trop.", ShopItemCategory::Material, 340, 10, 5));
            }
            if (rotation % 5 == 2)
            {
                shop.addItem(ShopItem("shock_bolts", "Carreaux conducteurs expérimentaux", "Très dangereux contre métal. Très cher. Très probablement volé.", ShopItemCategory::Material, 390, 11, 4));
            }
            if (rotation % 5 == 3)
            {
                shop.addItem(ShopItem("smoke_knives", "Couteaux fumigènes de fuite", "Pas faits pour tuer, mais parfaits pour survivre quand ça tourne mal.", ShopItemCategory::Material, 310, 10, 4));
            }
            if (rotation % 9 == 6)
            {
                shop.addItem(ShopItem("slime_color_codex", "Feuillet volé : slimes rares", "Renseignement de bibliothèque revendu illégalement, probablement recopié trop vite.", ShopItemCategory::Information, 240, 0, 1, true));
            }
            if (rotation % 9 == 7)
            {
                shop.addItem(ShopItem("weapon_training_notes", "Carnet d'entraînement volé", "Notes sales sur des techniques observées en arène. Trop cher, mais utile.", ShopItemCategory::Information, 280, 0, 1, true));
            }
            if (rotation % 4 == 3)
            {
                shop.addItem(ShopItem("training_crossbow", "Arbalète volée presque neuve", "Arme de contrebande : utilisable, chère, et vendue sans question.", ShopItemCategory::Weapon, 240, 30, 1));
            }
            if (rotation % 5 == 4)
            {
                shop.addItem(ShopItem("training_throwing_bandolier", "Bandoulière de lancer trafiquée", "Équipement de jet volé, utile aux profils furtifs ou désespérés.", ShopItemCategory::Weapon, 210, 26, 1));
            }
            if (rotation % 2 == 1)
            {
                shop.addItem(ShopItem("shock_resistance_potion", "Potion isolante volée", "Très utile si tu portes du métal et que la zone commence à grésiller.", ShopItemCategory::Consumable, 190, 24, 1));
                shop.addItem(ShopItem("frost_resistance_potion", "Potion anti-givre de contrebande", "Préparation froide mal étiquetée, mais efficace contre le givre.", ShopItemCategory::Consumable, 175, 22, 1));
            }
            if (rotation % 6 == 0)
            {
                shop.addItem(ShopItem("major_healing_potion", "Potion majeure sous scellé noir", "Trop rare pour une boutique normale, donc évidemment trop chère.", ShopItemCategory::Consumable, 620, 80, 1));
            }
            if (rotation % 7 == 3)
            {
                shop.addItem(ShopItem("tinkerer_complete_repair_kit", "Kit complet sans numéro de série", "Presque introuvable légalement. Le vendeur jure qu'il n'a rien volé.", ShopItemCategory::Material, 980, 160, 1));
            }
            if (rotation % 6 == 2)
            {
                shop.addItem(ShopItem("experimental_damage_potion", "Lot expérimental : rage instable", "Le vendeur refuse de promettre que la fiole ne te mordra pas en retour.", ShopItemCategory::Consumable, 690, 90, 1));
                shop.addItem(ShopItem("smoke_escape_vial", "Fiole de fuite trafiquée", "Très chère pour un plan B, mais certains survivants paient sans négocier.", ShopItemCategory::Consumable, 430, 40, 1));
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
