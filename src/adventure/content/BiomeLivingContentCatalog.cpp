#include "adventure/content/BiomeLivingContentCatalog.hpp"
#include <cstdint>
#include <unordered_map>

namespace
{
const std::unordered_map<std::string, BiomeLivingContentProfile>& profiles()
{
    static const std::unordered_map<std::string, BiomeLivingContentProfile> values = {
        {"Plaine sauvage", {
            "Une herbe haute ondule autour de pierres plates, de vieux chemins d'animaux et de quelques arbres isolés.",
            "Les fossés masqués par l'herbe et les sols détrempés après les passages rendent les courses aveugles plus risquées qu'elles n'en ont l'air.",
            "Herbes médicinales communes, fibres sèches et silex affleurent près des zones piétinées.",
            "Lièvres, alouettes et petits rongeurs réapparaissent rapidement dès que les prédateurs ou voyageurs quittent un secteur.",
            "Des foyers froids, traces de roues et bornes déplacées montrent que la plaine sert réellement de passage entre plusieurs communautés.",
            "Une ligne d'herbe couchée décrit un cercle presque parfait sans empreinte claire à son centre."}},
        {"Route commerciale", {
            "La chaussée est marquée par des roues de largeurs différentes, des relais improvisés et des panneaux réparés avec des matériaux de plusieurs villes.",
            "Les bas-côtés meubles et les charrettes arrêtées peuvent réduire brutalement l'espace disponible lorsqu'un affrontement éclate.",
            "Clous de ferrage, ficelles de colis, morceaux de toile huilée et quelques marchandises perdues restent parfois récupérables.",
            "Corbeaux, chiens de relais et chevaux de passage réagissent aux mouvements inhabituels bien avant les voyageurs distraits.",
            "Des marques de caravanes, annonces de péage et avis de recherche se superposent sur les poteaux au lieu d'être remplacés proprement.",
            "Un panneau indique deux distances différentes vers la même ville, écrites avec la même encre mais pas de la même main."}},
        {"Mares gélatineuses", {
            "Des cuvettes peu profondes brillent d'une pellicule colorée entre des îlots de terre ferme couverts de mousse.",
            "Le bord de certaines mares cède sous un poids soudain et les résidus gélatineux rendent les appuis glissants après le passage d'une créature.",
            "Gelée résiduelle, mousses filtrantes et petits cristaux digestifs peuvent être récoltés loin des noyaux actifs.",
            "Grenouilles, insectes d'eau et oiseaux à longues pattes exploitent les mares tant que les slimes restent calmes.",
            "Des piquets numérotés montrent que des habitants surveillent l'extension des mares plutôt que de simplement les éviter.",
            "Une mare parfaitement claire contient des bulles qui descendent lentement au lieu de remonter."}},
        {"Forêt ancienne", {
            "De vieux troncs massifs imposent des détours naturels et laissent entre eux des clairières plus jeunes, visiblement entretenues par le passage des bêtes.",
            "Racines hautes, branches mortes et sous-bois dense favorisent les embuscades et pénalisent davantage la précipitation que l'exploration attentive.",
            "Champignons, résine, bois tombé et plantes d'ombre peuvent être récoltés sans abattre les arbres anciens.",
            "Cerfs, pics, renards et colonies d'insectes occupent des zones différentes et laissent des signes distincts de leur présence.",
            "Des rubans de bûcherons et de chasseurs indiquent plusieurs usages concurrents de la forêt, certains anciens, d'autres très récents.",
            "Deux arbres éloignés portent exactement la même cicatrice de foudre à la même hauteur."}},
        {"Montagne froide", {
            "Des sentiers de pierre longent des pentes couvertes de plaques de neige et de végétation basse accrochée aux fissures.",
            "Les éboulis, le gel et les rafales latérales rendent certaines positions instables sans faire de chaque déplacement un jet arbitraire.",
            "Lichen froid, quartz, minerai pauvre et eau de fonte s'accumulent dans les replis protégés.",
            "Bouquetins, rapaces et petits rongeurs de roche empruntent des passages que les voyageurs lourds ne peuvent pas toujours suivre.",
            "Des cairns ont été déplacés puis reconstruits, preuve que les itinéraires changent avec les saisons et les chutes de pierre.",
            "Une plaque de neige reste intacte alors que des traces arrivent jusqu'à son bord depuis deux directions."}},
        {"Marais trouble", {
            "Des nappes d'eau brune alternent avec des langues de terre noire couvertes de roseaux, de racines et de bois flotté.",
            "La profondeur varie brutalement et la vase retient les pieds lourds, surtout lorsqu'un groupe tente de se déplacer trop vite.",
            "Roseaux solides, tourbe, plantes amères et poches de gaz exploitable apparaissent autour des zones stables.",
            "Hérons, grenouilles, sangsues et petits reptiles se concentrent près des eaux réellement vivantes plutôt que dans toutes les mares.",
            "Des passerelles réparées avec plusieurs types de bois montrent qu'un trajet local reste utilisé malgré le mauvais terrain.",
            "Une rangée de bulles traverse parfois une mare en ligne droite sans qu'aucune forme ne rompe la surface."}},
        {"Cimetière oublié", {
            "Des tombes inclinées disparaissent sous les herbes tandis que quelques mausolées restent étonnamment propres autour de leurs portes.",
            "Pierres descellées, fosses anciennes et grilles rouillées rendent les déplacements brusques dangereux même sans présence surnaturelle.",
            "Cire funéraire, herbes de deuil, petits objets votifs abandonnés et pierre taillée peuvent être trouvés sans profaner les sépultures.",
            "Corneilles, chats errants et insectes nocturnes utilisent le cimetière comme un habitat ordinaire dès qu'il reste calme.",
            "Certaines fleurs sont fraîches et certaines inscriptions nettoyées, signe que tout le monde n'a pas réellement oublié le lieu.",
            "Une tombe porte une date plus récente que la pierre qui l'entoure, mais aucune terre n'a été remuée devant elle."}},
        {"Ruines effondrées", {
            "Des pans de murs forment encore des pièces incomplètes entre des amas de pierre, de tuiles et de poutres noircies.",
            "Les sols creux et les linteaux fissurés peuvent céder après un choc ; le danger vient autant de la structure que des occupants.",
            "Métal récupérable, briques intactes, verre ancien et fragments de mobilier survivent dans les zones qui n'ont pas déjà été pillées.",
            "Lézards, chouettes et petits mammifères utilisent les étages ouverts tandis que les animaux plus lourds préfèrent les cours.",
            "Des marques de récupération montrent que plusieurs groupes ont fouillé les lieux avec des méthodes et des objectifs différents.",
            "Une porte encore verrouillée est encastrée dans un mur dont les deux côtés se sont pourtant complètement effondrés."}},
        {"Bocage aux lanternes", {
            "Des haies épaisses découpent les champs en couloirs irréguliers où pendent de petites lanternes entretenues par des mains différentes.",
            "Les passages étroits limitent les lignes de vue et les fossés cachés derrière les haies punissent surtout les charges sans reconnaissance.",
            "Baies, cire, bois souple et herbes aromatiques poussent près des chemins régulièrement entretenus.",
            "Hérissons, chouettes et insectes lumineux suivent les haies et évitent les lanternes dont la flamme brûle trop bas.",
            "Des fermiers ajoutent parfois un ruban ou un signe aux lanternes pour signaler un animal perdu, un danger ou un chemin praticable.",
            "Une lanterne éteinte projette pourtant une faible lueur sur la haie lorsqu'on la regarde depuis le champ opposé."}},
        {"Désert d'argile rouge", {
            "Des plaques d'argile rouge se fendent autour de buttes basses et de ravines sèches que le vent remplit puis vide de poussière.",
            "Les croûtes d'argile peuvent casser sous un poids concentré et les ravines canalisent brutalement les rafales.",
            "Argile dense, sels minéraux, pierres ferrugineuses et racines sèches apparaissent dans les couches exposées.",
            "Lézards rouges, scarabées et petits rapaces se déplacent surtout à l'ombre des reliefs plutôt qu'en plein terrain ouvert.",
            "Des cairns bas et des morceaux de tissu noués indiquent les puits connus et les détours récemment jugés dangereux.",
            "Une série de fissures forme un motif de pas réguliers sur plusieurs dizaines de mètres sans empreinte véritable."}},
        {"Quartier abandonné", {
            "Des maisons murées alternent avec des ateliers ouverts où des objets ordinaires sont restés à leur place comme après un départ précipité.",
            "Balcons fragiles, caves ouvertes et débris de rue rendent certains axes plus dangereux que d'autres sans condamner tout le quartier.",
            "Ferrures, tissus, outils simples et provisions oubliées subsistent surtout dans les bâtiments qui ont été difficiles à atteindre.",
            "Pigeons, chats et rats ont recréé leurs propres territoires entre les rues désertes.",
            "Graffitis, cadenas récents et traces de camp montrent que le quartier n'est pas aussi vide que son nom le prétend.",
            "Une fenêtre est nettoyée de l'intérieur chaque jour alors que sa porte d'entrée reste condamnée par des planches anciennes."}},
        {"Mine sifflante", {
            "Des galeries étroites traversent une roche striée de veines qui produisent un sifflement lorsque l'air circule dans certaines fissures.",
            "Étais fatigués, rails tordus et poches de poussière rendent les explosions et impacts puissants particulièrement dangereux.",
            "Minerai commun, cristaux sonores et pièces de mécanisme abandonnées restent accessibles dans les branches secondaires.",
            "Chauves-souris, insectes pâles et petits rongeurs suivent les zones où l'air reste respirable.",
            "Des marques de mineurs indiquent les galeries condamnées, mais certaines ont été barrées à une date beaucoup plus récente.",
            "Un tunnel souffle vers l'intérieur quelle que soit la direction du courant mesuré dans les galeries voisines."}},
        {"Verger des lucioles de fer", {
            "Des arbres noueux portent de petits fruits sombres tandis que des insectes métalliques brillent entre les branches au crépuscule.",
            "Branches basses, racines serrées et essaims attirés par le métal rendent les combats prolongés plus encombrés qu'en terrain ouvert.",
            "Fruits acides, résine conductrice, coques métalliques abandonnées et bois dense se récupèrent près des arbres matures.",
            "Oiseaux insectivores et lucioles de fer se disputent les mêmes rangées sans rendre chaque essaim agressif.",
            "Des paniers numérotés et rubans de récolte prouvent que des cueilleurs reviennent encore malgré les risques.",
            "Un arbre ne porte aucune feuille mais attire deux fois plus de lucioles que tous les autres réunis."}},
        {"Falaises des drakes gris", {
            "Des corniches calcaires surplombent des ravins où le vent emporte les sons avant qu'ils n'atteignent le fond.",
            "Les rebords friables et les rafales ascendantes rendent les positions proches du vide très puissantes mais réellement risquées.",
            "Écailles grises tombées, pierre légère, coquilles d'œufs cassées et herbes de falaise s'accumulent dans les anfractuosités.",
            "Chèvres sauvages et rapaces partagent les hauteurs avec les drakes tant qu'ils évitent les nids actifs.",
            "Des cordes coupées, pitons récents et signes de grimpeurs montrent que des chasseurs ou observateurs fréquentent encore les parois.",
            "Une cavité contient des marques de griffes orientées vers l'intérieur comme si quelque chose avait essayé d'entrer depuis la roche."}},
        {"Temple des cloches fendues", {
            "Des nefs ouvertes au vent laissent pendre des cloches cassées dont certaines vibrent sans son.",
            "Les dalles fendues résonnent différemment sous les pas lourds et peuvent trahir une cavité.",
            "De la cire de sanctuaire, des éclats de bronze et des fils de vœu restent coincés dans les autels.",
            "De petits geckos gris vivent derrière les plaques votives et détalent avant les combats.",
            "Des visiteurs ont noué des rubans récents à côté de serments vieux de plusieurs générations.",
            "Une cloche sans battant porte des marques de doigts sur sa face intérieure."}},
        {"Canaux de brume bleue", {
            "Les passerelles basses disparaissent par morceaux derrière une brume bleutée qui colle aux rambardes.",
            "Le niveau de l'eau change assez vite pour condamner un raccourci sans transformer tout le canal en piège arbitraire.",
            "Des roseaux bleus, perles de verre humide et cordages gonflés dérivent près des piles.",
            "Des martins-pêcheurs pâles suivent les barques sans jamais approcher les zones où la brume se fige.",
            "Des craies de bateliers indiquent quels ponts étaient praticables lors de leur dernier passage.",
            "Une barque vide est attachée avec un nœud encore humide, mais aucune empreinte ne quitte le quai."}},
        {"Carrière des os blancs", {
            "La poussière blanche couvre tout, sauf certaines strates fossiles trop lisses pour la retenir.",
            "Des pans de craie s'effondrent après les vibrations et rendent les charges brutales risquées.",
            "Craie dense, fragments fossiles et éclats de géant affleurent dans les couches les plus anciennes.",
            "Des lézards ivoire se chauffent sur les pierres et disparaissent quand les scarabées d'os approchent.",
            "Des marques de mineurs indiquent des galeries abandonnées, corrigées parfois par une main plus récente.",
            "Une série d'empreintes énormes s'arrête sous une paroi au lieu d'en sortir."}},
        {"Marché sous les ponts", {
            "Les étals utilisent les arches comme murs et changent d'emplacement dès qu'une patrouille devient trop curieuse.",
            "Les passerelles encombrées créent des angles morts où un groupe peut se séparer en quelques secondes.",
            "Jetons de contrebande, bordereaux scellés et ficelles codées traînent parmi les déchets du quai.",
            "Des chats de pont dorment sous les caisses des vendeurs fiables et évitent certains collecteurs masqués.",
            "Les prix écrits à la craie sont parfois corrigés par une seconde main avec un symbole de dette.",
            "Un étal fermé possède trois cadenas différents, dont un posé de l'intérieur."}},
        {"Jardin des statues qui pleurent", {
            "Des haies nobles entourent des statues dont les joues restent humides même sous un ciel sec.",
            "Les graviers clairs roulent sous les semelles tandis que les racines de marbre accrochent les chevilles.",
            "Pétales pétrifiés, larmes minérales et épines blanches peuvent être prélevés sans briser les œuvres.",
            "Des papillons de pierre restent immobiles des heures avant de changer soudainement de statue.",
            "Des bouquets récents prouvent que quelqu'un entretient encore une partie du jardin.",
            "Une statue a les mains propres alors que tout son corps est couvert de mousse."}},
        {"Bois de la Corruption", {
            "La lumière traverse les feuilles en teintes sales et certaines racines semblent avoir poussé autour d'anciennes traces de camp.",
            "Les zones noircies affaiblissent les appuis et rendent les blessures ouvertes plus dangereuses sans empoisonner gratuitement chaque pas.",
            "Fils d'ombre, bois noirci et noyaux instables apparaissent près des arbres réellement contaminés.",
            "Des corneilles saines nichent encore en périphérie et refusent d'entrer dans les clairières trop silencieuses.",
            "Des balises de bûcherons ont été retournées pour indiquer des chemins devenus impraticables.",
            "Un tronc porte deux ombres différentes malgré une seule source de lumière."}},
        {"Crypte du Sombre-Lien", {
            "Les alcôves sont reliées par des chaînes gravées de noms dont plusieurs ont été volontairement grattés.",
            "Certains sceaux réagissent au bruit ou aux noms prononcés, ce qui rend la discrétion différente du simple silence.",
            "Cire sombre, os liés et fragments de chaîne conservent une valeur rituelle et artisanale.",
            "De petits coléoptères blancs vivent dans les joints et fuient les salles où une présence se réveille.",
            "Des traces de craie montrent que des explorateurs ont compté les portes avant de revenir sur leurs propres chiffres.",
            "Une niche vide possède une plaque funéraire dont le nom change selon l'angle de lecture."}},
        {"Archives noyées", {
            "Des rayonnages dépassent d'une eau sombre où flottent encore des pages collées en paquets.",
            "L'encre dissoute masque parfois la profondeur et les planchers gonflés cèdent sous un poids mal réparti.",
            "Encre de marée, pages murmurantes et sceaux administratifs peuvent être récupérés sur les zones sèches.",
            "Des poissons translucides mangent uniquement les pages vierges et évitent les textes encore lisibles.",
            "Des annotations récentes contredisent parfois les anciens registres sans effacer les deux versions.",
            "Un livre fermé continue de produire de petites bulles alors qu'il est posé hors de l'eau."}},
        {"Foire abandonnée", {
            "Les couleurs fanées des stands résistent mieux que le bois et donnent au lieu un air de fête interrompue trop proprement.",
            "Cordes, planches mobiles et mécanismes de manège créent des dangers physiques avant même les créatures.",
            "Tickets déchirés, billes de miroir et ressorts décoratifs restent récupérables dans les stands.",
            "Des pigeons ont colonisé la grande roue mais refusent le chapiteau central.",
            "Des messages de forains indiquent des dettes, des rendez-vous et des numéros de stand qui n'existent plus.",
            "Une cible de tir possède une flèche plantée depuis l'arrière du panneau."}},
        {"Sanctuaire kitsuné des Neuf Étincelles", {
            "Neuf foyers bas dessinent un chemin dont la symétrie change dès qu'on cesse de le regarder directement.",
            "Les illusions déplacent surtout les repères : courir sans vérifier peut faire perdre du temps plutôt que téléporter arbitrairement le joueur.",
            "Braises kitsuné, perles de miroir et cendres parfumées restent près des autels secondaires.",
            "Des renards ordinaires traversent le sanctuaire librement et fixent parfois un visiteur avant de repartir.",
            "Des offrandes récentes utilisent plusieurs écritures, signe que le sanctuaire reçoit encore du monde.",
            "Une neuvième flamme reflète parfois une dixième silhouette dans les bols de cuivre."}},
        {"Désert des Protecteurs", {
            "Un sable presque blanc s'accumule autour de statues de gardes tournées vers des directions différentes.",
            "Les creux entre les statues concentrent chaleur et poussière : suivre une ligne droite n'est pas toujours le chemin le plus sûr.",
            "Sel lunaire, fragments de serment et éclats de dorure ancienne restent accessibles sans profaner les grands monuments.",
            "Des scarabées sacrés nettoient les socles et ignorent les voyageurs qui gardent leurs distances.",
            "Des cairns récents utilisent des morceaux de tissus de plusieurs villes, preuve de passages réels malgré l'isolement.",
            "Une statue porte une empreinte de main beaucoup plus récente que l'érosion de son visage."}},
        {"Sanctuaire antique des Veilleurs", {
            "Des portes de pierre encadrent des cours silencieuses où chaque seuil possède un symbole de jugement différent.",
            "Certaines dalles s'enfoncent sous un poids brutal : l'attention et la manière d'avancer comptent autant que la force.",
            "Poussière de sceau, fibres votives et fragments de volonté cristallisée apparaissent près des autels secondaires.",
            "De petits oiseaux dorés nichent dans les corniches mais quittent une salle avant l'activation d'une sentinelle.",
            "Des visiteurs ont gravé des réponses à d'anciennes questions morales sans que le sanctuaire confirme lesquelles étaient justes.",
            "Une porte reste entrouverte seulement quand personne ne la regarde directement, sans jamais changer de place."}},
        {"Quartier des Lames Muettes", {
            "Les ruelles absorbent les sons courts tandis que les enseignes sont peintes sur la tranche plutôt que sur la face.",
            "Cordes basses, fenêtres ouvertes et angles de tir rendent les déplacements prévisibles dangereux pour les groupes bruyants.",
            "Fils noirs, fioles vides et fragments de contrats brûlés subsistent dans les gouttières.",
            "Des chats maigres connaissent les chemins sûrs et disparaissent avant qu'une silhouette masquée ne traverse une ruelle.",
            "Des marques de craie sont effacées puis redessinées ailleurs, signe d'une organisation qui adapte réellement ses habitudes.",
            "Une porte sans poignée présente des traces d'usure exactement à hauteur d'épaule."}},
        {"Toits des Assassins", {
            "Tuiles, passerelles et cordages forment un second réseau de rues au-dessus des habitants qui ne lèvent presque jamais les yeux.",
            "Le vent change l'équilibre et les trajectoires : courir au bord d'une corniche reste une décision, pas une animation gratuite.",
            "Plumes dressées, crochets de corde et pointes d'arbalète émoussées se coincent dans les gouttières.",
            "Des corbeaux dressés se posent toujours sur certains clochers avant de repartir par paires.",
            "Des tissus noués aux cheminées semblent signaler des passages ou des dangers à ceux qui connaissent le code.",
            "Une corde coupée a été renouée avec un nœud différent, comme si deux groupes utilisaient la même route."}},
        {"Nid draconique rouge", {
            "La roche est vitrifiée par endroits et de profondes griffures convergent vers les zones les plus chaudes.",
            "Des poches de cendre cachent des pierres brûlantes : la précipitation coûte plus cher que la simple chaleur ambiante.",
            "Écailles rouges, verre de braise et coquilles minérales peuvent être trouvés loin des œufs encore surveillés.",
            "De petits lézards de cendre suivent les draconides et se figent lorsqu'un grand battement d'ailes résonne au loin.",
            "Des kobolds ont placé des offrandes triées par taille, suggérant une hiérarchie plus organisée qu'un simple repaire de bêtes.",
            "Une ancienne empreinte gigantesque est partiellement recouverte par plusieurs empreintes beaucoup plus petites."}},
        {"Coulées de lave noire", {
            "Des veines de lave presque noires brillent seulement aux fissures, entre des îlots de basalte qui semblent stables de loin.",
            "La roche peut céder après plusieurs chocs : un combat prolongé modifie les appuis sans condamner arbitrairement toute la zone.",
            "Scories denses, verre volcanique et noyaux de braise refroidie se forment aux frontières des coulées.",
            "Des insectes thermiques vivent sous les pierres froides et remontent quelques instants avant une poussée de chaleur locale.",
            "Des piquets métalliques datés montrent que d'autres voyageurs mesurent la progression des coulées au lieu de la deviner.",
            "Un cercle de basalte reste froid alors qu'il est entouré de roche encore rouge."}},
        {"Bosquet des Fées du Mana", {
            "Des fleurs trop lumineuses dessinent des chemins qui paraissent différents selon la hauteur du regard.",
            "Les concentrations de mana troublent surtout les repères et les sorts mal contrôlés, sans transformer chaque pas en piège.",
            "Pollen de mana, ailes abandonnées et gouttes de sève prismatique se trouvent autour des clairières calmes.",
            "Des lucioles de mana suivent certains visiteurs quelques minutes puis les abandonnent sans raison lisible.",
            "De petites offrandes et des rubans prouvent que des habitants viennent négocier avec les fées plutôt que seulement les combattre.",
            "Un cercle de champignons est intact sauf à un endroit où une minuscule chaise a été déplacée vers l'extérieur."}},
        {"Glacier des Serments froids", {
            "Des bannières gelées restent prises dans la glace avec des phrases incomplètes visibles sous plusieurs couches.",
            "Les surfaces polies transforment une charge mal préparée en glissade tandis que les crevasses amplifient certains sons.",
            "Cristaux de givre, métal froid et fragments de sceaux gelés affleurent après les variations de température.",
            "Des renards blancs suivent les traces humaines mais contournent systématiquement les chevaliers figés.",
            "Des cairns portent des noms barrés puis réécrits, comme si certains voyageurs étaient revenus corriger un ancien serment.",
            "Une lame prisonnière de la glace ne présente aucune bulle autour d'elle, contrairement à tous les autres objets gelés."}},
        {"Confluence du Mana pur", {
            "Plusieurs courants lumineux se croisent sans se mélanger immédiatement, comme des rivières superposées.",
            "Un sort mal stabilisé peut laisser une trace locale qui modifie brièvement le terrain sans annoncer le prochain événement.",
            "Poussière arcanique, noyaux élémentaires et condensation de mana se déposent sur les pierres froides.",
            "Des insectes prismatiques changent de trajectoire avant les poussées de mana et servent d'indicateur naturel.",
            "Des chercheurs ont planté des tiges graduées et datées plutôt que de prétendre comprendre la zone entièrement.",
            "Deux courants se croisent à un endroit où aucune lumière ne se reflète sur le sol."}},
        {"Archipel des îles flottantes", {
            "Des îles de tailles très différentes dérivent assez lentement pour conserver des chemins, mais pas assez pour rendre les cartes éternelles.",
            "Les rafales verticales et les pierres mobiles rendent les charges aveugles dangereuses près des bords.",
            "Poussière de mana, roche légère et éclats de chance consciente se logent dans les fractures exposées.",
            "De grands oiseaux suivent les courants ascendants tandis que de petites baleines de ciel restent très loin des zones habitées.",
            "Des cairns de voyageurs marquent les sauts sûrs connus au moment de leur passage, avec date et direction du vent.",
            "Une petite île porte des traces de feu autour d'un camp dont toutes les pierres sont retournées vers le ciel."}}
    };
    return values;
}

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
}

bool BiomeLivingContentCatalog::hasProfile(const std::string& biomeName)
{
    return profiles().find(biomeName) != profiles().end();
}

BiomeLivingContentProfile BiomeLivingContentCatalog::forBiome(const std::string& biomeName)
{
    const auto it = profiles().find(biomeName);
    return it == profiles().end() ? BiomeLivingContentProfile{} : it->second;
}

std::vector<std::string> BiomeLivingContentCatalog::buildCurrentObservationLines(const std::string& biomeName, int worldDay)
{
    const auto it = profiles().find(biomeName);
    if (it == profiles().end()) return {};
    const BiomeLivingContentProfile& p = it->second;
    const std::vector<std::string> pool = {
        "Identité du lieu : " + p.visualIdentity,
        "Danger de terrain : " + p.environmentalHazard,
        "Ressources visibles : " + p.resourceSign,
        "Vie neutre : " + p.neutralLife,
        "Trace sociale : " + p.socialTrace,
        "Détail inhabituel : " + p.unusualSign
    };
    const std::size_t start = (stableHash(biomeName) + static_cast<std::uint32_t>(worldDay < 0 ? 0 : worldDay)) % pool.size();
    return {pool[start], pool[(start + 2) % pool.size()], pool[(start + 4) % pool.size()]};
}
