// EN: CheatManager.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CheatManager.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Recognizes, confirms and activates hidden cheat alterations on the current character.
// Description : Reconnaît, confirme et active les altérations cachées du personnage actuel.

#include "cheat/CheatManager.hpp"

#include "boss/BossCatalog.hpp"
#include "class_system/ClassCatalog.hpp"
#include "core/Console.hpp"
#include "core/Random.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "progression/DifficultyRules.hpp"

#include <cctype>
#include <iostream>


namespace
{
    // EN: isKnownCheatCommand declares or implements a focused behavior used by this module.
    // FR: isKnownCheatCommand déclare ou implémente un comportement précis utilisé par ce module.
    bool isKnownCheatCommand(const std::string& normalizedCode)
    {
        return normalizedCode == "1302313" ||
            normalizedCode == "idontwanttodie" ||
            normalizedCode == "infinituseforeverything" ||
            normalizedCode == "bedrockequipment" ||
            normalizedCode == "myweaponandnotother" ||
            normalizedCode == "skipallstory" ||
            normalizedCode == "skipalllevels" ||
            normalizedCode == "givemesomegolds" ||
            normalizedCode == "givemealevel" ||
            normalizedCode == "helpmerefundmyaction" ||
            normalizedCode == "resetmycharacter" ||
            normalizedCode == "switchmyclassandweapon" ||
            normalizedCode == "fuckgrindinggimifight";
    }

    std::string knownOrUnknownVoice(const Player& player, int bossId, const std::string& knownName, const std::string& unknownLabel)
    {
        return player.isBossUnlocked(bossId) ? knownName : unknownLabel;
    }

    // EN: recordLethalCheatVoice declares or implements a focused behavior used by this module.
    // FR: recordLethalCheatVoice déclare ou implémente un comportement précis utilisé par ce module.
    void recordLethalCheatVoice(const std::string& entityName, const std::string& note)
    {
        BestiaryRuntimeProgress::recordEncounter(
            entityName,
            "Divinités / lore",
            note
        );
    }

    // EN: triggerLethalCheatEvent declares or implements a focused behavior used by this module.
    // FR: triggerLethalCheatEvent déclare ou implémente un comportement précis utilisé par ce module.
    void triggerLethalCheatEvent(Player& player)
    {
        std::string justiceVoice = knownOrUnknownVoice(player, 16, "Avatar de Lexior", "Voix inconnue 1");
        std::string anomalyVoice = knownOrUnknownVoice(player, 11, "L'Anomalie", "Voix inconnue 2");
        std::string moiranVoice = knownOrUnknownVoice(player, 30, "Manifestation de Moiran", "Voix inconnue 3");
        std::string oberionVoice = knownOrUnknownVoice(player, 26, "Écho fragmenté d'Obérion", "Voix inconnue 4");
        std::string fireVoice = knownOrUnknownVoice(player, 27, "Avatar affaibli de FireFlight", "Voix inconnue 5");

        player.applyLethalCheatAttemptPenalty();

        std::cout << "========== ÉVÉNEMENT LÉTHAL ==========" << std::endl;
        std::cout << "Le code n'entre pas." << std::endl;
        std::cout << "Il cogne contre quelque chose de plus ancien que le menu." << std::endl;
        std::cout << std::endl;
        std::cout << justiceVoice << " : La tentative est notée." << std::endl;
        std::cout << anomalyVoice << " : Tu as essayé d'écrire dans une page qui te regarde déjà." << std::endl;
        std::cout << moiranVoice << " : Tu as tenté de sortir de ta propre fin. Même les chemins interdits laissent des traces." << std::endl;
        std::cout << oberionVoice << " : Une simulation peut tricher. Une vie ne négocie pas son poids." << std::endl;
        std::cout << fireVoice << " : Non. Pas en Léthal. Pas avec un vrai personnage." << std::endl;
        std::cout << std::endl;
        std::cout << "Malus appliqués :" << std::endl;
        std::cout << "- PV actuels divisés environ par deux, sans te tuer directement." << std::endl;
        std::cout << "- 25% de ton or disparaît." << std::endl;
        std::cout << "- 15% de PV maximum sont retenus pendant 3 combats." << std::endl;
        std::cout << "======================================" << std::endl;
        std::cout << std::endl;

        recordLethalCheatVoice("Avatar de Lexior", "Note brouillée : une voix de justice a réagi à une tentative de triche en mode Léthal.");
        recordLethalCheatVoice("L'Anomalie", "Note brouillée : une anomalie a ri devant une tentative de triche en mode Léthal.");
        recordLethalCheatVoice("Manifestation de Moiran", "Note brouillée : le Destin a reconnu une tentative de sortie de ligne en mode Léthal.");
        recordLethalCheatVoice("Écho fragmenté d'Obérion", "Note brouillée : une autorité primordiale a refusé de mélanger simulation et vie réelle.");
        recordLethalCheatVoice("Avatar affaibli de FireFlight", "Note brouillée : le créateur limité a bloqué une commande interdite en mode Léthal.");
    }
}

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

// EN: confirmFirstAlteration declares or implements a focused behavior used by this module.
// FR: confirmFirstAlteration déclare ou implémente un comportement précis utilisé par ce module.
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
    std::cout << "même si toutes ses altérations sont un jour réduites au silence." << std::endl;
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

// EN: displayToggleResult declares or implements a focused behavior used by this module.
// FR: displayToggleResult déclare ou implémente un comportement précis utilisé par ce module.
void CheatManager::displayToggleResult(const std::string& effectText, bool enabled)
{
    std::cout << "Effet : " << effectText << std::endl;
    std::cout << "État : " << (enabled ? "activé" : "désactivé") << std::endl;
    std::cout << std::endl;
}

// EN: displayInstantResult declares or implements a focused behavior used by this module.
// FR: displayInstantResult déclare ou implémente un comportement précis utilisé par ce module.
void CheatManager::displayInstantResult(const std::string& effectText)
{
    std::cout << "Effet : " << effectText << std::endl;
    std::cout << "État : exécuté" << std::endl;
    std::cout << std::endl;
}

// EN: resetCharacter declares or implements a focused behavior used by this module.
// FR: resetCharacter déclare ou implémente un comportement précis utilisé par ce module.
void CheatManager::resetCharacter(Player& player, DifficultyMode difficulty)
{
    std::string name = player.getName();
    CharacterRace race = player.getRace();
    PlayerClass playerClass = ClassCatalog::createClassByName(player.getType());

    player = Player(name, playerClass);
    player.setRace(race);
    player.initializeStarterInventory(difficulty);
    player.setCheatState(true, false, false, false, false, false, false, 3);
    player.recordResetCheatUse();
}

// EN: switchClassAndStarterEquipment declares or implements a focused behavior used by this module.
// FR: switchClassAndStarterEquipment déclare ou implémente un comportement précis utilisé par ce module.
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
    player.setCheatState(true, false, false, false, false, false, false, 3);
    player.recordSwitchClassCheatUse();
}

// EN: displayKnownAlterations declares or implements a focused behavior used by this module.
// FR: displayKnownAlterations déclare ou implémente un comportement précis utilisé par ce module.
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
        std::cout << "  Effet : force l'ouverture d'archives liées à l'histoire et au bestiaire." << std::endl;
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

    if (player.isSpecialChallengeAccessKnown())
    {
        hasSomething = true;
        std::cout << "- fuckgrindinggimifight | accès spécial "
                  << (player.hasSpecialChallengeAccess() ? "débloqué" : "connu") << std::endl;
        std::cout << "  Effet : ouvre la sélection directe des personnages spéciaux et révèle les boss non finaux." << std::endl;
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

// EN: openAlteredDataMenu declares or implements a focused behavior used by this module.
// FR: openAlteredDataMenu déclare ou implémente un comportement précis utilisé par ce module.
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

// EN: tryActivateHiddenCode declares or implements a focused behavior used by this module.
// FR: tryActivateHiddenCode déclare ou implémente un comportement précis utilisé par ce module.
bool CheatManager::tryActivateHiddenCode(Player& player, DifficultyMode difficulty, const std::string& code)
{
    return activateCode(player, difficulty, code, false);
}

// EN: activateCode declares or implements a focused behavior used by this module.
// FR: activateCode déclare ou implémente un comportement précis utilisé par ce module.
bool CheatManager::activateCode(Player& player, DifficultyMode difficulty, const std::string& code, bool displayUnknownMessage)
{
    std::string normalizedCode = normalizeCode(code);

    if (normalizedCode.empty())
    {
        return false;
    }

    if (DifficultyRules::isPermanentDeath(difficulty) && isKnownCheatCommand(normalizedCode))
    {
        triggerLethalCheatEvent(player);
        return true;
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

    if (!isKnownCheatCommand(normalizedCode))
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
        displayToggleResult("les archives d'histoire et de bestiaire s'ouvrent de force autour de toi.", enabled);
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

    if (normalizedCode == "fuckgrindinggimifight")
    {
        player.unlockSpecialChallengeAccess();
        player.unlockBossRegistryExceptFinal(BossCatalog::getMaximumBossId(), 27);

        std::cout << "Effet : l'arène arrête de te faire grinder les rencontres spéciales." << std::endl;
        std::cout << "État : accès aux personnages spéciaux débloqué." << std::endl;
        std::cout << "État : toutes les variations de boss non finales sont détectées." << std::endl;
        std::cout << "Note : FireFlight reste verrouillé. Un vrai boss final ne s'ouvre pas avec un raccourci." << std::endl;
        std::cout << std::endl;
        return true;
    }

    return false;
}
