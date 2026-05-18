// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ENTITY_MONSTERCATALOG_HPP
#define INCLUDE_ENTITY_MONSTERCATALOG_HPP

#include "entity/Monster.hpp"

class MonsterCatalog
{
public:
    static Monster creerGobelin();
    static Monster creerGobelinBrutal();
    static Monster creerLoupAffame();
    static Monster creerSquelette();
    static Monster creerOrcMineur();

    static void displayAvailableMonsters();
};

#endif
