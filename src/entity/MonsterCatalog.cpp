// EN: MonsterCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: MonsterCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/MonsterCatalog.hpp"

#include <iostream>
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
    Monster chooseFromList(const std::vector<Monster>& monsters, Random& random)
    {
        int index = random.between(0, static_cast<int>(monsters.size()) - 1);
        return monsters[index];
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
    if (level <= 1)
    {
        return chooseFromList(createTierOneMonsters(), random);
    }

    if (level == 2)
    {
        return chooseFromList(createTierTwoMonsters(), random);
    }

    if (level == 3)
    {
        return chooseFromList(createTierThreeMonsters(), random);
    }

    if (level == 4)
    {
        return chooseFromList(createTierFourMonsters(), random);
    }

    return chooseFromList(createTierFivePlusMonsters(), random);
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

    return allMonsters;
}

// EN: displayAvailableMonsters declares or implements a focused behavior used by this module.
// FR: displayAvailableMonsters déclare ou implémente un comportement précis utilisé par ce module.
void MonsterCatalog::displayAvailableMonsters()
{
    std::vector<Monster> monsters = createAllPreviewMonsters();

    std::cout << "========== MONSTRES PRÉPARÉS ==========" << std::endl;

    for (std::size_t i = 0; i < monsters.size(); ++i)
    {
        std::cout << (i + 1) << " : " << monsters[i].getName()
                  << " | Race : " << monsters[i].getRaceText()
                  << " | Niveau : " << monsters[i].getLevel()
                  << std::endl;
    }

    std::cout << "=======================================" << std::endl;
    std::cout << std::endl;
}
