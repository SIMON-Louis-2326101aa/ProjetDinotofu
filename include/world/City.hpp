// English: Stable city metadata used by travel, local guilds, stocks and municipal storage.
// Français : Métadonnées stables des villes utilisées par le voyage, les guildes locales, les stocks et le coffre municipal.
#ifndef INCLUDE_WORLD_CITY_HPP
#define INCLUDE_WORLD_CITY_HPP

#include <string>
#include <vector>

struct CityBiomeDistance
{
    std::string biomeName;
    int distanceKm = 0;
    std::string backgroundTheme;
    bool initiallyKnown = true;
};

class City
{
private:
    std::string id;
    std::string name;
    std::string guildName;
    std::string description;
    std::vector<std::string> resourceSpecialties;
    std::vector<std::string> stockSpecialties;
    bool startingCity;
    int mapX;
    int mapY;
    int minimumLevel;
    int requiredBossId;
    std::string accessRequirementText;
    std::vector<CityBiomeDistance> biomeDistances;

public:
    City();
    City(
        const std::string& id,
        const std::string& name,
        const std::string& guildName,
        const std::string& description,
        const std::vector<std::string>& resourceSpecialties,
        const std::vector<std::string>& stockSpecialties,
        bool startingCity = false,
        int mapX = 0,
        int mapY = 0,
        int minimumLevel = 1,
        int requiredBossId = 0,
        const std::string& accessRequirementText = "",
        const std::vector<CityBiomeDistance>& biomeDistances = {}
    );

    const std::string& getId() const;
    const std::string& getName() const;
    const std::string& getGuildName() const;
    const std::string& getDescription() const;
    const std::vector<std::string>& getResourceSpecialties() const;
    const std::vector<std::string>& getStockSpecialties() const;
    bool isStartingCity() const;
    int getMapX() const;
    int getMapY() const;
    int getMinimumLevel() const;
    int getRequiredBossId() const;
    const std::string& getAccessRequirementText() const;
    const std::vector<CityBiomeDistance>& getBiomeDistances() const;
    int getDistanceToBiome(const std::string& biomeName) const;

    static const std::vector<City>& getCatalog();
    static const City* findById(const std::string& cityId);
    static int calculateDistanceBetween(const City& from, const City& to);
};

#endif
