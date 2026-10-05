#ifndef INCLUDE_SAVE_SAVESCHEMAVERSION_HPP
#define INCLUDE_SAVE_SAVESCHEMAVERSION_HPP

namespace SaveSchemaVersion
{
    // Internal JSON save schema. Increment only when the persisted structure changes in a way
    // that migration code/tests must explicitly recognize. This is independent from the game
    // version and from the mandatory "important save update" checkpoint.
    inline constexpr int Current = 26;
}

#endif
