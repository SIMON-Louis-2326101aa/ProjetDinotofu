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
        ShopType::Enchanter,
        ShopType::CityService,
        ShopType::Lodging,
        ShopType::Transport,
        ShopType::Church,
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
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("predator_fang", "Croc de prédateur", "Croc de bête carnassière non lupine.", ShopItemCategory::MonsterMaterial, 20, 7, 1 + rotation % 2));
            }
            if (rotation % 4 == 1)
            {
                shop.addItem(ShopItem("boar_tusk", "Défense de sanglier", "Défense dure issue d'une bête à charge.", ShopItemCategory::MonsterMaterial, 20, 7, 1));
            }
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("rodent_tooth", "Dent de rongeur", "Petite dent utile aux artisans précis.", ShopItemCategory::MonsterMaterial, 9, 3, 2 + rotation % 2));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("cracked_bone", "Os fissuré", "Matériau sombre lié aux morts-vivants et aux rites nécromantiques.", ShopItemCategory::MonsterMaterial, 22, 7, 2));
            }
            if (rotation % 3 == 2)
            {
                shop.addItem(ShopItem("slime_residue", "Résidu de slime", "Matière collante utile pour potions, pièges et réparations de fortune.", ShopItemCategory::MonsterMaterial, 11, 4, 3));
            }
            if (rotation % 4 == 0)
            {
                shop.addItem(ShopItem("battle_torn_badge", "Insigne abîmé d'aventurier", "Trophée utile pour les guildes et les contrats douteux.", ShopItemCategory::MonsterMaterial, 34, 15, 1));
            }
            if (rotation % 5 == 2)
            {
                shop.addItem(ShopItem("beast_hide", "Peau de bête robuste", "Matériau plus solide pour armures lourdes et réparations épaisses.", ShopItemCategory::MonsterMaterial, 44, 16, 1));
            }
            if (rotation % 6 == 3)
            {
                shop.addItem(ShopItem("shadow_thread", "Fil d'ombre", "Composant sombre très rare, lié aux assassins et aux ombres.", ShopItemCategory::MonsterMaterial, 95, 32, 1));
            }
            if (rotation % 5 == 4)
            {
                shop.addItem(ShopItem("monster_dissection_guide", "Technique passive : dissection de monstre", "Apprentissage rare vendu par un spécialiste des composants de monstres.", ShopItemCategory::Information, 180, 0, 1, true));
            }
            break;

        case ShopType::Material:
            shop.addItem(ShopItem("rusted_metal_fragment", "Fragment de métal rouillé", "Reste d'équipement brisé, encore utile pour des réparations basiques.", ShopItemCategory::Material, 10, 3, 5 + rotation % 5));
            shop.addItem(ShopItem("worn_leather_piece", "Morceau de cuir abîmé", "Matière simple pour armures légères.", ShopItemCategory::Material, 14, 5, 4 + rotation % 4));
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("runic_iron_shard", "Éclat de fer runique", "Métal de palier intermédiaire pour runes simples et armes stables.", ShopItemCategory::Material, 62, 24, 2 + rotation % 2));
            }
            if (rotation % 4 == 2)
            {
                shop.addItem(ShopItem("polished_scale_plate", "Plaque d'écailles polies", "Matériau de monstre préparé pour harnais et protections semi-lourdes.", ShopItemCategory::Material, 74, 30, 2));
            }
            if (rotation % 5 == 3)
            {
                shop.addItem(ShopItem("amber_tempering_oil", "Huile de trempe ambrée", "Huile d'atelier pour stabiliser des effets latents sans passer en relique.", ShopItemCategory::Material, 82, 34, 1));
            }
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("sun_dried_clay", "Argile rouge séchée", "Matériau sec pour plaques, sceaux et réparations propres.", ShopItemCategory::Material, 42, 16, 2 + rotation % 2));
                shop.addItem(ShopItem("cold_iron_nail", "Clou de fer froid", "Pièce de mine lourde utile aux mécanismes, armures et pièges.", ShopItemCategory::Material, 78, 31, 1 + rotation % 2));
            }
            if (rotation % 3 == 2)
            {
                shop.addItem(ShopItem("tiny_gear_spring", "Petit ressort d'engrenage", "Pièce mécanique pour arbalètes, pièges et objets bricolés.", ShopItemCategory::Material, 70, 27, 2));
            }
            if (rotation % 4 == 1)
            {
                shop.addItem(ShopItem("forgotten_camp_tag", "Plaque de camp oubliée", "Renseignement matériel récupéré sur de vieux relais de guilde.", ShopItemCategory::Material, 32, 12, 2));
            }
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
            if (rotation % 5 == 1)
            {
                shop.addItem(ShopItem("cracked_bell_clapper", "Battant de cloche fissuré", "Bronze sacré récupéré dans un vieux temple visitable.", ShopItemCategory::Material, 92, 37, 1));
                shop.addItem(ShopItem("white_bone_chalk", "Craie d'os blanc", "Minéral rituel utile aux cercles, archives et contrats de mages.", ShopItemCategory::Material, 64, 24, 2));
            }
            if (rotation % 5 == 3)
            {
                shop.addItem(ShopItem("weeping_stone_tear", "Larme de pierre pleureuse", "Minéral étrange pour statues, autels et réparations décoratives.", ShopItemCategory::Material, 84, 33, 1));
                shop.addItem(ShopItem("smuggler_token", "Jeton de contrebandier", "Jeton discret récupéré sous les ponts, utile aux quêtes de marché douteux.", ShopItemCategory::Material, 78, 30, 1));
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
            if (rotation % 2 == 1)
            {
                shop.addItem(ShopItem("mycelium_lantern", "Lanterne de mycélium", "Champignon-lampe pour potions nocturnes et encres vivantes.", ShopItemCategory::Plant, 45, 18, 2));
            }
            if (rotation % 3 == 2)
            {
                shop.addItem(ShopItem("living_vine_fiber", "Fibre de vigne vivante", "Fibre réactive pour robes, liens et réparations végétales.", ShopItemCategory::Plant, 66, 28, 1));
            }
            if (rotation % 4 == 3)
            {
                shop.addItem(ShopItem("echoing_resin", "Résine d'écho", "Résine sonore utile aux arcs, bâtons et pièges.", ShopItemCategory::Plant, 64, 26, 1));
            }
            if (rotation % 5 == 0)
            {
                shop.addItem(ShopItem("cliff_basil_leaf", "Feuille de basilic des falaises", "Plante salée pour tisanes, vertige et potions d'exploration.", ShopItemCategory::Plant, 58, 24, 2));
            }
            if (rotation % 6 == 4)
            {
                shop.addItem(ShopItem("luminous_moth_wing", "Aile de mite lumineuse", "Composant fragile pour encres, lanternes et fioles de garde-lucioles.", ShopItemCategory::Plant, 66, 26, 1));
            }
            if (rotation % 5 == 2)
            {
                shop.addItem(ShopItem("blue_mist_reed", "Roseau de brume bleue", "Plante aquatique pour potions de souffle, discrétion et exploration humide.", ShopItemCategory::Plant, 72, 27, 2));
            }
            if (rotation % 7 == 4)
            {
                shop.addItem(ShopItem("petrified_rose_petals", "Pétales de rose pétrifiée", "Relique florale étrange, utile aux hommages et aux sorts de pierre douce.", ShopItemCategory::Plant, 122, 49, 1));
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
            if (rotation % 2 == 1)
            {
                shop.addItem(ShopItem("reinforced_leather_armor", "Armure en cuir renforcée", "Premier vrai cran de protection mobile après le cuir usé.", ShopItemCategory::Armor, 155, 48, 1));
            }
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("guard_chainmail", "Cotte de maille de garde", "Protection de milice robuste, plus chère mais beaucoup plus rassurante.", ShopItemCategory::Armor, 225, 72, 1));
            }
            if (rotation % 4 == 1)
            {
                shop.addItem(ShopItem("runed_apprentice_robe", "Robe runique d'apprenti", "Tenue magique renforcée, utile aux mages qui prennent quand même des baffes.", ShopItemCategory::Armor, 190, 60, 1));
            }
            if (rotation % 5 == 2)
            {
                shop.addItem(ShopItem("crude_plate_armor", "Armure de plaques grossière", "Armure lourde bas-moyenne : efficace, bruyante et pas encore noble.", ShopItemCategory::Armor, 285, 92, 1));
            }
            if (rotation % 4 == 3)
            {
                shop.addItem(ShopItem("traveler_scale_vest", "Veste d'écailles de voyage", "Protection mobile intermédiaire, entre cuir renforcé et maille lourde.", ShopItemCategory::Armor, 235, 74, 1));
            }
            if (rotation % 5 == 4)
            {
                shop.addItem(ShopItem("threaded_rune_robe", "Robe à fils runiques", "Robe magique stable pour mages qui commencent à sortir des vrais sentiers.", ShopItemCategory::Armor, 245, 78, 1));
            }
            if (rotation % 6 == 5)
            {
                shop.addItem(ShopItem("militia_half_plate", "Demi-plaques de milice", "Protection lourde intermédiaire, moins noble qu'une plaque complète.", ShopItemCategory::Armor, 335, 108, 1));
            }
            if (rotation % 5 == 2)
            {
                shop.addItem(ShopItem("runic_chainmail", "Cotte runique de garde", "Maille renforcée avec une rune simple contre les chocs faibles.", ShopItemCategory::Armor, 430, 188, 1));
            }
            if (rotation % 7 == 3)
            {
                shop.addItem(ShopItem("shadow_thread_coat", "Manteau cousu d'ombre", "Tenue légère rare : discrétion, esquive et résistance magique correcte.", ShopItemCategory::Armor, 390, 160, 1));
            }
            if (rotation % 8 == 4)
            {
                shop.addItem(ShopItem("polished_scale_harness", "Harnais d'écailles polies", "Armure semi-lourde de monstre, solide mais plus exigeante à entretenir.", ShopItemCategory::Armor, 500, 210, 1));
            }
            if (rotation % 4 == 0)
            {
                shop.addItem(ShopItem("damaged_cartographer_coat", "Manteau de cartographe abîmé", "Tenue mobile pour exploration, discrétion et routes incertaines.", ShopItemCategory::Armor, 270, 84, 1));
                shop.addItem(ShopItem("cold_survival_parka", "Parka de survie glaciale", "Tenue équipée à la place d'une armure classique pour biomes froids.", ShopItemCategory::Armor, 320, 120, 1));
                shop.addItem(ShopItem("heat_survival_suit", "Tenue ignifugée de terrain", "Tenue équipée à la place d'une armure classique pour chaleur et feu faible.", ShopItemCategory::Armor, 350, 130, 1));
                if (rotation % 2 == 0)
                {
                    shop.addItem(ShopItem("insulated_explorer_coat", "Manteau isolant d'explorateur", "Tenue polyvalente contre températures pénibles, moins spécialisée mais très pratique.", ShopItemCategory::Armor, 405, 150, 1));
                }
            }
            if (rotation % 5 == 1)
            {
                shop.addItem(ShopItem("sun_dried_clay_breastplate", "Plastron d'argile séchée", "Protection semi-rigide, utile en terrain sec et routes poussiéreuses.", ShopItemCategory::Armor, 350, 112, 1));
            }
            if (rotation % 6 == 2)
            {
                shop.addItem(ShopItem("living_fiber_robe", "Robe aux fibres vivantes", "Robe magique souple qui supporte mieux les petites erreurs de canalisation.", ShopItemCategory::Armor, 385, 124, 1));
            }
            if (rotation % 7 == 5)
            {
                shop.addItem(ShopItem("whistling_miner_harness", "Harnais de mineur sifflant", "Protection lourde de mine : bruyante, solide et très utilitaire.", ShopItemCategory::Armor, 430, 145, 1));
            }
            if (rotation % 6 == 0)
            {
                shop.addItem(ShopItem("drowned_archivist_vest", "Gilet d'archiviste noyé", "Armure légère utile aux quêtes de paperasse, archives et sorties humides.", ShopItemCategory::Armor, 390, 130, 1));
            }
            if (rotation % 8 == 2)
            {
                shop.addItem(ShopItem("grey_drake_harness", "Harnais de drake gris", "Protection semi-lourde issue des falaises et des drakes gris.", ShopItemCategory::Armor, 540, 220, 1));
            }
            if (rotation % 7 == 1)
            {
                shop.addItem(ShopItem("patchwork_carnival_cape", "Cape de foire décousue", "Cape mobile à perles miroir, plus étrange que noble.", ShopItemCategory::Armor, 350, 112, 1));
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

            if (rotation % 2 == 1)
            {
                shop.addItem(ShopItem("iron_sword", "Épée de fer simple", "Arme de palier bas-moyen : plus fiable que l'équipement de départ.", ShopItemCategory::Weapon, 185, 58, 1));
                shop.addItem(ShopItem("reinforced_dagger", "Dague renforcée", "Dague plus nette pour assassins, duellistes et profils mobiles.", ShopItemCategory::Weapon, 165, 52, 1));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("guard_spear", "Lance de garde", "Lance de milice robuste, premier vrai cran au-dessus de l'entraînement.", ShopItemCategory::Weapon, 195, 60, 1));
                shop.addItem(ShopItem("hunting_bow", "Arc de chasse", "Arc plus stable, intéressant si les munitions sont gérées.", ShopItemCategory::Weapon, 205, 65, 1));
            }
            if (rotation % 4 == 1)
            {
                shop.addItem(ShopItem("apprentice_staff", "Bâton canalisateur", "Bâton renforcé pour mage ou voyageur prudent.", ShopItemCategory::Weapon, 175, 55, 1));
            }
            if (rotation % 5 == 2)
            {
                shop.addItem(ShopItem("heavy_iron_axe", "Hache lourde de fer", "Arme lourde plus sérieuse, rare en boutique normale.", ShopItemCategory::Weapon, 235, 72, 1));
            }
            if (rotation % 4 == 0)
            {
                shop.addItem(ShopItem("workshop_hammer", "Marteau d'atelier renforcé", "Premier marteau sérieux pour briseurs, artisans trop énervés et tanks offensifs.", ShopItemCategory::Weapon, 215, 66, 1));
            }
            if (rotation % 5 == 1)
            {
                shop.addItem(ShopItem("patrol_crossbow", "Arbalète de patrouille", "Arbalète plus fiable pour les routes dangereuses.", ShopItemCategory::Weapon, 225, 70, 1));
            }
            if (rotation % 6 == 3)
            {
                shop.addItem(ShopItem("balanced_rapier", "Rapière équilibrée", "Lame fine pour duelistes précis, rare chez les forgerons simples.", ShopItemCategory::Weapon, 205, 64, 1));
            }
            if (rotation % 4 == 3)
            {
                shop.addItem(ShopItem("mercenary_sabre", "Sabre de mercenaire", "Lame de route plus sérieuse, mais encore raisonnable pour l'économie bas-moyenne.", ShopItemCategory::Weapon, 265, 84, 1));
            }
            if (rotation % 5 == 4)
            {
                shop.addItem(ShopItem("curved_ambush_dagger", "Dague courbe d'embuscade", "Dague rapide pour profils mobiles qui veulent un vrai cran au-dessus.", ShopItemCategory::Weapon, 235, 76, 1));
            }
            if (rotation % 6 == 2)
            {
                shop.addItem(ShopItem("militia_longbow", "Arc long de milice", "Arc fiable pour tenir une ligne avec des munitions correctes.", ShopItemCategory::Weapon, 275, 88, 1));
            }
            if (rotation % 5 == 0)
            {
                shop.addItem(ShopItem("bound_oak_staff", "Bâton de chêne lié", "Catalyseur bas-moyen pour mages, plus durable qu'un bâton d'apprenti.", ShopItemCategory::Weapon, 255, 82, 1));
            }
            if (rotation % 5 == 1)
            {
                shop.addItem(ShopItem("runic_iron_blade", "Lame de fer runique", "Palier intermédiaire : dégâts réguliers et effet latent de brise-garde léger.", ShopItemCategory::Weapon, 420, 178, 1));
            }
            if (rotation % 6 == 4)
            {
                shop.addItem(ShopItem("amber_edge_dagger", "Dague d'ambre vive", "Dague rapide avec effet latent de saignement faible sur cible fragilisée.", ShopItemCategory::Weapon, 370, 152, 1));
            }
            if (rotation % 7 == 5)
            {
                shop.addItem(ShopItem("ashen_longbow", "Arc long cendré", "Arc stable pour munitions avancées et amorces élémentaires légères.", ShopItemCategory::Weapon, 395, 165, 1));
            }
            if (rotation % 8 == 6)
            {
                shop.addItem(ShopItem("channeling_scepter", "Sceptre canalisateur", "Catalyseur bas-moyen pour limiter l'instabilité des petits sorts.", ShopItemCategory::Weapon, 460, 190, 1));
            }
            if (rotation % 4 == 2)
            {
                shop.addItem(ShopItem("relay_falchion", "Fauchon de relais", "Lame de route solide, pensée pour protéger les relais et couloirs étroits.", ShopItemCategory::Weapon, 335, 112, 1));
            }
            if (rotation % 5 == 3)
            {
                shop.addItem(ShopItem("red_clay_sabre", "Sabre d'argile rouge", "Sabre fiable en terrain sec, renforcé par une couche d'argile cuite.", ShopItemCategory::Weapon, 390, 138, 1));
            }
            if (rotation % 6 == 1)
            {
                shop.addItem(ShopItem("broken_map_dagger", "Dague de carte brisée", "Dague d'éclaireur gravée à partir d'un fragment cartographique.", ShopItemCategory::Weapon, 370, 130, 1));
            }
            if (rotation % 7 == 4)
            {
                shop.addItem(ShopItem("singing_resin_staff", "Bâton de résine chantante", "Catalyseur végétal pour sorts simples et gestes de soutien.", ShopItemCategory::Weapon, 470, 172, 1));
            }
            if (rotation % 8 == 5)
            {
                shop.addItem(ShopItem("cold_lantern_bow", "Arc des lanternes froides", "Arc nocturne renforcé à la résine d'écho.", ShopItemCategory::Weapon, 440, 158, 1));
            }
            if (rotation % 9 == 7)
            {
                shop.addItem(ShopItem("whistling_mine_hammer", "Marteau de mine sifflante", "Marteau vibrant, brutal contre constructions et armures mal préparées.", ShopItemCategory::Weapon, 520, 190, 1));
            }
            if (rotation % 6 == 0)
            {
                shop.addItem(ShopItem("firefly_iron_rapier", "Rapière des lucioles de fer", "Lame fine et lumineuse pour duellistes prudents.", ShopItemCategory::Weapon, 535, 210, 1));
            }
            if (rotation % 7 == 2)
            {
                shop.addItem(ShopItem("drowned_ledger_mace", "Masse du registre noyé", "Arme lourde qui transforme la paperasse en percussion administrative.", ShopItemCategory::Weapon, 590, 235, 1));
            }
            if (rotation % 8 == 3)
            {
                shop.addItem(ShopItem("grey_cliff_spear", "Lance des falaises grises", "Lance stable renforcée par des écailles de drake gris.", ShopItemCategory::Weapon, 555, 218, 1));
            }
            if (rotation % 9 == 4)
            {
                shop.addItem(ShopItem("broken_carnival_whip", "Fouet de foire cassée", "Arme souple de diversion, dangereuse surtout quand la cible panique.", ShopItemCategory::Weapon, 505, 198, 1));
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
                shop.addItem(ShopItem("draconic_scale_fragment", "Fragment d'écaille draconique", "Matériau très rare pour protections anciennes.", ShopItemCategory::Material, 180, 45, 1));
            }
            break;

        case ShopType::Consumable:
            shop.addItem(ShopItem("survival_ration", "Ration de survie", "Ration simple : abordable, mais pas donnée, pour continuer une sortie longue sans rentrer.", ShopItemCategory::Material, 16, 7, 3));
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("thermal_survival_blanket", "Couverture de survie thermique", "Protection consommable contre les sorties froides ou glacées.", ShopItemCategory::Material, 42, 15, 1));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("cooling_survival_wrap", "Voile anti-chaleur", "Protection consommable contre chaleur et braises.", ShopItemCategory::Material, 46, 16, 1));
            }
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
                shop.addItem(ShopItem("defensive_potion", "Potion défensive", "Potion défensive instable, utile avant de prendre un mauvais coup.", ShopItemCategory::Consumable, 45, 15, 2));
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
                shop.addItem(ShopItem("precision_potion", "Potion de précision", "Buff de précision pour tirs, dagues et gestes minutieux.", ShopItemCategory::Consumable, 70, 20, 1));
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
                shop.addItem(ShopItem("unstable_core", "Noyau instable", "Composant dangereux pour alchimie et invocations instables.", ShopItemCategory::Material, 120, 40, 1));
            }
            if (rotation % 5 == 0)
            {
                shop.addItem(ShopItem("preservation_vials", "Fioles de conservation", "Outil de terrain pour mieux préserver liquides, braises et résidus.", ShopItemCategory::Material, 240, 110, 1));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("stabilizing_tea", "Thé stabilisant", "Infusion calme pour petites blessures et longues sorties.", ShopItemCategory::Consumable, 58, 16, 2));
            }
            if (rotation % 4 == 2)
            {
                shop.addItem(ShopItem("miner_bracing_tonic", "Tonique de mineur", "Buff robuste pour encaisser un mauvais choc.", ShopItemCategory::Consumable, 98, 28, 1));
            }
            if (rotation % 5 == 4)
            {
                shop.addItem(ShopItem("moon_salt_purifier", "Purificateur au sel lunaire", "Soin et nettoyage faible d'altérations, stock limité.", ShopItemCategory::Consumable, 145, 40, 1));
            }
            if (rotation % 4 == 0 || rotation % 4 == 3)
            {
                shop.addItem(ShopItem("vitality_healing_potion", "Potion de vitalité proportionnelle", "Potion de soin en pourcentage : récupère 35% des PV max, très utile sur les profils robustes.", ShopItemCategory::Consumable, 165, 42, 1));
            }
            if (rotation % 7 == 5)
            {
                shop.addItem(ShopItem("royal_vitality_healing_potion", "Potion de vitalité royale", "Potion rare en pourcentage : récupère 60% des PV max. Chère, mais monstrueuse sur un tank.", ShopItemCategory::Consumable, 360, 95, 1));
            }
            if (rotation % 5 == 2)
            {
                shop.addItem(ShopItem("witch_bottle", "Bouteille de sorcière vide", "Outil alchimique pour mélanges de terrain et futures recettes.", ShopItemCategory::Material, 92, 38, 1));
            }
            if (rotation % 6 == 0)
            {
                shop.addItem(ShopItem("cliff_basil_tea", "Tisane de basilic des falaises", "Soin léger et stabilisation pour longues sorties.", ShopItemCategory::Consumable, 104, 34, 1));
            }
            if (rotation % 6 == 2)
            {
                shop.addItem(ShopItem("firefly_guard_vial", "Fiole de garde-lucioles", "Buff lumineux court pour défense et exploration nocturne.", ShopItemCategory::Consumable, 148, 48, 1));
            }
            if (rotation % 7 == 3)
            {
                shop.addItem(ShopItem("carnival_diversion_ticket", "Ticket de diversion de foire", "Objet spécial pour détourner l'attention ou préparer une fuite.", ShopItemCategory::Consumable, 168, 50, 1));
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
            shop.addItem(ShopItem("basic_magic_manual", "Manuel de magie basique", "Introduit les bases de la progression magique et des sorts simples.", ShopItemCategory::Book, 80, 10, 1, true));
            if (rotation % 4 == 1)
            {
                shop.addItem(ShopItem("magic_learning_notes", "Notes sur supports magiques", "Explique pourquoi certains sorts existent en grimoire, en parchemin, ou seulement dans l'un des deux.", ShopItemCategory::Information, 52, 0, 1, true));
            }
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("arcane_spark_scroll", "Parchemin d'étincelle arcanique", "Sort scellé à usage unique : même un non-mage peut le déclencher, mais le papier sera perdu.", ShopItemCategory::Spell, 110, 0, 2));
                shop.addItem(ShopItem("wandering_ember_scroll", "Parchemin de braise errante", "Parchemin sans grimoire stable équivalent : puissant pour son prix, mais plus capricieux.", ShopItemCategory::Spell, 140, 0, 1));
                shop.addItem(ShopItem("minor_purification_scroll", "Parchemin de purification mineure", "Magie jetable : utile aux mages comme aux non-mages quand une altération devient trop lourde.", ShopItemCategory::Spell, 125, 0, 1));
                shop.addItem(ShopItem("crawling_venom_scroll", "Parchemin de venin rampant", "Sort jetable sans grimoire stable courant : empoisonne et fatigue la cible avant de brûler avec le papier.", ShopItemCategory::Spell, 150, 0, 1));
                shop.addItem(ShopItem("glass_step_scroll", "Parchemin de pas de verre", "Sort jetable d'évitement bref, utile quand l'engagement devient mauvais.", ShopItemCategory::Spell, 180, 0, 1));
                shop.addItem(ShopItem("vine_snare_scroll", "Parchemin de ronce-lien", "Sort jetable végétal qui gêne une cible mobile.", ShopItemCategory::Spell, 170, 0, 1));
                shop.addItem(ShopItem("archivist_focus_ink", "Encre de concentration d'archiviste", "Fiole de buff pour lecture, précision, rapport et petits calculs.", ShopItemCategory::Consumable, 118, 32, 1));
            }
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("arcane_binding_grimoire", "Grimoire d'entrave arcanique", "Apprentissage magique limité aux vrais profils capables de canaliser.", ShopItemCategory::Book, 165, 20, 1));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("elemental_ward_scroll", "Parchemin de voile élémentaire", "Protection magique à usage unique, chère mais accessible hors classe.", ShopItemCategory::Spell, 155, 0, 1));
                shop.addItem(ShopItem("elemental_ward_grimoire", "Grimoire du voile élémentaire", "Version étudiable du voile, réservée aux mages assez stables.", ShopItemCategory::Book, 210, 25, 1));
                shop.addItem(ShopItem("frost_needle_grimoire", "Grimoire d'aiguille de givre", "Sort apprenable sans parchemin courant : précis, froid, et réservé aux vrais canalisateurs.", ShopItemCategory::Book, 260, 28, 1));
            }
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("special_adventurer_notes", "Notes sur les groupes spéciaux", "Débloque des notes de bestiaire sur les groupes de personnages spéciaux.", ShopItemCategory::Information, 95, 0, 1, true));
            }
            if (rotation % 5 == 2)
            {
                shop.addItem(ShopItem("special_identity_rumors", "Dossier de rumeurs spéciales", "Confirme quelques personnages spéciaux sans offrir toutes leurs fiches gratuitement.", ShopItemCategory::Information, 145, 0, 1, true));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("summoning_notes", "Manuel d'invocations instables", "Ajoute des notes sur les invocations, les places actives et les ombres anciennes.", ShopItemCategory::Book, 90, 12, 1, true));
            }
            if (rotation % 3 == 2)
            {
                shop.addItem(ShopItem("boss_identity_scrap", "Fragment d'identité de boss", "Rappelle la règle lore : un boss doit révéler son nom lui-même.", ShopItemCategory::Information, 120, 0, 1, true));
                shop.addItem(ShopItem("elemental_weakness_notes", "Notes sur faiblesses élémentaires", "Récapitulatif de terrain sur résistances, faiblesses, races, classes et équipement.", ShopItemCategory::Information, 70, 0, 1, true));
                shop.addItem(ShopItem("mana_suture_grimoire", "Grimoire de suture de mana", "Sort apprenable sans parchemin courant : soin lent, exigeant et impossible à stabiliser sur papier simple.", ShopItemCategory::Book, 310, 34, 1));
                shop.addItem(ShopItem("occult_bramble_grimoire", "Grimoire des ronces occultes", "Sort apprenable sans parchemin courant : contrôle végétal sombre, venin et entrave légère.", ShopItemCategory::Book, 285, 32, 1));
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
                shop.addItem(ShopItem("necromancy_warning", "Avertissement nécromantique", "Évoque les ombres de Hazak sans révéler tout le rituel.", ShopItemCategory::Information, 100, 0, 1, true));
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
            if (rotation % 7 == 3)
            {
                shop.addItem(ShopItem("legend_child_tales", "Contes et légendes pour enfant", "Ajoute quelques rappels de lore léger, dont l’origine des Bras cassés.", ShopItemCategory::Information, 75, 0, 1, true));
            }
            if (rotation % 7 == 5)
            {
                shop.addItem(ShopItem("legend_trigger_notes", "Notes de conteur : légendes rares", "Explique les déclencheurs de légendes sans spoiler les salles de boss.", ShopItemCategory::Information, 90, 0, 1, true));
            }
            if (rotation % 8 == 6)
            {
                shop.addItem(ShopItem("legend_storyteller_routes", "Carnet des conteurs itinérants", "Ajoute des rumeurs de bibliothèque et de taverne pour le futur lore PNJ.", ShopItemCategory::Information, 105, 0, 1, true));
            }
            if (rotation % 9 == 2)
            {
                shop.addItem(ShopItem("sanctuary_wax_seal", "Sceau de cire sanctuaire", "Document sacré utile aux futures légendes de temple et aux quêtes administratives nobles.", ShopItemCategory::Information, 116, 0, 1, true));
            }
            if (rotation % 9 == 5)
            {
                shop.addItem(ShopItem("sealed_debt_slip", "Billet de dette scellé", "Document marchand à classer proprement avant de savoir qui doit quoi.", ShopItemCategory::Information, 132, 0, 1, true));
            }
            break;


        case ShopType::Blacksmith:
            shop.addItem(ShopItem("rusted_metal_fragment", "Fragment de métal rouillé", "Le forgeron revend des restes de métal pour les réparations d'urgence.", ShopItemCategory::Material, 14, 4, 6 + rotation % 4));
            shop.addItem(ShopItem("worn_leather_piece", "Morceau de cuir abîmé", "Utile pour rafistoler les armures légères.", ShopItemCategory::Material, 18, 5, 4 + rotation % 3));
            shop.addItem(ShopItem("runic_iron_shard", "Éclat de fer runique", "Composant d'atelier pour renforcer lame, maille ou catalyseur avant les vrais objets héroïques.", ShopItemCategory::Material, 70, 24, 2 + rotation % 2));
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("amber_tempering_oil", "Huile de trempe ambrée", "Huile rare pour stabiliser les effets latents d'une arme ou d'un catalyseur.", ShopItemCategory::Material, 88, 34, 1));
            }
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
            if (rotation % 4 == 0)
            {
                shop.addItem(ShopItem("firefly_iron_shell", "Carapace de luciole de fer", "Métal léger lumineux pour petites pièces, gardes et lanternes.", ShopItemCategory::Material, 78, 29, 2));
            }
            if (rotation % 6 == 2)
            {
                shop.addItem(ShopItem("grey_drake_scale", "Écaille de drake gris", "Matériau draconique de falaise, recherché pour harnais et lances.", ShopItemCategory::Material, 130, 44, 1));
            }
            if (rotation % 5 == 3)
            {
                shop.addItem(ShopItem("rusted_gear_core", "Noyau d'engrenage rouillé", "Pièce mécanique pour pièges, marteaux, arbalètes et automates.", ShopItemCategory::Material, 86, 35, 1));
            }
            if (rotation % 6 == 4)
            {
                shop.addItem(ShopItem("buried_giant_chip", "Éclat de géant enfoui", "Fragment osseux ou minéral lourd, prisé pour masses, plaques et rituels de poids.", ShopItemCategory::Material, 150, 58, 1));
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
            if (rotation % 5 == 4)
            {
                shop.addItem(ShopItem("elemental_ward_potion", "Potion de voile élémentaire", "Voile court contre les brûlures, poisons, givre et décharges.", ShopItemCategory::Consumable, 135, 30, 1));
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
                shop.addItem(ShopItem("weakening_debuff_potion", "Fiole d'affaiblissement", "Fiole offensive instable à lancer sur une cible.", ShopItemCategory::Consumable, 85, 25, 1));
            }
            if (rotation % 4 == 1)
            {
                shop.addItem(ShopItem("fragility_debuff_potion", "Fiole de fragilisation", "Fiole plus rare qui ouvre une faille dans la défense adverse.", ShopItemCategory::Consumable, 118, 32, 1));
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
            if (rotation % 5 == 0)
            {
                shop.addItem(ShopItem("tideworn_ink", "Encre rongée par la marée", "Encre magique humide pour parchemins, archives et mauvais contrats.", ShopItemCategory::Material, 82, 31, 1));
            }
            if (rotation % 6 == 1)
            {
                shop.addItem(ShopItem("mirror_glass_bead", "Perle de verre miroir", "Composant d'illusion, de diversion et de capes douteuses.", ShopItemCategory::Material, 92, 33, 1));
            }
            if (rotation % 5 == 0)
            {
                shop.addItem(ShopItem("mistglass_pearl", "Perle de verre-brume", "Composant rare des canaux, parfait pour fioles d'illusion et brouillards courts.", ShopItemCategory::Material, 136, 52, 1));
            }
            if (rotation % 6 == 2)
            {
                shop.addItem(ShopItem("weeping_stone_tear", "Larme de pierre pleureuse", "Minéral froid utilisé pour stabiliser quelques baumes trop chauds.", ShopItemCategory::Material, 90, 33, 1));
            }
            break;


        case ShopType::Enchanter:
            shop.addItem(ShopItem("arcane_dust", "Poussière arcanique", "Catalyseur de base pour runes mineures et réparations magiques instables.", ShopItemCategory::Material, 46, 14, 2 + rotation % 2));
            shop.addItem(ShopItem("runic_stabilizer", "Stabilisateur runique", "Composant de sécurité : réduit les risques quand une pièce approche des 5 enchantements.", ShopItemCategory::Material, 82, 34, 1));
            shop.addItem(ShopItem("runic_safety_seal", "Sceau anti-casse runique", "Protection d'atelier à usage unique : sauve un objet si une tentative d'enchantement tourne à la casse.", ShopItemCategory::Information, 124, 52, 1));
            shop.addItem(ShopItem("runic_extraction_note", "Note d'extraction runique", "Compte-rendu utile après un désenchantement ou avant de retenter une pièce instable.", ShopItemCategory::Information, 36, 12, 1));
            shop.addItem(ShopItem("runic_transfer_note", "Note de transfert runique", "Suivi d'atelier pour une rune déplacée. Utile, mais pas une garantie de sécurité.", ShopItemCategory::Information, 74, 28, 1));
            shop.addItem(ShopItem("runic_overload_limit_note", "Note de limite de surcharge", "Lecture prudente : rappelle quand une pièce approche une surcharge où l'empilement devient trop risqué.", ShopItemCategory::Information, 48, 18, 1));
            shop.addItem(ShopItem("minor_fire_rune_note", "Formule de rune anti-feu", "Notice vendue par l'enchanteur : utile pour comprendre les protections feu/chaleur, pas consommée comme une vraie rune.", ShopItemCategory::Information, 85, 20, 1, true));
            shop.addItem(ShopItem("minor_cold_rune_note", "Formule de rune anti-froid", "Notice vendue par l'enchanteur : utile pour comprendre les protections froid/givre.", ShopItemCategory::Information, 80, 20, 1, true));
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("mountain_blue_flower", "Fleur bleue de montagne", "Plante froide utilisée par les runes anti-givre mineures.", ShopItemCategory::Plant, 45, 12, 2));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("draconic_scale_fragment", "Fragment d'écaille draconique", "Composant rare pour enchantements chauds et équipements plus stables.", ShopItemCategory::Material, 185, 48, 1));
                shop.addItem(ShopItem("rare_fire_rune_core", "Cœur de rune ignifuge", "Noyau de rune plus cher pour protections feu/chaleur sérieuses.", ShopItemCategory::Material, 165, 58, 1));
            }
            if (rotation % 3 == 2)
            {
                shop.addItem(ShopItem("runic_iron_shard", "Éclat de fer runique", "Métal stable pour futurs enchantements appliqués aux armes.", ShopItemCategory::Material, 74, 26, 1 + rotation % 2));
                shop.addItem(ShopItem("rare_cold_rune_core", "Cœur de rune antigel", "Noyau de rune plus cher pour protections froid/givre sérieuses.", ShopItemCategory::Material, 158, 56, 1));
            }
            break;

        case ShopType::CityService:
            shop.addItem(ShopItem("city_service_stamp", "Tampon de service municipal", "Preuve de guichet : utile pour dossiers, plaintes, petites autorisations et services PNJ.", ShopItemCategory::Information, 16, 6, 4 + rotation % 3));
            shop.addItem(ShopItem("municipal_proof_letter", "Attestation municipale", "Lettre officielle plus propre qu'une simple parole. Les PNJ administratifs l'aiment beaucoup trop.", ShopItemCategory::Information, 42, 14, 1 + rotation % 2));
            shop.addItem(ShopItem("local_service_letter", "Lettre de service local", "Petit justificatif social : pas une prime, mais parfois une porte qui s'ouvre plus facilement.", ShopItemCategory::Information, 24, 8, 2));
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("local_reputation_note", "Note de réputation locale", "Mot court signé par un habitant ou un bureau. C'est léger, mais ça reste dans le dossier.", ShopItemCategory::Information, 15, 5, 2 + rotation % 2));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("client_recommendation", "Recommandation de client", "Recommandation simple pour un futur service. Trop faible pour une promotion, utile pour convaincre.", ShopItemCategory::Information, 34, 10, 1));
            }
            break;

        case ShopType::Lodging:
            shop.addItem(ShopItem("survival_ration", "Ration de survie", "Ration compacte pour rester dehors plus d'une journée complète sans rentrer.", ShopItemCategory::Material, 15, 7, 5 + rotation % 3));
            shop.addItem(ShopItem("fire_lantern", "Lanterne à feu", "Éclairage simple pour réduire les risques de sortie nocturne.", ShopItemCategory::Material, 42, 18, 2));
            shop.addItem(ShopItem("thermal_survival_blanket", "Couverture de survie thermique", "Couverture de secours pour ne pas transformer une nuit froide en tombeau narratif.", ShopItemCategory::Material, 42, 15, 1 + rotation % 2));
            if (rotation % 2 == 1)
            {
                shop.addItem(ShopItem("temperature_survival_kit", "Kit de survie thermique", "Kit général contre froid, chaleur et mauvaises décisions météo.", ShopItemCategory::Material, 72, 28, 1));
            }
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("night_survival_kit", "Kit de survie nocturne", "Matériel utile pour limiter les mauvaises surprises quand la route finit dans le noir.", ShopItemCategory::Material, 85, 34, 1));
            }
            shop.addItem(ShopItem("warm_meal_voucher", "Bon de repas chaud", "Repas simple, pratique pour remplacer les micro-primes ridicules et les excuses de comptoir.", ShopItemCategory::Information, 12, 6, 4 + rotation % 3));
            shop.addItem(ShopItem("lodging_bed_token", "Bon de lit d'auberge", "Une nuit simple à l'auberge. Pas luxueuse, mais mieux qu'un fossé narratif.", ShopItemCategory::Information, 24, 8, 2 + rotation % 2));
            shop.addItem(ShopItem("stable_stall_ticket", "Ticket d'écurie", "Réservation de place d'écurie ou de stockage court. Utile avec les relais et caravanes.", ShopItemCategory::Information, 28, 10, 1 + rotation % 2));
            shop.addItem(ShopItem("prepared_saddlebags", "Sacoches préparées", "Sacoches vérifiées pour éviter de perdre du temps sur une sortie éloignée.", ShopItemCategory::Information, 32, 12, 1));
            shop.addItem(ShopItem("stable_box_reservation", "Réservation de box sécurisé", "Box réservé : utile pour garder monture, sacs ou charge avant une sortie ou une quête de relais.", ShopItemCategory::Information, 46, 22, 1));
            shop.addItem(ShopItem("mount_rest_care", "Soin et repos de monture", "Retire la fatigue d'une monture personnelle avant que l'écurie ne refuse le départ.", ShopItemCategory::Information, 62, 28, 1));
            shop.addItem(ShopItem("stable_saddle_upgrade", "Selle renforcée de route", "Préparation durable pour longs trajets, utile avec une monture personnelle.", ShopItemCategory::Information, 118, 46, 1));
            shop.addItem(ShopItem("mount_comfort_bridle", "Bridon confortable", "Équipement durable : rend la monture personnelle plus agréable à guider sur les longues routes.", ShopItemCategory::Information, 86, 38, 1));
            shop.addItem(ShopItem("mount_weather_blanket", "Couverture météo de monture", "Protection durable pour monture personnelle : pluie, froid léger, nuits dehors et routes longues mieux tenues.", ShopItemCategory::Material, 74, 32, 1));
            shop.addItem(ShopItem("mount_pack_harness", "Harnais de bât ajusté", "Répartit mieux les charges d'une monture personnelle, utile sur les explorations longues sans rendre l'animal infatigable.", ShopItemCategory::Information, 92, 40, 1));
            shop.addItem(ShopItem("mount_road_shoes", "Ferrage de route simple", "Fers et sabots vérifiés : aide une monture personnelle à mieux tenir les routes répétées.", ShopItemCategory::Information, 88, 44, 1));
            shop.addItem(ShopItem("mount_grooming_kit", "Kit de pansage de monture", "Brosses et onguent simple pour travailler le lien sans remplacer un vrai repos.", ShopItemCategory::Material, 42, 18, 1 + rotation % 2));
            if (rotation % 2 == 1)
            {
                shop.addItem(ShopItem("loaded_pack_saddle", "Selle de bât chargée", "Selle préparée et charge équilibrée : excellente pour départs longs.", ShopItemCategory::Information, 52, 24, 1));
            }
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("temporary_stable_storage", "Dépôt temporaire d'écurie", "Reçu de stockage court : pratique quand un service parle de cargaison, box ou relais.", ShopItemCategory::Information, 18, 9, 1));
            }
            if (rotation % 3 == 0)
            {
                shop.addItem(ShopItem("local_reputation_note", "Note de réputation locale", "Tavia note les clients qui paient, rangent et ne mettent pas le feu à la nappe.", ShopItemCategory::Information, 14, 5, 2));
            }
            break;

        case ShopType::Transport:
            shop.addItem(ShopItem("survival_ration", "Ration de survie", "Ration simple vendue par les relais pour les sorties qui s'étirent loin de la ville.", ShopItemCategory::Material, 17, 7, 4 + rotation % 2));
            shop.addItem(ShopItem("travel_distance_mark", "Marque de distance de trajet", "Petite note de relais : utile quand une exploration demande un déplacement réel, pas juste trois pas derrière la boutique.", ShopItemCategory::Information, 16, 7, 3));
            shop.addItem(ShopItem("route_scout_note", "Note d'éclaireur de route", "Itinéraire relu : raccourci, relais à éviter ou pont encore utilisable. Très utile avant un biome éloigné.", ShopItemCategory::Information, 38, 16, 1));
            shop.addItem(ShopItem("rental_mount_voucher", "Bon de monture de location", "Monture louée pour réduire le temps de route sur les biomes lointains ou les longues sorties.", ShopItemCategory::Information, 70, 26, 1));
            if (rotation % 4 == 0)
            {
                shop.addItem(ShopItem("owned_mount_registration", "Acte de monture personnelle", "Contrat cher mais durable : une monture peut ensuite aider plusieurs trajets avant de fatiguer.", ShopItemCategory::Information, 240, 95, 1));
            }
            shop.addItem(ShopItem("relay_route_badge", "Badge de route du relais", "Badge officiel du relais : passage préparé, contrôles plus simples, route plus lisible.", ShopItemCategory::Information, 44, 18, 1 + rotation % 2));
            shop.addItem(ShopItem("fire_lantern", "Lanterne à feu", "Matériel simple pour éviter de traverser la nuit totalement à l'aveugle.", ShopItemCategory::Material, 45, 18, 2));
            shop.addItem(ShopItem("cooling_survival_wrap", "Voile anti-chaleur", "Voile traité pour routes brûlantes, déserts et abords volcaniques.", ShopItemCategory::Material, 48, 16, 1 + rotation % 2));
            if (rotation % 3 == 2)
            {
                shop.addItem(ShopItem("temperature_survival_kit", "Kit de survie thermique", "Kit complet pour route froide, chaude ou juste très stupide.", ShopItemCategory::Material, 76, 28, 1));
            }
            if (rotation % 2 == 1)
            {
                shop.addItem(ShopItem("night_survival_kit", "Kit de survie nocturne", "Kit plus complet pour les trajets longs et les explorations qui débordent après le soir.", ShopItemCategory::Material, 92, 34, 1));
            }
            shop.addItem(ShopItem("travel_pass_note", "Note de pass de voyage", "Indication officielle pour préparer un trajet, un pont, une porte ou un contrôle de route.", ShopItemCategory::Information, 22, 10, 2 + rotation % 2));
            shop.addItem(ShopItem("route_toll_receipt", "Reçu de péage de route", "Petit reçu utile pour justifier un passage et éviter de repayer le même garde de mauvaise humeur.", ShopItemCategory::Information, 28, 9, 2));
            shop.addItem(ShopItem("caravan_seat_ticket", "Place de caravane", "Ticket de transport simple. Lent, bruyant, mais beaucoup moins seul qu'une route au hasard.", ShopItemCategory::Information, 58, 18, 1 + rotation % 2));
            if (rotation % 2 == 1)
            {
                shop.addItem(ShopItem("guarded_transport_pass", "Pass de transport gardé", "Convoi escorté : cher, mais plus logique quand les routes sont remplies d'ennuis à dents.", ShopItemCategory::Information, 102, 32, 1));
            }
            break;

        case ShopType::Church:
            shop.addItem(ShopItem("holy_water_vial", "Fiole d'eau bénite", "Petite fiole préparée pour rites, diagnostics de malédiction et demandes d'église.", ShopItemCategory::Material, 72, 22, 2 + rotation % 3));
            shop.addItem(ShopItem("sanctuary_candle", "Cierge de veille", "Cierge consacré : utile pour veillées, prières courtes et rituels de protection.", ShopItemCategory::Material, 34, 11, 3 + rotation % 4));
            shop.addItem(ShopItem("exorcism_incense", "Encens d'exorcisme", "Encens utilisé par Sœur Maëlys quand un diagnostic doit aller plus loin qu'une simple intuition.", ShopItemCategory::Material, 64, 20, 1 + rotation % 3));
            shop.addItem(ShopItem("blessing_note", "Note de bénédiction", "Billet signé par Père Orwan ou Frère Calixte après une bénédiction de route.", ShopItemCategory::Information, 52, 14, 1 + rotation % 2, true));
            shop.addItem(ShopItem("exorcist_note", "Note d'exorciste", "Renseignement simple : toutes les malédictions ne se retirent pas par la prière.", ShopItemCategory::Information, 95, 18, 1, true));
            if (rotation % 2 == 0)
            {
                shop.addItem(ShopItem("moonlit_salt", "Sel lunaire", "Sel rituel déjà utilisé par certains sanctuaires contre les traces occultes.", ShopItemCategory::Material, 68, 24, 2));
            }
            if (rotation % 3 == 1)
            {
                shop.addItem(ShopItem("white_bone_chalk", "Craie d'os blanc", "Craie rituelle pour dessiner un cercle d'exorcisme stable.", ShopItemCategory::Material, 86, 30, 1));
            }
            if (rotation % 4 == 2)
            {
                shop.addItem(ShopItem("cracked_bell_clapper", "Battant de cloche fissuré", "Objet d'église abîmé, parfois utilisé pour briser une résonance maudite faible.", ShopItemCategory::Material, 115, 45, 1));
            }
            break;

        case ShopType::BlackMarket:
            shop.addItem(ShopItem(
                "black_market_barter_seal",
                "Sceau de troc douteux",
                "Marque discrète toujours acceptée par certains vendeurs louches à la place d'une petite partie de l'or.",
                ShopItemCategory::Information,
                72,
                24,
                1
            ));
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
                shop.addItem(ShopItem("conductive_knives", "Couteaux conducteurs interdits", "Projectiles métalliques capables d'attirer des réactions électriques.", ShopItemCategory::Material, 300, 9, 5));
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
                shop.addItem(ShopItem("elemental_ward_potion", "Voile élémentaire sous faux sceau", "Le vendeur dit que ça protège de presque tout. Il évite de dire combien de temps.", ShopItemCategory::Consumable, 260, 35, 1));
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
            if (rotation % 4 == 0)
            {
                shop.addItem(ShopItem("resistance_rift_scroll", "Parchemin de faille fragile", "Sort offensif à usage unique : ouvre une défense sans demander une vraie formation de mage.", ShopItemCategory::Spell, 360, 0, 1));
                shop.addItem(ShopItem("minor_purification_scroll", "Parchemin de purification volé", "Même effet qu'un parchemin honnête, mais l'origine est volontairement oubliée.", ShopItemCategory::Spell, 240, 0, 1));
                shop.addItem(ShopItem("crawling_venom_scroll", "Parchemin de venin rampant interdit", "Le même tracé que la bibliothèque refuse de garantir : efficace, sale, et impossible à rendre après lecture.", ShopItemCategory::Spell, 260, 0, 1));
            }
            if (rotation % 8 == 5)
            {
                shop.addItem(ShopItem("resistance_rift_grimoire", "Grimoire de faille de résistance", "Ouvrage volé et dangereux. Un non-mage n'en tirera qu'un mal de crâne.", ShopItemCategory::Book, 620, 35, 1));
            }
            if (rotation % 5 == 0)
            {
                shop.addItem(ShopItem("sealed_debt_slip", "Billet de dette scellé sans témoin", "Document de dette vendu sous le manteau. Le vendeur refuse de dire qui doit rembourser.", ShopItemCategory::Material, 260, 46, 1));
                shop.addItem(ShopItem("smuggler_token", "Jeton de pont noir", "Jeton de contrebandier lié aux marchés sous les ponts.", ShopItemCategory::Material, 220, 30, 2));
            }
            if (rotation % 7 == 4)
            {
                shop.addItem(ShopItem("mistglass_pearl", "Perle de verre-brume interdite", "Composant d'illusion vendu sans question, sans facture et sans morale.", ShopItemCategory::Material, 310, 52, 1));
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
