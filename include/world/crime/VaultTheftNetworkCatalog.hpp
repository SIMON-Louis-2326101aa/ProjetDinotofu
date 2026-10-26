#ifndef INCLUDE_WORLD_CRIME_VAULTTHEFTNETWORKCATALOG_HPP
#define INCLUDE_WORLD_CRIME_VAULTTHEFTNETWORKCATALOG_HPP

#include <string>
#include <vector>

struct VaultTheftNetworkContact
{
    std::string id;
    std::string cityId;
    std::string roleType; // receiver / accomplice / cache
    std::string displayName;
    std::string locationId;
    std::string locationLabel;
    std::string explorationBiome;
    std::string description;
    // V3.51.65: loose inter-city affiliation. It does NOT mean members know everything
    // about each other; it only gives known contacts a plausible channel for warnings.
    std::string networkId;
};

class VaultTheftNetworkCatalog
{
public:
    static const std::vector<VaultTheftNetworkContact>& getCatalog();
    static const VaultTheftNetworkContact* findById(const std::string& id);
    static const VaultTheftNetworkContact* pickFor(const std::string& cityId, const std::string& roleType, int salt = 0);
    static std::vector<const VaultTheftNetworkContact*> relatedContacts(const std::string& contactId, bool includeSameCity = true);
};

#endif
