#include "world/bounty/BountyIdentityCatalog.hpp"

#include <algorithm>
#include <functional>

const std::vector<BountyIdentityDefinition>& BountyIdentityCatalog::getCatalog()
{
    // These identities are persistent bounty actors, not anonymous rerolls. Their relatives and
    // issuers can remember an outcome later. Network membership only creates plausible channels;
    // it never reveals unknown criminal contacts to the player.
    static const std::vector<BountyIdentityDefinition> catalog = {
        {"marek_sans_lanterne", "Marek Sans-Lanterne", "prudent", "valebrume", "marche_gris",
            "orren", "Orren", "poste_orren", "Poste d'Orren", true,
            "alis_sans_lanterne", "Alis Sans-Lanterne", "sœur", 1, 6, 4, 8, 2},
        {"darel_coupe_corde", "Darel Coupe-Corde", "impulsif", "valebrume", "courriers_nocturnes",
            "mira", "Mira", "intendance_valebrume", "Intendance de Mira", true,
            "lenn_coupe_corde", "Lenn Coupe-Corde", "frère", 4, -2, 5, 3, 8},
        {"sira_la_grise", "Sira la Grise", "méfiante méthodique", "valebrume", "marche_gris",
            "soryn", "Soryn", "archives_soryn", "Archives de Soryn", true,
            "tevan_gris", "Tevan Gris", "ancien associé", 8, 8, 7, 10, 1},

        {"brann_pierre_courte", "Brann Pierre-Courte", "survivant discret", "rocheveille", "marche_gris",
            "guilde_rocheveille", "Maître de guilde de Rocheveille", "guilde_rocheveille", "Guilde locale de Rocheveille", true,
            "enna_pierre_courte", "Enna Pierre-Courte", "cousine", 10, 5, 12, 4, -2},
        {"kael_main_froide", "Kael Main-Froide", "calculateur", "rocheveille", "courriers_nocturnes",
            "contremaitre_mines", "Contremaître des mines", "mines_rocheveille", "Intendance des mines de Rocheveille", false,
            "senn_main_froide", "Senn Main-Froide", "ancien partenaire", 14, 8, 10, 5, 7},

        {"ilyne_des_cales", "Ilyne des Cales", "rusée opportuniste", "port_lanterne", "courriers_nocturnes",
            "capitainerie_port_lanterne", "Officier de la capitainerie", "capitainerie_port_lanterne", "Capitainerie de Port-Lanterne", true,
            "maero_des_cales", "Maero des Cales", "compagnon de route", 10, 4, 8, 12, 9},
        {"solen_sans_route", "Solen Sans-Route", "survivant discret", "port_lanterne", "marche_gris",
            "guilde_port_lanterne", "Maître de guilde de Port-Lanterne", "guilde_port_lanterne", "Guilde locale de Port-Lanterne", true,
            "tess_sans_route", "Tess Sans-Route", "sœur", 16, 10, 6, 14, 6},

        {"neris_le_fuyard", "Neris le Fuyard", "rusé opportuniste", "lys_de_givre", "courriers_nocturnes",
            "guilde_lys_givre", "Maître de guilde de Lys-de-Givre", "guilde_lys_givre", "Guilde locale de Lys-de-Givre", true,
            "elya_neris", "Elya Neris", "compagne", 18, 11, 5, 8, 14},
        {"orin_cloche_fendue", "Orin Cloche-Fendue", "méfiant méthodique", "lys_de_givre", "marche_gris",
            "gardes_lys_givre", "Prévôt du nord", "gardes_lys_givre", "Garde du nord de Lys-de-Givre", false,
            "maelis_cloche_fendue", "Maelis Cloche-Fendue", "fille", 22, 9, 13, 6, 4}
    };
    return catalog;
}

const BountyIdentityDefinition* BountyIdentityCatalog::findById(const std::string& id)
{
    const auto& catalog = getCatalog();
    const auto it = std::find_if(catalog.begin(), catalog.end(), [&](const BountyIdentityDefinition& value) {
        return value.id == id;
    });
    return it == catalog.end() ? nullptr : &(*it);
}

const BountyIdentityDefinition* BountyIdentityCatalog::findByName(const std::string& name)
{
    const auto& catalog = getCatalog();
    const auto it = std::find_if(catalog.begin(), catalog.end(), [&](const BountyIdentityDefinition& value) {
        return value.name == name;
    });
    return it == catalog.end() ? nullptr : &(*it);
}

const BountyIdentityDefinition* BountyIdentityCatalog::pickFor(
    const std::string& seed,
    int playerLevel,
    const std::string& preferredCityId)
{
    std::vector<const BountyIdentityDefinition*> candidates;
    for (const BountyIdentityDefinition& definition : getCatalog())
    {
        if (definition.minLevel > std::max(1, playerLevel)) continue;
        if (!preferredCityId.empty() && definition.homeCityId != preferredCityId) continue;
        candidates.push_back(&definition);
    }
    if (candidates.empty() && !preferredCityId.empty())
    {
        for (const BountyIdentityDefinition& definition : getCatalog())
            if (definition.minLevel <= std::max(1, playerLevel)) candidates.push_back(&definition);
    }
    if (candidates.empty()) return nullptr;
    const std::size_t index = std::hash<std::string>{}(seed) % candidates.size();
    return candidates[index];
}
