// EN: CharacterHistoryMenu displays durable history without inventing events the player never experienced.
// FR: CharacterHistoryMenu affiche l'historique durable sans inventer d'événements non vécus.
#include "interface/menu/progression/CharacterHistoryMenu.hpp"

#include "core/Console.hpp"
#include "entity/Player.hpp"
#include "entity/player/PlayerHistoryTypes.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"
#include "world/rumor/WorldRumorSystem.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace
{
    std::string categoryLabel(const std::string& category)
    {
        if (category == "achats_boutique") return "Achats";
        if (category == "ventes_boutique") return "Ventes";
        if (category == "rachats_boutique") return "Rachats";
        if (category == "equipements_portes") return "Équipement";
        if (category == "butin_notable") return "Butin remarquable";
        if (category == "equipment_lost") return "Équipement perdu";
        if (category == "equipment_destroyed") return "Équipement détruit";
        if (category == "equipment_forced_break") return "Casse forcée";
        if (category == "exchange_given") return "Objet donné";
        if (category == "exchange_received") return "Objet reçu";
        if (category == "objets_avec_memoire") return "Objets avec mémoire";
        if (category == "rivaux_confirmes" || category == "rival_created") return "Rivaux";
        if (category == "serments_pretes") return "Serments";
        if (category == "serments_rompus") return "Serments rompus";
        if (category == "cicatrices") return "Cicatrices";
        if (category == "traumatismes") return "Traumatismes";
        if (category == "heritage") return "Héritage";
        return category;
    }

    void showCanonicalCategory(const Player& player, const std::string& category, const std::string& title)
    {
        std::vector<PlayerJournalRecord> records;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category) records.push_back(record);
        }
        std::stable_sort(records.begin(), records.end(), [](const PlayerJournalRecord& a, const PlayerJournalRecord& b) {
            if (a.lastDay != b.lastDay) return a.lastDay > b.lastDay;
            return a.count > b.count;
        });

        std::vector<std::string> lines;
        if (records.empty())
        {
            lines.push_back("Aucune trace enregistrée dans cette catégorie.");
        }
        else
        {
            const std::size_t shown = std::min<std::size_t>(records.size(), 18);
            for (std::size_t i = 0; i < shown; ++i)
            {
                const PlayerJournalRecord& record = records[i];
                std::string line = record.label + " x" + std::to_string(record.count);
                line += " | dernier : jour " + std::to_string(std::max(1, record.lastDay + 1));
                if (!record.locationId.empty()) line += " | " + record.locationId;
                lines.push_back(line);
            }
            if (records.size() > shown)
            {
                lines.push_back("… " + std::to_string(records.size() - shown) + " autre(s) entrée(s) agrégée(s).");
            }
        }
        MessageScreen::show(title, "character_history.canonical", lines);
    }


    void showWorldRumors(const Player& player)
    {
        std::vector<WorldRumorStatus> rumors;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category != "world_rumors") continue;
            rumors.push_back(WorldRumorSystem::statusFor(player, record.key));
        }
        std::stable_sort(rumors.begin(), rumors.end(), [](const WorldRumorStatus& a, const WorldRumorStatus& b) {
            if (a.lastDay != b.lastDay) return a.lastDay > b.lastDay;
            return a.credibility > b.credibility;
        });

        std::vector<std::string> lines;
        if (rumors.empty())
        {
            lines.push_back("Aucune rumeur ou légende structurée n'a encore été entendue.");
        }
        else
        {
            const std::size_t shown = std::min<std::size_t>(rumors.size(), 18);
            for (std::size_t i = 0; i < shown; ++i)
            {
                const WorldRumorStatus& rumor = rumors[i];
                std::string state = rumor.confirmed ? "confirmée en partie" : "non confirmée";
                if (rumor.contradicted) state += ", éléments contradictoires";
                lines.push_back(rumor.text + " | " + state
                    + " | crédibilité " + std::to_string(rumor.credibility) + "%"
                    + " | preuve " + std::to_string(rumor.evidenceLevel));
            }
        }
        lines.push_back("Une rumeur confirmée dans son noyau peut encore contenir des détails faux ou exagérés : confirmation ne signifie jamais omniscience sur toute la légende.");
        MessageScreen::show("RUMEURS & LÉGENDES", "character_history.rumors", lines);
    }

    void showRecentHistoricalEvents(const Player& player)
    {
        const auto& events = player.getHistoricalEvents();
        std::vector<std::string> lines;
        if (events.empty())
        {
            lines.push_back("Aucun événement historique durable n'a encore été enregistré.");
        }
        else
        {
            int shown = 0;
            for (auto it = events.rbegin(); it != events.rend() && shown < 20; ++it, ++shown)
            {
                std::string line = "Jour " + std::to_string(std::max(1, it->day + 1)) + " — " + categoryLabel(it->category) + " : " + it->label;
                if (!it->locationId.empty()) line += " [" + it->locationId + "]";
                if (it->resolved) line += " [clos]";
                lines.push_back(line);
            }
        }
        MessageScreen::show("ÉVÉNEMENTS DURABLES", "character_history.events", lines);
    }
}

void CharacterHistoryMenu::open(const Player& player)
{
    while (true)
    {
        MenuScreen screen("HISTORIQUE DU PERSONNAGE", "character_history.hub");
        screen.addLine("Ce registre n'invente rien : il résume uniquement des actions et événements réellement persistés.");
        screen.addOption(1, "Événements durables récents", "Rivaux, serments, objets marquants, conséquences et autres traces historiques.", true, "character_history.events");
        screen.addOption(2, "Achats en boutique", "Objets réellement achetés, agrégés par entrée.", true, "character_history.purchases");
        screen.addOption(3, "Ventes au comptoir", "Objets réellement vendus, agrégés par entrée.", true, "character_history.sales");
        screen.addOption(4, "Rachats au comptoir", "Objets vendus puis réellement récupérés via le rachat.", true, "character_history.buybacks");
        screen.addOption(5, "Équipements portés", "Armes et armures effectivement équipées au moins une fois depuis l'activation du registre.", true, "character_history.equipment");
        screen.addOption(6, "Butin remarquable", "Équipements récupérés et matériaux rares/magiques dont l'origine a été enregistrée.", true, "character_history.loot");
        screen.addOption(7, "Échanges donnés", "Argent, objets et matériaux réellement transférés vers d'autres personnages.", true, "character_history.exchange_given");
        screen.addOption(8, "Échanges reçus", "Argent, objets et matériaux réellement reçus d'autres personnages.", true, "character_history.exchange_received");
        screen.addOption(9, "Rumeurs & légendes", "Rumeurs sourcées, crédibilité, confirmations et contradictions sans transformer le folklore en vérité automatique.", true, "character_history.rumors");
        screen.addBackOption("Retour", "character_history.back");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide. Choisis une section affichée.");
        Console::clear();
        if (choice == 0) return;
        if (choice == 1) showRecentHistoricalEvents(player);
        else if (choice == 2) showCanonicalCategory(player, "achats_boutique", "ACHATS EN BOUTIQUE");
        else if (choice == 3) showCanonicalCategory(player, "ventes_boutique", "VENTES AU COMPTOIR");
        else if (choice == 4) showCanonicalCategory(player, "rachats_boutique", "RACHATS AU COMPTOIR");
        else if (choice == 5) showCanonicalCategory(player, "equipements_portes", "ÉQUIPEMENTS PORTÉS");
        else if (choice == 6) showCanonicalCategory(player, "butin_notable", "BUTIN REMARQUABLE");
        else if (choice == 7) showCanonicalCategory(player, "echanges_donnes", "ÉCHANGES DONNÉS");
        else if (choice == 8) showCanonicalCategory(player, "echanges_recus", "ÉCHANGES REÇUS");
        else if (choice == 9) showWorldRumors(player);
    }
}
