# Changelog Dinotofu   

Historique détaillé des versions de Dinotofu. Les README restent volontairement centrés sur le jeu, son installation et les informations utiles au joueur.   

## V3.49.92 — Consolidation persistante, contrats et contenu vivant   

- Reconstruction et consolidation de la branche de travail sur une base compilable afin de conserver les ajouts de mémoire persistante sans réintroduire les anciens PATCHNOTE.   

- Les serments d’église sont désormais traités comme de vrais contrats persistants : ils ne consomment plus de place dans les 10 passifs, ne peuvent pas être activés/désactivés depuis le loadout et un serment rompu ne peut pas être réactivé gratuitement.   

- Le journal canonique distingue mieux les événements locaux : une action attachée à une ville ne migre plus vers une autre ville lorsqu’une clé identique est réutilisée. Les catégories narratives importantes restent prioritaires lors du nettoyage du journal.   

- La réputation locale reste centralisée dans `LocalReputationSystem` afin que villes et boutiques lisent la même réputation positive ou négative.   

- Les rivaux possèdent une identité individuelle persistante, un historique de fuites/retours/blessures et peuvent revenir comme le même adversaire. Une mort enregistrée met fin à leurs retours vivants.   

- Les statistiques proposent une vue `Mémoire du monde / Rivaux` qui affiche uniquement les faits déjà vécus ou enregistrés, sans prédire les intentions ou prochaines actions des entités.   

- Les armes et armures disposent d’identifiants persistants et peuvent accumuler des souvenirs liés à l’exemplaire exact, notamment via la Forge liée, certaines attaques marquantes et des réparations importantes.   

- Ajout d’une vraie action de technique combinée pour deux recrues : elle exige deux alliés disponibles, consomme leurs deux tours, met leurs techniques en récupération et choisit un effet selon les profils du duo.   

- Le mode histoire normal est volontairement limité après l’introduction du chapitre 3. Les anciennes scènes ultérieures restent dans le code pour refonte future mais ne sont plus injectées dans une progression normale.   

- Ajout d’un `make test` léger couvrant version, CHANGELOG, absence de PATCHNOTE et `.inc`, mémoire/rivaux, identité d’équipement, combos alliés, serments-contracts, réputation locale et limite histoire.   

- Nettoyage des warnings préparatoires afin de viser une compilation `-Wall -Wextra` sans warning.   

---   

## V3.49.91 — Mémoire persistante, vrais rivaux et identité des équipements   

- Séparation du journal canonique agrégé et des événements historiques persistants : les compteurs servent aux statistiques tandis que les faits narratifs importants disposent désormais d’événements individuels sauvegardés avec identifiant, sujet, lieu, jour et état résolu/non résolu.   

- Migration automatique des anciennes traces narratives importantes vers la nouvelle mémoire persistante lors du chargement des sauvegardes antérieures compatibles.   

- Ajout de vrais rivaux ennemis persistants : un ennemi ayant réellement survécu peut recevoir une identité individuelle, conserver son origine, son niveau, ses statistiques, ses blessures, ses fuites, ses retours, sa dernière localisation et son état vivant/mort.   

- Un rival qui revient est le même individu, identifié par le moteur et sauvegardé ; s’il est vaincu, il est marqué mort définitivement et ne peut plus être réinjecté comme rival vivant.   

- Ajout d’une inspection « Mémoire du monde / Rivaux » dans les statistiques afin de consulter uniquement les faits déjà vécus ou connus, sans révéler les intentions futures des rivaux.   

- Ajout d’une identité persistante propre à chaque exemplaire d’arme et d’armure lors de son entrée dans un inventaire ; cette identité suit l’objet pendant les copies, transferts, équipements et sauvegardes pris en charge.   

- Le Serment de la Forge liée et les compétences signatures ennemies peuvent désormais attacher une mémoire à l’exemplaire exact d’une arme ou d’une armure au lieu de confondre tous les objets portant le même nom.   

- L’inspection d’une arme ou d’une armure affiche ses souvenirs individuels connus ; une réparation après forte usure, une réparation d’un objet déjà marqué ou une réparation sous Serment de la Forge liée peut devenir une nouvelle trace historique de cet exemplaire.   

- Ajout de la première vraie mécanique de techniques combinées des recrues : une consigne explicite engage deux alliés disponibles dans une action commune et consomme leurs deux tours au lieu d’ajouter une attaque gratuite.   

- Les combinaisons varient déjà selon les profils du duo : Brèche sous garde, Faille relayée, Feu croisé, Croisement de lignes ou Assaut synchronisé ; le Serment des Liens facilite légèrement la coordination des duos moins expérimentés sans garantir le résultat gratuitement.   

- Les techniques combinées réellement exécutées sont enregistrées dans la mémoire persistante avec l’identité des deux recrues, afin de préparer leur évolution future selon l’expérience commune.   

- Abandon des fichiers `.inc` temporaires : aucun `.inc` ne doit rester dans `src/` ou `include/`. La réduction de `QuestMenu.cpp` doit se poursuivre uniquement avec de vrais modules `.cpp/.hpp`.   

- Le mode histoire possède maintenant une limite de développement centralisée : la progression normale s’arrête volontairement à l’introduction du chapitre 3 avec un message de fin temporaire. Les chapitres et quêtes déjà écrits après ce point restent dans le code pour leur future réécriture, mais ne sont plus injectés dans une nouvelle progression normale.   

- Renforcement de `make test` : contrôle des modules QuestMenu, de l’absence de `.inc`, de la mémoire persistante, des rivaux, de l’identité des équipements, des techniques combinées, de la limite histoire et de la cohérence de version.   

---   

## V3.49.90 — Consolidation technique et documentation   

- Migration de l’historique détaillé des versions depuis les README vers ce CHANGELOG unique.   

- Remplacement des anciens fichiers PATCHNOTE par ce CHANGELOG comme source d’historique de version.   

- Séparation progressive du très volumineux `QuestMenu.cpp` en fichiers de détail thématiques sans changer le comportement du jeu.   

- Consolidation des serments d’église afin qu’ils soient traités comme des contrats persistants et non comme de simples passifs activables ou désactivables.   

- Première consolidation du journal canonique : les traces historiques importantes sont protégées du nettoyage automatique et certaines statistiques locales restent attachées à leur ville réelle.   

- Centralisation du calcul de réputation locale dans `CityTravelRules`, utilisé maintenant par les règles de ville et les boutiques afin d’éviter deux scores concurrents.   

- Nettoyage de plusieurs warnings de compilation liés à des helpers préparatoires ou variables inutilisées, sans modifier le gameplay.   

- Ajout d’une cible `make test` légère pour vérifier les invariants essentiels du projet sans créer une infrastructure de tests lourde.   

---   

## Historique français   

## V3.49.89 — Rivaux et destin instable   

- La V3.49.89 ajoute les serments des Rivaux et du Destin instable. Les fuites, paniques et compétences signatures peuvent laisser des traces plus nettes de rival potentiel, tandis que le destin instable crée de petites oscillations seulement lorsqu'une trace existe déjà : mémoire, rupture, rival ou objet marqué.   


## V3.30.00 — routes et poids d’équipement   

- Les villes disposent désormais de distances entre elles, de distances vers les biomes et de conditions d’accès par niveau ou boss vaincu.   

- Les coffres restent indépendants par ville : le voyage change la ville active, mais ne mélange pas les contenus.   

- La carte d’exploration textuelle prépare les futurs fonds pixel-art par biome, avec lieux inconnus grisés ou enfumés.   

- Les armes et armures ont maintenant un poids léger, moyen ou lourd. Les bonus et contreparties sont appliqués au combat, aux dégâts et à la fuite, avec des malus volontairement modérés.   

## V3.31.00 — hubs de ville et journal canonique   

- La V3.31.00 rend la ville actuelle plus structurée : bâtiments locaux, contacts, verrous et indices pixel-art viennent maintenant des règles du monde plutôt que d'une liste fixe. Les destinations de ville fournissent des métadonnées IG structurées pour l'accès, la distance, le temps de trajet et le coût futur. Un journal canonique sauvegardé enregistre les événements importants comme les lieux visités, routes prises et mouvements de coffre, afin que les futurs Top 3 utilisent des données moteur au lieu de texte deviné.   

## V3.49.86 — Maîtrise tactique liée au build   

- La V3.49.86 relie davantage les maîtrises actives à la cohérence de l’équipement. Une arme ou armure cohérente avec la classe soutient légèrement les gestes liés, tandis qu’un [malus de classe] peut rendre une action moins propre malgré la maîtrise. L’audit de classe affiche aussi survie, critique et équipement attendu, et les passifs d’observation réduisent un peu les mauvaises surprises face aux compétences signatures ennemies.   

## V3.49.36 — Visée des créatures et textes plus immersifs   

- La V3.49.36 ajoute une lecture de visée aux profils ennemis : les petites créatures comme rats, fées, chauves-souris, insectes et profils sournois sont plus difficiles à cadrer, tandis que les brutes, constructions, dragons, plantes enracinées et gardiens ouvrent parfois des fenêtres plus faciles à toucher. L'observation et le bestiaire indiquent maintenant cette lecture de visée. Plusieurs textes en combat ont aussi été reformulés pour rester dans l'univers du jeu au lieu de parler comme une note de mise à jour.   

## V3.49.36 — Variantes ennemies et affinité féerique   

- La V3.49.36 étend les profils comportementaux avec des variantes plus précises : chauve-souris, rat/nuisible, chargeur massif, araignée, kobold, archer, alchimiste, fée, spectre, construction, dragon et serment sacré. Les profils ont maintenant une attaque signature, des réactions plus propres et une ligne de contre-jeu. Les fées gagnent aussi une vraie règle : 50% de résistance magique, mais 50% de faiblesse physique. Les parchemins offensifs comptent comme dégâts magiques pour que cette faiblesse/résistance soit réellement visible.   

## V3.49.34 — Profils de monstres et attaques signature   

- La V3.49.34 ajoute une première couche centrale de profils comportementaux pour les ennemis. Les slimes, voleurs, gobelins, brutes, prédateurs, gardiens, plantes, insectoïdes, supports et entités instables gagnent des descriptions d'attaque, forces, failles et réactions plus propres. L'observation active et le bestiaire affichent maintenant ces profils pour aider le joueur à comprendre pourquoi un ennemi agit différemment d'un autre.   

## V3.49.33 — Formations ennemies et rupture de formation   

- La V3.49.33 ajoute une couche de combat autour des tours de formation ennemie. Certaines vagues coordonnées peuvent maintenant utiliser couverture, précision courte, petite garde ou posture défensive au lieu d'attaquer basiquement. Le joueur reçoit la réponse **Casser la formation**, une action tactique qui perturbe plusieurs ennemis, peut retirer des postures défensives, et peut débloquer **Briseur de formation** après plusieurs vrais usages.   

## V3.49.31 — Brise-garde et posture de soutien   

- La V3.49.31 avait ajouté deux actions tactiques : **Brise-garde**, une action de contrôle courte avec affaiblissement/vulnérabilité, et **Tenir la ligne / couvrir**, une posture de soutien avec provocation courte, garde élémentaire et précision. Ces actions commençaient la progression passive vers **Casseur de garde** et **Rythme de soutien**.   

## V3.49.30 — Rumeur Hero Villager retardée et pression ennemie   

- La V3.49.30 corrige la rumeur Hero Villager trop précoce : la guilde ne parle plus de lui directement après l'inscription au jour 0. La rumeur et la rencontre rare sur route demandent maintenant une vraie progression : jours passés, niveau, contrats, histoire, actions tactiques ou observation. Les ennemis gagnent aussi une petite pression non basique : profils intelligents capables de feinte, créatures opportunistes qui exploitent les ouvertures visibles, et pression bestiale pouvant créer une vulnérabilité au lieu de toujours faire une attaque simple.   

## V3.49.29 — Préparation d'arme, lecture voleur et coffres reliés   

- La V3.49.29 ajoute l'action tactique **Enduire / fusionner vite l'arme**, qui consomme un composant pour appliquer un effet temporaire sur une cible : poison, choc, givre, vulnérabilité, précision ou puissance selon le matériau. Les dagues gagnent une synergie spéciale proche de l'idée de dague empoisonnée. L'observation active peut aussi apprendre **Lecture des serrures et failles** aux profils discrets, puis cette lecture aide réellement sur les coffres suspects d'exploration.   

## V3.49.28 — Ouvertures tactiques et artisanat de combat   

- La V3.49.28 enrichit le menu **Actions tactiques** avec **Exploiter une ouverture** et **Piège improvisé d'artisan**. Les blessures et statuts peuvent maintenant créer des réactions tactiques utiles, et certains petits matériaux deviennent consommables en combat pour gêner la ligne ennemie. Les actions tactiques font aussi progresser des passifs comme **Lecture du terrain** et **Improvisateur de combat**.   

## V3.49.27 — Audit retours, compagnon Dinotofu et combos de statuts   

- La V3.49.27 ajoute un premier **Compagnon Dinotofu** accessible depuis les activités et le menu hors combat. Il donne des conseils courts selon les PV, les quêtes, les lanternes, les compétences et le journal beta local. Les techniques d'arme profitent aussi de réactions de statuts, par exemple brûlure + givre, poison + saignement, choc + vulnérabilité, ou affaiblissement + vulnérabilité. Cette passe corrige aussi la cohérence des fichiers de version après la V3.49.26.   

## V3.49.26 — Actions tactiques, panneau de guilde et catégories rapides   

- La V3.49.26 ajoute le menu **Actions tactiques** en combat de vague : lancer une lanterne, jeter une lanterne au sol, repousser, utiliser la poussière ou observer activement. Le panneau de guilde utilise un libellé d'offre séparé du journal actif, affiche le rang et clarifie le cas même lieu. Le menu **Boutiques et comptoirs** gagne aussi plusieurs catégories rapides utiles.   

## V3.49.25 — Consignes alliées plus tactiques   

- La V3.49.25 ajoute des consignes de combat plus utiles pour les recrues : forcer une technique prête sur une recrue précise, demander une percée coordonnée de groupe, ou répartir les cibles pendant 1 tour contre plusieurs ennemis. Ces consignes ne consomment pas le tour du joueur, mais elles expirent après le tour allié, sauf la priorité de cible qui reste jusqu’à mort/disparition de la cible.   

## V3.49.25 — Variété combat et journal bêta   

- La V3.49.25 ajoute Rupture de ligne, Suture de fortune et Signal de focus, stabilise légèrement les attaques normales avec une énorme plage de dégâts, et ajoute un accès Journal bêta dans l’après-combat pour retrouver facilement `logs/dinotofu_session_latest.txt`.   

## V3.49.25 — Quêtes personnelles de recrues et contributions alliées plus riches   

- La V3.49.25 ajoute une première boucle de quêtes personnelles pour les recrues. Une recrue peut maintenant avoir un problème personnel lié à son profil, la progression peut se faire sur plusieurs tentatives, et la réussite améliore légèrement la loyauté tout en ajoutant de la réputation de clan. Les contributions alliées deviennent aussi plus lisibles : actions de soutien et coups de finition sont suivis, avec des techniques plus distinctes pour gardiens/roublards.   

## V3.49.19 — Dettes d’infirmerie bloquantes   

- La V3.49.19 rend les dettes d’infirmerie réellement contraignantes : tant qu’une dette existe, les soins payants sont bloqués. Si la dette totale dépasse 100 or, la récupération/réanimation d’une recrue prête à sortir est aussi bloquée jusqu’au remboursement partiel.   

## V3.49.17 — Infirmerie complète et auberge moins cheatée   

- La V3.49.17 ajoute un vrai service d’infirmerie : se soigner, soigner un membre d’équipe, ou gérer l’entrée/sortie des recrues KO. Les soins payants montent jusqu’à 90% PV maximum. L’auberge est rééquilibrée : les lits communs/simples plafonnent à 50% PV, tandis que la chambre sûre plus chère peut monter jusqu’à 90% sans full heal gratuit.   

## V3.49.16 — Recrues persistantes et évacuation infirmerie   

- La V3.49.16 continue d’améliorer les combats : les recrues ont maintenant de vrais PV persistants sauvegardés, plus des PV temporaires de combat. Elles peuvent commencer entre 70% et 100% de PV, garder leurs PV/potions entre les combats, et si une recrue tombe à 0 PV elle doit être amenée à l’infirmerie au lieu d’y être envoyée automatiquement. Si le joueur tombe aussi, plusieurs jours peuvent passer selon la gravité, et les recrues KO suivent le transfert vers les soins.   

## V3.49.11 — Recrues en combat PvE   

- Les recrues équipées commencent à agir dans les vrais combats PvE standard : soutien, dégâts simples, soin contextuel et partage de récompenses. Le joueur reste premier et conserve toujours la plus grosse part ; l’ordre manuel d’équipe reste prioritaire sur le tri automatique.   

## V3.49.70 — Marque mortelle collective de FireFlight   

- La V3.49.70 transforme la marque mortelle de FireFlight : elle ne vise plus une seule cible, mais tout le camp adverse en même temps. Pendant 2 tours de boss, chaque adversaire marqué qui tombe subit une vraie mort définitive, avec des lignes RP indiquant que FireFlight joue avec la panique du groupe entier.   

## V3.49.69 — Marque mortelle FireFlight et hésitation des recrues   

- La V3.49.69 transforme la fenêtre de panique de FireFlight en vraie **marque mortelle** ciblée sur le joueur. Pendant 2 tours de boss, tomber sous cette marque est traité comme une mort définitive et non comme une défaite non létale classique. La passe ajoute aussi une petite règle d’hésitation pour les recrues bas rang : sans ordre clair, elles peuvent renoncer à une technique trop avancée et revenir à une action plus simple.   

## V3.49.68 — Paliers variables et FireFlight mortel   

- La V3.49.68 corrige l’idée de plafond fixe : 10 niveaux actifs et 5 niveaux passifs restent seulement les maximums absolus. Les compétences simples peuvent avoir moins de paliers, et l’affichage montre maintenant le plafond local. FireFlight gagne aussi une fenêtre rare de 2 tours où les règles de mortel peuvent s’imposer temporairement, pour ajouter panique et stress sans changer la difficulté sauvegardée.   

## V3.49.67 — Progression actifs/passifs plus lisible   

- La V3.49.67 améliore les retours de progression des compétences. La maîtrise active affiche maintenant le prochain palier d’usage et le type de petit effet gagné, tandis que les passifs de maîtrise montrent leur progression en 5 niveaux et leur effet léger. Le menu des actions tactiques rappelle aussi que les paliers restent équilibrés : surtout fiabilité, précision, souffle, rythme ou contrôle léger, pas de montée cheat trop tôt. Les menus de consignes alliées rappellent aussi qu’une recrue faible rang comprend mieux un ordre clair qu’une intention de groupe trop vague.   

## V3.49.66 — Maturité de combat des recrues   

- La V3.49.66 ajoute une couche de maturité de combat aux recrues. Une recrue faible rang lit moins souvent le terrain, choisit des cibles plus simples, utilise rarement ses techniques avancées et profite davantage d’un ordre clair que de son instinct. Le rang, le niveau et l’équipement améliorent progressivement les réactions, commentaires tactiques, choix de cible et fiabilité des techniques pour mieux sentir son évolution.   

## V3.49.65 — Lectures vivantes ennemies et alliées   

- La V3.49.65 ajoute des lectures de combat plus contextuelles sans changer la structure des menus. Les lignes ennemies réagissent mieux à l’archétype, aux états visibles et au danger côté cible, tandis que les recrues peuvent commenter le premier profil ennemi, le danger sur le joueur, la pression de groupe et leur propre rôle ou race. **Monde / ville** gagne aussi de courtes lignes d’ambiance et de rumeur selon le moment, l’état du joueur et le contexte de quête.   

## V3.49.62 — Routage clair vers le menu de base   

- La V3.49.62 clarifie le flux après combat. **Continuer** signifie maintenant revenir au **Menu de base**. **Monde / ville** reste une activité normale du menu de base pour les lieux explorables, boutiques, guilde, PNJ et services, tandis que le menu rapide reste un hub constant pour personnage/session/sauvegarde.   

## V3.49.61 — Refonte du menu rapide et rangement personnage/monde   

- La V3.49.61 range les accès hors combat autour d’un menu rapide plus lisible. **Personnage** regroupe inventaire, titres, compétences actifs/passifs, quêtes acceptées, statistiques, équipement rapide, équipe et échange. Correction V3.49.61 : **Monde / ville** redevient une activité de lieux explorables accessible depuis les activités, pas depuis le menu rapide. L’après-combat renvoie vers le menu rapide pour le personnage/options/sauvegarde, ou vers Continuer pour retourner aux activités et visiter la ville.   

## V3.49.59 — Menu de charge et audit actifs/passifs   

- La V3.49.59 ajoute le menu hors combat de gestion des compétences dans le hub statistiques. Les actifs peuvent être équipés ou déséquipés, les passifs peuvent être activés ou désactivés, et les statistiques ouvertes pendant un combat restent seulement consultatives pour éviter de changer de build au milieu d’un tour. La passe vérifie aussi les techniques tactiques récentes pour que les identifiants d’actifs ne soient plus traités comme des effets passifs.   

## V3.49.58 — Charge de compétences et maîtrise progressive   

- La V3.49.58 ajoute une vraie base pour différencier les compétences connues, équipées et activées. Un personnage peut connaître plus de compétences, mais seules 10 compétences actives peuvent être équipées et seuls 10 passifs peuvent être activés en même temps. Les actifs restent des actions choisies, tandis que les passifs restent des effets automatiques ou semi-automatiques pouvant aussi servir hors combat selon leur nature.   

- Les actifs gagnent maintenant une maîtrise sur 10 paliers maximum, avec des seuils espacés pour ne pas devenir trop forts trop tôt dans une progression prévue jusqu'au niveau 255. Les passifs issus d'une pratique répétée doivent d'abord passer par trois essais réussis visibles avant de se débloquer réellement.   

## V3.49.57 — Séparation actif / maîtrise passive   

- La V3.49.57 clarifie la différence entre technique de combat sélectionnable et maîtrise passive. Les techniques d'affinité restent des actions actives du menu tactique, tandis que les vrais usages répétés débloquent désormais des maîtrises passives avec un nom séparé : **Maîtrise élémentaire**, **Garde circulaire**, **Trait entravant**, **Voix revigorante**, **Instinct canalisé** et **Rythme de lame**. Les anciens identifiants de sauvegarde restent reconnus, mais l'affichage ne donne plus l'impression que **Danse de lame** ou les autres techniques deviennent automatiques.   

## V3.49.56 — Techniques variées d'affinité   

- La V3.49.56 ajoute six techniques d’affinité supplémentaires pour éviter que les combats ne tournent seulement autour des ruptures/débuffs : **Lame élémentaire**, **Cercle protecteur**, **Flèche entravante**, **Chant revigorant**, **Instinct de bête** et **Danse de lame**. Les profils élémentaires, protecteurs, pisteurs, bardes/chefs, sauvages et duellistes gagnent chacun une option dédiée avec passif après vrais usages.   

## V3.49.55 — Affinités de classe élargies   

- La V3.49.55 ajoute cinq techniques d’affinité supplémentaires : **Rage maîtrisée**, **Ordre de bataille**, **Totem de souffle**, **Bombe d’atelier** et **Prière d’acier**. Cette passe élargit les classes spécialisées sans rendre toutes les actions universelles : front, commandement, nature, atelier et sacré gagnent chacun une option avec passif après vrais usages.   

## V3.49.53 — Techniques d’affinité de classe   

- La V3.49.53 ajoute une première couche de **techniques d’affinité de classe** : **Pas de l’ombre** n’est plus universel, et les profils sournois, soutien, arcanique, rempart et pisteur gagnent chacun une technique dédiée avec progression après vrais usages.   

## V3.49.50 — Allonge, remparts et ancrages   

- La V3.49.50 ajoute trois actions tactiques : **Rompre l’allonge**, **Percer le rempart** et **Rompre l’ancrage occulte**. Elle ajoute aussi les profils **Allonge prudente**, **Ancre nécrotique** et **Drain de vie**, avec réactions de distance, froid de tombe, récupération courte et contre-jeu par rupture d’espace ou d’ancrage.   

## V3.49.49 — Chaînes d’états, proies marquées et profils ennemis   

- La V3.49.49 ajoute trois actions tactiques : **Forcer une chaîne d’états**, **Marquer une proie** et **Retrait contrôlé**. Elle ajoute aussi les profils **Soigneur de fortune**, **Porte-bouclier**, **Berserker blessé** et **Porte-malédiction**, avec observations, faiblesses, attaques signatures et réactions mécaniques propres.   

## V3.49.38 — Taille, matière et résistance physique   

- La V3.49.38 ajoute une vraie lecture de résistance liée au corps des créatures : petit fragile, petit protégé, grande masse organique, grande masse dure, matière mauvaise, slime, spectre ou fée ne se comportent plus pareil face aux dégâts physiques. L'observation et le bestiaire affichent cette lecture, et le rapport de dégâts montre une phrase immersive quand le corps modifie l'impact. Les techniques de recrues gagnent aussi des gestes plus liés à leur profil.   

## V3.49.74 — Maîtrises élargies et préparation de sortie   

- La V3.49.74 continue les six chantiers actifs en même temps : plusieurs anciennes techniques d’affinité reçoivent de vrais effets de maîtrise, les passifs tactiques restent des soutiens et non des actions automatiques, la pression ennemie gagne des intentions contextuelles, les recrues utilisent davantage leurs techniques selon leur maturité, et Monde / ville gagne une lecture de préparation de sortie pour soin, outils, observation, coffres, artisanat combat et renforts mercenaires.   

## V3.49.73 — Passifs de maîtrise actifs et ville plus lisible   

- La V3.49.73 branche davantage les passifs de maîtrise dans les effets réels des actions tactiques : un passif débloqué et activé peut maintenant soutenir légèrement la puissance, la chance secondaire, la durée ou le rythme, sans jamais lancer l’actif à la place du joueur. Monde / ville affiche aussi plus de repères locaux : ville actuelle, inscription de guilde et temps de voyage.   

## V3.49.81 — Synergies lisibles et impact de maîtrise   

- La V3.49.81 ajoute une lecture globale du build dans le menu Personnage et la préparation de sortie : arme, armure, bonus/malus de classe et cohérence générale. Les maîtrises actives/passives utilisent des libellés plus clairs sur l’impact réel, les techniques de recrues profitent davantage de la maturité et des ordres, et les compétences signatures ennemies deviennent un peu plus présentes quand l’ennemi est entraîné, élite ou face à un joueur déjà ouvert.   

---   

## English history   

## V3.49.89 — Rivals and unstable fate   

- V3.49.89 adds the Rival and Unstable Fate church oaths. Escapes, panic reactions and signature skills can leave clearer rival traces, while unstable fate creates small oscillations only when an actual trace already exists: memory, broken oath, rival or marked item.   


## V3.30.00 — note — routes and equipment weight   

- Cities now have distances between each other, distances toward biomes, and access requirements based on level or defeated bosses.   

- Vaults remain independent per city: travel changes the active town but does not merge contents.   

- The textual exploration map prepares future pixel-art backgrounds per biome, with unknown places shown as grey or foggy.   

- Weapons and armor now have light, medium, or heavy weight classes. Bonuses and tradeoffs are applied to combat, damage, and escape, with intentionally moderate penalties.   

## V3.31.00 — note — city hubs and canonical journal   

- V3.31.00 turns the current city into a more structured hub: local buildings, contacts, lock states and future pixel-art hints now come from world rules instead of one fixed text list. City destinations expose structured GUI metadata for access, distance, travel time and future route costs. A saved canonical journal now records key world events such as places visited, routes taken and vault movements so later Top 3 screens can rely on engine data rather than parsed interface text.   

## V3.49.86 — Loadout-aware tactical mastery   

- V3.49.86 ties active mastery more directly to class/loadout coherence. A coherent weapon or armor can lightly support linked tactical actions, while a class malus can make the gesture less clean even with mastery. Class audit now shows survival, crit identity and expected equipment, and observation-style passives reduce surprise against enemy signature skills.   

## V3.49.36 — Creature aiming and more immersive combat text   

- V3.49.36 adds an aiming read to enemy profiles: small creatures such as rats, fairies, bats, insects and sneaky profiles are harder to frame, while brutes, constructs, dragons, rooted plants and guardians sometimes leave easier windows to hit. Observation and bestiary entries now expose this aiming read. Several combat text lines were also reworded to stay in the game world instead of sounding like patch notes.   

## V3.49.36 — Enemy variants and fairy affinity   

- V3.49.36 expands behavior profiles with more precise variants: bat, rat/pest, massive charger, spider, kobold, archer, alchemist, fairy, specter, construct, dragon and sacred oath. Profiles now have a signature attack, cleaner reactions and a counterplay line. Fairies also gain a real rule: 50% magical resistance, but 50% physical weakness. Offensive scrolls count as magical damage so this weakness/resistance is actually visible.   

## V3.49.34 — Monster profiles and signature attacks   

- V3.49.34 adds a first central behavior profile layer for enemies. Slimes, thieves, goblins, brutes, predators, guardians, plants, insectoids, supports and unstable entities gain more specific attack descriptions, strengths, weaknesses and reactions. Active observation and the bestiary now display those profiles to help the player understand why one enemy acts differently from another.   

## V3.49.33 — Enemy formations and formation break   

- V3.49.33 adds a new combat layer around enemy formation turns. Some coordinated waves can now spend a turn on coverage, short precision, light warding or defensive posture instead of always making a basic attack. The player gets the answer **Break formation**, a tactical action that perturbs several enemies, can remove defensive postures, and can unlock **Formation breaker** after repeated real use.   

## V3.49.31 — Guard break and support posture   

- V3.49.31 added two more tactical actions: **Guard Break**, a short control action with weakening/vulnerability, and **Hold the line / cover**, a support posture with short provocation, elemental guard and precision. These actions started passive progress toward **Guard Breaker** and **Support Rhythm**.   

## V3.49.30 — Delayed Hero Villager rumor and enemy pressure   

- V3.49.30 fixes the early Hero Villager rumor: the guild no longer mentions him immediately after registration on day 0. The rumor and the rare road encounter now require real progression first: enough days, level, contracts, story progress, tactical actions or observation. Enemies also gain a small non-basic pressure system: intelligent profiles can feint, wounded or opportunistic creatures can exploit visible openings, and bestial enemies can sometimes create vulnerability instead of always using a plain attack.   

## V3.49.29 — Weapon preparation, rogue reading and linked chests   

- V3.49.29 adds the **Coat / quick-fuse the weapon** tactical action, consuming a component to apply a temporary effect on a target: poison, shock, frost, vulnerability, precision or power depending on the material. Daggers receive a special synergy close to the poisoned dagger idea. Active observation can also teach **Lock and Fault Reading** to discreet profiles, then that reading actually helps with suspicious exploration chests.   

## V3.49.28 — Tactical openings and combat crafting   

- V3.49.28 expands the **Tactical Actions** menu with **Exploit an opening** and **Improvised artisan trap**. Wounds and statuses can now create useful tactical reactions, and some small materials can be consumed in combat to disrupt the enemy line. Tactical actions also progress passive skills such as **Terrain Reading** and **Combat Improviser**.   

## V3.49.27 — Feedback audit, Dinotofu companion and status combos   

- V3.49.27 adds a first **Dinotofu Companion** available from the activity menu and the out-of-combat menu. It gives short advice based on HP, quests, lanterns, skills and the local beta log. Weapon techniques also benefit from status reactions such as burning + frost, poison + bleeding, shock + vulnerability, or weakening + vulnerability. This pass also fixes version-file consistency after V3.49.26.   

## V3.49.26 — Tactical actions, guild board and quick categories   

- V3.49.26 adds the **Tactical Actions** menu in wave combat: throw a lantern, throw a lantern on the ground, push back, use dust or actively observe. The guild board now uses a separate offer label instead of active-quest wording, displays rank and clarifies same-location offers. The **Boutiques et comptoirs** menu also gains several useful quick categories.   

## V3.49.25 — More tactical ally directives   

- V3.49.25 adds more useful combat directives for recruits: force a ready technique on one recruit, request a coordinated group breakthrough, or spread targets for 1 turn against multiple enemies. These directives do not consume the player turn, but they expire after the ally turn, except focus priority which remains until the target dies or disappears.   

## V3.49.25 — Combat variety and beta log access   

- V3.49.25 adds Rupture de ligne, Suture de fortune and Signal de focus, slightly stabilizes normal attacks with very wide damage ranges, and adds a Beta log entry in the post-combat menu to find `logs/dinotofu_session_latest.txt` quickly.   

## V3.49.25 — Recruit personal quests and richer ally contributions   

- V3.49.25 adds a first personal-quest loop for recruits. A recruit can now ask for a profile-flavored personal issue to be solved, progress can be recorded across attempts, and completion improves loyalty slightly while adding clan reputation. Ally combat contribution also becomes more readable: support actions and finish blows are tracked, and guard/rogue-style recruits gain more distinct active techniques.   

## V3.49.19 — Blocking infirmary debts   

- V3.49.19 makes infirmary debts actually restrictive: as long as any debt remains, paid healing is blocked. If total debt exceeds 100 gold, recovering/reviving a recruit who is ready to leave is also blocked until the debt is reduced.   

## V3.49.17 — Full infirmary service and less overpowered inn healing   

- V3.49.17 adds a real infirmary service: heal yourself, heal a team member, or manage admission/recovery for KO recruits. Paid care restores up to 90% HP. Inn healing is rebalanced: common/simple beds cap at 50% HP, while the safer expensive room can reach 90% without giving a free full heal.   

## V3.49.16 — Persistent recruits and infirmary evacuation   

- V3.49.16 keeps improving combat feedback: recruited allies now have persistent saved HP instead of temporary combat HP. They can start below full health, keep their HP/potions between fights, and if one falls to 0 HP they must be evacuated to the infirmary rather than disappearing there automatically. If the player also falls, several days can pass depending on severity, and KO recruits are transferred to treatment too.   

## V3.49.11 — Recruited allies in PvE combat   

- Equipped recruits now start acting in standard PvE combat with simple support, damage, contextual healing and reward sharing. The player remains first and keeps the biggest share; manual team order stays above the automatic fallback.   

## V3.49.70 — FireFlight group mortal mark   

- V3.49.70 changes FireFlight's mortal mark from a single-target panic test into a group-wide mark over all opponents facing him. For 2 boss turns, any marked opponent who falls is treated as a real permanent death, with RP pressure lines making it clear that FireFlight is playing with the whole group rather than only one target.   

## V3.49.69 — FireFlight mortal mark and recruit hesitation   

- V3.49.69 turns FireFlight's panic window into a targeted **mortal mark** placed on the player. For 2 boss turns, falling under this mark is treated as permanent death rather than a normal non-lethal defeat. The pass also adds a small low-rank recruit hesitation rule: inexperienced allies may back out of an advanced technique without a clear order and fall back to a simpler action.   

## V3.49.68 — Variable mastery caps and FireFlight mortal window   

- V3.49.68 stops treating 10 active levels and 5 passive levels as mandatory caps for every skill. They are now absolute maximums only, while simpler skills can end earlier and display their local cap. FireFlight also gains a rare 2-turn mortal-rules window for extra panic without changing the saved difficulty.   

## V3.49.67 — Active/passive progression readability   

- V3.49.67 improves skill progression feedback. Active tactical mastery now explains both the next use threshold and the kind of small effect gained, while mastery passives show their own 5-level progress and light effect hint. Tactical actions also clarify that mastery stays balanced: mostly reliability, precision, breath, small rhythm or control rather than early overpowered scaling. Ally order menus now remind the player that low-rank recruits understand clear orders better than vague group intent.   

## V3.49.66 — Recruited ally combat maturity   

- V3.49.66 adds a combat maturity layer to recruited allies. Low-rank recruits now read the field less often, choose simpler targets, use advanced techniques more rarely and follow clear orders better than vague instinct. Higher rank, level and equipment gradually improve reactions, tactical comments, targeting and technique reliability so ally growth is more visible.   

## V3.49.65 — Living enemy and ally reads   

- V3.49.65 adds more contextual combat reading without changing the menu structure. Enemy behavior lines now react more clearly to archetype, visible statuses and target danger, while recruited allies can comment on the first enemy profile, player danger, group pressure and their own role or race. **World / city** also gains short ambience and rumor lines based on time of day, health and quest context.   

## V3.49.62 — Clear base menu routing after combat   

- V3.49.62 clarifies the menu flow after combat. **Continue** now means returning to the **Base menu**. **World / city** remains a normal base-menu activity for explorable places, shops, guild, NPCs and services, while the quick menu remains a constant character/session/save hub.   

## V3.49.61 — Quick menu correction and explorable city routing   

- V3.49.61 corrects the menu refactor: **Character** stays in the quick menu and groups inventory, titles, active/passive skills, accepted quests, statistics, quick equipment, team and exchange. **World / city** is no longer part of the quick menu because it represents explorable places; it stays available from the activity selection. Post-combat now sends the player to **Continue** for city visits, or to the quick menu for character/options/save actions.   

## V3.49.59 — Skill loadout menu and active/passive audit   

- V3.49.59 adds the out-of-combat skill loadout menu in the statistics hub. Active skills can be equipped or unequipped, passive skills can be enabled or disabled, and combat-opened statistics stay consult-only to avoid changing a build during a turn. The pass also audits recent tactical actions so active identifiers are no longer treated as passive effects.   

## V3.49.58 — Skill loadout and progressive mastery   

- V3.49.58 adds the base distinction between known, equipped and enabled skills. A character can know more skills, but only 10 active skills can be equipped and only 10 passives can be enabled at the same time. Active skills remain chosen actions, while passives remain automatic or semi-automatic effects that may also work outside combat depending on their nature.   

- Active skills now gain up to 10 mastery tiers, with spaced thresholds so they do not become too strong too early in a progression designed to reach level 255. Repeated-practice passives must first pass three visible successful trials before becoming real passives.   

## V3.49.57 — Active/passive mastery split   

- V3.49.57 clarifies the difference between selectable combat techniques and passive mastery. The affinity techniques remain active choices in the tactical menu, while repeated real uses now unlock separately named mastery passives such as **Elemental Mastery**, **Circular Guard**, **Binding Trait**, **Revigorating Voice**, **Channeled Instinct**, and **Blade Rhythm**. Legacy save identifiers remain recognized, but player-facing names no longer make it look as if **Blade Dance** or the other techniques became automatic passives.   

## V3.49.56 — Varied affinity techniques   

- V3.49.56 adds six more affinity techniques so combat does not revolve only around breaks/debuffs: **Elemental Blade**, **Protective Circle**, **Binding Shot**, **Inspiring Chant**, **Beast Instinct** and **Blade Dance**. Elemental, protector, skirmisher, bard/leader, wild and duelist profiles each gain a dedicated option with passive progression after real uses.   

## V3.49.55 — Wider class-affinity techniques   

- V3.49.55 adds five more class-affinity techniques: **Mastered Rage**, **Battle Order**, **Breath Totem**, **Workshop Bomb** and **Steel Prayer**. The pass widens specialized class gameplay without making every action universal: frontline, command, nature, workshop and sacred profiles each gain a dedicated option with passive progression after real uses.   

## V3.49.53 — Class-affinity techniques   

- V3.49.53 adds a first layer of **class-affinity tactical techniques**: **Shadow step** is no longer universal, and sneaky, support, arcane, rampart and skirmisher profiles each gain a dedicated technique with progression after real uses.   

## V3.49.50 — Reach, bulwarks and anchors   

- V3.49.50 adds three tactical actions: **Break reach**, **Pierce bulwark** and **Break occult anchor**. It also adds **Careful reach**, **Necrotic anchor** and **Life drain** profiles, with distance reactions, grave-cold pressure, short recovery and counterplay through broken spacing or broken anchors.   

## V3.49.49 — Status chains, marked prey and enemy profiles   

- V3.49.49 adds three tactical actions: **Force a status chain**, **Mark prey** and **Controlled retreat**. It also adds the **Field Healer**, **Shield Bearer**, **Wounded Berserker** and **Curse Bearer** profiles, with their own observation lines, weaknesses, signature attacks and mechanical reactions.   

## V3.49.38 — Size, material and physical durability   

- V3.49.38 adds a real body durability read for creatures: small fragile bodies, small protected bodies, large organic masses, large hard masses, poor material bodies, slimes, specters and fairies no longer react the same way to physical damage. Observation and bestiary entries expose that read, and the damage report shows an immersive line when the body changes the impact. Recruited ally techniques also gain gestures tied more closely to their profile.   

## V3.49.74 — Wider mastery effects and sortie preparation   

- V3.49.74 continues the six active work tracks at once: several older affinity techniques now receive real mastery scaling, tactical passives remain supporting effects instead of automatic actions, enemy pressure gains more contextual intent lines, recruited allies scale their technique frequency more clearly with maturity, and World / city gains a sortie preparation reader for care, tools, observation, chests, combat crafting and mercenary options.   

## V3.49.73 — Enabled mastery passives and clearer city signals   

- V3.49.73 connects enabled mastery passives to more real tactical effects: an unlocked and enabled passive can now lightly support power, secondary chance, duration or rhythm without ever triggering the active action by itself. World / city also shows clearer local signals such as current city, guild registration and travel time.   

## V3.49.81 — Readable synergies and mastery impact   

- V3.49.81 adds a global build read to the Character menu and run preparation: weapon, armor, class bonus/malus and overall coherence. Active/passive mastery labels describe impact more clearly, recruited ally techniques scale more with maturity and orders, and enemy signature skills become slightly more present when an enemy is trained, elite or facing an already-open player.   

