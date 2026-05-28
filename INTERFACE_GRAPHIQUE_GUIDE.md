# Dinotofu — interface graphique expérimentale   

Version jeu : **V2.0.03**   
Version de personnage encore acceptable avant recréation conseillée : **V1.32.03**   

## But de l'IG   

L'IG expérimentale rend Dinotofu jouable depuis une page HTML/JS locale, sans retirer le terminal. Le terminal reste la base stable et le secours si un cas rare doit être forcé.   

## Architecture actuelle   

- Le C++ génère des snapshots JSON : menus, combat, état IG.   
- Le serveur local Python expose `/gui/snapshot`, `/gui/status`, `/gui/input` et `/gui/clear-input`.   
- La page HTML lit les snapshots, affiche les actions/cartes, puis envoie les commandes au serveur.   
- Les commandes sont mises dans une file que le terminal consomme.   

## Fichiers importants   

- `tools/gui/dinotofu_gui_experimental.html`   
- `tools/gui/serve_gui_preview.py`   
- `tools/gui/run_gui_debug.sh`   
- `tools/gui/README_GUI_PREVIEW.md`   
- `include/interface/model/GuiSchemaVersion.hpp`   
- `include/interface/model/MenuScreen.hpp`   
- `include/interface/model/MessageScreen.hpp`   
- `include/interface/model/CombatStateSnapshot.hpp`   
- `src/interface/GraphicalInterface.cpp`   
- `src/interface/GuiDebugExporter.cpp`   

## Contrat JSON   

Les champs `schemaVersion` ne sont pas des versions du jeu. Ils indiquent seulement la version du contrat JSON lu par l'IG. Ils ne doivent être augmentés que si un champ structurant est ajouté, supprimé ou change de sens.   

Ne pas bump `schemaVersion` pour : couleur, style, texte HTML, positionnement, micro-polish visuel.   

## État du palier V2   

- Les menus principaux, activités, sessions, création, sauvegardes, inventaire, équipement, potions, boutiques, quêtes, exploration, progression, catalogues, combats et boss exposent des snapshots IG.   
- Les cartes métier gardent le numéro d'action visible.   
- Les contrats de saisie couvrent choix, texte, quantité, confirmation, Entrée vide et codes cachés.   
- Le serveur bloque les commandes d'ancien écran et les files déjà occupées.   
- Les recommandations ne doivent pas révéler une faiblesse inconnue.   
- Le terminal reste le secours.   

## Prochaines directions   

- Stabiliser les retours joueurs sur la V2.0.03.   
- Ajouter ensuite contenu, dialogues/lore vivants, compétences et polish progressif.   
- Passer en V3.0.0 avec le premier chapitre d'histoire codé.   
