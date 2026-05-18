// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/Monster.hpp"

#include <iostream>

Monster::Monster()
    : Entity(),
      level(1),
      race(Race::Unknown),
      invocation(false),
      elite(false),
      hiddenStats(false)
{
}

Monster::Monster(
    const std::string& name,
    const std::string& type,
    Race race,
    int level,
    int maxHp,
    int minDamage,
    int maxDamage,
    int criticalDamage,
    int healingPotionCount,
    int damagePotionCount,
    bool invocation,
    bool elite,
    bool hiddenStats
)
    : Entity(
          name,
          type,
          maxHp,
          minDamage,
          maxDamage,
          criticalDamage,
          healingPotionCount,
          damagePotionCount
      ),
      level(level),
      race(race),
      invocation(invocation),
      elite(elite),
      hiddenStats(hiddenStats)
{
}

int Monster::getLevel() const
{
    return level;
}

Race Monster::getRace() const
{
    return race;
}

std::string Monster::getRaceText() const
{
    return raceVersTexte(race);
}

bool Monster::isInvocation() const
{
    return invocation;
}

bool Monster::isElite() const
{
    return elite;
}

bool Monster::areStatsVisible() const
{
    return !hiddenStats;
}

void Monster::revealStats()
{
    hiddenStats = false;
}

void Monster::displayStats() const
{
    if (!areStatsVisible())
    {
        std::cout << "========== DONNÉES BROUILLÉES ==========" << std::endl;
        std::cout << "Nom : " << name << std::endl;
        std::cout << "Race : " << getRaceText() << std::endl;
        std::cout << "Type : " << type << std::endl;
        std::cout << "Niveau : ???" << std::endl;
        std::cout << "PV : ???" << std::endl;
        std::cout << "Dégâts : ???" << std::endl;
        std::cout << "Critique : ???" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << std::endl;
        return;
    }

    std::cout << "========== STATS MONSTRE ==========" << std::endl;
    std::cout << "Nom : " << name << std::endl;
    std::cout << "Race : " << getRaceText() << std::endl;
    std::cout << "Type : " << type << std::endl;
    std::cout << "Niveau : " << level << std::endl;
    std::cout << "PV : " << hp << "/" << maxHp << std::endl;
    std::cout << "Dégâts : " << minDamage << " - " << maxDamage << std::endl;
    std::cout << "Critique : " << criticalDamage << std::endl;

    if (invocation)
    {
        std::cout << "Statut : Invocation" << std::endl;
    }
    else if (elite)
    {
        std::cout << "Statut : Élite" << std::endl;
    }
    else
    {
        std::cout << "Statut : Monstre standard" << std::endl;
    }

    std::cout << "===================================" << std::endl;
    std::cout << std::endl;
}