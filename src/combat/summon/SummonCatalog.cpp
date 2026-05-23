// EN: SummonCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SummonCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Creates predefined summons for future summoner, necromancer, pactist and tamer classes.

#include "combat/summon/SummonCatalog.hpp"

#include <algorithm>
#include <cctype>

// EN: createMinorSpirit declares or implements a focused behavior used by this module.
// FR: createMinorSpirit déclare ou implémente un comportement précis utilisé par ce module.
Summon SummonCatalog::createMinorSpirit(const std::string& ownerName)
{
    return Summon("Esprit mineur", ownerName, 35, 3, 7, 3, 1, 1, true);
}

// EN: createBoneServant declares or implements a focused behavior used by this module.
// FR: createBoneServant déclare ou implémente un comportement précis utilisé par ce module.
Summon SummonCatalog::createBoneServant(const std::string& ownerName)
{
    return Summon("Serviteur osseux", ownerName, 45, 4, 8, 4, 1, 1, true);
}

// EN: createFoxFlame declares or implements a focused behavior used by this module.
// FR: createFoxFlame déclare ou implémente un comportement précis utilisé par ce module.
Summon SummonCatalog::createFoxFlame(const std::string& ownerName)
{
    return Summon("Flamme kitsune", ownerName, 30, 6, 11, 3, 1, 1, true);
}

// EN: createArcaneBeast declares or implements a focused behavior used by this module.
// FR: createArcaneBeast déclare ou implémente un comportement précis utilisé par ce module.
Summon SummonCatalog::createArcaneBeast(const std::string& ownerName)
{
    return Summon("Bête arcanique", ownerName, 55, 5, 10, 4, 2, 2, true);
}

// EN: createUnstableExperiment declares or implements a focused behavior used by this module.
// FR: createUnstableExperiment déclare ou implémente un comportement précis utilisé par ce module.
Summon SummonCatalog::createUnstableExperiment(const std::string& ownerName)
{
    return Summon("Expérience instable", ownerName, 28, 8, 14, 2, 1, 2, true);
}

// EN: createHazakShadow declares or implements a focused behavior used by this module.
// FR: createHazakShadow déclare ou implémente un comportement précis utilisé par ce module.
Summon SummonCatalog::createHazakShadow(const std::string& ownerName)
{
    return Summon("Ombre récente", ownerName, 42, 7, 13, 3, 1, 1, true);
}

// EN: createZodiacWisp declares or implements a focused behavior used by this module.
// FR: createZodiacWisp déclare ou implémente un comportement précis utilisé par ce module.
Summon SummonCatalog::createZodiacWisp(const std::string& ownerName)
{
    return Summon("Éclat zodiacal", ownerName, 26, 5, 16, 2, 1, 2, true);
}

Summon SummonCatalog::createGuardianTotem(const std::string& ownerName)
{
    return Summon("Totem gardien", ownerName, 70, 2, 5, 2, 2, 1, false);
}

Summon SummonCatalog::createRavenFamiliar(const std::string& ownerName)
{
    return Summon("Corbeau familier", ownerName, 24, 4, 9, 2, 1, 1, true);
}

Summon SummonCatalog::createMirrorShard(const std::string& ownerName)
{
    return Summon("Éclat de miroir", ownerName, 32, 3, 12, 2, 1, 2, true);
}

std::vector<Summon> SummonCatalog::createStarterSummonsForClass(
    const std::string& ownerName,
    const std::string& className
)
{
    std::string normalized = className;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (normalized.find("invocateur") != std::string::npos)
    {
        return {createMinorSpirit(ownerName), createArcaneBeast(ownerName), createGuardianTotem(ownerName)};
    }

    if (normalized.find("nécromancien") != std::string::npos || normalized.find("necromancien") != std::string::npos)
    {
        return {createBoneServant(ownerName), createRavenFamiliar(ownerName)};
    }

    if (normalized.find("pactisant") != std::string::npos)
    {
        return {createMinorSpirit(ownerName), createMirrorShard(ownerName)};
    }

    if (normalized.find("dompteur") != std::string::npos)
    {
        return {createArcaneBeast(ownerName), createRavenFamiliar(ownerName)};
    }

    if (ownerName == "Aoi")
    {
        return {createFoxFlame(ownerName)};
    }

    if (ownerName == "Hazak")
    {
        return {createHazakShadow(ownerName)};
    }

    if (ownerName == "Fail")
    {
        return {createUnstableExperiment(ownerName)};
    }

    if (ownerName == "Kanadé" || ownerName == "Kanade")
    {
        return {createZodiacWisp(ownerName)};
    }

    return {};
}
