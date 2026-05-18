// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ENTITY_MONSTERCATALOG_HPP
#define INCLUDE_ENTITY_MONSTERCATALOG_HPP

#include "entity/Monster.hpp"
#include "core/Random.hpp"

class MonsterCatalog
{
public:
    static Monster createScaredGoblin();
    static Monster createBrutalGoblin();
    static Monster createStarvingWolf();
    static Monster createCrackedSkeleton();
    static Monster createMinorOrc();
    static Monster createCaveBat();
    static Monster createWildBoar();
    static Monster createLostBandit();

    static Monster createRandomMonsterForLevel(int level, Random& random);

    static void displayAvailableMonsters();
};

#endif
