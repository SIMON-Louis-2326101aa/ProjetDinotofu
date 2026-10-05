# Dinotofu — GitHub Releases, installers et launchers   

Ce guide explique comment distribuer Dinotofu sans demander au joueur de compiler le projet.   


## Distribution simplifiée par OS   

Une release contient quatre fichiers. Les deux fichiers à conseiller aux joueurs commencent par **INSTALLER-DINOTOFU** :   

- `INSTALLER-DINOTOFU-WINDOWS-vX.YY.ZZ.zip` : uniquement l'installateur Windows et un dossier `Documentation/` contenant des `.txt` ;   
- `INSTALLER-DINOTOFU-LINUX-vX.YY.ZZ.7z` : uniquement l'installateur Linux et un dossier `Documentation/` contenant des `.txt` ;   
- `Dinotofu-Windows-vX.YY.ZZ-TECHNICAL-PAYLOAD.zip` et `Dinotofu-Linux-vX.YY.ZZ-TECHNICAL-PAYLOAD.7z` : payloads techniques complets téléchargés par l'installateur/updater et conservés pour la compatibilité.   

Le pack **Installer** ne contient donc plus le jeu lui-même. L'installateur récupère le payload correspondant depuis la release GitHub.   

## Jouer ou installer depuis une release GitHub   

1. ouvrir la dernière Release GitHub ;   
2. télécharger `INSTALLER-DINOTOFU-WINDOWS-v*.zip` ou `INSTALLER-DINOTOFU-LINUX-v*.7z` ;   
3. décompresser ce petit pack ;   
4. lancer `INSTALLER-DINOTOFU.cmd` sous Windows ou `INSTALLER-DINOTOFU.sh` sous Linux ;   
5. l'installateur télécharge et installe le payload technique du jeu.   

## Lanceur   

Le lancement normal utilise désormais le **Terminal par défaut**, car il s'agit de l'interface stable. Le mode graphique reste accessible explicitement pour le développement mais n'est plus proposé comme choix normal par défaut tant que sa refonte n'est pas terminée.   

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
