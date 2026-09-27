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

namespace
{
    int stablePlayerNormalDamageRoll(Random& random, int minimumDamage, int maximumDamage)
    {
        if (maximumDamage < minimumDamage)
        {
            maximumDamage = minimumDamage;
        }

        const int spread = maximumDamage - minimumDamage;
        if (spread <= 8)
        {
            return random.between(minimumDamage, maximumDamage);
        }

        const int firstRoll = random.between(minimumDamage, maximumDamage);
        const int secondRoll = random.between(minimumDamage, maximumDamage);
        int stabilized = (firstRoll + secondRoll + 1) / 2;

        if (spread >= 18 && random.between(1, 100) <= 10)
        {
            stabilized = random.between(minimumDamage, maximumDamage);
        }

        return std::clamp(stabilized, minimumDamage, maximumDamage);
    }
}

int Player::attack(Random& random, bool& dodged, bool& critical, int damageBonus)
{
    int resultat = random.rollD20();

    if (precisionBoostTurns > 0 && precisionRollBonus > 0)
    {
        resultat = std::min(20, resultat + precisionRollBonus);
    }

    dodged = false;
    critical = false;

    if (shockTurns > 0 && random.between(1, 100) <= 18)
    {
        dodged = true;
        clearLastConsumedAmmunition();
        clearNextAmmunitionChoice();
        MessageScreen::show("CHOC ÉLECTRIQUE", "player.attack.shock_failed", {name + " est perturbé par le choc électrique et rate son geste."}, false);
        return 0;
    }

    int dodgeThreshold = 3;
    int normalHitThreshold = 16;
    int frostDamagePercent = 100;

    if (frostTurns > 0)
    {
        dodgeThreshold += 1;
        normalHitThreshold += 1;
        frostDamagePercent = 85;
        MessageScreen::show("FROID", "player.attack.frost_slow", {name + " attaque avec des gestes ralentis par le froid."}, false);
    }

    int bonusMin = 0;
    int bonusMax = 0;
    int criticalBonus = 0;
    int weaponWeightDamagePercent = 100;

    if (hasPassiveSkill("battle_instinct"))
    {
        bonusMin += 1;
        bonusMax += 1;
    }

    if (hasPassiveSkill("survival_breath") && hp * 3 <= maxHp)
    {
        bonusMin += 1;
        bonusMax += 2;
    }

    if (hasPassiveSkill("veteran_rhythm"))
    {
        criticalBonus += 2;
    }

    if (hasPassiveSkill("scar_tissue") && hp * 2 <= maxHp)
    {
        criticalBonus += 2;
    }

    if (hasPassiveSkill("boss_memory") && bossesKilled > 0)
    {
        bonusMax += 1;
        criticalBonus += 1;
    }

    Weapon* equippedWeapon = inventory.getMutableWeapon(equippedWeaponIndex);
    clearLastConsumedAmmunition();

    bool bowWithoutAmmo = false;

    if (equippedWeapon != nullptr && !equippedWeapon->isBroken() && !bossEquipmentSealActive)
    {
        bonusMin = equippedWeapon->getMinDamageBonus();
        bonusMax = equippedWeapon->getMaxDamageBonus();
        criticalBonus = equippedWeapon->getCriticalBonus();
        dodgeThreshold = std::max(1, dodgeThreshold + EquipmentWeightRules::getWeaponDodgeThresholdAdjustment(*equippedWeapon));
        normalHitThreshold = std::max(dodgeThreshold + 8, std::min(18, normalHitThreshold + EquipmentWeightRules::getWeaponNormalHitThresholdAdjustment(*equippedWeapon)));
        weaponWeightDamagePercent = EquipmentWeightRules::getWeaponDamagePercent(*equippedWeapon);

        if (equippedWeapon->getType() == WeaponType::Bow)
        {
            if (hasPassiveSkill("ranger_eye"))
            {
                bonusMax += 2;
                criticalBonus += 3;
            }

            std::string ammoId = "training_arrows";
            std::string weaponText = equippedWeapon->getName();
            std::transform(weaponText.begin(), weaponText.end(), weaponText.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            std::string specialAmmoId = "barbed_arrows";
            std::string elementalAmmoId = "ash_arrows";
            if (weaponText.find("arbal") != std::string::npos || weaponText.find("carreau") != std::string::npos)
            {
                ammoId = "training_bolts";
                specialAmmoId = "piercing_bolts";
                elementalAmmoId = "frozen_bolts";
            }
            else if (weaponText.find("lancer") != std::string::npos || weaponText.find("couteau") != std::string::npos || weaponText.find("bandouli") != std::string::npos)
            {
                ammoId = "training_throwing_knives";
                specialAmmoId = "balanced_throwing_knives";
                elementalAmmoId = "conductive_knives";
            }

            std::string consumedAmmoId = ammoId;
            bool specialAmmo = false;

            bool forcedNoAmmo = false;

            if (!nextAmmunitionChoiceId.empty())
            {
                if (nextAmmunitionChoiceId == "__cancel_attack__")
                {
                    clearNextAmmunitionChoice();
                    dodged = true;
                    MessageScreen::show("TIR ANNULÉ", "player.attack.ammo_cancel", {name + " annule son tir et garde sa munition."}, false);
                    return 0;
                }

                if (nextAmmunitionChoiceId == "__no_ammo__" || nextAmmunitionChoiceId == "__emergency_defense__")
                {
                    forcedNoAmmo = true;
                }
                else
                {
                    consumedAmmoId = nextAmmunitionChoiceId;
                    specialAmmo = consumedAmmoId != ammoId;
                }
            }

            clearNextAmmunitionChoice();

            if (!forcedNoAmmo && inventory.countMaterialById(consumedAmmoId) > 0)
            {
                if (!infiniteConsumablesEnabled)
                {
                    inventory.removeMaterialQuantityById(consumedAmmoId, 1);
                }

                setLastConsumedAmmunition(consumedAmmoId);

                if (specialAmmo)
                {
                    if (consumedAmmoId == elementalAmmoId)
                    {
                        bonusMin += 3;
                        bonusMax += 5;
                        criticalBonus += 4;
                        MessageScreen::show("MUNITION ÉLÉMENTAIRE", "player.attack.elemental_ammo", {name + " consomme une munition élémentaire choisie pour attaquer à distance."}, false);
                    }
                    else
                    {
                        bonusMin += 2;
                        bonusMax += 4;
                        criticalBonus += 5;
                        MessageScreen::show("MUNITION SPÉCIALE", "player.attack.special_ammo", {name + " consomme une munition spéciale choisie pour attaquer à distance."}, false);
                    }
                }
                else
                {
                    MessageScreen::show("MUNITION", "player.attack.training_ammo", {name + " consomme une munition d'entraînement pour attaquer à distance."}, false);
                }
            }
            else
            {
                bowWithoutAmmo = true;
                bonusMin = 0;
                bonusMax = 1;
                criticalBonus = 0;

                setLastConsumedAmmunition("__emergency_defense__");
                MessageScreen::show(
                    "DÉFENSE D'URGENCE",
                    "player.attack.no_ammo",
                    {
                        name + " n'a pas de munition adaptée pour ce tir.",
                        "Défense d'urgence : aucun projectile n'est tiré.",
                        "Les dégâts représentent seulement un coup de poignée, de branche ou de crosse à très courte portée."
                    },
                    false
                );
            }
        }
    }

    if (equippedWeapon != nullptr && !equippedWeapon->isBroken() && bossEquipmentSealActive)
    {
        std::vector<std::string> sealLines;
        sealLines.push_back("Le sceau de boss bloque les bonus de " + equippedWeapon->getName() + ".");
        if (!bossEquipmentSealReason.empty())
        {
            sealLines.push_back(bossEquipmentSealReason);
        }
        MessageScreen::show("SCEAU DE BOSS", "player.attack.boss_equipment_seal", sealLines, false);
    }

    if (resultat <= dodgeThreshold)
    {
        dodged = true;
        return 0;
    }

    if (equippedWeapon != nullptr && !indestructibleEquipmentEnabled)
    {
        equippedWeapon->loseDurability(1);
    }

    if (equippedWeapon != nullptr && equippedWeapon->isBroken())
    {
        MessageScreen::show(
            "ARME CASSÉE",
            "player.attack.weapon_broken",
            {
                "L'arme de " + name + " s'abîme sous le choc...",
                equippedWeapon->getName() + " est maintenant cassée et ne donnera plus ses bonus."
            },
            false
        );
    }

    if (resultat <= normalHitThreshold)
    {
        int dealtDamage = stablePlayerNormalDamageRoll(
            random,
            minDamage + bonusMin,
            maxDamage + bonusMax
        ) + damageBonus;

        if (bowWithoutAmmo)
        {
            dealtDamage = std::max(1, dealtDamage / 2);
        }

        if (frostDamagePercent < 100)
        {
            dealtDamage = std::max(1, dealtDamage * frostDamagePercent / 100);
        }

        if (weaponWeightDamagePercent != 100)
        {
            dealtDamage = std::max(1, dealtDamage * weaponWeightDamagePercent / 100);
        }

        return applyPowerBoostToDamage(dealtDamage);
    }

    critical = true;
    int criticalResult = criticalDamage + criticalBonus + damageBonus;

    if (bowWithoutAmmo)
    {
        criticalResult = std::max(1, criticalResult / 2);
    }

    if (frostDamagePercent < 100)
    {
        criticalResult = std::max(1, criticalResult * frostDamagePercent / 100);
    }

    if (weaponWeightDamagePercent != 100)
    {
        criticalResult = std::max(1, criticalResult * weaponWeightDamagePercent / 100);
    }

    return applyPowerBoostToDamage(criticalResult);
}

// EN: displayStats declares or implements a focused behavior used by this module.
// FR: displayStats déclare ou implémente un comportement précis utilisé par ce module.
