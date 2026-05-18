// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "entity/Boss.hpp"

#include <iostream>

Boss::Boss() : Entity()
{
    bossId = 0;
    remainingUltimateTurns = 0;
    maxUltimateTurns = 0;
    ultimateCooldown = 0;
    maxUltimateCooldown = 0;
    specialEffect = 0;
    decryptedStats = false;
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

    this->remainingUltimateTurns = 0;
    this->maxUltimateTurns = maxUltimateTurns;

    this->ultimateCooldown = maxUltimateCooldown;
    this->maxUltimateCooldown = maxUltimateCooldown;

    this->specialEffect = 0;
    this->decryptedStats = false;
}

int Boss::getBossId() const
{
    return bossId;
}

int Boss::getRemainingUltimateTurns() const
{
    return remainingUltimateTurns;
}

int Boss::getMaxUltimateTurns() const
{
    return maxUltimateTurns;
}

int Boss::getUltimateCooldown() const
{
    return ultimateCooldown;
}

int Boss::getMaxUltimateCooldown() const
{
    return maxUltimateCooldown;
}

int Boss::getSpecialEffect() const
{
    return specialEffect;
}

void Boss::setSpecialEffect(int effet)
{
    specialEffect = effet;
}

bool Boss::canUseUltimate() const
{
    return remainingUltimateTurns <= 0 && ultimateCooldown <= 0 && hp <= (maxHp / 2);
}

bool Boss::isUltimateActive() const
{
    return remainingUltimateTurns > 0;
}

void Boss::activateUltimate()
{
    remainingUltimateTurns = maxUltimateTurns;
}

void Boss::reduceUltimate()
{
    if (remainingUltimateTurns > 0)
    {
        remainingUltimateTurns--;
    }
}

void Boss::reduceUltimateCooldown()
{
    if (remainingUltimateTurns <= 0 && ultimateCooldown > 0)
    {
        ultimateCooldown--;
    }
}

void Boss::resetUltimateCooldown()
{
    ultimateCooldown = maxUltimateCooldown;
    specialEffect = 0;
}

bool Boss::areStatsVisible() const
{
    return decryptedStats;
}

bool Boss::mustDecryptStats() const
{
    return !decryptedStats && hp <= (maxHp / 2);
}

void Boss::decryptStats()
{
    decryptedStats = true;
}

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
    std::cout << "Nom : " << name << std::endl;
    std::cout << "Type d'entité : " << type << std::endl;
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