// EN: StatisticsMenu.cpp implements a normalized statistics hub for terminal navigation.
// FR: StatisticsMenu.cpp implémente un centre de statistiques normalisé pour la navigation terminal.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/progression/StatisticsMenu.hpp"

#include "core/Console.hpp"
#include "entity/Player.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/TerminalInterface.hpp"
#include "item/Inventory.hpp"
#include "item/durability/DurabilityRules.hpp"
#include "progression/Level.hpp"
#include "progression/TitleCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <vector>


namespace
{


    bool equippedStatisticsTitleContains(const Player& player, const std::vector<std::string>& needles)
    {
        for (const std::string& title : player.getActiveTitles())
        {
            std::string loweredTitle = title;
            std::transform(loweredTitle.begin(), loweredTitle.end(), loweredTitle.begin(), [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            });
            for (std::string needle : needles)
            {
                std::transform(needle.begin(), needle.end(), needle.begin(), [](unsigned char c) {
                    return static_cast<char>(std::tolower(c));
                });
                if (loweredTitle.find(needle) != std::string::npos)
                {
                    return true;
                }
            }
        }
        return false;
    }

    std::vector<std::string> equippedTitleEquipmentLines(const Player& player)
    {
        std::vector<std::string> lines;
        if (player.getActiveTitles().empty())
        {
            return lines;
        }

        if (equippedStatisticsTitleContains(player, {"briseur", "automates", "armure", "forge", "reliques", "matériaux", "materiaux"}))
        {
            lines.push_back("- Titres équipés : lecture d'atelier un peu plus crédible. Effet faible : meilleurs commentaires d'usure, pas de réparation gratuite.");
        }
        if (equippedStatisticsTitleContains(player, {"tueur", "chasseur", "pisteur", "fléau", "fleau", "traqueur"}))
        {
            lines.push_back("- Titres équipés : identité de combattant affichée. Effet faible : intimidation/réputation, pas de bonus brut aux dégâts.");
        }
        if (equippedStatisticsTitleContains(player, {"anomal", "menu", "fissuré", "fissure", "débogueur", "debug"}))
        {
            lines.push_back("- Titres équipés : lecture des anomalies plus assumée. Effet faible : dialogues et méfiance, jamais immunité à l'interface.");
        }
        if (lines.empty())
        {
            lines.push_back("- Titres équipés : " + player.getActiveTitleSummary() + " | influence surtout lore/dialogues/réputation.");
        }
        return lines;
    }

    MenuOptionItemData makeStatisticsItemData(const std::string& actionType, const std::string& name, const std::string& detail)
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "progression";
        itemData.section = "Statistiques / progression";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = "Consultable";
        itemData.important = actionType == "inspect";
        return itemData;
    }

    bool titleIsRevealed(const Player& player, const TitleCatalog::TitleDefinition& title)
    {
        return title.visibleBeforeUnlock || player.hasTitle(title.name);
    }

    std::string secretTitleLine()
    {
        return "- [secret] ???? — Titre caché : découvre l'exploit pour révéler son nom.";
    }

    std::string skillNameFromId(const std::string& id)
    {
        static const std::map<std::string, std::string> names = {
            {"night_vision", "Vision nocturne"},
            {"survival_breath", "Souffle de survie"},
            {"steady_guard", "Garde stable"},
            {"living_rampart", "Rempart vivant"},
            {"sure_hand", "Main sûre"},
            {"careful_dosage", "Dosage prudent"},
            {"ranger_eye", "Œil de rôdeur"},
            {"chain_execution", "Enchaînement"},
            {"reflex_counter", "Contre réflexe"},
            {"cautious_channeling", "Canalisation prudente"},
            {"shadow_step", "Pas de l'ombre"},
            {"arcane_impulse", "Élan arcanique"},
            {"tracking_mark", "Marque de pisteur"},
            {"prepared_volley", "Salve préparée"},
            {"blade_discipline", "Discipline de lame"},
            {"splitting_blow", "Frappe fendue"},
            {"armor_crack", "Fracasse-garde"},
            {"reach_control", "Contrôle d'allonge"},
            {"semi_wolf_tracking", "Flair de meute"},
            {"semi_fox_cunning", "Flair rusé de renard"},
            {"semi_dog_loyal_scent", "Flair loyal"},
            {"semi_cat_reflexes", "Réflexes félins"},
            {"semi_lizard_scales", "Écailles tempérées"},
            {"semi_bird_open_sky", "Vue des hauteurs"},
            {"dragon_weather_blood", "Sang draconique d'endurance"},
            {"orcish_forced_march", "Marche forcée orque"},
            {"fairy_mana_sense", "Sens féerique du mana"},
            {"dwarven_mine_sense", "Sens des galeries"},
            {"elven_fine_perception", "Perception elfique"},
            {"halfling_lucky_step", "Pas chanceux"},
            {"temperature_adaptation", "Adaptation aux températures"},
            {"minor_fire_resistance", "Résistance légère au feu"},
            {"infernal_fire_resistance", "Résistance infernale"},
            {"minor_cold_resistance", "Résistance légère au froid"},
            {"fire_vulnerability", "Vulnérabilité au feu"},
            {"armor_habit", "Habitude d'armure"},
            {"loadout_memory", "Mémoire d'équipement"},
            {"field_maintenance", "Entretien de terrain"},
            {"curse_anchor_awareness", "Conscience d'ancrage"}
        };

        auto found = names.find(id);
        if (found != names.end())
        {
            return found->second;
        }

        return id;
    }

    void appendSkillList(std::vector<std::string>& lines, const std::string& title, const std::vector<std::string>& skills)
    {
        lines.push_back(title);
        if (skills.empty())
        {
            lines.push_back("- Aucune pour le moment.");
            return;
        }

        for (const std::string& skillId : skills)
        {
            lines.push_back("- " + skillNameFromId(skillId));
        }
    }

    std::string progressLine(const std::string& label, int current, int target)
    {
        if (current >= target)
        {
            return "- " + label + " : trace maîtrisée";
        }

        return "- " + label + " : " + std::to_string(current) + "/" + std::to_string(target);
    }

    std::string toLowerLocal(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });
        return value;
    }

    std::string masteryState(int current, int target)
    {
        if (target <= 0 || current >= target)
        {
            return "maîtrisée";
        }

        if (current <= 0)
        {
            return "à découvrir";
        }

        if (current * 2 >= target)
        {
            return "proche";
        }

        return "en éveil";
    }

    std::string curseCategorySummary(const std::string& categories)
    {
        if (categories.empty())
        {
            return "non classée";
        }

        std::stringstream input(categories);
        std::string token;
        std::vector<std::string> labels;

        while (std::getline(input, token, ','))
        {
            token.erase(std::remove_if(token.begin(), token.end(), [](unsigned char c) {
                return std::isspace(c) != 0;
            }), token.end());

            if (token == "health") labels.push_back("santé");
            else if (token == "attack") labels.push_back("attaque");
            else if (token == "mana") labels.push_back("mana");
            else if (token == "precision") labels.push_back("précision");
            else if (token == "defense") labels.push_back("défense");
            else if (token == "sleep") labels.push_back("sommeil");
            else if (token == "luck") labels.push_back("chance");
            else if (token == "equipment") labels.push_back("équipement");
            else if (token == "spirit") labels.push_back("esprit");
            else if (token == "corruption") labels.push_back("corruption");
            else if (token == "travel") labels.push_back("voyage");
            else if (token == "social") labels.push_back("social");
            else if (token == "interface") labels.push_back("interface");
            else if (token == "hallucination") labels.push_back("hallucinations");
            else if (!token.empty()) labels.push_back(token);
        }

        if (labels.empty())
        {
            return "non classée";
        }

        std::string result;
        for (const std::string& label : labels)
        {
            if (!result.empty()) result += ", ";
            result += label;
        }
        return result;
    }

    std::string curseAnchorText(const PlayerCurse& curse)
    {
        if (curse.bossIdRequiredToBreak > 0)
        {
            return "ancrée à un boss : reste hors combat jusqu'à revanche contre la source";
        }

        if (curse.removableByChurch)
        {
            return "diagnostic/exorcisme possible à l'église";
        }

        if (curse.lifeLong)
        {
            return "trace durable : solution spéciale requise";
        }

        if (curse.expiresAtDay >= 0)
        {
            return "trace temporaire";
        }

        return "source à confirmer";
    }

    void appendSkillRoadmap(std::vector<std::string>& lines, const Player& player)
    {
        const std::string playerClass = toLowerLocal(player.getType());

        lines.push_back("");
        lines.push_back("Pistes de maîtrise visibles :");

        if (playerClass.find("assassin") != std::string::npos)
        {
            lines.push_back("- Voie de classe | Source : assassin | État : mobilité et exécution à renforcer");
        }
        else if (playerClass.find("colosse") != std::string::npos)
        {
            lines.push_back("- Voie de classe | Source : colosse | État : endurance et protection à renforcer");
        }
        else if (playerClass.find("mage") != std::string::npos)
        {
            lines.push_back("- Voie de classe | Source : mage | État : canalisation et grimoires à renforcer");
        }
        else
        {
            lines.push_back("- Voie de classe | Source : " + player.getType() + " | État : style de combat à stabiliser");
        }

        lines.push_back("- Héritage racial | Source : " + player.getRaceText() + " | État : affinités à observer en combat");
        lines.push_back(
            "- Arme dominante | Source : " + player.getEquippedWeapon().getName()
            + " | État : " + masteryState(
                std::max({
                    player.getDaggerKillProgress(),
                    player.getBowKillProgress(),
                    player.getBareHandKillProgress(),
                    player.getStaffKillProgress(),
                    player.getSwordKillProgress(),
                    player.getAxeKillProgress(),
                    player.getHammerKillProgress(),
                    player.getSpearKillProgress()
                }),
                7
            )
        );
        lines.push_back("- Apprentissage libre | Source : bibliothèques, grimoires, essais et conditions spéciales | État : verrouillé par rareté et prérequis");
    }

    void showStatisticsScreen(const std::string& title, const std::string& screenId, const std::vector<std::string>& lines)
    {
        MessageScreen::show(title, screenId, lines);
    }
}


MenuScreen StatisticsMenu::buildHubScreen()
{
    MenuScreen screen("STATISTIQUES", "statistics.hub");
    screen.addOption(1, "Résumé du personnage", "Identité, niveau, expérience et état général.", true, "statistics.summary", makeStatisticsItemData("inspect", "Résumé du personnage", "Identité, niveau, expérience et état général."));
    screen.addOption(2, "Statistiques de combat", "Combats, boss, JcJ, morts et difficulté.", true, "statistics.combat", makeStatisticsItemData("inspect", "Statistiques de combat", "Combats, boss, JcJ, morts et difficulté."));
    screen.addOption(3, "Équipement et objets récents", "Équipement actuel et traces d'utilisation.", true, "statistics.equipment", makeStatisticsItemData("inspect", "Équipement et objets récents", "Équipement actuel et traces d'utilisation."));
    screen.addOption(4, "Compétences / progression", "Passifs, techniques et traces d'apprentissage.", true, "statistics.skills", makeStatisticsItemData("inspect", "Compétences / progression", "Passifs, techniques et traces d'apprentissage."));
    screen.addOption(5, "États spéciaux et conséquences", "Altérations, clones, dettes et marques de boss.", true, "statistics.states", makeStatisticsItemData("inspect", "États spéciaux", "Altérations, clones, dettes et marques de boss."));
    screen.addOption(6, "Titres disponibles et obtenus", "Lister les titres connus : guilde, chasse, anomalies et rangs.", true, "statistics.titles", makeStatisticsItemData("inspect", "Titres", "Lister les titres connus : guilde, chasse, anomalies et rangs."));
    screen.addOption(7, "Affichage complet historique", "Afficher les statistiques longues du personnage.", true, "statistics.full_history", makeStatisticsItemData("inspect", "Historique complet", "Afficher les statistiques longues du personnage."));
    screen.addBackOption("Retour", "statistics.back");
    return screen;
}

void StatisticsMenu::open(Player& player, DifficultyMode difficulty)
{
    const bool difficultyKnown = true;
    while (true)
    {
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            buildHubScreen(),
            "Choix invalide. Choisis une option affichée."
        );
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            displaySummary(player);
        }
        else if (choice == 2)
        {
            displayCombatStats(player, difficulty, difficultyKnown);
        }
        else if (choice == 3)
        {
            displayEquipmentUsage(player);
        }
        else if (choice == 4)
        {
            displaySkillStats(player);
        }
        else if (choice == 5)
        {
            displaySpecialStates(player);
        }
        else if (choice == 6)
        {
            displayTitleCatalog(player);
        }
        else if (choice == 7)
        {
            MessageScreen::show(
                "HISTORIQUE COMPLET",
                "statistics.full_history.intro",
                {
                    "Affichage long du personnage.",
                    "Cette vue garde encore le registre complet, même si certaines lignes restent très compactes."
                }
            );
            player.displayStats();
            player.displayCareerStatistics(difficulty);
            Console::waitForEnter();
            Console::clear();
        }
    }
}

void StatisticsMenu::open(Player& player)
{
    const bool difficultyKnown = false;
    DifficultyMode difficulty = DifficultyMode::Normal;

    while (true)
    {
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            buildHubScreen(),
            "Choix invalide. Choisis une option affichée."
        );
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 1)
        {
            displaySummary(player);
        }
        else if (choice == 2)
        {
            displayCombatStats(player, difficulty, difficultyKnown);
        }
        else if (choice == 3)
        {
            displayEquipmentUsage(player);
        }
        else if (choice == 4)
        {
            displaySkillStats(player);
        }
        else if (choice == 5)
        {
            displaySpecialStates(player);
        }
        else if (choice == 6)
        {
            displayTitleCatalog(player);
        }
        else if (choice == 7)
        {
            MessageScreen::show(
                "HISTORIQUE COMPLET",
                "statistics.full_history.intro.unknown_difficulty",
                {
                    "Affichage long du personnage.",
                    "La difficulté exacte n'est pas transmise par ce chemin de menu.",
                    "Cette vue garde encore le registre complet, même si certaines lignes restent très compactes."
                }
            );
            player.displayStats();
            player.displayCareerStatistics(difficulty);
            Console::waitForEnter();
            Console::clear();
        }
    }
}

void StatisticsMenu::displaySummary(const Player& player)
{
    std::vector<std::string> lines;
    lines.push_back("Nom : " + player.getName());
    lines.push_back("Race : " + player.getRaceText());
    lines.push_back("Classe : " + player.getType());
    lines.push_back("Titres équipés : " + player.getActiveTitleSummary());
    lines.push_back("Titres possédés : " + std::to_string(player.getTitles().size()));
    lines.push_back("Niveau : " + std::to_string(player.getLevel()));

    int nextLevelExperience = Level::getExperienceRequiredForNextLevel(player.getLevel());
    if (nextLevelExperience > 0)
    {
        lines.push_back("Expérience : " + std::to_string(player.getExperience()) + "/" + std::to_string(nextLevelExperience));
    }
    else
    {
        lines.push_back("Expérience : niveau maximum");
    }

    lines.push_back("PV : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()));
    lines.push_back("Or : " + std::to_string(player.getInventory().getGold()));
    lines.push_back("État : " + std::string(player.isAlteredByCheats() ? "Altéré" : "Normal"));
    lines.push_back("Clone : " + std::string(player.isClone() ? "oui" : "non"));

    showStatisticsScreen("RÉSUMÉ DU PERSONNAGE", "statistics.summary.detail", lines);
}

void StatisticsMenu::displayCombatStats(const Player& player, DifficultyMode difficulty, bool difficultyKnown)
{
    std::vector<std::string> lines;
    lines.push_back("Combats lancés : " + std::to_string(player.getCombatsStarted()));
    lines.push_back("Victoires : " + std::to_string(player.getVictories()));
    lines.push_back("Défaites : " + std::to_string(player.getDefeats()));
    lines.push_back("Fuites réussies : " + std::to_string(player.getEscapes()));
    lines.push_back("Ennemis vaincus : " + std::to_string(player.getEnemiesKilled()));
    lines.push_back("Boss vaincus : " + std::to_string(player.getBossesKilled()));
    lines.push_back("JcJ remportés : " + std::to_string(player.getPvpVictories()));
    lines.push_back("JcJ perdus : " + std::to_string(player.getPvpDefeats()));

    if (difficultyKnown && difficulty == DifficultyMode::Lethal)
    {
        lines.push_back("Morts du personnage : [STATISTIQUE CORROMPUE]");
        lines.push_back("Vous ne deviez pas mourir.");
    }
    else
    {
        lines.push_back("Morts du personnage : " + std::to_string(player.getDeaths()));

        if (!difficultyKnown)
        {
            lines.push_back("Note : le contexte de difficulté exact n'est pas connu depuis ce sous-menu.");
        }
    }

    const std::vector<std::string>& eliminations = player.getPvpLethalEliminations();
    if (!eliminations.empty())
    {
        lines.push_back("Éliminations Léthal en JcJ :");
        for (const std::string& entry : eliminations)
        {
            lines.push_back("- " + entry);
        }
    }

    showStatisticsScreen("STATISTIQUES DE COMBAT", "statistics.combat.detail", lines);
}

void StatisticsMenu::displayEquipmentUsage(const Player& player)
{
    std::vector<std::string> lines;
    lines.push_back("Équipement actuel :");
    lines.push_back("Note : les conseils d'entretien sont surtout regroupés ici pour éviter de spammer le joueur pendant toutes les quêtes.");
    lines.push_back("- Arme : " + player.getEquippedWeapon().getName());
    for (const std::string& warning : DurabilityRules::describeWeaponUseWarnings(player.getEquippedWeapon(), player.getRace()))
    {
        lines.push_back("  > " + warning);
    }
    lines.push_back("- Armure : " + player.getEquippedArmor().getName());
    for (const std::string& warning : DurabilityRules::describeArmorFitWarnings(player.getEquippedArmor(), player.getRace()))
    {
        lines.push_back("  > " + warning);
    }

    if (player.getCursePressureForCategory("equipment") > 0)
    {
        lines.push_back("- Pression maudite sur l'équipement : " + std::to_string(player.getCursePressureForCategory("equipment")) + "/12.");
        lines.push_back("  > Les alertes d'usure restent lisibles, mais une trace maudite peut rendre l'entretien moins fiable.");
    }

    std::vector<std::string> titleEquipmentLines = equippedTitleEquipmentLines(player);
    if (!titleEquipmentLines.empty())
    {
        lines.push_back("");
        lines.push_back("Influence faible des titres équipés :");
        lines.insert(lines.end(), titleEquipmentLines.begin(), titleEquipmentLines.end());
    }

    lines.push_back("");
    lines.push_back("Équipement récent :");

    const std::vector<std::string>& usage = player.getRecentCombatEquipmentUsage();
    if (usage.empty())
    {
        lines.push_back("- Aucun historique récent d'équipement.");
    }
    else
    {
        std::map<std::string, int> counts;
        for (const std::string& itemName : usage)
        {
            counts[itemName]++;
        }

        for (const auto& pair : counts)
        {
            lines.push_back("- " + pair.first + " : " + std::to_string(pair.second) + " apparition(s) récente(s)");
        }

        lines.push_back("");
        lines.push_back("Lecture durable :");
        lines.push_back("- 5 combats récents avec armure nourrissent Habitude d'armure.");
        lines.push_back("- 5 combats récents avec arme + armure nourrissent Mémoire d'équipement.");
        lines.push_back("- Une usure très basse peut nourrir Entretien de terrain si le personnage survit assez longtemps.");
    }

    showStatisticsScreen("ÉQUIPEMENT ET OBJETS RÉCENTS", "statistics.equipment.detail", lines);
}

void StatisticsMenu::displaySkillStats(const Player& player)
{
    std::vector<std::string> lines;
    appendSkillList(lines, "Passives connues :", player.getUnlockedPassiveSkills());
    lines.push_back("");
    appendSkillList(lines, "Actives connues :", player.getUnlockedActiveSkills());
    lines.push_back("");
    lines.push_back("Traces d'entraînement visibles :");
    lines.push_back(progressLine("Armes courtes / dagues", player.getDaggerKillProgress(), 5));
    lines.push_back(progressLine("Arcs et tirs", player.getBowKillProgress(), 8));
    lines.push_back(progressLine("Combat à mains nues", player.getBareHandKillProgress(), 10));
    lines.push_back(progressLine("Bâtons et canalisation", player.getStaffKillProgress(), 6));
    lines.push_back(progressLine("Épées et discipline", player.getSwordKillProgress(), 7));
    lines.push_back(progressLine("Haches et ouverture", player.getAxeKillProgress(), 7));
    lines.push_back(progressLine("Marteaux et fracture de garde", player.getHammerKillProgress(), 7));
    lines.push_back(progressLine("Lances et contrôle de distance", player.getSpearKillProgress(), 7));
    lines.push_back("");
    lines.push_back("Ces traces montrent ce que ton personnage répète assez souvent pour l'intégrer à son style.");
    appendSkillRoadmap(lines, player);

    showStatisticsScreen("COMPÉTENCES", "statistics.skills.detail", lines);
}

void StatisticsMenu::displayTitleCatalog(Player& player)
{
    while (true)
    {
        MenuScreen screen("TITRES", "statistics.titles.hub");
        screen.addLine("Titres équipés : " + player.getActiveTitleSummary());
        screen.addLine("Titres possédés : " + std::to_string(player.getTitles().size()));
        screen.addLine("Les titres cachés restent en ???? tant qu'ils ne sont pas obtenus.");
        screen.addOption(1, "Titres obtenus — liste simple", "Nom des titres possédés, avec marque des titres équipés.", true, "statistics.titles.owned.simple");
        screen.addOption(2, "Titres obtenus — liste détaillée", "Nom, obtention et effet de chaque titre possédé.", true, "statistics.titles.owned.detail");
        screen.addOption(3, "Titres disponibles — semi-détaillé", "Titres visibles avec condition d'obtention ; titres secrets masqués en ????", true, "statistics.titles.available.visible");
        screen.addOption(4, "Équiper / changer les titres", "Choisir jusqu'à 3 titres équipés, surtout pour le style et le lore.", true, "statistics.titles.equip");
        screen.addBackOption("Retour", "statistics.titles.back");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix invalide. Choisis une option affichée."
        );
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        std::vector<std::string> lines;
        lines.push_back("Titres équipés : " + player.getActiveTitleSummary());
        lines.push_back("Titres possédés : " + std::to_string(player.getTitles().size()));
        lines.push_back("");

        if (choice == 1)
        {
            lines.push_back("Titres obtenus — simple :");
            if (player.getTitles().empty())
            {
                lines.push_back("- Aucun titre obtenu pour l'instant.");
            }
            else
            {
                for (const std::string& title : player.getTitles())
                {
                    lines.push_back("- " + title + (std::find(player.getActiveTitles().begin(), player.getActiveTitles().end(), title) != player.getActiveTitles().end() ? " [équipé]" : ""));
                }
            }
            showStatisticsScreen("TITRES OBTENUS — SIMPLE", "statistics.titles.owned.simple.detail", lines);
        }
        else if (choice == 2)
        {
            lines.push_back("Titres obtenus — détaillé :");
            if (player.getTitles().empty())
            {
                lines.push_back("- Aucun titre obtenu pour l'instant.");
            }
            else
            {
                for (const std::string& title : player.getTitles())
                {
                    lines.push_back("- " + title + (std::find(player.getActiveTitles().begin(), player.getActiveTitles().end(), title) != player.getActiveTitles().end() ? " [équipé]" : ""));
                    lines.push_back("  Obtention : " + TitleCatalog::unlockHintFor(title));
                    lines.push_back("  Effet : " + TitleCatalog::effectFor(title));
                }
            }
            lines.push_back("");
            lines.push_back("Note : l'effet d'un titre reste surtout contextuel/roleplay. Les gros bonus directs seront évités pour ne pas forcer le joueur à optimiser les titres.");
            showStatisticsScreen("TITRES OBTENUS — DÉTAILLÉ", "statistics.titles.owned.detail.detail", lines);
        }
        else if (choice == 3)
        {
            lines.push_back("Titres disponibles — semi-détaillé :");
            lines.push_back("Les titres visibles affichent seulement comment les obtenir. Les titres secrets restent volontairement cachés.");
            std::string currentCategory;
            int hiddenCount = 0;
            for (const TitleCatalog::TitleDefinition& title : TitleCatalog::availableTitleDefinitions())
            {
                if (!titleIsRevealed(player, title))
                {
                    ++hiddenCount;
                    continue;
                }

                if (title.category != currentCategory)
                {
                    currentCategory = title.category;
                    lines.push_back("");
                    lines.push_back(currentCategory + " :");
                }

                const bool owned = player.hasTitle(title.name);
                lines.push_back("- " + std::string(owned ? "[obtenu] " : "[visible] ") + title.name);
                lines.push_back("  Comment l'obtenir : " + title.unlockHint);
            }

            if (hiddenCount > 0)
            {
                lines.push_back("");
                lines.push_back("Titres secrets non découverts : " + std::to_string(hiddenCount));
                for (int i = 0; i < hiddenCount; ++i)
                {
                    lines.push_back(secretTitleLine());
                }
            }

            lines.push_back("");
            lines.push_back("Note : cacher les titres rares évite de pousser le joueur à changer son gameplay juste pour cocher une condition.");
            showStatisticsScreen("TITRES DISPONIBLES", "statistics.titles.available.detail", lines);
        }
        else if (choice == 4)
        {
            displayTitleEquipMenu(player);
        }
    }
}

void StatisticsMenu::displayTitleEquipMenu(Player& player)
{
    if (player.getTitles().empty())
    {
        MessageScreen::show(
            "TITRES ÉQUIPÉS",
            "statistics.titles.equip.empty",
            {
                "Aucun titre possédé pour l\'instant.",
                "Les titres équipés servent surtout à afficher le style, la réputation et le lore du personnage."
            }
        );
        Console::clear();
        return;
    }

    while (true)
    {
        MenuScreen screen("ÉQUIPER DES TITRES", "statistics.titles.equip.hub");
        screen.addLine("Jusqu\'à 3 titres peuvent être équipés.");
        screen.addLine("Effets : très faibles, surtout dialogues, réputation et ambiance.");
        screen.addLine("Titres équipés : " + player.getActiveTitleSummary());
        screen.addLine("");
        const std::vector<std::string>& equipped = player.getActiveTitles();
        for (int slot = 0; slot < 3; ++slot)
        {
            const std::string slotText = static_cast<std::size_t>(slot) < equipped.size() ? equipped[static_cast<std::size_t>(slot)] : std::string("Vide");
            screen.addLine("Emplacement " + std::to_string(slot + 1) + " : " + slotText);
            screen.addOption(slot + 1, "Changer l\'emplacement " + std::to_string(slot + 1), "Choisir un titre possédé ou vider cet emplacement.", true, "statistics.titles.equip.slot");
        }
        screen.addOption(4, "Voir les influences équipées", "Résumé des effets faibles et contextuels des titres équipés.", true, "statistics.titles.equip.effects");
        screen.addBackOption("Retour", "statistics.titles.equip.back");

        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide. Choisis un emplacement affiché.");
        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (choice == 4)
        {
            std::vector<std::string> lines = player.describeActiveTitleEffects();
            showStatisticsScreen("INFLUENCE DES TITRES", "statistics.titles.equip.effects.detail", lines);
            continue;
        }

        if (choice < 1 || choice > 3)
        {
            continue;
        }

        const int slotIndex = choice - 1;
        MenuScreen chooseScreen("CHOISIR UN TITRE", "statistics.titles.equip.choose");
        chooseScreen.addLine("Emplacement " + std::to_string(choice) + ".");
        chooseScreen.addLine("Les doublons sont évités automatiquement.");
        chooseScreen.addOption(1, "Vider cet emplacement", "Retire le titre équipé de cet emplacement.", true, "statistics.titles.equip.clear");

        const std::vector<std::string>& ownedTitles = player.getTitles();
        for (std::size_t i = 0; i < ownedTitles.size(); ++i)
        {
            const bool alreadyEquipped = std::find(player.getActiveTitles().begin(), player.getActiveTitles().end(), ownedTitles[i]) != player.getActiveTitles().end();
            chooseScreen.addOption(static_cast<int>(i) + 2, ownedTitles[i] + (alreadyEquipped ? " [déjà équipé]" : ""), TitleCatalog::effectFor(ownedTitles[i]), true, "statistics.titles.equip.title");
        }
        chooseScreen.addBackOption("Retour", "statistics.titles.equip.choose.back");

        const int titleChoice = TerminalInterface::askMenuChoiceFromOptions(chooseScreen, "Choix invalide. Choisis un titre affiché.");
        Console::clear();

        if (titleChoice == 0)
        {
            continue;
        }

        if (titleChoice == 1)
        {
            player.unequipActiveTitleSlot(slotIndex);
            MessageScreen::show("TITRE RETIRÉ", "statistics.titles.equip.cleared", {"Emplacement vidé.", "Titres équipés : " + player.getActiveTitleSummary()});
            Console::clear();
            continue;
        }

        const int ownedIndex = titleChoice - 2;
        if (ownedIndex >= 0 && static_cast<std::size_t>(ownedIndex) < ownedTitles.size())
        {
            const std::string chosenTitle = ownedTitles[static_cast<std::size_t>(ownedIndex)];
            if (player.setActiveTitleSlot(slotIndex, chosenTitle))
            {
                MessageScreen::show("TITRE ÉQUIPÉ", "statistics.titles.equip.updated", {"Titre équipé : " + chosenTitle, "Titres équipés : " + player.getActiveTitleSummary(), "Bonus : très faible, surtout lore/dialogues."});
            }
            else
            {
                MessageScreen::show("TITRE NON ÉQUIPÉ", "statistics.titles.equip.failed", {"Impossible d\'équiper ce titre pour le moment."});
            }
            Console::clear();
        }
    }
}

void StatisticsMenu::displaySpecialStates(const Player& player)
{
    std::vector<std::string> lines;
    lines.push_back("Altéré : " + std::string(player.isAlteredByCheats() ? "oui" : "non"));
    lines.push_back("Clone : " + std::string(player.isClone() ? "oui" : "non"));

    if (player.hasWorldGazePenalty())
    {
        lines.push_back("Regard du monde : actif");
        lines.push_back("Tentatives de rupture Léthal : " + std::to_string(player.getLethalCheatAttemptCount()));
    }
    else if (player.getLethalCheatAttemptCount() > 0)
    {
        lines.push_back("Trace interdite : ancienne tentative de cheat Léthal détectée");
        lines.push_back("Tentatives : " + std::to_string(player.getLethalCheatAttemptCount()));
    }

    if (player.hasZelefCorrosionPresent())
    {
        lines.push_back("Corrosion présente : " + std::to_string(player.getZelefMaxHpStolen()) + " PV max retenus");
    }

    if (player.hasGrinkaBossTheftPresent())
    {
        lines.push_back("Volé par un boss : Grinka détient encore quelque chose.");
    }

    if (player.hasBossEquipmentSeal())
    {
        lines.push_back("Équipement scellé : " + player.getBossEquipmentSealReason());
    }

    const std::vector<PlayerCurse>& curses = player.getActiveCurses();
    if (!curses.empty())
    {
        lines.push_back("");
        lines.push_back("Malédictions actives :");
        for (const PlayerCurse& curse : curses)
        {
            lines.push_back("- " + curse.name + " | niveau " + std::to_string(curse.curseLevel) + "/" + std::to_string(curse.maxCurseLevel) + " | " + curse.severity);
            lines.push_back("  > Catégories : " + curseCategorySummary(curse.symptomCategories));
            lines.push_back("  > Ancrage : " + curseAnchorText(curse));
            if (!curse.removalHint.empty())
            {
                lines.push_back("  > Piste : " + curse.removalHint);
            }
        }
    }

    if (lines.size() == 2)
    {
        lines.push_back("Aucune conséquence spéciale majeure visible pour le moment.");
    }

    showStatisticsScreen("ÉTATS SPÉCIAUX", "statistics.states.detail", lines);
}
