// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/ai/CombatAI.hpp"

#include "entity/Player.hpp"

#include "item/Inventory.hpp"
#include "item/consumable/ConsumableType.hpp"

bool CombatAI::canUseHealingPotion(const Entity& entity)
{
    const Player* player = dynamic_cast<const Player*>(&entity);

    if (player != nullptr)
    {
        return player->getInventory().countConsumables(ConsumableType::Healing) > 0;
    }

    return entity.getHealingPotions() > 0;
}

bool CombatAI::canUseDamagePotion(const Entity& entity)
{
    const Player* player = dynamic_cast<const Player*>(&entity);

    if (player != nullptr)
    {
        return player->getInventory().countConsumables(ConsumableType::Damage) > 0;
    }

    return entity.getDamagePotions() > 0;
}

int CombatAI::calculateHpPercentage(const Entity& entity)
{
    if (entity.getMaxHp() <= 0)
    {
        return 0;
    }

    return entity.getHp() * 100 / entity.getMaxHp();
}

AIAction CombatAI::chooseAIAction(const Entity& ai, Random& random)
{
    bool healingAvailable = canUseHealingPotion(ai);
    bool damageAvailable = canUseDamagePotion(ai);

    int hpPercentage = calculateHpPercentage(ai);
    int roll = random.between(1, 100);

    if (hpPercentage <= 30)
    {
        return chooseDangerAIAction(
            healingAvailable,
            damageAvailable,
            roll
        );
    }

    if (hpPercentage <= 60)
    {
        return chooseMidHealthAIAction(
            healingAvailable,
            damageAvailable,
            roll
        );
    }

    return chooseStableAIAction(
        damageAvailable,
        roll
    );
}

AIAction CombatAI::chooseBossAction(const Boss& boss, Random& random)
{
    bool healingAvailable = boss.getHealingPotions() > 0;
    bool damageAvailable = boss.getDamagePotions() > 0;
    bool ultimateAvailable = boss.canUseUltimate();

    int hpPercentage = calculateHpPercentage(boss);
    int roll = random.between(1, 100);

    if (hpPercentage <= 25)
    {
        return chooseDangerBossAction(
            healingAvailable,
            damageAvailable,
            ultimateAvailable,
            roll
        );
    }

    if (ultimateAvailable)
    {
        return chooseBossActionWithUltimateAvailable(
            damageAvailable,
            roll
        );
    }

    return chooseStableBossAction(
        damageAvailable,
        roll
    );
}

AIAction CombatAI::chooseDangerAIAction(
    bool healingAvailable,
    bool damageAvailable,
    int roll
)
{
    if (healingAvailable && roll <= 60)
    {
        return AIAction::HealingPotion;
    }

    if (damageAvailable && roll <= 75)
    {
        return AIAction::DamagePotion;
    }

    if (roll <= 97)
    {
        return AIAction::Attack;
    }

    return AIAction::SkipTurn;
}

AIAction CombatAI::chooseMidHealthAIAction(
    bool healingAvailable,
    bool damageAvailable,
    int roll
)
{
    if (healingAvailable && roll <= 30)
    {
        return AIAction::HealingPotion;
    }

    if (damageAvailable && roll <= 48)
    {
        return AIAction::DamagePotion;
    }

    if (roll <= 96)
    {
        return AIAction::Attack;
    }

    return AIAction::SkipTurn;
}

AIAction CombatAI::chooseStableAIAction(
    bool damageAvailable,
    int roll
)
{
    if (damageAvailable && roll <= 18)
    {
        return AIAction::DamagePotion;
    }

    if (roll <= 96)
    {
        return AIAction::Attack;
    }

    return AIAction::SkipTurn;
}

AIAction CombatAI::chooseDangerBossAction(
    bool healingAvailable,
    bool damageAvailable,
    bool ultimateAvailable,
    int roll
)
{
    if (healingAvailable && roll <= 35)
    {
        return AIAction::HealingPotion;
    }

    if (ultimateAvailable && roll <= 75)
    {
        return AIAction::Ultimate;
    }

    if (damageAvailable && roll <= 87)
    {
        return AIAction::DamagePotion;
    }

    return AIAction::Attack;
}

AIAction CombatAI::chooseBossActionWithUltimateAvailable(
    bool damageAvailable,
    int roll
)
{
    if (roll <= 55)
    {
        return AIAction::Ultimate;
    }

    if (damageAvailable && roll <= 70)
    {
        return AIAction::DamagePotion;
    }

    return AIAction::Attack;
}

AIAction CombatAI::chooseStableBossAction(
    bool damageAvailable,
    int roll
)
{
    if (damageAvailable && roll <= 18)
    {
        return AIAction::DamagePotion;
    }

    return AIAction::Attack;
}