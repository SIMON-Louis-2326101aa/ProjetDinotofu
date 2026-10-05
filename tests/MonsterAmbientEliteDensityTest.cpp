#include "core/Random.hpp"
#include "entity/MonsterCatalog.hpp"

#include <cassert>

namespace
{
    int countAmbientElitesAtLevel(int level, int sampleCount, Random& random)
    {
        int elites = 0;
        for (int i = 0; i < sampleCount; ++i)
        {
            if (MonsterCatalog::createRandomMonsterForLevel(level, random).isElite())
            {
                ++elites;
            }
        }
        return elites;
    }
}

int main()
{
    Random random;
    constexpr int sampleCount = 4000;

    // Early game must never be flooded by elites.
    assert(countAmbientElitesAtLevel(1, sampleCount, random) == 0);
    assert(countAmbientElitesAtLevel(2, sampleCount, random) == 0);

    // Levels 3-4 use a 5% ambient elite budget. 10% is a deliberately loose
    // non-flaky ceiling that still catches a return to unrestricted catalogue draws.
    assert(countAmbientElitesAtLevel(3, sampleCount, random) <= 400);
    assert(countAmbientElitesAtLevel(4, sampleCount, random) <= 400);

    return 0;
}
