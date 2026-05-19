// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Implements future dialogue lines for special character groups.

#include "character/relationship/SpecialCharacterGroupDialogueCatalog.hpp"

#include <algorithm>
#include <iostream>

void SpecialCharacterGroupDialogueCatalog::displayEntranceDialogue(const std::vector<std::string>& names)
{
    std::vector<std::string> lines = getEntranceLines(names);

    if (lines.empty())
    {
        return;
    }

    std::cout << "========== DIALOGUE DE GROUPE ==========" << std::endl;

    for (const std::string& line : lines)
    {
        std::cout << line << std::endl;
    }

    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
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
        return {
            "Henrique avance déjà. Hazak ne le retient pas : il ajuste seulement l'angle de sa lame.",
            "Hazak : Ne tombe pas trop vite.",
            "Henrique : Je te retourne le conseil."
        };
    }

    if (hasMattzelda && hasLouis && hasTrexof)
    {
        return {
            "Mattzelda rit avant même que le combat commence.",
            "Louis : On peut rester amis après, hein ?",
            "Trexof : D'abord on teste. Après on négocie."
        };
    }

    if (hasAoi && hasKanade && hasSanctus)
    {
        return {
            "Sanctus se place devant les deux mages comme un rempart vivant.",
            "Kanadé : Génial. Encore une arène. Encore du bruit. Encore quelqu'un à exploser.",
            "Aoi : Je... je prépare les incantations. Ne les laissez pas approcher."
        };
    }

    if (hasHazak && hasHestia)
    {
        return {
            "Hestia reste légèrement derrière Hazak, les mains crispées autour d'une magie qu'elle ne comprend pas entièrement.",
            "Hazak : Reste derrière moi. Ce combat ne deviendra pas un massacre.",
            "Hestia : Je ne veux juste pas revoir ça..."
        };
    }

    if (hasHazak && hasFail)
    {
        return {
            "Fail sourit à l'arène. Hazak, lui, ne sourit pas.",
            "Fail : Promis, je respecte le contrat. Je n'explose pas Hazak.",
            "Hazak : Essaie déjà de ne pas exploser tout le reste."
        };
    }

    if (hasSkuro)
    {
        return {
            "Skuro lève sa lame comme si le poids du métal était une invitation.",
            "Personne dans son groupe ne semble totalement rassuré par sa présence.",
            "Skuro : Si ça crie, c'est que ça coupe bien."
        };
    }

    if (hasFireFlight && hasMattPro)
    {
        return {
            "Fire Flight observe le terrain et donne un ordre bref.",
            "Matt (PRO) ne répond pas. Il se place simplement, avec le respect froid d'un adversaire sérieux.",
            "Fire Flight : Pas de haine. Juste une ligne à tenir."
        };
    }

    return {};
}
