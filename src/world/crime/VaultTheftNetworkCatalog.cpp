#include "world/crime/VaultTheftNetworkCatalog.hpp"

#include <algorithm>

const std::vector<VaultTheftNetworkContact>& VaultTheftNetworkCatalog::getCatalog()
{
    static const std::vector<VaultTheftNetworkContact> contacts = {
        {"valebrume_receleur_brocanteur", "valebrume", "receiver", "Serrin le brocanteur", "market", "Marché local de Valebrume", "Route commerciale", "Un brocanteur discret qui rachète surtout des lots sans provenance claire.", "marche_gris"},
        {"valebrume_complice_courriere", "valebrume", "accomplice", "Vela la courrière", "gate", "Abords de la grande porte", "Plaine sauvage", "Une messagère officieuse qui connaît les chemins secondaires autour de Valebrume.", "courriers_nocturnes"},
        {"valebrume_cache_sous_bois", "valebrume", "cache", "Cache du vieux sous-bois", "wild_cache", "Vieux sous-bois hors des axes", "Forêt ancienne", "Une cache de transit utilisée pour laisser reposer un butin avant revente.", ""},

        {"rocheveille_receleur_peseur", "rocheveille", "receiver", "Dorn le peseur", "forge_heavy", "Arrière-cour des forges de Rocheveille", "Mine sifflante", "Un peseur de métal qui accepte parfois des pièces dont l'origine est volontairement floue.", "marche_gris"},
        {"rocheveille_complice_rail", "rocheveille", "accomplice", "Kael des rails", "mine_lift", "Rails secondaires de l'ascenseur de mine", "Mine sifflante", "Un manutentionnaire capable de faire passer un petit chargement dans les rotations de minerai.", "courriers_nocturnes"},
        {"rocheveille_cache_carriere", "rocheveille", "cache", "Niche de la carrière blanche", "wild_cache", "Carrière périphérique", "Carrière des os blancs", "Une cavité sèche derrière des blocs de taille, assez loin des ateliers officiels.", ""},

        {"port_lanterne_receleur_collecteur", "port_lanterne", "receiver", "Collecteur masqué", "underbridge", "Marché sous les ponts", "Marché sous les ponts", "Un acheteur déjà connu des circuits douteux du port, prudent sur les provenances.", "marche_gris"},
        {"port_lanterne_complice_quais", "port_lanterne", "accomplice", "Seline des amarres", "harbor", "Quais des lanternes", "Canaux de brume bleue", "Une intermédiaire qui mélange parfois petits colis privés et cargaisons ordinaires.", "courriers_nocturnes"},
        {"port_lanterne_cache_quartier", "port_lanterne", "cache", "Réserve murée du vieux quartier", "wild_cache", "Quartier abandonné", "Quartier abandonné", "Une ancienne réserve commerciale murée puis réouverte par intermittence.", ""},

        {"lys_givre_receleur_tisseur", "lys_de_givre", "receiver", "Orven le tisseur froid", "market", "Marché couvert de Lys-de-Givre", "Montagne froide", "Un marchand de tissus isolants qui sait aussi écouler discrètement de petits objets de voyage.", "marche_gris"},
        {"lys_givre_complice_porteur", "lys_de_givre", "accomplice", "Maelis la porteuse", "frost_gate", "Porte du nord", "Glacier des Serments froids", "Une porteuse habituée aux expéditions, assez mobile pour déplacer un paquet sans attirer l'attention.", "courriers_nocturnes"},
        {"lys_givre_cache_temple", "lys_de_givre", "cache", "Crypte latérale aux cloches", "wild_cache", "Temple des cloches fendues", "Temple des cloches fendues", "Une alcôve froide et rarement visitée où un lot peut rester plusieurs jours.", ""}
    };
    return contacts;
}

const VaultTheftNetworkContact* VaultTheftNetworkCatalog::findById(const std::string& id)
{
    const auto& contacts = getCatalog();
    const auto it = std::find_if(contacts.begin(), contacts.end(), [&](const VaultTheftNetworkContact& c) { return c.id == id; });
    return it == contacts.end() ? nullptr : &(*it);
}

const VaultTheftNetworkContact* VaultTheftNetworkCatalog::pickFor(const std::string& cityId, const std::string& roleType, int salt)
{
    std::vector<const VaultTheftNetworkContact*> matches;
    for (const VaultTheftNetworkContact& c : getCatalog())
    {
        if (c.cityId == cityId && c.roleType == roleType) matches.push_back(&c);
    }
    if (matches.empty()) return nullptr;
    const int index = std::max(0, salt) % static_cast<int>(matches.size());
    return matches[static_cast<std::size_t>(index)];
}

std::vector<const VaultTheftNetworkContact*> VaultTheftNetworkCatalog::relatedContacts(const std::string& contactId, bool includeSameCity)
{
    std::vector<const VaultTheftNetworkContact*> result;
    const VaultTheftNetworkContact* source = findById(contactId);
    if (source == nullptr || source->networkId.empty()) return result;
    for (const VaultTheftNetworkContact& contact : getCatalog())
    {
        if (contact.id == source->id || contact.networkId != source->networkId) continue;
        if (!includeSameCity && contact.cityId == source->cityId) continue;
        result.push_back(&contact);
    }
    return result;
}
