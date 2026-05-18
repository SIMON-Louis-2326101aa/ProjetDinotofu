#include "combat/ActionsCombat.hpp"

#include "combat/action/AttaqueCombat.hpp"
#include "combat/action/PotionCombat.hpp"

void ActionsCombat::executerAttaque(
    Entite& attaquant,
    Entite& defenseur,
    Random& random
)
{
    AttaqueCombat::executerAttaque(
        attaquant,
        defenseur,
        random
    );
}

void ActionsCombat::executerAttaqueBoostee(
    Entite& attaquant,
    Entite& defenseur,
    Random& random,
    int bonusDegats
)
{
    AttaqueCombat::executerAttaqueBoostee(
        attaquant,
        defenseur,
        random,
        bonusDegats
    );
}

bool ActionsCombat::executerPotionSoin(
    Entite& entite,
    int soinPotion
)
{
    return PotionCombat::executerPotionSoin(
        entite,
        soinPotion
    );
}

bool ActionsCombat::executerPotionDegats(
    Entite& attaquant,
    Entite& defenseur,
    Random& random,
    int bonusPotionDegats
)
{
    return PotionCombat::executerPotionDegats(
        attaquant,
        defenseur,
        random,
        bonusPotionDegats
    );
}