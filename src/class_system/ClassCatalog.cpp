// EN: ClassCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: ClassCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Central catalog for playable classes. It keeps the old direct index access, while adding category menus.
// Description : Catalogue central des classes jouables. Il conserve l'ancien accès par index direct tout en ajoutant les menus par catégories.

#include "class_system/ClassCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <vector>

namespace
{
    struct ClassTemplate
    {
        const char* name;
        int maxHp;
        int minDamage;
        int maxDamage;
        int criticalDamage;
        int healingPotionCount;
        int damagePotionCount;
        const char* role;
        ClassCategory category;
    };

    // EN: getClassTemplates declares or implements a focused behavior used by this module.
    // FR: getClassTemplates déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<ClassTemplate>& getClassTemplates()
    {
        static const std::vector<ClassTemplate> classes = {
            {"Chevalier", 200, 5, 20, 30, 4, 3, "équilibré, simple et fiable", ClassCategory::Melee},
            {"Assassin", 150, 11, 31, 48, 6, 2, "rapide, fragile, gros critique", ClassCategory::Melee},
            {"Colosse", 400, 4, 10, 18, 2, 1, "très tanky, lent, lourd", ClassCategory::Melee},
            {"Barbare", 260, 12, 28, 38, 3, 3, "brutal, violent, peu subtil", ClassCategory::Melee},
            {"Guerrier", 230, 9, 24, 34, 4, 3, "maîtrise des armes", ClassCategory::Melee},
            {"Moine", 175, 9, 25, 36, 5, 2, "esquive, discipline et poings", ClassCategory::Melee},
            {"Duelliste", 170, 12, 27, 43, 4, 2, "1v1, style et précision", ClassCategory::Melee},
            {"Berserker", 240, 14, 34, 46, 2, 4, "énormes dégâts, défense discutable", ClassCategory::Melee},
            {"Lancier", 210, 10, 27, 39, 4, 3, "portée, contrôle et contre-attaque", ClassCategory::Melee},
            {"Briseur lourd", 300, 15, 38, 58, 2, 2, "précision instable, dégâts énormes", ClassCategory::Melee},

            {"Archer", 160, 10, 29, 44, 5, 3, "distance, précision et mobilité", ClassCategory::Distance},
            {"Rôdeur", 180, 9, 27, 40, 5, 3, "distance, pistage et survie", ClassCategory::Distance},
            {"Arbalétrier", 175, 12, 32, 46, 3, 3, "tir lourd, plus lent mais puissant", ClassCategory::Distance},
            {"Chasseur", 190, 9, 24, 37, 4, 3, "traque, pièges et survie", ClassCategory::Distance},
            {"Lanceur de dagues", 150, 11, 26, 44, 5, 2, "distance courte, multi-projectiles", ClassCategory::Distance},
            {"Tireur", 165, 11, 31, 45, 3, 3, "visée, armes de trait et précision", ClassCategory::Distance},

            {"Mage", 140, 6, 29, 46, 4, 5, "magie classique, fragile mais dangereux", ClassCategory::Magic},
            {"Ensorceleur", 145, 8, 27, 42, 4, 4, "magie instinctive et explosive", ClassCategory::Magic},
            {"Sorcier", 160, 7, 25, 40, 4, 4, "pactes, malédictions et risques", ClassCategory::Magic},
            {"Magicien", 135, 6, 30, 48, 4, 5, "gros potentiel magique, fragile", ClassCategory::Magic},
            {"Pyromancien", 145, 8, 32, 50, 3, 5, "feu, burst et risques", ClassCategory::Magic},
            {"Cryomancien", 160, 6, 25, 38, 4, 4, "contrôle, ralentissement et survie", ClassCategory::Magic},
            {"Arcaniste", 150, 7, 31, 49, 4, 5, "magie pure et instable", ClassCategory::Magic},
            {"Occultiste", 155, 9, 29, 45, 3, 5, "ombres, pactes et dégâts étranges", ClassCategory::Magic},
            {"Mage fou", 140, 1, 36, 55, 3, 6, "résultats absurdes, parfois géniaux", ClassCategory::Magic},
            {"Mage flame", 150, 9, 34, 52, 3, 5, "flammes rapides et dégâts croissants", ClassCategory::Magic},
            {"Mage zodiacal", 155, 7, 33, 51, 4, 5, "sorts aléatoires liés aux signes", ClassCategory::Magic},

            {"Invocateur", 160, 6, 22, 34, 5, 4, "alliés invoqués et stratégie", ClassCategory::Invocation},
            {"Nécromancien", 150, 7, 26, 42, 4, 5, "morts-vivants, sacrifices et contrôle", ClassCategory::Invocation},
            {"Dompteur", 185, 8, 23, 35, 5, 3, "compagnons, bêtes et adaptation", ClassCategory::Invocation},
            {"Pactisant", 175, 9, 28, 43, 4, 4, "contrats dangereux et pouvoirs empruntés", ClassCategory::Invocation},

            {"Barde", 170, 6, 18, 28, 5, 3, "support, charme et chaos social", ClassCategory::Support},
            {"Clerc", 220, 6, 17, 26, 6, 2, "soin, foi et protection", ClassCategory::Support},
            {"Prêtre", 205, 5, 18, 28, 7, 2, "soin, lumière et soutien", ClassCategory::Support},
            {"Paladin", 280, 8, 22, 34, 5, 3, "tank sacré, protection et lumière", ClassCategory::Support},
            {"Gardien", 320, 4, 14, 22, 4, 1, "protection pure, très défensif", ClassCategory::Support},
            {"Templier", 260, 7, 22, 33, 5, 3, "foi martiale et anti-magie", ClassCategory::Support},
            {"Tank sacré", 340, 4, 15, 24, 4, 1, "mur vivant, entrave et lumière", ClassCategory::Support},

            {"Druide", 190, 7, 20, 30, 5, 3, "nature, survie et magie sauvage", ClassCategory::Hybrid},
            {"Chevalier runique", 240, 8, 26, 39, 4, 4, "arme lourde et magie gravée", ClassCategory::Hybrid},
            {"Ombrelame", 160, 12, 31, 50, 4, 3, "assassin magique, très agressif", ClassCategory::Hybrid},
            {"Mage-lame", 180, 9, 28, 43, 4, 4, "mélange arme réelle et magie", ClassCategory::Hybrid},
            {"Shaman", 180, 7, 24, 36, 5, 4, "esprits, rituels et soutien", ClassCategory::Hybrid},

            {"Artificier", 175, 8, 24, 36, 4, 4, "gadgets, précision et projectiles", ClassCategory::Craft},
            {"Forgeron", 185, 14, 28, 38, 3, 2, "force, forge et économie d'usure des kits", ClassCategory::Craft},
            {"Alchimiste", 165, 5, 18, 28, 6, 5, "potions, mélanges et effets instables", ClassCategory::Craft}
        };

        return classes;
    }

    // EN: displayClassTemplate declares or implements a focused behavior used by this module.
    // FR: displayClassTemplate déclare ou implémente un comportement précis utilisé par ce module.
    void displayClassTemplate(std::size_t displayedIndex, const ClassTemplate& currentClass)
    {
        std::cout << displayedIndex << " : " << currentClass.name << std::endl;
        std::cout << "    Famille : " << classCategoryToText(currentClass.category) << std::endl;
        std::cout << "    Rôle : " << currentClass.role << std::endl;
        std::cout << "    PV : " << currentClass.maxHp << std::endl;
        std::cout << "    Dégâts : " << currentClass.minDamage << " - " << currentClass.maxDamage
                  << " | Critique : " << currentClass.criticalDamage << std::endl;
        std::cout << "    Potions de soin : " << currentClass.healingPotionCount << std::endl;
        std::cout << "    Potions de dégâts : " << currentClass.damagePotionCount << std::endl;
        std::cout << std::endl;
    }

    std::vector<const ClassTemplate*> getClassesByCategory(ClassCategory category)
    {
        std::vector<const ClassTemplate*> filteredClasses;

        for (const ClassTemplate& currentClass : getClassTemplates())
        {
            if (currentClass.category == category)
            {
                filteredClasses.push_back(&currentClass);
            }
        }

        return filteredClasses;
    }

    // EN: createPlayerClassFromTemplate declares or implements a focused behavior used by this module.
    // FR: createPlayerClassFromTemplate déclare ou implémente un comportement précis utilisé par ce module.
    PlayerClass createPlayerClassFromTemplate(const ClassTemplate& selectedClass)
    {
        return PlayerClass(
            selectedClass.name,
            selectedClass.maxHp,
            selectedClass.minDamage,
            selectedClass.maxDamage,
            selectedClass.criticalDamage,
            selectedClass.healingPotionCount,
            selectedClass.damagePotionCount
        );
    }

    ClassOptionInfo toClassOptionInfo(const ClassTemplate& currentClass)
    {
        ClassOptionInfo info;
        info.name = currentClass.name;
        info.role = currentClass.role;
        info.categoryName = classCategoryToText(currentClass.category);
        info.maxHp = currentClass.maxHp;
        info.minDamage = currentClass.minDamage;
        info.maxDamage = currentClass.maxDamage;
        info.criticalDamage = currentClass.criticalDamage;
        info.healingPotionCount = currentClass.healingPotionCount;
        info.damagePotionCount = currentClass.damagePotionCount;
        return info;
    }
}

// EN: displayBasicClasses declares or implements a focused behavior used by this module.
// FR: displayBasicClasses déclare ou implémente un comportement précis utilisé par ce module.
void ClassCatalog::displayBasicClasses()
{
    const std::vector<ClassTemplate>& classes = getClassTemplates();

    for (std::size_t i = 0; i < classes.size(); ++i)
    {
        displayClassTemplate(i + 1, classes[i]);
    }
}

// EN: displayClassCategories declares or implements a focused behavior used by this module.
// FR: displayClassCategories déclare ou implémente un comportement précis utilisé par ce module.
void ClassCatalog::displayClassCategories()
{
    std::vector<ClassCategory> categories = getClassCategories();

    for (std::size_t i = 0; i < categories.size(); ++i)
    {
        ClassCategory category = categories[i];

        std::cout << (i + 1) << " : " << classCategoryToText(category)
                  << " (" << getPlayableClassCountByCategory(category) << " classes)"
                  << std::endl;
    }
}

// EN: displayClassesByCategory declares or implements a focused behavior used by this module.
// FR: displayClassesByCategory déclare ou implémente un comportement précis utilisé par ce module.
void ClassCatalog::displayClassesByCategory(ClassCategory category)
{
    std::vector<const ClassTemplate*> classes = getClassesByCategory(category);

    for (std::size_t i = 0; i < classes.size(); ++i)
    {
        displayClassTemplate(i + 1, *classes[i]);
    }
}

// EN: displayClassesByCategoryChoice declares or implements a focused behavior used by this module.
// FR: displayClassesByCategoryChoice déclare ou implémente un comportement précis utilisé par ce module.
void ClassCatalog::displayClassesByCategoryChoice(int categoryChoice)
{
    displayClassesByCategory(getClassCategoryByChoice(categoryChoice));
}

// EN: getPlayableClassCount declares or implements a focused behavior used by this module.
// FR: getPlayableClassCount déclare ou implémente un comportement précis utilisé par ce module.
int ClassCatalog::getPlayableClassCount()
{
    return static_cast<int>(getClassTemplates().size());
}

// EN: getClassCategoryCount declares or implements a focused behavior used by this module.
// FR: getClassCategoryCount déclare ou implémente un comportement précis utilisé par ce module.
int ClassCatalog::getClassCategoryCount()
{
    return static_cast<int>(getClassCategories().size());
}

// EN: getPlayableClassCountByCategory declares or implements a focused behavior used by this module.
// FR: getPlayableClassCountByCategory déclare ou implémente un comportement précis utilisé par ce module.
int ClassCatalog::getPlayableClassCountByCategory(ClassCategory category)
{
    return static_cast<int>(getClassesByCategory(category).size());
}

// EN: getPlayableClassCountByCategoryChoice declares or implements a focused behavior used by this module.
// FR: getPlayableClassCountByCategoryChoice déclare ou implémente un comportement précis utilisé par ce module.
int ClassCatalog::getPlayableClassCountByCategoryChoice(int categoryChoice)
{
    return getPlayableClassCountByCategory(getClassCategoryByChoice(categoryChoice));
}

std::vector<std::string> ClassCatalog::getPlayableClassNames()
{
    std::vector<std::string> names;

    for (const ClassTemplate& currentClass : getClassTemplates())
    {
        names.push_back(currentClass.name);
    }

    return names;
}

std::vector<ClassCategory> ClassCatalog::getClassCategories()
{
    return {
        ClassCategory::Melee,
        ClassCategory::Distance,
        ClassCategory::Magic,
        ClassCategory::Invocation,
        ClassCategory::Support,
        ClassCategory::Hybrid,
        ClassCategory::Craft
    };
}

// EN: getClassCategoryByChoice declares or implements a focused behavior used by this module.
// FR: getClassCategoryByChoice déclare ou implémente un comportement précis utilisé par ce module.
ClassCategory ClassCatalog::getClassCategoryByChoice(int categoryChoice)
{
    std::vector<ClassCategory> categories = getClassCategories();

    if (categoryChoice < 1 || categoryChoice > static_cast<int>(categories.size()))
    {
        categoryChoice = 1;
    }

    return categories[categoryChoice - 1];
}

std::string ClassCatalog::getClassCategoryNameByChoice(int categoryChoice)
{
    return classCategoryToText(getClassCategoryByChoice(categoryChoice));
}

std::vector<ClassOptionInfo> ClassCatalog::getClassOptionsByCategoryChoice(int categoryChoice)
{
    std::vector<ClassOptionInfo> options;
    ClassCategory category = getClassCategoryByChoice(categoryChoice);

    for (const ClassTemplate* currentClass : getClassesByCategory(category))
    {
        options.push_back(toClassOptionInfo(*currentClass));
    }

    return options;
}

// EN: createBaseClass declares or implements a focused behavior used by this module.
// FR: createBaseClass déclare ou implémente un comportement précis utilisé par ce module.
PlayerClass ClassCatalog::createBaseClass(int choice)
{
    const std::vector<ClassTemplate>& classes = getClassTemplates();

    if (choice < 1 || choice > static_cast<int>(classes.size()))
    {
        choice = 1;
    }

    return createPlayerClassFromTemplate(classes[choice - 1]);
}

// EN: createClassByCategoryChoice declares or implements a focused behavior used by this module.
// FR: createClassByCategoryChoice déclare ou implémente un comportement précis utilisé par ce module.
PlayerClass ClassCatalog::createClassByCategoryChoice(int categoryChoice, int classChoice)
{
    ClassCategory category = getClassCategoryByChoice(categoryChoice);
    std::vector<const ClassTemplate*> classes = getClassesByCategory(category);

    if (classes.empty())
    {
        return createBaseClass(1);
    }

    if (classChoice < 1 || classChoice > static_cast<int>(classes.size()))
    {
        classChoice = 1;
    }

    return createPlayerClassFromTemplate(*classes[classChoice - 1]);
}

// EN: createClassByName declares or implements a focused behavior used by this module.
// FR: createClassByName déclare ou implémente un comportement précis utilisé par ce module.
PlayerClass ClassCatalog::createClassByName(const std::string& className)
{
    return createClassFromNormalizedName(normalizeClassName(className));
}

// EN: createEvolvedClassFromClass declares or implements a focused behavior used by this module.
// FR: createEvolvedClassFromClass déclare ou implémente un comportement précis utilisé par ce module.
PlayerClass ClassCatalog::createEvolvedClassFromClass(const std::string& currentClass)
{
    std::string normalizedName = normalizeClassName(currentClass);

    if (normalizedName == normalizeClassName("Chevalier"))
    {
        return PlayerClass("Chevalier éveillé", 300, 9, 28, 42, 5, 3);
    }

    if (normalizedName == normalizeClassName("Assassin")
        || normalizedName == normalizeClassName("Classe Légère")
        || normalizedName == normalizeClassName("Roublard"))
    {
        return PlayerClass("Assassin éveillé", 210, 13, 34, 55, 6, 2);
    }

    if (normalizedName == normalizeClassName("Colosse")
        || normalizedName == normalizeClassName("Classe Lourde"))
    {
        return PlayerClass("Colosse éveillé", 500, 7, 18, 30, 2, 3);
    }

    if (normalizedName == normalizeClassName("Mage flame")
        || normalizedName == normalizeClassName("Pyromancien"))
    {
        return createClassByName("Arcaniste");
    }

    if (normalizedName == normalizeClassName("Tank sacré")
        || normalizedName == normalizeClassName("Sanctus"))
    {
        return createClassByName("Paladin");
    }

    if (normalizedName == normalizeClassName("Lancier"))
    {
        return PlayerClass("Maître de la hampe", 290, 12, 32, 48, 4, 3);
    }

    if (normalizedName == normalizeClassName("Archer")
        || normalizedName == normalizeClassName("Rôdeur")
        || normalizedName == normalizeClassName("Chasseur"))
    {
        return PlayerClass("Tireur éveillé", 230, 12, 34, 50, 4, 3);
    }

    if (normalizedName == normalizeClassName("Mage")
        || normalizedName == normalizeClassName("Magicien")
        || normalizedName == normalizeClassName("Ensorceleur")
        || normalizedName == normalizeClassName("Sorcier"))
    {
        return PlayerClass("Mage éveillé", 210, 9, 36, 58, 4, 5);
    }

    PlayerClass baseClass = createClassByName(currentClass);
    return PlayerClass(
        baseClass.getName() + " éveillé",
        baseClass.getMaxHp() + 70,
        baseClass.getMinDamage() + 2,
        baseClass.getMaxDamage() + 6,
        baseClass.getCriticalDamage() + 10,
        baseClass.getHealingPotionCount(),
        baseClass.getDamagePotionCount()
    );
}

// EN: createClassFromNormalizedName declares or implements a focused behavior used by this module.
// FR: createClassFromNormalizedName déclare ou implémente un comportement précis utilisé par ce module.
PlayerClass ClassCatalog::createClassFromNormalizedName(const std::string& normalizedName)
{
    const std::vector<ClassTemplate>& classes = getClassTemplates();

    for (const ClassTemplate& currentClass : classes)
    {
        if (normalizeClassName(currentClass.name) == normalizedName)
        {
            return createPlayerClassFromTemplate(currentClass);
        }
    }

    return createBaseClass(1);
}

std::string ClassCatalog::normalizeClassName(const std::string& className)
{
    std::string normalized = className;

    std::transform(
        normalized.begin(),
        normalized.end(),
        normalized.begin(),
        // EN: [] declares or implements a focused behavior used by this module.
        // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        }
    );

    return normalized;
}
