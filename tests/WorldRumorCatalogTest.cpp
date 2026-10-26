#include <cassert>

#include "world/rumor/WorldRumorCatalog.hpp"

int main()
{
    const auto& catalog = WorldRumorCatalog::getCatalog();
    assert(catalog.size() >= 10);
    assert(WorldRumorCatalog::findById("rocheveille_mine_siffle") != nullptr);
    assert(WorldRumorCatalog::findById("fees_mana_politesse") != nullptr);
    assert(WorldRumorCatalog::findById("fitoria_chaines_lumiere") != nullptr);
    assert(WorldRumorCatalog::findById("zelef_sang_noir") != nullptr);
    assert(WorldRumorCatalog::findById("atlas_dernier_rempart") != nullptr);
    assert(WorldRumorCatalog::findById("guilde_registre_rouge") != nullptr);
    assert(WorldRumorCatalog::findById("courriers_nocturnes_relais") != nullptr);
    assert(WorldRumorCatalog::findById("torvald_trois_lecons") != nullptr);
    assert(WorldRumorCatalog::findById("grinka_taxe_retard") != nullptr);

    const auto* mine = WorldRumorCatalog::pickAmbient("rocheveille", "Mine sifflante", 25, 0);
    assert(mine != nullptr);
    assert(mine->subjectId.find("reapers") == std::string::npos);

    const auto* cieux = WorldRumorCatalog::findById("cieux_legendes_repondent");
    assert(cieux != nullptr);
    assert(cieux->minLevel == 85);
    return 0;
}
