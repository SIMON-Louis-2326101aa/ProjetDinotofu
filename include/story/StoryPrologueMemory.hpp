// EN: StoryPrologueMemory.hpp isolates the playable high-level memory that precedes the white-fog awakening.
// FR: StoryPrologueMemory.hpp isole le souvenir jouable de haut niveau qui précède le réveil dans la brume blanche.
#ifndef INCLUDE_STORY_STORYPROLOGUEMEMORY_HPP
#define INCLUDE_STORY_STORYPROLOGUEMEMORY_HPP

#include "core/Random.hpp"
#include "entity/Monster.hpp"
#include "entity/Player.hpp"
#include "progression/DifficultyMode.hpp"

#include <string>
#include <vector>

enum class StoryPrologueOutcome
{
    Victory,
    Retreat,
    Defeat
};

struct StoryPrologueCombatResult
{
    StoryPrologueOutcome outcome = StoryPrologueOutcome::Defeat;
    int turns = 0;
    int defeatedEnemies = 0;
    int remainingHp = 0;
};

class StoryPrologueMemory
{
public:
    static constexpr int MEMORY_LEVEL = 42;

    static Player createTemporaryPlayer(const Player& source, DifficultyMode difficulty);
    static std::vector<Monster> createPackHunt(DifficultyMode difficulty = DifficultyMode::Normal);
    static std::string obscuredCompanionLabel(bool firstCompanion, int phase);

    static std::vector<std::string> buildMissionFragmentLines(const Player& memoryPlayer);
    static std::vector<std::string> buildTravelLines(const Player& memoryPlayer, int approachChoice);
    static std::vector<std::string> buildFogTransitionLines(
        const Player& sourcePlayer,
        const StoryPrologueCombatResult& result
    );

    static StoryPrologueCombatResult runPackHunt(
        Player& memoryPlayer,
        Random& random,
        DifficultyMode difficulty,
        int approachChoice
    );
};

#endif
