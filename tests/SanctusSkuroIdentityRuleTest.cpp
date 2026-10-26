#include <cassert>
#include "character/SpecialCharacterTransformationRules.hpp"

int main()
{
    assert(SpecialCharacterTransformationRules::canTransformIntoSkuro("Sanctus"));
    assert(!SpecialCharacterTransformationRules::canAppearTogether("Sanctus", "Skuro"));
    assert(!SpecialCharacterTransformationRules::canAppearTogether("skuro", "sanctus"));
    assert(SpecialCharacterTransformationRules::canAppearTogether("Hazak", "Skuro"));
    return 0;
}
