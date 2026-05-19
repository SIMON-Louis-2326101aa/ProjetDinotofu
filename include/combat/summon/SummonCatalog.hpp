// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Creates predefined summons for future summoner, necromancer, pactist and tamer classes.

#ifndef INCLUDE_COMBAT_SUMMON_SUMMONCATALOG_HPP
#define INCLUDE_COMBAT_SUMMON_SUMMONCATALOG_HPP

#include "combat/summon/Summon.hpp"

#include <string>
#include <vector>

class SummonCatalog
{
public:
    static Summon createMinorSpirit(const std::string& ownerName);
    static Summon createBoneServant(const std::string& ownerName);
    static Summon createFoxFlame(const std::string& ownerName);
    static Summon createArcaneBeast(const std::string& ownerName);

    static std::vector<Summon> createStarterSummonsForClass(
        const std::string& ownerName,
        const std::string& className
    );
};

#endif
