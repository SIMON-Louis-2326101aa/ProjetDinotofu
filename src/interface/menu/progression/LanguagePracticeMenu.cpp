#include "interface/menu/progression/LanguagePracticeMenu.hpp"
#include "core/Console.hpp"
#include "entity/Player.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"
#include "progression/language/LanguageSystem.hpp"
#include <string>
#include <vector>

void LanguagePracticeMenu::open(Player& player)
{
    while (true)
    {
        std::vector<std::string> candidates;
        for (const PlayerLanguageKnowledge& knowledge : player.getLanguageKnowledge())
        {
            if (LanguageSystem::canPracticeTowardFluency(player, knowledge.languageId))
            {
                candidates.push_back(knowledge.languageId);
            }
        }

        MenuScreen screen("ATELIER DE LANGUES", "language.practice");
        screen.addLine("Le niveau conversation s'apprend avec les ouvrages ; la fluidité demande ensuite de vraies séances guidées.");
        screen.addLine("Une séance fait avancer le temps et la progression est sauvegardée.");
        screen.addLine("La notation anormale reste volontairement limitée : ce n'est pas une langue stable à parler couramment.");
        screen.addBackOption("Retour à la bibliothèque", "language.practice.back");

        if (candidates.empty())
        {
            screen.addLine("Aucune langue n'est actuellement au niveau conversation en attente de pratique.");
            screen.addLine("Achète ou étudie d'abord un cours avancé, ou reviens après avoir commencé une autre langue.");
            TerminalInterface::renderMenuScreen(screen);
            Console::waitForEnter();
            return;
        }

        for (std::size_t i = 0; i < candidates.size(); ++i)
        {
            const std::string& languageId = candidates[i];
            screen.addOption(
                static_cast<int>(i + 1),
                LanguageSystem::displayName(languageId) + " — " + std::to_string(player.getLanguageStudyProgress(languageId)) + "%",
                "Séance supervisée : prononciation, compréhension rapide, expressions et correction de faux amis.",
                true,
                "language.practice." + languageId
            );
        }

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une langue à pratiquer, ou 0 pour revenir.");
        if (choice == 0) return;
        if (choice < 1 || choice > static_cast<int>(candidates.size())) continue;

        std::vector<std::string> notes;
        const std::string languageId = candidates[static_cast<std::size_t>(choice - 1)];
        if (LanguageSystem::applyGuidedPractice(player, languageId, 2, &notes))
        {
            player.advanceWorldDayUnits(1);
            notes.push_back("Temps écoulé : une unité de journée.");
        }
        MessageScreen::show("PRATIQUE LINGUISTIQUE", "language.practice.result", notes, false);
    }
}
