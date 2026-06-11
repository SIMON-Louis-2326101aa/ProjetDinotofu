# Dinotofu - interface graphique expérimentale   

Version jeu : **V3.18.00**   
Version minimale conseillée des personnages : **V3.00.00**   

## Lancement rapide Linux / WSL   

Depuis la racine du projet :   

```bash   
./tools/gui/run_gui_debug.sh   
```   

Ou manuellement :   

```bash   
DINOTOFU_GUI_DEBUG_DIR=gui_debug DINOTOFU_GUI_INPUT_MODE=1 ./output/Dinotofu   
python3 tools/gui/serve_gui_preview.py --root . --port 8787   
```   

Puis ouvrir :   

```text   
http://127.0.0.1:8787/tools/gui/dinotofu_gui_experimental.html   
```   

## Rôle actuel   

L'IG lit les snapshots produits par le jeu, affiche les menus, combats et cartes, puis envoie les choix dans une file de commandes consommée par le moteur. En combat, les choix du joueur disparaissent dès leur validation, le déroulement du tour reste regroupé dans un même écran et les petites transitions peuvent avancer automatiquement après un délai de lecture. L'écran ajoute aussi un bandeau permanent avec PV, jour, moment de journée et portefeuille complet, plus une couche de cartes de lecture graphique pour rendre les informations terminal plus rapides à comprendre. Le terminal reste disponible comme secours.   

## Entrées supportées   

- choix numériques ;   
- quantités ;   
- confirmations exactes ;   
- textes libres ;   
- codes cachés ;   
- clics sur actions, cartes métier, focus cards et unités ciblables ;   
- listes de quêtes condensées, inspection des contrats acceptés et demandes d'informations avant acceptation.   

## Règles importantes   

- Les numéros doivent rester visibles sur les actions.   
- Les recommandations ne doivent pas forcer la main.   
- Les attaques ne doivent être mises en avant que si une faiblesse est connue/débloquée.   
- Le heal peut être recommandé quand il est pertinent.   
- Une unique action sûre peut être mise en avant.   
- Le terminal reste obligatoire comme secours.   

## Jalons   

- V2.01.03 : légendes automatiques moins répétitives après reprise de sauvegarde et dialogues d’ambiance PNJ/guilde ajoutés.   
- V3.00.00 : premier chapitre d'histoire.   
- Multijoueur en ligne : grosse version future, pas prioritaire.   

## Note V2.01.03   

- Le bestiaire expose maintenant un registre séparé des légendes.   
- Les sous-sections de légendes doivent rester lisibles dans l'IG comme dans le terminal : retour séparé, choix numérotés, textes blancs sur fonds sombres.   
- La V2.01.03 est maintenant le socle de sauvegarde conseillé côté registre/lore ; les personnages plus anciens déclenchent une recommandation forte de recréation.   
