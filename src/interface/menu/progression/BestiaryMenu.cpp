// EN: BestiaryMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BestiaryMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// English: Displays the first usable bestiary menu with prepared categories and detailed entries.
// Français : Affiche le premier bestiaire utilisable avec catégories préparées et fiches détaillées.

#include "interface/menu/progression/BestiaryMenu.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "lore/LegendTriggerSystem.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "progression/material/MaterialKnowledgeProgress.hpp"
#include "progression/material/MaterialExperimentLog.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace
{
    struct BestiaryPreviewEntry
    {
        std::string category;
        std::string name;
        std::string simpleDescription;
        std::string detailedDescription;
        std::string status;
        int encounters;
        int kills;
    };

    struct KnowledgeHints
    {
        std::string dangerRank;
        std::string habitat;
        std::string weaknesses;
        std::string resistances;
        std::string drops;
        std::string strategy;
    };

    struct SpecialDiscoveryEntry
    {
        std::string name;
        std::string knownLabel;
        std::string hiddenLabel;
        std::string family;
        std::string unlockHint;
        bool knownFromStart;
    };

    void displayLegendArchiveList(const std::string& title, const std::string& categoryFilter, bool discoveredOnly);

    std::string lowerCopy(const std::string& value)
    {
        std::string output = value;
        std::transform(output.begin(), output.end(), output.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return output;
    }

    bool containsText(const std::string& haystack, const std::string& needle)
    {
        return lowerCopy(haystack).find(lowerCopy(needle)) != std::string::npos;
    }

    KnowledgeHints buildKnowledgeHints(
        const std::string& category,
        const std::string& name,
        const std::string& description
    )
    {
        KnowledgeHints hints;
        hints.dangerRank = "Variable";
        hints.habitat = "Habitat encore mal confirmé.";
        hints.weaknesses = "Faiblesses non confirmées.";
        hints.resistances = "Résistances non confirmées.";
        hints.drops = "Butin à confirmer par observation, récolte ou achat d'information.";
        hints.strategy = "Observer le comportement avant de conclure.";

        const std::string probe = lowerCopy(category + " " + name + " " + description);

        if (containsText(probe, "slime") || containsText(probe, "gel"))
        {
            hints.dangerRank = "Faible à instable selon couleur";
            hints.habitat = "Mares gélatineuses, grottes humides, ruines chargées de résidus.";
            hints.weaknesses = "Impact net, froid ou feu selon couleur. Les slimes dorés et chromatiques demandent observation.";
            hints.resistances = "Saignement très faible ; poison souvent inutile sur gelées simples.";
            hints.drops = "Résidus de slime, gel coloré, parfois noyau instable ou matière brillante avalée.";
            hints.strategy = "Identifier la couleur avant d'utiliser une ressource rare.";
        }
        else if (containsText(probe, "gobelin") || containsText(probe, "hobgobelin"))
        {
            hints.dangerRank = "Faible seul, dangereux en bande";
            hints.habitat = "Routes, caches, campements de fortune, grottes proches des villes.";
            hints.weaknesses = "Pression rapide, dégâts de zone, neutralisation du shaman ou du chef.";
            hints.resistances = "Peu de résistance naturelle ; l'équipement change tout.";
            hints.drops = "Oreilles, insignes, pièces volées, cuir ou métal médiocre.";
            hints.strategy = "Ne pas laisser le soutien installer la bande.";
        }
        else if (containsText(probe, "loup") || containsText(probe, "bête") || containsText(probe, "sanglier") || containsText(probe, "renard"))
        {
            hints.dangerRank = "Faible à moyen";
            hints.habitat = "Plaines, forêts, routes isolées et lisières.";
            hints.weaknesses = "Entrave, feu maîtrisé, contre après charge.";
            hints.resistances = "Peu de résistance magique ; bon instinct contre les pièges évidents.";
            hints.drops = "Peau, crocs, fourrure, viande, cuir robuste selon propreté.";
            hints.strategy = "Rester groupé et éviter de finir blessé face à une meute.";
        }
        else if (containsText(probe, "squelette") || containsText(probe, "mort") || containsText(probe, "osseux") || containsText(probe, "ombre") || containsText(probe, "cimetière"))
        {
            hints.dangerRank = "Moyen, parfois trompeur";
            hints.habitat = "Cimetières, ruines, lieux froids ou chargés de mana mort.";
            hints.weaknesses = "Lumière, feu, coups lourds contre les os, purification.";
            hints.resistances = "Poison et saignement souvent très faibles.";
            hints.drops = "Os fissurés, fil d'ombre, poussière funéraire, fragments de sceau.";
            hints.strategy = "Chercher rupture ou purification plutôt que poison.";
        }
        else if (containsText(probe, "plante") || containsText(probe, "racine") || containsText(probe, "ronce"))
        {
            hints.dangerRank = "Moyen si terrain serré";
            hints.habitat = "Forêts anciennes, marais, ruines recouvertes.";
            hints.weaknesses = "Feu, coupe nette, gel sur fibres humides.";
            hints.resistances = "Poison peu utile ; entraves difficiles à retourner.";
            hints.drops = "Fibres, sève, graines, ronces occultes.";
            hints.strategy = "Libérer le mouvement avant de chercher le gros dégât.";
        }
        else if (containsText(probe, "bandit") || containsText(probe, "alchimiste") || containsText(probe, "mage renégat") || containsText(probe, "humain hostile") || containsText(probe, "chasseur de primes"))
        {
            hints.dangerRank = "Variable selon équipement";
            hints.habitat = "Routes commerciales, ruelles, camps improvisés et contrats douteux.";
            hints.weaknesses = "Désarmement, pression rapide sur les soutiens, interruption des fioles ou incantations.";
            hints.resistances = "Pas de résistance naturelle stable ; armure, potions et préparation font la différence.";
            hints.drops = "Insignes abîmés, cuir, fioles, poussière arcanique ou pièces volées.";
            hints.strategy = "Identifier le rôle : apothicaire, mage, chef ou tireur ne se gèrent pas dans le même ordre.";
        }
        else if (containsText(probe, "orc") || containsText(probe, "berserker") || containsText(probe, "colosse"))
        {
            hints.dangerRank = "Moyen à lourd";
            hints.habitat = "Campements robustes, zones de chasse, arènes et passages disputés.";
            hints.weaknesses = "Entrave, allonge, usure contrôlée et attaques qui ignorent partiellement l'armure.";
            hints.resistances = "Bonne résistance physique brute ; difficile à faire fuir au contact.";
            hints.drops = "Cuir épais, métal cabossé, trophées de guerre et matériaux de réparation.";
            hints.strategy = "Ne pas échanger coup pour coup sans garde : le rythme lent cache souvent un gros impact.";
        }
        else if (containsText(probe, "araignée") || containsText(probe, "insect") || containsText(probe, "venimeuse"))
        {
            hints.dangerRank = "Faible à dangereux si poison accumulé";
            hints.habitat = "Grottes, forêts humides, marais et coins non entretenus.";
            hints.weaknesses = "Feu contrôlé, attaques de zone, purification du poison.";
            hints.resistances = "Bon instinct d'esquive ; poison parfois moins efficace sur variantes déjà toxiques.";
            hints.drops = "Mandibules, venin, soie, chitine légère.";
            hints.strategy = "Ne pas attendre que le poison fasse boule de neige : traiter tôt ou finir vite.";
        }
        else if (containsText(probe, "dragon") || containsText(probe, "draconide") || containsText(probe, "kobold"))
        {
            hints.dangerRank = "Élevé quand l'âge augmente";
            hints.habitat = "Nids rocheux, montagnes, anciennes ruines et territoires gardés.";
            hints.weaknesses = "Faille de résistance, contrôle de souffle, éléments opposés selon lignée.";
            hints.resistances = "Écailles, souffle, fierté et résistance naturelle supérieure à la moyenne.";
            hints.drops = "Écailles, braises, griffes, fragments draconiques.";
            hints.strategy = "Préparer le combat : une victoire brutale peut abîmer les matériaux les plus précieux.";
        }
        else if (containsText(probe, "construction") || containsText(probe, "armure") || containsText(probe, "sentinelle") || containsText(probe, "gardien ancien"))
        {
            hints.dangerRank = "Lourd et défensif";
            hints.habitat = "Ruines, portes scellées, salles de garde et anciens ateliers.";
            hints.weaknesses = "Choc ciblé, marteau, fissures, corrosion ou brise-garde.";
            hints.resistances = "Poison et saignement quasi inutiles ; très bonne résistance aux petits coups.";
            hints.drops = "Fragments de métal, plaques, noyaux, anciennes pièces de mécanisme.";
            hints.strategy = "Chercher les fissures plutôt que de multiplier les entailles faibles.";
        }
        else if (containsText(probe, "élémentaire") || containsText(probe, "méphaïte") || containsText(probe, "braise") || containsText(probe, "anomalie"))
        {
            hints.dangerRank = "Instable";
            hints.habitat = "Failles magiques, ruines chargées, zones extrêmes ou expériences ratées.";
            hints.weaknesses = "Élément opposé, stabilisation, rupture de noyau ou voile élémentaire bien placé.";
            hints.resistances = "Résiste souvent à son propre élément ; certaines anomalies changent de réaction.";
            hints.drops = "Poussière arcanique, noyau instable, éclat élémentaire.";
            hints.strategy = "Lire la première réaction avant d'insister : l'instabilité punit les habitudes.";
        }
        else if (category == "Habitats / zones")
        {
            hints.dangerRank = "Zone";
            hints.habitat = "Lieu d'exploration avec familles dominantes, ressources et variantes rares.";
            hints.weaknesses = "La préparation dépend du biome : antidotes, anti-givre, arme lourde, feu ou voile élémentaire.";
            hints.resistances = "Une zone ne résiste pas, mais elle favorise certaines rencontres et pièges.";
            hints.drops = "Ressources locales, coffres, traces, contrats et matériaux liés aux familles présentes.";
            hints.strategy = "Lire la zone avant d'y entrer : les bons consommables valent parfois plus qu'un niveau.";
        }
        else if (containsText(probe, "brûlure") || containsText(probe, "poison") || containsText(probe, "givre") || containsText(probe, "choc") || containsText(probe, "saignement") || containsText(probe, "affaiblissement") || containsText(probe, "faille") || containsText(probe, "voile") || containsText(probe, "suture"))
        {
            hints.dangerRank = "Effet persistant";
            hints.habitat = "Sorts, munitions, potions, monstres élémentaires ou boss spécialisés.";
            hints.weaknesses = "Purification, potion adaptée, voile élémentaire ou fin naturelle de durée.";
            hints.resistances = "Race, classe, armure, matériau et catalyseur peuvent réduire ou amplifier l'effet.";
            hints.drops = "Pas un loot direct, mais une connaissance utile pour préparer les consommables.";
            hints.strategy = "Lire l'icône d'état : ignorer un effet faible peut coûter moins cher que gaspiller une potion.";
        }
        else if (category == "Boss")
        {
            hints.dangerRank = "Boss";
            hints.habitat = "Arène, territoire scellé ou lieu lié à sa légende.";
            hints.weaknesses = "Failles de phase à découvrir par dialogue, registre ou observation.";
            hints.resistances = "Résistances élevées ; changement de règles possible sous 50% PV.";
            hints.drops = "Fragment, invitation, matériau unique ou trace de boss.";
            hints.strategy = "Préparer potions, munitions et lecture de puissance avant d'entrer.";
        }
        else if (category == "Matériaux et plantes")
        {
            hints.dangerRank = "Ressource";
            hints.habitat = "Zone, monstre ou qualité de récolte associé.";
            hints.weaknesses = "Peut être brûlé, troué, écrasé ou souillé par une mauvaise méthode.";
            hints.resistances = "La haute qualité conserve mieux son intérêt en craft.";
            hints.drops = "Quantité et qualité dépendent du combat, de la récolte et parfois de la chance.";
            hints.strategy = "Choisir entre vente rapide et conservation pour recette rare.";
        }
        else if (category == "Objets rares")
        {
            hints.dangerRank = "Objet";
            hints.habitat = "Bibliothèque, marché noir, coffre, boss, troc ou récompense spéciale.";
            hints.weaknesses = "Usage limité, coût élevé, instabilité ou prérequis.";
            hints.resistances = "Garde sa valeur si utilisé dans le bon contexte.";
            hints.drops = "Obtention par achat, troc, exploration ou secret.";
            hints.strategy = "Vérifier si l'objet est jetable, apprenable, réparable ou unique.";
        }

        return hints;
    }

    // EN: getEntries declares or implements a focused behavior used by this module.
    // FR: getEntries déclare ou implémente un comportement précis utilisé par ce module.
    const std::vector<BestiaryPreviewEntry>& getEntries()
    {
        static const std::vector<BestiaryPreviewEntry> entries = {
            {"Races", "Humain", "Race commune, adaptable et ambitieuse.", "Les humains sont présents dans beaucoup de villes. Ils servent de base pour les aventuriers, bandits, chevaliers et mages.", "Connu", 0, 0},
            {"Races", "Elfe noir", "Race discrète, souvent liée aux ombres.", "Les elfes noirs peuvent être craints selon les régions. Hazak représente une version très dangereuse de cette race.", "Partiellement connu", 0, 0},
            {"Races", "Kitsune", "Race semi-humaine liée aux illusions et aux flammes.", "Les kitsunes gardent souvent un lien naturel avec la magie, la ruse et les flammes. Aoi en est un exemple spécial.", "Partiellement connu", 0, 0},
            {"Habitats / zones", "Plaine sauvage", "Biome ouvert pour les premiers pas.", "Plaines, hautes herbes, petits prédateurs et gobelins faibles. Les coffres visibles peuvent exister, mais les traces rares restent liées au risque et au niveau.", "Carte commune", 0, 0},
            {"Habitats / zones", "Route commerciale", "Passage de voyageurs, marchands et bandits.", "Zone idéale pour rencontrer humanoïdes, pilleurs, gobelins organisés, clients de guilde et objets volés. Les soutiens crédibles y apparaissent plus naturellement.", "Carte commune", 0, 0},
            {"Habitats / zones", "Mares gélatineuses", "Biome des slimes et réactions colorées.", "Les mares concentrent les variantes de slimes : vert, bleu, rouge, jaune, violet, ambré, doré, chromatique et formes anciennes. Le bon élément dépend toujours de la couleur.", "Carte commune", 0, 0},
            {"Habitats / zones", "Forêt ancienne", "Biome humide, végétal et plus sérieux.", "Racines, bêtes couvertes de mousse, plantes hostiles et esprits sylvestres. Les attaques trop brutales peuvent abîmer des composants végétaux précieux.", "Carte étudiée", 0, 0},
            {"Habitats / zones", "Montagne froide", "Biome de falaises et de froid.", "La montagne favorise le givre, les draconides froids, les bêtes lourdes et les constructions rocheuses. Anti-givre, voile élémentaire et armes solides y prennent plus de valeur.", "Carte étudiée", 0, 0},
            {"Habitats / zones", "Marais trouble", "Biome toxique et lent.", "Le marais attire slimes putrides, noyés, chamans de vase et mages sales. Antidotes, purification et gestion de la durée y sont essentiels.", "Carte dangereuse", 0, 0},
            {"Habitats / zones", "Cimetière oublié", "Biome des morts et des noms perdus.", "Morts-vivants, esprits, goules, oracles de tombe et ombres rares. Poison et saignement perdent souvent de la valeur ; lumière, feu et coups lourds en gagnent.", "Carte dangereuse", 0, 0},
            {"Habitats / zones", "Ruines effondrées", "Biome ancien, cassé et arcanique.", "Spectres, armures mortes, anomalies, sentinelles brisées et squelettes runiques. Les constructions y demandent des réponses différentes des chairs vivantes.", "Carte dangereuse", 0, 0},
            {"Entités hostiles / ennemis", "Gobelin peureux", "Petit ennemi rapide qui préfère parfois survivre.", "Un gobelin basique. Pas très noble, mais capable de gratter des dégâts et de fuir si le combat devient humiliant.", "Déjà rencontré dans les arènes libres", 1, 0},
            {"Entités hostiles / ennemis", "Famille gobeline", "Petits humanoïdes opportunistes avec plusieurs rôles.", "Les gobelins faibles servent de chair à canon, les frondeurs harcèlent, les gardes tiennent plus longtemps, les shamans peuvent soigner leurs alliés, et les hobgobelins organisent mieux les bandes. Un gobelin normal ne soigne pas par miracle : seuls les chamans et profils préparés savent maintenir une bande debout.", "Renseignement évolutif", 0, 0},
            {"Entités hostiles / ennemis", "Famille slime", "Gelées célèbres aux couleurs très lisibles.", "Vert = classique/régénération légère, bleu/blanc = froid, rouge = chaleur/brûlure, violet/noir = poison, jaune = choc, ambré = colle et gêne les mouvements, rose = rebond imprévisible, doré = attiré par les choses brillantes, chromatique/prisme/miroir = réaction instable. Les mares gélatineuses concentrent presque toutes les couleurs, mais certaines couleurs apparaissent aussi dans les lieux qui les attirent.", "Renseignement évolutif", 0, 0},
            {"Entités hostiles / ennemis", "Slime rouge", "Gelée chaude qui irrite la peau.", "Slime attiré par la chaleur : risque de brûlure légère, résiste mieux aux flammes faibles et supporte mal le froid propre.", "Couleur documentée", 0, 0},
            {"Entités hostiles / ennemis", "Slime bleu", "Gelée froide et lente.", "Slime lié au froid : gêne les mouvements, peut installer du givre et craint souvent la chaleur soutenue.", "Couleur documentée", 0, 0},
            {"Entités hostiles / ennemis", "Slime violet", "Gelée toxique des lieux humides.", "Slime de poison : très mauvais candidat pour les poisons classiques. Les antidotes et la distance restent précieux.", "Couleur documentée", 0, 0},
            {"Entités hostiles / ennemis", "Slime jaune", "Gelée chargée d'étincelles.", "Le choc devient dangereux contre l'équipement métallique. Éviter d'empiler arme métal et armure métal contre lui si possible.", "Couleur documentée", 0, 0},
            {"Entités hostiles / ennemis", "Slime ambré", "Gelée collante qui ralentit.", "Moins létal qu'un slime élémentaire pur, mais très pénible : il ouvre des fenêtres pour ses alliés.", "Couleur documentée", 0, 0},
            {"Entités hostiles / ennemis", "Slime doré", "Gelée attirée par les objets brillants.", "Il peut justifier une trouvaille étrange, mais ne doit pas être vu comme une source fiable d'or. Sa valeur vient surtout du risque et de la rareté.", "Rare", 0, 0},
            {"Entités hostiles / ennemis", "Slime chromatique", "Gelée instable aux réactions difficiles.", "Il change de réaction selon l'impact reçu. L'observer vaut souvent mieux que brûler toutes ses ressources au premier tour.", "Rare instable", 0, 0},
            {"Entités hostiles / ennemis", "Monstres soigneurs crédibles", "Tous les ennemis ne savent pas boire ou lancer une potion.", "Un loup, un slime simple ou une bête sauvage ne comprend pas l'alchimie. Les soins ennemis doivent venir d'un shaman, chamane, apothicaire, oracle, mage ou monstre spécial avec une raison lore. Les IA alliées peuvent aussi soigner joueur ou allié si leur personnalité et leur inventaire s'y prêtent.", "Règle de cohérence", 0, 0},
            {"Entités hostiles / ennemis", "Loup affamé", "Prédateur opportuniste.", "Le loup teste les proies faibles. Son instinct le pousse surtout vers les proies faibles, isolées ou déjà blessées.", "Informations communes", 0, 0},
            {"Entités hostiles / ennemis", "Racine étrangleuse", "Plante qui gagne quand la cible cesse de bouger.", "Elle ne cherche pas seulement à blesser : elle ferme le terrain, use les tours et prépare les autres plantes.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Squelette archer rouillé", "Mort-vivant à distance, fragile mais pénible.", "Ses os supportent mal les coups lourds, mais le poison et le saignement ne changent presque rien. Le métal rouillé peut cacher de petits composants.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Oracle fissuré", "Support étrange qui lit mal l'avenir.", "Il peut soutenir, gêner ou annoncer des dangers. À éliminer tôt si le combat devient long.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Bandit apothicaire", "Humanoïde capable de soins crédibles.", "Contrairement à un loup ou un slime, il sait utiliser fioles et remèdes. Le focus soutien devient logique.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Chauve-souris des cavernes", "Créature rapide et fragile.", "Elle ne gagne pas par force brute : elle gêne, esquive et profite des endroits sombres. Les attaques précises ou de zone courte l'empêchent de tourner longtemps.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Rat géant", "Bête nuisible de bas niveau.", "Le rat géant sert de première menace sale : faible seul, plus dangereux si le joueur arrive déjà blessé ou empoisonné.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Kobold paniqué", "Petit draconide nerveux.", "Il n'a pas la puissance d'un vrai dragon, mais garde une logique de draconide : méfiant, territorial et parfois plus dangereux près d'un nid.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Araignée venimeuse", "Insectoïde toxique.", "Son poison force à choisir entre finir vite ou consommer une réponse. Les variantes toxiques résistent mieux aux poisons banals.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Sanglier sauvage", "Bête qui encaisse et charge.", "Il punit les joueurs trop confiants : esquive ou entrave avant impact, puis contre proprement après la charge.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Ours brun", "Bête massive et directe.", "L'ours ne demande pas une énigme : il demande de respecter sa force. Garde, distance, entrave et soins préparés valent mieux qu'un duel d'orgueil.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Squelette fissuré", "Mort-vivant fragile mais insensible aux mauvais outils.", "Ses os cassent sous les chocs lourds. Poison et saignement gaspillent souvent un tour.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Goule affamée", "Mort-vivant agressif.", "Plus organique qu'un squelette, mais toujours lié à la mort. Le feu et la purification restent de meilleures pistes que les poisons ordinaires.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Alchimiste renégat", "Humain dangereux avec fioles.", "Il peut faire plus qu'attaquer : affaiblir, empoisonner, brûler ou se sauver avec un remède. L'interrompre évite beaucoup de dégâts indirects.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Mage renégat", "Humain arcanique instable.", "Il dépend de sa canalisation et de son catalyseur. Lui laisser plusieurs tours revient souvent à accepter une altération ou une faille.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Orc mineur", "Combattant lourd encore lisible.", "Son niveau reste bas, mais son corps encaisse mieux qu'un gobelin. L'allonge et l'entrave évitent de subir son rythme.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Orc berserker", "Briseur sauvage dangereux blessé.", "Plus il prend de coups, plus l'échange devient risqué. Une faille ouverte doit être exploitée vite, pas entretenue par orgueil.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Chevalier corrompu", "Humain en armure devenu menace.", "L'armure réduit les petits coups, mais le choc, le marteau, la faille ou le brise-garde ouvrent de vraies fenêtres.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Méphaïte de braise", "Petit élémentaire de feu.", "Il supporte mal qu'on le traite comme une bête normale : feu contre feu déçoit, froid propre et voile élémentaire aident davantage.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Élémentaire instable", "Énergie condensée difficile à lire.", "Sa réaction peut changer selon l'élément et l'impact. Observer le premier échange évite de nourrir sa résistance.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Anomalie arcanique instable", "Erreur magique vivante.", "Elle n'obéit pas toujours aux catégories simples. La stabilité, le voile élémentaire et les attaques mesurées comptent plus que les automatismes.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Armure morte", "Construction hantée ou mort-vivant blindé.", "Poison et saignement sont presque inutiles. Cherche fissure, choc, marteau, purification ou rupture du noyau hanté.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Dragon mineur", "Jeune menace draconique.", "Même jeune, un dragon impose préparation : élément, souffle, écailles et butin fragile. Le battre salement peut gâcher ce qu'il laisse.", "Observation de terrain", 0, 0},
            {"Entités passives / alliées", "Aventurier errant", "Humain ou semi-humain généré par l'arène.", "Un adversaire sans légende connue, avec son propre nom, sa race et sa classe. Selon les rencontres, il peut devenir rival ou visage familier.", "Fiche archivée", 0, 0},
            {"Invocations", "Invocation mineure", "Renfort temporaire appelé par certaines classes.", "Les invocations suivent les mêmes règles globales de ciblage que les monstres, mais peuvent appartenir au joueur ou à l'ennemi.", "Fiche archivée", 0, 0},
            {"Invocations", "Esprit mineur", "Petit esprit stable et facile à maintenir.", "Invocation simple : faible coût de slot, maintien léger, attaque régulière et petite régénération. Elle sert de base aux invocateurs.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Serviteur osseux", "Invocation nécromantique solide.", "Le serviteur osseux peut sacrifier une partie de son ossature pour frapper plus fort. Son sacrifice final inflige une rupture dangereuse.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Flamme kitsune", "Flamme liée aux kitsunes.", "La Flamme kitsune brûle vite, frappe fort et reste fragile. Elle correspond particulièrement à Aoi et aux voies de feu liées aux pactes.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Bête arcanique", "Invocation lourde consommant plus de slots.", "La Bête arcanique occupe deux slots, encaisse mieux et se nourrit du mana ambiant. Elle transforme le duel en affrontement de groupe.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Expérience instable", "Créature dangereuse de laboratoire.", "L'Expérience instable tape fort, dure peu et peut exploser. Elle correspond aux expériences de Fail et aux risques de laboratoire.", "Fiche d'invocation", 0, 0},
            {"Invocations", "Ombre récente", "Ombre née d'un lien violent.", "L'Ombre récente suit les traces de Hazak. Certaines ombres ne naissent qu'après un acte violent ou un lien trop sombre.", "Secret partiel", 0, 0},
            {"Invocations", "Éclat zodiacal", "Fragment magique lié aux signes.", "L'Éclat zodiacal tire un signe sur treize. Le treizième signe peut prolonger son existence et rappelle les anciens sorts zodiacaux liés à Kanadé.", "Fiche d'invocation", 0, 0},
            {"Boss", "???", "Nom inconnu.", "Un boss ne révèle son vrai nom que s'il le prononce, à l'entrée, à 50%, avant sa défaite, ou via des écrits crédibles trouvés avant.", "Identité verrouillée", 0, 0},
            // EN: Special characters are not all revealed from the start.
            // FR: Les personnages spéciaux ne sont pas tous révélés dès le départ.
            {"Personnages spéciaux", "Les bras cassés", "Groupe d'aventuriers déjà connu.", "Hazak, Fail, Aoi, Kanadé et Sanctus forment un groupe de cinq héros chaotiques. Ils ont libéré plusieurs villages et habitants, mais leur ego et leur rapport très libre aux règles ne plaisent pas à tout le monde.", "Groupe connu", 0, 0},
            {"Personnages spéciaux", "Hazak", "Assassin elfe noir, sombre et efficace.", "Hazak prend les gens de haut, cherche la victoire et protège Hestia d'une violence trop traumatisante. Il fait partie des Bras cassés.", "Fiche connue", 0, 0},
            {"Personnages spéciaux", "Fail", "Mage fou de race fée, imprévisible et dangereux.", "Fail expérimente, dérègle les situations et possède parfois des sorts uniques. Son contrat de non-agression avec Hazak n'empêche pas tout le monde de le craindre.", "Fiche connue", 0, 0},
            {"Personnages spéciaux", "Aoi", "Kitsune timide, mage de flammes et invocatrice de flammes.", "Aoi protège ses incantations et devient très dangereuse si on lui laisse le temps de préparer sa magie. Elle fait partie des Bras cassés.", "Fiche connue", 0, 0},
            {"Personnages spéciaux", "Kanadé", "Semi-dragonne rageuse aux sorts zodiacaux.", "Kanadé râle souvent, mais elle avance quand même. Ses sorts les plus étranges tournent autour des treize signes du zodiaque.", "Fiche connue", 0, 0},
            {"Personnages spéciaux", "Sanctus", "Protecteur semi-mage lié à la lumière.", "Sanctus protège, entrave et s'appuie sur une croyance lumineuse. Il fait partie des Bras cassés et sert souvent de point d'ancrage au groupe.", "Fiche connue", 0, 0},
            {"Personnages spéciaux", "Matt (PRO)", "Identité spéciale non confirmée.", "Ce personnage spécial doit être débloqué par rencontre, rumeur fiable, arène ou progression. Tant que ton personnage ne le connaît pas, le bestiaire garde volontairement ses détails cachés.", "Identité verrouillée", 0, 0},
            {"Personnages spéciaux", "Skuro", "Identité spéciale non confirmée.", "Ce personnage spécial doit être débloqué progressivement. Le registre sait qu'une silhouette lourde existe, mais pas encore pourquoi elle compte.", "Identité verrouillée", 0, 0},
            {"Personnages spéciaux", "Hestia", "Identité spéciale non confirmée.", "Ce personnage spécial doit être débloqué par l'histoire, une rencontre ou une information crédible. Le bestiaire ne révèle pas ses origines à l'avance.", "Identité verrouillée", 0, 0},
            {"Personnages spéciaux", "Henrique", "Identité spéciale non confirmée.", "Ce personnage spécial doit être découvert en jeu. Le registre garde ses liens et capacités cachés tant que le joueur ne les a pas vus.", "Identité verrouillée", 0, 0},
            {"Personnages spéciaux", "Louis", "Identité spéciale non confirmée.", "Ce personnage spécial doit être débloqué progressivement. Le bestiaire évite de spoiler son rôle, ses amis et son style de combat.", "Identité verrouillée", 0, 0},
            {"Personnages spéciaux", "Trexof", "Identité spéciale non confirmée.", "Ce personnage spécial doit être débloqué par progression ou rencontre. Ses liens et son vrai style restent cachés au départ.", "Identité verrouillée", 0, 0},
            {"Personnages spéciaux", "Mattzelda", "Identité spéciale non confirmée.", "Ce personnage spécial doit être débloqué progressivement. Le registre ne dévoile pas encore son profil de colosse humain.", "Identité verrouillée", 0, 0},
            {"Légendes / contes", "Légendes de bibliothèque", "Rayonnage de récits du monde.", "La bibliothèque classe des légendes longues, des histoires pour enfant et des récits incomplets. Ces textes ajoutent du lore sans bloquer le voyage.", "Archive de bibliothèque", 0, 0},
            {"Légendes / contes", "Légendes de salle de boss", "Certains lieux racontent plus qu'un combat.", "Certaines salles anciennes ou certains conteurs laissent parfois une légende rare. Le registre les traite comme des moments spéciaux, pas comme un rituel obligatoire avant chaque boss.", "Rumeur de salle", 0, 0},
            {"Légendes / contes", "Conte des Bras cassés", "Un conte populaire parle d'un groupe trop bruyant pour être discret.", "Les enfants retiennent surtout la version drôle : cinq héros sauvent un village, se disputent avec les règles, puis repartent comme si le chaos était une méthode officielle. La vraie réputation reste plus nuancée.", "Conte connu", 0, 0},
            {"Légendes / contes", "Conte pour enfant : les cinq héros trop bruyants", "Version adoucie et drôle des Bras cassés.", "Cette entrée est rangée dans le Registre des légendes. Elle sert à lire le lore progressivement, sans mélanger les récits avec les monstres ou les boss.", "Conte connu", 0, 0},
            {"Légendes / contes", "Origine du nom des Bras cassés", "Rumeur sur le bras perdu de Hazak.", "On raconte que Hazak, chef du groupe, s'est fait couper le bras par un orc avant de réussir à se régénérer après deux semaines. Le nom vient aussi du fait que le groupe fait beaucoup de conneries, mais finit presque toujours par réussir.", "Rumeur connue", 0, 0},
            {"Matériaux et plantes", "Fleur bleue de montagne", "Plante calme et rare.", "Clin d'œil à la fleur bleue de Zelda BOTW. Elle sert aux remèdes, quêtes et secrets liés aux plantes.", "Renseignement de base", 0, 0},
            {"Matériaux et plantes", "Oreille de gobelin", "Composant commun de monstre.", "Matériau simple récupérable sur certains gobelins. Utile pour les premières ventes, les contrats et les crafts simples.", "Loot possible", 0, 0},
            {"Matériaux et plantes", "Peau de bête robuste", "Matériau de réparation épaisse.", "Sert déjà à fabriquer un kit moyen via rafistolage renforcé. Aussi recherchée pour les armures, les tanks et l'équipement de survie.", "Loot rare", 0, 0},
            {"Matériaux et plantes", "Fil d'ombre", "Composant sombre rare.", "Lié aux assassins, aux morts-vivants et aux ombres de Hazak. Les nécromanciens évitent d'en parler devant les guildes.", "Secret partiel", 0, 0},
            {"Matériaux et plantes", "Braise kitsune", "Braise magique instable.", "Composant lié à Aoi, aux flammes kitsune, à certaines potions avancées et aux invocations.", "Rare", 0, 0},
            {"Matériaux et plantes", "Fragment d'écaille draconique", "Fragment défensif rare.", "Matériau très solide, recherché pour protections rares, armures lourdes et crafts semi-dragons.", "Rare", 0, 0},
            {"Matériaux et plantes", "Noyau instable", "Cœur magique dangereux.", "Sert de base aux expériences de Fail, à l'alchimie risquée, aux objets explosifs et aux invocations instables.", "Dangereux", 0, 0},
            {"Matériaux et plantes", "Qualités de matériaux", "Les qualités modifient valeur et efficacité.", "Impur/faible qualité compte moins, normal compte normalement, pur/haute qualité compte plus et exceptionnel peut déclencher de petites particularités de craft.", "Renseignement commun", 0, 0},
            {"Effets et altérations", "Brûlure", "Dégât persistant de feu.", "La brûlure grignote les PV, menace les plantes et peut abîmer certains composants. Baume, voile élémentaire ou résistance au feu la rendent plus supportable.", "Effet connu", 0, 0},
            {"Effets et altérations", "Poison", "Altération lente et dangereuse.", "Très utile contre les chairs vivantes, beaucoup moins contre morts-vivants, slimes simples ou constructions. L'antidote reste le contre le plus direct.", "Effet connu", 0, 0},
            {"Effets et altérations", "Givre", "Froid qui ralentit et fragilise le rythme.", "Le givre punit les combattants qui comptent sur l'initiative. Les sources chaudes, anti-givre et protections élémentaires l'atténuent.", "Effet connu", 0, 0},
            {"Effets et altérations", "Choc", "Décharge amplifiée par le métal.", "Le choc devient plus sévère si arme et armure métalliques conduisent l'impact. Potion isolante et équipement adapté comptent beaucoup.", "Effet connu", 0, 0},
            {"Effets et altérations", "Saignement", "Perte de sang après entaille.", "Fort contre les êtres vivants, faible contre os, gelées et constructions. Les dagues et munitions barbelées l'exploitent bien.", "Effet connu", 0, 0},
            {"Effets et altérations", "Affaiblissement", "Force offensive réduite pendant quelques tours.", "L'affaiblissement ne tue pas vite, mais il protège énormément sur les longs combats. Purification ou attente peuvent le retirer.", "Effet étudié", 0, 0},
            {"Effets et altérations", "Faille ouverte", "Défense temporairement fissurée.", "La faille augmente les dégâts reçus. Elle se combine très bien avec une attaque lourde, mais doit être exploitée avant de se refermer.", "Effet étudié", 0, 0},
            {"Effets et altérations", "Voile élémentaire", "Protection courte contre plusieurs altérations.", "Réduit la force des brûlures, poisons, givre, choc et saignement. Excellent avant un boss ou une zone à élément dominant.", "Effet étudié", 0, 0},
            {"Effets et altérations", "Suture de mana", "Régénération courte par mana stabilisé.", "Ne remplace pas une potion d'urgence, mais soutient un combat long si le catalyseur reste fiable.", "Effet étudié", 0, 0},
            {"Objets rares", "Supports magiques", "Parchemins et grimoires ne servent pas au même usage.", "Un parchemin est un consommable à usage unique : un non-mage peut déclencher certains effets, et un mage peut aussi s'en servir. Un grimoire permet d'apprendre durablement un sort, mais demande classe compatible, niveau, catalyseur et stabilité. Certains sorts existent seulement en grimoire, d'autres seulement en parchemin.", "Renseignement de bibliothèque", 0, 0},
            {"Objets rares", "Catalyseurs de mage", "La magie dépend aussi de l'objet qui la canalise.", "Un bâton, une baguette, une pierre ou un autre vrai catalyseur rend la magie plus stable. Un catalyseur brisé ou épuisé fait baisser la réussite. Une arme non magique peut forcer le sort à passer, mais avec moins de contrôle.", "Renseignement de bibliothèque", 0, 0},
            {"Objets rares", "Purification mineure", "Parchemin sans grimoire courant.", "Cette magie existe surtout sur papier : elle arrache plusieurs altérations simples puis disparaît avec le support. Un mage peut l'utiliser aussi, même s'il sait déjà lancer d'autres sorts.", "Renseignement de bibliothèque", 0, 0},
            {"Objets rares", "Suture de mana", "Grimoire sans parchemin courant.", "Sort étudiable par les vrais canalisateurs : il ne rend pas tous les PV immédiatement, mais pose une régénération courte. Mauvais catalyseur = réussite plus basse.", "Renseignement de bibliothèque", 0, 0},
            {"Objets rares", "Ronces occultes", "Grimoire sans parchemin courant.", "Sort d'entrave sombre : ronces de mana, poison léger et affaiblissement. Il demande un vrai profil magique et un catalyseur correct.", "Renseignement de bibliothèque", 0, 0},
            {"Objets rares", "Venin rampant", "Parchemin sans grimoire courant.", "Magie jetable : utilisable par mage ou non-mage, mais perdue à l'usage. Elle empoisonne et fatigue sans donner un sort durable au personnage.", "Renseignement de bibliothèque", 0, 0},
            {"Objets rares", "Munitions spéciales", "Les armes à distance dépendent du type de munition.", "Arc = flèches, arbalète = carreaux, équipement de lancer = couteaux. Le joueur choisit la munition à chaque tir si plusieurs lots sont compatibles. Si aucune munition compatible n'existe, l'arme ne tire pas : elle ne sert qu'en défense d'urgence, sans devenir magique par hasard.", "Étude active", 0, 0},
            {"Races", "Spécialités de classes en combat", "Les classes commencent à se ressentir davantage.", "Assassin/ombrelame peuvent ouvrir un saignement, classes lourdes s'ancrent davantage, mages peuvent laisser une trace élémentaire faible, classes sacrées peuvent récupérer un peu sous pression. Les effets restent légers : la guilde enseigne d'abord les bases avant les vraies spécialisations.", "Étude active", 0, 0},
            {"Races", "Résistances et faiblesses", "Les éléments réagissent avec races, classes et matières.", "Le feu, le froid, le poison, le choc, le saignement et les vulnérabilités ne doivent pas être universels. Le métal attire mieux certains chocs, les plantes craignent davantage le feu, les corps organiques réagissent différemment au poison, et l'équipement porté peut aider ou aggraver l'effet.", "Étude active", 0, 0},
            {"Matériaux et plantes", "Dégradation de récolte", "La façon de combattre peut abîmer un composant.", "Flèches, feu, explosions ou armes lourdes peuvent produire des composants troués, brûlés ou écrasés. La maîtrise d'arme réduit peu à peu ces risques.", "Renseignement commun", 0, 0},
            {"Matériaux et plantes", "Récolte propre", "Une récolte bien faite peut améliorer une qualité.", "Une récolte propre peut faire passer impur vers normal, normal vers pur/haute qualité, puis exceptionnel. L'achat direct d'exceptionnel reste impossible.", "Renseignement commun", 0, 0},
            {"Classes", "Gladiateur", "Classe de contact orientée arène.", "Le Gladiateur cherche la pression constante : pas aussi noble qu'un Chevalier, pas aussi suicidaire qu'un Berserker, mais très bon pour garder l'adversaire dans le rythme qu'il impose.", "Classe jouable", 0, 0},
            {"Classes", "Ravageur", "Briseur frontal très risqué.", "Le Ravageur existe pour casser une garde, une armure ou une ligne ennemie. Il frappe fort, mais son style laisse des ouvertures si le combat dure trop.", "Classe jouable", 0, 0},
            {"Classes", "Maître d'armes", "Combattant martial polyvalent.", "Le Maître d'armes préfère comprendre l'équipement avant d'agir. Il n'a pas encore de système complet de styles, mais sa présence prépare les futures spécialisations d'armes.", "Classe jouable", 0, 0},
            {"Classes", "Pisteur", "Classe de distance liée aux traces.", "Le Pisteur donne une identité plus sauvage aux combattants à distance : lire un terrain, prévoir une proie, choisir la munition utile plutôt que tirer au hasard.", "Classe jouable", 0, 0},
            {"Classes", "Frondeur tactique", "Harceleur à distance économique.", "Le Frondeur tactique représente les profils qui gagnent par petites ouvertures, gestion de consommables et gêne répétée. Il sera utile quand les munitions auront encore plus de poids.", "Classe jouable", 0, 0},
            {"Classes", "Éclaireur d'élite", "Distance mobile et préparation.", "L'Éclaireur d'élite sert à ceux qui veulent commencer les combats avec de l'information, choisir les cibles et sortir avant que la ligne casse.", "Classe jouable", 0, 0},
            {"Classes", "Électromancien", "Mage du choc et des réactions métalliques.", "L'Électromancien prépare les futurs liens entre équipement métallique, humidité, choc et interruptions. Il est fort quand l'ennemi porte trop de métal sans protection.", "Classe jouable", 0, 0},
            {"Classes", "Gravemage", "Mage de runes lentes.", "Le Gravemage ne cherche pas toujours le gros chiffre immédiat. Il prépare, verrouille, grave et force le combat à respecter une cadence moins nerveuse.", "Classe jouable", 0, 0},
            {"Classes", "Miragien", "Mage d'illusions et d'erreurs.", "Le Miragien annonce les mécaniques futures de confusion, faux choix, images et lectures trompeuses, sans donner gratuitement les pouvoirs d'Inakari.", "Classe jouable", 0, 0},
            {"Classes", "Marionnettiste", "Invocateur à fils et contrôle.", "Le Marionnettiste préfère manipuler une présence fragile plutôt que se battre seul. Sa logique prépare les invocations contrôlées et les ennemis à pantins.", "Classe jouable", 0, 0},
            {"Classes", "Totémiste", "Invocateur de soutien de zone.", "Le Totémiste pose une présence qui aide le combat à durer : protection, patience, zone de repos ou pression rituelle selon les futurs totems.", "Classe jouable", 0, 0},
            {"Classes", "Corbeaumancien", "Invocateur de familier et mauvais présages.", "Le Corbeaumancien donne une place aux familiers de reconnaissance, petites malédictions et messages. Il peut devenir très lore avec les cimetières et les ruines.", "Classe jouable", 0, 0},
            {"Classes", "Oracle", "Support de lecture du danger.", "L'Oracle n'est pas forcément un soigneur pur : il devine, prévient, détourne et explique parfois une menace avant que l'équipe comprenne pourquoi elle saigne.", "Classe jouable", 0, 0},
            {"Classes", "Protecteur", "Tank d'interception.", "Le Protecteur sert à incarner la garde d'un allié précis. Il ne tape pas forcément fort, mais il existe pour décider qui a le droit d'atteindre la cible fragile.", "Classe jouable", 0, 0},
            {"Classes", "Médecin de terrain", "Support pratique et non sacré.", "Le Médecin de terrain permet d'avoir du soin sans forcément passer par la foi ou la lumière. Pansements, remèdes, stabilisation et survie deviennent son identité.", "Classe jouable", 0, 0},
            {"Classes", "Chevalier draconique", "Hybride martial à écailles.", "Le Chevalier draconique prépare un style entre arme lourde, résistance, souffle mineur et matériaux draconiques, sans devenir un dragon complet gratuitement.", "Classe jouable", 0, 0},
            {"Classes", "Rôdeur arcanique", "Hybride distance-magie.", "Le Rôdeur arcanique mélange munitions, terrain et magie légère. Il devrait aimer les flèches spéciales et les grimoires utilitaires.", "Classe jouable", 0, 0},
            {"Classes", "Moine solaire", "Hybride discipline-lumière.", "Le Moine solaire ajoute un profil de contact plus lumineux que le Moine classique, avec une identité de contre-pression et purification future.", "Classe jouable", 0, 0},
            {"Classes", "Cuisinier de guilde", "Artisan des rations et petits buffs.", "Le Cuisinier de guilde n'est pas une blague gratuite : en aventure longue, une bonne ration peut éviter de gaspiller une potion chère avant un boss.", "Classe jouable", 0, 0},
            {"Classes", "Cartographe", "Artisan de lecture de zone.", "Le Cartographe sert à renforcer l'exploration : cartes, dangers probables, coffres, traces, biomes et contrats de terrain.", "Classe jouable", 0, 0},
            {"Classes", "Récupérateur", "Artisan du loot propre.", "Le Récupérateur représente le joueur qui garde tout, démonte tout et finit par fabriquer une solution avec trois bouts de cuir et un clou suspect.", "Classe jouable", 0, 0},
            {"Entités hostiles / ennemis", "Moustique des hautes herbes", "Petit insectoïde des plaines.", "Faible seul, mais pénible quand il arrive après un combat déjà long. Son rôle est de rappeler que toutes les menaces ne doivent pas être énormes pour coûter une ressource.", "Entrée de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Pousse mordante", "Jeune plante hostile.", "Elle mord les bottes, accroche les tissus et apprend au joueur que même une plante faible peut gêner le mouvement si on la sous-estime.", "Entrée de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Renard cendré", "Petite bête attirée par les feux.", "Il rôde autour des campements et des restes de braises. Pas vraiment démoniaque, mais assez malin pour sentir où un sac a été mal fermé.", "Entrée de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Slime rose nerveux", "Gelée bondissante.", "La couleur rose n'est pas forcément mignonne : cette gelée rebondit, change d'angle et peut rendre un combat simple plus ridicule que prévu.", "Entrée de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Armure cabossée", "Construction hantée faible.", "Ancienne protection vide qui avance encore. Le poison ne sert presque à rien ; les fissures, chocs et armes lourdes racontent mieux la solution.", "Entrée de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Mage de verre", "Lanceur fragile mais dangereux.", "Le Mage de verre tombe vite si on l'atteint, mais il peut brûler une mauvaise décision en un tour. Le laisser libre est souvent plus cher qu'une potion.", "Entrée de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Golem d'atelier", "Construction créée pour travailler, pas discuter.", "Il frappe comme une machine qui n'a jamais compris la notion de pause. Le démonter proprement peut garder de meilleurs matériaux.", "Entrée de terrain", 0, 0},
            {"Habitats / zones", "Campement abandonné", "Micro-zone de repos piégé.", "Un feu éteint, des sacs éventrés et une corde trop bien placée peuvent annoncer bandits, bêtes opportunistes ou une rumeur utile avant le prochain chemin.", "Carte mineure", 0, 0},
            {"Habitats / zones", "Pont marchand", "Passage étroit et social.", "Un pont concentre embuscades, taxes, contrôles, rencontres de PNJ et choix de fuite. Bon lieu pour tester intimidation, réputation et contrats.", "Carte mineure", 0, 0},
            {"Habitats / zones", "Clairière aux braises", "Zone forestière marquée par le feu.", "Les braises attirent renards cendrés, slimes rouges, kitsune rumeurs et plantes sèches. Un bon endroit pour faire exister le feu sans lancer un boss.", "Carte mineure", 0, 0},
            {"Boss", "??? - boss non révélé", "Identité verrouillée.", "Cette entrée volontairement floue rappelle que le nom d'un boss ne doit pas être ajouté gratuitement. Il faut une révélation directe ou un écrit crédible.", "Verrouillé", 0, 0},
            {"Boss", "Fitoria", "Ange de jugement lumineux.", "Fitoria juge, enchaîne, soigne de façon contrôlée et libère Sentence céleste après son passage sous les 50% PV.", "Révélé par le registre initial", 1, 0},
            {"Boss", "Zelef", "Démon de corruption et de sang noir.", "Zelef peut voler de la vie, corrompre et laisser une Corrosion présente si le joueur perd. Le vaincre peut rendre ce qu'il a pris.", "Révélé par le registre initial", 1, 0},
            {"Boss", "Atlas", "Protecteur universel déchu.", "Atlas encaisse avec ses plaques, fissures et contre-gardes. Son Dernier rempart transforme sa défense en menace.", "Révélé par le registre initial", 1, 0},
            {"Boss", "Lyknir", "Écho de la Meute.", "Lyknir traque les habitudes du joueur. Son instinct de prédateur n'est pas moral : il lit une proie, pas une faute.", "Trace de boss", 0, 0},
            {"Boss", "Grinka", "Reine gobeline de l'avarice.", "Grinka transforme le combat en dette, taxe et enchère gobeline. Si elle gagne, certaines pertes deviennent réelles jusqu'à revanche.", "Trace de boss", 0, 0},
            {"Boss", "Fragment de Thamarys", "Fragment draconique.", "Thamarys possède des écailles à briser et un souffle d'origine. Le combattre revient à apprendre quelle défense draconique est encore debout.", "Trace de boss", 0, 0},
            {"Boss", "Mojo", "Esprit mythique de la forêt.", "Mojo peut être vaincu brutalement ou apaisé. La victoire respectueuse ouvre une récompense et une lecture très différentes.", "Trace de boss", 0, 0},
            {"Boss", "Reflet d'Inakari", "Reflet kitsune trompeur.", "Inakari ment avec élégance, propose de fausses récompenses et transforme les choix en miroirs dangereux.", "Trace de boss", 0, 0},
            {"Boss", "L'Anomalie", "Nom presque effacé.", "Aucun dossier exploitable : les témoins sont morts, brisés ou réinitialisés. Le registre refuse les informations tactiques avant une vraie confrontation.", "Nom seulement", 0, 0},
            {"Boss", "Manifestation de Moiran", "Trace de destin avancée.", "Moiran reste un boss avancé lié aux lignes possibles et aux altérations, mais il n'est plus classé dans les boss particuliers finaux. Les détails tactiques restent à découvrir par vraie rencontre.", "Trace de boss", 0, 0},
            {"Boss", "Écho fragmenté d'Obérion", "Nom presque effacé.", "Fragment d'origine trop haut pour un registre ordinaire. Les informations exploitables restent bloquées tant que le joueur ne l'a pas réellement affronté.", "Trace finale verrouillée", 0, 0},
            {"Secret", "Avatar affaibli de FireFlight", "Identité finale scellée.", "Aucune information avant la fin : cette entrée n'est censée exister qu'après avoir surmonté toutes les épreuves majeures concernées.", "Verrouillé final secret", 0, 0},
            {"Divinités / lore", "FireFlight", "Créateur du monde et trace du créateur.", "FireFlight peut être personnage, créateur, test final et regard derrière le jeu. Ses dialogues changent avec les personnages spéciaux et les altérations.", "Lore sensible", 0, 0},
            {"Divinités / lore", "Moiran", "Trace de destin.", "La trace existe dans le lore du destin, sans être rangée avec les figures particulières finales comme L'Anomalie, Obérion ou FireFlight.", "Trace divine", 0, 0},
            {"Divinités / lore", "Obérion", "Dieu universel et père des primordiaux.", "Obérion complet dépasse le combat actuel. Seuls des fragments approuvés sont affrontables sans briser l'échelle du monde.", "Trace divine", 0, 0},
            {"Objets rares", "Invitations scellées", "Preuves nécessaires pour FireFlight.", "La première victoire contre chaque boss requis laisse une preuve unique. Refaire le même combat ne crée pas une nouvelle invitation. Quand toutes les preuves nécessaires existent, l'entrée du test final accepte enfin de s'ouvrir.", "Système final secret", 0, 0},
            {"Objets rares", "Fragments de boss avancés", "Matériaux uniques de boss.", "Fragments de nom perdu, miroir fendu, noyau de version instable, sceaux et traces divines nourrissent les crafts ou reliques majeures.", "Fiche archivée", 0, 0},
            {"Objets rares", "Particularités de craft", "Effets faibles nés de matériaux exceptionnels.", "Un objet crafté peut recevoir une petite particularité si plus de 50% de sa valeur de craft vient de matériaux exceptionnels. Les classes d'artisanat augmentent maintenant légèrement cette chance.", "Étude active", 0, 0},
            {"Habitats / zones", "Traces de territoire", "Les zones racontent parfois le monstre avant le combat.", "Griffures sur un arbre, os déplacés, gel anormal, suie récente ou silence soudain peuvent ajouter une entrée sans tuer la créature. Une trace ne donne pas tout : elle confirme surtout l'existence et le terrain probable.", "Méthode d'observation", 0, 0},
            {"Habitats / zones", "Cadavre étudié", "Une victoire n'est pas la seule manière d'apprendre.", "Un corps ancien, une mue, un nid abandonné ou une arme cassée peut révéler faiblesse, résistance ou butin possible. Les informations restent moins fiables qu'une vraie rencontre, mais elles aident à préparer la chasse.", "Méthode d'observation", 0, 0},
            {"Matériaux et plantes", "Qualité de dépouille", "La manière de vaincre change ce qui reste.", "Une dépouille brûlée, explosée ou percée trop violemment peut donner moins de composants propres. Une mise à mort contrôlée, une arme adaptée ou une récolte calme augmente les chances de matière propre.", "Méthode d'observation", 0, 0},
            {"Objets rares", "Pages de terrain annotées", "Certaines pages ne donnent pas un sort, mais une lecture du monde.", "Les pages trouvées ou achetées peuvent ajouter une fiche, améliorer une famille ou donner une piste de butin. Elles ne remplacent pas l'expérience, mais peuvent éviter de mourir idiot devant une faiblesse évidente.", "Renseignement de bibliothèque", 0, 0},
            {"Boss", "Rumeurs de boss", "Une rumeur ne suffit pas à révéler un nom.", "Le registre peut noter une silhouette, un titre ou un comportement sans donner l'identité complète. Le nom reste caché tant que le boss ne l'a pas offert lui-même ou qu'un écrit fiable ne le confirme pas.", "Renseignement incomplet", 0, 0},
            {"Classes jouables", "Hallebardier", "Contact à grande allonge.", "Profil martial pensé pour tenir une ligne : il aime commencer le combat à bonne distance, punir les approches et éviter d'être encerclé.", "Classe jouable", 0, 0},
            {"Classes jouables", "Martelier", "Briseur lourd orienté armures.", "Il apporte une réponse naturelle aux constructions, armures mortes et ennemis trop solides pour les petites lames.", "Classe jouable", 0, 0},
            {"Classes jouables", "Arquebusier expérimental", "Distance lourde et risquée.", "Tire très fort mais demande préparation, munitions et acceptation du raté. Une classe drôle si on aime les gros coups qui ne pardonnent pas.", "Classe jouable", 0, 0},
            {"Classes jouables", "Bibliomancien", "Mage de savoir et grimoires.", "Il valorise les livres, indices et préparations. Fragile, mais très intéressant pour un joueur qui veut lier magie et progression de connaissance.", "Classe jouable", 0, 0},
            {"Classes jouables", "Reliquaire vivant", "Invocateur d'artefacts temporaires.", "Il ne se contente pas d'appeler des bêtes : il anime des objets, charges et reliques instables, avec un risque cohérent avec le craft futur.", "Classe jouable", 0, 0},
            {"Classes jouables", "Archiviste de terrain", "Support de connaissance.", "Il soutient moins par les dégâts que par l'identification, la préparation et la lecture des menaces. Très cohérent avec le bestiaire progressif.", "Classe jouable", 0, 0},
            {"Classes jouables", "Herboriste de combat", "Artisan de remèdes en situation dangereuse.", "Il donne une raison jouable de valoriser les plantes, antidotes et récoltes propres sans transformer tout le monde en mage.", "Classe jouable", 0, 0},
            {"Entités hostiles / ennemis", "Faux péager souriant", "Humain hostile de route.", "Un bon exemple d'ennemi non monstrueux : peu de résistance naturelle, mais rusé, socialement pénible et parfois accompagné.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Escalier qui mord", "Piège-construction presque vivant.", "Les ruines ne doivent pas seulement contenir des monstres classiques. Certains dangers peuvent être une salle, un meuble ou un mécanisme qui refuse de rester mort.", "Observation de terrain", 0, 0},
            {"Entités hostiles / ennemis", "Slime d'argent silencieux", "Gelée rare absorbant les vibrations.", "Il encourage à ne pas spammer la même logique contre toutes les gelées rares : les sons, chocs et réactions peuvent être modifiés.", "Rare", 0, 0},
            {"Entités hostiles / ennemis", "Bibliothèque agressive", "Construction possédée par trop de notes.", "Menace de ruine idéale pour lier combat, livre, archive et humour noir. Elle peut justifier poussière arcanique, pages et savoir abîmé.", "Rare", 0, 0},
            {"Entités hostiles / ennemis", "Reine moustique de fièvre", "Insectoïde rare de marais.", "Elle rend les soins et antidotes importants, mais doit rester rare pour ne pas transformer chaque marais en punition permanente.", "Rare", 0, 0},
            {"Quêtes / guilde", "Contrats de service", "Petites missions utiles mais peu rentables.", "Les services donnent parfois faveurs, recommandations ou lettres plutôt que beaucoup d'or. Ils remplissent le monde sans devenir meilleur farm.", "Règle économique", 0, 0},
            {"Quêtes / guilde", "Contrats de bestiaire", "Missions de connaissance.", "Elles ne demandent pas toujours de tuer : observer, confirmer une famille ou récupérer un indice suffit parfois à faire progresser le registre.", "Règle de progression", 0, 0},
            {"Légendes / contes", "Objets qui racontent", "Un meuble, une porte ou un outil peut devenir une trace lore.", "La V2.08.00 ajoute davantage de petits récits pour que le contenu donne une impression de monde vivant sans forcer une lecture constante.", "Archive", 0, 0}
        };

        return entries;
    }

    std::vector<BestiaryPreviewEntry> filterEntries(const std::string& category)
    {
        std::vector<BestiaryPreviewEntry> filtered;

        for (BestiaryPreviewEntry entry : getEntries())
        {
            entry.encounters += BestiaryRuntimeProgress::getEncounterCount(entry.name);
            entry.kills += BestiaryRuntimeProgress::getKillCount(entry.name);
            entry.status = BestiaryRuntimeProgress::getStatusFor(entry.name, entry.status);

            if (category == "Tout" || entry.category == category)
            {
                filtered.push_back(entry);
            }
        }

        for (const BestiaryRuntimeRecord& record : BestiaryRuntimeProgress::getRecords())
        {
            bool alreadyListed = false;

            for (const BestiaryPreviewEntry& entry : filtered)
            {
                if (entry.name == record.name)
                {
                    alreadyListed = true;
                    break;
                }
            }

            if (!alreadyListed && (category == "Tout" || record.category == category))
            {
                filtered.push_back({
                    record.category,
                    record.name,
                    record.description,
                    record.description,
                    record.status,
                    record.encounters,
                    record.kills
                });
            }
        }

        for (const MaterialKnowledgeRecord& materialRecord : MaterialKnowledgeProgress::getRecords())
        {
            if (category != "Tout" && category != "Matériaux et plantes")
            {
                continue;
            }

            std::string label = materialRecord.name;

            if (!materialRecord.quality.empty() && materialRecord.quality != "normal")
            {
                label += " [" + materialRecord.quality + "]";
            }

            bool alreadyListed = false;

            for (const BestiaryPreviewEntry& entry : filtered)
            {
                if (entry.name == label)
                {
                    alreadyListed = true;
                    break;
                }
            }

            if (!alreadyListed)
            {
                std::string detail = "Journal réel : " + label
                    + " a été récupéré "
                    + std::to_string(materialRecord.discoveredQuantity)
                    + " fois. Catégorie : "
                    + materialRecord.category
                    + ". Poids de craft observé : "
                    + std::to_string(materialRecord.bestQualityWeight)
                    + ".";

                filtered.push_back({
                    "Matériaux et plantes",
                    label,
                    "Matériau réellement découvert pendant l'aventure.",
                    detail,
                    "Découvert en jeu",
                    materialRecord.discoveredQuantity,
                    0
                });
            }
        }

        return filtered;
    }


    // EN: calculateKnowledgeLevel declares or implements a focused behavior used by this module.
    // FR: calculateKnowledgeLevel déclare ou implémente un comportement précis utilisé par ce module.
    int calculateKnowledgeLevel(const BestiaryPreviewEntry& entry)
    {
        const std::string statusLower = lowerCopy(entry.status);

        if (statusLower.find("final secret") != std::string::npos
            || statusLower.find("verrouillé final") != std::string::npos
            || statusLower.find("verrouille final") != std::string::npos)
        {
            return 0;
        }

        if (statusLower.find("nom seulement") != std::string::npos
            || statusLower.find("trace finale verrouillée") != std::string::npos
            || statusLower.find("trace finale verrouillee") != std::string::npos)
        {
            return 1;
        }

        if (statusLower.find("verrouill") != std::string::npos
            || statusLower.find("identité verrouillée") != std::string::npos)
        {
            return 0;
        }

        int level = 0;

        if (statusLower.find("connu") != std::string::npos
            || statusLower.find("spécial") != std::string::npos
            || statusLower.find("renseignement") != std::string::npos
            || statusLower.find("document") != std::string::npos
            || statusLower.find("effet") != std::string::npos
            || statusLower.find("carte") != std::string::npos
            || statusLower.find("danger") != std::string::npos
            || statusLower.find("observation") != std::string::npos
            || statusLower.find("fiche") != std::string::npos
            || entry.encounters > 0)
        {
            level = 1;
        }

        if (entry.kills > 0
            || statusLower.find("combattu") != std::string::npos
            || statusLower.find("acheté") != std::string::npos
            || statusLower.find("lu") != std::string::npos
            || statusLower.find("étudi") != std::string::npos
            || statusLower.find("dangereuse") != std::string::npos
            || statusLower.find("valid") != std::string::npos)
        {
            level = 2;
        }

        if (entry.kills >= 3
            || statusLower.find("renseignement acheté") != std::string::npos
            || statusLower.find("spécial") != std::string::npos
            || statusLower.find("lore valid") != std::string::npos)
        {
            level = 3;
        }

        return level;
    }

    std::string knowledgeLabel(const BestiaryPreviewEntry& entry)
    {
        int level = calculateKnowledgeLevel(entry);

        if (level == 0) return "0 - inconnu / verrouillé";
        if (level == 1) return "1 - existence confirmée";
        if (level == 2) return "2 - informations utiles";
        return "3 - fiche complète actuelle";
    }

    std::string knowledgeProgressHint(const BestiaryPreviewEntry& entry)
    {
        const int level = calculateKnowledgeLevel(entry);

        if (level <= 0)
        {
            return "Trouver une preuve crédible, rencontrer l'entité ou acheter un renseignement fiable.";
        }

        if (level == 1)
        {
            return "Encore 1 victoire, 3 rencontres environ, ou une page de bibliothèque peuvent rendre les infos utiles.";
        }

        if (level == 2)
        {
            return "Plusieurs observations, environ 3 victoires ou un renseignement spécialisé peuvent compléter la fiche.";
        }

        return "Fiche suffisamment complète pour la préparation actuelle.";
    }

    std::string displayNameForEntry(const BestiaryPreviewEntry& entry)
    {
        if (calculateKnowledgeLevel(entry) == 0)
        {
            return "???";
        }

        return entry.name;
    }

    bool isObtentionEntryCategory(const std::string& category)
    {
        return category == "Matériaux et plantes"
            || category == "Objets rares"
            || category == "Effets et altérations";
    }

    std::string informationReliabilityLabel(const BestiaryPreviewEntry& entry)
    {
        const int level = calculateKnowledgeLevel(entry);
        if (level <= 0) return "Inconnue";
        if (level == 1) return "Rumeur : piste entendue, non vérifiée personnellement";
        if (level == 2 && entry.encounters <= 1 && entry.kills <= 0) return "Observé : obtenu ou constaté une fois";
        if (level == 2) return "Confirmé : plusieurs traces concordantes";
        return "Expert : information achetée, étudiée ou suffisamment vérifiée";
    }

    MenuScreen buildEntryDetailScreen(const BestiaryPreviewEntry& entry)
    {
        int knowledgeLevel = calculateKnowledgeLevel(entry);
        KnowledgeHints hints = buildKnowledgeHints(entry.category, entry.name, entry.detailedDescription);

        MenuScreen screen("FICHE BESTIAIRE", "bestiary.entry.detail");
        screen.addLine("Catégorie : " + entry.category);
        screen.addLine("Nom : " + displayNameForEntry(entry));
        screen.addLine("Statut : " + entry.status);
        screen.addLine("Niveau de connaissance : " + knowledgeLabel(entry));
        screen.addLine("Progression : " + knowledgeProgressHint(entry));
        screen.addLine("Danger estimé : " + std::string(knowledgeLevel <= 0 ? "???" : hints.dangerRank));
        screen.addLine("Rencontres : " + std::to_string(entry.encounters));
        screen.addLine("Tués par le joueur : " + std::to_string(entry.kills));

        if (knowledgeLevel <= 0)
        {
            screen.addLine("Information verrouillée.");
            screen.addLine("Le nom, les détails et les indices importants restent cachés tant que ton personnage n'a pas découvert une preuve crédible.");
        }
        else
        {
            screen.addLine("Résumé : " + std::string(knowledgeLevel == 1 ? entry.simpleDescription : entry.detailedDescription));
            screen.addLine("Fiabilité : " + informationReliabilityLabel(entry));
            screen.addLine("Habitat / origine : " + hints.habitat);

            if (isObtentionEntryCategory(entry.category))
            {
                if (knowledgeLevel == 1)
                {
                    screen.addLine("Piste d'obtention : " + hints.habitat);
                    screen.addLine("Meilleure source actuelle : rumeur seulement, aucune chance chiffrée fiable.");
                }
                else if (knowledgeLevel == 2)
                {
                    screen.addLine("Sources les plus probables : " + hints.habitat);
                    screen.addLine("Souvent lié à : " + hints.drops);
                }
                else
                {
                    screen.addLine("Source privilégiée connue : " + hints.habitat);
                    screen.addLine("Méthode / provenance la plus crédible : " + hints.drops);
                    screen.addLine("Les pourcentages exacts restent cachés tant que le moteur ne fournit pas de statistiques structurées suffisantes.");
                }
            }

            if (knowledgeLevel >= 2)
            {
                screen.addLine("Faiblesses probables : " + hints.weaknesses);
                screen.addLine("Résistances probables : " + hints.resistances);
            }
            else
            {
                screen.addLine("Faiblesses : encore imprécises.");
                screen.addLine("Résistances : encore imprécises.");
            }

            if (knowledgeLevel >= 3)
            {
                screen.addLine("Butin / ressource liée : " + hints.drops);
                screen.addLine("Conseil de chasse : " + hints.strategy);
            }
            else
            {
                screen.addLine("Butin / ressource liée : à confirmer.");
                screen.addLine("Conseil de chasse : continuer l'observation pour fiabiliser la fiche.");
            }
        }

        screen.addOption(0, "Continuer", "", true, "bestiary.entry.continue");
        return screen;
    }

    void displayEntryDetail(const BestiaryPreviewEntry& entry)
    {
        TerminalInterface::renderMenuScreen(buildEntryDetailScreen(entry));
    }

    void displayEntrySelectionList(const std::string& title, const std::vector<BestiaryPreviewEntry>& entries)
    {
        if (entries.empty())
        {
            MessageScreen::show(
                "BESTIAIRE",
                "bestiary.entry.empty",
                {"Aucune entrée préparée pour cette sélection."}
            );
            return;
        }

        const std::size_t itemsPerPage = 12;
        std::size_t pageIndex = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(entries.size(), itemsPerPage);
            const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(entries.size(), pageIndex, itemsPerPage);

            MenuScreen screen(title, "bestiary.entry.list");
            screen.addLine(PagedMenu::pageInfoText(pageIndex, totalPages, entries.size()));
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, entries.size()));

            for (std::size_t i = first; i < last; ++i)
            {
                const int localChoice = static_cast<int>(i - first + 1);
                const BestiaryPreviewEntry& entry = entries[i];
                KnowledgeHints hints = buildKnowledgeHints(entry.category, entry.name, entry.detailedDescription);

                std::string label = displayNameForEntry(entry)
                    + " | " + entry.category
                    + " | Connaissance : " + knowledgeLabel(entry)
                    + " | Danger : " + std::string(calculateKnowledgeLevel(entry) <= 0 ? "???" : hints.dangerRank)
                    + " | Tués : " + std::to_string(entry.kills);

                MenuOptionItemData itemData;
                itemData.structured = true;
                itemData.kind = "bestiary";
                itemData.section = entry.category;
                itemData.actionType = "inspect";
                itemData.name = displayNameForEntry(entry);
                itemData.detail = calculateKnowledgeLevel(entry) <= 0 ? "Information verrouillée" : entry.simpleDescription;
                itemData.status = knowledgeLabel(entry);
                itemData.progress = "Tués : " + std::to_string(entry.kills);
                itemData.owner = calculateKnowledgeLevel(entry) <= 0 ? "???" : hints.habitat;
                itemData.important = calculateKnowledgeLevel(entry) >= 2 || entry.kills > 0;

                screen.addOption(localChoice, label, "Inspecter cette fiche du bestiaire.", true, "bestiary.entry.select." + std::to_string(i), itemData);
            }

            PagedMenu::addNavigationOptions(
                screen,
                pageIndex,
                totalPages,
                "bestiary.entry.back",
                "bestiary.entry.previous",
                "bestiary.entry.next",
                "Revenir aux catégories du bestiaire.",
                "Afficher les fiches précédentes.",
                "Afficher les fiches suivantes."
            );
            int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Choix invalide."
            );

            Console::clear();

            if (choice == 0)
            {
                return;
            }

            if (choice == 98 && pageIndex > 0)
            {
                pageIndex--;
                continue;
            }

            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                pageIndex++;
                continue;
            }

            const int visibleCount = static_cast<int>(last - first);
            if (choice < 1 || choice > visibleCount)
            {
                MessageScreen::show(
                    "ENTRÉE NON AFFICHÉE",
                    "bestiary.entry.not_visible",
                    {
                        "Cette entrée n'est pas visible sur la page actuelle.",
                        "Utilise les options de page ou choisis une fiche affichée."
                    }
                );
                continue;
            }

            const std::size_t selectedIndex = first + static_cast<std::size_t>(choice - 1);
            displayEntryDetail(entries[selectedIndex]);
            Console::waitForEnter();
            Console::clear();
        }
    }

    // EN: displayEntryList declares or implements a focused behavior used by this module.
    // FR: displayEntryList déclare ou implémente un comportement précis utilisé par ce module.
    void displayEntryList(const std::string& category)
    {
        displayEntrySelectionList(category, filterEntries(category));
    }

    std::vector<SpecialDiscoveryEntry> getSpecialDiscoveryEntries()
    {
        return {
            {"Les bras cassés", "Les bras cassés", "Groupe de héros déjà connu", "Groupe connu", "Déjà présent dans les rumeurs publiques et les contes de taverne.", true},
            {"Hazak", "Hazak", "Assassin elfe noir", "Bras cassés", "Connu de base avec le groupe, puis complété par rencontre, victoire ou rumeur fiable.", true},
            {"Fail", "Fail", "Mage fou de race fée", "Bras cassés", "Connu de base avec le groupe, puis complété par rencontre, victoire ou rumeur fiable.", true},
            {"Aoi", "Aoi", "Kitsune de flammes", "Bras cassés", "Connu de base avec le groupe, puis complété par rencontre, victoire ou rumeur fiable.", true},
            {"Kanadé", "Kanadé", "Semi-dragonne zodiacale", "Bras cassés", "Connu de base avec le groupe, puis complété par rencontre, victoire ou rumeur fiable.", true},
            {"Sanctus", "Sanctus", "Protecteur lumineux", "Bras cassés", "Connu de base avec le groupe, puis complété par rencontre, victoire ou rumeur fiable.", true},
            {"Matt (PRO)", "Matt (PRO)", "Champion d'arène non confirmé", "Arène spéciale", "Rencontre d'arène, défi spécial ou rumeur fiable avant affichage complet du nom.", false},
            {"Skuro", "Skuro", "Silhouette lourde non confirmée", "Arène spéciale", "Rencontre, transformation crédible ou information de registre avant dévoilement.", false},
            {"Hestia", "Hestia", "Mage protégée non confirmée", "Liens spéciaux", "Rencontre, rumeur de groupe ou trace liée à une protection magique.", false},
            {"Henrique", "Henrique", "Nom non confirmé", "Liens spéciaux", "Rencontre ou information crédible avant de révéler ses liens.", false},
            {"Louis", "Louis", "Artificier non confirmé", "Trio d'arène", "Rencontre, défi ou rumeur fiable avant de révéler son rôle.", false},
            {"Trexof", "Trexof", "Analyste non confirmé", "Trio d'arène", "Rencontre, défi ou rumeur fiable avant de révéler son style.", false},
            {"Mattzelda", "Mattzelda", "Colosse non confirmé", "Trio d'arène", "Rencontre, défi ou rumeur fiable avant de révéler son profil.", false}
        };
    }

    bool tryFindBestiaryEntryByName(const std::string& name, BestiaryPreviewEntry& output)
    {
        for (const BestiaryPreviewEntry& entry : filterEntries("Personnages spéciaux"))
        {
            if (entry.name == name)
            {
                output = entry;
                return true;
            }
        }

        return false;
    }

    bool isSpecialDiscoveryKnown(const SpecialDiscoveryEntry& discovery)
    {
        if (discovery.knownFromStart)
        {
            return true;
        }

        BestiaryPreviewEntry entry;
        return tryFindBestiaryEntryByName(discovery.name, entry) && calculateKnowledgeLevel(entry) > 0;
    }

    std::string specialDiscoveryProgressText(const SpecialDiscoveryEntry& discovery)
    {
        BestiaryPreviewEntry entry;

        if (!tryFindBestiaryEntryByName(discovery.name, entry))
        {
            return discovery.knownFromStart ? "Connu de base" : "0 - inconnu / verrouillé";
        }

        return knowledgeLabel(entry)
            + " | Rencontres : " + std::to_string(entry.encounters)
            + " | Victoires : " + std::to_string(entry.kills);
    }

    MenuOptionItemData makeSpecialDiscoveryItemData(
        const SpecialDiscoveryEntry& discovery,
        bool known,
        const std::string& progress
    )
    {
        MenuOptionItemData itemData;
        itemData.structured = true;
        itemData.kind = "special_character_tracker";
        itemData.section = "Personnages spéciaux";
        itemData.actionType = known ? "inspect" : "locked";
        itemData.name = known ? discovery.knownLabel : "???";
        itemData.detail = known ? discovery.hiddenLabel : "Identité gardée par le registre.";
        itemData.status = known ? "Découvert" : "Verrouillé";
        itemData.progress = progress;
        itemData.owner = discovery.family;
        itemData.reward = discovery.unlockHint;
        itemData.important = !known || discovery.knownFromStart;
        return itemData;
    }

    void displaySpecialDiscoveryTracker()
    {
        const std::vector<SpecialDiscoveryEntry> entries = getSpecialDiscoveryEntries();
        constexpr std::size_t itemsPerPage = 7;
        std::size_t pageIndex = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(entries.size(), itemsPerPage);
            const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(entries.size(), pageIndex, itemsPerPage);

            int knownCount = 0;
            for (const SpecialDiscoveryEntry& discovery : entries)
            {
                if (isSpecialDiscoveryKnown(discovery))
                {
                    ++knownCount;
                }
            }

            MenuScreen screen("SUIVI DES PERSONNAGES SPÉCIAUX", "bestiary.special.discovery_tracker");
            screen.addLine("Ce suivi évite de révéler tous les personnages spéciaux gratuitement.");
            screen.addLine("Un nom verrouillé devient lisible après rencontre, défi d'arène, victoire ou renseignement crédible.");
            screen.addLine("Découverts : " + std::to_string(knownCount) + " / " + std::to_string(entries.size()) + " | Verrouillés : " + std::to_string(static_cast<int>(entries.size()) - knownCount));
            screen.addLine(PagedMenu::pageInfoText(pageIndex, totalPages, entries.size()));
            screen.addLine("Affichage : " + PagedMenu::rangeText(first, last, entries.size()));

            for (std::size_t index = first; index < last; ++index)
            {
                const SpecialDiscoveryEntry& discovery = entries[index];
                const bool known = isSpecialDiscoveryKnown(discovery);
                const std::string progress = specialDiscoveryProgressText(discovery);
                const std::string visibleName = known ? discovery.knownLabel : "???";
                const std::string label = visibleName
                    + " | " + discovery.family
                    + " | " + progress;

                screen.addOption(
                    static_cast<int>(index - first + 1),
                    label,
                    known ? "Ouvrir la fiche si elle existe dans le bestiaire." : discovery.unlockHint,
                    true,
                    "bestiary.special.discovery." + std::to_string(index),
                    makeSpecialDiscoveryItemData(discovery, known, progress)
                );
            }

            PagedMenu::addNavigationOptions(
                screen,
                pageIndex,
                totalPages,
                "bestiary.special.discovery.back",
                "bestiary.special.discovery.previous",
                "bestiary.special.discovery.next",
                "Revenir au menu des personnages spéciaux.",
                "Afficher les personnages précédents.",
                "Afficher les personnages suivants."
            );

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();

            if (choice == 0)
            {
                return;
            }

            if (choice == 98 && pageIndex > 0)
            {
                --pageIndex;
                continue;
            }

            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                ++pageIndex;
                continue;
            }

            const std::size_t selectedIndex = first + static_cast<std::size_t>(choice - 1);
            if (choice < 1 || selectedIndex >= last || selectedIndex >= entries.size())
            {
                MessageScreen::show(
                    "PERSONNAGE NON AFFICHÉ",
                    "bestiary.special.discovery.not_visible",
                    {"Ce personnage n'est pas visible sur la page actuelle."}
                );
                Console::clear();
                continue;
            }

            const SpecialDiscoveryEntry& selected = entries[selectedIndex];
            if (!isSpecialDiscoveryKnown(selected))
            {
                MessageScreen::show(
                    "IDENTITÉ VERROUILLÉE",
                    "bestiary.special.discovery.locked",
                    {
                        "Le registre sait qu'une trace existe, mais il refuse de donner un nom sans preuve.",
                        selected.unlockHint
                    }
                );
                Console::clear();
                continue;
            }

            BestiaryPreviewEntry entry;
            if (tryFindBestiaryEntryByName(selected.name, entry))
            {
                displayEntryDetail(entry);
                Console::waitForEnter();
                Console::clear();
            }
            else
            {
                MessageScreen::show(
                    "TRACE CONNUE",
                    "bestiary.special.discovery.known_without_sheet",
                    {
                        selected.knownLabel + " est connu par réputation, mais sa fiche détaillée n'est pas encore assez complète.",
                        selected.unlockHint
                    }
                );
                Console::clear();
            }
        }
    }

    void displaySpecialCharactersHub()
    {
        while (true)
        {
            MenuScreen screen("PERSONNAGES SPÉCIAUX", "bestiary.special.hub");
            screen.addLine("Cette section garde les personnages spéciaux à part des monstres classiques.");
            screen.addLine("Les Bras cassés sont déjà connus par réputation ; les autres profils doivent être découverts progressivement.");

            screen.addOption(0, "Retour", "Revenir au bestiaire.", true, "bestiary.special.back");
            screen.addOption(1, "Fiches découvertes", "Ouvrir les fiches classiques de personnages spéciaux.", true, "bestiary.special.entries");
            screen.addOption(2, "Suivi de découverte", "Voir qui est connu, verrouillé ou encore à confirmer.", true, "bestiary.special.discovery_tracker");
            screen.addOption(3, "Rumeurs et groupes", "Ouvrir les légendes liées aux groupes et aux héros découverts.", true, "bestiary.special.legends");

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");
            Console::clear();

            if (choice == 0)
            {
                return;
            }

            if (choice == 1)
            {
                displayEntryList("Personnages spéciaux");
            }
            else if (choice == 2)
            {
                displaySpecialDiscoveryTracker();
            }
            else if (choice == 3)
            {
                displayLegendArchiveList("GROUPES ET HÉROS DÉCOUVERTS", "Groupes et héros", true);
            }

            Console::clear();
        }
    }

    void displayKnowledgeLevelBrowser()
    {
        while (true)
        {
            MenuScreen screen("REGISTRE PAR CONNAISSANCE", "bestiary.knowledge_levels");
            screen.addOption(0, "Retour", "", true, "bestiary.knowledge.back");
            screen.addOption(1, "Entrées inconnues / verrouillées", "", true, "bestiary.knowledge.level0");
            screen.addOption(2, "Existence confirmée", "", true, "bestiary.knowledge.level1");
            screen.addOption(3, "Informations utiles", "", true, "bestiary.knowledge.level2");
            screen.addOption(4, "Fiches complètes actuelles", "", true, "bestiary.knowledge.level3");
            int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Choix invalide."
            );
            Console::clear();

            if (choice == 0)
            {
                return;
            }

            const int wantedLevel = choice - 1;
            std::vector<BestiaryPreviewEntry> filtered;

            for (const BestiaryPreviewEntry& entry : filterEntries("Tout"))
            {
                if (calculateKnowledgeLevel(entry) == wantedLevel)
                {
                    filtered.push_back(entry);
                }
            }

            std::string title;
            if (wantedLevel == 0) title = "Entrées inconnues / verrouillées";
            else if (wantedLevel == 1) title = "Existence confirmée";
            else if (wantedLevel == 2) title = "Informations utiles";
            else title = "Fiches complètes actuelles";

            displayEntrySelectionList(title, filtered);
            Console::clear();
        }
    }

    void showBestiaryInfoScreen(const std::string& title, const std::string& screenId, const std::vector<std::string>& lines)
    {
        MessageScreen::show(title, screenId, lines);
    }

    void displayHuntingNotebook()
    {
        std::vector<BestiaryPreviewEntry> entries = filterEntries("Tout");
        std::sort(entries.begin(), entries.end(), [](const BestiaryPreviewEntry& a, const BestiaryPreviewEntry& b) {
            const int scoreA = calculateKnowledgeLevel(a) * 100 + a.encounters + a.kills * 3;
            const int scoreB = calculateKnowledgeLevel(b) * 100 + b.encounters + b.kills * 3;

            if (scoreA == scoreB)
            {
                return a.name < b.name;
            }

            return scoreA < scoreB;
        });

        std::vector<std::string> lines;
        lines.push_back("Le carnet sert à choisir quoi observer ensuite au lieu d'ouvrir toute la liste.");
        lines.push_back("");
        lines.push_back("Fiches à compléter en priorité :");

        int shown = 0;
        for (const BestiaryPreviewEntry& entry : entries)
        {
            const int level = calculateKnowledgeLevel(entry);

            if (level >= 3)
            {
                continue;
            }

            lines.push_back("- " + displayNameForEntry(entry)
                + " | " + entry.category
                + " | " + knowledgeLabel(entry)
                + " | " + knowledgeProgressHint(entry));

            shown++;

            if (shown >= 8)
            {
                break;
            }
        }

        if (shown == 0)
        {
            lines.push_back("- Aucune priorité évidente : le registre est déjà très solide.");
        }

        lines.push_back("");
        lines.push_back("Pistes de terrain :");
        lines.push_back("- 1 rencontre confirme souvent l'existence.");
        lines.push_back("- 3 rencontres ou 1 victoire rendent la fiche plus utile.");
        lines.push_back("- Plusieurs victoires, notes de bibliothèque ou observations rares complètent la préparation.");
        lines.push_back("- Les boss gardent leur nom masqué sans révélation crédible.");

        showBestiaryInfoScreen("CARNET DE TRAQUE", "bestiary.hunting_notebook.detail", lines);
    }


    // EN: displayMaterialJournal declares or implements a focused behavior used by this module.
    // FR: displayMaterialJournal déclare ou implémente un comportement précis utilisé par ce module.
    void displayMaterialJournal()
    {
        std::vector<std::string> lines;
        lines.push_back("Ce journal complète le bestiaire : il explique comment un composant peut être trouvé, abîmé, amélioré ou utilisé.");
        lines.push_back("");
        lines.push_back("Qualités principales :");
        lines.push_back("- composants de monstre : impur / normal / pur / exceptionnel");
        lines.push_back("- plantes et matériaux classiques : faible qualité / normal / haute qualité / exceptionnel");
        lines.push_back("");
        lines.push_back("Règles utiles :");
        lines.push_back("- deux qualités différentes ne stackent pas ensemble ;");
        lines.push_back("- une action brutale peut dégrader un composant ;");
        lines.push_back("- une récolte propre peut améliorer la qualité d'un cran ;");
        lines.push_back("- les matériaux exceptionnels ne sont pas achetables directement ;");
        lines.push_back("- si plus de 50% de la valeur d'un craft vient d'exceptionnel, une particularité faible peut apparaître.");
        lines.push_back("");

        const std::vector<MaterialKnowledgeRecord> records = MaterialKnowledgeProgress::getRecords();
        lines.push_back("Matériaux réellement observés :");
        if (records.empty())
        {
            lines.push_back("- Aucun composant suivi pour le moment. Récupérer un matériau l'ajoutera au journal et au bestiaire.");
        }
        else
        {
            int shown = 0;
            for (const MaterialKnowledgeRecord& record : records)
            {
                std::string quality = record.quality.empty() ? "normal" : record.quality;
                std::string state = "utilisable";

                if (record.bestQualityWeight >= 4)
                {
                    state = "excellent pour craft à particularité";
                }
                else if (record.bestQualityWeight <= 1)
                {
                    state = "qualité faible/impure, rendement réduit";
                }
                else if (record.discoveredQuantity >= 5)
                {
                    state = "bien connu par l'expérience";
                }

                lines.push_back("- " + record.name
                    + " [" + quality + "] x" + std::to_string(record.discoveredQuantity)
                    + " | " + record.category
                    + " | " + state);
                ++shown;

                if (shown >= 10)
                {
                    lines.push_back("- ... autres composants suivis dans les fiches Matériaux et plantes.");
                    break;
                }
            }
        }

        lines.push_back("");
        lines.push_back("Exemples de lecture de terrain :");
        lines.push_back("- Oreille de gobelin : peut être trouée, brûlée ou propre selon le combat ;");
        lines.push_back("- Peau de bête robuste : utile aux kits et armures ;");
        lines.push_back("- Braise kitsune : composant magique sensible aux invocations ;");
        lines.push_back("- Noyau instable : puissant mais dangereux pour l'alchimie et les expériences.");

        showBestiaryInfoScreen(
            "JOURNAL DES MATÉRIAUX",
            "bestiary.material_journal.detail",
            lines
        );
    }

    // EN: displayCraftJournal declares or implements a focused behavior used by this module.
    // FR: displayCraftJournal déclare ou implémente un comportement précis utilisé par ce module.
    void displayCraftJournal()
    {
        std::vector<std::string> lines;
        lines.push_back("Ce journal suit les fabrications réussies pendant la session actuelle.");
        lines.push_back("Il nourrit les savoir-faire liés à l'artisanat, l'alchimie, la forge et aux armes utilisées.");
        lines.push_back("");
        lines.push_back("Total fabriqué cette session : " + std::to_string(MaterialExperimentLog::getTotalCrafted()));

        const std::vector<CraftExperimentRecord>& records = MaterialExperimentLog::getCraftRecords();

        if (records.empty())
        {
            lines.push_back("Aucun craft suivi pour le moment.");
        }
        else
        {
            for (const CraftExperimentRecord& record : records)
            {
                lines.push_back("- " + record.recipeName + " : " + std::to_string(record.craftedCount) + " fabrication(s)");
            }
        }

        lines.push_back("");
        lines.push_back("Rappel : les matériaux exceptionnels majoritaires peuvent créer une particularité faible.");
        lines.push_back("Forgeron, Alchimiste et Artificier ont maintenant un léger avantage économique et artisanal.");

        showBestiaryInfoScreen("JOURNAL DU CRAFT", "bestiary.craft_journal.detail", lines);
    }

    // EN: displaySummonJournal declares or implements a focused behavior used by this module.
    // FR: displaySummonJournal déclare ou implémente un comportement précis utilisé par ce module.
    void displaySummonJournal()
    {
        showBestiaryInfoScreen(
            "JOURNAL DES INVOCATIONS",
            "bestiary.summon_journal.detail",
            {
                "Les invocations utilisent maintenant une base plus claire : slots, maintien, durée, contrôle manuel et sacrifice.",
                "",
                "Règles observées :",
                "- slots d'invocation : certaines invocations lourdes occupent plus d'un slot ;",
                "- maintien : une invocation peut prolonger son lien au lieu d'attaquer ;",
                "- sacrifice : une invocation peut rompre son lien pour infliger une rupture ;",
                "- ciblage : les ennemis peuvent attaquer les invocations selon leur logique ;",
                "- conduite : instinctive ou guidée selon le lien avec l'invocateur.",
                "",
                "Notes incomplètes : certains rituels parlent encore de mana profond, de sacrifices avancés, d'ombres de Hazak et de formations de groupe."
            }
        );
    }

    // EN: displayInformationShopPreview declares or implements a focused behavior used by this module.
    // FR: displayInformationShopPreview déclare ou implémente un comportement précis utilisé par ce module.
    void displayInformationShopPreview()
    {
        showBestiaryInfoScreen(
            "ACHAT D'INFORMATIONS",
            "bestiary.info_shop.detail",
            {
                "La bibliothèque débloque des renseignements qui montent le niveau de connaissance du bestiaire.",
                "Les secrets, les boss et les noms importants devront toujours se mériter.",
                "La bibliothèque accueille aussi des légendes, contes pour enfant, notes de conteur et rumeurs anciennes.",
                "",
                "Un nom de boss ne sera pas donné gratuitement si personne ne l'a prononcé."
            }
        );
    }

    MenuOptionItemData makeLegendOptionData(
        const std::string& name,
        const std::string& detail,
        const std::string& section,
        const std::string& source,
        const std::string& progress = "",
        bool important = false
    )
    {
        MenuOptionItemData data;
        data.structured = true;
        data.kind = "legend";
        data.section = section.empty() ? "Registre des légendes" : section;
        data.actionType = "read";
        data.name = name;
        data.detail = detail;
        data.owner = source;
        data.progress = progress;
        data.important = important;
        return data;
    }

    MenuOptionItemData makeLegendOptionData(const LegendArchiveEntry& entry, bool discovered)
    {
        return makeLegendOptionData(
            entry.title,
            entry.shortDescription,
            entry.category,
            entry.source,
            discovered ? "Découverte" : "Archive / règle",
            entry.category == "Salles de boss" || entry.category == "Groupes et héros"
        );
    }

    bool isLegendEntryDiscovered(const LegendArchiveEntry& entry)
    {
        if (entry.category == "Règles du registre")
        {
            return true;
        }

        if (BestiaryRuntimeProgress::getEncounterCount(entry.title) > 0)
        {
            return true;
        }

        // FR: l'achat du rayonnage de bibliothèque donne accès aux contes légers,
        // sans révéler d'avance les légendes de salles de boss.
        const bool libraryLegendsUnlocked = BestiaryRuntimeProgress::getEncounterCount("Légendes de bibliothèque") > 0;
        if (libraryLegendsUnlocked && (entry.category == "Contes pour enfant" || entry.id == "bras_casses_origin"))
        {
            return true;
        }

        return false;
    }

    int countLegendEntries(const std::string& categoryFilter, bool discoveredOnly)
    {
        int count = 0;
        const std::vector<LegendArchiveEntry> allEntries = LegendTriggerSystem::getArchiveEntries();
        for (const LegendArchiveEntry& entry : allEntries)
        {
            if (categoryFilter != "Toutes" && entry.category != categoryFilter)
            {
                continue;
            }

            if (!discoveredOnly || isLegendEntryDiscovered(entry))
            {
                ++count;
            }
        }

        return count;
    }

    void displayLegendArchiveList(const std::string& title, const std::string& categoryFilter, bool discoveredOnly)
    {
        const std::vector<LegendArchiveEntry> allEntries = LegendTriggerSystem::getArchiveEntries();
        std::vector<LegendArchiveEntry> entries;

        for (const LegendArchiveEntry& entry : allEntries)
        {
            if (categoryFilter != "Toutes" && entry.category != categoryFilter)
            {
                continue;
            }

            if (discoveredOnly && !isLegendEntryDiscovered(entry))
            {
                continue;
            }

            entries.push_back(entry);
        }

        const std::size_t itemsPerPage = 8;
        std::size_t pageIndex = 0;

        while (true)
        {
            const std::size_t totalPages = PagedMenu::pageCount(entries.size(), itemsPerPage);
            if (pageIndex >= totalPages)
            {
                pageIndex = totalPages - 1;
            }

            const std::size_t first = PagedMenu::firstIndex(pageIndex, itemsPerPage);
            const std::size_t last = PagedMenu::lastIndexExclusive(entries.size(), pageIndex, itemsPerPage);

            MenuScreen screen(title, "bestiary.legends.archive.list");
            screen.addLine("Section spéciale du bestiaire : les légendes sont rangées à part des monstres, boss et matériaux.");
            screen.addLine("Catégorie : " + categoryFilter);
            screen.addLine(discoveredOnly
                ? "Filtre : seulement les légendes déjà découvertes, lues ou ouvertes par la bibliothèque."
                : "Filtre : archives complètes de développement, à éviter en jeu normal si tu veux garder les surprises.");

            if (entries.empty())
            {
                screen.addLine("Aucune légende découverte dans cette sous-section pour le moment.");
                screen.addLine("Indice : la bibliothèque, certains PNJ ou quelques salles de boss pourront remplir ce registre petit à petit.");
            }
            else
            {
                screen.addLine(PagedMenu::pageInfoText(pageIndex, totalPages, entries.size()));
                screen.addLine("Récits affichés : " + PagedMenu::rangeText(first, last, entries.size()));
            }

            for (std::size_t i = first; i < last; ++i)
            {
                const int localChoice = static_cast<int>(i - first + 1);
                const LegendArchiveEntry& entry = entries[i];
                const bool discovered = isLegendEntryDiscovered(entry);
                screen.addOption(
                    localChoice,
                    entry.title,
                    entry.category + " · " + entry.source + " · " + entry.shortDescription,
                    true,
                    "bestiary.legends.archive.read." + std::to_string(i),
                    makeLegendOptionData(entry, discovered)
                );
            }

            PagedMenu::addNavigationOptions(
                screen,
                pageIndex,
                totalPages,
                "bestiary.legends.archive.back",
                "bestiary.legends.archive.previous",
                "bestiary.legends.archive.next",
                "Revenir aux sections de légendes.",
                "Afficher les récits précédents.",
                "Afficher les récits suivants."
            );

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");

            if (choice == 0)
            {
                return;
            }

            if (choice == 98 && pageIndex > 0)
            {
                pageIndex--;
                Console::clear();
                continue;
            }

            if (choice == 99 && pageIndex + 1 < totalPages)
            {
                pageIndex++;
                Console::clear();
                continue;
            }

            const int visibleCount = static_cast<int>(last - first);
            if (choice < 1 || choice > visibleCount)
            {
                MessageScreen::show(
                    "LÉGENDE NON AFFICHÉE",
                    "bestiary.legends.archive.not_visible",
                    {
                        "Cette légende n'est pas visible sur la page actuelle.",
                        "Utilise les options de page ou choisis un récit affiché."
                    }
                );
                Console::clear();
                continue;
            }

            const std::size_t selectedIndex = first + static_cast<std::size_t>(choice - 1);
            LegendTriggerSystem::displayArchiveEntry(entries[selectedIndex].id);
            Console::clear();
        }
    }

    void displayLegendsArchive()
    {
        while (true)
        {
            const int discoveredTotal = countLegendEntries("Toutes", true);
            const int archiveTotal = countLegendEntries("Toutes", false);

            MenuScreen screen("REGISTRE DES LÉGENDES", "bestiary.legends.hub");
            screen.addLine("Section particulière du bestiaire : ici, on range les récits, contes, rumeurs et traces rares.");
            screen.addLine("Les légendes ne sont pas mélangées avec les fiches de monstres : elles servent au lore, pas à forcer une stratégie.");
            screen.addLine("Certaines peuvent venir d'un PNJ, d'une salle de boss, d'une bibliothèque ou d'un déclencheur rare.");
            screen.addLine("Progression du registre : " + std::to_string(discoveredTotal) + " / " + std::to_string(archiveTotal) + " légende(s) visibles sans spoiler forcé.");

            screen.addOption(0, "Retour", "Revenir au bestiaire.", true, "bestiary.legends.back");
            screen.addOption(1, "Fiches bestiaire des légendes", "Voir les entrées Légendes / contes enregistrées comme fiches classiques.", true, "bestiary.legends.register", makeLegendOptionData("Fiches bestiaire des légendes", "Entrées légendes rangées comme fiches classiques, utiles si tu veux rester dans le bestiaire normal.", "Accès", "Bestiaire", "Fiches classiques", true));
            screen.addOption(2, "Légendes découvertes", "Lire seulement les légendes déjà rencontrées, achetées ou débloquées.", true, "bestiary.legends.discovered", makeLegendOptionData("Légendes découvertes", "Lecture sans spoiler forcé : uniquement les récits déjà ouverts par rencontre, achat ou déclencheur.", "Accès", "Registre", std::to_string(discoveredTotal) + " / " + std::to_string(archiveTotal), true));
            screen.addOption(3, "Groupes et héros découverts", "Rumeurs de groupes connus, héros chaotiques et réputations.", true, "bestiary.legends.groups", makeLegendOptionData("Groupes et héros découverts", "Rumeurs sur les groupes connus, les héros chaotiques et les réputations qui circulent.", "Groupes et héros", "Rumeurs de ville", std::to_string(countLegendEntries("Groupes et héros", true)) + " visible(s)", true));
            screen.addOption(4, "Contes pour enfant découverts", "Versions adoucies, drôles ou populaires des récits.", true, "bestiary.legends.children", makeLegendOptionData("Contes pour enfant découverts", "Versions adoucies, drôles ou populaires des récits, souvent obtenues via bibliothèque.", "Contes pour enfant", "Bibliothèque", std::to_string(countLegendEntries("Contes pour enfant", true)) + " visible(s)"));
            screen.addOption(5, "Salles de boss découvertes", "Traces rares réellement entendues ou lues avant certaines arènes.", true, "bestiary.legends.boss_rooms", makeLegendOptionData("Salles de boss découvertes", "Traces rares réellement entendues ou lues avant certaines arènes, sans déclenchement systématique.", "Salles de boss", "Arènes / boss", std::to_string(countLegendEntries("Salles de boss", true)) + " visible(s)", true));
            screen.addOption(6, "Déclencheurs et rumeurs", "Notes de PNJ, bibliothèque ou lieux qui expliquent quand une légende peut apparaître.", true, "bestiary.legends.triggers", makeLegendOptionData("Déclencheurs et rumeurs", "Notes de PNJ, bibliothèque ou lieux qui expliquent quand une légende peut apparaître.", "Déclencheurs et rumeurs", "PNJ / bibliothèque", std::to_string(countLegendEntries("Déclencheurs et rumeurs", true)) + " visible(s)"));
            screen.addOption(7, "Règles du registre", "Notes qui expliquent comment les légendes apparaissent sans devenir lourdes.", true, "bestiary.legends.rules", makeLegendOptionData("Règles du registre", "Notes de fonctionnement : les récits restent optionnels, rares et rangés à part du bestiaire tactique.", "Règles du registre", "Système", std::to_string(countLegendEntries("Règles du registre", false)) + " note(s)", true));

            int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choix invalide.");

            if (choice == 0)
            {
                return;
            }

            if (choice == 1)
            {
                displayEntryList("Légendes / contes");
            }
            else if (choice == 2)
            {
                displayLegendArchiveList("LÉGENDES DÉCOUVERTES", "Toutes", true);
            }
            else if (choice == 3)
            {
                displayLegendArchiveList("GROUPES ET HÉROS DÉCOUVERTS", "Groupes et héros", true);
            }
            else if (choice == 4)
            {
                displayLegendArchiveList("CONTES POUR ENFANT DÉCOUVERTS", "Contes pour enfant", true);
            }
            else if (choice == 5)
            {
                displayLegendArchiveList("SALLES DE BOSS DÉCOUVERTES", "Salles de boss", true);
            }
            else if (choice == 6)
            {
                displayLegendArchiveList("DÉCLENCHEURS ET RUMEURS", "Déclencheurs et rumeurs", true);
            }
            else if (choice == 7)
            {
                displayLegendArchiveList("RÈGLES DU REGISTRE", "Règles du registre", false);
            }
        }
    }


    void addCategoryCount(
        std::vector<std::pair<std::string, int>>& counts,
        const std::string& category
    )
    {
        for (std::pair<std::string, int>& count : counts)
        {
            if (count.first == category)
            {
                count.second++;
                return;
            }
        }

        counts.push_back({category, 1});
    }

    void displayKnowledgeSummary()
    {
        std::vector<BestiaryPreviewEntry> entries = filterEntries("Tout");
        int level0 = 0;
        int level1 = 0;
        int level2 = 0;
        int level3 = 0;
        int totalKills = 0;
        int totalEncounters = 0;
        std::vector<std::pair<std::string, int>> categoryCounts;

        for (const BestiaryPreviewEntry& entry : entries)
        {
            const int level = calculateKnowledgeLevel(entry);

            if (level <= 0) level0++;
            else if (level == 1) level1++;
            else if (level == 2) level2++;
            else level3++;

            totalKills += entry.kills;
            totalEncounters += entry.encounters;
            addCategoryCount(categoryCounts, entry.category);
        }

        std::vector<std::string> lines;
        lines.push_back("Entrées suivies : " + std::to_string(entries.size()));
        lines.push_back("Rencontres enregistrées : " + std::to_string(totalEncounters));
        lines.push_back("Tués enregistrés : " + std::to_string(totalKills));
        lines.push_back("");
        lines.push_back("Connaissance 0 / verrouillée : " + std::to_string(level0));
        lines.push_back("Connaissance 1 / existence : " + std::to_string(level1));
        lines.push_back("Connaissance 2 / utile : " + std::to_string(level2));
        lines.push_back("Connaissance 3 / complète actuelle : " + std::to_string(level3));
        lines.push_back("");
        lines.push_back("Répartition par catégorie :");

        for (const std::pair<std::string, int>& count : categoryCounts)
        {
            lines.push_back("- " + count.first + " : " + std::to_string(count.second));
        }

        std::vector<BestiaryRuntimeRecord> runtimeRecords = BestiaryRuntimeProgress::getRecords();
        std::sort(runtimeRecords.begin(), runtimeRecords.end(), [](const BestiaryRuntimeRecord& a, const BestiaryRuntimeRecord& b) {
            const int scoreA = a.encounters + a.kills * 3;
            const int scoreB = b.encounters + b.kills * 3;

            if (scoreA == scoreB)
            {
                return a.name < b.name;
            }

            return scoreA > scoreB;
        });

        lines.push_back("");
        lines.push_back("Découvertes réelles les plus observées :");

        if (runtimeRecords.empty())
        {
            lines.push_back("- Aucune trace réelle enregistrée dans cette session ou sauvegarde.");
        }
        else
        {
            const int limit = std::min(5, static_cast<int>(runtimeRecords.size()));

            for (int i = 0; i < limit; ++i)
            {
                const BestiaryRuntimeRecord& record = runtimeRecords[i];
                lines.push_back("- " + record.name
                    + " | " + record.category
                    + " | rencontres " + std::to_string(record.encounters)
                    + " | tués " + std::to_string(record.kills)
                    + " | " + record.status);
            }
        }

        showBestiaryInfoScreen("SYNTHÈSE DU BESTIAIRE", "bestiary.summary.detail", lines);
    }

    void displayTacticalIndex()
    {
        showBestiaryInfoScreen(
            "INDEX TACTIQUE",
            "bestiary.tactical_index.detail",
            {
                "Lecture rapide des grandes familles. Les fiches individuelles restent plus précises quand elles sont observées.",
                "",
                "Gelées / slimes",
                "- Saignement faible, poison souvent mauvais, couleur à identifier avant de lancer un sort rare.",
                "- Rouge : froid utile. Bleu/blanc : chaleur utile. Jaune : attention au métal. Violet/noir : antidotes utiles.",
                "",
                "Humanoïdes / gobelins / bandits",
                "- Peu de résistances naturelles, mais l'équipement et les rôles changent tout.",
                "- Les soutiens crédibles se focus : shaman, apothicaire, oracle, chef.",
                "",
                "Bêtes",
                "- Dangereuses sur cible isolée ou blessée. Entrave et contre après charge sont utiles.",
                "- Les matériaux se dégradent si le combat est trop brutal.",
                "",
                "Morts-vivants / os / ombres",
                "- Poison et saignement souvent faibles. Lumière, feu, rupture d'os ou purification sont plus fiables.",
                "- Les supports spectraux et oracles doivent être traités comme des soutiens, pas comme de simples sacs à PV.",
                "",
                "Constructions / armures mortes",
                "- Les entailles faibles déçoivent. Marteau, choc ciblé, faille, corrosion ou brise-garde gagnent en valeur.",
                "- Elles donnent souvent de bons composants si le combat ne les broie pas n'importe comment.",
                "",
                "Draconides / dragons",
                "- Même jeunes, ils demandent préparation : souffle, écailles et matériaux fragiles après combat.",
                "- Une victoire trop brutale peut donner moins de ressources propres.",
                "",
                "Plantes",
                "- Feu et coupe nette. Le poison est souvent décevant. L'entrave doit être cassée vite.",
                "",
                "Élémentaires / effets",
                "- Un élément résiste souvent à lui-même. Le métal amplifie le choc. Le voile élémentaire réduit plusieurs altérations.",
                "- Les anomalies peuvent punir les automatismes : observer le premier tour vaut parfois une potion économisée.",
                "",
                "Zones",
                "- Plaine = bases, Route = humanoïdes, Mares = slimes, Forêt = entraves, Montagne = froid/roche, Marais = poison, Cimetière = morts, Ruines = anomalies/constructions."
            }
        );
    }

    std::vector<BestiaryPreviewEntry> collectEntriesForCategories(const std::vector<std::string>& categories)
    {
        std::vector<BestiaryPreviewEntry> combined;

        for (const std::string& category : categories)
        {
            for (const BestiaryPreviewEntry& entry : filterEntries(category))
            {
                bool alreadyListed = false;

                for (const BestiaryPreviewEntry& existing : combined)
                {
                    if (existing.category == entry.category && existing.name == entry.name)
                    {
                        alreadyListed = true;
                        break;
                    }
                }

                if (!alreadyListed)
                {
                    combined.push_back(entry);
                }
            }
        }

        return combined;
    }

    void displayCreatureRegister()
    {
        displayEntrySelectionList(
            "Créatures et personnages",
            collectEntriesForCategories({
                "Entités hostiles / ennemis",
                "Entités passives / alliées",
                "Invocations",
                "Boss",
                "Personnages spéciaux",
                "Secret"
            })
        );
    }

    void displayEncyclopediaRegister()
    {
        displayEntrySelectionList(
            "Encyclopédie",
            collectEntriesForCategories({
                "Races",
                "Matériaux et plantes",
                "Objets rares",
                "Effets et altérations",
                "Habitats / zones",
                "Classes jouables",
                "Quêtes / guilde"
            })
        );
    }

    void displayDiscoveryNotebookRegister()
    {
        displayEntrySelectionList(
            "Carnet de découvertes",
            collectEntriesForCategories({
                "Divinités / lore",
                "Légendes / contes",
                "Secret"
            })
        );
    }

}

// EN: open declares or implements a focused behavior used by this module.
// FR: open déclare ou implémente un comportement précis utilisé par ce module.
void BestiaryMenu::open()
{
    openBestiary();
}

void BestiaryMenu::openBestiary()
{
    while (true)
    {
        MenuScreen screen("BESTIAIRE", "bestiary.hub");
        screen.addLine("Objet spécial de base : Bestiaire");
        screen.addLine("Ce registre est réservé aux créatures, ennemis, alliés, invocations, boss et personnages rencontrés.");
        screen.addLine("Les matériaux, races et objets sont rangés dans l'Encyclopédie ; le lore et les légendes dans le Carnet de découvertes.");
        screen.addOption(0, "Retour", "", true, "bestiary.back");
        screen.addOption(1, "Toutes les créatures connues", "", true, "bestiary.all_creatures");
        screen.addOption(2, "Entités hostiles / ennemis", "", true, "bestiary.hostiles");
        screen.addOption(3, "Entités passives / alliées", "", true, "bestiary.passives");
        screen.addOption(4, "Invocations", "", true, "bestiary.summons");
        screen.addOption(5, "Boss", "", true, "bestiary.bosses");
        screen.addOption(6, "Personnages spéciaux", "", true, "bestiary.special_characters");
        screen.addOption(7, "Journal des invocations", "", true, "bestiary.summon_journal");
        screen.addOption(8, "Index tactique", "", true, "bestiary.tactical_index");
        screen.addOption(9, "Synthèse du bestiaire", "", true, "bestiary.summary");
        screen.addOption(10, "Registre par niveau de connaissance", "", true, "bestiary.knowledge_levels");
        screen.addOption(11, "Carnet de traque", "", true, "bestiary.hunting_notebook");

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix invalide. Choisis une option affichée."
        );
        Console::clear();

        if (choice == 0) return;
        if (choice == 1) displayCreatureRegister();
        else if (choice == 2) displayEntryList("Entités hostiles / ennemis");
        else if (choice == 3) displayEntryList("Entités passives / alliées");
        else if (choice == 4) displayEntryList("Invocations");
        else if (choice == 5) displayEntryList("Boss");
        else if (choice == 6) displaySpecialCharactersHub();
        else if (choice == 7) displaySummonJournal();
        else if (choice == 8) displayTacticalIndex();
        else if (choice == 9) displayKnowledgeSummary();
        else if (choice == 10) displayKnowledgeLevelBrowser();
        else if (choice == 11) displayHuntingNotebook();
    }
}

void BestiaryMenu::openEncyclopedia()
{
    while (true)
    {
        MenuScreen screen("ENCYCLOPÉDIE", "encyclopedia.hub");
        screen.addLine("Objet spécial de base : Encyclopédie");
        screen.addLine("Impossible à perdre : elle classe les connaissances utiles au jeu, hors créatures et hors lore pur.");
        screen.addOption(0, "Retour", "", true, "encyclopedia.back");
        screen.addOption(1, "Toutes les entrées d'encyclopédie", "", true, "encyclopedia.all");
        screen.addOption(2, "Races", "", true, "encyclopedia.races");
        screen.addOption(3, "Matériaux et plantes", "", true, "encyclopedia.materials");
        screen.addOption(4, "Objets rares", "", true, "encyclopedia.rare_items");
        screen.addOption(5, "Effets et altérations", "", true, "encyclopedia.effects");
        screen.addOption(6, "Habitats / zones", "", true, "encyclopedia.habitats");
        screen.addOption(7, "Journal des matériaux", "", true, "encyclopedia.material_journal");
        screen.addOption(8, "Acheter des informations communes", "", true, "encyclopedia.info_shop");
        screen.addOption(9, "Journal du craft", "", true, "encyclopedia.craft_journal");
        screen.addOption(10, "Classes jouables", "", true, "encyclopedia.playable_classes");

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix invalide. Choisis une option affichée."
        );
        Console::clear();

        if (choice == 0) return;
        if (choice == 1) displayEncyclopediaRegister();
        else if (choice == 2) displayEntryList("Races");
        else if (choice == 3) displayEntryList("Matériaux et plantes");
        else if (choice == 4) displayEntryList("Objets rares");
        else if (choice == 5) displayEntryList("Effets et altérations");
        else if (choice == 6) displayEntryList("Habitats / zones");
        else if (choice == 7) displayMaterialJournal();
        else if (choice == 8) displayInformationShopPreview();
        else if (choice == 9) displayCraftJournal();
        else if (choice == 10) displayEntryList("Classes jouables");
    }
}

void BestiaryMenu::openDiscoveryNotebook()
{
    while (true)
    {
        MenuScreen screen("CARNET DE DÉCOUVERTES", "discovery_notebook.hub");
        screen.addLine("Objet spécial de base : Carnet de découvertes");
        screen.addLine("Impossible à perdre : il garde le lore, les légendes, les rumeurs vérifiées et les traces de découverte non mécaniques.");
        screen.addLine("Il évite que l'Encyclopédie devienne un fourre-tout tout en gardant les histoires accessibles.");
        screen.addOption(0, "Retour", "", true, "discovery_notebook.back");
        screen.addOption(1, "Toutes les découvertes narratives", "", true, "discovery_notebook.all");
        screen.addOption(2, "Registre des légendes", "Contes, rumeurs rares, salles de boss et histoires pour enfant.", true, "discovery_notebook.legends");
        screen.addOption(3, "Divinités / lore", "", true, "discovery_notebook.lore");
        screen.addOption(4, "Découvertes étranges", "Entrées secrètes ou difficiles à classer.", true, "discovery_notebook.secret");

        int choice = TerminalInterface::askMenuChoiceFromOptions(
            screen,
            "Choix invalide. Choisis une option affichée."
        );
        Console::clear();

        if (choice == 0) return;
        if (choice == 1) displayDiscoveryNotebookRegister();
        else if (choice == 2) displayLegendsArchive();
        else if (choice == 3) displayEntryList("Divinités / lore");
        else if (choice == 4) displayEntryList("Secret");
    }
}

// EN: displayObjectEntry declares or implements a focused behavior used by this module.
// FR: displayObjectEntry déclare ou implémente un comportement précis utilisé par ce module.
void BestiaryMenu::displayObjectEntry(const std::string& entryName)
{
    for (BestiaryPreviewEntry entry : getEntries())
    {
        if (entry.name == entryName)
        {
            entry.encounters += BestiaryRuntimeProgress::getEncounterCount(entry.name);
            entry.kills += BestiaryRuntimeProgress::getKillCount(entry.name);
            entry.status = BestiaryRuntimeProgress::getStatusFor(entry.name, entry.status);
            displayEntryDetail(entry);
            return;
        }
    }

    for (const BestiaryRuntimeRecord& record : BestiaryRuntimeProgress::getRecords())
    {
        if (record.name == entryName)
        {
            displayEntryDetail({
                record.category,
                record.name,
                record.description,
                record.description,
                record.status,
                record.encounters,
                record.kills
            });
            return;
        }
    }

    MenuScreen screen("BESTIAIRE", "bestiary.object.unknown");
    screen.addLine("Recherche : " + entryName);
    screen.addLine("Entrée inconnue ou pas encore découverte.");
    screen.addLine("Quand ton personnage découvrira cette chose, le bestiaire pourra afficher son nom, sa description, ses rencontres et ses tués.");
    screen.addOption(0, "Continuer", "", true, "bestiary.object.unknown.continue");
    TerminalInterface::renderMenuScreen(screen);
}
