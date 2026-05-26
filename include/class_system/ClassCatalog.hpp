// EN: ClassCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: ClassCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Provides every playable class and utilities to display them by category.
// Description : Fournit toutes les classes jouables et les outils pour les afficher par catégorie.

#ifndef INCLUDE_CLASS_SYSTEM_CLASSCATALOG_HPP
#define INCLUDE_CLASS_SYSTEM_CLASSCATALOG_HPP

#include "class_system/ClassCategory.hpp"
#include "class_system/PlayerClass.hpp"

#include <string>
#include <vector>

struct ClassOptionInfo
{
    std::string name;
    std::string role;
    std::string categoryName;
    int maxHp = 0;
    int minDamage = 0;
    int maxDamage = 0;
    int criticalDamage = 0;
    int healingPotionCount = 0;
    int damagePotionCount = 0;
};

class ClassCatalog
{
public:
    // EN: displayBasicClasses declares or implements a focused behavior used by this module.
    // FR: displayBasicClasses déclare ou implémente un comportement précis utilisé par ce module.
    static void displayBasicClasses();
    // EN: displayClassCategories declares or implements a focused behavior used by this module.
    // FR: displayClassCategories déclare ou implémente un comportement précis utilisé par ce module.
    static void displayClassCategories();
    // EN: displayClassesByCategory declares or implements a focused behavior used by this module.
    // FR: displayClassesByCategory déclare ou implémente un comportement précis utilisé par ce module.
    static void displayClassesByCategory(ClassCategory category);
    // EN: displayClassesByCategoryChoice declares or implements a focused behavior used by this module.
    // FR: displayClassesByCategoryChoice déclare ou implémente un comportement précis utilisé par ce module.
    static void displayClassesByCategoryChoice(int categoryChoice);

    // EN: getPlayableClassCount declares or implements a focused behavior used by this module.
    // FR: getPlayableClassCount déclare ou implémente un comportement précis utilisé par ce module.
    static int getPlayableClassCount();
    // EN: getClassCategoryCount declares or implements a focused behavior used by this module.
    // FR: getClassCategoryCount déclare ou implémente un comportement précis utilisé par ce module.
    static int getClassCategoryCount();
    // EN: getPlayableClassCountByCategory declares or implements a focused behavior used by this module.
    // FR: getPlayableClassCountByCategory déclare ou implémente un comportement précis utilisé par ce module.
    static int getPlayableClassCountByCategory(ClassCategory category);
    // EN: getPlayableClassCountByCategoryChoice declares or implements a focused behavior used by this module.
    // FR: getPlayableClassCountByCategoryChoice déclare ou implémente un comportement précis utilisé par ce module.
    static int getPlayableClassCountByCategoryChoice(int categoryChoice);

    // EN: getPlayableClassNames declares or implements a focused behavior used by this module.
    // FR: getPlayableClassNames déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<std::string> getPlayableClassNames();
    // EN: getClassCategories declares or implements a focused behavior used by this module.
    // FR: getClassCategories déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<ClassCategory> getClassCategories();

    // EN: getClassCategoryByChoice declares or implements a focused behavior used by this module.
    // FR: getClassCategoryByChoice déclare ou implémente un comportement précis utilisé par ce module.
    static ClassCategory getClassCategoryByChoice(int categoryChoice);
    // EN: getClassCategoryNameByChoice declares or implements a focused behavior used by this module.
    // FR: getClassCategoryNameByChoice déclare ou implémente un comportement précis utilisé par ce module.
    static std::string getClassCategoryNameByChoice(int categoryChoice);

    static std::vector<ClassOptionInfo> getClassOptionsByCategoryChoice(int categoryChoice);

    // EN: createBaseClass declares or implements a focused behavior used by this module.
    // FR: createBaseClass déclare ou implémente un comportement précis utilisé par ce module.
    static PlayerClass createBaseClass(int choice);
    // EN: createClassByCategoryChoice declares or implements a focused behavior used by this module.
    // FR: createClassByCategoryChoice déclare ou implémente un comportement précis utilisé par ce module.
    static PlayerClass createClassByCategoryChoice(int categoryChoice, int classChoice);
    // EN: createClassByName declares or implements a focused behavior used by this module.
    // FR: createClassByName déclare ou implémente un comportement précis utilisé par ce module.
    static PlayerClass createClassByName(const std::string& className);
    // EN: createEvolvedClassFromClass declares or implements a focused behavior used by this module.
    // FR: createEvolvedClassFromClass déclare ou implémente un comportement précis utilisé par ce module.
    static PlayerClass createEvolvedClassFromClass(const std::string& currentClass);

private:
    // EN: createClassFromNormalizedName declares or implements a focused behavior used by this module.
    // FR: createClassFromNormalizedName déclare ou implémente un comportement précis utilisé par ce module.
    static PlayerClass createClassFromNormalizedName(const std::string& normalizedName);
    // EN: normalizeClassName declares or implements a focused behavior used by this module.
    // FR: normalizeClassName déclare ou implémente un comportement précis utilisé par ce module.
    static std::string normalizeClassName(const std::string& className);
};

#endif
