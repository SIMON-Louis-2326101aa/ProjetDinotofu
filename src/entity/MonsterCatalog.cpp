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

    Monster createSplittingSlime(
        const std::string& name,
        const std::string& type,
        int level,
        int maxHp,
        int minDamage,
        int maxDamage,
        int criticalDamage,
        int splitMinCount,
        int splitMaxCount,
        const std::string& childName,
        bool elite = false,
        bool hiddenStats = false,
        bool evolved = false
    )
    {
        Monster slime = createMonster(
            name,
            type,
            Race::Slime,
            level,
            maxHp,
            minDamage,
            maxDamage,
            criticalDamage,
            0,
            0,
            false,
            elite,
            hiddenStats,
            evolved
        );
        slime.configureSplitOnDeath(splitMinCount, splitMaxCount, childName);
        return slime;
    }

    std::vector<Monster> createTierOneMonsters()
    {
        return {
            createMonster("Gobelin peureux", "Assassin primitif", Race::Gobelin, 1, 55, 8, 14, 18),
            createMonster("Chauve-souris des cavernes", "Créature rapide", Race::Bete, 1, 45, 7, 15, 20),
            createMonster("Rat géant", "Bête nuisible", Race::Bete, 1, 50, 6, 13, 18),
            createMonster("Slime fragile", "Gelée vivante", Race::Slime, 1, 60, 4, 12, 16),
            createMonster("Racine agitée", "Plante hostile", Race::Plante, 1, 70, 5, 14, 18),
            createMonster("Kobold paniqué", "Petit draconide", Race::Draconide, 1, 58, 7, 15, 20),
            createMonster("Moustique des hautes herbes", "Insectoïde minuscule mais pénible", Race::Insectoide, 1, 36, 4, 10, 14),
            createMonster("Pousse mordante", "Plante jeune qui mord les bottes", Race::Plante, 1, 52, 4, 11, 15),
            createMonster("Rat de grenier nerveux", "Bête fragile", Race::Bete, 1, 34, 1, 5, 9),
            createMonster("Petit gobelin taxeur", "Gobelin voleur", Race::Gobelin, 1, 42, 2, 7, 12),
            createMonster("Slime de flaque froide", "Slime froid", Race::Slime, 1, 48, 1, 6, 10),
            createMonster("Éclat de bougie arcanique", "Élémentaire mineur", Race::Elementaire, 1, 36, 3, 8, 14),
            createMonster("Chien errant affamé", "Bête affamée", Race::Bete, 1, 55, 3, 9, 15)
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
            createMonster("Méphaïte de braise", "Petit élémentaire", Race::Elementaire, 2, 72, 9, 21, 26),
            createMonster("Renard cendré", "Bête vive attirée par les feux de camp", Race::Bete, 2, 60, 8, 19, 25),
            createMonster("Slime rose nerveux", "Gelée bondissante imprévisible", Race::Slime, 2, 66, 7, 16, 22),
            createSplittingSlime("Gros slime fusionné", "Plusieurs petites gelées soudées ensemble", 2, 118, 5, 15, 21, 3, 4, "Petit slime de fusion"),
            createMonster("Archer gobelin trop fier", "Gobelin archer", Race::Gobelin, 2, 92, 7, 17, 26),
            createMonster("Loup à crocs ternes", "Bête de meute", Race::Bete, 2, 105, 8, 18, 28),
            createMonster("Pilleur au bouclier fendu", "Humanoïde pillard", Race::Humain, 2, 115, 7, 19, 30, 1, 0),
            createMonster("Essaim d'insectes de cave", "Nuée fragile", Race::Insectoide, 2, 84, 5, 20, 31),
            createMonster("Racine étrangleuse jeune", "Plante entravante", Race::Plante, 2, 125, 6, 17, 27)
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
            createMonster("Alchimiste renégat", "Humain dangereux", Race::Humain, 3, 88, 6, 18, 28, 1, 2),
            createMonster("Frondeur gobelin", "Gobelin à distance", Race::Gobelin, 3, 76, 9, 23, 32),
            createMonster("Racine fouetteuse", "Plante agressive", Race::Plante, 3, 100, 8, 21, 28),
            createMonster("Garde-hache hobgobelin", "Humanoïde discipliné", Race::Hobgobelin, 3, 185, 13, 29, 42, 1, 0, false, true),
            createMonster("Chevalier sans bannière", "Humanoïde armuré", Race::Humain, 3, 210, 11, 28, 43, 1, 0, false, true),
            createMonster("Fée des ronces piquantes", "Fée hostile", Race::Fee, 3, 150, 12, 31, 49, 0, 1),
            createMonster("Draconide à écailles grises", "Draconide jeune", Race::Draconide, 3, 230, 12, 30, 45, 0, 1, false, true),
            createMonster("Araignée verrière", "Insectoïde rapide", Race::Insectoide, 3, 175, 10, 34, 51)
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
            createMonster("Anomalie arcanique instable", "Erreur magique", Race::AnomalieArcanique, 4, 110, 12, 32, 46, 0, 1, false, false, true),
            createMonster("Armure cabossée", "Construction hantée faible", Race::Construction, 4, 145, 10, 24, 32, 0, 0, false, true),
            createMonster("Brigand au couteau", "Humain hostile rapide", Race::Humain, 4, 96, 15, 27, 38, 1, 0),
            createMonster("Capitaine orc balafré", "Mini-boss brutal", Race::Orc, 4, 310, 20, 45, 68, 0, 1, false, true, true),
            createMonster("Golem de forge instable", "Construction brûlante", Race::Construction, 4, 340, 17, 42, 61, 0, 0, false, true, true),
            createMonster("Spectre de serment brisé", "Esprit vengeur", Race::Esprit, 4, 255, 18, 47, 73, 0, 1, false, true, true),
            createMonster("Serpent d'écailles noires", "Draconide toxique", Race::Draconide, 4, 290, 21, 46, 70, 0, 1, false, true, true),
            createMonster("Ruche mutée des anciennes mines", "Insectoïde évolué", Race::Insectoide, 4, 275, 16, 50, 76, 0, 0, false, true, true)
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
            createMonster("Dragon mineur", "Jeune menace draconique", Race::Dragon, 8, 320, 25, 55, 75, 0, 2, false, true, true),
            createMonster("Mage de verre", "Humain arcanique fragile", Race::Humain, 6, 118, 14, 44, 68, 1, 2, false, true, true),
            createMonster("Araignée cuirassée", "Insectoïde protégé", Race::Insectoide, 6, 185, 18, 36, 48, 0, 0, false, true),
            createMonster("Golem d'atelier", "Construction usée", Race::Construction, 7, 310, 19, 41, 54, 0, 0, false, true, true),
            createMonster("Avatar mineur d'une dette oubliée", "Anomalie contractuelle", Race::AnomalieArcanique, 7, 430, 26, 65, 100, 0, 2, false, true, true),
            createMonster("Dragonnet d'obsidienne adulte", "Dragon sombre", Race::Dragon, 8, 520, 28, 70, 108, 0, 2, false, true, true),
            createMonster("Paladin déchu du vieux serment", "Humanoïde sacré corrompu", Race::Humain, 7, 480, 24, 62, 95, 2, 1, false, true, true),
            createMonster("Ange de registre cassé", "Ange déformé", Race::Ange, 8, 455, 27, 68, 106, 1, 2, false, true, true),
            createMonster("Bête lunaire sans ombre", "Bête onirique", Race::Bete, 8, 500, 30, 73, 115, 0, 1, false, true, true),
            createMonster("Loup de cendre blanche", "Prédateur marqué par une ancienne braise", Race::Bete, 6, 210, 24, 46, 62, 0, 1, false, true, true),
            createMonster("Garde-lame hobgobelin", "Soldat organisé de route", Race::Hobgobelin, 6, 205, 20, 43, 58, 1, 1, false, true, true),
            createMonster("Écorceuse ancienne", "Plante qui râpe les armures", Race::Plante, 7, 240, 19, 42, 60, 0, 1, false, true, true),
            createMonster("Moineau de verre", "Bête fragile aux ailes coupantes", Race::Bete, 5, 120, 22, 48, 68, 0, 0, false, true, true),
            createMonster("Scribe gobelin taxeur", "Gobelin qui soutient les voleurs par des dettes absurdes", Race::Gobelin, 6, 150, 12, 34, 50, 2, 1, false, true, true),
            createMonster("Éclat de sentinelle", "Petit reste de construction encore actif", Race::Construction, 6, 260, 17, 38, 52, 0, 0, false, true, true),
            createMonster("Démon de poche contrarié", "Petit démon invoqué trop vite", Race::Demon, 7, 230, 24, 50, 75, 0, 2, false, true, true),
            createMonster("Larve de ruine", "Insectoïde nourri par poussière arcanique", Race::Insectoide, 6, 185, 21, 44, 60, 0, 1, false, true, true)
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
                createMonster("Renard fouilleur", "Petite bête attirée par les sacs mal fermés", Race::Bete, 2, 52, 6, 16, 22),
                createMonster("Mule sauvage effrayée", "Bête de route devenue agressive", Race::Bete, 2, 84, 5, 15, 20),
                createMonster("Épouvantail ensorcelé", "Construction de champ animée", Race::Construction, 3, 92, 8, 20, 26)
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
                createMonster("Mini-ruche gélatineuse", "Petit amas de slimes faibles qui se déplacent ensemble", Race::Slime, 6, 150, 7, 24, 34, 0, 0, false, true),
                createSplittingSlime("Slime fusionné instable", "Masse gélatineuse gonflée par plusieurs noyaux", 6, 188, 8, 27, 38, 3, 5, "Petit slime instable", true)
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

        if (biomeName == "Bocage aux lanternes")
        {
            return {
                createMonster("Champignon-lanterne peureux", "Fonge lumineuse qui fuit mal", Race::Plante, 8, 145, 11, 30, 42, 0, 1, false, true),
                createMonster("Loup aux spores froides", "Prédateur couvert de poussière claire", Race::Bete, 9, 190, 19, 40, 54, 0, 0, false, true),
                createMonster("Racine phosphorescente", "Plante lente qui éclaire son propre piège", Race::Plante, 8, 175, 13, 34, 46, 0, 1, false, true),
                createMonster("Slime nacré de mousse", "Gelée claire nourrie par les champignons", Race::Slime, 8, 165, 14, 36, 50, 0, 1, false, true),
                createMonster("Papillon de veille", "Insectoïde lumineux attiré par les blessures", Race::Insectoide, 9, 130, 18, 44, 62, 0, 1, false, true)
            };
        }

        if (biomeName == "Désert d'argile rouge")
        {
            return {
                createMonster("Scarabée d'argile", "Insectoïde sec à carapace rouge", Race::Insectoide, 10, 220, 18, 42, 56, 0, 0, false, true),
                createMonster("Chacal poussiéreux", "Bête de piste rapide", Race::Bete, 10, 185, 23, 50, 66, 0, 0, false, true),
                createMonster("Slime salin", "Gelée pâle desséchante", Race::Slime, 11, 210, 17, 46, 64, 0, 1, false, true, true),
                createMonster("Pilleur de dunes", "Humain habitué aux fausses oasis", Race::Humain, 11, 210, 22, 48, 66, 1, 1, false, true),
                createMonster("Totem fissuré", "Construction sèche mal réveillée", Race::Construction, 11, 270, 20, 44, 58, 0, 0, false, true)
            };
        }

        if (biomeName == "Quartier abandonné")
        {
            return {
                createMonster("Rat de grenier armé", "Bête urbaine trop habituée aux humains", Race::Bete, 8, 150, 17, 38, 52, 0, 0, false, true),
                createMonster("Voleur de ruelle", "Humain opportuniste des maisons vides", Race::Humain, 9, 170, 20, 44, 60, 1, 0, false, true),
                createMonster("Gobelin serrurier", "Gobelin qui ouvre surtout ce qu'il casse", Race::Gobelin, 9, 160, 16, 40, 56, 1, 1, false, true),
                createMonster("Mannequin animé", "Construction de boutique abandonnée", Race::Construction, 10, 230, 18, 42, 58, 0, 0, false, true),
                createMonster("Corbeau de toiture", "Bête voleuse de petits objets brillants", Race::Bete, 8, 120, 19, 46, 62, 0, 0, false, true)
            };
        }

        if (biomeName == "Mine sifflante")
        {
            return {
                createMonster("Taupe foreuse nerveuse", "Bête de galerie agressive", Race::Bete, 14, 260, 24, 52, 68, 0, 0, false, true),
                createMonster("Golem de rails", "Construction bricolée avec des rails tordus", Race::Construction, 15, 390, 24, 48, 64, 0, 0, false, true, true),
                createMonster("Slime de charbon froid", "Gelée sombre chargée de poussière minérale", Race::Slime, 14, 285, 20, 50, 70, 0, 1, false, true, true),
                createMonster("Gobelin contremaître", "Gobelin qui crie plus qu'il ne travaille", Race::Gobelin, 14, 245, 22, 52, 72, 2, 1, false, true, true),
                createMonster("Éclat de fer vivant", "Petit élémentaire métallique", Race::Elementaire, 15, 250, 25, 58, 78, 0, 2, false, true, true)
            };
        }

        if (biomeName == "Verger des lucioles de fer")
        {
            return {
                createMonster("Luciole de fer solitaire", "Insecte métallique lumineux", Race::Insectoide, 6, 125, 14, 32, 44, 0, 1, false, true),
                createMonster("Mite lumineuse fragile", "Insecte clair attiré par les armes brillantes", Race::Insectoide, 6, 105, 16, 38, 52, 0, 1, false, true),
                createMonster("Renard voleur de fruits", "Bête rapide qui fuit avec les sacs mal fermés", Race::Bete, 7, 150, 19, 40, 54, 0, 0, false, true),
                createMonster("Épouvantail à lanternes", "Construction de verger trop consciencieuse", Race::Construction, 8, 210, 16, 38, 50, 0, 1, false, true, true),
                createMonster("Racine de pommier-lampe", "Plante nocturne qui accroche les bottes", Race::Plante, 7, 180, 13, 34, 46, 0, 1, false, true)
            };
        }

        if (biomeName == "Archives noyées")
        {
            return {
                createMonster("Rat de registre humide", "Bête urbaine nourrie au papier", Race::Bete, 12, 205, 22, 48, 64, 0, 0, false, true),
                createMonster("Slime d'encre diluée", "Gelée noire chargée de vieux contrats", Race::Slime, 12, 245, 18, 48, 66, 0, 1, false, true, true),
                createMonster("Scribe noyé", "Mort-vivant qui protège encore ses pages", Race::MortVivant, 13, 270, 21, 52, 70, 1, 1, false, true, true),
                createMonster("Reliure mordante", "Livre animé qui refuse les emprunts", Race::Construction, 13, 225, 24, 55, 74, 0, 2, false, true, true),
                createMonster("Greffier de vase", "Humanoïde de bureau devenu beaucoup trop liquide", Race::Humain, 14, 240, 19, 54, 78, 2, 1, false, true, true)
            };
        }

        if (biomeName == "Falaises des drakes gris")
        {
            return {
                createMonster("Chèvre de corniche hostile", "Bête têtue qui pousse vers le vide", Race::Bete, 18, 340, 34, 72, 96, 0, 0, false, true, true),
                createMonster("Harpie grise des cordes", "Bête humanoïde qui coupe les ancrages", Race::Bete, 18, 285, 38, 86, 118, 0, 1, false, true, true),
                createMonster("Drake gris jeune", "Draconide territorial encore trop confiant", Race::Draconide, 19, 430, 40, 88, 120, 0, 2, false, true, true),
                createMonster("Éboulis vivant", "Construction naturelle qui déteste les pas lourds", Race::Construction, 18, 470, 32, 74, 100, 0, 0, false, true, true),
                createMonster("Vautour de falaise", "Bête patiente, donc insultante", Race::Bete, 18, 260, 37, 84, 112, 0, 1, false, true, true)
            };
        }

        if (biomeName == "Foire abandonnée")
        {
            return {
                createMonster("Pantin de stand grinçant", "Construction légère qui applaudit mal", Race::Construction, 10, 210, 21, 48, 66, 0, 1, false, true, true),
                createMonster("Rat jongleur agressif", "Bête urbaine trop douée avec les objets volés", Race::Bete, 10, 165, 25, 56, 76, 0, 0, false, true),
                createMonster("Forain creux", "Humain ou souvenir d'humain sous maquillage fissuré", Race::Humain, 11, 220, 24, 58, 82, 1, 1, false, true, true),
                createMonster("Slime confetti", "Gelée colorée qui laisse des preuves partout", Race::Slime, 10, 200, 19, 52, 74, 0, 1, false, true, true),
                createMonster("Miroir rieur fendu", "Petit esprit de foire qui imite le joueur", Race::Esprit, 11, 185, 21, 66, 92, 0, 2, false, true, true)
            };
        }


        if (biomeName == "Temple des cloches fendues")
        {
            return {
                createMonster("Rat de sacristie", "Bête nerveuse attirée par la cire", Race::Bete, 15, 210, 22, 44, 58, 0, 0, false, true),
                createMonster("Novice fantôme", "Esprit de sanctuaire incomplet", Race::Esprit, 16, 220, 20, 56, 74, 1, 1, false, true, true),
                createMonster("Gardien de nef fissuré", "Construction sacrée abîmée", Race::Construction, 17, 330, 24, 52, 68, 0, 0, false, true, true)
            };
        }

        if (biomeName == "Canaux de brume bleue")
        {
            return {
                createMonster("Anguille de brume pâle", "Bête d'eau presque invisible", Race::Bete, 9, 135, 16, 36, 50, 0, 0, false, false, true),
                createMonster("Voleur de quai mouillé", "Humain opportuniste", Race::Humain, 10, 160, 20, 42, 58, 1, 0, false, true),
                createMonster("Slime d'eau pâle", "Gelée froide des canaux", Race::Slime, 10, 185, 15, 38, 52, 0, 0, false, true)
            };
        }

        if (biomeName == "Carrière des os blancs")
        {
            return {
                createMonster("Scarabée d'os", "Insectoïde pâle et bruyant", Race::Insectoide, 20, 300, 30, 62, 82, 0, 0, false, true),
                createMonster("Golem de craie jeune", "Construction blanche friable", Race::Construction, 21, 420, 28, 60, 78, 0, 0, false, true, true),
                createMonster("Mineur pâle", "Humanoïde perdu dans la poussière", Race::Humain, 20, 280, 32, 66, 88, 1, 0, false, true, true)
            };
        }

        if (biomeName == "Marché sous les ponts")
        {
            return {
                createMonster("Chien de quai nerveux", "Bête dressée pour flairer les dettes", Race::Bete, 12, 180, 24, 50, 68, 0, 0, false, true),
                createMonster("Gobelin prêteur", "Gobelin sûr de ses taux", Race::Gobelin, 12, 170, 20, 48, 70, 1, 1, false, true),
                createMonster("Contrebandier à cape sale", "Humain armé d'excuses et d'une lame", Race::Humain, 13, 210, 27, 58, 76, 1, 0, false, true)
            };
        }

        if (biomeName == "Jardin des statues qui pleurent")
        {
            return {
                createMonster("Oiseau de pierre", "Construction légère perchée sur les grilles", Race::Construction, 14, 205, 24, 54, 72, 0, 0, false, true, true),
                createMonster("Ronce blanche", "Plante noble devenue hostile", Race::Plante, 15, 260, 20, 56, 75, 0, 1, false, true, true),
                createMonster("Statue fissurée", "Gardien de jardin presque immobile", Race::Construction, 16, 380, 25, 58, 76, 0, 0, false, true, true)
            };
        }

        if (biomeName == "Bois de la Corruption")
        {
            return {
                createMonster("Ronce noire rampante", "Plante corrompue qui cherche les chevilles", Race::Plante, 18, 310, 26, 68, 92, 0, 1, false, true, true),
                createMonster("Loup au museau d'encre", "Bête contaminée par le bois sombre", Race::Bete, 19, 285, 36, 78, 105, 0, 0, false, true, true),
                createMonster("Esprit collant", "Mémoire sombre qui s'accroche aux vivants", Race::Esprit, 20, 250, 28, 86, 120, 0, 2, false, true, true)
            };
        }

        if (biomeName == "Crypte du Sombre-Lien")
        {
            return {
                createMonster("Squelette lié", "Mort-vivant attaché à un vieux nom", Race::MortVivant, 20, 330, 32, 72, 96, 0, 0, false, true, true),
                createMonster("Cultiste pâle", "Humain qui a trop lu les murs", Race::Humain, 21, 290, 26, 90, 126, 2, 2, false, true, true),
                createMonster("Chaîne d'ombre rampante", "Construction d'ombre plus têtue que solide", Race::Construction, 22, 390, 28, 82, 110, 0, 1, false, true, true)
            };
        }

        if (biomeName == "Désert des Protecteurs")
        {
            return {
                createMonster("Scarabée sacré", "Insectoïde doré du sable antique", Race::Insectoide, 22, 320, 34, 78, 104, 0, 1, false, true),
                createMonster("Chacal de sable blanc", "Bête rapide qui tourne autour des statues", Race::Bete, 23, 310, 40, 88, 118, 0, 0, false, true),
                createMonster("Gardien fissuré", "Protecteur ancien encore fonctionnel", Race::Construction, 24, 500, 34, 82, 110, 0, 0, false, true, true)
            };
        }

        if (biomeName == "Sanctuaire antique des Veilleurs")
        {
            return {
                createMonster("Golem de seuil", "Construction qui juge les entrées", Race::Construction, 24, 520, 36, 84, 112, 0, 1, false, true, true),
                createMonster("Novice spectral antique", "Esprit de prière resté en service", Race::Esprit, 25, 330, 28, 98, 135, 1, 2, false, true, true),
                createMonster("Sentinelle antique", "Gardien lent mais précis", Race::Construction, 26, 560, 40, 90, 120, 0, 0, false, true, true)
            };
        }

        if (biomeName == "Quartier des Lames Muettes")
        {
            return {
                createMonster("Éclaireur masqué", "Assassin junior qui compte les fenêtres", Race::Humain, 28, 360, 48, 112, 150, 1, 1, false, true, true),
                createMonster("Chien d'ombre dressé", "Bête silencieuse trop bien entraînée", Race::Bete, 29, 390, 52, 118, 158, 0, 0, false, true, true),
                createMonster("Voleur sans écho", "Humain léger qui refuse les combats honnêtes", Race::Humain, 30, 350, 46, 126, 178, 1, 2, false, true, true)
            };
        }

        if (biomeName == "Toits des Assassins")
        {
            return {
                createMonster("Archer de toit", "Tireur patient des corniches", Race::Humain, 30, 370, 50, 130, 182, 1, 2, false, true, true),
                createMonster("Coureur masqué", "Humain rapide qui ne descend jamais où il faut", Race::Humain, 31, 360, 55, 124, 170, 1, 1, false, true, true),
                createMonster("Corbeau dressé au poison", "Bête de repérage malveillante", Race::Bete, 30, 290, 48, 118, 165, 0, 2, false, false, true)
            };
        }

        if (biomeName == "Nid draconique rouge")
        {
            return {
                createMonster("Kobold rouge nerveux", "Serviteur de couvée", Race::Draconide, 36, 520, 58, 140, 190, 0, 1, false, true, true),
                createMonster("Draconide jeune de braise", "Sang draconique encore instable", Race::Draconide, 37, 650, 64, 150, 205, 0, 2, false, true, true),
                createMonster("Lézard de cendre lourde", "Bête couverte de plaques chaudes", Race::Bete, 36, 700, 58, 132, 180, 0, 0, false, true, true)
            };
        }

        if (biomeName == "Coulées de lave noire")
        {
            return {
                createMonster("Slime de lave sombre", "Gelée brûlante et lourde", Race::Slime, 38, 620, 52, 150, 210, 0, 2, false, true, true),
                createMonster("Élémentaire de braise noire", "Feu compact sous forme presque humaine", Race::Elementaire, 39, 610, 58, 165, 230, 0, 3, false, true, true),
                createMonster("Golem de basalte vivant", "Construction volcanique", Race::Construction, 40, 850, 64, 150, 205, 0, 1, false, true, true)
            };
        }

        if (biomeName == "Glacier des Serments froids")
        {
            return {
                createMonster("Loup de givre juré", "Bête gardienne du froid", Race::Bete, 38, 560, 62, 152, 210, 0, 1, false, true, true),
                createMonster("Chevalier gelé", "Mort-vivant figé dans son dernier ordre", Race::MortVivant, 39, 760, 58, 148, 200, 0, 1, false, true, true),
                createMonster("Slime blanc cristallin", "Gelée froide qui conserve les chocs", Race::Slime, 38, 640, 50, 140, 196, 0, 2, false, true, true)
            };
        }

        if (biomeName == "Bosquet des Fées du Mana")
        {
            return {
                createMonster("Fée joueuse", "Semi-humaine minuscule qui teste les réactions", Race::Fee, 45, 460, 54, 190, 265, 3, 4, false, true, true),
                createMonster("Plante de mana bleue", "Végétal saturé de magie douce", Race::Plante, 46, 720, 48, 170, 238, 1, 4, false, true, true),
                createMonster("Luciole de mana", "Insectoïde lumineux difficile à viser", Race::Insectoide, 45, 390, 60, 182, 255, 0, 3, false, false, true)
            };
        }

        if (biomeName == "Sanctuaire kitsuné des Neuf Étincelles")
        {
            return {
                createMonster("Kitsuné mineur", "Renard-esprit à demi-visible", Race::Kitsune, 46, 520, 64, 188, 260, 2, 4, false, true, true),
                createMonster("Renard de flamme fine", "Bête magique qui mord les illusions", Race::Bete, 47, 560, 70, 176, 245, 0, 3, false, true, true),
                createMonster("Lanterne d'illusion", "Esprit qui marche devant son propre feu", Race::Esprit, 46, 480, 54, 200, 280, 1, 4, false, true, true)
            };
        }

        if (biomeName == "Confluence du Mana pur")
        {
            return {
                createMonster("Élémentaire mineur prismatique", "Énergie brute aux couleurs instables", Race::Elementaire, 50, 660, 68, 210, 295, 0, 5, false, true, true),
                createMonster("Slime prismatique", "Gelée capable de changer de nuance en combat", Race::Slime, 51, 740, 58, 188, 270, 0, 4, false, true, true),
                createMonster("Anomalie douce", "Faille presque polie, donc suspecte", Race::AnomalieArcanique, 52, 620, 62, 230, 320, 1, 5, false, true, true)
            };
        }

        if (biomeName == "Bastion majeur scellé")
        {
            return {
                createMonster("Sentinelle majeure", "Gardien de forteresse scellée", Race::Construction, 60, 1200, 90, 260, 360, 0, 3, false, true, true),
                createMonster("Chevalier du sceau", "Mort-vivant royal qui attend son chapitre", Race::MortVivant, 61, 1050, 95, 280, 385, 0, 3, false, true, true),
                createMonster("Témoin muet", "Esprit observateur qui punit les mensonges", Race::Esprit, 62, 840, 72, 310, 430, 2, 5, false, true, true)
            };
        }

        if (biomeName == "Archipel des îles flottantes")
        {
            return {
                createMonster("Harpie haute", "Bête semi-humaine des vents d'altitude", Race::Bete, 70, 920, 115, 340, 475, 1, 4, false, true, true),
                createMonster("Slime de nuage", "Gelée légère qui refuse de tomber", Race::Slime, 71, 1000, 92, 300, 420, 0, 5, false, true, true),
                createMonster("Pierre éveillée flottante", "Construction naturelle qui a appris la gravité à l'envers", Race::Construction, 72, 1500, 100, 315, 440, 0, 3, false, true, true)
            };
        }

        if (biomeName == "Ponts translucides de mana")
        {
            return {
                createMonster("Gardien de pont bleu", "Construction de mana presque transparente", Race::Construction, 72, 1350, 105, 330, 460, 0, 4, false, true, true),
                createMonster("Reflet de voyageur", "Esprit copiant les hésitations", Race::Esprit, 73, 880, 88, 370, 520, 2, 6, false, true, true),
                createMonster("Élémentaire d'air dense", "Vent assez compact pour te frapper", Race::Elementaire, 74, 980, 120, 360, 500, 0, 5, false, true, true)
            };
        }

        if (biomeName == "Cieux des Légendes")
        {
            return {
                createMonster("Écho héroïque", "Souvenir de combattant devenu réel", Race::Esprit, 90, 1300, 150, 520, 730, 2, 7, false, true, true),
                createMonster("Ange mineur de récit", "Messager lié à une légende locale", Race::Ange, 91, 1450, 135, 500, 700, 3, 6, false, true, true),
                createMonster("Constellation armée", "Lumière ancienne tenant une arme trop nette", Race::Elementaire, 92, 1600, 140, 550, 770, 0, 8, false, true, true)
            };
        }

        if (biomeName == "Parvis des Divinités")
        {
            return {
                createMonster("Messager céleste", "Ange gardien du seuil supérieur", Race::Ange, 100, 1700, 165, 620, 870, 4, 8, false, true, true),
                createMonster("Statue vivante divine", "Construction sacrée presque indifférente", Race::Construction, 102, 2300, 150, 570, 800, 1, 6, false, true, true),
                createMonster("Gardien de seuil céleste", "Entité qui ne confond pas courage et permission", Race::Aasimar, 101, 1900, 155, 600, 850, 3, 7, false, true, true)
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
                createMonster("Chauve-souris de passage", "Créature rapide", Race::Bete, 2, 62, 8, 19, 26),
                createMonster("Renard des clôtures", "Bête rusée qui mord puis fuit", Race::Bete, 3, 78, 12, 25, 34),
                createMonster("Gobelin cueilleur de poches", "Petit voleur qui vise surtout les sacs ouverts", Race::Gobelin, 3, 70, 10, 24, 32),
                createMonster("Ronce de sentier", "Plante faible mais gênante", Race::Plante, 3, 92, 7, 18, 24)
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
                createMonster("Slime rouge bouillonnant", "Gelée brûlante qui punit les contacts directs", Race::Slime, 11, 220, 18, 46, 64, 0, 1, false, true, true),
                createMonster("Slime cendré", "Gelée refroidie par d'anciennes braises", Race::Slime, 10, 210, 17, 41, 56, 0, 1, false, true, true),
                createMonster("Slime opalin", "Gelée claire qui réfléchit mal la lumière", Race::Slime, 11, 225, 15, 45, 60, 0, 1, false, true, true),
                createMonster("Slime de savon gris", "Gelée glissante issue d'un vieux lavoir", Race::Slime, 9, 180, 12, 34, 46, 0, 0, false, true),
                createMonster("Slime carmin coagulé", "Gelée rouge épaisse qui colle aux blessures", Race::Slime, 11, 230, 18, 47, 66, 0, 1, false, true, true),
                createMonster("Bulle-mère gélatineuse", "Petite ruche de bulles vivantes", Race::Slime, 12, 260, 14, 43, 62, 0, 1, false, true, true),
                createSplittingSlime("Masse gélatineuse fusionnée", "Gros slime lourd dont les noyaux se disputent le même corps", 12, 340, 15, 46, 66, 4, 5, "Petit slime fusionné", true, false, true)
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
                createMonster("Bandit apothicaire", "Humain de route avec quelques fioles douteuses", Race::Humain, 7, 132, 12, 31, 46, 2, 2, false, true),
                createMonster("Porteur de coffre piégé", "Humain qui protège un butin suspect", Race::Humain, 6, 150, 13, 30, 42, 1, 1, false, true),
                createMonster("Chien de convoi", "Bête dressée pour garder les roues", Race::Bete, 5, 130, 16, 33, 44, 0, 0, false, true),
                createMonster("Faux péager souriant", "Humain qui a inventé sa propre loi", Race::Humain, 6, 148, 15, 36, 50, 1, 1, false, true),
                createMonster("Hobgobelin comptable", "Organisateur de butin qui protège les sacs", Race::Hobgobelin, 7, 165, 12, 31, 45, 2, 1, false, true, true),
                createMonster("Mule mordante", "Bête de convoi traumatisée par les bandits", Race::Bete, 5, 150, 15, 34, 44, 0, 0, false, true)
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
                createMonster("Liane guetteuse", "Plante fine qui préfère immobiliser avant de blesser", Race::Plante, 7, 150, 11, 33, 45, 0, 1, false, true),
                createMonster("Renarde kitsune sauvage", "Bête illusionnée par une vieille braise", Race::Bete, 8, 168, 18, 42, 58, 0, 1, false, true, true),
                createMonster("Abeille de sève", "Insectoïde forestier attiré par les plaies", Race::Insectoide, 7, 128, 17, 36, 49, 0, 1, false, true),
                createMonster("Biche aux yeux verts", "Bête paisible devenue dangereuse si acculée", Race::Bete, 7, 175, 16, 35, 48, 0, 0, false, true),
                createMonster("Mousse rampante", "Plante lente qui recouvre les bottes", Race::Plante, 8, 210, 10, 30, 42, 0, 1, false, true),
                createMonster("Pixie des épines", "Petite fée agressive des ronces", Race::Fee, 8, 125, 18, 44, 62, 1, 2, false, true, true)
            };
        }

        if (biomeName == "Montagne froide")
        {
            return {
                createMonster("Golem fissuré", "Construction rocheuse", Race::Construction, 10, 300, 22, 44, 58, 0, 0, false, true, true),
                createMonster("Chasseur des cimes", "Humanoïde montagnard", Race::Humain, 9, 190, 21, 42, 56, 1, 1, false, true),
                createMonster("Méphaïte de givre", "Petit élémentaire", Race::Elementaire, 8, 160, 18, 41, 54, 0, 1, false, true, true),
                createMonster("Bouc des neiges agressif", "Bête de falaise qui charge plus qu'elle ne réfléchit", Race::Bete, 9, 205, 21, 43, 58, 0, 0, false, true),
                createMonster("Golem d'éboulis", "Construction instable qui frappe fort mais se fissure", Race::Construction, 11, 335, 25, 50, 66, 0, 0, false, true, true),
                createMonster("Harpie des neiges", "Bête humanoïde de falaise", Race::Bete, 10, 188, 23, 50, 68, 0, 1, false, true, true),
                createMonster("Kobold des cristaux", "Draconide mineur attiré par les runes", Race::Draconide, 9, 176, 20, 44, 58, 0, 1, false, true),
                createMonster("Chèvre des corniches", "Bête têtue qui pousse vers le vide", Race::Bete, 9, 190, 22, 45, 60, 0, 0, false, true),
                createMonster("Éclat de givre vivant", "Petit élémentaire coupant", Race::Elementaire, 10, 150, 24, 50, 70, 0, 1, false, true, true),
                createMonster("Nain renégat du col", "Humanoïde robuste lié aux anciennes mines", Race::Nain, 10, 230, 22, 48, 64, 1, 1, false, true, true)
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
                createMonster("Moustique sanguin majeur", "Insectoïde de marais attiré par les blessés", Race::Insectoide, 17, 210, 28, 58, 76, 0, 1, false, true, true),
                createMonster("Noyé aux lanternes", "Mort-vivant qui attire les voyageurs", Race::MortVivant, 17, 300, 22, 55, 74, 0, 1, false, true, true),
                createMonster("Slime de vase lourde", "Gelée lente qui use les armures", Race::Slime, 18, 340, 20, 52, 70, 0, 1, false, true, true),
                createMonster("Main de tourbe", "Mort-vivant incomplet qui tire vers l'eau", Race::MortVivant, 17, 260, 21, 52, 72, 0, 1, false, true, true),
                createMonster("Fleur-mâchoire du marais", "Plante carnivore qui attend les erreurs", Race::Plante, 18, 310, 23, 55, 76, 0, 1, false, true, true),
                createMonster("Libellule de fièvre", "Insectoïde rapide qui transmet des états", Race::Insectoide, 16, 190, 28, 60, 82, 0, 1, false, true, true)
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
                createMonster("Chien de tombe", "Bête maigre dressée par l'odeur des morts", Race::Bete, 14, 270, 27, 55, 72, 0, 0, false, true, true),
                createMonster("Veuve de crypte", "Insectoïde funéraire toxique", Race::Insectoide, 15, 245, 25, 58, 78, 0, 1, false, true, true),
                createMonster("Cloche sans son", "Esprit de cimetière qui fatigue les vivants", Race::Esprit, 16, 235, 18, 62, 88, 0, 2, false, true, true),
                createMonster("Porteur de cercueil vide", "Mort-vivant qui cherche encore son occupant", Race::MortVivant, 16, 330, 24, 55, 75, 0, 0, false, true, true),
                createMonster("Corbeau funéraire géant", "Bête de tombe attirée par les combats longs", Race::Bete, 15, 210, 27, 60, 80, 0, 1, false, true, true),
                createMonster("Dame aux bandelettes", "Esprit qui imite une plainte humaine", Race::Esprit, 17, 245, 20, 68, 95, 1, 2, false, true, true)
            };
        }
        if (biomeName == "Ruines effondrées")
        {
            return {
                createMonster("Spectre errant", "Esprit des ruines", Race::Esprit, 18, 270, 23, 55, 72, 0, 1, false, true, true),
                createMonster("Armure morte", "Armure hantée", Race::MortVivant, 19, 360, 24, 50, 66, 0, 0, false, true, true),
                createMonster("Anomalie mineure", "Erreur arcanique", Race::AnomalieArcanique, 18, 295, 22, 58, 78, 0, 2, false, true, true),
                createMonster("Squelette archer rouillé", "Mort-vivant à distance aux gestes mécaniques", Race::MortVivant, 18, 245, 24, 60, 80, 0, 0, false, true, true),
                createMonster("Slime gris poussiéreux", "Gelée nourrie par les pierres mortes", Race::Slime, 17, 285, 20, 52, 70, 0, 0, false, true, true),
                createMonster("Gardien à genou", "Construction ancienne presque détruite", Race::Construction, 18, 420, 18, 48, 64, 0, 0, false, true, true),
                createMonster("Scribe sans bouche", "Mort-vivant runique qui lance des signes", Race::MortVivant, 18, 260, 18, 66, 92, 0, 2, false, true, true),
                createMonster("Escalier qui mord", "Construction piège presque vivante", Race::Construction, 18, 390, 18, 52, 74, 0, 0, false, true, true),
                createMonster("Moitié de statue furieuse", "Gardien amputé mais encore violent", Race::Construction, 19, 470, 24, 62, 84, 0, 0, false, true, true),
                createMonster("Anomalie de marge", "Erreur arcanique coincée dans un couloir", Race::AnomalieArcanique, 18, 280, 25, 70, 98, 0, 2, false, true, true)
            };
        }

        if (biomeName == "Bocage aux lanternes")
        {
            return {
                createMonster("Gardien mycélien", "Fonge massive qui protège les lanternes", Race::Plante, 13, 310, 21, 52, 70, 0, 2, false, true, true),
                createMonster("Cerf aux bois phosphorescents", "Bête ancienne qui charge dans la lumière", Race::Bete, 13, 330, 27, 62, 84, 0, 1, false, true, true),
                createMonster("Rôdeur de résine", "Humanoïde collé par la sève chantante", Race::Humain, 12, 260, 22, 54, 74, 1, 1, false, true, true),
                createMonster("Araignée de filament clair", "Insectoïde qui tend des fils lumineux", Race::Insectoide, 12, 235, 25, 60, 82, 0, 1, false, true, true),
                createMonster("Slime miroir de lune", "Gelée pâle qui reflète les gestes", Race::Slime, 13, 300, 23, 64, 90, 0, 2, false, true, true)
            };
        }

        if (biomeName == "Désert d'argile rouge")
        {
            return {
                createMonster("Sentinelle d'argile cuite", "Construction sèche au pas lourd", Race::Construction, 16, 460, 29, 62, 82, 0, 0, false, true, true),
                createMonster("Dresseur de chacals", "Humain des dunes accompagné de cicatrices", Race::Humain, 16, 320, 29, 68, 92, 2, 1, false, true, true),
                createMonster("Scorpion de sel lunaire", "Insectoïde pâle aux pinces brillantes", Race::Insectoide, 17, 360, 33, 76, 105, 0, 2, false, true, true),
                createMonster("Slime de verre chaud", "Gelée transparente chauffée par le sable", Race::Slime, 16, 330, 28, 70, 98, 0, 2, false, true, true),
                createMonster("Esprit d'oasis sèche", "Esprit qui promet de l'eau puis frappe", Race::Esprit, 17, 310, 26, 82, 116, 0, 3, false, true, true)
            };
        }

        if (biomeName == "Quartier abandonné")
        {
            return {
                createMonster("Receleur de cave", "Humain qui garde trop de clés", Race::Humain, 13, 275, 25, 58, 78, 2, 1, false, true, true),
                createMonster("Automate de boutique cassé", "Construction commerciale devenue hostile", Race::Construction, 14, 370, 22, 54, 72, 0, 1, false, true, true),
                createMonster("Meute de chiens sans maître", "Bêtes urbaines affamées", Race::Bete, 13, 295, 28, 60, 80, 0, 0, false, true, true),
                createMonster("Ombre de fenêtre", "Esprit faible coincé dans une maison vide", Race::Esprit, 14, 250, 20, 68, 94, 0, 2, false, true, true),
                createMonster("Gobelin notaire douteux", "Gobelin armé de contrats et de mensonges", Race::Gobelin, 14, 260, 21, 62, 88, 3, 2, false, true, true)
            };
        }

        if (biomeName == "Mine sifflante")
        {
            return {
                createMonster("Foreuse animée", "Construction de mine devenue autonome", Race::Construction, 20, 560, 34, 70, 94, 0, 1, false, true, true),
                createMonster("Nain renégat des galeries", "Humanoïde robuste qui connaît les raccourcis", Race::Nain, 19, 410, 32, 72, 98, 2, 1, false, true, true),
                createMonster("Élémentaire de fer froid", "Énergie métallique dense", Race::Elementaire, 20, 430, 35, 84, 116, 0, 3, false, true, true),
                createMonster("Slime aimanté", "Gelée qui attire les petites pièces", Race::Slime, 19, 450, 28, 76, 104, 0, 2, false, true, true),
                createMonster("Chauve-souris de minerai", "Bête rapide qui coupe les cordes", Race::Bete, 18, 300, 35, 78, 105, 0, 1, false, true, true)
            };
        }

        if (biomeName == "Verger des lucioles de fer")
        {
            return {
                createMonster("Essaim de lucioles blindées", "Nuage lumineux qui frappe comme de la grêle", Race::Insectoide, 12, 280, 26, 62, 86, 0, 2, false, true, true),
                createMonster("Arbre-lampe jaloux", "Plante lumineuse qui déteste les torches concurrentes", Race::Plante, 13, 360, 23, 58, 78, 0, 2, false, true, true),
                createMonster("Renard à carapace claire", "Bête mutée par les vergers nocturnes", Race::Bete, 12, 270, 30, 66, 92, 0, 1, false, true, true),
                createMonster("Gardien de ruche métallique", "Construction/insecte impossible à classer proprement", Race::Construction, 14, 390, 28, 65, 88, 0, 2, false, true, true)
            };
        }

        if (biomeName == "Archives noyées")
        {
            return {
                createMonster("Greffier fantôme", "Esprit administratif qui refuse les erreurs", Race::Esprit, 18, 360, 27, 84, 118, 1, 3, false, true, true),
                createMonster("Archive vivante mineure", "Livre géant qui respire sous l'eau", Race::AnomalieArcanique, 19, 430, 30, 88, 125, 1, 3, false, true, true),
                createMonster("Slime d'encre profonde", "Gelée lourde qui tache même les souvenirs", Race::Slime, 18, 410, 28, 78, 110, 0, 2, false, true, true),
                createMonster("Archiviste noyé", "Mort-vivant lettré avec une haine des pages pliées", Race::MortVivant, 19, 395, 31, 82, 115, 2, 2, false, true, true)
            };
        }

        if (biomeName == "Falaises des drakes gris")
        {
            return {
                createMonster("Drake gris de corniche", "Draconide assez grand pour mépriser les cordes", Race::Draconide, 24, 620, 52, 116, 158, 0, 3, false, true, true),
                createMonster("Harpie coupeuse d'ancrage", "Bête ailée qui vise les sécurités avant les aventuriers", Race::Bete, 23, 430, 48, 112, 150, 0, 2, false, true, true),
                createMonster("Esprit du vide bas", "Esprit né des chutes et des regrets", Race::Esprit, 24, 390, 42, 128, 176, 0, 3, false, true, true),
                createMonster("Éboulement à cœur gris", "Construction naturelle qui se reforme lentement", Race::Construction, 25, 720, 46, 108, 145, 0, 1, false, true, true)
            };
        }

        if (biomeName == "Foire abandonnée")
        {
            return {
                createMonster("Maître de stand sans rire", "Humain creux qui vend encore des tickets", Race::Humain, 16, 315, 31, 76, 105, 2, 2, false, true, true),
                createMonster("Manège animé grinçant", "Construction circulaire qui n'accepte plus les descentes", Race::Construction, 17, 520, 30, 72, 98, 0, 2, false, true, true),
                createMonster("Miroir menteur", "Esprit de reflet qui répond à ta place", Race::Esprit, 17, 300, 26, 92, 130, 1, 3, false, true, true),
                createMonster("Slime barbe-à-papa acide", "Gelée rose, mignonne, puis mauvaise idée", Race::Slime, 16, 340, 29, 78, 108, 0, 2, false, true, true)
            };
        }


        if (biomeName == "Temple des cloches fendues")
        {
            return {
                createMonster("Sonneur creux", "Esprit de cloche qui frappe sans bras", Race::Esprit, 21, 420, 34, 92, 126, 1, 3, false, true, true),
                createMonster("Autel animé", "Construction sacrée qui refuse les mains sales", Race::Construction, 22, 600, 32, 84, 115, 0, 2, false, true, true),
                createMonster("Chevalier de vœu fissuré", "Mort-vivant lié à une promesse mal tenue", Race::MortVivant, 23, 560, 40, 96, 130, 0, 1, false, true, true)
            };
        }

        if (biomeName == "Canaux de brume bleue")
        {
            return {
                createMonster("Passeur sans visage", "Humain ou esprit qui demande le prix après le trajet", Race::Esprit, 18, 360, 34, 92, 130, 2, 2, false, true, true),
                createMonster("Nixe ancienne", "Esprit d'eau qui parle trop doucement", Race::Esprit, 19, 340, 28, 108, 150, 2, 4, false, true, true),
                createMonster("Brume consciente", "Anomalie froide qui choisit les impasses", Race::AnomalieArcanique, 20, 390, 30, 116, 160, 0, 4, false, true, true)
            };
        }

        if (biomeName == "Carrière des os blancs")
        {
            return {
                createMonster("Sculpteur d'os", "Humain pâle qui travaille encore", Race::Humain, 28, 480, 44, 118, 165, 1, 2, false, true, true),
                createMonster("Colosse de poussière", "Construction qui se recompose mal", Race::Construction, 30, 900, 46, 112, 150, 0, 1, false, true, true),
                createMonster("Fossile nerveux", "Assemblage d'ossements incomplets", Race::MortVivant, 29, 620, 48, 120, 168, 0, 1, false, true, true)
            };
        }

        if (biomeName == "Marché sous les ponts")
        {
            return {
                createMonster("Collecteur masqué", "Humain précis, poli et absolument pas rassurant", Race::Humain, 20, 430, 42, 112, 158, 3, 2, false, true, true),
                createMonster("Arbitre de dette", "Esprit comptable qui tranche les litiges au couteau", Race::Esprit, 21, 390, 36, 125, 175, 2, 4, false, true, true),
                createMonster("Ombre de pont", "Créature qui vit entre deux passages", Race::Esprit, 22, 410, 44, 132, 184, 0, 3, false, true, true)
            };
        }

        if (biomeName == "Jardin des statues qui pleurent")
        {
            return {
                createMonster("Jardinier sans visage", "Humain ou esprit armé d'un sécateur lourd", Race::Esprit, 22, 460, 42, 112, 154, 1, 2, false, true, true),
                createMonster("Rosier de marbre", "Plante impossible qui pousse dans la pierre", Race::Plante, 23, 650, 36, 106, 148, 0, 3, false, true, true),
                createMonster("Muse pétrifiée", "Statue belle de loin, agressive de près", Race::Construction, 24, 700, 40, 118, 165, 1, 2, false, true, true)
            };
        }

        if (biomeName == "Bois de la Corruption")
        {
            return {
                createMonster("Dryade déformée", "Esprit végétal devenu possessif", Race::Plante, 26, 620, 48, 128, 180, 1, 4, false, true, true),
                createMonster("Ombre à crocs", "Prédateur noir dont la gueule arrive avant le corps", Race::Esprit, 27, 540, 58, 150, 210, 0, 4, false, true, true),
                createMonster("Arbre de bile", "Plante corrompue qui soigne mal ses alliés", Race::Plante, 28, 800, 42, 118, 165, 3, 3, false, true, true)
            };
        }

        if (biomeName == "Crypte du Sombre-Lien")
        {
            return {
                createMonster("Prêtre sans regard", "Cultiste mort qui récite encore", Race::MortVivant, 28, 500, 36, 145, 205, 3, 4, false, true, true),
                createMonster("Gardien de serment noir", "Chevalier lié par une faute ancienne", Race::MortVivant, 30, 760, 54, 138, 190, 0, 2, false, true, true),
                createMonster("Ossuaire lié", "Construction d'os et de chaînes", Race::Construction, 29, 900, 48, 124, 170, 0, 1, false, true, true)
            };
        }

        if (biomeName == "Désert des Protecteurs")
        {
            return {
                createMonster("Protecteur éveillé", "Golem ancien qui reconnaît les menteurs", Race::Construction, 32, 900, 58, 150, 205, 1, 3, false, true, true),
                createMonster("Sphinx de serment", "Esprit antique qui pose ses questions en combat", Race::Esprit, 33, 720, 48, 170, 240, 2, 5, false, true, true),
                createMonster("Statue de divinité mineure", "Construction sacrée, trop vieille pour être polie", Race::Construction, 34, 980, 60, 160, 220, 1, 3, false, true, true)
            };
        }

        if (biomeName == "Sanctuaire antique des Veilleurs")
        {
            return {
                createMonster("Veilleur doré", "Sentinelle qui corrige la posture avant les os", Race::Construction, 34, 980, 62, 165, 230, 1, 3, false, true, true),
                createMonster("Prêtresse de sable", "Aasimar ancienne liée au sanctuaire", Race::Aasimar, 35, 760, 46, 190, 265, 3, 5, false, true, true),
                createMonster("Juge de pierre", "Construction lente dont les verdicts cognent fort", Race::Construction, 36, 1100, 66, 172, 240, 0, 4, false, true, true)
            };
        }

        if (biomeName == "Quartier des Lames Muettes")
        {
            return {
                createMonster("Assassin sans souffle", "Humain entraîné à ne pas annoncer le combat", Race::Humain, 40, 600, 90, 240, 335, 2, 4, false, true, true),
                createMonster("Maître de poison", "Humain fragile mais odieux à laisser jouer", Race::Humain, 39, 520, 60, 230, 330, 4, 6, false, true, true),
                createMonster("Lame de guilde noire", "Dueliste de contrat", Race::Humain, 41, 680, 95, 250, 350, 2, 3, false, true, true)
            };
        }

        if (biomeName == "Toits des Assassins")
        {
            return {
                createMonster("Duelliste de corniche", "Combattant qui aime les sols trop étroits", Race::Humain, 42, 700, 96, 260, 360, 2, 3, false, true, true),
                createMonster("Ombre de balcon", "Esprit né d'une chute discrète", Race::Esprit, 43, 620, 80, 280, 390, 1, 5, false, true, true),
                createMonster("Exécuteur de contrat", "Humain qui a déjà signé la fin", Race::Humain, 44, 760, 105, 270, 378, 3, 4, false, true, true)
            };
        }

        if (biomeName == "Nid draconique rouge")
        {
            return {
                createMonster("Drake rouge", "Dragon territorial de couvée", Race::Dragon, 52, 1300, 125, 330, 460, 0, 5, false, true, true),
                createMonster("Mère de nid nerveuse", "Dragonne jeune mais très peu négociable", Race::Dragon, 54, 1550, 135, 350, 490, 0, 6, false, true, true),
                createMonster("Gardien de couvée", "Draconide massif qui protège les œufs", Race::Draconide, 53, 1400, 115, 310, 435, 1, 4, false, true, true)
            };
        }

        if (biomeName == "Coulées de lave noire")
        {
            return {
                createMonster("Seigneur de magma jeune", "Élémentaire de feu dominant", Race::Elementaire, 54, 1200, 120, 360, 505, 0, 7, false, true, true),
                createMonster("Cœur volcanique", "Anomalie chaude au centre trop calme", Race::AnomalieArcanique, 55, 1100, 100, 390, 550, 1, 7, false, true, true),
                createMonster("Salamandre noire", "Bête de lave qui marche où les autres fondent", Race::Bete, 53, 1050, 135, 320, 450, 0, 5, false, true, true)
            };
        }

        if (biomeName == "Glacier des Serments froids")
        {
            return {
                createMonster("Drake de glace", "Dragon froid dont le souffle ralentit même les pensées", Race::Dragon, 54, 1300, 118, 330, 465, 0, 6, false, true, true),
                createMonster("Serment cristallisé", "Anomalie née d'une promesse brisée", Race::AnomalieArcanique, 55, 980, 90, 380, 540, 1, 7, false, true, true),
                createMonster("Reine des congères", "Esprit de tempête froide", Race::Esprit, 56, 1000, 104, 400, 560, 2, 7, false, true, true)
            };
        }

        if (biomeName == "Bosquet des Fées du Mana")
        {
            return {
                createMonster("Noble fée", "Fée qui sourit avant de déplacer les règles", Race::Fee, 64, 720, 92, 420, 590, 5, 8, false, true, true),
                createMonster("Gardien de pacte vert", "Esprit végétal garant d'une promesse", Race::Esprit, 65, 1100, 100, 380, 535, 3, 7, false, true, true),
                createMonster("Farceur majeur", "Fée spécialisée dans les mauvaises bonnes idées", Race::Fee, 66, 700, 88, 450, 630, 4, 9, false, true, true)
            };
        }

        if (biomeName == "Sanctuaire kitsuné des Neuf Étincelles")
        {
            return {
                createMonster("Prêtresse kitsuné", "Kitsuné capable de bénir puis brûler la même phrase", Race::Kitsune, 66, 860, 110, 430, 600, 4, 8, false, true, true),
                createMonster("Renard à neuf queues incomplet", "Esprit-renard presque légendaire", Race::Kitsune, 68, 980, 130, 450, 630, 2, 9, false, true, true),
                createMonster("Miroir de feu", "Esprit-reflet qui garde une flamme volée", Race::Esprit, 67, 780, 90, 470, 660, 3, 9, false, true, true)
            };
        }

        if (biomeName == "Confluence du Mana pur")
        {
            return {
                createMonster("Noyau pur", "Énergie concentrée qui apprend vite", Race::Elementaire, 70, 1150, 120, 500, 700, 1, 10, false, true, true),
                createMonster("Archimage errant", "Humain ou semi-humain noyé dans sa propre magie", Race::Humain, 72, 920, 90, 560, 790, 6, 10, false, true, true),
                createMonster("Tempête consciente", "Anomalie météo capable de choisir une cible", Race::AnomalieArcanique, 74, 1250, 135, 540, 760, 1, 10, false, true, true)
            };
        }

        if (biomeName == "Bastion majeur scellé")
        {
            return {
                createMonster("Gardien de chapitre", "Construction qui protège une scène pas encore écrite", Race::Construction, 82, 2200, 160, 610, 850, 2, 8, false, true, true),
                createMonster("Serment royal", "Esprit de loyauté devenu dangereux", Race::Esprit, 84, 1500, 130, 700, 980, 4, 10, false, true, true),
                createMonster("Fragment de trône", "Anomalie noble et tranchante", Race::AnomalieArcanique, 85, 1800, 145, 740, 1040, 3, 11, false, true, true)
            };
        }

        if (biomeName == "Archipel des îles flottantes")
        {
            return {
                createMonster("Baleine de ciel jeune", "Bête immense portée par le mana", Race::Bete, 95, 2800, 190, 820, 1150, 2, 10, false, true, true),
                createMonster("Chevalier du vide", "Humain ou esprit habitué au manque de sol", Race::Esprit, 96, 1700, 210, 900, 1260, 4, 12, false, true, true),
                createMonster("Drake d'altitude", "Dragon mince des courants hauts", Race::Dragon, 98, 2400, 240, 920, 1290, 1, 12, false, true, true)
            };
        }

        if (biomeName == "Ponts translucides de mana")
        {
            return {
                createMonster("Architecte de mana", "Construction consciente chargée de réparer les ponts avec les intrus", Race::Construction, 98, 2300, 200, 910, 1280, 3, 12, false, true, true),
                createMonster("Reflet parfait", "Esprit qui copie une version trop bonne de toi", Race::Esprit, 100, 1600, 170, 1050, 1470, 5, 14, false, true, true),
                createMonster("Briseur de passerelle", "Anomalie qui attaque surtout le sol sous tes pieds", Race::AnomalieArcanique, 99, 2100, 220, 940, 1320, 2, 12, false, true, true)
            };
        }

        if (biomeName == "Cieux des Légendes")
        {
            return {
                createMonster("Légende éveillée", "Récit héroïque devenu adversaire", Race::Esprit, 125, 3000, 300, 1350, 1900, 5, 16, false, true, true),
                createMonster("Héros sans tombe", "Âme de champion encore debout", Race::Humain, 128, 3300, 340, 1420, 2000, 4, 15, false, true, true),
                createMonster("Étoile consciente", "Élémentaire céleste qui pense en constellations", Race::Elementaire, 130, 3600, 310, 1500, 2100, 2, 18, false, true, true)
            };
        }

        if (biomeName == "Parvis des Divinités")
        {
            return {
                createMonster("Avatar mineur", "Présence divine très incomplète, ce qui suffit déjà", Race::Ange, 140, 4200, 380, 1680, 2350, 6, 20, false, true, true),
                createMonster("Juge des cieux", "Entité qui pèse les actes avant les dégâts", Race::Ange, 145, 4600, 360, 1780, 2500, 8, 20, false, true, true),
                createMonster("Fragment de divinité", "Anomalie céleste qu'il ne faut pas confondre avec un vrai dieu", Race::AnomalieArcanique, 150, 5000, 400, 1900, 2700, 5, 22, false, true, true)
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
                createMonster("Renard chapardeur", "Petite bête maligne attirée par les sacs ouverts", Race::Bete, 6, 145, 17, 38, 50, 0, 0, false, true),
                createMonster("Cerf blanc de plaine", "Bête rare qui charge quand on la poursuit", Race::Bete, 8, 260, 24, 50, 68, 0, 0, false, true, true),
                createMonster("Gobelin porte-chance", "Petit gobelin rare entouré de malchance pour les autres", Race::Gobelin, 7, 190, 18, 48, 68, 2, 2, false, true, true)
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
                createMonster("Slime miroir ancien", "Gelée ancienne qui semble copier la pression autour d'elle", Race::Slime, 19, 455, 28, 72, 102, 0, 2, false, true, true),
                createMonster("Slime d'argent silencieux", "Gelée rare qui absorbe les vibrations", Race::Slime, 18, 430, 25, 76, 110, 0, 2, false, true, true),
                createMonster("Couronne de bulles", "Noyau rare entouré de bulles agressives", Race::Slime, 20, 500, 24, 80, 115, 0, 2, false, true, true),
                createSplittingSlime("Colosse de gelée fusionnée", "Accumulation rare de noyaux gélatineux incapables de rester séparés", 20, 620, 26, 84, 122, 4, 5, "Éclat de colosse gélatineux", true, true, true)
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
                createMonster("Gobelin chef de sacs", "Gobelin organisé qui protège surtout le butin", Race::Gobelin, 11, 245, 24, 54, 72, 1, 1, false, true, true),
                createMonster("Huissier de Grinka raté", "Gobelin persuadé que toutes les routes lui doivent une taxe", Race::Gobelin, 13, 300, 26, 66, 90, 2, 2, false, true, true),
                createMonster("Dueliste de caravane", "Humain rapide engagé pour les colis sensibles", Race::Humain, 12, 270, 31, 68, 92, 1, 1, false, true, true)
            };
        }

        if (biomeName == "Forêt ancienne")
        {
            return {
                createMonster("Gardien de ronces", "Protecteur végétal", Race::Plante, 12, 340, 25, 54, 70, 0, 1, false, true, true),
                createMonster("Esprit sylvestre", "Mémoire de forêt", Race::Esprit, 12, 290, 23, 60, 78, 0, 2, false, true, true),
                createMonster("Alpha de mousse", "Prédateur ancien", Race::Bete, 13, 360, 28, 56, 72, 0, 0, false, true),
                createMonster("Loup couvert de lichen", "Prédateur de forêt presque confondu avec le sol", Race::Bete, 12, 310, 27, 58, 76, 0, 0, false, true, true),
                createMonster("Racine étrangleuse", "Plante rare qui ralentit avant de frapper", Race::Plante, 13, 335, 22, 56, 74, 0, 1, false, true, true),
                createMonster("Gardien champignon couronné", "Fonge rare qui défend une clairière", Race::Plante, 14, 360, 18, 62, 88, 0, 2, false, true, true),
                createMonster("Cerf aux bois runiques", "Bête ancienne marquée par une vieille magie", Race::Bete, 14, 390, 30, 66, 90, 0, 1, false, true, true)
            };
        }

        if (biomeName == "Montagne froide")
        {
            return {
                createMonster("Yéti des falaises", "Prédateur des cimes", Race::Bete, 16, 430, 33, 66, 84, 0, 0, false, true, true),
                createMonster("Draconide froid majeur", "Draconide territorial", Race::Draconide, 17, 410, 36, 74, 98, 0, 1, false, true, true),
                createMonster("Élite rocheuse", "Construction de montagne", Race::Construction, 18, 520, 32, 67, 86, 0, 0, false, true, true),
                createMonster("Drake des congères", "Jeune dragon de neige", Race::Dragon, 19, 560, 38, 82, 112, 0, 2, false, true, true),
                createMonster("Colosse de sel gelé", "Construction minérale qui casse les lames faibles", Race::Construction, 18, 650, 34, 72, 96, 0, 0, false, true, true)
            };
        }

        if (biomeName == "Marais trouble")
        {
            return {
                createMonster("Slime couronné", "Gelée dominante", Race::Slime, 24, 560, 36, 76, 98, 0, 1, false, true, true),
                createMonster("Slime doré engloutisseur", "Gelée rare attirée par ce qui brille", Race::Slime, 23, 500, 28, 72, 100, 0, 0, false, true, true),
                createMonster("Mage putride", "Sorcier des eaux sales", Race::Humain, 25, 430, 30, 90, 125, 2, 3, false, true, true),
                createMonster("Grand chamane de vase", "Soigneur marécageux capable de maintenir ses alliés debout", Race::Humain, 26, 455, 24, 84, 118, 3, 2, false, true, true),
                createMonster("Noyé ancien", "Mort-vivant de profondeur", Race::MortVivant, 26, 620, 39, 82, 105, 0, 0, false, true, true),
                createMonster("Reine moustique de fièvre", "Insectoïde rare qui rend le soin urgent", Race::Insectoide, 25, 460, 45, 96, 132, 0, 2, false, true, true),
                createMonster("Slime noir des vieilles bottes", "Gelée rare qui dévore les protections mal entretenues", Race::Slime, 26, 610, 34, 90, 126, 0, 2, false, true, true)
            };
        }


        if (biomeName == "Cimetière oublié")
        {
            return {
                createMonster("Chevalier du dernier serment", "Mort-vivant lourd et honorable", Race::MortVivant, 22, 560, 36, 78, 102, 0, 0, false, true, true),
                createMonster("Oracle de pierre tombale", "Support spectral qui comprend trop de choses", Race::Esprit, 23, 440, 28, 92, 128, 3, 2, false, true, true),
                createMonster("Ossuaire rampant", "Construction d'os assemblés", Race::Construction, 24, 700, 34, 76, 100, 0, 0, false, true, true),
                createMonster("Ombre de nom perdu", "Esprit rare lié aux identités effacées", Race::Esprit, 25, 500, 38, 105, 145, 0, 3, false, true, true),
                createMonster("Goule couronnée de poussière", "Prédateur mort-vivant de caveau noble", Race::MortVivant, 24, 620, 42, 88, 116, 0, 1, false, true, true),
                createMonster("Cheval noir sans cavalier", "Bête spectrale de procession", Race::Esprit, 25, 540, 40, 98, 134, 0, 2, false, true, true),
                createMonster("Ossuaire noble", "Assemblage d'os portant encore des bagues", Race::Construction, 25, 760, 36, 86, 112, 0, 1, false, true, true)
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
                createMonster("Sentinelle brisée", "Construction abîmée, lente, presque facile à lire mais très dure à casser", Race::Construction, 25, 780, 28, 72, 95, 0, 0, false, true, true),
                createMonster("Bibliothèque agressive", "Meuble ancien possédé par des notes violentes", Race::Construction, 27, 640, 30, 90, 130, 0, 3, false, true, true),
                createMonster("Anomalie de porte ouverte", "Faille qui mène parfois au même couloir", Race::AnomalieArcanique, 29, 620, 42, 112, 150, 0, 3, false, true, true)
            };
        }

        if (biomeName == "Bocage aux lanternes")
        {
            return {
                createMonster("Roi-lanterne fongique", "Fonge rare qui commande les lumières", Race::Plante, 22, 620, 34, 86, 118, 0, 3, false, true, true),
                createMonster("Bête blanche sous mycélium", "Prédateur ancien couvert de lumière froide", Race::Bete, 23, 650, 39, 92, 126, 0, 1, false, true, true),
                createMonster("Esprit du bocage clair", "Mémoire végétale dangereusement calme", Race::Esprit, 24, 520, 30, 104, 145, 0, 4, false, true, true)
            };
        }

        if (biomeName == "Désert d'argile rouge")
        {
            return {
                createMonster("Colosse d'argile solaire", "Construction sèche presque rituelle", Race::Construction, 28, 850, 46, 108, 145, 0, 1, false, true, true),
                createMonster("Chacal de sel lunaire", "Bête rare qui chasse à la lumière froide", Race::Bete, 27, 620, 48, 116, 160, 0, 2, false, true, true),
                createMonster("Sphinx de marché perdu", "Esprit ancien qui pose des questions avec ses griffes", Race::Esprit, 29, 690, 40, 130, 180, 1, 4, false, true, true)
            };
        }

        if (biomeName == "Quartier abandonné")
        {
            return {
                createMonster("Propriétaire sans visage", "Esprit qui refuse de quitter sa maison", Race::Esprit, 22, 530, 32, 98, 135, 1, 3, false, true, true),
                createMonster("Collecteur de dettes masqué", "Humain ou presque, trop calme pour être honnête", Race::Humain, 23, 560, 42, 104, 142, 3, 2, false, true, true),
                createMonster("Automate municipal fou", "Construction prévue pour protéger, puis oubliée", Race::Construction, 24, 760, 38, 96, 130, 0, 2, false, true, true)
            };
        }

        if (biomeName == "Mine sifflante")
        {
            return {
                createMonster("Chef de galerie ferrugineux", "Construction de mine au marteau lourd", Race::Construction, 31, 960, 52, 122, 165, 0, 2, false, true, true),
                createMonster("Dragonnet de minerai froid", "Jeune dragon attiré par le fer silencieux", Race::Dragon, 32, 820, 56, 136, 185, 0, 3, false, true, true),
                createMonster("Cœur de machine éveillé", "Anomalie mécanique qui bat comme un organe", Race::AnomalieArcanique, 33, 740, 46, 150, 210, 1, 4, false, true, true)
            };
        }

        if (biomeName == "Verger des lucioles de fer")
        {
            return {
                createMonster("Reine-luciole de fer", "Insectoïde lumineuse entourée d'une armure vivante", Race::Insectoide, 22, 620, 40, 104, 145, 1, 3, false, true, true),
                createMonster("Pommier-lampe ancien", "Plante gardienne qui éclaire les fautes avant de punir", Race::Plante, 23, 760, 36, 92, 132, 1, 3, false, true, true),
                createMonster("Cerf aux bois lumineux", "Bête rare du verger, presque mythique si on oublie les coups", Race::Bete, 24, 700, 45, 112, 158, 0, 2, false, true, true)
            };
        }

        if (biomeName == "Archives noyées")
        {
            return {
                createMonster("Grand registre avaleur", "Archive vivante qui classe les aventuriers par goût", Race::AnomalieArcanique, 28, 820, 42, 135, 190, 2, 4, false, true, true),
                createMonster("Bibliothécaire sous-marine", "Mort-vivant calme, donc beaucoup trop dangereux", Race::MortVivant, 27, 760, 46, 120, 170, 3, 3, false, true, true),
                createMonster("Sceau noyé de jugement", "Construction magique qui tamponne les os", Race::Construction, 29, 900, 48, 124, 175, 1, 3, false, true, true)
            };
        }

        if (biomeName == "Falaises des drakes gris")
        {
            return {
                createMonster("Drake gris adulte", "Dragon de falaise territorial et très peu impressionné", Race::Dragon, 34, 1050, 66, 152, 205, 0, 4, false, true, true),
                createMonster("Matriarche des corniches", "Harpie ancienne qui a vu tomber trop d'aventuriers", Race::Bete, 33, 780, 62, 150, 210, 1, 4, false, true, true),
                createMonster("Esprit du dernier pas", "Esprit de chute qui attend une erreur simple", Race::Esprit, 35, 720, 54, 170, 238, 1, 5, false, true, true)
            };
        }

        if (biomeName == "Foire abandonnée")
        {
            return {
                createMonster("Maître de piste masqué", "Chef de foire qui annonce les coups avant de tricher", Race::Humain, 25, 640, 46, 120, 168, 3, 3, false, true, true),
                createMonster("Grand miroir du chapiteau", "Esprit-reflet capable de copier les mauvaises habitudes", Race::Esprit, 26, 590, 38, 148, 210, 2, 5, false, true, true),
                createMonster("Carrousel carnivore", "Construction de fête qui tourne autour des survivants", Race::Construction, 27, 900, 44, 112, 155, 1, 3, false, true, true)
            };
        }


        if (biomeName == "Temple des cloches fendues")
        {
            return {
                createMonster("Grand sonneur de silence", "Esprit rare dont la cloche coupe les certitudes", Race::Esprit, 34, 720, 56, 170, 240, 2, 6, false, true, true),
                createMonster("Chevalier du vœu final", "Mort-vivant sacré qui ne recule pas", Race::MortVivant, 35, 980, 70, 160, 225, 0, 4, false, true, true)
            };
        }

        if (biomeName == "Canaux de brume bleue")
        {
            return {
                createMonster("Barque sans passeur", "Construction hantée qui navigue sur la peur", Race::Construction, 30, 760, 48, 150, 210, 1, 5, false, true, true),
                createMonster("Dame de brume bleue", "Esprit d'eau presque noble", Race::Esprit, 32, 620, 42, 190, 270, 3, 7, false, true, true)
            };
        }

        if (biomeName == "Carrière des os blancs")
        {
            return {
                createMonster("Géant enfoui", "Colosse dont on n'a extrait qu'une partie", Race::Construction, 40, 1400, 80, 210, 295, 0, 4, false, true, true),
                createMonster("Maître des fossiles blancs", "Esprit d'ossements et de craie", Race::Esprit, 42, 920, 62, 250, 350, 3, 7, false, true, true)
            };
        }

        if (biomeName == "Marché sous les ponts")
        {
            return {
                createMonster("Notaire de l'ombre", "Esprit qui transforme les promesses en chaînes", Race::Esprit, 32, 650, 50, 210, 295, 4, 7, false, true, true),
                createMonster("Roi des mauvais trocs", "Humain presque mythique du marché noir", Race::Humain, 34, 820, 76, 220, 310, 5, 5, false, true, true)
            };
        }

        if (biomeName == "Jardin des statues qui pleurent")
        {
            return {
                createMonster("Reine de marbre triste", "Statue rare qui pleure sans pardonner", Race::Construction, 34, 1100, 62, 205, 285, 2, 6, false, true, true),
                createMonster("Rosier du dernier bal", "Plante noble nourrie par les regrets", Race::Plante, 35, 900, 54, 230, 325, 2, 7, false, true, true)
            };
        }

        if (biomeName == "Bois de la Corruption")
        {
            return {
                createMonster("Cœur noir du bois", "Noyau corrompu protégé par des racines conscientes", Race::Plante, 38, 1250, 75, 260, 365, 2, 8, false, true, true),
                createMonster("Cerf sans lumière", "Bête noble vidée par la corruption", Race::Bete, 40, 980, 95, 280, 395, 0, 7, false, true, true)
            };
        }

        if (biomeName == "Crypte du Sombre-Lien")
        {
            return {
                createMonster("Nom enterré", "Esprit rare qui attaque l'identité", Race::Esprit, 42, 880, 72, 300, 425, 3, 9, false, true, true),
                createMonster("Reliquaire du pacte noir", "Construction d'ossements, de cire et de chaînes", Race::Construction, 44, 1400, 88, 270, 380, 1, 7, false, true, true)
            };
        }

        if (biomeName == "Désert des Protecteurs")
        {
            return {
                createMonster("Protecteur de divinité oubliée", "Statue sacrée au jugement incomplet", Race::Construction, 48, 1600, 105, 330, 465, 2, 8, false, true, true),
                createMonster("Sphinx du dernier serment", "Esprit antique qui n'accepte pas les réponses faciles", Race::Esprit, 50, 1200, 84, 380, 535, 4, 10, false, true, true)
            };
        }

        if (biomeName == "Sanctuaire antique des Veilleurs")
        {
            return {
                createMonster("Grand Veilleur antique", "Gardien majeur du sanctuaire", Race::Construction, 52, 1800, 110, 360, 505, 2, 9, false, true, true),
                createMonster("Oracle du sable protecteur", "Aasimar ancien qui lit les intentions", Race::Aasimar, 54, 1250, 82, 420, 590, 5, 12, false, true, true)
            };
        }

        if (biomeName == "Quartier des Lames Muettes")
        {
            return {
                createMonster("Maître des Lames Muettes", "Assassin rare qui transforme le silence en arme", Race::Humain, 58, 1100, 160, 500, 700, 4, 10, false, true, true),
                createMonster("Ombre d'exécution", "Esprit d'un contrat qui a trop bien marché", Race::Esprit, 60, 980, 135, 540, 760, 2, 12, false, true, true)
            };
        }

        if (biomeName == "Toits des Assassins")
        {
            return {
                createMonster("Roi des corniches", "Dueliste des hauteurs qui ne descend jamais", Race::Humain, 62, 1300, 170, 520, 735, 4, 10, false, true, true),
                createMonster("Flèche sans tireur", "Anomalie de meurtre à distance", Race::AnomalieArcanique, 64, 1000, 150, 590, 830, 1, 14, false, true, true)
            };
        }

        if (biomeName == "Nid draconique rouge")
        {
            return {
                createMonster("Matriarche rouge", "Dragonne gardienne du nid", Race::Dragon, 72, 2600, 240, 720, 1010, 1, 12, false, true, true),
                createMonster("Œuf qui rêve de feu", "Anomalie draconique très mauvaise idée", Race::AnomalieArcanique, 70, 1900, 170, 760, 1070, 2, 14, false, true, true)
            };
        }

        if (biomeName == "Coulées de lave noire")
        {
            return {
                createMonster("Seigneur de magma noir", "Élémentaire volcanique rare", Race::Elementaire, 74, 2400, 220, 780, 1100, 1, 14, false, true, true),
                createMonster("Rivière qui marche", "Anomalie de lave qui n'a pas compris les limites", Race::AnomalieArcanique, 76, 2200, 200, 820, 1150, 0, 15, false, true, true)
            };
        }

        if (biomeName == "Glacier des Serments froids")
        {
            return {
                createMonster("Roi des serments gelés", "Mort-vivant royal emprisonné par son propre ordre", Race::MortVivant, 76, 2500, 210, 760, 1070, 2, 13, false, true, true),
                createMonster("Dragon de givre silencieux", "Dragon qui gèle avant de rugir", Race::Dragon, 78, 2700, 230, 790, 1120, 1, 14, false, true, true)
            };
        }

        if (biomeName == "Bosquet des Fées du Mana")
        {
            return {
                createMonster("Duchesse fée du Mana", "Noble fée capable de changer le prix d'une erreur", Race::Fee, 86, 1400, 150, 900, 1260, 7, 16, false, true, true),
                createMonster("Arbre aux pactes vivants", "Plante ancienne qui garde trop de promesses", Race::Plante, 88, 2600, 160, 760, 1070, 5, 14, false, true, true)
            };
        }

        if (biomeName == "Sanctuaire kitsuné des Neuf Étincelles")
        {
            return {
                createMonster("Kitsuné aux neuf étincelles", "Renard-esprit presque complet", Race::Kitsune, 90, 1700, 200, 940, 1320, 6, 18, false, true, true),
                createMonster("Torii qui ment", "Construction sacrée qui ouvre le mauvais chemin", Race::Construction, 88, 2200, 160, 850, 1200, 4, 16, false, true, true)
            };
        }

        if (biomeName == "Confluence du Mana pur")
        {
            return {
                createMonster("Source du mana pur", "Élémentaire majeur né de la confluence", Race::Elementaire, 95, 2400, 210, 1050, 1470, 4, 20, false, true, true),
                createMonster("Mage devenu rivière", "Anomalie arcanique qui se souvient de ses sorts", Race::AnomalieArcanique, 96, 1900, 160, 1180, 1660, 8, 22, false, true, true)
            };
        }

        if (biomeName == "Bastion majeur scellé")
        {
            return {
                createMonster("Commandant du Bastion scellé", "Gardien majeur d'un arc futur", Race::Aasimar, 112, 3600, 280, 1300, 1820, 6, 18, false, true, true),
                createMonster("Porte qui refuse la fin", "Anomalie de scénario condensée", Race::AnomalieArcanique, 115, 4200, 250, 1450, 2040, 4, 22, false, true, true)
            };
        }

        if (biomeName == "Archipel des îles flottantes")
        {
            return {
                createMonster("Baleine-citadelle du ciel", "Bête légendaire des hautes îles", Race::Bete, 135, 6200, 380, 1800, 2550, 5, 24, false, true, true),
                createMonster("Dragon des ponts d'altitude", "Dragon aérien qui chasse entre les îles", Race::Dragon, 132, 4800, 420, 1900, 2670, 4, 24, false, true, true)
            };
        }

        if (biomeName == "Ponts translucides de mana")
        {
            return {
                createMonster("Pont vivant", "Construction de mana qui décide qui traverse", Race::Construction, 138, 6000, 360, 1850, 2600, 6, 24, false, true, true),
                createMonster("Reflet du mauvais futur", "Esprit qui copie une défaite possible", Race::Esprit, 140, 4100, 300, 2100, 2950, 8, 28, false, true, true)
            };
        }

        if (biomeName == "Cieux des Légendes")
        {
            return {
                createMonster("Champion de légende", "Héros complet dont le nom refuse de mourir", Race::Humain, 175, 7200, 550, 2800, 3950, 8, 32, false, true, true),
                createMonster("Constellation du dernier récit", "Élémentaire céleste quasi mythique", Race::Elementaire, 180, 8000, 520, 3000, 4200, 6, 34, false, true, true)
            };
        }

        if (biomeName == "Parvis des Divinités")
        {
            return {
                createMonster("Avatar du seuil divin", "Avatar mineur mais presque injouable sans préparation", Race::Ange, 195, 9500, 650, 3500, 4900, 10, 38, false, true, true),
                createMonster("Fragment du ciel juge", "Morceau de volonté divine, pas une vraie divinité", Race::AnomalieArcanique, 200, 11000, 700, 3800, 5350, 8, 42, false, true, true)
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

        Monster scaled = createMonster(
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
            baseMonster.isElite(),
            !baseMonster.areStatsVisible() || targetLevel >= baseMonster.getLevel() + 8,
            baseMonster.isEvolved()
        );

        if (baseMonster.doesSplitOnDeath())
        {
            scaled.copySplitBehaviorFrom(baseMonster);
        }

        return scaled;
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


Monster MonsterCatalog::createGiantSlimeMiniBoss(int level)
{
    level = std::max(4, level);

    Monster giant = createMonster(
        "Géant slime des quatre divisions",
        "Mini-boss gélatineux aux noyaux emboîtés",
        Race::Slime,
        level,
        240 + level * 34,
        4 + level,
        9 + level * 2,
        14 + level * 3,
        0,
        0,
        false,
        true,
        false,
        true
    );
    giant.configureSplitTree(
        2,
        {"Gros slime", "Slime", "Petit slime", "Âme du slime"},
        true
    );
    return giant;
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
    int levelBonus = random.between(1, 3);
    int hpPercent = baseMonster.isElite() ? random.between(185, 225) : random.between(160, 195);
    int damagePercent = baseMonster.isElite() ? random.between(150, 178) : random.between(135, 160);

    bool hiddenStats = !baseMonster.areStatsVisible() || random.between(1, 100) <= 18;
    bool elite = true;

    Monster evolved = createMonster(
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

    if (baseMonster.doesSplitOnDeath())
    {
        evolved.copySplitBehaviorFrom(baseMonster);
    }

    return evolved;
}

Monster MonsterCatalog::createEliteVariant(const Monster& baseMonster, Random& random)
{
    int hpPercent = random.between(175, 215);
    int damagePercent = random.between(140, 168);
    bool hiddenStats = !baseMonster.areStatsVisible() || random.between(1, 100) <= 12;

    Monster eliteVariant = createMonster(
        baseMonster.getName() + " élite",
        baseMonster.getType() + " / élite locale",
        baseMonster.getRace(),
        baseMonster.getLevel(),
        scaledValue(baseMonster.getMaxHp(), hpPercent),
        scaledValue(baseMonster.getMinDamage(), damagePercent),
        scaledValue(baseMonster.getMaxDamage(), damagePercent),
        scaledValue(baseMonster.getCriticalDamage(), damagePercent + 4),
        baseMonster.getHealingPotionCount() + (random.between(1, 100) <= 18 ? 1 : 0),
        baseMonster.getDamagePotionCount() + (random.between(1, 100) <= 24 ? 1 : 0),
        baseMonster.isInvocation(),
        true,
        hiddenStats,
        baseMonster.isEvolved()
    );

    if (baseMonster.doesSplitOnDeath())
    {
        eliteVariant.copySplitBehaviorFrom(baseMonster);
    }

    return eliteVariant;
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

    allMonsters.push_back(createGiantSlimeMiniBoss(10));

    std::vector<std::string> biomes = {
        "Plaine sauvage",
        "Route commerciale",
        "Mares gélatineuses",
        "Forêt ancienne",
        "Montagne froide",
        "Marais trouble",
        "Cimetière oublié",
        "Ruines effondrées",
        "Bocage aux lanternes",
        "Désert d'argile rouge",
        "Quartier abandonné",
        "Mine sifflante",
        "Verger des lucioles de fer",
        "Archives noyées",
        "Falaises des drakes gris",
        "Foire abandonnée",
        "Temple des cloches fendues",
        "Canaux de brume bleue",
        "Carrière des os blancs",
        "Marché sous les ponts",
        "Jardin des statues qui pleurent",
        "Bois de la Corruption",
        "Crypte du Sombre-Lien",
        "Désert des Protecteurs",
        "Sanctuaire antique des Veilleurs",
        "Quartier des Lames Muettes",
        "Toits des Assassins",
        "Nid draconique rouge",
        "Coulées de lave noire",
        "Glacier des Serments froids",
        "Bosquet des Fées du Mana",
        "Sanctuaire kitsuné des Neuf Étincelles",
        "Confluence du Mana pur",
        "Bastion majeur scellé",
        "Archipel des îles flottantes",
        "Ponts translucides de mana",
        "Cieux des Légendes",
        "Parvis des Divinités"
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

