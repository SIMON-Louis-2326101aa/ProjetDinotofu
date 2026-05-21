// EN: QuestCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: QuestCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Provides guild quests and personal NPC requests.

#ifndef INCLUDE_QUEST_QUESTCATALOG_HPP
#define INCLUDE_QUEST_QUESTCATALOG_HPP

#include "quest/Quest.hpp"

#include <string>
#include <vector>

class QuestCatalog
{
public:
    // EN: createGuildBoard declares or implements a focused behavior used by this module.
    // FR: createGuildBoard déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<Quest> createGuildBoard(int playerLevel);
    // EN: createForgemasterMaterialRequest declares or implements a focused behavior used by this module.
    // FR: createForgemasterMaterialRequest déclare ou implémente un comportement précis utilisé par ce module.
    static Quest createForgemasterMaterialRequest(int playerLevel);
    // EN: createAlchemistIngredientRequest declares or implements a focused behavior used by this module.
    // FR: createAlchemistIngredientRequest déclare ou implémente un comportement précis utilisé par ce module.
    static Quest createAlchemistIngredientRequest(int playerLevel);
    // EN: createVillagerMonsterFearRequest declares or implements a focused behavior used by this module.
    // FR: createVillagerMonsterFearRequest déclare ou implémente un comportement précis utilisé par ce module.
    static Quest createVillagerMonsterFearRequest(int playerLevel);
    // EN: createMerchantDeliveryRequest declares or implements a focused behavior used by this module.
    // FR: createMerchantDeliveryRequest déclare ou implémente un comportement précis utilisé par ce module.
    static Quest createMerchantDeliveryRequest(int playerLevel);
    // EN: createMonsterMaterialVendorRequest declares or implements a focused behavior used by this module.
    // FR: createMonsterMaterialVendorRequest déclare ou implémente un comportement précis utilisé par ce module.
    static Quest createMonsterMaterialVendorRequest(int playerLevel);
    // EN: createMaterialVendorRequest declares or implements a focused behavior used by this module.
    // FR: createMaterialVendorRequest déclare ou implémente un comportement précis utilisé par ce module.
    static Quest createMaterialVendorRequest(int playerLevel);
    // EN: createHerbalistRequest declares or implements a focused behavior used by this module.
    // FR: createHerbalistRequest déclare ou implémente un comportement précis utilisé par ce module.
    static Quest createHerbalistRequest(int playerLevel);
    // EN: createArmorerRequest declares or implements a focused behavior used by this module.
    // FR: createArmorerRequest déclare ou implémente un comportement précis utilisé par ce module.
    static Quest createArmorerRequest(int playerLevel);
    // EN: createWeaponVendorRequest declares or implements a focused behavior used by this module.
    // FR: createWeaponVendorRequest déclare ou implémente un comportement précis utilisé par ce module.
    static Quest createWeaponVendorRequest(int playerLevel);
    // EN: createConsumableVendorRequest declares or implements a focused behavior used by this module.
    // FR: createConsumableVendorRequest déclare ou implémente un comportement précis utilisé par ce module.
    static Quest createConsumableVendorRequest(int playerLevel);
    // EN: createLibrarianRequest declares or implements a focused behavior used by this module.
    // FR: createLibrarianRequest déclare ou implémente un comportement précis utilisé par ce module.
    static Quest createLibrarianRequest(int playerLevel);
    static Quest createBiomeRequest(int playerLevel, const std::string& biomeName, const std::string& preferredClient = "");
};

#endif
