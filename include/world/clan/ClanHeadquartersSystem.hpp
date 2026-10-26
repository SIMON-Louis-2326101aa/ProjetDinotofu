#ifndef INCLUDE_WORLD_CLAN_CLANHEADQUARTERSSYSTEM_HPP
#define INCLUDE_WORLD_CLAN_CLANHEADQUARTERSSYSTEM_HPP

#include <string>
#include <vector>

class Player;

struct ClanHeadquartersMaterialRequirement
{
    std::string materialId;
    std::string label;
    int quantity = 0;
};

enum class ClanHeadquartersUpgradeKind
{
    None,
    BuildingExpansion,
    ExtraBed,
    QuestBoard,
    MissionChest,
    CommonRoomBar,
    ComfortDecor,
    ForgeWorkshop
};

struct ClanHeadquartersStatus
{
    bool constructionStarted = false;
    bool built = false;
    std::string cityId;
    int constructionStartedDay = -1;
    int constructionReadyDay = -1;
    int buildingLevel = 0;
    int bedCapacity = 0;
    int maxBedCapacity = 0;
    int moduleCapacity = 0;
    int questBoardLevel = 0;
    int missionChestLevel = 0;
    int commonRoomBarLevel = 0;
    int comfortDecorLevel = 0;
    int forgeWorkshopLevel = 0;
    long long missionChestStoredCopper = 0;
    long long missionChestCapacityCopper = 0;
    int modulesUsed = 0;
    bool counterAvailable = false;
};

struct ClanHeadquartersUpgradeStatus
{
    bool active = false;
    ClanHeadquartersUpgradeKind kind = ClanHeadquartersUpgradeKind::None;
    int targetLevel = 0;
    int startedDay = -1;
    int readyDay = -1;
};

struct ClanHeadquartersCelebrationReport
{
    long long paidCopper = 0;
    int participantCount = 0;
    int moraleChargesGranted = 0;
    int clanReputationGranted = 0;
    std::vector<std::string> participantNames;
    std::vector<std::string> lines;
};

struct ClanHeadquartersPassiveReport
{
    int elapsedDays = 0;
    int missionCycles = 0;
    int missionsCompleted = 0;
    int totalRecruitXp = 0;
    long long generatedMissionCopper = 0;
    long long storedMissionCopper = 0;
    long long discardedMissionCopper = 0;
    std::vector<std::string> lines;
};

class ClanHeadquartersSystem
{
public:
    static constexpr long long BaseConstructionCostCopper = 16000;
    static constexpr int BaseConstructionDays = 4;
    static constexpr int BaseBedCapacity = 2;
    static constexpr int BaseModuleCapacity = 3;
    static constexpr int MaxBuildingLevel = 5;

    // First real progression tier. Values are deliberately meaningful money/material sinks.
    static constexpr long long BuildingLevel2CostCopper = 26000;
    static constexpr int BuildingLevel2Days = 7;
    static constexpr int BuildingLevel2MaxBeds = 5;
    static constexpr int BuildingLevel2ModuleCapacity = 6;

    // Second structural phase. Higher phases (4/5) remain design targets until their own
    // module-level prerequisites exist; the global cap is already fixed at level 5.
    static constexpr long long BuildingLevel3CostCopper = 48000;
    static constexpr int BuildingLevel3Days = 10;
    static constexpr int BuildingLevel3MaxBeds = 8;
    static constexpr int BuildingLevel3ModuleCapacity = 9;

    static constexpr long long ExtraBedCostCopper = 4500;
    static constexpr int ExtraBedDays = 2;

    static constexpr long long QuestBoardLevel2CostCopper = 12000;
    static constexpr int QuestBoardLevel2Days = 3;

    static constexpr long long MissionChestLevel1CostCopper = 9000;
    static constexpr int MissionChestLevel1Days = 2;
    static constexpr long long MissionChestLevel1CapacityCopper = 2500;

    static constexpr long long CommonRoomBarLevel1CostCopper = 10500;
    static constexpr int CommonRoomBarLevel1Days = 3;
    static constexpr long long CommonRoomBarVisitCostCopper = 35;

    static constexpr long long ComfortDecorLevel1CostCopper = 7500;
    static constexpr int ComfortDecorLevel1Days = 2;
    static constexpr long long ForgeWorkshopLevel1CostCopper = 18000;
    static constexpr int ForgeWorkshopLevel1Days = 4;
    static constexpr long long CelebrationBaseCostCopper = 900;
    static constexpr long long CelebrationBarDiscountCopper = 150;
    static constexpr int CelebrationCooldownDays = 14;

    static ClanHeadquartersStatus statusFor(const Player& player);
    static ClanHeadquartersUpgradeStatus upgradeStatusFor(const Player& player);

    static std::vector<ClanHeadquartersMaterialRequirement> baseConstructionMaterials();
    static std::vector<ClanHeadquartersMaterialRequirement> upgradeMaterials(ClanHeadquartersUpgradeKind kind);
    static std::vector<ClanHeadquartersMaterialRequirement> upgradeMaterials(const Player& player, ClanHeadquartersUpgradeKind kind);
    static long long upgradeCostCopper(ClanHeadquartersUpgradeKind kind);
    static long long upgradeCostCopper(const Player& player, ClanHeadquartersUpgradeKind kind);
    static int upgradeDurationDays(ClanHeadquartersUpgradeKind kind);
    static int upgradeDurationDays(const Player& player, ClanHeadquartersUpgradeKind kind);
    static std::string upgradeDisplayName(ClanHeadquartersUpgradeKind kind);

    static bool canStartConstruction(const Player& player, std::string& reason);
    static bool startConstruction(Player& player, std::string& reason);
    static bool completeConstructionIfReady(Player& player);

    static bool canStartUpgrade(const Player& player, ClanHeadquartersUpgradeKind kind, std::string& reason);
    static bool startUpgrade(Player& player, ClanHeadquartersUpgradeKind kind, std::string& reason);
    static bool completeUpgradeIfReady(Player& player, std::string* completionMessage = nullptr);

    static std::vector<std::string> eligibleRecruitNames(const Player& player);
    static std::vector<std::string> residentRecruitNames(const Player& player);
    static bool isResidentRecruit(const Player& player, const std::string& recruitName);
    static bool assignRecruit(Player& player, const std::string& recruitName, std::string& reason);
    static bool removeRecruit(Player& player, const std::string& recruitName, std::string& reason);

    static bool collectMissionChest(Player& player, long long& collectedCopper, std::string& reason);
    static bool useCommonRoomBar(Player& player, int& healedHp, long long& paidCopper, std::string& reason);
    static long long celebrationCostCopper(const Player& player);
    static bool canOrganizeCelebration(const Player& player, std::string& reason);
    static bool organizeCelebration(Player& player, ClanHeadquartersCelebrationReport& report, std::string& reason);

    // Slow off-party progression. This deliberately grants much less XP than adventuring with the player.
    static ClanHeadquartersPassiveReport processPassiveActivity(Player& player);
};

#endif
