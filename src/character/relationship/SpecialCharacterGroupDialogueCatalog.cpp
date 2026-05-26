// EN: SpecialCharacterGroupDialogueCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterGroupDialogueCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Implements relationship dialogue lines for special character groups.

#include "character/relationship/SpecialCharacterGroupDialogueCatalog.hpp"

#include <algorithm>
#include <iostream>

// EN: displayEntranceDialogue declares or implements a focused behavior used by this module.
// FR: displayEntranceDialogue déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterGroupDialogueCatalog::displayEntranceDialogue(const std::vector<std::string>& names)
{
    displayLines("DIALOGUE DE GROUPE", getEntranceLines(names));
}

// EN: displayVictoryDialogue declares or implements a focused behavior used by this module.
// FR: displayVictoryDialogue déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterGroupDialogueCatalog::displayVictoryDialogue(const std::vector<std::string>& names)
{
    displayLines("VICTOIRE DU GROUPE SPÉCIAL", getVictoryLines(names));
}

// EN: displayDefeatDialogue declares or implements a focused behavior used by this module.
// FR: displayDefeatDialogue déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterGroupDialogueCatalog::displayDefeatDialogue(const std::vector<std::string>& names)
{
    displayLines("DÉFAITE DU GROUPE SPÉCIAL", getDefeatLines(names));
}

bool SpecialCharacterGroupDialogueCatalog::containsName(
    const std::vector<std::string>& names,
    const std::string& expectedName
)
{
    return std::find(names.begin(), names.end(), expectedName) != names.end();
}

std::vector<std::string> SpecialCharacterGroupDialogueCatalog::getEntranceLines(const std::vector<std::string>& names)
{
    bool hasHazak = containsName(names, "Hazak");
    bool hasHenrique = containsName(names, "Henrique");
    bool hasMattzelda = containsName(names, "Mattzelda");
    bool hasLouis = containsName(names, "Louis");
    bool hasTrexof = containsName(names, "Trexof");
    bool hasAoi = containsName(names, "Aoi");
    bool hasKanade = containsName(names, "Kanadé");
    bool hasSanctus = containsName(names, "Sanctus");
    bool hasHestia = containsName(names, "Hestia");
    bool hasFail = containsName(names, "Fail");
    bool hasSkuro = containsName(names, "Skuro");
    bool hasFireFlight = containsName(names, "Fire Flight");
    bool hasMattPro = containsName(names, "Matt (PRO)");

    if (hasHazak && hasHenrique)
    {
        return {"Henrique avance déjà. Hazak ne le retient pas : il ajuste seulement l'angle de sa lame.", "Hazak : Ne tombe pas trop vite.", "Henrique : Je te retourne le conseil."};
    }

    if (hasMattzelda && hasLouis && hasTrexof)
    {
        return {"Mattzelda rit avant même que le combat commence.", "Louis : On peut rester amis après, hein ?", "Trexof : D'abord on teste. Après on négocie."};
    }

    if (hasAoi && hasKanade && hasSanctus)
    {
        return {"Sanctus se place devant les deux mages comme un rempart vivant.", "Kanadé : Génial. Encore une arène. Encore du bruit. Encore quelqu'un à exploser.", "Aoi : Je... je prépare les incantations. Ne les laissez pas approcher."};
    }

    if (hasHazak && hasHestia)
    {
        return {"Hestia reste légèrement derrière Hazak, les mains crispées autour d'une magie qu'elle ne comprend pas entièrement.", "Hazak : Reste derrière moi. Ce combat ne deviendra pas un massacre.", "Hestia : Je ne veux juste pas revoir ça..."};
    }

    if (hasHazak && hasFail)
    {
        return {"Fail sourit à l'arène. Hazak, lui, ne sourit pas.", "Fail : Promis, je respecte le contrat. Je n'explose pas Hazak.", "Hazak : Essaie déjà de ne pas exploser tout le reste."};
    }

    if (hasHestia && hasSanctus && hasHazak)
    {
        return {"Sanctus se place devant Hestia. Hazak se place à l'endroit où personne ne devrait passer.", "Hestia : Je... je vais essayer de ne pas fermer les yeux.", "Hazak : Si tu les fermes, on combattra quand même.", "Sanctus : Alors personne ne l'atteindra."};
    }

    if (hasFail && hasAoi && hasKanade)
    {
        return {"Fail observe les deux mages comme si l'arène était déjà un laboratoire.", "Aoi : Je peux stabiliser une partie des flammes... je crois.", "Kanadé : Stabiliser ? Non. On va faire plus fort.", "Fail : Voilà. C'est exactement l'esprit scientifique."};
    }

    if (hasLouis && hasFireFlight && hasTrexof)
    {
        return {"Fire Flight lève une main. Louis prépare trop de projectiles. Trexof regarde déjà les limites de l'arène.", "Fire Flight : On ne gagne pas par hasard. On gagne parce que le chaos obéit trois secondes.", "Louis : Trois secondes, c'est large, non ?", "Trexof : Pour une faille, oui."};
    }

    if (hasHenrique && hasMattzelda && hasSkuro)
    {
        return {"Henrique avance en premier. Mattzelda rit derrière lui. Skuro ne rit pas : il choisit déjà où couper.", "Mattzelda : Je sens que ce groupe est sain et équilibré.", "Henrique : Mensonge, mais avance.", "Skuro : Tant que ça finit ouvert."};
    }

    if (hasSkuro)
    {
        return {"Skuro lève sa lame comme si le poids du métal était une invitation.", "Personne dans son groupe ne semble totalement rassuré par sa présence.", "Skuro : Si ça crie, c'est que ça coupe bien."};
    }

    if (hasFireFlight && hasMattPro)
    {
        return {"Fire Flight observe le terrain et donne un ordre bref.", "Matt (PRO) ne répond pas. Il se place simplement, avec le respect froid d'un adversaire sérieux.", "Fire Flight : Pas de haine. Juste une ligne à tenir."};
    }

    return {};
}

std::vector<std::string> SpecialCharacterGroupDialogueCatalog::getVictoryLines(const std::vector<std::string>& names)
{
    if (containsName(names, "Hazak") && containsName(names, "Hestia"))
    {
        return {"Hazak nettoie sa lame sans regarder Hestia.", "Hazak : C'est terminé. Tu n'as pas besoin de retenir cette image.", "Hestia : Merci... je crois."};
    }

    if (containsName(names, "Mattzelda") && containsName(names, "Louis") && containsName(names, "Trexof"))
    {
        return {"Mattzelda : Victoire des potes, let's gooo.", "Louis : On a gagné ET personne n'est trop fâché ?", "Trexof : Statistiquement, c'est le meilleur résultat."};
    }

    if (containsName(names, "Aoi") && containsName(names, "Kanadé") && containsName(names, "Sanctus"))
    {
        return {"Sanctus : Menace repoussée.", "Kanadé : J'ai gagné, mais cette arène m'énerve toujours.", "Aoi : Les flammes peuvent se calmer maintenant..."};
    }

    if (containsName(names, "Fail") && containsName(names, "Aoi") && containsName(names, "Kanadé"))
    {
        return {"Fail : Données validées. Plusieurs explosions, aucune perte vraiment regrettable.", "Aoi : C'est... censé être rassurant ?", "Kanadé : On recommence quand ?"};
    }

    if (containsName(names, "Louis") && containsName(names, "Fire Flight") && containsName(names, "Trexof"))
    {
        return {"Fire Flight : Test terminé.", "Louis : Et on a encore des amis ?", "Trexof : Selon les résultats, ça dépend de la définition d'ami."};
    }

    if (containsName(names, "Skuro"))
    {
        return {"Skuro semble déçu que le combat s'arrête déjà.", "Skuro : Pas assez profond. Pas assez propre. Mais ça fera l'affaire."};
    }

    if (containsName(names, "Hazak") && containsName(names, "Henrique"))
    {
        return {"Henrique sourit malgré les impacts.", "Henrique : Toujours debout.", "Hazak : Pour une fois, c'était utile."};
    }

    return {};
}

std::vector<std::string> SpecialCharacterGroupDialogueCatalog::getDefeatLines(const std::vector<std::string>& names)
{
    if (containsName(names, "Hazak") && containsName(names, "Henrique"))
    {
        return {"Henrique tend la main à Hazak avant même de vérifier ses propres blessures.", "Hazak : Ne m'aide pas.", "Henrique : Trop tard."};
    }

    if (containsName(names, "Hazak") && containsName(names, "Hestia"))
    {
        return {"Hazak reste entre toi et Hestia même au sol.", "Hazak : Le combat est terminé. Pas un pas de plus.", "Hestia : Je... je ne veux pas qu'il recommence."};
    }

    if (containsName(names, "Mattzelda") && containsName(names, "Louis") && containsName(names, "Trexof"))
    {
        return {"Mattzelda : Bon, celle-là compte comme échauffement collectif raté.", "Louis : On peut oublier cette partie ?", "Trexof : Non. C'est justement le genre de test qu'on garde."};
    }

    if (containsName(names, "Aoi") && containsName(names, "Kanadé") && containsName(names, "Sanctus"))
    {
        return {"Sanctus baisse un genou, toujours devant les autres.", "Kanadé : Je déteste perdre. Vraiment.", "Aoi : L'incantation... n'a pas tenu."};
    }

    if (containsName(names, "Fail") && containsName(names, "Aoi") && containsName(names, "Kanadé"))
    {
        return {"Fail : Échec très instructif.", "Kanadé : Je vais t'instruire mon poing si tu répètes ça.", "Aoi : On peut... juste partir ?"};
    }

    if (containsName(names, "Louis") && containsName(names, "Fire Flight") && containsName(names, "Trexof"))
    {
        return {"Louis : On a perdu, mais c'était presque stylé.", "Fire Flight : Presque n'est pas une condition de victoire.", "Trexof : Mais c'est une donnée utile."};
    }

    if (containsName(names, "Skuro"))
    {
        return {"Skuro rit malgré sa défaite.", "Skuro : Tu as gagné le tour. Pas le droit de garder ta tête éternellement."};
    }

    return {};
}

void SpecialCharacterGroupDialogueCatalog::displayLines(
    const std::string& title,
    const std::vector<std::string>& lines
)
{
    if (lines.empty())
    {
        return;
    }

    std::cout << "========== " << title << " ==========" << std::endl;

    for (const std::string& line : lines)
    {
        std::cout << line << std::endl;
    }

    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
}
