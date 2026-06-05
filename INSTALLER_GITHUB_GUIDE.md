# Dinotofu — GitHub Releases, installers et launchers   

Ce guide explique comment distribuer Dinotofu sans demander au joueur de compiler le projet.   


## Installer depuis une release GitHub   

Pour installer Dinotofu sans compiler le projet manuellement :   

1. aller sur la page du dépôt GitHub ;   
2. ouvrir la dernière Release affichée à droite du dépôt ;   
3. télécharger le launcher correspondant à ton système d'exploitation : Windows ou Linux ;   
4. lancer ce launcher, qui s'occupe ensuite d'ouvrir la bonne version du jeu.   

Sur les versions desktop Windows/Linux, l'installateur ou le launcher doit créer deux entrées claires :   

- **ProjetDinotofu Launcher** : lancement normal / Auto, via le launcher adapté à l'OS ;   
- **ProjetDinotofu Launcher Terminal version** : lancement forcé en terminal, via le launcher Windows/Linux correspondant.   

## Fichiers générés attendus   

Les releases GitHub doivent fournir des archives propres pour Windows et Linux, ainsi que les launchers/installateurs correspondants.   

Les raccourcis visibles doivent rester clairs :   

- **ProjetDinotofu Launcher** : lancement normal / Auto ;   
- **ProjetDinotofu Launcher Terminal version** : lancement forcé en terminal.   

## Logique de version   

La logique reste simple :   

- correction simple : patch ;   
- ajout de contenu ou système compatible : version intermédiaire ;   
- gros jalon qui change fortement la base du jeu : version majeure ou nouvelle base importante.   

La base actuelle de recréation conseillée reste **V3.00.00**.   

## Contrôle avant partage   

```bash   
make -j4   
./output/Dinotofu --version   
./scripts/validate_release_tree.sh   
./scripts/package_source_clean.sh   
```   

Le ZIP source ne doit pas contenir `build/`, `output/`, d'exécutable, de cache local, de fichier de reprise ou de données privées de sauvegarde.   
