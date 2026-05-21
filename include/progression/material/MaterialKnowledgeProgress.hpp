// EN: MaterialKnowledgeProgress.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: MaterialKnowledgeProgress.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Tracks material discoveries and feeds the bestiary material journal.

#ifndef INCLUDE_PROGRESSION_MATERIAL_MATERIALKNOWLEDGEPROGRESS_HPP
#define INCLUDE_PROGRESSION_MATERIAL_MATERIALKNOWLEDGEPROGRESS_HPP

#include "item/material/Material.hpp"

#include <string>
#include <vector>

struct MaterialKnowledgeRecord
{
    std::string id;
    std::string name;
    std::string category;
    std::string quality;
    int discoveredQuantity;
    int bestQualityWeight;
};

class MaterialKnowledgeProgress
{
public:
    // EN: clear declares or implements a focused behavior used by this module.
    // FR: clear déclare ou implémente un comportement précis utilisé par ce module.
    static void clear();
    // EN: recordDiscovery declares or implements a focused behavior used by this module.
    // FR: recordDiscovery déclare ou implémente un comportement précis utilisé par ce module.
    static void recordDiscovery(const Material& material);
    // EN: getRecords declares or implements a focused behavior used by this module.
    // FR: getRecords déclare ou implémente un comportement précis utilisé par ce module.
    static std::vector<MaterialKnowledgeRecord> getRecords();
};

#endif
