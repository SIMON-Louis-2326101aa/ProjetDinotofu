// EN: WaveCombatSystem.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: WaveCombatSystem.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/system/WaveCombatSystem.hpp"

#include "combat/wave/WaveGenerator.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <algorithm>
#include <cctype>
#include <random>
#include <string>
#include <vector>


namespace
{
    std::string lowerAscii(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool containsAny(const std::string& value, const std::vector<std::string>& words)
    {
        for (const std::string& word : words)
        {
            if (value.find(word) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    std::string randomLine(const std::vector<std::string>& lines)
    {
        if (lines.empty())
        {
            return "";
        }

        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(0, static_cast<int>(lines.size()) - 1);
        return lines[distribution(generator)];
    }

    std::vector<std::string> getIntelligentEnemyIntroLines(const Monster& monster)
    {
        const std::string focus = lowerAscii(monster.getName() + " " + monster.getType() + " " + monster.getRaceText());
        const bool canSpeak = containsAny(focus, {
            "bandit", "voleur", "gobelin", "goblin", "orc", "shaman", "chamane", "oracle",
            "mage", "apothicaire", "soldat", "mercenaire", "humain", "elfe", "démon", "demon"
        });

        if (!canSpeak)
        {
            return {};
        }

        if (containsAny(focus, {"gobelin", "goblin"}))
        {
            return {
                monster.getName() + " crache quelques mots dans sa langue : « Grak'thûm, shin rava ! »",
                "Tu ne comprends pas tout, mais le rire derrière la phrase est assez clair."
            };
        }

        if (containsAny(focus, {"orc"}))
        {
            return {
                monster.getName() + " frappe son arme contre sa paume et gronde dans une langue rocailleuse.",
                "Même sans traduction, la diplomatie semble avoir quitté la pièce."
            };
        }

        if (containsAny(focus, {"shaman", "chamane", "oracle", "mage", "apothicaire"}))
        {
            return {
                monster.getName() + " murmure une formule avant le combat.",
                randomLine({
                    "« Ton sang dira si la prophétie était sérieuse ou juste mal écrite. »",
                    "« Avance. Les mauvais choix font parfois de très bons composants. »",
                    "« Je sens déjà la peur dans tes gestes. Très utile, la peur. »"
                })
            };
        }

        return {
            monster.getName() + " prend la parole avant l'impact.",
            randomLine({
                "« Tu peux encore reculer. Ce serait moins drôle, mais plus intelligent. »",
                "« On m'avait promis un aventurier. J'espère que tu n'es pas seulement un inventaire sur pattes. »",
                "« Essaie de tenir debout assez longtemps pour que je retienne ton nom. »"
            })
        };
    }
}


EnemyCombatQueue WaveCombatSystem::createWaveForPlayer(
    const Player& player,
    Random& random
)
{
    return WaveGenerator::createWaveForPlayer(player, random, DifficultyMode::Normal);
}

EnemyCombatQueue WaveCombatSystem::createWaveForPlayer(
    const Player& player,
    Random& random,
    DifficultyMode difficulty
)
{
    return WaveGenerator::createWaveForPlayer(player, random, difficulty);
}

// EN: displayWaveIntroduction declares or implements a focused behavior used by this module.
// FR: displayWaveIntroduction déclare ou implémente un comportement précis utilisé par ce module.
void WaveCombatSystem::displayWaveIntroduction()
{
    Console::clear();

    MessageScreen::show(
        "VAGUE EN APPROCHE",
        "combat.wave.introduction",
        {
            "Une nouvelle vague de monstres approche.",
            "La composition change selon ton niveau, mais ils ne viennent pas tous en même temps...",
            "Tant que la file n'est pas vide, le combat continue."
        },
        false
    );

    Console::pauseSeconds(2);
}

// EN: displayFrontLineArrival declares or implements a focused behavior used by this module.
// FR: displayFrontLineArrival déclare ou implémente un comportement précis utilisé par ce module.
void WaveCombatSystem::displayFrontLineArrival(const EnemyCombatQueue& wave)
{
    std::vector<std::string> lines;
    lines.push_back("La première ligne ennemie entre dans l'arène.");

    for (int i = 0; i < wave.getActiveEnemyCount(); ++i)
    {
        const std::vector<std::string> introLines = getIntelligentEnemyIntroLines(wave.getActiveEnemy(i));
        if (!introLines.empty())
        {
            lines.push_back("");
            for (const std::string& line : introLines)
            {
                lines.push_back(line);
            }
        }
    }

    MessageScreen::show(
        "PREMIÈRE LIGNE ENNEMIE",
        "combat.wave.front_line_arrival",
        lines,
        false
    );

    wave.displayActiveEnemies();
    wave.displayQueueSummary();
}
