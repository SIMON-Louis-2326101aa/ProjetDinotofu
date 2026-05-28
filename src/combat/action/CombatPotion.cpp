// EN: CombatPotion.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatPotion.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/action/CombatPotion.hpp"

#include "combat/action/CombatAttack.hpp"
#include "combat/threat/ThreatSystem.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include "entity/Player.hpp"

#include "item/consumable/Consumable.hpp"
#include "item/consumable/ConsumableType.hpp"

#include <iostream>
#include <string>
#include <vector>

bool CombatPotion::executeHealingPotion(
    Entity& entity,
    int potionHealAmount
)
{
    Player* player = dynamic_cast<Player*>(&entity);

    if (player != nullptr)
    {
        Consumable potion;
        int potionIndex = player->getInventory().findFirstConsumable(ConsumableType::Healing);

        if (potionIndex == -1)
        {
            MessageScreen::show(
                "POTION DE SOIN INTROUVABLE",
                "combat.potion.healing.missing.player",
                {
                    player->getName() + " fouille son inventaire...",
                    "Mais aucune potion de soin n'est disponible.",
                    "Il peut encore tenter autre chose."
                }
            );

            return false;
        }

        potion = player->getInventory().getConsumable(potionIndex);

        if (!player->hasInfiniteConsumables())
        {
            player->getInventory().removeConsumable(potionIndex);
        }

        int hpBefore = player->getHp();
        player->heal(potion.getPower());
        ThreatSystem::markSelfHealingAction(*player);

        MessageScreen::show(
            "POTION DE SOIN",
            "combat.potion.healing.result.player",
            {
                player->getName() + " utilise : " + potion.getName() + ".",
                "Soin : +" + std::to_string(player->getHp() - hpBefore) + " PV.",
                "PV : " + std::to_string(hpBefore) + " -> " + std::to_string(player->getHp()) + "/" + std::to_string(player->getMaxHp()) + "."
            }
        );

        return true;
    }

    if (entity.useHealingPotion(potionHealAmount))
    {
        ThreatSystem::markSelfHealingAction(entity);

        MessageScreen::show(
            "POTION DE SOIN",
            "combat.potion.healing.result.entity",
            {
                entity.getName() + " utilise une potion de soin.",
                "Sa vitalité revient lentement.",
                "PV actuels : " + std::to_string(entity.getHp()) + "/" + std::to_string(entity.getMaxHp()) + "."
            }
        );

        return true;
    }

    MessageScreen::show(
        "POTION DE SOIN INTROUVABLE",
        "combat.potion.healing.missing.entity",
        {
            entity.getName() + " n'a plus aucune potion de soin.",
            "L'action est annulée."
        }
    );

    return false;
}

bool CombatPotion::executeDamagePotion(
    Entity& attacker,
    Entity& defender,
    Random& random,
    int potionDamageBonus
)
{
    int usedBonus = potionDamageBonus;

    Player* player = dynamic_cast<Player*>(&attacker);

    if (player != nullptr)
    {
        Consumable potion;
        int potionIndex = player->getInventory().findFirstConsumable(ConsumableType::Damage);

        if (potionIndex == -1)
        {
            MessageScreen::show(
                "POTION OFFENSIVE INTROUVABLE",
                "combat.potion.damage.missing.player",
                {
                    player->getName() + " cherche une potion de rage dans son inventaire...",
                    "Mais aucune potion offensive n'est disponible.",
                    "Il peut encore tenter autre chose."
                }
            );

            return false;
        }

        potion = player->getInventory().getConsumable(potionIndex);

        if (!player->hasInfiniteConsumables())
        {
            player->getInventory().removeConsumable(potionIndex);
        }

        usedBonus = potion.getPower();

        MessageScreen::show(
            "POTION OFFENSIVE",
            "combat.potion.damage.consume.player",
            {
                player->getName() + " utilise : " + potion.getName() + ".",
                "Bonus de puissance : +" + std::to_string(usedBonus) + ".",
                "L'attaque renforcée va être résolue."
            },
            false
        );
    }
    else
    {
        if (!attacker.consumeDamagePotion())
        {
            MessageScreen::show(
                "POTION OFFENSIVE ÉPUISÉE",
                "combat.potion.damage.missing.entity",
                {
                    attacker.getName() + " cherche une potion offensive, mais sa rage est déjà épuisée.",
                    "Il peut encore tenter autre chose."
                }
            );

            return false;
        }
    }

    MessageScreen::show(
        "RAGE OFFENSIVE",
        "combat.potion.damage.charge",
        {
            attacker.getName() + " sent ses forces monter d'un coup.",
            "Une rage brutale s'empare de lui..."
        },
        false
    );

    Console::pauseSeconds(1);

    CombatAttack::executeBoostedAttack(
        attacker,
        defender,
        random,
        usedBonus
    );

    return true;
}
