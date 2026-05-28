# Dinotofu - interface graphique expérimentale   
   
Version jeu : **V2.0.09**   
Version minimale conseillée des personnages : **V1.32.03**   
   
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
   
L'IG lit les snapshots produits par le jeu, affiche les menus/combat/cartes, puis envoie les choix dans une file de commandes consommée par le terminal. Elle sert à jouer plus confortablement tout en gardant le terminal comme secours.   
   
## Entrées supportées   
   
- choix numériques ;   
- quantités ;   
- confirmations exactes ;   
- textes libres ;   
- codes cachés ;   
- clics sur actions, cartes métier, focus cards et unités ciblables.   
   
## Règles importantes   
   
- Les numéros doivent rester visibles sur les actions.   
- Les recommandations ne doivent pas forcer la main.   
- Les attaques ne doivent être mises en avant que si une faiblesse est connue/débloquée.   
- Le heal peut être recommandé quand il est pertinent.   
- Une unique action sûre peut être mise en avant.   
- Le terminal reste obligatoire comme secours.   
   
## Jalons   
   
- V2.0.09 : palier IG jouable/stabilisé.   
- V3.0.0 : premier chapitre d'histoire.   
- Multijoueur en ligne : grosse version future, pas prioritaire.   
