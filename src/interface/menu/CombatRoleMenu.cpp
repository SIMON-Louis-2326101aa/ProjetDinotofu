// EN: CombatRoleMenu.cpp centralizes combat role screens for terminal and future GUI rendering.
// FR: CombatRoleMenu.cpp centralise les écrans de rôle de combat pour le terminal et la future IG.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Displays active combat role actions such as provocation, threat reduction, and future ally support skills.

#include "interface/menu/CombatRoleMenu.hpp"

#include "combat/role/CombatRoleActionSystem.hpp"
#include "combat/role/CombatRoleSystem.hpp"
#include "core/Console.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <string>


namespace
{
    MenuOptionItemData buildRoleActionData(
        const Entity& entity,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status,
        bool important = false
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "combat_role";
        itemData.section = "Compétences de rôle";
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

MenuScreen CombatRoleMenu::buildScreen(const Entity& entity)
{
    MenuScreen screen("COMPÉTENCES DE RÔLE", "combat.role");
    screen.addSubtitle("Rôle actif de " + entity.getName());
    screen.addBackOption("Retour", "combat.role.back");
    screen.addOption(
        1,
        "Voir mon rôle",
        "Résumé de la menace, du style de combat et du rôle actuel.",
        true,
        "combat.role.identity",
        buildRoleActionData(
            entity,
            "inspect",
            "Voir mon rôle",
            "Résumé de la menace, du style de combat et du rôle actuel.",
            "Information"
        )
    );
    screen.addOption(
        2,
        "Provocation",
        "Attirer l'attention et activer une posture défensive.",
        true,
        "combat.role.provoke",
        buildRoleActionData(
            entity,
            "provoke",
            "Provocation",
            "Attire l'attention et active automatiquement la posture de défense.",
            "Action consommée",
            true
        )
    );
    screen.addOption(
        3,
        "Réduire ma menace",
        CombatRoleSystem::isAssassin(entity) ? "Profil assassin détecté." : "Action verrouillée : demande une vraie approche d'assassin.",
        CombatRoleSystem::isAssassin(entity),
        "combat.role.reduce_threat",
        buildRoleActionData(
            entity,
            "reduce_threat",
            "Réduire ma menace",
            "Efface provocation et menace de soin si le profil le permet.",
            CombatRoleSystem::isAssassin(entity) ? "Disponible" : "Verrouillé"
        )
    );
    screen.addOption(
        4,
        "Protection / soin d'allié",
        "Aucun allié stable à portée dans ce combat.",
        false,
        "combat.role.ally_support",
        buildRoleActionData(
            entity,
            "support",
            "Protection / soin d'allié",
            "Prévu pour les combats de groupe plus stables.",
            "Indisponible"
        )
    );
    return screen;
}

bool CombatRoleMenu::open(Entity& entity)
{
    while (true)
    {
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            buildScreen(entity),
            "Choix invalide. Entre un chiffre entre 0 et 4."
        );

        Console::clear();

        if (choice == 0)
        {
            return false;
        }

        if (choice == 1)
        {
            CombatRoleActionSystem::displayRoleIdentity(entity);
            continue;
        }

        if (choice == 2)
        {
            if (CombatRoleActionSystem::activateManualProvocation(entity, 2))
            {
                return true;
            }

            continue;
        }

        if (choice == 3)
        {
            entity.clearHealingThreat();
            entity.clearProvocation();

            MessageScreen::show(
                "MENACE RÉDUITE",
                "combat.role.reduce_threat.result",
                {
                    entity.getName() + " efface sa présence et réduit sa menace immédiate.",
                    "Les ennemis auront plus de mal à le garder comme priorité.",
                    "Action consommée : oui."
                }
            );

            return true;
        }

        if (choice == 4)
        {
            MessageScreen::show(
                "RÔLE INDISPONIBLE",
                "combat.role.ally_support.locked",
                {
                    "Aucun allié stable ne se trouve dans une position où tu peux le protéger ainsi.",
                    "Le geste est noté, mais personne ne peut en profiter maintenant."
                }
            );
            continue;
        }
    }
}
