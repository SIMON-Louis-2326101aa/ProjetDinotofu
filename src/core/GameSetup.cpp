// EN: Character creation and local party setup extracted from Game.cpp.
// FR: Création de personnage et configuration du groupe local extraites de Game.cpp.

#include "core/Game.hpp"
#include "diagnostic/RuntimeLog.hpp"
#include "core/Console.hpp"
#include "core/Random.hpp"
#include "core/VersionInfo.hpp"
#include "class_system/ClassCatalog.hpp"
#include "combat/Combat.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "combat/modes/pve/MonsterPveMode.hpp"
#include "boss/BossCatalog.hpp"
#include "entity/Monster.hpp"
#include "character/RaceCatalog.hpp"
#include "character/SpecialCharacterNativeBonus.hpp"
#include "save/SaveManager.hpp"
#include "save/menu/AccountMenu.hpp"
#include "save/menu/CharacterMenu.hpp"
#include "economy/shop/ShopRotationSystem.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "interface/menu/shop/ShopMenu.hpp"
#include "interface/menu/progression/AttributeMenu.hpp"
#include "interface/menu/progression/StatisticsMenu.hpp"
#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/PostCombatMenu.hpp"
#include "interface/menu/training/TrainingGroundMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "cheat/CheatManager.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/DeathRuleRules.hpp"
#include "progression/death/DeathPenaltySystem.hpp"
#include "item/weapon/Weapon.hpp"
#include "item/armor/Armor.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/material/Material.hpp"
#include "story/StoryCampaign.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <exception>
#include <set>
#include <array>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    std::string normalizeClassAuditText(std::string value)
    {
        std::string out;
        for (unsigned char character : value)
        {
            if (std::isalnum(character))
            {
                out += static_cast<char>(std::tolower(character));
            }
        }
        return out;
    }

    bool classAuditContainsAny(const std::string& value, const std::vector<std::string>& words)
    {
        const std::string normalized = normalizeClassAuditText(value);
        for (const std::string& word : words)
        {
            if (normalized.find(normalizeClassAuditText(word)) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    std::string classCategoryBriefExample(ClassCategory category)
    {
        switch (category)
        {
            case ClassCategory::Melee:
                return "Contact direct, pression et armes proches. Exemple : Chevalier.";
            case ClassCategory::Distance:
                return "Tirs, munitions et placement. Exemple : Archer.";
            case ClassCategory::Magic:
                return "Sorts, catalyseurs et statuts. Exemple : Mage.";
            case ClassCategory::Invocation:
                return "Alliés appelés, sacrifices et tempo. Exemple : Invocateur.";
            case ClassCategory::Support:
                return "Protection, soin et contrôle défensif. Exemple : Clerc.";
            case ClassCategory::Hybrid:
                return "Mélange arme, magie ou rôle spécial. Exemple : Mage-lame.";
            case ClassCategory::Craft:
                return "Objets, économie, kits et expériences. Exemple : Forgeron.";
            case ClassCategory::Special:
            default:
                return "Profil spécial ou rare, plutôt lié à une histoire précise.";
        }
    }

    std::vector<std::string> classFuturePotentialLines(const ClassOptionInfo& info)
    {
        std::vector<std::string> lines;
        const std::string category = info.categoryName;
        const std::string name = info.name;

        if (category.find("Corps") != std::string::npos)
        {
            lines.push_back("Plus tard : styles d'arme, posture, riposte, provocation ou brise-garde selon la classe.");
        }
        else if (category == "Distance")
        {
            lines.push_back("Plus tard : munitions spéciales, pistage, tirs préparés, pièges et lecture de terrain.");
        }
        else if (category == "Magie")
        {
            lines.push_back("Plus tard : grimoires, sorts apprenables, catalyseurs, risques de canalisation et affinités élémentaires.");
        }
        else if (category == "Invocation")
        {
            lines.push_back("Plus tard : invocations maintenues, contrôle d'alliés, coût en mana et pertes possibles si le rythme casse.");
        }
        else if (category == "Soutien")
        {
            lines.push_back("Plus tard : soin, protection, stabilisation, lecture du danger et réactions aux alliés en difficulté.");
        }
        else if (category == "Hybride")
        {
            lines.push_back("Plus tard : mélange d'arme, magie et utilitaire, avec des bonus plus contextuels qu'une classe pure.");
        }
        else if (category.find("Artisanat") != std::string::npos)
        {
            lines.push_back("Plus tard : recettes, réparation, récolte propre, économie de ressources et services hors combat.");
        }
        else
        {
            lines.push_back("Plus tard : identité spéciale, effets plus liés au lore, à l'histoire ou à une règle unique.");
        }

        if (name.find("Rôdeur") != std::string::npos || name.find("Pisteur") != std::string::npos || name.find("Cartographe") != std::string::npos || name.find("Éclaireur") != std::string::npos)
        {
            lines.push_back("Évolution probable : meilleur repérage, aide aux explorations longues, distances mieux préparées et risques nocturnes mieux lus.");
        }
        if (name.find("Médecin") != std::string::npos || name.find("Clerc") != std::string::npos || name.find("Lumomancien") != std::string::npos)
        {
            lines.push_back("Évolution probable : soins plus propres, stabilisation, potions mieux utilisées et récupération hors combat améliorée.");
        }
        if (name.find("Forgeron") != std::string::npos || name.find("Récupérateur") != std::string::npos || name.find("Herboriste") != std::string::npos || name.find("Cuisinier") != std::string::npos)
        {
            lines.push_back("Évolution probable : meilleures recettes, rendement de matériaux, réparations et bonus de préparation avant sortie.");
        }
        if (name.find("Assassin") != std::string::npos || name.find("Umbromancien") != std::string::npos || name.find("Occultiste") != std::string::npos)
        {
            lines.push_back("Évolution probable : furtivité, ombre, critique ou approches nocturnes plus dangereuses mais plus rentables.");
        }

        lines.push_back("Note : ces évolutions prévues n'activent pas tout de suite un pouvoir gratuit ; elles servent à choisir une direction claire.");
        return lines;
    }

    std::vector<std::string> classWeaponGuidanceLines(const ClassOptionInfo& info)
    {
        std::vector<std::string> lines;
        const std::string name = info.name;
        const std::string category = info.categoryName;

        if (name.find("Archer") != std::string::npos || name.find("Rôdeur") != std::string::npos || name.find("Tireur") != std::string::npos || name.find("Arbal") != std::string::npos || name.find("Chasseur") != std::string::npos || name.find("Guetteur") != std::string::npos || name.find("Trappeur") != std::string::npos || category == "Distance")
        {
            lines.push_back("Recommandé : arc, arbalète/armes de trait, javelot ou dague de secours selon la classe.");
            lines.push_back("Maladroit : marteau/hache lourde, sauf classe explicitement prévue pour ça.");
        }
        else if (name.find("Assassin") != std::string::npos || name.find("Ombrelame") != std::string::npos || name.find("Lanceur de dagues") != std::string::npos || name.find("Danseur lunaire") != std::string::npos)
        {
            lines.push_back("Recommandé : dague, lame courte, sabre léger ou arme discrète.");
            lines.push_back("Maladroit : marteau, hache lourde ou bâton de canalisation trop voyant.");
        }
        else if (name.find("Mage") != std::string::npos || name.find("mancien") != std::string::npos || name.find("Sorcier") != std::string::npos || name.find("Arcaniste") != std::string::npos || name.find("Invoc") != std::string::npos || name.find("Nécro") != std::string::npos || category == "Magie" || category == "Invocation")
        {
            lines.push_back("Recommandé : bâton, sceptre, catalyseur ou dague légère de secours.");
            lines.push_back("Maladroit : armes très lourdes et arcs non canalisateurs, sauf classe hybride prévue pour ça.");
        }
        else if (name.find("Moine") != std::string::npos || name.find("Pugiliste") != std::string::npos || name.find("Cogneur") != std::string::npos)
        {
            lines.push_back("Recommandé : mains nues, gants, armes très courtes ou équipement léger.");
            lines.push_back("Maladroit : arme lourde qui casse le rythme du corps-à-corps.");
        }
        else if (name.find("Lancier") != std::string::npos || name.find("Hallebardier") != std::string::npos || name.find("Javelinier") != std::string::npos)
        {
            lines.push_back("Recommandé : lance, hallebarde, javelot ou arme d'allonge.");
            lines.push_back("Maladroit : arme trop courte si la classe mise tout sur la portée.");
        }
        else if (name.find("Barbare") != std::string::npos || name.find("Berserker") != std::string::npos || name.find("Briseur") != std::string::npos || name.find("Martelier") != std::string::npos || name.find("Colosse") != std::string::npos)
        {
            lines.push_back("Recommandé : hache, marteau, grande arme ou lame lourde.");
            lines.push_back("Maladroit : arc fin, bâton fragile ou arme trop subtile.");
        }
        else if (name.find("Chevalier") != std::string::npos || name.find("Guerrier") != std::string::npos || name.find("Paladin") != std::string::npos || name.find("Templier") != std::string::npos || name.find("Gladiateur") != std::string::npos)
        {
            lines.push_back("Recommandé : épée, lance, marteau de guerre ou arme martiale stable.");
            lines.push_back("Maladroit : bâton de mage ou arc si la classe n'a pas appris ce rythme.");
        }
        else
        {
            lines.push_back("Recommandé : arme cohérente avec le rôle affiché ; les hybrides tolèrent plus de choix.");
            lines.push_back("Maladroit : arme qui contredit totalement le style de classe.");
        }

        lines.push_back("Règle : l'arme influence maintenant à la fois la précision et les dégâts, avec bonus léger ou malus visible en combat.");
        return lines;
    }

    int classCriticalThresholdFromInfo(const ClassOptionInfo& info)
    {
        const std::string name = info.name;
        if (classAuditContainsAny(name, {"Assassin", "Ombrelame", "Duelliste", "Sabreur", "Lanceur de dagues", "Tireur", "Archer", "Éclaireur", "Eclaireur", "Danseur lunaire", "Corsaire", "Fauche-âme", "Fauche-ame"}))
        {
            return 15;
        }
        if (classAuditContainsAny(name, {"Colosse", "Gardien", "Tank", "Chevalier bouclier", "Protecteur", "Porte-bannière", "Porte-banniere", "Infirmier", "Médecin", "Medecin", "Intendant", "Aumônier", "Aumonier"}))
        {
            return 17;
        }
        return 16;
    }

    int estimateExpectedDamagePerAttack(const ClassOptionInfo& info)
    {
        const int threshold = classCriticalThresholdFromInfo(info);
        const int normalRollCount = std::max(0, threshold - 3);
        const int criticalRollCount = std::max(0, 20 - threshold);
        const int averageNormalDamage = (info.minDamage + info.maxDamage + 1) / 2;
        return (normalRollCount * averageNormalDamage + criticalRollCount * info.criticalDamage + 10) / 20;
    }

    std::string classOffenseBand(const ClassOptionInfo& info)
    {
        const int expectedDamage = estimateExpectedDamagePerAttack(info);
        if (expectedDamage >= 25) return "très haute mais à risque";
        if (expectedDamage >= 21) return "forte";
        if (expectedDamage >= 17) return "correcte";
        if (expectedDamage >= 13) return "basse mais compensée";
        return "très basse, rôle surtout utilitaire/défensif";
    }

    std::string classSurvivalBand(const ClassOptionInfo& info)
    {
        const int survivalScore = info.maxHp + info.healingPotionCount * 22;
        if (survivalScore >= 430) return "mur très solide";
        if (survivalScore >= 330) return "très bonne tenue";
        if (survivalScore >= 250) return "tenue correcte";
        if (survivalScore >= 200) return "fragile mais jouable";
        return "très fragile";
    }

    std::vector<std::string> classBalanceAuditLines(const ClassOptionInfo& info)
    {
        std::vector<std::string> lines;
        const int threshold = classCriticalThresholdFromInfo(info);
        const int critChance = std::max(0, 20 - threshold) * 5;
        const int expectedDamage = estimateExpectedDamagePerAttack(info);
        lines.push_back("Audit d'équilibre : offense " + classOffenseBand(info)
            + " | survie " + classSurvivalBand(info)
            + " | critique environ " + std::to_string(critChance) + "%.");
        lines.push_back("Dégât moyen théorique par attaque simple : environ " + std::to_string(expectedDamage)
            + " avant équipement, armure adverse, résistance, potions, maîtrise et effets de statut.");

        if (threshold <= 15)
        {
            lines.push_back("Identité : critique plus fréquent. La classe doit être dangereuse quand elle trouve l'ouverture, mais reste punie par une mauvaise lecture ou une défense faible.");
        }
        else if (threshold >= 17)
        {
            lines.push_back("Identité : critique moins fréquent. La classe compense par PV, garde, protection, soin ou stabilité.");
        }
        else if (classAuditContainsAny(info.name, {"Mage", "mancien", "Sorcier", "Arcaniste", "Démoniste", "Demoniste"}))
        {
            lines.push_back("Identité : magie dangereuse et effets visibles, mais dépendance au catalyseur, aux ressources et aux fenêtres de canalisation.");
        }
        else if (classAuditContainsAny(info.name, {"Barbare", "Berserker", "Ravageur", "Briseur", "Martelier", "Faucheur"}))
        {
            lines.push_back("Identité : impact lourd qui doit se sentir, mais avec risques de tempo, précision ou exposition.");
        }
        else if (info.categoryName == "Soutien")
        {
            lines.push_back("Identité : dégâts directs moins hauts, compensation par soins, garde, moral, lecture et survie d'équipe.");
        }
        else
        {
            lines.push_back("Identité : profil standard ou hybride, équilibré par équipement, compétences futures et contexte de combat.");
        }

        if (expectedDamage >= 24 && info.maxHp <= 165)
        {
            lines.push_back("Point de vigilance : fort potentiel offensif, mais si l'entrée est ratée la classe peut tomber vite.");
        }
        else if (expectedDamage <= 13 && info.maxHp >= 300)
        {
            lines.push_back("Point de vigilance : dégâts directs faibles volontairement, sinon le rôle de mur deviendrait trop gratuit.");
        }
        else if (expectedDamage <= 14 && info.categoryName == "Soutien")
        {
            lines.push_back("Point de vigilance : cette classe doit gagner par sécurité, tempo ou soutien, pas par le meilleur DPS brut.");
        }
        return lines;
    }

    std::vector<std::string> buildClassInspectionLines(const ClassOptionInfo& info)
    {
        std::vector<std::string> lines;
        lines.push_back("Classe : " + info.name + ".");
        lines.push_back("Famille : " + info.categoryName + ".");
        lines.push_back("Rôle de base : " + info.role + ".");
        lines.push_back("Stats de départ : PV " + std::to_string(info.maxHp)
            + " | dégâts " + std::to_string(info.minDamage) + "-" + std::to_string(info.maxDamage)
            + " | critique " + std::to_string(info.criticalDamage) + ".");
        lines.push_back("Ressources de départ : potions soin " + std::to_string(info.healingPotionCount)
            + " | potions dégâts " + std::to_string(info.damagePotionCount) + ".");
        lines.push_back("");
        lines.push_back("Lecture équilibre :");
        const std::vector<std::string> balanceLines = classBalanceAuditLines(info);
        for (const std::string& line : balanceLines)
        {
            lines.push_back("- " + line);
        }
        lines.push_back("");
        lines.push_back("Affinités d'armes :");
        const std::vector<std::string> weaponLines = classWeaponGuidanceLines(info);
        for (const std::string& line : weaponLines)
        {
            lines.push_back("- " + line);
        }
        lines.push_back("");
        lines.push_back("Ce qu'elle a de base :");
        lines.push_back("- Une identité de combat, des statistiques et un kit initial adaptés à la difficulté.");
        lines.push_back("- Des armes/potions de départ qui seront ensuite complétées par l'inventaire, les boutiques et les quêtes.");
        lines.push_back("");
        lines.push_back("Ce qu'elle pourrait avoir plus tard :");
        const std::vector<std::string> futureLines = classFuturePotentialLines(info);
        for (const std::string& line : futureLines)
        {
            lines.push_back("- " + line);
        }
        return lines;
    }

    MenuOptionItemData makeCreationItemData(
        const std::string& kind,
        const std::string& section,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = "",
        const std::string& progress = "",
        const std::string& reward = "",
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = kind;
        itemData.section = section;
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.progress = progress;
        itemData.reward = reward;
        itemData.owner = "Création de personnage";
        itemData.important = important;
        return itemData;
    }

    MenuOptionItemData makeSessionItemData(
        const Player& player,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = "",
        const std::string& progress = "",
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "session";
        itemData.section = "Session";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.progress = progress.empty() ? "Joueur 1 : " + player.getName() : progress;
        itemData.owner = player.getName();
        itemData.important = important;
        return itemData;
    }

    std::string difficultyModeLabel(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                return "Facile";
            case DifficultyMode::Hard:
                return "Difficile";
            case DifficultyMode::Nightmare:
                return "Cauchemar";
            case DifficultyMode::Lethal:
                return "Léthal";
            case DifficultyMode::Normal:
            default:
                return "Normal";
        }
    }

    std::string difficultyRespawnSummary(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                return "Respawn non définitif : environ 75% PV.";
            case DifficultyMode::Hard:
                return "Respawn non définitif : environ 30% PV.";
            case DifficultyMode::Nightmare:
                return "Respawn non définitif : environ 10% PV.";
            case DifficultyMode::Lethal:
                return "Respawn : mort définitive possible, sauf exception narrative rarissime.";
            case DifficultyMode::Normal:
            default:
                return "Respawn non définitif : règles standards.";
        }
    }

    std::string difficultyRiskSummary(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                return "Risque : plus permissif, pertes limitées.";
            case DifficultyMode::Hard:
                return "Risque : pertes et durabilité plus tendues.";
            case DifficultyMode::Nightmare:
                return "Risque : très punitif, équipement rarement intouchable.";
            case DifficultyMode::Lethal:
                return "Risque : le registre peut perdre ton personnage.";
            case DifficultyMode::Normal:
            default:
                return "Risque : équilibre standard.";
        }
    }

    std::string difficultyRewardSummary(DifficultyMode difficulty)
    {
        switch (difficulty)
        {
            case DifficultyMode::Easy:
                return "Récompenses : départ plus confortable.";
            case DifficultyMode::Hard:
                return "Récompenses : gains à surveiller face aux prix.";
            case DifficultyMode::Nightmare:
                return "Récompenses : chaque gain compte davantage.";
            case DifficultyMode::Lethal:
                return "Récompenses : progression risquée, trace de mort spéciale.";
            case DifficultyMode::Normal:
            default:
                return "Récompenses : valeurs de référence.";
        }
    }

}

void Game::displayIntroduction()
{
    MessageScreen::show(
        "DINOTOFU V" + VersionInfo::currentVersion(),
        "game.introduction",
        {
            "Bonjour voyageur, et bienvenue dans Dinotofu.",
            "Un monde de fantaisie, d'arènes et de baston,",
            "où chaque choix peut transformer un simple combattant en légende."
        }
    );
}

void Game::askAccountName()
{
    accountName = AccountMenu::open();
}

void Game::askPlayerName()
{
    CharacterMenuResult result = CharacterMenu::open(accountName, mainPlayer);

    characterLoaded = result.characterLoaded;
    specialIdentityValidated = result.specialIdentityValidated;
    playerName = result.playerName;

    if (result.specialIdentityValidated)
    {
        selectedRace = result.forcedRace;
    }

    if (characterLoaded)
    {
        selectedDifficulty = result.difficulty;
        selectedDeathRule = result.deathRule;
        selectedRace = mainPlayer.getRace();
        mainPlayer.forceTerminalImagePolicy();
    }
}

void Game::chooseDifficulty()
{
    MenuScreen screen("DIFFICULTÉ", "character.creation.difficulty");
    screen.addSubtitle("Ce choix influence le kit de départ, les récompenses, la mort et le respawn.");
    screen.addOption(
        1,
        "Facile",
        "Plus d'or, plus de sécurité, retour à 75% PV après une mort non définitive.",
        true,
        "difficulty.easy",
        makeCreationItemData("difficulty", "Création - difficulté", "select", "Facile", "Départ plus permissif avec davantage de sécurité.", "Sécurisé", "Respawn non définitif : 75% PV", "Ressources de départ améliorées")
    );
    screen.addOption(
        2,
        "Normal",
        "L'expérience Dinotofu standard.",
        true,
        "difficulty.normal",
        makeCreationItemData("difficulty", "Création - difficulté", "select", "Normal", "Équilibre prévu pour la majorité des premières parties.", "Standard", "Règles normales", "Kit de départ standard")
    );
    screen.addOption(
        3,
        "Difficile",
        "Moins de ressources, pénalités plus dures, retour à 30% PV.",
        true,
        "difficulty.hard",
        makeCreationItemData("difficulty", "Création - difficulté", "select", "Difficile", "Moins de ressources et pénalités de mort plus fortes.", "Punitif", "Respawn non définitif : 30% PV", "Récompenses et pertes plus tendues", true)
    );
    screen.addOption(
        4,
        "Cauchemar",
        "Très punitif, retour à 10% PV, et la mort commence vraiment à avoir des dents.",
        true,
        "difficulty.nightmare",
        makeCreationItemData("difficulty", "Création - difficulté", "select", "Cauchemar", "Mode très dur placé juste avant le Léthal.", "Très dangereux", "Respawn non définitif : 10% PV", "Pertes sévères", true)
    );
    screen.addOption(
        5,
        "Léthal",
        "Le registre ne pardonne pas : une vraie chute peut effacer ton nom.",
        true,
        "difficulty.lethal",
        makeCreationItemData("difficulty", "Création - difficulté", "select", "Léthal", "Mort définitive possible : le personnage peut quitter le registre des vivants.", "Permadeath", "Historique des morts définitives", "Statistiques corrompues possibles", true)
    );

    int choice = TerminalInterface::askMenuChoiceFromOptions(
        screen,
        "Veuillez entrer un chiffre correspondant à une difficulté affichée."
    );

    switch (choice)
    {
        case 1:
            selectedDifficulty = DifficultyMode::Easy;
            break;

        case 3:
            selectedDifficulty = DifficultyMode::Hard;
            break;

        case 4:
            selectedDifficulty = DifficultyMode::Nightmare;
            break;

        case 5:
            selectedDifficulty = DifficultyMode::Lethal;
            break;

        case 2:
        default:
            selectedDifficulty = DifficultyMode::Normal;
            break;
    }

    Console::clear();

    MenuScreen confirmation("DIFFICULTÉ VALIDÉE", "character.creation.difficulty.confirmation");
    confirmation.setContinueInput("Valide pour continuer vers la suite de création.");
    confirmation.addSubtitle("Transition de création");
    confirmation.addLine("Difficulté sélectionnée : " + getDifficultyName() + ".");
    confirmation.addLine(difficultyRiskSummary(selectedDifficulty));
    confirmation.addLine(difficultyRespawnSummary(selectedDifficulty));
    confirmation.addLine(difficultyRewardSummary(selectedDifficulty));
    confirmation.addLine("Prochaine étape : règle de mort du personnage.");
    confirmation.addLine("Ton départ sera ajusté en conséquence.");

    if (specialIdentityValidated)
    {
        confirmation.addLine("Identité spéciale reconnue : le choix de race est verrouillé par son histoire.");
        confirmation.addLine("Race imposée : " + characterRaceToText(selectedRace) + ".");
    }

    TerminalInterface::renderMenuScreen(confirmation, false);
    Console::waitForEnter();
    Console::clear();
}

void Game::chooseDeathRule()
{
    selectedDeathRule = DeathRuleRules::normalizeForDifficulty(selectedDifficulty, selectedDeathRule);

    if (DeathRuleRules::isChoiceForced(selectedDifficulty))
    {
        MenuScreen forcedScreen("RÈGLE DE MORT", "character.creation.death_rule.forced");
        forcedScreen.addSubtitle("Règle imposée par la difficulté");
        forcedScreen.addLine("Difficulté : " + getDifficultyName() + ".");
        forcedScreen.addLine("Règle appliquée : " + DeathRuleRules::displayName(selectedDeathRule) + ".");
        forcedScreen.addLine(DeathRuleRules::shortSummary(selectedDifficulty, selectedDeathRule));
        forcedScreen.addLine("Prochaine étape : " + std::string(specialIdentityValidated ? "classe" : "race"));
        forcedScreen.setContinueInput("Valide pour continuer.");
        TerminalInterface::renderMenuScreen(forcedScreen, false);
        Console::waitForEnter();
        Console::clear();
        return;
    }

    MenuScreen screen("RÈGLE DE MORT", "character.creation.death_rule");
    screen.addSubtitle("Ce choix est séparé de la difficulté.");
    screen.addLine("Difficulté choisie : " + getDifficultyName() + ".");
    screen.addLine("Facile bloque toujours la mort définitive ; Léthal la force toujours.");
    screen.addOption(
        1,
        "Mort non définitive",
        "Tu peux subir des pertes et pénalités, mais le personnage reste jouable après respawn.",
        true,
        "death_rule.non_definitive",
        makeCreationItemData("death_rule", "Création - règle de mort", "select", "Mort non définitive", "Respawn conservé malgré les pénalités.", "Sécurisé", "Pas d'effacement jouable", "Compatible hors Facile/Léthal")
    );
    screen.addOption(
        2,
        "Mort définitive",
        "Challenge supplémentaire : une vraie chute peut déplacer le personnage au registre des morts.",
        true,
        "death_rule.definitive",
        makeCreationItemData("death_rule", "Création - règle de mort", "select", "Mort définitive", "Le risque de registre mort s'ajoute à la difficulté choisie.", "Permadeath", "Personnage supprimé des jouables si mort", "Même garde-fous JcJ que Léthal", true)
    );

    int choice = TerminalInterface::askMenuChoiceFromOptions(
        screen,
        "Veuillez choisir une règle de mort affichée."
    );

    selectedDeathRule = DeathRuleRules::normalizeForDifficulty(
        selectedDifficulty,
        choice == 2 ? DeathRuleMode::Definitive : DeathRuleMode::NonDefinitive
    );

    Console::clear();

    MenuScreen confirmation("RÈGLE DE MORT VALIDÉE", "character.creation.death_rule.confirmation");
    confirmation.setContinueInput("Valide pour continuer vers la suite de création.");
    confirmation.addSubtitle("Transition de création");
    confirmation.addLine("Difficulté : " + getDifficultyName() + ".");
    confirmation.addLine("Règle de mort : " + getDeathRuleName() + ".");
    confirmation.addLine(DeathRuleRules::shortSummary(selectedDifficulty, selectedDeathRule));
    confirmation.addLine("Prochaine étape : " + std::string(specialIdentityValidated ? "classe" : "race"));
    TerminalInterface::renderMenuScreen(confirmation, false);
    Console::waitForEnter();
    Console::clear();
}

void Game::choosePlayerRace()
{
    struct RaceCreationGroup
    {
        std::string title;
        std::string hint;
        std::vector<CharacterRace> races;
    };

    const std::vector<RaceCreationGroup> groups = {
        {
            "Races classiques",
            "Humain, elfes, nain, gnome, halfelin et orc.",
            {
                CharacterRace::Human,
                CharacterRace::Elf,
                CharacterRace::DarkElf,
                CharacterRace::Dwarf,
                CharacterRace::Gnome,
                CharacterRace::Halfling,
                CharacterRace::Orc
            }
        },
        {
            "Races mystiques",
            "Origines marquées par la magie, la lumière, l'ombre ou les pactes.",
            {
                CharacterRace::Tiefling,
                CharacterRace::Aasimar,
                CharacterRace::Kitsune,
                CharacterRace::Fairy,
                CharacterRace::Vampire,
                CharacterRace::Demon
            }
        },
        {
            "Semi-humains",
            "Catégorie dédiée aux peuples semi-humains et à leurs sous-types animaux.",
            {
                CharacterRace::SemiHuman,
                CharacterRace::SemiWolf,
                CharacterRace::SemiFox,
                CharacterRace::SemiDog,
                CharacterRace::SemiCat,
                CharacterRace::SemiLizard,
                CharacterRace::SemiBird
            }
        },
        {
            "Hybrides rares",
            "Lignées moins communes, proches des semi-humains mais traitées à part.",
            {
                CharacterRace::HalfDragon
            }
        }
    };

    while (true)
    {
        MenuScreen categoryScreen("RACE", "character.creation.race.category");
        categoryScreen.addSubtitle("Choisis d'abord une catégorie");
        categoryScreen.addLine("La liste est séparée pour éviter les pages trop longues.");
        categoryScreen.addLine("Les semi-humains et leurs sous-types sont rangés ensemble.");

        for (std::size_t i = 0; i < groups.size(); ++i)
        {
            categoryScreen.addOption(
                static_cast<int>(i + 1),
                groups[i].title,
                groups[i].hint,
                true,
                "character.race.category",
                makeCreationItemData(
                    "race_category",
                    "Création - catégorie de race",
                    "select",
                    groups[i].title,
                    groups[i].hint,
                    "Catégorie",
                    std::to_string(groups[i].races.size()) + " choix",
                    "Page courte"
                )
            );
        }

        const int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(
            categoryScreen,
            "Veuillez entrer un chiffre correspondant à une catégorie affichée."
        );
        Console::clear();

        const RaceCreationGroup& group = groups[static_cast<std::size_t>(categoryChoice - 1)];

        MenuScreen raceScreen("RACE — " + group.title, "character.creation.race");
        raceScreen.addSubtitle(group.hint);
        raceScreen.addLine("Retour permet de changer de catégorie avant de valider la race.");
        raceScreen.addBackOption("Changer de catégorie", "character.race.category.back");

        for (std::size_t i = 0; i < group.races.size(); ++i)
        {
            CharacterRace race = group.races[i];
            RaceStartingBonus bonus = RaceCatalog::getStartingBonus(race);

            std::ostringstream hint;
            hint << RaceCatalog::getGameplayIdentity(race)
                 << " | PV " << bonus.maxHpBonus
                 << " | Dégâts " << bonus.minDamageBonus << "/" << bonus.maxDamageBonus
                 << " | Critique " << bonus.criticalDamageBonus;

            if (RaceCatalog::hasInnateNightVision(race))
            {
                hint << " | Vision nocturne";
            }

            if (race == CharacterRace::Demon)
            {
                hint << " | Commerce tendu";
            }

            raceScreen.addOption(
                static_cast<int>(i + 1),
                characterRaceToText(race),
                hint.str(),
                true,
                "character.race.select",
                makeCreationItemData(
                    "race",
                    "Création - race",
                    "select",
                    characterRaceToText(race),
                    RaceCatalog::getShortDescription(race),
                    RaceCatalog::getGameplayIdentity(race),
                    "PV " + std::to_string(bonus.maxHpBonus)
                        + " | Dégâts " + std::to_string(bonus.minDamageBonus)
                        + "/" + std::to_string(bonus.maxDamageBonus)
                        + " | Critique " + std::to_string(bonus.criticalDamageBonus),
                    race == CharacterRace::Demon ? "Commerce plus tendu" : group.title,
                    race == CharacterRace::Demon
                )
            );
        }

        const int raceChoice = TerminalInterface::askMenuChoiceFromOptions(
            raceScreen,
            "Veuillez entrer un chiffre correspondant à une race affichée."
        );
        Console::clear();

        if (raceChoice == 0)
        {
            continue;
        }

        selectedRace = group.races[static_cast<std::size_t>(raceChoice - 1)];
        break;
    }

    MenuScreen confirmation("RACE VALIDÉE", "character.creation.race.confirmation");
    confirmation.setContinueInput("Valide pour continuer vers le choix de classe.");
    confirmation.addLine("Race sélectionnée : " + characterRaceToText(selectedRace) + ".");
    confirmation.addLine(RaceCatalog::getShortDescription(selectedRace));
    confirmation.addLine(RaceCatalog::getInnatePassiveLine(selectedRace));

    if (selectedRace == CharacterRace::Demon)
    {
        confirmation.addLine("Note commerce : certains marchands risquent de serrer les dents en te voyant arriver.");
        confirmation.addLine("Les prix pourront être plus élevés que la norme, surtout dans les villes peu habituées aux démons.");
    }

    TerminalInterface::renderMenuScreen(confirmation, false);
    Console::waitForEnter();
    Console::clear();
}

void Game::choosePlayerAppearance()
{
    const int maximumAge = RaceCatalog::getMaximumAge(selectedRace);
    selectedAge = MessageScreen::askQuantity(
        "ÂGE DU PERSONNAGE",
        "character.creation.appearance.age",
        {
            "Race sélectionnée : " + characterRaceToText(selectedRace) + ".",
            "Âge minimum jouable : 15 ans.",
            "Âge maximum retenu pour cette race : " + std::to_string(maximumAge) + " ans.",
            "L'âge exact sert au registre et aux futurs filtres d'images ; il ne modifie pas encore les statistiques."
        },
        15,
        maximumAge,
        "Entre un âge valide pour cette race."
    );
    Console::clear();

    MenuScreen presentationScreen("PRÉSENTATION VISUELLE", "character.creation.appearance.presentation");
    presentationScreen.addSubtitle("Ce choix sert à la description et aux futurs sprites. Il n'accorde aucun bonus.");
    presentationScreen.addOption(1, "Femme", "Filtre visuel féminin.", true, "character.appearance.presentation.female");
    presentationScreen.addOption(2, "Homme", "Filtre visuel masculin.", true, "character.appearance.presentation.male");
    presentationScreen.addOption(3, "Non-binaire / autre", "Filtre visuel non-binaire ou personnalisé.", true, "character.appearance.presentation.other");
    presentationScreen.addOption(4, "Ne pas préciser", "Conserve une présentation neutre dans le registre.", true, "character.appearance.presentation.unspecified");
    const int presentationChoice = TerminalInterface::askMenuChoiceFromOptions(presentationScreen, "Choisis une présentation visuelle.");
    selectedVisualPresentation = presentationChoice == 1 ? "Femme" : presentationChoice == 2 ? "Homme" : presentationChoice == 3 ? "Non-binaire / autre" : "Non précisé";
    Console::clear();

    MenuScreen variantScreen("VARIANTE VISUELLE", "character.creation.appearance.variant");
    variantScreen.addSubtitle("Deux propositions finales seront utilisées quand le catalogue pixel-art existera.");
    variantScreen.addLine("Le terminal n'affiche pas d'image : il conserve uniquement une description courte et fiable.");
    variantScreen.addOption(1, "Variante A — dynamique", "Silhouette plus légère, posture mobile, équipement présenté de façon vive.", true, "character.appearance.variant.a");
    variantScreen.addOption(2, "Variante B — imposante", "Silhouette plus posée, posture robuste, équipement présenté de façon lourde.", true, "character.appearance.variant.b");
    const int variantChoice = TerminalInterface::askMenuChoiceFromOptions(variantScreen, "Choisis la variante finale.");
    selectedVisualVariant = variantChoice == 2 ? "Variante B — imposante" : "Variante A — dynamique";
    Console::clear();

    MessageScreen::show(
        "APPARENCE VALIDÉE",
        "character.creation.appearance.confirmation",
        {
            "Race : " + characterRaceToText(selectedRace) + ".",
            "Âge : " + std::to_string(selectedAge) + " ans — tranche " + RaceCatalog::getAgeBand(selectedRace, selectedAge) + ".",
            "Présentation : " + selectedVisualPresentation + ".",
            "Choix final : " + selectedVisualVariant + ".",
            "L'IG utilisera plus tard ces filtres pour proposer seulement les images compatibles."
        }
    );
}

void Game::choosePlayerClass()
{
    while (true)
    {
        MenuScreen categoryScreen("FAMILLE DE CLASSE", "character.creation.class.category");
        categoryScreen.addSubtitle("L'arène range les classes par style, sans te noyer dans une liste complète.");
        categoryScreen.addLine("Choisis une famille pour entrer dedans. Depuis la liste des classes, 0 permet de revenir ici.");

        const std::vector<ClassCategory> categories = ClassCatalog::getClassCategories();

        for (std::size_t i = 0; i < categories.size(); ++i)
        {
            ClassCategory category = categories[i];
            const int categoryChoice = static_cast<int>(i + 1);
            const std::string categoryName = classCategoryToText(category);
            const std::string categoryCount = std::to_string(ClassCatalog::getPlayableClassCountByCategory(category)) + " classes disponibles";
            const std::string categoryBrief = classCategoryBriefExample(category);

            categoryScreen.addOption(
                categoryChoice,
                categoryName,
                categoryCount + " | " + categoryBrief,
                true,
                "character.class.category.select",
                makeCreationItemData(
                    "class_category",
                    "Création - famille de classe",
                    "select",
                    categoryName,
                    categoryBrief,
                    categoryCount,
                    "Entre dans la famille puis reviens avec 0 si besoin"
                )
            );
        }

        int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(
            categoryScreen,
            "Veuillez entrer un chiffre correspondant à une famille affichée."
        );

        Console::clear();

        const std::vector<ClassOptionInfo> classOptions = ClassCatalog::getClassOptionsByCategoryChoice(categoryChoice);

        while (true)
        {
            MenuScreen classScreen("CLASSE", "character.creation.class.select");
            classScreen.addSubtitle("Famille sélectionnée : " + ClassCatalog::getClassCategoryNameByChoice(categoryChoice) + ".");
            classScreen.addLine("Tu peux revenir aux familles avec 0 si le groupe ne te plaît pas.");

            classScreen.addOption(
                0,
                "Retour aux familles de classe",
                "Revenir au choix du groupe sans valider de classe.",
                true,
                "character.class.category.back",
                makeCreationItemData(
                    "class_navigation",
                    "Création - classe",
                    "back",
                    "Retour aux familles",
                    "Changer de groupe de classe avant de valider le personnage.",
                    "Navigation",
                    "Aucune classe validée"
                )
            );

            for (std::size_t i = 0; i < classOptions.size(); ++i)
            {
                const ClassOptionInfo& info = classOptions[i];
                std::ostringstream hint;
                hint << info.role
                     << " | PV " << info.maxHp
                     << " | Dégâts " << info.minDamage << "-" << info.maxDamage
                     << " | Critique " << info.criticalDamage
                     << " | Offense " << classOffenseBand(info)
                     << " | Survie " << classSurvivalBand(info)
                     << " | Potions " << info.healingPotionCount << "/" << info.damagePotionCount;

                classScreen.addOption(
                    static_cast<int>(i + 1),
                    info.name,
                    hint.str(),
                    true,
                    "character.class.select",
                    makeCreationItemData(
                        "class",
                        "Création - classe",
                        "select",
                        info.name,
                        info.role,
                        "Classe jouable",
                        "PV " + std::to_string(info.maxHp)
                            + " | Dégâts " + std::to_string(info.minDamage)
                            + "-" + std::to_string(info.maxDamage)
                            + " | Critique " + std::to_string(info.criticalDamage),
                        "Potions " + std::to_string(info.healingPotionCount) + "/" + std::to_string(info.damagePotionCount)
                    )
                );
            }

            int classChoice = TerminalInterface::askMenuChoiceFromOptions(
                classScreen,
                "Veuillez entrer un chiffre correspondant à une classe affichée, ou 0 pour revenir."
            );

            Console::clear();

            if (classChoice == 0)
            {
                break;
            }

            if (classChoice < 1 || static_cast<std::size_t>(classChoice) > classOptions.size())
            {
                continue;
            }

            const ClassOptionInfo& selectedInfo = classOptions[static_cast<std::size_t>(classChoice - 1)];
            bool stayOnClassAction = true;
            while (stayOnClassAction)
            {
                MenuScreen classActionScreen("CLASSE — " + selectedInfo.name, "character.creation.class.action");
                classActionScreen.addSubtitle("Inspecter ne valide rien. Sélectionner grave vraiment la classe.");
                const std::vector<std::string> previewLines = buildClassInspectionLines(selectedInfo);
                for (const std::string& line : previewLines)
                {
                    classActionScreen.addLine(line);
                }
                classActionScreen.addOption(0, "Retour à la liste", "Ne valide pas cette classe.", true, "character.class.action.back");
                classActionScreen.addOption(1, "Inspecter", "Voir clairement ce que la classe possède de base et ce qu'elle pourrait gagner plus tard.", true, "character.class.inspect");
                classActionScreen.addOption(2, "Sélectionner", "Valider cette classe pour le personnage.", true, "character.class.confirm");

                const int actionChoice = TerminalInterface::askMenuChoiceFromOptions(
                    classActionScreen,
                    "Choisis Inspecter, Sélectionner ou Retour."
                );
                Console::clear();

                if (actionChoice == 0)
                {
                    stayOnClassAction = false;
                    continue;
                }

                if (actionChoice == 1)
                {
                    MessageScreen::show(
                        "INSPECTION — " + selectedInfo.name,
                        "character.creation.class.inspect",
                        buildClassInspectionLines(selectedInfo),
                        false
                    );
                    Console::waitForEnter();
                    Console::clear();
                    continue;
                }

                if (actionChoice != 2)
                {
                    continue;
                }

                PlayerClass chosenClass = ClassCatalog::createClassByCategoryChoice(
                    categoryChoice,
                    classChoice
                );

                mainPlayer = Player(playerName, chosenClass);
                mainPlayer.forceTerminalImagePolicy();
                mainPlayer.setRace(selectedRace);
                mainPlayer.setAppearanceProfile(selectedAge, selectedVisualPresentation, selectedVisualVariant);

                bool nativeBonusApplied = SpecialCharacterNativeBonus::applyIfNativeMatch(mainPlayer);

                mainPlayer.initializeStarterInventory(selectedDifficulty);

                Console::clear();

                MenuScreen confirmation("PERSONNAGE GRAVÉ", "character.creation.summary");
                confirmation.setContinueInput("Valide pour entrer dans le jeu avec ce personnage.");
                confirmation.addLine(playerName + ", tu as choisi : " + characterRaceToText(selectedRace) + " / " + chosenClass.getName() + ".");
                confirmation.addLine("Famille : " + ClassCatalog::getClassCategoryNameByChoice(categoryChoice) + ".");
                confirmation.addLine("Apparence : " + mainPlayer.getAppearanceDescription() + ".");
                confirmation.addLine(RaceCatalog::getInnatePassiveLine(selectedRace));
                confirmation.addLine("Difficulté : " + getDifficultyName() + ".");
                confirmation.addLine("Règle de mort : " + getDeathRuleName() + ".");
                confirmation.addLine("Tes statistiques ont été gravées dans l'arène avec succès.");
                confirmation.addLine("Ton équipement et tes ressources de départ ont été adaptés à la difficulté.");
                confirmation.addLine("Créé le " + mainPlayer.getCreatedAtText() + " V" + mainPlayer.getCreatedForVersion());
                confirmation.addLine("Dernière adaptation faite pour la V" + mainPlayer.getLastAdaptedVersion());

                if (nativeBonusApplied)
                {
                    confirmation.addLine("Bonus natif : actif.");
                }

                TerminalInterface::renderMenuScreen(confirmation, false);
                mainPlayer.displayStats();
                mainPlayer.displaySimpleEquipment();

                saveCurrentProgress("Création du personnage");

                Console::waitForEnter();
                Console::clear();
                return;
            }
        }
    }
}

bool Game::isMultiplayerSession() const
{
    return partyPlayers.size() > 1;
}

void Game::savePartyProgress(const std::string& reason) const
{
    if (ephemeralSandboxSession)
    {
        MessageScreen::show(
            "CLONE ÉPHÉMÈRE",
            "save.party.ephemeral_skipped",
            {
                "Session de bac à sable éphémère : aucune progression réelle n’est sauvegardée.",
                "Le personnage histoire original restera intact."
            },
            false
        );
        return;
    }

    if (mainPlayer.isDead() && DifficultyRules::isPermanentDeath(selectedDifficulty, selectedDeathRule))
    {
        SaveManager::savePlayerSnapshot(mainPlayer, accountName, selectedDifficulty, selectedDeathRule);
        if (SaveManager::movePlayableCharacterToDead(accountName, mainPlayer.getName()))
        {
            MessageScreen::show(
                "REGISTRE LÉTHAL",
                "save.party.lethal.main_removed",
                {"Le registre Léthal retire " + mainPlayer.getName() + " des personnages jouables de " + accountName + "."},
                false
            );
        }
        return;
    }

    saveCurrentProgress(reason);

    for (std::size_t i = 0; i < partyPlayers.size(); ++i)
    {
        if (i >= partyAccountNames.size() || i >= partyDifficulties.size())
        {
            continue;
        }

        const Player& partyPlayer = partyPlayers[i];
        const std::string& ownerAccount = partyAccountNames[i];
        DifficultyMode playerDifficulty = partyDifficulties[i];
        DeathRuleMode playerDeathRule = i < partyDeathRules.size()
            ? partyDeathRules[i]
            : DeathRuleRules::defaultForDifficulty(playerDifficulty);

        if (partyPlayer.isDead() && DifficultyRules::isPermanentDeath(playerDifficulty, playerDeathRule))
        {
            SaveManager::savePlayerSnapshot(partyPlayer, ownerAccount, playerDifficulty, playerDeathRule);
            if (SaveManager::movePlayableCharacterToDead(ownerAccount, partyPlayer.getName()))
            {
                MessageScreen::show(
                    "REGISTRE LÉTHAL",
                    "save.party.lethal.member_removed",
                    {"Le registre Léthal retire " + partyPlayer.getName() + " des personnages jouables de " + ownerAccount + "."},
                    false
                );
            }
            else
            {
                MessageScreen::show(
                    "REGISTRE DES MORTS",
                    "save.party.lethal.member_refused",
                    {"Le registre des morts refuse d'emporter " + partyPlayer.getName() + " dans le registre des morts."},
                    false
                );
            }
            continue;
        }

        SaveManager::savePlayerSnapshot(partyPlayer, ownerAccount, playerDifficulty, playerDeathRule);
    }
}

bool Game::addSecondaryPlayerToParty(int playerNumber)
{
    std::vector<AccountSaveSummary> accounts = SaveManager::listAccounts();
    std::vector<AccountSaveSummary> availableAccounts;

    for (const AccountSaveSummary& account : accounts)
    {
        if (account.accountName == accountName)
        {
            continue;
        }

        bool alreadyUsed = false;
        for (const std::string& usedAccount : partyAccountNames)
        {
            if (usedAccount == account.accountName)
            {
                alreadyUsed = true;
                break;
            }
        }

        if (!alreadyUsed)
        {
            availableAccounts.push_back(account);
        }
    }

    if (availableAccounts.empty())
    {
        MenuScreen emptyScreen("JOUEUR " + std::to_string(playerNumber), "session.party.secondary.no_account");
        emptyScreen.addLine("Aucun autre compte local disponible pour le joueur " + std::to_string(playerNumber) + ".");
        emptyScreen.addLine("La coop nécessite des comptes différents, et donc des personnages différents.");
        TerminalInterface::renderMenuScreen(emptyScreen, false);
        Console::waitForEnter();
        Console::clear();
        return false;
    }

    constexpr std::size_t accountsPerPage = 10;
    std::size_t accountPage = 0;
    std::string secondaryAccount;

    while (secondaryAccount.empty())
    {
        const std::size_t totalItems = availableAccounts.size();
        const std::size_t totalPages = PagedMenu::pageCount(totalItems, accountsPerPage);
        const std::size_t first = PagedMenu::firstIndex(accountPage, accountsPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, accountPage, accountsPerPage);

        MenuScreen accountScreen("JOUEUR " + std::to_string(playerNumber), "session.party.secondary.account");
        accountScreen.addSubtitle("Compte du joueur " + std::to_string(playerNumber));
        accountScreen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalItems));

        for (std::size_t i = first; i < last; ++i)
        {
            accountScreen.addOption(
                static_cast<int>(i - first + 1),
                availableAccounts[i].accountName,
                "Compte local disponible pour cette session.",
                true,
                "session.party.account.select"
            );
        }

        PagedMenu::addNavigationOptions(accountScreen, accountPage, totalPages);

        int accountChoice = TerminalInterface::askMenuChoiceFromOptions(
            accountScreen,
            "Choisis un compte affiché."
        );
        Console::clear();

        if (accountChoice == 0)
        {
            return false;
        }

        if (accountChoice == 98 && accountPage > 0)
        {
            --accountPage;
            continue;
        }

        if (accountChoice == 99 && accountPage + 1 < totalPages)
        {
            ++accountPage;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (accountChoice >= 1 && accountChoice <= visibleCount)
        {
            secondaryAccount = availableAccounts[first + static_cast<std::size_t>(accountChoice - 1)].accountName;
        }
    }

    std::vector<CharacterSaveSummary> characters = SaveManager::listPlayableCharacters(secondaryAccount);

    if (characters.empty())
    {
        MenuScreen emptyCharacterScreen("PERSONNAGE JOUEUR " + std::to_string(playerNumber), "session.party.secondary.no_character");
        emptyCharacterScreen.addLine("Ce compte n'a aucun personnage jouable.");
        emptyCharacterScreen.addLine("Compte choisi : " + secondaryAccount + ".");
        TerminalInterface::renderMenuScreen(emptyCharacterScreen, false);
        Console::waitForEnter();
        Console::clear();
        return false;
    }

    constexpr std::size_t charactersPerPage = 8;
    std::size_t characterPage = 0;
    CharacterSaveSummary summary;
    bool characterSelected = false;

    while (!characterSelected)
    {
        const std::size_t totalItems = characters.size();
        const std::size_t totalPages = PagedMenu::pageCount(totalItems, charactersPerPage);
        const std::size_t first = PagedMenu::firstIndex(characterPage, charactersPerPage);
        const std::size_t last = PagedMenu::lastIndexExclusive(totalItems, characterPage, charactersPerPage);

        MenuScreen characterScreen("PERSONNAGE JOUEUR " + std::to_string(playerNumber), "session.party.secondary.character");
        characterScreen.addSubtitle("Compte : " + secondaryAccount);
        characterScreen.addLine("Affichage : " + PagedMenu::rangeText(first, last, totalItems));

        for (std::size_t i = first; i < last; ++i)
        {
            const CharacterSaveSummary& character = characters[i];
            std::string label = character.characterName
                + " | " + character.raceName
                + " / " + character.className
                + " | Niveau " + std::to_string(character.level);

            characterScreen.addOption(
                static_cast<int>(i - first + 1),
                label,
                "Maître : " + character.currentOwnerAccountName,
                true,
                "session.party.character.select"
            );
        }

        PagedMenu::addNavigationOptions(characterScreen, characterPage, totalPages);

        int characterChoice = TerminalInterface::askMenuChoiceFromOptions(
            characterScreen,
            "Choisis un personnage affiché."
        );
        Console::clear();

        if (characterChoice == 0)
        {
            return false;
        }

        if (characterChoice == 98 && characterPage > 0)
        {
            --characterPage;
            continue;
        }

        if (characterChoice == 99 && characterPage + 1 < totalPages)
        {
            ++characterPage;
            continue;
        }

        const int visibleCount = static_cast<int>(last - first);
        if (characterChoice >= 1 && characterChoice <= visibleCount)
        {
            summary = characters[first + static_cast<std::size_t>(characterChoice - 1)];
            characterSelected = true;
        }
    }

    if (summary.currentOwnerAccountName != secondaryAccount || summary.accountName != secondaryAccount)
    {
        MenuScreen refusedScreen("MAÎTRISE REFUSÉE", "session.party.secondary.owner_refused");
        refusedScreen.addLine("Le fil de maîtrise refuse ce chargement.");
        refusedScreen.addLine("Un personnage n'a qu'un seul maître.");
        refusedScreen.addLine("Maître inscrit : " + summary.currentOwnerAccountName);
        refusedScreen.addLine("Compte choisi : " + secondaryAccount);
        TerminalInterface::renderMenuScreen(refusedScreen, false);
        Console::waitForEnter();
        Console::clear();
        return false;
    }

    Player secondaryPlayer;
    DifficultyMode secondaryDifficulty = DifficultyMode::Normal;
    DeathRuleMode secondaryDeathRule = DeathRuleRules::defaultForDifficulty(secondaryDifficulty);

    if (!SaveManager::loadPlayerSnapshot(summary, secondaryPlayer, secondaryDifficulty, secondaryDeathRule))
    {
        MenuScreen errorScreen("CHARGEMENT IMPOSSIBLE", "session.party.secondary.load_failed");
        errorScreen.addLine("Impossible de charger ce personnage.");
        TerminalInterface::renderMenuScreen(errorScreen, false);
        Console::waitForEnter();
        Console::clear();
        return false;
    }

    partyAccountNames.push_back(secondaryAccount);
    partyDifficulties.push_back(secondaryDifficulty);
    partyDeathRules.push_back(secondaryDeathRule);
    partyPlayers.push_back(secondaryPlayer);

    MenuScreen successScreen("JOUEUR AJOUTÉ", "session.party.secondary.added");
    successScreen.addSubtitle("Résumé du joueur secondaire");
    successScreen.addLine("Joueur " + std::to_string(playerNumber) + " ajouté : " + secondaryPlayer.getName() + " (" + secondaryAccount + ").");
    successScreen.addLine("Race / classe : " + secondaryPlayer.getRaceText() + " / " + secondaryPlayer.getType());
    successScreen.addLine("Difficulté personnelle : " + difficultyModeLabel(secondaryDifficulty));
    successScreen.addLine("Règle de mort : " + DeathRuleRules::displayName(secondaryDeathRule));
    successScreen.addLine("PV : " + std::to_string(secondaryPlayer.getHp()) + "/" + std::to_string(secondaryPlayer.getMaxHp()));
    TerminalInterface::renderMenuScreen(successScreen, false);
    Console::waitForEnter();
    Console::clear();
    return true;
}

void Game::configurePartyMode()
{
    partyPlayers.clear();
    partyAccountNames.clear();
    partyDifficulties.clear();
    partyDeathRules.clear();

    MenuScreen screen("SESSION", "session.party.mode");
    screen.addSubtitle("Le joueur 1 reste le point d'ancrage de la partie.");
    screen.addOption(
        1,
        "Solo",
        "Un seul personnage actif.",
        true,
        "session.solo",
        makeSessionItemData(mainPlayer, "session", "Solo", "Un seul personnage actif.", "Classique", "Difficulté : " + getDifficultyName())
    );
    screen.addOption(
        2,
        "Multi local - 2 joueurs",
        "Un allié joueur intervient surtout en combat et récompenses individuelles.",
        true,
        "session.coop.2",
        makeSessionItemData(mainPlayer, "session", "Multi local - 2 joueurs", "Un allié joueur intervient surtout en combat.", "Coop locale", "Récompenses individuelles", true)
    );
    screen.addOption(
        3,
        "Multi local - 3 joueurs",
        "Deux alliés joueurs avec inventaires et récompenses séparés.",
        true,
        "session.coop.3",
        makeSessionItemData(mainPlayer, "session", "Multi local - 3 joueurs", "Deux alliés joueurs rejoignent surtout les combats.", "Coop locale", "Récompenses individuelles", true)
    );

    int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une session affichée.");
    Console::clear();

    if (choice == 1)
    {
        MenuScreen confirmation("SESSION SOLO", "session.party.confirmation.solo");
        confirmation.setContinueInput("Valide pour ouvrir les activités disponibles.");
        confirmation.addSubtitle("Joueur actif");
        confirmation.addLine("Session solo sélectionnée.");
        confirmation.addLine("Personnage : " + mainPlayer.getName());
        confirmation.addLine("Race / classe : " + mainPlayer.getRaceText() + " / " + mainPlayer.getType());
        confirmation.addLine("Difficulté : " + getDifficultyName());
        confirmation.addLine("Règle de mort : " + getDeathRuleName());
        confirmation.addLine("PV : " + std::to_string(mainPlayer.getHp()) + "/" + std::to_string(mainPlayer.getMaxHp()));
        confirmation.addLine("Argent séparé : " + mainPlayer.getInventory().getWalletLine());
        confirmation.addLine("Argent total : " + mainPlayer.getInventory().getWalletTotalLine());
        confirmation.addLine("Prochaine étape : activités disponibles.");
        TerminalInterface::renderMenuScreen(confirmation, false);
        Console::waitForEnter();
        Console::clear();
        return;
    }

    MenuScreen coopIntro("SESSION COOP", "session.party.confirmation.coop");
    coopIntro.setContinueInput("Valide pour choisir les autres joueurs.");
    coopIntro.addSubtitle("Règle de session");
    coopIntro.addLine("Le joueur 1 reste le point d'ancrage : voyage, boss, niveau de session, événements et monstres.");
    coopIntro.addLine("Joueur 1 : " + mainPlayer.getName() + " | " + mainPlayer.getRaceText() + " / " + mainPlayer.getType() + " | " + getDifficultyName() + " | " + getDeathRuleName());
    coopIntro.addLine("Les autres joueurs interviennent surtout en combat, avec leur inventaire et leurs récompenses individuelles.");
    coopIntro.addLine("Boss coop : tous les joueurs doivent avoir l'accès requis.");
    TerminalInterface::renderMenuScreen(coopIntro, false);
    Console::waitForEnter();
    Console::clear();

    for (int playerNumber = 2; playerNumber <= choice; ++playerNumber)
    {
        if (!addSecondaryPlayerToParty(playerNumber))
        {
            MessageScreen::show(
                "SESSION COOP",
                "session.party.partial",
                {"La session repasse sur les joueurs déjà validés."}
            );
            break;
        }
    }

    MenuScreen result("GROUPE", "session.party.result");
    result.setContinueInput("Valide pour ouvrir les activités disponibles.");

    if (partyPlayers.empty())
    {
        result.addSubtitle("Retour solo");
        result.addLine("Aucun joueur secondaire validé. Session solo conservée.");
        result.addLine("Personnage : " + mainPlayer.getName());
        result.addLine("Difficulté : " + getDifficultyName());
        result.addLine("Règle de mort J1 : " + getDeathRuleName());
    }
    else
    {
        result.addSubtitle("Groupe validé");
        result.addLine("Groupe actif : " + std::to_string(partyPlayers.size() + 1) + " joueurs.");
        result.addLine("- J1 " + mainPlayer.getName() + " | " + mainPlayer.getRaceText() + " / " + mainPlayer.getType() + " | " + getDifficultyName() + " | " + getDeathRuleName());
        for (std::size_t i = 0; i < partyPlayers.size(); ++i)
        {
            result.addLine(
                "- J" + std::to_string(i + 2) + " " + partyPlayers[i].getName()
                + " | " + partyPlayers[i].getRaceText()
                + " / " + partyPlayers[i].getType()
                + " | " + difficultyModeLabel(partyDifficulties[i])
                + " | " + (i < partyDeathRules.size() ? DeathRuleRules::displayName(partyDeathRules[i]) : DeathRuleRules::displayName(DeathRuleRules::defaultForDifficulty(partyDifficulties[i])))
            );
        }
    }

    TerminalInterface::renderMenuScreen(result, false);
    Console::waitForEnter();
    Console::clear();
}

