// EN: ExchangeTransferSystem centralizes voluntary transfer order and durable provenance.
// FR: ExchangeTransferSystem centralise l'ordre des transferts volontaires et leur provenance durable.
#include "core/ExchangeTransferSystem.hpp"

#include "economy/Money.hpp"
#include "entity/Player.hpp"
#include "item/armor/Armor.hpp"
#include "item/consumable/Consumable.hpp"
#include "item/material/Material.hpp"
#include "item/weapon/Weapon.hpp"

#include <string>

namespace
{
    std::string uniqueItemSubjectId(const std::string& prefix, const Item& item)
    {
        if (!item.getPersistentId().empty())
        {
            return item.getPersistentId();
        }
        return prefix + ":" + item.getName();
    }

    std::string materialSubjectId(const Material& material)
    {
        return "material:" + material.getId() + ":" + material.getQuality();
    }

    std::string partnerKey(const std::string& subjectId, const std::string& direction, const Player& partner)
    {
        return subjectId + "|" + direction + ":" + partner.getName();
    }

    void recordExchangeSummary(
        Player& giver,
        Player& receiver,
        const std::string& subjectId,
        const std::string& displayName,
        int amount
    )
    {
        if (subjectId.empty() || displayName.empty() || amount <= 0)
        {
            return;
        }

        giver.recordCanonicalEvent(
            "echanges_donnes",
            partnerKey(subjectId, "to", receiver),
            displayName + " donné à " + receiver.getName(),
            amount
        );
        receiver.recordCanonicalEvent(
            "echanges_recus",
            partnerKey(subjectId, "from", giver),
            displayName + " reçu de " + giver.getName(),
            amount
        );
    }

    void recordTransferHistory(
        Player& giver,
        Player& receiver,
        const std::string& subjectId,
        const std::string& detail
    )
    {
        if (subjectId.empty() || detail.empty())
        {
            return;
        }

        giver.recordHistoricalEvent(
            "exchange_given",
            subjectId,
            detail + " donné à " + receiver.getName() + ".",
            true
        );
        receiver.recordHistoricalEvent(
            "exchange_received",
            subjectId,
            detail + " reçu de " + giver.getName() + ".",
            true
        );
    }
}

bool ExchangeTransferSystem::transferEconomyUnits(Player& giver, Player& receiver, int amount)
{
    if (amount <= 0 || !giver.getInventory().spendEconomyUnits(amount))
    {
        return false;
    }

    receiver.getInventory().earnEconomyUnits(amount);
    giver.refreshCurrencyTitles();
    receiver.refreshCurrencyTitles();

    recordExchangeSummary(
        giver,
        receiver,
        "currency:economy_units",
        "Transfert d'argent",
        1
    );
    recordTransferHistory(giver, receiver, "currency:economy_units", Money::formatEconomyUnits(amount));
    return true;
}

bool ExchangeTransferSystem::transferWeapon(Player& giver, Player& receiver, int index)
{
    if (!giver.getInventory().hasWeapon(index))
    {
        return false;
    }

    Weapon weapon = giver.getInventory().getWeapon(index);
    if (!weapon.canChangeOwner())
    {
        return false;
    }
    weapon.ensurePersistentId("weapon");
    const std::string subjectId = uniqueItemSubjectId("weapon", weapon);

    if (!giver.getInventory().removeWeapon(index))
    {
        return false;
    }

    receiver.getInventory().addWeapon(weapon);
    recordExchangeSummary(giver, receiver, subjectId, weapon.getName() + " (arme)", 1);
    recordTransferHistory(giver, receiver, subjectId, weapon.getName() + " (arme)");
    return true;
}

bool ExchangeTransferSystem::transferArmor(Player& giver, Player& receiver, int index)
{
    if (!giver.getInventory().hasArmor(index))
    {
        return false;
    }

    Armor armor = giver.getInventory().getArmor(index);
    if (!armor.canChangeOwner())
    {
        return false;
    }
    armor.ensurePersistentId("armor");
    const std::string subjectId = uniqueItemSubjectId("armor", armor);

    if (!giver.getInventory().removeArmor(index))
    {
        return false;
    }

    receiver.getInventory().addArmor(armor);
    recordExchangeSummary(giver, receiver, subjectId, armor.getName() + " (armure)", 1);
    recordTransferHistory(giver, receiver, subjectId, armor.getName() + " (armure)");
    return true;
}

bool ExchangeTransferSystem::transferConsumable(Player& giver, Player& receiver, int index)
{
    if (!giver.getInventory().hasConsumable(index))
    {
        return false;
    }

    Consumable consumable = giver.getInventory().getConsumable(index);
    if (!consumable.canChangeOwner())
    {
        return false;
    }
    const std::string subjectId = "consumable:" + consumable.getName();

    if (!giver.getInventory().removeConsumable(index))
    {
        return false;
    }

    receiver.getInventory().addConsumable(consumable);
    recordExchangeSummary(giver, receiver, subjectId, consumable.getName() + " (consommable)", 1);
    recordTransferHistory(giver, receiver, subjectId, consumable.getName() + " (consommable)");
    return true;
}

bool ExchangeTransferSystem::transferMaterial(Player& giver, Player& receiver, int index, int quantity)
{
    if (!giver.getInventory().hasMaterial(index) || quantity <= 0)
    {
        return false;
    }

    Material material = giver.getInventory().getMaterial(index);
    if (!material.canChangeOwner())
    {
        return false;
    }
    if (quantity > material.getQuantity())
    {
        return false;
    }

    const std::string subjectId = materialSubjectId(material);
    material.setQuantity(quantity);

    if (!giver.getInventory().removeMaterialQuantity(index, quantity))
    {
        return false;
    }

    receiver.getInventory().addMaterial(material);
    const std::string qualitySuffix = material.hasSpecialQuality() ? " [" + material.getQualityLabel() + "]" : "";
    recordExchangeSummary(giver, receiver, subjectId, material.getName() + qualitySuffix, quantity);
    recordTransferHistory(
        giver,
        receiver,
        subjectId,
        material.getName() + qualitySuffix + " x" + std::to_string(quantity)
    );
    return true;
}
