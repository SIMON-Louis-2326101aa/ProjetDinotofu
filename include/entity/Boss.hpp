// EN: Boss.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: Boss.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ENTITY_BOSS_HPP
#define INCLUDE_ENTITY_BOSS_HPP

#include "entity/Entity.hpp"

#include <vector>

class Boss : public Entity
{
private:
    int bossId;
    bool identityRevealed;
    int remainingUltimateTurns;
    int maxUltimateTurns;
    int ultimateCooldown;
    int maxUltimateCooldown;
    int specialEffect;
    bool decryptedStats;
    bool ultimateUnlocked;


public:
    // EN: Boss declares or implements a focused behavior used by this module.
    // FR: Boss déclare ou implémente un comportement précis utilisé par ce module.
    Boss();

    Boss(
        int bossId,
        const std::string& name,
        const std::string& type,
        int maxHp,
        int minDamage,
        int maxDamage,
        int criticalDamage,
        int healingPotionCount,
        int damagePotionCount,
        int maxUltimateTurns,
        int maxUltimateCooldown
    );

    // EN: getBossId declares or implements a focused behavior used by this module.
    // FR: getBossId déclare ou implémente un comportement précis utilisé par ce module.
    int getBossId() const;
    std::string getName() const override;
    std::string getType() const override;
    bool isIdentityRevealed() const;
    void revealIdentity();

    // EN: getRemainingUltimateTurns declares or implements a focused behavior used by this module.
    // FR: getRemainingUltimateTurns déclare ou implémente un comportement précis utilisé par ce module.
    int getRemainingUltimateTurns() const;
    // EN: getMaxUltimateTurns declares or implements a focused behavior used by this module.
    // FR: getMaxUltimateTurns déclare ou implémente un comportement précis utilisé par ce module.
    int getMaxUltimateTurns() const;

    // EN: getUltimateCooldown declares or implements a focused behavior used by this module.
    // FR: getUltimateCooldown déclare ou implémente un comportement précis utilisé par ce module.
    int getUltimateCooldown() const;
    // EN: getMaxUltimateCooldown declares or implements a focused behavior used by this module.
    // FR: getMaxUltimateCooldown déclare ou implémente un comportement précis utilisé par ce module.
    int getMaxUltimateCooldown() const;

    // EN: getSpecialEffect declares or implements a focused behavior used by this module.
    // FR: getSpecialEffect déclare ou implémente un comportement précis utilisé par ce module.
    int getSpecialEffect() const;
    // EN: setSpecialEffect declares or implements a focused behavior used by this module.
    // FR: setSpecialEffect déclare ou implémente un comportement précis utilisé par ce module.
    void setSpecialEffect(int effet);

    // EN: canUseUltimate declares or implements a focused behavior used by this module.
    // FR: canUseUltimate déclare ou implémente un comportement précis utilisé par ce module.
    bool canUseUltimate() const;
    // EN: isUltimateActive declares or implements a focused behavior used by this module.
    // FR: isUltimateActive déclare ou implémente un comportement précis utilisé par ce module.
    bool isUltimateActive() const;
    // EN: isUltimateUnlocked declares or implements a focused behavior used by this module.
    // FR: isUltimateUnlocked déclare ou implémente un comportement précis utilisé par ce module.
    bool isUltimateUnlocked() const;
    // EN: shouldUnlockUltimate declares or implements a focused behavior used by this module.
    // FR: shouldUnlockUltimate déclare ou implémente un comportement précis utilisé par ce module.
    bool shouldUnlockUltimate() const;

    // EN: unlockUltimate declares or implements a focused behavior used by this module.
    // FR: unlockUltimate déclare ou implémente un comportement précis utilisé par ce module.
    void unlockUltimate();
    // EN: activateUltimate declares or implements a focused behavior used by this module.
    // FR: activateUltimate déclare ou implémente un comportement précis utilisé par ce module.
    void activateUltimate();
    // EN: reduceUltimate declares or implements a focused behavior used by this module.
    // FR: reduceUltimate déclare ou implémente un comportement précis utilisé par ce module.
    void reduceUltimate();
    // EN: reduceUltimateCooldown declares or implements a focused behavior used by this module.
    // FR: reduceUltimateCooldown déclare ou implémente un comportement précis utilisé par ce module.
    void reduceUltimateCooldown();
    // EN: resetUltimateCooldown declares or implements a focused behavior used by this module.
    // FR: resetUltimateCooldown déclare ou implémente un comportement précis utilisé par ce module.
    void resetUltimateCooldown();

    // EN: areStatsVisible declares or implements a focused behavior used by this module.
    // FR: areStatsVisible déclare ou implémente un comportement précis utilisé par ce module.
    bool areStatsVisible() const override;
    // EN: mustDecryptStats declares or implements a focused behavior used by this module.
    // FR: mustDecryptStats déclare ou implémente un comportement précis utilisé par ce module.
    bool mustDecryptStats() const;
    // EN: decryptStats declares or implements a focused behavior used by this module.
    // FR: decryptStats déclare ou implémente un comportement précis utilisé par ce module.
    void decryptStats();

    std::vector<std::string> toDisplayLines() const;

    // EN: displayStats declares or implements a focused behavior used by this module.
    // FR: displayStats déclare ou implémente un comportement précis utilisé par ce module.
    void displayStats() const override;
};

#endif
