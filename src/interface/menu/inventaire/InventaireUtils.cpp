#include "interface/menu/inventaire/InventaireUtils.hpp"

#include "objet/Inventaire.hpp"
#include "objet/consommable/Consommable.hpp"

GroupeConsommable::GroupeConsommable()
    : premierIndex(-1),
      quantite(0),
      nom(""),
      type(TypeConsommable::Inconnu),
      puissance(0)
{
}

std::string InventaireUtils::typeConsommableVersTexte(TypeConsommable type)
{
    switch (type)
    {
        case TypeConsommable::Soin:
            return "Soin";

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

std::string InventaireUtils::durabiliteArmeTexte(const Arme& arme)
{
    if (arme.estIndestructible())
    {
        return "indestructible";
    }

    return std::to_string(arme.getDurabilite()) + "/" + std::to_string(arme.getDurabiliteMax());
}

std::string InventaireUtils::durabiliteArmureTexte(const Armure& armure)
{
    if (armure.estIndestructible())
    {
        return "indestructible";
    }

    return std::to_string(armure.getDurabilite()) + "/" + std::to_string(armure.getDurabiliteMax());
}

std::vector<GroupeConsommable> InventaireUtils::grouperConsommables(const Joueur& joueur)
{
    std::vector<GroupeConsommable> groupes;
    const std::vector<Consommable>& consommables = joueur.getInventaire().getConsommables();

    for (int i = 0; i < static_cast<int>(consommables.size()); ++i)
    {
        const Consommable& consommable = consommables[i];
        bool trouve = false;

        for (GroupeConsommable& groupe : groupes)
        {
            if (groupe.nom == consommable.getNom()
                && groupe.type == consommable.getType()
                && groupe.puissance == consommable.getPuissance())
            {
                groupe.quantite++;
                trouve = true;
                break;
            }
        }

        if (!trouve)
        {
            GroupeConsommable groupe;
            groupe.premierIndex = i;
            groupe.quantite = 1;
            groupe.nom = consommable.getNom();
            groupe.type = consommable.getType();
            groupe.puissance = consommable.getPuissance();

            groupes.push_back(groupe);
        }
    }

    return groupes;
}