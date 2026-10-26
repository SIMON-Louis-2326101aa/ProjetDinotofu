#include "world/CityProfileSystem.hpp"

#include <algorithm>

namespace
{
    const std::vector<CityGameplayProfile>& catalog()
    {
        static const std::vector<CityGameplayProfile> profiles = {
            {
                "valebrume",
                CitySecurityTier::Watched,
                CityWealthTier::Modest,
                {
                    {ShopType::Material, 2, "artisanat et matériaux de route"},
                    {ShopType::Plant, 2, "plantes et ressources locales"},
                    {ShopType::Consumable, 1, "survie et fournitures simples"},
                    {ShopType::Lodging, 1, "services d'aventuriers"}
                }
            },
            {
                "rocheveille",
                CitySecurityTier::HighlySecured,
                CityWealthTier::Modest,
                {
                    {ShopType::Weapon, 3, "armes et forge lourde"},
                    {ShopType::Armor, 3, "armures et protection minière"},
                    {ShopType::Blacksmith, 3, "forge, entretien et réparations"},
                    {ShopType::Material, 2, "minerais et pièces mécaniques"}
                }
            },
            {
                "port_lanterne",
                CitySecurityTier::NeutralIndependent,
                CityWealthTier::Rich,
                {
                    {ShopType::Transport, 3, "transport, caravanes et routes marchandes"},
                    {ShopType::Material, 2, "marchandises importées"},
                    {ShopType::CityService, 2, "services marchands et administratifs"},
                    {ShopType::BlackMarket, 2, "réseaux parallèles et contrebande"},
                    {ShopType::Consumable, 1, "produits de passage"}
                }
            },
            {
                "lys_de_givre",
                CitySecurityTier::HighlySecured,
                CityWealthTier::Rich,
                {
                    {ShopType::Enchanter, 3, "enchantement et magie froide"},
                    {ShopType::Alchemist, 3, "soins rares et alchimie boréale"},
                    {ShopType::Church, 2, "soins et rites spécialisés"},
                    {ShopType::Material, 2, "composants magiques et d'expédition"},
                    {ShopType::Lodging, 2, "hébergement d'expédition"}
                }
            }
        };
        return profiles;
    }

    int wealthBuyModifier(CityWealthTier tier)
    {
        switch (tier)
        {
            case CityWealthTier::Poor: return -8;
            case CityWealthTier::Modest: return -3;
            case CityWealthTier::Comfortable: return 0;
            case CityWealthTier::Rich: return 5;
            case CityWealthTier::Opulent: return 9;
        }
        return 0;
    }

    int wealthSellModifier(CityWealthTier tier)
    {
        switch (tier)
        {
            case CityWealthTier::Poor: return -8;
            case CityWealthTier::Modest: return -3;
            case CityWealthTier::Comfortable: return 0;
            case CityWealthTier::Rich: return 6;
            case CityWealthTier::Opulent: return 10;
        }
        return 0;
    }

    int wealthQualityBase(CityWealthTier tier)
    {
        switch (tier)
        {
            case CityWealthTier::Poor: return 36;
            case CityWealthTier::Modest: return 46;
            case CityWealthTier::Comfortable: return 56;
            case CityWealthTier::Rich: return 68;
            case CityWealthTier::Opulent: return 78;
        }
        return 50;
    }

    const CityEconomicSpecialization* findSpecialization(const CityGameplayProfile& profile, ShopType type)
    {
        for (const CityEconomicSpecialization& specialization : profile.specializations)
        {
            if (specialization.shopType == type)
            {
                return &specialization;
            }
        }
        return nullptr;
    }
}

CityGameplayProfile CityProfileSystem::profileFor(const std::string& cityId)
{
    for (const CityGameplayProfile& profile : catalog())
    {
        if (profile.cityId == cityId)
        {
            return profile;
        }
    }

    CityGameplayProfile fallback;
    fallback.cityId = cityId.empty() ? "unknown_region" : cityId;
    fallback.security = CitySecurityTier::NeutralIndependent;
    fallback.wealth = CityWealthTier::Comfortable;
    return fallback;
}

CityShopContext CityProfileSystem::shopContextFor(const std::string& cityId, ShopType shopType)
{
    const CityGameplayProfile profile = profileFor(cityId);
    CityShopContext context;
    context.buyPriceModifierPercent = wealthBuyModifier(profile.wealth);
    context.sellPriceModifierPercent = wealthSellModifier(profile.wealth);
    context.serviceQualityScore = wealthQualityBase(profile.wealth);

    if (const CityEconomicSpecialization* specialization = findSpecialization(profile, shopType))
    {
        context.specializationStrength = std::clamp(specialization->strength, 1, 3);
        context.specializationLabel = specialization->label;

        // Specialization is intentionally stronger than wealth: a modest mining city may beat a
        // rich generalist city for weapons, repairs or armor because that is what it actually does well.
        static const int buyDiscounts[] = {0, 4, 8, 12};
        static const int sellBonuses[] = {0, 3, 6, 9};
        static const int qualityBonuses[] = {0, 12, 25, 40};
        context.buyPriceModifierPercent -= buyDiscounts[context.specializationStrength];
        context.sellPriceModifierPercent += sellBonuses[context.specializationStrength];
        context.serviceQualityScore += qualityBonuses[context.specializationStrength];
    }

    context.buyPriceModifierPercent = std::clamp(context.buyPriceModifierPercent, -18, 18);
    context.sellPriceModifierPercent = std::clamp(context.sellPriceModifierPercent, -15, 18);
    context.serviceQualityScore = std::clamp(context.serviceQualityScore, 20, 98);
    return context;
}

CityBountyEnvironment CityProfileSystem::bountyEnvironmentFor(const std::string& cityId)
{
    const CityGameplayProfile profile = profileFor(cityId);
    CityBountyEnvironment environment;

    switch (profile.security)
    {
        case CitySecurityTier::HighlySecured:
            environment.awarenessDelta = 7;
            environment.localSupportDelta = -16;
            environment.resourcesDelta = -5;
            environment.mobilityDelta = -14;
            break;
        case CitySecurityTier::Watched:
            environment.awarenessDelta = 4;
            environment.localSupportDelta = -8;
            environment.resourcesDelta = -2;
            environment.mobilityDelta = -5;
            break;
        case CitySecurityTier::NeutralIndependent:
            break;
        case CitySecurityTier::Unstable:
            environment.awarenessDelta = 2;
            environment.localSupportDelta = 7;
            environment.resourcesDelta = 4;
            environment.mobilityDelta = 6;
            break;
        case CitySecurityTier::ChaoticAnarchic:
            environment.awarenessDelta = 4;
            environment.localSupportDelta = 14;
            environment.resourcesDelta = 7;
            environment.mobilityDelta = 12;
            break;
    }

    return environment;
}

std::string CityProfileSystem::securityLabel(CitySecurityTier tier)
{
    switch (tier)
    {
        case CitySecurityTier::HighlySecured: return "très sécurisée";
        case CitySecurityTier::Watched: return "surveillée";
        case CitySecurityTier::NeutralIndependent: return "neutre / indépendante";
        case CitySecurityTier::Unstable: return "instable";
        case CitySecurityTier::ChaoticAnarchic: return "anarchique / chaotique";
    }
    return "inconnue";
}

std::string CityProfileSystem::wealthLabel(CityWealthTier tier)
{
    switch (tier)
    {
        case CityWealthTier::Poor: return "pauvre";
        case CityWealthTier::Modest: return "modeste";
        case CityWealthTier::Comfortable: return "aisée";
        case CityWealthTier::Rich: return "riche";
        case CityWealthTier::Opulent: return "opulente";
    }
    return "inconnue";
}

std::string CityProfileSystem::securityDescription(CitySecurityTier tier)
{
    switch (tier)
    {
        case CitySecurityTier::HighlySecured: return "gardes nombreux, contrôles réguliers et refuges criminels difficiles à tenir";
        case CitySecurityTier::Watched: return "présence de garde réelle, routes suivies et activités suspectes plus risquées";
        case CitySecurityTier::NeutralIndependent: return "ordre local limité : ni refuge facile, ni filet de surveillance permanent";
        case CitySecurityTier::Unstable: return "contrôle irrégulier, autorités débordées et réseaux locaux plus libres";
        case CitySecurityTier::ChaoticAnarchic: return "ordre fragmenté, caches nombreuses et poursuites difficiles à verrouiller";
    }
    return "profil de sécurité inconnu";
}

std::string CityProfileSystem::wealthDescription(CityWealthTier tier)
{
    switch (tier)
    {
        case CityWealthTier::Poor: return "prix souvent bas mais services généralistes limités";
        case CityWealthTier::Modest: return "coûts contenus et qualité très dépendante des savoir-faire locaux";
        case CityWealthTier::Comfortable: return "prix et qualité globalement équilibrés";
        case CityWealthTier::Rich: return "services généralistes solides mais coût de vie plus élevé";
        case CityWealthTier::Opulent: return "forte capacité de service, avec tarifs élevés hors spécialités";
    }
    return "niveau économique inconnu";
}

std::string CityProfileSystem::profileSummary(const std::string& cityId)
{
    const CityGameplayProfile profile = profileFor(cityId);
    return "Sécurité : " + securityLabel(profile.security)
        + " | Économie : " + wealthLabel(profile.wealth)
        + ". Une spécialité locale peut dépasser largement ce que la richesse seule laisserait prévoir.";
}
