#include <cassert>
#include "world/reapers/ReaperFamilySecretSystem.hpp"

int main()
{
    ReaperFamilyRecognitionContext stranger{"Diane", true, true, true};
    assert(!ReaperFamilySecretSystem::canRevealBuriedName(stranger));
    assert(ReaperFamilySecretSystem::buriedNameIfRecognized(stranger).empty());

    ReaperFamilyRecognitionContext aroneTooSoon{"Arone Aster", true, false, true};
    assert(!ReaperFamilySecretSystem::canRevealBuriedName(aroneTooSoon));

    ReaperFamilyRecognitionContext arone{"Arone Aster", true, true, true};
    assert(ReaperFamilySecretSystem::canRevealBuriedName(arone));
    assert(ReaperFamilySecretSystem::buriedNameIfRecognized(arone) == "Caël Aster");
    return 0;
}
