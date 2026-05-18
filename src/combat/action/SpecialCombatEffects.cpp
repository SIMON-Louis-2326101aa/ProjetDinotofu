// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/action/SpecialCombatEffects.hpp"

#include "core/Console.hpp"

#include "entity/Boss.hpp"

#include <iostream>

bool SpecialCombatEffects::atlasBlocksAttack(
    Entity& attacker,
    Entity& defender,
    int damage
)
{
    Boss* bossDefenseur = dynamic_cast<Boss*>(&defender);

    if (bossDefenseur == nullptr)
    {
        return false;
    }

    if (bossDefenseur->getBossId() != 3 || !bossDefenseur->isUltimateActive())
    {
        return false;
    }

    int reflectedDamage = damage / 3;

    attacker.takeDamage(reflectedDamage);

    std::cout << attacker.getName()
              << " frappe de toutes ses forces..."
              << std::endl;

    Console::pauseSeconds(1);

    std::cout << "Mais l'armure d'"
              << bossDefenseur->getName()
              << " absorbe l'impact."
              << std::endl;

    std::cout << "Une partie de la puissance est renvoyée à "
              << attacker.getName()
              << ", qui subit "
              << reflectedDamage
              << " dégâts."
              << std::endl;

    std::cout << std::endl;

    std::cout << attacker.getName()
              << " possède maintenant "
              << attacker.getHp()
              << "/"
              << attacker.getMaxHp()
              << " PV."
              << std::endl;

    std::cout << std::endl;

    return true;
}

void SpecialCombatEffects::applyDemonLifestealIfNeeded(
    Entity& attacker,
    int damageDealt
)
{
    Boss* bossAttaquant = dynamic_cast<Boss*>(&attacker);

    if (bossAttaquant == nullptr)
    {
        return;
    }

    if (bossAttaquant->getBossId() != 2 || !bossAttaquant->isUltimateActive())
    {
        return;
    }

    if (bossAttaquant->getSpecialEffect() != 2
        && bossAttaquant->getSpecialEffect() != 3)
    {
        return;
    }

    int healing = damageDealt * 50 / 100;

    if (healing <= 0)
    {
        return;
    }

    bossAttaquant->heal(healing);

    std::cout << bossAttaquant->getName()
              << " absorbe le sang de l'attaque et récupère "
              << healing
              << " PV."
              << std::endl;

    std::cout << std::endl;
}