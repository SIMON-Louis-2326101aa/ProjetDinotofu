# Dinotofu — GitHub Releases, installers et launchers   

Ce guide explique comment distribuer Dinotofu sans demander au joueur de compiler le projet.   


## Distribution simplifiée par OS   

Pour éviter toute confusion (« trop de fichiers cliquables » dans la release), chaque release GitHub publie désormais un unique package complet par système d'exploitation :   

- `Dinotofu-Windows-vX.YY.ZZ.zip` : package Windows au format ZIP natif (compression Deflate maximale), directement décompressable sur Windows 10 et 11 sans nécessiter 7-Zip ou logiciel tiers ;   
- `Dinotofu-Linux-vX.YY.ZZ.7z` : package Linux au format 7z (compression maximale LZMA2).   

Chaque package contient l'exécutable, les scripts de lancement direct et l'installateur optionnel pour configurer les raccourcis.   


## Jouer ou installer depuis une release GitHub   

Pour jouer à Dinotofu sans compiler le projet manuellement :   

1. aller sur la page du dépôt GitHub ;   
2. ouvrir la dernière Release affichée à droite du dépôt ;   
3. télécharger l’archive correspondant à ton système : `Dinotofu-Windows-v*.zip` ou `Dinotofu-Linux-v*.7z` ;   
4. décompresser l’archive où tu veux ;   
5. lancer directement le jeu via **Lancer-Dinotofu** (ou l'exécutable) !   
6. si tu souhaites installer le jeu dans ton dossier utilisateur et créer un raccourci Bureau, lance **Installer-Dinotofu**. L'installateur te demandera confirmation avant d'ajouter le raccourci.   

## Lanceur unifié et raccourci unique   

Sur le Bureau, le jeu ne crée plus qu'un **seul et unique raccourci** pour éviter toute surcharge visuelle :   

- **ProjetDinotofu Launcher** : au lancement, une invite interactive te propose de choisir le mode :   
  - `1. Interface Graphique (GUI / Navigateur web)`   
  - `2. Mode Terminal (Classique dans la console)`   

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
