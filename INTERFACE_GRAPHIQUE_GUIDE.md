# Dinotofu — interface graphique   

L’interface graphique reprend le fonctionnement du jeu terminal dans une présentation plus lisible, interactive et adaptée à une utilisation desktop.   

## Principe général   

- Le moteur C++ reste la source de vérité.   
- L’interface lit les écrans et états de combat fournis par le jeu sous forme de snapshots JSON.   
- Une action affichée dans l’interface correspond toujours à une action réellement disponible dans le moteur.   
- Le contenu joueur doit rester fidèle au terminal : aucune information cachée ne doit être révélée et aucune information affichée par le terminal ne doit disparaître.   

## Organisation de l’écran   

L’interface n’affiche qu’un seul écran principal à la fois :   

- le menu ou la scène en cours hors combat ;   
- le combat lorsque le moteur indique qu’un combat est actif.   

Les titres, contextes, descriptions, informations complémentaires, choix et rappels sont séparés visuellement. Les panneaux spécialisés ne doivent pas dupliquer le contenu déjà fourni par le moteur.   

Un guide contextuel peut ajouter une lecture propre à l'écran actif : étapes de création de personnage, résumé de l'inventaire, rappel de conversion en boutique, objectifs connus d'une exploration ou état d'une quête. Ce guide se base uniquement sur les champs et textes déjà transmis par le moteur.   

Un bandeau permanent affiche les informations qui doivent rester visibles pendant le jeu : PV du personnage, jour, moment de la journée, portefeuille séparé par type de pièce et total converti. Ces données sont extraites des snapshots et des lignes terminal lorsqu'elles sont visibles, sans inventer de valeur cachée.   

## Choix et interactions   

- Les choix n’affichent pas de tags techniques ou décoratifs inutiles.   
- Chaque choix affiche son numéro, son intitulé et son explication utile lorsqu’elle existe.   
- Le texte revient automatiquement à la ligne et ne doit pas être tronqué lorsqu’il dépasse la largeur de la case.   
- Une ligne contient au maximum cinq choix sur un grand écran.   
- Le nombre de colonnes diminue automatiquement sur les écrans plus étroits.   
- Les actions indisponibles restent identifiables et affichent leur cause exacte : fermeture nocturne, progression requise, service absent, ressource manquante ou autre verrou réellement fourni par le moteur.   
- Un numéro correspondant à une action verrouillée ne doit plus produire le même message qu’un numéro inexistant : le joueur voit l’intitulé de l’action et une phrase claire, de préférence formulée dans le lore.   

## Navigation du monde   

- Le menu principal affiche directement **Combats**, **Exploration**, **Quêtes**, **PNJ notables** et **Lieux notables**.   
- **Exploration** conserve son rôle de sortie par biome : ressources, traces, coffres, risques et rencontres.   
- **Lieux notables** sert aux endroits précis du monde et propose d’abord **Tout afficher**, puis **Ville**, **Extérieur** et **Boutiques**.   
- **PNJ notables** propose également **Tout afficher** avant les catégories histoire, boutiques/services, autres notables et contacts temporaires.   
- Ce tri est identique en bac à sable et en histoire. Le mode histoire ne crée aucune route parallèle : il masque uniquement les lieux, PNJ et services qui n’existent pas encore ou ne sont pas réellement accessibles.   
- Dans l’IG, même la vue complète reste regroupée visuellement par catégorie. Les lieux et PNJ utilisent des accents discrets différents afin d’être reconnus rapidement sans recolorer tout l’écran.   

## Combats   

L’écran de combat reprend uniquement les informations communiquées par le moteur :   

- phase et numéro du tour ;   
- acteur et cible actuels ;   
- possibilité de fuite lorsque celle-ci existe ;   
- actions du joueur ;   
- résumé du combat ;   
- unités, niveau, type, points de vie et états visibles ;   
- journal récent.   

Les unités sont rangées en groupes lisibles : joueur et alliés, invocations alliées, ennemis, invocations ennemies et unités neutres. Les statistiques non affichées dans le terminal ne sont pas ajoutées artificiellement dans l’interface.   

L'interface peut ajouter une synthèse tactique visuelle des unités déjà visibles : PV cumulés visibles, unités encore actives, invocations, acteur actuel et cible actuelle. Une file d'initiative lisible place l'acteur courant en premier et présente ensuite les unités actives déjà connues ; elle ne prétend pas connaître un ordre futur que le moteur n'a pas encore communiqué. Cette synthèse ne remplace pas le journal et ne doit pas exposer d'information absente du snapshot.   

Le combat garde la même vue pendant les annonces et les résultats :   

- « Tour de X » est conservé dans un fil de déroulement ;   
- l’action, les dégâts, les soins ou les changements d’état apparaissent juste en dessous ;   
- après un choix du joueur, la liste d’actions disparaît immédiatement et un état de résolution la remplace ;   
- les petits écrans intermédiaires peuvent avancer automatiquement après un délai de lecture calculé selon la longueur du texte ;   
- les introductions, règles, victoires, défaites, morts, récompenses et bilans importants restent contrôlés par le joueur.   


## Quêtes   

Les listes de quêtes restent volontairement courtes. Une entrée affiche seulement :   

- le titre principal ;   
- le lieu cible ;   
- les récompenses potentielles ;   
- l’avancement.   

Une quête acceptée ouvre un menu court contenant **Inspecter**. Une offre non acceptée propose d’abord **Demander plus d’informations** ; le joueur peut refuser immédiatement une demande de PNJ ou laisser un contrat de guilde de côté. Les quêtes principales terminées restent consultables dans **Principales finies**, tandis que les PNJ notables sont rangés en PNJ d’histoire, boutiques/services, autres notables et PNJ de quêtes temporaires, avec une vue complète disponible en premier. Les écrans de mission et de validation d’histoire ne doivent jamais révéler toute une chaîne future : ils montrent les étapes `[fait]`, l’étape actuelle et un simple compteur pour la suite masquée. Les quêtes de synthèse peuvent déclarer des quêtes liées avec un état attendu (`connue`, `terminée` ou `rendue`) ; leur progression est recalculée depuis le journal, y compris lorsque le joueur a accompli une branche avant l'ouverture de la synthèse. Les escortes, choix et combats qui doivent commencer après un dialogue restent non rétroactifs.   

Dans l’IG, seules les sections concernées reçoivent un accent de couleur : vert léger pour `[fait]`, jaune-orangé discret pour un lieu portant un **objectif de quête**. Le reste du texte conserve sa couleur normale.   

## Apparence   

- L’interface normale utilise un fond sombre et des contrastes clairs.   
- Le combat utilise une ambiance noire légèrement rougeâtre afin d’ajouter de la pression sans réduire la lisibilité.   
- Les cartes, titres et boutons utilisent une hiérarchie visuelle cohérente.   
- En combat, les noms, verbes importants, dégâts, soins, réductions et totaux reçoivent un gras ciblé ; hors combat, ce principe s’applique aux noms de PNJ, lieux et récompenses utiles.   
- Les cartes de lecture graphique servent à rendre le terminal plus agréable : résumé, éléments cliquables, notices, focus utiles et recherche/filtre, sans cacher le texte terminal complet.   
- Les cartes d'objet peuvent afficher le type d'action, la durabilité et les effets/enchantements fournis par le moteur.   
- L'inspection d'un équipement peut afficher un tableau **Actuel / Inspecté / Écart** pour les statistiques réellement fournies par le moteur, suivi d'un verdict prudent : amélioration, compromis ou objet inutilisable.   
- Les écrans d'exploration peuvent construire une chronologie compacte de deux à six événements à partir des lignes déjà connues, sans inventer de rencontre.   
- Les cartes de compte et de personnage peuvent afficher version de sauvegarde, compatibilité, progression d'histoire et dernière activité connue.   
- La hiérarchie de puissance des boss reste un outil interne de conception. L'IG ne montre ni palier, ni couleur de palier, ni identité que le personnage n'a pas réellement découverte. Les mini-boss uniques ne rejoignent pas le registre des boss majeurs.   
- Les écrans de création utilisent une progression visuelle compte → identité → race → classe → validation, sans empêcher le joueur de lire les contraintes complètes.   
- Les boutiques rappellent que les prix historiques en or sont réglés depuis le portefeuille multi-pièces converti automatiquement.   
- Les écrans doivent rester compréhensibles sans répéter les mêmes informations dans plusieurs panneaux.   

## Architecture   

- `src/interface/` : passerelle C++ vers l’interface ;   
- `include/interface/` : modèles et contrats ;   
- `tools/gui/` : interface HTML/JavaScript et serveur local ;   
- `include/interface/model/GuiSchemaVersion.hpp` : version du contrat JSON.   

## Contrat JSON   

`schemaVersion` désigne la version du contrat entre le moteur et l’interface, pas la version du jeu.   

Cette valeur ne doit changer que lorsqu’un champ structurant est ajouté, supprimé ou change de sens. Une modification de couleur, de placement ou de mise en page ne justifie pas une nouvelle version du contrat.   

## Objectif de stabilité   

La version terminal reste un moyen de secours. Toute fonctionnalité jouable dans le terminal doit pouvoir être représentée dans l’interface sans réécrire la logique du jeu ni créer une seconde logique parallèle.   

### Registre des boss et connaissances   

- Une première victoire peut ajouter une seule entrée inconnue suivante au registre.   
- Refaire un boss déjà vaincu ne doit jamais produire une nouvelle carte de boss.   
- Une piste d'exploration rarissime peut ajouter une entrée `???`, uniquement comme emplacement approximatif.   
- L'IG ne doit afficher ni l'identité, ni le lore, ni les mécaniques, ni la position précise tant que ces informations ne sont pas découvertes.   
- FireFlight reste invisible et inaccessible tant que toutes ses conditions spéciales ne sont pas remplies.   


## Quêtes et variété d’exploration — V3.20.00   

- L’inspection d’une quête affiche une **Prochaine action** produite par le moteur : contact de rendu, sous-objectif visible et lieu conseillé.   
- L’IG met cette action en avant sans afficher les étapes encore masquées.   
- Les épreuves et grandes familles d’événements récemment rencontrées sont mémorisées dans la sauvegarde afin de réduire les répétitions rapprochées.   
- Cette mémoire change uniquement le tirage ; aucun compteur technique ou historique caché n’est affiché au joueur.   


## Inventaire, registre et chapitre 4 — V3.21.00   

- Les cartes d’inventaire sont regroupées avec l’équipement porté et les objets importants en premier, puis triées pour accélérer la lecture.   
- Un résumé visuel de l’équipement porté peut afficher les emplacements détectés, les actions disponibles et les objets visiblement endommagés.   
- Le registre affiche une progression construite uniquement à partir des cartes et lignes déjà visibles : vaincu, connu ou verrouillé. Il ne doit jamais déduire une identité cachée.   
- Les écrans d’histoire reconnaissent la première phase du chapitre 4 et conservent les conséquences visibles des choix du chapitre 3.   
- Les cooldowns d’exploration restent entièrement internes : l’IG ne montre ni durée, ni compteur, ni raison technique de reroll.   

## V3.22.00 — commerce et dossiers marchands   

- Les cartes et lignes de boutique peuvent signaler l'offre de trois jours, son jour actuel, le prix normal, le prix remisé et la quantité restante d'un déstockage.   
- Les vendeurs temporaires recommandés conservent leur propre nom et leur style de vente ; ils ne récupèrent pas automatiquement les quêtes ou services spéciaux d'une boutique permanente.   
- Les services de Prunigil distinguent réussite, réussite partielle et erreur complète lorsqu'un document demande aussi une décision de confidentialité ou de conservation comptable.   
- Les nouveaux titres marchands et la prochaine étape de confiance restent visibles depuis les écrans déjà alimentés par le moteur.   

## V3.23.00 — défis de guilde et marchands légendaires   

- La guilde possède un onglet **Défis** séparé des contrats ordinaires : trois offres sont générées chaque jour et chaque défi accepté reste valable jusqu’à la fin du jour suivant.   
- Les cartes de défi affichent la contrainte, la difficulté, la récompense modeste, le nombre de **Marques de défi** et l’action **Accepter** ou **Valider** selon l’état réel du moteur.   
- Une condition déjà active ne doit pas être reproposée dans la rotation du lendemain ; un défi expiré peut revenir plus tard dans une autre rotation aléatoire.   
- Les titres de défi sont accordés uniquement après vérification des actions réellement utilisées en combat : attaque simple, technique, compétence, consommable, défense, attente, fuite et dégâts subis.   
- Le Hero Villager et Bob et Maurice apparaissent comme vendeurs temporaires distincts, avec leurs noms, dialogues, calendriers et quêtes propres.   
- L’épreuve amicale des caisses utilise un sous-inventaire temporaire et ne doit jamais modifier l’inventaire réel, l’équipement réel, les morts ou les pénalités du personnage.   
- Les interventions légendaires, rumeurs et validations automatiques restent affichées comme des scènes du moteur ; l’IG ne doit pas les résumer au point de masquer leurs dialogues.   


## V3.24.00 — inventaire, comparaison, registre et initiative collective   

- Le résumé d’inventaire affiche les catégories réellement visibles, le nombre d’actions utilisables, l’équipement repéré et les objets à surveiller.   
- La comparaison d’équipement ajoute une lecture visuelle de l’amplitude des écarts sans changer les calculs du moteur.   
- Le registre distingue clairement les entrées vaincues, connues et masquées sans déduire un nom absent.   
- Les combats de groupe peuvent afficher un ordre d’initiative commun comprenant joueurs, ennemis et invocations.   
- Les défis collectifs et le Comptoir des Marques utilisent les mêmes cartes structurées que les autres actions de guilde.   
- Les bonus temporaires de la Lucky Potion — puissance, précision et garde — rejoignent les statuts déjà visibles sans remplacer les messages détaillés du moteur.   
- Les séparations de slimes restent annoncées dans le journal de combat ; les nouveaux petits slimes apparaissent ensuite normalement dans la ligne active ou la file d’attente, sans carte fantôme ni identité cachée supplémentaire.   


## V3.25.00 — listes déroulantes complètes et mini-boss gélatineux   

- En mode IG, les helpers de pagination exposent désormais l’intégralité des options d’un écran en une seule liste ; le terminal conserve sa pagination habituelle.   
- Les actions « Page suivante » et « Page précédente » ne sont plus affichées dans l’IG. Chaque option disponible reste une carte cliquable envoyant directement son choix au moteur.   
- Les longues listes possèdent une hauteur bornée, une barre de défilement, le défilement à la molette et un support des touches Page précédente / Page suivante pour déplacer la vue sans changer de page logique.   
- Les lignes textuelles résiduelles de type « Page X / Y » sont masquées dans l’IG afin de ne pas recréer visuellement une pagination supprimée.   
- Le terminal n’est pas affecté : il continue à présenter des pages courtes avec les commandes 98 et 99 lorsque la liste est longue.   
- Le Géant slime des quatre divisions et ses descendants utilisent la file ennemie normale. Les Âmes de slime finales sont identifiées visuellement comme des invocations ennemies.   

## V3.26.00 — sélecteur graphique de quantité   

- Toute saisie de quantité reçue par l’IG utilise un carrousel central au lieu d’exiger l’écriture manuelle du nombre.   
- Les flèches simples modifient la quantité de 1 ; les doubles flèches modifient la quantité de 10.   
- La valeur centrale reste directement éditable au clavier. Une valeur supérieure au maximum est immédiatement ramenée au maximum réellement disponible.   
- Le minimum respecte le contrat du moteur : 0 lorsque l’annulation ou l’absence de transfert est autorisée, sinon 1 ou la limite minimale fournie.   
- Les boutons se désactivent lorsqu’une limite est atteinte et pendant l’envoi d’une commande afin d’éviter les doubles saisies.   
- Les touches fléchées gauche/droite ou bas/haut reproduisent le pas de 1 ; maintenir Maj avec une flèche reproduit le pas de 10. Les quatre boutons visibles restent `<<`, `<`, `>` et `>>`.   
- Le passage à un autre article ou à une autre demande réinitialise correctement la valeur, même lorsque l’identifiant technique de l’écran reste identique.   
- Le terminal continue à demander le nombre sous forme textuelle et n’est pas modifié.   


## V3.27.00 — atelier desktop, protections et suivi global   

### Inventaires et boutiques en deux panneaux   

- La colonne gauche contient la liste scrollable et directement cliquable.   
- La colonne droite reste visible pendant le défilement et présente l’objet survolé ou sélectionné.   
- Les statistiques détectables sont comparées à l’équipement porté : vert léger pour un gain, rouge léger pour une perte.   
- La barre supérieure reste visible et combine recherche, catégories, rareté et quantité minimale.   

### Favori, Verrouillé et Suivi   

- **Favori** protège les ventes rapides et impose une confirmation supplémentaire avant une action destructive.   
- **Verrouillé** bloque directement les ventes, suppressions, consommations et transferts destructifs envoyés depuis l’IG.   
- **Suivi** met l’article en évidence dans tous les écrans et textes où son nom est reconnu : inventaire, boutique, craft, récompense, combat, butin ou exploration.   
- Ces trois états appartiennent à l’interface locale et ne modifient pas les règles du terminal.   

### Achats, prix et actions groupées   

- Un panier local calcule le total estimé et l’argent estimé après achat avant d’envoyer les commandes une par une au moteur.   
- L’historique local retient les prix observés par boutique et signale le meilleur prix déjà rencontré.   
- Les ventes rapides excluent toujours les favoris et les verrouillés.   
- Réparer tout et transférer plusieurs piles utilisent la même file protégée et respectent les écrans intermédiaires.   

### Combat et sécurité   

- La barre rapide mémorise les consommables déjà observés et ouvre automatiquement leur menu lorsque l’action existe.   
- Le journal de combat est repliable et filtrable par dégâts, soins, statuts et invocations.   
- Les actions sensibles ouvrent une fenêtre de confirmation résumant les pertes ou consommations connues.   
- Le fil d’Ariane n’invente aucune navigation : il envoie successivement les véritables actions Retour du moteur.   

### Sélecteur de quantité   

- L’ordre visuel reste `<<`, `<`, `[quantité éditable]`, `>`, `>>`, puis **MAX**.   
- Les doubles flèches changent la quantité de 10 et ne correspondent jamais aux pages précédentes/suivantes.   

## V3.28.00 — atelier personnalisable, registres séparés et socles persistants   

### Profils, précets et charge visuelle   

- L’interface possède trois profils de contexte : **Exploration**, **Combat** et **Commerce**. Le profil peut suivre automatiquement l’écran courant ou être choisi manuellement.   
- Trois précets nommables maximum permettent d’enregistrer puis de restaurer une combinaison de panneaux, densité, taille d’interface et préférences d’affichage.   
- Le **mode concentration** masque les panneaux secondaires pour ne conserver que l’objectif, l’action principale et les informations vitales.   
- La taille de l’interface, la réduction des animations, le combat compact/détaillé et les panneaux optionnels restent réglables séparément.   
- Les gains et pertes utilisent toujours un signe, une flèche ou un texte en plus de la couleur afin de rester lisibles sans dépendre du vert et du rouge.   

### Inventaire, équipement et planification   

- La zone d’équipement présente une silhouette textuelle et des emplacements reconnus. Aucun sprite générique n’est inventé : les futurs visuels utiliseront la race, l’âge, la présentation, la classe et la variante sauvegardés par le moteur.   
- Jusqu’à trois objets peuvent être épinglés pour une comparaison côte à côte. L’IG peut suggérer le meilleur objet selon un critère visible, mais n’équipe jamais automatiquement sans validation.   
- Les jeux d’équipement mémorisent les objets observés et rejouent les véritables commandes du moteur. Un objet absent, cassé ou volé est signalé au lieu d’être ignoré.   
- Les tris personnalisés, marqueurs visuels, notes et historiques de provenance sont stockés localement dans l’atelier.   
- La liste de souhaits n’accepte un objet précis que si le joueur l’a déjà possédé au moins une fois. Elle reste distincte du suivi global.   

### Butin, commerce et statistiques observées   

- Un récapitulatif de butin rassemble objets reconnus, nouveautés, éléments suivis, matériaux rares, argent et progressions visibles sans inventer une récompense absente du texte moteur.   
- Les prix observés affichent prix actuel, moyenne, meilleur prix, variation récente et une ligne simple d’historique.   
- Les statistiques **Top 3** comptent uniquement les événements réellement observés par l’IG : ennemis rencontrés ou vaincus, boss, matériaux, consommables, armes, lieux, PNJ servis et types de quêtes. Le total de chaque catégorie reste visible.   
- Ces statistiques locales ne remplacent pas encore un futur historique canonique fourni par les sauvegardes du moteur.   

### Combat lisible sans avantage illégal   

- L’aperçu d’action reprend uniquement les chances, coûts, portées, dégâts, réactions ou risques de défi déjà exposés par le moteur.   
- La file d’initiative est une lecture visuelle. Aucun ennemi n’est déplaçable. Un allié ne pourra être déplacé que par un futur effet moteur autorisé, jamais avant son initiative minimale ni avant son délai minimal.   
- Les divisions, invocations et alertes contextuelles sont signalées quand les données visibles les confirment. Une mécanique inconnue ou un risque particulier inconnu n’est jamais révélé par déduction.   
- Les modes compact et détaillé peuvent être changés pendant le combat sans modifier les règles.   

### Connaissance, fiabilité et registres   

- Le **Bestiaire** est réservé aux créatures, ennemis, invocations, boss et personnages spéciaux.   
- L’**Encyclopédie** rassemble les connaissances utiles de gameplay : races, matériaux, objets, effets, zones, classes, craft et guildes.   
- Le **Carnet de découvertes** rassemble les connaissances narratives : légendes, divinités, lore, rumeurs, contes et découvertes étranges. Les trois registres restent au sommet de l’inventaire et sont impossibles à perdre.   
- Les sources d’obtention utilisent des niveaux de fiabilité : **Rumeur**, **Observé**, **Confirmé** et **Expert**.   
- Les lieux ou adversaires les plus probables sont affichés seulement lorsqu’une source connue permet de le dire ; aucune probabilité exacte n’est inventée.   
- Une recette reste complètement masquée tant que tous ses composants n’ont pas été découverts. La connaissance d’un matériau est persistante même après consommation de son dernier exemplaire : elle prouve seulement que le joueur connaît le matériau, pas qu’il le possède encore.   

### Coffres municipaux et villes   

- Chaque ville possède son propre coffre municipal personnel : achat, niveau, capacité et contenu sont indépendants.   
- Le coffre local est séparé de l’inventaire transporté et n’est pas détruit par la survie léthale exceptionnelle.   
- Depuis un coffre local, le joueur peut consulter les autres coffres déjà achetés en lecture seule. Aucun retrait distant n’est possible.   
- Un futur transfert municipal pourra être limité, coûteux et différé, mais il ne doit jamais remplacer l’intérêt de voyager ou de gérer ses stocks.   

### Survie léthale exceptionnelle   

- Une bénédiction compatible peut repousser une mort définitive une seule fois. Le moteur consomme alors toutes les bénédictions, détruit l’inventaire transporté, l’équipement et l’argent, puis replace le personnage à exactement 1 PV.   
- Le personnage reçoit la **Marque de survie interdite**, une malédiction permanente et impossible à retirer : aucune future bénédiction et seulement 95 % des soins normaux.   
- Le coffre municipal est épargné, puisqu’il n’est pas transporté pendant le combat.   
- La route d’acquisition de cette bénédiction reste volontairement absente tant qu’un contenu narratif réel ne l’accorde pas.   


## V3.29.00 — registres en trois objets, coffres indépendants et futures cartes visuelles   

### Registres spéciaux   

- Le sommet de l’inventaire affiche maintenant trois objets spéciaux permanents : **Bestiaire**, **Encyclopédie** et **Carnet de découvertes**.   
- Le Bestiaire donne raison au vocabulaire joueur : il concerne les êtres, créatures, invocations, boss et PNJ spéciaux.   
- L’Encyclopédie reste le registre pratique : objets, matériaux, zones, races, classes, effets et craft.   
- Le Carnet de découvertes évite le fourre-tout : lore, légendes, divinités, rumeurs, histoires et découvertes étranges.   

### Coffres par ville   

- Les coffres de Valebrume, Rocheveille, Port-Lanterne et Lys-de-Givre sont sauvegardés séparément.   
- Améliorer le coffre d’une ville n’améliore pas les autres.   
- La consultation distante est volontairement limitée à la lecture seule : voir le contenu, sans retrait et sans transfert instantané.   

### Ramassage automatique futur   

- Les règles de ramassage ne doivent jamais devenir un bouton « convertir tout en argent ».   
- Ignorer automatiquement un déchet commun signifie seulement ne pas le prendre sans confirmation. L’objet doit rester visible dans le récapitulatif de butin pour que le joueur puisse le prendre, le vendre ou le laisser.   
- Les objets suivis, rares ou liés à une quête doivent être signalés clairement avant toute décision automatique.   

### Préparation pixel-art et carte   

- Plus tard, l’IG pourra afficher une vraie image de ville avec bâtiments cliquables : guilde, boutiques, coffre, bibliothèque, arène urbaine et grande porte de sortie des remparts.   
- L’exploration pourra utiliser une grande carte avec biomes et lieux cliquables. Les lieux inconnus devront être grisés, enfumés ou masqués.   
- Les explorations pourront avoir de petites animations dont la durée dépend de la longueur choisie, et certaines quêtes ou événements pourront afficher une illustration dédiée.   
- Rien de tout cela ne doit être simulé en terminal : sans asset validé, le terminal affiche seulement une description courte et utile.   

## V3.30.00 — préparation carte, villes et poids d'équipement   

- L'IG devra plus tard représenter chaque ville par une image cliquable : bâtiments, guilde, boutiques, coffre, porte de sortie et arène urbaine.   
- La porte des remparts déclenchera la carte d'exploration ; l'arène déclenchera les combats uniques ou défis de ville.   
- La carte d'exploration devra afficher plusieurs biomes et lieux cliquables. Les lieux inconnus seront grisés, enfumés ou partiellement masqués.   
- Les animations d'exploration doivent être séparées : d'abord le trajet depuis la ville ou le lieu de départ, puis l'animation dans le lieu choisi.   
- Chaque trajet et chaque lieu doivent utiliser un fond cohérent avec leur biome afin d'améliorer l'immersion.   
- Les armes et armures affichent désormais leur poids : léger, moyen ou lourd. L'IG doit présenter les contreparties avec des termes simples, sans noyer le joueur dans les calculs.   

## V3.41.00 — pont vers les pixel-arts   

L'IG affiche maintenant un panneau de préparation visuelle sur les écrans de ville, carte, coffre et micro-quêtes. Ce panneau ne remplace pas les futurs assets : il sert à réserver les emplacements de l'interface, à montrer le flux Ville → Porte → Trajet → Biome → Lieu, et à éviter d'ajouter de fausses images avant la bible pixel-art.   

Les blocs peuvent ensuite devenir : bâtiments cliquables, zones enfumées, fonds de trajet, fonds de lieu, événements illustrés, micro-quêtes et panneaux de ville.   

## V3.47.00 — Branding actif validé   

- L’IG conserve le mode sûr V3.46 : pas de panneaux parasites, pas de workbench d’objets permanent, pas de placeholders pixel-art imposés partout.   
- Le logo officiel Dinotofu est affiché dans l’en-tête IG.   
- La bannière validée est utilisée comme fond visuel de l’interface, avec une couche sombre pour garder le texte lisible.   
- Les images de branding sont actives, mais elles ne remplacent aucun choix, prix, danger, condition, texte de quête ou information de combat.   
- Les icônes de lancement normal et terminal sont rangées dans `assets/branding/` pour les raccourcis Windows/Linux.   
