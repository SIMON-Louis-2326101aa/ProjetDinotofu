#include "interface/menu/potions/PotionsCombatUtils.hpp"

#include "objet/Inventaire.hpp"
#include "objet/consommable/Consommable.hpp"

std::vector<int> PotionsCombatUtils::recupererIndicesPotions(
    const Joueur& joueur,
    TypeConsommable type
)
{
    std::vector<int> indices;

    const std::vector<Consommable>& consommables =
        joueur.getInventaire().getConsommables();

    for (int i = 0; i < static_cast<int>(consommables.size()); ++i)
    {
        if (consommables[i].getType() == type)
        {
            indices.push_back(i);
        }
    }

    return indices;
}

std::string PotionsCombatUtils::typeVersTexte(TypeConsommable type)
{
    switch (type)
    {
        case TypeConsommable::Soin:
            return "Curative";

        case TypeConsommable::Degats:
            return "Offensive";

        case TypeConsommable::Buff:
            return "Buff";

        case TypeConsommable::Debuff:
            return "Debuff";

        case TypeConsommable::Special:
            return "Spéciale";

        default:
            return "Inconnue";
    }
}