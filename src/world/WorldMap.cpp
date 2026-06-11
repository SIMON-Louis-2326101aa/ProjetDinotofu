// EN: Stable exploration map catalogue for future biome screens and current terminal previews.
// FR: Catalogue stable de carte d'exploration pour les futurs écrans de biomes et les aperçus terminal.
#include "world/WorldMap.hpp"

#include <algorithm>

const std::vector<WorldMapPlace>& WorldMap::getPlaces()
{
    static const std::vector<WorldMapPlace> places = {
        {"plain_gate_path", "Chemin de la grande porte", "Plaine sauvage", "Route juste après les remparts : faible danger, bons repères, herbes simples.", "prairie, remparts au loin, vent léger", 1, true, false, false},
        {"merchant_stone_marks", "Bornes des marchands", "Route commerciale", "Suite de bornes, traces de roues et points d'embuscade faciles à lire.", "route pavée, poussière de convois", 2, true, false, true},
        {"old_forest_edge", "Lisière ancienne", "Forêt ancienne", "Entrée de forêt où les racines gardent des marques de passage.", "sous-bois vert sombre", 4, true, false, true},
        {"slime_marsh_pools", "Flaques des slimes", "Mares gélatineuses", "Petites mares vivantes, utiles pour matériaux de slime mais salissantes.", "flaques colorées, brume basse", 3, true, false, true},
        {"lantern_bocage_core", "Bosquet aux lanternes", "Bocage aux lanternes", "Champignons lumineux, spores et pistes presque invisibles sans préparation.", "nuit bleue, lumières de champignons", 8, false, false, true},
        {"red_clay_first_dunes", "Premières dunes rouges", "Désert d'argile rouge", "Argile craquelée, chaleur sèche et illusions de distance.", "dunes rouges, mirages légers", 10, false, false, true},
        {"whistling_mine_lift", "Ascenseur de la Mine sifflante", "Mine sifflante", "Rails vibrants, cages métalliques et vieux échos de pioche.", "galerie minière, lampes chaudes", 5, true, false, true},
        {"cold_mountain_pass", "Col de la montagne froide", "Montagne froide", "Chemin exposé aux rafales, bon pour minerais et rencontres dangereuses.", "rochers froids, ciel blanc", 7, true, false, false},
        {"blue_mist_canals", "Canaux de brume bleue", "Canaux de brume bleue", "Canaux humides autour de Port-Lanterne, pleins de rumeurs et d'objets perdus.", "ponts bas, eau bleutée", 8, true, false, true},
        {"frost_oath_gate", "Porte du Glacier des Serments", "Glacier des Serments froids", "Glace bleue, serments anciens et froid mordant.", "glacier bleu, vent boréal", 15, true, false, true},
        {"city_arena_valebrume", "Arène urbaine de Valebrume", "Ville", "Lieu prévu pour les combats uniques en ville.", "arène de pierre, gradins simples", 1, true, true, false}
    };
    return places;
}

std::vector<WorldMapPlace> WorldMap::getPlacesForBiome(const std::string& biomeName)
{
    std::vector<WorldMapPlace> result;
    for (const WorldMapPlace& place : getPlaces())
    {
        if (place.biome == biomeName)
        {
            result.push_back(place);
        }
    }
    return result;
}

std::vector<std::string> WorldMap::buildPlacePreviewLines(const std::string& biomeName, int distanceKm, bool known)
{
    std::vector<std::string> lines;
    lines.push_back("Biome : " + biomeName + " — " + fogStateText(known) + ".");
    lines.push_back("Distance depuis la ville actuelle : " + (distanceKm >= 0 ? std::to_string(distanceKm) + " km." : std::string("inconnue.")));
    lines.push_back("Animation future : trajet depuis la porte des remparts, puis animation dans le lieu choisi.");

    const std::vector<WorldMapPlace> places = getPlacesForBiome(biomeName);
    if (places.empty())
    {
        lines.push_back("Aucun lieu détaillé n'est encore catalogué pour ce biome.");
        return lines;
    }

    for (const WorldMapPlace& place : places)
    {
        const bool placeKnown = known && place.initiallyKnown;
        lines.push_back("- " + (placeKnown ? place.name : std::string("???")) + " [" + fogStateText(placeKnown) + "]");
        if (placeKnown)
        {
            lines.push_back("  " + place.description);
            lines.push_back("  Fond futur : " + place.backgroundTheme + ". Niveau conseillé : " + std::to_string(place.recommendedLevel) + ".");
        }
        else
        {
            lines.push_back("  Silhouette de lieu grisée/enfumée : le nom et le détail restent inconnus.");
        }
    }
    return lines;
}

std::string WorldMap::fogStateText(bool known)
{
    return known ? "connu" : "grisé/enfumé";
}
