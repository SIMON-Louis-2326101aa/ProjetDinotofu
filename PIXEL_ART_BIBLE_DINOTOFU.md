# Bible visuelle pixel-art Dinotofu — brouillon V3.32.00   

## Objectif   

Préparer les futurs assets sans les générer trop tôt. Cette bible sert de contrat visuel avant de produire les villes, bâtiments, biomes, routes, événements, boss et portraits.   

## Principes généraux   

- Style : pixel-art lisible, fantasy médiéval avec touches DND/anime.   
- Priorité : lisibilité gameplay avant détail décoratif.   
- Interface : ne jamais inonder l'écran ; utiliser des couches, infobulles et paramètres d'affichage.   
- Terminal : aucune image affichée. Le terminal donne seulement une description courte des éléments importants.   
- IG desktop : image + description courte.   

## Formats prévus   

- Hub de ville : grande image horizontale avec bâtiments cliquables.   
- Bâtiments : zones cliquables nommées, visibles sans forcer le joueur à deviner.   
- Carte d'exploration : grande carte par biomes, lieux cliquables, inconnus en gris/enfumé.   
- Routes : background propre au trajet entre départ et arrivée.   
- Lieu d'exploration : background propre au biome/lieu, différent du trajet.   
- Quêtes/événements : petite illustration seulement pour événements importants ou rares.   
- Boss : illustration dédiée quand le boss a assez d'importance.   
- Portrait PNJ : optionnel au début, prioritaire pour PNJ récurrents.   

## Ville cliquable   

Chaque ville doit avoir une vraie identité visuelle.   

### Bâtiments communs   

- Guilde locale.   
- Coffre municipal.   
- Grande porte des remparts, utilisée pour partir en exploration.   
- Arène urbaine, utilisée pour combat unique ou entraînement spécial.   
- Marché local.   
- Archives locales.   
- Forge ou atelier selon la ville.   

### Règle d'interaction   

Un bâtiment doit être cliquable si son service existe vraiment. Si le service est verrouillé, le bâtiment peut être visible mais marqué comme fermé, gardé ou inaccessible.   

## Exploration   

L'exploration future doit être en deux temps visuels :   

1. Animation de trajet depuis la ville, la porte des remparts ou le point de départ.   
2. Animation dans le lieu sélectionné.   

Le background doit changer selon :   

- la ville de départ ;   
- la route utilisée ;   
- le biome traversé ;   
- la durée d'exploration ;   
- le lieu d'arrivée ;   
- la météo ou l'événement futur.   

## Carte / biomes   

- Les biomes connus sont visibles normalement.   
- Les lieux inconnus sont en gris/enfumé.   
- Un lieu inconnu ne doit pas révéler son nom précis si le joueur ne l'a pas découvert.   
- La distance vers un biome dépend de la ville de départ.   
- Les grandes découvertes peuvent enlever le brouillard progressivement.   

## Identité des villes   

### Valebrume   

- Village fortifié en reconstruction.   
- Remparts simples, forge de Bram, intendance de Mira.   
- Couleurs : bois, pierre, brume légère, verts doux.   

### Rocheveille   

- Ville minière verticale.   
- Forges lourdes, rails, ascenseur de mine, roche froide.   
- Couleurs : gris pierre, orange forge, fer froid.   

### Port-Lanterne   

- Ville marchande humide.   
- Quais, ponts, lanternes, marché sous les arches.   
- Couleurs : bleu brume, jaune lanterne, bois humide.   

### Lys-de-Givre   

- Ville froide et magique.   
- Sanctuaire, porte nord, givre, vitraux bleus.   
- Couleurs : bleu glacé, blanc, violet/lunaire discret.   

## Accessibilité et confort   

- Prévoir taille d'interface réglable.   
- Prévoir réduction des animations.   
- Ne jamais dépendre uniquement des couleurs rouge/vert : ajouter +, -, flèches ou texte.   
- L'Anomalie peut perturber l'interface, mais pas rendre l'expérience physiquement pénible.   

## Nommage recommandé des assets futurs   

- `city_<id>_hub.png`   
- `city_<id>_building_<building_id>.png`   
- `route_<origin>_<destination>_<route_type>.png`   
- `biome_<biome_id>_map.png`   
- `place_<place_id>_background.png`   
- `event_<event_id>_illustration.png`   
- `boss_<boss_id>_portrait.png`   
- `npc_<npc_id>_portrait.png`   

## À ne pas faire maintenant   

- Ne pas générer de faux assets temporaires définitifs.   
- Ne pas mettre d'image dans le terminal.   
- Ne pas créer une carte cliquable sans les données de lieux, distances et verrouillages.   
- Ne pas révéler les lieux inconnus avec une image trop précise.   

## V3.34.00 — routes de nuit, carte schématique et missions déléguées   

- Les routes normales entre villes doivent visuellement pouvoir être fermées la nuit : portes closes, gardes, lanternes, panneau ou herse.   
- Un convoi gardé nocturne devra avoir une identité visuelle distincte : torches, chariot, escorte, registre de départ, coût clairement affiché.   
- La carte schématique actuelle prépare la future carte pixel-art : villes en points, routes en lignes, biomes en zones, lieux inconnus gris/enfumés.   
- Les missions déléguées devront plus tard avoir un bureau ou comptoir visible : panneau d'annonces, aventuriers PNJ, employés de ville, retour de mission avec petite scène de compte rendu.   
- Les événements importants de route doivent être illustrables sans se répéter à l'infini : pont cassé, convoi bloqué, patrouille, camp abandonné, marchand ambulant, lieu suspect.   

## Ajout V3.41.00 — préparation IG sans faux assets   

La V3.41.00 ajoute un pont visuel expérimental dans l'interface graphique pour préparer les futurs pixel-arts sans afficher de fausses images.   

À prévoir comme premiers lots d'assets :   

- une image de ville par ville majeure ;   
- bâtiments séparés et cliquables : guilde, auberge, coffre municipal, marché, porte/remparts, arène, archives ;   
- grande carte d'exploration avec biomes visibles, lieux cliquables et zones inconnues grisées/enfumées ;   
- backgrounds de trajet entre ville et lieu ;   
- backgrounds propres aux lieux/biomes ;   
- petites illustrations pour événements rares, micro-quêtes de route, sanctions de guilde et rapports de groupe.   

Règle importante : tant qu'un asset n'existe pas, l'IG doit afficher un wireframe propre ou une description, pas une image inventée.   


## Ajout V3.43.00 — politique images et accessibilité   

- Terminal : images toujours désactivées et non activables.   
- IG : les images doivent pouvoir être désactivées depuis les paramètres.   
- Les images sont une couche d’ambiance et de lisibilité, jamais une source unique d’information.   
- Même si un décor, un bâtiment, un biome ou un personnage est affiché, les textes doivent conserver les coûts, conditions, dangers, distances, récompenses, stocks et conséquences.   
- Le fond d’un intérieur doit rester derrière le texte et le fond du texte doit rester prioritaire pour la lisibilité.   


## Ajout V3.44.00 — priorité gameplay avant visuel   

- Les pixel-arts restent en pause côté intégration tant que les corrections jouables passent avant.   
- Les rencontres élites doivent rester rares et lisibles : l’image ne doit jamais masquer qu’un combat est spécial ou dangereux.   
- Même avec décor d’exploration, le texte doit toujours afficher le type de rencontre, le niveau, la zone et les avertissements utiles.   

## V3.47.00 — Assets branding officiels   

Dossier actif : `assets/branding/`.   

- `dinotofu_logo.png` : logo principal du jeu.   
- `dinotofu_site_logo.png` : logo pour site / page projet.   
- `dinotofu_banner.png` : bannière officielle validée.   
- `dinotofu_intro_cover.png` : couverture / intro menu.   
- `dinotofu_launcher_graphical.png` et `.ico` : raccourci launcher normal / graphique.   
- `dinotofu_launcher_terminal.png` et `.ico` : raccourci version terminale.   

Le logo peut utiliser la mascotte dino-tofu car elle colle au titre. En revanche, les bannières et écrans larges doivent représenter les races/personnages du monde, pas une équipe de petits dinotofus jouables.   
