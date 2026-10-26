#include "world/CityEconomyDynamicsSystem.hpp"
#include "world/CityProfileSystem.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "entity/Player.hpp"

#include <cassert>
#include <iostream>
#include <string>

int main()
{
    // Week index 1 starts with the local monster hunt.
    const CityEconomicPulse hunt = CityEconomyDynamicsSystem::stateFor("valebrume", 7, 0, 0);
    assert(hunt.temporary);
    assert(hunt.eventId == "monster_hunt");
    assert(CityEconomyDynamicsSystem::demandSellBonusPercent(hunt, ShopType::MonsterMaterial) >= 10);

    // Two days before the following fair, the city already starts buying relevant supplies.
    const CityEconomicPulse fairPreparation = CityEconomyDynamicsSystem::stateFor("valebrume", 12, 0, 0);
    assert(fairPreparation.temporary);
    assert(fairPreparation.eventId == "merchant_fair_preparation");
    assert(CityEconomyDynamicsSystem::demandSellBonusPercent(fairPreparation, ShopType::Material) > 0);
    assert(fairPreparation.effectiveWealth >= fairPreparation.structuralWealth);

    // Actual reconstruction temporarily lowers current prosperity, but never rewrites the structural tier.
    const CityEconomicPulse repair = CityEconomyDynamicsSystem::stateFor("port_lanterne", 14, 6, 0);
    assert(repair.eventId == "reconstruction");
    assert(repair.structuralWealth == CityWealthTier::Rich);
    assert(repair.effectiveWealth != repair.structuralWealth);
    assert(repair.prosperityShift < 0);
    assert(CityEconomyDynamicsSystem::demandSellBonusPercent(repair, ShopType::Material) >= 10);
    assert(CityEconomyDynamicsSystem::adjustedServiceQuality(70, repair) < 70);
    const int portTransportDuringRepair = CityEconomyDynamicsSystem::serviceQualityFor(
        "port_lanterne", 14, 6, 0, ShopType::Transport);
    const int portTransportStable = CityEconomyDynamicsSystem::serviceQualityFor(
        "port_lanterne", 17, 0, 0, ShopType::Transport);
    assert(portTransportDuringRepair < portTransportStable);

    // Specialization remains more important than raw city wealth for the service it actually masters.
    const int rocheveilleForge = CityEconomyDynamicsSystem::serviceQualityFor(
        "rocheveille", 17, 0, 0, ShopType::Blacksmith);
    const int rocheveilleAlchemy = CityEconomyDynamicsSystem::serviceQualityFor(
        "rocheveille", 17, 0, 0, ShopType::Alchemist);
    const int portTransport = CityEconomyDynamicsSystem::serviceQualityFor(
        "port_lanterne", 17, 0, 0, ShopType::Transport);
    const int portForge = CityEconomyDynamicsSystem::serviceQualityFor(
        "port_lanterne", 17, 0, 0, ShopType::Blacksmith);
    assert(rocheveilleForge > rocheveilleAlchemy);
    assert(portTransport > portForge);

    // Once repairs are over and the weekly pressure is gone, the structural city economy is the anchor again.
    const CityEconomicPulse recovered = CityEconomyDynamicsSystem::stateFor("port_lanterne", 17, 0, 0);
    assert(recovered.structuralWealth == CityWealthTier::Rich);
    assert(recovered.effectiveWealth == CityWealthTier::Rich);

    // Information desk must expose structure + temporary context without claiming rich = universally better.
    const CityEconomicPulse collection = CityEconomyDynamicsSystem::stateFor("valebrume", 56, 0, 0);
    assert(collection.eventId == "community_collection");
    assert(CityEconomyDynamicsSystem::demandSellBonusPercent(collection, ShopType::Material) >= 8);
    assert(CityEconomyDynamicsSystem::demandSellBonusPercent(collection, ShopType::Consumable) >= 6);

    const CityEconomicPulse raidPreparation = CityEconomyDynamicsSystem::stateFor("rocheveille", 61, 0, 0);
    assert(raidPreparation.eventId == "raid_defense_preparation");
    assert(CityEconomyDynamicsSystem::demandSellBonusPercent(raidPreparation, ShopType::Material) >= 12);
    assert(CityEconomyDynamicsSystem::demandSellBonusPercent(raidPreparation, ShopType::Weapon) >= 10);
    assert(CityEconomyDynamicsSystem::demandSellBonusPercent(raidPreparation, ShopType::Armor) >= 10);
    assert(CityEconomyDynamicsSystem::demandSellBonusPercent(raidPreparation, ShopType::Blacksmith) >= 10);

    const auto info = CityEconomyDynamicsSystem::informationLines("rocheveille", 12, 0, 0);
    std::string joined;
    for (const auto& line : info) joined += line + "\n";
    assert(joined.find("Profil structurel") != std::string::npos);
    assert(joined.find("Spécialités locales") != std::string::npos);
    assert(joined.find("Qualité locale des services") != std::string::npos);
    assert(joined.find("Forge / réparations") != std::string::npos);
    assert(joined.find("Transport") != std::string::npos);
    assert(joined.find("riche ne veut pas dire meilleur partout") != std::string::npos);

    // The correct buyer matters for materials. Monster components are best sold to the guild/specialist
    // collection channel (ShopType::MonsterMaterial) rather than a general material counter.
    Player seller;
    seller.setCurrentCityId("valebrume");
    seller.getInventory().addMaterial(MaterialCatalog::createById("wolf_fang", 1));
    const int fangIndex = seller.getInventory().findMaterialIndexById("wolf_fang");
    assert(fangIndex >= 0);
    const int specialistPrice = ShopTransactionSystem::getSellPriceForEntry(seller, ShopType::MonsterMaterial, fangIndex);
    const int genericPrice = ShopTransactionSystem::getSellPriceForEntry(seller, ShopType::Material, fangIndex);
    assert(specialistPrice > genericPrice);

    std::cout << "CityEconomyDynamicsSystemTest OK\n";
    return 0;
}
