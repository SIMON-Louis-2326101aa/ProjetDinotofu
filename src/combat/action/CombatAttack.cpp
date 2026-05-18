// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/action/CombatAttack.hpp"

#include "combat/DamageReport.hpp"
#include "combat/system/DamageSystem.hpp"
#include "combat/action/SpecialCombatEffects.hpp"

#include <iostream>

void CombatAttack::executeAttack(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    executeBoostedAttack(
        attacker,
        defender,
        random,
        0
    );
}

void CombatAttack::executeBoostedAttack(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int damageBonus
)
{
    bool dodged = false;
    bool critical = false;

    int rawDamage = attacker.attack(
        random,
        dodged,
        critical,
        damageBonus
    );

    if (dodged)
    {
        std::cout << attacker.getName()
                  << " attaque, mais "
                  << defender.getName()
                  << " esquive au dernier moment."
                  << std::endl;
        std::cout << std::endl;
        return;
    }

    if (SpecialCombatEffects::atlasBlocksAttack(
        attacker,
        defender,
        rawDamage
    ))
    {
        return;
    }

    if (critical)
    {
        std::cout << attacker.getName()
                  << " frappe avec une violence monstrueuse et inflige "
                  << rawDamage
                  << " dégâts bruts critiques."
                  << std::endl;
    }
    else if (damageBonus > 0)
    {
        std::cout << attacker.getName()
                  << " attaque avec une puissance renforcée et inflige "
                  << rawDamage
                  << " dégâts bruts."
                  << std::endl;
    }
    else
    {
        std::cout << attacker.getName()
                  << " attaque et inflige "
                  << rawDamage
                  << " dégâts bruts."
                  << std::endl;
    }

    DamageReport rapport = DamageSystem::calculateReceivedDamage(
        defender,
        rawDamage
    );

    DamageSystem::displayDamageReport(
        defender,
        rapport
    );

    defender.takeDamage(rapport.receivedDamage);

    SpecialCombatEffects::applyDemonLifestealIfNeeded(
        attacker,
        rapport.receivedDamage
    );

    std::cout << defender.getName()
              << " reçoit "
              << rapport.receivedDamage
              << " dégâts."
              << std::endl;

    std::cout << defender.getName()
              << " possède maintenant "
              << defender.getHp()
              << "/"
              << defender.getMaxHp()
              << " PV."
              << std::endl;

    std::cout << std::endl;
}