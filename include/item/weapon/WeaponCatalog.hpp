// EN: WeaponCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: WeaponCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ITEM_WEAPON_WEAPONCATALOG_HPP
#define INCLUDE_ITEM_WEAPON_WEAPONCATALOG_HPP

#include "item/weapon/Weapon.hpp"

#include <string>

class WeaponCatalog
{
public:
    // EN: createBareHands declares or implements a focused behavior used by this module.
    // FR: createBareHands déclare ou implémente un comportement précis utilisé par ce module.
    static Weapon createBareHands();
    // EN: createRustySword declares or implements a focused behavior used by this module.
    // FR: createRustySword déclare ou implémente un comportement précis utilisé par ce module.
    static Weapon createRustySword();
    static Weapon createTrainingDagger();
    static Weapon createTrainingSpear();
    static Weapon createTrainingBow();
    static Weapon createTrainingCrossbow();
    static Weapon createTrainingThrowingBandolier();
    static Weapon createTrainingStaff();
    static Weapon createHeavyTrainingAxe();
    static Weapon createEmergencyWoodKnife();
    static Weapon createStarterWeaponForClass(const std::string& className);
    // EN: createArenaBlade declares or implements a focused behavior used by this module.
    // FR: createArenaBlade déclare ou implémente un comportement précis utilisé par ce module.
    static Weapon createArenaBlade();
};

#endif
