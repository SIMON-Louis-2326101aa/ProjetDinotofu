// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/TurnManager.hpp"

#include "combat/CombatActions.hpp"
#include "combat/BossCombat.hpp"

#include "combat/turn/HumanCombatTurn.hpp"
#include "combat/turn/AICombatTurn.hpp"
#include "combat/turn/BossCombatTurn.hpp"

bool TurnManager::playHumanTurn(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int potionHealAmount,
    int potionDamageBonus
)
{
    return HumanCombatTurn::jouer(
        attacker,
        defender,
        random,
        potionHealAmount,
        potionDamageBonus
    );
}

bool TurnManager::playAITurn(
    Entity& ai,
    Entity& defender,
    Random& random,
    int potionHealAmount,
    int potionDamageBonus
)
{
    return AICombatTurn::jouer(
        ai,
        defender,
        random,
        potionHealAmount,
        potionDamageBonus
    );
}

bool TurnManager::playBossTurn(
    Boss& boss,
    Entity& player,
    Random& random
)
{
    return BossCombatTurn::jouer(
        boss,
        player,
        random
    );
}

void TurnManager::checkBossDecryption(Boss& boss)
{
    BossCombat::checkBossDecryption(boss);
}

void TurnManager::executeAttack(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    CombatActions::executeAttack(attacker, defender, random);
}

bool TurnManager::executeHealingPotion(
    Entity& entity,
    int potionHealAmount
)
{
    return CombatActions::executeHealingPotion(entity, potionHealAmount);
}

bool TurnManager::executeDamagePotion(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int potionDamageBonus
)
{
    return CombatActions::executeDamagePotion(
        attacker,
        defender,
        random,
        potionDamageBonus
    );
}