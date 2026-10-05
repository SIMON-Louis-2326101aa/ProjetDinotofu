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

Cette page n’est plus une tentative d’interface jouable. Elle sert uniquement d’accueil graphique léger pendant la reconstruction de la vraie application desktop.   

- **Jouer** affiche l’état « interface graphique en production ».   
- **Arrêter et passer à la version terminale** envoie une demande locale au launcher.   
- Le launcher coupe le petit serveur d’accueil puis démarre la version terminale.   
- Aucun moteur C++ de partie ne tourne en arrière-plan pendant l’attente sur l’accueil.   
- Si le serveur local n’est pas disponible, la page indique d’utiliser le raccourci Terminal.   

Le serveur conserve temporairement ses anciens endpoints de debug/snapshot pour compatibilité avec les outils de développement, mais l’accueil public ne les utilise plus.   
