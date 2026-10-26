// EN: Shared confirmation helpers for inventory locks in player-triggered actions.
// FR: Helpers de confirmation partagés pour les verrous d'inventaire lors d'actions volontaires du joueur.
#include "interface/menu/inventory/InventoryProtection.hpp"

#include "entity/Player.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "item/Inventory.hpp"

namespace InventoryProtection
{
    bool confirmLockedItemUse(
        Player& player,
        const std::string& itemKey,
        const std::string& itemLabel,
        const std::string& actionLabel,
        const std::string& screenId
    )
    {
        if (itemKey.empty() || !player.getInventory().isLockedItemKey(itemKey))
        {
            return true;
        }

        MenuScreen screen("OBJET VERROUILLÉ", screenId);
        screen.addLine("Objet : " + itemLabel + ".");
        screen.addLine("Action : " + actionLabel + ".");
        screen.addLine("Le verrou protège cet objet contre une vente ou une utilisation accidentelle.");
        screen.addOption(1, "Confirmer exceptionnellement", "L'action est autorisée une fois ; le verrou reste actif ensuite.", true, "confirm_once");
        screen.addOption(2, "Annuler", "Conserver l'objet et son verrou.", true, "cancel");
        return TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis 1 ou 2.") == 1;
    }

    bool confirmLockedMaterialUse(
        Player& player,
        const std::string& materialId,
        const std::string& materialLabel,
        const std::string& actionLabel,
        const std::string& screenId
    )
    {
        return confirmLockedItemUse(
            player,
            Inventory::materialItemKeyById(materialId),
            materialLabel,
            actionLabel,
            screenId
        );
    }
}
