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

class ClassCatalog
{
public:
    static void displayBasicClasses();
    static void displayClassCategories();
    static void displayClassesByCategory(ClassCategory category);
    static void displayClassesByCategoryChoice(int categoryChoice);

    static int getPlayableClassCount();
    static int getClassCategoryCount();
    static int getPlayableClassCountByCategory(ClassCategory category);
    static int getPlayableClassCountByCategoryChoice(int categoryChoice);

    static std::vector<std::string> getPlayableClassNames();
    static std::vector<ClassCategory> getClassCategories();

    static ClassCategory getClassCategoryByChoice(int categoryChoice);
    static std::string getClassCategoryNameByChoice(int categoryChoice);

    static PlayerClass createBaseClass(int choice);
    static PlayerClass createClassByCategoryChoice(int categoryChoice, int classChoice);
    static PlayerClass createClassByName(const std::string& className);
    static PlayerClass createEvolvedClassFromClass(const std::string& currentClass);

private:
    static PlayerClass createClassFromNormalizedName(const std::string& normalizedName);
    static std::string normalizeClassName(const std::string& className);
};

#endif
