// EN: ExchangeTransferSystem performs safe voluntary transfers between persistent characters.
// FR: ExchangeTransferSystem effectue les transferts volontaires sûrs entre personnages persistants.
#ifndef INCLUDE_CORE_EXCHANGETRANSFERSYSTEM_HPP
#define INCLUDE_CORE_EXCHANGETRANSFERSYSTEM_HPP

class Player;

namespace ExchangeTransferSystem
{
    bool transferEconomyUnits(Player& giver, Player& receiver, int amount);
    bool transferWeapon(Player& giver, Player& receiver, int index);
    bool transferArmor(Player& giver, Player& receiver, int index);
    bool transferConsumable(Player& giver, Player& receiver, int index);
    bool transferMaterial(Player& giver, Player& receiver, int index, int quantity);
}

#endif
