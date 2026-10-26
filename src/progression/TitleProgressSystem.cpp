#include "progression/TitleProgressSystem.hpp"

#include "entity/Player.hpp"

#include <algorithm>
#include <string>

namespace
{
    bool contains(const std::string& text, const std::string& fragment)
    {
        return text.find(fragment) != std::string::npos;
    }

    int total(const Player& player, const std::string& category)
    {
        return player.getCanonicalJournalCategoryTotal(category);
    }

    int countFor(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key) return record.count;
        }
        return 0;
    }
}

void TitleProgressSystem::onCanonicalEvent(
    Player& player,
    const std::string& category,
    const std::string& key,
    const std::string& label)
{
    // Clan headquarters and artisans.
    if (category == "clan_hq_state" && key == "built") player.grantTitle("On a un QG !");
    if (category == "clan_hq_building_expansions" && key == "level_2") player.grantTitle("Ce n'est plus une cabane");
    if (category == "clan_hq_building_expansions" && key == "level_3") player.grantTitle("Ça devient sérieux");
    if (category == "clan_hq_extra_beds" && key == "bed_5") player.grantTitle("Il reste une chambre ?");
    if (category == "clan_hq_quest_board_upgrades" && key == "level_2") player.grantTitle("Le tableau travaille aussi");
    if (category == "clan_hq_mission_chest_withdrawals" && total(player, category) > 0) player.grantTitle("La caisse du patron");
    if (category == "clan_hq_bar_visits" && total(player, category) > 0) player.grantTitle("Une tournée pour le clan !");
    if (category == "clan_hq_festivals" && total(player, category) > 0) player.grantTitle("On avait dit petite fête");
    if (category == "clan_hq_comfort_decor_upgrades" && key == "level_1") player.grantTitle("Comme à la maison");
    if (category == "clan_hq_forge_workshop_upgrades" && key == "level_1") player.grantTitle("Ça va forger.");
    if (category == "clan_hq_blacksmith_hires" && key == "active") player.grantTitle("Forgeron maison");
    if (category == "clan_hq_blacksmith_milestones" && contains(label, "niveau d'artisan 5")) player.grantTitle("C'est en forgeant...");
    if (category == "clan_hq_blacksmith_specializations" && total(player, category) > 0) player.grantTitle("Chacun sa spécialité");
    if (category == "clan_hq_blacksmith_friend_discounts" && total(player, category) > 0) player.grantTitle("Prix d'ami");

    // Home and marriage.
    if (category == "logement_principal" && contains(label, "Maison")) player.grantTitle("Enfin chez moi");
    if (category == "logement_principal" && contains(label, "achetée définitivement")) player.grantTitle("Plus de propriétaire");
    if (category == "loyers_logement_payes" && total(player, category) >= 3) player.grantTitle("Le loyer, cette quête hebdomadaire");
    if (category == "repos_maison" && total(player, category) > 0) player.grantTitle("Mon lit, mes règles");
    if (category == "mariage_pnj")
    {
        player.grantTitle("J'ai dit oui");
        player.grantTitle("Une place pour deux");
        player.grantTitle("Jusqu'à ce que la sauvegarde nous sépare");
    }
    if (category == "mariage_joueur")
    {
        player.grantTitle("J'ai dit oui");
        player.grantTitle("Jusqu'à ce que le ping nous sépare");
    }

    // Hestia. Titles after the rescue are intentionally not all ultimate prerequisites:
    // permanent-death runs can lose an AI companion for good before later relationship milestones.
    if (category == "hestia_arc" && key == "rescue") player.grantTitle("Dossier : retrouvée");
    if (category == "hestia_residence" && key == "player_home") player.grantTitle("Une colocataire de plus");
    if (category == "hestia_companion_training" && total(player, category) >= 5) player.grantTitle("Elle apprend... doucement");
    if (category == "hestia_artefact" && key == "granted") player.grantTitle("Un gant un peu trop attachant");
    if (category == "hestia_artefact" && key == "second_form") player.grantTitle("Deux formes valent mieux qu'une");
    if (category == "hestia_arc" && key == "sacrifice") player.grantTitle("Elle ne m'a pas laissé mourir");
    if (category == "hestia_visites" && total(player, category) >= 5 && player.getHestiaStoryState().trust >= 25)
        player.grantTitle("C'était ma première vraie amie");

    // Targeted bounties and persistent pursuit.
    if (category == "primes_enquetes" && total(player, category) > 0) player.grantTitle("Sur la piste");
    if (category == "primes_capturees_vivantes" && contains(label, "après reddition")) player.grantTitle("Viens sans faire d'histoire");
    if (category == "primes_capturees_vivantes" && contains(label, "après confrontation")) player.grantTitle("Il fallait vraiment en arriver là ?");
    if (category == "primes_capture_apres_relocalisation" && total(player, category) > 0) player.grantTitle("Pas assez loin");
    if (category == "primes_evasions_cibles" && countFor(player, category, key) >= 3) player.grantTitle("Tu peux courir...");
    if (category == "primes_pistes_interville_exploitees" && total(player, category) > 0) player.grantTitle("J'avais gardé la piste");

    // Quest follow-ups and intercity work.
    if (category == "suites_quete_terminees" && key == "mandatory") player.grantTitle("Évidemment, ce n'était pas fini.");
    if (category == "suites_quete_terminees" && key == "optional") player.grantTitle("Puisqu'on y est...");
    if (category == "contrats_interville_termines" && key == "information") player.grantTitle("Facteur officiel");
    if (category == "contrats_interville_termines" && key == "materials") player.grantTitle("Ce n'est pas une lettre, ça.");
    if (category == "contrats_interville_relais_multiples" && total(player, category) > 0) player.grantTitle("Avec correspondance");

    // Living-city and trade incidents.
    if (category == "evenements_ville_participation" && key == "harvest_festival") player.grantTitle("J'ai touché de l'herbe");
    if (category == "evenements_ville_participation" && key == "raid_defense_mobilization") player.grantTitle("On barricade d'abord");
    if (category == "incidents_commerciaux_resolus" && total(player, category) > 0)
    {
        player.grantTitle("Le convoi est arrivé... à peu près");
        player.grantTitle("Livreur sous tension");
    }
    if (category == "incidents_commerciaux_primes_publiees" && total(player, category) > 0)
        player.grantTitle("Il manquait quelque chose au chargement");
    if (category == "tensions_economiques_aidees" && total(player, category) > 0)
        player.grantTitle("Commis de crise");
}
