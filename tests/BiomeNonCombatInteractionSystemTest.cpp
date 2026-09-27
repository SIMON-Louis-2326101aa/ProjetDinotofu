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

    const BiomeNonCombatInteraction absent = BiomeNonCombatInteractionSystem::buildCurrentInteraction("Biome inexistant", 1, 0);
    assert(!absent.active);
    return 0;
}
