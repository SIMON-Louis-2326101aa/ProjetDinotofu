// EN: MonsterCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: MonsterCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ENTITY_MONSTERCATALOG_HPP
#define INCLUDE_ENTITY_MONSTERCATALOG_HPP

#include "entity/Monster.hpp"
#include "core/Random.hpp"

#include <vector>

class MonsterCatalog
{
public:
    // EN: createScaredGoblin declares or implements a focused behavior used by this module.
    // FR: createScaredGoblin déclare ou implémente un comportement précis utilisé par ce module.
    static Monster createScaredGoblin();
    // EN: createBrutalGoblin declares or implements a focused behavior used by this module.
    // FR: createBrutalGoblin déclare ou implémente un comportement précis utilisé par ce module.
    static Monster createBrutalGoblin();
    // EN: createStarvingWolf declares or implements a focused behavior used by this module.
    // FR: createStarvingWolf déclare ou implémente un comportement précis utilisé par ce module.
    static Monster createStarvingWolf();
    // EN: createCrackedSkeleton declares or implements a focused behavior used by this module.
    // FR: createCrackedSkeleton déclare ou implémente un comportement précis utilisé par ce module.
    static Monster createCrackedSkeleton();
    // EN: createMinorOrc declares or implements a focused behavior used by this module.
    // FR: createMinorOrc déclare ou implémente un comportement précis utilisé par ce module.
    static Monster createMinorOrc();
    // EN: createCaveBat declares or implements a focused behavior used by this module.
    // FR: createCaveBat déclare ou implémente un comportement précis utilisé par ce module.
    static Monster createCaveBat();
    // EN: createWildBoar declares or implements a focused behavior used by this module.
    // FR: createWildBoar déclare ou implémente un comportement précis utilisé par ce module.
    static Monster createWildBoar();
    // EN: createLostBandit declares or implements a focused behavior used by this module.
    // FR: createLostBandit déclare ou implémente un comportement précis utilisé par ce module.
    static Monster createLostBandit();

    // EN: createRandomMonsterForLevel declares or implements a focused behavior used by this module.
    // FR: createRandomMonsterForLevel déclare ou implémente un comportement précis utilisé par ce module.
    static Monster createRandomMonsterForLevel(int level, Random& random);
    // EN: createEvolvedVariant declares or implements a focused behavior used by this module.
    // FR: createEvolvedVariant déclare ou implémente un comportement précis utilisé par ce module.
    static Monster createEvolvedVariant(const Monster& baseMonster, Random& random);
    // EN: createAllPreviewMonsters declares or implements a focused behavior used by this module.
    // FR: createAllPreviewMonsters déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<Monster> createAllPreviewMonsters();

    // EN: displayAvailableMonsters declares or implements a focused behavior used by this module.
    // FR: displayAvailableMonsters déclare ou implémente un comportement précis utilisé par ce module.
    static void displayAvailableMonsters();
};

#endif
