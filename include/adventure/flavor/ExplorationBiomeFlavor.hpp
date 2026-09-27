#ifndef INCLUDE_ADVENTURE_FLAVOR_EXPLORATIONBIOMEFLAVOR_HPP
#define INCLUDE_ADVENTURE_FLAVOR_EXPLORATIONBIOMEFLAVOR_HPP

#include <string>

class ExplorationBiomeFlavor
{
public:
    static std::string miniBossName(const std::string& biomeName, bool evolved);
    static std::string miniBossQuestFamily(const std::string& biomeName, bool evolved);
    static std::string dangerousSiteName(const std::string& biomeName);
    static std::string dangerousSiteWarning(const std::string& biomeName);
    static std::string bossTrace(const std::string& biomeName);
    static std::string environmentalHazard(const std::string& biomeName);
    static std::string environmentalObservation(const std::string& biomeName);
};

#endif
