// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_ENTITY_PLAYER_HPP
#define INCLUDE_ENTITY_PLAYER_HPP

#include "entity/Entity.hpp"
#include "class_system/PlayerClass.hpp"
#include "item/Inventory.hpp"
#include "progression/DifficultyMode.hpp"
#include "character/CharacterRace.hpp"
#include "progression/DndAttributes.hpp"

class Player : public Entity
{
public:
    static const int MAX_LEVEL = 255;

private:
    int level;
    int experience;

    Inventory inventory;
    int equippedWeaponIndex;
    int equippedArmorIndex;
    CharacterRace race;
    DndAttributes attributes;
    int unspentAttributePoints;

    int combatsStarted;
    int victories;
    int defeats;
    int escapes;
    int deaths;
    int enemiesKilled;
    int bossesKilled;

    bool alteredByCheats;
    bool godModeEnabled;
    bool infiniteConsumablesEnabled;
    bool indestructibleEquipmentEnabled;
    bool equipmentProtectionEnabled;
    bool storySkipEnabled;

    bool godModeKnown;
    bool infiniteConsumablesKnown;
    bool indestructibleEquipmentKnown;
    bool equipmentProtectionKnown;
    bool storySkipKnown;
    bool creatorMessageKnown;

    int goldCheatUseCount;
    int levelCheatUseCount;
    int maxLevelCheatUseCount;
    int refundCheatUseCount;
    int resetCheatUseCount;
    int switchClassCheatUseCount;

    int refundUsesRemaining;

    int getEquippedArmorMaxHpBonus() const;
    void applyRaceStartingBonus(CharacterRace selectedRace);

public:
    Player();

    Player(
        const std::string& name,
        const PlayerClass& playerClass
    );

    int getLevel() const;
    int getExperience() const;
    int getUnspentAttributePoints() const;
    const DndAttributes& getAttributes() const;

    CharacterRace getRace() const;
    std::string getRaceText() const;
    void setRace(CharacterRace selectedRace);
    void applyFlatStatBonus(int maxHpBonus, int minDamageBonus, int maxDamageBonus, int criticalDamageBonus);
    void setLoadedProgress(int loadedLevel, int loadedExperience, int loadedHp);
    void setLoadedAttributes(const DndAttributes& loadedAttributes, int loadedUnspentPoints);
    bool spendAttributePoint(int attributeChoice);
    void displayAttributes() const;

    int getCombatsStarted() const;
    int getVictories() const;
    int getDefeats() const;
    int getEscapes() const;
    int getDeaths() const;
    int getEnemiesKilled() const;
    int getBossesKilled() const;

    void setLoadedStatistics(
        int loadedCombatsStarted,
        int loadedVictories,
        int loadedDefeats,
        int loadedEscapes,
        int loadedDeaths,
        int loadedEnemiesKilled,
        int loadedBossesKilled
    );

    void recordCombatStarted();
    void recordVictory();
    void recordDefeat();
    void recordEscape();
    void recordDeath();
    void recordEnemyKills(int amount);
    void recordBossKill();
    void displayCareerStatistics(DifficultyMode difficulty) const;

    bool isAlteredByCheats() const;
    bool isGodModeEnabled() const;
    bool hasInfiniteConsumables() const;
    bool hasIndestructibleEquipment() const;
    bool hasEquipmentProtection() const;
    bool hasStorySkip() const;
    int getRefundUsesRemaining() const;

    bool isGodModeKnown() const;
    bool isInfiniteConsumablesKnown() const;
    bool isIndestructibleEquipmentKnown() const;
    bool isEquipmentProtectionKnown() const;
    bool isStorySkipKnown() const;
    bool isCreatorMessageKnown() const;

    int getGoldCheatUseCount() const;
    int getLevelCheatUseCount() const;
    int getMaxLevelCheatUseCount() const;
    int getRefundCheatUseCount() const;
    int getResetCheatUseCount() const;
    int getSwitchClassCheatUseCount() const;

    void setCheatState(
        bool altered,
        bool godMode,
        bool infiniteConsumables,
        bool indestructibleEquipment,
        bool equipmentProtection,
        bool storySkip,
        int refundUses
    );

    void markAsAlteredByCheats();

    bool toggleGodMode();
    bool toggleInfiniteConsumables();
    bool toggleIndestructibleEquipment();
    bool toggleEquipmentProtection();
    bool toggleStorySkip();

    void enableGodMode();
    void enableInfiniteConsumables();
    void enableIndestructibleEquipment();
    void enableEquipmentProtection();
    void enableStorySkip();

    void markCreatorMessageSeen();
    void recordGoldCheatUse();
    void recordLevelCheatUse();
    void recordMaxLevelCheatUse();
    void recordRefundCheatUse();
    void recordResetCheatUse();
    void recordSwitchClassCheatUse();
    bool consumeRefundUse();
    void forceLevelToMaximum();
    void gainOneLevelByCheat();

    void takeDamage(int damage) override;

    Inventory& getInventory();
    const Inventory& getInventory() const;

    int getEquippedWeaponIndex() const;
    bool hasEquippedWeapon() const;
    Weapon getEquippedWeapon() const;
    bool equipWeapon(int index);
    void unequipWeapon();

    int getEquippedArmorIndex() const;
    bool hasEquippedArmor() const;
    Armor getEquippedArmor() const;
    bool equipArmor(int index);
    void unequipArmor();

    void initializeStarterInventory();
    void initializeStarterInventory(DifficultyMode difficulty);

    bool destroyEquippedWeapon();
    bool destroyEquippedArmor();

    void gainExperience(int amount);
    void loseExperience(int amount);
    void levelUp();

    int attack(Random& random, bool& dodged, bool& critical, int damageBonus = 0) override;

    void displayStats() const override;
    void displayInventory() const;
    void displaySimpleEquipment() const;
    void displayDetailedEquipment() const;
};

#endif
