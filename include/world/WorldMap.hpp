// EN: World map data used by city travel, exploration previews and future clickable GUI maps.
// FR: Données de carte utilisées par les voyages, l'exploration et les futures cartes IG cliquables.
#ifndef INCLUDE_WORLD_WORLDMAP_HPP
#define INCLUDE_WORLD_WORLDMAP_HPP

#include <string>
#include <vector>

struct WorldMapPlace
{
    std::string id;
    std::string name;
    std::string biome;
    std::string description;
    std::string backgroundTheme;
    int recommendedLevel = 1;
    bool initiallyKnown = true;
    bool combatArena = false;
    bool questIllustrationRecommended = false;
};

class WorldMap
{
public:
    static const std::vector<WorldMapPlace>& getPlaces();
    static std::vector<WorldMapPlace> getPlacesForBiome(const std::string& biomeName);
    static std::vector<std::string> buildPlacePreviewLines(const std::string& biomeName, int distanceKm, bool known);
    static std::string fogStateText(bool known);
};

#endif
