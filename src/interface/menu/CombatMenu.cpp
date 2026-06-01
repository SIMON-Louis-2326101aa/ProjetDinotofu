// EN: CombatMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/CombatMenu.hpp"

#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <sstream>
#include <string>

namespace
{
    MenuOptionItemData buildCombatTurnActionData(
        const Entity& entity,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = "Disponible",
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "combat_action";
        itemData.section = "Actions de combat";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.owner = entity.getName();
        itemData.progress = "PV : " + std::to_string(entity.getHp()) + "/" + std::to_string(entity.getMaxHp());
        itemData.important = important;
        return itemData;
    }
}

MenuScreen CombatMenu::buildTurnScreen(const Entity& entity)
{
    MenuScreen screen("COMBAT", "combat.turn");
    screen.addSubtitle("Tour de " + entity.getName());

    if (entity.getClassSkillCooldownTurns() > 0)
    {
        std::ostringstream cooldownText;
        cooldownText << "Compétence de classe : récupération "
                     << entity.getClassSkillCooldownTurns()
                     << " tour(s).";
        screen.addLine(cooldownText.str());
    }

    const bool lowHealth = entity.getMaxHp() > 0 && entity.getHp() * 100 <= entity.getMaxHp() * 35;
    const bool hasHealingTool = entity.getHealingPotionCount() > 0;
    const std::string quickHealActionId = (lowHealth && hasHealingTool)
        ? "combat.quick_heal.recommend_heal.low_hp"
        : "combat.quick_heal";

    screen.addOption(
        1,
        "Attaquer",
        "Attaque simple, technique d'arme, attaque lourde/rapide ou compétence.",
        true,
        "combat.attack",
        buildCombatTurnActionData(
            entity,
            "attack",
            "Attaquer",
            "Ouvre les styles offensifs disponibles pour ce tour.",
            "Action principale",
            true
        )
    );
    screen.addOption(
        2,
        "Potion de soin rapide",
        "Liste seulement les potions de soin utilisables rapidement.",
        true,
        quickHealActionId,
        buildCombatTurnActionData(
            entity,
            "use",
            "Potion de soin rapide",
            "Raccourci vers les potions de soin utilisables immédiatement.",
            hasHealingTool ? "Potion disponible" : "Aucune potion détectée",
            lowHealth && hasHealingTool
        )
    );
    screen.addOption(
        3,
        "Potions",
        "Curatif, défensif, buff, offensive ou debuff.",
        true,
        "combat.potions",
        buildCombatTurnActionData(entity, "use", "Potions", "Ouvre toutes les familles de potions de combat.")
    );
    screen.addOption(
        4,
        "Équipement",
        "Voir ou changer rapidement l'arme et la tenue.",
        true,
        "combat.equipment",
        buildCombatTurnActionData(entity, "equip", "Équipement", "Consulte ou modifie l'équipement sans consommer le tour.")
    );
    screen.addOption(
        5,
        "Inventaire / bestiaire",
        "Consulter les objets, matériaux et informations connues.",
        true,
        "combat.inventory",
        buildCombatTurnActionData(entity, "inspect", "Inventaire / bestiaire", "Consulte les objets et les informations connues sans révéler de faiblesse cachée.")
    );
    screen.addOption(
        6,
        "Posture de défense",
        "Renforce la survie jusqu'au prochain tour.",
        true,
        "combat.defend",
        buildCombatTurnActionData(entity, "defend", "Posture de défense", "Consomme le tour pour améliorer la survie immédiate.", "Action défensive")
    );
    screen.addOption(
        7,
        "Passer son tour",
        "Ne rien faire volontairement.",
        true,
        "combat.wait",
        buildCombatTurnActionData(entity, "wait", "Passer son tour", "Abandonne volontairement l'action de ce tour.", "Action volontaire")
    );
    screen.addOption(
        8,
        "Fuir",
        "Impossible contre un boss, variable contre les autres ennemis.",
        true,
        "combat.flee",
        buildCombatTurnActionData(entity, "flee", "Fuir", "Tente de quitter le combat quand le contexte le permet.", "Risque : tour consommé")
    );
    screen.addOption(
        0,
        "Interface / aide rapide",
        "Rappelle les règles principales du combat.",
        true,
        "combat.help"
    );

    return screen;
}

void CombatMenu::displayTurnMenu(const Entity& entity)
{
    TerminalInterface::renderMenuScreen(buildTurnScreen(entity));
}

// EN: displayUnavailableOption declares or implements a focused behavior used by this module.
// FR: displayUnavailableOption déclare ou implémente un comportement précis utilisé par ce module.
void CombatMenu::displayUnavailableOption()
{
    MessageScreen::show(
        "OPTION INACCESSIBLE",
        "combat.option.unavailable",
        {
            "Cette option est inaccessible dans ce combat.",
            "Choisis une action encore possible pour ce tour."
        }
    );
}
