#pragma once

#include <string>
#include <vector>

struct BountyIdentityDefinition
{
    std::string id;
    std::string name;
    std::string temperament;
    std::string homeCityId;
    std::string networkId;

    std::string issuerId;
    std::string issuerName;
    std::string issuerOrganizationId;
    std::string issuerOrganizationName;
    bool issuerPrefersLiveCapture = true;

    std::string closeRelationId;
    std::string closeRelationName;
    std::string closeRelationType;

    int minLevel = 1;
    int awarenessBonus = 0;
    int resourcesBonus = 0;
    int supportBonus = 0;
    int mobilityBonus = 0;
};

namespace BountyIdentityCatalog
{
    const std::vector<BountyIdentityDefinition>& getCatalog();
    const BountyIdentityDefinition* findById(const std::string& id);
    const BountyIdentityDefinition* findByName(const std::string& name);
    const BountyIdentityDefinition* pickFor(
        const std::string& seed,
        int playerLevel,
        const std::string& preferredCityId = ""
    );
}
