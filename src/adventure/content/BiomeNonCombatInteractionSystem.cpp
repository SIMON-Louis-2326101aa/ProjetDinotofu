#include "adventure/content/BiomeNonCombatInteractionSystem.hpp"

#include <cstdint>
#include <unordered_map>

namespace
{
std::uint32_t stableHash(const std::string& text)
{
    std::uint32_t value = 2166136261u;
    for (unsigned char c : text)
    {
        value ^= c;
        value *= 16777619u;
    }
    return value;
}

using Interaction = BiomeNonCombatInteraction;

const std::unordered_map<std::string, Interaction>& catalog()
{
    static const std::unordered_map<std::string, Interaction> values = {
        {"Route commerciale", {true, "borne_abimee", "Borne de route abîmée",
            "Une borne a été remise debout de travers. Deux voyageurs discutent pour savoir quelle direction conserver.",
            {"Les deux voyageurs ont des marques de relais différentes : personne ici ne possède une vérité automatique."},
            {
                {1, "Comparer les marques", "Prendre le temps de comparer les signes de relais et les ornières.", "Tu recoupes les marques sans effacer celles qui te contredisent.", -2, 1},
                {2, "Aider à remettre la borne", "Réparer la borne selon l'orientation la mieux étayée.", "La borne tient de nouveau ; tu laisses aussi l'ancienne marque visible pour qu'un autre puisse vérifier.", -3, 1},
                {3, "Ne pas trancher", "Noter le désaccord puis repartir.", "Tu refuses de fabriquer une certitude à partir de deux témoignages incomplets.", 0, 0}
            }}},
        {"Forêt ancienne", {true, "collet_de_chasseur", "Collet oublié",
            "Un petit animal vivant s'est pris dans un vieux collet. Le piège porte une marque de chasseur presque effacée.",
            {"Le bruit attire l'attention, mais le laisser là modifiera aussi la vie locale."},
            {
                {1, "Libérer l'animal", "Démonter le collet sans garder le métal.", "L'animal disparaît dans les fougères ; les traces alentour redeviennent plus naturelles à lire.", -2, 1},
                {2, "Démonter et baliser", "Retirer le piège puis laisser la marque visible pour le propriétaire.", "Tu enlèves un danger sans prétendre que le piège n'a jamais existé.", -1, 2},
                {3, "Contourner", "Ne pas intervenir.", "Tu conserves seulement l'emplacement du collet dans tes notes.", 1, 0}
            }}},
        {"Marais trouble", {true, "passerelle_fendue", "Passerelle fendue",
            "Une passerelle de bois utilisée par les habitants s'enfonce dans la vase à un endroit précis.",
            {"Des empreintes montrent qu'elle sert encore ; la réparer peut changer les déplacements locaux aujourd'hui."},
            {
                {1, "Caler les planches", "Utiliser branches et pierres pour stabiliser le passage.", "La passerelle reste médiocre, mais elle ne piège plus le premier pied pressé.", -3, 1},
                {2, "Marquer le danger", "Tracer un avertissement visible sans réparer.", "Le danger reste présent mais devient lisible pour les prochains voyageurs.", -1, 1},
                {3, "Passer ailleurs", "Éviter le problème.", "Tu choisis une langue de terre plus longue et laisses la passerelle telle quelle.", 1, 0}
            }}},
        {"Cimetière oublié", {true, "nom_nettoye", "Nom sous la mousse",
            "Une pierre tombale encore fleurie est presque entièrement couverte de mousse, sauf une lettre récemment nettoyée.",
            {"Quelqu'un revient donc ici. Le lieu n'est pas seulement un décor abandonné."},
            {
                {1, "Nettoyer sans déplacer", "Dégager doucement le nom et laisser les offrandes intactes.", "Le nom redevient lisible sans que tu touches aux objets laissés par les proches.", -2, 1},
                {2, "Observer seulement", "Comparer les fleurs, la terre et les passages récents.", "Tu notes qu'une personne entretient encore cette tombe, sans inventer son identité.", -1, 1},
                {3, "Quitter la tombe", "Respecter le lieu et repartir.", "Tu ne tires aucune conclusion de plus que ce que le terrain permet.", 0, 0}
            }}},
        {"Archives noyées", {true, "feuillet_a_secher", "Feuillet détrempé",
            "Une page de registre flotte contre une étagère. Quelques lignes restent visibles si elle sèche correctement.",
            {"La sauver donnera peut-être un indice, pas automatiquement une vérité complète."},
            {
                {1, "Sécher lentement", "Étaler la page à l'abri et préserver les encres encore lisibles.", "Tu sauves plusieurs lignes et les marques de correction restent visibles.", -3, 2},
                {2, "Copier ce qui reste", "Recopier seulement les fragments sûrs puis laisser la page.", "Tu obtiens une copie partielle mais honnête, avec des blancs clairement signalés.", -1, 1},
                {3, "Ne pas toucher", "Éviter d'abîmer davantage l'archive.", "Tu laisses la page en place et mémorises seulement son emplacement.", 0, 0}
            }}},
        {"Falaises des drakes gris", {true, "corde_de_nid", "Corde près d'un nid",
            "Une vieille corde de grimpe traverse un passage proche d'un nid de drake. Elle peut aider les voyageurs mais gêne aussi l'animal.",
            {"Les écailles au sol indiquent une présence récente, sans certifier que le drake est encore là."},
            {
                {1, "Déplacer la corde", "Créer un passage plus bas, loin du nid.", "Le trajet devient un peu plus long mais évite de couper directement la zone de nidification.", -2, 1},
                {2, "Tester la corde", "Vérifier sa solidité et baliser le risque.", "Tu confirmes quels nœuds tiennent encore et lesquels doivent être évités.", -1, 1},
                {3, "Laisser en l'état", "Ne rien modifier.", "Tu continues sans savoir qui utilisera encore cette corde.", 1, 0}
            }}},
        {"Marché sous les ponts", {true, "dispute_de_dette", "Dette disputée",
            "Deux vendeurs se disputent autour d'un papier scellé. Chacun cite une version différente de la même dette.",
            {"Le marché écoute : ici, une parole répétée peut devenir une rumeur avant de devenir une preuve."},
            {
                {1, "Comparer les sceaux", "Regarder seulement les éléments matériels disponibles.", "Les sceaux ne suffisent pas à trancher, mais l'un d'eux est nettement plus ancien que le récit associé.", -2, 1},
                {2, "Écouter les témoins", "Demander qui a réellement vu l'échange initial.", "Tu sépares deux témoins directs de plusieurs personnes qui ne font que répéter l'histoire.", -1, 2},
                {3, "Refuser l'affaire", "Ne pas devenir arbitre improvisé.", "Tu repars avant que quelqu'un transforme ton silence en soutien officiel.", 0, 0}
            }}},
        {"Temple des cloches fendues", {true, "cloche_sans_nom", "Cloche sans nom",
            "Une petite cloche fendue porte un serment dont le nom du signataire a été volontairement gratté.",
            {"Le texte subsiste, mais l'identité effacée empêche d'en faire une preuve personnelle complète."},
            {
                {1, "Copier le serment", "Recopier le texte sans compléter le nom manquant.", "Tu conserves les mots exacts et laisses le vide là où la preuve manque.", -2, 2},
                {2, "Comparer les fissures", "Étudier si l'effacement est ancien ou récent.", "Les fissures montrent que le nom a été gratté bien après la fabrication de la cloche.", -1, 1},
                {3, "Respecter le silence", "Ne rien copier et repartir.", "Tu laisses la cloche parler seulement à ceux qui reviendront la voir.", 0, 0}
            }}},
        {"Plaine sauvage", {true, "charrette_renversee", "Charrette renversée",
            "Une petite charrette vide s'est couchée dans un fossé. Les roues portent encore de la boue fraîche et deux pistes repartent dans des directions différentes.",
            {"Rien n'indique automatiquement si les occupants ont fui, été aidés ou simplement changé de véhicule."},
            {
                {1, "Examiner les traces", "Distinguer les pas, roues et traces animales avant de toucher à la charrette.", "Tu sépares une piste humaine récente d'anciennes marques de bétail sans inventer ce qui s'est passé ensuite.", -2, 1},
                {2, "Redresser la charrette", "La remettre hors du fossé sans déplacer ce qu'elle contient.", "La route redevient plus lisible et tu laisses les traces principales intactes pour un éventuel propriétaire.", -1, 1},
                {3, "Passer ton chemin", "Ne rien déplacer et conserver seulement l'emplacement.", "Tu notes la charrette comme incident de route non résolu.", 0, 0}
            }}},
        {"Bocage aux lanternes", {true, "lanterne_eteinte", "Lanterne éteinte",
            "Une lanterne suspendue au bord d'un sentier est éteinte alors que les autres brillent encore. La mèche a été retirée proprement.",
            {"Quelqu'un a agi ici ; l'absence de lumière n'est pas une preuve de danger à elle seule."},
            {
                {1, "Comparer les lanternes", "Vérifier huile, attaches et marques sur les lanternes voisines.", "Tu confirmes que celle-ci a été volontairement neutralisée, sans savoir encore par qui.", -2, 1},
                {2, "Remettre une mèche", "Réparer la lanterne en gardant l'ancienne mèche à part.", "Le passage retrouve un repère visible et l'ancienne mèche reste disponible comme trace.", -2, 1},
                {3, "Laisser l'ombre", "Ne pas modifier la scène.", "Tu continues avec un point sombre de plus dans le bocage.", 1, 0}
            }}},
        {"Désert d'argile rouge", {true, "puits_balisage_double", "Puits à double balisage",
            "Deux marques de caravanes indiquent le même puits, mais l'une avertit d'une eau impropre et l'autre annonce une halte sûre.",
            {"Les deux marques peuvent dater de moments différents ; choisir la plus rassurante n'en fait pas la plus récente."},
            {
                {1, "Tester sans boire", "Prélever un peu d'eau et observer dépôt, odeur et traces autour du puits.", "Tu obtiens un constat actuel sans transformer un test sommaire en certificat de potabilité.", -3, 2},
                {2, "Comparer les dates de passage", "Lire les traces de roues et les couches de peinture des balises.", "La marque d'avertissement est plus récente que l'autre, ce qui change la lecture du lieu.", -2, 1},
                {3, "Contourner le puits", "Conserver les deux versions et chercher un autre point d'eau.", "Tu refuses de risquer une intoxication juste pour trancher une dispute de balises.", 0, 0}
            }}},
        {"Quartier abandonné", {true, "fenetre_marquee", "Fenêtre marquée",
            "Une fenêtre condamnée porte trois traits de craie récents. Une seconde marque, plus ancienne, est cachée sous la poussière.",
            {"Le quartier semble vide, mais certaines personnes continuent clairement d'y passer."},
            {
                {1, "Observer sans entrer", "Comparer craie, poussière et passages au seuil.", "Tu confirmes des visites récentes sans attribuer la marque à un groupe précis.", -2, 1},
                {2, "Copier les deux marques", "Reproduire séparément le symbole récent et celui plus ancien.", "Tu gardes deux versions distinctes au lieu de les fusionner en un seul signe imaginaire.", -1, 2},
                {3, "Ne pas toucher", "Laisser le bâtiment tel quel.", "Le quartier conserve son secret et tu poursuis l'exploration.", 0, 0}
            }}},
        {"Mine sifflante", {true, "etai_fendu", "Étai fendu",
            "Un étai craque à intervalles réguliers dans une galerie secondaire. De fines poussières tombent quand le vent siffle dans la roche.",
            {"La galerie n'est pas encore effondrée, mais continuer sans lire le terrain augmente clairement le risque."},
            {
                {1, "Renforcer l'étai", "Caler le bois avec des pierres et une pièce de soutien disponible.", "Le passage cesse de travailler à chaque rafale et devient plus sûr pour cette exploration.", -3, 1},
                {2, "Marquer la galerie", "Baliser l'accès sans tenter une réparation improvisée.", "Tu rends le danger visible sans prétendre avoir réparé la structure.", -1, 1},
                {3, "Prendre une autre galerie", "Éviter la zone instable.", "Le détour coûte du temps mais évite de tester la résistance du plafond.", 1, 0}
            }}},
        {"Verger des lucioles de fer", {true, "outil_magnetise", "Outil aimanté par les lucioles",
            "Une poignée de lucioles métalliques s'est regroupée autour d'un vieil outil planté dans un tronc. Elles vibrent quand on approche du métal.",
            {"Le phénomène est observable maintenant ; rien ne garantit qu'il se reproduira demain au même endroit."},
            {
                {1, "Observer avec un petit objet", "Approcher lentement une pièce métallique sans toucher l'essaim.", "Les lucioles changent d'orientation avant de revenir à l'outil, ce qui confirme une réaction au métal.", -2, 1},
                {2, "Retirer l'outil", "Libérer doucement le tronc et poser l'outil au sol.", "Une partie de l'essaim suit l'outil tandis que le reste demeure près de l'arbre : le lien n'est donc pas unique.", -1, 2},
                {3, "Laisser l'essaim", "Ne pas perturber le phénomène.", "Tu gardes seulement une observation datée dans tes notes.", 0, 0}
            }}},
        {"Bois de la Corruption", {true, "ecorce_noire_recente", "Écorce noire récente",
            "Un arbre sain porte une bande d'écorce noircie qui n'était manifestement pas là depuis longtemps. Les plantes voisines ne sont pas toutes touchées.",
            {"La corruption n'avance donc pas comme une vague uniforme ; certaines traces doivent être comparées avant d'agir."},
            {
                {1, "Prélever un fragment", "Prendre seulement un petit morceau isolé et l'emballer séparément.", "Tu conserves un échantillon sans arracher toute l'écorce ni contaminer le reste du sac.", -2, 2},
                {2, "Baliser l'arbre", "Marquer l'emplacement et comparer les végétaux voisins.", "Tu crées un point de comparaison utile pour un futur passage sans prétendre avoir stoppé la corruption.", -1, 1},
                {3, "Éviter le contact", "Ne rien toucher et modifier légèrement l'itinéraire.", "Tu gardes tes distances et laisses la zone inchangée.", 1, 0}
            }}},
        {"Bosquet des Fées du Mana", {true, "cercle_offrandes", "Cercle d'offrandes déplacé",
            "Un cercle d'offrandes a été déplacé de quelques pas : fleurs fraîches d'un côté, anciennes pierres de l'autre.",
            {"Deux usages du même lieu semblent coexister ; remettre tout 'comme avant' favoriserait forcément l'un des deux."},
            {
                {1, "Comparer les deux usages", "Observer quelles traces appartiennent aux visiteurs récents et lesquelles sont plus anciennes.", "Tu identifies deux traditions différentes sans déclarer l'une authentique et l'autre fausse.", -2, 2},
                {2, "Créer un passage entre les deux", "Déplacer seulement une branche qui coupe l'accès, sans toucher aux offrandes.", "Les deux cercles restent distincts mais redeviennent accessibles.", -1, 1},
                {3, "Ne rien modifier", "Respecter les deux installations et continuer.", "Tu repars sans transformer le bosquet en puzzle à résoudre de force.", 0, 0}
            }}},
        {"Sanctuaire kitsuné des Neuf Étincelles", {true, "rubans_neuf_versions", "Rubans aux neuf versions",
            "Neuf rubans portent des formulations légèrement différentes d'une même promesse. Deux utilisent des caractères plus anciens.",
            {"Même sans tout traduire, l'ordre, l'encre et les corrections montrent que le texte a évolué."},
            {
                {1, "Copier les variantes", "Recopier les différences exactes sans choisir la version qui te plaît le plus.", "Tu conserves les neuf formulations comme variantes distinctes et datables.", -2, 2},
                {2, "Comparer encres et nœuds", "Observer la matière plutôt que supposer le sens des caractères inconnus.", "Les deux rubans archaïques sont aussi les plus anciens matériellement, ce qui constitue une vraie trace.", -1, 1},
                {3, "Respecter le sanctuaire", "Ne pas toucher aux rubans.", "Tu gardes seulement l'ordre visible des neuf promesses.", 0, 0}
            }}},
        {"Archipel des îles flottantes", {true, "amarre_derivante", "Amarre dérivante",
            "Une corde d'amarrage tendue entre deux îlots ne rejoint plus exactement l'ancien anneau de pierre. Le déplacement est faible mais mesurable.",
            {"Le terrain lui-même bouge ; une carte correcte hier peut devenir approximative sans que personne ait menti."},
            {
                {1, "Mesurer le décalage", "Comparer corde, anneau et ombre des deux îlots.", "Tu notes un déplacement réel et limité au lieu de conclure que toute la carte est fausse.", -3, 2},
                {2, "Retendre l'amarre", "Sécuriser le passage actuel en laissant l'ancien nœud visible.", "Le trajet redevient praticable et l'ancien point d'ancrage reste lisible comme preuve du déplacement.", -2, 1},
                {3, "Chercher une autre liaison", "Ne pas forcer la corde et contourner par un autre îlot.", "Tu acceptes un détour plutôt que tester une amarre dont la géométrie a changé.", 1, 0}
            }}}

    };
    return values;
}
}

BiomeNonCombatInteraction BiomeNonCombatInteractionSystem::buildCurrentInteraction(const std::string& biomeName, int worldDay, int dayProgressUnit)
{
    const auto it = catalog().find(biomeName);
    if (it == catalog().end()) return {};

    const int safeDay = worldDay < 0 ? 0 : worldDay;
    const int safeUnit = dayProgressUnit < 0 ? 0 : dayProgressUnit;
    const std::uint32_t seed = stableHash(biomeName + "|interaction|" + std::to_string(safeDay) + "|" + std::to_string(safeUnit / 2));

    // Roughly half of the visits expose a social/environmental choice. Normal quiet visits remain possible.
    if (seed % 100u >= 58u) return {};
    return it->second;
}

BiomeNonCombatInteractionResult BiomeNonCombatInteractionSystem::resolve(const BiomeNonCombatInteraction& interaction, int choiceId)
{
    BiomeNonCombatInteractionResult result;
    if (!interaction.active) return result;

    for (const BiomeNonCombatChoice& choice : interaction.choices)
    {
        if (choice.id != choiceId) continue;
        result.resolved = true;
        result.interactionId = interaction.id;
        result.choiceLabel = choice.label;
        result.explorationRollShift = choice.explorationRollShift;
        result.questProgress = choice.questProgress;
        result.lines = {choice.resultLine};
        return result;
    }
    return result;
}

std::string BiomeNonCombatInteractionSystem::journalKey(const std::string& biomeName, int worldDay, const std::string& interactionId)
{
    return biomeName + "|jour=" + std::to_string(worldDay < 0 ? 0 : worldDay) + "|" + interactionId;
}
