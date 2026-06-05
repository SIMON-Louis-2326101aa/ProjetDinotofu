// EN: SpecialCharacterNameGuard.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterNameGuard.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Guard preventing simple identity theft of special characters during name selection.

#include "character/SpecialCharacterNameGuard.hpp"

#include "character/SpecialCharacterCatalog.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <iostream>
#include <vector>
#include <cctype>


namespace
{
    std::string normalizeSpecialIdentityName(std::string value)
    {
        for (char& c : value)
        {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }

        std::string compact;
        for (unsigned char c : value)
        {
            if (std::isalnum(c))
            {
                compact += static_cast<char>(c);
            }
        }
        return compact;
    }

    std::vector<std::string> getSpecialIdentityReactionLines(const SpecialCharacter& character)
    {
        const std::string name = normalizeSpecialIdentityName(character.getName());

        if (name == "fireflight")
        {
            return {
                "Une voix traverse l'écran avant même que la date soit demandée.",
                "FireFlight : Non.",
                "FireFlight : Les dieux peuvent prêter des avatars. Les créateurs peuvent laisser des fragments. Les devs peuvent oublier des portes.",
                "FireFlight : Mais moi, je ne suis pas une sauvegarde à charger. Je suis le test qui attend au bout.",
                "FireFlight : Si tu veux me croiser, bats les règles. Ne les emprunte pas.",
                "Verdict : identité non incarnable. Aucune date spéciale n'existe plus pour ce chemin."
            };
        }

        if (name == "hazak")
        {
            return {"Hazak ne lève même pas la voix.", "Hazak : Porter mon nom sans mes morts ? Mauvaise idée.", "Hazak : Prouve que tu es moi, ou choisis un nom qui survivra plus longtemps."};
        }

        if (name == "kanade")
        {
            return {"Kanadé tourne lentement la tête.", "Kanadé : Encore quelqu'un qui pense qu'une semi-dragonne, c'est juste un lézard avec du drama ?", "Kanadé : Essaie de voler mon nom et je te jure que même les constellations vont se plaindre."};
        }

        if (name == "mattzelda")
        {
            return {"Mattzelda regarde le nom, puis l'interface, puis le nom encore une fois.", "Mattzelda : Sérieusement ? Tu veux porter mon nom sans porter mes blagues, mes bleus et ma façon douteuse de survivre ?", "Mattzelda : Si c'est une usurpation, préviens tes dents : elles vont faire connaissance avec le sol."};
        }

        if (name == "aoi")
        {
            return {"Aoi serre son katana contre elle, les flammes kitsune basses mais vivantes.", "Aoi : Ce nom... ce n'est pas un masque.", "Aoi : Si tu mens, mes invocations le verront avant moi."};
        }

        if (name == "fail")
        {
            return {"Fail sourit trop vite pour que ce soit rassurant.", "Fail : Oh ! Une usurpation ! Ça se teste ?", "Fail : Non, attends. Si tu exploses, je note les résultats."};
        }

        if (name == "skuro")
        {
            return {"Skuro pose son épée lourde au sol. Le sol répond par un bruit inquiet.", "Skuro : Mon nom pèse plus lourd que toi.", "Skuro : Tu peux essayer de le porter. Tu peux aussi garder tes genoux."};
        }

        if (name == "sanctus")
        {
            return {"Sanctus ferme les yeux comme s'il priait pour ta mauvaise décision.", "Sanctus : Une identité n'est pas un déguisement. C'est une charge.", "Sanctus : Si tu mens, ma lumière ne te jugera pas deux fois."};
        }

        if (name == "hestia")
        {
            return {"Hestia recule d'un pas, puis son dôme apparaît par réflexe.", "Hestia : Je ne sais même pas encore tout ce que je suis...", "Hestia : Alors ne prends pas mon nom pour te cacher dedans."};
        }

        if (name == "louis")
        {
            return {"Louis cligne des yeux, presque triste avant d'être inquiet.", "Louis : Euh... tu veux vraiment être moi ?", "Louis : Je cherche déjà assez qui je suis, donc évite de brouiller la carte, s'il te plaît."};
        }

        if (name == "henrique")
        {
            return {"Henrique avance d'un pas, sans hausser la voix.", "Henrique : Mon nom n'est pas une armure qu'on vole sur un cadavre.", "Henrique : Si tu mens, je ne te poursuivrai pas longtemps. Je cours droit."};
        }

        if (name == "trexof")
        {
            return {"Trexof observe ton nom sans vraiment te regarder.", "Trexof : Ce nom appartient à quelqu'un qui repère les failles.", "Trexof : Si tu n'es pas la limite, tu vas juste la heurter."};
        }

        if (name == "mattpro")
        {
            return {"Matt (PRO) ne dit rien.", "Le silence dure assez longtemps pour devenir une menace pédagogique.", "Verdict : non jouable. Même le menu semble éviter son regard."};
        }

        return {
            character.getName() + " réagit à l'appel de son nom.",
            "Ce personnage spécial n'est pas une coquille vide : il possède déjà une voix, une histoire et une réaction face à l'usurpation."
        };
    }
    std::vector<std::string> getSpecialIdentityAcceptedLines(const SpecialCharacter& character)
    {
        const std::string name = normalizeSpecialIdentityName(character.getName());

        if (name == "hazak")
        {
            return {"Hazak fixe la date, puis ton regard.", "Hazak : ...Alors ce n'était pas une usurpation.", "Hazak : Très bien. Reprends la route. Mais souviens-toi : porter mon nom veut dire survivre à ce qu'il attire."};
        }

        if (name == "kanade")
        {
            return {"Kanadé cligne des yeux, surprise malgré elle.", "Kanadé : Ah. D'accord. Là, c'est vraiment toi.", "Kanadé : Bon... bienvenue. Mais si quelqu'un te traite encore de lézard, on mord ensemble."};
        }

        if (name == "mattzelda")
        {
            return {"Mattzelda plisse les yeux, puis éclate d'un rire beaucoup trop sincère.", "Mattzelda : Attends... c'est vraiment toi ? Bon bah là, je retire deux menaces et je garde une blague.", "Mattzelda : Bienvenue. Si le monde tape fort, réponds plus fort. Et si ça ne marche pas, fais semblant que c'était prévu."};
        }

        if (name == "aoi")
        {
            return {"Les flammes kitsune d'Aoi cessent de trembler.", "Aoi : La date répond... donc le lien existe.", "Aoi : Alors approche. Je garderai mes invocations calmes, mais ne fais pas mentir ce nom."};
        }

        if (name == "fail")
        {
            return {"Fail penche la tête, sincèrement étonné.", "Fail : Oh. Ça a marché.", "Fail : Bienvenue dans ton propre problème ! Promis, je ne note pas tout. Enfin... presque pas."};
        }

        if (name == "skuro")
        {
            return {"Skuro soulève son arme sans te menacer cette fois.", "Skuro : Le poids tient.", "Skuro : Alors marche. Si tes genoux lâchent maintenant, je fais semblant de ne pas avoir validé."};
        }

        if (name == "sanctus")
        {
            return {"Sanctus ouvre les yeux, la lumière moins hostile.", "Sanctus : La preuve est complète.", "Sanctus : Sois digne de cette identité. Une charge reconnue reste une charge."};
        }

        if (name == "hestia")
        {
            return {"Le dôme d'Hestia disparaît lentement.", "Hestia : C'est... vraiment reconnu ?", "Hestia : Alors bienvenue. Reste près du dôme si le monde devient trop bruyant."};
        }

        if (name == "louis")
        {
            return {"Louis relit la date deux fois, puis sourit avec soulagement.", "Louis : Ah, donc je ne suis pas en train de perdre encore un morceau de moi-même.", "Louis : Bienvenue. Enfin... bienvenue à moi ? C'est bizarre, mais ça ira."};
        }

        if (name == "henrique")
        {
            return {"Henrique reste immobile une seconde, surpris que la preuve tienne debout.", "Henrique : ...D'accord. Ce nom ne vient pas d'être volé.", "Henrique : Alors avance. Si tu tombes, relève-toi avant que quelqu'un décide que c'est terminé."};
        }

        if (name == "trexof")
        {
            return {"Trexof observe ton nom, puis hoche très légèrement la tête.", "Trexof : Le nom t'a reconnu.", "Trexof : Alors avance. Mais ne confonds pas reconnaissance et sécurité."};
        }

        if (name == "mattpro")
        {
            return {"Matt (PRO) reste silencieux.", "Le menu hésite, puis refuse quand même d'aller plus loin.", "Même reconnu, ce chemin n'est pas jouable."};
        }

        return {
            character.getName() + " reste surpris un instant.",
            "La date répond correctement : l'identité n'est pas volée, elle est reconnue.",
            "Accueil prudent : cette histoire peut continuer, mais elle sait maintenant que tu portes un nom lourd."
        };
    }

}

// EN: isProtectedName declares or implements a focused behavior used by this module.
// FR: isProtectedName déclare ou implémente un comportement précis utilisé par ce module.
bool SpecialCharacterNameGuard::isProtectedName(const std::string& name)
{
    return SpecialCharacterCatalog::isProtectedName(name);
}

bool SpecialCharacterNameGuard::tryGetProtectedCharacter(
    const std::string& name,
    SpecialCharacter& result
)
{
    return SpecialCharacterCatalog::findByName(name, result);
}

// EN: displayIdentityWarning declares or implements a focused behavior used by this module.
// FR: displayIdentityWarning déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterNameGuard::displayIdentityWarning(const SpecialCharacter& character)
{
    std::vector<std::string> lines;
    lines.push_back("Ce nom ne t'appartient pas vraiment.");
    lines.push_back(character.getName() + " possède déjà son histoire, ses cicatrices, ses choix et ses fautes.");
    lines.push_back("Usurper son identité n'est pas une simple fantaisie.");

    const std::vector<std::string> reactionLines = getSpecialIdentityReactionLines(character);
    for (const std::string& line : reactionLines)
    {
        lines.push_back(line);
    }

    if (character.isPermanentlyNonPlayable())
    {
        lines.push_back("Cette identité est verrouillée. Elle refuse d'être incarnée par ce chemin.");
    }
    else
    {
        lines.push_back("Si tu es réellement lié à ce personnage, il faudra le prouver.");
        lines.push_back("La date spéciale peut déverrouiller cette identité.");
    }

    MessageScreen::show("IDENTITÉ PROTÉGÉE", "character.special_name.warning", lines);
}

// EN: displayIdentityAccepted declares or implements a focused behavior used by this module.
// FR: displayIdentityAccepted déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterNameGuard::displayIdentityAccepted(const SpecialCharacter& character)
{
    std::vector<std::string> lines = {
        "Identité reconnue.",
        "Tu ne joues pas une copie.",
        "Tu réveilles une histoire déjà commencée : " + character.getName() + "."
    };

    const std::vector<std::string> acceptedLines = getSpecialIdentityAcceptedLines(character);
    for (const std::string& line : acceptedLines)
    {
        lines.push_back(line);
    }

    MessageScreen::show(
        "IDENTITÉ RECONNUE",
        "character.special_name.accepted",
        lines
    );
}

// EN: displayIdentityRefused declares or implements a focused behavior used by this module.
// FR: displayIdentityRefused déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterNameGuard::displayIdentityRefused(const SpecialCharacter& character)
{
    MessageScreen::show(
        "IDENTITÉ REFUSÉE",
        "character.special_name.refused",
        {
            "Date incorrecte.",
            character.getName() + " refuse de répondre à ton appel.",
            "La scène se referme comme si le personnage avait lui-même repoussé la sauvegarde.",
            "Choisis un autre nom."
        }
    );
}
