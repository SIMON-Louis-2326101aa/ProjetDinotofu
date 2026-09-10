// EN: MonsterBehaviorProfile.cpp defines readable enemy behavior profiles for Dinotofu combat.
// FR: MonsterBehaviorProfile.cpp définit des profils de comportement ennemis lisibles pour les combats de Dinotofu.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "combat/profile/MonsterBehaviorProfile.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
    std::string normalizeProfileText(std::string value)
    {
        for (char& character : value)
        {
            character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
        }
        return value;
    }

    bool containsAny(const std::string& text, const std::vector<std::string>& needles)
    {
        const std::string normalized = normalizeProfileText(text);
        for (const std::string& needle : needles)
        {
            if (normalized.find(normalizeProfileText(needle)) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    std::string joinShortList(const std::vector<std::string>& values)
    {
        std::ostringstream stream;
        for (std::size_t index = 0; index < values.size(); ++index)
        {
            if (index > 0)
            {
                stream << (index + 1 == values.size() ? " et " : ", ");
            }
            stream << values[index];
        }
        return stream.str();
    }

    bool profileLooksSmallAndFragile(const MonsterBehaviorProfile& profile)
    {
        const std::string archetype = normalizeProfileText(profile.archetype);
        return profile.physicalDamageModifierPercent >= 10
            || archetype.find("fée") != std::string::npos
            || archetype.find("nuisible") != std::string::npos
            || archetype.find("chauve-souris") != std::string::npos
            || archetype.find("kobold") != std::string::npos
            || archetype.find("insecto") != std::string::npos
            || archetype.find("araignée") != std::string::npos;
    }

    bool profileLooksHardOrArmored(const MonsterBehaviorProfile& profile)
    {
        const std::string archetype = normalizeProfileText(profile.archetype);
        return profile.physicalDamageModifierPercent <= -12
            || archetype.find("construction") != std::string::npos
            || archetype.find("carapace") != std::string::npos
            || archetype.find("gardien") != std::string::npos
            || archetype.find("draconique") != std::string::npos
            || archetype.find("serment") != std::string::npos;
    }

    bool profileLooksLarge(const MonsterBehaviorProfile& profile)
    {
        const std::string archetype = normalizeProfileText(profile.archetype);
        return archetype.find("chargeur massif") != std::string::npos
            || archetype.find("brute") != std::string::npos
            || archetype.find("construction") != std::string::npos
            || archetype.find("draconique") != std::string::npos
            || archetype.find("gardien") != std::string::npos;
    }

    bool profileLooksSoftOrRotten(const MonsterBehaviorProfile& profile, const Monster& monster)
    {
        const std::string combined = monster.getName() + " " + monster.getType() + " " + monster.getRaceText() + " " + profile.archetype;
        return containsAny(combined, {
            "mousse", "moisi", "pourri", "putride", "fragile", "vase molle", "bois mort",
            "champignon", "spore", "lambeau", "chair molle", "os friable"
        });
    }

    MonsterBehaviorProfile makeDefaultProfile(const Monster& monster)
    {
        MonsterBehaviorProfile profile;
        profile.archetype = monster.getRaceText();
        profile.behaviorLine = "Comportement lisible : attaque directe, adaptation limitée, réactions surtout liées à sa race.";
        profile.attackDescription = monster.getName() + " attaque avec son instinct propre";
        profile.signatureMove = "attaque directe sans rituel particulier";
        profile.counterplayLine = "Réponse conseillée : observer une fois, puis choisir une action tactique adaptée au statut ou à la posture.";
        profile.strengths = {"profil simple à lire", "pression régulière"};
        profile.weaknesses = {"sensible à l'observation active", "peut être puni par les actions tactiques"};
        profile.reactions = {"réagit surtout aux dégâts reçus et aux statuts visibles"};
        profile.incomingAccuracyModifier = 0;
        profile.accuracyLine = "Sa silhouette ne change pas vraiment la difficulté de viser.";
        profile.physicalDamageModifierPercent = 0;
        profile.magicalDamageModifierPercent = 0;
        profile.durabilityLine = "Sa chair ne montre pas de résistance ou de fragilité évidente.";
        return profile;
    }

    void applyMaterialAndBodyTraits(MonsterBehaviorProfile& profile, const Monster& monster, const std::string& combined)
    {
        (void)monster;
        const std::string archetype = normalizeProfileText(profile.archetype);
        const bool hardMaterial = containsAny(combined, {
            "golem", "construction", "armure", "plaques", "pierre", "roche", "roc", "fer", "acier",
            "cristal", "cristallin", "noyau", "carapace", "écailles", "ecailles", "os", "squelette",
            "sentinelle", "gardien", "dragon", "draconide"
        }) || archetype.find("carapace") != std::string::npos
           || archetype.find("construction") != std::string::npos
           || archetype.find("gardien") != std::string::npos
           || archetype.find("draconique") != std::string::npos;
        const bool poorMaterial = containsAny(combined, {
            "mousse", "moisi", "pourri", "putride", "fragile", "faible", "tissu", "vase molle", "bois mort",
            "champignon", "spore", "lambeau", "chair molle", "os friable"
        });
        const bool smallBody = containsAny(combined, {
            "rat", "souris", "nuisible", "chauve", "fée", "fee", "pixie", "insecte", "insectoïde",
            "insectoide", "araignée", "araignee", "kobold", "gobelin", "moustique", "frelon", "scarabée", "scarabee"
        }) || archetype.find("nuisible") != std::string::npos
           || archetype.find("fée") != std::string::npos
           || archetype.find("carapace basse") != std::string::npos;
        const bool largeBody = containsAny(combined, {
            "massif", "massive", "colosse", "brute", "ours", "sanglier", "géant", "geant", "golem",
            "construction", "dragon", "draconide", "gardien", "sentinelle", "titan", "ogre"
        }) || archetype.find("brute") != std::string::npos
           || archetype.find("chargeur massif") != std::string::npos
           || archetype.find("construction") != std::string::npos
           || archetype.find("draconique") != std::string::npos;

        if (archetype.find("fée") != std::string::npos)
        {
            profile.physicalDamageModifierPercent = 50;
            profile.magicalDamageModifierPercent = -50;
            profile.durabilityLine = "Petit corps féerique : la magie glisse en partie, mais la chair cède vite sous un impact physique.";
            return;
        }

        if (archetype.find("spectral") != std::string::npos)
        {
            profile.physicalDamageModifierPercent = -18;
            profile.durabilityLine = "Corps spectral : une part du choc physique traverse mal la cible.";
            profile.strengths.push_back("corps difficile à blesser par simple choc physique");
            profile.weaknesses.push_back("lumière ou feu pour fixer la forme");
            return;
        }

        if (archetype.find("slime") != std::string::npos)
        {
            profile.physicalDamageModifierPercent = -8;
            profile.durabilityLine = "Masse gélatineuse : les coups physiques se perdent souvent dans la matière tant que le noyau n'est pas ouvert.";
            profile.weaknesses.push_back("noyau à révéler avant les gros impacts");
            return;
        }

        if (hardMaterial)
        {
            if (smallBody)
            {
                profile.physicalDamageModifierPercent = -8;
                profile.durabilityLine = "Petit gabarit protégé : difficile à cadrer, mais la matière dure absorbe une partie des impacts qui touchent.";
            }
            else if (largeBody)
            {
                profile.physicalDamageModifierPercent = -22;
                profile.durabilityLine = "Grande masse dure : facile à atteindre, nettement moins facile à blesser en profondeur.";
            }
            else
            {
                profile.physicalDamageModifierPercent = -14;
                profile.durabilityLine = "Matière dure : les impacts physiques doivent trouver une jointure ou une faille.";
            }
            profile.strengths.push_back("résistance physique de matière");
            profile.weaknesses.push_back("jointures, noyau ou garde cassée");
            return;
        }

        if (smallBody)
        {
            if (archetype.find("insecto") != std::string::npos)
            {
                profile.physicalDamageModifierPercent = 8;
                profile.durabilityLine = "Petit corps chitineux : dur à toucher, mais un impact propre reste très dangereux.";
            }
            else if (archetype.find("kobold") != std::string::npos || archetype.find("gobelin") != std::string::npos)
            {
                profile.physicalDamageModifierPercent = 12;
                profile.durabilityLine = "Petit gabarit nerveux : il évite beaucoup, mais encaisse mal les vrais coups.";
            }
            else
            {
                profile.physicalDamageModifierPercent = 22;
                profile.durabilityLine = "Petit corps fragile : la difficulté est de toucher, pas de traverser la chair.";
            }
            profile.weaknesses.push_back("fragilité physique si le coup touche vraiment");
            return;
        }

        if (largeBody)
        {
            if (poorMaterial)
            {
                profile.physicalDamageModifierPercent = 4;
                profile.durabilityLine = "Grand corps mal tenu : facile à toucher et pas assez dense pour vraiment arrêter les impacts.";
                profile.weaknesses.push_back("matière trop mauvaise pour son volume");
            }
            else
            {
                profile.physicalDamageModifierPercent = -12;
                profile.durabilityLine = "Grand corps dense : il offre une large cible, mais sa masse avale une partie du choc physique.";
                profile.strengths.push_back("masse résistante aux impacts simples");
            }
            return;
        }

        if (archetype.find("plante") != std::string::npos)
        {
            if (poorMaterial)
            {
                profile.physicalDamageModifierPercent = 10;
                profile.durabilityLine = "Tissu végétal mou : les lames et chocs propres entament facilement la matière.";
            }
            else
            {
                profile.physicalDamageModifierPercent = -6;
                profile.durabilityLine = "Fibres végétales : les coups physiques s'accrochent et perdent une partie de leur force.";
            }
            return;
        }

        if (archetype.find("mort-vivant") != std::string::npos)
        {
            profile.physicalDamageModifierPercent = -4;
            profile.durabilityLine = "Corps sans douleur : il encaisse mal en chair, mais ne cède pas toujours quand il devrait.";
            return;
        }

        profile.durabilityLine = "Corps lisible : la taille, la matière et le rythme ne modifient pas fortement les dégâts physiques.";
    }
}

namespace MonsterBehaviorProfileCatalog
{
    MonsterBehaviorProfile build(const Monster& monster)
    {
        const std::string combined = monster.getName() + " " + monster.getType() + " " + monster.getRaceText();
        MonsterBehaviorProfile profile = makeDefaultProfile(monster);

        if (containsAny(combined, {"rameuteur", "rameuteuse", "hurleur", "hurleuse", "corneur", "guetteur", "guetteuse", "crieur", "crieuse", "tambour", "alarme"})
            && containsAny(combined, {"gobelin", "loup", "meute", "chien", "rat", "nuisible", "kobold", "insecte", "insectoïde", "insectoide", "araignée", "araignee", "orc", "bandit", "pillard"}))
        {
            profile.archetype = "Rameuteur de terrain";
            profile.behaviorLine = "Comportement rameuteur : il n'est pas seulement dangereux par ses coups, mais par sa capacité à garder le groupe autour de lui.";
            profile.attackDescription = monster.getName() + " frappe en gardant assez de souffle pour lancer un signal";
            profile.signatureMove = "signal de renfort qui peut ramener deux ou trois alliés communs si la ligne craque";
            profile.counterplayLine = "Réponse conseillée : l'isoler, casser sa formation, le finir vite en bas de vie ou l'empêcher de garder le souffle.";
            profile.strengths = {"appel de renfort fiable", "pression de groupe", "ne combat presque jamais seul longtemps"};
            profile.weaknesses = {"isolement", "brise-garde", "casser la formation", "burst sur bas PV"};
            profile.reactions = {"appelle quand la ligne tombe", "hurle si la blessure dure", "cherche à survivre assez longtemps pour rameuter"};
            profile.incomingAccuracyModifier = containsAny(combined, {"rat", "nuisible", "fée", "fee"}) ? -13 : -5;
            profile.accuracyLine = "Il bouge moins pour tuer que pour garder un angle de fuite et un souffle de signal.";
        }
        else if (containsAny(monster.getRaceText(), {"slime"}) || containsAny(combined, {"gelée", "gelee", "gélatine", "gelatine"}))
        {
            profile.archetype = "Slime / masse gélatineuse";
            profile.behaviorLine = "Comportement slime : il ne frappe pas vraiment, il s'étire, colle, rebondit et cherche à salir le rythme du combat.";
            profile.attackDescription = monster.getName() + " projette une masse gluante au lieu d'un coup propre";
            profile.signatureMove = "contact gélatineux qui colle, glisse ou change selon la couleur";
            profile.counterplayLine = "Réponse conseillée : viser le noyau, casser la garde ou utiliser feu, froid ou poussière selon la matière.";
            profile.strengths = {"corps souple", "angles imprévisibles", "statuts selon couleur"};
            profile.weaknesses = {"noyau exposable", "mauvaise gestion des ruptures de garde", "lecture utile via observation"};
            profile.reactions = {"peut coller", "peut ralentir", "peut laisser une matière dangereuse"};
            profile.incomingAccuracyModifier = -3;
            profile.accuracyLine = "La masse molle se déforme autour du trajet de l'arme.";

            if (containsAny(combined, {"toxique", "putride", "venime", "poison", "violet", "noir", "vaseux"}))
            {
                profile.archetype = "Slime toxique";
                profile.attackDescription = monster.getName() + " éclabousse avec une gelée toxique qui cherche la moindre égratignure";
                profile.strengths = {"poison", "pression sur combats longs", "matière nocive"};
                profile.weaknesses = {"antidote utile", "noyau fragile une fois révélé", "brûlure ou rupture de rythme"};
                profile.reactions = {"empoisonne parfois", "devient pénible si le joueur tarde", "profite des blessures déjà ouvertes"};
                profile.incomingAccuracyModifier = -4;
                profile.accuracyLine = "Sa vase toxique se replie avant le centre du coup.";
            }
            else if (containsAny(combined, {"ambré", "ambre", "collant", "poisseux", "sève", "seve"}))
            {
                profile.archetype = "Slime collant";
                profile.attackDescription = monster.getName() + " s'écrase en rubans collants pour bloquer les appuis";
                profile.strengths = {"entrave", "affaiblissement court", "gêne l'esquive"};
                profile.weaknesses = {"sensible au contrôle", "peu explosif seul", "mauvais contre les attaques préparées"};
                profile.reactions = {"ralentit", "colle", "force souvent une mauvaise position"};
                profile.incomingAccuracyModifier = -6;
                profile.accuracyLine = "La matière poisseuse tire l'arme hors de sa ligne.";
            }
            else if (containsAny(combined, {"froid", "froide", "givre", "gel", "bleu", "blanc", "neige"}))
            {
                profile.archetype = "Slime froid";
                profile.attackDescription = monster.getName() + " étale une gelée froide qui mord les articulations";
                profile.strengths = {"givre", "ralentissement", "défense souple"};
                profile.weaknesses = {"chaleur", "pression continue", "noyau durci facile à viser après observation"};
                profile.reactions = {"ralentit parfois", "encaisse mieux les petits impacts", "perd du rythme si sa garde est cassée"};
                profile.incomingAccuracyModifier = -2;
                profile.accuracyLine = "Le noyau durci se lit mieux, mais la gelée glisse encore.";
            }
            else if (containsAny(combined, {"rouge", "chaud", "brûl", "brul", "lave", "cendre", "feu"}))
            {
                profile.archetype = "Slime chaud";
                profile.attackDescription = monster.getName() + " claque une vague de gelée chaude sur la cible";
                profile.strengths = {"brûlure", "dégâts réguliers", "pression agressive"};
                profile.weaknesses = {"froid", "contrôle", "failles créées par observation"};
                profile.reactions = {"brûle parfois", "punit les combats statiques", "devient dangereux si ignoré"};
                profile.incomingAccuracyModifier = -3;
                profile.accuracyLine = "La chaleur brouille le bord exact de la masse.";
            }
            else if (containsAny(combined, {"rose", "bond", "nerveux", "vibrant"}))
            {
                profile.archetype = "Slime bondissant";
                profile.attackDescription = monster.getName() + " rebondit en biais et transforme le sol en piège vivant";
                profile.strengths = {"imprévisible", "angles étranges", "bonne pression sur l'esquive"};
                profile.weaknesses = {"poussière", "coup de pied", "formation cassée"};
                profile.reactions = {"change d'angle", "rate parfois son propre rythme", "devient lisible avec observation"};
                profile.incomingAccuracyModifier = -12;
                profile.accuracyLine = "Ses rebonds brisent la trajectoire au dernier moment.";
            }
            else if (containsAny(combined, {"doré", "dore", "brillant", "or", "nacré", "nacre"}))
            {
                profile.archetype = "Slime brillant";
                profile.attackDescription = monster.getName() + " frappe en protégeant instinctivement ce qui brille dans sa masse";
                profile.strengths = {"petite garde", "noyau précieux", "réflexe défensif"};
                profile.weaknesses = {"brise-garde", "observation active", "attaque précise sur noyau"};
                profile.reactions = {"se protège", "attire l'attention", "garde souvent son noyau"};
                profile.incomingAccuracyModifier = 2;
                profile.accuracyLine = "Le noyau attire l'oeil, même quand la gelée tente de le cacher.";
            }
        }
        else if (containsAny(combined, {"chauve-souris", "chauve souris"}))
        {
            profile.archetype = "Chauve-souris de harcèlement";
            profile.behaviorLine = "Comportement aérien : elle ne veut pas encaisser, elle gêne la lecture, coupe la respiration et repart avant la riposte.";
            profile.attackDescription = monster.getName() + " rase le visage avec un battement d'ailes nerveux avant de mordre";
            profile.signatureMove = "harcèlement aérien qui peut affaiblir la précision du prochain geste";
            profile.counterplayLine = "Réponse conseillée : poussière, coup large, provocation ou tenir la ligne pour casser son angle.";
            profile.strengths = {"mobilité", "angles hauts", "gêne la lecture"};
            profile.weaknesses = {"poussière", "attaque de zone", "garde stable"};
            profile.reactions = {"recule après l'impact", "punit les cibles isolées", "perd beaucoup si son vol est perturbé"};
            profile.incomingAccuracyModifier = -16;
            profile.accuracyLine = "Elle est petite, haute et rarement dans la ligne du coup.";
        }
        else if (containsAny(combined, {"rat", "grenier", "nuisible"}))
        {
            profile.archetype = "Nuisible mordeur";
            profile.behaviorLine = "Comportement nuisible : il ne gagne pas par puissance, mais par morsures répétées, panique et petites plaies sales.";
            profile.attackDescription = monster.getName() + " se glisse bas et mord là où l'armure protège le moins";
            profile.signatureMove = "morsure sale qui peut faire saigner ou salir une blessure déjà ouverte";
            profile.counterplayLine = "Réponse conseillée : coup de pied, lanterne au sol ou action de zone pour l'empêcher de tourner autour.";
            profile.strengths = {"petite taille", "morsures rapides", "pression sur combats longs"};
            profile.weaknesses = {"coup de pied", "feu au sol", "contrôle court"};
            profile.reactions = {"devient plus pénible en groupe", "profite des cibles déjà blessées", "panique si on reprend l'espace"};
            profile.incomingAccuracyModifier = -15;
            profile.accuracyLine = "Trop bas, trop vif, il passe sous beaucoup de coups propres.";
        }
        else if (containsAny(combined, {"sanglier", "ours", "massive", "massif"}))
        {
            profile.archetype = "Chargeur massif";
            profile.behaviorLine = "Comportement massif : il annonce presque ses coups, mais l'impact déplace la ligne si personne ne l'arrête.";
            profile.attackDescription = monster.getName() + " charge avec tout son poids au lieu de chercher un duel propre";
            profile.signatureMove = "charge lourde qui peut affaiblir la garde et pousser à subir le prochain impact";
            profile.counterplayLine = "Réponse conseillée : brise-garde, tenir la ligne, ralentissement ou exploitation d'ouverture après sa charge.";
            profile.strengths = {"poids", "impact frontal", "pression sur défense passive"};
            profile.weaknesses = {"ralentissement", "rupture de garde", "esquive préparée"};
            profile.reactions = {"s'énerve s'il est ralenti", "perd son rythme après une charge ratée", "devient dangereux quand il prend de l'élan"};
            profile.incomingAccuracyModifier = 13;
            profile.accuracyLine = "Sa masse laisse peu d'endroits où frapper le vide.";
        }
        else if (containsAny(combined, {"araignée", "araignee", "toile", "verrière", "verriere"}))
        {
            profile.archetype = "Araignée tactique";
            profile.behaviorLine = "Comportement araignée : elle ne cherche pas seulement à piquer, elle prépare le terrain avec toile, angle mort et patience.";
            profile.attackDescription = monster.getName() + " plante une patte, tend une toile courte et pique dans l'ouverture";
            profile.signatureMove = "toile courte qui peut affaiblir ou empoisonner selon l'espèce";
            profile.counterplayLine = "Réponse conseillée : feu, poussière, coup large ou observation avant de laisser la toile s'installer.";
            profile.strengths = {"toile", "poison possible", "angle mort"};
            profile.weaknesses = {"feu", "poussière", "pression directe"};
            profile.reactions = {"ralentit la cible", "change d'angle après une riposte", "devient forte si le joueur reste immobile"};
            profile.incomingAccuracyModifier = -8;
            profile.accuracyLine = "Ses pattes changent le centre du corps sans prévenir.";
        }
        else if (containsAny(combined, {"kobold"}))
        {
            profile.archetype = "Kobold nerveux";
            profile.behaviorLine = "Comportement kobold : il joue la peur comme une arme, recule, provoque, puis frappe quand la cible sous-estime son élan.";
            profile.attackDescription = monster.getName() + " bondit trop vite, presque paniqué, avec une pointe tenue à deux mains";
            profile.signatureMove = "petite feinte draconide qui peut ouvrir la cible au coup suivant";
            profile.counterplayLine = "Réponse conseillée : provocation, observation ou tenue de ligne pour l'empêcher de transformer sa panique en angle.";
            profile.strengths = {"nervosité", "petites feintes", "survie par recul"};
            profile.weaknesses = {"intimidation", "formation cassée", "pression stable"};
            profile.reactions = {"recule si dominé", "devient dangereux si ignoré", "cherche le flanc plutôt que le centre"};
            profile.incomingAccuracyModifier = -7;
            profile.accuracyLine = "Sa panique le rend irrégulier et difficile à cadrer.";
        }
        else if (containsAny(combined, {"frondeur", "archer", "tireur"}))
        {
            profile.archetype = "Harceleur à distance";
            profile.behaviorLine = "Comportement à distance : il n'a pas besoin de gagner le duel au corps à corps, il veut imposer une mauvaise position.";
            profile.attackDescription = monster.getName() + " garde l'écart et vise les mains, les sangles ou les yeux";
            profile.signatureMove = "tir de gêne qui peut créer une faille de prochain coup";
            profile.counterplayLine = "Réponse conseillée : casser la formation, protéger la ligne ou forcer un contact rapide.";
            profile.strengths = {"distance", "harcèlement", "ciblage des appuis"};
            profile.weaknesses = {"pression rapide", "couverture", "provocation"};
            profile.reactions = {"recule quand on avance", "punit les défenses immobiles", "aime tirer après un allié lourd"};
            profile.incomingAccuracyModifier = -5;
            profile.accuracyLine = "Il garde une distance sale, juste assez loin pour casser le geste.";
        }
        else if (containsAny(combined, {"lancier", "piquier", "hallebardier", "javelot", "vouge", "pertuisane"}))
        {
            profile.archetype = "Allonge prudente";
            profile.behaviorLine = "Comportement d'allonge : il ne veut pas entrer dans l'échange court, il veut garder une pointe entre lui et la menace.";
            profile.attackDescription = monster.getName() + " garde la pointe devant lui et frappe dès qu'un pas dépasse la ligne";
            profile.signatureMove = "pointe d'arrêt qui peut casser l'élan ou ouvrir le prochain coup";
            profile.counterplayLine = "Réponse conseillée : rompre l'allonge, poussière, balayage bas ou retrait contrôlé avant de revenir.";
            profile.strengths = {"distance", "pointe d'arrêt", "bonne punition des charges"};
            profile.weaknesses = {"pression de côté", "allonge rompue", "appuis bas"};
            profile.reactions = {"recule pour retrouver la pointe", "punit les lignes droites", "perd beaucoup si l'espace est fermé"};
            profile.incomingAccuracyModifier = -3;
            profile.accuracyLine = "Son corps est visible, mais l'arme force à traverser une mauvaise distance.";
        }
        else if (containsAny(combined, {"alchimiste", "renégat", "renegat", "apothicaire"}))
        {
            profile.archetype = "Alchimiste instable";
            profile.behaviorLine = "Comportement alchimiste : il transforme le combat en expérience, avec fioles ratées, résidus et effets parfois sales.";
            profile.attackDescription = monster.getName() + " frappe en brisant une petite fiole contre l'impact";
            profile.signatureMove = "fiole instable pouvant brûler, empoisonner ou affaiblir";
            profile.counterplayLine = "Réponse conseillée : pression rapide, observation, ou action qui le force à gaspiller ses préparations.";
            profile.strengths = {"statuts variés", "préparation", "imprévisibilité"};
            profile.weaknesses = {"faible sous pression", "peu stable au contact", "punitions fortes si son tour est cassé"};
            profile.reactions = {"change d'effet selon la fiole", "peut se soigner légèrement", "devient gênant si on lui laisse du temps"};
            profile.incomingAccuracyModifier = -4;
            profile.accuracyLine = "Il recule en protégeant ses fioles, sans tenir une vraie ligne.";
        }
        else if (containsAny(combined, {"fée", "fee", "ronces piquantes"}))
        {
            profile.archetype = "Fée des ronces";
            profile.behaviorLine = "Comportement féerique hostile : elle semble légère, mais elle piège la trajectoire avec des ronces et des détours trompeurs.";
            profile.attackDescription = monster.getName() + " dessine une courbe brillante avant que des ronces ne mordent la peau";
            profile.signatureMove = "ronces féeriques qui peuvent saigner ou rendre la cible vulnérable";
            profile.counterplayLine = "Réponse conseillée : observation, pression physique directe, feu contrôlé ou brise-garde avant qu'elle relance ses détours.";
            profile.strengths = {"mobilité magique", "ronces", "50% de résistance magique", "fausses trajectoires"};
            profile.weaknesses = {"50% de faiblesse physique", "feu", "pression directe", "lecture active"};
            profile.reactions = {"recule en riant", "punit les poursuites maladroites", "aime affaiblir avant de frapper"};
            profile.incomingAccuracyModifier = -14;
            profile.accuracyLine = "Son petit corps et ses détours féeriques compliquent les frappes directes.";
        }
        else if (containsAny(combined, {"spectre", "esprit", "serment brisé", "serment brise", "âme", "ame"}))
        {
            profile.archetype = "Esprit spectral";
            profile.behaviorLine = "Comportement spectral : il ne pousse pas le corps, il traverse le rythme et vise la volonté avant la chair.";
            profile.attackDescription = monster.getName() + " glisse dans la garde avec un froid qui semble arriver de derrière";
            profile.signatureMove = "frisson spectral qui peut geler ou fragiliser la prochaine défense";
            profile.counterplayLine = "Réponse conseillée : lumière, feu, observation ou rupture de rythme avant qu'il impose son tempo.";
            profile.strengths = {"froid", "angles irréels", "pression mentale"};
            profile.weaknesses = {"feu", "lumière", "rythme cassé"};
            profile.reactions = {"ignore la peur", "avance malgré les blessures", "devient lisible quand son tempo est compris"};
            profile.incomingAccuracyModifier = -10;
            profile.accuracyLine = "Le corps n'est jamais exactement là où le regard le place.";
        }
        else if (containsAny(combined, {"golem", "construction", "armure cabossée", "armure cabossee", "éclat de sentinelle", "eclat de sentinelle"}))
        {
            profile.archetype = "Construction à noyau";
            profile.behaviorLine = "Comportement construit : elle ne ressent pas la douleur comme une bête, elle protège sa faille mécanique et avance par séquences.";
            profile.attackDescription = monster.getName() + " frappe par séquence lourde, puis referme ses plaques autour du noyau";
            profile.signatureMove = "coup mécanique suivi d'une protection courte du noyau";
            profile.counterplayLine = "Réponse conseillée : brise-garde, observation du noyau, choc, ou attaque préparée après ouverture.";
            profile.strengths = {"armure", "noyau protégé", "régularité"};
            profile.weaknesses = {"choc", "brise-garde", "attaque précise sur mécanisme"};
            profile.reactions = {"se verrouille", "perd beaucoup quand sa posture saute", "répète ses séquences"};
            profile.incomingAccuracyModifier = 12;
            profile.accuracyLine = "Lourd et séquencé, il offre une cible large malgré sa garde.";
        }
        else if (containsAny(combined, {"dragon", "dragonnet", "draconide", "écailles", "ecailles"}))
        {
            profile.archetype = "Sang draconique";
            profile.behaviorLine = "Comportement draconique : même jeune ou affaibli, il mélange orgueil, souffle et pression physique.";
            profile.attackDescription = monster.getName() + " accompagne le coup d'un souffle court et d'un poids ancien";
            profile.signatureMove = "souffle court qui peut brûler, choquer ou renforcer l'impact";
            profile.counterplayLine = "Réponse conseillée : garde élémentaire, observation, faille de prochain coup ou rupture de posture.";
            profile.strengths = {"souffle", "écailles", "impact lourd"};
            profile.weaknesses = {"garde élémentaire", "failles ciblées", "contrôle avant souffle"};
            profile.reactions = {"punit les cibles immobiles", "protège ses écailles faibles", "devient dangereux si ignoré"};
            profile.incomingAccuracyModifier = 9;
            profile.accuracyLine = "La taille draconique aide à toucher, pas forcément à blesser au bon endroit.";
        }
        else if (containsAny(combined, {"ange", "paladin", "sacré", "sacre"}))
        {
            profile.archetype = "Serment sacré tordu";
            profile.behaviorLine = "Comportement sacré corrompu : il garde une discipline noble, mais chaque geste ressemble à un jugement mal dirigé.";
            profile.attackDescription = monster.getName() + " abat une frappe droite, presque cérémonielle, qui cherche à juger la faille";
            profile.signatureMove = "jugement bref qui peut protéger le porteur ou rendre la cible vulnérable";
            profile.counterplayLine = "Réponse conseillée : casser la formation, brise-garde, ou le forcer à perdre son rythme solennel.";
            profile.strengths = {"discipline", "garde courte", "pression de jugement"};
            profile.weaknesses = {"rupture de posture", "actions sournoises", "pression continue"};
            profile.reactions = {"se protège après avoir jugé", "déteste les interruptions", "garde un rythme très lisible"};
            profile.incomingAccuracyModifier = 3;
            profile.accuracyLine = "Sa discipline rend ses lignes lisibles quand on ose avancer.";
        }
        else if (containsAny(combined, {"piégeur", "piegeur", "trappeur", "filet", "collet", "chasseur de primes", "chasseur-de-primes"}))
        {
            profile.archetype = "Piégeur de terrain";
            profile.behaviorLine = "Comportement piégeur : il ne cherche pas le coup parfait, il force un mauvais pas puis frappe quand la ligne est déjà prise.";
            profile.attackDescription = monster.getName() + " lance un crochet bas avant d'avancer";
            profile.signatureMove = "collet court qui peut affaiblir ou préparer une faille";
            profile.counterplayLine = "Réponse conseillée : retrait contrôlé, observation active ou marquage avant de traverser son espace.";
            profile.strengths = {"contrôle du terrain", "mauvais appuis", "pression sur cibles pressées"};
            profile.weaknesses = {"cibles patientes", "retrait contrôlé", "attaque préparée"};
            profile.reactions = {"punit les pas trop directs", "peut laisser un collet gênant", "perd beaucoup si la distance est cassée proprement"};
            profile.incomingAccuracyModifier = -4;
            profile.accuracyLine = "Il ne bouge pas vite, mais oblige souvent l'arme à passer par un angle sale.";
        }
        else if (containsAny(combined, {"duelliste", "escrimeur", "escrimeuse", "sabreur", "lame vive", "lame-vive", "maître lame", "maitre lame"}))
        {
            profile.archetype = "Duelliste nerveux";
            profile.behaviorLine = "Comportement de duel : il lit la main, provoque une réponse, puis prend le poignet au lieu de chercher la force brute.";
            profile.attackDescription = monster.getName() + " feinte du poignet avant de chercher l'ouverture";
            profile.signatureMove = "feinte de lame qui peut préparer le prochain impact";
            profile.counterplayLine = "Réponse conseillée : tenir la ligne, brise-garde ou marquer la proie avant de répondre.";
            profile.strengths = {"feinte", "précision", "punition des gestes pressés"};
            profile.weaknesses = {"garde stable", "pression de corps", "failles de fatigue"};
            profile.reactions = {"transforme parfois une esquive en angle", "aime les duels isolés", "n'aime pas être forcé en défense"};
            profile.incomingAccuracyModifier = -8;
            profile.accuracyLine = "Le corps reste visible, mais la lame attire l'oeil au mauvais endroit.";
        }
        else if (containsAny(combined, {"champignon", "spore", "fongique", "mycélium", "mycelium", "moisissure", "mousse vivante"}))
        {
            profile.archetype = "Champignon sporifère";
            profile.behaviorLine = "Comportement fongique : il avance peu, mais remplit l'air de spores et rend chaque respiration moins sûre.";
            profile.attackDescription = monster.getName() + " secoue une poche de spores avant l'impact";
            profile.signatureMove = "nuage de spores qui affaiblit ou empoisonne lentement";
            profile.counterplayLine = "Réponse conseillée : feu, retrait contrôlé, lanterne au sol ou frappe rapide avant que l'air devienne lourd.";
            profile.strengths = {"spores", "pression lente", "zone pénible"};
            profile.weaknesses = {"feu", "coup net", "distance reprise"};
            profile.reactions = {"peut relâcher des spores au contact", "affaiblit les combats longs", "supporte mal une brûlure propre"};
            profile.incomingAccuracyModifier = 3;
            profile.accuracyLine = "Le corps est lent et lisible, mais l'air autour de lui gêne la respiration.";
        }
        else if (containsAny(combined, {"cristal", "cristallin", "gemme", "verre", "miroir", "éclat", "eclat"}))
        {
            profile.archetype = "Cristal résonant";
            profile.behaviorLine = "Comportement cristallin : il encaisse par facettes, puis renvoie une vibration sèche si le coup arrive mal.";
            profile.attackDescription = monster.getName() + " frappe avec une vibration de cristal";
            profile.signatureMove = "résonance qui peut choquer ou verrouiller la matière";
            profile.counterplayLine = "Réponse conseillée : viser une fissure, casser la garde ou éviter les petits impacts répétés.";
            profile.strengths = {"matière dure", "résonance", "bonne tenue aux coups faibles"};
            profile.weaknesses = {"fissures", "impact lourd", "faille de corps"};
            profile.reactions = {"peut verrouiller ses facettes", "peut renvoyer un choc court", "cède mieux sous un angle préparé"};
            profile.incomingAccuracyModifier = 4;
            profile.accuracyLine = "Les facettes sont visibles, mais la bonne fissure demande un geste précis.";
        }
        else if (containsAny(combined, {"bannière", "banniere", "porte-étendard", "porte etendard", "étendard", "etendard", "champion", "porte-drapeau", "porte drapeau"}))
        {
            profile.archetype = "Champion de bannière";
            profile.behaviorLine = "Comportement d'étendard : il transforme sa présence en courage, avance proprement et pousse la ligne autour de lui.";
            profile.attackDescription = monster.getName() + " frappe en ramenant l'étendard dans l'axe de la mêlée";
            profile.signatureMove = "cri d'étendard qui peut relever précision, garde ou puissance courte";
            profile.counterplayLine = "Réponse conseillée : le marquer, forcer une chaîne d'états ou le séparer des alliés qui profitent de sa présence.";
            profile.strengths = {"présence de groupe", "buff court", "bonne reprise après blessure"};
            profile.weaknesses = {"isolement", "proie marquée", "pression continue"};
            profile.reactions = {"resserre sa ligne quand il respire", "garde le moral malgré les blessures", "perd beaucoup si l'étendard tombe"};
            profile.incomingAccuracyModifier = 1;
            profile.accuracyLine = "L'étendard le rend visible, mais ses alliés compliquent l'approche directe.";
        }
        else if (containsAny(combined, {"transfuseur", "transfusion", "sangsue", "sanglier-mage", "suture", "liant de sang", "mèche de sang", "meche de sang"}))
        {
            profile.archetype = "Transfuseur de souffle";
            profile.behaviorLine = "Comportement de transfert : il préfère voler un souffle court ou déplacer la fatigue plutôt que gagner par un seul impact.";
            profile.attackDescription = monster.getName() + " cherche une prise vivante pour tirer un souffle sale";
            profile.signatureMove = "transfert court qui peut voler un peu de vigueur ou rendre sa garde plus stable";
            profile.counterplayLine = "Réponse conseillée : voile d'urgence, signe de vigueur, rupture d'ancrage ou attaque rapide avant qu'il pose sa prise.";
            profile.strengths = {"vol de souffle", "survie lente", "profite des plaies"};
            profile.weaknesses = {"protection courte", "absence de blessure", "choc rapide"};
            profile.reactions = {"récupère parfois un souffle", "cherche les cibles déjà blessées", "devient fragile si son lien est coupé"};
            profile.incomingAccuracyModifier = -2;
            profile.accuracyLine = "Il se penche vers les plaies et change d'angle avec les respirations adverses.";
        }
        else if (containsAny(combined, {"rune", "runique", "gardien runique", "sceau", "glyph", "glyphe", "graveur", "inscription vivante"}))
        {
            profile.archetype = "Garde runique";
            profile.behaviorLine = "Comportement runique : ses gestes sont lents, mais chaque symbole protège une partie de son corps ou prépare une réponse.";
            profile.attackDescription = monster.getName() + " abat un coup lourd guidé par des signes gravés";
            profile.signatureMove = "glyphe défensif qui peut donner garde, résistance ou choc court";
            profile.counterplayLine = "Réponse conseillée : étouffer la concentration, viser une faille de corps ou rompre l'ancrage occulte.";
            profile.strengths = {"garde magique", "résistance courte", "contre élémentaire"};
            profile.weaknesses = {"glyphes fissurés", "concentration brisée", "coup précis"};
            profile.reactions = {"verrouille parfois une rune", "renvoie une vibration", "perd sa tenue si le symbole est ciblé"};
            profile.incomingAccuracyModifier = 3;
            profile.accuracyLine = "Les glyphes dessinent des points visibles, mais tous ne sont pas de vraies failles.";
        }
        else if (containsAny(combined, {"illusion", "illusionniste", "mirage", "miroir", "brume-miroir", "brume miroir", "farceur arcanique"}))
        {
            profile.archetype = "Illusionniste de brume";
            profile.behaviorLine = "Comportement d'illusion : il ne cherche pas toujours à toucher, il cherche surtout à faire frapper le mauvais angle.";
            profile.attackDescription = monster.getName() + " laisse une image trembler avant que le vrai coup n'arrive";
            profile.signatureMove = "fausse ouverture qui peut affaiblir la précision ou préparer une faille";
            profile.counterplayLine = "Réponse conseillée : observation active, image trompeuse en retour, lire la menace ou frapper large.";
            profile.strengths = {"mirages", "mauvais angles", "bonne fuite de regard"};
            profile.weaknesses = {"observation", "zone large", "pression constante"};
            profile.reactions = {"peut détourner un impact", "laisse souvent une trace si le mirage casse", "déteste les attaques qui couvrent plusieurs axes"};
            profile.incomingAccuracyModifier = -12;
            profile.accuracyLine = "La silhouette n'est pas toujours la bonne ; viser demande de lire le souffle, pas seulement l'image.";
        }
        else if (containsAny(combined, {"dresseur", "dompteur", "maître de bêtes", "maitre de betes", "berger noir", "chef de bêtes", "chef de betes"}))
        {
            profile.archetype = "Dompteur de ligne";
            profile.behaviorLine = "Comportement de dompteur : il parle aux bêtes par gestes courts et transforme leur instinct en ordre simple.";
            profile.attackDescription = monster.getName() + " frappe en gardant une main prête à guider la bête voisine";
            profile.signatureMove = "ordre animal qui peut donner précision ou mauvaise retraite aux alliés compatibles";
            profile.counterplayLine = "Réponse conseillée : couper le signal, marquer la proie ou séparer le dompteur de ses bêtes.";
            profile.strengths = {"coordination de bêtes", "ordre court", "pression de poursuite"};
            profile.weaknesses = {"isolement", "signal coupé", "ligne sans animal compatible"};
            profile.reactions = {"resserre les bêtes proches", "panique moins qu'un simple bandit", "perd beaucoup si sa cible disparaît"};
            profile.incomingAccuracyModifier = -3;
            profile.accuracyLine = "Il reste lisible, mais ses bêtes coupent souvent la trajectoire directe.";
        }
        else if (containsAny(combined, {"moine", "pugiliste", "boxeur", "maître martial", "maitre martial", "poing nu", "ascète", "ascete"}))
        {
            profile.archetype = "Moine martial";
            profile.behaviorLine = "Comportement martial : il préfère le rythme, les coudes et les appuis aux grandes armes visibles.";
            profile.attackDescription = monster.getName() + " entre court avec un enchaînement de coude et de genou";
            profile.signatureMove = "enchaînement de souffle qui peut donner précision et garde courte";
            profile.counterplayLine = "Réponse conseillée : garder la distance, coup en arc, retrait contrôlé ou brise-garde si le souffle est lu.";
            profile.strengths = {"appuis", "enchaînement", "bonne reprise après esquive"};
            profile.weaknesses = {"allonge", "terrain gêné", "souffle cassé"};
            profile.reactions = {"transforme parfois un raté en contre court", "supporte mieux les petits chocs", "déteste être fixé par une marque"};
            profile.incomingAccuracyModifier = -7;
            profile.accuracyLine = "Le corps reste proche, mais les appuis changent juste avant l'impact.";
        }
        else if (containsAny(combined, {"forgeron de guerre", "forgeur", "marteleur", "enclume", "bricoleur lourd", "mécano", "mecano"}))
        {
            profile.archetype = "Forgeur de guerre";
            profile.behaviorLine = "Comportement d'atelier : il améliore ce qu'il a sous la main, même si la réparation ne dure qu'un souffle.";
            profile.attackDescription = monster.getName() + " frappe avec une pièce renforcée à la hâte";
            profile.signatureMove = "retouche de terrain qui peut donner garde ou transformer un petit impact en vrai choc";
            profile.counterplayLine = "Réponse conseillée : étouffer la concentration, bombe d'atelier ou viser une faille de corps avant que la pièce tienne.";
            profile.strengths = {"réparation courte", "matière", "choc lourd"};
            profile.weaknesses = {"concentration cassée", "jointures", "poudre instable"};
            profile.reactions = {"resserre une plaque", "récupère de la tenue si ignoré", "ouvre parfois une fissure quand la pièce saute"};
            profile.incomingAccuracyModifier = 2;
            profile.accuracyLine = "Il est assez large pour être touché, mais l'outil peut dévier le bon angle.";
        }
        else if (containsAny(combined, {"oracle", "devin", "voyant", "augure", "prophète", "prophete", "lecteur de signes"}))
        {
            profile.archetype = "Oracle hésitant";
            profile.behaviorLine = "Comportement d'augure : il anticipe par fragments, mais chaque vision le force à choisir trop tôt.";
            profile.attackDescription = monster.getName() + " suit un signe bref avant de frapper là où tu allais être";
            profile.signatureMove = "présage court qui peut donner précision ou révéler une mauvaise position";
            profile.counterplayLine = "Réponse conseillée : pas de l'ombre, image trompeuse, retrait contrôlé ou changement brutal de rythme.";
            profile.strengths = {"anticipation", "précision", "lecture des gestes répétés"};
            profile.weaknesses = {"imprévisible", "mirage", "rythme cassé"};
            profile.reactions = {"punit la répétition", "rate davantage si le joueur change d'axe", "devient fragile quand sa vision est forcée"};
            profile.incomingAccuracyModifier = -4;
            profile.accuracyLine = "Il anticipe certains gestes, mais son corps ne suit pas toujours la vision.";
        }
        else if (containsAny(combined, {"tireur", "archer", "arbalétrier", "arbaletrier", "frondeur", "soutien à distance", "soutien distance", "couverture"}))
        {
            profile.archetype = "Tireur de couverture";
            profile.behaviorLine = "Comportement de couverture : il ne cherche pas toujours le gros tir, il force surtout les mauvais déplacements.";
            profile.attackDescription = monster.getName() + " tire dans la trajectoire plutôt que dans la cible évidente";
            profile.signatureMove = "tir de couverture qui peut marquer, gêner ou ouvrir le prochain angle";
            profile.counterplayLine = "Réponse conseillée : rompre l'allonge, pas de l'ombre, retrait contrôlé ou coup en arc si la ligne est compacte.";
            profile.strengths = {"contrôle de couloir", "pression à distance", "punit les fuites droites"};
            profile.weaknesses = {"distance fermée", "pas imprévisible", "pression de groupe"};
            profile.reactions = {"repositionne après un tir", "garde les cibles blessées dans son angle", "déteste perdre l'allonge"};
            profile.incomingAccuracyModifier = -5;
            profile.accuracyLine = "Il reste mobile et garde toujours un obstacle ou un allié dans la ligne.";
        }
        else if (containsAny(combined, {"barde", "tambour", "tambourin", "clairon", "chant de guerre", "chanteur", "chanteuse", "cornemuse"}))
        {
            profile.archetype = "Barde de guerre";
            profile.behaviorLine = "Comportement de ralliement : il ne domine pas par la force, mais par le tempo qu'il donne aux autres.";
            profile.attackDescription = monster.getName() + " frappe au rythme d'un chant court qui relève la ligne";
            profile.signatureMove = "appel de tempo qui peut renforcer sa précision ou sa garde";
            profile.counterplayLine = "Réponse conseillée : étouffer la concentration, couper le signal ou le forcer au contact avant que le rythme s'installe.";
            profile.strengths = {"tempo", "soutien", "précision de groupe"};
            profile.weaknesses = {"pression directe", "silence", "concentration brisée"};
            profile.reactions = {"gagne en précision s'il respire", "protège mieux les alliés proches", "perd beaucoup si son rythme casse"};
            profile.incomingAccuracyModifier = -3;
            profile.accuracyLine = "Il bouge au rythme de son propre signal, ce qui rend les gestes un peu moins simples à caler.";
        }
        else if (containsAny(combined, {"pyromane", "bombardier", "grenadier", "bombe", "explosif", "poudre noire", "artificier ennemi"}))
        {
            profile.archetype = "Bombardier instable";
            profile.behaviorLine = "Comportement explosif : il aime transformer un mauvais angle en bruit, fumée et panique.";
            profile.attackDescription = monster.getName() + " secoue une charge courte avant de la jeter";
            profile.signatureMove = "charge instable qui peut brûler, choquer ou blesser plusieurs appuis";
            profile.counterplayLine = "Réponse conseillée : retrait contrôlé, étouffer la concentration ou frapper avant que la charge parte.";
            profile.strengths = {"zone", "brûlure", "pression explosive"};
            profile.weaknesses = {"mauvaise concentration", "approche rapide", "retour de flamme"};
            profile.reactions = {"peut laisser une brûlure", "peut se rendre vulnérable si la charge claque mal", "déteste les coups qui cassent le lancer"};
            profile.incomingAccuracyModifier = 2;
            profile.accuracyLine = "Les charges le rendent lisible, mais personne n'aime entrer dans l'angle de poudre.";
        }
        else if (containsAny(combined, {"danseur", "danseuse", "acrobate", "voltigeur", "voltigeuse", "lame dansante", "pas léger"}))
        {
            profile.archetype = "Danseur d'esquive";
            profile.behaviorLine = "Comportement d'esquive : il gagne du temps avec ses appuis, puis frappe dans le décalage plutôt que dans le duel direct.";
            profile.attackDescription = monster.getName() + " tourne autour du coup avant de rentrer par le côté";
            profile.signatureMove = "pas glissé qui peut donner précision ou ouvrir un mauvais angle";
            profile.counterplayLine = "Réponse conseillée : balayage bas, marquer la proie ou tenir la ligne pour lui retirer l'espace.";
            profile.strengths = {"esquive", "mobilité", "précision après déplacement"};
            profile.weaknesses = {"balayage", "terrain gêné", "proie marquée"};
            profile.reactions = {"punit les attaques verticales", "perd son rythme si les jambes sont prises", "supporte mal les zones sales"};
            profile.incomingAccuracyModifier = -10;
            profile.accuracyLine = "Le corps existe, mais il arrive rarement là où l'oeil le plaçait.";
        }
        else if (containsAny(combined, {"totem", "porte-totem", "porte totem", "idolâtre", "idolatre", "chaman", "chamane", "shaman"}))
        {
            profile.archetype = "Porte-totem";
            profile.behaviorLine = "Comportement rituel : il ancre le combat autour d'un symbole, protège son souffle et donne une couleur étrange aux coups.";
            profile.attackDescription = monster.getName() + " frappe en tirant un rythme lourd depuis son totem";
            profile.signatureMove = "totem court qui peut garder, régénérer ou charger une petite magie";
            profile.counterplayLine = "Réponse conseillée : rompre l'ancrage occulte, étouffer la concentration ou viser la faille de corps.";
            profile.strengths = {"ancrage", "soutien rituel", "garde élémentaire"};
            profile.weaknesses = {"ancrage brisé", "concentration coupée", "pression avant le rituel"};
            profile.reactions = {"resserre sa garde autour du totem", "peut récupérer un souffle court", "devient moins dangereux si le symbole tremble"};
            profile.incomingAccuracyModifier = 1;
            profile.accuracyLine = "Le totem le rend plus stable qu'agile, mais son aura brouille parfois le bon angle.";
        }
        else if (containsAny(combined, {"soigneur", "soigneuse", "guérisseur", "guerisseur", "prêtre", "pretre", "prêtresse", "pretresse", "médic", "medic"}))
        {
            profile.archetype = "Soigneur de fortune";
            profile.behaviorLine = "Comportement de soutien : il protège les blessés, retarde la chute de la ligne et déteste être forcé au contact.";
            profile.attackDescription = monster.getName() + " frappe en gardant une main prête à refermer une plaie";
            profile.signatureMove = "soin court ou garde légère si on lui laisse respirer";
            profile.counterplayLine = "Réponse conseillée : le marquer vite, couper son signal ou le forcer à perdre ses mains de soutien.";
            profile.strengths = {"soutien", "soin court", "garde des alliés blessés"};
            profile.weaknesses = {"pression directe", "proie marquée", "signal coupé", "faible sous poursuite"};
            profile.reactions = {"reprend son souffle", "protège une blessure", "panique si la ligne tombe trop vite"};
            profile.incomingAccuracyModifier = -2;
            profile.accuracyLine = "Il reste derrière les autres, mais ses gestes de soin le rendent lisible quand il agit.";
        }
        else if (containsAny(combined, {"porte-bouclier", "porte bouclier", "bouclier", "rempart", "phalange", "mur de fer"}))
        {
            profile.archetype = "Porte-bouclier";
            profile.behaviorLine = "Comportement de rempart : il cherche moins à tuer vite qu'à casser l'élan, couvrir les alliés et punir les coups trop frontaux.";
            profile.attackDescription = monster.getName() + " pousse derrière son bouclier avant de frapper court";
            profile.signatureMove = "pression de bouclier qui peut fermer une faille et affaiblir le bras";
            profile.counterplayLine = "Réponse conseillée : brise-garde, faille de corps, contourner la formation ou forcer une chaîne d'états.";
            profile.strengths = {"garde", "couverture", "résistance frontale"};
            profile.weaknesses = {"brise-garde", "attaque de flanc", "failles de corps"};
            profile.reactions = {"resserre la garde", "protège la ligne", "punit les impacts trop faibles"};
            profile.incomingAccuracyModifier = 4;
            profile.accuracyLine = "Le bouclier est large : toucher est simple, trouver une vraie ouverture l'est moins.";
        }
        else if (containsAny(combined, {"berserker", "furieux", "enragé", "enrage", "frénétique", "frenetique", "rageur"}))
        {
            profile.archetype = "Berserker blessé";
            profile.behaviorLine = "Comportement enragé : plus il saigne, plus il cherche l'échange court, quitte à exposer son corps.";
            profile.attackDescription = monster.getName() + " transforme la douleur en coup trop large pour être propre";
            profile.signatureMove = "frappe de rage plus forte quand ses PV tombent";
            profile.counterplayLine = "Réponse conseillée : l'affaiblir, reculer proprement ou marquer la proie avant son élan.";
            profile.strengths = {"rage", "pression à bas PV", "gros échanges"};
            profile.weaknesses = {"affaiblissement", "retrait contrôlé", "proie marquée"};
            profile.reactions = {"s'ouvre en attaquant", "déteste perdre sa cible", "peut frapper plus fort blessé"};
            profile.incomingAccuracyModifier = 10;
            profile.accuracyLine = "Son élan expose beaucoup de corps, même si le coup qui revient est dangereux.";
        }
        else if (containsAny(combined, {"maudit", "maudite", "malédiction", "malediction", "occultiste", "hex", "sorcenoir"}))
        {
            profile.archetype = "Porte-malédiction";
            profile.behaviorLine = "Comportement occulte : il préfère salir le rythme, laisser une marque et attendre que la blessure devienne une erreur.";
            profile.attackDescription = monster.getName() + " trace un signe noir dans l'air avant que le coup n'arrive";
            profile.signatureMove = "marque occulte qui affaiblit ou ouvre le prochain impact";
            profile.counterplayLine = "Réponse conseillée : observation, pression rapide, garde élémentaire ou retrait contrôlé avant que la marque s'installe.";
            profile.strengths = {"affaiblissement", "failles retardées", "pression mentale"};
            profile.weaknesses = {"pression rapide", "garde élémentaire", "observation"};
            profile.reactions = {"laisse des marques", "punit les combats longs", "craint d'être forcé au contact"};
            profile.incomingAccuracyModifier = -3;
            profile.accuracyLine = "Ses gestes sont petits, mais les marques dans l'air trahissent parfois sa position.";
        }
        else if (containsAny(combined, {"voleur", "roublard", "brigand", "assassin", "bandit", "détrousseur", "detrousseur", "pillard"}))
        {
            profile.archetype = "Profil sournois / voleur";
            profile.behaviorLine = "Comportement sournois : il ne cherche pas le duel propre, il lit les sacs ouverts, les failles et les mauvais appuis.";
            profile.attackDescription = monster.getName() + " feinte le coup frontal puis cherche une ouverture basse";
            profile.signatureMove = "feinte basse qui prépare une faille pour le prochain vrai coup";
            profile.counterplayLine = "Réponse conseillée : observation, provocation ou tenue de ligne pour lui retirer ses angles sales.";
            profile.strengths = {"feintes", "exploitation des cibles blessées", "petites failles"};
            profile.weaknesses = {"provocation", "observation active", "pression de zone"};
            profile.reactions = {"peut rendre vulnérable", "profite du poison ou du saignement", "devient plus dangereux si le joueur est bas PV"};
            profile.incomingAccuracyModifier = -8;
            profile.accuracyLine = "Il ne reste jamais face au coup, seulement dans l'angle d'après.";
        }
        else if (containsAny(combined, {"gobelin", "taxeur", "cueilleur de poches"}))
        {
            profile.archetype = "Gobelin opportuniste";
            profile.behaviorLine = "Comportement gobelin : il teste, recule, revient quand la cible semble faible et cherche rarement un échange loyal.";
            profile.attackDescription = monster.getName() + " attaque vite, comme s'il voulait voler le tour autant que les poches";
            profile.signatureMove = "coup opportuniste plus dangereux sur une cible blessée";
            profile.counterplayLine = "Réponse conseillée : tenir la ligne, casser sa formation ou l'intimider avant qu'il sente la faiblesse.";
            profile.strengths = {"opportunisme", "pression sur cible blessée", "coordination sale en groupe"};
            profile.weaknesses = {"tenue de ligne", "formation cassée", "intimidation"};
            profile.reactions = {"punit les PV bas", "aime les formations de fortune", "s'énerve quand le plan échoue"};
            profile.incomingAccuracyModifier = -6;
            profile.accuracyLine = "Petit et nerveux, il fait rater les gestes trop propres.";
        }
        else if (containsAny(combined, {"orc", "hobgobelin", "brute", "colosse"}))
        {
            profile.archetype = "Brute lourde";
            profile.behaviorLine = "Comportement brutal : peu subtil, mais il transforme chaque blessure en raison de frapper plus fort.";
            profile.attackDescription = monster.getName() + " abat un coup lourd qui cherche à traverser la garde";
            profile.signatureMove = "impact de rage renforcé quand il est blessé";
            profile.counterplayLine = "Réponse conseillée : affaiblissement, brise-garde ou ralentissement avant son gros coup.";
            profile.strengths = {"force brute", "rage quand blessé", "pression directe"};
            profile.weaknesses = {"affaiblissement", "esquive préparée", "brise-garde avant son gros coup"};
            profile.reactions = {"devient plus violent blessé", "punit les défenses passives", "déteste être ralenti"};
            profile.incomingAccuracyModifier = 8;
            profile.accuracyLine = "Il est large, frontal et moins dur à atteindre qu'à arrêter.";
        }
        else if (containsAny(combined, {"loup", "meute", "chien sauvage", "canidé", "canide"}))
        {
            profile.archetype = "Chasseur de meute";
            profile.behaviorLine = "Comportement de meute : il teste la ligne, tourne autour des blessés et frappe rarement seul dans son intention.";
            profile.attackDescription = monster.getName() + " coupe la retraite avant de mordre dans l'angle laissé par la meute";
            profile.signatureMove = "morsure de poursuite plus dure si la cible saigne, faiblit ou tombe bas en vie";
            profile.counterplayLine = "Réponse conseillée : provocation, tenir la ligne, feu au sol ou casser la formation pour briser la poursuite.";
            profile.strengths = {"poursuite", "pression sur blessés", "coordination de groupe"};
            profile.weaknesses = {"provocation", "tenue de ligne", "formation brisée"};
            profile.reactions = {"sent le sang", "change de cible si la ligne craque", "recule mal face à une garde stable"};
            profile.incomingAccuracyModifier = -4;
            profile.accuracyLine = "Il reste mobile, mais sa trajectoire de chasse se lit mieux qu'un nuisible minuscule.";
        }
        else if (containsAny(combined, {"prédateur", "predateur", "bête", "bete", "renard", "sangsue", "moustique"}))
        {
            profile.archetype = "Prédateur";
            profile.behaviorLine = "Comportement prédateur : il attend une odeur de faiblesse, puis force la poursuite au lieu d'échanger honnêtement.";
            profile.attackDescription = monster.getName() + " bondit vers la blessure la plus visible";
            profile.signatureMove = "attaque de chasse qui mord plus fort sur cible basse en PV";
            profile.counterplayLine = "Réponse conseillée : provocation, garde de groupe ou contrôle court pour briser la poursuite.";
            profile.strengths = {"vitesse", "chasse aux cibles basses PV", "pression continue"};
            profile.weaknesses = {"provocation", "garde de groupe", "contrôle court"};
            profile.reactions = {"devient dangereux sur cible blessée", "réagit aux saignements", "évite les échanges trop propres"};
            profile.incomingAccuracyModifier = -5;
            profile.accuracyLine = "La vitesse de chasse rend la cible mouvante, sans la rendre minuscule.";
        }
        else if (containsAny(combined, {"nécromancien", "necromancien", "nécro", "necro", "tombe", "crypte", "ossement", "ossements", "fossoyeur"}))
        {
            profile.archetype = "Ancre nécrotique";
            profile.behaviorLine = "Comportement nécrotique : il ne frappe pas seulement le corps, il accroche l'air froid autour de la cible.";
            profile.attackDescription = monster.getName() + " tire un fil froid depuis les os ou la terre avant de frapper";
            profile.signatureMove = "ancrage de tombe qui peut geler le tempo ou ouvrir une faille";
            profile.counterplayLine = "Réponse conseillée : rompre l'ancrage occulte, feu court, lumière ou pression avant que le fil se tende.";
            profile.strengths = {"ancrage froid", "pression longue", "marques de tombe"};
            profile.weaknesses = {"feu", "lumière", "ancrage rompu", "garde élémentaire"};
            profile.reactions = {"cherche à fixer la cible", "devient pénible en combat long", "perd du rythme si le lien casse"};
            profile.incomingAccuracyModifier = -2;
            profile.accuracyLine = "Le corps se voit, mais le danger vient surtout du fil froid qui le précède.";
        }
        else if (containsAny(combined, {"vampire", "sangsue", "drain", "buveur", "buveuse", "parasite"}))
        {
            profile.archetype = "Drain de vie";
            profile.behaviorLine = "Comportement drainant : il vise les plaies, garde le contact et cherche à reprendre du souffle sur la blessure adverse.";
            profile.attackDescription = monster.getName() + " cherche une plaie chaude avant de refermer la distance";
            profile.signatureMove = "morsure ou contact drainant qui peut affaiblir la cible";
            profile.counterplayLine = "Réponse conseillée : retrait contrôlé, rupture d'ancrage, feu ou marquer la proie avant qu'elle recolle.";
            profile.strengths = {"pression sur blessés", "récupération courte", "contact dangereux"};
            profile.weaknesses = {"feu", "distance rompue", "faible si privé de plaie"};
            profile.reactions = {"profite du saignement", "déteste perdre le contact", "reprend parfois un souffle sale"};
            profile.incomingAccuracyModifier = -4;
            profile.accuracyLine = "Il bouge autour de la plaie qu'il veut rouvrir.";
        }
        else if (containsAny(combined, {"mort-vivant", "mort vivant", "spectre", "zombie", "squelette", "âme", "ame"}))
        {
            profile.archetype = "Mort-vivant";
            profile.behaviorLine = "Comportement mort-vivant : peu de peur, peu de douleur, mais un rythme froid et prévisible si on l'observe.";
            profile.attackDescription = monster.getName() + " frappe avec une inertie froide, comme si le coup arrivait trop tard pour être naturel";
            profile.signatureMove = "coup froid au tempo rigide, souvent lisible après observation";
            profile.counterplayLine = "Réponse conseillée : feu, lumière ou rupture de rythme.";
            profile.strengths = {"pression froide", "insensible à la peur", "avance régulière"};
            profile.weaknesses = {"lumière", "feu", "rupture de rythme"};
            profile.reactions = {"avance malgré les blessures", "peut laisser du froid", "garde un tempo rigide"};
            profile.incomingAccuracyModifier = 4;
            profile.accuracyLine = "Son tempo raide donne des fenêtres assez prévisibles.";
        }
        else if (containsAny(combined, {"plante", "liane", "ronce", "racine", "champignon", "mousse"}))
        {
            profile.archetype = "Plante / fongique";
            profile.behaviorLine = "Comportement végétal : elle contrôle l'espace, accroche les jambes et transforme le terrain en arme.";
            profile.attackDescription = monster.getName() + " fouette avec des fibres et cherche à ancrer la cible";
            profile.signatureMove = "entrave de terrain qui gêne les appuis";
            profile.counterplayLine = "Réponse conseillée : feu, coupe nette, déplacement forcé ou pression avant enracinement.";
            profile.strengths = {"contrôle du terrain", "entrave", "pression lente"};
            profile.weaknesses = {"feu", "coupe nette", "déplacement forcé"};
            profile.reactions = {"accroche", "ralentit", "protège parfois ses racines"};
            profile.incomingAccuracyModifier = 4;
            profile.accuracyLine = "Ses racines bougent peu, même quand les lianes fouettent.";
        }
        else if (containsAny(combined, {"crabe", "tortue", "scorpion", "carapace", "coquille", "cloporte"}))
        {
            profile.archetype = "Carapace basse";
            profile.behaviorLine = "Comportement à carapace : il reste bas, protège son ventre et oblige à chercher le bon angle plutôt que de taper au hasard.";
            profile.attackDescription = monster.getName() + " avance sous sa carapace puis frappe par dessous";
            profile.signatureMove = "pince, dard ou charge basse qui profite d'un coup mal placé";
            profile.counterplayLine = "Réponse conseillée : retourner la garde avec brise-garde, poussière, choc ou attaque précise sur les jointures.";
            profile.strengths = {"carapace", "profil bas", "jointures difficiles"};
            profile.weaknesses = {"ventre exposable", "choc", "jointures après observation"};
            profile.reactions = {"se referme si le coup glisse", "punit les attaques trop hautes", "perd beaucoup si la carapace est retournée"};
            profile.incomingAccuracyModifier = -7;
            profile.accuracyLine = "Bas et couvert, il donne peu de matière utile à l'arme.";
        }
        else if (containsAny(combined, {"insectoïde", "insectoide", "scarabée", "scarabee", "frelon", "araignée", "araignee"}))
        {
            profile.archetype = "Insectoïde";
            profile.behaviorLine = "Comportement insectoïde : il pique, recule, revient par angle mort et vise surtout les zones mal protégées.";
            profile.attackDescription = monster.getName() + " pique dans un angle étroit, presque impossible à couvrir parfaitement";
            profile.signatureMove = "piqûre d'angle mort qui peut faire saigner";
            profile.counterplayLine = "Réponse conseillée : poussière, zone large ou garde stable.";
            profile.strengths = {"angles morts", "piqûres", "pression nerveuse"};
            profile.weaknesses = {"zone large", "poussière", "garde stable"};
            profile.reactions = {"peut faire saigner", "change vite de cible", "déteste perdre ses appuis"};
            profile.incomingAccuracyModifier = -9;
            profile.accuracyLine = "Le corps bas et les angles de pattes rendent le centre difficile à trouver.";
        }
        else if (containsAny(combined, {"garde", "sentinelle", "chevalier", "armure", "construction", "golem", "gardien"}))
        {
            profile.archetype = "Gardien défensif";
            profile.behaviorLine = "Comportement gardien : il prend de la place, protège son point faible et cherche à punir les coups mal préparés.";
            profile.attackDescription = monster.getName() + " frappe derrière une garde courte, sans exposer son centre";
            profile.signatureMove = "frappe couverte qui peut relancer une posture défensive";
            profile.counterplayLine = "Réponse conseillée : brise-garde, casser la formation ou préparer une faille de prochain coup.";
            profile.strengths = {"posture défensive", "contre potentiel", "résistance stable"};
            profile.weaknesses = {"brise-garde", "casser la formation", "vulnérabilité de prochain coup"};
            profile.reactions = {"se couvre", "protège les alliés", "perd beaucoup si sa garde saute"};
            profile.incomingAccuracyModifier = 5;
            profile.accuracyLine = "Sa garde est visible : dur à traverser, moins dur à atteindre.";
        }
        else if (containsAny(combined, {"mage", "sorcier", "sorcière", "sorciere", "chaman", "chamane", "oracle", "apothicaire", "scribe"}))
        {
            profile.archetype = "Lanceur / support";
            profile.behaviorLine = "Comportement magique ou soutien : il cherche rarement le meilleur coup physique, mais amplifie le problème si on lui laisse un tour.";
            profile.attackDescription = monster.getName() + " accompagne son attaque d'un geste rituel bref";
            profile.signatureMove = "geste rituel qui prépare des statuts ou aide ses alliés";
            profile.counterplayLine = "Réponse conseillée : cible prioritaire, pression rapide ou silence tactique futur.";
            profile.strengths = {"soutien", "statuts", "soins ou boosts courts"};
            profile.weaknesses = {"pression rapide", "silence tactique", "cible prioritaire en groupe"};
            profile.reactions = {"soigne parfois", "soutient les alliés", "prépare des statuts"};
            profile.incomingAccuracyModifier = -2;
            profile.accuracyLine = "Ses pas restent courts, mais il protège ses mains et son souffle.";
        }
        else if (containsAny(combined, {"élémentaire", "elementaire", "démon", "demon", "anomalie", "dragon", "draconide"}))
        {
            profile.archetype = "Énergie instable";
            profile.behaviorLine = "Comportement instable : la logique du corps est moins importante que l'énergie qui déborde.";
            profile.attackDescription = monster.getName() + " libère un impact chargé d'énergie instable";
            profile.signatureMove = "surcharge imprévisible avec brûlure, choc ou densité brutale";
            profile.counterplayLine = "Réponse conseillée : garde élémentaire, observation ou forcer une faille avant surcharge.";
            profile.strengths = {"surcharge", "dégâts soudains", "statuts possibles"};
            profile.weaknesses = {"observation", "garde élémentaire", "forcer une faille"};
            profile.reactions = {"peut changer le rythme", "réagit aux statuts élémentaires", "devient dangereux si ignoré"};
            profile.incomingAccuracyModifier = -6;
            profile.accuracyLine = "L'énergie déborde autour du corps et fausse les contours.";
        }

        applyMaterialAndBodyTraits(profile, monster, combined);

        const bool dedicatedCaller = containsAny(combined, {"chef", "capitaine", "sergent", "crieur", "crieuse", "corneur", "corne", "hurleur", "hurleuse", "rameuteur", "rameuteuse", "guetteur", "guetteuse", "tambour", "éclaireur", "eclaireur", "sentinelle", "alarme", "reine", "matriarche", "nid", "alpha", "scribe", "chaman", "chamane", "shaman"});
        const bool commonBandRace = containsAny(combined, {"meute", "slime", "gobelin", "kobold", "loup", "rat", "nuisible", "insecte", "insectoïde", "insectoide", "araignée", "araignee"});
        if (monster.canUseWeakenedReinforcementSignal())
        {
            profile.strengths.push_back("second signal possible mais instable");
            profile.weaknesses.push_back("le pousser très bas en vie avant qu'il garde son souffle");
            profile.reactions.push_back("peut tenter un appel tardif seulement dans une situation très mauvaise");
            profile.counterplayLine += " Un appelant arrivé après le premier signal doit être surveillé, mais son cri porte beaucoup moins loin.";
        }
        else if (dedicatedCaller)
        {
            profile.strengths.push_back("appel de renforts possible si la ligne craque");
            profile.weaknesses.push_back("couper le cri, l'isoler ou le finir vite dès qu'il baisse trop en vie");
            profile.reactions.push_back("peut chercher des renforts si trop d'alliés disparaissent ou si la blessure dure");
            profile.counterplayLine += " Les cris, hurlements, vibrations de noyau et signes de nid doivent être coupés tôt.";
        }
        else if (commonBandRace)
        {
            profile.strengths.push_back("instinct de bande si sa vie tient trop longtemps à un fil");
            profile.weaknesses.push_back("le forcer à fuir ou finir le combat avant que l'instinct de survie réponde");
            profile.reactions.push_back("n'appelle que rarement, surtout pour sa propre survie");
            profile.counterplayLine += " Sans vrai rameuteur vivant, ce type ne cherche les siens qu'après une blessure basse et prolongée.";
        }

        if (monster.isElite())
        {
            profile.strengths.push_back("élite : comportement plus fiable et punition plus dangereuse");
        }
        if (monster.isEvolved())
        {
            profile.reactions.push_back("évolution : instincts plus nets que la version normale");
        }

        return profile;
    }

    std::vector<std::string> buildObservationLines(const Monster& monster, bool detailed)
    {
        const MonsterBehaviorProfile profile = build(monster);
        std::vector<std::string> lines;
        lines.push_back("Lecture : " + profile.archetype + ".");
        lines.push_back(profile.behaviorLine);
        if (!profile.signatureMove.empty())
        {
            lines.push_back("Attaque signature : " + profile.signatureMove + ".");
        }

        if (detailed)
        {
            lines.push_back("Forces probables : " + joinShortList(profile.strengths) + ".");
            lines.push_back("Failles probables : " + joinShortList(profile.weaknesses) + ".");
            lines.push_back("Réactions attendues : " + joinShortList(profile.reactions) + ".");
            if (!profile.accuracyLine.empty())
            {
                lines.push_back("Lecture de visée : " + profile.accuracyLine);
            }
            if (!profile.durabilityLine.empty())
            {
                lines.push_back("Lecture de résistance : " + profile.durabilityLine);
            }
            if (!profile.counterplayLine.empty())
            {
                lines.push_back(profile.counterplayLine);
            }
        }
        else
        {
            lines.push_back("Lecture courte : " + (profile.weaknesses.empty() ? "l'observation active reste utile." : profile.weaknesses.front() + "."));
        }

        return lines;
    }

    std::string buildBestiarySentence(const Monster& monster)
    {
        const MonsterBehaviorProfile profile = build(monster);
        std::string sentence = " Comportement : " + profile.archetype + ". " + profile.behaviorLine;
        if (!profile.strengths.empty())
        {
            sentence += " Forces : " + joinShortList(profile.strengths) + ".";
        }
        if (!profile.weaknesses.empty())
        {
            sentence += " Failles : " + joinShortList(profile.weaknesses) + ".";
        }
        if (!profile.signatureMove.empty())
        {
            sentence += " Attaque signature : " + profile.signatureMove + ".";
        }
        if (!profile.accuracyLine.empty())
        {
            sentence += " Visée : " + profile.accuracyLine;
        }
        if (!profile.durabilityLine.empty())
        {
            sentence += " Résistance : " + profile.durabilityLine;
        }
        if (!profile.counterplayLine.empty())
        {
            sentence += " " + profile.counterplayLine;
        }
        return sentence;
    }


    std::vector<std::string> buildTurnFlavorLines(const Monster& monster, const Entity& defender, bool detailed)
    {
        const MonsterBehaviorProfile profile = build(monster);
        std::vector<std::string> lines;
        const int monsterHpPercent = monster.getMaxHp() > 0 ? monster.getHp() * 100 / monster.getMaxHp() : 0;
        const int defenderHpPercent = defender.getMaxHp() > 0 ? defender.getHp() * 100 / defender.getMaxHp() : 0;

        lines.push_back("Lecture vivante : " + profile.archetype + " - " + profile.attackDescription + ".");

        const std::string archetype = normalizeProfileText(profile.archetype);
        const int monsterStatusPressure = (monster.hasBurning() ? 1 : 0)
            + (monster.hasFrost() ? 1 : 0)
            + (monster.hasPoison() ? 1 : 0)
            + (monster.hasBleeding() ? 1 : 0)
            + (monster.hasShock() ? 1 : 0)
            + (monster.hasWeakening() ? 1 : 0)
            + (monster.hasVulnerability() ? 1 : 0)
            + (monster.hasNextHitVulnerability() ? 1 : 0);
        const int defenderStatusPressure = (defender.hasBurning() ? 1 : 0)
            + (defender.hasFrost() ? 1 : 0)
            + (defender.hasPoison() ? 1 : 0)
            + (defender.hasBleeding() ? 1 : 0)
            + (defender.hasShock() ? 1 : 0)
            + (defender.hasWeakening() ? 1 : 0)
            + (defender.hasVulnerability() ? 1 : 0)
            + (defender.hasNextHitVulnerability() ? 1 : 0);

        if (archetype.find("rameuteur") != std::string::npos)
        {
            lines.push_back("Intention lisible : il protège surtout son souffle et son angle de signal, pas seulement sa lame.");
        }
        else if (archetype.find("meute") != std::string::npos || archetype.find("dresseur") != std::string::npos || archetype.find("dompteur") != std::string::npos)
        {
            lines.push_back("Intention lisible : son regard ne reste jamais seul sur toi, il cherche une seconde pression autour de la ligne.");
        }
        else if (archetype.find("soigneur") != std::string::npos || archetype.find("transfuseur") != std::string::npos)
        {
            lines.push_back("Intention lisible : il lit les plaies comme des ressources et tente de choisir qui doit tenir une respiration de plus.");
        }
        else if (archetype.find("porte-bouclier") != std::string::npos || archetype.find("gardien") != std::string::npos || archetype.find("garde runique") != std::string::npos)
        {
            lines.push_back("Intention lisible : il ne recule pas vraiment, il cherche à rendre chaque impact moins rentable.");
        }
        else if (archetype.find("piégeur") != std::string::npos || archetype.find("illusionniste") != std::string::npos)
        {
            lines.push_back("Intention lisible : il veut surtout te faire répondre trop vite, au mauvais endroit.");
        }
        else if (archetype.find("champion de bannière") != std::string::npos || archetype.find("barde") != std::string::npos)
        {
            lines.push_back("Intention lisible : sa présence parle au groupe avant même que son arme ne bouge.");
        }
        else if (archetype.find("slime") != std::string::npos || archetype.find("fongique") != std::string::npos || archetype.find("champignon") != std::string::npos)
        {
            lines.push_back("Intention lisible : il occupe l'espace plus qu'il ne cherche un duel propre.");
        }
        else if (archetype.find("draconique") != std::string::npos || archetype.find("cristal") != std::string::npos)
        {
            lines.push_back("Intention lisible : son corps lui-même impose un rythme, comme si l'impact devait respecter sa matière.");
        }

        if (monsterStatusPressure >= 3)
        {
            lines.push_back("Pression d'états : plusieurs altérations se croisent sur la cible, son prochain geste devient moins propre mais parfois plus brutal.");
        }
        if (defenderStatusPressure >= 3)
        {
            lines.push_back("Lecture de danger : trop de signes mauvais sont ouverts sur toi, l'ennemi n'a pas besoin de deviner longtemps où appuyer.");
        }

        if (monsterHpPercent <= 25)
        {
            if (!profile.reactions.empty())
            {
                lines.push_back("Réaction blessée : " + profile.reactions.front() + ".");
            }
            else
            {
                lines.push_back("Réaction blessée : son rythme devient plus court, plus urgent et moins propre.");
            }
        }
        else if (monster.hasBurning())
        {
            lines.push_back("Réaction au feu : la douleur raccourcit son geste et rend sa prochaine intention plus brutale.");
        }
        else if (monster.hasFrost())
        {
            lines.push_back("Réaction au givre : ses appuis grincent, mais il cherche encore une ligne d'attaque.");
        }
        else if (monster.hasPoison())
        {
            lines.push_back("Réaction au poison : son souffle se dérègle, sans suffire à couper son agressivité.");
        }
        else if (monster.hasBleeding())
        {
            lines.push_back("Réaction au saignement : la blessure attire son attention et casse une partie de son rythme.");
        }
        else if (!profile.reactions.empty())
        {
            const std::size_t index = static_cast<std::size_t>(std::max(0, monster.getLevel() + monster.getHp())) % profile.reactions.size();
            lines.push_back("Réaction probable : " + profile.reactions[index] + ".");
        }

        if (defenderHpPercent <= 35)
        {
            lines.push_back("Lecture de cible : tes blessures se voient, et l'ennemi ajuste son angle plutôt que de frapper au hasard.");
        }
        else if (defender.hasBleeding() || defender.hasPoison())
        {
            lines.push_back("Lecture de cible : il sent une altération déjà ouverte et cherche à la faire compter.");
        }
        else if (defender.hasWeakening() || defender.hasVulnerability() || defender.hasNextHitVulnerability())
        {
            lines.push_back("Lecture de cible : ta défense laisse une fenêtre assez claire pour attirer son prochain geste.");
        }
        else if (defender.hasFrost() || defender.hasShock())
        {
            lines.push_back("Lecture de cible : ton tempo est perturbé, l'ennemi attend le mauvais pas.");
        }

        if (detailed)
        {
            if (!profile.counterplayLine.empty())
            {
                lines.push_back(profile.counterplayLine);
            }
            if (!profile.durabilityLine.empty())
            {
                lines.push_back("Corps lu : " + profile.durabilityLine);
            }
        }

        return lines;
    }

    std::string buildAttackImpactLine(const Monster& monster, int rawDamage, bool critical, bool boosted)
    {
        const MonsterBehaviorProfile profile = build(monster);
        std::ostringstream stream;
        stream << profile.attackDescription;
        if (!profile.signatureMove.empty())
        {
            stream << " (" << profile.signatureMove << ")";
        }
        if (critical)
        {
            stream << " avec un impact critique";
        }
        else if (boosted)
        {
            stream << " avec une puissance renforcée";
        }
        stream << " et inflige " << rawDamage << " dégâts bruts.";
        return stream.str();
    }

    int getIncomingAccuracyModifier(const Monster& monster)
    {
        return build(monster).incomingAccuracyModifier;
    }

    std::string getAccuracyLine(const Monster& monster)
    {
        return build(monster).accuracyLine;
    }

    int getPhysicalDamageModifierPercent(const Monster& monster)
    {
        return build(monster).physicalDamageModifierPercent;
    }

    int getMagicalDamageModifierPercent(const Monster& monster)
    {
        return build(monster).magicalDamageModifierPercent;
    }

    std::string getDurabilityLine(const Monster& monster)
    {
        return build(monster).durabilityLine;
    }

    bool applyEvasiveMissReaction(Monster& monster, Entity& attacker, Random& random, std::ostream& output)
    {
        const MonsterBehaviorProfile profile = build(monster);
        const std::string archetype = normalizeProfileText(profile.archetype);
        const int level = std::max(1, monster.getLevel());

        if (archetype.find("nuisible") != std::string::npos && random.between(1, 100) <= 28)
        {
            attacker.applyBleeding(1, std::max(1, level / 35 + 1));
            output << monster.getName() << " passe sous le coup et mord la cheville avant de filer." << std::endl;
            return true;
        }

        if (archetype.find("chauve-souris") != std::string::npos && random.between(1, 100) <= 28)
        {
            attacker.applyWeakening(1, 4 + level / 32);
            output << monster.getName() << " disparaît dans l'angle haut et brouille la respiration." << std::endl;
            return true;
        }

        if (archetype.find("fée") != std::string::npos && random.between(1, 100) <= 30)
        {
            attacker.applyVulnerability(1, 5 + level / 30);
            output << monster.getName() << " laisse une traînée de ronces là où le coup vient de passer." << std::endl;
            return true;
        }

        if (archetype.find("araignée") != std::string::npos && random.between(1, 100) <= 26)
        {
            attacker.applyWeakening(1, 5 + level / 30);
            output << monster.getName() << " décroche sur un fil et tire les appuis en arrière." << std::endl;
            return true;
        }

        if (archetype.find("insecto") != std::string::npos && random.between(1, 100) <= 24)
        {
            attacker.applyBleeding(1, std::max(1, level / 36 + 1));
            output << monster.getName() << " glisse dans l'angle mort et pique en passant." << std::endl;
            return true;
        }

        if ((archetype.find("sournois") != std::string::npos || archetype.find("kobold") != std::string::npos)
            && random.between(1, 100) <= 25)
        {
            attacker.applyNextHitVulnerability(1, 5 + level / 28);
            output << monster.getName() << " transforme l'esquive en feinte et garde le prochain angle ouvert." << std::endl;
            return true;
        }

        if (archetype.find("carapace") != std::string::npos && random.between(1, 100) <= 24)
        {
            attacker.applyWeakening(1, 4 + level / 34);
            output << monster.getName() << " se tasse sous la carapace et le coup tape trop haut." << std::endl;
            return true;
        }

        if (archetype.find("chasseur de meute") != std::string::npos && random.between(1, 100) <= 24)
        {
            attacker.applyNextHitVulnerability(1, 5 + level / 32);
            output << monster.getName() << " recule juste assez pour laisser la poursuite reprendre l'angle." << std::endl;
            return true;
        }

        if (archetype.find("spectral") != std::string::npos && random.between(1, 100) <= 22)
        {
            attacker.applyFrost(1);
            output << monster.getName() << " n'esquive pas vraiment : le corps traverse trop tard et laisse un froid dans les doigts." << std::endl;
            return true;
        }

        if (archetype.find("slime bondissant") != std::string::npos && random.between(1, 100) <= 24)
        {
            attacker.applyWeakening(1, 4 + level / 34);
            output << monster.getName() << " rebondit contre le sol et fait perdre le rythme du pas suivant." << std::endl;
            return true;
        }

        if (archetype.find("piégeur") != std::string::npos && random.between(1, 100) <= 25)
        {
            attacker.applyWeakening(1, 5 + level / 32);
            output << monster.getName() << " laisse le coup passer trop loin et referme un collet sur l'appui." << std::endl;
            return true;
        }

        if (archetype.find("duelliste") != std::string::npos && random.between(1, 100) <= 25)
        {
            attacker.applyNextHitVulnerability(1, 6 + level / 30);
            output << monster.getName() << " efface son centre et garde la pointe prête pour la reprise." << std::endl;
            return true;
        }

        if (archetype.find("allonge prudente") != std::string::npos && random.between(1, 100) <= 24)
        {
            attacker.applyWeakening(1, 5 + level / 32);
            output << monster.getName() << " recule d'un demi-pas et laisse la pointe voler le rythme du coup." << std::endl;
            return true;
        }

        if (archetype.find("drain de vie") != std::string::npos && random.between(1, 100) <= 20)
        {
            attacker.applyBleeding(1, std::max(1, level / 36 + 1));
            output << monster.getName() << " esquive en gardant une morsure ouverte sur la reprise." << std::endl;
            return true;
        }

        if (archetype.find("danseur d'esquive") != std::string::npos && random.between(1, 100) <= 30)
        {
            monster.applyPrecisionBoost(1, 2);
            attacker.applyNextHitVulnerability(1, 5 + level / 34);
            output << monster.getName() << " transforme l'esquive en pas de côté et garde le tempo pour revenir." << std::endl;
            return true;
        }

        if (archetype.find("barde de guerre") != std::string::npos && random.between(1, 100) <= 22)
        {
            monster.applyGuardBoost(1, 5 + level / 36);
            output << monster.getName() << " garde le rythme malgré l'esquive et relève sa garde." << std::endl;
            return true;
        }

        if (archetype.find("tireur de couverture") != std::string::npos && random.between(1, 100) <= 24)
        {
            attacker.applyNextHitVulnerability(1, 5 + level / 34);
            output << monster.getName() << " recule derrière sa ligne et garde un tir dans le prochain pas." << std::endl;
            return true;
        }

        if (archetype.find("champion de bannière") != std::string::npos && random.between(1, 100) <= 20)
        {
            monster.applyPowerBoost(1, 5 + level / 36);
            output << monster.getName() << " relève l'étendard et transforme l'esquive en avancée." << std::endl;
            return true;
        }

        return false;
    }

    bool applyIncomingHitReaction(Monster& monster, Entity& attacker, Random& random, int receivedDamage, std::ostream& output)
    {
        if (receivedDamage <= 0 || monster.isDead())
        {
            return false;
        }

        const MonsterBehaviorProfile profile = build(monster);
        const std::string archetype = normalizeProfileText(profile.archetype);
        const int level = std::max(1, monster.getLevel());
        const int smallBreakThreshold = std::max(4, monster.getMaxHp() / 9);
        const int largeStaggerThreshold = std::max(8, monster.getMaxHp() / 7);
        const int hardPingThreshold = std::max(3, monster.getMaxHp() / 26);

        if (profileLooksSmallAndFragile(profile)
            && receivedDamage >= smallBreakThreshold
            && random.between(1, 100) <= 32)
        {
            monster.applyWeakening(1, 5 + level / 32);
            if (receivedDamage >= smallBreakThreshold * 2)
            {
                monster.applyNextHitVulnerability(1, 5 + level / 34);
                output << "Le petit corps encaisse mal le vrai impact et garde une faille ouverte." << std::endl;
            }
            else
            {
                output << "Le petit corps évite souvent, mais ce coup le secoue vraiment." << std::endl;
            }
            return true;
        }

        if (profileLooksHardOrArmored(profile)
            && receivedDamage <= hardPingThreshold
            && random.between(1, 100) <= 34)
        {
            monster.startDefensePosture(8, 2, "Matière resserrée");
            output << "L'impact sonne contre la matière dure sans ouvrir de vraie brèche." << std::endl;
            return true;
        }

        if (profileLooksLarge(profile)
            && receivedDamage >= largeStaggerThreshold
            && random.between(1, 100) <= 24)
        {
            monster.applyNextHitVulnerability(1, 5 + level / 36);
            output << "La masse tient debout, mais l'appui vient de plier une fraction de seconde." << std::endl;
            return true;
        }

        if (profileLooksLarge(profile)
            && receivedDamage <= std::max(4, monster.getMaxHp() / 34)
            && random.between(1, 100) <= 18)
        {
            attacker.applyWeakening(1, 4 + level / 36);
            output << "Le choc revient dans le bras plus fort que la plaie laissée dans la masse." << std::endl;
            return true;
        }

        if (profileLooksSoftOrRotten(profile, monster)
            && receivedDamage >= std::max(5, monster.getMaxHp() / 11)
            && random.between(1, 100) <= 26)
        {
            monster.applyVulnerability(1, 5 + level / 36);
            output << "La matière cède mal et laisse une déchirure facile à reprendre." << std::endl;
            return true;
        }

        if (archetype.find("soigneur de fortune") != std::string::npos && receivedDamage >= std::max(5, monster.getMaxHp() / 12) && random.between(1, 100) <= 24)
        {
            monster.applyWeakening(1, 5 + level / 34);
            output << "La main de soin tremble assez pour perdre son prochain geste propre." << std::endl;
            return true;
        }

        if (archetype.find("porte-bouclier") != std::string::npos && receivedDamage <= std::max(5, monster.getMaxHp() / 24) && random.between(1, 100) <= 32)
        {
            monster.startDefensePosture(12, 4, "Bouclier planté");
            output << "Le bouclier absorbe le coup et se plante plus bas dans le sol." << std::endl;
            return true;
        }

        if (archetype.find("berserker") != std::string::npos && receivedDamage >= std::max(6, monster.getMaxHp() / 10) && random.between(1, 100) <= 26)
        {
            monster.applyPowerBoost(1, 8 + level / 32);
            monster.applyVulnerability(1, 5 + level / 38);
            output << "La blessure nourrit la rage, mais laisse une grande ouverture." << std::endl;
            return true;
        }

        if (archetype.find("porte-malédiction") != std::string::npos && random.between(1, 100) <= 20)
        {
            attacker.applyNextHitVulnerability(1, 5 + level / 34);
            output << "La marque occulte se colle à l'impact et cherche le prochain faux pas." << std::endl;
            return true;
        }

        if (archetype.find("slime toxique") != std::string::npos && random.between(1, 100) <= 24)
        {
            attacker.applyPoison(2, std::max(1, level / 32 + 1));
            output << "La gelée toxique éclabousse la main qui a percé la masse." << std::endl;
            return true;
        }

        if (archetype.find("slime collant") != std::string::npos && random.between(1, 100) <= 28)
        {
            attacker.applyWeakening(1, 5 + level / 30);
            output << "Des filaments collants restent accrochés à l'arme et tirent sur le bras." << std::endl;
            return true;
        }

        if (archetype.find("slime froid") != std::string::npos && random.between(1, 100) <= 22)
        {
            attacker.applyFrost(1);
            output << "La gelée froide remonte dans les doigts après l'impact." << std::endl;
            return true;
        }

        if (archetype.find("slime chaud") != std::string::npos && random.between(1, 100) <= 22)
        {
            attacker.applyBurning(1, std::max(1, level / 34 + 1));
            output << "La chaleur du slime colle à la peau malgré le recul." << std::endl;
            return true;
        }

        if (archetype.find("slime brillant") != std::string::npos && random.between(1, 100) <= 26)
        {
            monster.startDefensePosture(12, 3, "Noyau brillant replié");
            output << "Le noyau brillant se replie aussitôt derrière la gelée restante." << std::endl;
            return true;
        }

        if (archetype.find("plante") != std::string::npos && random.between(1, 100) <= 22)
        {
            attacker.applyWeakening(1, 5 + level / 30);
            output << "Les fibres arrachées se referment sur les appuis avant de céder." << std::endl;
            return true;
        }

        if (archetype.find("fée") != std::string::npos && random.between(1, 100) <= 20)
        {
            attacker.applyBleeding(1, std::max(1, level / 38 + 1));
            output << "Même touchée, la fée laisse des ronces minuscules griffer la riposte." << std::endl;
            return true;
        }

        if (archetype.find("construction") != std::string::npos)
        {
            const int sturdyThreshold = std::max(6, monster.getMaxHp() / 28);
            if (receivedDamage <= sturdyThreshold && random.between(1, 100) <= 35)
            {
                monster.startDefensePosture(14, 3, "Plaques verrouillées");
                output << "Le choc glisse sur les plaques et le noyau se verrouille plus bas." << std::endl;
                return true;
            }
        }

        if (archetype.find("draconique") != std::string::npos && random.between(1, 100) <= 18)
        {
            if (random.between(1, 2) == 1)
            {
                attacker.applyBurning(1, std::max(1, level / 34 + 1));
                output << "Les écailles gardent assez de chaleur pour mordre au contact." << std::endl;
            }
            else
            {
                attacker.applyWeakening(1, 5 + level / 32);
                output << "Le poids draconique force le bras à reculer trop vite." << std::endl;
            }
            return true;
        }

        if (archetype.find("serment sacré") != std::string::npos && random.between(1, 100) <= 18)
        {
            monster.startDefensePosture(10, 3, "Serment resserré");
            output << "La frappe reçue referme son serment autour de la garde." << std::endl;
            return true;
        }

        if (archetype.find("spectral") != std::string::npos && random.between(1, 100) <= 18)
        {
            attacker.applyFrost(1);
            output << "Le contact avec l'esprit laisse un froid impossible à secouer tout de suite." << std::endl;
            return true;
        }

        if (archetype.find("champignon") != std::string::npos && random.between(1, 100) <= 24)
        {
            attacker.applyPoison(2, std::max(1, level / 34 + 1));
            attacker.applyWeakening(1, 4 + level / 36);
            output << "La chair fongique éclate en spores basses autour du coup." << std::endl;
            return true;
        }

        if (archetype.find("cristal") != std::string::npos && random.between(1, 100) <= 22)
        {
            if (receivedDamage <= std::max(5, monster.getMaxHp() / 22))
            {
                attacker.applyShock(1);
                output << "Le cristal chante contre l'arme et renvoie une vibration sèche." << std::endl;
            }
            else
            {
                monster.applyNextHitVulnerability(1, 6 + level / 34);
                output << "Une facette se fissure et garde une ligne claire pour la suite." << std::endl;
            }
            return true;
        }

        if (archetype.find("allonge prudente") != std::string::npos && receivedDamage >= std::max(5, monster.getMaxHp() / 12) && random.between(1, 100) <= 22)
        {
            monster.applyWeakening(1, 5 + level / 34);
            output << "La pointe reste en main, mais la distance vient de se casser assez pour gêner la reprise." << std::endl;
            return true;
        }

        if (archetype.find("ancre nécrotique") != std::string::npos && random.between(1, 100) <= 22)
        {
            attacker.applyFrost(1);
            output << "Le lien de tombe se froisse et laisse un froid court dans le bras." << std::endl;
            return true;
        }

        if (archetype.find("drain de vie") != std::string::npos && random.between(1, 100) <= 20)
        {
            monster.applyRegeneration(1, std::max(1, level / 34 + 1));
            attacker.applyWeakening(1, 4 + level / 36);
            output << "Le contact rend un souffle sale à la créature avant qu'elle recule." << std::endl;
            return true;
        }

        if (archetype.find("bombardier") != std::string::npos && random.between(1, 100) <= 24)
        {
            if (receivedDamage >= std::max(5, monster.getMaxHp() / 12))
            {
                monster.applyVulnerability(1, 7 + level / 34);
                output << "La charge cogne contre son propre harnais et laisse une ouverture dangereuse." << std::endl;
            }
            else
            {
                attacker.applyBurning(1, std::max(1, level / 36 + 1));
                output << "Une étincelle de poudre saute au contact du coup." << std::endl;
            }
            return true;
        }

        if (archetype.find("porte-totem") != std::string::npos && random.between(1, 100) <= 22)
        {
            monster.applyElementalWard(1, 8 + level / 34);
            output << "Le totem absorbe une partie du choc et laisse une lueur courte autour du porteur." << std::endl;
            return true;
        }

        if (archetype.find("champion de bannière") != std::string::npos && random.between(1, 100) <= 22)
        {
            monster.applyGuardBoost(1, 6 + level / 36);
            output << "L'étendard tremble, mais le porteur reprend la ligne au lieu de céder." << std::endl;
            return true;
        }

        if (archetype.find("garde runique") != std::string::npos && random.between(1, 100) <= 24)
        {
            if (receivedDamage <= std::max(5, monster.getMaxHp() / 20))
            {
                attacker.applyShock(1);
                output << "La rune frappée renvoie une vibration sèche dans le bras." << std::endl;
            }
            else
            {
                monster.applyVulnerability(1, 5 + level / 36);
                output << "Un glyphe se fend et laisse une ligne exploitable." << std::endl;
            }
            return true;
        }

        return false;
    }

    int applySignaturePreImpact(Monster& monster, Entity& defender, Random& random, int currentRawDamage, std::ostream& output)
    {
        const MonsterBehaviorProfile profile = build(monster);
        const std::string archetype = normalizeProfileText(profile.archetype);
        int bonusDamage = 0;
        const int level = std::max(1, monster.getLevel());

        if (archetype.find("slime toxique") != std::string::npos && random.between(1, 100) <= 18)
        {
            defender.applyPoison(2, std::max(1, 2 + level / 18));
            output << "la gelée toxique cherche les micro-coupures avant même l'impact." << std::endl;
        }
        else if (archetype.find("slime collant") != std::string::npos && random.between(1, 100) <= 24)
        {
            defender.applyWeakening(1, 6 + level / 26);
            output << "la matière collante tire sur les appuis et affaiblit le prochain mouvement." << std::endl;
        }
        else if (archetype.find("slime froid") != std::string::npos && random.between(1, 100) <= 18)
        {
            defender.applyFrost(1);
            output << "le froid de la gelée ralentit la lecture du coup." << std::endl;
        }
        else if (archetype.find("slime chaud") != std::string::npos && random.between(1, 100) <= 18)
        {
            defender.applyBurning(1, std::max(1, 2 + level / 22));
            output << "la gelée chaude accroche une brûlure courte." << std::endl;
        }
        else if (archetype.find("slime bondissant") != std::string::npos && random.between(1, 100) <= 20)
        {
            bonusDamage += std::max(1, currentRawDamage / 10);
            output << "le rebond change l'angle au dernier moment." << std::endl;
        }
        else if (archetype.find("slime brillant") != std::string::npos && random.between(1, 100) <= 22)
        {
            monster.startDefensePosture(10, 3, "Noyau brillant protégé");
            output << "le noyau brillant se replie derrière une mince couche de gelée." << std::endl;
        }
        else if (archetype.find("chauve-souris") != std::string::npos && random.between(1, 100) <= 22)
        {
            defender.applyWeakening(1, 5 + level / 28);
            output << "le harcèlement aérien brouille les appuis et la lecture." << std::endl;
        }
        else if (archetype.find("nuisible") != std::string::npos && random.between(1, 100) <= 22)
        {
            defender.applyBleeding(1, std::max(1, level / 34 + 1));
            output << "la morsure sale laisse une petite plaie pénible." << std::endl;
        }
        else if (archetype.find("chargeur massif") != std::string::npos && random.between(1, 100) <= 24)
        {
            bonusDamage += 2 + level / 18;
            defender.applyWeakening(1, 5 + level / 30);
            output << "la charge massive secoue la garde avant l'impact." << std::endl;
        }
        else if (archetype.find("araignée") != std::string::npos && random.between(1, 100) <= 24)
        {
            if (containsAny(monster.getType() + " " + monster.getName(), {"venime", "poison", "toxique"}))
            {
                defender.applyPoison(2, std::max(1, 1 + level / 25));
                output << "la toile toxique laisse un venin lent." << std::endl;
            }
            else
            {
                defender.applyWeakening(1, 6 + level / 28);
                output << "la toile courte tire sur les appuis." << std::endl;
            }
        }
        else if (archetype.find("kobold") != std::string::npos && random.between(1, 100) <= 20)
        {
            defender.applyNextHitVulnerability(1, 5 + level / 28);
            output << "la feinte nerveuse du kobold ouvre un angle pour le prochain coup." << std::endl;
        }
        else if (archetype.find("harceleur à distance") != std::string::npos && random.between(1, 100) <= 24)
        {
            defender.applyVulnerability(1, 5 + level / 30);
            output << "le tir de gêne force une mauvaise position." << std::endl;
        }
        else if (archetype.find("alchimiste") != std::string::npos && random.between(1, 100) <= 28)
        {
            const int flaskRoll = random.between(1, 3);
            if (flaskRoll == 1)
            {
                defender.applyPoison(2, std::max(1, 1 + level / 24));
                output << "une fiole verte éclate et salit la blessure." << std::endl;
            }
            else if (flaskRoll == 2)
            {
                defender.applyBurning(1, std::max(1, 2 + level / 26));
                output << "une étincelle chimique mord la peau." << std::endl;
            }
            else
            {
                defender.applyWeakening(1, 6 + level / 28);
                output << "un résidu visqueux ralentit le prochain geste." << std::endl;
            }
        }
        else if (archetype.find("fée") != std::string::npos && random.between(1, 100) <= 22)
        {
            defender.applyBleeding(1, std::max(1, level / 32 + 1));
            defender.applyVulnerability(1, 4 + level / 32);
            output << "les ronces féeriques griffent puis guident le prochain impact." << std::endl;
        }
        else if (archetype.find("spectral") != std::string::npos && random.between(1, 100) <= 24)
        {
            if (random.between(1, 2) == 1)
            {
                defender.applyFrost(1);
                output << "le frisson spectral fige le tempo." << std::endl;
            }
            else
            {
                defender.applyVulnerability(1, 5 + level / 28);
                output << "l'esprit traverse la garde et laisse une faille froide." << std::endl;
            }
        }
        else if (archetype.find("carapace") != std::string::npos && random.between(1, 100) <= 24)
        {
            if (random.between(1, 2) == 1)
            {
                monster.startDefensePosture(12, 3, "Carapace basse");
                output << "la carapace se ferme avant que le ventre puisse être repris." << std::endl;
            }
            else
            {
                defender.applyWeakening(1, 5 + level / 30);
                output << "la pince basse force le pas à se tordre." << std::endl;
            }
        }
        else if (archetype.find("chasseur de meute") != std::string::npos && random.between(1, 100) <= 24)
        {
            if (defender.hasBleeding() || defender.hasWeakening() || defender.hasVulnerability() || defender.hasNextHitVulnerability())
            {
                bonusDamage += 2 + level / 18;
                output << "la meute sent la faille et resserre la poursuite." << std::endl;
            }
            else
            {
                defender.applyNextHitVulnerability(1, 5 + level / 32);
                output << "la morsure force la retraite vers un mauvais angle." << std::endl;
            }
        }
        else if (archetype.find("construction") != std::string::npos && random.between(1, 100) <= 24)
        {
            monster.startDefensePosture(14, 3, "Noyau verrouillé");
            output << "les plaques se referment autour du noyau après la séquence." << std::endl;
        }
        else if (archetype.find("draconique") != std::string::npos && random.between(1, 100) <= 24)
        {
            const int breathRoll = random.between(1, 3);
            if (breathRoll == 1)
            {
                defender.applyBurning(1, std::max(1, 2 + level / 22));
                output << "un souffle court laisse une chaleur de dragon." << std::endl;
            }
            else if (breathRoll == 2)
            {
                defender.applyShock(1);
                output << "les écailles crépitent et le choc passe dans l'impact." << std::endl;
            }
            else
            {
                bonusDamage += 2 + level / 16;
                output << "le sang draconique alourdit brutalement le coup." << std::endl;
            }
        }
        else if (archetype.find("serment sacré") != std::string::npos && random.between(1, 100) <= 22)
        {
            if (random.between(1, 2) == 1)
            {
                monster.startDefensePosture(10, 3, "Serment tordu");
                output << "le serment tordu referme une garde cérémonielle." << std::endl;
            }
            else
            {
                defender.applyVulnerability(1, 6 + level / 28);
                output << "le jugement bref marque la prochaine faille." << std::endl;
            }
        }
        else if (archetype.find("soigneur de fortune") != std::string::npos && random.between(1, 100) <= 22)
        {
            monster.applyRegeneration(1, std::max(1, level / 26 + 2));
            monster.applyGuardBoost(1, 6 + level / 34);
            output << "un geste de soin referme juste assez la ligne autour de lui." << std::endl;
        }
        else if (archetype.find("porte-bouclier") != std::string::npos && random.between(1, 100) <= 24)
        {
            monster.startDefensePosture(14, 4, "Bouclier resserré");
            defender.applyWeakening(1, 5 + level / 30);
            output << "le bouclier pousse avant l'arme et casse l'élan du bras." << std::endl;
        }
        else if (archetype.find("berserker") != std::string::npos && monster.getMaxHp() > 0 && monster.getHp() * 100 <= monster.getMaxHp() * 45)
        {
            bonusDamage += 3 + level / 14;
            monster.applyVulnerability(1, 5 + level / 40);
            output << "la rage ouvre son corps autant qu'elle alourdit son coup." << std::endl;
        }
        else if (archetype.find("porte-malédiction") != std::string::npos && random.between(1, 100) <= 24)
        {
            defender.applyWeakening(1, 6 + level / 30);
            defender.applyNextHitVulnerability(1, 6 + level / 32);
            output << "la marque noire ralentit la défense et garde une faille en suspens." << std::endl;
        }
        else if (archetype.find("sournois") != std::string::npos && random.between(1, 100) <= 24)
        {
            defender.applyNextHitVulnerability(1, 6 + level / 24);
            output << "le profil sournois prépare une faille pour le prochain vrai coup." << std::endl;
        }
        else if (archetype.find("gobelin") != std::string::npos && defender.getMaxHp() > 0 && defender.getHp() * 100 <= defender.getMaxHp() * 50)
        {
            bonusDamage += 1 + level / 20;
            output << "l'opportunisme gobelin grandit dès que les blessures se voient." << std::endl;
        }
        else if (archetype.find("brute") != std::string::npos && monster.getMaxHp() > 0 && monster.getHp() * 100 <= monster.getMaxHp() * 55)
        {
            bonusDamage += 2 + level / 16;
            output << "la brute transforme sa blessure en élan." << std::endl;
        }
        else if (archetype.find("prédateur") != std::string::npos && defender.getMaxHp() > 0 && defender.getHp() * 100 <= defender.getMaxHp() * 45)
        {
            bonusDamage += 2 + level / 18;
            defender.applyBleeding(1, std::max(1, level / 30 + 1));
            output << "le prédateur vise la blessure la plus visible." << std::endl;
        }
        else if (archetype.find("plante") != std::string::npos && random.between(1, 100) <= 18)
        {
            defender.applyWeakening(1, 5 + level / 26);
            output << "les fibres végétales accrochent les jambes." << std::endl;
        }
        else if (archetype.find("insecto") != std::string::npos && random.between(1, 100) <= 18)
        {
            defender.applyBleeding(1, std::max(1, level / 32 + 1));
            output << "la piqûre cherche une couture d'armure." << std::endl;
        }
        else if (archetype.find("gardien") != std::string::npos && random.between(1, 100) <= 20)
        {
            monster.startDefensePosture(12, 4, "Garde signature");
            output << "le gardien frappe sans ouvrir complètement sa défense." << std::endl;
        }
        else if (archetype.find("lanceur") != std::string::npos && random.between(1, 100) <= 18)
        {
            defender.applyVulnerability(1, 5 + level / 28);
            output << "le geste rituel rend l'impact suivant plus facile à lire pour ses alliés." << std::endl;
        }
        else if (archetype.find("énergie") != std::string::npos && random.between(1, 100) <= 18)
        {
            const int roll = random.between(1, 3);
            if (roll == 1)
            {
                defender.applyShock(1);
                output << "une surcharge électrique déborde de l'impact." << std::endl;
            }
            else if (roll == 2)
            {
                defender.applyBurning(1, std::max(1, 2 + level / 24));
                output << "l'énergie instable laisse une chaleur courte." << std::endl;
            }
            else
            {
                bonusDamage += std::max(1, currentRawDamage / 12);
                output << "la surcharge rend l'impact plus dense." << std::endl;
            }
        }
        else if (archetype.find("piégeur") != std::string::npos && random.between(1, 100) <= 24)
        {
            defender.applyWeakening(1, 6 + level / 28);
            defender.applyNextHitVulnerability(1, 5 + level / 34);
            output << "le collet bas force un mauvais pas avant la reprise." << std::endl;
        }
        else if (archetype.find("duelliste") != std::string::npos && random.between(1, 100) <= 24)
        {
            defender.applyNextHitVulnerability(1, 7 + level / 28);
            output << "la feinte de lame prend le poignet plutôt que la garde." << std::endl;
        }
        else if (archetype.find("champignon") != std::string::npos && random.between(1, 100) <= 23)
        {
            defender.applyPoison(2, std::max(1, level / 30 + 1));
            defender.applyWeakening(1, 5 + level / 34);
            output << "les spores collent au souffle avant que le coup arrive." << std::endl;
        }
        else if (archetype.find("cristal") != std::string::npos && random.between(1, 100) <= 22)
        {
            if (random.between(1, 2) == 1)
            {
                defender.applyShock(1);
                output << "la résonance cristalline fait trembler le bras." << std::endl;
            }
            else
            {
                monster.startDefensePosture(12, 3, "Facettes refermées");
                output << "les facettes se referment autour de la fissure visible." << std::endl;
            }
        }
        else if (archetype.find("allonge prudente") != std::string::npos && random.between(1, 100) <= 24)
        {
            defender.applyNextHitVulnerability(1, 5 + level / 32);
            output << "la pointe garde assez d'espace pour punir le prochain pas trop droit." << std::endl;
        }
        else if (archetype.find("ancre nécrotique") != std::string::npos && random.between(1, 100) <= 24)
        {
            defender.applyFrost(1);
            defender.applyVulnerability(1, 5 + level / 34);
            output << "le fil de tombe accroche la défense et refroidit le rythme." << std::endl;
        }
        else if (archetype.find("drain de vie") != std::string::npos && random.between(1, 100) <= 24)
        {
            if (defender.hasBleeding() || defender.hasWeakening() || defender.hasVulnerability())
            {
                bonusDamage += 2 + level / 20;
                monster.applyRegeneration(1, std::max(1, level / 32 + 1));
                output << "la plaie déjà ouverte attire le contact drainant." << std::endl;
            }
            else
            {
                defender.applyBleeding(1, std::max(1, level / 36 + 1));
                output << "la morsure cherche d'abord une plaie à exploiter." << std::endl;
            }
        }
        else if (archetype.find("barde de guerre") != std::string::npos && random.between(1, 100) <= 25)
        {
            monster.applyPrecisionBoost(1, 2);
            monster.applyGuardBoost(1, 5 + level / 35);
            output << "le rythme de guerre relève son bras et resserre sa garde." << std::endl;
        }
        else if (archetype.find("bombardier") != std::string::npos && random.between(1, 100) <= 26)
        {
            const int blastRoll = random.between(1, 3);
            if (blastRoll == 1)
            {
                defender.applyBurning(1, std::max(1, level / 28 + 2));
                output << "une charge crache une langue de feu au ras du sol." << std::endl;
            }
            else if (blastRoll == 2)
            {
                defender.applyWeakening(1, 6 + level / 30);
                output << "la détonation basse vole l'appui avant le choc." << std::endl;
            }
            else
            {
                bonusDamage += 2 + level / 16;
                monster.applyVulnerability(1, 4 + level / 42);
                output << "l'explosion pousse fort, mais expose son lanceur une fraction de seconde." << std::endl;
            }
        }
        else if (archetype.find("danseur d'esquive") != std::string::npos && random.between(1, 100) <= 26)
        {
            monster.applyPrecisionBoost(1, 2);
            defender.applyNextHitVulnerability(1, 5 + level / 32);
            output << "le pas glissé transforme le mouvement en angle de reprise." << std::endl;
        }
        else if (archetype.find("porte-totem") != std::string::npos && random.between(1, 100) <= 25)
        {
            const int totemRoll = random.between(1, 3);
            if (totemRoll == 1)
            {
                monster.applyRegeneration(1, std::max(1, level / 34 + 1));
                output << "le totem rend un souffle court à son porteur." << std::endl;
            }
            else if (totemRoll == 2)
            {
                monster.applyElementalWard(1, 8 + level / 34);
                output << "une garde rituelle se ferme autour du symbole." << std::endl;
            }
            else
            {
                defender.applyFrost(1);
                output << "le rythme du totem tire un froid lourd dans la défense." << std::endl;
            }
        }
        else if (archetype.find("champion de bannière") != std::string::npos && random.between(1, 100) <= 25)
        {
            monster.applyPrecisionBoost(1, 2);
            monster.applyPowerBoost(1, 5 + level / 36);
            output << "l'étendard tire la frappe vers l'avant." << std::endl;
        }
        else if (archetype.find("transfuseur de souffle") != std::string::npos && random.between(1, 100) <= 24)
        {
            if (defender.hasBleeding() || defender.hasPoison() || defender.hasWeakening())
            {
                bonusDamage += 1 + level / 22;
                monster.applyRegeneration(1, std::max(1, level / 34 + 1));
                output << "le lien tire un souffle sale depuis la blessure déjà ouverte." << std::endl;
            }
            else
            {
                defender.applyWeakening(1, 5 + level / 34);
                output << "la prise cherche d'abord à voler le souffle." << std::endl;
            }
        }
        else if (archetype.find("garde runique") != std::string::npos && random.between(1, 100) <= 24)
        {
            const int runeRoll = random.between(1, 3);
            if (runeRoll == 1)
            {
                monster.applyElementalWard(1, 9 + level / 34);
                output << "un glyphe protège brièvement le corps." << std::endl;
            }
            else if (runeRoll == 2)
            {
                defender.applyShock(1);
                output << "la rune claque dans l'impact et laisse un choc court." << std::endl;
            }
            else
            {
                monster.startDefensePosture(10, 3, "Rune verrouillée");
                output << "les signes se referment en garde courte." << std::endl;
            }
        }
        else if (archetype.find("tireur de couverture") != std::string::npos && random.between(1, 100) <= 24)
        {
            defender.applyNextHitVulnerability(1, 5 + level / 34);
            output << "le tir force le pas dans une trajectoire trop lisible." << std::endl;
        }

        return bonusDamage;
    }
}
