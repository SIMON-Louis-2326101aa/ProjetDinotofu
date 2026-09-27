#include "adventure/flavor/ExplorationBiomeFlavor.hpp"

std::string ExplorationBiomeFlavor::miniBossName(const std::string& name, bool evolved)
{
    if (name == "Forêt ancienne") return evolved ? "Loup ancien à mousse noire" : "Gardien de ronces";
    if (name == "Montagne froide") return evolved ? "Yéti aux éclats de givre" : "Briseur de roche gelée";
    if (name == "Marais trouble") return evolved ? "Slime putride couronné" : "Noyeur du marais";
    if (name == "Route commerciale") return evolved ? "Pillard vétéran marqué" : "Chef de bande opportuniste";
    if (name == "Ruines effondrées") return evolved ? "Sentinelle osseuse éveillée" : "Gardien fissuré des ruines";
    if (name == "Bocage aux lanternes") return evolved ? "Roi-lanterne fongique" : "Gardien mycélien";
    if (name == "Désert d'argile rouge") return evolved ? "Colosse d'argile solaire" : "Sentinelle d'argile cuite";
    if (name == "Quartier abandonné") return evolved ? "Propriétaire sans visage" : "Receleur de cave";
    if (name == "Mine sifflante") return evolved ? "Cœur de machine éveillé" : "Foreuse animée";
    if (name == "Cimetière oublié") return evolved ? "Ombre de nom perdu" : "Veilleur sans sépulture";
    if (name == "Temple des cloches fendues") return evolved ? "Sonneur creux du serment" : "Gardien de nef fissuré";
    if (name == "Canaux de brume bleue") return evolved ? "Passeur sans visage" : "Nixe de quai brumeux";
    if (name == "Carrière des os blancs") return evolved ? "Géant enfoui qui respire" : "Golem de craie blanche";
    if (name == "Marché sous les ponts") return evolved ? "Arbitre de dette masqué" : "Collecteur de pont noir";
    if (name == "Jardin des statues qui pleurent") return evolved ? "Muse pétrifiée en larmes" : "Jardinier de marbre";
    if (name == "Sanctuaire kitsuné des Neuf Étincelles") return evolved ? "Renard des neuf reflets" : "Gardien aux trois lanternes";
    if (name == "Confluence du Mana pur") return evolved ? "Nœud vivant de mana" : "Écho de confluence";
    if (name == "Archipel des îles flottantes") return evolved ? "Roc-aile des courants hauts" : "Prédateur des îlots suspendus";
    if (name == "Plaine sauvage") return evolved ? "Alpha aux crocs longs" : "Bête territoriale";
    return evolved ? "Créature évoluée locale" : "Menace locale isolée";
}

std::string ExplorationBiomeFlavor::miniBossQuestFamily(const std::string& name, bool evolved)
{
    if (evolved) return "Mini-boss / menace évoluée";
    if (name == "Route commerciale") return "Humanoïdes / embuscades";
    if (name == "Cimetière oublié") return "Morts-vivants / ombres";
    if (name == "Bocage aux lanternes") return "Plantes lumineuses / bêtes nocturnes";
    if (name == "Désert d'argile rouge") return "Désert / argile / sel lunaire";
    if (name == "Quartier abandonné") return "Humanoïdes urbains / automates";
    if (name == "Mine sifflante") return "Mines / constructions / machines";
    if (name == "Temple des cloches fendues") return "Sanctuaire / gardiens de nef";
    if (name == "Canaux de brume bleue") return "Canaux / brume / passeurs";
    if (name == "Carrière des os blancs") return "Carrière / os blancs / géants";
    if (name == "Marché sous les ponts") return "Humanoïdes / dettes / contrebande";
    if (name == "Jardin des statues qui pleurent") return "Statues / ronces / noblesse abandonnée";
    if (name == "Sanctuaire kitsuné des Neuf Étincelles") return "Kitsune / esprits / illusions";
    if (name == "Confluence du Mana pur") return "Mana / élémentaires / anomalies";
    if (name == "Archipel des îles flottantes") return "Créatures aériennes / drakes";
    if (name == "Forêt ancienne" || name == "Plaine sauvage") return "Créatures locales";
    return "Élite / menace";
}

std::string ExplorationBiomeFlavor::dangerousSiteName(const std::string& name)
{
    if (name == "Forêt ancienne") return "clairière aux racines closes";
    if (name == "Montagne froide") return "faille bleue sous la glace";
    if (name == "Marais trouble") return "mare noire qui respire";
    if (name == "Route commerciale") return "ancien relais barricadé";
    if (name == "Ruines effondrées") return "salle basse aux piliers brisés";
    if (name == "Bocage aux lanternes") return "clairière où les lanternes respirent toutes ensemble";
    if (name == "Désert d'argile rouge") return "oasis sèche entourée de statues fendues";
    if (name == "Quartier abandonné") return "maison scellée avec des clés encore dans la porte";
    if (name == "Mine sifflante") return "ascenseur de mine bloqué qui siffle sans vent";
    if (name == "Cimetière oublié") return "allée de tombes qui ne portent plus de noms";
    if (name == "Archives noyées") return "salle de lecture dont les pages tournent sous l'eau";
    if (name == "Foire abandonnée") return "chapiteau dont les cordes sont encore tendues";
    if (name == "Sanctuaire kitsuné des Neuf Étincelles") return "pavillon aux neuf lanternes désaccordées";
    if (name == "Confluence du Mana pur") return "berge où les trois courants se touchent";
    if (name == "Archipel des îles flottantes") return "îlot qui dérive contre le vent";
    if (name == "Plaine sauvage") return "cercle d'herbes couchées";
    return "lieu dangereux sans nom";
}

std::string ExplorationBiomeFlavor::dangerousSiteWarning(const std::string& name)
{
    if (name == "Forêt ancienne") return "Les arbres se penchent vers le centre comme s'ils voulaient enfermer un souvenir.";
    if (name == "Montagne froide") return "La neige tombe vers le haut pendant quelques secondes.";
    if (name == "Marais trouble") return "La boue forme des bulles régulières, presque comme une respiration.";
    if (name == "Route commerciale") return "Des roues abandonnées grincent alors qu'aucun chariot ne bouge.";
    if (name == "Ruines effondrées") return "Les pierres portent des griffures trop longues pour venir d'un outil.";
    if (name == "Bocage aux lanternes") return "Les champignons s'éteignent un par un, comme si quelqu'un fermait des yeux.";
    if (name == "Désert d'argile rouge") return "Le sable rouge garde des empreintes qui ne sont pas encore passées.";
    if (name == "Quartier abandonné") return "Une fenêtre s'ouvre alors que la maison est censée être vide depuis des années.";
    if (name == "Mine sifflante") return "Un rail vibre doucement, mais aucun wagon ne bouge.";
    if (name == "Cimetière oublié") return "Certaines tombes semblent plus récentes que les dates gravées dessus.";
    if (name == "Archives noyées") return "Une phrase lisible apparaît sur une page immergée puis s'efface lorsque tu approches.";
    if (name == "Foire abandonnée") return "Une clochette de stand répond deux fois à chacun de tes pas.";
    if (name == "Sanctuaire kitsuné des Neuf Étincelles") return "Une dixième lumière apparaît toujours hors de ton champ de vision.";
    if (name == "Confluence du Mana pur") return "Ta voix revient avec un mot que tu n'as pas prononcé.";
    if (name == "Archipel des îles flottantes") return "Les cailloux tombent latéralement vers un autre îlot.";
    if (name == "Plaine sauvage") return "Tous les insectes se taisent au même moment.";
    return "L'air devient lourd et refuse de circuler normalement.";
}

std::string ExplorationBiomeFlavor::bossTrace(const std::string& name)
{
    if (name == "Forêt ancienne") return "une silhouette de bois ancien et de mousse t'observe sans agressivité, mais sans faiblesse";
    if (name == "Montagne froide") return "une masse draconique ou rocheuse fait vibrer la glace sans se montrer entièrement";
    if (name == "Marais trouble") return "quelque chose sous l'eau déplace la surface comme une paupière immense";
    if (name == "Route commerciale") return "une présence compte les pièces, les dettes et les battements de cœur";
    if (name == "Ruines effondrées") return "une ombre ancienne rejoue le même pas entre deux piliers brisés";
    if (name == "Bocage aux lanternes") return "une couronne fongique s'allume au loin, chaque lumière suivant ton souffle";
    if (name == "Désert d'argile rouge") return "une silhouette d'argile immense laisse des traces sèches dans le sel lunaire";
    if (name == "Quartier abandonné") return "quelqu'un compte les portes fermées depuis l'intérieur des maisons vides";
    if (name == "Mine sifflante") return "un cœur mécanique bat quelque part derrière les rails et les clous froids";
    if (name == "Cimetière oublié") return "un nom gravé disparaît lentement d'une pierre pendant que tu le regardes";
    if (name == "Archives noyées") return "une ombre tourne les pages d'un livre que l'eau ne touche pas";
    if (name == "Foire abandonnée") return "une silhouette trop haute passe derrière trois stands sans jamais apparaître entre eux";
    if (name == "Sanctuaire kitsuné des Neuf Étincelles") return "neuf queues de lumière se séparent puis se rejoignent derrière le dernier torii";
    if (name == "Confluence du Mana pur") return "un volume invisible déforme les trois courants comme s'il respirait";
    if (name == "Archipel des îles flottantes") return "une aile immense coupe le soleil puis disparaît sous un îlot";
    if (name == "Plaine sauvage") return "un alpha invisible tourne autour de toi, assez loin pour ne laisser qu'une pression";
    return "une variation d'énergie anormale refuse de porter un nom stable";
}

std::string ExplorationBiomeFlavor::environmentalHazard(const std::string& name)
{
    if (name == "Forêt ancienne") return "des lianes se referment sur un passage couvert de feuilles médicinales";
    if (name == "Montagne froide") return "une corniche gelée cache un filon sous une plaque de neige instable";
    if (name == "Marais trouble") return "une poche de gaz noir remonte sous des plantes utiles";
    if (name == "Route commerciale") return "un ancien chariot renversé grince au bord d'une embuscade possible";
    if (name == "Ruines effondrées") return "un plafond fissuré protège encore un fragment arcanique";
    if (name == "Cimetière oublié") return "une dalle funéraire bouge comme si quelque chose respirait dessous";
    if (name == "Mares gélatineuses") return "une nappe de gelée transparente recouvre des résidus encore propres";
    if (name == "Bocage aux lanternes") return "des spores lumineuses flottent au-dessus d'une poche de résine encore fraîche";
    if (name == "Désert d'argile rouge") return "une plaque d'argile creuse cache des cristaux de sel lunaire";
    if (name == "Quartier abandonné") return "un plancher usé menace de céder sous une cache de vieilles pièces";
    if (name == "Mine sifflante") return "une poutre rouillée retient un petit mécanisme encore récupérable";
    if (name == "Archives noyées") return "un rayonnage penché retient une poche d'air et plusieurs feuillets encore secs";
    if (name == "Foire abandonnée") return "une toile de chapiteau prête à céder couvre une caisse encore fermée";
    if (name == "Sanctuaire kitsuné des Neuf Étincelles") return "un feu follet garde une offrande sans attaquer tant qu'on ne franchit pas sa ligne de lanternes";
    if (name == "Confluence du Mana pur") return "une vague de mana change de polarité autour d'un cristal exploitable";
    if (name == "Archipel des îles flottantes") return "un courant ascendant violent protège un nid rempli de matériaux légers";
    if (name == "Plaine sauvage") return "un terrier frais cache des restes utiles, mais le sol tremble légèrement";
    return "un obstacle naturel bloque une ressource exploitable";
}

std::string ExplorationBiomeFlavor::environmentalObservation(const std::string& name)
{
    if (name == "Cimetière oublié") return "Observation : les morts-vivants du cimetière réagissent souvent aux noms, aux sépultures et aux objets volés aux tombes.";
    if (name == "Mares gélatineuses") return "Observation : les slimes sont plus variés près des eaux stagnantes, surtout quand la gelée paraît trop propre ou trop brillante.";
    if (name == "Route commerciale") return "Observation : les humanoïdes de route protègent souvent les caches, car elles servent de réserve ou de piège.";
    if (name == "Forêt ancienne") return "Observation : la forêt répond aux gestes brusques. Une récolte propre attire moins les prédateurs végétaux.";
    if (name == "Montagne froide") return "Observation : le froid cache autant les filons que les prédateurs. Les traces se lisent mieux près des corniches.";
    if (name == "Marais trouble") return "Observation : le marais annonce souvent le danger par l'odeur avant de le montrer.";
    if (name == "Ruines effondrées") return "Observation : les ruines gardent des sentinelles lentes mais tenaces près des fragments arcaniques.";
    if (name == "Bocage aux lanternes") return "Observation : les lumières du bocage réagissent aux bruits. Une approche calme préserve mieux les lanternes de mycélium.";
    if (name == "Désert d'argile rouge") return "Observation : l'argile rouge protège souvent le sel lunaire, mais les fausses oasis attirent les pilleurs.";
    if (name == "Quartier abandonné") return "Observation : les maisons vides gardent surtout des preuves, contrats, cartes et petits objets oubliés.";
    if (name == "Mine sifflante") return "Observation : la mine répond aux vibrations. Les ressorts et clous rares se trouvent près des machines encore tièdes.";
    if (name == "Archives noyées") return "Observation : certaines encres deviennent lisibles uniquement sous l'eau ; la langue du texte compte autant que son état.";
    if (name == "Foire abandonnée") return "Observation : les mécanismes de foire réagissent encore aux poids, aux sons et parfois à la présence d'une foule qui n'existe plus.";
    if (name == "Sanctuaire kitsuné des Neuf Étincelles") return "Observation : les feux du sanctuaire semblent répondre davantage à l'intention et au respect du lieu qu'à la force brute.";
    if (name == "Confluence du Mana pur") return "Observation : les trois courants n'altèrent pas les mêmes sorts ; observer leur rythme avant d'agir réduit les mauvaises surprises.";
    if (name == "Archipel des îles flottantes") return "Observation : ici les traces sont aussi verticales. Plumes, poussière et débris indiquent souvent quel courant relie deux îlots.";
    return "Observation : ce biome récompense l'étude autant que la prise de risque.";
}
