#pragma once

#include "economy/shop/ShopType.hpp"

#include <string>
#include <vector>

enum class CitySecurityTier
{
    HighlySecured,
    Watched,
    NeutralIndependent,
    Unstable,
    ChaoticAnarchic
};

enum class CityWealthTier
{
    Poor,
    Modest,
    Comfortable,
    Rich,
    Opulent
};

struct CityEconomicSpecialization
{
    ShopType shopType = ShopType::Unknown;
    int strength = 0; // 1 local strength, 2 strong identity, 3 reference-level specialty.
    std::string label;
};

struct CityGameplayProfile
{
    std::string cityId;
    CitySecurityTier security = CitySecurityTier::NeutralIndependent;
    CityWealthTier wealth = CityWealthTier::Comfortable;
    std::vector<CityEconomicSpecialization> specializations;
};

struct CityShopContext
{
    int buyPriceModifierPercent = 0;
    int sellPriceModifierPercent = 0;
    int serviceQualityScore = 50; // 0..100. Wealth helps, specialization can matter much more.
    int specializationStrength = 0;
    std::string specializationLabel;
};

struct CityBountyEnvironment
{
    int awarenessDelta = 0;
    int localSupportDelta = 0;
    int resourcesDelta = 0;
    int mobilityDelta = 0;
};

namespace CityProfileSystem
{
    CityGameplayProfile profileFor(const std::string& cityId);
    CityShopContext shopContextFor(const std::string& cityId, ShopType shopType);
    CityBountyEnvironment bountyEnvironmentFor(const std::string& cityId);

    std::string securityLabel(CitySecurityTier tier);
    std::string wealthLabel(CityWealthTier tier);
    std::string securityDescription(CitySecurityTier tier);
    std::string wealthDescription(CityWealthTier tier);
    std::string profileSummary(const std::string& cityId);
}
