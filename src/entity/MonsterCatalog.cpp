// EN: MonsterCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: MonsterCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/MonsterCatalog.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <vector>
#include <algorithm>

namespace
{
    Monster createMonster(
        const std::string& name,
        const std::string& type,
        Race race,
        int level,
        int maxHp,
        int minDamage,
        int maxDamage,
        int criticalDamage,
        int healingPotionCount = 0,
        int damagePotionCount = 0,
        bool invocation = false,
        bool elite = false,
        bool hiddenStats = false,
        bool evolved = false
    )
    {
        return Monster(
            name,
            type,
            race,
            level,
            maxHp,
            minDamage,
            maxDamage,
            criticalDamage,
            healingPotionCount,
            damagePotionCount,
            invocation,
            elite,
            hiddenStats,
            evolved
        );
    }

    std::vector<Monster> createTierOneMonsters()
    {
        return {
            createMonster("Gobelin peureux", "Assassin primitif", Race::Gobelin, 1, 55, 8, 14, 18),
            createMonster("Chauve-souris des cavernes", "Créature rapide", Race::Bete, 1, 45, 7, 15, 20),
            createMonster("Rat géant", "Bête nuisible", Race::Bete, 1, 50, 6, 13, 18),
            createMonster("Slime fragile", "Gelée vivante", Race::Slime, 1, 60, 4, 12, 16),
            createMonster("Racine agitée", "Plante hostile", Race::Plante, 1, 70, 5, 14, 18),
            createMonster("Kobold paniqué", "Petit draconide", Race::Draconide, 1, 58, 7, 15, 20)
        };
    }

    std::vector<Monster> createTierTwoMonsters()
    {
        return {
            createMonster("Gobelin brutal", "Bagarreur sauvage", Race::Gobelin, 2, 75, 10, 18, 22),
            createMonster("Loup affamé", "Prédateur rapide", Race::Bete, 2, 65, 9, 20, 25),
            createMonster("Araignée venimeuse", "Insectoïde toxique", Race::Insectoide, 2, 70, 8, 19, 24),
            createMonster("Bandit maladroit", "Humain hostile", Race::Humain, 2, 80, 9, 18, 24, 1, 0),
            createMonster("Squelette instable", "Mort-vivant fragile", Race::MortVivant, 2, 78, 10, 19, 24),
            createMonster("Méphaïte de braise", "Petit élémentaire", Race::Elementaire, 2, 72, 9, 21, 26)
        };
    }

    std::vector<Monster> createTierThreeMonsters()
    {
        return {
            createMonster("Squelette fissuré", "Mort-vivant fragile", Race::MortVivant, 3, 80, 12, 22, 24),
            createMonster("Sanglier sauvage", "Bête résistante", Race::Bete, 3, 95, 12, 21, 24),
            createMonster("Gobelin pillard", "Voleur opportuniste", Race::Gobelin, 3, 90, 11, 22, 30, 1, 0),
            createMonster("Goule affamée", "Mort-vivant agressif", Race::MortVivant, 3, 105, 12, 24, 30),
            createMonster("Esprit mineur", "Entité flottante", Race::Esprit, 3, 85, 10, 25, 32, 0, 1),
            createMonster("Alchimiste renégat", "Humain dangereux", Race::Humain, 3, 88, 6, 18, 28, 1, 2)
        };
    }

    std::vector<Monster> createTierFourMonsters()
    {
        return {
            createMonster("Orc mineur", "Combattant lourd", Race::Orc, 4, 120, 15, 26, 28, 0, 0, false, true),
            createMonster("Bandit perdu", "Humain opportuniste", Race::Humain, 4, 100, 14, 24, 30, 1, 0),
            createMonster("Orc éclaireur", "Pillard mobile", Race::Orc, 4, 115, 14, 27, 34),
            createMonster("Mage renégat", "Humain arcanique", Race::Humain, 4, 92, 10, 30, 44, 1, 2),
            createMonster("Chevalier corrompu", "Humain en armure", Race::Humain, 4, 135, 13, 25, 34, 1, 0, false, true),
            createMonster("Anomalie arcanique instable", "Erreur magique", Race::AnomalieArcanique, 4, 110, 12, 32, 46, 0, 1, false, false, true)
        };
    }

    std::vector<Monster> createTierFivePlusMonsters()
    {
        return {
            createMonster("Ours brun", "Bête massive", Race::Bete, 5, 160, 18, 34, 42, 0, 0, false, true),
            createMonster("Orc berserker", "Briseur sauvage", Race::Orc, 5, 170, 20, 38, 50, 0, 1, false, true),
            createMonster("Revenant silencieux", "Mort-vivant tenace", Race::MortVivant, 5, 145, 16, 33, 45, 0, 0, false, true, true),
            createMonster("Démon mineur", "Créature infernale", Race::Demon, 5, 150, 18, 36, 48, 0, 1, false, true, true),
            createMonster("Élémentaire instable", "Énergie condensée", Race::Elementaire, 5, 140, 16, 39, 52, 0, 1, false, true, true),
            createMonster("Chevalier sans âme", "Armure morte", Race::MortVivant, 6, 190, 18, 35, 48, 0, 0, false, true, true),
            createMonster("Colosse sauvage", "Bête humanoïde", Race::Aberration, 7, 260, 22, 46, 60, 0, 1, false, true, true),
            createMonster("Dragon mineur", "Jeune menace draconique", Race::Dragon, 8, 320, 25, 55, 75, 0, 2, false, true, true)
        };
    }


    std::string evolvedNameFor(const Monster& monster, Random& random)
    {
        if (monster.getRace() == Race::Slime)
        {
            std::vector<std::string> slimeSuffixes = {" à noyau dense", " gélatineux majeur", " chromatique", " vorace"};
            return monster.getName() + slimeSuffixes[random.between(0, static_cast<int>(slimeSuffixes.size()) - 1)];
        }

        if (monster.getRace() == Race::Gobelin
            && monster.getName().find("shaman") == std::string::npos
            && monster.getName().find("chamane") == std::string::npos
            && monster.getName().find("Shaman") == std::string::npos
            && monster.getName().find("Chamane") == std::string::npos
            && random.between(1, 100) <= 30)
        {
            return "Gobelin shaman issu de " + monster.getName();
        }

        std::vector<std::string> prefixes = {
            "Évolué - ",
            "Ancien - ",
            "Muté - ",
            "Marqué - ",
            "Alpha - "
        };

        std::vector<std::string> suffixes = {
            " renforcé",
            " éveillé",
            " instable",
            " survivant",
            " à variation"
        };

        if (random.between(1, 100) <= 55)
        {
            return prefixes[random.between(0, static_cast<int>(prefixes.size()) - 1)] + monster.getName();
        }

        return monster.getName() + suffixes[random.between(0, static_cast<int>(suffixes.size()) - 1)];
    }

    // EN: scaledValue declares or implements a focused behavior used by this module.
    // FR: scaledValue déclare ou implémente un comportement précis utilisé par ce module.
    int scaledValue(int value, int percent)
    {
        return std::max(1, value * percent / 100);
    }

    // EN: chooseFromList declares or implements a focused behavior used by this module.
    // FR: chooseFromList déclare ou implémente un comportement précis utilisé par ce module.
    Monster scaleMonsterToTargetLevel(const Monster& baseMonster, int targetLevel);

    Monster chooseFromList(const std::vector<Monster>& monsters, Random& random)
    {
        int index = random.between(0, static_cast<int>(monsters.size()) - 1);
        return monsters[index];
    }



    std::vector<Monster> createBiomeCommonMonsters(const std::string& biomeName)
    {
        if (biomeName == "Plaine sauvage")
        {
            return {
                createMonster("Loup de plaine", "Prédateur simple", Race::Bete, 2, 70, 8, 19, 24),
                createMonster("Sanglier des herbes", "Bête résistante", Race::Bete, 3, 95, 12, 22, 26),
                createMonster("Slime vert de rosée", "Gelée verte faible", Race::Slime, 1, 58, 4, 12, 16),
                createMonster("Slime bleu paresseux", "Gelée bleue défensive", Race::Slime, 2, 72, 3, 11, 15),
                createMonster("Slime jaune curieux", "Gelée nerveuse attirée par le mouvement", Race::Slime, 2, 62, 6, 14, 18),
                createMonster("Slime rouge tiède", "Gelée rouge encore peu dangereuse", Race::Slime, 2, 66, 7, 15, 20),
                createMonster("Rat des hautes herbes", "Bête nuisible", Race::Bete, 1, 50, 6, 13, 18),
                createMonster("Gobelin trouillard", "Gobelin désorganisé", Race::Gobelin, 1, 42, 3, 8, 12),
                createMonster("Gobelin ramasseur", "Gobelin attiré par ce qui brille mais mauvais combattant", Race::Gobelin, 1, 46, 4, 10, 14),
                createMonster("Gobelin maladroit", "Gobelin nerveux, plus bruyant que dangereux", Race::Gobelin, 1, 40, 2, 7, 10),
                createMonster("Gobelin éclaireur de talus", "Gobelin prudent qui préfère prévenir les siens", Race::Gobelin, 2, 58, 5, 13, 18),
                createMonster("Lapin cornu nerveux", "Petite bête territoriale", Race::Bete, 1, 38, 4, 11, 16),
                createMonster("Racine sèche errante", "Plante presque passive mais accrocheuse", Race::Plante, 2, 78, 3, 12, 16),
                createMonster("Slime clair dormant", "Gelée presque passive qui gêne plus qu'elle ne tue", Race::Slime, 1, 64, 3, 9, 12),
                createMonster("Renard fouilleur", "Petite bête attirée par les sacs mal fermés", Race::Bete, 2, 52, 6, 16, 22)
            };
        }

        if (biomeName == "Mares gélatineuses")
        {
            return {
                createMonster("Slime vert limpide", "Gelée verte classique", Race::Slime, 3, 92, 7, 18, 24),
                createMonster("Slime bleu flottant", "Gelée bleue souple", Race::Slime, 4, 118, 6, 19, 25),
                createMonster("Slime jaune vibrant", "Gelée nerveuse et lumineuse", Race::Slime, 4, 105, 9, 22, 30),
                createMonster("Slime rouge chaud", "Gelée rouge irritante", Race::Slime, 5, 124, 11, 26, 35),
                createMonster("Slime ambré collant", "Gelée qui aime les zones riches en sève", Race::Slime, 5, 132, 8, 24, 32),
                createMonster("Slime gris de vase", "Gelée lourde chargée de poussière", Race::Slime, 6, 148, 9, 27, 36),
                createMonster("Slime rose nerveux", "Gelée vive qui bondit sans comprendre ce qu'elle fait", Race::Slime, 5, 112, 10, 25, 34),
                createMonster("Slime translucide", "Gelée presque invisible dans l'eau stagnante", Race::Slime, 6, 130, 8, 28, 38, 0, 0, false, false, true),
                createMonster("Mini-ruche gélatineuse", "Petit amas de slimes faibles qui se déplacent ensemble", Race::Slime, 6, 150, 7, 24, 34, 0, 0, false, true)
            };
        }

        if (biomeName == "Route commerciale")
        {
            return {
                createMonster("Bandit de grand chemin", "Humain opportuniste", Race::Humain, 3, 92, 11, 23, 30, 1, 0),
                createMonster("Gobelin détrousseur", "Voleur opportuniste", Race::Gobelin, 2, 74, 10, 19, 24),
                createMonster("Pilleur de chariot", "Humanoïde agressif", Race::Humain, 4, 108, 13, 25, 34, 1, 0),
                createMonster("Orc éclaireur de route", "Pillard mobile", Race::Orc, 4, 115, 14, 27, 34),
                createMonster("Coursier louche", "Humain fuyant", Race::Humain, 2, 62, 6, 16, 24, 0, 0),
                createMonster("Gobelin porteur", "Gobelin chargé de sacs", Race::Gobelin, 2, 88, 7, 16, 22),
                createMonster("Slime doré minuscule", "Gelée brillante attirée par les pièces perdues", Race::Slime, 3, 74, 5, 17, 24),
                createMonster("Apprenti receleur", "Humain nerveux avec une fiole et trop d'excuses", Race::Humain, 3, 78, 8, 19, 28, 1, 1)
            };
        }

        if (biomeName == "Forêt ancienne")
        {
            return {
                createMonster("Loup forestier", "Prédateur discret", Race::Bete, 4, 105, 13, 25, 32),
                createMonster("Racine vivante", "Plante hostile", Race::Plante, 3, 105, 8, 20, 26),
                createMonster("Plante mordante", "Végétal agressif", Race::Plante, 4, 115, 10, 24, 30),
                createMonster("Slime vert mousse", "Gelée camouflée", Race::Slime, 4, 112, 8, 21, 28),
                createMonster("Slime ambré de sève", "Gelée collante attirée par les vieux arbres", Race::Slime, 5, 126, 9, 24, 32),
                createMonster("Slime vert ancien", "Gelée stable nourrie par la mousse", Race::Slime, 6, 142, 10, 27, 36, 0, 0, false, true),
                createMonster("Sanglier moussu", "Bête ancienne", Race::Bete, 5, 150, 15, 30, 38, 0, 0, false, true)
            };
        }

        if (biomeName == "Montagne froide")
        {
            return {
                createMonster("Bête de givre", "Prédateur froid", Race::Bete, 7, 165, 18, 34, 44, 0, 0, false, true),
                createMonster("Élémentaire de neige", "Énergie froide", Race::Elementaire, 7, 150, 17, 38, 50, 0, 1, false, true, true),
                createMonster("Slime bleu gelé", "Gelée froide durcie par l'altitude", Race::Slime, 7, 155, 13, 31, 42, 0, 0, false, true),
                createMonster("Slime blanc poudreux", "Gelée froide qui absorbe la neige", Race::Slime, 8, 170, 12, 34, 46, 0, 0, false, true),
                createMonster("Briseur rocheux", "Créature massive", Race::Construction, 8, 210, 18, 36, 46, 0, 0, false, true),
                createMonster("Draconide froid", "Menace draconique", Race::Draconide, 9, 220, 21, 44, 60, 0, 1, false, true, true)
            };
        }

        if (biomeName == "Marais trouble")
        {
            return {
                createMonster("Slime violet toxique", "Gelée violette toxique", Race::Slime, 12, 220, 18, 36, 46, 0, 0, false, true),
                createMonster("Slime noir vaseux", "Gelée noire collante", Race::Slime, 13, 245, 14, 34, 44, 0, 0, false, true, true),
                createMonster("Slime vert putride jeune", "Gelée marécageuse chargée de toxines", Race::Slime, 13, 250, 16, 37, 48, 0, 0, false, true, true),
                createMonster("Noyé du marais", "Mort-vivant humide", Race::MortVivant, 13, 235, 19, 38, 50, 0, 0, false, true, true),
                createMonster("Insectoïde des eaux sales", "Prédateur toxique", Race::Insectoide, 12, 205, 18, 40, 52, 0, 1, false, true),
                createMonster("Plante toxique", "Végétal venimeux", Race::Plante, 14, 240, 16, 42, 56, 0, 1, false, true, true)
            };
        }

        if (biomeName == "Cimetière oublié")
        {
            return {
                createMonster("Squelette de fosse", "Mort-vivant fragile", Race::MortVivant, 10, 175, 17, 34, 44, 0, 0, false, true, true),
                createMonster("Corbeau charognard", "Bête attirée par les tombes", Race::Bete, 9, 135, 18, 38, 52, 0, 0, false, false, true),
                createMonster("Goule de caveau", "Mort-vivant affamé", Race::MortVivant, 11, 205, 20, 42, 56, 0, 0, false, true, true),
                createMonster("Lanterne d'âme faible", "Esprit errant lumineux", Race::Esprit, 10, 155, 16, 44, 60, 0, 1, false, true, true),
                createMonster("Rat de tombe", "Bête nuisible contaminée", Race::Bete, 9, 150, 16, 34, 46, 0, 0, false, false, true),
                createMonster("Ossements rassemblés", "Amas instable de morts", Race::MortVivant, 11, 240, 15, 36, 48, 0, 0, false, true, true)
            };
        }

        if (biomeName == "Ruines effondrées")
        {
            return {
                createMonster("Squelette des ruines", "Mort-vivant ancien", Race::MortVivant, 14, 230, 20, 40, 52, 0, 0, false, true, true),
                createMonster("Goule poussiéreuse", "Mort-vivant agressif", Race::MortVivant, 15, 255, 21, 43, 58, 0, 0, false, true, true),
                createMonster("Esprit fissuré", "Entité instable", Race::Esprit, 14, 215, 18, 46, 62, 0, 1, false, true, true),
                createMonster("Armure fissurée", "Construction hantée", Race::Construction, 16, 310, 20, 44, 56, 0, 0, false, true, true),
                createMonster("Slime gris gravé", "Gelée poussiéreuse qui a absorbé des runes", Race::Slime, 15, 250, 19, 48, 66, 0, 1, false, true, true)
            };
        }

        return createTierFivePlusMonsters();
    }

    std::vector<Monster> createBiomeUnusualMonsters(const std::string& biomeName)
    {
        if (biomeName == "Plaine sauvage")
        {
            return {
                createMonster("Gobelin éclaireur", "Petit pillard perdu", Race::Gobelin, 3, 86, 11, 22, 28),
                createMonster("Gobelin lance-cailloux", "Petit harceleur opportuniste", Race::Gobelin, 3, 72, 8, 20, 28),
                createMonster("Gobelin shaman débutant", "Petit soigneur paniqué", Race::Gobelin, 4, 82, 6, 16, 22, 2, 0, false, true),
                createMonster("Gobelin garde-boue", "Petit défenseur qui protège mal mais longtemps", Race::Gobelin, 4, 118, 6, 14, 20, 0, 0, false, true),
                createMonster("Gobelin frondeur", "Harceleur prudent qui garde ses distances", Race::Gobelin, 4, 76, 10, 26, 36, 0, 0, false, true),
                createMonster("Plante agressive isolée", "Végétal errant", Race::Plante, 3, 98, 8, 20, 25),
                createMonster("Chauve-souris de passage", "Créature rapide", Race::Bete, 2, 62, 8, 19, 26)
            };
        }

        if (biomeName == "Mares gélatineuses")
        {
            return {
                createMonster("Slime violet piquant", "Gelée toxique jeune", Race::Slime, 8, 172, 14, 34, 46, 0, 0, false, true),
                createMonster("Slime blanc froid", "Gelée refroidie par une nappe souterraine", Race::Slime, 8, 168, 13, 33, 44, 0, 0, false, true),
                createMonster("Slime noir poisseux", "Gelée dense qui colle aux bottes", Race::Slime, 9, 190, 13, 35, 48, 0, 0, false, true, true),
                createMonster("Slime rouge irritant majeur", "Gelée chaude et agressive", Race::Slime, 9, 184, 17, 39, 52, 0, 1, false, true),
                createMonster("Slime vert putride naissant", "Gelée contaminée mais encore instable", Race::Slime, 10, 205, 15, 38, 52, 0, 0, false, true, true),
                createMonster("Slime jaune d'orage", "Gelée vibrante chargée de petites décharges", Race::Slime, 10, 198, 16, 42, 58, 0, 1, false, true, true),
                createMonster("Slime ambré royal", "Gelée collante qui immobilise les imprudents", Race::Slime, 11, 235, 14, 40, 56, 0, 0, false, true, true),
                createMonster("Slime rose bondissant", "Gelée qui attaque par rebonds imprévisibles", Race::Slime, 10, 188, 18, 43, 58, 0, 0, false, true),
                createMonster("Slime nacré sensible", "Gelée rare qui réagit violemment aux gestes brusques", Race::Slime, 11, 210, 13, 44, 62, 0, 1, false, true, true),
                createMonster("Slime vert régénérant", "Gelée calme qui se recompose lentement", Race::Slime, 10, 245, 10, 34, 48, 0, 0, false, true, true),
                createMonster("Slime rouge bouillonnant", "Gelée brûlante qui punit les contacts directs", Race::Slime, 11, 220, 18, 46, 64, 0, 1, false, true, true)
            };
        }

        if (biomeName == "Route commerciale")
        {
            return {
                createMonster("Mercenaire douteux", "Combattant payé", Race::Humain, 5, 145, 16, 31, 40, 1, 1, false, true),
                createMonster("Archer embusqué", "Tireur de route", Race::Humain, 5, 125, 18, 36, 48, 1, 0, false, true),
                createMonster("Gobelin shaman novice", "Chaman gobelin soigneur", Race::Gobelin, 6, 122, 10, 30, 44, 2, 1, false, true, true),
                createMonster("Gobelin rapiéceur", "Soutien gobelin maladroit", Race::Gobelin, 5, 115, 9, 24, 34, 1, 0, false, true),
                createMonster("Hobgobelin receleur", "Organisateur de pillage", Race::Hobgobelin, 6, 160, 18, 34, 46, 1, 1, false, true),
                createMonster("Gobelin shaman de route", "Chaman gobelin qui soigne surtout ses alliés", Race::Gobelin, 7, 140, 11, 34, 48, 2, 1, false, true, true),
                createMonster("Hobgobelin tambour", "Support de bande qui galvanise les autres sans être un vrai mage", Race::Hobgobelin, 7, 175, 14, 32, 44, 1, 1, false, true),
                createMonster("Gobelin tireur nerveux", "Harceleur à distance qui préfère blesser puis fuir", Race::Gobelin, 6, 118, 13, 35, 48, 1, 0, false, true),
                createMonster("Bandit apothicaire", "Humain de route avec quelques fioles douteuses", Race::Humain, 7, 132, 12, 31, 46, 2, 2, false, true)
            };
        }

        if (biomeName == "Forêt ancienne")
        {
            return {
                createMonster("Dryade corrompue", "Esprit végétal", Race::Esprit, 6, 155, 15, 35, 48, 0, 1, false, true, true),
                createMonster("Sanglier ancien", "Bête massive", Race::Bete, 7, 210, 19, 36, 46, 0, 0, false, true),
                createMonster("Araignée de mousse", "Insectoïde forestier", Race::Insectoide, 6, 145, 16, 34, 44, 0, 1, false, true),
                createMonster("Mante-feuille", "Insectoïde camouflé dans les branches", Race::Insectoide, 7, 135, 19, 39, 52, 0, 1, false, true),
                createMonster("Racine veilleuse", "Plante ancienne qui protège son sol", Race::Plante, 7, 190, 14, 34, 46, 0, 1, false, true),
                createMonster("Champignon hurleur", "Fonge vivante qui attire parfois d'autres dangers", Race::Plante, 8, 165, 12, 36, 50, 0, 1, false, true, true),
                createMonster("Cerf moussu inquiet", "Bête ancienne rarement agressive mais dangereuse acculée", Race::Bete, 8, 230, 17, 38, 52, 0, 0, false, true),
                createMonster("Liane guetteuse", "Plante fine qui préfère immobiliser avant de blesser", Race::Plante, 7, 150, 11, 33, 45, 0, 1, false, true)
            };
        }

        if (biomeName == "Montagne froide")
        {
            return {
                createMonster("Golem fissuré", "Construction rocheuse", Race::Construction, 10, 300, 22, 44, 58, 0, 0, false, true, true),
                createMonster("Chasseur des cimes", "Humanoïde montagnard", Race::Humain, 9, 190, 21, 42, 56, 1, 1, false, true),
                createMonster("Méphaïte de givre", "Petit élémentaire", Race::Elementaire, 8, 160, 18, 41, 54, 0, 1, false, true, true),
                createMonster("Bouc des neiges agressif", "Bête de falaise qui charge plus qu'elle ne réfléchit", Race::Bete, 9, 205, 21, 43, 58, 0, 0, false, true),
                createMonster("Golem d'éboulis", "Construction instable qui frappe fort mais se fissure", Race::Construction, 11, 335, 25, 50, 66, 0, 0, false, true, true)
            };
        }

        if (biomeName == "Marais trouble")
        {
            return {
                createMonster("Goule humide", "Mort-vivant contaminé", Race::MortVivant, 16, 285, 23, 47, 60, 0, 0, false, true, true),
                createMonster("Sangsue géante", "Insectoïde parasite", Race::Insectoide, 15, 255, 22, 45, 58, 0, 1, false, true),
                createMonster("Slime rouge irritant", "Gelée rouge agressive", Race::Slime, 16, 270, 25, 50, 66, 0, 1, false, true),
                createMonster("Slime vert putride", "Gelée de marais à relents toxiques", Race::Slime, 16, 292, 20, 48, 64, 0, 1, false, true, true),
                createMonster("Sorcier putride novice", "Humain corrompu", Race::Humain, 17, 250, 18, 54, 74, 1, 2, false, true, true),
                createMonster("Chamane de vase", "Soigneur toxique du marais", Race::Humain, 18, 265, 16, 50, 70, 2, 1, false, true, true),
                createMonster("Grenouille géante venimeuse", "Bête amphibie toxique", Race::Bete, 16, 305, 24, 52, 70, 0, 1, false, true, true),
                createMonster("Moustique sanguin majeur", "Insectoïde de marais attiré par les blessés", Race::Insectoide, 17, 210, 28, 58, 76, 0, 1, false, true, true)
            };
        }


        if (biomeName == "Cimetière oublié")
        {
            return {
                createMonster("Veilleur sans sépulture", "Mort-vivant gardien", Race::MortVivant, 15, 300, 24, 52, 68, 0, 0, false, true, true),
                createMonster("Ombre de fossoyeur", "Esprit discret et rancunier", Race::Esprit, 14, 245, 22, 58, 78, 0, 1, false, true, true),
                createMonster("Goule à griffes noires", "Mort-vivant agressif", Race::MortVivant, 15, 280, 28, 56, 74, 0, 0, false, true, true),
                createMonster("Squelette porte-bannière", "Support mort-vivant de vieille guerre", Race::MortVivant, 16, 260, 20, 50, 70, 1, 1, false, true, true),
                createMonster("Lanterne funéraire", "Esprit qui attire les vivants", Race::Esprit, 16, 250, 22, 64, 86, 0, 2, false, true, true),
                createMonster("Chien de tombe", "Bête maigre dressée par l'odeur des morts", Race::Bete, 14, 270, 27, 55, 72, 0, 0, false, true, true)
            };
        }
        if (biomeName == "Ruines effondrées")
        {
            return {
                createMonster("Spectre errant", "Esprit des ruines", Race::Esprit, 18, 270, 23, 55, 72, 0, 1, false, true, true),
                createMonster("Armure morte", "Armure hantée", Race::MortVivant, 19, 360, 24, 50, 66, 0, 0, false, true, true),
                createMonster("Anomalie mineure", "Erreur arcanique", Race::AnomalieArcanique, 18, 295, 22, 58, 78, 0, 2, false, true, true),
                createMonster("Squelette archer rouillé", "Mort-vivant à distance aux gestes mécaniques", Race::MortVivant, 18, 245, 24, 60, 80, 0, 0, false, true, true),
                createMonster("Slime gris poussiéreux", "Gelée nourrie par les pierres mortes", Race::Slime, 17, 285, 20, 52, 70, 0, 0, false, true, true)
            };
        }

        return createTierFivePlusMonsters();
    }

    std::vector<Monster> createBiomeRareMonsters(const std::string& biomeName)
    {
        if (biomeName == "Plaine sauvage")
        {
            return {
                createMonster("Loup alpha jeune", "Alpha de plaine", Race::Bete, 6, 180, 18, 36, 48, 0, 0, false, true),
                createMonster("Ours errant", "Bête massive", Race::Bete, 7, 230, 21, 42, 54, 0, 0, false, true),
                createMonster("Slime brillant avaleur", "Gelée attirée par l'or", Race::Slime, 6, 170, 12, 32, 42, 0, 0, false, true),
                createMonster("Sanglier cuirassé jeune", "Bête de plaine dont la peau commence à durcir", Race::Bete, 8, 255, 23, 45, 58, 0, 0, false, true),
                createMonster("Renard chapardeur", "Petite bête maligne attirée par les sacs ouverts", Race::Bete, 6, 145, 17, 38, 50, 0, 0, false, true)
            };
        }

        if (biomeName == "Mares gélatineuses")
        {
            return {
                createMonster("Slime chromatique", "Gelée rare instable", Race::Slime, 14, 330, 22, 56, 76, 0, 2, false, true, true),
                createMonster("Slime doré avaleur", "Gelée attirée par l'or et les objets brillants", Race::Slime, 15, 350, 20, 58, 84, 0, 0, false, true, true),
                createMonster("Slime noir ancien", "Gelée profonde presque intelligente", Race::Slime, 16, 390, 24, 62, 88, 0, 1, false, true, true),
                createMonster("Slime prisme instable", "Gelée rarissime qui change de réaction selon la lumière", Race::Slime, 17, 405, 26, 68, 96, 0, 2, false, true, true),
                createMonster("Noyau de ruche gélatineuse", "Masse centrale entourée de petits slimes", Race::Slime, 18, 470, 22, 64, 90, 0, 1, false, true, true),
                createMonster("Slime miroir ancien", "Gelée ancienne qui semble copier la pression autour d'elle", Race::Slime, 19, 455, 28, 72, 102, 0, 2, false, true, true)
            };
        }

        if (biomeName == "Route commerciale")
        {
            return {
                createMonster("Chef bandit", "Commandant de route", Race::Humain, 9, 250, 25, 50, 65, 2, 1, false, true),
                createMonster("Pilleur vétéran", "Humanoïde organisé", Race::Humain, 10, 270, 26, 52, 68, 1, 1, false, true),
                createMonster("Chasseur de primes rouillé", "Traqueur illégal", Race::Humain, 11, 255, 30, 60, 80, 1, 2, false, true, true),
                createMonster("Gobelin grand shaman", "Évolution rare d'un shaman gobelin, capable de maintenir une petite bande", Race::Gobelin, 12, 260, 22, 62, 84, 4, 2, false, true, true),
                createMonster("Hobgobelin garde du marché noir", "Protecteur brutal engagé pour ne pas poser de questions", Race::Hobgobelin, 13, 360, 30, 66, 88, 1, 1, false, true, true),
                createMonster("Médecin de bande douteux", "Support humain capable de sauver un allié mais pas toujours proprement", Race::Humain, 12, 235, 18, 52, 70, 4, 2, false, true, true),
                createMonster("Gobelin chef de sacs", "Gobelin organisé qui protège surtout le butin", Race::Gobelin, 11, 245, 24, 54, 72, 1, 1, false, true, true)
            };
        }

        if (biomeName == "Forêt ancienne")
        {
            return {
                createMonster("Gardien de ronces", "Protecteur végétal", Race::Plante, 12, 340, 25, 54, 70, 0, 1, false, true, true),
                createMonster("Esprit sylvestre", "Mémoire de forêt", Race::Esprit, 12, 290, 23, 60, 78, 0, 2, false, true, true),
                createMonster("Alpha de mousse", "Prédateur ancien", Race::Bete, 13, 360, 28, 56, 72, 0, 0, false, true),
                createMonster("Loup couvert de lichen", "Prédateur de forêt presque confondu avec le sol", Race::Bete, 12, 310, 27, 58, 76, 0, 0, false, true, true),
                createMonster("Racine étrangleuse", "Plante rare qui ralentit avant de frapper", Race::Plante, 13, 335, 22, 56, 74, 0, 1, false, true, true)
            };
        }

        if (biomeName == "Montagne froide")
        {
            return {
                createMonster("Yéti des falaises", "Prédateur des cimes", Race::Bete, 16, 430, 33, 66, 84, 0, 0, false, true, true),
                createMonster("Draconide froid majeur", "Draconide territorial", Race::Draconide, 17, 410, 36, 74, 98, 0, 1, false, true, true),
                createMonster("Élite rocheuse", "Construction de montagne", Race::Construction, 18, 520, 32, 67, 86, 0, 0, false, true, true)
            };
        }

        if (biomeName == "Marais trouble")
        {
            return {
                createMonster("Slime couronné", "Gelée dominante", Race::Slime, 24, 560, 36, 76, 98, 0, 1, false, true, true),
                createMonster("Slime doré engloutisseur", "Gelée rare attirée par ce qui brille", Race::Slime, 23, 500, 28, 72, 100, 0, 0, false, true, true),
                createMonster("Mage putride", "Sorcier des eaux sales", Race::Humain, 25, 430, 30, 90, 125, 2, 3, false, true, true),
                createMonster("Grand chamane de vase", "Soigneur marécageux capable de maintenir ses alliés debout", Race::Humain, 26, 455, 24, 84, 118, 3, 2, false, true, true),
                createMonster("Noyé ancien", "Mort-vivant de profondeur", Race::MortVivant, 26, 620, 39, 82, 105, 0, 0, false, true, true)
            };
        }


        if (biomeName == "Cimetière oublié")
        {
            return {
                createMonster("Chevalier du dernier serment", "Mort-vivant lourd et honorable", Race::MortVivant, 22, 560, 36, 78, 102, 0, 0, false, true, true),
                createMonster("Oracle de pierre tombale", "Support spectral qui comprend trop de choses", Race::Esprit, 23, 440, 28, 92, 128, 3, 2, false, true, true),
                createMonster("Ossuaire rampant", "Construction d'os assemblés", Race::Construction, 24, 700, 34, 76, 100, 0, 0, false, true, true),
                createMonster("Ombre de nom perdu", "Esprit rare lié aux identités effacées", Race::Esprit, 25, 500, 38, 105, 145, 0, 3, false, true, true),
                createMonster("Goule couronnée de poussière", "Prédateur mort-vivant de caveau noble", Race::MortVivant, 24, 620, 42, 88, 116, 0, 1, false, true, true)
            };
        }
        if (biomeName == "Ruines effondrées")
        {
            return {
                createMonster("Revenant ancien", "Mort-vivant noble", Race::MortVivant, 28, 620, 42, 88, 116, 0, 1, false, true, true),
                createMonster("Gardien ancien", "Sentinelle oubliée", Race::Construction, 30, 760, 44, 92, 120, 0, 0, false, true, true),
                createMonster("Anomalie instable", "Faille vivante", Race::AnomalieArcanique, 30, 650, 40, 105, 145, 0, 3, false, true, true),
                createMonster("Oracle fissuré", "Support arcanique des ruines", Race::Esprit, 29, 560, 34, 100, 138, 3, 2, false, true, true),
                createMonster("Scribe mort des ruines", "Mort-vivant faible mais chargé de runes dangereuses", Race::MortVivant, 24, 390, 24, 74, 102, 0, 2, false, true, true),
                createMonster("Sentinelle brisée", "Construction abîmée, lente, presque facile à lire mais très dure à casser", Race::Construction, 25, 780, 28, 72, 95, 0, 0, false, true, true)
            };
        }

        return createTierFivePlusMonsters();
    }

    Monster chooseBiomeMonster(const std::string& biomeName, int targetLevel, Random& random)
    {
        int roll = random.between(1, 100);
        std::vector<Monster> pool;

        if (roll <= 70)
        {
            pool = createBiomeCommonMonsters(biomeName);
        }
        else if (roll <= 90)
        {
            pool = createBiomeUnusualMonsters(biomeName);
        }
        else
        {
            pool = createBiomeRareMonsters(biomeName);
        }

        Monster monster = chooseFromList(pool, random);
        return scaleMonsterToTargetLevel(monster, targetLevel);
    }

    Monster scaleMonsterToTargetLevel(const Monster& baseMonster, int targetLevel)
    {
        if (targetLevel < 1)
        {
            targetLevel = 1;
        }

        int gap = targetLevel - baseMonster.getLevel();

        if (gap == 0)
        {
            return baseMonster;
        }

        int hpPercent = 100 + gap * 13;
        int damagePercent = 100 + gap * 9;

        if (hpPercent < 55) hpPercent = 55;
        if (damagePercent < 60) damagePercent = 60;

        return createMonster(
            baseMonster.getName(),
            baseMonster.getType(),
            baseMonster.getRace(),
            targetLevel,
            scaledValue(baseMonster.getMaxHp(), hpPercent),
            scaledValue(baseMonster.getMinDamage(), damagePercent),
            scaledValue(baseMonster.getMaxDamage(), damagePercent),
            scaledValue(baseMonster.getCriticalDamage(), damagePercent + 3),
            baseMonster.getHealingPotionCount(),
            baseMonster.getDamagePotionCount(),
            baseMonster.isInvocation(),
            baseMonster.isElite() || targetLevel >= baseMonster.getLevel() + 5,
            !baseMonster.areStatsVisible() || targetLevel >= baseMonster.getLevel() + 8,
            baseMonster.isEvolved()
        );
    }

}

// EN: createScaredGoblin declares or implements a focused behavior used by this module.
// FR: createScaredGoblin déclare ou implémente un comportement précis utilisé par ce module.
Monster MonsterCatalog::createScaredGoblin()
{
    return createMonster("Gobelin peureux", "Assassin primitif", Race::Gobelin, 1, 55, 8, 14, 18);
}

// EN: createBrutalGoblin declares or implements a focused behavior used by this module.
// FR: createBrutalGoblin déclare ou implémente un comportement précis utilisé par ce module.
Monster MonsterCatalog::createBrutalGoblin()
{
    return createMonster("Gobelin brutal", "Bagarreur sauvage", Race::Gobelin, 2, 75, 10, 18, 22);
}

// EN: createStarvingWolf declares or implements a focused behavior used by this module.
// FR: createStarvingWolf déclare ou implémente un comportement précis utilisé par ce module.
Monster MonsterCatalog::createStarvingWolf()
{
    return createMonster("Loup affamé", "Prédateur rapide", Race::Bete, 2, 65, 9, 20, 25);
}

// EN: createCrackedSkeleton declares or implements a focused behavior used by this module.
// FR: createCrackedSkeleton déclare ou implémente un comportement précis utilisé par ce module.
Monster MonsterCatalog::createCrackedSkeleton()
{
    return createMonster("Squelette fissuré", "Mort-vivant fragile", Race::MortVivant, 3, 80, 12, 22, 24);
}

// EN: createMinorOrc declares or implements a focused behavior used by this module.
// FR: createMinorOrc déclare ou implémente un comportement précis utilisé par ce module.
Monster MonsterCatalog::createMinorOrc()
{
    return createMonster("Orc mineur", "Combattant lourd", Race::Orc, 4, 120, 15, 26, 28, 0, 0, false, true);
}

// EN: createCaveBat declares or implements a focused behavior used by this module.
// FR: createCaveBat déclare ou implémente un comportement précis utilisé par ce module.
Monster MonsterCatalog::createCaveBat()
{
    return createMonster("Chauve-souris des cavernes", "Créature rapide", Race::Bete, 1, 45, 7, 15, 20);
}

// EN: createWildBoar declares or implements a focused behavior used by this module.
// FR: createWildBoar déclare ou implémente un comportement précis utilisé par ce module.
Monster MonsterCatalog::createWildBoar()
{
    return createMonster("Sanglier sauvage", "Bête résistante", Race::Bete, 3, 95, 12, 21, 24);
}

// EN: createLostBandit declares or implements a focused behavior used by this module.
// FR: createLostBandit déclare ou implémente un comportement précis utilisé par ce module.
Monster MonsterCatalog::createLostBandit()
{
    return createMonster("Bandit perdu", "Humain opportuniste", Race::Humain, 4, 100, 14, 24, 30, 1, 0);
}

// EN: createRandomMonsterForLevel declares or implements a focused behavior used by this module.
// FR: createRandomMonsterForLevel déclare ou implémente un comportement précis utilisé par ce module.
Monster MonsterCatalog::createRandomMonsterForLevel(int level, Random& random)
{
    if (level < 1)
    {
        level = 1;
    }

    Monster monster;

    if (level <= 1)
    {
        monster = chooseFromList(createTierOneMonsters(), random);
    }
    else if (level == 2)
    {
        monster = chooseFromList(createTierTwoMonsters(), random);
    }
    else if (level == 3)
    {
        monster = chooseFromList(createTierThreeMonsters(), random);
    }
    else if (level == 4)
    {
        monster = chooseFromList(createTierFourMonsters(), random);
    }
    else
    {
        monster = chooseFromList(createTierFivePlusMonsters(), random);
    }

    return scaleMonsterToTargetLevel(monster, level);
}


Monster MonsterCatalog::createRandomMonsterForBiome(const std::string& biomeName, int level, Random& random)
{
    if (level < 1)
    {
        level = 1;
    }

    return chooseBiomeMonster(biomeName, level, random);
}


// EN: createEvolvedVariant declares or implements a focused behavior used by this module.
// FR: createEvolvedVariant déclare ou implémente un comportement précis utilisé par ce module.
Monster MonsterCatalog::createEvolvedVariant(const Monster& baseMonster, Random& random)
{
    int levelBonus = random.between(1, 2);
    int hpPercent = baseMonster.isElite() ? random.between(130, 150) : random.between(120, 140);
    int damagePercent = baseMonster.isElite() ? random.between(120, 135) : random.between(112, 128);

    bool hiddenStats = !baseMonster.areStatsVisible() || random.between(1, 100) <= 18;
    bool elite = true;

    return createMonster(
        evolvedNameFor(baseMonster, random),
        baseMonster.getType() + " / créature évoluée",
        baseMonster.getRace(),
        baseMonster.getLevel() + levelBonus,
        scaledValue(baseMonster.getMaxHp(), hpPercent),
        scaledValue(baseMonster.getMinDamage(), damagePercent),
        scaledValue(baseMonster.getMaxDamage(), damagePercent),
        scaledValue(baseMonster.getCriticalDamage(), damagePercent + 5),
        baseMonster.getHealingPotionCount() + (random.between(1, 100) <= 25 ? 1 : 0),
        baseMonster.getDamagePotionCount() + (random.between(1, 100) <= 35 ? 1 : 0),
        baseMonster.isInvocation(),
        elite,
        hiddenStats,
        true
    );
}

std::vector<Monster> MonsterCatalog::createAllPreviewMonsters()
{
    std::vector<Monster> allMonsters;

    std::vector<std::vector<Monster>> tiers = {
        createTierOneMonsters(),
        createTierTwoMonsters(),
        createTierThreeMonsters(),
        createTierFourMonsters(),
        createTierFivePlusMonsters()
    };

    for (const std::vector<Monster>& tier : tiers)
    {
        for (const Monster& monster : tier)
        {
            allMonsters.push_back(monster);
        }
    }

    std::vector<std::string> biomes = {
        "Plaine sauvage",
        "Route commerciale",
        "Mares gélatineuses",
        "Forêt ancienne",
        "Montagne froide",
        "Marais trouble",
        "Cimetière oublié",
        "Ruines effondrées"
    };

    for (const std::string& biome : biomes)
    {
        std::vector<std::vector<Monster>> biomePools = {
            createBiomeCommonMonsters(biome),
            createBiomeUnusualMonsters(biome),
            createBiomeRareMonsters(biome)
        };

        for (const std::vector<Monster>& pool : biomePools)
        {
            for (const Monster& monster : pool)
            {
                allMonsters.push_back(monster);
            }
        }
    }

    return allMonsters;
}

// EN: displayAvailableMonsters declares or implements a focused behavior used by this module.
// FR: displayAvailableMonsters déclare ou implémente un comportement précis utilisé par ce module.
std::vector<std::string> MonsterCatalog::getAvailableMonsterLines()
{
    std::vector<std::string> lines;
    std::vector<Monster> monsters = createAllPreviewMonsters();

    for (std::size_t i = 0; i < monsters.size(); ++i)
    {
        lines.push_back(
            std::to_string(i + 1) + " : " + monsters[i].getName()
            + " | Race : " + monsters[i].getRaceText()
            + " | Niveau : " + std::to_string(monsters[i].getLevel())
        );
    }

    return lines;
}


// EN: displayAvailableMonsters declares or implements a focused behavior used by this module.
// FR: displayAvailableMonsters déclare ou implémente un comportement précis utilisé par ce module.
void MonsterCatalog::displayAvailableMonsters()
{
    MessageScreen::show("MONSTRES PRÉPARÉS", "catalog.monsters.available", getAvailableMonsterLines(), false);
}
