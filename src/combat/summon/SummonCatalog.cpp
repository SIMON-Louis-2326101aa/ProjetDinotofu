// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Creates predefined summons for future summoner, necromancer, pactist and tamer classes.

#include "combat/summon/SummonCatalog.hpp"

Summon SummonCatalog::createMinorSpirit(const std::string& ownerName)
{
    return Summon("Esprit mineur", ownerName, 35, 3, 7, 3);
}

Summon SummonCatalog::createBoneServant(const std::string& ownerName)
{
    return Summon("Serviteur osseux", ownerName, 45, 4, 8, 4);
}

Summon SummonCatalog::createFoxFlame(const std::string& ownerName)
{
    return Summon("Flamme kitsune", ownerName, 30, 6, 11, 3);
}

Summon SummonCatalog::createArcaneBeast(const std::string& ownerName)
{
    return Summon("Bête arcanique", ownerName, 55, 5, 10, 4);
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

    return {};
}
