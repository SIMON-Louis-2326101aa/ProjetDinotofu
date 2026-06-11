// EN: HumanCombatTurn.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: HumanCombatTurn.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/turn/HumanCombatTurn.hpp"

#include "combat/CombatActions.hpp"
#include "combat/system/EscapeSystem.hpp"
#include "combat/system/ObservationSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"
#include "combat/summon/SummonCombatSystem.hpp"

#include "core/Console.hpp"

#include "entity/Player.hpp"
#include "entity/Boss.hpp"

#include "interface/menu/CombatMenu.hpp"
#include "interface/menu/EquipmentMenu.hpp"
#include "interface/menu/InventoryMenu.hpp"
#include "interface/menu/CombatPotionMenu.hpp"
#include "interface/menu/CombatRoleMenu.hpp"
#include "interface/menu/CombatGroupTargetMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/progression/BestiaryMenu.hpp"
#include "interface/menu/progression/StatisticsMenu.hpp"

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

namespace
{
    MenuOptionItemData buildAttackTypeItemData(
        const std::string& attackerName,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status,
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "combat_attack";
        itemData.section = "Styles d'attaque";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.owner = attackerName;
        itemData.important = important;
        return itemData;
    }

    MenuOptionItemData buildCombatInterfaceItemData(
        const Entity& entity,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = "Information",
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "combat_interface";
        itemData.section = "Interface de combat";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.owner = entity.getName();
        itemData.progress = "PV : " + std::to_string(entity.getHp()) + "/" + std::to_string(entity.getMaxHp());
        itemData.important = important;
        return itemData;
    }

    MenuScreen buildAttackTypeScreen(const std::string& attackerName)
    {
        MenuScreen screen("TYPE D'ATTAQUE", "combat.attack_type");
        screen.addSubtitle("Action offensive de " + attackerName);
        screen.addLine("Choisis le style d'attaque à exécuter.");
        screen.addBackOption("Retour", "combat.attack.back");
        screen.addOption(
            1,
            "Attaque simple",
            "Frappe fiable avec l'arme équipée.",
            true,
            "combat.attack.simple",
            buildAttackTypeItemData(attackerName, "attack", "Attaque simple", "Frappe fiable avec l'arme équipée.", "Offense stable", true)
        );
        screen.addOption(
            2,
            "Technique d'arme",
            "Utilise le style propre à ton arme actuelle.",
            true,
            "combat.attack.weapon_technique",
            buildAttackTypeItemData(attackerName, "attack", "Technique d'arme", "Action liée au type d'arme équipée.", "Dépend de l'arme")
        );
        screen.addOption(
            3,
            "Attaque lourde",
            "Plus risquée, plus violente.",
            true,
            "combat.attack.heavy",
            buildAttackTypeItemData(attackerName, "attack", "Attaque lourde", "Cherche un gros impact, mais laisse plus d'ouverture.", "Risque augmenté")
        );
        screen.addOption(
            4,
            "Attaque rapide",
            "Moins brutale, mais plus nerveuse.",
            true,
            "combat.attack.quick",
            buildAttackTypeItemData(attackerName, "attack", "Attaque rapide", "Action plus nerveuse, pensée pour garder le rythme.", "Offense rapide")
        );
        screen.addOption(
            5,
            "Compétence de classe",
            "Tente l'action spéciale liée au rôle.",
            true,
            "combat.attack.class_skill",
            buildAttackTypeItemData(attackerName, "attack", "Compétence de classe", "Action spéciale liée à la classe et au cooldown.", "Rôle / classe")
        );
        return screen;
    }



    void showCapturedHumanTurnText(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    )
    {
        if (!lines.empty())
        {
            MessageScreen::show(title, screenId, lines, true);
        }
    }

    bool executeCapturedHumanAction(
        const std::string& title,
        const std::string& screenId,
        const std::function<bool()>& action
    )
    {
        (void)title;
        (void)screenId;
        return action();
    }

    bool executeTrackedHumanAction(
        Entity& attacker,
        const std::string& actionKind,
        const std::function<bool()>& action
    )
    {
        const bool consumedTurn = action();
        if (consumedTurn)
        {
            if (Player* player = dynamic_cast<Player*>(&attacker))
            {
                player->recordChallengeCombatAction(actionKind);
            }
        }
        return consumedTurn;
    }

    int getKnownOrHiddenCursePressure(const Player& player, const std::string& category)
    {
        return player.getCursePressureForCategory(category) + player.getKnownCursePressureForCategory(category);
    }

    void maybeShowInterfaceCursePulse(Entity& attacker, const Entity& defender, Random& random, bool enemySummonsPresent)
    {
        Player* player = dynamic_cast<Player*>(&attacker);
        if (player == nullptr)
        {
            return;
        }

        const int interfacePressure = player->getCursePressureForCategory("interface");
        const int hallucinationPressure = player->getCursePressureForCategory("hallucination");
        const int pressure = interfacePressure + hallucinationPressure;
        if (pressure <= 0)
        {
            return;
        }

        const int chance = std::min(48, 8 + pressure * 4);
        if (random.between(1, 100) > chance)
        {
            return;
        }

        const int knownInterface = player->getKnownCursePressureForCategory("interface");
        const int knownHallucination = player->getKnownCursePressureForCategory("hallucination");
        const int variant = random.between(1, 8);

        std::vector<std::string> lines;
        if (knownInterface > 0 || knownHallucination > 0)
        {
            lines.push_back("Symptôme confirmé : la malédiction perturbe la lecture du combat.");
        }
        else
        {
            lines.push_back("Quelque chose brouille la vision de combat, mais la cause reste inconnue.");
        }

        if (variant == 1)
        {
            lines.push_back("L'interface affiche une option vide, puis la retire avant que tu puisses la choisir.");
            lines.push_back("Trouble de la vision : les contours doublent, puis reviennent avec une demi-seconde de retard.");
        }
        else if (variant == 2)
        {
            lines.push_back("Pendant un instant, le combat ressemble à un PvE contre plusieurs silhouettes.");
            lines.push_back(enemySummonsPresent
                ? "Les invocations réelles et les fausses silhouettes se mélangent avant de redevenir lisibles."
                : "Les silhouettes disparaissent : il n'y avait qu'une seule menace devant toi.");
        }
        else if (variant == 3)
        {
            lines.push_back("La cible prend ton nom pendant un battement de coeur.");
            lines.push_back("Tu as presque l'impression de devoir te battre contre toi-même.");
        }
        else if (variant == 4)
        {
            lines.push_back("Les PV affichés hésitent, reviennent, puis jurent qu'ils n'ont jamais bougé.");
            lines.push_back(defender.getName() + " reste pourtant bien devant toi.");
        }
        else if (variant == 5)
        {
            lines.push_back("Un curseur invisible sélectionne une action que tu n'as pas demandée.");
            lines.push_back("Tu reprends le contrôle, mais l'interface garde une seconde de retard.");
        }
        else if (variant == 6)
        {
            lines.push_back("La ligne de cible se corrompt : [#7A?] CIBLE=" + attacker.getName() + " / CIBLE=" + defender.getName() + " / CIBLE=???");
            lines.push_back("Quelques caractères restent imprimés derrière tes paupières : 0xAN0M4L13::regarde_mieux.");
        }
        else if (variant == 7)
        {
            lines.push_back("Une voix grésille dans l'affichage : \"Tu regardes les chiffres parce que tu n'oses pas regarder la cible.\"");
            lines.push_back("Le menu répond avant toi : \"Mauvais choix. Pas encore choisi, mais mauvais quand même.\"");
        }
        else
        {
            lines.push_back("Les PV, le nom de la cible et le tour actuel se mélangent en caractères cassés : HP? TOI? EUX? [!!]");
            lines.push_back("L'Anomalie provoque : \"Même ton interface hésite à te défendre.\"");
        }

        if (getKnownOrHiddenCursePressure(*player, "interface") >= 5 || getKnownOrHiddenCursePressure(*player, "hallucination") >= 5)
        {
            lines.push_back("Pression élevée : cette perturbation risque de revenir tant que la source n'est pas brisée.");
        }

        MessageScreen::show(
            "VISION BROUILLÉE",
            "combat.interface.curse_pulse",
            lines,
            false
        );
    }
}

bool HumanCombatTurn::play(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int potionHealAmount,
    int potionDamageBonus
)
{
    (void)potionHealAmount;

    maybeShowInterfaceCursePulse(attacker, defender, random, false);

    const MenuScreen turnScreen = CombatMenu::buildTurnScreen(attacker);
    int option = TerminalInterface::askMenuChoiceFromOptions(
        turnScreen,
        "Choix invalide. Entre un chiffre entre 0 et 8."
    );

    Console::clear();

    if (option == 0)
    {
        return openObservationInterface(attacker, defender);
    }

    if (option == 1)
    {
        const MenuScreen attackScreen = buildAttackTypeScreen(attacker.getName());
        int attackChoice = TerminalInterface::askMenuChoiceFromOptions(attackScreen, "Choix invalide.");
        Console::clear();

        if (attackChoice == 0)
        {
            return false;
        }

        if (attackChoice == 1)
        {
            return executeTrackedHumanAction(attacker, "basic_attack", [&]() {
                return executeCapturedHumanAction(
                    "ATTAQUE SIMPLE",
                    "combat.human.attack.simple",
                    [&]() { CombatActions::executeAttack(attacker, defender, random); return true; }
                );
            });
        }

        if (attackChoice == 2)
        {
            return executeTrackedHumanAction(attacker, "skill", [&]() {
                return executeCapturedHumanAction(
                    "TECHNIQUE D'ARME",
                    "combat.human.attack.weapon_technique",
                    [&]() { CombatActions::executeWeaponTechnique(attacker, defender, random); return true; }
                );
            });
        }

        if (attackChoice == 3)
        {
            return executeTrackedHumanAction(attacker, "special_attack", [&]() {
                return executeCapturedHumanAction(
                    "ATTAQUE LOURDE",
                    "combat.human.attack.heavy",
                    [&]() { CombatActions::executeHeavyAttack(attacker, defender, random); return true; }
                );
            });
        }

        if (attackChoice == 4)
        {
            return executeTrackedHumanAction(attacker, "special_attack", [&]() {
                return executeCapturedHumanAction(
                    "ATTAQUE RAPIDE",
                    "combat.human.attack.quick",
                    [&]() { CombatActions::executeQuickAttack(attacker, defender, random); return true; }
                );
            });
        }

        if (attackChoice == 5)
        {
            return executeTrackedHumanAction(attacker, "skill", [&]() {
                return executeCapturedHumanAction(
                    "COMPÉTENCE DE CLASSE",
                    "combat.human.attack.class_skill",
                    [&]() { return CombatActions::executeClassSkill(attacker, defender, random); }
                );
            });
        }
    }

    if (option == 2)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        return executeTrackedHumanAction(attacker, "consumable", [&]() {
            return CombatPotionMenu::openQuickHealing(*player);
        });
    }

    if (option == 3)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        return executeTrackedHumanAction(attacker, "consumable", [&]() {
            return CombatPotionMenu::openAgainstSingleTarget(
                *player,
                defender,
                random,
                potionDamageBonus
            );
        });
    }

    if (option == 4)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        EquipmentMenu::open(*player);
        return false;
    }

    if (option == 5)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        return InventoryMenu::open(*player);
    }

    if (option == 6)
    {
        return executeTrackedHumanAction(attacker, "defense", [&]() {
            DefensePostureSystem::enterDefensePosture(attacker);
            return true;
        });
    }

    if (option == 7)
    {
        MessageScreen::show(
            "TOUR PASSÉ",
            "combat.wait",
            {
                attacker.getName() + " baisse sa garde et passe son tour.",
                "Parfois, attendre le bon moment est déjà une décision."
            },
            false
        );

        if (Player* player = dynamic_cast<Player*>(&attacker))
        {
            player->recordChallengeCombatAction("wait");
        }
        return true;
    }

    if (option == 8)
    {
        return executeTrackedHumanAction(attacker, "escape", [&]() {
            return handleEscape(attacker, defender, random);
        });
    }

    return false;
}


bool HumanCombatTurn::playWithEnemySummons(
    Entity& attacker,
    Entity& defender,
    std::vector<Summon>& enemySummons,
    Random& random,
    int potionHealAmount,
    int potionDamageBonus
)
{
    (void)potionHealAmount;

    maybeShowInterfaceCursePulse(
        attacker,
        defender,
        random,
        SummonCombatSystem::hasTargetableSummons(enemySummons)
    );

    const MenuScreen turnScreen = CombatMenu::buildTurnScreen(attacker);

    if (SummonCombatSystem::hasTargetableSummons(enemySummons))
    {
        MessageScreen::show(
            "INVOCATIONS ADVERSAIRES",
            "combat.enemy_summons.note",
            {
                "L'adversaire possède une ou plusieurs invocations ciblables.",
                "Si tu attaques, tu pourras choisir entre l'invocateur et ses renforts."
            },
            false
        );
    }

    int option = TerminalInterface::askMenuChoiceFromOptions(
        turnScreen,
        "Choix invalide. Entre un chiffre entre 0 et 8."
    );

    Console::clear();

    if (option == 0)
    {
        return inspectCombatTarget(attacker, defender, enemySummons);
    }

    if (option == 1)
    {
        return chooseAndExecuteAttack(attacker, defender, enemySummons, random);
    }

    if (option == 2)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        return executeTrackedHumanAction(attacker, "consumable", [&]() {
            return CombatPotionMenu::openQuickHealing(*player);
        });
    }

    if (option == 3)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        MessageScreen::show(
            "FIOLE INSTABLE",
            "combat.potion.single_target_warning",
            {
                "La fiole cherche la menace principale et ignore les cibles secondaires.",
                "Les invocations restent hors de portée de ce mélange instable."
            },
            false
        );

        return executeTrackedHumanAction(attacker, "consumable", [&]() {
            return CombatPotionMenu::openAgainstSingleTarget(
                *player,
                defender,
                random,
                potionDamageBonus
            );
        });
    }

    if (option == 4)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        EquipmentMenu::open(*player);
        return false;
    }

    if (option == 5)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        return InventoryMenu::open(*player);
    }

    if (option == 6)
    {
        return executeTrackedHumanAction(attacker, "defense", [&]() {
            DefensePostureSystem::enterDefensePosture(attacker);
            return true;
        });
    }

    if (option == 7)
    {
        MessageScreen::show(
            "TOUR PASSÉ",
            "combat.wait",
            {
                attacker.getName() + " baisse sa garde et passe son tour.",
                "Parfois, attendre le bon moment est déjà une décision."
            },
            false
        );

        if (Player* player = dynamic_cast<Player*>(&attacker))
        {
            player->recordChallengeCombatAction("wait");
        }
        return true;
    }

    if (option == 8)
    {
        return executeTrackedHumanAction(attacker, "escape", [&]() {
            return handleEscape(attacker, defender, random);
        });
    }

    return false;
}

bool HumanCombatTurn::openObservationInterface(
    Entity& interfacePlayer,
    Entity& target
)
{
    MenuScreen screen("INTERFACE DE COMBAT", "combat.interface.duel");
    screen.addSubtitle(interfacePlayer.getName() + " face à " + target.getName());
    screen.addBackOption("Retour", "combat.interface.back");
    screen.addOption(1, "Voir l'état du combat", "PV et situation directe.", true, "combat.interface.state", buildCombatInterfaceItemData(interfacePlayer, "inspect", "État du combat", "PV et situation directe.", "Résumé"));
    screen.addOption(2, "Voir mes statistiques", "Ouvre les statistiques du combattant.", true, "combat.interface.stats", buildCombatInterfaceItemData(interfacePlayer, "inspect", "Mes statistiques", "Ouvre les statistiques du combattant.", "Détails joueur"));
    screen.addOption(3, "Résumé équipement", "Affichage simple de l'équipement.", true, "combat.interface.equipment", buildCombatInterfaceItemData(interfacePlayer, "inspect", "Résumé équipement", "Affichage simple de l'équipement.", "Équipement"));
    screen.addOption(4, "Compétences de rôle", "Actions et rappels liés au rôle.", true, "combat.interface.role", buildCombatInterfaceItemData(interfacePlayer, "inspect", "Compétences de rôle", "Actions et rappels liés au rôle.", "Rôle"));
    screen.addOption(5, "Observer / analyser l'adversaire", "Lecture terrain de l'ennemi.", true, "combat.interface.observe", buildCombatInterfaceItemData(target, "inspect", "Observer l'adversaire", "Lecture terrain de l'ennemi.", "Observation", true));
    screen.addOption(6, "Voir l'adversaire dans le bestiaire", "Ouvre l'entrée connue.", true, "combat.interface.bestiary", buildCombatInterfaceItemData(target, "inspect", "Bestiaire adverse", "Ouvre l'entrée connue sans révéler de faiblesse cachée.", "Bestiaire"));
    screen.addOption(7, "Ordres aux alliés", "Indisponible sans allié stable.", true, "combat.interface.allies", buildCombatInterfaceItemData(interfacePlayer, "support", "Ordres aux alliés", "Indisponible sans allié stable.", "Indisponible"));
    screen.addOption(8, "Contrôle des invocations", "Rappel des ordres actuels.", true, "combat.interface.summons", buildCombatInterfaceItemData(interfacePlayer, "support", "Contrôle des invocations", "Rappel des ordres actuels.", "Rappel"));

    int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");

    Console::clear();

    if (choice == 0)
    {
        return false;
    }

    if (choice == 1)
    {
        MessageScreen::show(
            "ÉTAT DU COMBAT",
            "combat.interface.state",
            {
                interfacePlayer.getName() + " : " + std::to_string(interfacePlayer.getHp()) + "/" + std::to_string(interfacePlayer.getMaxHp()) + " PV",
                target.getName() + " : " + std::to_string(target.getHp()) + "/" + std::to_string(target.getMaxHp()) + " PV"
            },
            false
        );
        return false;
    }

    if (choice == 2)
    {
        Player* player = dynamic_cast<Player*>(&interfacePlayer);

        if (player == nullptr)
        {
            showCapturedHumanTurnText(
                "STATISTIQUES",
                "combat.interface.stats.raw_entity",
                Console::captureLines([&]() { interfacePlayer.displayStats(); })
            );
            return false;
        }

        StatisticsMenu::open(*player);
        return false;
    }

    if (choice == 3)
    {
        Player* player = dynamic_cast<Player*>(&interfacePlayer);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        showCapturedHumanTurnText(
            "ÉQUIPEMENT",
            "combat.interface.equipment.summary",
            Console::captureLines([&]() { player->displaySimpleEquipment(); })
        );
        return false;
    }

    if (choice == 4)
    {
        return CombatRoleMenu::open(interfacePlayer);
    }

    if (choice == 5)
    {
        showCapturedHumanTurnText(
            "OBSERVATION",
            "combat.interface.observe.target",
            Console::captureLines([&]() { ObservationSystem::displayTerminalStats(target); })
        );
        return false;
    }

    if (choice == 6)
    {
        BestiaryMenu::displayObjectEntry(target.getName());
        return false;
    }

    if (choice == 7)
    {
        MessageScreen::show(
            "ORDRES AUX ALLIÉS",
            "combat.interface.allies_unavailable",
            {"Aucun allié stable n'attend d'ordre sur ce champ de bataille."},
            false
        );
        return false;
    }

    if (choice == 8)
    {
        MessageScreen::show(
            "CONTRÔLE DES INVOCATIONS",
            "combat.interface.summons_order",
            {
                "Tes invocations suivent l'ordre donné au début du combat.",
                "Changer cet ordre au milieu du chaos demande une ouverture que tu n'as pas encore."
            },
            false
        );
        return false;
    }

    return false;
}


bool HumanCombatTurn::chooseAndExecuteAttack(
    Entity& attacker,
    Entity& defender,
    std::vector<Summon>& enemySummons,
    Random& random
)
{
    return executeTrackedHumanAction(attacker, "basic_attack", [&]() {
        return CombatGroupTargetMenu::openSingleEnemyAttack(
            attacker,
            defender,
            enemySummons,
            random
        );
    });
}

bool HumanCombatTurn::inspectCombatTarget(
    Entity& interfacePlayer,
    Entity& target,
    const std::vector<Summon>& enemySummons
)
{
    MenuScreen screen("INTERFACE DE COMBAT", "combat.interface.summoner_duel");
    screen.addSubtitle(interfacePlayer.getName() + " face à " + target.getName());
    screen.addBackOption("Retour", "combat.interface.back");
    screen.addOption(1, "Voir l'état du combat", "PV, menace principale et invocations ciblables.", true, "combat.interface.state", buildCombatInterfaceItemData(interfacePlayer, "inspect", "État du combat", "PV, menace principale et invocations ciblables.", "Résumé", true));
    screen.addOption(2, "Voir mes statistiques", "Ouvre les statistiques du combattant.", true, "combat.interface.stats", buildCombatInterfaceItemData(interfacePlayer, "inspect", "Mes statistiques", "Ouvre les statistiques du combattant.", "Détails joueur"));
    screen.addOption(3, "Résumé équipement", "Affichage simple de l'équipement.", true, "combat.interface.equipment", buildCombatInterfaceItemData(interfacePlayer, "inspect", "Résumé équipement", "Affichage simple de l'équipement.", "Équipement"));
    screen.addOption(4, "Compétences de rôle", "Actions et rappels liés au rôle.", true, "combat.interface.role", buildCombatInterfaceItemData(interfacePlayer, "inspect", "Compétences de rôle", "Actions et rappels liés au rôle.", "Rôle"));
    screen.addOption(5, "Observer / analyser l'adversaire principal", "Lecture terrain de l'invocateur.", true, "combat.interface.observe_main", buildCombatInterfaceItemData(target, "inspect", "Observer l'adversaire principal", "Lecture terrain de l'invocateur.", "Observation", true));
    screen.addOption(6, "Voir l'adversaire dans le bestiaire", "Ouvre l'entrée connue.", true, "combat.interface.bestiary", buildCombatInterfaceItemData(target, "inspect", "Bestiaire adverse", "Ouvre l'entrée connue sans révéler de faiblesse cachée.", "Bestiaire"));
    screen.addOption(7, "Voir les invocations adverses", "Liste les renforts ciblables.", true, "combat.interface.enemy_summons", buildCombatInterfaceItemData(target, "target", "Invocations adverses", "Liste les renforts ciblables.", "Cibles secondaires"));
    screen.addOption(8, "Ordres aux alliés", "Indisponible sans allié stable.", true, "combat.interface.allies", buildCombatInterfaceItemData(interfacePlayer, "support", "Ordres aux alliés", "Indisponible sans allié stable.", "Indisponible"));
    screen.addOption(9, "Contrôle des invocations", "Rappel des ordres actuels.", true, "combat.interface.summons", buildCombatInterfaceItemData(interfacePlayer, "support", "Contrôle des invocations", "Rappel des ordres actuels.", "Rappel"));

    int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");

    Console::clear();

    if (choice == 0)
    {
        return false;
    }

    if (choice == 1)
    {
        MessageScreen::show(
            "ÉTAT DU COMBAT",
            "combat.interface.state_with_summons",
            {
                interfacePlayer.getName() + " : " + std::to_string(interfacePlayer.getHp()) + "/" + std::to_string(interfacePlayer.getMaxHp()) + " PV",
                target.getName() + " : " + std::to_string(target.getHp()) + "/" + std::to_string(target.getMaxHp()) + " PV",
                SummonCombatSystem::hasTargetableSummons(enemySummons)
                    ? "Des invocations adverses ciblables entourent encore la menace principale."
                    : "Aucune invocation adverse ciblable pour le moment."
            },
            false
        );

        if (SummonCombatSystem::hasTargetableSummons(enemySummons))
        {
            showCapturedHumanTurnText(
                "INVOCATIONS ADVERSAIRES",
                "combat.interface.enemy_summons.list",
                Console::captureLines([&]() { SummonCombatSystem::displayTargetableSummons(enemySummons); })
            );
        }

        return false;
    }

    if (choice == 2)
    {
        Player* player = dynamic_cast<Player*>(&interfacePlayer);

        if (player == nullptr)
        {
            showCapturedHumanTurnText(
                "STATISTIQUES",
                "combat.interface.stats.raw_entity",
                Console::captureLines([&]() { interfacePlayer.displayStats(); })
            );
            return false;
        }

        StatisticsMenu::open(*player);
        return false;
    }

    if (choice == 3)
    {
        Player* player = dynamic_cast<Player*>(&interfacePlayer);

        if (player == nullptr)
        {
            CombatMenu::displayUnavailableOption();
            return false;
        }

        showCapturedHumanTurnText(
            "ÉQUIPEMENT",
            "combat.interface.equipment.summary",
            Console::captureLines([&]() { player->displaySimpleEquipment(); })
        );
        return false;
    }

    if (choice == 4)
    {
        return CombatRoleMenu::open(interfacePlayer);
    }

    if (choice == 5)
    {
        showCapturedHumanTurnText(
            "OBSERVATION",
            "combat.interface.observe.target",
            Console::captureLines([&]() { ObservationSystem::displayTerminalStats(target); })
        );
        return false;
    }

    if (choice == 6)
    {
        BestiaryMenu::displayObjectEntry(target.getName());
        return false;
    }

    if (choice == 7)
    {
        if (SummonCombatSystem::hasTargetableSummons(enemySummons))
        {
            showCapturedHumanTurnText(
                "INVOCATIONS ADVERSAIRES",
                "combat.interface.enemy_summons.list",
                Console::captureLines([&]() { SummonCombatSystem::displayTargetableSummons(enemySummons); })
            );
        }
        else
        {
            MessageScreen::show(
                "INVOCATIONS ADVERSAIRES",
                "combat.interface.enemy_summons_empty",
                {"Aucune invocation adverse ciblable pour le moment."},
                false
            );
        }

        return false;
    }

    if (choice == 8)
    {
        MessageScreen::show(
            "ORDRES AUX ALLIÉS",
            "combat.interface.allies_unavailable",
            {"Aucun allié stable n'attend d'ordre sur ce champ de bataille."},
            false
        );
        return false;
    }

    if (choice == 9)
    {
        MessageScreen::show(
            "CONTRÔLE DES INVOCATIONS",
            "combat.interface.summons_order",
            {
                "Tes invocations suivent l'ordre donné au début du combat.",
                "Changer cet ordre au milieu du chaos demande une ouverture que tu n'as pas encore."
            },
            false
        );
        return false;
    }

    return false;
}

bool HumanCombatTurn::handleEscape(
    Entity& attacker,
    Entity& defender,
    Random& random
)
{
    Boss* targetBoss = dynamic_cast<Boss*>(&defender);

    if (targetBoss != nullptr)
    {
        Player* player = dynamic_cast<Player*>(&attacker);

        if (player != nullptr)
        {
            EscapeSystem::playerAttemptsBossEscape(*player, *targetBoss);
        }
        else
        {
            MessageScreen::show(
                "FUITE IMPOSSIBLE",
                "combat.escape.boss_closed",
                {"L'arène du boss se referme. Aucune sortie ne répond à ton mouvement."},
                false
            );
        }

        return true;
    }

    Player* player = dynamic_cast<Player*>(&attacker);

    if (player == nullptr)
    {
        CombatMenu::displayUnavailableOption();
        return false;
    }

    EscapeSystem::playerAttemptsDuelEscape(*player, defender, random);

    return true;
}
