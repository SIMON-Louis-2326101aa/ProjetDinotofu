#include <cassert>
#include <string>

#include "entity/Player.hpp"
#include "item/armor/Armor.hpp"
#include "item/weapon/Weapon.hpp"

int main()
{
    Player player;

    Weapon weapon(
        "Lame confisquée",
        "Arme exacte utilisée pour tester une récupération Grinka sans recréation.",
        420,
        WeaponType::Sword,
        6,
        12,
        5,
        140
    );
    weapon.setPersistentId("grinka_weapon_exact_001");
    weapon.loseDurability(37);
    weapon.addEnchantment("Rune témoin");

    Armor armor(
        "Armure confisquée",
        "Armure exacte utilisée pour tester une récupération Grinka sans recréation.",
        510,
        ArmorType::Chainmail,
        18,
        4,
        170
    );
    armor.setPersistentId("grinka_armor_exact_001");
    armor.loseDurability(51);
    armor.addEnchantment("Sceau témoin");

    player.recordGrinkaStolenWeapon(weapon);
    player.recordGrinkaStolenArmor(armor);
    player.recordGrinkaGoldTheft(73);

    assert(player.hasGrinkaBossTheftPresent());
    assert(player.hasGrinkaStolenWeapon());
    assert(player.hasGrinkaStolenArmor());
    assert(player.getGrinkaStolenGold() == 73);

    const int weaponCountBefore = player.getInventory().getWeaponCount();
    const int armorCountBefore = player.getInventory().getArmorCount();
    const long long moneyBefore = player.getInventory().getEconomyUnits();

    assert(player.recoverGrinkaStolenWeapon());
    assert(!player.hasGrinkaStolenWeapon());
    assert(player.hasGrinkaBossTheftPresent());
    assert(player.getInventory().getWeaponCount() == weaponCountBefore + 1);
    const Weapon& recoveredWeapon = player.getInventory().getWeapons().back();
    assert(recoveredWeapon.getPersistentId() == "grinka_weapon_exact_001");
    assert(recoveredWeapon.getDurability() == weapon.getDurability());
    assert(recoveredWeapon.getEnchantmentsSaveText() == weapon.getEnchantmentsSaveText());

    assert(player.recoverGrinkaStolenArmor());
    assert(!player.hasGrinkaStolenArmor());
    assert(player.hasGrinkaBossTheftPresent());
    assert(player.getInventory().getArmorCount() == armorCountBefore + 1);
    const Armor& recoveredArmor = player.getInventory().getArmors().back();
    assert(recoveredArmor.getPersistentId() == "grinka_armor_exact_001");
    assert(recoveredArmor.getDurability() == armor.getDurability());
    assert(recoveredArmor.getEnchantmentsSaveText() == armor.getEnchantmentsSaveText());

    assert(player.recoverGrinkaStolenGold() == 73);
    assert(player.getInventory().getEconomyUnits() == moneyBefore + 73);
    assert(player.getGrinkaStolenGold() == 0);
    assert(!player.hasGrinkaBossTheftPresent());

    // No duplication on repeated recovery attempts.
    assert(!player.recoverGrinkaStolenWeapon());
    assert(!player.recoverGrinkaStolenArmor());
    assert(player.recoverGrinkaStolenGold() == 0);
    assert(player.getInventory().getWeaponCount() == weaponCountBefore + 1);
    assert(player.getInventory().getArmorCount() == armorCountBefore + 1);
    assert(player.getInventory().getEconomyUnits() == moneyBefore + 73);

    return 0;
}
