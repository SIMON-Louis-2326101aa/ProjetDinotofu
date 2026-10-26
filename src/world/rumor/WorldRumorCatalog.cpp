#include "world/rumor/WorldRumorCatalog.hpp"

#include <algorithm>

const std::vector<WorldRumorDefinition>& WorldRumorCatalog::getCatalog()
{
    // These are deliberately rumors, not automatic truths. They give cities, monsters and zones
    // folklore of their own so REAPERS-13 is not the only subject surrounded by stories.
    static const std::vector<WorldRumorDefinition> catalog = {
        {"valebrume_route_lanternes", "city:valebrume", "city", "Certains voyageurs jurent que des lanternes apparaissent parfois sur les chemins secondaires avant les gros orages, mais personne ne s'accorde sur qui les allume.", "valebrume", "Route", 1, 38, WorldRumorSourceType::Traveler},
        {"valebrume_foret_pas", "biome:foret_ancienne", "biome", "Dans la Forêt ancienne, des bûcherons disent avoir suivi des traces qui revenaient exactement sur leurs propres pas, comme si quelque chose imitait leur trajet.", "valebrume", "Forêt ancienne", 4, 34, WorldRumorSourceType::Witness},
        {"rocheveille_mine_siffle", "city:rocheveille", "city", "À Rocheveille, certains mineurs assurent que la Mine sifflante change de tonalité avant un éboulement. D'autres disent simplement que les anciens savent mieux écouter la roche.", "rocheveille", "Mine sifflante", 10, 45, WorldRumorSourceType::Traveler},
        {"rocheveille_os_blancs", "biome:carriere_os_blancs", "monster", "Des carriers parlent de traces trop grandes dans la craie blanche et prétendent qu'un géant dort encore sous les couches les plus anciennes.", "rocheveille", "Carrière des os blancs", 18, 28, WorldRumorSourceType::Tavern},
        {"port_lanterne_marche_double_prix", "city:port_lanterne", "city", "Au Marché sous les ponts, on raconte qu'un objet vendu deux fois au même prix n'a probablement pas le même propriétaire la deuxième fois.", "port_lanterne", "Marché sous les ponts", 10, 42, WorldRumorSourceType::Traveler},
        {"port_lanterne_brume_noms", "biome:canaux_brume_bleue", "biome", "Dans les canaux de brume bleue, des passeurs jurent avoir entendu leur propre nom venir d'une rive vide. Les plus prudents refusent d'y répondre.", "port_lanterne", "Canaux de brume bleue", 14, 31, WorldRumorSourceType::Witness},
        {"lys_givre_cloches_sans_vent", "city:lys_de_givre", "city", "À Lys-de-Givre, une vieille histoire prétend que les cloches fendues sonnent parfois sans vent quand une expédition ne reviendra pas par la route prévue.", "lys_de_givre", "Temple des cloches fendues", 18, 36, WorldRumorSourceType::Traveler},
        {"lys_givre_serments_glace", "biome:glacier_serments", "biome", "Sur le Glacier des Serments froids, certains guides affirment que les promesses prononcées à voix haute restent audibles dans la glace plusieurs jours.", "lys_de_givre", "Glacier des Serments froids", 30, 25, WorldRumorSourceType::Tavern},
        {"statues_pleurent_bougent", "biome:jardin_statues", "monster", "Des visiteurs du Jardin des statues qui pleurent disent que certaines sculptures ne regardent jamais dans la même direction au retour.", "", "Jardin des statues qui pleurent", 14, 30, WorldRumorSourceType::Witness},
        {"fees_mana_politesse", "biome:bosquet_fees", "culture", "Les voyageurs qui reviennent du Bosquet des Fées du Mana prétendent que la politesse y protège parfois mieux qu'une armure. Les survivants impolis racontent évidemment l'inverse.", "", "Bosquet des Fées du Mana", 40, 46, WorldRumorSourceType::Traveler},
        {"lames_muettes_contrat", "faction:lames_muettes", "faction", "Dans certains quartiers, on murmure qu'un contrat des Lames Muettes cesse d'exister dès que sa cible comprend qui l'a commandé. Personne ne sait si c'est une règle ou une menace.", "", "Lames Muettes", 25, 29, WorldRumorSourceType::Tavern},
        {"lames_muettes_deux_lames", "faction:lames_muettes", "faction", "Une vieille rumeur de mercenaires prétend que deux Lames Muettes ne poursuivent jamais la même cible par le même chemin. Certains pensent que c'est une méthode ; d'autres, une superstition de survivants.", "", "", 28, 33, WorldRumorSourceType::Traveler},
        {"guilde_registre_rouge", "faction:guildes_locales", "faction", "Des aventuriers jurent qu'il existe dans certaines guildes un registre rouge réservé aux contrats que personne ne veut afficher au panneau. Les maîtres de guilde répondent généralement qu'un registre rouge est surtout très pratique pour l'encre rouge.", "", "", 12, 31, WorldRumorSourceType::Tavern},
        {"marche_gris_un_seul_prix", "network:marche_gris", "crime", "Chez certains receleurs, on raconte qu'un objet sans provenance possède trois prix : celui pour l'acheter, celui pour oublier qui l'a vendu, et celui pour ne jamais poser la question.", "", "Marché", 10, 41, WorldRumorSourceType::Traveler},
        {"courriers_nocturnes_relais", "network:courriers_nocturnes", "crime", "Des convoyeurs disent que les Courriers nocturnes ne connaissent jamais toute une route : chacun ne connaîtrait que le relais qui lui a remis le paquet et celui qui doit le reprendre.", "", "Route", 14, 37, WorldRumorSourceType::Traveler},
        {"torvald_trois_lecons", "person:torvald", "hero", "À la guilde, certains novices prétendent que Torvald peut repérer trois erreurs de garde avant même le premier échange. Les anciens répondent qu'il en voit surtout une quatrième qu'il ne dit pas tout de suite.", "valebrume", "Guilde", 4, 57, WorldRumorSourceType::Witness},
        {"grinka_taxe_retard", "boss:grinka", "boss", "Des aventuriers racontent que Grinka tient mieux ses comptes que certains marchands et qu'elle se souvient d'une dette plus longtemps qu'une défaite. Ceux qui ont perdu leur bourse trouvent la plaisanterie moins drôle.", "", "", 12, 48, WorldRumorSourceType::Tavern},
        {"chasseurs_prime_sorties", "profession:chasseur_prime", "bounty", "Les vieux chasseurs de primes disent qu'une bonne chasse commence par compter les sorties avant de regarder la cible. Les jeunes trouvent ça excessif jusqu'à leur première poursuite ratée.", "", "Route", 8, 58, WorldRumorSourceType::Witness},
        {"fitoria_chaines_lumiere", "boss:fitoria", "boss", "À propos de Fitoria, certains novices racontent que ses chaînes de lumière apparaissent avant même qu'elle ait décidé de juger quelqu'un. Les survivants plus expérimentés disent surtout qu'ils n'ont pas vu venir le premier mouvement.", "", "", 12, 36, WorldRumorSourceType::Traveler},
        {"zelef_sang_noir", "boss:zelef", "boss", "Des chasseurs revenus d'un affrontement avec Zelef parlent d'un sang trop sombre qui resterait sur l'acier plus longtemps qu'il ne devrait. Les forgerons préfèrent parler de corrosion et de mauvais nettoyage.", "", "", 15, 43, WorldRumorSourceType::Witness},
        {"atlas_dernier_rempart", "boss:atlas", "boss", "On raconte qu'Atlas aurait déjà tenu seul une porte condamnée assez longtemps pour que ceux derrière lui oublient pourquoi ils fuyaient. Personne ne sait quelle partie de l'histoire appartient au protecteur et quelle partie appartient aux bardes.", "", "", 18, 35, WorldRumorSourceType::Tavern},
        {"cieux_legendes_repondent", "biome:cieux_legendes", "myth", "Les rares récits des Cieux des Légendes disent que certains exploits anciens peuvent répondre à ceux qui tentent de les reproduire. Les archives ne confirment presque rien.", "", "Cieux des Légendes", 85, 22, WorldRumorSourceType::Traveler}
    };
    return catalog;
}

const WorldRumorDefinition* WorldRumorCatalog::findById(const std::string& id)
{
    const auto& catalog = getCatalog();
    const auto it = std::find_if(catalog.begin(), catalog.end(), [&](const WorldRumorDefinition& rumor) {
        return rumor.id == id;
    });
    return it == catalog.end() ? nullptr : &(*it);
}

const WorldRumorDefinition* WorldRumorCatalog::pickAmbient(
    const std::string& cityId,
    const std::string& biomeName,
    int playerLevel,
    int salt)
{
    std::vector<const WorldRumorDefinition*> matches;
    for (const WorldRumorDefinition& rumor : getCatalog())
    {
        if (playerLevel < rumor.minLevel) continue;
        const bool cityMatch = rumor.cityId.empty() || rumor.cityId == cityId;
        const bool biomeMatch = rumor.biomeContains.empty() || biomeName.find(rumor.biomeContains) != std::string::npos;
        if (!cityMatch || !biomeMatch) continue;
        matches.push_back(&rumor);
    }
    if (matches.empty()) return nullptr;
    const int index = std::max(0, salt) % static_cast<int>(matches.size());
    return matches[static_cast<std::size_t>(index)];
}
