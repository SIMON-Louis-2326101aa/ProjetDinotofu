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

bool interactionChoiceIsNotableForLongTermHistory(const std::string& interactionId, int choiceId)
{
    // Most local interactions remain field notes only. Long-term character history is
    // reserved for discoveries/actions that are unusual enough to matter later.
    if (choiceId <= 0) return false;

    if (interactionId == "feuillet_a_secher") return choiceId <= 2;
    if (interactionId == "barque_sans_passeur") return choiceId <= 2;
    if (interactionId == "manege_un_tour") return choiceId <= 2;
    if (interactionId == "cloche_sans_nom") return choiceId <= 2;
    if (interactionId == "rubans_neuf_versions") return choiceId <= 2;
    if (interactionId == "amarre_derivante") return choiceId <= 2;

    return false;
}

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
        {"Mares gélatineuses", {true, "passage_de_slimes_neutres", "Passage de slimes neutres",
            "Une file de petits slimes translucides traverse lentement le sentier vers une mare plus profonde sans montrer d'agressivité.",
            {"Leur trajet recoupe une zone de récolte ; intervenir peut aider le passage ou simplement déplacer le problème."},
            {
                {1, "Observer leur trajet", "Attendre quelques minutes et noter ce qu'ils évitent naturellement.", "Le groupe contourne une eau trop brillante et révèle un bord de mare plus sûr que le centre.", -2, 1},
                {2, "Dégager le sentier", "Déplacer quelques pierres sans toucher directement aux slimes.", "Le passage devient plus fluide et les slimes quittent la zone sans réaction hostile.", -2, 1},
                {3, "Faire un détour", "Laisser la petite migration suivre son cours.", "Tu changes de rive et conserves seulement l'heure et la direction du passage dans tes notes.", 0, 0}
            }}},
        {"Montagne froide", {true, "cairn_ecroule", "Cairn écroulé",
            "Un cairn de voyageurs s'est affaissé sous le gel. Deux pierres portent encore des marques de route lisibles.",
            {"Le réparer peut aider les prochains passages, mais une mauvaise orientation serait pire qu'un cairn cassé."},
            {
                {1, "Comparer les marques", "Vérifier pente, vent et anciennes traces avant de replacer quoi que ce soit.", "Les deux marques indiquent bien le même col ; l'une a simplement pivoté avec l'effondrement.", -2, 2},
                {2, "Reconstruire prudemment", "Remonter le cairn en conservant l'orientation vérifiée.", "Le repère tient de nouveau et reste assez distinct pour ne pas être confondu avec une simple pile de pierres.", -3, 1},
                {3, "Ne pas toucher", "Éviter de créer un faux repère sous la neige.", "Tu notes le cairn comme endommagé et continues sans modifier la pente.", 0, 0}
            }}},
        {"Ruines effondrées", {true, "poutre_sur_mosaique", "Poutre sur la mosaïque",
            "Une poutre tombée masque une partie d'une mosaïque et retient en même temps quelques pierres instables du plafond.",
            {"Dégager l'image trop vite peut détruire exactement ce que tu essaies d'étudier."},
            {
                {1, "Lire les fragments visibles", "Reconstituer seulement ce que montrent les morceaux accessibles.", "Tu identifies un motif de sentinelle sans déplacer la poutre ni prétendre connaître la scène entière.", -2, 2},
                {2, "Caler avant de déplacer", "Renforcer le plafond puis soulever légèrement la poutre.", "Une portion supplémentaire apparaît sans provoquer d'effondrement ; le reste demeure volontairement couvert.", -1, 2},
                {3, "Laisser la structure", "Ne pas risquer les ruines pour une image incomplète.", "Tu notes l'emplacement et conserves la mosaïque en l'état.", 0, 0}
            }}},
        {"Canaux de brume bleue", {true, "barque_sans_passeur", "Barque sans passeur",
            "Une petite barque vide cogne doucement contre le quai. La corde est humide et le nœud a été refait récemment.",
            {"La brume cache l'autre rive par intermittence ; utiliser la barque sans contexte peut transformer un raccourci en disparition administrative."},
            {
                {1, "Examiner le nœud", "Comparer la corde, les marques du quai et le niveau de l'eau.", "Le nœud vient d'un batelier habitué aux canaux ; rien ne prouve pourtant qu'il ait abandonné la barque volontairement.", -2, 2},
                {2, "Sécuriser l'amarre", "Refaire l'attache sans déplacer la barque du quai.", "La barque cesse de dériver et le passage reste disponible à son propriétaire.", -2, 1},
                {3, "Laisser la barque", "Ne pas utiliser un moyen de transport qui ne t'appartient pas.", "Tu gardes seulement la position du quai et l'heure du constat.", 0, 0}
            }}},
        {"Foire abandonnée", {true, "manege_un_tour", "Manège d'un seul tour",
            "Un petit manège tourne d'un cran puis s'arrête alors qu'aucun vent ne pousse sa mécanique.",
            {"Les pigeons du toit s'envolent juste avant le mouvement et reviennent aussitôt après."},
            {
                {1, "Observer le mécanisme", "Chercher ressort, contrepoids et déclencheur sans monter sur le manège.", "Un ressort travaille encore, mais il n'explique pas pourquoi le mouvement commence toujours après le départ des oiseaux.", -1, 2},
                {2, "Bloquer la roue", "Sécuriser temporairement le mécanisme avec une cale visible.", "Le manège reste immobile pour cette visite ; la cale porte une marque claire afin qu'un autre explorateur sache qu'elle n'est pas d'origine.", -2, 1},
                {3, "Attendre un second mouvement", "Ne rien toucher et vérifier si le phénomène se répète.", "Après plusieurs minutes, rien ne bouge. Tu repars avec un événement daté plutôt qu'une théorie définitive.", 0, 1}
            }}},
        {"Carrière des os blancs", {true, "marque_de_taille_recente", "Marque de taille récente",
            "Une strate fossile porte une marque de taille très récente au milieu de poussières anciennes.",
            {"Quelqu'un travaille donc encore ici, mais la carrière ne dit pas si cette présence est autorisée, perdue ou dangereuse."},
            {
                {1, "Comparer les outils", "Lire largeur, profondeur et angle de la marque sans prélever le fossile.", "La trace vient d'un outil de mineur plus fin que ceux utilisés autrefois dans cette couche.", -2, 2},
                {2, "Baliser la strate", "Rendre l'emplacement visible sans écrire sur le fossile.", "La strate pourra être retrouvée sans ajouter une nouvelle marque directement sur l'os blanc.", -1, 1},
                {3, "Continuer", "Ne pas transformer chaque coup d'outil récent en enquête.", "Tu conserves la position dans tes notes et poursuis la carrière.", 0, 0}
            }}},
        {"Jardin des statues qui pleurent", {true, "bouquet_devant_ange", "Bouquet devant l'ange",
            "Un bouquet très récent repose devant une statue en pleurs. Une fleur a été déplacée hors du cercle, comme si quelqu'un avait interrompu le geste.",
            {"Le jardin est réputé dans plusieurs villes : toucher aux offrandes peut modifier autant les traces humaines que le phénomène de pierre lui-même."},
            {
                {1, "Comparer les traces", "Observer graviers, humidité et tiges sans déplacer le bouquet.", "Deux séries de pas arrivent au socle, mais une seule repart. Tu notes le fait sans inventer ce qui manque.", -2, 2},
                {2, "Redresser seulement la fleur", "Remettre la fleur tombée sans réorganiser le reste de l'offrande.", "La fleur retrouve le cercle. Quand tu relèves les yeux, la main de la statue semble légèrement plus proche du bouquet.", -1, 1},
                {3, "Laisser l'offrande intacte", "Ne rien toucher et mémoriser la disposition actuelle.", "Tu repars avec un repère précis : bouquet, socle, humidité et orientation du visage restent notés pour comparaison future.", 0, 1}
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
        result.notableForLongTermHistory = interactionChoiceIsNotableForLongTermHistory(interaction.id, choice.id);
        result.lines = {choice.resultLine};
        return result;
    }
    return result;
}

std::string BiomeNonCombatInteractionSystem::journalKey(const std::string& biomeName, int worldDay, const std::string& interactionId)
{
    return biomeName + "|jour=" + std::to_string(worldDay < 0 ? 0 : worldDay) + "|" + interactionId;
}
