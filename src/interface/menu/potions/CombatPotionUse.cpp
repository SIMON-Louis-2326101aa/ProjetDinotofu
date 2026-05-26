// EN: CombatPotionUse.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatPotionUse.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/menu/potions/CombatPotionUse.hpp"

#include "combat/action/CombatAttack.hpp"
#include "combat/threat/ThreatSystem.hpp"
#include "combat/system/DefensePostureSystem.hpp"
#include "combat/system/DamageSystem.hpp"
#include "combat/system/ElementalAffinitySystem.hpp"
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

    bool playerLooksLikeNaturalCaster(const Player& player)
    {
        std::string className = normalizePotionText(player.getType());
        return className.find("mage") != std::string::npos
            || className.find("sorc") != std::string::npos
            || className.find("arcan") != std::string::npos
            || className.find("pyrom") != std::string::npos
            || className.find("cryo") != std::string::npos
            || className.find("occult") != std::string::npos
            || className.find("invoc") != std::string::npos
            || className.find("nécro") != std::string::npos
            || className.find("necro") != std::string::npos
            || className.find("pact") != std::string::npos
            || className.find("clerc") != std::string::npos
            || className.find("prêtre") != std::string::npos
            || className.find("pretre") != std::string::npos
            || className.find("paladin") != std::string::npos
            || className.find("druide") != std::string::npos
            || className.find("shaman") != std::string::npos
            || className.find("templier") != std::string::npos;
    }

    bool resolveScrollStability(Player& player, const Consumable& potion, Random& random)
    {
        int successChance = playerLooksLikeNaturalCaster(player) ? 92 : 78;

        if (!playerLooksLikeNaturalCaster(player) && player.getLevel() <= 2)
        {
            successChance -= 8;
        }

        std::cout << player.getName() << " déchire " << potion.getName() << "." << std::endl;
        std::cout << "Stabilité du parchemin : " << successChance << "%" << std::endl;

        if (random.between(1, 100) <= successChance)
        {
            return true;
        }

        std::cout << "Les runes brûlent trop vite : la magie se disperse avant de prendre forme." << std::endl;
        std::cout << std::endl;
        return false;
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
        else if (potionNameContains(potion, "défensive") || potionNameContains(potion, "defensive") || potionNameContains(potion, "précision") || potionNameContains(potion, "precision"))
        {
            cured = player.cureWeakening();
            if (!cured) cured = player.cureVulnerability();
            if (cured) std::cout << "Le corps retrouve assez de stabilité pour refermer la faille active." << std::endl;
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
        bool elementalWard = potionNameContains(potion, "voile")
            || potionNameContains(potion, "élémentaire")
            || potionNameContains(potion, "elementaire")
            || potionNameContains(potion, "isolante")
            || potionNameContains(potion, "givre");

        player.heal(stabilisation);
        DefensePostureSystem::enterDefensePosture(player);
        if (elementalWard)
        {
            int wardPower = potionNameContains(potion, "voile") ? potion.getPower() : std::max(8, potion.getPower() / 2);
            player.applyElementalWard(3, wardPower);
        }

        std::cout << player.getName() << " utilise " << potion.getName() << "." << std::endl;
        std::cout << "Son corps se stabilise : +" << stabilisation << " PV et posture défensive immédiate." << std::endl;
        if (elementalWard)
        {
            std::cout << "Un voile court rend les altérations élémentaires moins mordantes." << std::endl;
        }
        if (curedStatus)
        {
            std::cout << "La potion avait aussi le bon effet pour purger un statut actif." << std::endl;
        }
        else
        {
            std::cout << "L'effet reste court, mais ton corps gagne assez de stabilité pour tenir." << std::endl;
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
        bool fragilityPotion = potionNameContains(potion, "fragilis") || potionNameContains(potion, "faille");
        if (fragilityPotion)
        {
            debuffTarget->applyVulnerability(3, 14 + std::max(0, potion.getPower() / 14));
        }
        else
        {
            debuffTarget->applyWeakening(3, 12 + std::max(0, potion.getPower() / 12));
        }

        std::cout << player.getName() << " lance " << potion.getName() << " sur " << debuffTarget->getName() << "." << std::endl;
        if (fragilityPotion)
        {
            std::cout << "La cible garde une faille ouverte et reçoit " << report.receivedDamage << " dégâts chimiques." << std::endl;
        }
        else
        {
            std::cout << "La cible est affaiblie pendant plusieurs tours et reçoit " << report.receivedDamage << " dégâts chimiques." << std::endl;
        }
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
        bool isScroll = potionNameContains(potion, "parchemin");

        if (isScroll)
        {
            Entity* spellTarget = target;
            if (spellTarget == nullptr && wave != nullptr && wave->hasActiveEnemies())
            {
                spellTarget = &wave->getActiveEnemy(0);
            }

            bool defensiveScroll = potionNameContains(potion, "voile");
            bool purificationScroll = potionNameContains(potion, "purification");
            bool selfTargetScroll = defensiveScroll || purificationScroll;
            if (!selfTargetScroll && spellTarget == nullptr)
            {
                std::cout << "Aucune cible n'est assez proche pour recevoir le sort du parchemin." << std::endl;
                std::cout << std::endl;
                return false;
            }

            if (!player.hasInfiniteConsumables())
            {
                player.getInventory().removeConsumable(consumableIndex);
            }

            if (!resolveScrollStability(player, potion, random))
            {
                return true;
            }

            if (purificationScroll)
            {
                int cured = 0;
                if (player.cureBurning()) cured++;
                if (player.curePoison()) cured++;
                if (player.cureFrost()) cured++;
                if (player.cureShock()) cured++;
                if (player.cureBleeding()) cured++;

                if (cured == 0)
                {
                    player.applyElementalWard(2, std::max(8, potion.getPower() / 3));
                    std::cout << "Les runes ne trouvent aucun mal à arracher, alors elles laissent une protection faible." << std::endl;
                }
                else
                {
                    std::cout << "La purification arrache " << cured << " altération(s) au corps." << std::endl;
                }
                std::cout << std::endl;
                return true;
            }

            if (defensiveScroll)
            {
                player.applyElementalWard(3, potion.getPower());
                DefensePostureSystem::enterDefensePosture(player);
                std::cout << "Le voile s'accroche au corps : protection élémentaire courte et posture défensive." << std::endl;
                std::cout << std::endl;
                return true;
            }

            if (potionNameContains(potion, "braise"))
            {
                DamageReport report = DamageSystem::calculateReceivedDamage(*spellTarget, std::max(6, potion.getPower() - 4));
                DamageSystem::displayDamageReport(*spellTarget, report);
                spellTarget->takeDamage(report.receivedDamage);
                ElementalAffinitySystem::applyBurning(*spellTarget, 3, 2 + potion.getPower() / 26);

                if (random.between(1, 100) <= 18)
                {
                    ElementalAffinitySystem::applyBurning(player, 1, 1);
                    std::cout << "La braise revient lécher la main qui l'a libérée." << std::endl;
                }

                std::cout << "La braise errante mord " << spellTarget->getName() << ", puis cherche une autre faim." << std::endl;
                std::cout << spellTarget->getName() << " possède maintenant " << spellTarget->getHp() << "/" << spellTarget->getMaxHp() << " PV." << std::endl;
                std::cout << std::endl;
                if (wave != nullptr) wave->removeDeadAndReplace();
                return true;
            }

            if (potionNameContains(potion, "venin"))
            {
                DamageReport report = DamageSystem::calculateReceivedDamage(*spellTarget, std::max(5, potion.getPower() / 3));
                DamageSystem::displayDamageReport(*spellTarget, report);
                spellTarget->takeDamage(report.receivedDamage);
                ElementalAffinitySystem::applyPoison(*spellTarget, 4, 2 + potion.getPower() / 24);
                spellTarget->applyWeakening(2, 10 + std::max(1, potion.getPower() / 16));
                if (random.between(1, 100) <= 25)
                {
                    ElementalAffinitySystem::applyBleeding(*spellTarget, 1, 1);
                    std::cout << "Le venin trouve une ouverture et laisse une trace rouge sombre." << std::endl;
                }
                std::cout << "Le venin rampant mord " << spellTarget->getName() << " et fatigue son prochain échange." << std::endl;
                std::cout << spellTarget->getName() << " possède maintenant " << spellTarget->getHp() << "/" << spellTarget->getMaxHp() << " PV." << std::endl;
                std::cout << std::endl;
                if (wave != nullptr) wave->removeDeadAndReplace();
                return true;
            }

            if (potionNameContains(potion, "faille"))
            {
                DamageReport report = DamageSystem::calculateReceivedDamage(*spellTarget, std::max(8, potion.getPower() / 2));
                DamageSystem::displayDamageReport(*spellTarget, report);
                spellTarget->takeDamage(report.receivedDamage);
                spellTarget->applyVulnerability(3, 14 + std::max(1, potion.getPower() / 10));
                int element = random.between(1, 3);
                if (element == 1) ElementalAffinitySystem::applyBurning(*spellTarget, 2, 2 + potion.getPower() / 24);
                else if (element == 2) ElementalAffinitySystem::applyFrost(*spellTarget, 2);
                else ElementalAffinitySystem::applyShock(*spellTarget, 1);
                std::cout << "La faille mord la défense de " << spellTarget->getName() << "." << std::endl;
                std::cout << spellTarget->getName() << " possède maintenant " << spellTarget->getHp() << "/" << spellTarget->getMaxHp() << " PV." << std::endl;
                std::cout << std::endl;
                if (wave != nullptr) wave->removeDeadAndReplace();
                return true;
            }

            DamageReport report = DamageSystem::calculateReceivedDamage(*spellTarget, potion.getPower());
            DamageSystem::displayDamageReport(*spellTarget, report);
            spellTarget->takeDamage(report.receivedDamage);
            if (random.between(1, 100) <= 45) ElementalAffinitySystem::applyShock(*spellTarget, 1);
            else ElementalAffinitySystem::applyBurning(*spellTarget, 2, 2);
            std::cout << "L'étincelle arcanique frappe " << spellTarget->getName() << "." << std::endl;
            std::cout << spellTarget->getName() << " possède maintenant " << spellTarget->getHp() << "/" << spellTarget->getMaxHp() << " PV." << std::endl;
            std::cout << std::endl;
            if (wave != nullptr) wave->removeDeadAndReplace();
            return true;
        }

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

    std::cout << "[cette option est inaccessible dans ce combat]" << std::endl;
    std::cout << std::endl;

    return false;
}
