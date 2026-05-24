// EN: CombatPotionUse.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatPotionUse.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/potions/CombatPotionUse.hpp"

#include "combat/action/CombatAttack.hpp"
#include "combat/threat/ThreatSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"
#include "combat/system/DamageSystem.hpp"
#include "combat/DamageReport.hpp"

#include "interface/menu/CombatTargetMenu.hpp"

#include "item/Inventory.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>


namespace
{
    std::string normalizePotionText(std::string value)
    {
        for (char& c : value)
        {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return value;
    }

    bool potionNameContains(const Consumable& potion, const std::string& text)
    {
        return normalizePotionText(potion.getName()).find(normalizePotionText(text)) != std::string::npos;
    }

    bool applyCurativeStatusEffect(Player& player, const Consumable& potion)
    {
        bool cured = false;

        if (potionNameContains(potion, "antidote"))
        {
            cured = player.curePoison();
            if (cured) std::cout << "Le poison est neutralisé." << std::endl;
        }
        else if (potionNameContains(potion, "anti-br") || potionNameContains(potion, "brûl") || potionNameContains(potion, "brul"))
        {
            cured = player.cureBurning();
            if (cured) std::cout << "La brûlure est apaisée avant de continuer à ronger les chairs." << std::endl;
        }
        else if (potionNameContains(potion, "givre") || potionNameContains(potion, "tiède") || potionNameContains(potion, "tiede"))
        {
            cured = player.cureFrost();
            if (cured) std::cout << "Le froid quitte les articulations et les gestes redeviennent plus fluides." << std::endl;
        }
        else if (potionNameContains(potion, "isolante") || potionNameContains(potion, "décharge") || potionNameContains(potion, "decharge"))
        {
            cured = player.cureShock();
            if (cured) std::cout << "La conduction électrique est coupée avant de perturber un nouveau geste." << std::endl;
        }

        return cured;
    }
}

bool CombatPotionUse::useHealingPotion(
    Player& player,
    int consumableIndex,
    const Consumable& potion
)
{
    if (!player.getInventory().hasConsumable(consumableIndex))
    {
        std::cout << "Cette potion n'est plus disponible." << std::endl;
        std::cout << std::endl;
        return false;
    }

    bool curedStatus = applyCurativeStatusEffect(player, potion);
    player.heal(potion.getPower());
    ThreatSystem::markSelfHealingAction(player);

    if (!player.hasInfiniteConsumables())
    {
        player.getInventory().removeConsumable(consumableIndex);
    }

    std::cout << player.getName()
              << " boit "
              << potion.getName()
              << " et récupère "
              << potion.getPower()
              << " PV."
              << std::endl;

    if (!curedStatus && player.hasActiveCombatStatus())
    {
        std::cout << "La potion soigne les PV, mais elle n'est pas adaptée aux statuts encore actifs." << std::endl;
    }

    std::cout << std::endl;

    return true;
}

bool CombatPotionUse::useSelectedPotion(
    Player& player,
    int consumableIndex,
    ConsumableType type,
    Entity* target,
    EnemyCombatQueue* wave,
    Random& random,
    int potionDamageBonus
)
{
    if (!player.getInventory().hasConsumable(consumableIndex))
    {
        std::cout << "Cette potion n'est plus disponible." << std::endl;
        std::cout << std::endl;
        return false;
    }

    Consumable potion = player.getInventory().getConsumable(consumableIndex);

    if (type == ConsumableType::Healing)
    {
        return useHealingPotion(player, consumableIndex, potion);
    }

    if (type == ConsumableType::Damage)
    {
        int totalBonus = potion.getPower();

        if (potionDamageBonus > 0)
        {
            totalBonus = potion.getPower();
        }

        if (wave != nullptr)
        {
            bool attackLaunched = CombatTargetMenu::openForBoostedAttack(
                player,
                *wave,
                random,
                totalBonus
            );

            if (attackLaunched && !player.hasInfiniteConsumables())
            {
                player.getInventory().removeConsumable(consumableIndex);
            }

            return attackLaunched;
        }

        if (target != nullptr)
        {
            if (!player.hasInfiniteConsumables())
            {
                player.getInventory().removeConsumable(consumableIndex);
            }

            std::cout << player.getName()
                      << " utilise "
                      << potion.getName()
                      << "."
                      << std::endl;
            std::cout << std::endl;

            CombatAttack::executeBoostedAttack(
                player,
                *target,
                random,
                totalBonus
            );

            return true;
        }

        std::cout << "Aucune cible offensive disponible." << std::endl;
        std::cout << std::endl;
        return false;
    }

    if (type == ConsumableType::Buff)
    {
        if (!player.hasInfiniteConsumables())
        {
            player.getInventory().removeConsumable(consumableIndex);
        }

        int stabilisation = std::max(1, potion.getPower() / 3);
        bool curedStatus = applyCurativeStatusEffect(player, potion);
        player.heal(stabilisation);
        DefensePostureSystem::enterDefensePosture(player);

        std::cout << player.getName() << " utilise " << potion.getName() << "." << std::endl;
        std::cout << "Son corps se stabilise : +" << stabilisation << " PV et posture défensive immédiate." << std::endl;
        if (curedStatus)
        {
            std::cout << "La potion avait aussi le bon effet pour purger un statut actif." << std::endl;
        }
        else
        {
            std::cout << "Ce n'est pas encore un buff long, mais ce n'est plus une potion décorative." << std::endl;
        }
        std::cout << std::endl;
        return true;
    }

    if (type == ConsumableType::Debuff)
    {
        Entity* debuffTarget = target;

        if (debuffTarget == nullptr && wave != nullptr && wave->hasActiveEnemies())
        {
            debuffTarget = &wave->getActiveEnemy(0);
        }

        if (debuffTarget == nullptr)
        {
            std::cout << "Aucune cible n'est assez proche pour recevoir cette fiole." << std::endl;
            std::cout << std::endl;
            return false;
        }

        if (!player.hasInfiniteConsumables())
        {
            player.getInventory().removeConsumable(consumableIndex);
        }

        int rawDamage = potion.getPower();
        DamageReport report = DamageSystem::calculateReceivedDamage(*debuffTarget, rawDamage);
        DamageSystem::displayDamageReport(*debuffTarget, report);
        debuffTarget->takeDamage(report.receivedDamage);

        std::cout << player.getName() << " lance " << potion.getName() << " sur " << debuffTarget->getName() << "." << std::endl;
        std::cout << "La cible est affaiblie et reçoit " << report.receivedDamage << " dégâts chimiques." << std::endl;
        std::cout << debuffTarget->getName() << " possède maintenant " << debuffTarget->getHp() << "/" << debuffTarget->getMaxHp() << " PV." << std::endl;
        std::cout << std::endl;

        if (wave != nullptr)
        {
            wave->removeDeadAndReplace();
        }

        return true;
    }

    if (type == ConsumableType::Special)
    {
        if (potionNameContains(potion, "fumée") || potionNameContains(potion, "fumee"))
        {
            if (!player.hasInfiniteConsumables())
            {
                player.getInventory().removeConsumable(consumableIndex);
            }

            DefensePostureSystem::enterDefensePosture(player);
            player.clearProvocation();
            std::cout << player.getName() << " brise une " << potion.getName() << "." << std::endl;
            std::cout << "La fumée ne téléporte pas et ne garantit pas la fuite, mais elle casse la pression immédiate." << std::endl;
            std::cout << "Effet actuel : posture défensive et provocation annulée." << std::endl;
            std::cout << std::endl;
            return true;
        }
    }

    std::cout << "[cette option n'est pas encore accessible pour ce mode]" << std::endl;
    std::cout << std::endl;

    return false;
}
