// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_BOSS_BOSSCATALOG_HPP
#define INCLUDE_BOSS_BOSSCATALOG_HPP

#include "entity/Boss.hpp"

class BossCatalog
{
public:
    static void displayAvailableBosses();
    static Boss creerBoss(int choice);
};

#endif
