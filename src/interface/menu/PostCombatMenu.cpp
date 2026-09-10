// EN: PostCombatMenu.cpp prepares the post-combat screen for both terminal rendering and future GUI reuse.
// FR: PostCombatMenu.cpp prépare l'écran d'après-combat pour le terminal et la future IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/PostCombatMenu.hpp"

#include "interface/TerminalInterface.hpp"

#include <string>

namespace
{
    MenuOptionItemData makePostCombatItemData(
        const Player& player,
        const std::string& actionType,
        const std::string& name,
        const std::string& detail,
        const std::string& status = ""
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "post_combat";
        itemData.section = "Après-combat";
        itemData.actionType = actionType;
        itemData.name = name;
        itemData.detail = detail;
        itemData.status = status;
        itemData.progress = "Niveau " + std::to_string(player.getLevel());
        itemData.owner = player.getName();
        itemData.important = actionType == "save" || actionType == "continue" || status == "Altéré";
        return itemData;
    }
}

MenuScreen PostCombatMenu::buildScreen(const Player& player, bool hasLastCombatRecap)
{
    MenuScreen screen("INTERMÈDE APRÈS SORTIE", "post_combat.intermission");

    screen.addSubtitle(player.getName() + " | Niveau " + std::to_string(player.getLevel()));
    screen.addLine("La poussière retombe quelques secondes avant de reprendre la route.");
    screen.addLine("PV : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()));
    screen.addLine("Argent séparé : " + player.getInventory().getWalletLine());
    screen.addLine("Argent total : " + player.getInventory().getWalletTotalLine());
    screen.addLine("Version de création : " + player.getCreatedForVersion());
    screen.addLine("Dernière adaptation : " + player.getLastAdaptedVersion());

    if (player.isAlteredByCheats())
    {
        screen.addLine("État : données altérées");
    }

    screen.addLine(hasLastCombatRecap ? "Un résumé court de la dernière sortie est affiché par le moteur avant ce menu." : "Aucun bilan récent n'est disponible pour cette session.");
    screen.addLine("Continuer ramène au Menu de voyage. Monde / ville y reste l'activité des lieux visitables.");

    screen.addOption(0, "Continuer", "Retourner au Menu de voyage.", true, "post_combat.continue", makePostCombatItemData(player, "continue", "Continuer", "Retourner au Menu de voyage."));
    screen.addOption(1, "Menu rapide", "Personnage, saisie libre, options de partie et sauvegarde.", true, "post_combat.quick_menu", makePostCombatItemData(player, "open", "Menu rapide", "Hub constant hors combat."));
    screen.addOption(2, "Personnage", "Inventaire, titres, compétences, quêtes acceptées, statistiques, équipe et échange.", true, "post_combat.character", makePostCombatItemData(player, "open", "Personnage", "Accès direct aux données du personnage."));
    screen.addOption(3, "Dernier récap détaillé", hasLastCombatRecap ? "Relire le bilan complet avant/après combat." : "Aucun combat récent enregistré dans cette session.", hasLastCombatRecap, "post_combat.last_recap", makePostCombatItemData(player, "inspect", "Dernier récap", "Relire le bilan complet avant/après combat.", hasLastCombatRecap ? "Disponible" : "Indisponible"));
    screen.addOption(4, "Journal bêta", "Voir où trouver le journal de session à envoyer au dev.", true, "post_combat.beta_log", makePostCombatItemData(player, "inspect", "Journal bêta", "Chemin du fichier de logs local à envoyer en cas de bug ou combat étrange.", "Logs"));

    return screen;
}

void PostCombatMenu::display(const Player& player, bool hasLastCombatRecap)
{
    TerminalInterface::renderMenuScreen(buildScreen(player, hasLastCombatRecap));
}

int PostCombatMenu::getMaxChoice(const Player& player, bool hasLastCombatRecap)
{
    return buildScreen(player, hasLastCombatRecap).getHighestOptionNumber();
}
