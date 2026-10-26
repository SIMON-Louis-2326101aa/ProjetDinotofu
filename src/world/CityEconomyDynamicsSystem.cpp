#include "world/CityEconomyDynamicsSystem.hpp"
#include "world/CityTradeFlowSystem.hpp"

#include <algorithm>

namespace
{
    int wealthIndex(CityWealthTier tier)
    {
        switch (tier)
        {
            case CityWealthTier::Poor: return 0;
            case CityWealthTier::Modest: return 1;
            case CityWealthTier::Comfortable: return 2;
            case CityWealthTier::Rich: return 3;
            case CityWealthTier::Opulent: return 4;
        }
        return 2;
    }

    CityWealthTier wealthFromIndex(int value)
    {
        switch (std::clamp(value, 0, 4))
        {
            case 0: return CityWealthTier::Poor;
            case 1: return CityWealthTier::Modest;
            case 2: return CityWealthTier::Comfortable;
            case 3: return CityWealthTier::Rich;
            default: return CityWealthTier::Opulent;
        }
    }

    void addDemand(CityEconomicPulse& state, ShopType goodsType, const std::string& label, int bonus)
    {
        for (CityMarketDemand& demand : state.demands)
        {
            if (demand.goodsType == goodsType)
            {
                demand.sellBonusPercent = std::max(demand.sellBonusPercent, bonus);
                if (demand.label.empty()) demand.label = label;
                return;
            }
        }
        state.demands.push_back({goodsType, label, std::clamp(bonus, 0, 18)});
    }

    std::string serviceQualityLabel(int score)
    {
        if (score < 40) return "fragile / limité";
        if (score < 55) return "ordinaire";
        if (score < 70) return "correct";
        if (score < 85) return "très bon";
        return "référence locale";
    }

    std::string serviceDisplayName(ShopType type)
    {
        switch (type)
        {
            case ShopType::Blacksmith: return "Forge / réparations";
            case ShopType::Alchemist: return "Alchimie";
            case ShopType::Enchanter: return "Enchantement";
            case ShopType::Transport: return "Transport";
            default: return shopTypeToText(type);
        }
    }

    void applyRegularCityRhythm(CityEconomicPulse& state, int worldDay)
    {
        const int safeDay = std::max(0, worldDay);
        const int dayInWeek = safeDay % 7;
        const int weekIndex = safeDay / 7;
        const int currentEvent = weekIndex % 10;
        const int nextEvent = (weekIndex + 1) % 10;

        // The event itself affects day 0 and leaves a one-day aftermath. Days 5-6 are preparations
        // for next week's event. This makes local demand visible before an event instead of spawning
        // magically on the exact morning it happens.
        int eventIndex = -1;
        bool preparation = false;
        if (dayInWeek <= 1)
        {
            eventIndex = currentEvent;
            state.remainingDays = 2 - dayInWeek;
        }
        else if (dayInWeek >= 5)
        {
            eventIndex = nextEvent;
            preparation = true;
            state.remainingDays = 7 - dayInWeek;
        }

        if (eventIndex < 0)
        {
            state.eventId = "stable_market";
            state.headline = "activité économique ordinaire";
            return;
        }

        state.temporary = true;
        switch (eventIndex)
        {
            case 0: // Guild tournament
                state.eventId = preparation ? "guild_tournament_preparation" : "guild_tournament";
                state.headline = preparation ? "préparatifs du tournoi de guilde" : "tournoi de guilde en cours";
                addDemand(state, ShopType::Weapon, "armes d'entraînement et de remplacement", 8);
                addDemand(state, ShopType::Armor, "protections et pièces d'armure", 8);
                addDemand(state, ShopType::Blacksmith, "réparations, métal et pièces de forge", 7);
                addDemand(state, ShopType::MonsterMaterial, "composants de créatures utiles aux trophées et ateliers", 5);
                state.generalBuyModifierPercent += 1;
                break;
            case 1: // Monster hunt
                state.eventId = preparation ? "monster_hunt_preparation" : "monster_hunt";
                state.headline = preparation ? "préparatifs d'une chasse aux monstres" : "chasse aux monstres locale";
                addDemand(state, ShopType::MonsterMaterial, "composants et preuves de chasse", 14);
                addDemand(state, ShopType::Consumable, "rations, antidotes et consommables de terrain", 7);
                addDemand(state, ShopType::Weapon, "armes adaptées aux sorties de chasse", 5);
                break;
            case 2: // Merchant fair
                state.eventId = preparation ? "merchant_fair_preparation" : "merchant_fair";
                state.headline = preparation ? "installation de la grande foire marchande" : "grande foire marchande";
                addDemand(state, ShopType::Material, "matériaux et marchandises revendables", 8);
                addDemand(state, ShopType::Plant, "plantes locales et produits frais", 7);
                addDemand(state, ShopType::Consumable, "produits de passage et petites fournitures", 6);
                state.prosperityShift += 1;
                state.generalSellModifierPercent += 3;
                state.serviceQualityDelta += 4;
                break;
            case 3: // Knowledge day
                state.eventId = preparation ? "knowledge_day_preparation" : "knowledge_day";
                state.headline = preparation ? "préparatifs de la journée du savoir" : "journée du savoir";
                addDemand(state, ShopType::Library, "livres, notes et renseignements vérifiables", 10);
                addDemand(state, ShopType::Enchanter, "composants et supports d'étude magique", 6);
                break;
            case 4: // Mission exchange
                state.eventId = preparation ? "mission_exchange_preparation" : "mission_exchange";
                state.headline = preparation ? "préparation de la bourse aux missions" : "bourse aux missions";
                addDemand(state, ShopType::Consumable, "fournitures de départ et rations", 7);
                addDemand(state, ShopType::Material, "matériaux pratiques demandés par les petits contrats", 6);
                addDemand(state, ShopType::Transport, "équipement de route et logistique", 5);
                break;
            case 5: // Honor ceremony
                state.eventId = preparation ? "honor_ceremony_preparation" : "honor_ceremony";
                state.headline = preparation ? "préparatifs de la cérémonie d'honneur" : "cérémonie d'honneur";
                addDemand(state, ShopType::CityService, "documents, fournitures et services municipaux", 5);
                state.serviceQualityDelta += 2;
                break;
            case 6: // Village games
                state.eventId = preparation ? "village_games_preparation" : "village_games";
                state.headline = preparation ? "préparatifs des jeux du village" : "jeux du village";
                addDemand(state, ShopType::Consumable, "boissons, repas et petites fournitures", 7);
                addDemand(state, ShopType::Lodging, "hébergement et services pour visiteurs", 5);
                state.prosperityShift += 1;
                state.generalSellModifierPercent += 2;
                break;
            case 7: // Harvest / solstice
                state.eventId = preparation ? "harvest_festival_preparation" : "harvest_festival";
                state.headline = preparation ? "préparatifs de la fête des moissons / solstice" : "fête des moissons / solstice";
                addDemand(state, ShopType::Plant, "plantes, récoltes et ingrédients locaux", 11);
                addDemand(state, ShopType::Consumable, "rations, nourriture et boissons", 8);
                addDemand(state, ShopType::Alchemist, "herbes et ingrédients d'alchimie", 6);
                state.prosperityShift += 1;
                state.generalSellModifierPercent += 3;
                state.serviceQualityDelta += 3;
                break;
            case 8: // Community collection
                state.eventId = preparation ? "community_collection_preparation" : "community_collection";
                state.headline = preparation ? "préparatifs d'une collecte communautaire" : "collecte communautaire de quartier";
                addDemand(state, ShopType::Material, "bois, toile, cordes et matériaux utiles aux habitants", 10);
                addDemand(state, ShopType::Consumable, "rations et fournitures simples à redistribuer", 8);
                addDemand(state, ShopType::Plant, "plantes utiles, aliments et herbes communes", 6);
                addDemand(state, ShopType::CityService, "registres, transport court et organisation municipale", 5);
                state.generalSellModifierPercent += 1;
                state.serviceQualityDelta += 2;
                break;
            default: // Raid / city-defense preparation
                state.eventId = preparation ? "raid_defense_preparation" : "raid_defense_mobilization";
                state.headline = preparation ? "préparatifs d'une défense contre un raid annoncé" : "mobilisation locale face à une menace de raid";
                addDemand(state, ShopType::Material, "fer, cuir, bois solide et matériaux de barricade", 15);
                addDemand(state, ShopType::Weapon, "armes prêtes à équiper les gardes et volontaires", 13);
                addDemand(state, ShopType::Armor, "armures, boucliers et protections de remplacement", 13);
                addDemand(state, ShopType::Blacksmith, "réparations urgentes, pièces de forge et métal ouvré", 14);
                addDemand(state, ShopType::Consumable, "rations, bandages et consommables de défense", 8);
                // Une mobilisation sérieuse tend les services ordinaires : la ville vit, mais ce n'est pas un festival.
                state.serviceQualityDelta -= 2;
                break;
        }
    }
}

CityEconomicPulse CityEconomyDynamicsSystem::stateFor(
    const std::string& cityId,
    int worldDay,
    int repairDaysRemaining,
    int recentEventDays
)
{
    const CityGameplayProfile profile = CityProfileSystem::profileFor(cityId);
    CityEconomicPulse state;
    state.structuralWealth = profile.wealth;
    state.effectiveWealth = profile.wealth;

    applyRegularCityRhythm(state, worldDay);

    // Actual damage/reconstruction overrides the gentle weekly rhythm. It is intentionally temporary:
    // when the repair marker reaches zero, the city's structural tier immediately becomes its anchor again.
    if (repairDaysRemaining > 0)
    {
        state.temporary = true;
        state.eventId = "reconstruction";
        state.headline = "réparations et remise en route des commerces";
        state.remainingDays = repairDaysRemaining;
        state.prosperityShift = repairDaysRemaining >= 5 ? -2 : -1;
        state.generalBuyModifierPercent = 0; // ShopMenu already handles the explicit crisis premium.
        state.generalSellModifierPercent = -4;
        state.serviceQualityDelta = repairDaysRemaining >= 5 ? -12 : -7;
        state.demands.clear();
        addDemand(state, ShopType::Material, "métal, cuir, bois et matériaux de reconstruction", 14);
        addDemand(state, ShopType::Blacksmith, "pièces de réparation et outils", 12);
        addDemand(state, ShopType::Plant, "plantes de soin et fournitures d'infirmerie", 10);
        addDemand(state, ShopType::Consumable, "rations et consommables pour ouvriers et gardes", 8);
    }
    else if (recentEventDays > 0)
    {
        // Recent events keep the city busy but should not create a second artificial crisis.
        state.serviceQualityDelta -= std::min(3, recentEventDays);
    }

    state.prosperityShift = std::clamp(state.prosperityShift, -2, 1);
    state.effectiveWealth = wealthFromIndex(wealthIndex(state.structuralWealth) + state.prosperityShift);

    if (state.prosperityShift > 0)
    {
        state.generalBuyModifierPercent += 1;
        state.generalSellModifierPercent += 2;
        state.serviceQualityDelta += 2;
    }
    else if (state.prosperityShift < 0)
    {
        state.generalSellModifierPercent -= 2 * (-state.prosperityShift);
        state.serviceQualityDelta -= 3 * (-state.prosperityShift);
    }

    state.generalBuyModifierPercent = std::clamp(state.generalBuyModifierPercent, -4, 4);
    state.generalSellModifierPercent = std::clamp(state.generalSellModifierPercent, -10, 8);
    state.serviceQualityDelta = std::clamp(state.serviceQualityDelta, -20, 12);
    return state;
}

int CityEconomyDynamicsSystem::demandSellBonusPercent(const CityEconomicPulse& state, ShopType goodsType)
{
    int best = 0;
    for (const CityMarketDemand& demand : state.demands)
    {
        if (demand.goodsType == goodsType)
        {
            best = std::max(best, demand.sellBonusPercent);
        }
    }
    return std::clamp(best, 0, 18);
}

int CityEconomyDynamicsSystem::adjustedServiceQuality(int structuralQuality, const CityEconomicPulse& state)
{
    return std::clamp(structuralQuality + state.serviceQualityDelta, 20, 98);
}

int CityEconomyDynamicsSystem::serviceQualityFor(
    const std::string& cityId,
    int worldDay,
    int repairDaysRemaining,
    int recentEventDays,
    ShopType serviceType)
{
    const CityShopContext structural = CityProfileSystem::shopContextFor(cityId, serviceType);
    const CityEconomicPulse pulse = stateFor(cityId, worldDay, repairDaysRemaining, recentEventDays);
    return adjustedServiceQuality(structural.serviceQualityScore, pulse);
}

std::vector<std::string> CityEconomyDynamicsSystem::informationLines(
    const std::string& cityId,
    int worldDay,
    int repairDaysRemaining,
    int recentEventDays
)
{
    const CityGameplayProfile profile = CityProfileSystem::profileFor(cityId);
    const CityEconomicPulse state = stateFor(cityId, worldDay, repairDaysRemaining, recentEventDays);
    std::vector<std::string> lines;

    lines.push_back("Profil structurel : richesse " + CityProfileSystem::wealthLabel(profile.wealth)
        + " | sécurité " + CityProfileSystem::securityLabel(profile.security) + ".");
    lines.push_back("Sécurité locale : " + CityProfileSystem::securityDescription(profile.security) + ".");

    if (state.temporary)
    {
        lines.push_back("Conjoncture actuelle : " + state.headline + ".");
        lines.push_back("Richesse ressentie aujourd'hui : " + CityProfileSystem::wealthLabel(state.effectiveWealth)
            + " (base " + CityProfileSystem::wealthLabel(state.structuralWealth) + ").");
        if (state.remainingDays > 0)
        {
            lines.push_back("Horizon actuel : environ " + std::to_string(state.remainingDays)
                + " jour(s) avant que cette pression immédiate ne se dissipe ou soit réévaluée.");
        }
        lines.push_back("La richesse structurelle n'est pas remplacée : une crise ou un boom temporaire ne transforme pas définitivement l'identité économique de la ville.");
    }
    else
    {
        lines.push_back("Conjoncture actuelle : activité ordinaire, sans boom ni crise économique temporaire majeure.");
    }

    if (!profile.specializations.empty())
    {
        lines.push_back("Spécialités locales :");
        for (const CityEconomicSpecialization& specialization : profile.specializations)
        {
            lines.push_back("- " + specialization.label + " (niveau " + std::to_string(std::clamp(specialization.strength, 1, 3)) + "/3)");
        }
    }

    lines.push_back("Qualité locale des services (richesse, spécialisation et conjoncture combinées) :");
    for (ShopType serviceType : {ShopType::Blacksmith, ShopType::Alchemist, ShopType::Enchanter, ShopType::Transport})
    {
        const int quality = serviceQualityFor(cityId, worldDay, repairDaysRemaining, recentEventDays, serviceType);
        const CityShopContext structural = CityProfileSystem::shopContextFor(cityId, serviceType);
        std::string detail = "- " + serviceDisplayName(serviceType) + " : " + std::to_string(quality) + "/100 (" + serviceQualityLabel(quality) + ")";
        if (structural.specializationStrength > 0)
        {
            detail += " | spécialité niv. " + std::to_string(structural.specializationStrength) + "/3";
        }
        detail += ".";
        lines.push_back(detail);
    }

    if (!state.demands.empty())
    {
        lines.push_back("Demandes renforcées du moment — vendre au bon comptoir peut rapporter un peu plus :");
        for (const CityMarketDemand& demand : state.demands)
        {
            lines.push_back("- " + demand.label + " : jusqu'à +" + std::to_string(demand.sellBonusPercent) + "% de demande locale sur la bonne filière.");
        }
    }
    else
    {
        lines.push_back("Demandes renforcées : aucune catégorie précise n'est annoncée aujourd'hui.");
    }

    const std::vector<std::string> tradeLines = CityTradeFlowSystem::informationLines(cityId, worldDay, state.eventId);
    if (!tradeLines.empty())
    {
        lines.push_back("Flux commerciaux régionaux :");
        lines.insert(lines.end(), tradeLines.begin(), tradeLines.end());
    }
    else
    {
        lines.push_back("Flux commerciaux régionaux : aucun arrivage ou départ majeur signalé aujourd'hui.");
    }

    lines.push_back("Règle locale : riche ne veut pas dire meilleur partout ; une spécialité forte peut dépasser une ville plus riche dans son domaine.");
    return lines;
}
