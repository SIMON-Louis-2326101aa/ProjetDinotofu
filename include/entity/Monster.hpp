// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ENTITY_MONSTER_HPP
#define INCLUDE_ENTITY_MONSTER_HPP

#include "entity/Entity.hpp"
#include "entity/Race.hpp"

class Monster : public Entity
{
private:
    int level;
    Race race;

    bool invocation;
    bool elite;
    bool statsCachees;

public:
    Monster();

    Monster(
        const std::string& name,
        const std::string& type,
        Race race,
        int level,
        int maxHp,
        int minDamage,
        int maxDamage,
        int criticalDamage,
        int healingPotions,
        int damagePotions,
        bool invocation = false,
        bool elite = false,
        bool statsCachees = false
    );

    int getLevel() const;
    Race getRace() const;
    std::string getRaceTexte() const;

    bool estInvocation() const;
    bool estElite() const;

    bool areStatsVisible() const override;
    void revelerStats();

    void displayStats() const override;
};

#endif
