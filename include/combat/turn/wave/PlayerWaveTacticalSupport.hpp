#ifndef INCLUDE_COMBAT_TURN_WAVE_PLAYERWAVETACTICALSUPPORT_HPP
#define INCLUDE_COMBAT_TURN_WAVE_PLAYERWAVETACTICALSUPPORT_HPP

#include <string>
#include <vector>

#include "entity/Player.hpp"
#include "entity/Monster.hpp"
#include "combat/EnemyCombatQueue.hpp"
#include "item/weapon/Weapon.hpp"

namespace PlayerWaveTacticalSupport
{
    struct TacticalLantern
    {
        std::string id;
        std::string name;
        bool mycelium = false;
    };

    struct PreparedWeaponCoating
    {
        std::string id;
        std::string name;
        std::string label;
    };

    std::string normalizeTacticalText(std::string value);
    bool playerClassContainsAny(const Player& player, const std::vector<std::string>& needles);
    bool playerHasShadowStepAffinity(const Player& player);
    bool playerHasRogueTechniqueAffinity(const Player& player);
    bool playerHasGuardianTechniqueAffinity(const Player& player);
    bool playerHasArcaneTechniqueAffinity(const Player& player);
    bool playerHasSupportTechniqueAffinity(const Player& player);
    bool playerHasSkirmisherTechniqueAffinity(const Player& player);
    bool playerHasWarriorBurstAffinity(const Player& player);
    bool playerHasCommanderTechniqueAffinity(const Player& player);
    bool playerHasNatureTechniqueAffinity(const Player& player);
    bool playerHasArtificeTechniqueAffinity(const Player& player);
    bool playerHasSacredTechniqueAffinity(const Player& player);
    bool playerHasMonkTechniqueAffinity(const Player& player);
    bool playerHasIllusionTechniqueAffinity(const Player& player);
    bool playerHasSummonerTechniqueAffinity(const Player& player);
    bool playerHasBloodTechniqueAffinity(const Player& player);
    bool playerHasMedicOrCraftTechniqueAffinity(const Player& player);
    bool playerHasElementalistTechniqueAffinity(const Player& player);
    bool playerHasBardTechniqueAffinity(const Player& player);
    bool playerHasBeastTechniqueAffinity(const Player& player);
    bool weaponIsLightBladeForDance(const Weapon& weapon);
    int countLightWeaponsForBladeDance(const Player& player);
    bool playerHasDuelTechniqueAffinity(const Player& player);
    bool playerHasWardenTechniqueAffinity(const Player& player);
    bool tacticalMonsterProfileContainsAny(const Monster& monster, const std::vector<std::string>& needles);
    bool tacticalMonsterIsDedicatedCaller(const Monster& monster);
    bool tacticalMonsterIsCommonBandRace(const Monster& monster);
    bool tacticalMonsterMayTryToSignal(const Monster& monster);
    bool waveHasSignalToCut(const EnemyCombatQueue& wave);
    std::string describeSignalTargetHint(const Monster& monster);
    int chooseSignalTarget(EnemyCombatQueue& wave, const std::string& screenId);
    bool consumeTacticalLantern(Player& player, TacticalLantern& lantern);
    int canonicalEventCount(const Player& player, const std::string& category, const std::string& key);
    int totalTacticalActionCount(const Player& player);
    int tacticalMasteryLevel(const Player& player, const std::string& actionKey);
    std::string passiveMasteryIdForAction(const std::string& actionKey);
    bool tacticalActionUsesWeaponTempo(const std::string& actionKey);
    bool tacticalActionUsesArmorTempo(const std::string& actionKey);
    int tacticalLoadoutSynergyModifier(const Player& player, const std::string& actionKey);
    std::string tacticalLoadoutSynergyLine(const Player& player, const std::string& actionKey);
    int tacticalPassiveMasteryLevel(const Player& player, const std::string& actionKey);
    int tacticalMasterySoftBonus(const Player& player, const std::string& actionKey);
    int tacticalMasteryChanceBonus(const Player& player, const std::string& actionKey);
    int tacticalMasteryDurationBonus(const Player& player, const std::string& actionKey);
    std::string tacticalMasteryShortResultLine(const Player& player, const std::string& actionKey);
    std::string tacticalMasteryLine(const Player& player, const std::string& actionKey);
    std::string tacticalMasteryMenuHint(const Player& player, const std::string& actionKey);
    void updateTacticalLearning(Player& player, const std::string& actionKey);
    bool hasWeaponCoatingMaterial(const Player& player);
    PreparedWeaponCoating consumeWeaponCoatingMaterial(Player& player);
    bool hasImprovisedTrapMaterial(const Player& player);
    std::string consumeImprovisedTrapMaterial(Player& player);
    bool hasExploitableOpening(const Monster& target);
    bool waveHasExploitableOpening(const EnemyCombatQueue& wave);
    bool waveHasFormationToBreak(const EnemyCombatQueue& wave);
    int countCombatStatuses(const Monster& monster);
    bool hasStatusChain(const Monster& monster);
    bool waveHasStatusChain(const EnemyCombatQueue& wave);
    bool isLowOrMobileTarget(const Monster& monster);
    bool isFocusOrSupportTarget(const Monster& monster);
    bool waveHasLowOrMobileTarget(const EnemyCombatQueue& wave);
    bool waveHasFocusOrSupportTarget(const EnemyCombatQueue& wave);
    bool isReachOrBacklineTarget(const Monster& monster);
    bool waveHasReachOrBacklineTarget(const EnemyCombatQueue& wave);
    bool isShieldOrArmorTarget(const Monster& monster);
    bool waveHasShieldOrArmorTarget(const EnemyCombatQueue& wave);
    bool isOccultAnchorTarget(const Monster& monster);
    bool waveHasOccultAnchorTarget(const EnemyCombatQueue& wave);
    std::string describeStatusChainHint(const Monster& monster);
    int chooseStatusChainTarget(EnemyCombatQueue& wave, const std::string& screenId);
    std::string describeBodyTargetHint(const Monster& monster);
    int chooseBodyTarget(EnemyCombatQueue& wave, const std::string& screenId);
    int chooseTacticalTarget(EnemyCombatQueue& wave, const std::string& screenId);
}

#endif
