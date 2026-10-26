// EN: TitleProgressSystem centralizes title unlocks driven by persistent gameplay events.
// FR: TitleProgressSystem centralise les déblocages de titres pilotés par les événements persistants du gameplay.
#ifndef INCLUDE_PROGRESSION_TITLEPROGRESSSYSTEM_HPP
#define INCLUDE_PROGRESSION_TITLEPROGRESSSYSTEM_HPP

#include <string>

class Player;

namespace TitleProgressSystem
{
    void onCanonicalEvent(
        Player& player,
        const std::string& category,
        const std::string& key,
        const std::string& label
    );
}

#endif
