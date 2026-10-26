// EN: CheatManager.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CheatManager.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Recognizes, confirms and activates hidden cheat alterations on the current character.
// Description : Reconnaît, confirme et active les altérations cachées du personnage actuel.

#include "cheat/CheatManager.hpp"
#include "cheat/SystemCallManager.hpp"

#include "boss/BossCatalog.hpp"
#include "class_system/ClassCatalog.hpp"
#include "core/Console.hpp"
#include "core/Random.hpp"
#include "economy/Money.hpp"
#include "economy/GuildFinanceSystem.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "combat/death/LethalSpiritResistanceSystem.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/model/MenuScreen.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "progression/blessing/Blessing.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/DeathRuleRules.hpp"
#include "save/SaveManager.hpp"

#include <algorithm>
#include <cctype>
#include <vector>


namespace
{
    // EN: isKnownCheatCommand declares or implements a focused behavior used by this module.
    // FR: isKnownCheatCommand déclare ou implémente un comportement précis utilisé par ce module.
    bool isKnownCheatCommand(const std::string& normalizedCode)
    {
        return normalizedCode == "1302313" ||
            normalizedCode == "dinotofu:13-02313" ||
            normalizedCode == "13023131302313" ||
            normalizedCode == "iamnotgoblinfood" ||
            normalizedCode == "letmecook" ||
            normalizedCode == "whereismyplotarmor" ||
            normalizedCode == "talktothedev" ||
            normalizedCode == "oopsallbosses" ||
            normalizedCode == "randomizemyfate" ||
            normalizedCode == "whatdebt" ||
            normalizedCode == "thatsalotofpower" ||
            normalizedCode == "icanfeelit" ||
            normalizedCode == "bornforit" ||
            normalizedCode == "nottoday" ||
            normalizedCode == "fearmeinstead" ||
            normalizedCode == "trustmebro" ||
            normalizedCode == "fillmypocket" ||
            normalizedCode == "justanotherday" ||
            normalizedCode == "luckytofu" ||
            normalizedCode == "onemorechance" ||
            normalizedCode == "everybodylovesme" ||
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

    bool isAlwaysNonAlteringSecret(const std::string& normalizedCode)
    {
        return normalizedCode == "1302313" || normalizedCode == "talktothedev";
    }

    bool isAlteringCheatCommand(const std::string& normalizedCode)
    {
        return isKnownCheatCommand(normalizedCode)
            && !isAlwaysNonAlteringSecret(normalizedCode)
            && normalizedCode != "dinotofu:13-02313";
    }

    std::string lowerText(std::string text)
    {
        std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return text;
    }

    int canonicalCount(const Player& player, const std::string& category, const std::string& key)
    {
        int total = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key) total += record.count;
        }
        return total;
    }

    bool protocol13AlreadyUsed(const Player& player)
    {
        return canonicalCount(player, "protocoles_secrets", "reapers13_protocol_13_signal") > 0
            || canonicalCount(player, "protocol_usage", "reapers13_protocol_13_once") > 0;
    }

    bool hasCredibleReaperKnowledge(const Player& player)
    {
        if (player.getCanonicalJournalCategoryTotal("reapers13_knowledge") > 0) return true;

        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == "protocoles_secrets" || record.category == "cheat_code_usage") continue;
            const std::string combined = lowerText(record.category + " " + record.key + " " + record.label);
            if (combined.find("reaper") != std::string::npos) return true;
        }

        for (const PlayerHistoricalEvent& event : player.getHistoricalEvents())
        {
            if (event.category == "reapers13_protocol_signal") continue;
            const std::string combined = lowerText(event.category + " " + event.subjectId + " " + event.label);
            if (combined.find("reaper") != std::string::npos) return true;
        }
        return false;
    }

    void recordCheatUsage(Player& player, const std::string& normalizedCode)
    {
        player.recordCanonicalEvent("cheat_code_usage", normalizedCode, normalizedCode, 1);
    }

    void recordSecretUsage(Player& player, const std::string& normalizedCode)
    {
        player.recordCanonicalEvent("secret_code_usage", normalizedCode, normalizedCode, 1);
    }

    void grantAccountCheatTrophy(Player& player)
    {
        if (!player.getCurrentOwnerAccountName().empty())
        {
            SaveManager::mergeAccountUnlockedTitles(player.getCurrentOwnerAccountName(), {"On ne dira rien..."});
        }
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
            "ÉVÉNEMENT DE MORT DÉFINITIVE",
            "cheat.lethal.refusal",
            {
                "Le code n'entre pas.",
                "Il cogne contre quelque chose de plus ancien que le menu.",
                justiceVoice + " : La tentative est notée.",
                anomalyVoice + " : Tu as essayé d'écrire dans une page qui te regarde déjà.",
                moiranVoice + " : Tu as tenté de sortir de ta propre fin. Même les chemins interdits laissent des traces.",
                oberionVoice + " : Une simulation peut tricher. Une vie ne négocie pas son poids.",
                fireVoice + " : Non. Pas avec une mort définitive. Pas avec un vrai personnage.",
                "Malus appliqués :",
                "- PV actuels divisés environ par deux, sans te tuer directement.",
                "- 25% de ton or disparaît.",
                "- 15% de PV maximum sont retenus pendant 3 combats."
            }
        );

        recordLethalCheatVoice("Avatar de Lexior", "Note brouillée : une voix de justice a réagi à une tentative de triche en mort définitive.");
        recordLethalCheatVoice("L'Anomalie", "Note brouillée : une anomalie a ri devant une tentative de triche en mort définitive.");
        recordLethalCheatVoice("Manifestation de Moiran", "Note brouillée : le Destin a reconnu une tentative de sortie de ligne en mort définitive.");
        recordLethalCheatVoice("Écho fragmenté d'Obérion", "Note brouillée : une autorité primordiale a refusé de mélanger simulation et vie réelle.");
        recordLethalCheatVoice("Avatar affaibli de FireFlight", "Note brouillée : le créateur limité a bloqué une commande interdite en mort définitive.");
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
    screen.addLine("Les nouveaux titres normaux ne pourront plus enrichir ce personnage ni les trophées du compte.");
    screen.addLine("Le registre accordera seulement la marque spéciale : On ne dira rien...");
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
    const std::string creatorAccount = player.getCreatorAccountName();
    const std::string ownerAccount = player.getCurrentOwnerAccountName();
    PlayerClass playerClass = ClassCatalog::createClassByName(player.getType());

    player = Player(name, playerClass);
    player.setRace(race);
    player.setOwnershipMetadata(creatorAccount, ownerAccount);
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
    const std::string creatorAccount = player.getCreatorAccountName();
    const std::string ownerAccount = player.getCurrentOwnerAccountName();
    int oldLevel = player.getLevel();
    int oldExperience = player.getExperience();
    const long long oldCopper = player.getInventory().getTotalCopper();

    player = Player(name, newClass);
    player.setRace(race);
    player.setOwnershipMetadata(creatorAccount, ownerAccount);
    player.initializeStarterInventory(difficulty);
    player.setLoadedProgress(oldLevel, oldExperience, player.getMaxHp());
    player.getInventory().setTotalCopper(oldCopper);
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
        addAlteration("givemesomegolds", "utilisé " + std::to_string(player.getGoldCheatUseCount()) + " fois", "donne une bourse aléatoire de 100 à 1000 PF de valeur.");
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

    const std::vector<std::string> genericTrackedCodes = {
        "thatsalotofpower", "icanfeelit", "bornforit", "nottoday", "fearmeinstead",
        "trustmebro", "fillmypocket", "justanotherday", "luckytofu", "onemorechance",
        "whatdebt", "everybodylovesme", "iamnotgoblinfood", "letmecook",
        "whereismyplotarmor", "oopsallbosses", "randomizemyfate", "13023131302313"
    };
    for (const std::string& tracked : genericTrackedCodes)
    {
        const int uses = canonicalCount(player, "cheat_code_usage", tracked);
        if (uses <= 0) continue;
        addAlteration(tracked, "utilisé " + std::to_string(uses) + " fois", "altération officielle enregistrée dans l'historique canonique.");
    }

    if (!hasSomething)
    {
        screen.addLine("Aucune altération connue.");
    }

    screen.setDisplayOnlyInput("État des altérations affiché sans saisie directe.");
    TerminalInterface::renderMenuScreen(screen, false);
}

void CheatManager::openAlteredDataMenu(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule)
{
    bool menuOpen = true;

    while (menuOpen)
    {
        MenuScreen screen("DONNÉES ALTÉRÉES", "cheat.altered_data.main");
        screen.addLine("Ces commandes appartiennent maintenant à la partie altérée du personnage.");
        screen.addLine("Les commandes déjà assumées restent tapables proprement, sans brouiller les archives altérées.");
        screen.addOption(0, "Retour", "Revenir à l'après-combat.", true, "cheat.altered_data.back");
        screen.addOption(1, "Voir les altérations connues", "Consulter les marques déjà inscrites sur ce personnage.", true, "cheat.altered_data.known");
        screen.addOption(2, "Entrer une nouvelle commande", "Saisir une commande déjà assumée par un personnage altéré.", true, "cheat.altered_data.enter_code");

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
                    "Entre une commande.",
                    "Le personnage est déjà altéré : l'interface traite cette saisie comme une donnée assumée."
                },
                "Commande",
                "Choix, texte ou commande.",
                true,
                0,
                64
            );
            Console::clear();

            if (!code.empty())
            {
                activateCode(player, difficulty, deathRule, code);
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
bool CheatManager::tryActivateHiddenCode(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule, const std::string& code)
{
    return activateCode(player, difficulty, deathRule, code, false);
}

// EN: activateCode declares or implements a focused behavior used by this module.
// FR: activateCode déclare ou implémente un comportement précis utilisé par ce module.
bool CheatManager::activateCode(Player& player, DifficultyMode difficulty, DeathRuleMode deathRule, const std::string& code, bool displayUnknownMessage)
{
    const std::string normalizedCode = normalizeCode(code);

    if (normalizedCode.empty())
    {
        return false;
    }

    if (normalizedCode == "dinotofu:13-02313" && protocol13AlreadyUsed(player))
    {
        const bool channelSilent = (player.getCurrentWorldDayNumber() % 2) == 0;
        MessageScreen::show(
            "PROTOCOL 13",
            "cheat.reapers13.protocol13.already_used",
            {
                channelSilent
                    ? "Aucune réponse. Le canal REAPERS-13 n'est plus joignable pour cette balise."
                    : "Protocol 13 déjà transmis : cette identification a déjà été remplie pour ce personnage.",
                "Une seconde balise ne sera pas envoyée."
            }
        );
        return true;
    }

    const bool protocol13NeedsAlteration = normalizedCode == "dinotofu:13-02313"
        && !hasCredibleReaperKnowledge(player);

    if (DifficultyRules::isPermanentDeath(difficulty, deathRule)
        && (isAlteringCheatCommand(normalizedCode) || protocol13NeedsAlteration))
    {
        if (normalizedCode == "dinotofu:13-02313" && protocol13NeedsAlteration)
        {
            player.recordCanonicalEvent(
                "protocol_usage",
                "reapers13_protocol_13_once",
                "Tentative unique d'ouverture du Protocol 13",
                1
            );
            player.recordHistoricalEvent(
                "reapers13_protocol_blocked_lethal",
                "REAPERS-13",
                "Le personnage a tenté d'ouvrir le canal 13-02313 sans connaissance préalable en mort définitive ; la tentative unique a été consommée avant transmission.",
                true
            );
        }
        triggerLethalCheatEvent(player);
        return true;
    }

    if (normalizedCode == "system_call")
    {
        recordSecretUsage(player, normalizedCode);
        SystemCallManager::open(player, difficulty, deathRule);
        return true;
    }

    if (normalizedCode == "1302313")
    {
        player.markCreatorMessageSeen();
        recordSecretUsage(player, normalizedCode);
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

    if (normalizedCode == "talktothedev")
    {
        recordSecretUsage(player, normalizedCode);
        MessageScreen::show(
            "NOTE DU CRÉATEUR",
            "cheat.talk_to_dev",
            {
                "Salut.",
                "Oui, cette commande existe vraiment.",
                "Non, elle ne donne rien. Tu voulais parler au dev, pas braquer le registre."
            }
        );
        return true;
    }

    if (normalizedCode == "dinotofu:13-02313")
    {
        player.recordCanonicalEvent(
            "protocol_usage",
            "reapers13_protocol_13_once",
            "Utilisation unique du Protocol 13",
            1
        );

        if (protocol13NeedsAlteration)
        {
            if (!confirmFirstAlteration(player))
            {
                MessageScreen::show(
                    "PROTOCOL 13",
                    "cheat.reapers13.protocol13.cancelled",
                    {
                        "Le personnage n'a encore aucune raison crédible de connaître les Reapers.",
                        "La transmission a été annulée avant de laisser une trace."
                    }
                );
                return true;
            }

            const bool firstAlteration = !player.isAlteredByCheats();
            if (firstAlteration && !player.getCurrentOwnerAccountName().empty())
            {
                SaveManager::mergeAccountUnlockedTitles(player.getCurrentOwnerAccountName(), player.getTitles());
            }
            player.markAsAlteredByCheats();
            if (firstAlteration) grantAccountCheatTrophy(player);
            recordCheatUsage(player, normalizedCode);
            player.recordHistoricalEvent(
                "reapers13_protocol_without_knowledge",
                "REAPERS-13",
                "Le personnage a utilisé le code 13-02313 sans aucune connaissance préalable crédible des Reapers.",
                true
            );
        }
        else
        {
            recordSecretUsage(player, normalizedCode);
        }

        player.recordCanonicalEvent(
            "protocoles_secrets",
            "reapers13_protocol_13_signal",
            "Canal interdit REAPERS-13 — Protocol 13",
            1
        );
        player.recordHistoricalEvent(
            "reapers13_protocol_signal",
            "REAPERS-13",
            "Le personnage a transmis le code d'identification 13-02313 sur un canal interdit des Reapers.",
            true
        );
        if (!protocol13NeedsAlteration)
        {
            player.grantTitle("Canal interdit");
        }

        MessageScreen::show(
            "PROTOCOL 13",
            "cheat.reapers13.protocol13",
            {
                "Canal interdit ouvert.",
                "Code d'identification 13-02313 reconnu.",
                "Balise REAPERS-13 transmise.",
                protocol13NeedsAlteration
                    ? "Aucune trace antérieure des Reapers n'expliquait cette connaissance : le personnage est désormais Altéré."
                    : "Une trace crédible des Reapers existait déjà : le registre accorde le bénéfice du doute.",
                "Cette balise est utilisable une seule fois par personnage.",
                "Le signal n'offre ni recrutement, ni victoire, ni apparition gratuite : le monde décidera de la réponse."
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

    const bool firstAlteration = !player.isAlteredByCheats();
    if (firstAlteration && !player.getCurrentOwnerAccountName().empty())
    {
        // Freeze every legitimate title earned before the first alteration.
        SaveManager::mergeAccountUnlockedTitles(player.getCurrentOwnerAccountName(), player.getTitles());
    }

    player.markAsAlteredByCheats();
    if (firstAlteration) grantAccountCheatTrophy(player);
    recordCheatUsage(player, normalizedCode);

    if (normalizedCode == "13023131302313")
    {
        player.heal(player.getMaxHp());
        player.restoreManaToFull();
        player.recordCanonicalEvent("cheat_effects", "divine_sequence_13023131302313", "Séquence divine complète déclenchée", 1);
        player.recordCanonicalEvent("cheat_effects", "divine_sequence_pending_execution", "Exécution divine armée sur la prochaine cible hostile ordinaire", 1);
        MessageScreen::show(
            "SÉQUENCE DIVINE",
            "cheat.divine_sequence",
            {
                "La séquence 13-02313-1302313 traverse le registre.",
                "Le personnage est entièrement soigné et son mana est restauré.",
                "L'exécution divine est armée : la prochaine cible hostile ordinaire réellement attaquée sera abattue.",
                "Les boss restent soumis à leurs mécaniques propres : cette exécution ne contourne pas une rencontre spéciale."
            }
        );
        return true;
    }

    if (normalizedCode == "iamnotgoblinfood")
    {
        player.recordCanonicalEvent("cheat_effects", "goblin_fear", "Les gobelins reconnaissent une présence anormalement terrifiante", 1);
        displayInstantResult("les gobelins sensibles à la morale peuvent paniquer plus tôt face à ce personnage.");
        return true;
    }

    if (normalizedCode == "letmecook")
    {
        player.recordCanonicalEvent("cheat_effects", "letmecook_critical", "Chance critique artificiellement renforcée pour la journée", 1);
        displayInstantResult("la fenêtre de critique est améliorée jusqu'au prochain changement de jour du monde.");
        return true;
    }

    if (normalizedCode == "whereismyplotarmor")
    {
        const bool added = player.addBlessing(Blessing(
            "cheat_plot_armor",
            "Armure scénaristique douteuse",
            "Une protection artificielle peut repousser une mort définitive, avec les vraies conséquences du système de bénédiction.",
            true
        ));
        if (added)
        {
            displayInstantResult("une bénédiction de survie léthale artificielle a été ajoutée.");
        }
        else
        {
            MessageScreen::show(
                "ALTÉRATION",
                "cheat.plot_armor.unavailable",
                {"La protection n'a pas pu être ajoutée : elle existe déjà ou le personnage ne peut plus recevoir de bénédiction."}
            );
        }
        return true;
    }

    if (normalizedCode == "oopsallbosses")
    {
        player.unlockBossRegistryExceptFinal(BossCatalog::getMaximumBossId(), -1);
        player.recordCanonicalEvent("cheat_effects", "oops_all_bosses", "Tous les boss actuels sont révélés par chaos", 1);
        displayInstantResult("le registre révèle tous les boss actuels, y compris les rencontres normalement tardives.");
        return true;
    }

    if (normalizedCode == "randomizemyfate")
    {
        Random random;
        const int categoryChoice = random.between(1, std::max(1, ClassCatalog::getClassCategoryCount()));
        const int classCount = std::max(1, ClassCatalog::getPlayableClassCountByCategoryChoice(categoryChoice));
        const int classChoice = random.between(1, classCount);
        PlayerClass newClass = ClassCatalog::createClassByCategoryChoice(categoryChoice, classChoice);

        const std::string name = player.getName();
        const CharacterRace race = player.getRace();
        const std::string creatorAccount = player.getCreatorAccountName();
        const std::string ownerAccount = player.getCurrentOwnerAccountName();
        const int oldLevel = player.getLevel();

        player = Player(name, newClass);
        player.setRace(race);
        player.setOwnershipMetadata(creatorAccount, ownerAccount);
        player.initializeStarterInventory(difficulty);
        player.setLoadedProgress(oldLevel, 0, player.getMaxHp());
        player.setCheatState(true, false, false, false, false, false, false, 3);
        recordCheatUsage(player, normalizedCode);
        player.recordCanonicalEvent("cheat_effects", "randomized_fate", "Classe et inventaire de départ réattribués aléatoirement", 1);
        grantAccountCheatTrophy(player);
        displayInstantResult("le Destin a réattribué aléatoirement la classe et l'inventaire de départ tout en conservant le niveau courant.");
        return true;
    }


    if (normalizedCode == "thatsalotofpower")
    {
        player.scaleCombatStats(125, 125);
        player.recordCanonicalEvent("cheat_effects", "thatsalotofpower", "Puissance de combat existante amplifiée de 25%", 1);
        displayInstantResult("les capacités déjà présentes gagnent une forte amplification de puissance sans éveiller de mana ni offrir de maîtrise martiale gratuite.");
        return true;
    }

    if (normalizedCode == "icanfeelit")
    {
        player.recordCanonicalEvent("mana_awakening", "cheat_awakening", "Éveil artificiel du mana par icanfeelit", 1);
        player.restoreManaToFull();
        displayInstantResult("le registre considère le mana comme artificiellement éveillé pour les futurs contrôles d'accès magique.");
        return true;
    }

    if (normalizedCode == "bornforit")
    {
        player.unlockActiveSkill("inner_mantra", "Mantra intérieur", true);
        player.unlockPassiveSkill("inner_mantra_mastery", "Maîtrise du mantra intérieur", true);
        player.enablePassiveSkill("inner_mantra_mastery");
        player.equipActiveSkill("inner_mantra");
        player.recordCanonicalEvent("cheat_effects", "bornforit", "Maîtrise martiale artificielle accordée", 1);
        displayInstantResult("une base martiale avancée est accordée artificiellement et le Mantra intérieur est débloqué.");
        return true;
    }

    if (normalizedCode == "nottoday")
    {
        player.heal(player.getMaxHp());
        player.restoreManaToFull();
        player.cureBurning();
        player.curePoison();
        player.cureFrost();
        player.cureShock();
        player.cureBleeding();
        player.cureWeakening();
        player.cureVulnerability();
        player.recordCanonicalEvent("cheat_effects", "nottoday", "Restauration complète artificielle", 1);
        displayInstantResult("PV et mana restaurés ; les principaux états de combat négatifs sont retirés.");
        return true;
    }

    if (normalizedCode == "fearmeinstead")
    {
        player.recordCanonicalEvent("cheat_fear_resistance", "enabled", "Résistance artificielle à la peur et à la panique", 1);
        player.unlockPassiveSkill("steady_guard", "Garde stable", true);
        player.enablePassiveSkill("steady_guard");
        displayInstantResult("la peur et la panique futures pourront reconnaître cette résistance artificielle ; Garde stable est aussi débloquée.");
        return true;
    }

    if (normalizedCode == "trustmebro")
    {
        player.recordCanonicalEvent("cheat_trustmebro", "enabled", "Loyauté artificiellement poussée au maximum", 1);
        displayInstantResult("les systèmes de loyauté d'équipe reconnaissent désormais une confiance artificiellement maximale.");
        return true;
    }

    if (normalizedCode == "fillmypocket")
    {
        Random random;
        const int monsterA = random.between(1, 5), monsterB = random.between(1, 5);
        const int oreA = random.between(1, 5), oreB = random.between(1, 5);
        const int plantA = random.between(1, 5), plantB = random.between(1, 5);
        const int arcaneA = random.between(1, 5), arcaneB = random.between(1, 5);
        player.getInventory().addMaterial(MaterialCatalog::createGoblinEar(monsterA));
        player.getInventory().addMaterial(MaterialCatalog::createWolfFang(monsterB));
        player.getInventory().addMaterial(MaterialCatalog::createRustedMetalFragment(oreA));
        player.getInventory().addMaterial(MaterialCatalog::createRunicIronShard(oreB));
        player.getInventory().addMaterial(MaterialCatalog::createMountainBlueFlower(plantA));
        player.getInventory().addMaterial(MaterialCatalog::createBitterHealingLeaf(plantB));
        player.getInventory().addMaterial(MaterialCatalog::createArcaneDust(arcaneA));
        player.getInventory().addMaterial(MaterialCatalog::createMinorFireRuneNote(arcaneB));
        player.recordCanonicalEvent("cheat_effects", "fillmypocket", "Lots aléatoires de matériaux générés", 1);
        displayInstantResult("1 à 5 unités de plusieurs matériaux de grandes catégories ont été ajoutées ; aucun objet unique de quête/boss n'est généré.");
        return true;
    }

    if (normalizedCode == "justanotherday")
    {
        player.advanceWorldDays(1);
        player.recordCanonicalEvent("cheat_effects", "justanotherday", "Une journée complète avancée artificiellement", 1);
        displayInstantResult("un jour complet du monde s'est écoulé avec les conséquences normales liées au temps.");
        return true;
    }

    if (normalizedCode == "luckytofu")
    {
        player.unlockPassiveSkill("halfling_lucky_step", "Pas chanceux", true);
        player.enablePassiveSkill("halfling_lucky_step");
        player.recordCanonicalEvent("cheat_luck", "enabled", "Chance artificielle luckytofu", 1);
        displayInstantResult("Pas chanceux est débloqué et les futurs systèmes de chance peuvent reconnaître l'altération luckytofu.");
        return true;
    }

    if (normalizedCode == "onemorechance")
    {
        player.recordCanonicalEvent("cheat_spirit_rearm", "rearmed", "Résistance spirituelle réarmée artificiellement", 1);
        displayInstantResult("une résistance spirituelle létale supplémentaire est réarmée. Elle sera consommée lors d'une prochaine tentative réelle.");
        return true;
    }

    if (normalizedCode == "everybodylovesme")
    {
        player.recordCanonicalEvent("reputation_locale_positive", "cheat_everybodylovesme", "Réputation artificiellement élevée", 100);
        player.recordCanonicalEvent("cheat_effects", "everybodylovesme", "Réputation artificielle poussée très haut", 1);
        displayInstantResult("la réputation locale actuelle reçoit une poussée artificielle majeure.");
        return true;
    }

    if (normalizedCode == "whatdebt")
    {
        const GuildDebtRepaymentResult cleared = GuildFinanceSystem::clearAllDebtsByCheat(player);
        player.recordCanonicalEvent("cheat_effects", "whatdebt_clear", "Dettes de guilde effacées artificiellement", 1);
        MessageScreen::show(
            "ALTÉRATION",
            "cheat.whatdebt",
            {
                "Effet : les dettes de guilde enregistrées ont été effacées artificiellement.",
                "Découvert effacé : " + Money::formatCopper(cleared.overdraftPaidCopper) + ".",
                "Prêt effacé : " + Money::formatCopper(cleared.loanPaidCopper) + ".",
                "Aucune pièce n'a été prélevée : cette opération reste une triche et ne compte jamais comme un remboursement honorable."
            }
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
        int economyUnits = random.between(100, 1000);
        player.recordGoldCheatUse();
        player.getInventory().earnEconomyUnits(economyUnits);
        player.refreshCurrencyTitles();
        displayInstantResult("+" + Money::formatEconomyUnits(economyUnits) + ".");
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
        recordCheatUsage(player, normalizedCode);
        grantAccountCheatTrophy(player);
        displayInstantResult("le personnage a été détruit puis recréé sur sa base actuelle.");
        return true;
    }

    if (normalizedCode == "switchmyclassandweapon")
    {
        switchClassAndStarterEquipment(player, difficulty);
        recordCheatUsage(player, normalizedCode);
        grantAccountCheatTrophy(player);
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
                "Note : FireFlight reste verrouillé. Certains tests ne s'ouvrent pas avec un raccourci."
            }
        );
        return true;
    }

    return false;
}
