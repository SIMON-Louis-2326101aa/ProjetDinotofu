#include "world/CityProfileSystem.hpp"

#include <cassert>
#include <iostream>
#include <string>

int main()
{
    const CityGameplayProfile rocheveille = CityProfileSystem::profileFor("rocheveille");
    const CityGameplayProfile port = CityProfileSystem::profileFor("port_lanterne");
    assert(rocheveille.wealth == CityWealthTier::Modest);
    assert(rocheveille.security == CitySecurityTier::HighlySecured);
    assert(port.wealth == CityWealthTier::Rich);
    assert(port.security == CitySecurityTier::NeutralIndependent);

    // Core design rule: richer does NOT mean better at everything.
    const CityShopContext rocheveilleWeapon = CityProfileSystem::shopContextFor("rocheveille", ShopType::Weapon);
    const CityShopContext portWeapon = CityProfileSystem::shopContextFor("port_lanterne", ShopType::Weapon);
    assert(rocheveilleWeapon.specializationStrength == 3);
    assert(rocheveilleWeapon.buyPriceModifierPercent < portWeapon.buyPriceModifierPercent);
    assert(rocheveilleWeapon.serviceQualityScore > portWeapon.serviceQualityScore);

    // A rich city can still be excellent when the service matches its own identity.
    const CityShopContext portTransport = CityProfileSystem::shopContextFor("port_lanterne", ShopType::Transport);
    assert(portTransport.specializationStrength == 3);
    assert(portTransport.serviceQualityScore > portWeapon.serviceQualityScore);

    // Security changes the actual bounty environment rather than being only a label.
    const CityBountyEnvironment guarded = CityProfileSystem::bountyEnvironmentFor("rocheveille");
    const CityBountyEnvironment neutral = CityProfileSystem::bountyEnvironmentFor("port_lanterne");
    assert(guarded.localSupportDelta < neutral.localSupportDelta);
    assert(guarded.mobilityDelta < neutral.mobilityDelta);
    assert(guarded.awarenessDelta > neutral.awarenessDelta);

    const std::string summary = CityProfileSystem::profileSummary("rocheveille");
    assert(summary.find("Sécurité") != std::string::npos);
    assert(summary.find("Économie") != std::string::npos);

    std::cout << "CityProfileSystemTest OK\n";
    return 0;
}
