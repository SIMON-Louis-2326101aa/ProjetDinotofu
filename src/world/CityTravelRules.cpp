// English: City travel and access rules for the future map and current terminal menus.
// Français : Règles de voyage et d'accès aux villes pour la future carte et les menus terminal actuels.
#include "world/CityTravelRules.hpp"

#include "economy/EconomyBalance.hpp"
#include "world/WorldMap.hpp"

#include "boss/BossCatalog.hpp"

#include <algorithm>
#include <cstddef>


namespace
{
    int localReputationScore(const Player& player, const std::string& cityId)
    {
        int score = 0;
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.locationId != cityId)
            {
                continue;
            }
            if (record.category == "pnj_servis") score += record.count * 3;
            else if (record.category == "types_quetes_completees") score += record.count * 2;
            else if (record.category == "lieux_visites") score += record.count;
            else if (record.category == "coffres_achetes" || record.category == "coffres_ameliores") score += record.count;
        }
        return score;
    }

    std::string localReputationLabel(int score)
    {
        if (score >= 80) return "héros local";
        if (score >= 45) return "fiable";
        if (score >= 20) return "apprécié";
        if (score >= 8) return "connu";
        if (score < 0) return "suspect";
        return "neutre";
    }
}


CityAccessReport CityTravelRules::evaluateAccess(const Player& player, const City& city)
{
    CityAccessReport report;
    report.allowed = true;
    report.lines.push_back("Ville : " + city.getName() + ".");
    report.lines.push_back("Condition officielle : " + city.getAccessRequirementText());
    const int reputationScore = localReputationScore(player, city.getId());
    report.lines.push_back("Réputation locale : " + localReputationLabel(reputationScore) + " (" + std::to_string(reputationScore) + ").");

    if (player.getLevel() < city.getMinimumLevel())
    {
        report.allowed = false;
        report.lines.push_back("Refus : niveau " + std::to_string(player.getLevel()) + "/" + std::to_string(city.getMinimumLevel()) + ".");
    }
    else
    {
        report.lines.push_back("Niveau : accepté (" + std::to_string(player.getLevel()) + "/" + std::to_string(city.getMinimumLevel()) + ").");
    }

    if (city.getRequiredBossId() > 0)
    {
        const Boss requiredBoss = BossCatalog::createBoss(city.getRequiredBossId());
        if (!player.isBossDefeated(city.getRequiredBossId()))
        {
            report.allowed = false;
            report.lines.push_back("Refus : preuve de valeur manquante — vaincre " + requiredBoss.getName() + ".");
        }
        else
        {
            report.lines.push_back("Preuve de valeur : " + requiredBoss.getName() + " déjà vaincu.");
        }
    }

    if (city.getId() == "port_lanterne")
    {
        const bool hasTravelProof = player.hasTitle("Aventurier") || player.getInventory().countMaterialById("guild_card") > 0 || player.getInventory().countMaterialById("municipal_proof_letter") > 0;
        if (!hasTravelProof)
        {
            report.allowed = false;
            report.lines.push_back("Refus : le port demande une carte de guilde, une attestation municipale ou un dossier d'aventurier reconnu.");
        }
        else
        {
            report.lines.push_back("Dossier : justificatif de voyage accepté par le guichet du port.");
        }
    }

    if (city.getId() == "rocheveille")
    {
        const bool hasMineProof = player.hasTitle("Aventurier") || player.getInventory().countMaterialById("city_service_stamp") > 0;
        if (!hasMineProof)
        {
            report.allowed = false;
            report.lines.push_back("Refus : Rocheveille demande une carte d'aventurier ou un tampon municipal pour éviter les débutants perdus dans les mines.");
        }
        else
        {
            report.lines.push_back("Dossier : autorisation de mine ou statut d'aventurier reconnu.");
        }
    }

    if (report.allowed)
    {
        report.lines.push_back("Accès : les portes peuvent s'ouvrir.");
    }
    else
    {
        report.lines.push_back("Accès : ville fermée pour l'instant, sans téléportation forcée ni raccourci gratuit.");
    }

    return report;
}

int CityTravelRules::getDistanceBetweenCities(const std::string& fromCityId, const std::string& toCityId)
{
    const City* from = City::findById(fromCityId);
    const City* to = City::findById(toCityId);
    if (from == nullptr || to == nullptr)
    {
        return -1;
    }
    return City::calculateDistanceBetween(*from, *to);
}

int CityTravelRules::getTravelTaxCopper(const Player& player, const City& destination, int distanceKm)
{
    return EconomyBalance::cityChangeTaxCopper(destination.getId(), distanceKm, player.hasCityVaultInCity(destination.getId()));
}

int CityTravelRules::getRouteDiscoveryLimit(int distanceKm)
{
    if (distanceKm <= 0) return 1;
    if (distanceKm <= 20) return 2;
    if (distanceKm <= 45) return 3;
    return 4;
}

std::string CityTravelRules::buildNormalizedRouteKey(const std::string& fromCityId, const std::string& toCityId)
{
    if (fromCityId <= toCityId)
    {
        return fromCityId + "<->" + toCityId;
    }
    return toCityId + "<->" + fromCityId;
}

int CityTravelRules::getRouteDiscoveryCount(const Player& player, const std::string& fromCityId, const std::string& toCityId)
{
    const std::string routeKey = buildNormalizedRouteKey(fromCityId, toCityId) + "::";
    int total = 0;
    for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
    {
        if (record.category == "decouvertes_route" && record.key.rfind(routeKey, 0) == 0 && record.count > 0)
        {
            total += record.count;
        }
    }
    return total;
}

bool CityTravelRules::isNightTravelClosed(const Player& player)
{
    return player.getCurrentDayPartName() == "Nuit";
}

std::vector<std::string> CityTravelRules::buildNightTravelWarningLines(const Player& player)
{
    std::vector<std::string> lines;
    lines.push_back("Moment actuel : " + player.formatWorldDateTimeLine() + ".");
    if (isNightTravelClosed(player))
    {
        lines.push_back("Les portes de route contrôlée sont fermées par les gardes pendant la nuit.");
        lines.push_back("But gameplay : éviter que le joueur marche volontairement sans payer d'auberge juste pour faire passer la nuit.");
        lines.push_back("Voyage nocturne futur : seulement via convoi gardé, permis spécial, urgence de quête ou événement scénarisé.");
        lines.push_back("Aucune attente gratuite jusqu'au matin n'est ajoutée ici, sinon l'auberge deviendrait inutile.");
    }
    else
    {
        lines.push_back("Route contrôlée ouverte : les gardes laissent encore sortir les voyageurs réguliers.");
        if (player.getWorldDayProgressUnits() >= 3)
        {
            lines.push_back("Attention : départ tardif. Un long trajet peut finir proche de la nuit ; les convois et auberges restent plus sûrs.");
        }
    }
    return lines;
}

int CityTravelRules::getRouteEventLimit(int distanceKm)
{
    if (distanceKm <= 0) return 1;
    if (distanceKm <= 20) return 2;
    if (distanceKm <= 45) return 3;
    return 4;
}

int CityTravelRules::getRouteEventCount(const Player& player, const std::string& fromCityId, const std::string& toCityId)
{
    const std::string routeKey = buildNormalizedRouteKey(fromCityId, toCityId) + "::";
    int total = 0;
    for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
    {
        if (record.category == "evenements_route" && record.key.rfind(routeKey, 0) == 0 && record.count > 0)
        {
            total += record.count;
        }
    }
    return total;
}

std::vector<std::string> CityTravelRules::buildTravelPreviewLines(const Player& player, const City& destination)
{
    std::vector<std::string> lines;
    const City* origin = City::findById(player.getCurrentCityId());
    if (origin == nullptr)
    {
        lines.push_back("Ville actuelle inconnue : le trajet ne peut pas être estimé proprement.");
        return lines;
    }

    const int distance = City::calculateDistanceBetween(*origin, destination);
    lines.push_back("Départ : " + origin->getName() + ".");
    lines.push_back("Arrivée : " + destination.getName() + ".");
    lines.push_back("Distance ville ↔ ville : " + std::to_string(distance) + " km environ.");
    lines.push_back("Temps de voyage : " + std::to_string(EconomyBalance::travelTimeUnitsForDistance(distance)) + " segment(s).");
    const int normalTax = EconomyBalance::cityChangeTaxCopper(destination.getId(), distance);
    const int actualTax = getTravelTaxCopper(player, destination, distance);
    lines.push_back("Taxe de changement de ville : " + std::to_string(actualTax) + " cuivre, prélevée seulement si tu changes réellement de ville.");
    if (player.hasCityVaultInCity(destination.getId()))
    {
        lines.push_back("Réduction : coffre municipal possédé à " + destination.getName() + " → taxe divisée par deux (base " + std::to_string(normalTax) + " cuivre).");
    }
    lines.push_back("Important : aucune taxe n'est prévue pour chaque exploration, afin d'éviter le côté relou à chaque sortie.");
    lines.push_back("Référence économie : trajet estimé à " + std::to_string(EconomyBalance::estimatedTravelCopperCost(distance)) + " cuivre pour billets/convois et équilibrage des routes.");
    lines.push_back("Animation future : chemin entre les remparts de " + origin->getName() + " et les portes de " + destination.getName() + ".");
    lines.push_back("Background de trajet futur : route, relais, météo et biome traversé selon la distance réelle.");
    const std::vector<std::string> nightLines = buildNightTravelWarningLines(player);
    for (const std::string& nightLine : nightLines)
    {
        lines.push_back(nightLine);
    }

    if (distance <= 20)
    {
        lines.push_back("Durée estimée : trajet court, animation rapide.");
    }
    else if (distance <= 45)
    {
        lines.push_back("Durée estimée : trajet moyen, animation de route complète.");
    }
    else
    {
        lines.push_back("Durée estimée : long voyage, animation plus longue et transitions possibles.");
    }

    const std::vector<std::string> routeLines = buildRouteSystemLines(player, destination);
    lines.push_back("");
    lines.insert(lines.end(), routeLines.begin(), routeLines.end());

    const CityAccessReport report = evaluateAccess(player, destination);
    lines.push_back("");
    lines.insert(lines.end(), report.lines.begin(), report.lines.end());
    return lines;
}

std::vector<CityBuildingPreview> CityTravelRules::getBuildingsForCity(const Player& player, const City& city)
{
    const bool registered = player.isRegisteredAtCityGuild(city.getId());
    std::vector<CityBuildingPreview> buildings = {
        {"guild", city.getGuildName(), "Guilde", "Maître de guilde", "Contrats officiels, mise à niveau locale, sanctions, médiation et rendu des quêtes de cette ville.", "bâtiment de guilde avec emblème local", true},
        {"delegated_office", "Bureau des mandats", "Guilde", "Gestionnaire des groupes", "Missions déléguées, quêtes publiées, dossiers de groupes et contacts progressifs.", "panneau de requêtes, bancs et comptoir", player.hasTitle("Aventurier")},
        {"inn", "Auberge locale", "Repos", "Aubergiste", "Lit commun, chambre sûre, repas, sécurité et gestion progressive de la fatigue.", "enseigne chaude, fenêtres éclairées", true},
        {"vault", "Coffre municipal", "Service", "Intendant du coffre", "Coffre personnel propre à cette ville : achat, niveau, contenu indépendant et transport encadré de matériaux.", "guichet sécurisé, casiers numérotés", true},
        {"gate", "Grande porte des remparts", "Exploration", "Noro le palefrenier", "Départ vers la grande carte d'exploration, les biomes proches et les routes longues.", "porte de remparts cliquable, route visible dehors", true},
        {"arena", "Arène de ville", "Combat unique", "Maître d'arène", "Combat unique ou entraînement spécial sans passer par les routes sauvages.", "arène urbaine, sable ou pierre selon la ville", true},
        {"market", "Marché local", "Commerce", "Prunigil le marchand", "Stocks différents selon les ressources, arrivages et spécialités de la ville.", "étals et caisses aux couleurs locales", true},
        {"archives", "Archives locales", "Savoir", "Archiviste Meron", "Rumeurs, cartes, légendes et informations achetables selon la région.", "salle d'archives, cartes murales", registered}
    };

    if (city.getId() == "rocheveille")
    {
        buildings.push_back({"forge_heavy", "Grande forge des mineurs", "Forge", "Bram le foreur", "Armes lourdes, armures robustes, minerais et réparations avancées.", "forge de montagne, enclumes massives", true});
        buildings.push_back({"mine_lift", "Ascenseur de mine", "Exploration", "Contremaître des rails", "Accès rapide aux lieux miniers proches quand ils sont connus.", "plateforme métallique suspendue", player.getLevel() >= 5});
    }
    else if (city.getId() == "port_lanterne")
    {
        buildings.push_back({"harbor", "Quais des lanternes", "Commerce", "Négociante des routes salées", "Commandes marchandes, cargaisons variables et futures réservations.", "quais, barques, lanternes basses", true});
        buildings.push_back({"underbridge", "Marché sous les ponts", "Risque", "Collecteur masqué", "Stocks rares, parfois douteux, à encadrer plus tard par réputation et risques.", "arches sombres, étals cachés", player.getLevel() >= 8});
    }
    else if (city.getId() == "lys_de_givre")
    {
        buildings.push_back({"sanctuary", "Sanctuaire du Lys boréal", "Soin", "Soigneuse du nord", "Soins rares, résistances élémentaires et futures bénédictions contrôlées.", "sanctuaire glacé, vitraux bleus", player.getLevel() >= 15});
        buildings.push_back({"frost_gate", "Porte du nord", "Exploration", "Garde boréal", "Accès aux glaciers et routes froides après preuve de valeur.", "grande porte gelée", player.isBossDefeated(1)});
    }
    else
    {
        buildings.push_back({"mira_office", "Intendance de Mira", "Histoire", "Mira", "Développement de Valebrume, priorités de survie et objectifs de chapitre.", "bureau simple, cartes et listes de matériaux", true});
        buildings.push_back({"bram_forge", "Forge de Bram", "Forge", "Bram", "Réparations, outils de départ et récupération de matériaux utiles.", "petite forge usée mais active", true});
    }

    return buildings;
}

std::vector<std::string> CityTravelRules::buildCityHubLines(const Player& player)
{
    std::vector<std::string> lines;
    const City* city = City::findById(player.getCurrentCityId());
    if (city == nullptr)
    {
        lines.push_back("Ville actuelle inconnue.");
        return lines;
    }

    lines.push_back("Ville : " + city->getName() + ".");
    lines.push_back(city->getDescription());
    lines.push_back("Guilde : " + city->getGuildName() + (player.isRegisteredAtCityGuild(city->getId()) ? " [enregistrée]." : " [mise à niveau locale non faite]."));
    lines.push_back("Future IG : image de ville cliquable, bâtiments séparés, grande porte pour explorer et arène urbaine pour combat unique.");
    lines.push_back("Politique images : elles restent 100% supplément visuel ; les textes et menus conservent toutes les informations utiles.");
    const std::vector<std::string> localLines = buildLocalCityDifferentiationLines(player);
    lines.push_back("");
    lines.insert(lines.end(), localLines.begin(), localLines.end());
    lines.push_back("");

    const std::vector<CityBuildingPreview> buildings = getBuildingsForCity(player, *city);
    for (const CityBuildingPreview& building : buildings)
    {
        lines.push_back("- " + building.name + " [" + building.category + "] " + (building.unlocked ? "accessible" : "verrouillé") + ".");
        lines.push_back("  " + building.detail);
        lines.push_back("  Asset futur : " + building.pixelArtHint + ".");
    }
    return lines;
}


std::vector<std::string> CityTravelRules::buildLocalCityDifferentiationLines(const Player& player)
{
    std::vector<std::string> lines;
    const City* city = City::findById(player.getCurrentCityId());
    if (city == nullptr)
    {
        return {"Ville actuelle inconnue : profil local indisponible."};
    }

    lines.push_back("Profil local : " + city->getName() + ".");
    lines.push_back("Guilde locale : " + city->getGuildName() + ".");
    lines.push_back("Inscription locale : " + std::string(player.isRegisteredAtCityGuild(city->getId()) ? "validée" : "mise à niveau disponible") + ".");
    const int reputationScore = localReputationScore(player, city->getId());
    lines.push_back("Réputation locale : " + localReputationLabel(reputationScore) + " (score " + std::to_string(reputationScore) + ").");
    lines.push_back("Effet futur : prix, négociations, accès, gardes, réservations et quêtes locales pourront utiliser ce score.");
    lines.push_back("Ressources proches :");
    for (const std::string& resource : city->getResourceSpecialties())
    {
        lines.push_back("- " + resource);
    }
    lines.push_back("Stocks et services plus probables :");
    for (const std::string& stock : city->getStockSpecialties())
    {
        lines.push_back("- " + stock);
    }

    if (city->getId() == "valebrume")
    {
        lines.push_back("PNJ principaux : Mira, Bram, Prunigil, Noro, Meron.");
        lines.push_back("Économie : prix doux, matériel de départ, réparations simples, bonnes rumeurs de route courte.");
        lines.push_back("Quêtes locales : reconstruction, défense, livraison et premiers contrats de guilde.");
    }
    else if (city->getId() == "rocheveille")
    {
        lines.push_back("PNJ principaux : Bram le foreur, Contremaître des rails, maître de forge.");
        lines.push_back("Économie : minerais moins chers, armes lourdes plus fréquentes, réparations solides mais contrôlées.");
        lines.push_back("Quêtes locales : escorte de mine, récupération de minerai, routes rocheuses et sécurité des galeries.");
    }
    else if (city->getId() == "port_lanterne")
    {
        lines.push_back("PNJ principaux : négociante des routes salées, collecteur masqué, archiviste du port.");
        lines.push_back("Économie : stocks variables, importations, commandes marchandes, prix soumis aux arrivages.");
        lines.push_back("Quêtes locales : cargaisons, rumeurs de quai, escortes marchandes et dossiers de contrebande.");
    }
    else if (city->getId() == "lys_de_givre")
    {
        lines.push_back("PNJ principaux : soigneuse du nord, garde boréal, archiviste des serments.");
        lines.push_back("Économie : soins rares, résistances élémentaires, équipement froid, prix plus élevés.");
        lines.push_back("Quêtes locales : preuve de valeur, expéditions gelées, fragments lunaires et serments anciens.");
    }

    lines.push_back("Règle future : une quête de guilde locale devra généralement être rendue à la bonne guilde, pas dans n'importe quelle ville.");
    return lines;
}

std::vector<std::string> CityTravelRules::buildRouteSystemLines(const Player& player, const City& destination)
{
    std::vector<std::string> lines;
    const City* origin = City::findById(player.getCurrentCityId());
    if (origin == nullptr)
    {
        return {"Route inconnue : ville de départ introuvable."};
    }

    const int distance = City::calculateDistanceBetween(*origin, destination);
    const int baseTax = getTravelTaxCopper(player, destination, distance);
    const int discoveryCount = getRouteDiscoveryCount(player, origin->getId(), destination.getId());
    const int discoveryLimit = getRouteDiscoveryLimit(distance);
    const int eventCount = getRouteEventCount(player, origin->getId(), destination.getId());
    const int eventLimit = getRouteEventLimit(distance);
    lines.push_back("Routes possibles entre " + origin->getName() + " et " + destination.getName() + " :");
    lines.push_back("- Route contrôlée : choix par défaut, taxe " + std::to_string(baseTax) + " cuivre, danger normal, fiable pour le moteur actuel.");
    lines.push_back("- Route sûre : plus lente, danger réduit, coût futur plus élevé via escorte/convoi.");
    lines.push_back("- Route rapide : plus courte en animation, danger supérieur, événements de route plus probables.");
    lines.push_back("- Convoi marchand : payant, stable, utile pour les longues distances et futures commandes.");
    lines.push_back("- Raccourci découvert : indisponible tant que le lieu ou la route n'a pas été trouvé en exploration.");
    lines.push_back("Découvertes de route : " + std::to_string(discoveryCount) + "/" + std::to_string(discoveryLimit) + " pour cette liaison. Une fois la limite atteinte, la route est considérée comme bien connue.");
    lines.push_back("Événements importants de route : " + std::to_string(eventCount) + "/" + std::to_string(eventLimit) + ". Les gros événements ne peuvent pas se répéter à l'infini.");
    lines.push_back("État V3.34 : le joueur choisit maintenant une route réelle avant validation ; les effets restent légers tant que les systèmes d'embuscade/convoi ne sont pas complets.");
    lines.push_back("Rappel taxe : elle s'applique seulement au changement de ville, jamais à chaque exploration depuis la porte.");
    return lines;
}

std::vector<std::string> CityTravelRules::buildExplorationMapLines(const Player& player)
{
    std::vector<std::string> lines;
    const City* currentCity = City::findById(player.getCurrentCityId());
    if (currentCity == nullptr)
    {
        lines.push_back("Carte : ville actuelle inconnue.");
        return lines;
    }

    lines.push_back("Ville actuelle : " + currentCity->getName() + ".");
    lines.push_back("Future IG : grande carte par biomes, lieux cliquables, zones inconnues grisées ou enfumées.");
    lines.push_back("Même avec les images activées en IG, les distances, dangers, conditions et récompenses restent écrits.");
    lines.push_back("La distance d'un même biome varie selon la ville de départ.");
    for (const CityBiomeDistance& distance : currentCity->getBiomeDistances())
    {
        const std::string knowledge = distance.initiallyKnown ? "connu" : "gris/enfumé tant que non découvert";
        lines.push_back("- " + distance.biomeName + " : " + std::to_string(distance.distanceKm) + " km | " + knowledge + " | fond futur : " + distance.backgroundTheme + ".");
        std::vector<std::string> placeLines = WorldMap::buildPlacePreviewLines(distance.biomeName, distance.distanceKm, distance.initiallyKnown);
        for (const std::string& placeLine : placeLines)
        {
            lines.push_back("  " + placeLine);
        }
    }
    return lines;
}

std::vector<std::string> CityTravelRules::buildSchematicMapLines(const Player& player)
{
    std::vector<std::string> lines;
    const City* currentCity = City::findById(player.getCurrentCityId());
    lines.push_back("Carte schématique IG temporaire : points de ville, routes et biomes avant le vrai pixel-art.");
    lines.push_back("Ville actuelle : " + std::string(currentCity == nullptr ? "inconnue" : currentCity->getName()) + ".");
    lines.push_back("");
    lines.push_back("Villes :");
    for (const City& city : City::getCatalog())
    {
        const CityAccessReport access = evaluateAccess(player, city);
        std::string marker = currentCity != nullptr && city.getId() == currentCity->getId() ? "[ICI] " : "";
        marker += access.allowed ? "[ouverte] " : "[fermée] ";
        lines.push_back("- " + marker + city.getName() + " (x=" + std::to_string(city.getMapX()) + ", y=" + std::to_string(city.getMapY()) + ") — " + city.getGuildName() + ".");
    }
    lines.push_back("");
    lines.push_back("Routes connues :");
    for (std::size_t i = 0; i < City::getCatalog().size(); ++i)
    {
        for (std::size_t j = i + 1; j < City::getCatalog().size(); ++j)
        {
            const City& a = City::getCatalog()[i];
            const City& b = City::getCatalog()[j];
            const int distance = City::calculateDistanceBetween(a, b);
            const int discoveries = getRouteDiscoveryCount(player, a.getId(), b.getId());
            const int discoveryLimit = getRouteDiscoveryLimit(distance);
            const int events = getRouteEventCount(player, a.getId(), b.getId());
            lines.push_back("- " + a.getName() + " ↔ " + b.getName() + " : " + std::to_string(distance) + " km | découvertes " + std::to_string(discoveries) + "/" + std::to_string(discoveryLimit) + " | événements " + std::to_string(events) + "/" + std::to_string(getRouteEventLimit(distance)) + ".");
        }
    }
    lines.push_back("");
    lines.push_back("Biomes autour de la ville actuelle :");
    if (currentCity != nullptr)
    {
        for (const CityBiomeDistance& biome : currentCity->getBiomeDistances())
        {
            lines.push_back("- " + std::string(biome.initiallyKnown ? "visible" : "gris/enfumé") + " : " + biome.biomeName + " à " + std::to_string(biome.distanceKm) + " km — fond futur : " + biome.backgroundTheme + ".");
        }
    }
    lines.push_back("Note : cette carte est volontairement schématique. Le vrai hub cliquable arrivera avec la bible pixel-art et les assets.");
    lines.push_back("Accessibilité : l’image ne devra jamais cacher une condition, un danger, un coût ou une récompense.");
    return lines;
}

std::vector<std::string> CityTravelRules::buildFutureVisualPlanningLines()
{
    return {
        "Ville future : une vraie image pixel-art de ville servira de hub.",
        "Paramètres : en terminal, les images sont désactivées et non activables ; en IG, elles pourront être désactivées.",
        "Règle accessibilité : aucune information ne doit être retirée du texte sous prétexte qu’une image existe.",
        "Bâtiments : guilde, coffre, forge, boutiques, auberge, archives et autres services deviendront cliquables.",
        "Exploration : départ par la grande porte des remparts.",
        "Combat unique : accès par une arène visible en ville.",
        "Carte du monde : grande carte à biomes avec lieux cliquables ; zones inconnues grisées/enfumées.",
        "Animation : d'abord le chemin entre le lieu de départ et le lieu d'arrivée, puis l'animation dans le lieu choisi.",
        "Background : chaque trajet et chaque lieu doit utiliser un fond cohérent avec son thème, pas un décor générique répété.",
        "Nuit : les routes normales sont fermées par les gardes ; seuls convoi gardé, permis spécial ou urgence de quête pourront autoriser un départ nocturne.",
        "Missions déléguées : des aventuriers/PNJ pourront partir plusieurs jours et revenir faire leur compte rendu comme si le joueur était le client.",
        "Quêtes/événements : certaines étapes auront une petite illustration dédiée quand les assets existeront."
    };
}
