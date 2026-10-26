// EN: Shared confirmation helpers for inventory locks in player-triggered actions.
// FR: Helpers de confirmation partagés pour les verrous d'inventaire lors d'actions volontaires du joueur.
#ifndef INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYPROTECTION_HPP
#define INCLUDE_INTERFACE_MENU_INVENTORY_INVENTORYPROTECTION_HPP

#include <string>

class Player;

namespace InventoryProtection
{
    bool confirmLockedItemUse(
        Player& player,
        const std::string& itemKey,
        const std::string& itemLabel,
        const std::string& actionLabel,
        const std::string& screenId = "inventory.lock.confirm"
    );

    bool confirmLockedMaterialUse(
        Player& player,
        const std::string& materialId,
        const std::string& materialLabel,
        const std::string& actionLabel,
        const std::string& screenId = "inventory.lock.material.confirm"
    );
}

#endif
