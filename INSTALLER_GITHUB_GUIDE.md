# Dinotofu — GitHub Releases, installers et launchers   

Ce guide explique comment distribuer Dinotofu sans demander au joueur de compiler le projet.   


## Jouer ou installer depuis une release GitHub   

Pour jouer à Dinotofu sans compiler le projet manuellement :   

1. aller sur la page du dépôt GitHub ;   
2. ouvrir la dernière Release affichée à droite du dépôt ;   
3. télécharger l’archive `.7z` correspondant à ton système : **Dinotofu-Windows** ou **Dinotofu-Linux** (format `.7z`, compression maximale LZMA2) ;   
4. décompresser l’archive où tu veux ;   
5. lancer directement le jeu via **Lancer-Dinotofu** (ou l'exécutable) !   
6. si tu souhaites créer des raccourcis sur ton bureau, lance **Installer-Dinotofu** inclus directement dans le dossier du jeu.   

Sur les versions desktop Windows/Linux, le launcher ou l'installateur crée deux entrées claires :   

- **ProjetDinotofu Launcher** : lancement normal / Auto, via le launcher adapté à l'OS ;   
- **ProjetDinotofu Launcher Terminal version** : lancement forcé en terminal, via le launcher Windows/Linux correspondant.   

## Fichiers générés attendus   

Les releases GitHub exposent exactement les deux archives portables au format `.7z` (compression maximale LZMA2) :   
- **Dinotofu-Windows-vX.YY.ZZ.7z** : version portable Windows tout-en-un ;   
- **Dinotofu-Linux-vX.YY.ZZ.7z** : version portable Linux tout-en-un.   

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

L'archive source ne doit pas contenir `build/`, `output/`, d'exécutable, de cache local, de fichier de reprise ou de données privées de sauvegarde.   
