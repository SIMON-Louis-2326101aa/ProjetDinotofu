// EN: Boss.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: Boss.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/Boss.hpp"

#include <iostream>

// EN: Boss declares or implements a focused behavior used by this module.
// FR: Boss déclare ou implémente un comportement précis utilisé par ce module.
Boss::Boss() : Entity()
{
    bossId = 0;
    identityRevealed = true;
    remainingUltimateTurns = 0;
    maxUltimateTurns = 0;
    ultimateCooldown = 0;
    maxUltimateCooldown = 0;
    specialEffect = 0;
    decryptedStats = false;
    ultimateUnlocked = false;
}

Boss::Boss(
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
) : Entity(
        name,
        type,
        maxHp,
        minDamage,
        maxDamage,
        criticalDamage,
        healingPotionCount,
        damagePotionCount
    )
{
    this->bossId = bossId;
    this->identityRevealed = bossId <= 3;

    this->remainingUltimateTurns = 0;
    this->maxUltimateTurns = maxUltimateTurns;

    this->ultimateCooldown = maxUltimateCooldown;
    this->maxUltimateCooldown = maxUltimateCooldown;

    this->specialEffect = 0;
    this->decryptedStats = false;
    this->ultimateUnlocked = false;
}

// EN: getBossId declares or implements a focused behavior used by this module.
// FR: getBossId déclare ou implémente un comportement précis utilisé par ce module.
int Boss::getBossId() const
{
    return bossId;
}

std::string Boss::getName() const
{
    if (!identityRevealed)
    {
        return "???";
    }

    return name;
}

std::string Boss::getType() const
{
    if (!identityRevealed)
    {
        return "Type brouillé";
    }

    return type;
}

bool Boss::isIdentityRevealed() const
{
    return identityRevealed;
}

void Boss::revealIdentity()
{
    identityRevealed = true;
}

// EN: getRemainingUltimateTurns declares or implements a focused behavior used by this module.
// FR: getRemainingUltimateTurns déclare ou implémente un comportement précis utilisé par ce module.
int Boss::getRemainingUltimateTurns() const
{
    return remainingUltimateTurns;
}

// EN: getMaxUltimateTurns declares or implements a focused behavior used by this module.
// FR: getMaxUltimateTurns déclare ou implémente un comportement précis utilisé par ce module.
int Boss::getMaxUltimateTurns() const
{
    return maxUltimateTurns;
}

// EN: getUltimateCooldown declares or implements a focused behavior used by this module.
// FR: getUltimateCooldown déclare ou implémente un comportement précis utilisé par ce module.
int Boss::getUltimateCooldown() const
{
    return ultimateCooldown;
}

// EN: getMaxUltimateCooldown declares or implements a focused behavior used by this module.
// FR: getMaxUltimateCooldown déclare ou implémente un comportement précis utilisé par ce module.
int Boss::getMaxUltimateCooldown() const
{
    return maxUltimateCooldown;
}

// EN: getSpecialEffect declares or implements a focused behavior used by this module.
// FR: getSpecialEffect déclare ou implémente un comportement précis utilisé par ce module.
int Boss::getSpecialEffect() const
{
    return specialEffect;
}

// EN: setSpecialEffect declares or implements a focused behavior used by this module.
// FR: setSpecialEffect déclare ou implémente un comportement précis utilisé par ce module.
void Boss::setSpecialEffect(int effet)
{
    specialEffect = effet;
}

// EN: canUseUltimate declares or implements a focused behavior used by this module.
// FR: canUseUltimate déclare ou implémente un comportement précis utilisé par ce module.
bool Boss::canUseUltimate() const
{
    return ultimateUnlocked && remainingUltimateTurns <= 0 && ultimateCooldown <= 0;
}

// EN: isUltimateUnlocked declares or implements a focused behavior used by this module.
// FR: isUltimateUnlocked déclare ou implémente un comportement précis utilisé par ce module.
bool Boss::isUltimateUnlocked() const
{
    return ultimateUnlocked;
}

// EN: shouldUnlockUltimate declares or implements a focused behavior used by this module.
// FR: shouldUnlockUltimate déclare ou implémente un comportement précis utilisé par ce module.
bool Boss::shouldUnlockUltimate() const
{
    return !ultimateUnlocked && hp <= (maxHp / 2);
}

// EN: unlockUltimate declares or implements a focused behavior used by this module.
// FR: unlockUltimate déclare ou implémente un comportement précis utilisé par ce module.
void Boss::unlockUltimate()
{
    ultimateUnlocked = true;
    if (ultimateCooldown > 2)
    {
        ultimateCooldown = 2;
    }
}

// EN: isUltimateActive declares or implements a focused behavior used by this module.
// FR: isUltimateActive déclare ou implémente un comportement précis utilisé par ce module.
bool Boss::isUltimateActive() const
{
    return remainingUltimateTurns > 0;
}

// EN: activateUltimate declares or implements a focused behavior used by this module.
// FR: activateUltimate déclare ou implémente un comportement précis utilisé par ce module.
void Boss::activateUltimate()
{
    remainingUltimateTurns = maxUltimateTurns;
}

// EN: reduceUltimate declares or implements a focused behavior used by this module.
// FR: reduceUltimate déclare ou implémente un comportement précis utilisé par ce module.
void Boss::reduceUltimate()
{
    if (remainingUltimateTurns > 0)
    {
        remainingUltimateTurns--;
    }
}

// EN: reduceUltimateCooldown declares or implements a focused behavior used by this module.
// FR: reduceUltimateCooldown déclare ou implémente un comportement précis utilisé par ce module.
void Boss::reduceUltimateCooldown()
{
    if (remainingUltimateTurns <= 0 && ultimateCooldown > 0)
    {
        ultimateCooldown--;
    }
}

// EN: resetUltimateCooldown declares or implements a focused behavior used by this module.
// FR: resetUltimateCooldown déclare ou implémente un comportement précis utilisé par ce module.
void Boss::resetUltimateCooldown()
{
    ultimateCooldown = maxUltimateCooldown;
}

// EN: areStatsVisible declares or implements a focused behavior used by this module.
// FR: areStatsVisible déclare ou implémente un comportement précis utilisé par ce module.
bool Boss::areStatsVisible() const
{
    return decryptedStats;
}

// EN: mustDecryptStats declares or implements a focused behavior used by this module.
// FR: mustDecryptStats déclare ou implémente un comportement précis utilisé par ce module.
bool Boss::mustDecryptStats() const
{
    return !decryptedStats && hp <= (maxHp / 2);
}

// EN: decryptStats declares or implements a focused behavior used by this module.
// FR: decryptStats déclare ou implémente un comportement précis utilisé par ce module.
void Boss::decryptStats()
{
    decryptedStats = true;
    revealIdentity();
}

// EN: displayStats declares or implements a focused behavior used by this module.
// FR: displayStats déclare ou implémente un comportement précis utilisé par ce module.
void Boss::displayStats() const
{
    if (!decryptedStats)
    {
        std::cout << "Tentative de décryptage des statistiques de l'entité échouée." << std::endl;
        std::cout << "Sa puissance brouille encore toute lecture fiable." << std::endl;
        std::cout << std::endl;
        return;
    }

    std::cout << "===== STATISTIQUES DE L'ENTITÉ =====" << std::endl;
    std::cout << "Nom : " << getName() << std::endl;
    std::cout << "Type d'entité : " << getType() << std::endl;
    std::cout << "PV : " << hp << "/" << maxHp << std::endl;
    std::cout << "Dégâts : " << minDamage << " - " << maxDamage << std::endl;
    std::cout << "Critique : " << criticalDamage << std::endl;
    std::cout << "Potions de soin : " << healingPotionCount << std::endl;
    std::cout << "Potions de dégâts : " << damagePotionCount << std::endl;
    std::cout << "Ultime restant : " << remainingUltimateTurns << std::endl;
    std::cout << "Délai ultime : " << ultimateCooldown << std::endl;

    if (specialEffect > 0)
    {
        std::cout << "Effet spécial actif : " << specialEffect << std::endl;
    }

    std::cout << "====================================" << std::endl;
    std::cout << std::endl;
}
