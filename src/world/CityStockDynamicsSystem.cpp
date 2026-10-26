#include "world/CityStockDynamicsSystem.hpp"

#include "world/CityProfileSystem.hpp"
#include "world/CityTradeFlowSystem.hpp"

#include <algorithm>
#include <cmath>

namespace
{
    int eventSupplyModifier(const CityEconomicPulse& pulse, ShopType type)
    {
        const std::string& id = pulse.eventId;

        if (id == "merchant_fair" || id == "merchant_fair_preparation")
        {
            if (type == ShopType::Material || type == ShopType::Plant || type == ShopType::Consumable || type == ShopType::Transport) return 24;
            return 12;
        }
        if (id == "community_collection" || id == "community_collection_preparation")
        {
            if (type == ShopType::Material || type == ShopType::Consumable) return 18;
            if (type == ShopType::Plant || type == ShopType::CityService) return 10;
        }
        if (id == "harvest_festival" || id == "harvest_festival_preparation")
        {
            if (type == ShopType::Plant || type == ShopType::Consumable) return 22;
            if (type == ShopType::Alchemist) return 12;
        }
        if (id == "monster_hunt")
        {
            if (type == ShopType::MonsterMaterial) return 24; // hunters are bringing trophies back into town.
            if (type == ShopType::Consumable || type == ShopType::Weapon) return -10;
        }
        if (id == "monster_hunt_preparation")
        {
            if (type == ShopType::Consumable || type == ShopType::Weapon) return -14;
        }
        if (id == "raid_defense_preparation" || id == "raid_defense_mobilization")
        {
            if (type == ShopType::Material) return -28;
            if (type == ShopType::Weapon || type == ShopType::Armor || type == ShopType::Blacksmith) return -24;
            if (type == ShopType::Consumable) return -12;
        }
        if (id == "reconstruction")
        {
            if (type == ShopType::Material || type == ShopType::Blacksmith) return -24;
            if (type == ShopType::Consumable || type == ShopType::Plant) return -10;
        }
        if (id == "guild_tournament" || id == "guild_tournament_preparation")
        {
            if (type == ShopType::Weapon || type == ShopType::Armor) return -8;
        }
        if (id == "village_games" || id == "village_games_preparation")
        {
            if (type == ShopType::Consumable || type == ShopType::Lodging) return 12;
        }
        return 0;
    }

    std::string modifierReason(int specializationStrength, const CityEconomicPulse& pulse, int eventModifier)
    {
        std::string reason;
        if (specializationStrength > 0)
        {
            reason = "spécialisation locale niv. " + std::to_string(specializationStrength) + "/3";
        }
        if (eventModifier != 0)
        {
            if (!reason.empty()) reason += " + ";
            reason += pulse.headline;
        }
        if (reason.empty()) reason = "stock local ordinaire";
        return reason;
    }
}

CityStockAdjustment CityStockDynamicsSystem::adjustmentFor(
    const std::string& cityId,
    const CityEconomicPulse& pulse,
    ShopType shopType)
{
    return adjustmentFor(cityId, -1, pulse, shopType);
}

CityStockAdjustment CityStockDynamicsSystem::adjustmentFor(
    const std::string& cityId,
    int worldDay,
    const CityEconomicPulse& pulse,
    ShopType shopType)
{
    CityStockAdjustment result;
    const CityShopContext context = CityProfileSystem::shopContextFor(cityId, shopType);
    result.specializationStrength = context.specializationStrength;

    static const int specializationStockBonus[] = {0, 14, 30, 48};
    const int specializationBonus = specializationStockBonus[std::clamp(result.specializationStrength, 0, 3)];
    int eventModifier = eventSupplyModifier(pulse, shopType);

    // A reference-level local industry absorbs shortages much better than a generalist town.
    if (eventModifier < 0 && result.specializationStrength > 0)
    {
        eventModifier += std::min(-eventModifier, result.specializationStrength * 5);
    }

    const CityTradePressure trade = worldDay >= 0
        ? CityTradeFlowSystem::pressureFor(cityId, worldDay, shopType, pulse.eventId)
        : CityTradePressure{};
    result.tradeStockModifierPercent = trade.netStockModifierPercent;
    result.stockModifierPercent = std::clamp(specializationBonus + eventModifier + result.tradeStockModifierPercent, -35, 65);
    result.reason = modifierReason(result.specializationStrength, pulse, eventModifier);
    if (worldDay >= 0 && (!trade.status.empty() || trade.netStockModifierPercent != 0))
    {
        result.reason += " + flux régional: " + (trade.status.empty() ? std::string("ajustement de convoi") : trade.status);
    }
    return result;
}

CityStockAdjustment CityStockDynamicsSystem::applyToShop(
    const std::string& cityId,
    int worldDay,
    int repairDaysRemaining,
    int recentEventDays,
    ShopInventory& shop)
{
    const CityEconomicPulse pulse = CityEconomyDynamicsSystem::stateFor(
        cityId, worldDay, repairDaysRemaining, recentEventDays);
    CityStockAdjustment result = adjustmentFor(cityId, worldDay, pulse, shop.getType());

    if (result.stockModifierPercent == 0) return result;

    for (ShopItem& item : shop.getMutableItems())
    {
        const int oldStock = item.getStock();
        if (oldStock < 0) continue; // unlimited services stay unlimited.
        if (oldStock == 0) continue; // sold-out/disabled entries must not be resurrected accidentally.

        int newStock = static_cast<int>(std::lround(oldStock * (100.0 + result.stockModifierPercent) / 100.0));
        newStock = std::max(0, newStock);

        // Strong shortages should be allowed to remove a genuinely scarce one-off item, but not every one-off.
        if (oldStock == 1 && result.stockModifierPercent <= -25)
        {
            const std::size_t h = std::hash<std::string>{}(cityId + "|" + pulse.eventId + "|" + item.getId());
            if ((h % 100) < static_cast<std::size_t>(-result.stockModifierPercent - 8)) newStock = 0;
        }

        while (item.getStock() > newStock && item.consumeOneStock()) {}
        if (item.getStock() >= 0 && item.getStock() < newStock) item.addStock(newStock - item.getStock());
        if (newStock != oldStock) ++result.finiteItemsTouched;
    }
    return result;
}

std::string CityStockDynamicsSystem::availabilityLine(
    const std::string& cityId,
    int worldDay,
    int repairDaysRemaining,
    int recentEventDays,
    ShopType shopType)
{
    const CityEconomicPulse pulse = CityEconomyDynamicsSystem::stateFor(
        cityId, worldDay, repairDaysRemaining, recentEventDays);
    const CityStockAdjustment adjustment = adjustmentFor(cityId, worldDay, pulse, shopType);

    if (adjustment.stockModifierPercent >= 35)
        return "Disponibilité locale : stock nettement renforcé (" + adjustment.reason + ").";
    if (adjustment.stockModifierPercent >= 12)
        return "Disponibilité locale : stock renforcé (" + adjustment.reason + ").";
    if (adjustment.stockModifierPercent <= -25)
        return "Disponibilité locale : forte tension / ruptures possibles (" + adjustment.reason + ").";
    if (adjustment.stockModifierPercent <= -8)
        return "Disponibilité locale : stock sous tension (" + adjustment.reason + ").";
    return "Disponibilité locale : stock proche de la normale (" + adjustment.reason + ").";
}
