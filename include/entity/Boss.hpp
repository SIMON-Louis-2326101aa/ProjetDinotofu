// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ENTITY_BOSS_HPP
#define INCLUDE_ENTITY_BOSS_HPP

#include "entity/Entity.hpp"

class Boss : public Entity
{
private:
    int bossId;
    int remainingUltimateTurns;
    int maxUltimateTurns;
    int ultimateCooldown;
    int maxUltimateCooldown;
    int specialEffect;
    bool decryptedStats;

public:
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

    int getBossId() const;

    int getRemainingUltimateTurns() const;
    int getMaxUltimateTurns() const;

    int getUltimateCooldown() const;
    int getMaxUltimateCooldown() const;

    int getSpecialEffect() const;
    void setSpecialEffect(int effet);

    bool canUseUltimate() const;
    bool isUltimateActive() const;

    void activateUltimate();
    void reduceUltimate();
    void reduceUltimateCooldown();
    void resetUltimateCooldown();

    bool areStatsVisible() const override;
    bool mustDecryptStats() const;
    void decryptStats();

    void displayStats() const override;
};

#endif
