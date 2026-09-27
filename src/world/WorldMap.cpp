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
        {"troubled_marsh_reeds", "Roselière aux pas noyés", "Marais trouble", "Une langue d'eau sombre où des bulles suivent parfois les voyageurs à contre-courant.", "roseaux noirs, eau trouble, insectes pâles", 5, true, false, true},
        {"forgotten_cemetery_wall", "Mur des tombes sans nom", "Cimetière oublié", "Des stèles effacées se serrent derrière un mur fendu ; les traces fraîches y sont rarement rassurantes.", "pierres grises, herbes hautes, brume froide", 7, false, false, true},
        {"collapsed_ruins_gallery", "Galerie des arches tombées", "Ruines effondrées", "Un ancien passage dont les dalles réagissent encore parfois au mana et au poids des pas.", "arches brisées, poussière arcanique", 9, false, false, true},
        {"iron_firefly_orchard", "Verger des lucioles de fer", "Verger des lucioles de fer", "Des arbres noueux abritent des insectes métalliques dont les essaims dessinent des chemins trompeurs.", "arbres sombres, lueurs métalliques", 8, false, false, true},
        {"drowned_archive_stacks", "Rayonnages sous l'eau", "Archives noyées", "Des livres gonflés d'eau flottent entre des rayonnages inclinés ; certains fragments restent lisibles dans des langues anciennes.", "bibliothèque inondée, reflets verts", 10, false, false, true},
        {"gray_drake_nests", "Corniche des nids gris", "Falaises des drakes gris", "La roche porte des griffures profondes et des restes d'œufs trop gros pour des oiseaux.", "falaises, nids, ciel battu par le vent", 12, false, false, true},
        {"abandoned_fair_mirror", "Allée des miroirs ternis", "Foire abandonnée", "Des stands grincent sans vent et les miroirs cassés renvoient parfois un mouvement de trop.", "fanions déchirés, bois peint, miroirs", 11, false, false, true},
        {"split_bell_temple_steps", "Cent marches fendues", "Temple des cloches fendues", "Chaque marche porte une gravure différente ; les cloches sonnent parfois sans être touchées.", "sanctuaire givré, bronze fendu", 17, false, false, true},
        {"white_bone_quarry_cut", "Tranchée des fossiles blancs", "Carrière des os blancs", "La craie révèle ossements, outils abandonnés et sillons laissés par des machines disparues.", "craie blanche, fossiles, treuils", 12, false, false, true},
        {"underbridge_market", "Marché des arches basses", "Marché sous les ponts", "Une succession d'étals mobiles où l'origine d'un objet dépend beaucoup de la personne qui raconte son histoire.", "arches humides, lanternes basses", 9, true, false, true},
        {"weeping_statue_garden", "Jardin des visages mouillés", "Jardin des statues qui pleurent", "Les statues ruissellent même par temps sec et semblent changer d'expression quand on revient sur ses pas.", "marbre humide, haies silencieuses", 13, false, false, true},
        {"corruption_wood_vein", "Veine noire du sous-bois", "Bois de la Corruption", "La sève y est sombre et certaines racines se contractent au passage des êtres vivants.", "forêt malade, sève noire, spores", 16, false, false, true},
        {"dark_link_crypt", "Crypte des chaînes froides", "Crypte du Sombre-Lien", "Des anneaux rouillés couvrent les murs d'une crypte où le silence semble retenir les sons.", "crypte, chaînes, lueurs violettes", 18, false, false, true},
        {"mana_fairy_grove", "Clairière aux poussières bleues", "Bosquet des Fées du Mana", "Le mana se condense en poussière lumineuse et les petits habitants du lieu déplacent parfois les balises des voyageurs.", "clairière lumineuse, pollen de mana", 16, false, false, true},
        {"kitsune_nine_sparks", "Sentier des Neuf Étincelles", "Sanctuaire kitsuné des Neuf Étincelles", "Neuf lanternes marquent un sentier dont l'ordre change selon l'heure et l'humeur des esprits.", "torii, lanternes, feu follet", 18, false, false, true},
        {"pure_mana_confluence", "Confluence des trois halos", "Confluence du Mana pur", "Trois courants de mana se rencontrent sans se mélanger et altèrent brièvement sons, couleurs et sorts.", "rivières de mana, halos instables", 20, false, false, true},
        {"floating_island_anchor", "Ancre des îles flottantes", "Archipel des îles flottantes", "Des blocs de roche dérivent autour d'une ancienne ancre de pierre, reliés par des courants ascendants imprévisibles.", "îlots suspendus, nuages rapides", 23, false, false, true},
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
