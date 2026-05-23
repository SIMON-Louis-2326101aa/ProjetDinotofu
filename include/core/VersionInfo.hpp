#ifndef INCLUDE_CORE_VERSIONINFO_HPP
#define INCLUDE_CORE_VERSIONINFO_HPP

#include <string>

struct VersionNumber
{
    int major;
    int minor;
    int patch;
    bool known;
};

enum class VersionCompatibilityImpact
{
    None,
    MidUpdate,
    RecreateRecommended
};

class VersionInfo
{
public:
    static std::string currentVersion();
    static std::string recreateRecommendedBeforeVersion();
    static std::string currentDateText();

    static VersionNumber parse(const std::string& versionText);
    static int compare(const std::string& left, const std::string& right);
    static VersionCompatibilityImpact evaluateCompatibility(const std::string& lastAdaptedVersion);
    static std::string compatibilityMessage(VersionCompatibilityImpact impact);
};

#endif
