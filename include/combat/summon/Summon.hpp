// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Stores basic summoned combat entity data for future 1v1-to-PvE transitions.

#ifndef INCLUDE_COMBAT_SUMMON_SUMMON_HPP
#define INCLUDE_COMBAT_SUMMON_SUMMON_HPP

#include <string>

class Summon
{
private:
    std::string name;
    std::string ownerName;
    int hp;
    int maxHp;
    int minDamage;
    int maxDamage;
    int durationTurns;

public:
    Summon();

    Summon(
        const std::string& name,
        const std::string& ownerName,
        int maxHp,
        int minDamage,
        int maxDamage,
        int durationTurns
    );

    std::string getName() const;
    std::string getOwnerName() const;
    int getHp() const;
    int getMaxHp() const;
    int getMinDamage() const;
    int getMaxDamage() const;
    int getDurationTurns() const;

    bool isDead() const;
    bool isExpired() const;

    void takeDamage(int damage);
    void decreaseDuration();
};

#endif
