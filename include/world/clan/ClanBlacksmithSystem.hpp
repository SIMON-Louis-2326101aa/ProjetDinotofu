#ifndef INCLUDE_WORLD_CLAN_CLANBLACKSMITHSYSTEM_HPP
#define INCLUDE_WORLD_CLAN_CLANBLACKSMITHSYSTEM_HPP

#include <string>

enum class ClanBlacksmithSpecialization
{
    None,
    Weapons,
    Armor,
    Maintenance
};

class Player;

struct ClanBlacksmithStatus
{
    bool workshopAvailable = false;
    bool hired = false;
    std::string name;
    int skillLevel = 0;
    int experience = 0;
    int jobsCompleted = 0;
    int repairCapPercent = 0;
    int nextLevelExperience = 0;
    ClanBlacksmithSpecialization specialization = ClanBlacksmithSpecialization::None;
    std::string specializationLabel;
    int friendDiscountPercent = 0;
    bool specializationChoiceAvailable = false;
};

struct ClanBlacksmithRepairQuote
{
    bool valid = false;
    bool armor = false;
    int inventoryIndex = -1;
    std::string itemName;
    int currentDurability = 0;
    int maxDurability = 0;
    int targetDurability = 0;
    int durabilityRestored = 0;
    std::string materialId;
    std::string materialLabel;
    int materialQuantity = 0;
    long long baseCostCopper = 0;
    long long costCopper = 0;
    int friendDiscountPercent = 0;
    std::string reason;
};

class ClanBlacksmithSystem
{
public:
    static constexpr long long HireCostCopper = 3200;
    static constexpr int MaxSkillLevel = 5;

    static ClanBlacksmithStatus statusFor(const Player& player);
    static std::string candidateNameFor(const Player& player);
    static int experienceRequiredForLevel(int level);
    static int repairCapPercentForSkill(int skillLevel);
    static int friendDiscountPercentForSkill(int skillLevel);
    static std::string specializationLabel(ClanBlacksmithSpecialization specialization);
    static bool canChooseSpecialization(const Player& player, std::string& reason);
    static bool chooseSpecialization(Player& player, ClanBlacksmithSpecialization specialization, std::string& reason);

    static bool canHire(const Player& player, std::string& reason);
    static bool hire(Player& player, std::string& reason);

    static ClanBlacksmithRepairQuote quoteWeaponRepair(const Player& player, int weaponIndex);
    static ClanBlacksmithRepairQuote quoteArmorRepair(const Player& player, int armorIndex);
    static bool repairWeapon(Player& player, int weaponIndex, ClanBlacksmithRepairQuote& result, std::string& reason);
    static bool repairArmor(Player& player, int armorIndex, ClanBlacksmithRepairQuote& result, std::string& reason);
};

#endif
