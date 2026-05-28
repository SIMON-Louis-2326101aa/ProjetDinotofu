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
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "progression/DifficultyRules.hpp"

#include <cctype>
#include <vector>


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

        MessageScreen::show(
            "ÉVÉNEMENT LÉTHAL",
            "cheat.lethal.refusal",
            {
                "Le code n'entre pas.",
                "Il cogne contre quelque chose de plus ancien que le menu.",
                justiceVoice + " : La tentative est notée.",
                anomalyVoice + " : Tu as essayé d'écrire dans une page qui te regarde déjà.",
                moiranVoice + " : Tu as tenté de sortir de ta propre fin. Même les chemins interdits laissent des traces.",
                oberionVoice + " : Une simulation peut tricher. Une vie ne négocie pas son poids.",
                fireVoice + " : Non. Pas en Léthal. Pas avec un vrai personnage.",
                "Malus appliqués :",
                "- PV actuels divisés environ par deux, sans te tuer directement.",
                "- 25% de ton or disparaît.",
                "- 15% de PV maximum sont retenus pendant 3 combats."
            }
        );

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

    MenuScreen screen("AVERTISSEMENT", "cheat.first_alteration.warning");
    screen.addLine("Une règle ancienne vient d'être touchée.");
    screen.addLine("Attention : cela pourrait affecter ton expérience,");
    screen.addLine("tes statistiques, tes souvenirs, et la trace laissée par ce personnage.");
    screen.addLine("Un personnage altéré ne retrouvera jamais un historique parfaitement officiel,");
    screen.addLine("même si toutes ses altérations sont un jour réduites au silence.");
    screen.addLine("Voulez-vous procéder à l'activation ?");
    screen.addOption(1, "Oui, altérer ce personnage", "Assumer l'altération et continuer.", true, "cheat.first_alteration.confirm");
    screen.addOption(2, "Non, revenir en arrière", "Refuser la commande avant qu'elle laisse une trace.", true, "cheat.first_alteration.cancel");

    int choice = TerminalInterface::askMenuChoice(
        screen,
        1,
        2,
        "Veuillez choisir 1 ou 2."
    );
    Console::clear();

    return choice == 1;
}

// EN: displayToggleResult declares or implements a focused behavior used by this module.
// FR: displayToggleResult déclare ou implémente un comportement précis utilisé par ce module.
void CheatManager::displayToggleResult(const std::string& effectText, bool enabled)
{
    MessageScreen::show(
        "ALTÉRATION",
        "cheat.result.toggle",
        {
            "Effet : " + effectText,
            std::string("État : ") + (enabled ? "activé" : "désactivé")
        }
    );
}

// EN: displayInstantResult declares or implements a focused behavior used by this module.
// FR: displayInstantResult déclare ou implémente un comportement précis utilisé par ce module.
void CheatManager::displayInstantResult(const std::string& effectText)
{
    MessageScreen::show(
        "ALTÉRATION",
        "cheat.result.instant",
        {
            "Effet : " + effectText,
            "État : exécuté"
        }
    );
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
    MenuScreen categoryScreen("CHANGEMENT DE CLASSE", "cheat.switch_class.category");
    categoryScreen.addLine("Choisis la nouvelle famille de classe.");

    for (int choice = 1; choice <= ClassCatalog::getClassCategoryCount(); ++choice)
    {
        const std::string categoryName = ClassCatalog::getClassCategoryNameByChoice(choice);
        categoryScreen.addOption(
            choice,
            categoryName,
            std::to_string(ClassCatalog::getPlayableClassCountByCategoryChoice(choice)) + " classe(s) disponible(s).",
            true,
            "cheat.switch_class.category.option"
        );
    }

    int categoryChoice = TerminalInterface::askMenuChoiceFromOptions(
        categoryScreen,
        "Veuillez choisir une famille affichée."
    );

    Console::clear();

    MenuScreen classScreen("NOUVELLE CLASSE", "cheat.switch_class.class");
    classScreen.addSubtitle("Famille : " + ClassCatalog::getClassCategoryNameByChoice(categoryChoice) + ".");
    classScreen.addLine("Choisis la classe qui remplacera l'ancienne base du personnage.");

    const std::vector<ClassOptionInfo> classOptions = ClassCatalog::getClassOptionsByCategoryChoice(categoryChoice);
    for (std::size_t i = 0; i < classOptions.size(); ++i)
    {
        const ClassOptionInfo& info = classOptions[i];
        classScreen.addOption(
            static_cast<int>(i + 1),
            info.name,
            info.role + " | PV " + std::to_string(info.maxHp)
                + " | Dégâts " + std::to_string(info.minDamage)
                + "-" + std::to_string(info.maxDamage),
            true,
            "cheat.switch_class.class.option"
        );
    }

    int classChoice = TerminalInterface::askMenuChoiceFromOptions(
        classScreen,
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
    MenuScreen screen("ALTÉRATIONS CONNUES", "cheat.altered_data.known_list");
    screen.addLine("Statut du personnage : Altéré.");
    screen.addLine("Même désactivées, les altérations restent dans l'histoire du personnage.");

    bool hasSomething = false;

    auto addAlteration = [&](const std::string& code, const std::string& status, const std::string& effect) {
        hasSomething = true;
        screen.addLine(code + " | " + status);
        screen.addLine("Effet : " + effect);
    };

    if (player.isGodModeKnown())
    {
        addAlteration("idontwanttodie", player.isGodModeEnabled() ? "activé" : "désactivé", "bloque les pertes de PV.");
    }

    if (player.isInfiniteConsumablesKnown())
    {
        addAlteration("infinituseforeverything", player.hasInfiniteConsumables() ? "activé" : "désactivé", "empêche les consommables de diminuer.");
    }

    if (player.isIndestructibleEquipmentKnown())
    {
        addAlteration("bedrockequipment", player.hasIndestructibleEquipment() ? "activé" : "désactivé", "empêche l'équipement de perdre de la durabilité.");
    }

    if (player.isEquipmentProtectionKnown())
    {
        addAlteration("myweaponandnotother", player.hasEquipmentProtection() ? "activé" : "désactivé", "protège l'équipement contre perte, vol et destruction.");
    }

    if (player.isStorySkipKnown())
    {
        addAlteration("skipallstory", player.hasStorySkip() ? "activé" : "désactivé", "force l'ouverture d'archives liées à l'histoire et au bestiaire.");
    }

    if (player.getGoldCheatUseCount() > 0)
    {
        addAlteration("givemesomegolds", "utilisé " + std::to_string(player.getGoldCheatUseCount()) + " fois", "donne entre 100 et 1000 pièces d'or.");
    }

    if (player.getLevelCheatUseCount() > 0)
    {
        addAlteration("givemealevel", "utilisé " + std::to_string(player.getLevelCheatUseCount()) + " fois", "donne un niveau.");
    }

    if (player.getMaxLevelCheatUseCount() > 0)
    {
        addAlteration("skipalllevels", "utilisé " + std::to_string(player.getMaxLevelCheatUseCount()) + " fois", "force le niveau 255.");
    }

    if (player.getRefundCheatUseCount() > 0)
    {
        addAlteration("helpmerefundmyaction", "utilisé " + std::to_string(player.getRefundCheatUseCount()) + " fois", "consomme une aide de remboursement.");
    }

    if (player.getResetCheatUseCount() > 0)
    {
        addAlteration("resetmycharacter", "utilisé " + std::to_string(player.getResetCheatUseCount()) + " fois", "reconstruit le personnage.");
    }

    if (player.getSwitchClassCheatUseCount() > 0)
    {
        addAlteration("switchmyclassandweapon", "utilisé " + std::to_string(player.getSwitchClassCheatUseCount()) + " fois", "change la classe et recalcule l'équipement.");
    }

    if (player.isSpecialChallengeAccessKnown())
    {
        addAlteration(
            "fuckgrindinggimifight",
            player.hasSpecialChallengeAccess() ? "accès spécial débloqué" : "accès spécial connu",
            "ouvre la sélection directe des personnages spéciaux et révèle les boss non finaux."
        );
    }

    if (player.isCreatorMessageKnown())
    {
        addAlteration("1302313", "message du créateur consulté", "le créateur sait que tu as fouillé.");
    }

    if (!hasSomething)
    {
        screen.addLine("Aucune altération connue.");
    }

    TerminalInterface::renderMenuScreen(screen, false);
}

void CheatManager::openAlteredDataMenu(Player& player, DifficultyMode difficulty)
{
    bool menuOpen = true;

    while (menuOpen)
    {
        MenuScreen screen("DONNÉES ALTÉRÉES", "cheat.altered_data.main");
        screen.addLine("Ces commandes appartiennent maintenant à la partie altérée du personnage.");
        screen.addLine("Les codes restent tapables proprement, sans brouiller les archives altérées.");
        screen.addOption(0, "Retour", "Revenir à l'après-combat.", true, "cheat.altered_data.back");
        screen.addOption(1, "Voir les altérations connues", "Consulter les marques déjà inscrites sur ce personnage.", true, "cheat.altered_data.known");
        screen.addOption(2, "Entrer une nouvelle commande", "Saisir un code caché déjà assumé par un personnage altéré.", true, "cheat.altered_data.enter_code");

        int choice = TerminalInterface::askMenuChoice(
            screen,
            0,
            2,
            "Veuillez choisir une option affichée."
        );
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
            std::string code = MessageScreen::askText(
                "COMMANDE ALTÉRÉE",
                "cheat.altered_data.enter_code.text",
                {
                    "Entre une commande cachée.",
                    "Le personnage est déjà altéré : l'interface n'a plus besoin de faire semblant de ne rien voir."
                },
                "Commande cachée",
                "Code de triche / commande altérée.",
                true,
                0,
                64
            );
            Console::clear();

            if (!code.empty())
            {
                activateCode(player, difficulty, code);
                Console::waitForEnter();
                Console::clear();
            }
            else
            {
                MessageScreen::show(
                    "COMMANDE IGNORÉE",
                    "cheat.altered_data.enter_code.empty",
                    {"Aucune commande inscrite. Même l'altération a besoin d'un minimum de texte."}
                );
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
            MessageScreen::show(
                "ACTIVATION ANNULÉE",
                "cheat.activation.cancelled.creator",
                {"La commande ne laisse aucune nouvelle trace."}
            );
            return true;
        }

        player.markCreatorMessageSeen();
        MessageScreen::show(
            "MESSAGE DU CRÉATEUR",
            "cheat.creator_message",
            {
                "Tu as trouvé mon code personnel. Maintenant, fais semblant que c'était évident.",
                "Dinotofu te regarde. Moi aussi, un peu."
            }
        );
        return true;
    }

    if (!isKnownCheatCommand(normalizedCode))
    {
        if (displayUnknownMessage)
        {
            MessageScreen::show(
                "COMMANDE INCONNUE",
                "cheat.unknown_command",
                {
                    "Commande inconnue.",
                    "Même les tricheurs doivent écrire correctement."
                }
            );
        }

        return false;
    }

    if (!confirmFirstAlteration(player))
    {
        MessageScreen::show(
            "ACTIVATION ANNULÉE",
            "cheat.activation.cancelled",
            {"La commande ne laisse aucune nouvelle trace."}
        );
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
            MessageScreen::show(
                "ALTÉRATION REFUSÉE",
                "cheat.refund.refused",
                {
                    "Effet refusé : tu as déjà utilisé tes 3 aides de remboursement.",
                    "État : refusé"
                }
            );
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

        MessageScreen::show(
            "ALTÉRATION",
            "cheat.special_challenge.unlock",
            {
                "Effet : l'arène arrête de te faire grinder les rencontres spéciales.",
                "État : accès aux personnages spéciaux débloqué.",
                "État : toutes les variations de boss non finales sont détectées.",
                "Note : FireFlight reste verrouillé. Un vrai boss final ne s'ouvre pas avec un raccourci."
            }
        );
        return true;
    }

    return false;
}
