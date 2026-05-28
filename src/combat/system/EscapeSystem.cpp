// EN: EscapeSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: EscapeSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/EscapeSystem.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "progression/DifficultyRules.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    int requiredEscapeSuccessesForEnemyCount(int enemyCount)
    {
        if (enemyCount <= 3)
        {
            return 1;
        }

        // EN: Rounded to nearest for enemyCount / 3.0, with .5 and above rounded up.
        // FR: Arrondi au plus proche pour nbAdversaires / 3.0, avec .5 et plus vers le haut.
        int required = (enemyCount + 1) / 3;
        return std::max(1, required);
    }
}

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
    return playerAttemptsEscape(player, random, difficulty, 1);
}

bool EscapeSystem::playerAttemptsEscape(Player& player, Random& random, DifficultyMode difficulty, int enemyCount)
{
    int escapeChance = CombatClassSystem::getBaseEscapeChance(player)
        + DifficultyRules::getPlayerEscapeChanceModifier(difficulty);

    escapeChance = std::max(10, std::min(escapeChance, 90));

    if (enemyCount < 1)
    {
        enemyCount = 1;
    }

    int requiredSuccesses = requiredEscapeSuccessesForEnemyCount(enemyCount);
    int successes = 0;
    std::vector<std::string> lines;

    lines.push_back(player.getName() + " cherche une ouverture pour fuir le combat...");
    lines.push_back("Adversaires encore capables de bloquer la fuite : " + std::to_string(enemyCount));
    lines.push_back("Réussites nécessaires : " + std::to_string(requiredSuccesses));
    lines.push_back("Chance estimée par ouverture : " + std::to_string(escapeChance) + "%");

    for (int attempt = 1; attempt <= requiredSuccesses; ++attempt)
    {
        int roll = random.between(1, 100);

        if (roll <= escapeChance)
        {
            ++successes;
            lines.push_back("Ouverture " + std::to_string(attempt) + "/" + std::to_string(requiredSuccesses) + " : réussie.");
            continue;
        }

        lines.push_back("Ouverture " + std::to_string(attempt) + "/" + std::to_string(requiredSuccesses) + " : bloquée.");
        lines.push_back("Résultat : fuite échouée.");
        lines.push_back("Trop de présences ferment la trajectoire. Le tour est perdu.");

        MessageScreen::show("FUITE ÉCHOUÉE", "combat.escape.result.failed", lines);
        return false;
    }

    lines.push_back("Résultat : fuite réussie.");
    lines.push_back(player.getName() + " enchaîne assez d'ouvertures pour quitter l'affrontement avant d'être encerclé.");

    MessageScreen::show("FUITE RÉUSSIE", "combat.escape.result.success", lines);
    return successes >= requiredSuccesses;
}

// EN: playerAttemptsBossEscape declares or implements a focused behavior used by this module.
// FR: playerAttemptsBossEscape déclare ou implémente un comportement précis utilisé par ce module.
bool EscapeSystem::playerAttemptsBossEscape(const Player& player, const Boss& boss)
{
    MessageScreen::show(
        "FUITE IMPOSSIBLE",
        "combat.escape.boss.blocked",
        {
            player.getName() + " cherche une issue...",
            "L'air devient lourd.",
            "L'arène semble se refermer comme une cage.",
            boss.getName() + " bloque toute échappatoire par sa seule présence.",
            "L'arène ne possède plus de sortie : elle n'attend qu'un vainqueur, ou un corps.",
            "La tentative échoue. Le tour est perdu."
        }
    );

    return false;
}

bool EscapeSystem::playerAttemptsDuelEscape(
    Player& runner,
    Entity& opponent,
    Random& random
)
{
    int escapeChance = calculateDuelEscapeChance(runner, opponent);
    int roll = random.between(1, 100);
    std::vector<std::string> lines;

    lines.push_back(runner.getName() + " cherche une ouverture pour quitter le duel...");
    lines.push_back("En duel, fuir revient à abandonner le combat.");
    lines.push_back("Chance estimée : " + std::to_string(escapeChance) + "%");

    if (roll <= escapeChance)
    {
        lines.push_back("Résultat : fuite réussie.");
        lines.push_back(runner.getName() + " sort de l'arène avant de se faire achever.");
        lines.push_back(opponent.getName() + " remporte l'affrontement.");
        lines.push_back("Répartition prévue des récompenses :");
        lines.push_back("- " + runner.getName() + " : 25%");
        lines.push_back("- " + opponent.getName() + " : 75%");

        MessageScreen::show("ABANDON DU DUEL", "combat.escape.duel.success", lines);
        endCombatBySurrender(runner);
        return true;
    }

    lines.push_back("Résultat : fuite échouée.");
    lines.push_back(opponent.getName() + " coupe la trajectoire de " + runner.getName() + ".");
    lines.push_back("Le duel continue, mais le tour est perdu.");

    MessageScreen::show("FUITE DE DUEL ÉCHOUÉE", "combat.escape.duel.failed", lines);
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
        MessageScreen::show(
            "ENNEMI EN FUITE",
            "combat.escape.monster.success",
            {
                monster.getName() + " panique et tente de fuir...",
                monster.getName() + " disparaît de l'affrontement avant de recevoir le coup fatal."
            }
        );

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
