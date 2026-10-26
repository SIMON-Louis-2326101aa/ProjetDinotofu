#pragma once

#include "world/rumor/WorldRumorSystem.hpp"

#include <string>
#include <vector>

struct WorldRumorDefinition
{
    std::string id;
    std::string subjectId;
    std::string category;
    std::string text;
    std::string cityId;
    std::string biomeContains;
    int minLevel = 1;
    int baseCredibility = 35;
    WorldRumorSourceType sourceType = WorldRumorSourceType::Traveler;
};

namespace WorldRumorCatalog
{
    const std::vector<WorldRumorDefinition>& getCatalog();
    const WorldRumorDefinition* findById(const std::string& id);
    const WorldRumorDefinition* pickAmbient(
        const std::string& cityId,
        const std::string& biomeName,
        int playerLevel,
        int salt = 0
    );
}
