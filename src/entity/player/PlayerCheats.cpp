// EN: Split from Player.cpp to keep Player responsibilities maintainable.
// FR: Extrait de Player.cpp afin de garder des responsabilités Player maintenables.
#include "entity/Player.hpp"
#include "progression/language/LanguageSystem.hpp"
#include "economy/EconomyBalance.hpp"
#include "core/VersionInfo.hpp"
#include "economy/Money.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "progression/DifficultyRules.hpp"
#include "progression/Level.hpp"
#include "progression/TitleCatalog.hpp"
#include "character/RaceCatalog.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "item/equipment/EquipmentWeightRules.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <random>
#include <sstream>
#include <tuple>
#include <vector>

bool Player::isAlteredByCheats() const
{
    return alteredByCheats;
}

std::vector<std::string> Player::describeCheatAndProtocolHistory() const
{
    std::vector<std::string> lines;
    auto findUsage = [&](const std::string& category, const std::string& key) -> const PlayerJournalRecord* {
        for (const PlayerJournalRecord& record : canonicalJournalRecords)
        {
            if (record.category == category && record.key == key && record.count > 0) return &record;
        }
        return nullptr;
    };

    auto statusFor = [&](const std::string& code, const PlayerJournalRecord& record) {
        if (code == "idontwanttodie") return std::string(godModeEnabled ? "Actif" : "Inactif");
        if (code == "infinituseforeverything") return std::string(infiniteConsumablesEnabled ? "Actif" : "Inactif");
        if (code == "bedrockequipment") return std::string(indestructibleEquipmentEnabled ? "Actif" : "Inactif");
        if (code == "myweaponandnotother") return std::string(equipmentProtectionEnabled ? "Actif" : "Inactif");
        if (code == "skipallstory") return std::string(storySkipEnabled ? "Actif" : "Inactif");
        if (code == "helpmerefundmyaction") return std::string("Ponctuel — ") + std::to_string(refundUsesRemaining) + "/3 aide(s) restante(s)";
        if (code == "letmecook") return std::string(record.lastDay == worldDaysElapsed ? "Actif jusqu'au prochain jour" : "Inactif");
        if (code == "whereismyplotarmor") return std::string(hasBlessing("cheat_plot_armor") ? "Actif" : "Consommé ou indisponible");
        if (code == "iamnotgoblinfood") return std::string("Actif");
        if (code == "fuckgrindinggimifight") return std::string(specialChallengeAccessUnlocked ? "Actif" : "Connu / désactivé");
        return std::string("Ponctuel");
    };

    for (const PlayerJournalRecord& record : canonicalJournalRecords)
    {
        if (record.category != "cheat_code_usage" || record.key == "dinotofu:13-02313") continue;
        lines.push_back("- " + record.key + " | " + statusFor(record.key, record) + " | utilisation(s) : " + std::to_string(record.count));
    }

    for (const PlayerJournalRecord& record : canonicalJournalRecords)
    {
        if (record.category != "secret_code_usage" || record.key == "dinotofu:13-02313") continue;
        lines.push_back("- " + record.key + " | Secret non altérant | utilisation(s) : " + std::to_string(record.count));
    }

    const PlayerJournalRecord* protocol = findUsage("protocoles_secrets", "reapers13_protocol_13_signal");
    if (protocol != nullptr)
    {
        const bool alteredUse = findUsage("cheat_code_usage", "dinotofu:13-02313") != nullptr;
        lines.push_back(std::string("- DINOTOFU:13-02313 | Protocol 13 transmis | ")
            + (alteredUse ? "usage altérant faute de connaissance préalable" : "usage diégétique non altérant"));
    }

    return lines;
}

// EN: isGodModeEnabled declares or implements a focused behavior used by this module.
// FR: isGodModeEnabled déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isGodModeEnabled() const
{
    return godModeEnabled;
}

// EN: hasInfiniteConsumables declares or implements a focused behavior used by this module.
// FR: hasInfiniteConsumables déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasInfiniteConsumables() const
{
    return infiniteConsumablesEnabled;
}

// EN: hasIndestructibleEquipment declares or implements a focused behavior used by this module.
// FR: hasIndestructibleEquipment déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasIndestructibleEquipment() const
{
    return indestructibleEquipmentEnabled;
}

// EN: hasEquipmentProtection declares or implements a focused behavior used by this module.
// FR: hasEquipmentProtection déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasEquipmentProtection() const
{
    return equipmentProtectionEnabled;
}

// EN: hasStorySkip declares or implements a focused behavior used by this module.
// FR: hasStorySkip déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasStorySkip() const
{
    return storySkipEnabled;
}

// EN: hasSpecialChallengeAccess declares or implements a focused behavior used by this module.
// FR: hasSpecialChallengeAccess déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasSpecialChallengeAccess() const
{
    return specialChallengeAccessUnlocked;
}

// EN: getRefundUsesRemaining declares or implements a focused behavior used by this module.
// FR: getRefundUsesRemaining déclare ou implémente un comportement précis utilisé par ce module.
int Player::getRefundUsesRemaining() const
{
    return refundUsesRemaining;
}


// EN: isGodModeKnown declares or implements a focused behavior used by this module.
// FR: isGodModeKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isGodModeKnown() const
{
    return godModeKnown;
}

// EN: isInfiniteConsumablesKnown declares or implements a focused behavior used by this module.
// FR: isInfiniteConsumablesKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isInfiniteConsumablesKnown() const
{
    return infiniteConsumablesKnown;
}

// EN: isIndestructibleEquipmentKnown declares or implements a focused behavior used by this module.
// FR: isIndestructibleEquipmentKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isIndestructibleEquipmentKnown() const
{
    return indestructibleEquipmentKnown;
}

// EN: isEquipmentProtectionKnown declares or implements a focused behavior used by this module.
// FR: isEquipmentProtectionKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isEquipmentProtectionKnown() const
{
    return equipmentProtectionKnown;
}

// EN: isStorySkipKnown declares or implements a focused behavior used by this module.
// FR: isStorySkipKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isStorySkipKnown() const
{
    return storySkipKnown;
}

// EN: isSpecialChallengeAccessKnown declares or implements a focused behavior used by this module.
// FR: isSpecialChallengeAccessKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isSpecialChallengeAccessKnown() const
{
    return specialChallengeAccessKnown;
}

// EN: isCreatorMessageKnown declares or implements a focused behavior used by this module.
// FR: isCreatorMessageKnown déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isCreatorMessageKnown() const
{
    return creatorMessageKnown;
}

// EN: getGoldCheatUseCount declares or implements a focused behavior used by this module.
// FR: getGoldCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getGoldCheatUseCount() const
{
    return goldCheatUseCount;
}

// EN: getLevelCheatUseCount declares or implements a focused behavior used by this module.
// FR: getLevelCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getLevelCheatUseCount() const
{
    return levelCheatUseCount;
}

// EN: getMaxLevelCheatUseCount declares or implements a focused behavior used by this module.
// FR: getMaxLevelCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getMaxLevelCheatUseCount() const
{
    return maxLevelCheatUseCount;
}

// EN: getRefundCheatUseCount declares or implements a focused behavior used by this module.
// FR: getRefundCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getRefundCheatUseCount() const
{
    return refundCheatUseCount;
}

// EN: getResetCheatUseCount declares or implements a focused behavior used by this module.
// FR: getResetCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getResetCheatUseCount() const
{
    return resetCheatUseCount;
}

// EN: getSwitchClassCheatUseCount declares or implements a focused behavior used by this module.
// FR: getSwitchClassCheatUseCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getSwitchClassCheatUseCount() const
{
    return switchClassCheatUseCount;
}

void Player::setCheatState(
    bool altered,
    bool godMode,
    bool infiniteConsumables,
    bool indestructibleEquipment,
    bool equipmentProtection,
    bool storySkip,
    bool specialChallengeAccess,
    int refundUses
)
{
    alteredByCheats = false;
    if (altered)
    {
        markAsAlteredByCheats();
    }
    godModeEnabled = godMode;
    infiniteConsumablesEnabled = infiniteConsumables;
    indestructibleEquipmentEnabled = indestructibleEquipment;
    equipmentProtectionEnabled = equipmentProtection;
    storySkipEnabled = storySkip;
    specialChallengeAccessUnlocked = specialChallengeAccess;

    if (godMode)
    {
        godModeKnown = true;
    }

    if (infiniteConsumables)
    {
        infiniteConsumablesKnown = true;
    }

    if (indestructibleEquipment)
    {
        indestructibleEquipmentKnown = true;
    }

    if (equipmentProtection)
    {
        equipmentProtectionKnown = true;
    }

    if (storySkip)
    {
        storySkipKnown = true;
    }

    if (specialChallengeAccess)
    {
        specialChallengeAccessKnown = true;
    }

    if (refundUses < 0)
    {
        refundUses = 0;
    }

    if (refundUses > 3)
    {
        refundUses = 3;
    }

    refundUsesRemaining = refundUses;
}


void Player::setCheatKnowledgeState(
    bool godModeWasKnown,
    bool infiniteConsumablesWasKnown,
    bool indestructibleEquipmentWasKnown,
    bool equipmentProtectionWasKnown,
    bool storySkipWasKnown,
    bool specialChallengeAccessWasKnown,
    bool creatorMessageWasKnown
)
{
    godModeKnown = godModeWasKnown;
    infiniteConsumablesKnown = infiniteConsumablesWasKnown;
    indestructibleEquipmentKnown = indestructibleEquipmentWasKnown;
    equipmentProtectionKnown = equipmentProtectionWasKnown;
    storySkipKnown = storySkipWasKnown;
    specialChallengeAccessKnown = specialChallengeAccessWasKnown;
    creatorMessageKnown = creatorMessageWasKnown;

    if (godModeEnabled) godModeKnown = true;
    if (infiniteConsumablesEnabled) infiniteConsumablesKnown = true;
    if (indestructibleEquipmentEnabled) indestructibleEquipmentKnown = true;
    if (equipmentProtectionEnabled) equipmentProtectionKnown = true;
    if (storySkipEnabled) storySkipKnown = true;
}

// EN: markAsAlteredByCheats declares or implements a focused behavior used by this module.
// FR: markAsAlteredByCheats déclare ou implémente un comportement précis utilisé par ce module.
void Player::markAsAlteredByCheats()
{
    const bool wasAlreadyAltered = alteredByCheats;
    alteredByCheats = true;
    if (!wasAlreadyAltered)
    {
        grantCharacterOnlyTitleFromCheat("On ne dira rien...");
    }
}

// EN: toggleGodMode declares or implements a focused behavior used by this module.
// FR: toggleGodMode déclare ou implémente un comportement précis utilisé par ce module.
bool Player::toggleGodMode()
{
    markAsAlteredByCheats();
    godModeKnown = true;
    godModeEnabled = !godModeEnabled;
    return godModeEnabled;
}

// EN: toggleInfiniteConsumables declares or implements a focused behavior used by this module.
// FR: toggleInfiniteConsumables déclare ou implémente un comportement précis utilisé par ce module.
bool Player::toggleInfiniteConsumables()
{
    markAsAlteredByCheats();
    infiniteConsumablesKnown = true;
    infiniteConsumablesEnabled = !infiniteConsumablesEnabled;
    return infiniteConsumablesEnabled;
}

// EN: toggleIndestructibleEquipment declares or implements a focused behavior used by this module.
// FR: toggleIndestructibleEquipment déclare ou implémente un comportement précis utilisé par ce module.
bool Player::toggleIndestructibleEquipment()
{
    markAsAlteredByCheats();
    indestructibleEquipmentKnown = true;
    indestructibleEquipmentEnabled = !indestructibleEquipmentEnabled;
    return indestructibleEquipmentEnabled;
}

// EN: toggleEquipmentProtection declares or implements a focused behavior used by this module.
// FR: toggleEquipmentProtection déclare ou implémente un comportement précis utilisé par ce module.
bool Player::toggleEquipmentProtection()
{
    markAsAlteredByCheats();
    equipmentProtectionKnown = true;
    equipmentProtectionEnabled = !equipmentProtectionEnabled;
    return equipmentProtectionEnabled;
}

// EN: toggleStorySkip declares or implements a focused behavior used by this module.
// FR: toggleStorySkip déclare ou implémente un comportement précis utilisé par ce module.
bool Player::toggleStorySkip()
{
    markAsAlteredByCheats();
    storySkipKnown = true;
    storySkipEnabled = !storySkipEnabled;
    return storySkipEnabled;
}

// EN: unlockSpecialChallengeAccess declares or implements a focused behavior used by this module.
// FR: unlockSpecialChallengeAccess déclare ou implémente un comportement précis utilisé par ce module.
void Player::unlockSpecialChallengeAccess()
{
    markAsAlteredByCheats();
    specialChallengeAccessKnown = true;
    specialChallengeAccessUnlocked = true;
}

// EN: unlockBossRegistryExceptFinal declares or implements a focused behavior used by this module.
// FR: unlockBossRegistryExceptFinal déclare ou implémente un comportement précis utilisé par ce module.
void Player::unlockBossRegistryExceptFinal(int maximumBossId, int finalBossId)
{
    for (int id = 1; id <= maximumBossId; ++id)
    {
        if (id == finalBossId)
        {
            continue;
        }

        if (!isBossUnlocked(id))
        {
            unlockedBossIds.push_back(id);
        }
    }
}

// EN: enableGodMode declares or implements a focused behavior used by this module.
// FR: enableGodMode déclare ou implémente un comportement précis utilisé par ce module.
void Player::enableGodMode()
{
    markAsAlteredByCheats();
    godModeKnown = true;
    godModeEnabled = true;
}

// EN: enableInfiniteConsumables declares or implements a focused behavior used by this module.
// FR: enableInfiniteConsumables déclare ou implémente un comportement précis utilisé par ce module.
void Player::enableInfiniteConsumables()
{
    markAsAlteredByCheats();
    infiniteConsumablesKnown = true;
    infiniteConsumablesEnabled = true;
}

// EN: enableIndestructibleEquipment declares or implements a focused behavior used by this module.
// FR: enableIndestructibleEquipment déclare ou implémente un comportement précis utilisé par ce module.
void Player::enableIndestructibleEquipment()
{
    markAsAlteredByCheats();
    indestructibleEquipmentKnown = true;
    indestructibleEquipmentEnabled = true;
}

// EN: enableEquipmentProtection declares or implements a focused behavior used by this module.
// FR: enableEquipmentProtection déclare ou implémente un comportement précis utilisé par ce module.
void Player::enableEquipmentProtection()
{
    markAsAlteredByCheats();
    equipmentProtectionKnown = true;
    equipmentProtectionEnabled = true;
}

// EN: enableStorySkip declares or implements a focused behavior used by this module.
// FR: enableStorySkip déclare ou implémente un comportement précis utilisé par ce module.
void Player::enableStorySkip()
{
    markAsAlteredByCheats();
    storySkipKnown = true;
    storySkipEnabled = true;
}

bool Player::hasActiveCheatPower() const
{
    return godModeEnabled
        || infiniteConsumablesEnabled
        || indestructibleEquipmentEnabled
        || equipmentProtectionEnabled
        || storySkipEnabled
        || specialChallengeAccessUnlocked;
}

int Player::clearActiveCheatPowersForFireFlight()
{
    int cleared = 0;
    if (godModeEnabled) { godModeEnabled = false; ++cleared; }
    if (infiniteConsumablesEnabled) { infiniteConsumablesEnabled = false; ++cleared; }
    if (indestructibleEquipmentEnabled) { indestructibleEquipmentEnabled = false; ++cleared; }
    if (equipmentProtectionEnabled) { equipmentProtectionEnabled = false; ++cleared; }
    if (storySkipEnabled) { storySkipEnabled = false; ++cleared; }
    if (specialChallengeAccessUnlocked) { specialChallengeAccessUnlocked = false; ++cleared; }
    return cleared;
}

bool Player::disableGodModeForFireFlight()
{
    if (!godModeEnabled)
    {
        return false;
    }

    godModeEnabled = false;
    godModeKnown = true;
    return true;
}

// EN: markCreatorMessageSeen declares or implements a focused behavior used by this module.
// FR: markCreatorMessageSeen déclare ou implémente un comportement précis utilisé par ce module.
void Player::markCreatorMessageSeen()
{
    creatorMessageKnown = true;
}

// EN: recordGoldCheatUse declares or implements a focused behavior used by this module.
// FR: recordGoldCheatUse déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordGoldCheatUse()
{
    markAsAlteredByCheats();
    goldCheatUseCount++;
}

// EN: recordLevelCheatUse declares or implements a focused behavior used by this module.
// FR: recordLevelCheatUse déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordLevelCheatUse()
{
    markAsAlteredByCheats();
    levelCheatUseCount++;
}

// EN: recordMaxLevelCheatUse declares or implements a focused behavior used by this module.
// FR: recordMaxLevelCheatUse déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordMaxLevelCheatUse()
{
    markAsAlteredByCheats();
    maxLevelCheatUseCount++;
}

// EN: recordRefundCheatUse declares or implements a focused behavior used by this module.
// FR: recordRefundCheatUse déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordRefundCheatUse()
{
    markAsAlteredByCheats();
    refundCheatUseCount++;
}

// EN: recordResetCheatUse declares or implements a focused behavior used by this module.
// FR: recordResetCheatUse déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordResetCheatUse()
{
    markAsAlteredByCheats();
    resetCheatUseCount++;
}

// EN: recordSwitchClassCheatUse declares or implements a focused behavior used by this module.
// FR: recordSwitchClassCheatUse déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordSwitchClassCheatUse()
{
    markAsAlteredByCheats();
    switchClassCheatUseCount++;
}

// EN: consumeRefundUse declares or implements a focused behavior used by this module.
// FR: consumeRefundUse déclare ou implémente un comportement précis utilisé par ce module.
bool Player::consumeRefundUse()
{
    markAsAlteredByCheats();

    if (refundUsesRemaining <= 0)
    {
        return false;
    }

    refundUsesRemaining--;
    return true;
}

// EN: forceLevelToMaximum declares or implements a focused behavior used by this module.
// FR: forceLevelToMaximum déclare ou implémente un comportement précis utilisé par ce module.
void Player::forceLevelToMaximum()
{
    markAsAlteredByCheats();

    while (level < MAX_LEVEL)
    {
        levelUp();
    }

    experience = 0;
}

// EN: gainOneLevelByCheat declares or implements a focused behavior used by this module.
// FR: gainOneLevelByCheat déclare ou implémente un comportement précis utilisé par ce module.
void Player::gainOneLevelByCheat()
{
    markAsAlteredByCheats();
    levelUp();
}

// EN: takeDamage declares or implements a focused behavior used by this module.
// FR: takeDamage déclare ou implémente un comportement précis utilisé par ce module.
