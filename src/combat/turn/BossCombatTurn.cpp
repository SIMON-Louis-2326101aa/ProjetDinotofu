// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/BossCombatTurn.hpp"

#include "combat/CombatActions.hpp"
#include "combat/BossCombat.hpp"
#include "combat/ai/CombatAI.hpp"
#include "combat/ai/AIAction.hpp"

#include "core/Console.hpp"

#include <iostream>

bool BossCombatTurn::jouer(
    Boss& boss,
    Entity& player,
    Random& random
)
{
    std::cout << "Tour de " << boss.getName() << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(1);

    AIAction action = CombatAI::chooseBossAction(boss, random);

    if (action == AIAction::Attack)
    {
        CombatActions::executeAttack(boss, player, random);
        return BossCombat::handleBossEndTurn(boss, player);
    }

    if (action == AIAction::HealingPotion)
    {
        bool potionUsed = boss.useHealingPotion(boss.getMaxHp() * 10 / 100);

        if (potionUsed)
        {
            std::cout << boss.getName() << " récupère une partie de sa vitalité." << std::endl;
            std::cout << std::endl;
        }
        else
        {
            std::cout << boss.getName()
                      << " cherche une source de régénération, mais rien ne répond."
                      << std::endl;
            std::cout << std::endl;

            CombatActions::executeAttack(boss, player, random);
        }

        return BossCombat::handleBossEndTurn(boss, player);
    }

    if (action == AIAction::DamagePotion)
    {
        bool potionUsed = CombatActions::executeDamagePotion(
            boss,
            player,
            random,
            50
        );

        if (!potionUsed)
        {
            CombatActions::executeAttack(boss, player, random);
        }

        return BossCombat::handleBossEndTurn(boss, player);
    }

    if (action == AIAction::Ultimate)
    {
        BossCombat::executeBossUltimate(boss, player, random);
        return BossCombat::handleBossEndTurn(boss, player);
    }

    std::cout << boss.getName()
              << " reste immobile, comme s'il observait déjà ta fin."
              << std::endl;
    std::cout << std::endl;

    return BossCombat::handleBossEndTurn(boss, player);
}