// EN: PostCombatMenu.cpp prepares the post-combat screen for both terminal rendering and future GUI reuse.
// FR: PostCombatMenu.cpp prépare l'écran d'après-combat pour le terminal et la future IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/PostCombatMenu.hpp"

#include "interface/TerminalInterface.hpp"

MenuScreen PostCombatMenu::buildScreen(const Player& player)
{
    MenuScreen screen("APRÈS-COMBAT", "post_combat.main");

    screen.addOption(0, "Continuer", "Retourner au choix du mode de jeu.", true, "post_combat.continue");
    screen.addOption(1, "Ouvrir les boutiques", "Acheter, vendre ou consulter les offres disponibles.", true, "post_combat.shop");
    screen.addOption(2, "Voir mes statistiques", "Résumé du personnage et de la session.", true, "post_combat.statistics");
    screen.addOption(3, "Ouvrir l'inventaire / équipement / potions", "Gérer objets, équipement et consommables hors combat.", true, "post_combat.inventory");
    screen.addOption(4, "Améliorer mes attributs", "Cette voie reste scellée pour l'instant.", true, "post_combat.attributes");
    screen.addOption(5, "Sauvegarde rapide", "Sauvegarder sans quitter.", true, "post_combat.quick_save");
    screen.addOption(6, "Sauvegarder et quitter", "Sauvegarder puis fermer la session.", true, "post_combat.save_quit");
    screen.addOption(7, "Consulter mes quêtes", "Voir les quêtes actives et terminées.", true, "post_combat.quests");
    screen.addOption(8, "Lieux visitables", "Accéder aux lieux disponibles entre deux combats.", true, "post_combat.locations");
    screen.addOption(9, "PNJ notables", "Parler aux personnages accessibles.", true, "post_combat.npcs");
    screen.addOption(10, "Échange / don entre personnages", "Transférer des ressources autorisées entre comptes.", true, "post_combat.exchange");
    screen.addOption(11, "Voir mes compétences", "Consulter la progression et les techniques connues.", true, "post_combat.skills");
    screen.addOption(12, "Voir rapidement mon équipement", "Affichage court de l'équipement actuel.", true, "post_combat.quick_equipment");

    if (player.isAlteredByCheats())
    {
        screen.addOption(13, "Données altérées", "Voir les altérations connues de ce personnage.", true, "post_combat.altered_data");
    }

    return screen;
}

void PostCombatMenu::display(const Player& player)
{
    TerminalInterface::renderMenuScreen(buildScreen(player));
}

int PostCombatMenu::getMaxChoice(const Player& player)
{
    return buildScreen(player).getHighestOptionNumber();
}
