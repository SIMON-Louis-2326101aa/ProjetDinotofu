#include "entity/Player.hpp"

#include <cassert>

int main()
{
    Player player;

    player.recordCanonicalEvent("clan_hq_state", "built", "QG construit", 1);
    assert(player.hasTitle("On a un QG !"));
    player.recordCanonicalEvent("clan_hq_building_expansions", "level_2", "QG niveau 2", 1);
    assert(player.hasTitle("Ce n'est plus une cabane"));
    player.recordCanonicalEvent("clan_hq_building_expansions", "level_3", "QG niveau 3", 1);
    assert(player.hasTitle("Ça devient sérieux"));

    player.recordCanonicalEvent("logement_principal", "arteria", "Maison avec loyer", 1);
    assert(player.hasTitle("Enfin chez moi"));
    for (int i = 0; i < 3; ++i)
        player.recordCanonicalEvent("loyers_logement_payes", "arteria", "Loyer du logement principal payé", 1);
    assert(player.hasTitle("Le loyer, cette quête hebdomadaire"));
    player.recordCanonicalEvent("repos_maison", "arteria", "Nuit gratuite dans la maison principale", 1);
    assert(player.hasTitle("Mon lit, mes règles"));

    Player npcMarriage;
    npcMarriage.recordCanonicalEvent("mariage_pnj", "spouse:test", "Mariage PNJ validé", 1);
    assert(npcMarriage.hasTitle("J'ai dit oui"));
    assert(npcMarriage.hasTitle("Une place pour deux"));
    assert(npcMarriage.hasTitle("Jusqu'à ce que la sauvegarde nous sépare"));
    assert(!npcMarriage.hasTitle("Jusqu'à ce que le ping nous sépare"));

    Player playerMarriage;
    playerMarriage.recordCanonicalEvent("mariage_joueur", "spouse:player", "Mariage joueur validé", 1);
    assert(playerMarriage.hasTitle("J'ai dit oui"));
    assert(playerMarriage.hasTitle("Jusqu'à ce que le ping nous sépare"));
    assert(!playerMarriage.hasTitle("Jusqu'à ce que la sauvegarde nous sépare"));

    player.recordCanonicalEvent("hestia_arc", "rescue", "Hestia sauvée", 1);
    assert(player.hasTitle("Dossier : retrouvée"));

    player.recordCanonicalEvent("primes_enquetes", "prime:test", "Une piste progresse", 1);
    assert(player.hasTitle("Sur la piste"));
    player.recordCanonicalEvent("primes_capturees_vivantes", "prime:test", "Cible capturée vivante après reddition", 1);
    assert(player.hasTitle("Viens sans faire d'histoire"));
    player.recordCanonicalEvent("primes_evasions_cibles", "prime:test", "Évasion", 1);
    player.recordCanonicalEvent("primes_evasions_cibles", "prime:test", "Évasion", 1);
    player.recordCanonicalEvent("primes_evasions_cibles", "prime:test", "Évasion", 1);
    assert(player.hasTitle("Tu peux courir..."));

    player.recordCanonicalEvent("suites_quete_terminees", "mandatory", "Suite obligatoire", 1);
    assert(player.hasTitle("Évidemment, ce n'était pas fini."));
    player.recordCanonicalEvent("suites_quete_terminees", "optional", "Suite facultative", 1);
    assert(player.hasTitle("Puisqu'on y est..."));

    player.recordCanonicalEvent("contrats_interville_termines", "information", "Transmission", 1);
    assert(player.hasTitle("Facteur officiel"));
    player.recordCanonicalEvent("contrats_interville_termines", "materials", "Matériaux", 1);
    assert(player.hasTitle("Ce n'est pas une lettre, ça."));
    player.recordCanonicalEvent("contrats_interville_relais_multiples", "route:test", "Plusieurs relais", 1);
    assert(player.hasTitle("Avec correspondance"));

    player.recordCanonicalEvent("evenements_ville_participation", "harvest_festival", "Fête des moissons", 1);
    assert(player.hasTitle("J'ai touché de l'herbe"));
    player.recordCanonicalEvent("evenements_ville_participation", "raid_defense_mobilization", "Préparatifs de raid", 1);
    assert(player.hasTitle("On barricade d'abord"));

    player.recordCanonicalEvent("incidents_commerciaux_resolus", "incident:test", "Convoi résolu", 1);
    assert(player.hasTitle("Le convoi est arrivé... à peu près"));
    assert(player.hasTitle("Livreur sous tension"));
    player.recordCanonicalEvent("incidents_commerciaux_primes_publiees", "incident:test", "Prime publiée", 1);
    assert(player.hasTitle("Il manquait quelque chose au chargement"));

    Player outrightOwner;
    outrightOwner.recordCanonicalEvent("logement_principal", "arteria", "Maison achetée définitivement", 1);
    assert(outrightOwner.hasTitle("Enfin chez moi"));
    assert(outrightOwner.hasTitle("Plus de propriétaire"));

    return 0;
}
