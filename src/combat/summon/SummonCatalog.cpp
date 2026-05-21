// EN: SummonCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SummonCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Creates predefined summons for future summoner, necromancer, pactist and tamer classes.

#include "combat/summon/SummonCatalog.hpp"

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

std::vector<Summon> SummonCatalog::createStarterSummonsForClass(
    const std::string& ownerName,
    const std::string& className
)
{
    if (className == "Invocateur")
    {
        return {createMinorSpirit(ownerName), createArcaneBeast(ownerName)};
    }

    if (className == "Nécromancien")
    {
        return {createBoneServant(ownerName)};
    }

    if (className == "Pactisant")
    {
        return {createMinorSpirit(ownerName)};
    }

    if (className == "Dompteur")
    {
        return {createArcaneBeast(ownerName)};
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
