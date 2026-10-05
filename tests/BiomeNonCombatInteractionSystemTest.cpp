#include "adventure/content/BiomeNonCombatInteractionSystem.hpp"
#include <cassert>
#include <vector>
#include <string>

int main()
{
    bool found = false;
    for (int day = 0; day < 12 && !found; ++day)
    {
        for (int unit = 0; unit < 5 && !found; ++unit)
        {
            const BiomeNonCombatInteraction interaction = BiomeNonCombatInteractionSystem::buildCurrentInteraction("Route commerciale", day, unit);
            if (!interaction.active) continue;
            found = true;
            assert(!interaction.id.empty());
            assert(interaction.choices.size() >= 2);
            const BiomeNonCombatInteractionResult result = BiomeNonCombatInteractionSystem::resolve(interaction, interaction.choices.front().id);
            assert(result.resolved);
            assert(!result.choiceLabel.empty());
            assert(result.explorationRollShift <= 0);
            assert(!result.notableForLongTermHistory); // petite aide routière : note locale, pas souvenir historique durable
            assert(BiomeNonCombatInteractionSystem::journalKey("Route commerciale", day, interaction.id).find("Route commerciale") != std::string::npos);
        }
    }
    assert(found);

    const std::vector<std::string> expandedBiomes = {
        "Plaine sauvage", "Bocage aux lanternes", "Désert d'argile rouge", "Quartier abandonné",
        "Mine sifflante", "Verger des lucioles de fer", "Bois de la Corruption",
        "Bosquet des Fées du Mana", "Sanctuaire kitsuné des Neuf Étincelles", "Archipel des îles flottantes"
    };
    for (const std::string& biome : expandedBiomes)
    {
        bool biomeFound = false;
        for (int day = 0; day < 16 && !biomeFound; ++day)
        {
            for (int unit = 0; unit < 6 && !biomeFound; ++unit)
            {
                const BiomeNonCombatInteraction interaction = BiomeNonCombatInteractionSystem::buildCurrentInteraction(biome, day, unit);
                if (!interaction.active) continue;
                biomeFound = true;
                assert(!interaction.title.empty());
                assert(interaction.choices.size() >= 3);
                assert(BiomeNonCombatInteractionSystem::resolve(interaction, interaction.choices[1].id).resolved);
            }
        }
        assert(biomeFound);
    }

    bool foundNotableArchive = false;
    for (int day = 0; day < 40 && !foundNotableArchive; ++day)
    {
        const BiomeNonCombatInteraction archive = BiomeNonCombatInteractionSystem::buildCurrentInteraction("Archives noyées", day, 0);
        if (!archive.active) continue;
        foundNotableArchive = true;
        const BiomeNonCombatInteractionResult notable = BiomeNonCombatInteractionSystem::resolve(archive, 1);
        assert(notable.resolved);
        assert(notable.notableForLongTermHistory);
        const BiomeNonCombatInteractionResult passive = BiomeNonCombatInteractionSystem::resolve(archive, 3);
        assert(passive.resolved);
        assert(!passive.notableForLongTermHistory);
    }
    assert(foundNotableArchive);

    const BiomeNonCombatInteraction absent = BiomeNonCombatInteractionSystem::buildCurrentInteraction("Biome inexistant", 1, 0);
    assert(!absent.active);
    for (const std::string biomeName : {
        "Mares gélatineuses",
        "Montagne froide",
        "Ruines effondrées",
        "Canaux de brume bleue",
        "Foire abandonnée",
        "Carrière des os blancs"
    })
    {
        bool found = false;
        for (int day = 0; day < 40 && !found; ++day)
        {
            const BiomeNonCombatInteraction interaction = BiomeNonCombatInteractionSystem::buildCurrentInteraction(biomeName, day, 0);
            if (!interaction.active) continue;
            found = true;
            assert(!interaction.id.empty());
            assert(interaction.choices.size() == 3);
            assert(BiomeNonCombatInteractionSystem::resolve(interaction, interaction.choices.front().id).resolved);
        }
        assert(found);
    }

    bool foundGardenInteraction = false;
    for (int day = 0; day < 40 && !foundGardenInteraction; ++day)
    {
        const BiomeNonCombatInteraction garden = BiomeNonCombatInteractionSystem::buildCurrentInteraction(
            "Jardin des statues qui pleurent", day, 0);
        if (!garden.active) continue;
        foundGardenInteraction = true;
        assert(garden.id == "bouquet_devant_ange");
        assert(garden.choices.size() == 3);
        const BiomeNonCombatInteractionResult gardenResult = BiomeNonCombatInteractionSystem::resolve(garden, 1);
        assert(gardenResult.resolved);
        assert(gardenResult.questProgress >= 1);
    }
    assert(foundGardenInteraction);

    return 0;
}
