#include "core/VersionInfo.hpp"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

std::string VersionInfo::currentVersion()
{
    return "3.42.00";
}

std::string VersionInfo::recreateRecommendedBeforeVersion()
{
    return "3.00.00";
}

std::string VersionInfo::importantSaveUpdateVersion()
{
    return "3.00.00";
}

std::string VersionInfo::currentDateText()
{
    std::time_t now = std::time(nullptr);
    std::tm localTime{};

#if defined(_WIN32)
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    std::ostringstream output;
    output << std::setfill('0')
           << std::setw(2) << localTime.tm_mday << "/"
           << std::setw(2) << (localTime.tm_mon + 1) << "/"
           << (localTime.tm_year + 1900);
    return output.str();
}

VersionNumber VersionInfo::parse(const std::string& versionText)
{
    std::string clean = versionText;
    clean.erase(std::remove_if(clean.begin(), clean.end(), [](unsigned char c) {
        return c == 'v' || c == 'V' || c == ' ' || c == '\t' || c == '\r' || c == '\n';
    }), clean.end());

    std::vector<int> parts;
    std::string current;

    for (char c : clean)
    {
        if (c == '.')
        {
            if (current.empty())
            {
                return {0, 0, 0, false};
            }

            try
            {
                parts.push_back(std::stoi(current));
            }
            catch (...)
            {
                return {0, 0, 0, false};
            }

            current.clear();
            continue;
        }

        if (c < '0' || c > '9')
        {
            return {0, 0, 0, false};
        }

        current += c;
    }

    if (!current.empty())
    {
        try
        {
            parts.push_back(std::stoi(current));
        }
        catch (...)
        {
            return {0, 0, 0, false};
        }
    }

    if (parts.empty())
    {
        return {0, 0, 0, false};
    }

    while (parts.size() < 3)
    {
        parts.push_back(0);
    }

    return {parts[0], parts[1], parts[2], true};
}

int VersionInfo::compare(const std::string& left, const std::string& right)
{
    VersionNumber a = parse(left);
    VersionNumber b = parse(right);

    if (!a.known && !b.known) return 0;
    if (!a.known) return -1;
    if (!b.known) return 1;

    if (a.major != b.major) return a.major < b.major ? -1 : 1;
    if (a.minor != b.minor) return a.minor < b.minor ? -1 : 1;
    if (a.patch != b.patch) return a.patch < b.patch ? -1 : 1;
    return 0;
}

VersionCompatibilityImpact VersionInfo::evaluateCompatibility(const std::string& lastAdaptedVersion)
{
    VersionNumber saved = parse(lastAdaptedVersion);
    VersionNumber current = parse(currentVersion());

    if (!saved.known)
    {
        return VersionCompatibilityImpact::RecreateRecommended;
    }

    if (compare(lastAdaptedVersion, recreateRecommendedBeforeVersion()) < 0)
    {
        return VersionCompatibilityImpact::RecreateRecommended;
    }

    // EN: V3.00.00 is the accepted save baseline for the first playable story milestone.
    // Older saves receive the stronger recreation recommendation before this mid-update warning.
    // FR: La V3.00.00 devient le socle accepté pour le premier jalon histoire jouable.
    // Les sauvegardes plus anciennes reçoivent la recommandation forte avant cette alerte intermédiaire.
    if (compare(lastAdaptedVersion, importantSaveUpdateVersion()) < 0
        && compare(currentVersion(), importantSaveUpdateVersion()) >= 0)
    {
        return VersionCompatibilityImpact::MidUpdate;
    }

    if (saved.major != current.major)
    {
        // EN: Major jumps still receive a strong recreation recommendation.
        // FR: Les gros sauts de version gardent une recommandation forte de recréation.
        return VersionCompatibilityImpact::RecreateRecommended;
    }

    if (saved.minor != current.minor)
    {
        return VersionCompatibilityImpact::MidUpdate;
    }

    if (saved.patch != current.patch && compare(lastAdaptedVersion, currentVersion()) < 0)
    {
        return VersionCompatibilityImpact::PatchUpdate;
    }

    return VersionCompatibilityImpact::None;
}

std::string VersionInfo::compatibilityMessage(VersionCompatibilityImpact impact)
{
    switch (impact)
    {
        case VersionCompatibilityImpact::RecreateRecommended:
            return "Attention : il est conseillé de recréer un personnage pour cette version plus récente.";
        case VersionCompatibilityImpact::PatchUpdate:
            return "Petit patch détecté : quelques corrections ou ajouts mineurs ont été appliqués depuis la dernière adaptation de ce personnage.";
        case VersionCompatibilityImpact::MidUpdate:
            return "Mise à jour importante détectée : les règles de mort, l’économie, l’IG ou certaines données de registre ont changé depuis la dernière adaptation de ce personnage.";
        case VersionCompatibilityImpact::None:
        default:
            return "";
    }
}
