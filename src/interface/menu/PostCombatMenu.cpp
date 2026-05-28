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
    MenuScreen screen("APRÈS-COMBAT", "post_combat.main");

    screen.addSubtitle(player.getName() + " | Niveau " + std::to_string(player.getLevel()));
    screen.addLine("PV : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()));
    screen.addLine("Or : " + std::to_string(player.getInventory().getGold()) + " pièces");
    screen.addLine("Version de création : " + player.getCreatedForVersion());
    screen.addLine("Dernière adaptation : " + player.getLastAdaptedVersion());

    if (player.isAlteredByCheats())
    {
        screen.addLine("État : données altérées");
    }

    screen.addOption(0, "Continuer", "Retourner au choix du mode de jeu.", true, "post_combat.continue", makePostCombatItemData(player, "continue", "Continuer", "Retourner au choix du mode de jeu."));
    screen.addOption(1, "Ouvrir les boutiques", "Acheter, vendre ou consulter les offres disponibles.", true, "post_combat.shop", makePostCombatItemData(player, "open", "Boutiques", "Acheter, vendre ou consulter les offres disponibles."));
    screen.addOption(2, "Voir mes statistiques", "Résumé du personnage et de la session.", true, "post_combat.statistics", makePostCombatItemData(player, "open", "Statistiques", "Résumé du personnage et de la session."));
    screen.addOption(3, "Améliorer mes attributs", "Cette voie reste scellée pour l'instant.", true, "post_combat.attributes", makePostCombatItemData(player, "open", "Attributs", "Cette voie reste scellée pour l'instant.", "Scellé"));
    screen.addOption(4, "Consulter mes quêtes", "Voir les quêtes actives et terminées.", true, "post_combat.quests", makePostCombatItemData(player, "quest", "Quêtes", "Voir les quêtes actives et terminées."));
    screen.addOption(5, "Lieux visitables", "Accéder aux lieux disponibles entre deux combats.", true, "post_combat.locations", makePostCombatItemData(player, "travel", "Lieux visitables", "Accéder aux lieux disponibles entre deux combats."));
    screen.addOption(6, "PNJ notables", "Parler aux personnages accessibles.", true, "post_combat.npcs", makePostCombatItemData(player, "talk", "PNJ notables", "Parler aux personnages accessibles."));
    screen.addOption(7, "Échange / don entre personnages", "Transférer des ressources autorisées entre comptes.", true, "post_combat.exchange", makePostCombatItemData(player, "open", "Échange / don", "Transférer des ressources autorisées entre comptes."));
    screen.addOption(8, "Voir mes compétences", "Consulter la progression et les techniques connues.", true, "post_combat.skills", makePostCombatItemData(player, "open", "Compétences", "Consulter la progression et les techniques connues."));
    screen.addOption(9, "Voir rapidement mon équipement", "Affichage court de l'équipement actuel.", true, "post_combat.quick_equipment", makePostCombatItemData(player, "inspect", "Équipement rapide", "Affichage court de l'équipement actuel."));
    screen.addOption(10, "Dernier récap de combat", hasLastCombatRecap ? "Relire le dernier bilan avant/après combat." : "Aucun combat récent enregistré dans cette session.", hasLastCombatRecap, "post_combat.last_recap", makePostCombatItemData(player, "inspect", "Dernier récap", "Relire le dernier bilan avant/après combat.", hasLastCombatRecap ? "Disponible" : "Indisponible"));

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
