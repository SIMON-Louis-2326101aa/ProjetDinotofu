// EN: TurnManager.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: TurnManager.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    return HumanCombatTurn::play(
        attacker,
        defender,
        random,
        potionHealAmount,
        potionDamageBonus
    );
}

bool TurnManager::playHumanTurnWithEnemySummons(
    Entity& attacker,
    Entity& defender,
    std::vector<Summon>& enemySummons,
    Random& random,
    int potionHealAmount,
    int potionDamageBonus
)
{
    return HumanCombatTurn::playWithEnemySummons(
        attacker,
        defender,
        enemySummons,
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
    return AICombatTurn::play(
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
    return BossCombatTurn::play(
        boss,
        player,
        random
    );
}

// EN: checkBossDecryption declares or implements a focused behavior used by this module.
// FR: checkBossDecryption déclare ou implémente un comportement précis utilisé par ce module.
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
