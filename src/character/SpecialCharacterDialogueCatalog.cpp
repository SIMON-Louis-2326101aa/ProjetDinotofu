// EN: SpecialCharacterDialogueCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialCharacterDialogueCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Implementation of personality-based dialogue lines for special characters.
// Description : Implémentation des répliques liées à la personnalité des personnages spéciaux.

#include "character/SpecialCharacterDialogueCatalog.hpp"

#include <algorithm>
#include <cctype>
#include "interface/menu/common/MessageScreen.hpp"

#include <iostream>

// EN: hasDialogueFor declares or implements a focused behavior used by this module.
// FR: hasDialogueFor déclare ou implémente un comportement précis utilisé par ce module.
bool SpecialCharacterDialogueCatalog::hasDialogueFor(const std::string& characterName)
{
    std::string normalizedName = normalizeName(characterName);
    return !getEntranceLines(normalizedName).empty();
}

// EN: displayEntranceDialogue declares or implements a focused behavior used by this module.
// FR: displayEntranceDialogue déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterDialogueCatalog::displayEntranceDialogue(const std::string& characterName)
{
    displayLines(characterName, getEntranceLines(normalizeName(characterName)));
}

// EN: displayLowHealthDialogue declares or implements a focused behavior used by this module.
// FR: displayLowHealthDialogue déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterDialogueCatalog::displayLowHealthDialogue(const std::string& characterName)
{
    displayLines(characterName, getLowHealthLines(normalizeName(characterName)));
}

// EN: displayCombatActionDialogue declares or implements a focused behavior used by this module.
// FR: displayCombatActionDialogue déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterDialogueCatalog::displayCombatActionDialogue(const std::string& characterName, const std::string& actionLabel)
{
    displayLines(characterName, getCombatActionLines(normalizeName(characterName), actionLabel));
}

// EN: displayVictoryDialogue declares or implements a focused behavior used by this module.
// FR: displayVictoryDialogue déclare ou implémente un comportement précis utilisé par ce module.
void SpecialCharacterDialogueCatalog::displayVictoryDialogue(const std::string& characterName)
{
    displayLines(characterName, getVictoryLines(normalizeName(characterName)));
}

// EN: displayDefeatDialogue declares or implements a focused behavior used by this module.
// FR: displayDefeatDialogue déclare ou implémente un comportement précis utilisé par ce module.
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
        // EN: [] declares or implements a focused behavior used by this module.
        // FR: [] déclare ou implémente un comportement précis utilisé par ce module.
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

    if (normalizedName == "aoi")
    {
        return {"Aoi recule d'un pas, ses flammes tremblent autour d'elle.", "Je... je dois finir l'incantation avant qu'il soit trop tard."};
    }

    if (normalizedName == "kanade")
    {
        return {"Kanadé essuie le sang au coin de sa bouche.", "Ça y est, ça m'énerve. Vraiment."};
    }

    if (normalizedName == "sanctus")
    {
        return {"Sanctus encaisse en silence, puis resserre sa garde.", "Une protection fissurée reste une protection."};
    }

    if (normalizedName == "skuro")
    {
        return {"Skuro rit d'une voix basse.", "La douleur rend la coupe plus propre."};
    }

    if (normalizedName == "louis")
    {
        return {"Louis recule, surpris que ses gadgets n'aient pas suffi.", "Okay... note personnelle : améliorer tout. Vraiment tout."};
    }

    if (normalizedName == "trexof")
    {
        return {"Trexof change sa prise sur son arme.", "Limite trouvée. Maintenant, on voit si elle casse."};
    }

    if (normalizedName == "mattzelda")
    {
        return {"Mattzelda sourit encore, mais ses appuis deviennent sérieux.", "Ah ouais ? Là tu m'obliges à arrêter les blagues deux secondes."};
    }

    return {};
}

std::vector<std::string> SpecialCharacterDialogueCatalog::getCombatActionLines(
    const std::string& normalizedName,
    const std::string& actionLabel
)
{
    if (actionLabel == "healing")
    {
        if (normalizedName == "hestia") return {"Hestia cherche instinctivement à refermer ses blessures avant de paniquer davantage."};
        if (normalizedName == "sanctus") return {"Sanctus transforme son soin en serment : il ne tombera pas tant qu'il protège encore quelque chose."};
        if (normalizedName == "hazak") return {"Hazak se soigne sans fierté inutile. Survivre fait partie du meurtre."};
    }

    if (actionLabel == "damage")
    {
        if (normalizedName == "skuro") return {"Skuro renforce son prochain coup. La lame réclame un angle plus violent."};
        if (normalizedName == "fail") return {"Fail ajoute un réactif de trop. Évidemment."};
        if (normalizedName == "louis") return {"Louis arme un projectile supplémentaire avec un sourire beaucoup trop innocent."};
    }

    if (actionLabel == "attack")
    {
        if (normalizedName == "matt (pro)") return {"Matt attaque sans annonce. Propre, direct, presque vexant."};
        if (normalizedName == "aoi") return {"Aoi frappe surtout pour garder la distance et protéger ses incantations."};
        if (normalizedName == "kanade") return {"Kanadé attaque en râlant, comme si chaque coup était la faute de l'arène."};
        if (normalizedName == "trexof") return {"Trexof vise une ouverture précise, plus testeur que bourrin."};
        if (normalizedName == "henrique") return {"Henrique charge comme si reculer était une erreur de traduction."};
        if (normalizedName == "fire flight") return {"Fire Flight donne l'impression de commander une salve entière, même seul."};
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

    if (normalizedName == "matt (pro)") return {"Matt baisse sa garde. Pas de provocation. Juste le silence d'un adversaire qui a fait son travail."};
    if (normalizedName == "aoi") return {"Aoi laisse ses flammes disparaître.", "Je suis désolée... mais je devais me protéger."};
    if (normalizedName == "kanade") return {"Kanadé souffle bruyamment.", "Voilà. J'ai gagné. Et oui, je vais quand même me plaindre."};
    if (normalizedName == "sanctus") return {"Sanctus ne célèbre pas. Il vérifie seulement que personne derrière lui n'est tombé."};
    if (normalizedName == "hestia") return {"Hestia ouvre un œil, surprise d'être encore debout.", "C'est... fini ?"};
    if (normalizedName == "louis") return {"Louis range ses outils avec soulagement.", "On peut être amis maintenant ? Non ? Trop tôt ?"};
    if (normalizedName == "trexof") return {"Trexof hoche la tête.", "Test concluant. Tu étais plus solide que je pensais."};
    if (normalizedName == "henrique") return {"Henrique souffle, droit malgré les impacts.", "Un pas de plus. Toujours."};
    if (normalizedName == "fire flight") return {"Fire Flight abaisse son arme.", "La ligne a tenu. C'était le seul objectif."};

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

    if (normalizedName == "matt (pro)") return {"Matt accepte la défaite sans un mot. C'est presque plus stressant que s'il parlait."};
    if (normalizedName == "hazak") return {"Hazak pose un genou au sol, furieux surtout contre lui-même.", "Profite. Une erreur ne devient pas une habitude."};
    if (normalizedName == "aoi") return {"Aoi serre son focus contre elle.", "Je n'ai pas réussi à finir l'incantation..."};
    if (normalizedName == "sanctus") return {"Sanctus baisse son arme, mais pas son regard.", "Même au sol, je reste entre toi et ceux que je protège."};
    if (normalizedName == "hestia") return {"Hestia reste immobile, comme si la peur avait gagné avant la douleur.", "Je... je veux juste que ça s'arrête."};
    if (normalizedName == "mattzelda") return {"Mattzelda grimace, puis tente quand même une blague.", "Bon. Celle-là, je la note comme échauffement raté."};
    if (normalizedName == "trexof") return {"Trexof expire lentement.", "Limite dépassée. Bien joué."};
    if (normalizedName == "henrique") return {"Henrique tombe lourdement, mais son regard reste vivant.", "Je t'avais dit qu'une fois ne suffisait pas toujours."};
    if (normalizedName == "fire flight") return {"Fire Flight baisse les yeux, inquiet plus que vaincu.", "J'espère seulement que cette hostilité avait une raison."};

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

    MessageScreen::show(
        "DIALOGUE SPÉCIAL : " + characterName,
        "dialogue.special." + normalizeName(characterName),
        lines,
        false
    );
}
