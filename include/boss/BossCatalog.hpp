// EN: BossCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: BossCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_BOSS_BOSSCATALOG_HPP
#define INCLUDE_BOSS_BOSSCATALOG_HPP

#include "entity/Boss.hpp"

#include <string>
#include <vector>

class BossCatalog
{
public:
    // EN: displayAvailableBosses declares or implements a focused behavior used by this module.
    // FR: displayAvailableBosses déclare ou implémente un comportement précis utilisé par ce module.
    static void displayAvailableBosses();
    static void displayAvailableBosses(const std::vector<int>& bossIds);
    static std::string getRegistryDisplayName(int bossId);
    static std::string getRegistryHint(int bossId);
    // EN: getMaximumBossId declares or implements a focused behavior used by this module.
    // FR: getMaximumBossId déclare ou implémente un comportement précis utilisé par ce module.
    static int getMaximumBossId();
    // EN: createBoss declares or implements a focused behavior used by this module.
    // FR: createBoss déclare ou implémente un comportement précis utilisé par ce module.
    static Boss createBoss(int choice);
};

#endif
