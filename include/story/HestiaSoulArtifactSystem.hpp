#pragma once

#include <string>
#include <vector>

class Player;
class Weapon;

struct HestiaArtifactKillResult
{
    bool counted = false;
    bool levelUp = false;
    bool secondFormJustUnlocked = false;
    int level = 1;
    int damage = 3;
    int killsRemaining = 1;
    std::string reason;
};

namespace HestiaSoulArtifactSystem
{
    constexpr int SoulsPerFamilyLimit = 5;

    bool grant(Player& player, const std::string& adaptedPrimaryForm);
    bool setAdaptedPrimaryForm(Player& player, const std::string& form);
    bool chooseSecondForm(Player& player, const std::string& form);
    bool switchToPrimary(Player& player);
    bool switchToSecondary(Player& player);
    std::vector<std::string> weaponFormChoices();

    bool isEquipped(const Player& player);
    bool equip(Player& player);
    bool unequip(Player& player);
    Weapon buildWeapon(const Player& player);

    int damage(const Player& player);
    int killsRemaining(const Player& player);
    std::string restingForm(const Player& player);
    std::string activeWeaponForm(const Player& player);
    std::vector<std::string> statusLines(const Player& player);

    HestiaArtifactKillResult recordKillingBlow(
        Player& player,
        const std::string& targetFamilyId,
        bool artifactDealtKillingBlow
    );
}
