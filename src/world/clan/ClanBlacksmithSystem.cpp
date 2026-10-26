#include "world/clan/ClanBlacksmithSystem.hpp"

#include "character/RandomNameGenerator.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "item/Inventory.hpp"
#include "item/armor/Armor.hpp"
#include "item/weapon/Weapon.hpp"
#include "world/clan/ClanHeadquartersSystem.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <sstream>

namespace
{
    int recordCount(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key) return record.count;
        }
        return 0;
    }

    const PlayerJournalRecord* findRecord(const Player& player, const std::string& category, const std::string& key)
    {
        for (const PlayerJournalRecord& record : player.getCanonicalJournalRecords())
        {
            if (record.category == category && record.key == key) return &record;
        }
        return nullptr;
    }

    std::string stringField(const std::string& label, const std::string& fieldName, const std::string& fallback)
    {
        const std::string marker = fieldName + "=";
        const std::size_t position = label.find(marker);
        if (position == std::string::npos) return fallback;
        const std::size_t start = position + marker.size();
        std::size_t end = label.find(" | ", start);
        if (end == std::string::npos) end = label.size();
        if (end <= start) return fallback;
        return label.substr(start, end - start);
    }

    ClanBlacksmithSpecialization specializationFromId(const std::string& id)
    {
        if (id == "weapons") return ClanBlacksmithSpecialization::Weapons;
        if (id == "armor") return ClanBlacksmithSpecialization::Armor;
        if (id == "maintenance") return ClanBlacksmithSpecialization::Maintenance;
        return ClanBlacksmithSpecialization::None;
    }

    std::string specializationId(ClanBlacksmithSpecialization specialization)
    {
        switch (specialization)
        {
            case ClanBlacksmithSpecialization::Weapons: return "weapons";
            case ClanBlacksmithSpecialization::Armor: return "armor";
            case ClanBlacksmithSpecialization::Maintenance: return "maintenance";
            case ClanBlacksmithSpecialization::None: break;
        }
        return "none";
    }

    std::uint32_t stableSeed(const std::string& text)
    {
        std::uint32_t hash = 2166136261u;
        for (unsigned char c : text)
        {
            hash ^= static_cast<std::uint32_t>(c);
            hash *= 16777619u;
        }
        return hash;
    }

    int skillLevelFromExperience(int experience)
    {
        int level = 1;
        for (int candidate = 2; candidate <= ClanBlacksmithSystem::MaxSkillLevel; ++candidate)
        {
            if (experience >= ClanBlacksmithSystem::experienceRequiredForLevel(candidate)) level = candidate;
            else break;
        }
        return level;
    }

    std::pair<std::string, std::string> weaponRepairMaterial(const Weapon& weapon)
    {
        switch (weapon.getType())
        {
            case WeaponType::Bow:
                return {"salted_rope_knot", "Cordages / ligatures solides"};
            case WeaponType::Staff:
                return {"worn_leather_piece", "Cuir / renforts de poignée"};
            case WeaponType::Dagger:
            case WeaponType::Sword:
            case WeaponType::Axe:
            case WeaponType::Hammer:
            case WeaponType::Spear:
            case WeaponType::BareHands:
            case WeaponType::Unknown:
            default:
                return {"rusted_metal_fragment", "Fragments de métal utilisables"};
        }
    }

    std::pair<std::string, std::string> armorRepairMaterial(const Armor& armor)
    {
        switch (armor.getType())
        {
            case ArmorType::Cloth:
            case ArmorType::Leather:
                return {"worn_leather_piece", "Cuir / pièces de renfort"};
            case ArmorType::Chainmail:
            case ArmorType::Plate:
                return {"rusted_metal_fragment", "Fragments de métal utilisables"};
            case ArmorType::Magical:
                return {"", ""};
            case ArmorType::Unknown:
            default:
                return {"worn_leather_piece", "Matériaux de réparation polyvalents"};
        }
    }

    ClanBlacksmithRepairQuote baseQuote(
        const Player& player,
        bool armor,
        int inventoryIndex,
        const std::string& itemName,
        int durability,
        int maxDurability,
        bool indestructible,
        const std::pair<std::string, std::string>& material
    )
    {
        ClanBlacksmithRepairQuote quote;
        quote.armor = armor;
        quote.inventoryIndex = inventoryIndex;
        quote.itemName = itemName;
        quote.currentDurability = durability;
        quote.maxDurability = maxDurability;
        quote.materialId = material.first;
        quote.materialLabel = material.second;

        const ClanHeadquartersStatus hq = ClanHeadquartersSystem::statusFor(player);
        const ClanBlacksmithStatus smith = ClanBlacksmithSystem::statusFor(player);
        if (!hq.built || hq.forgeWorkshopLevel <= 0)
        {
            quote.reason = "Aucun atelier de forge n'est installé au QG.";
            return quote;
        }
        if (!smith.hired)
        {
            quote.reason = "L'atelier existe, mais aucun forgeron n'a encore été recruté.";
            return quote;
        }
        if (player.getCurrentCityId() != hq.cityId)
        {
            quote.reason = "Il faut être dans la ville du QG pour confier l'équipement au forgeron du clan.";
            return quote;
        }
        if (indestructible)
        {
            quote.reason = "Cet équipement est indestructible : aucune réparation n'est nécessaire.";
            return quote;
        }
        if (maxDurability <= 0)
        {
            quote.reason = "Cet équipement n'a pas de durabilité réparable.";
            return quote;
        }
        if (armor && material.first.empty())
        {
            quote.reason = "L'atelier de forge niveau 1 ne sait pas restaurer correctement une armure magique. Il faudra un artisan ou un module plus spécialisé.";
            return quote;
        }

        int effectiveRepairCap = smith.repairCapPercent;
        if (!armor && smith.specialization == ClanBlacksmithSpecialization::Weapons) effectiveRepairCap = std::min(95, effectiveRepairCap + 5);
        if (armor && smith.specialization == ClanBlacksmithSpecialization::Armor) effectiveRepairCap = std::min(95, effectiveRepairCap + 5);

        quote.targetDurability = std::max(1, maxDurability * effectiveRepairCap / 100);
        if (durability >= quote.targetDurability)
        {
            quote.reason = smith.name + " ne peut pas améliorer davantage cette pièce avec son niveau, sa spécialisation et cet atelier. Un meilleur artisan ou une forge spécialisée reste nécessaire.";
            return quote;
        }

        quote.durabilityRestored = quote.targetDurability - durability;
        quote.materialQuantity = std::max(1, (quote.durabilityRestored + 24) / 25);
        if (smith.specialization == ClanBlacksmithSpecialization::Maintenance && quote.materialQuantity >= 3)
            quote.materialQuantity -= 1;

        quote.baseCostCopper = 25 + static_cast<long long>(quote.durabilityRestored) * 2;
        quote.friendDiscountPercent = smith.friendDiscountPercent;
        if (smith.specialization == ClanBlacksmithSpecialization::Maintenance)
            quote.friendDiscountPercent = std::min(13, quote.friendDiscountPercent + 3);
        quote.costCopper = std::max<long long>(1, (quote.baseCostCopper * (100 - quote.friendDiscountPercent) + 99) / 100);

        if (player.getInventory().countMaterialById(quote.materialId) < quote.materialQuantity)
        {
            quote.reason = "Matériaux insuffisants : " + quote.materialLabel + " x" + std::to_string(quote.materialQuantity) + ".";
            return quote;
        }
        if (player.getInventory().getTotalCopper() < quote.costCopper)
        {
            quote.reason = "Fonds insuffisants pour payer le travail du forgeron.";
            return quote;
        }

        quote.valid = true;
        quote.reason = "Réparation possible jusqu'à " + std::to_string(effectiveRepairCap) + "% environ de durabilité maximale.";
        if (quote.friendDiscountPercent > 0)
            quote.reason += " Prix du clan : -" + std::to_string(quote.friendDiscountPercent) + "% sur la main-d'oeuvre.";
        return quote;
    }

    int artisanXpForRepair(const ClanBlacksmithRepairQuote& quote)
    {
        return std::max(6, 6 + quote.durabilityRestored / 4);
    }

    void recordCompletedRepair(Player& player, const ClanBlacksmithRepairQuote& quote, int previousSkill)
    {
        const ClanBlacksmithStatus before = ClanBlacksmithSystem::statusFor(player);
        const int gainedXp = artisanXpForRepair(quote);
        player.recordCanonicalEvent(
            "clan_hq_blacksmith_jobs",
            before.name,
            before.name + " termine une réparation réelle au QG : " + quote.itemName,
            1
        );
        player.recordCanonicalEvent(
            "clan_hq_blacksmith_xp",
            before.name,
            before.name + " gagne de l'expérience en travaillant réellement à la forge",
            gainedXp
        );
        player.recordCanonicalEvent(
            "clan_hq_blacksmith_payments",
            before.name,
            "Paiement d'une commande de forge : " + quote.itemName,
            static_cast<int>(quote.costCopper)
        );
        if (quote.friendDiscountPercent > 0)
        {
            player.recordCanonicalEvent(
                "clan_hq_blacksmith_friend_discounts",
                before.name,
                "Réparation bénéficiant du prix d'ami du forgeron du clan",
                1
            );
        }
        const ClanBlacksmithStatus after = ClanBlacksmithSystem::statusFor(player);
        if (after.skillLevel > previousSkill)
        {
            player.recordCanonicalEvent(
                "clan_hq_blacksmith_milestones",
                before.name,
                before.name + " atteint le niveau d'artisan " + std::to_string(after.skillLevel),
                1
            );
        }
    }
}

int ClanBlacksmithSystem::experienceRequiredForLevel(int level)
{
    switch (std::clamp(level, 1, MaxSkillLevel))
    {
        case 1: return 0;
        case 2: return 60;
        case 3: return 160;
        case 4: return 320;
        case 5: return 560;
        default: return 0;
    }
}

int ClanBlacksmithSystem::repairCapPercentForSkill(int skillLevel)
{
    switch (std::clamp(skillLevel, 1, MaxSkillLevel))
    {
        case 1: return 60;
        case 2: return 68;
        case 3: return 76;
        case 4: return 84;
        case 5: return 90;
        default: return 60;
    }
}

int ClanBlacksmithSystem::friendDiscountPercentForSkill(int skillLevel)
{
    switch (std::clamp(skillLevel, 1, MaxSkillLevel))
    {
        case 1: return 0;
        case 2: return 0;
        case 3: return 4;
        case 4: return 7;
        case 5: return 10;
        default: return 0;
    }
}

std::string ClanBlacksmithSystem::specializationLabel(ClanBlacksmithSpecialization specialization)
{
    switch (specialization)
    {
        case ClanBlacksmithSpecialization::Weapons: return "Forge d'armes";
        case ClanBlacksmithSpecialization::Armor: return "Armures et protections";
        case ClanBlacksmithSpecialization::Maintenance: return "Entretien économique";
        case ClanBlacksmithSpecialization::None: break;
    }
    return "Aucune";
}

bool ClanBlacksmithSystem::canChooseSpecialization(const Player& player, std::string& reason)
{
    const ClanBlacksmithStatus smith = statusFor(player);
    if (!smith.hired)
    {
        reason = "Aucun forgeron n'est recruté au QG.";
        return false;
    }
    if (smith.skillLevel < 3)
    {
        reason = "Le forgeron doit atteindre le niveau d'artisan 3 avant de choisir une spécialisation.";
        return false;
    }
    if (smith.specialization != ClanBlacksmithSpecialization::None)
    {
        reason = smith.name + " est déjà spécialisé en " + smith.specializationLabel + ".";
        return false;
    }
    reason = "L'expérience du forgeron permet maintenant de choisir une spécialisation permanente.";
    return true;
}

bool ClanBlacksmithSystem::chooseSpecialization(Player& player, ClanBlacksmithSpecialization specialization, std::string& reason)
{
    if (specialization == ClanBlacksmithSpecialization::None)
    {
        reason = "Choisis une spécialisation réelle.";
        return false;
    }
    if (!canChooseSpecialization(player, reason)) return false;

    const ClanBlacksmithStatus smith = statusFor(player);
    const std::string id = specializationId(specialization);
    player.recordCanonicalEvent(
        "clan_hq_blacksmith_specializations",
        smith.name,
        "specialization=" + id + " | chosen_day=" + std::to_string(player.getWorldDaysElapsed()),
        1
    );
    player.recordCanonicalEvent(
        "clan_hq_blacksmith_milestones",
        smith.name,
        smith.name + " choisit la spécialisation : " + specializationLabel(specialization),
        1
    );
    reason = smith.name + " se spécialise en " + specializationLabel(specialization) + ". Ce choix est permanent pour cet artisan.";
    return true;
}

ClanBlacksmithStatus ClanBlacksmithSystem::statusFor(const Player& player)
{
    ClanBlacksmithStatus status;
    const ClanHeadquartersStatus hq = ClanHeadquartersSystem::statusFor(player);
    status.workshopAvailable = hq.built && hq.forgeWorkshopLevel > 0;
    if (!status.workshopAvailable) return status;

    const PlayerJournalRecord* hire = findRecord(player, "clan_hq_blacksmith_hires", "active");
    if (hire == nullptr || hire->count <= 0) return status;

    status.hired = true;
    status.name = stringField(hire->label, "name", "Forgeron du clan");
    status.experience = std::max(0, recordCount(player, "clan_hq_blacksmith_xp", status.name));
    status.jobsCompleted = std::max(0, recordCount(player, "clan_hq_blacksmith_jobs", status.name));
    status.skillLevel = skillLevelFromExperience(status.experience);
    status.repairCapPercent = repairCapPercentForSkill(status.skillLevel);
    status.friendDiscountPercent = friendDiscountPercentForSkill(status.skillLevel);
    if (const PlayerJournalRecord* specialization = findRecord(player, "clan_hq_blacksmith_specializations", status.name))
        status.specialization = specializationFromId(stringField(specialization->label, "specialization", "none"));
    status.specializationLabel = specializationLabel(status.specialization);
    status.specializationChoiceAvailable = status.skillLevel >= 3 && status.specialization == ClanBlacksmithSpecialization::None;
    status.nextLevelExperience = status.skillLevel >= MaxSkillLevel
        ? experienceRequiredForLevel(MaxSkillLevel)
        : experienceRequiredForLevel(status.skillLevel + 1);
    return status;
}

std::string ClanBlacksmithSystem::candidateNameFor(const Player& player)
{
    const ClanHeadquartersStatus hq = ClanHeadquartersSystem::statusFor(player);
    const std::string basis = player.getName() + "|" + hq.cityId + "|clan_blacksmith_candidate";
    Random random(stableSeed(basis));
    return RandomNameGenerator::generateAnyName(random);
}

bool ClanBlacksmithSystem::canHire(const Player& player, std::string& reason)
{
    const ClanHeadquartersStatus hq = ClanHeadquartersSystem::statusFor(player);
    const ClanBlacksmithStatus smith = statusFor(player);
    if (!hq.built || hq.forgeWorkshopLevel <= 0)
    {
        reason = "Construis d'abord un atelier de forge au QG.";
        return false;
    }
    if (smith.hired)
    {
        reason = smith.name + " est déjà le forgeron du clan.";
        return false;
    }
    if (player.getCurrentCityId() != hq.cityId)
    {
        reason = "Il faut être dans la ville du QG pour finaliser le recrutement d'un artisan.";
        return false;
    }
    if (player.getInventory().getTotalCopper() < HireCostCopper)
    {
        reason = "Fonds insuffisants pour l'acompte, les outils personnels et le contrat du forgeron.";
        return false;
    }
    reason = "Un artisan intéressé est disponible pour rejoindre le QG.";
    return true;
}

bool ClanBlacksmithSystem::hire(Player& player, std::string& reason)
{
    if (!canHire(player, reason)) return false;
    if (!player.getInventory().spendCopper(HireCostCopper))
    {
        reason = "Le paiement du contrat du forgeron a échoué.";
        return false;
    }

    const std::string name = candidateNameFor(player);
    std::ostringstream label;
    label << "name=" << name
          << " | role=forgeron_clan"
          << " | hired_day=" << player.getWorldDaysElapsed()
          << " | starting_skill=1";
    player.recordCanonicalEvent("clan_hq_blacksmith_hires", "active", label.str(), 1);
    player.recordCanonicalEvent("clan_hq_artisans_joined", name, name + " rejoint le QG comme forgeron du clan", 1);
    player.recordCanonicalEvent("clan_hq_blacksmith_contracts", name, "Acompte et installation personnelle du forgeron", static_cast<int>(HireCostCopper));
    reason = name + " rejoint le clan comme forgeron. Il commence modestement et progressera uniquement en réalisant de vraies commandes.";
    return true;
}

ClanBlacksmithRepairQuote ClanBlacksmithSystem::quoteWeaponRepair(const Player& player, int weaponIndex)
{
    if (!player.getInventory().hasWeapon(weaponIndex))
    {
        ClanBlacksmithRepairQuote quote;
        quote.inventoryIndex = weaponIndex;
        quote.reason = "Arme introuvable dans l'inventaire.";
        return quote;
    }
    const Weapon weapon = player.getInventory().getWeapon(weaponIndex);
    return baseQuote(
        player,
        false,
        weaponIndex,
        weapon.getName(),
        weapon.getDurability(),
        weapon.getMaxDurability(),
        weapon.isIndestructible(),
        weaponRepairMaterial(weapon)
    );
}

ClanBlacksmithRepairQuote ClanBlacksmithSystem::quoteArmorRepair(const Player& player, int armorIndex)
{
    if (!player.getInventory().hasArmor(armorIndex))
    {
        ClanBlacksmithRepairQuote quote;
        quote.armor = true;
        quote.inventoryIndex = armorIndex;
        quote.reason = "Armure introuvable dans l'inventaire.";
        return quote;
    }
    const Armor armor = player.getInventory().getArmor(armorIndex);
    return baseQuote(
        player,
        true,
        armorIndex,
        armor.getName(),
        armor.getDurability(),
        armor.getMaxDurability(),
        armor.isIndestructible(),
        armorRepairMaterial(armor)
    );
}

bool ClanBlacksmithSystem::repairWeapon(Player& player, int weaponIndex, ClanBlacksmithRepairQuote& result, std::string& reason)
{
    result = quoteWeaponRepair(player, weaponIndex);
    if (!result.valid)
    {
        reason = result.reason;
        return false;
    }

    Weapon* weapon = player.getInventory().getMutableWeapon(weaponIndex);
    if (weapon == nullptr)
    {
        reason = "L'arme a disparu de l'inventaire avant le début du travail.";
        return false;
    }
    if (!player.getInventory().spendCopper(result.costCopper))
    {
        reason = "Le paiement de la réparation a échoué.";
        return false;
    }
    if (!player.getInventory().removeMaterialQuantityById(result.materialId, result.materialQuantity))
    {
        player.getInventory().earnCopper(result.costCopper);
        reason = "Les matériaux ne sont plus disponibles. Le paiement a été rendu.";
        return false;
    }

    const int previousSkill = statusFor(player).skillLevel;
    weapon->repair(result.durabilityRestored);
    recordCompletedRepair(player, result, previousSkill);
    const ClanBlacksmithStatus after = statusFor(player);
    reason = after.name + " restaure " + std::to_string(result.durabilityRestored) + " points de durabilité sur " + result.itemName
        + ". Prix payé : " + std::to_string(result.costCopper) + " PC"
        + (result.friendDiscountPercent > 0 ? " après remise clan de " + std::to_string(result.friendDiscountPercent) + "%" : "") + "."
        + (after.skillLevel > previousSkill ? " Son expérience lui fait atteindre le niveau d'artisan " + std::to_string(after.skillLevel) + "." : "");
    return true;
}

bool ClanBlacksmithSystem::repairArmor(Player& player, int armorIndex, ClanBlacksmithRepairQuote& result, std::string& reason)
{
    result = quoteArmorRepair(player, armorIndex);
    if (!result.valid)
    {
        reason = result.reason;
        return false;
    }

    Armor* armor = player.getInventory().getMutableArmor(armorIndex);
    if (armor == nullptr)
    {
        reason = "L'armure a disparu de l'inventaire avant le début du travail.";
        return false;
    }
    if (!player.getInventory().spendCopper(result.costCopper))
    {
        reason = "Le paiement de la réparation a échoué.";
        return false;
    }
    if (!player.getInventory().removeMaterialQuantityById(result.materialId, result.materialQuantity))
    {
        player.getInventory().earnCopper(result.costCopper);
        reason = "Les matériaux ne sont plus disponibles. Le paiement a été rendu.";
        return false;
    }

    const int previousSkill = statusFor(player).skillLevel;
    armor->repair(result.durabilityRestored);
    recordCompletedRepair(player, result, previousSkill);
    const ClanBlacksmithStatus after = statusFor(player);
    reason = after.name + " restaure " + std::to_string(result.durabilityRestored) + " points de durabilité sur " + result.itemName
        + ". Prix payé : " + std::to_string(result.costCopper) + " PC"
        + (result.friendDiscountPercent > 0 ? " après remise clan de " + std::to_string(result.friendDiscountPercent) + "%" : "") + "."
        + (after.skillLevel > previousSkill ? " Son expérience lui fait atteindre le niveau d'artisan " + std::to_string(after.skillLevel) + "." : "");
    return true;
}
