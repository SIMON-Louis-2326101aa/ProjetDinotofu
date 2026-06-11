// English: Stable city metadata used by travel, local guilds, stocks and municipal storage.
// Français : Métadonnées stables des villes utilisées par le voyage, les guildes locales, les stocks et le coffre municipal.
#include "world/City.hpp"

#include <cmath>
#include <limits>

City::City()
    : id(""), name("Ville inconnue"), guildName("Guilde inconnue"), description(""), startingCity(false), mapX(0), mapY(0), minimumLevel(1), requiredBossId(0), accessRequirementText("")
{
}

City::City(
    const std::string& id,
    const std::string& name,
    const std::string& guildName,
    const std::string& description,
    const std::vector<std::string>& resourceSpecialties,
    const std::vector<std::string>& stockSpecialties,
    bool startingCity,
    int mapX,
    int mapY,
    int minimumLevel,
    int requiredBossId,
    const std::string& accessRequirementText,
    const std::vector<CityBiomeDistance>& biomeDistances
)
    : id(id),
      name(name),
      guildName(guildName),
      description(description),
      resourceSpecialties(resourceSpecialties),
      stockSpecialties(stockSpecialties),
      startingCity(startingCity),
      mapX(mapX),
      mapY(mapY),
      minimumLevel(minimumLevel < 1 ? 1 : minimumLevel),
      requiredBossId(requiredBossId < 0 ? 0 : requiredBossId),
      accessRequirementText(accessRequirementText),
      biomeDistances(biomeDistances)
{
}

const std::string& City::getId() const { return id; }
const std::string& City::getName() const { return name; }
const std::string& City::getGuildName() const { return guildName; }
const std::string& City::getDescription() const { return description; }
const std::vector<std::string>& City::getResourceSpecialties() const { return resourceSpecialties; }
const std::vector<std::string>& City::getStockSpecialties() const { return stockSpecialties; }
bool City::isStartingCity() const { return startingCity; }
int City::getMapX() const { return mapX; }
int City::getMapY() const { return mapY; }
int City::getMinimumLevel() const { return minimumLevel; }
int City::getRequiredBossId() const { return requiredBossId; }
const std::string& City::getAccessRequirementText() const { return accessRequirementText; }
const std::vector<CityBiomeDistance>& City::getBiomeDistances() const { return biomeDistances; }

int City::getDistanceToBiome(const std::string& biomeName) const
{
    for (const CityBiomeDistance& distance : biomeDistances)
    {
        if (distance.biomeName == biomeName)
        {
            return distance.distanceKm;
        }
    }
    return -1;
}

const std::vector<City>& City::getCatalog()
{
    static const std::vector<City> cities = {
        City(
            "valebrume",
            "Valebrume",
            "Guilde de Valebrume",
            "Ville de départ bâtie autour d'un ancien village. Elle mélange artisans, aventuriers, humains et semi-humains.",
            {"bois", "résines", "plantes de route", "matériaux de survie"},
            {"équipement de départ", "réparations", "potions simples", "renseignements locaux"},
            true,
            0,
            0,
            1,
            0,
            "Ville ouverte aux nouveaux aventuriers.",
            {
                {"Plaine sauvage", 3, "prairie ouverte / chemins de village", true},
                {"Route commerciale", 6, "route pavée, bornes et convois", true},
                {"Forêt ancienne", 12, "lisière verte et sous-bois", true},
                {"Mares gélatineuses", 15, "sol humide et flaques colorées", true},
                {"Bocage aux lanternes", 28, "bosquets nocturnes et champignons-lampes", false},
                {"Désert d'argile rouge", 45, "poussière rouge et fausses oasis", false}
            }
        ),
        City(
            "rocheveille",
            "Rocheveille",
            "Guilde du Marteau Veilleur",
            "Ville minière accrochée aux parois, connue pour ses forges lourdes et ses convois protégés.",
            {"minerais", "fer froid", "cristaux", "pièces mécaniques"},
            {"armes lourdes", "armures", "outils de récolte", "réparations avancées"},
            false,
            32,
            -14,
            5,
            0,
            "Niveau 5 conseillé : les gardes refusent les aventuriers trop fragiles pour les routes de mine.",
            {
                {"Mine sifflante", 4, "galeries, rails et lampes de mine", true},
                {"Montagne froide", 8, "rochers froids et vent de pente", true},
                {"Ruines effondrées", 18, "pierres cassées et poussière arcanique", false},
                {"Carrière des os blancs", 25, "craie blanche, fossiles et machines de taille", false},
                {"Falaises des drakes gris", 31, "corniches et nids de drakes", false}
            }
        ),
        City(
            "port_lanterne",
            "Port-Lanterne",
            "Guilde des Routes Salées",
            "Cité marchande où se croisent caravanes, pêcheurs, contrebandiers et négociants venus de plusieurs régions.",
            {"poissons", "sels", "cordages", "marchandises importées"},
            {"stocks variables", "objets étrangers", "commandes marchandes", "transport"},
            false,
            -26,
            22,
            8,
            0,
            "Niveau 8 conseillé : la ville demande de savoir survivre à une vraie route marchande.",
            {
                {"Canaux de brume bleue", 5, "ponts bas, roseaux bleutés et barques", true},
                {"Archives noyées", 12, "bibliothèque inondée et couloirs humides", false},
                {"Marché sous les ponts", 9, "arches, étals illégaux et lanternes basses", true},
                {"Foire abandonnée", 22, "stands morts et fanions déchirés", false},
                {"Désert d'argile rouge", 38, "dunes rouges et caravanes", false}
            }
        ),
        City(
            "lys_de_givre",
            "Lys-de-Givre",
            "Guilde du Lys Boréal",
            "Ville froide tournée vers la magie, les soins rares et les expéditions dans les zones gelées.",
            {"herbes froides", "glace magique", "tissus isolants", "fragments lunaires"},
            {"résistances élémentaires", "objets magiques", "soins rares", "équipement d'expédition"},
            false,
            48,
            35,
            15,
            1,
            "Niveau 15 et victoire contre Fitoria demandés : le conseil veut une preuve de valeur avant d'ouvrir les portes du nord.",
            {
                {"Glacier des Serments froids", 5, "glace bleue, serments et congères", true},
                {"Montagne froide", 14, "cols gelés et vents coupants", true},
                {"Temple des cloches fendues", 26, "sanctuaire givré et cloches cassées", false},
                {"Bosquet des Fées du Mana", 33, "lumières de mana et fées joueuses", false},
                {"Confluence du Mana pur", 42, "rivières magiques et halos instables", false}
            }
        )
    };
    return cities;
}

const City* City::findById(const std::string& cityId)
{
    for (const City& city : getCatalog())
    {
        if (city.getId() == cityId)
        {
            return &city;
        }
    }
    return nullptr;
}

int City::calculateDistanceBetween(const City& from, const City& to)
{
    const int dx = from.getMapX() - to.getMapX();
    const int dy = from.getMapY() - to.getMapY();
    const double distance = std::sqrt(static_cast<double>(dx * dx + dy * dy));
    return static_cast<int>(std::round(distance));
}
