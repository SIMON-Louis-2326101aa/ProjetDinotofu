#include "world/clan/ClanHeadquartersSystem.hpp"

#include "entity/Player.hpp"
#include "item/material/MaterialCatalog.hpp"

#include <cassert>
#include <string>

namespace
{
    std::string recruitLabel(const std::string& name, int level = 8)
    {
        return name + " | race=Humain | profil=Éclaireur | trait=fiable | niveau_recrutement=" + std::to_string(level)
            + " | rang_origine_index=1 | part_base=18 | salaire=90 | profil_seulement=1";
    }

    int count(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
            if (record.category == category && record.key == key) return record.count;
        return 0;
    }

    void addUpgradeResources(Player& player, ClanHeadquartersUpgradeKind kind)
    {
        player.getInventory().earnCopper(ClanHeadquartersSystem::upgradeCostCopper(kind));
        for (const ClanHeadquartersMaterialRequirement& requirement : ClanHeadquartersSystem::upgradeMaterials(kind))
            player.getInventory().addMaterial(MaterialCatalog::createById(requirement.materialId, requirement.quantity));
    }

    void addRecruit(Player& player, const std::string& name)
    {
        player.recordCanonicalEvent("pnj_recrutables_retenus", name, recruitLabel(name), 1);
        player.recordCanonicalEvent("recrues_engagees", name, name + " rejoint réellement le groupe", 1);
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
    player.recordCanonicalEvent("clan_hq_passive_clock", "last_check", "Horloge initialisée", 1);

    addRecruit(player, "Aline");
    addRecruit(player, "Boros");
    std::string reason;
    assert(ClanHeadquartersSystem::assignRecruit(player, "Aline", reason));
    assert(ClanHeadquartersSystem::assignRecruit(player, "Boros", reason));

    // A plain HQ with two residents still cannot host the larger clan celebration.
    assert(!ClanHeadquartersSystem::canOrganizeCelebration(player, reason));

    const int modulesBeforeComfort = ClanHeadquartersSystem::statusFor(player).modulesUsed;
    addUpgradeResources(player, ClanHeadquartersUpgradeKind::ComfortDecor);
    assert(ClanHeadquartersSystem::canStartUpgrade(player, ClanHeadquartersUpgradeKind::ComfortDecor, reason));
    assert(ClanHeadquartersSystem::startUpgrade(player, ClanHeadquartersUpgradeKind::ComfortDecor, reason));
    player.advanceWorldDays(ClanHeadquartersSystem::ComfortDecorLevel1Days);
    assert(ClanHeadquartersSystem::completeUpgradeIfReady(player));

    ClanHeadquartersStatus status = ClanHeadquartersSystem::statusFor(player);
    assert(status.comfortDecorLevel == 1);
    // Furnishings improve existing rooms and must not steal the last future workshop slot.
    assert(status.modulesUsed == modulesBeforeComfort);

    // The bar is optional for a celebration, but installing it reduces the logistics cost.
    const long long partyWithoutBar = ClanHeadquartersSystem::celebrationCostCopper(player);
    assert(partyWithoutBar == ClanHeadquartersSystem::CelebrationBaseCostCopper);
    addUpgradeResources(player, ClanHeadquartersUpgradeKind::CommonRoomBar);
    assert(ClanHeadquartersSystem::startUpgrade(player, ClanHeadquartersUpgradeKind::CommonRoomBar, reason));
    player.advanceWorldDays(ClanHeadquartersSystem::CommonRoomBarLevel1Days);
    assert(ClanHeadquartersSystem::completeUpgradeIfReady(player));
    assert(ClanHeadquartersSystem::celebrationCostCopper(player)
        == ClanHeadquartersSystem::CelebrationBaseCostCopper - ClanHeadquartersSystem::CelebrationBarDiscountCopper);

    player.getInventory().earnCopper(ClanHeadquartersSystem::celebrationCostCopper(player) * 2);
    ClanHeadquartersCelebrationReport celebration;
    assert(ClanHeadquartersSystem::canOrganizeCelebration(player, reason));
    assert(ClanHeadquartersSystem::organizeCelebration(player, celebration, reason));
    assert(celebration.participantCount == 2);
    assert(celebration.moraleChargesGranted == 2);
    assert(count(player, "clan_hq_festivals", "headquarters_party") == 1);
    assert(count(player, "clan_hq_festive_morale", "Aline") == 1);
    assert(count(player, "clan_hq_festive_morale", "Boros") == 1);
    assert(!ClanHeadquartersSystem::canOrganizeCelebration(player, reason));

    // The celebration boost is tiny, only affects one autonomous mission per resident,
    // and remains inside the global 15-25% autonomous-XP rule.
    player.advanceWorldDays(3);
    ClanHeadquartersPassiveReport passive = ClanHeadquartersSystem::processPassiveActivity(player);
    assert(passive.missionsCompleted == 4); // 8 elapsed days from the initial HQ clock -> two cycles for two residents.
    assert(count(player, "clan_hq_festive_morale_consumed", "Aline") == 1);
    assert(count(player, "clan_hq_festive_morale_consumed", "Boros") == 1);

    // Cooldown is measured from the celebration day, not from the later mission processing.
    const int elapsedSinceParty = player.getWorldDaysElapsed() - (ClanHeadquartersSystem::ComfortDecorLevel1Days + ClanHeadquartersSystem::CommonRoomBarLevel1Days);
    const int remaining = ClanHeadquartersSystem::CelebrationCooldownDays - elapsedSinceParty;
    if (remaining > 0) player.advanceWorldDays(remaining);
    assert(ClanHeadquartersSystem::canOrganizeCelebration(player, reason));

    return 0;
}
