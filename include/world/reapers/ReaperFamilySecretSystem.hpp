#pragma once

#include <string>

struct ReaperFamilyRecognitionContext
{
    std::string observerName;
    bool faceToFace = false;
    bool meaningfulExchange = false;
    bool personalRecognition = false;
};

namespace ReaperFamilySecretSystem
{
    bool canRevealBuriedName(const ReaperFamilyRecognitionContext& context);
    std::string identityKnownTo(const ReaperFamilyRecognitionContext& context);
    std::string buriedNameIfRecognized(const ReaperFamilyRecognitionContext& context);
}
