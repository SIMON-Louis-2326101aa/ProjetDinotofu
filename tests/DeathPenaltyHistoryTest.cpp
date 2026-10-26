#include <cassert>
#include <string>

#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "item/armor/Armor.hpp"
#include "item/weapon/Weapon.hpp"
#include "progression/DifficultyMode.hpp"
#include "progression/death/DeathPenaltySystem.hpp"

namespace
{
    const PlayerHistoricalEvent* findEvent(
        const Player& player,
        const std::string& category,
        const std::string& subjectId
    )
    {
        const auto& events = player.getHistoricalEvents();
        for (auto it = events.rbegin(); it != events.rend(); ++it)
        {
            if (it->category == category && it->subjectId == subjectId)
            {
                return &(*it);
            }
        }
        return nullptr;
    }
}

int main()
{
    {
        Player player;
        player.getInventory().addWeapon(Weapon(
            "Épée témoin",
            "Arme de test pour vérifier la provenance d'une perte forcée.",
            100,
            WeaponType::Sword,
            1,
            3,
            0,
            100
        ));
        assert(player.equipWeapon(0));
        const std::string weaponId = player.getEquippedWeapon().getPersistentId();
        assert(!weaponId.empty());

        Random random(9); // premier jet std::mt19937 : 2 -> vol en difficulté normale (4 %)
        const DeathPenaltyResult result = DeathPenaltySystem::applyNonLethalDeathPenalty(
            player,
            DifficultyMode::Normal,
            random,
            true
        );

        assert(result.wasWeaponStolen());
        assert(!player.hasEquippedWeapon());
        const PlayerHistoricalEvent* event = findEvent(player, "equipment_lost", weaponId);
        assert(event != nullptr);
        assert(event->resolved);
        assert(event->label.find("Épée témoin") != std::string::npos);
    }

    {
        Player player;
        player.getInventory().addWeapon(Weapon(
            "Lame de rupture",
            "Arme de test pour vérifier une casse forcée sans destruction.",
            100,
            WeaponType::Sword,
            1,
            3,
            0,
            100
        ));
        assert(player.equipWeapon(0));
        const std::string weaponId = player.getEquippedWeapon().getPersistentId();

        Random random(7); // jets : 8 puis 23 -> casse Nightmare (30 %), pas irréparable (18 %)
        const DeathPenaltyResult result = DeathPenaltySystem::applyNonLethalDeathPenalty(
            player,
            DifficultyMode::Nightmare,
            random,
            false
        );

        assert(result.wasWeaponBroken());
        assert(!result.wasWeaponIrreparable());
        assert(player.hasEquippedWeapon());
        assert(player.getEquippedWeapon().isBroken());
        const PlayerHistoricalEvent* event = findEvent(player, "equipment_forced_break", weaponId);
        assert(event != nullptr);
        assert(event->label.find("Lame de rupture") != std::string::npos);
    }

    {
        Player player;
        player.getInventory().addArmor(Armor(
            "Armure condamnée",
            "Armure de test pour vérifier une destruction irréparable.",
            100,
            ArmorType::Leather,
            0,
            1,
            100
        ));
        assert(player.equipArmor(0));
        const std::string armorId = player.getEquippedArmor().getPersistentId();
        assert(!armorId.empty());

        Random random(5); // jets : 23 puis 6 -> casse Nightmare puis irréparable
        const DeathPenaltyResult result = DeathPenaltySystem::applyNonLethalDeathPenalty(
            player,
            DifficultyMode::Nightmare,
            random,
            false
        );

        assert(result.wasArmorBroken());
        assert(result.wasArmorIrreparable());
        assert(!player.hasEquippedArmor());
        const PlayerHistoricalEvent* event = findEvent(player, "equipment_destroyed", armorId);
        assert(event != nullptr);
        assert(event->resolved);
        assert(event->label.find("Armure condamnée") != std::string::npos);
    }

    return 0;
}
