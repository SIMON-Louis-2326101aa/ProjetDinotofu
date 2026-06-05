# Dinotofu — interface graphique expérimentale   

L'interface graphique prépare une version desktop plus confortable, mais la version terminal reste actuellement la base la plus stable.   

## But de l'IG   

- Afficher les menus, choix, combats, inventaires, quêtes et boutiques avec une interface plus lisible.   
- Garder le terminal comme secours pour ne jamais bloquer le joueur.   
- Permettre une future version desktop plus propre sans réécrire tout le moteur de jeu.   

## Architecture actuelle   

- Le moteur C++ reste la source de vérité.   
- L'IG lit des snapshots JSON fournis par le jeu.   
- Les actions envoyées par l'IG doivent correspondre aux choix déjà valides côté moteur.   

## Fichiers importants   

- `src/interface/` : passerelle C++ vers l'interface ;   
- `include/interface/` : modèles et contrats ;   
- `tools/gui/` : interface HTML/JS et serveur local ;   
- `include/interface/model/GuiSchemaVersion.hpp` : version du contrat JSON.   

## Contrat JSON   

Les champs `schemaVersion` ne sont pas des versions du jeu. Ils indiquent seulement la version du contrat JSON lu par l'IG.   

Ne pas modifier `schemaVersion` pour une couleur, un texte, un placement visuel ou un micro-polish. Le modifier seulement si un champ structurant est ajouté, supprimé ou change de sens.   

## Priorité actuelle   

- Garder tous les écrans jouables via terminal.   
- Stabiliser l'IG avant le gros polish visuel.   
- Ajouter les contenus histoire après écriture claire des chapitres, quêtes et boss obligatoires.   
