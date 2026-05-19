// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Recognizes, confirms and activates hidden cheat alterations on the current character.
// Description : Reconnaît, confirme et active les altérations cachées du personnage actuel.

#include "cheat/CheatManager.hpp"

#include "class_system/ClassCatalog.hpp"
#include "core/Console.hpp"
#include "core/Random.hpp"

#include <cctype>
#include <iostream>

std::string CheatManager::normalizeCode(const std::string& code)
{
    std::string normalized;

    for (unsigned char character : code)
    {
        if (!std::isspace(character))
        {
            normalized += static_cast<char>(std::tolower(character));
        }
    }

    return normalized;
}

bool CheatManager::confirmFirstAlteration(Player& player)
{
    if (player.isAlteredByCheats())
    {
        return true;
    }

    std::cout << "========== AVERTISSEMENT ==========" << std::endl;
    std::cout << "Une règle ancienne vient d'être touchée." << std::endl;
    std::cout << std::endl;
    std::cout << "Attention : cela pourrait affecter ton expérience," << std::endl;
    std::cout << "tes statistiques, tes souvenirs, et la trace laissée par ce personnage." << std::endl;
    std::cout << std::endl;
    std::cout << "Un personnage altéré ne retrouvera jamais un historique parfaitement officiel," << std::endl;
    std::cout << "même si toutes ses altérations sont désactivées plus tard." << std::endl;
    std::cout << std::endl;
    std::cout << "Voulez-vous procéder à l'activation ?" << std::endl;
    std::cout << "1 : Oui, altérer ce personnage" << std::endl;
    std::cout << "2 : Non, revenir en arrière" << std::endl;
    std::cout << "===================================" << std::endl;
    std::cout << "> ";

    int choice = Console::askNumberBetween(1, 2, "Veuillez choisir 1 ou 2.");
    Console::clear();

    return choice == 1;
}

void CheatManager::displayToggleResult(const std::string& effectText, bool enabled)
{
    std::cout << "Effet : " << effectText << std::endl;
    std::cout << "État : " << (enabled ? "activé" : "désactivé") << std::endl;
    std::cout << std::endl;
}

void CheatManager::displayInstantResult(const std::string& effectText)
{
    std::cout << "Effet : " << effectText << std::endl;
    std::cout << "État : exécuté" << std::endl;
    std::cout << std::endl;
}

void CheatManager::resetCharacter(Player& player, DifficultyMode difficulty)
{
    std::string name = player.getName();
    CharacterRace race = player.getRace();
    PlayerClass playerClass = ClassCatalog::createClassByName(player.getType());

    player = Player(name, playerClass);
    player.setRace(race);
    player.initializeStarterInventory(difficulty);
    player.setCheatState(true, false, false, false, false, false, 3);
    player.recordResetCheatUse();
}

void CheatManager::switchClassAndStarterEquipment(Player& player, DifficultyMode difficulty)
{
    std::cout << "Choisis la nouvelle famille de classe." << std::endl;
    std::cout << std::endl;

    ClassCatalog::displayClassCategories();

    std::cout << std::endl;
    std::cout << "> ";

    int categoryChoice = Console::askNumberBetween(
        1,
        ClassCatalog::getClassCategoryCount(),
        "Veuillez choisir une famille affichée."
    );

    Console::clear();

    std::cout << "Choisis la nouvelle classe." << std::endl;
    std::cout << std::endl;

    ClassCatalog::displayClassesByCategoryChoice(categoryChoice);

    std::cout << "> ";

    int classChoice = Console::askNumberBetween(
        1,
        ClassCatalog::getPlayableClassCountByCategoryChoice(categoryChoice),
        "Veuillez choisir une classe affichée."
    );

    PlayerClass newClass = ClassCatalog::createClassByCategoryChoice(categoryChoice, classChoice);
    std::string name = player.getName();
    CharacterRace race = player.getRace();
    int oldLevel = player.getLevel();
    int oldExperience = player.getExperience();
    int oldGold = player.getInventory().getGold();

    player = Player(name, newClass);
    player.setRace(race);
    player.initializeStarterInventory(difficulty);
    player.setLoadedProgress(oldLevel, oldExperience, player.getMaxHp());
    player.getInventory().earnGold(oldGold);
    player.setCheatState(true, false, false, false, false, false, 3);
    player.recordSwitchClassCheatUse();
}

void CheatManager::displayKnownAlterations(const Player& player)
{
    std::cout << "========== ALTÉRATIONS CONNUES ==========" << std::endl;

    bool hasSomething = false;

    if (player.isGodModeKnown())
    {
        hasSomething = true;
        std::cout << "- idontwanttodie | " << (player.isGodModeEnabled() ? "activé" : "désactivé") << std::endl;
        std::cout << "  Effet : bloque les pertes de PV." << std::endl;
    }

    if (player.isInfiniteConsumablesKnown())
    {
        hasSomething = true;
        std::cout << "- infinituseforeverything | " << (player.hasInfiniteConsumables() ? "activé" : "désactivé") << std::endl;
        std::cout << "  Effet : empêche les consommables de diminuer." << std::endl;
    }

    if (player.isIndestructibleEquipmentKnown())
    {
        hasSomething = true;
        std::cout << "- bedrockequipment | " << (player.hasIndestructibleEquipment() ? "activé" : "désactivé") << std::endl;
        std::cout << "  Effet : empêche l'équipement de perdre de la durabilité." << std::endl;
    }

    if (player.isEquipmentProtectionKnown())
    {
        hasSomething = true;
        std::cout << "- myweaponandnotother | " << (player.hasEquipmentProtection() ? "activé" : "désactivé") << std::endl;
        std::cout << "  Effet : protège l'équipement contre perte, vol et destruction." << std::endl;
    }

    if (player.isStorySkipKnown())
    {
        hasSomething = true;
        std::cout << "- skipallstory | " << (player.hasStorySkip() ? "activé" : "désactivé") << std::endl;
        std::cout << "  Effet : prépare le déblocage futur histoire/bestiaire." << std::endl;
    }

    if (player.getGoldCheatUseCount() > 0)
    {
        hasSomething = true;
        std::cout << "- givemesomegolds | utilisé " << player.getGoldCheatUseCount() << " fois" << std::endl;
        std::cout << "  Effet : donne entre 100 et 1000 pièces d'or." << std::endl;
    }

    if (player.getLevelCheatUseCount() > 0)
    {
        hasSomething = true;
        std::cout << "- givemealevel | utilisé " << player.getLevelCheatUseCount() << " fois" << std::endl;
        std::cout << "  Effet : donne un niveau." << std::endl;
    }

    if (player.getMaxLevelCheatUseCount() > 0)
    {
        hasSomething = true;
        std::cout << "- skipalllevels | utilisé " << player.getMaxLevelCheatUseCount() << " fois" << std::endl;
        std::cout << "  Effet : force le niveau 255." << std::endl;
    }

    if (player.getRefundCheatUseCount() > 0)
    {
        hasSomething = true;
        std::cout << "- helpmerefundmyaction | utilisé " << player.getRefundCheatUseCount() << " fois" << std::endl;
        std::cout << "  Effet : consomme une aide de remboursement." << std::endl;
    }

    if (player.getResetCheatUseCount() > 0)
    {
        hasSomething = true;
        std::cout << "- resetmycharacter | utilisé " << player.getResetCheatUseCount() << " fois" << std::endl;
        std::cout << "  Effet : reconstruit le personnage." << std::endl;
    }

    if (player.getSwitchClassCheatUseCount() > 0)
    {
        hasSomething = true;
        std::cout << "- switchmyclassandweapon | utilisé " << player.getSwitchClassCheatUseCount() << " fois" << std::endl;
        std::cout << "  Effet : change la classe et recalcule l'équipement." << std::endl;
    }

    if (player.isCreatorMessageKnown())
    {
        hasSomething = true;
        std::cout << "- 1302313 | message du créateur consulté" << std::endl;
        std::cout << "  Effet : le créateur sait que tu as fouillé." << std::endl;
    }

    if (!hasSomething)
    {
        std::cout << "Aucune altération connue." << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Statut du personnage : Altéré" << std::endl;
    std::cout << "Même désactivées, les altérations restent dans l'histoire du personnage." << std::endl;
    std::cout << "=========================================" << std::endl;
    std::cout << std::endl;
}

void CheatManager::openAlteredDataMenu(Player& player, DifficultyMode difficulty)
{
    bool menuOpen = true;

    while (menuOpen)
    {
        std::cout << "========== DONNÉES ALTÉRÉES ==========" << std::endl;
        std::cout << "0 : Retour" << std::endl;
        std::cout << "1 : Voir les altérations connues" << std::endl;
        std::cout << "2 : Entrer une nouvelle commande" << std::endl;
        std::cout << "======================================" << std::endl;
        std::cout << std::endl;
        std::cout << "> ";

        int choice = Console::askNumberBetween(0, 2, "Veuillez choisir une option affichée.");
        Console::clear();

        if (choice == 0)
        {
            menuOpen = false;
        }
        else if (choice == 1)
        {
            displayKnownAlterations(player);
            Console::waitForEnter();
            Console::clear();
        }
        else if (choice == 2)
        {
            std::cout << "Commande :" << std::endl;
            std::cout << "> ";

            std::string code;
            std::getline(std::cin >> std::ws, code);
            Console::clear();

            if (!code.empty())
            {
                activateCode(player, difficulty, code);
                Console::waitForEnter();
                Console::clear();
            }
        }
    }
}

bool CheatManager::tryActivateHiddenCode(Player& player, DifficultyMode difficulty, const std::string& code)
{
    return activateCode(player, difficulty, code, false);
}

bool CheatManager::activateCode(Player& player, DifficultyMode difficulty, const std::string& code, bool displayUnknownMessage)
{
    std::string normalizedCode = normalizeCode(code);

    if (normalizedCode.empty())
    {
        return false;
    }

    if (normalizedCode == "1302313")
    {
        if (!confirmFirstAlteration(player))
        {
            std::cout << "Activation annulée." << std::endl;
            std::cout << std::endl;
            return true;
        }

        player.markCreatorMessageSeen();
        std::cout << "Message du créateur :" << std::endl;
        std::cout << "Tu as trouvé mon code personnel. Maintenant, fais semblant que c'était évident." << std::endl;
        std::cout << "Dinotofu te regarde. Moi aussi, un peu." << std::endl;
        std::cout << std::endl;
        return true;
    }

    if (normalizedCode != "idontwanttodie" &&
        normalizedCode != "infinituseforeverything" &&
        normalizedCode != "bedrockequipment" &&
        normalizedCode != "myweaponandnotother" &&
        normalizedCode != "skipallstory" &&
        normalizedCode != "skipalllevels" &&
        normalizedCode != "givemesomegolds" &&
        normalizedCode != "givemealevel" &&
        normalizedCode != "helpmerefundmyaction" &&
        normalizedCode != "resetmycharacter" &&
        normalizedCode != "switchmyclassandweapon")
    {
        if (displayUnknownMessage)
        {
            std::cout << "Commande inconnue." << std::endl;
            std::cout << "Même les tricheurs doivent écrire correctement." << std::endl;
            std::cout << std::endl;
        }

        return false;
    }

    if (!confirmFirstAlteration(player))
    {
        std::cout << "Activation annulée." << std::endl;
        std::cout << std::endl;
        return true;
    }

    if (normalizedCode == "idontwanttodie")
    {
        bool enabled = player.toggleGodMode();
        displayToggleResult("les pertes de PV du personnage sont ignorées.", enabled);
        return true;
    }

    if (normalizedCode == "infinituseforeverything")
    {
        bool enabled = player.toggleInfiniteConsumables();
        displayToggleResult("les consommables ne disparaissent plus lors de leur utilisation.", enabled);
        return true;
    }

    if (normalizedCode == "bedrockequipment")
    {
        bool enabled = player.toggleIndestructibleEquipment();
        displayToggleResult("l'équipement porté ne perd plus de durabilité en combat.", enabled);
        return true;
    }

    if (normalizedCode == "myweaponandnotother")
    {
        bool enabled = player.toggleEquipmentProtection();
        displayToggleResult("l'équipement est protégé contre le vol et la destruction de mort.", enabled);
        return true;
    }

    if (normalizedCode == "skipallstory")
    {
        bool enabled = player.toggleStorySkip();
        displayToggleResult("la progression d'histoire/bestiaire est préparée comme débloquée pour les systèmes futurs.", enabled);
        return true;
    }

    if (normalizedCode == "skipalllevels")
    {
        player.recordMaxLevelCheatUse();
        player.forceLevelToMaximum();
        displayInstantResult("niveau maximum atteint. 255. Tous les bits sont à 1.");
        return true;
    }

    if (normalizedCode == "givemesomegolds")
    {
        Random random;
        int gold = random.between(100, 1000);
        player.recordGoldCheatUse();
        player.getInventory().earnGold(gold);
        displayInstantResult("+" + std::to_string(gold) + " pièces d'or.");
        return true;
    }

    if (normalizedCode == "givemealevel")
    {
        player.recordLevelCheatUse();
        player.gainOneLevelByCheat();
        displayInstantResult("+1 niveau.");
        return true;
    }

    if (normalizedCode == "helpmerefundmyaction")
    {
        player.recordRefundCheatUse();

        if (player.consumeRefundUse())
        {
            displayInstantResult("jeton de remboursement consommé. Utilisations restantes : " + std::to_string(player.getRefundUsesRemaining()) + ".");
        }
        else
        {
            std::cout << "Effet refusé : tu as déjà utilisé tes 3 aides de remboursement." << std::endl;
            std::cout << "État : refusé" << std::endl;
            std::cout << std::endl;
        }

        return true;
    }

    if (normalizedCode == "resetmycharacter")
    {
        resetCharacter(player, difficulty);
        displayInstantResult("le personnage a été détruit puis recréé sur sa base actuelle.");
        return true;
    }

    if (normalizedCode == "switchmyclassandweapon")
    {
        switchClassAndStarterEquipment(player, difficulty);
        displayInstantResult("classe changée et équipement de départ recalculé.");
        return true;
    }

    return false;
}
