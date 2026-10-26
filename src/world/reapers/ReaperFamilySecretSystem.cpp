#include "world/reapers/ReaperFamilySecretSystem.hpp"

#include <algorithm>
#include <cctype>

namespace
{
    std::string normalize(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return value;
    }
}

bool ReaperFamilySecretSystem::canRevealBuriedName(const ReaperFamilyRecognitionContext& context)
{
    return normalize(context.observerName) == "arone aster"
        && context.faceToFace
        && context.meaningfulExchange
        && context.personalRecognition;
}

std::string ReaperFamilySecretSystem::identityKnownTo(const ReaperFamilyRecognitionContext& context)
{
    return canRevealBuriedName(context) ? "Reaper-13 — reconnu personnellement comme son frère disparu" : "Reaper-13";
}

std::string ReaperFamilySecretSystem::buriedNameIfRecognized(const ReaperFamilyRecognitionContext& context)
{
    return canRevealBuriedName(context) ? "Caël Aster" : "";
}
