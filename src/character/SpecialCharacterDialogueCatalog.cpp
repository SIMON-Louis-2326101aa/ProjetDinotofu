// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Implementation of personality-based dialogue lines for special characters.
// Description : Implémentation des répliques liées à la personnalité des personnages spéciaux.

#include "character/SpecialCharacterDialogueCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>

bool SpecialCharacterDialogueCatalog::hasDialogueFor(const std::string& characterName)
{
    std::string normalizedName = normalizeName(characterName);
    return !getEntranceLines(normalizedName).empty();
}

void SpecialCharacterDialogueCatalog::displayEntranceDialogue(const std::string& characterName)
{
    displayLines(characterName, getEntranceLines(normalizeName(characterName)));
}

void SpecialCharacterDialogueCatalog::displayLowHealthDialogue(const std::string& characterName)
{
    displayLines(characterName, getLowHealthLines(normalizeName(characterName)));
}

void SpecialCharacterDialogueCatalog::displayVictoryDialogue(const std::string& characterName)
{
    displayLines(characterName, getVictoryLines(normalizeName(characterName)));
}

void SpecialCharacterDialogueCatalog::displayDefeatDialogue(const std::string& characterName)
{
    displayLines(characterName, getDefeatLines(normalizeName(characterName)));
}

std::string SpecialCharacterDialogueCatalog::normalizeName(const std::string& name)
{
    std::string normalized = name;

    std::transform(
        normalized.begin(),
        normalized.end(),
        normalized.begin(),
        [](unsigned char character)
        {
            return static_cast<char>(std::tolower(character));
        }
    );

    // English: Tiny accent fallback for the current special names.
    // Français : Petit fallback d'accents pour les noms spéciaux actuels.
    std::string::size_type position = 0;
    while ((position = normalized.find("é", position)) != std::string::npos)
    {
        normalized.replace(position, 2, "e");
        position += 1;
    }

    return normalized;
}

std::vector<std::string> SpecialCharacterDialogueCatalog::getEntranceLines(const std::string& normalizedName)
{
    if (normalizedName == "matt (pro)")
    {
        return {
            "Matt ajuste sa posture sans dire un mot.",
            "Il n'a pas l'air impressionné. C'est probablement ça le problème."
        };
    }

    if (normalizedName == "hazak")
    {
        return {
            "Hazak observe l'arène comme s'il avait déjà choisi où tu tomberas.",
            "Je vais faire simple. Tu respires encore, donc le travail n'est pas terminé."
        };
    }

    if (normalizedName == "skuro")
    {
        return {
            "Skuro fait glisser son énorme lame sur le sol.",
            "J'espère que ta tête tient bien. J'aime quand le tranchant entre dans la matière."
        };
    }

    if (normalizedName == "kanade")
    {
        return {
            "Kanadé serre les poings en râlant déjà contre l'arène.",
            "Évidemment que ça tombe sur moi... Bon. Je vais gagner, mais je vais me plaindre tout du long."
        };
    }

    if (normalizedName == "aoi")
    {
        return {
            "Aoi baisse légèrement les yeux, puis trace un symbole brûlant dans l'air.",
            "Je... je ne voulais pas forcément me battre. Mais si tu approches de mes incantations, je te repousse."
        };
    }

    if (normalizedName == "sanctus")
    {
        return {
            "Sanctus lève son arme comme un rempart devant une lumière invisible.",
            "Je protège ce qui doit l'être. Si tu te places sur mon chemin, je te repousserai aussi."
        };
    }

    if (normalizedName == "fail")
    {
        return {
            "Fail sourit comme si le combat était déjà une expérience mal contrôlée.",
            "Parfait. Une menace, un terrain, deux ou trois explosions... on va apprendre quelque chose."
        };
    }

    if (normalizedName == "hestia")
    {
        return {
            "Hestia ferme les yeux beaucoup trop fort pour quelqu'un qui entre en combat.",
            "Je ne veux pas faire ça... mais si je fonce assez vite, peut-être que la peur restera derrière moi."
        };
    }

    if (normalizedName == "fire flight")
    {
        return {
            "Fire Flight analyse le terrain comme un commandant qui cherche encore une raison de croire au dialogue.",
            "Je préfère mener que détruire. Mais si tu forces la ligne, je ferai pleuvoir les projectiles."
        };
    }

    if (normalizedName == "louis")
    {
        return {
            "Louis vérifie ses mécanismes, puis te regarde avec un sourire un peu trop honnête.",
            "On peut se battre proprement, hein ? Après, si tu veux, on peut même devenir amis. Enfin... si tu survis."
        };
    }

    if (normalizedName == "mattzelda")
    {
        return {
            "Mattzelda arrive avec l'énergie d'un mur qui aurait appris à faire des blagues.",
            "T'inquiète, je vais y aller doucement. Enfin... doucement pour moi, pas forcément pour toi."
        };
    }

    if (normalizedName == "trexof")
    {
        return {
            "Trexof tourne sa lame entre ses doigts, calme et concentré.",
            "Je suis là pour tester les limites. Les tiennes, les miennes, et peut-être celles du jeu."
        };
    }

    if (normalizedName == "henrique")
    {
        return {
            "Henrique avance sans ralentir, bouclier haut et regard fixé droit devant.",
            "Je tombe peut-être une fois. Mais si tu crois que ça suffit à m'arrêter, tu vas être déçu."
        };
    }

    return {};
}

std::vector<std::string> SpecialCharacterDialogueCatalog::getLowHealthLines(const std::string& normalizedName)
{
    if (normalizedName == "hazak")
    {
        return {"Hazak sourit à peine.", "Enfin. Quelque chose qui ressemble vaguement à une menace."};
    }

    if (normalizedName == "fire flight")
    {
        return {"Fire Flight relève la tête.", "Sous la moitié de mes PV ? Mauvais timing pour te croire en sécurité."};
    }

    if (normalizedName == "hestia")
    {
        return {"Hestia tremble, mais sa magie devient plus dense.", "Non non non... Je veux rentrer. Mais d'abord, je dois survivre."};
    }

    if (normalizedName == "henrique")
    {
        return {"Henrique vacille, puis plante le pied au sol.", "Pas encore. Il me reste au moins une promesse à tenir."};
    }

    return {};
}

std::vector<std::string> SpecialCharacterDialogueCatalog::getVictoryLines(const std::string& normalizedName)
{
    if (normalizedName == "hazak")
    {
        return {"Hazak range sa lame sans regarder le corps tomber.", "Efficace. C'est tout ce qui compte."};
    }

    if (normalizedName == "mattzelda")
    {
        return {"Mattzelda éclate de rire.", "Bon, techniquement, j'avais dit doucement. C'est toi qui n'as pas précisé ton niveau."};
    }

    if (normalizedName == "fail")
    {
        return {"Fail prend mentalement des notes au milieu des dégâts.", "Conclusion : spectaculaire, mais améliorable. On recommence quand ?"};
    }

    return {};
}

std::vector<std::string> SpecialCharacterDialogueCatalog::getDefeatLines(const std::string& normalizedName)
{
    if (normalizedName == "kanade")
    {
        return {"Kanadé serre les dents.", "Je déteste perdre. Je déteste ce sol. Je déteste cette arène. Mais je reviendrai."};
    }

    if (normalizedName == "louis")
    {
        return {"Louis baisse son arme, presque gêné.", "Bon... on peut quand même rester en bons termes ?"};
    }

    if (normalizedName == "skuro")
    {
        return {"Skuro crache sur le côté.", "Ma lame n'a pas fini de manger. Retenir son tranchant ne veut pas dire l'arrêter."};
    }

    return {};
}

void SpecialCharacterDialogueCatalog::displayLines(
    const std::string& characterName,
    const std::vector<std::string>& lines
)
{
    if (lines.empty())
    {
        return;
    }

    std::cout << "========== DIALOGUE SPÉCIAL : " << characterName << " ==========" << std::endl;

    for (const std::string& line : lines)
    {
        std::cout << line << std::endl;
    }

    std::cout << "====================================================" << std::endl;
    std::cout << std::endl;
}
