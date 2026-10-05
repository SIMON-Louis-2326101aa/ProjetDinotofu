// EN: Player.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Player.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/Player.hpp"
#include "entity/player/PlayerUiSupport.hpp"
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


namespace
{

    bool canonicalCategoryIsLocalScoped(const std::string& category)
    {
        static const std::vector<std::string> localCategories = {
            "pnj_servis", "types_quetes_completees", "lieux_visites", "coffres",
            "reputation_locale_positive", "reputation_locale_negative",
            "services_locaux_reussis", "services_locaux_echoues", "avertissements_locaux", "rehabilitations_locales",
            "quetes_reussies", "quetes_echouees",
            "consultations_comptoir_mercenaire", "participation_recrues",
            "fuites_ennemies", "fugitifs_signales", "rivaux_confirmes", "rumeurs_rivaux"
        };
        return std::find(localCategories.begin(), localCategories.end(), category) != localCategories.end();
    }

    bool canonicalCategoryIsNarrativePersistent(const std::string& category)
    {
        static const std::vector<std::string> protectedCategories = {
            "rivaux_potentiels", "rivaux_confirmes", "rumeurs_rivaux", "destin_instable", "serments_eglise", "serments_rompus",
            "objets_avec_memoire", "cicatrices", "traumatismes", "heritage", "tombes",
            "techniques_combinees_alliees", "serments_fragilises"
        };
        return std::find(protectedCategories.begin(), protectedCategories.end(), category) != protectedCategories.end();
    }

    std::string playerWeaponCategoryLabel(WeaponType type)
    {
        switch (type)
        {
            case WeaponType::Dagger: return "Dague / arme légère";
            case WeaponType::Bow: return "Arc / arme à distance";
            case WeaponType::Staff: return "Bâton / catalyseur";
            case WeaponType::Sword: return "Épée";
            case WeaponType::Axe: return "Hache";
            case WeaponType::Hammer: return "Marteau / arme lourde";
            case WeaponType::Spear: return "Lance";
            case WeaponType::BareHands: return "Mains nues";
            default: return "Arme inconnue";
        }
    }

    std::string normalizeLegacyTitleName(const std::string& title)
    {
        if (title == "Premier titre du registre")
        {
            return "Une sorte de titre ?";
        }
        return title;
    }
    std::vector<std::string> splitCurseTokenList(const std::string& value)
    {
        std::vector<std::string> tokens;
        std::stringstream stream(value);
        std::string token;
        while (std::getline(stream, token, ','))
        {
            token.erase(std::remove_if(token.begin(), token.end(), [](unsigned char c) { return std::isspace(c) != 0; }), token.end());
            if (!token.empty() && std::find(tokens.begin(), tokens.end(), token) == tokens.end())
            {
                tokens.push_back(token);
            }
        }
        return tokens;
    }

    std::string joinCurseTokenList(const std::vector<std::string>& tokens)
    {
        std::string result;
        for (const std::string& token : tokens)
        {
            if (token.empty()) continue;
            if (!result.empty()) result += ",";
            result += token;
        }
        return result;
    }

    bool curseTokenListContains(const std::string& value, const std::string& token)
    {
        const std::vector<std::string> tokens = splitCurseTokenList(value);
        return std::find(tokens.begin(), tokens.end(), token) != tokens.end();
    }

    bool addCurseTokenToList(std::string& value, const std::string& token)
    {
        if (token.empty()) return false;
        std::vector<std::string> tokens = splitCurseTokenList(value);
        if (std::find(tokens.begin(), tokens.end(), token) != tokens.end())
        {
            return false;
        }
        tokens.push_back(token);
        value = joinCurseTokenList(tokens);
        return true;
    }

    std::string curseCategoryLabel(const std::string& category)
    {
        if (category == "health") return "santé";
        if (category == "attack") return "attaque";
        if (category == "mana") return "mana";
        if (category == "precision") return "précision";
        if (category == "defense") return "défense";
        if (category == "sleep") return "sommeil";
        if (category == "luck") return "chance";
        if (category == "equipment") return "équipement";
        if (category == "spirit") return "esprit";
        if (category == "corruption") return "corruption";
        if (category == "travel") return "voyage";
        if (category == "social") return "présence sociale";
        if (category == "interface") return "interface";
        if (category == "hallucination") return "hallucinations";
        return category;
    }

    std::string curseSeverityForLevel(int level)
    {
        if (level >= 4) return "critique";
        if (level >= 3) return "majeure";
        if (level == 2) return "moyenne";
        return "mineure";
    }

    void refreshCursePresentationFromLevel(PlayerCurse& curse)
    {
        curse.curseLevel = std::max(1, curse.curseLevel);
        curse.maxCurseLevel = std::max(curse.curseLevel, curse.maxCurseLevel);
        if (curse.severity.empty() || curse.evolvesOverTime)
        {
            curse.severity = curseSeverityForLevel(curse.curseLevel);
        }

        if (curse.id == "runic_backlash")
        {
            curse.name = "Contrecoup runique niv." + std::to_string(curse.curseLevel);
            if (curse.curseLevel >= 4)
            {
                curse.description = "La rune brisée ne colle plus seulement à la peau : elle cherche maintenant une logique plus profonde dans le personnage.";
            }
        }
        else if (curse.id == "lyknir_prey_mark" && curse.curseLevel > 1)
        {
            curse.name = "Marque de proie de Lyknir niv." + std::to_string(curse.curseLevel);
        }
        else if (curse.id == "anomaly_interface_desync" || curse.id == "anomaly_source_core_desync")
        {
            curse.name = (curse.id == "anomaly_source_core_desync" ? "Désynchronisation de la Source niv." : "Désynchronisation de l'Anomalie niv.") + std::to_string(curse.curseLevel);
            if (curse.curseLevel >= 4)
            {
                curse.description = "L'Anomalie ne perturbe plus seulement les sens : elle laisse croire au personnage que l'interface, les cibles et même son propre nom peuvent mentir.";
            }
        }

        if (curse.becomesSpecialRemovalWhenTooHigh && curse.curseLevel > curse.churchRemovalMaxLevel)
        {
            curse.removableByChurch = false;
            curse.lifeLong = true;
            curse.expiresAtDay = -1;
            if (!curse.highLevelRemovalHint.empty())
            {
                curse.removalHint = curse.highLevelRemovalHint;
            }
        }
    }

    int defaultLocalSubscriptionRenewalPrice(const std::string& subscriptionId)
    {
        if (subscriptionId == "lodging_modest_weekly") return 110;
        if (subscriptionId == "stable_relay_weekly") return 135;
        if (subscriptionId == "guild_adventurer_standard_weekly") return 160;
        if (subscriptionId == "guild_adventurer_silver_weekly") return 260;
        if (subscriptionId == "trade_route_weekly") return 240;
        if (subscriptionId == "merchant_cotisation_weekly") return 210;
        return 0;
    }

}

// EN: Player declares or implements a focused behavior used by this module.
// FR: Player déclare ou implémente un comportement précis utilisé par ce module.
Player::Player() : Entity()
{
    level = 1;
    experience = 0;
    equippedWeaponIndex = -1;
    equippedArmorIndex = -1;
    cityVaultPurchased = false;
    cityVaultLevel = 0;
    currentCityId = "valebrume";
    registeredGuildCityIds.clear();
    race = CharacterRace::Human;
    unspentAttributePoints = 0;
    daggerKillProgress = 0;
    bowKillProgress = 0;
    bareHandKillProgress = 0;
    staffKillProgress = 0;
    swordKillProgress = 0;
    axeKillProgress = 0;
    hammerKillProgress = 0;
    spearKillProgress = 0;

    combatsStarted = 0;
    worldDaysElapsed = 0;
    worldDayProgressUnits = 0;
    activeCurses.clear();
    blessingInventory.clear();
    localSubscriptionRenewalPaidThisWeek = 0;
    pendingWorldTimeReportLines.clear();
    victories = 0;
    defeats = 0;
    escapes = 0;
    deaths = 0;
    enemiesKilled = 0;
    bossesKilled = 0;
    pvpVictories = 0;
    pvpDefeats = 0;
    pvpLethalEliminations.clear();
    unlockedBossIds = {1, 2, 3};
    recentBossIds.clear();
    defeatedBossIds.clear();
    recentBossCooldownExpiresAtDay = -1;
    rareBossDiscoveryCooldownExpiresAtDay = -1;
    bossDiscoveryLocations.assign(37, "");
    recentExplorationEventKeys.clear();
    recentExplorationChallengeKeys.clear();
    explorationSceneCooldowns.clear();
    shopPromotionPurchaseCounters.clear();
    recentCombatEquipmentUsage.clear();
    bossEquipmentSealActive = false;
    bossEquipmentSealReason = "";
    challengeCombatTrackingActive = false;
    challengeCombatConsumablesUsed = 0;
    challengeCombatSkillsUsed = 0;
    challengeCombatNonBasicAttacksUsed = 0;
    challengeCombatBasicAttacksUsed = 0;
    challengeCombatDefenseTurns = 0;
    challengeCombatTurnsTaken = 0;
    challengeCombatDamageTaken = 0;
    challengeCombatSummonActions = 0;
    challengeCombatPartySize = 1;
    challengeCombatAlivePartyCount = 1;
    zelefCorrosionPresent = false;
    zelefMaxHpStolen = 0;
    grinkaBossTheftPresent = false;
    grinkaStolenGold = 0;
    grinkaHasStolenWeapon = false;
    grinkaHasStolenArmor = false;
    cloneCharacter = false;

    alteredByCheats = false;
    godModeEnabled = false;
    infiniteConsumablesEnabled = false;
    indestructibleEquipmentEnabled = false;
    equipmentProtectionEnabled = false;
    storySkipEnabled = false;
    specialChallengeAccessUnlocked = false;

    godModeKnown = false;
    infiniteConsumablesKnown = false;
    indestructibleEquipmentKnown = false;
    equipmentProtectionKnown = false;
    storySkipKnown = false;
    specialChallengeAccessKnown = false;
    creatorMessageKnown = false;

    goldCheatUseCount = 0;
    levelCheatUseCount = 0;
    maxLevelCheatUseCount = 0;
    refundCheatUseCount = 0;
    resetCheatUseCount = 0;
    switchClassCheatUseCount = 0;
    lethalCheatAttemptCount = 0;
    worldGazeCombatsRemaining = 0;
    worldGazeMaxHpPenalty = 0;

    refundUsesRemaining = 3;
    createdAtText = VersionInfo::currentDateText();
    accumulatedRealPlaySeconds = 0;
    realPlaySessionStartedAt = std::chrono::steady_clock::now();
    createdForVersion = VersionInfo::currentVersion();
    lastAdaptedVersion = VersionInfo::currentVersion();
    creatorAccountName = "";
    currentOwnerAccountName = "";
    titles.clear();
    activeTitle = "";
    activeTitles.clear();
    interfaceHintFrequency = "faible";
    graphicalImagesEnabled = false;
    storyChapter = 0;
    storyStep = 0;
    storyCityDevelopmentLevel = 0;
    storyModeStarted = false;
    nextAmmunitionChoiceId = "";
    lastConsumedAmmunitionId = "";
}

Player::Player(
    const std::string& name,
    const PlayerClass& playerClass
) : Entity(
        name,
        playerClass.getName(),
        playerClass.getMaxHp(),
        playerClass.getMinDamage(),
        playerClass.getMaxDamage(),
        playerClass.getCriticalDamage(),
        playerClass.getHealingPotionCount(),
        // EN: playerClass.getDamagePotionCount declares or implements a focused behavior used by this module.
        // FR: playerClass.getDamagePotionCount déclare ou implémente un comportement précis utilisé par ce module.
        playerClass.getDamagePotionCount()
    )
{
    level = 1;
    experience = 0;
    equippedWeaponIndex = -1;
    equippedArmorIndex = -1;
    cityVaultPurchased = false;
    cityVaultLevel = 0;
    currentCityId = "valebrume";
    registeredGuildCityIds.clear();
    race = CharacterRace::Human;
    unspentAttributePoints = 0;
    daggerKillProgress = 0;
    bowKillProgress = 0;
    bareHandKillProgress = 0;
    staffKillProgress = 0;
    swordKillProgress = 0;
    axeKillProgress = 0;
    hammerKillProgress = 0;
    spearKillProgress = 0;

    combatsStarted = 0;
    worldDaysElapsed = 0;
    worldDayProgressUnits = 0;
    activeCurses.clear();
    blessingInventory.clear();
    localSubscriptionRenewalPaidThisWeek = 0;
    pendingWorldTimeReportLines.clear();
    victories = 0;
    defeats = 0;
    escapes = 0;
    deaths = 0;
    enemiesKilled = 0;
    bossesKilled = 0;
    pvpVictories = 0;
    pvpDefeats = 0;
    pvpLethalEliminations.clear();
    unlockedBossIds = {1, 2, 3};
    recentBossIds.clear();
    defeatedBossIds.clear();
    recentBossCooldownExpiresAtDay = -1;
    rareBossDiscoveryCooldownExpiresAtDay = -1;
    bossDiscoveryLocations.assign(37, "");
    recentExplorationEventKeys.clear();
    recentExplorationChallengeKeys.clear();
    explorationSceneCooldowns.clear();
    shopPromotionPurchaseCounters.clear();
    recentCombatEquipmentUsage.clear();
    bossEquipmentSealActive = false;
    bossEquipmentSealReason = "";
    challengeCombatTrackingActive = false;
    challengeCombatConsumablesUsed = 0;
    challengeCombatSkillsUsed = 0;
    challengeCombatNonBasicAttacksUsed = 0;
    challengeCombatBasicAttacksUsed = 0;
    challengeCombatDefenseTurns = 0;
    challengeCombatTurnsTaken = 0;
    challengeCombatDamageTaken = 0;
    challengeCombatSummonActions = 0;
    challengeCombatPartySize = 1;
    challengeCombatAlivePartyCount = 1;
    zelefCorrosionPresent = false;
    zelefMaxHpStolen = 0;
    grinkaBossTheftPresent = false;
    grinkaStolenGold = 0;
    grinkaHasStolenWeapon = false;
    grinkaHasStolenArmor = false;
    cloneCharacter = false;

    alteredByCheats = false;
    godModeEnabled = false;
    infiniteConsumablesEnabled = false;
    indestructibleEquipmentEnabled = false;
    equipmentProtectionEnabled = false;
    storySkipEnabled = false;
    specialChallengeAccessUnlocked = false;

    godModeKnown = false;
    infiniteConsumablesKnown = false;
    indestructibleEquipmentKnown = false;
    equipmentProtectionKnown = false;
    storySkipKnown = false;
    specialChallengeAccessKnown = false;
    creatorMessageKnown = false;

    goldCheatUseCount = 0;
    levelCheatUseCount = 0;
    maxLevelCheatUseCount = 0;
    refundCheatUseCount = 0;
    resetCheatUseCount = 0;
    switchClassCheatUseCount = 0;
    lethalCheatAttemptCount = 0;
    worldGazeCombatsRemaining = 0;
    worldGazeMaxHpPenalty = 0;

    refundUsesRemaining = 3;
    createdAtText = VersionInfo::currentDateText();
    accumulatedRealPlaySeconds = 0;
    realPlaySessionStartedAt = std::chrono::steady_clock::now();
    createdForVersion = VersionInfo::currentVersion();
    lastAdaptedVersion = VersionInfo::currentVersion();
    creatorAccountName = "";
    currentOwnerAccountName = "";
    titles.clear();
    activeTitle = "";
    activeTitles.clear();
    interfaceHintFrequency = "faible";
    graphicalImagesEnabled = false;
    storyChapter = 0;
    storyStep = 0;
    storyCityDevelopmentLevel = 0;
    storyModeStarted = false;
    nextAmmunitionChoiceId = "";
    lastConsumedAmmunitionId = "";
}


void Player::setNextAmmunitionChoice(const std::string& ammunitionId)
{
    nextAmmunitionChoiceId = ammunitionId;
}

std::string Player::getNextAmmunitionChoice() const
{
    return nextAmmunitionChoiceId;
}

void Player::clearNextAmmunitionChoice()
{
    nextAmmunitionChoiceId.clear();
}

void Player::setLastConsumedAmmunition(const std::string& ammunitionId)
{
    lastConsumedAmmunitionId = ammunitionId;
}

std::string Player::getLastConsumedAmmunition() const
{
    return lastConsumedAmmunitionId;
}

void Player::clearLastConsumedAmmunition()
{
    lastConsumedAmmunitionId.clear();
}

const std::string& Player::getCreatedAtText() const
{
    return createdAtText;
}

long long Player::getRealPlayTimeSeconds() const
{
    const auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - realPlaySessionStartedAt
    ).count();
    return std::max(0LL, accumulatedRealPlaySeconds + elapsed);
}

void Player::setLoadedRealPlayTimeSeconds(long long seconds)
{
    accumulatedRealPlaySeconds = std::max(0LL, seconds);
    realPlaySessionStartedAt = std::chrono::steady_clock::now();
}

const std::string& Player::getCreatorAccountName() const
{
    return creatorAccountName;
}

const std::string& Player::getCurrentOwnerAccountName() const
{
    return currentOwnerAccountName;
}

void Player::setOwnershipMetadata(const std::string& creatorAccount, const std::string& currentOwnerAccount)
{
    creatorAccountName = creatorAccount;
    currentOwnerAccountName = currentOwnerAccount;
}


const std::string& Player::getCreatedForVersion() const
{
    return createdForVersion;
}

const std::string& Player::getLastAdaptedVersion() const
{
    return lastAdaptedVersion;
}

void Player::setVersionMetadata(const std::string& createdAt, const std::string& createdFor, const std::string& lastAdapted)
{
    createdAtText = createdAt.empty() ? "Inconnue" : createdAt;
    createdForVersion = createdFor.empty() ? "inconnue" : createdFor;
    lastAdaptedVersion = lastAdapted.empty() ? createdForVersion : lastAdapted;
}

const std::vector<std::string>& Player::getTitles() const
{
    return titles;
}

const std::string& Player::getActiveTitle() const
{
    return activeTitle;
}

const std::vector<std::string>& Player::getActiveTitles() const
{
    return activeTitles;
}

std::string Player::getActiveTitleSummary() const
{
    if (activeTitles.empty())
    {
        return "Aucun";
    }

    std::string summary;
    for (std::size_t i = 0; i < activeTitles.size(); ++i)
    {
        if (i > 0)
        {
            summary += " / ";
        }
        summary += activeTitles[i];
    }
    return summary;
}

const std::string& Player::getInterfaceHintFrequency() const
{
    return interfaceHintFrequency;
}

std::string Player::getInterfaceHintFrequencyLabel() const
{
    if (interfaceHintFrequency == "null")
    {
        return "null";
    }
    if (interfaceHintFrequency == "normal")
    {
        return "normal";
    }
    if (interfaceHintFrequency == "forte")
    {
        return "forte";
    }
    return "faible";
}

void Player::setInterfaceHintFrequency(const std::string& frequency)
{
    if (frequency == "null" || frequency == "faible" || frequency == "normal" || frequency == "forte")
    {
        interfaceHintFrequency = frequency;
        return;
    }

    interfaceHintFrequency = "faible";
}

bool Player::areInterfaceHintsDisabled() const
{
    return interfaceHintFrequency == "null";
}

bool Player::areInterfaceHintsFrequent() const
{
    return interfaceHintFrequency == "forte";
}


bool Player::areGraphicalImagesEnabled() const
{
    return graphicalImagesEnabled;
}

std::string Player::getGraphicalImagesSettingLabel() const
{
    return graphicalImagesEnabled ? "activées en IG" : "désactivées";
}

void Player::setGraphicalImagesEnabled(bool enabled)
{
    graphicalImagesEnabled = enabled;
}

void Player::forceTerminalImagePolicy()
{
    graphicalImagesEnabled = false;
}

int Player::getStoryChapter() const
{
    return storyChapter;
}

int Player::getStoryStep() const
{
    return storyStep;
}

int Player::getStoryCityDevelopmentLevel() const
{
    return storyCityDevelopmentLevel;
}

bool Player::hasStoryModeStarted() const
{
    return storyModeStarted;
}

std::string Player::getStoryProgressLabel() const
{
    if (!storyModeStarted || storyChapter <= 0)
    {
        return "Histoire non commencée";
    }

    if (storyChapter >= 99)
    {
        return "Archives ouvertes par altération";
    }

    return "Chapitre " + std::to_string(storyChapter)
        + " | étape " + std::to_string(storyStep)
        + " | développement ville " + std::to_string(storyCityDevelopmentLevel);
}

void Player::setLoadedStoryProgress(int chapter, int step, int cityDevelopment, bool started)
{
    storyChapter = chapter < 0 ? 0 : chapter;
    storyStep = step < 0 ? 0 : step;
    storyCityDevelopmentLevel = cityDevelopment < 0 ? 0 : cityDevelopment;
    storyModeStarted = started || storyChapter > 0 || storyStep > 0 || storyCityDevelopmentLevel > 0;
}

bool Player::startStoryMode()
{
    bool changed = false;
    if (!storyModeStarted)
    {
        storyModeStarted = true;
        changed = true;
    }
    if (storyChapter < 1)
    {
        storyChapter = 1;
        changed = true;
    }
    if (storyStep < 1)
    {
        storyStep = 1;
        changed = true;
    }
    if (storyCityDevelopmentLevel < 1)
    {
        storyCityDevelopmentLevel = 1;
        changed = true;
    }
    return changed;
}

bool Player::setStoryProgress(int chapter, int step, int cityDevelopment)
{
    const int normalizedChapter = chapter < 0 ? 0 : chapter;
    const int normalizedStep = step < 0 ? 0 : step;
    const int normalizedCity = cityDevelopment < 0 ? 0 : cityDevelopment;

    const bool changed = storyChapter != normalizedChapter
        || storyStep != normalizedStep
        || storyCityDevelopmentLevel != normalizedCity
        || (!storyModeStarted && normalizedChapter > 0);

    storyChapter = normalizedChapter;
    storyStep = normalizedStep;
    storyCityDevelopmentLevel = normalizedCity;
    if (normalizedChapter > 0 || normalizedStep > 0 || normalizedCity > 0)
    {
        storyModeStarted = true;
    }

    return changed;
}

std::vector<std::string> Player::describeActiveTitleEffects() const
{
    std::vector<std::string> lines;
    if (activeTitles.empty())
    {
        lines.push_back("- Aucun titre équipé : aucune influence de réputation affichée.");
        return lines;
    }

    for (std::size_t i = 0; i < activeTitles.size(); ++i)
    {
        const std::string& title = activeTitles[i];
        lines.push_back("- Emplacement " + std::to_string(i + 1) + " : " + title);
        lines.push_back("  > Influence : " + TitleCatalog::effectFor(title));
        lines.push_back("  > Bonus : très faible et surtout lore/dialogues ; aucun gros bonus brut de combat.");
    }

    lines.push_back("Note : les trois titres équipés servent à afficher l'identité du personnage. Ils peuvent orienter de petites réactions sociales, mais ne doivent pas remplacer les compétences, l'équipement ou le niveau.");
    return lines;
}

bool Player::hasTitle(const std::string& title) const
{
    const std::string normalizedTitle = normalizeLegacyTitleName(title);
    return std::find(titles.begin(), titles.end(), normalizedTitle) != titles.end();
}

bool Player::grantTitle(const std::string& title)
{
    const std::string normalizedTitle = normalizeLegacyTitleName(title);
    if (normalizedTitle.empty() || hasTitle(normalizedTitle))
    {
        return false;
    }

    const std::string firstTitle = "Une sorte de titre ?";
    const bool shouldGrantFirstTitle = normalizedTitle != firstTitle && !hasTitle(firstTitle);

    titles.push_back(normalizedTitle);
    if (shouldGrantFirstTitle)
    {
        titles.push_back(firstTitle);
    }

    if (activeTitles.empty())
    {
        activeTitles.push_back(normalizedTitle);
        activeTitle = normalizedTitle;
    }
    else if (activeTitle.empty())
    {
        activeTitle = activeTitles.front();
    }

    const std::string ultimateTitle = "Complétionniste de l'impossible";
    if (normalizedTitle != ultimateTitle
        && !hasTitle(ultimateTitle)
        && TitleCatalog::hasAllUltimatePrerequisites(titles))
    {
        titles.push_back(ultimateTitle);
    }

    return true;
}

bool Player::setActiveTitle(const std::string& title)
{
    const std::string normalizedTitle = normalizeLegacyTitleName(title);
    if (normalizedTitle.empty())
    {
        activeTitle.clear();
        activeTitles.clear();
        return true;
    }

    if (!hasTitle(normalizedTitle))
    {
        return false;
    }

    activeTitles.erase(std::remove(activeTitles.begin(), activeTitles.end(), normalizedTitle), activeTitles.end());
    activeTitles.insert(activeTitles.begin(), normalizedTitle);
    while (activeTitles.size() > 3)
    {
        activeTitles.pop_back();
    }

    activeTitle = activeTitles.front();
    return true;
}

bool Player::setActiveTitleSlot(int slotIndex, const std::string& title)
{
    const std::string normalizedTitle = normalizeLegacyTitleName(title);
    if (slotIndex < 0 || slotIndex >= 3)
    {
        return false;
    }

    if (normalizedTitle.empty())
    {
        return unequipActiveTitleSlot(slotIndex);
    }

    if (!hasTitle(normalizedTitle))
    {
        return false;
    }

    activeTitles.erase(std::remove(activeTitles.begin(), activeTitles.end(), normalizedTitle), activeTitles.end());
    while (activeTitles.size() <= static_cast<std::size_t>(slotIndex))
    {
        activeTitles.push_back("");
    }

    activeTitles[static_cast<std::size_t>(slotIndex)] = normalizedTitle;
    activeTitles.erase(std::remove(activeTitles.begin(), activeTitles.end(), std::string()), activeTitles.end());
    while (activeTitles.size() > 3)
    {
        activeTitles.pop_back();
    }

    activeTitle = activeTitles.empty() ? std::string() : activeTitles.front();
    return true;
}

bool Player::unequipActiveTitleSlot(int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= 3)
    {
        return false;
    }

    if (static_cast<std::size_t>(slotIndex) >= activeTitles.size())
    {
        return true;
    }

    activeTitles.erase(activeTitles.begin() + slotIndex);
    activeTitle = activeTitles.empty() ? std::string() : activeTitles.front();
    return true;
}

bool Player::equipTitle(const std::string& title)
{
    const std::string normalizedTitle = normalizeLegacyTitleName(title);
    if (normalizedTitle.empty() || !hasTitle(normalizedTitle))
    {
        return false;
    }

    if (std::find(activeTitles.begin(), activeTitles.end(), normalizedTitle) != activeTitles.end())
    {
        return true;
    }

    if (activeTitles.size() >= 3)
    {
        return false;
    }

    activeTitles.push_back(normalizedTitle);
    if (activeTitle.empty())
    {
        activeTitle = activeTitles.front();
    }
    return true;
}

void Player::setLoadedTitles(const std::vector<std::string>& loadedTitles, const std::string& loadedActiveTitle)
{
    setLoadedTitles(loadedTitles, loadedActiveTitle, {});
}

void Player::setLoadedTitles(const std::vector<std::string>& loadedTitles, const std::string& loadedActiveTitle, const std::vector<std::string>& loadedActiveTitles)
{
    titles.clear();
    activeTitles.clear();

    for (const std::string& title : loadedTitles)
    {
        const std::string normalizedTitle = normalizeLegacyTitleName(title);
        if (!normalizedTitle.empty() && !hasTitle(normalizedTitle))
        {
            titles.push_back(normalizedTitle);
        }
    }

    for (const std::string& title : loadedActiveTitles)
    {
        const std::string normalizedTitle = normalizeLegacyTitleName(title);
        if (!normalizedTitle.empty()
            && hasTitle(normalizedTitle)
            && std::find(activeTitles.begin(), activeTitles.end(), normalizedTitle) == activeTitles.end())
        {
            activeTitles.push_back(normalizedTitle);
            if (activeTitles.size() >= 3)
            {
                break;
            }
        }
    }

    const std::string normalizedLoadedActiveTitle = normalizeLegacyTitleName(loadedActiveTitle);
    if (activeTitles.empty() && !normalizedLoadedActiveTitle.empty() && hasTitle(normalizedLoadedActiveTitle))
    {
        activeTitles.push_back(normalizedLoadedActiveTitle);
    }
    else if (activeTitles.empty() && !titles.empty())
    {
        activeTitles.push_back(titles.front());
    }

    activeTitle = activeTitles.empty() ? std::string() : activeTitles.front();
}

void Player::markAdaptedToCurrentVersion()
{
    lastAdaptedVersion = VersionInfo::currentVersion();
}

const std::vector<std::string>& Player::getStarterKitLog() const
{
    return starterKitLog;
}

void Player::setLoadedStarterKitLog(const std::vector<std::string>& log)
{
    starterKitLog = log;
}

void Player::recordStarterKitEntry(const std::string& entry)
{
    if (!entry.empty())
    {
        starterKitLog.push_back(entry);
    }
}

// EN: applyRaceStartingBonus declares or implements a focused behavior used by this module.
// FR: applyRaceStartingBonus déclare ou implémente un comportement précis utilisé par ce module.
void Player::applyRaceStartingBonus(CharacterRace selectedRace)
{
    RaceStartingBonus bonus = RaceCatalog::getStartingBonus(selectedRace);

    maxHp += bonus.maxHpBonus;
    hp += bonus.maxHpBonus;

    minDamage += bonus.minDamageBonus;
    maxDamage += bonus.maxDamageBonus;
    criticalDamage += bonus.criticalDamageBonus;

    if (maxHp < 1)
    {
        maxHp = 1;
    }

    if (hp < 1)
    {
        hp = 1;
    }

    if (minDamage < 0)
    {
        minDamage = 0;
    }

    if (maxDamage < minDamage)
    {
        maxDamage = minDamage;
    }

    if (criticalDamage < maxDamage)
    {
        criticalDamage = maxDamage;
    }
}

// EN: getEquippedArmorMaxHpBonus declares or implements a focused behavior used by this module.
// FR: getEquippedArmorMaxHpBonus déclare ou implémente un comportement précis utilisé par ce module.
int Player::getEquippedArmorMaxHpBonus() const
{
    if (!hasEquippedArmor())
    {
        return 0;
    }

    Armor armor = getEquippedArmor();

    if (armor.isBroken())
    {
        return 0;
    }

    return armor.getMaxHpBonus();
}

// EN: getLevel declares or implements a focused behavior used by this module.
// FR: getLevel déclare ou implémente un comportement précis utilisé par ce module.
int Player::getLevel() const
{
    return level;
}

// EN: getExperience declares or implements a focused behavior used by this module.
// FR: getExperience déclare ou implémente un comportement précis utilisé par ce module.
int Player::getExperience() const
{
    return experience;
}

// EN: getUnspentAttributePoints declares or implements a focused behavior used by this module.
// FR: getUnspentAttributePoints déclare ou implémente un comportement précis utilisé par ce module.
int Player::getUnspentAttributePoints() const
{
    return unspentAttributePoints;
}

// EN: getAttributes declares or implements a focused behavior used by this module.
// FR: getAttributes déclare ou implémente un comportement précis utilisé par ce module.
const DndAttributes& Player::getAttributes() const
{
    return attributes;
}

// EN: getRace declares or implements a focused behavior used by this module.
// FR: getRace déclare ou implémente un comportement précis utilisé par ce module.
CharacterRace Player::getRace() const
{
    return race;
}

std::string Player::getRaceText() const
{
    return characterRaceToText(race);
}

int Player::getCharacterAge() const
{
    return characterAge;
}

const std::string& Player::getVisualPresentation() const
{
    return visualPresentation;
}

const std::string& Player::getVisualVariant() const
{
    return visualVariant;
}

std::string Player::getAgeBandText() const
{
    return RaceCatalog::getAgeBand(race, characterAge);
}

std::string Player::getAppearanceDescription() const
{
    std::ostringstream description;
    description << getRaceText() << " · " << characterAge << " ans (" << getAgeBandText() << ")";
    if (!visualPresentation.empty()) description << " · " << visualPresentation;
    if (!visualVariant.empty()) description << " · " << visualVariant;
    description << " · silhouette adaptée à la classe " << type;
    return description.str();
}

void Player::setAppearanceProfile(int age, const std::string& presentation, const std::string& variant)
{
    characterAge = std::max(15, std::min(age, RaceCatalog::getMaximumAge(race)));
    visualPresentation = presentation.empty() ? "Non précisé" : presentation;
    visualVariant = variant.empty() ? "Variante A" : variant;
}

// EN: setRace declares or implements a focused behavior used by this module.
// FR: setRace déclare ou implémente un comportement précis utilisé par ce module.
void Player::setRace(CharacterRace selectedRace)
{
    race = selectedRace;
    characterAge = std::max(15, std::min(characterAge, RaceCatalog::getMaximumAge(selectedRace)));
    LanguageSystem::ensureStarterLanguages(*this);
    applyRaceStartingBonus(selectedRace);
    refreshLevelAndIdentitySkills();
}


// EN: setLoadedProgress declares or implements a focused behavior used by this module.
// FR: setLoadedProgress déclare ou implémente un comportement précis utilisé par ce module.
void Player::setLoadedProgress(int loadedLevel, int loadedExperience, int loadedHp)
{
    if (loadedLevel < 1)
    {
        loadedLevel = 1;
    }

    if (loadedLevel > MAX_LEVEL)
    {
        loadedLevel = MAX_LEVEL;
    }

    if (loadedExperience < 0)
    {
        loadedExperience = 0;
    }

    if (loadedHp < 1)
    {
        loadedHp = 1;
    }

    if (loadedHp > maxHp)
    {
        loadedHp = maxHp;
    }

    level = loadedLevel;
    experience = loadedExperience;
    hp = loadedHp;
}


void Player::setLoadedCombatStats(
    int loadedMaxHp,
    int loadedMinDamage,
    int loadedMaxDamage,
    int loadedCriticalDamage,
    int loadedHp
)
{
    if (loadedMaxHp < 1)
    {
        loadedMaxHp = 1;
    }

    if (loadedMinDamage < 0)
    {
        loadedMinDamage = 0;
    }

    if (loadedMaxDamage < loadedMinDamage)
    {
        loadedMaxDamage = loadedMinDamage;
    }

    if (loadedCriticalDamage < loadedMaxDamage)
    {
        loadedCriticalDamage = loadedMaxDamage;
    }

    if (loadedHp < 1)
    {
        loadedHp = 1;
    }

    if (loadedHp > loadedMaxHp)
    {
        loadedHp = loadedMaxHp;
    }

    maxHp = loadedMaxHp;
    minDamage = loadedMinDamage;
    maxDamage = loadedMaxDamage;
    criticalDamage = loadedCriticalDamage;
    hp = loadedHp;
}

// EN: setLoadedAttributes declares or implements a focused behavior used by this module.
// FR: setLoadedAttributes déclare ou implémente un comportement précis utilisé par ce module.
void Player::setLoadedAttributes(const DndAttributes& loadedAttributes, int loadedUnspentPoints)
{
    attributes = loadedAttributes;

    if (loadedUnspentPoints < 0)
    {
        loadedUnspentPoints = 0;
    }

    unspentAttributePoints = loadedUnspentPoints;
}

// EN: spendAttributePoint declares or implements a focused behavior used by this module.
// FR: spendAttributePoint déclare ou implémente un comportement précis utilisé par ce module.
bool Player::spendAttributePoint(int attributeChoice)
{
    if (unspentAttributePoints <= 0)
    {
        return false;
    }

    if (!attributes.increaseByChoice(attributeChoice))
    {
        return false;
    }

    unspentAttributePoints--;

    if (attributeChoice == 1)
    {
        minDamage += 1;
        maxDamage += 1;
    }
    else if (attributeChoice == 2)
    {
        criticalDamage += 2;
    }
    else if (attributeChoice == 3)
    {
        maxHp += 5;
        hp += 5;
    }

    if (maxDamage < minDamage)
    {
        maxDamage = minDamage;
    }

    if (criticalDamage < maxDamage)
    {
        criticalDamage = maxDamage;
    }

    if (hp > maxHp)
    {
        hp = maxHp;
    }

    return true;
}


// EN: displayAttributes declares or implements a focused behavior used by this module.
// FR: displayAttributes déclare ou implémente un comportement précis utilisé par ce module.
void Player::displayAttributes() const
{
    PlayerUiSupport::showPlayerScreen(
        "ATTRIBUTS",
        "player.attributes",
        {
            "Force : " + std::to_string(attributes.getStrength()),
            "Dextérité : " + std::to_string(attributes.getDexterity()),
            "Constitution : " + std::to_string(attributes.getConstitution()),
            "Intelligence : " + std::to_string(attributes.getIntelligence()),
            "Sagesse : " + std::to_string(attributes.getWisdom()),
            "Charisme : " + std::to_string(attributes.getCharisma()),
            "Points disponibles : " + std::to_string(unspentAttributePoints)
        },
        false
    );
}


int Player::getCombatsStarted() const
{
    return combatsStarted;
}

int Player::getWorldDaysElapsed() const
{
    return worldDaysElapsed;
}

int Player::getShopPromotionPurchaseCount(const std::string& key) const
{
    if (key.empty())
    {
        return 0;
    }

    for (const PlayerPersistentCounter& counter : shopPromotionPurchaseCounters)
    {
        if (counter.key == key)
        {
            return std::max(0, counter.value);
        }
    }
    return 0;
}

void Player::recordShopPromotionPurchase(const std::string& key, int amount)
{
    if (key.empty() || amount <= 0)
    {
        return;
    }

    for (PlayerPersistentCounter& counter : shopPromotionPurchaseCounters)
    {
        if (counter.key == key)
        {
            counter.value = std::max(0, counter.value + amount);
            return;
        }
    }

    shopPromotionPurchaseCounters.push_back({key, amount});
    if (shopPromotionPurchaseCounters.size() > 48)
    {
        shopPromotionPurchaseCounters.erase(shopPromotionPurchaseCounters.begin(), shopPromotionPurchaseCounters.begin() + 16);
    }
}

const std::vector<PlayerPersistentCounter>& Player::getShopPromotionPurchaseCounters() const
{
    return shopPromotionPurchaseCounters;
}

void Player::setLoadedShopPromotionPurchaseCounters(const std::vector<PlayerPersistentCounter>& counters)
{
    shopPromotionPurchaseCounters.clear();
    for (const PlayerPersistentCounter& counter : counters)
    {
        if (!counter.key.empty() && counter.value > 0)
        {
            shopPromotionPurchaseCounters.push_back(counter);
        }
    }
    if (shopPromotionPurchaseCounters.size() > 48)
    {
        shopPromotionPurchaseCounters.erase(shopPromotionPurchaseCounters.begin(), shopPromotionPurchaseCounters.end() - 48);
    }
}

void Player::recordCanonicalEvent(const std::string& category, const std::string& key, const std::string& label, int amount)
{
    if (category.empty() || key.empty() || amount <= 0)
    {
        return;
    }

    const std::string cleanLabel = label.empty() ? key : label;
    const bool localScoped = canonicalCategoryIsLocalScoped(category);
    for (PlayerJournalRecord& record : canonicalJournalRecords)
    {
        const bool sameLocation = !localScoped || record.locationId == currentCityId;
        if (record.category == category && record.key == key && sameLocation)
        {
            record.count = std::max(0, record.count + amount);
            record.label = cleanLabel;
            // A global historical/statistical record keeps its original place. A local record
            // is already matched against the current city and therefore cannot migrate.
            if (record.locationId.empty())
            {
                record.locationId = currentCityId;
            }
            record.lastDay = worldDaysElapsed;
            return;
        }
    }

    PlayerJournalRecord record;
    record.category = category;
    record.key = key;
    record.label = cleanLabel;
    record.locationId = currentCityId;
    record.count = amount;
    record.lastDay = worldDaysElapsed;
    canonicalJournalRecords.push_back(record);

    if (canonicalJournalRecords.size() > 260)
    {
        std::stable_sort(canonicalJournalRecords.begin(), canonicalJournalRecords.end(), [](const PlayerJournalRecord& a, const PlayerJournalRecord& b) {
            const bool aProtected = canonicalCategoryIsNarrativePersistent(a.category);
            const bool bProtected = canonicalCategoryIsNarrativePersistent(b.category);
            if (aProtected != bProtected) return aProtected;
            if (a.count != b.count) return a.count > b.count;
            return a.lastDay > b.lastDay;
        });
        canonicalJournalRecords.resize(220);
    }
}

const std::vector<PlayerJournalRecord>& Player::getCanonicalJournalRecords() const
{
    return canonicalJournalRecords;
}

std::vector<PlayerJournalRecord> Player::getTopCanonicalJournalRecords(const std::string& category, int limit) const
{
    std::vector<PlayerJournalRecord> records;
    if (limit <= 0)
    {
        return records;
    }

    for (const PlayerJournalRecord& record : canonicalJournalRecords)
    {
        if (record.category == category && record.count > 0)
        {
            records.push_back(record);
        }
    }

    std::sort(records.begin(), records.end(), [](const PlayerJournalRecord& a, const PlayerJournalRecord& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.lastDay != b.lastDay) return a.lastDay > b.lastDay;
        return a.label < b.label;
    });

    if (records.size() > static_cast<std::size_t>(limit))
    {
        records.resize(static_cast<std::size_t>(limit));
    }
    return records;
}


int Player::getCanonicalJournalCategoryTotal(const std::string& category) const
{
    int total = 0;
    for (const PlayerJournalRecord& record : canonicalJournalRecords)
    {
        if (record.category == category && record.count > 0)
        {
            total += record.count;
        }
    }
    return total;
}

void Player::recordEnemyEncounter(const std::string& enemyName, int amount)
{
    recordCanonicalEvent("ennemis_croises", enemyName, enemyName, amount);
}

void Player::recordEnemyKillByName(const std::string& enemyName, int amount)
{
    recordCanonicalEvent("ennemis_tues", enemyName, enemyName, amount);
}

void Player::recordBossKillByName(const std::string& bossName, int amount)
{
    recordCanonicalEvent("boss_tues", bossName, bossName, amount);
}

void Player::recordMaterialCollected(const std::string& materialId, const std::string& materialName, int quantity)
{
    recordCanonicalEvent("materiaux_ramasses", materialId.empty() ? materialName : materialId, materialName.empty() ? materialId : materialName, quantity);
}

void Player::recordConsumableUsed(const std::string& consumableName, int amount)
{
    recordCanonicalEvent("consommables_utilises", consumableName, consumableName, amount);
}

void Player::recordWeaponCategoryUsed(const std::string& categoryName, int amount)
{
    recordCanonicalEvent("categories_armes_utilisees", categoryName, categoryName, amount);
}

void Player::recordPnjServed(const std::string& pnjName, int amount)
{
    recordCanonicalEvent("pnj_servis", pnjName, pnjName, amount);
}

void Player::recordQuestTypeCompleted(const std::string& questTypeName, int amount)
{
    recordCanonicalEvent("types_quetes_completees", questTypeName, questTypeName, amount);
}

void Player::setLoadedCanonicalJournalRecords(const std::vector<PlayerJournalRecord>& records)
{
    canonicalJournalRecords.clear();
    for (const PlayerJournalRecord& record : records)
    {
        if (record.category.empty() || record.key.empty() || record.count <= 0)
        {
            continue;
        }
        PlayerJournalRecord clean = record;
        clean.label = clean.label.empty() ? clean.key : clean.label;
        clean.count = std::max(1, clean.count);
        clean.lastDay = std::max(0, clean.lastDay);
        canonicalJournalRecords.push_back(clean);
    }
    if (canonicalJournalRecords.size() > 260)
    {
        canonicalJournalRecords.resize(260);
    }
}

int Player::getWorldDayProgressUnits() const
{
    return worldDayProgressUnits;
}

int Player::getWorldDayUnitsPerDay() const
{
    return 5;
}

int Player::getCurrentWorldDayNumber() const
{
    return worldDaysElapsed + 1;
}

std::string Player::getCurrentWeekdayName() const
{
    static const std::vector<std::string> names = {
        "dimanche",
        "lundi",
        "mardi",
        "mercredi",
        "jeudi",
        "vendredi",
        "samedi"
    };

    const int dayIndex = ((getCurrentWorldDayNumber() - 1) % static_cast<int>(names.size()) + static_cast<int>(names.size())) % static_cast<int>(names.size());
    return names[static_cast<std::size_t>(dayIndex)];
}

std::string Player::getCurrentDayPartName() const
{
    const int unit = std::max(0, std::min(worldDayProgressUnits, getWorldDayUnitsPerDay() - 1));
    if (unit == 0) return "Matin";
    if (unit == 1) return "Midi";
    if (unit == 2) return "Après-midi";
    if (unit == 3) return "Soir";
    return "Nuit";
}

std::string Player::formatWorldDateLine() const
{
    return "Jour " + std::to_string(getCurrentWorldDayNumber()) + " (" + getCurrentWeekdayName() + ")";
}

std::string Player::formatWorldDayPartLine() const
{
    const int part = std::max(0, std::min(worldDayProgressUnits, getWorldDayUnitsPerDay() - 1)) + 1;
    return getCurrentDayPartName() + " " + std::to_string(part) + "/" + std::to_string(getWorldDayUnitsPerDay());
}

std::string Player::formatWorldDateTimeLine() const
{
    return formatWorldDateLine() + " — " + formatWorldDayPartLine();
}

std::string Player::formatWorldTimeChange(int beforeDay, int beforeProgress) const
{
    const int unitsPerDay = getWorldDayUnitsPerDay();
    if (beforeDay < 0) beforeDay = 0;
    if (beforeProgress < 0) beforeProgress = 0;
    if (beforeProgress >= unitsPerDay)
    {
        beforeDay += beforeProgress / unitsPerDay;
        beforeProgress %= unitsPerDay;
    }

    const int beforeDayNumber = beforeDay + 1;
    static const std::vector<std::string> names = {"dimanche", "lundi", "mardi", "mercredi", "jeudi", "vendredi", "samedi"};
    static const std::vector<std::string> moments = {"Matin", "Midi", "Après-midi", "Soir", "Nuit"};
    const int beforeWeekdayIndex = ((beforeDayNumber - 1) % static_cast<int>(names.size()) + static_cast<int>(names.size())) % static_cast<int>(names.size());
    const int beforeMomentIndex = std::max(0, std::min(beforeProgress, unitsPerDay - 1));
    const std::string beforeMoment = moments[static_cast<std::size_t>(beforeMomentIndex)];

    return "Date : Jour " + std::to_string(beforeDayNumber) + " (" + names[static_cast<std::size_t>(beforeWeekdayIndex)] + ") -> " + formatWorldDateLine()
        + " | Moment : " + beforeMoment + " " + std::to_string(beforeMomentIndex + 1) + "/" + std::to_string(unitsPerDay)
        + " -> " + formatWorldDayPartLine() + ".";
}

void Player::processEndOfWorldDay()
{
    int paidToday = 0;
    int renewedToday = 0;
    std::vector<std::string> dayLines;

    for (PlayerLocalSubscription& subscription : localSubscriptions)
    {
        if (subscription.expiresAtDay != worldDaysElapsed)
        {
            continue;
        }

        const std::string subscriptionName = subscription.name.empty() ? subscription.id : subscription.name;

        if (subscription.cancellationRequested)
        {
            dayLines.push_back("Abonnement terminé : " + subscriptionName + " (annulation demandée, aucun renouvellement payé).");
            continue;
        }

        int renewalPrice = subscription.renewalPrice > 0
            ? subscription.renewalPrice
            : defaultLocalSubscriptionRenewalPrice(subscription.id);

        if (renewalPrice <= 0)
        {
            dayLines.push_back("Abonnement expiré : " + subscriptionName + " (aucun tarif de renouvellement connu).");
            continue;
        }

        subscription.renewalPrice = renewalPrice;
        if (inventory.spendEconomyUnits(renewalPrice))
        {
            subscription.expiresAtDay = worldDaysElapsed + 7;
            subscription.cancellationRequested = false;
            paidToday += renewalPrice;
            ++renewedToday;
            dayLines.push_back(
                "Renouvellement payé : " + subscriptionName
                + " — " + Money::formatEconomyUnits(renewalPrice)
                + " (actif jusqu'à la fin du jour " + std::to_string(subscription.expiresAtDay + 1) + ")."
            );
        }
        else
        {
            dayLines.push_back(
                "Renouvellement impossible : " + subscriptionName
                + " demandait " + Money::formatEconomyUnits(renewalPrice)
                + ", mais les fonds sont insuffisants. L'abonnement s'arrête."
            );
        }
    }

    const int sleepPressure = getCursePressureForCategory("sleep");
    const int equipmentPressure = getCursePressureForCategory("equipment");
    const int socialPressure = getCursePressureForCategory("social");
    const int manaPressure = getCursePressureForCategory("mana");
    const int spiritPressure = getCursePressureForCategory("spirit");
    const int corruptionPressure = getCursePressureForCategory("corruption");
    const int interfacePressure = getCursePressureForCategory("interface");
    const int hallucinationPressure = getCursePressureForCategory("hallucination");
    if (sleepPressure + equipmentPressure + socialPressure + manaPressure + spiritPressure + corruptionPressure + interfacePressure + hallucinationPressure > 0)
    {
        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> chance(1, 100);
        if (sleepPressure > 0 && chance(generator) <= std::min(45, 12 + sleepPressure * 6))
        {
            dayLines.push_back(
                getKnownCursePressureForCategory("sleep") > 0
                    ? "Sommeil maudit : le repos compte, mais il laisse un rêve collé derrière les paupières."
                    : "Sommeil étrange : tu te réveilles avec l'impression d'avoir oublié un rêve important."
            );
        }
        if (equipmentPressure > 0 && chance(generator) <= std::min(35, 8 + equipmentPressure * 5))
        {
            dayLines.push_back(
                getKnownCursePressureForCategory("equipment") > 0
                    ? "Équipement maudit : l'arme ou l'armure répond avec un léger retard quand tu la ranges."
                    : "Équipement étrange : une boucle, une lame ou une couture semble avoir bougé sans raison."
            );
        }
        if (socialPressure > 0 && chance(generator) <= std::min(30, 7 + socialPressure * 4))
        {
            dayLines.push_back(
                getKnownCursePressureForCategory("social") > 0
                    ? "Présence sociale troublée : quelques regards restent trop longtemps sur toi."
                    : "Présence étrange : tu as l'impression qu'une conversation s'arrête quand tu arrives."
            );
        }
        if (manaPressure > 0 && chance(generator) <= std::min(35, 8 + manaPressure * 5))
        {
            dayLines.push_back(
                getKnownCursePressureForCategory("mana") > 0
                    ? "Mana instable : ta réserve magique semble respirer à contretemps."
                    : "Sensation étrange : quelque chose circule mal, sans que tu saches si c'est magique ou nerveux."
            );
        }
        if (spiritPressure > 0 && chance(generator) <= std::min(32, 7 + spiritPressure * 4))
        {
            dayLines.push_back(
                getKnownCursePressureForCategory("spirit") > 0
                    ? "Esprit troublé : une pensée étrangère revient toujours au même endroit."
                    : "Esprit étrange : tu perds une phrase simple, puis elle revient comme si quelqu'un l'avait gardée pour toi."
            );
        }
        if (corruptionPressure > 0 && chance(generator) <= std::min(30, 6 + corruptionPressure * 4))
        {
            dayLines.push_back(
                getKnownCursePressureForCategory("corruption") > 0
                    ? "Corruption diagnostiquée : la trace sombre ne progresse pas forcément, mais elle réagit au repos."
                    : "Sensation sombre : une fatigue sale reste sur la peau même après t'être posé."
            );
        }
        if (interfacePressure > 0 && chance(generator) <= std::min(42, 10 + interfacePressure * 5))
        {
            dayLines.push_back(
                getKnownCursePressureForCategory("interface") > 0
                    ? "Interface maudite : une ligne de menu apparaît hors combat, affiche [PV=???], puis se ferme toute seule."
                    : "Interface étrange : pendant une seconde, un choix inexistant semble flotter devant toi."
            );
        }
        if (hallucinationPressure > 0 && chance(generator) <= std::min(42, 10 + hallucinationPressure * 5))
        {
            dayLines.push_back(
                getKnownCursePressureForCategory("hallucination") > 0
                    ? "Hallucination diagnostiquée : les contours doublent brièvement, comme si ta vision refusait de choisir une seule réalité."
                    : "Trouble de la vision : un passant se dédouble, puis redevient normal quand tu clignes des yeux."
            );
        }
    }

    const int cityRepairDaysBefore = inventory.countMaterialById("city_repair_days_marker");
    if (cityRepairDaysBefore > 0)
    {
        inventory.removeMaterialQuantityById("city_repair_days_marker", 1);
        const int cityRepairDaysAfter = std::max(0, cityRepairDaysBefore - 1);
        if (cityRepairDaysAfter > 0)
        {
            dayLines.push_back("Ville : les réparations continuent. Boutiques ouvertes au compte-gouttes pendant encore " + std::to_string(cityRepairDaysAfter) + " jour(s).");
        }
        else
        {
            const int noticeCount = inventory.countMaterialById("city_damage_notice");
            if (noticeCount > 0)
            {
                inventory.removeMaterialQuantityById("city_damage_notice", noticeCount);
            }
            dayLines.push_back("Ville : les réparations principales sont terminées. Les boutiques reprennent un rythme normal.");
        }
    }

    const int cityEventRecentDaysBefore = inventory.countMaterialById("city_event_recent_days_marker");
    if (cityEventRecentDaysBefore > 0)
    {
        inventory.removeMaterialQuantityById("city_event_recent_days_marker", 1);
        const int cityEventRecentDaysAfter = std::max(0, cityEventRecentDaysBefore - 1);
        if (cityEventRecentDaysAfter > 0)
        {
            dayLines.push_back("Ville : les affiches récentes occupent encore les organisateurs pendant " + std::to_string(cityEventRecentDaysAfter) + " jour(s). Les événements exceptionnels restent moins probables.");
        }
    }

    const int defenseGratitudeDaysBefore = inventory.countMaterialById("city_defense_gratitude_days_marker");
    if (defenseGratitudeDaysBefore > 0)
    {
        inventory.removeMaterialQuantityById("city_defense_gratitude_days_marker", 1);
        const int defenseGratitudeDaysAfter = std::max(0, defenseGratitudeDaysBefore - 1);
        if (defenseGratitudeDaysAfter > 0)
        {
            dayLines.push_back("Ville : la reconnaissance après la défense tient encore " + std::to_string(defenseGratitudeDaysAfter) + " jour(s) chez les commerçants locaux.");
        }
        else
        {
            dayLines.push_back("Ville : les petites remises de défense s'effacent. Les marchands reviennent à leurs prix habituels.");
        }
    }

    if (!dayLines.empty())
    {
        pendingWorldTimeReportLines.push_back(
            "Fin de journée : " + std::to_string(renewedToday)
            + " renouvellement(s) payé(s), total " + Money::formatEconomyUnits(paidToday) + "."
        );
        pendingWorldTimeReportLines.insert(pendingWorldTimeReportLines.end(), dayLines.begin(), dayLines.end());
    }

    localSubscriptionRenewalPaidThisWeek += paidToday;
}

void Player::appendWeeklyRenewalSummaryIfNeeded()
{
    if (worldDaysElapsed > 0 && worldDaysElapsed % 7 == 0)
    {
        pendingWorldTimeReportLines.push_back(
            "Bilan de fin de semaine : renouvellements d'abonnements payés cette semaine = "
            + Money::formatEconomyUnits(localSubscriptionRenewalPaidThisWeek) + "."
        );
        localSubscriptionRenewalPaidThisWeek = 0;
    }
}

void Player::advanceWorldDays(int days)
{
    if (days <= 0)
    {
        return;
    }

    for (int i = 0; i < days; ++i)
    {
        processEndOfWorldDay();
        ++worldDaysElapsed;
        removeExpiredLocalSubscriptions();
        processCurseEscalations();
        removeExpiredCurses();
        appendWeeklyRenewalSummaryIfNeeded();
        maybeAppendCurseMalaiseLine(i + 1);
    }
}

void Player::advanceWorldDayUnits(int units)
{
    if (units <= 0)
    {
        return;
    }

    worldDayProgressUnits += units;

    const int unitsPerDay = getWorldDayUnitsPerDay();
    while (worldDayProgressUnits >= unitsPerDay)
    {
        processEndOfWorldDay();
        ++worldDaysElapsed;
        worldDayProgressUnits -= unitsPerDay;
        removeExpiredLocalSubscriptions();
        processCurseEscalations();
        removeExpiredCurses();
        appendWeeklyRenewalSummaryIfNeeded();
    }

    removeExpiredLocalSubscriptions();
    removeExpiredCurses();
    maybeAppendCurseMalaiseLine(units);
}

const std::vector<PlayerLocalSubscription>& Player::getLocalSubscriptions() const
{
    return localSubscriptions;
}

bool Player::hasActiveLocalSubscription(const std::string& subscriptionId) const
{
    for (const PlayerLocalSubscription& subscription : localSubscriptions)
    {
        if (subscription.id == subscriptionId && subscription.expiresAtDay >= worldDaysElapsed)
        {
            return true;
        }
    }

    return false;
}

bool Player::isLocalSubscriptionCancellationRequested(const std::string& subscriptionId) const
{
    for (const PlayerLocalSubscription& subscription : localSubscriptions)
    {
        if (subscription.id == subscriptionId && subscription.expiresAtDay >= worldDaysElapsed)
        {
            return subscription.cancellationRequested;
        }
    }

    return false;
}

int Player::getLocalSubscriptionExpiresAtDay(const std::string& subscriptionId) const
{
    for (const PlayerLocalSubscription& subscription : localSubscriptions)
    {
        if (subscription.id == subscriptionId && subscription.expiresAtDay >= worldDaysElapsed)
        {
            return subscription.expiresAtDay;
        }
    }

    return -1;
}

void Player::activateLocalSubscription(const std::string& subscriptionId, const std::string& name, int durationDays, int renewalPrice)
{
    if (subscriptionId.empty())
    {
        return;
    }

    if (durationDays <= 0)
    {
        durationDays = 7;
    }

    if (renewalPrice <= 0)
    {
        renewalPrice = defaultLocalSubscriptionRenewalPrice(subscriptionId);
    }

    const int newExpiresAtDay = worldDaysElapsed + durationDays - 1;
    for (PlayerLocalSubscription& subscription : localSubscriptions)
    {
        if (subscription.id == subscriptionId)
        {
            subscription.name = name.empty() ? subscription.id : name;
            subscription.expiresAtDay = newExpiresAtDay;
            subscription.cancellationRequested = false;
            subscription.renewalPrice = renewalPrice;
            return;
        }
    }

    PlayerLocalSubscription subscription;
    subscription.id = subscriptionId;
    subscription.name = name.empty() ? subscriptionId : name;
    subscription.expiresAtDay = newExpiresAtDay;
    subscription.cancellationRequested = false;
    subscription.renewalPrice = renewalPrice;
    localSubscriptions.push_back(subscription);
}

bool Player::requestLocalSubscriptionCancellation(const std::string& subscriptionId)
{
    for (PlayerLocalSubscription& subscription : localSubscriptions)
    {
        if (subscription.id == subscriptionId && subscription.expiresAtDay >= worldDaysElapsed)
        {
            subscription.cancellationRequested = true;
            return true;
        }
    }

    return false;
}

void Player::removeExpiredLocalSubscriptions()
{
    localSubscriptions.erase(
        std::remove_if(localSubscriptions.begin(), localSubscriptions.end(), [this](const PlayerLocalSubscription& subscription) {
            return subscription.expiresAtDay >= 0 && subscription.expiresAtDay < worldDaysElapsed;
        }),
        localSubscriptions.end()
    );
}

void Player::setLoadedLocalSubscriptions(const std::vector<PlayerLocalSubscription>& subscriptions)
{
    localSubscriptions.clear();
    for (PlayerLocalSubscription subscription : subscriptions)
    {
        if (subscription.id.empty() || subscription.expiresAtDay < worldDaysElapsed)
        {
            continue;
        }

        if (subscription.name.empty())
        {
            subscription.name = subscription.id;
        }
        if (subscription.renewalPrice <= 0)
        {
            subscription.renewalPrice = defaultLocalSubscriptionRenewalPrice(subscription.id);
        }
        localSubscriptions.push_back(subscription);
    }
}


const std::vector<PlayerCurse>& Player::getActiveCurses() const
{
    return activeCurses;
}

int Player::getActiveCurseCount() const
{
    return static_cast<int>(activeCurses.size());
}

bool Player::hasActiveCurse(const std::string& curseId) const
{
    for (const PlayerCurse& curse : activeCurses)
    {
        if (curse.id == curseId)
        {
            return true;
        }
    }

    return false;
}

bool Player::addOrRefreshCurse(const PlayerCurse& curse)
{
    if (curse.id.empty())
    {
        return false;
    }

    PlayerCurse prepared = curse;
    if (prepared.name.empty()) prepared.name = prepared.id;
    if (prepared.appliedAtDay < 0) prepared.appliedAtDay = worldDaysElapsed;
    prepared.exorcismProgress = std::max(0, prepared.exorcismProgress);
    prepared.exorcismRequiredVisits = std::max(0, prepared.exorcismRequiredVisits);
    prepared.diagnosisLevel = std::max(0, std::min(3, prepared.diagnosisLevel));
    prepared.curseLevel = std::max(1, prepared.curseLevel);
    prepared.maxCurseLevel = std::max(prepared.curseLevel, prepared.maxCurseLevel);
    prepared.escalationIntervalDays = std::max(0, prepared.escalationIntervalDays);
    if (prepared.evolvesOverTime && prepared.nextEscalationDay < 0 && prepared.escalationIntervalDays > 0 && prepared.curseLevel < prepared.maxCurseLevel)
    {
        prepared.nextEscalationDay = worldDaysElapsed + prepared.escalationIntervalDays;
    }
    if (!prepared.evolvesOverTime)
    {
        prepared.nextEscalationDay = -1;
    }
    refreshCursePresentationFromLevel(prepared);

    for (PlayerCurse& active : activeCurses)
    {
        if (active.id == prepared.id)
        {
            const int preservedProgress = std::max(active.exorcismProgress, prepared.exorcismProgress);
            const int preservedDiagnosis = std::max(active.diagnosisLevel, prepared.diagnosisLevel);
            const std::string preservedDiscovered = active.discoveredSymptomCategories.empty() ? prepared.discoveredSymptomCategories : active.discoveredSymptomCategories;
            const std::string preservedExcluded = active.excludedSymptomCategories.empty() ? prepared.excludedSymptomCategories : active.excludedSymptomCategories;
            active = prepared;
            active.diagnosisLevel = std::min(3, preservedDiagnosis);
            active.discoveredSymptomCategories = preservedDiscovered;
            active.excludedSymptomCategories = preservedExcluded;
            active.exorcismProgress = std::min(preservedProgress, std::max(0, active.exorcismRequiredVisits));
            refreshCursePresentationFromLevel(active);
            if (active.id == "lethal_survival_scar")
            {
                setHealingReceivedPercent(95);
            }
            if (activeCurses.size() >= 3)
            {
                grantTitle("Triplement maudit");
            }
            return false;
        }
    }

    activeCurses.push_back(prepared);
    if (prepared.id == "lethal_survival_scar")
    {
        setHealingReceivedPercent(95);
    }
    if (activeCurses.size() >= 3)
    {
        grantTitle("Triplement maudit");
    }
    return true;
}

bool Player::removeCurse(const std::string& curseId)
{
    if (curseId == "lethal_survival_scar")
    {
        return false;
    }

    const std::size_t before = activeCurses.size();
    activeCurses.erase(
        std::remove_if(activeCurses.begin(), activeCurses.end(), [&curseId](const PlayerCurse& curse) {
            return curse.id == curseId;
        }),
        activeCurses.end()
    );

    return activeCurses.size() != before;
}

int Player::removeExpiredCurses()
{
    const std::size_t before = activeCurses.size();
    activeCurses.erase(
        std::remove_if(activeCurses.begin(), activeCurses.end(), [this](const PlayerCurse& curse) {
            return curse.expiresAtDay >= 0 && curse.expiresAtDay < worldDaysElapsed;
        }),
        activeCurses.end()
    );

    return static_cast<int>(before - activeCurses.size());
}

void Player::setLoadedCurses(const std::vector<PlayerCurse>& curses)
{
    activeCurses.clear();
    setHealingReceivedPercent(100);
    for (PlayerCurse curse : curses)
    {
        if (curse.id.empty())
        {
            continue;
        }
        if (curse.expiresAtDay >= 0 && curse.expiresAtDay < worldDaysElapsed)
        {
            continue;
        }
        if (curse.name.empty()) curse.name = curse.id;
        if (curse.appliedAtDay < 0) curse.appliedAtDay = worldDaysElapsed;
        curse.exorcismProgress = std::max(0, curse.exorcismProgress);
        curse.exorcismRequiredVisits = std::max(0, curse.exorcismRequiredVisits);
        curse.diagnosisLevel = std::max(0, std::min(3, curse.diagnosisLevel));
        curse.curseLevel = std::max(1, curse.curseLevel);
        curse.maxCurseLevel = std::max(curse.curseLevel, curse.maxCurseLevel);
        curse.escalationIntervalDays = std::max(0, curse.escalationIntervalDays);
        if (!curse.evolvesOverTime)
        {
            curse.nextEscalationDay = -1;
        }
        refreshCursePresentationFromLevel(curse);
        if (curse.exorcismRequiredVisits > 0)
        {
            curse.exorcismProgress = std::min(curse.exorcismProgress, curse.exorcismRequiredVisits);
        }
        activeCurses.push_back(curse);
        if (curse.id == "lethal_survival_scar")
        {
            setHealingReceivedPercent(95);
        }
    }
    if (activeCurses.size() >= 3)
    {
        grantTitle("Triplement maudit");
    }
}

const std::vector<Blessing>& Player::getActiveBlessings() const
{
    return blessingInventory.getAll();
}

int Player::getActiveBlessingCount() const
{
    return blessingInventory.count();
}

bool Player::hasBlessing(const std::string& blessingId) const
{
    return blessingInventory.contains(blessingId);
}

bool Player::hasLethalSurvivalBlessing() const
{
    return blessingInventory.hasLethalSurvivalProtection();
}

bool Player::canReceiveBlessings() const
{
    return !hasActiveCurse("lethal_survival_scar");
}

bool Player::addBlessing(const Blessing& blessing)
{
    if (!canReceiveBlessings())
    {
        return false;
    }
    return blessingInventory.add(blessing);
}

void Player::consumeAllBlessings()
{
    blessingInventory.clear();
}

void Player::setLoadedBlessings(const std::vector<Blessing>& blessings)
{
    if (!canReceiveBlessings())
    {
        blessingInventory.clear();
        return;
    }
    blessingInventory.setLoaded(blessings);
}

bool Player::revealCurseSymptomCategory(const std::string& curseId, const std::string& category)
{
    for (PlayerCurse& curse : activeCurses)
    {
        if (curse.id != curseId)
        {
            continue;
        }

        if (!curseTokenListContains(curse.symptomCategories, category))
        {
            return false;
        }

        addCurseTokenToList(curse.discoveredSymptomCategories, category);
        if (curse.diagnosisLevel < 1)
        {
            curse.diagnosisLevel = 1;
        }
        return true;
    }

    return false;
}

bool Player::excludeCurseSymptomCategory(const std::string& curseId, const std::string& category)
{
    for (PlayerCurse& curse : activeCurses)
    {
        if (curse.id != curseId)
        {
            continue;
        }

        if (curseTokenListContains(curse.symptomCategories, category))
        {
            return false;
        }

        return addCurseTokenToList(curse.excludedSymptomCategories, category);
    }

    return false;
}

int Player::autoExcludeWrongCurseSymptomCategories(const std::string& curseId, int percentToExclude)
{
    static const std::vector<std::string> allCategories = {
        "health", "attack", "mana", "precision", "defense", "sleep",
        "luck", "equipment", "spirit", "corruption", "travel", "social"
    };

    percentToExclude = std::max(0, std::min(100, percentToExclude));
    for (PlayerCurse& curse : activeCurses)
    {
        if (curse.id != curseId)
        {
            continue;
        }

        std::vector<std::string> candidates;
        for (const std::string& category : allCategories)
        {
            if (curseTokenListContains(curse.symptomCategories, category)) continue;
            if (curseTokenListContains(curse.excludedSymptomCategories, category)) continue;
            candidates.push_back(category);
        }

        if (candidates.empty())
        {
            return 0;
        }

        static std::mt19937 generator(std::random_device{}());
        std::shuffle(candidates.begin(), candidates.end(), generator);
        const int toRemove = std::max(1, static_cast<int>((candidates.size() * percentToExclude + 99) / 100));
        int removed = 0;
        for (const std::string& category : candidates)
        {
            if (removed >= toRemove) break;
            if (addCurseTokenToList(curse.excludedSymptomCategories, category))
            {
                ++removed;
            }
        }
        return removed;
    }

    return 0;
}

bool Player::setCurseDiagnosisLevel(const std::string& curseId, int level)
{
    level = std::max(0, std::min(3, level));
    for (PlayerCurse& curse : activeCurses)
    {
        if (curse.id == curseId)
        {
            if (level <= curse.diagnosisLevel)
            {
                return false;
            }
            curse.diagnosisLevel = level;
            return true;
        }
    }

    return false;
}

bool Player::advanceChurchExorcism(const std::string& curseId)
{
    for (std::size_t i = 0; i < activeCurses.size(); ++i)
    {
        PlayerCurse& curse = activeCurses[i];
        if (curse.id != curseId)
        {
            continue;
        }
        if (!curse.removableByChurch || curse.bossIdRequiredToBreak > 0 || curse.diagnosisLevel < 1)
        {
            return false;
        }

        if (curse.exorcismRequiredVisits <= 1)
        {
            activeCurses.erase(activeCurses.begin() + static_cast<std::ptrdiff_t>(i));
            return true;
        }

        curse.exorcismProgress = std::min(curse.exorcismRequiredVisits, curse.exorcismProgress + 1);
        if (curse.exorcismProgress >= curse.exorcismRequiredVisits)
        {
            activeCurses.erase(activeCurses.begin() + static_cast<std::ptrdiff_t>(i));
        }
        return true;
    }

    return false;
}

int Player::processCurseEscalations()
{
    int escalated = 0;
    for (PlayerCurse& curse : activeCurses)
    {
        if (!curse.evolvesOverTime || curse.escalationIntervalDays <= 0 || curse.curseLevel >= curse.maxCurseLevel)
        {
            continue;
        }

        if (curse.nextEscalationDay < 0)
        {
            curse.nextEscalationDay = curse.appliedAtDay + curse.escalationIntervalDays;
        }

        if (worldDaysElapsed < curse.nextEscalationDay)
        {
            continue;
        }

        while (curse.curseLevel < curse.maxCurseLevel
            && curse.nextEscalationDay >= 0
            && worldDaysElapsed >= curse.nextEscalationDay)
        {
            ++curse.curseLevel;
            ++escalated;
            curse.nextEscalationDay += curse.escalationIntervalDays;
        }

        refreshCursePresentationFromLevel(curse);

        if (curse.becomesSpecialRemovalWhenTooHigh && curse.curseLevel > curse.churchRemovalMaxLevel)
        {
            pendingWorldTimeReportLines.push_back(
                "Aggravation : une malédiction a dépassé ce que l'église peut retirer seule. Il faudra une autre solution."
            );
        }
        else
        {
            pendingWorldTimeReportLines.push_back(
                "Aggravation : une trace maudite semble plus lourde qu'hier. Un diagnostic à l'église serait prudent."
            );
        }
    }

    return escalated;
}

int Player::removeCursesLockedByBoss(int bossId)
{
    if (bossId <= 0)
    {
        return 0;
    }

    const std::size_t before = activeCurses.size();
    activeCurses.erase(
        std::remove_if(activeCurses.begin(), activeCurses.end(), [bossId](const PlayerCurse& curse) {
            return curse.bossIdRequiredToBreak == bossId;
        }),
        activeCurses.end()
    );

    return static_cast<int>(before - activeCurses.size());
}


bool Player::hasCurseEligibleForSpecialSolution(const std::string& solutionId) const
{
    for (const PlayerCurse& curse : activeCurses)
    {
        if (curse.diagnosisLevel < 3)
        {
            continue;
        }

        if (solutionId == "destroy_source_object"
            && (curse.id == "haunted_chest_echo" || curse.id == "anchored_relic_shadow" || curse.id == "cursed_equipment_whisper"))
        {
            return true;
        }

        if (solutionId == "break_oath" && curse.id == "oath_binding_trace")
        {
            return true;
        }

        if (solutionId == "break_pact" && curse.id == "voluntary_pact_mark")
        {
            return true;
        }

        if (solutionId == "read_counter_legend"
            && (curse.id == "unread_legend_weight" || curse.id == "boss_threshold_omen"))
        {
            return true;
        }

        if (solutionId == "seal_source" && (curse.id == "boss_threshold_omen" || curse.id == "abandoned_altar_brand"))
        {
            return true;
        }

        if (solutionId == "confirm_source_defeated" && curse.id == "boss_threshold_omen")
        {
            return true;
        }
    }

    return false;
}

int Player::resolveSpecialCurseSolution(const std::string& solutionId)
{
    const std::size_t before = activeCurses.size();

    activeCurses.erase(
        std::remove_if(activeCurses.begin(), activeCurses.end(), [&solutionId](const PlayerCurse& curse) {
            if (curse.diagnosisLevel < 3)
            {
                return false;
            }

            if (solutionId == "destroy_source_object")
            {
                return curse.id == "haunted_chest_echo" || curse.id == "anchored_relic_shadow" || curse.id == "cursed_equipment_whisper";
            }

            if (solutionId == "break_oath")
            {
                return curse.id == "oath_binding_trace";
            }

            if (solutionId == "break_pact")
            {
                return curse.id == "voluntary_pact_mark";
            }

            if (solutionId == "read_counter_legend")
            {
                return curse.id == "unread_legend_weight" || curse.id == "boss_threshold_omen";
            }

            if (solutionId == "seal_source")
            {
                return curse.id == "boss_threshold_omen" || curse.id == "abandoned_altar_brand";
            }

            if (solutionId == "confirm_source_defeated")
            {
                return curse.id == "boss_threshold_omen";
            }

            return false;
        }),
        activeCurses.end()
    );

    return static_cast<int>(before - activeCurses.size());
}

int Player::getCursePressureForCategory(const std::string& category) const
{
    if (category.empty())
    {
        return 0;
    }

    int pressure = 0;
    for (const PlayerCurse& curse : activeCurses)
    {
        if (curseTokenListContains(curse.symptomCategories, category))
        {
            pressure += std::max(1, curse.curseLevel);
        }
    }

    return std::min(12, pressure);
}

int Player::getKnownCursePressureForCategory(const std::string& category) const
{
    if (category.empty())
    {
        return 0;
    }

    int pressure = 0;
    for (const PlayerCurse& curse : activeCurses)
    {
        if (!curseTokenListContains(curse.symptomCategories, category))
        {
            continue;
        }

        const bool categoryKnown = curse.diagnosisLevel >= 3
            || curseTokenListContains(curse.discoveredSymptomCategories, category);
        if (categoryKnown)
        {
            pressure += std::max(1, curse.curseLevel);
        }
    }

    return std::min(12, pressure);
}

bool Player::hasHighRunicBacklashNeedingEnchanter() const
{
    for (const PlayerCurse& curse : activeCurses)
    {
        if (curse.id == "runic_backlash"
            && curse.becomesSpecialRemovalWhenTooHigh
            && curse.curseLevel > curse.churchRemovalMaxLevel)
        {
            return true;
        }
    }

    return false;
}

bool Player::stabilizeHighRunicBacklashForEnchanter()
{
    for (PlayerCurse& curse : activeCurses)
    {
        if (curse.id != "runic_backlash"
            || !curse.becomesSpecialRemovalWhenTooHigh
            || curse.curseLevel <= curse.churchRemovalMaxLevel)
        {
            continue;
        }

        curse.removableByChurch = true;
        curse.lifeLong = false;
        curse.expiresAtDay = -1;
        curse.exorcismProgress = 0;
        curse.exorcismRequiredVisits = std::max(curse.exorcismRequiredVisits, 3);
        curse.removalHint = "retourner à l'église après stabilisation runique pour accomplir un rite total.";
        curse.highLevelRemovalHint = "la rune est stabilisée : l'église peut reprendre le relais, mais pas en un seul passage.";
        curse.churchRemovalMaxLevel = std::max(curse.churchRemovalMaxLevel, curse.curseLevel);
        refreshCursePresentationFromLevel(curse);
        return true;
    }

    return false;
}

std::vector<std::string> Player::describeActiveCurses() const
{
    std::vector<std::string> lines;
    if (activeCurses.empty())
    {
        lines.push_back("Aucune malédiction active connue.");
        return lines;
    }

    for (const PlayerCurse& curse : activeCurses)
    {
        const bool unknown = curse.diagnosisLevel <= 0;
        std::string line = "- " + (unknown ? "?????" : curse.name);
        line += " | connaissance : " + std::to_string(std::max(0, std::min(3, curse.diagnosisLevel))) + "/3";
        if (curse.diagnosisLevel >= 1)
        {
            line += " | niveau : " + std::to_string(std::max(1, curse.curseLevel)) + "/" + std::to_string(std::max(1, curse.maxCurseLevel));
        }

        if (curse.diagnosisLevel >= 2 && !curse.origin.empty())
        {
            line += " | origine : " + curse.origin;
        }

        if (curse.diagnosisLevel >= 2)
        {
            if (curse.evolvesOverTime && curse.curseLevel < curse.maxCurseLevel && curse.nextEscalationDay >= 0)
            {
                line += " | aggravation possible après le jour " + std::to_string(curse.nextEscalationDay + 1);
            }
            if (curse.expiresAtDay >= 0)
            {
                line += " | expire après le jour " + std::to_string(curse.expiresAtDay + 1);
            }
            else if (curse.lifeLong)
            {
                line += " | durée : vie entière / indéfinie";
            }
            else
            {
                line += " | durée : indéfinie";
            }
        }
        else
        {
            line += " | durée : ?????";
        }
        lines.push_back(line);

        const std::vector<std::string> discovered = splitCurseTokenList(curse.discoveredSymptomCategories);
        if (!discovered.empty())
        {
            std::string categoriesLine = "  Symptômes confirmés : ";
            for (std::size_t i = 0; i < discovered.size(); ++i)
            {
                if (i > 0) categoriesLine += ", ";
                categoriesLine += curseCategoryLabel(discovered[i]);
            }
            lines.push_back(categoriesLine + ".");
        }
        else
        {
            lines.push_back(unknown ? "  Symptômes : ?????" : "  Symptômes : piste encore vague, rien de précis confirmé.");
        }

        const std::vector<std::string> excluded = splitCurseTokenList(curse.excludedSymptomCategories);
        if (!excluded.empty())
        {
            std::string excludedLine = "  Pistes presque écartées : ";
            for (std::size_t i = 0; i < excluded.size(); ++i)
            {
                if (i > 0) excludedLine += ", ";
                excludedLine += curseCategoryLabel(excluded[i]);
            }
            lines.push_back(excludedLine + ".");
        }

        if (curse.diagnosisLevel >= 3 && !curse.description.empty())
        {
            lines.push_back("  Lecture complète : " + curse.description);
        }

        if (curse.diagnosisLevel < 1)
        {
            lines.push_back("  Exorcisme : refusé tant que la trace n'est pas connue au moins au niveau 1.");
        }
        else if (curse.bossIdRequiredToBreak > 0)
        {
            lines.push_back("  Ancrage hors combat : la trace reste sur le personnage jusqu'à ce que le boss lié soit rebattu.");
            lines.push_back("  Exorcisme : impossible ici. Condition : " + (curse.diagnosisLevel >= 3 ? curse.removalHint : "source trop forte, recherche totale conseillée."));
        }
        else if (curse.removableByChurch)
        {
            if (curse.evolvesOverTime && curse.becomesSpecialRemovalWhenTooHigh && curse.curseLevel <= curse.churchRemovalMaxLevel)
            {
                lines.push_back("  Attention : si cette malédiction monte trop haut, l'église risque de ne plus suffire.");
            }
            if (curse.exorcismRequiredVisits > 1)
            {
                lines.push_back("  Exorcisme : " + std::to_string(curse.exorcismProgress) + "/" + std::to_string(curse.exorcismRequiredVisits) + " passage(s) terminé(s).");
            }
            else
            {
                lines.push_back("  Exorcisme : retirable par un prêtre/exorciste.");
            }
        }
        else if (!curse.removalHint.empty() && curse.diagnosisLevel >= 3)
        {
            lines.push_back("  Condition spéciale : " + curse.removalHint);
        }
        else
        {
            lines.push_back("  Condition spéciale : ?????");
        }
    }

    return lines;
}

void Player::recordLyknirDefeatCurse()
{
    PlayerCurse curse;
    curse.id = "lyknir_prey_mark";
    curse.name = "Marque de proie de Lyknir";
    curse.origin = "Écho de Lyknir";
    curse.description = "La meute garde ton odeur. Les prêtres peuvent la reconnaître, mais aucun rite ordinaire ne la rompt.";
    curse.removalHint = "vaincre Lyknir et reprendre symboliquement ta place de chasseur.";
    curse.severity = "majeure";
    curse.symptomCategories = "spirit,travel,precision";
    curse.discoveredSymptomCategories = "";
    curse.excludedSymptomCategories = "";
    curse.diagnosisLevel = 0;
    curse.appliedAtDay = worldDaysElapsed;
    curse.expiresAtDay = -1;
    curse.exorcismProgress = 0;
    curse.exorcismRequiredVisits = 0;
    curse.curseLevel = 1;
    curse.maxCurseLevel = 3;
    curse.evolvesOverTime = true;
    curse.escalationIntervalDays = 3;
    curse.nextEscalationDay = worldDaysElapsed + curse.escalationIntervalDays;
    curse.churchRemovalMaxLevel = 0;
    curse.becomesSpecialRemovalWhenTooHigh = false;
    curse.highLevelRemovalHint = "vaincre Lyknir : plus la chasse dure, plus son odeur reste sur toi.";
    curse.removableByChurch = false;
    curse.bossIdRequiredToBreak = 4;
    curse.lifeLong = false;
    addOrRefreshCurse(curse);
}

void Player::maybeAppendCurseMalaiseLine(int salt)
{
    if (activeCurses.empty())
    {
        return;
    }

    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> chance(1, 100);

    const PlayerCurse* anomalyCurse = nullptr;
    for (const PlayerCurse& activeCurse : activeCurses)
    {
        if (activeCurse.id == "anomaly_interface_desync" || activeCurse.id == "anomaly_source_core_desync")
        {
            anomalyCurse = &activeCurse;
            break;
        }
    }

    if (anomalyCurse != nullptr)
    {
        const int anomalyLevel = std::max(1, anomalyCurse->curseLevel);
        const int anomalyChance = std::min(45, 12 + anomalyLevel * 6);
        if (chance(generator) <= anomalyChance)
        {
            std::vector<std::vector<std::string>> anomalyPanels = {
                {
                    "[INTERFACE] Rapport hors combat remplacé pendant une seconde.",
                    "1 : Continuer / 2 : Se retourner / 3 : Ne pas croire tes yeux / 4 : ▓▒░??",
                    "Trouble de la vision : les contours se décalent, puis le menu se recale avec un léger retard."
                },
                {
                    "[PV=???] [ZONE=ville?] [CIBLE=toi/eux/personne]",
                    "Une ligne parasite traverse l'air : 0xAN0M4L13::hors_combat::regarde_mieux",
                    "L'affichage se referme avant de pouvoir mentir plus fort."
                },
                {
                    "Pendant un instant, la rue prend le cadre d'une arène PvE.",
                    "Cibles détectées : 5. Cibles réelles : 0 ou 1. Confiance : non.",
                    "Les passants redeviennent des passants, mais ton interface garde une trace de griffure."
                },
                {
                    "Le titre du menu tremble : FIN DE JOURNÉE / FIN DE TOI / FIN DE ???",
                    "L'Anomalie grésille : \"Même sans combat, tu cherches encore quel bouton va te sauver ?\"",
                    "Les caractères se rangent d'eux-mêmes avant que la corruption devienne dangereuse."
                },
                {
                    "Un faux choix apparaît hors combat : 1 Attaquer, 2 Fuir, 3 S'excuser auprès de soi-même.",
                    "Ton instinct comprend que ce n'est qu'un mensonge visuel, pas un vrai ordre.",
                    "L'Anomalie souffle dans le texte : \"Tu hésites même quand rien ne t'attaque.\""
                },
                {
                    "▓▒░ RAPPORT_CASSÉ ░▒▓  NOM=" + getName() + " / NOM=??? / NOM=la cible",
                    "Trouble de la vision : ton ombre semble sélectionner une option à ta place.",
                    "L'interface revient lentement, comme si elle n'était pas certaine de t'appartenir."
                }
            };

            if (anomalyCurse->diagnosisLevel <= 0)
            {
                anomalyPanels.push_back({
                    "[INTERFACE] Une couche inconnue se superpose au monde hors combat.",
                    "Tu n'as pas encore de nom propre à mettre sur ce parasite visuel.",
                    "Le menu disparaît en laissant seulement quelques caractères : ?A?N?O?"
                });
            }
            else
            {
                anomalyPanels.push_back({
                    "Désynchronisation confirmée : l'Anomalie mord encore l'interface hors combat.",
                    "Tant qu'elle n'est pas rebattue, les menus peuvent clignoter, mentir ou provoquer sans prévenir.",
                    "La perturbation reste moins violente qu'en combat, mais elle n'a pas disparu."
                });
            }

            if (anomalyLevel >= 3)
            {
                anomalyPanels.push_back({
                    "[ERREUR VISUELLE PERSISTANTE] Journal / Inventaire / Cible / Toi : ordre illisible.",
                    "L'Anomalie chuchote : \"Je n'ai pas besoin d'être dans l'arène pour toucher tes yeux.\"",
                    "Quelques symboles restent imprimés derrière tes paupières : ░0░x░A░N░0░"
                });
            }

            std::uniform_int_distribution<int> anomalyPick(0, static_cast<int>(anomalyPanels.size()) - 1);
            const std::vector<std::string>& pickedPanel = anomalyPanels[static_cast<std::size_t>(anomalyPick(generator))];
            pendingWorldTimeReportLines.insert(pendingWorldTimeReportLines.end(), pickedPanel.begin(), pickedPanel.end());
            return;
        }
    }

    if (chance(generator) > 12)
    {
        return;
    }

    const PlayerCurse& curse = activeCurses[static_cast<std::size_t>(std::abs(salt + worldDaysElapsed + worldDayProgressUnits)) % activeCurses.size()];
    std::vector<std::string> lines;
    if (curse.diagnosisLevel <= 0)
    {
        lines = {
            "Ton personnage murmure : \"Je ne sais pas ce que j'ai... mais quelque chose cloche.\"",
            "Ton personnage ralentit un instant : \"Pourquoi j'ai l'impression que mon corps ment ?\"",
            "Ton personnage serre les dents : \"Ce n'est pas de la fatigue normale... enfin, je crois.\""
        };
    }
    else if (curseTokenListContains(curse.discoveredSymptomCategories, "mana"))
    {
        lines = {"Ton personnage frissonne : \"Ma magie ne circule pas comme d'habitude...\""};
    }
    else if (curseTokenListContains(curse.discoveredSymptomCategories, "health"))
    {
        lines = {"Ton personnage souffle : \"J'ai l'impression d'être malade sans vraiment l'être.\""};
    }
    else if (curseTokenListContains(curse.discoveredSymptomCategories, "sleep"))
    {
        lines = {
            "Ton personnage cligne des yeux : \"J'ai rêvé d'un endroit où je n'ai jamais mis les pieds...\"",
            "Ton personnage murmure : \"Même réveillé, j'ai l'impression qu'un cauchemar continue.\""
        };
    }
    else if (curseTokenListContains(curse.discoveredSymptomCategories, "equipment"))
    {
        lines = {"Ton personnage vérifie son équipement : \"Quelque chose répond trop lentement quand je le touche.\""};
    }
    else if (curseTokenListContains(curse.discoveredSymptomCategories, "interface"))
    {
        lines = {
            "Ton personnage fixe le vide : \"L'interface vient d'afficher un choix qui n'existe pas...\"",
            "Ton personnage cligne des yeux : \"Pourquoi mon nom vient d'apparaître à la place de la cible ?\""
        };
    }
    else if (curseTokenListContains(curse.discoveredSymptomCategories, "hallucination"))
    {
        lines = {
            "Ton personnage se retourne : \"J'ai entendu des pas derrière moi... alors qu'il n'y a personne.\"",
            "Ton personnage murmure : \"J'ai vu une équipe ennemie entière, puis plus rien.\""
        };
    }
    else if (curseTokenListContains(curse.discoveredSymptomCategories, "social"))
    {
        lines = {"Ton personnage baisse la voix : \"Les gens me regardent bizarrement... ou alors je l'imagine.\""};
    }
    else if (curseTokenListContains(curse.discoveredSymptomCategories, "spirit"))
    {
        lines = {"Ton personnage se fige : \"J'ai pensé quelque chose... mais ce n'était pas vraiment ma phrase.\""};
    }
    else if (curseTokenListContains(curse.discoveredSymptomCategories, "corruption"))
    {
        lines = {"Ton personnage essuie sa paume : \"Cette trace laisse une sensation sale, même quand rien ne se voit.\""};
    }
    else if (curseTokenListContains(curse.discoveredSymptomCategories, "luck"))
    {
        lines = {"Ton personnage soupire : \"Même les petits hasards tombent mal aujourd'hui.\""};
    }
    else if (curseTokenListContains(curse.discoveredSymptomCategories, "travel"))
    {
        lines = {"Ton personnage regarde derrière lui : \"La route donne l'impression de ne pas vouloir finir.\""};
    }
    else if (curseTokenListContains(curse.discoveredSymptomCategories, "attack") || curseTokenListContains(curse.discoveredSymptomCategories, "precision"))
    {
        lines = {"Ton personnage regarde ses mains : \"Mes gestes sont bizarres aujourd'hui...\""};
    }
    else
    {
        lines = {
            "Ton personnage murmure : \"Cette trace ne me laisse pas tranquille.\"",
            "Ton personnage détourne le regard : \"Je me sens étrange... mais pas assez pour comprendre pourquoi.\""
        };
    }

    if (!lines.empty())
    {
        std::uniform_int_distribution<int> pick(0, static_cast<int>(lines.size()) - 1);
        pendingWorldTimeReportLines.push_back(lines[static_cast<std::size_t>(pick(generator))]);
    }
}

int Player::getLocalSubscriptionRenewalPaidThisWeek() const
{
    return localSubscriptionRenewalPaidThisWeek;
}

void Player::setLocalSubscriptionRenewalPaidThisWeek(int amount)
{
    localSubscriptionRenewalPaidThisWeek = std::max(0, amount);
}

const std::vector<std::string>& Player::getPendingWorldTimeReportLines() const
{
    return pendingWorldTimeReportLines;
}

std::vector<std::string> Player::consumeWorldTimeReportLines()
{
    std::vector<std::string> lines = pendingWorldTimeReportLines;
    pendingWorldTimeReportLines.clear();
    return lines;
}

// EN: getVictories declares or implements a focused behavior used by this module.
// FR: getVictories déclare ou implémente un comportement précis utilisé par ce module.
int Player::getVictories() const
{
    return victories;
}

// EN: getDefeats declares or implements a focused behavior used by this module.
// FR: getDefeats déclare ou implémente un comportement précis utilisé par ce module.
int Player::getDefeats() const
{
    return defeats;
}

// EN: getEscapes declares or implements a focused behavior used by this module.
// FR: getEscapes déclare ou implémente un comportement précis utilisé par ce module.
int Player::getEscapes() const
{
    return escapes;
}

// EN: getDeaths declares or implements a focused behavior used by this module.
// FR: getDeaths déclare ou implémente un comportement précis utilisé par ce module.
int Player::getDeaths() const
{
    return deaths;
}

// EN: getEnemiesKilled declares or implements a focused behavior used by this module.
// FR: getEnemiesKilled déclare ou implémente un comportement précis utilisé par ce module.
int Player::getEnemiesKilled() const
{
    return enemiesKilled;
}

// EN: getBossesKilled declares or implements a focused behavior used by this module.
// FR: getBossesKilled déclare ou implémente un comportement précis utilisé par ce module.
int Player::getBossesKilled() const
{
    return bossesKilled;
}

// EN: getPvpVictories declares or implements a focused behavior used by this module.
// FR: getPvpVictories déclare ou implémente un comportement précis utilisé par ce module.
int Player::getPvpVictories() const
{
    return pvpVictories;
}

// EN: getPvpDefeats declares or implements a focused behavior used by this module.
// FR: getPvpDefeats déclare ou implémente un comportement précis utilisé par ce module.
int Player::getPvpDefeats() const
{
    return pvpDefeats;
}

// EN: getPvpLethalEliminations declares or implements a focused behavior used by this module.
// FR: getPvpLethalEliminations déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<std::string>& Player::getPvpLethalEliminations() const
{
    return pvpLethalEliminations;
}


// EN: getUnlockedBossIds declares or implements a focused behavior used by this module.
// FR: getUnlockedBossIds déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<int>& Player::getUnlockedBossIds() const
{
    return unlockedBossIds;
}

// EN: getRecentBossIds declares or implements a focused behavior used by this module.
// FR: getRecentBossIds déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<int>& Player::getRecentBossIds() const
{
    return recentBossIds;
}

int Player::getRecentBossCooldownExpiresAtDay() const
{
    return recentBossCooldownExpiresAtDay;
}

int Player::getRareBossDiscoveryCooldownExpiresAtDay() const
{
    return rareBossDiscoveryCooldownExpiresAtDay;
}

bool Player::canUseRareBossDiscovery() const
{
    return rareBossDiscoveryCooldownExpiresAtDay < 0
        || worldDaysElapsed >= rareBossDiscoveryCooldownExpiresAtDay;
}

// EN: getRecentCombatEquipmentUsage declares or implements a focused behavior used by this module.
// FR: getRecentCombatEquipmentUsage déclare ou implémente un comportement précis utilisé par ce module.
const std::vector<std::string>& Player::getRecentCombatEquipmentUsage() const
{
    return recentCombatEquipmentUsage;
}

// EN: recordCurrentEquipmentUsage declares or implements a focused behavior used by this module.
// FR: recordCurrentEquipmentUsage déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordCurrentEquipmentUsage()
{
    std::string label;

    if (hasEquippedWeapon())
    {
        label += "Arme:" + getEquippedWeapon().getName();
    }

    if (hasEquippedArmor())
    {
        if (!label.empty())
        {
            label += " | ";
        }
        label += "Armure:" + getEquippedArmor().getName();
    }

    if (label.empty())
    {
        label = "Aucun équipement majeur";
    }

    recentCombatEquipmentUsage.push_back(label);

    while (recentCombatEquipmentUsage.size() > 10)
    {
        recentCombatEquipmentUsage.erase(recentCombatEquipmentUsage.begin());
    }
}

// EN: setLoadedRecentCombatEquipmentUsage declares or implements a focused behavior used by this module.
// FR: setLoadedRecentCombatEquipmentUsage déclare ou implémente un comportement précis utilisé par ce module.
void Player::setLoadedRecentCombatEquipmentUsage(const std::vector<std::string>& usage)
{
    recentCombatEquipmentUsage.clear();

    std::size_t start = usage.size() > 10 ? usage.size() - 10 : 0;
    for (std::size_t i = start; i < usage.size(); ++i)
    {
        if (!usage[i].empty())
        {
            recentCombatEquipmentUsage.push_back(usage[i]);
        }
    }
}

// EN: hasBossEquipmentSeal declares or implements a focused behavior used by this module.
// FR: hasBossEquipmentSeal déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasBossEquipmentSeal() const
{
    return bossEquipmentSealActive;
}

std::string Player::getBossEquipmentSealReason() const
{
    return bossEquipmentSealReason;
}

// EN: activateBossEquipmentSeal declares or implements a focused behavior used by this module.
// FR: activateBossEquipmentSeal déclare ou implémente un comportement précis utilisé par ce module.
void Player::activateBossEquipmentSeal(const std::string& reason)
{
    bossEquipmentSealActive = true;
    bossEquipmentSealReason = reason;
}

// EN: clearBossEquipmentSeal declares or implements a focused behavior used by this module.
// FR: clearBossEquipmentSeal déclare ou implémente un comportement précis utilisé par ce module.
void Player::clearBossEquipmentSeal()
{
    bossEquipmentSealActive = false;
    bossEquipmentSealReason = "";
}

// EN: isBossUnlocked declares or implements a focused behavior used by this module.
// FR: isBossUnlocked déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isBossUnlocked(int bossId) const
{
    for (int id : unlockedBossIds)
    {
        if (id == bossId)
        {
            return true;
        }
    }

    return false;
}

bool Player::unlockBoss(int bossId)
{
    if (bossId <= 0 || bossId > 36 || isBossUnlocked(bossId))
    {
        return false;
    }

    unlockedBossIds.push_back(bossId);
    std::sort(unlockedBossIds.begin(), unlockedBossIds.end());
    return true;
}

// EN: isBossRecentlyDefeated declares or implements a focused behavior used by this module.
// FR: isBossRecentlyDefeated déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isBossRecentlyDefeated(int bossId) const
{
    if (recentBossCooldownExpiresAtDay >= 0 && worldDaysElapsed >= recentBossCooldownExpiresAtDay)
    {
        return false;
    }

    for (int id : recentBossIds)
    {
        if (id == bossId)
        {
            return true;
        }
    }

    return false;
}

bool Player::isBossDefeated(int bossId) const
{
    return std::find(defeatedBossIds.begin(), defeatedBossIds.end(), bossId) != defeatedBossIds.end();
}

const std::vector<int>& Player::getDefeatedBossIds() const
{
    return defeatedBossIds;
}

bool hasDefeatedEveryBossExceptFinalInternal(const std::vector<int>& defeatedIds, int finalBossId)
{
    const int maximumBossId = 36;

    for (int id = 1; id <= maximumBossId; ++id)
    {
        if (id == finalBossId)
        {
            continue;
        }

        if (std::find(defeatedIds.begin(), defeatedIds.end(), id) == defeatedIds.end())
        {
            return false;
        }
    }

    return true;
}

std::vector<int> Player::getAvailableBossIds() const
{
    std::vector<int> available;

    for (int id : unlockedBossIds)
    {
        if (!isBossRecentlyDefeated(id))
        {
            available.push_back(id);
        }
    }

    if (available.empty())
    {
        return unlockedBossIds;
    }

    return available;
}

// EN: unlockNextBossVariation declares or implements a focused behavior used by this module.
// FR: unlockNextBossVariation déclare ou implémente un comportement précis utilisé par ce module.
bool Player::unlockNextBossVariation()
{
    // FireFlight is not a normal ordered reveal. The final entry opens only when every
    // other required boss is genuinely defeated.
    if (!isBossUnlocked(27) && hasDefeatedEveryBossExceptFinalInternal(defeatedBossIds, 27))
    {
        unlockedBossIds.push_back(27);
        std::sort(unlockedBossIds.begin(), unlockedBossIds.end());
        return true;
    }

    // Internal progression order. It is never exposed as a complete list to the player.
    // One unique victory normally advances one position. If that position was already
    // defeated through story/coop, the registry may look only three entries farther.
    const std::vector<int> progressionOrder = {
        4, 5, 6,
        7, 8, 9,
        10, 12,
        13, 14, 15,
        16, 17, 18,
        19, 20, 21,
        22, 24, 25,
        28, 29, 31,
        32, 33, 34, 35,
        23,
        30,
        11,
        26,
        36,
        27
    };

    const int uniqueVictoryCount = static_cast<int>(defeatedBossIds.size());
    if (uniqueVictoryCount <= 0)
    {
        return false;
    }

    const std::size_t expectedIndex = static_cast<std::size_t>(uniqueVictoryCount - 1);
    if (expectedIndex >= progressionOrder.size())
    {
        return false;
    }

    constexpr std::size_t maximumForwardSkip = 3;
    const std::size_t lastIndex = std::min(progressionOrder.size() - 1, expectedIndex + maximumForwardSkip);

    for (std::size_t index = expectedIndex; index <= lastIndex; ++index)
    {
        const int nextId = progressionOrder[index];

        // Only an entry already defeated can be skipped. An already known but undefeated
        // entry is still the current target, so no farther information is revealed.
        if (isBossDefeated(nextId))
        {
            continue;
        }
        if (isBossUnlocked(nextId))
        {
            return false;
        }

        // FireFlight remains outside normal progression. Its entry only stabilizes after
        // every other required boss has genuinely been defeated.
        if (nextId == 27 && !hasDefeatedEveryBossExceptFinalInternal(defeatedBossIds, 27))
        {
            return false;
        }

        unlockedBossIds.push_back(nextId);
        std::sort(unlockedBossIds.begin(), unlockedBossIds.end());
        return true;
    }

    // More than three already-defeated entries separate the player from the next unknown
    // presence: this victory gives no additional location or registry information.
    return false;
}

bool Player::unlockNextBossVariationFromRareDiscovery(const std::string& location, int cooldownDays)
{
    if (!canUseRareBossDiscovery())
    {
        return false;
    }

    // The final FireFlight condition can never be consumed by an exploration clue.
    if (hasDefeatedEveryBossExceptFinalInternal(defeatedBossIds, 27))
    {
        return false;
    }

    const std::vector<int> previouslyUnlocked = unlockedBossIds;
    if (!unlockNextBossVariation())
    {
        return false;
    }

    int newlyUnlockedBossId = 0;
    for (int id : unlockedBossIds)
    {
        if (std::find(previouslyUnlocked.begin(), previouslyUnlocked.end(), id) == previouslyUnlocked.end())
        {
            newlyUnlockedBossId = id;
            break;
        }
    }

    if (newlyUnlockedBossId > 0)
    {
        if (bossDiscoveryLocations.size() <= static_cast<std::size_t>(newlyUnlockedBossId))
        {
            bossDiscoveryLocations.resize(static_cast<std::size_t>(newlyUnlockedBossId) + 1);
        }
        bossDiscoveryLocations[static_cast<std::size_t>(newlyUnlockedBossId)] = location.empty()
            ? "Zone non précisée"
            : location;
    }

    rareBossDiscoveryCooldownExpiresAtDay = worldDaysElapsed + std::max(1, cooldownDays);
    return true;
}

std::string Player::getBossDiscoveryLocation(int bossId) const
{
    if (bossId <= 0 || static_cast<std::size_t>(bossId) >= bossDiscoveryLocations.size())
    {
        return "";
    }
    return bossDiscoveryLocations[static_cast<std::size_t>(bossId)];
}

const std::vector<std::string>& Player::getBossDiscoveryLocations() const
{
    return bossDiscoveryLocations;
}

void Player::setLoadedBossDiscoveryLocations(const std::vector<std::string>& locations)
{
    bossDiscoveryLocations.assign(37, "");
    const std::size_t limit = std::min<std::size_t>(bossDiscoveryLocations.size(), locations.size());
    for (std::size_t index = 1; index < limit; ++index)
    {
        bossDiscoveryLocations[index] = locations[index];
    }
}

bool Player::wasExplorationEventRecentlySeen(const std::string& key) const
{
    return !key.empty() && std::find(recentExplorationEventKeys.begin(), recentExplorationEventKeys.end(), key) != recentExplorationEventKeys.end();
}

bool Player::wasExplorationChallengeRecentlySeen(const std::string& key) const
{
    return !key.empty() && std::find(recentExplorationChallengeKeys.begin(), recentExplorationChallengeKeys.end(), key) != recentExplorationChallengeKeys.end();
}

void Player::recordExplorationEventKey(const std::string& key)
{
    if (key.empty()) return;
    recentExplorationEventKeys.erase(
        std::remove(recentExplorationEventKeys.begin(), recentExplorationEventKeys.end(), key),
        recentExplorationEventKeys.end()
    );
    recentExplorationEventKeys.push_back(key);
    while (recentExplorationEventKeys.size() > 6)
    {
        recentExplorationEventKeys.erase(recentExplorationEventKeys.begin());
    }
}

void Player::recordExplorationChallengeKey(const std::string& key)
{
    if (key.empty()) return;
    recentExplorationChallengeKeys.erase(
        std::remove(recentExplorationChallengeKeys.begin(), recentExplorationChallengeKeys.end(), key),
        recentExplorationChallengeKeys.end()
    );
    recentExplorationChallengeKeys.push_back(key);
    while (recentExplorationChallengeKeys.size() > 10)
    {
        recentExplorationChallengeKeys.erase(recentExplorationChallengeKeys.begin());
    }
}

const std::vector<std::string>& Player::getRecentExplorationEventKeys() const
{
    return recentExplorationEventKeys;
}

const std::vector<std::string>& Player::getRecentExplorationChallengeKeys() const
{
    return recentExplorationChallengeKeys;
}

void Player::setLoadedExplorationVarietyHistory(
    const std::vector<std::string>& eventKeys,
    const std::vector<std::string>& challengeKeys
)
{
    recentExplorationEventKeys.clear();
    recentExplorationChallengeKeys.clear();
    for (const std::string& key : eventKeys) recordExplorationEventKey(key);
    for (const std::string& key : challengeKeys) recordExplorationChallengeKey(key);
}

bool Player::isExplorationSceneOnCooldown(const std::string& key) const
{
    if (key.empty()) return false;
    for (const PlayerExplorationCooldown& cooldown : explorationSceneCooldowns)
    {
        if (cooldown.key == key && cooldown.expiresAtDay > worldDaysElapsed)
        {
            return true;
        }
    }
    return false;
}

int Player::getExplorationSceneCooldownRemainingDays(const std::string& key) const
{
    if (key.empty()) return 0;
    for (const PlayerExplorationCooldown& cooldown : explorationSceneCooldowns)
    {
        if (cooldown.key == key)
        {
            return std::max(0, cooldown.expiresAtDay - worldDaysElapsed);
        }
    }
    return 0;
}

void Player::startExplorationSceneCooldown(const std::string& key, int durationDays)
{
    if (key.empty() || durationDays <= 0) return;
    const int expiresAtDay = worldDaysElapsed + durationDays;
    for (PlayerExplorationCooldown& cooldown : explorationSceneCooldowns)
    {
        if (cooldown.key == key)
        {
            cooldown.expiresAtDay = std::max(cooldown.expiresAtDay, expiresAtDay);
            return;
        }
    }
    explorationSceneCooldowns.push_back({key, expiresAtDay});
}

const std::vector<PlayerExplorationCooldown>& Player::getExplorationSceneCooldowns() const
{
    return explorationSceneCooldowns;
}

void Player::setLoadedExplorationSceneCooldowns(const std::vector<PlayerExplorationCooldown>& cooldowns)
{
    explorationSceneCooldowns.clear();
    for (const PlayerExplorationCooldown& cooldown : cooldowns)
    {
        if (cooldown.key.empty() || cooldown.expiresAtDay <= worldDaysElapsed) continue;
        startExplorationSceneCooldown(cooldown.key, cooldown.expiresAtDay - worldDaysElapsed);
    }
}

// EN: recordBossVictoryInRegistry declares or implements a focused behavior used by this module.
// FR: recordBossVictoryInRegistry déclare ou implémente un comportement précis utilisé par ce module.
bool Player::recordBossVictoryInRegistry(int bossId)
{
    if (bossId <= 0)
    {
        return false;
    }

    const bool firstVictory = !isBossDefeated(bossId);
    if (firstVictory)
    {
        // A boss reached through story, coop or an exceptional route must remain visible
        // in the registry after the victory, even if its entry was not unlocked normally.
        unlockBoss(bossId);
        defeatedBossIds.push_back(bossId);
        std::sort(defeatedBossIds.begin(), defeatedBossIds.end());
    }

    recentBossIds.push_back(bossId);

    while (recentBossIds.size() > 2)
    {
        recentBossIds.erase(recentBossIds.begin());
    }

    recentBossCooldownExpiresAtDay = worldDaysElapsed + 3;

    // Repeating an already defeated boss can still give rewards and clear its curse,
    // but it must never reveal another registry entry.
    if (!firstVictory)
    {
        return false;
    }

    return unlockNextBossVariation();
}

// EN: setLoadedBossRegistry declares or implements a focused behavior used by this module.
// FR: setLoadedBossRegistry déclare ou implémente un comportement précis utilisé par ce module.
void Player::setLoadedBossRegistry(const std::vector<int>& unlockedIds, const std::vector<int>& recentIds, const std::vector<int>& defeatedIds, int recentCooldownExpiresAtDay, int rareDiscoveryCooldownExpiresAtDay)
{
    unlockedBossIds.clear();

    if (unlockedIds.empty())
    {
        unlockedBossIds = {1, 2, 3};
    }
    else
    {
        for (int id : unlockedIds)
        {
            if (id > 0 && !isBossUnlocked(id))
            {
                unlockedBossIds.push_back(id);
            }
        }
    }

    for (int baseId = 1; baseId <= 3; ++baseId)
    {
        if (!isBossUnlocked(baseId))
        {
            unlockedBossIds.push_back(baseId);
        }
    }

    recentBossIds.clear();

    for (int id : recentIds)
    {
        if (id > 0 && isBossUnlocked(id))
        {
            recentBossIds.push_back(id);
        }
    }

    while (recentBossIds.size() > 2)
    {
        recentBossIds.erase(recentBossIds.begin());
    }

    defeatedBossIds.clear();
    for (int id : defeatedIds)
    {
        if (id > 0 && !isBossDefeated(id))
        {
            defeatedBossIds.push_back(id);
            if (!isBossUnlocked(id))
            {
                unlockedBossIds.push_back(id);
            }
        }
    }
    std::sort(unlockedBossIds.begin(), unlockedBossIds.end());
    std::sort(defeatedBossIds.begin(), defeatedBossIds.end());

    this->recentBossCooldownExpiresAtDay = recentCooldownExpiresAtDay;
    this->rareBossDiscoveryCooldownExpiresAtDay = rareDiscoveryCooldownExpiresAtDay;
}


// EN: hasZelefCorrosionPresent declares or implements a focused behavior used by this module.
// FR: hasZelefCorrosionPresent déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasZelefCorrosionPresent() const
{
    return zelefCorrosionPresent;
}

// EN: getZelefMaxHpStolen declares or implements a focused behavior used by this module.
// FR: getZelefMaxHpStolen déclare ou implémente un comportement précis utilisé par ce module.
int Player::getZelefMaxHpStolen() const
{
    return zelefMaxHpStolen;
}

// EN: hasGrinkaBossTheftPresent declares or implements a focused behavior used by this module.
// FR: hasGrinkaBossTheftPresent déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasGrinkaBossTheftPresent() const
{
    return grinkaBossTheftPresent;
}

// EN: getGrinkaStolenGold declares or implements a focused behavior used by this module.
// FR: getGrinkaStolenGold déclare ou implémente un comportement précis utilisé par ce module.
int Player::getGrinkaStolenGold() const
{
    return grinkaStolenGold;
}

// EN: hasGrinkaStolenWeapon declares or implements a focused behavior used by this module.
// FR: hasGrinkaStolenWeapon déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasGrinkaStolenWeapon() const
{
    return grinkaHasStolenWeapon;
}

// EN: hasGrinkaStolenArmor declares or implements a focused behavior used by this module.
// FR: hasGrinkaStolenArmor déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasGrinkaStolenArmor() const
{
    return grinkaHasStolenArmor;
}

// EN: getGrinkaStolenWeapon declares or implements a focused behavior used by this module.
// FR: getGrinkaStolenWeapon déclare ou implémente un comportement précis utilisé par ce module.
Weapon Player::getGrinkaStolenWeapon() const
{
    return grinkaStolenWeapon;
}

// EN: getGrinkaStolenArmor declares or implements a focused behavior used by this module.
// FR: getGrinkaStolenArmor déclare ou implémente un comportement précis utilisé par ce module.
Armor Player::getGrinkaStolenArmor() const
{
    return grinkaStolenArmor;
}

// EN: recordZelefCorrosionLoss declares or implements a focused behavior used by this module.
// FR: recordZelefCorrosionLoss déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordZelefCorrosionLoss(int maxHpLoss)
{
    if (maxHpLoss <= 0)
    {
        return;
    }

    zelefCorrosionPresent = true;
    zelefMaxHpStolen += maxHpLoss;
}

// EN: restoreZelefCorrosionLoss declares or implements a focused behavior used by this module.
// FR: restoreZelefCorrosionLoss déclare ou implémente un comportement précis utilisé par ce module.
void Player::restoreZelefCorrosionLoss()
{
    if (!zelefCorrosionPresent || zelefMaxHpStolen <= 0)
    {
        zelefCorrosionPresent = false;
        zelefMaxHpStolen = 0;
        return;
    }

    maxHp += zelefMaxHpStolen;
    hp += zelefMaxHpStolen;
    zelefCorrosionPresent = false;
    zelefMaxHpStolen = 0;
}

// EN: recordGrinkaGoldTheft declares or implements a focused behavior used by this module.
// FR: recordGrinkaGoldTheft déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordGrinkaGoldTheft(int goldAmount)
{
    if (goldAmount <= 0)
    {
        return;
    }

    grinkaBossTheftPresent = true;
    grinkaStolenGold += goldAmount;
}

// EN: recordGrinkaStolenWeapon declares or implements a focused behavior used by this module.
// FR: recordGrinkaStolenWeapon déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordGrinkaStolenWeapon(const Weapon& weapon)
{
    grinkaBossTheftPresent = true;
    grinkaHasStolenWeapon = true;
    grinkaStolenWeapon = weapon;
}

// EN: recordGrinkaStolenArmor declares or implements a focused behavior used by this module.
// FR: recordGrinkaStolenArmor déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordGrinkaStolenArmor(const Armor& armor)
{
    grinkaBossTheftPresent = true;
    grinkaHasStolenArmor = true;
    grinkaStolenArmor = armor;
}

// EN: restoreGrinkaBossTheft declares or implements a focused behavior used by this module.
// FR: restoreGrinkaBossTheft déclare ou implémente un comportement précis utilisé par ce module.
void Player::restoreGrinkaBossTheft()
{
    if (!grinkaBossTheftPresent)
    {
        return;
    }

    if (grinkaStolenGold > 0)
    {
        inventory.earnEconomyUnits(grinkaStolenGold);
        refreshCurrencyTitles();
    }

    if (grinkaHasStolenWeapon)
    {
        inventory.addWeapon(grinkaStolenWeapon);
    }

    if (grinkaHasStolenArmor)
    {
        inventory.addArmor(grinkaStolenArmor);
    }

    grinkaBossTheftPresent = false;
    grinkaStolenGold = 0;
    grinkaHasStolenWeapon = false;
    grinkaHasStolenArmor = false;
}

void Player::setLoadedBossConsequences(
    bool zelefCorrosion,
    int zelefMaxHpLoss,
    bool grinkaTheft,
    int grinkaGoldLoss,
    bool hasStolenWeapon,
    const Weapon& stolenWeapon,
    bool hasStolenArmor,
    const Armor& stolenArmor
)
{
    zelefCorrosionPresent = zelefCorrosion;
    zelefMaxHpStolen = zelefMaxHpLoss < 0 ? 0 : zelefMaxHpLoss;

    grinkaBossTheftPresent = grinkaTheft;
    grinkaStolenGold = grinkaGoldLoss < 0 ? 0 : grinkaGoldLoss;
    grinkaHasStolenWeapon = hasStolenWeapon;
    grinkaStolenWeapon = stolenWeapon;
    grinkaHasStolenArmor = hasStolenArmor;
    grinkaStolenArmor = stolenArmor;
}


// EN: getLethalCheatAttemptCount declares or implements a focused behavior used by this module.
// FR: getLethalCheatAttemptCount déclare ou implémente un comportement précis utilisé par ce module.
int Player::getLethalCheatAttemptCount() const
{
    return lethalCheatAttemptCount;
}

// EN: getWorldGazeCombatsRemaining declares or implements a focused behavior used by this module.
// FR: getWorldGazeCombatsRemaining déclare ou implémente un comportement précis utilisé par ce module.
int Player::getWorldGazeCombatsRemaining() const
{
    return worldGazeCombatsRemaining;
}

// EN: getWorldGazeMaxHpPenalty declares or implements a focused behavior used by this module.
// FR: getWorldGazeMaxHpPenalty déclare ou implémente un comportement précis utilisé par ce module.
int Player::getWorldGazeMaxHpPenalty() const
{
    return worldGazeMaxHpPenalty;
}

// EN: hasWorldGazePenalty declares or implements a focused behavior used by this module.
// FR: hasWorldGazePenalty déclare ou implémente un comportement précis utilisé par ce module.
bool Player::hasWorldGazePenalty() const
{
    return worldGazeCombatsRemaining > 0 || worldGazeMaxHpPenalty > 0;
}

// EN: setLethalCheatPenaltyState declares or implements a focused behavior used by this module.
// FR: setLethalCheatPenaltyState déclare ou implémente un comportement précis utilisé par ce module.
void Player::setLethalCheatPenaltyState(int attempts, int remainingCombats, int maxHpPenalty)
{
    lethalCheatAttemptCount = attempts < 0 ? 0 : attempts;
    worldGazeCombatsRemaining = remainingCombats < 0 ? 0 : remainingCombats;
    worldGazeMaxHpPenalty = maxHpPenalty < 0 ? 0 : maxHpPenalty;
}

// EN: applyLethalCheatAttemptPenalty declares or implements a focused behavior used by this module.
// FR: applyLethalCheatAttemptPenalty déclare ou implémente un comportement précis utilisé par ce module.
void Player::applyLethalCheatAttemptPenalty()
{
    lethalCheatAttemptCount++;

    int directDamage = hp / 2;
    if (directDamage < 1 && hp > 1)
    {
        directDamage = 1;
    }
    hp -= directDamage;
    if (hp < 1)
    {
        hp = 1;
    }

    const long long currentCopper = inventory.getTotalCopper();
    const long long copperLoss = currentCopper / 4;
    inventory.spendCopper(copperLoss);

    if (worldGazeMaxHpPenalty > 0)
    {
        applyFlatStatBonus(worldGazeMaxHpPenalty, 0, 0, 0);
        worldGazeMaxHpPenalty = 0;
    }

    int maxHpLoss = maxHp * 15 / 100;
    if (maxHpLoss < 1)
    {
        maxHpLoss = 1;
    }

    reduceMaxHp(maxHpLoss);
    worldGazeMaxHpPenalty = maxHpLoss;
    worldGazeCombatsRemaining = 3;
}

// EN: reduceWorldGazeDurationAfterCombat declares or implements a focused behavior used by this module.
// FR: reduceWorldGazeDurationAfterCombat déclare ou implémente un comportement précis utilisé par ce module.
void Player::reduceWorldGazeDurationAfterCombat()
{
    if (worldGazeCombatsRemaining <= 0)
    {
        return;
    }

    worldGazeCombatsRemaining--;

    if (worldGazeCombatsRemaining == 0 && worldGazeMaxHpPenalty > 0)
    {
        applyFlatStatBonus(worldGazeMaxHpPenalty, 0, 0, 0);
        worldGazeMaxHpPenalty = 0;
        MessageScreen::show(
            "REGARD DU MONDE",
            "player.world_gaze.removed",
            {"Le Regard du monde se retire. Tes PV maximum reviennent à leur état précédent."},
            false
        );
    }
}

// EN: isClone declares or implements a focused behavior used by this module.
// FR: isClone déclare ou implémente un comportement précis utilisé par ce module.
bool Player::isClone() const
{
    return cloneCharacter;
}

// EN: setClone declares or implements a focused behavior used by this module.
// FR: setClone déclare ou implémente un comportement précis utilisé par ce module.
void Player::setClone(bool clone)
{
    cloneCharacter = clone;
}

void Player::setLoadedStatistics(
    int loadedCombatsStarted,
    int loadedWorldDaysElapsed,
    int loadedWorldDayProgressUnits,
    int loadedVictories,
    int loadedDefeats,
    int loadedEscapes,
    int loadedDeaths,
    int loadedEnemiesKilled,
    int loadedBossesKilled,
    int loadedPvpVictories,
    int loadedPvpDefeats
)
{
    combatsStarted = loadedCombatsStarted < 0 ? 0 : loadedCombatsStarted;
    worldDaysElapsed = loadedWorldDaysElapsed < 0 ? combatsStarted : loadedWorldDaysElapsed;
    if (worldDaysElapsed < combatsStarted)
    {
        worldDaysElapsed = combatsStarted;
    }

    worldDayProgressUnits = loadedWorldDayProgressUnits < 0 ? 0 : loadedWorldDayProgressUnits;
    if (worldDayProgressUnits >= getWorldDayUnitsPerDay())
    {
        worldDaysElapsed += worldDayProgressUnits / getWorldDayUnitsPerDay();
        worldDayProgressUnits %= getWorldDayUnitsPerDay();
    }

    victories = loadedVictories < 0 ? 0 : loadedVictories;
    defeats = loadedDefeats < 0 ? 0 : loadedDefeats;
    escapes = loadedEscapes < 0 ? 0 : loadedEscapes;
    deaths = loadedDeaths < 0 ? 0 : loadedDeaths;
    enemiesKilled = loadedEnemiesKilled < 0 ? 0 : loadedEnemiesKilled;
    bossesKilled = loadedBossesKilled < 0 ? 0 : loadedBossesKilled;
    pvpVictories = loadedPvpVictories < 0 ? 0 : loadedPvpVictories;
    pvpDefeats = loadedPvpDefeats < 0 ? 0 : loadedPvpDefeats;
}

// EN: recordCombatStarted declares or implements a focused behavior used by this module.
// FR: recordCombatStarted déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordCombatStarted()
{
    combatsStarted++;
    advanceWorldDays(1);
    resetClassSkillCooldown();
    recordCurrentEquipmentUsage();
    refreshCareerSkillProgress();
}

// EN: recordVictory declares or implements a focused behavior used by this module.
// FR: recordVictory déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordVictory()
{
    victories++;
    recordWeaponCategoryUsed(playerWeaponCategoryLabel(getEquippedWeapon().getType()));
    reduceWorldGazeDurationAfterCombat();
    refreshCareerSkillProgress();
}

// EN: recordDefeat declares or implements a focused behavior used by this module.
// FR: recordDefeat déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordDefeat()
{
    defeats++;
    reduceWorldGazeDurationAfterCombat();
    refreshCareerSkillProgress();
}

// EN: recordEscape declares or implements a focused behavior used by this module.
// FR: recordEscape déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordEscape()
{
    escapes++;
    reduceWorldGazeDurationAfterCombat();
    refreshCareerSkillProgress();
}

// EN: recordDeath declares or implements a focused behavior used by this module.
// FR: recordDeath déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordDeath()
{
    deaths++;
    if (deaths >= 1)
    {
        grantTitle("Première chute officielle");
        grantTitle("Survivant administratif");
        grantTitle("J'ai glissé chef");
    }
    refreshCareerSkillProgress();
}

// EN: recordEnemyKills declares or implements a focused behavior used by this module.
// FR: recordEnemyKills déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordEnemyKills(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    enemiesKilled += amount;

    if (enemiesKilled >= 10)
    {
        grantTitle("Ça compte comme entraînement");
    }
    if (enemiesKilled >= 100)
    {
        grantTitle("Nettoyeur professionnel");
    }
    if (enemiesKilled >= 1000)
    {
        grantTitle("Le compteur a abandonné");
    }
    if (enemiesKilled >= 10000)
    {
        grantTitle("Pourquoi ils étaient autant ?");
    }

    recordGameplaySkillProgressForKills(amount);
}

// EN: recordBossKill declares or implements a focused behavior used by this module.
// FR: recordBossKill déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordBossKill()
{
    bossesKilled++;
    refreshCareerSkillProgress();
}

// EN: recordPvpVictory declares or implements a focused behavior used by this module.
// FR: recordPvpVictory déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordPvpVictory()
{
    pvpVictories++;
}

// EN: recordPvpDefeat declares or implements a focused behavior used by this module.
// FR: recordPvpDefeat déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordPvpDefeat()
{
    pvpDefeats++;
}

// EN: recordPvpLethalElimination declares or implements a focused behavior used by this module.
// FR: recordPvpLethalElimination déclare ou implémente un comportement précis utilisé par ce module.
void Player::recordPvpLethalElimination(const std::string& characterName, const std::string& accountName)
{
    std::string label = characterName + " | compte : " + accountName;
    pvpLethalEliminations.push_back(label);
}

// EN: setLoadedPvpLethalEliminations declares or implements a focused behavior used by this module.
// FR: setLoadedPvpLethalEliminations déclare ou implémente un comportement précis utilisé par ce module.
void Player::setLoadedPvpLethalEliminations(const std::vector<std::string>& eliminations)
{
    pvpLethalEliminations = eliminations;
}

// EN: displayCareerStatistics declares or implements a focused behavior used by this module.
// FR: displayCareerStatistics déclare ou implémente un comportement précis utilisé par ce module.
void Player::displayCareerStatistics(DifficultyMode difficulty) const
{
    std::vector<std::string> lines;
    lines.push_back("Date actuelle : " + formatWorldDateLine());
    lines.push_back("Moment actuel : " + formatWorldDayPartLine());
    lines.push_back("Jours écoulés : " + std::to_string(worldDaysElapsed));
    lines.push_back("Combats lancés : " + std::to_string(combatsStarted));
    lines.push_back("Victoires : " + std::to_string(victories));
    lines.push_back("Défaites : " + std::to_string(defeats));
    lines.push_back("Fuites : " + std::to_string(escapes));

    if (difficulty == DifficultyMode::Lethal)
    {
        lines.push_back("Morts du personnage : [STATISTIQUE CORROMPUE]");
        lines.push_back("But de mission : survivre.");
        lines.push_back("Vous ne deviez pas mourir.");
    }
    else
    {
        lines.push_back("Morts du personnage : " + std::to_string(deaths));
    }

    lines.push_back("Ennemis tués : " + std::to_string(enemiesKilled));
    lines.push_back("Boss vaincus : " + std::to_string(bossesKilled));
    lines.push_back("JcJ remportés : " + std::to_string(pvpVictories));
    lines.push_back("JcJ perdus : " + std::to_string(pvpDefeats));

    if (!pvpLethalEliminations.empty())
    {
        lines.push_back("");
        lines.push_back("Éliminations JcJ Léthal :");

        for (const std::string& elimination : pvpLethalEliminations)
        {
            lines.push_back("- " + elimination);
        }
    }

    PlayerUiSupport::showPlayerScreen("STATISTIQUES DE PARCOURS", "player.career_statistics", lines, false);
}


// EN: isAlteredByCheats declares or implements a focused behavior used by this module.
// FR: isAlteredByCheats déclare ou implémente un comportement précis utilisé par ce module.
void Player::takeDamage(int damage)
{
    if (godModeEnabled && damage > 0)
    {
        MessageScreen::show(
            "MODE GOD",
            "player.god_mode.damage_refused",
            {name + " devrait perdre " + std::to_string(damage) + " PV, mais le mode god refuse la réalité."},
            false
        );
        return;
    }

    const int hpBefore = hp;
    Entity::takeDamage(damage);
    if (challengeCombatTrackingActive && hpBefore > hp)
    {
        challengeCombatDamageTaken += hpBefore - hp;
    }
}

void Player::applyFlatStatBonus(
    int maxHpBonus,
    int minDamageBonus,
    int maxDamageBonus,
    int criticalDamageBonus
)
{
    maxHp += maxHpBonus;
    hp += maxHpBonus;

    minDamage += minDamageBonus;
    maxDamage += maxDamageBonus;
    this->criticalDamage += criticalDamageBonus;

    if (maxHp < 1)
    {
        maxHp = 1;
    }

    if (hp < 1)
    {
        hp = 1;
    }

    if (minDamage < 0)
    {
        minDamage = 0;
    }

    if (maxDamage < minDamage)
    {
        maxDamage = minDamage;
    }

    if (this->criticalDamage < maxDamage)
    {
        this->criticalDamage = maxDamage;
    }
}

// EN: getInventory declares or implements a focused behavior used by this module.
// FR: getInventory déclare ou implémente un comportement précis utilisé par ce module.
QuestLog& Player::getQuestLog()
{
    return questLog;
}

// EN: getQuestLog declares or implements a focused behavior used by this module.
// FR: getQuestLog déclare ou implémente un comportement précis utilisé par ce module.
const QuestLog& Player::getQuestLog() const
{
    return questLog;
}

// EN: getEquippedWeaponIndex declares or implements a focused behavior used by this module.
// FR: getEquippedWeaponIndex déclare ou implémente un comportement précis utilisé par ce module.
void Player::gainExperience(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    experience += amount;

    while (level < MAX_LEVEL)
    {
        int experienceRequired = Level::getExperienceRequiredForNextLevel(level);

        if (experienceRequired <= 0 || experience < experienceRequired)
        {
            break;
        }

        experience -= experienceRequired;
        levelUp();
    }

    if (level >= MAX_LEVEL)
    {
        level = MAX_LEVEL;
        experience = 0;
    }
}

// EN: loseExperience declares or implements a focused behavior used by this module.
// FR: loseExperience déclare ou implémente un comportement précis utilisé par ce module.
void Player::loseExperience(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    experience -= amount;

    if (experience < 0)
    {
        experience = 0;
    }
}

// EN: levelUp declares or implements a focused behavior used by this module.
// FR: levelUp déclare ou implémente un comportement précis utilisé par ce module.
void Player::levelUp()
{
    if (level >= MAX_LEVEL)
    {
        level = MAX_LEVEL;
        experience = 0;

        MessageScreen::show(
            "NIVEAU MAXIMUM",
            "player.level.max",
            {
                name + " est déjà au niveau maximum : 255.",
                "Tous les bits sont à 1. Plus haut, ce serait de la triche... enfin presque."
            },
            false
        );
        return;
    }

    const int previousHp = std::max(0, hp);
    const int previousMaxHp = std::max(1, maxHp);

    level++;

    maxHp += 20;
    if (previousHp <= 0)
    {
        hp = 0;
    }
    else
    {
        hp = std::max(1, std::min(maxHp, (previousHp * maxHp + previousMaxHp - 1) / previousMaxHp));
    }

    minDamage += 1;
    maxDamage += 2;
    criticalDamage += 3;

    MessageScreen::show(
        "NIVEAU SUPÉRIEUR",
        "player.level.up",
        {
            name + " monte au niveau " + std::to_string(level) + " !",
            "Ses PV maximum augmentent, mais ses blessures ne disparaissent pas gratuitement.",
            "PV conservés au même pourcentage : " + std::to_string(hp) + "/" + std::to_string(maxHp) + ".",
            "Les attributs avancés dorment encore dans les registres du personnage."
        },
        false
    );

    refreshLevelAndIdentitySkills();
}

// EN: attack declares or implements a focused behavior used by this module.
// FR: attack déclare ou implémente un comportement précis utilisé par ce module.
void Player::displayStats() const
{
    std::vector<std::string> lines;
    lines.push_back("Nom : " + name);
    lines.push_back("Race : " + getRaceText());
    lines.push_back("Âge : " + std::to_string(characterAge) + " ans (" + getAgeBandText() + ")");
    lines.push_back("Présentation visuelle : " + visualPresentation + " | " + visualVariant);
    lines.push_back("Description visuelle : " + getAppearanceDescription());
    lines.push_back("Classe : " + type);
    lines.push_back("Titres équipés : " + getActiveTitleSummary());
    lines.push_back("Titres possédés : " + std::to_string(titles.size()));
    if (!titles.empty())
    {
        std::string titleLine = "Liste des titres : ";
        for (std::size_t i = 0; i < titles.size(); ++i)
        {
            if (i > 0) titleLine += ", ";
            const bool equipped = std::find(activeTitles.begin(), activeTitles.end(), titles[i]) != activeTitles.end();
            titleLine += titles[i] + (equipped ? std::string(" [équipé]") : std::string());
        }
        lines.push_back(titleLine);
    }
    lines.push_back("Niveau : " + std::to_string(level));

    int nextLevelExperience = Level::getExperienceRequiredForNextLevel(level);
    if (nextLevelExperience > 0)
    {
        lines.push_back("Expérience : " + std::to_string(experience) + "/" + std::to_string(nextLevelExperience));
    }
    else
    {
        lines.push_back("Expérience : niveau maximum");
    }

    lines.push_back("PV : " + std::to_string(hp) + "/" + std::to_string(maxHp));
    lines.push_back("Dégâts de base : " + std::to_string(minDamage) + " - " + std::to_string(maxDamage));
    lines.push_back("Critique de base : " + std::to_string(criticalDamage));

    if (alteredByCheats)
    {
        lines.push_back("");
        lines.push_back("Statut : Altéré");
        lines.push_back("Les souvenirs de ce personnage portent déjà une trace anormale.");
    }

    if (hasWorldGazePenalty())
    {
        lines.push_back("");
        lines.push_back("Effet temporaire : Regard du monde");
        lines.push_back("Les règles t'ont vu essayer de les contourner.");
        lines.push_back("Combats restants : " + std::to_string(worldGazeCombatsRemaining));
        lines.push_back("PV maximum retenus temporairement : " + std::to_string(worldGazeMaxHpPenalty));
    }

    if (bossEquipmentSealActive)
    {
        lines.push_back("");
        lines.push_back("Effet temporaire de boss : équipement scellé");
        if (!bossEquipmentSealReason.empty())
        {
            lines.push_back(bossEquipmentSealReason);
        }
    }

    if (zelefCorrosionPresent)
    {
        lines.push_back("");
        lines.push_back("Effet boss : Corrosion présente");
        lines.push_back("Zelef garde encore " + std::to_string(zelefMaxHpStolen) + " PV maximum quelque part dans son sang noir.");
    }

    if (grinkaBossTheftPresent)
    {
        lines.push_back("");
        lines.push_back("Effet boss : Volé par un boss");
        lines.push_back("Grinka possède encore quelque chose qui appartenait à ce personnage.");
    }

    if (!unlockedPassiveSkills.empty() || !unlockedActiveSkills.empty())
    {
        lines.push_back("");
        lines.push_back(
            "Compétences : "
            + std::to_string(unlockedPassiveSkills.size())
            + " passives, "
            + std::to_string(unlockedActiveSkills.size())
            + " actives"
        );
    }

    lines.push_back("");
    lines.push_back("Potions de soin : " + std::to_string(inventory.countConsumables(ConsumableType::Healing)));
    lines.push_back("Potions de rage : " + std::to_string(inventory.countConsumables(ConsumableType::Damage)));
    lines.push_back("Argent séparé : " + inventory.getWalletLine());
    lines.push_back("Argent total : " + inventory.getWalletTotalLine());
    lines.push_back("Créé le : " + createdAtText + " | V" + createdForVersion);
    lines.push_back("Dernière adaptation faite pour la V" + lastAdaptedVersion);

    PlayerUiSupport::showPlayerScreen("STATS JOUEUR", "player.stats", lines, false);
}


// EN: displayInventory declares or implements a focused behavior used by this module.
// FR: displayInventory déclare ou implémente un comportement précis utilisé par ce module.
void Player::displayInventory() const
{
    inventory.display();
    displaySimpleEquipment();
}

// EN: displaySimpleEquipment declares or implements a focused behavior used by this module.
// FR: displaySimpleEquipment déclare ou implémente un comportement précis utilisé par ce module.
void Player::displaySimpleEquipment() const
{
    std::vector<std::string> lines;

    if (hasEquippedWeapon())
    {
        Weapon weapon = getEquippedWeapon();
        std::string weaponLine = "Arme équipée : " + weapon.getName();

        if (!weapon.isIndestructible())
        {
            weaponLine += " (" + std::to_string(weapon.getDurability()) + "/" + std::to_string(weapon.getMaxDurability()) + ")";
        }

        if (weapon.isBroken())
        {
            weaponLine += " - Cassée";
        }

        lines.push_back(weaponLine);

        if (!weapon.isBroken() && !hasBossEquipmentSeal())
        {
            std::string affinityLabel = CombatClassSystem::getWeaponAffinityLabel(
                *this,
                weapon.getType(),
                weapon.getName()
            );

            if (!affinityLabel.empty())
            {
                lines.push_back("Affinité arme/classe : active, " + affinityLabel + ".");
            }
            else
            {
                lines.push_back("Affinité arme/classe : aucune maîtrise particulière avec cette arme.");
            }
        }
        else if (weapon.isBroken())
        {
            lines.push_back("Affinité arme/classe : inactive, l'arme est cassée.");
        }
        else if (hasBossEquipmentSeal())
        {
            lines.push_back("Affinité arme/classe : bloquée par le sceau de boss.");
        }
    }
    else
    {
        lines.push_back("Arme équipée : Aucune");
    }

    if (hasEquippedArmor())
    {
        Armor armor = getEquippedArmor();
        std::string armorLine = "Armure équipée : " + armor.getName();

        if (!armor.isIndestructible())
        {
            armorLine += " (" + std::to_string(armor.getDurability()) + "/" + std::to_string(armor.getMaxDurability()) + ")";
        }

        if (armor.isBroken())
        {
            armorLine += " - Cassée";
        }

        lines.push_back(armorLine);
    }
    else
    {
        lines.push_back("Armure équipée : Aucune");
    }

    lines.push_back("Argent séparé : " + inventory.getWalletLine());
    lines.push_back("Argent total : " + inventory.getWalletTotalLine());
    PlayerUiSupport::showPlayerScreen("ÉQUIPEMENT", "player.equipment.simple", lines, false);
}


// EN: displayDetailedEquipment declares or implements a focused behavior used by this module.
// FR: displayDetailedEquipment déclare ou implémente un comportement précis utilisé par ce module.
void Player::displayDetailedEquipment() const
{
    std::vector<std::string> lines;

    lines.push_back("Arme équipée :");
    if (hasEquippedWeapon())
    {
        Weapon weapon = getEquippedWeapon();
        lines.push_back("Nom : " + weapon.getName());
        lines.push_back("Description : " + weapon.getDescription());
        lines.push_back(
            "Bonus dégâts : +"
            + std::to_string(weapon.getMinDamageBonus())
            + " à +"
            + std::to_string(weapon.getMaxDamageBonus())
        );
        lines.push_back("Bonus critique : +" + std::to_string(weapon.getCriticalBonus()));

        if (weapon.isIndestructible())
        {
            lines.push_back("Durabilité : Indestructible");
        }
        else
        {
            lines.push_back("Durabilité : " + std::to_string(weapon.getDurability()) + "/" + std::to_string(weapon.getMaxDurability()));
        }

        if (weapon.isBroken())
        {
            lines.push_back("État : Cassée, ses bonus ne s'appliquent plus.");
            lines.push_back("Affinité arme/classe : inactive, l'arme est cassée.");
        }
        else if (hasBossEquipmentSeal())
        {
            lines.push_back("État : utilisable, mais le sceau de boss bloque ses bonus.");
            lines.push_back("Affinité arme/classe : bloquée par le sceau de boss.");
        }
        else
        {
            lines.push_back("État : Utilisable");

            std::string affinityLabel = CombatClassSystem::getWeaponAffinityLabel(
                *this,
                weapon.getType(),
                weapon.getName()
            );

            if (!affinityLabel.empty())
            {
                lines.push_back("Affinité arme/classe : active, " + affinityLabel + ".");
                lines.push_back("Effet : très léger bonus de dégâts en combat.");
            }
            else
            {
                lines.push_back("Affinité arme/classe : aucune maîtrise particulière avec cette arme.");
            }
        }
    }
    else
    {
        lines.push_back("Aucune arme équipée.");
    }

    lines.push_back("");
    lines.push_back("Armure équipée :");
    if (hasEquippedArmor())
    {
        Armor armor = getEquippedArmor();
        lines.push_back("Nom : " + armor.getName());
        lines.push_back("Description : " + armor.getDescription());
        lines.push_back("Bonus PV max : +" + std::to_string(armor.getMaxHpBonus()));
        lines.push_back("Réduction dégâts : " + std::to_string(armor.getDamageReduction()));

        if (armor.isIndestructible())
        {
            lines.push_back("Durabilité : Indestructible");
        }
        else
        {
            lines.push_back("Durabilité : " + std::to_string(armor.getDurability()) + "/" + std::to_string(armor.getMaxDurability()));
        }

        if (armor.isBroken())
        {
            lines.push_back("État : Cassée, ses bonus ne s'appliquent plus.");
        }
        else
        {
            lines.push_back("État : Utilisable");
        }
    }
    else
    {
        lines.push_back("Aucune armure équipée.");
    }

    lines.push_back("");
    lines.push_back("Argent séparé : " + inventory.getWalletLine());
    lines.push_back("Argent total : " + inventory.getWalletTotalLine());
    PlayerUiSupport::showPlayerScreen("ÉQUIPEMENT DÉTAILLÉ", "player.equipment.detailed", lines, false);
}

