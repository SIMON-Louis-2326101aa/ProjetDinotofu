// EN: SummonCatalog.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: SummonCatalog.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
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
    // EN: createMinorSpirit declares or implements a focused behavior used by this module.
    // FR: createMinorSpirit déclare ou implémente un comportement précis utilisé par ce module.
    static Summon createMinorSpirit(const std::string& ownerName);
    // EN: createBoneServant declares or implements a focused behavior used by this module.
    // FR: createBoneServant déclare ou implémente un comportement précis utilisé par ce module.
    static Summon createBoneServant(const std::string& ownerName);
    // EN: createFoxFlame declares or implements a focused behavior used by this module.
    // FR: createFoxFlame déclare ou implémente un comportement précis utilisé par ce module.
    static Summon createFoxFlame(const std::string& ownerName);
    // EN: createArcaneBeast declares or implements a focused behavior used by this module.
    // FR: createArcaneBeast déclare ou implémente un comportement précis utilisé par ce module.
    static Summon createArcaneBeast(const std::string& ownerName);
    // EN: createUnstableExperiment declares or implements a focused behavior used by this module.
    // FR: createUnstableExperiment déclare ou implémente un comportement précis utilisé par ce module.
    static Summon createUnstableExperiment(const std::string& ownerName);
    // EN: createHazakShadow declares or implements a focused behavior used by this module.
    // FR: createHazakShadow déclare ou implémente un comportement précis utilisé par ce module.
    static Summon createHazakShadow(const std::string& ownerName);
    // EN: createZodiacWisp declares or implements a focused behavior used by this module.
    // FR: createZodiacWisp déclare ou implémente un comportement précis utilisé par ce module.
    static Summon createZodiacWisp(const std::string& ownerName);

    static std::vector<Summon> createStarterSummonsForClass(
        const std::string& ownerName,
        const std::string& className
    );
};

#endif
