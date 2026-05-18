// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_CLASS_SYSTEM_CLASSCATALOG_HPP
#define INCLUDE_CLASS_SYSTEM_CLASSCATALOG_HPP

#include "class_system/PlayerClass.hpp"

class ClassCatalog
{
public:
    static void displayBasicClasses();

    static PlayerClass createBaseClass(int choice);
    static PlayerClass createEvolvedClassFromClass(const std::string& currentClass);
};

#endif
