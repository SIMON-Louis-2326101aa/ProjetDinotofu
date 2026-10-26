#include "world/clan/ClanBlacksmithSystem.hpp"
#include "world/clan/ClanHeadquartersSystem.hpp"

#include "entity/Player.hpp"
#include "item/armor/Armor.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "item/weapon/Weapon.hpp"

#include <cassert>
#include <string>

namespace
{
    void addUpgradeResources(Player& player, ClanHeadquartersUpgradeKind kind)
    {
        player.getInventory().earnCopper(ClanHeadquartersSystem::upgradeCostCopper(kind));
        for (const ClanHeadquartersMaterialRequirement& requirement : ClanHeadquartersSystem::upgradeMaterials(kind))
            player.getInventory().addMaterial(MaterialCatalog::createById(requirement.materialId, requirement.quantity));
    }
}

int main()
{
    Player player;
    player.recordCanonicalEvent(
        "clan_hq_state",
        "built",
        "city=valebrume | built_day=0 | building_level=1 | beds=2 | quest_board=1 | counter=1",
        1
    );
    player.recordCanonicalEvent("clan_hq_building_expansions", "level_2", "Agrandissement général terminé : QG niveau 2", 1);
    player.recordCanonicalEvent("clan_hq_modules", "counter", "Comptoir du QG installé", 1);
    player.recordCanonicalEvent("clan_hq_modules", "quest_board", "Tableau de quêtes niveau 1", 1);
    player.recordCanonicalEvent("clan_hq_modules", "bedroom_base", "Deux chambres", 1);

    std::string reason;
    ClanHeadquartersStatus hq = ClanHeadquartersSystem::statusFor(player);
    assert(hq.forgeWorkshopLevel == 0);
    assert(!ClanBlacksmithSystem::canHire(player, reason));

    addUpgradeResources(player, ClanHeadquartersUpgradeKind::ForgeWorkshop);
    assert(ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::ForgeWorkshop, reason));
    assert(ClanHeadquartersSystem::startUpgrade(player, ClanHeadquartersUpgradeKind::ForgeWorkshop, reason));
    player.advanceWorldDays(ClanHeadquartersSystem::ForgeWorkshopLevel1Days);
    assert(ClanHeadquartersSystem::completeUpgradeIfReady(player));

    hq = ClanHeadquartersSystem::statusFor(player);
    assert(hq.forgeWorkshopLevel == 1);
    assert(hq.modulesUsed == 4);

    const std::string candidate = ClanBlacksmithSystem::candidateNameFor(player);
    assert(!candidate.empty());
    player.getInventory().earnCopper(ClanBlacksmithSystem::HireCostCopper);
    assert(ClanBlacksmithSystem::canHire(player, reason));
    assert(ClanBlacksmithSystem::hire(player, reason));

    ClanBlacksmithStatus smith = ClanBlacksmithSystem::statusFor(player);
    assert(smith.hired);
    assert(smith.name == candidate);
    assert(smith.skillLevel == 1);
    assert(smith.repairCapPercent == 60);

    Weapon sword("Épée de test", "Lame volontairement usée pour tester la forge du clan.", 50, WeaponType::Sword, 1, 3, 0, 100);
    sword.loseDurability(80);
    player.getInventory().addWeapon(sword);
    player.getInventory().addMaterial(MaterialCatalog::createById("rusted_metal_fragment", 4));
    player.getInventory().earnCopper(1000);

    ClanBlacksmithRepairQuote quote = ClanBlacksmithSystem::quoteWeaponRepair(player, 0);
    assert(quote.valid);
    assert(quote.currentDurability == 20);
    assert(quote.targetDurability == 60);
    assert(quote.durabilityRestored == 40);
    assert(quote.materialQuantity == 2);

    ClanBlacksmithRepairQuote result;
    assert(ClanBlacksmithSystem::repairWeapon(player, 0, result, reason));
    assert(player.getInventory().getWeapon(0).getDurability() == 60);
    smith = ClanBlacksmithSystem::statusFor(player);
    assert(smith.jobsCompleted == 1);
    assert(smith.experience > 0);

    // A basic forge cannot continue repairing above the artisan's current cap.
    assert(!ClanBlacksmithSystem::quoteWeaponRepair(player, 0).valid);

    // Simulate accumulated workshop experience and verify that progression unlocks a higher cap.
    player.recordCanonicalEvent("clan_hq_blacksmith_xp", smith.name, "Expérience de commandes supplémentaires", 145);
    smith = ClanBlacksmithSystem::statusFor(player);
    assert(smith.skillLevel >= 3);
    assert(smith.repairCapPercent >= 76);
    assert(smith.friendDiscountPercent >= 4);
    assert(smith.specializationChoiceAvailable);

    // Level 3 unlocks one permanent specialization for this artisan.
    assert(ClanBlacksmithSystem::canChooseSpecialization(player, reason));
    assert(ClanBlacksmithSystem::chooseSpecialization(player, ClanBlacksmithSpecialization::Weapons, reason));
    smith = ClanBlacksmithSystem::statusFor(player);
    assert(smith.specialization == ClanBlacksmithSpecialization::Weapons);
    assert(!smith.specializationChoiceAvailable);
    assert(!ClanBlacksmithSystem::chooseSpecialization(player, ClanBlacksmithSpecialization::Armor, reason));

    Weapon secondSword("Épée spécialisée", "Vérifie le bonus d'une spécialisation armes.", 55, WeaponType::Sword, 1, 3, 0, 100);
    secondSword.loseDurability(80);
    player.getInventory().addWeapon(secondSword);
    player.getInventory().addMaterial(MaterialCatalog::createById("rusted_metal_fragment", 4));
    ClanBlacksmithRepairQuote specializedWeaponQuote = ClanBlacksmithSystem::quoteWeaponRepair(player, 1);
    assert(specializedWeaponQuote.valid);
    assert(specializedWeaponQuote.targetDurability == 81); // 76% base +5 from weapon specialization.
    assert(specializedWeaponQuote.friendDiscountPercent >= 4);
    assert(specializedWeaponQuote.costCopper < specializedWeaponQuote.baseCostCopper);

    Armor leather("Armure de cuir de test", "Protection usée pour une commande réelle.", 40, ArmorType::Leather, 5, 1, 100);
    leather.loseDurability(80);
    player.getInventory().addArmor(leather);
    player.getInventory().addMaterial(MaterialCatalog::createById("worn_leather_piece", 4));

    quote = ClanBlacksmithSystem::quoteArmorRepair(player, 0);
    assert(quote.valid);
    assert(quote.targetDurability >= 68);
    assert(ClanBlacksmithSystem::repairArmor(player, 0, result, reason));
    assert(player.getInventory().getArmor(0).getDurability() >= 68);

    Armor magical("Robe arcanique de test", "Une pièce magique hors compétence de la forge de base.", 100, ArmorType::Magical, 8, 2, 100);
    magical.loseDurability(80);
    player.getInventory().addArmor(magical);
    const ClanBlacksmithRepairQuote magicalQuote = ClanBlacksmithSystem::quoteArmorRepair(player, 1);
    assert(!magicalQuote.valid);
    assert(magicalQuote.reason.find("magique") != std::string::npos);

    return 0;
}
