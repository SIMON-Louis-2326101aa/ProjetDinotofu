// EN: CombatPotion.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: CombatPotion.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/action/CombatPotion.hpp"

#include "combat/action/CombatAttack.hpp"
#include "combat/threat/ThreatSystem.hpp"

#include "core/Console.hpp"

#include "entity/Player.hpp"

#include "item/consumable/Consumable.hpp"
#include "item/consumable/ConsumableType.hpp"

#include <iostream>

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
            std::cout << player->getName()
                      << " fouille son inventaire..."
                      << std::endl;
            std::cout << "Mais aucune potion de soin n'est disponible." << std::endl;
            std::cout << "Il peut encore tenter autre chose." << std::endl;
            std::cout << std::endl;

            return false;
        }

        potion = player->getInventory().getConsumable(potionIndex);

        if (!player->hasInfiniteConsumables())
        {
            player->getInventory().removeConsumable(potionIndex);
        }

        player->heal(potion.getPower());
        ThreatSystem::markSelfHealingAction(*player);

        std::cout << player->getName()
                  << " utilise : "
                  << potion.getName()
                  << "."
                  << std::endl;

        std::cout << "Ses blessures se referment, et il récupère "
                  << potion.getPower()
                  << " PV."
                  << std::endl;

        std::cout << player->getName()
                  << " possède maintenant "
                  << player->getHp()
                  << "/"
                  << player->getMaxHp()
                  << " PV."
                  << std::endl;

        std::cout << std::endl;

        return true;
    }

    if (entity.useHealingPotion(potionHealAmount))
    {
        ThreatSystem::markSelfHealingAction(entity);

        std::cout << entity.getName()
                  << " utilise une potion de soin."
                  << std::endl;
        std::cout << "Sa vitalité revient lentement." << std::endl;
        std::cout << std::endl;

        return true;
    }

    std::cout << entity.getName()
              << " n'a plus aucune potion de soin."
              << std::endl;
    std::cout << std::endl;

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
            std::cout << player->getName()
                      << " cherche une potion de rage dans son inventaire..."
                      << std::endl;
            std::cout << "Mais aucune potion offensive n'est disponible." << std::endl;
            std::cout << "Il peut encore tenter autre chose." << std::endl;
            std::cout << std::endl;

            return false;
        }

        potion = player->getInventory().getConsumable(potionIndex);

        if (!player->hasInfiniteConsumables())
        {
            player->getInventory().removeConsumable(potionIndex);
        }

        usedBonus = potion.getPower();

        std::cout << player->getName()
                  << " utilise : "
                  << potion.getName()
                  << "."
                  << std::endl;
    }
    else
    {
        if (!attacker.consumeDamagePotion())
        {
            std::cout << attacker.getName()
                      << " cherche une potion offensive, mais sa rage est déjà épuisée."
                      << std::endl;
            std::cout << "Il peut encore tenter autre chose." << std::endl;
            std::cout << std::endl;

            return false;
        }
    }

    std::cout << attacker.getName()
              << " sent ses forces monter d'un coup."
              << std::endl;
    std::cout << "Une rage brutale s'empare de lui..." << std::endl;
    std::cout << std::endl;

    Console::pauseSeconds(1);

    CombatAttack::executeBoostedAttack(
        attacker,
        defender,
        random,
        usedBonus
    );

    return true;
}
