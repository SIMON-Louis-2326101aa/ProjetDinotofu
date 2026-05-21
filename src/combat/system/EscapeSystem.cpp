// EN: EscapeSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: EscapeSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/EscapeSystem.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "progression/DifficultyRules.hpp"

#include "core/Console.hpp"

#include <algorithm>
#include <iostream>

// EN: playerAttemptsEscape declares or implements a focused behavior used by this module.
// FR: playerAttemptsEscape déclare ou implémente un comportement précis utilisé par ce module.
bool EscapeSystem::playerAttemptsEscape(Player& player, Random& random)
{
    return playerAttemptsEscape(player, random, DifficultyMode::Normal);
}

// EN: playerAttemptsEscape declares or implements a focused behavior used by this module.
// FR: playerAttemptsEscape déclare ou implémente un comportement précis utilisé par ce module.
bool EscapeSystem::playerAttemptsEscape(Player& player, Random& random, DifficultyMode difficulty)
{
    std::cout << player.getName() << " cherche une ouverture pour fuir le combat..." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(1);

    int escapeChance = CombatClassSystem::getBaseEscapeChance(player)
        + DifficultyRules::getPlayerEscapeChanceModifier(difficulty);

    escapeChance = std::max(10, std::min(escapeChance, 90));
    int roll = random.between(1, 100);

    std::cout << "Chance estimée de fuite : " << escapeChance << "%" << std::endl;
    std::cout << std::endl;

    if (roll <= escapeChance)
    {
        std::cout << "Fuite réussie." << std::endl;
        std::cout << player.getName()
                  << " parvient à quitter l'affrontement avant d'être encerclé."
                  << std::endl;
        std::cout << std::endl;

        return true;
    }

    std::cout << "Fuite échouée." << std::endl;
    std::cout << "Les ennemis bloquent le passage. Ton tour est perdu." << std::endl;
    std::cout << std::endl;

    return false;
}

// EN: playerAttemptsBossEscape declares or implements a focused behavior used by this module.
// FR: playerAttemptsBossEscape déclare ou implémente un comportement précis utilisé par ce module.
bool EscapeSystem::playerAttemptsBossEscape(const Player& player, const Boss& boss)
{
    std::cout << player.getName() << " cherche une issue..." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(1);

    std::cout << "L'air devient lourd." << std::endl;
    std::cout << "L'arène semble se refermer comme une cage." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(1);

    std::cout << boss.getName() << " bloque toute échappatoire par sa seule présence." << std::endl;
    std::cout << "L'arène ne possède plus de sortie : elle n'attend qu'un vainqueur, ou un corps." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(1);

    std::cout << "Tu fais un pas vers la fuite." << std::endl;
    std::cout << boss.getName() << " tourne lentement la tête vers toi." << std::endl;
    std::cout << "Même le silence te repousse vers le centre du combat." << std::endl;
    std::cout << "La tentative échoue. Ton tour est perdu." << std::endl;
    std::cout << std::endl;

    return false;
}

bool EscapeSystem::playerAttemptsDuelEscape(
    Player& runner,
    Entity& opponent,
    Random& random
)
{
    std::cout << runner.getName() << " cherche une ouverture pour quitter le duel..." << std::endl;
    std::cout << "En duel, fuir revient à abandonner le combat." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(1);

    int escapeChance = calculateDuelEscapeChance(runner, opponent);
    int roll = random.between(1, 100);

    if (roll <= escapeChance)
    {
        std::cout << "Fuite réussie." << std::endl;
        std::cout << runner.getName() << " sort de l'arène avant de se faire achever." << std::endl;
        std::cout << opponent.getName() << " remporte l'affrontement." << std::endl;
        std::cout << std::endl;

        std::cout << "Répartition prévue des récompenses :" << std::endl;
        std::cout << "- " << runner.getName() << " : 25%" << std::endl;
        std::cout << "- " << opponent.getName() << " : 75%" << std::endl;
        std::cout << std::endl;

        endCombatBySurrender(runner);
        return true;
    }

    std::cout << "Fuite échouée." << std::endl;
    std::cout << opponent.getName()
              << " coupe la trajectoire de "
              << runner.getName()
              << "."
              << std::endl;
    std::cout << "Le duel continue, mais le tour est perdu." << std::endl;
    std::cout << std::endl;

    return false;
}

// EN: monsterCanAttemptEscape declares or implements a focused behavior used by this module.
// FR: monsterCanAttemptEscape déclare ou implémente un comportement précis utilisé par ce module.
bool EscapeSystem::monsterCanAttemptEscape(const Monster& monster)
{
    if (monster.isInvocation())
    {
        return false;
    }

    if (monster.isElite())
    {
        return false;
    }

    if (monster.getHealingPotionCount() > 0)
    {
        return false;
    }

    if (monster.getMaxHp() <= 0)
    {
        return false;
    }

    int hpPercentage = monster.getHp() * 100 / monster.getMaxHp();

    return hpPercentage <= 10;
}

// EN: monsterAttemptsEscape declares or implements a focused behavior used by this module.
// FR: monsterAttemptsEscape déclare ou implémente un comportement précis utilisé par ce module.
bool EscapeSystem::monsterAttemptsEscape(Monster& monster, Random& random)
{
    if (!monsterCanAttemptEscape(monster))
    {
        return false;
    }

    int roll = random.between(1, 100);

    if (roll <= LOW_MONSTER_ESCAPE_CHANCE)
    {
        std::cout << monster.getName() << " panique et tente de fuir..." << std::endl;
        Console::pauseSeconds(1);

        std::cout << monster.getName()
                  << " disparaît de l'affrontement avant de recevoir le coup fatal."
                  << std::endl;
        std::cout << std::endl;

        return true;
    }

    return false;
}

int EscapeSystem::calculateDuelEscapeChance(
    const Player& runner,
    const Entity& opponent
)
{
    int chance = CombatClassSystem::getBaseEscapeChance(runner);

    const Player* opposingPlayer = dynamic_cast<const Player*>(&opponent);

    if (opposingPlayer != nullptr)
    {
        int ecartNiveau = opposingPlayer->getLevel() - runner.getLevel();
        chance -= ecartNiveau * 5;
    }

    int chanceAdversaire = CombatClassSystem::getBaseEscapeChance(opponent);

    if (chanceAdversaire <= 35)
    {
        chance -= 10;
    }
    else if (chanceAdversaire >= 65)
    {
        chance += 5;
    }

    return std::max(15, std::min(chance, 85));
}

// EN: endCombatBySurrender declares or implements a focused behavior used by this module.
// FR: endCombatBySurrender déclare ou implémente un comportement précis utilisé par ce module.
void EscapeSystem::endCombatBySurrender(Player& runner)
{
    runner.takeDamage(runner.getHp());
}
