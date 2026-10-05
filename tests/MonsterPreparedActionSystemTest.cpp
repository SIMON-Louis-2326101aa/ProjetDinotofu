#include "combat/system/MonsterPreparedActionSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"
#include "entity/Monster.hpp"
#include "entity/Player.hpp"
#include <cassert>

int main()
{
    Player player;
    Monster brute("Brute d'essai", "Brute lourde", Race::Orc, 20, 120, 8, 14, 20, 0, 0, false, true, false, false);
    brute.startPreparedSignature("Charge écrasante", 2);
    const int hpBefore = player.getHp();
    MonsterPreparedActionResolution resolved = MonsterPreparedActionSystem::resolve(brute, player);
    assert(resolved.hadPreparation);
    assert(resolved.resolved);
    assert(!resolved.interrupted);
    assert(resolved.family == "charge_lourde");
    assert(resolved.damageInterruptPercent == 12);
    assert(resolved.forcedReposition);
    assert(resolved.damage > 0);
    assert(player.getHp() < hpBefore);
    assert(player.hasWeakening());
    assert(player.hasNextHitVulnerability());
    assert(!brute.hasPreparedSignature());


    Player guardedPlayer;
    Monster guardedBrute("Brute gardée", "Brute lourde", Race::Orc, 20, 120, 8, 14, 20, 0, 0, false, true, false, false);
    guardedBrute.startPreparedSignature("Charge écrasante", 2);
    guardedPlayer.startDefensePosture(35, 0, "Posture de test");
    const int guardedHpBefore = guardedPlayer.getHp();
    MonsterPreparedActionResolution guarded = MonsterPreparedActionSystem::resolve(guardedBrute, guardedPlayer);
    assert(guarded.resolved);
    assert(guarded.damage > 0);
    assert(guarded.damage < resolved.damage);
    assert(guardedPlayer.getHp() == guardedHpBefore - guarded.damage);
    assert(!guardedPlayer.isInDefensePosture());

    Player secondPlayer;
    Monster shocked("Golem chargé", "Construction lourde", Race::Construction, 25, 140, 9, 15, 22, 0, 0, false, true, false, false);
    shocked.startPreparedSignature("Marteau runique", 3);
    shocked.applyShock(1);
    const int secondHp = secondPlayer.getHp();
    MonsterPreparedActionResolution interrupted = MonsterPreparedActionSystem::resolve(shocked, secondPlayer);
    assert(interrupted.hadPreparation);
    assert(interrupted.interrupted);
    assert(!interrupted.resolved);
    assert(secondPlayer.getHp() == secondHp);
    assert(shocked.hasVulnerability());
    assert(!shocked.hasPreparedSignature());

    Player thirdPlayer;
    Monster pressured("Colosse blessé", "Colosse", Race::Orc, 30, 200, 10, 18, 25, 0, 0, false, true, false, false);
    pressured.startPreparedSignature("Heurt frontal", 2);
    pressured.takeDamage(25); // charge lourde: >12% des PV max.
    MonsterPreparedActionResolution pressureInterrupted = MonsterPreparedActionSystem::resolve(pressured, thirdPlayer);
    assert(pressureInterrupted.interrupted);
    assert(pressureInterrupted.interruptReason.find("dégâts") != std::string::npos);

    Player trappedPlayer;
    Monster rootCaster("Tisseuse d'essai", "Plante entravante", Race::Plante, 24, 180, 8, 16, 22, 0, 0, false, true, false, false);
    rootCaster.startPreparedSignature("Toile géante de racines", 2);
    MonsterPreparedActionResolution roots = MonsterPreparedActionSystem::resolve(rootCaster, trappedPlayer);
    assert(roots.resolved);
    assert(roots.family == "entrave_massive");
    assert(roots.damageInterruptPercent == 7);
    assert(trappedPlayer.hasEntanglement());
    assert(!roots.forcedReposition);

    Player ritualPlayer;
    Monster ritualist("Sonneur d'essai", "Esprit rituel", Race::Esprit, 28, 220, 10, 20, 30, 0, 0, false, true, false, false);
    ritualist.startPreparedSignature("Rituel du grand serment", 3);
    MonsterPreparedActionResolution ritual = MonsterPreparedActionSystem::resolve(ritualist, ritualPlayer);
    assert(ritual.resolved);
    assert(ritual.family == "rituel");
    assert(ritual.damageInterruptPercent == 8);
    assert(!ritual.forcedReposition);
    assert(ritualPlayer.hasWeakening());

    assert(MonsterPreparedActionSystem::telegraphLineForLabel("Tir lourd au canon").find("Télégraphe visible") != std::string::npos);
    assert(MonsterPreparedActionSystem::interruptHintForLabel("Tir lourd au canon").find("9 %") != std::string::npos);

    Player breathPlayer;
    Monster breather("Drake d'essai", "Dragon de falaise", Race::Dragon, 30, 260, 16, 28, 38, 0, 0, false, true, true);
    breather.startPreparedSignature("Souffle de givre concentré", 3);
    MonsterPreparedActionResolution breath = MonsterPreparedActionSystem::resolve(breather, breathPlayer);
    assert(breath.resolved);
    assert(breath.family == "souffle_elementaire");
    assert(breath.damageInterruptPercent == 10);
    assert(!breath.forcedReposition);
    assert(breathPlayer.hasWeakening());

    Player divePlayer;
    Monster diver("Harpie d'essai", "Prédateur aérien", Race::Bete, 26, 210, 14, 25, 36, 0, 0, false, true, true);
    diver.startPreparedSignature("Piqué aérien aux serres", 2);
    MonsterPreparedActionResolution dive = MonsterPreparedActionSystem::resolve(diver, divePlayer);
    assert(dive.resolved);
    assert(dive.family == "plongee_aerienne");
    assert(dive.damageInterruptPercent == 11);
    assert(dive.forcedReposition);
    assert(divePlayer.hasNextHitVulnerability());

    Player venomPlayer;
    Monster stinger("Dardeur d'essai", "Insectoïde venimeux", Race::Insectoide, 22, 180, 12, 22, 31, 0, 0, false, true, true);
    stinger.startPreparedSignature("Dard de venin concentré", 2);
    MonsterPreparedActionResolution venom = MonsterPreparedActionSystem::resolve(stinger, venomPlayer);
    assert(venom.resolved);
    assert(venom.family == "venin_prepare");
    assert(venom.damageInterruptPercent == 6);
    assert(venomPlayer.hasPoison());
    assert(!venom.forcedReposition);
    return 0;
}
