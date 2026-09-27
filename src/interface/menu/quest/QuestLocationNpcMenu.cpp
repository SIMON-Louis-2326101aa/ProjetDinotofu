// EN: QuestLocationNpcMenu.cpp handles quest-facing locations and notable NPC navigation.
// FR: QuestLocationNpcMenu.cpp gère la navigation des lieux et PNJ liés aux quêtes.
#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/menu/quest/QuestClientNavigationSupport.hpp"
#include "interface/menu/quest/QuestTeamMenu.hpp"
#include "interface/menu/quest/QuestWorldMenuSupport.hpp"
#include "interface/menu/quest/QuestDeadlineSupport.hpp"
#include "interface/menu/training/TrainingGroundMenu.hpp"
#include "interface/menu/LocalReputationRepairMenu.hpp"
#include "core/Console.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "story/StoryCampaign.hpp"
#include "world/City.hpp"
#include "world/CityTravelRules.hpp"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

using QuestClientNavigationSupport::ClientQuestCounts;
using QuestClientNavigationSupport::collectRecommendedClients;
using QuestClientNavigationSupport::countQuestsForClient;
using QuestClientNavigationSupport::clientQuestHintText;
using QuestClientNavigationSupport::makeClientQuestNavigationItemData;
using QuestWorldMenuSupport::guildRequestRankDUnlocked;
using QuestWorldMenuSupport::openCityTravelMenu;
using QuestWorldMenuSupport::openCityVault;
using QuestDeadlineSupport::expireOverdueQuestDeadlines;

// EN: openLocations declares or implements a focused behavior used by this module.
// FR: openLocations déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openLocations(Player& player)
{
    expireOverdueQuestDeadlines(player, "quest.locations");
    maybeShowTorvaldRankDIntro(player);

    enum class LocationCategory
    {
        City,
        Outside,
        Shop
    };

    struct LocationEntry
    {
        std::string label;
        std::string detail;
        std::string client;
        LocationCategory category;
        bool guild = false;
        bool vault = false;
        bool travel = false;
        bool training = false;
        bool observation = false;
        bool mercenary = false;
        bool guildTrainer = false;
        bool infirmary = false;
        bool rehabilitation = false;
    };

    std::vector<LocationEntry> entries = {
        {"Guilde", "Contrats officiels, panneau, journal et services de guilde.", "Maître de guilde", LocationCategory::City, true, false},
        {"Coffre municipal", "Stockage personnel sécurisé, améliorable et séparé de l'inventaire transporté.", "Intendant du coffre", LocationCategory::City, false, true},
        {"Stand d'entraînement", "Séances courtes louables : technique d'arme, observation, appuis et résistance environnementale.", "Maître d'armes", LocationCategory::City, false, false, false, true},
        {"Poste d'observation", "Lecture de terrain : quêtes actives, mêmes lieux, indices et progression hors combat.", "Veilleur de terrain", LocationCategory::City, false, false, false, false, true},
        {"Comptoir mercenaire", "Missions déléguées, groupes connus, demandes légales/risquées et contrats publiés.", "Intendant mercenaire", LocationCategory::City, false, false, false, false, false, true},
        {"Intendance de Mira", "Priorités du village, murs, matériaux et validation de la route principale.", "Mira", LocationCategory::City},
        {"Poste d'Orren", "Routes, ponts, bornes déplacées et disparitions hors des murs.", "Orren", LocationCategory::City},
        {"Infirmerie de Lysa", "Se soigner, soigner un membre, amener une recrue KO ou récupérer une recrue prête.", "Lysa", LocationCategory::City, false, false, false, false, false, false, false, true},
        {"Forge de Bram", "Atelier de survie, outils, réparations urgentes et matériaux récupérables.", "Bram", LocationCategory::City},
        {"Archives de Soryn", "Traces, légendes, rumeurs et indices à vérifier.", "Soryn", LocationCategory::City},
        {"Place du village", "Rumeurs, habitants et petites demandes locales.", "Villageois nerveux", LocationCategory::City},
        {"Quartier abandonné", "Caves, maisons vides, contrats douteux, vieilles pièces et automates oubliés.", "Rika des clés", LocationCategory::City},
        {"Bureau de médiation locale", "Amendes réparatrices, services communautaires et voies concrètes pour réparer une mauvaise réputation.", "Médiatrice municipale", LocationCategory::City, false, false, false, false, false, false, false, false, true},
        {"Bureau des inscriptions", "Paperasse, pastilles, abonnements et litiges administratifs [objectif de quête probable].", "Scribe Ysolde", LocationCategory::City},
        {"Bibliothèque des cartes", "Cartes, bestiaire, magie, plantes et transitions [objectif de quête probable].", "Archiviste Meron", LocationCategory::City},
        {"Relais des routes", "Voyage entre villes, distances, conditions d'entrée, carte des biomes et bons de livraison [objectif de quête probable].", "Noro le palefrenier", LocationCategory::City, false, false, true},
        {"Auberge du Repos Bruyant", "Hébergement, additions et services de ville [objectif de quête probable].", "Tavia l'aubergiste", LocationCategory::City},

        {"Route commerciale", "Marchands, convois, bornes, risques de voyage et pistes à contrôler.", "Marchand inquiet", LocationCategory::Outside},
        {"Bocage aux lanternes", "Champignons-lampes, résine d'écho, spores calmes et bêtes attirées par la lumière.", "Mila des lanternes", LocationCategory::Outside},
        {"Désert d'argile rouge", "Argile rouge, sel lunaire, fausses oasis, pilleurs et constructions fissurées.", "Safa la pisteuse", LocationCategory::Outside},
        {"Mine sifflante", "Rails vibrants, fer froid, ressorts, vieux mécanismes et golems de mine.", "Bram le foreur", LocationCategory::Outside},

        {"Forge", "Commandes, réparations et demandes générales du forgeron.", "Forgeron", LocationCategory::Shop},
        {"Herboristerie", "Plantes, ingrédients, remèdes et demandes de l'alchimiste.", "Alchimiste", LocationCategory::Shop},
        {"Comptoir de Prunigil", "Marchandage, calcul, factures et registres [objectif de quête probable].", "Prunigil le marchand", LocationCategory::Shop},
        {"Boutique de monstres", "Composants de créatures et revente spécialisée.", "Vendeur de composants", LocationCategory::Shop},
        {"Boutique de matériaux", "Matériaux, stocks et approvisionnement.", "Vendeur de matériaux", LocationCategory::Shop},
        {"Armurerie défensive", "Protections, pièces d'armure et commandes.", "Armurier", LocationCategory::Shop},
        {"Forge d'armes", "Armes, réparation et approvisionnement.", "Vendeur d'armes", LocationCategory::Shop},
        {"Boutique de consommables", "Potions, consommables et réserves.", "Vendeur de consommables", LocationCategory::Shop},
        {"Bibliothèque", "Notes, savoirs, renseignements et pistes de recherche.", "Bibliothécaire", LocationCategory::Shop},
        {"Laboratoire de Maëra", "Alchimie, dosages, étiquettes de potions et sécurité [objectif de quête probable].", "Maëra l'alchimiste", LocationCategory::Shop}
    };

    if (guildRequestRankDUnlocked(player))
    {
        entries.push_back({
            "Terrain de Torvald",
            "Entraîneur de guilde : 1v1 amical, embauche légale, candidats recrutables et bases de groupe.",
            "Torvald l'entraîneur de guilde",
            LocationCategory::City,
            false,
            false,
            false,
            false,
            false,
            false,
            true
        });
    }

    if (const City* city = City::findById(player.getCurrentCityId()))
    {
        std::vector<CityBuildingPreview> cityBuildings = CityTravelRules::getBuildingsForCity(player, *city);
        for (const CityBuildingPreview& building : cityBuildings)
        {
            const bool guildBuilding = building.id == "guild";
            const bool vaultBuilding = building.id == "vault";
            const bool travelBuilding = building.id == "gate";
            const bool alreadyStatic = guildBuilding || vaultBuilding || travelBuilding;
            if (alreadyStatic)
            {
                continue;
            }

            entries.push_back({
                city->getName() + " — " + building.name + (building.unlocked ? "" : " [verrouillé]"),
                building.detail + " Asset futur : " + building.pixelArtHint + ".",
                building.contact,
                LocationCategory::City,
                false,
                false,
                false
            });
        }
    }

    if (player.hasStoryModeStarted() && !player.hasStorySkip())
    {
        const int chapter = player.getStoryChapter();
        const int step = player.getStoryStep();
        const auto storyLocationUnlocked = [&](const LocationEntry& entry)
        {
            if (entry.guild || entry.vault || entry.training || entry.observation || entry.mercenary || entry.guildTrainer || entry.infirmary || entry.rehabilitation)
            {
                return true;
            }

            const std::string& client = entry.client;
            if (client == "Mira" || client == "Villageois nerveux")
            {
                return true;
            }

            const bool firstReferentsPresent = chapter >= 2 || step >= 3;
            if (firstReferentsPresent
                && (client == "Orren" || client == "Lysa" || client == "Bram" || client == "Soryn"
                    || client == "Forgeron" || client == "Alchimiste"))
            {
                return true;
            }

            if (chapter >= 2 && client == "Marchand inquiet")
            {
                return true;
            }

            if (chapter >= 2 && step >= 7 && client == "Noro le palefrenier")
            {
                return true;
            }

            if (chapter >= 2 && step >= 9
                && (client == "Prunigil le marchand" || client == "Vendeur de composants" || client == "Vendeur de matériaux"
                    || client == "Armurier" || client == "Vendeur d'armes" || client == "Vendeur de consommables"
                    || client == "Scribe Ysolde" || client == "Maëra l'alchimiste" || client == "Tavia l'aubergiste"
                    || client == "Mila des lanternes" || client == "Safa la pisteuse" || client == "Rika des clés"))
            {
                return true;
            }

            if (chapter >= 2 && step >= 10 && client == "Bibliothécaire")
            {
                return true;
            }

            if (chapter >= 2 && step >= 12 && (client == "Archiviste Meron" || client == "Bram le foreur"))
            {
                return true;
            }

            return false;
        };

        entries.erase(
            std::remove_if(entries.begin(), entries.end(), [&](const LocationEntry& entry) { return !storyLocationUnlocked(entry); }),
            entries.end()
        );
    }

    while (true)
    {
        std::vector<LocationEntry> cityEntries;
        std::vector<LocationEntry> outsideEntries;
        std::vector<LocationEntry> shopEntries;
        for (const LocationEntry& entry : entries)
        {
            if (entry.category == LocationCategory::City)
            {
                cityEntries.push_back(entry);
            }
            else if (entry.category == LocationCategory::Outside)
            {
                outsideEntries.push_back(entry);
            }
            else
            {
                shopEntries.push_back(entry);
            }
        }

        std::vector<LocationEntry> allEntries = entries;

        struct LocationCategoryView
        {
            int choice;
            std::string title;
            std::string hint;
            std::vector<LocationEntry>* entries;
            bool all = false;
        };

        std::vector<LocationCategoryView> categoryViews;
        int nextChoice = 1;
        auto addCategory = [&](const std::string& title, const std::string& hint, std::vector<LocationEntry>& categoryEntries, bool all = false) {
            if (!categoryEntries.empty())
            {
                categoryViews.push_back({nextChoice++, title, hint, &categoryEntries, all});
            }
        };

        addCategory("Tout afficher", "Tous les lieux actuellement accessibles dans une seule liste, sans devoir fouiller les sections.", allEntries, true);
        addCategory("Ville", "Guilde, bâtiments, places, archives, auberge et contacts installés dans le village.", cityEntries);
        addCategory("Extérieur", "Routes et lieux précis situés hors des zones habitées.", outsideEntries);
        addCategory("Boutiques", "Commerces, ateliers, comptoirs et services où acheter, vendre ou se renseigner.", shopEntries);

        MenuScreen categoryScreen("LIEUX NOTABLES", "quest.locations.categories");
        categoryScreen.addSubtitle("Vue complète ou lieux classés par type");
        categoryScreen.addLine("Tout afficher regroupe les lieux accessibles pour aller vite. Les sections restent disponibles pour une recherche plus propre.");
        categoryScreen.addLine("Exploration reste réservée aux sorties par biome. Ici, tu choisis un endroit précis du monde.");
        categoryScreen.addLine("Ce classement est identique en bac à sable et en histoire ; l'histoire masque seulement les lieux qui n'existent pas encore ou ne sont pas accessibles.");
        categoryScreen.addBackOption("Retour", "quest.locations.categories.back");

        for (const LocationCategoryView& view : categoryViews)
        {
            int ready = 0;
            int active = 0;
            for (const LocationEntry& entry : *view.entries)
            {
                const ClientQuestCounts counts = countQuestsForClient(player, entry.client);
                ready += counts.ready;
                active += counts.active;
            }

            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "location_group";
            itemData.section = "Lieux notables";
            itemData.actionType = "open";
            itemData.name = view.title;
            itemData.detail = view.hint;
            itemData.status = std::to_string(view.entries->size()) + " lieu(x)";
            if (ready > 0)
            {
                itemData.progress = std::to_string(ready) + " objectif(s) à rendre";
                itemData.important = true;
            }
            else if (active > 0)
            {
                itemData.progress = std::to_string(active) + " objectif(s) en cours";
            }
            else
            {
                itemData.progress = "Aucun objectif signalé";
            }

            categoryScreen.addOption(
                view.choice,
                view.title + (ready > 0 ? " [" + std::to_string(ready) + " à rendre]" : ""),
                view.hint,
                true,
                "quest.locations.category." + std::to_string(view.choice),
                itemData
            );
        }

        const int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(
            categoryScreen,
            "Choisis une section de lieux notables affichée."
        );
        Console::clear();

        if (categoryChoice == 0)
        {
            return;
        }

        const auto selectedCategoryIt = std::find_if(
            categoryViews.begin(),
            categoryViews.end(),
            [&](const LocationCategoryView& view) { return view.choice == categoryChoice; }
        );
        if (selectedCategoryIt == categoryViews.end())
        {
            continue;
        }

        std::vector<LocationEntry>& selectedEntries = *selectedCategoryIt->entries;
        const std::string selectedSection = selectedCategoryIt->title;
        const std::string selectedHint = selectedCategoryIt->hint;
        const bool showingAllLocations = selectedCategoryIt->all;
        const auto locationSectionName = [](LocationCategory category) {
            if (category == LocationCategory::City) return std::string("Ville");
            if (category == LocationCategory::Outside) return std::string("Extérieur");
            return std::string("Boutiques");
        };
        constexpr std::size_t locationsPerPage = 8;
        std::size_t pageIndex = 0;
        bool sectionOpen = true;

        while (sectionOpen)
        {
            const std::size_t totalPages = PagedMenu::pageCount(selectedEntries.size(), locationsPerPage);
            if (pageIndex >= totalPages)
            {
                pageIndex = totalPages == 0 ? 0 : totalPages - 1;
            }

            const std::size_t first = PagedMenu::firstIndex(pageIndex, locationsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(selectedEntries.size(), pageIndex, locationsPerPage);

            MenuScreen screen("LIEUX NOTABLES — " + selectedSection, "quest.locations.category_list");
            screen.setPagination(pageIndex, totalPages);
            screen.addLine(selectedHint);
            screen.addLine("Sélectionne un lieu pour parler au contact associé, consulter ses demandes ou rendre un objectif terminé.");
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, selectedEntries.size()));
            screen.addBackOption("Changer de section", "quest.locations.category_list.back");

            for (std::size_t i = first; i < last; ++i)
            {
                const LocationEntry& entry = selectedEntries[i];
                const ClientQuestCounts counts = countQuestsForClient(player, entry.client);
                const std::string entrySection = locationSectionName(entry.category);
                std::string label = (showingAllLocations ? "[" + entrySection + "] " : "") + entry.label + " — " + entry.client;
                if (counts.ready > 0)
                {
                    label += " [" + std::to_string(counts.ready) + " à rendre]";
                }
                else if (counts.active > 0)
                {
                    label += " [" + std::to_string(counts.active) + " en cours]";
                }

                MenuOptionItemData itemData = makeClientQuestNavigationItemData(
                    entry.client,
                    selectedSection,
                    entry.detail,
                    counts
                );
                itemData.kind = "location";
                itemData.section = entrySection;
                itemData.actionType = entry.guild ? "quest" : (entry.vault ? "storage" : (entry.travel ? "travel" : (entry.training ? "train" : (entry.observation ? "inspect" : (entry.mercenary ? "quest" : (entry.guildTrainer ? "train" : (entry.infirmary ? "heal" : "talk")))))));
                itemData.name = entry.label;
                itemData.owner = entry.client;
                itemData.progress = "Lieu " + std::to_string(i + 1) + "/" + std::to_string(selectedEntries.size());
                itemData.important = counts.ready > 0;

                screen.addOption(
                    static_cast<int>(i - first + 1),
                    label,
                    entry.detail + " " + clientQuestHintText(counts),
                    true,
                    "quest.locations.category_list.select." + std::to_string(i + 1),
                    itemData
                );
            }

            PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un lieu affiché.");
            Console::clear();

            if (choice == 0)
            {
                sectionOpen = false;
                continue;
            }
            if (choice == 98 && pageIndex > 0)
            {
                --pageIndex;
                continue;
            }
            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                ++pageIndex;
                continue;
            }
            if (choice < 1 || static_cast<std::size_t>(choice) > (last - first))
            {
                MessageScreen::show(
                    "LIEU INDISPONIBLE",
                    "quest.locations.invalid_choice",
                    {
                        "Cette entrée n'existe pas dans la section actuelle.",
                        "Utilise les choix affichés ou les boutons de pagination."
                    }
                );
                continue;
            }

            const LocationEntry& selected = selectedEntries[first + static_cast<std::size_t>(choice - 1)];
            if (selected.guild)
            {
                openGuild(player);
            }
            else if (selected.vault)
            {
                openCityVault(player);
            }
            else if (selected.travel)
            {
                openCityTravelMenu(player);
            }
            else if (selected.training)
            {
                TrainingGroundMenu::open(player);
            }
            else if (selected.observation)
            {
                openFieldObservationDesk(player);
            }
            else if (selected.mercenary)
            {
                openMercenaryCounter(player);
            }
            else if (selected.guildTrainer)
            {
                openTorvaldGuildTrainer(player);
            }
            else if (selected.infirmary)
            {
                openInfirmaryServiceMenu(player);
            }
            else if (selected.rehabilitation)
            {
                LocalReputationRepairMenu::open(player);
            }
            else
            {
                talkToClient(player, selected.client);
            }
        }
    }
}



// EN: openNotableNpcMenu declares or implements a focused behavior used by this module.
// FR: openNotableNpcMenu déclare ou implémente un comportement précis utilisé par ce module.
void QuestMenu::openNotableNpcMenu(Player& player)
{
    expireOverdueQuestDeadlines(player, "quest.notable_npcs");
    constexpr std::size_t clientsPerPage = 8;

    using NpcEntry = std::pair<std::string, std::string>;

    while (true)
    {
        syncMainStoryQuests(player);

        std::vector<NpcEntry> storyEntries = {
            {"Mira", "intendante de quartier / référente histoire"},
            {"Orren", "vieux garde / référent de route"},
            {"Lysa", "soigneuse de fortune"},
            {"Bram", "forgeron fatigué"},
            {"Soryn", "archiviste"},
            {"Nell la messagère", "messagère de relais / survivante de route"},
            {"Eda", "comptable des routes courtes / stocks réels"}
        };

        std::vector<NpcEntry> shopEntries = {
            {"Maître de guilde", "guilde / contrats officiels"},
            {"Forgeron", "forge / réparations et équipement"},
            {"Alchimiste", "alchimie / potions"},
            {"Prunigil le marchand", "comptoir / QCM marchand"},
            {"Vendeur de composants", "boutique de composants"},
            {"Vendeur de matériaux", "boutique de matériaux"},
            {"Herboriste", "plantes et remèdes"},
            {"Armurier", "armures et protections"},
            {"Vendeur d'armes", "armes"},
            {"Vendeur de consommables", "consommables"},
            {"Bibliothécaire", "bibliothèque et renseignements"},
            {"Archiviste Meron", "bibliothèque / QCM de connaissances"},
            {"Scribe Ysolde", "inscriptions / paperasse et guilde"},
            {"Maëra l'alchimiste", "laboratoire / alchimie QCM"},
            {"Noro le palefrenier", "relais / transport et routes"},
            {"Tavia l'aubergiste", "auberge / hébergement et services"},
            {"Veilleur de terrain", "poste d'observation / lecture de quêtes"},
            {"Intendant mercenaire", "comptoir mercenaire / missions déléguées"}
        };

        if (guildRequestRankDUnlocked(player))
        {
            shopEntries.push_back({"Torvald l'entraîneur de guilde", "guilde / entraînement d'équipe et recrutement"});
        }

        std::vector<NpcEntry> otherEntries = {
            {"Villageois nerveux", "habitant / événement et rumeurs"},
            {"Marchand inquiet", "habitant / commerce et rumeurs"}
        };

        if (player.hasStoryModeStarted() && !player.hasStorySkip())
        {
            const int chapter = player.getStoryChapter();
            const int step = player.getStoryStep();

            storyEntries.erase(
                std::remove_if(storyEntries.begin(), storyEntries.end(), [&](const NpcEntry& entry)
                {
                    const std::string& name = entry.first;
                    if (name == "Mira") return false;
                    if (name == "Orren" || name == "Lysa" || name == "Bram" || name == "Soryn")
                    {
                        return chapter < 2 && step < 3;
                    }
                    if (name == "Nell la messagère") return chapter < 2 || step < 7;
                    if (name == "Eda") return chapter < 2 || step < 12;
                    return true;
                }),
                storyEntries.end()
            );

            shopEntries.erase(
                std::remove_if(shopEntries.begin(), shopEntries.end(), [&](const NpcEntry& entry)
                {
                    const std::string& name = entry.first;
                    if (name == "Maître de guilde") return false;
                    if (name == "Forgeron" || name == "Alchimiste" || name == "Herboriste")
                    {
                        return chapter < 2 && step < 3;
                    }
                    if (name == "Noro le palefrenier") return chapter < 2 || step < 7;
                    if (name == "Veilleur de terrain" || name == "Intendant mercenaire") return false;
                    if (chapter >= 2 && step >= 9
                        && (name == "Prunigil le marchand" || name == "Vendeur de composants" || name == "Vendeur de matériaux"
                            || name == "Armurier" || name == "Vendeur d'armes" || name == "Vendeur de consommables"
                            || name == "Scribe Ysolde" || name == "Maëra l'alchimiste" || name == "Tavia l'aubergiste")) return false;
                    if (chapter >= 2 && step >= 10 && name == "Bibliothécaire") return false;
                    if (chapter >= 2 && step >= 12 && name == "Archiviste Meron") return false;
                    return true;
                }),
                shopEntries.end()
            );

            otherEntries.erase(
                std::remove_if(otherEntries.begin(), otherEntries.end(), [&](const NpcEntry& entry)
                {
                    return entry.first == "Marchand inquiet" && chapter < 2;
                }),
                otherEntries.end()
            );
        }

        std::vector<NpcEntry> temporaryEntries;
        const std::vector<std::string> recommendedClients = collectRecommendedClients(player);
        for (const std::string& clientName : recommendedClients)
        {
            temporaryEntries.push_back({clientName, "PNJ de quête temporaire / maximum 5 demandes"});
        }

        auto appendUnique = [](std::vector<NpcEntry>& destination, const std::vector<NpcEntry>& source) {
            for (const NpcEntry& entry : source)
            {
                const bool exists = std::any_of(destination.begin(), destination.end(), [&](const NpcEntry& current) {
                    return current.first == entry.first;
                });
                if (!exists)
                {
                    destination.push_back(entry);
                }
            }
        };

        std::vector<NpcEntry> allEntries;
        appendUnique(allEntries, storyEntries);
        appendUnique(allEntries, shopEntries);
        appendUnique(allEntries, otherEntries);
        appendUnique(allEntries, temporaryEntries);

        auto countReadyForEntries = [&](const std::vector<NpcEntry>& entries) {
            int ready = 0;
            for (const NpcEntry& entry : entries)
            {
                ready += countQuestsForClient(player, entry.first).ready;
            }
            return ready;
        };

        MenuScreen categoryScreen("PNJ NOTABLES", "quest.notable_npc.categories");
        categoryScreen.addSubtitle("Vue complète ou contacts classés par rôle");
        categoryScreen.addLine("Tout afficher regroupe les contacts accessibles pour aller vite. Les catégories restent disponibles pour une recherche plus propre.");
        categoryScreen.addLine("Un PNJ d'histoire existe aussi dans le bac à sable : cette organisation change seulement l'affichage, jamais le monde.");
        categoryScreen.addLine("Les PNJ de quête temporaires proviennent de recommandations et peuvent proposer au maximum 5 demandes par contact.");
        if (player.hasStoryModeStarted() && !player.hasStorySkip())
        {
            categoryScreen.addLine("Mode histoire : seuls les PNJ déjà arrivés ou rencontrés sont affichés. Les futurs contacts n'encombrent pas la liste.");
        }
        categoryScreen.addBackOption("Retour", "quest.notable_npc.categories.back");

        auto addCategoryOption = [&](int number, const std::string& label, const std::string& hint, const std::vector<NpcEntry>& entries, const std::string& id, bool enabled = true) {
            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "npc_group";
            itemData.section = "PNJ notables";
            itemData.actionType = "open";
            itemData.name = label;
            itemData.detail = hint;
            itemData.status = std::to_string(entries.size()) + " contact(s)";
            const int ready = countReadyForEntries(entries);
            itemData.progress = ready > 0 ? std::to_string(ready) + " demande(s) à rendre" : "Aucune demande prête";
            itemData.important = ready > 0;
            categoryScreen.addOption(number, label + (ready > 0 ? " [" + std::to_string(ready) + " à rendre]" : ""), hint, enabled, id, itemData);
        };

        addCategoryOption(1, "Tout afficher", "Afficher tous les contacts disponibles dans une seule liste paginée.", allEntries, "quest.notable_npc.category.all");
        addCategoryOption(2, "PNJ d'histoire", "Mira, référents, survivants et contacts liés aux chapitres.", storyEntries, "quest.notable_npc.category.story");
        addCategoryOption(3, "PNJ de boutique et services", "Guilde, forge, alchimie, commerces, bibliothèque, relais et auberge.", shopEntries, "quest.notable_npc.category.shops");
        addCategoryOption(4, "Autres PNJ notables", "Habitants, rumeurs, événements et contacts permanents hors histoire/boutiques.", otherEntries, "quest.notable_npc.category.other");
        addCategoryOption(5, "PNJ de quête temporaires", "Contacts recommandés, limités à cinq demandes chacun.", temporaryEntries, "quest.notable_npc.category.temporary", !temporaryEntries.empty());

        const int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(categoryScreen, "Choisis une catégorie de PNJ.");
        Console::clear();
        if (categoryChoice == 0)
        {
            return;
        }

        const std::vector<NpcEntry>* selectedEntries = nullptr;
        std::string categoryTitle;
        std::string categoryHint;
        const bool showingAllNpcs = categoryChoice == 1;
        if (categoryChoice == 1)
        {
            selectedEntries = &allEntries;
            categoryTitle = "TOUS LES PNJ NOTABLES";
            categoryHint = "Vue complète de tous les contacts actuellement disponibles, regroupés visuellement par rôle dans l'IG.";
        }
        else if (categoryChoice == 2)
        {
            selectedEntries = &storyEntries;
            categoryTitle = "PNJ D'HISTOIRE";
            categoryHint = "Ces personnages restent présents dans le monde et dans le bac à sable, même lorsqu'ils servent aussi la route principale.";
        }
        else if (categoryChoice == 3)
        {
            selectedEntries = &shopEntries;
            categoryTitle = "PNJ DE BOUTIQUE ET SERVICES";
            categoryHint = "Contacts associés à une boutique, un comptoir, la guilde ou un service permanent.";
        }
        else if (categoryChoice == 4)
        {
            selectedEntries = &otherEntries;
            categoryTitle = "AUTRES PNJ NOTABLES";
            categoryHint = "Habitants et contacts permanents qui ne sont ni des référents d'histoire ni des vendeurs.";
        }
        else if (categoryChoice == 5 && !temporaryEntries.empty())
        {
            selectedEntries = &temporaryEntries;
            categoryTitle = "PNJ DE QUÊTE TEMPORAIRES";
            categoryHint = "Contacts obtenus par recommandation. Chacun disparaît de cette catégorie après avoir atteint sa limite de cinq demandes.";
        }
        else
        {
            continue;
        }

        const auto containsNpc = [](const std::vector<NpcEntry>& entries, const std::string& name) {
            return std::any_of(entries.begin(), entries.end(), [&](const NpcEntry& entry) { return entry.first == name; });
        };
        const auto npcSectionName = [&](const std::string& name) {
            if (containsNpc(temporaryEntries, name)) return std::string("PNJ de quête temporaires");
            if (containsNpc(storyEntries, name)) return std::string("PNJ d'histoire");
            if (containsNpc(shopEntries, name)) return std::string("Boutiques et services");
            return std::string("Autres PNJ");
        };

        std::size_t pageIndex = 0;
        bool categoryOpen = true;
        while (categoryOpen)
        {
            const std::size_t totalPages = PagedMenu::pageCount(selectedEntries->size(), clientsPerPage);
            if (pageIndex >= totalPages)
            {
                pageIndex = totalPages == 0 ? 0 : totalPages - 1;
            }

            const std::size_t first = PagedMenu::firstIndex(pageIndex, clientsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(selectedEntries->size(), pageIndex, clientsPerPage);

            MenuScreen screen(categoryTitle, "quest.notable_npc.category_list");
            screen.setPagination(pageIndex, totalPages);
            screen.addLine(categoryHint);
            screen.addLine("Sélectionne un contact pour parler, consulter ses demandes ou rendre ce qui est terminé.");
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, selectedEntries->size()));
            screen.addBackOption("Changer de catégorie", "quest.notable_npc.category_list.back");

            for (std::size_t i = first; i < last; ++i)
            {
                const NpcEntry& entry = (*selectedEntries)[i];
                const ClientQuestCounts counts = countQuestsForClient(player, entry.first);
                const std::string entrySection = npcSectionName(entry.first);
                std::string label = (showingAllNpcs ? "[" + entrySection + "] " : "") + entry.first + " (" + entry.second + ")";
                if (counts.ready > 0)
                {
                    label += " [" + std::to_string(counts.ready) + " à rendre]";
                }
                else if (counts.active > 0)
                {
                    label += " [" + std::to_string(counts.active) + " en cours]";
                }

                MenuOptionItemData itemData = makeClientQuestNavigationItemData(entry.first, showingAllNpcs ? entrySection : categoryTitle, entry.second, counts);
                itemData.section = showingAllNpcs ? entrySection : categoryTitle;
                itemData.status = entry.first == "Maître de guilde"
                    ? "Contrats officiels / panneau de guilde"
                    : (entry.first == "Veilleur de terrain"
                        ? "Observation / mêmes lieux"
                        : (entry.first == "Intendant mercenaire" ? "Missions déléguées" : clientQuestStatusText(counts)));
                itemData.actionType = entry.first == "Maître de guilde"
                    ? "quest"
                    : (entry.first == "Veilleur de terrain" ? "inspect" : (entry.first == "Intendant mercenaire" ? "quest" : itemData.actionType));
                itemData.progress = "Contact " + std::to_string(i + 1) + "/" + std::to_string(selectedEntries->size());
                itemData.important = counts.ready > 0;

                screen.addOption(
                    static_cast<int>(i - first + 1),
                    label,
                    entry.first == "Maître de guilde" ? "Ouvrir le panneau officiel de guilde." : (entry.first == "Veilleur de terrain" ? "Ouvrir le poste d'observation." : (entry.first == "Intendant mercenaire" ? "Ouvrir le comptoir mercenaire." : "Parler, consulter ou rendre une demande auprès de ce contact.")),
                    true,
                    "quest.notable_npc.category_list.select." + std::to_string(i + 1),
                    itemData
                );
            }

            PagedMenu::addNavigationOptions(screen, pageIndex, totalPages);
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();

            if (choice == 0)
            {
                categoryOpen = false;
                continue;
            }
            if (choice == 98 && pageIndex > 0)
            {
                --pageIndex;
                continue;
            }
            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                ++pageIndex;
                continue;
            }
            if (choice < 1 || static_cast<std::size_t>(choice) > (last - first))
            {
                MessageScreen::show(
                    "CONTACT INDISPONIBLE",
                    "quest.notable_npc.invalid_choice",
                    {"Cette entrée n'existe pas sur la page actuelle.", "Utilise les choix affichés ou les boutons de pagination."}
                );
                continue;
            }

            const std::string selectedClient = (*selectedEntries)[first + static_cast<std::size_t>(choice - 1)].first;
            if (selectedClient == "Maître de guilde")
            {
                openGuild(player);
            }
            else if (selectedClient == "Veilleur de terrain")
            {
                openFieldObservationDesk(player);
            }
            else if (selectedClient == "Intendant mercenaire")
            {
                openMercenaryCounter(player);
            }
            else if (selectedClient == "Torvald l'entraîneur de guilde")
            {
                openTorvaldGuildTrainer(player);
            }
            else
            {
                talkToClient(player, selectedClient);
            }
        }
    }
}



