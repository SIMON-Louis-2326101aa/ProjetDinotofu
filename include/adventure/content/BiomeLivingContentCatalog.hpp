#ifndef INCLUDE_ADVENTURE_CONTENT_BIOMELIVINGCONTENTCATALOG_HPP
#define INCLUDE_ADVENTURE_CONTENT_BIOMELIVINGCONTENTCATALOG_HPP

#include <string>
#include <vector>

struct BiomeLivingContentProfile
{
    std::string visualIdentity;
    std::string environmentalHazard;
    std::string resourceSign;
    std::string neutralLife;
    std::string socialTrace;
    std::string unusualSign;
};

class BiomeLivingContentCatalog
{
public:
    static bool hasProfile(const std::string& biomeName);
    static BiomeLivingContentProfile forBiome(const std::string& biomeName);
    static std::vector<std::string> buildCurrentObservationLines(const std::string& biomeName, int worldDay);
};

#endif
