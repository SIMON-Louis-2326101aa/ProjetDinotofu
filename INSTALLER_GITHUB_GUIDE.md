# Dinotofu — GitHub Releases, installers et launchers   

Version actuelle : **V2.0.03**   
Dernière version de personnage encore acceptable : **V1.32.03**   

## Objectif   

Les releases GitHub doivent fournir des ZIP propres pour Windows/Linux, ainsi que des installateurs capables d'installer ou mettre à jour Dinotofu sans demander au joueur de compiler le projet.   

## Fichiers générés attendus   

- `Dinotofu-Windows-vX.Y.Z.zip`   
- `Dinotofu-Linux-vX.Y.Z.zip`   
- `DinotofuInstaller-Windows-vX.Y.Z.zip`   
- `DinotofuInstaller-Linux-vX.Y.Z.zip`   

## Raccourcis/lanceurs visibles Windows/Linux   

Côté joueur, éviter les doublons. Sur Windows comme sur Linux, il faut seulement deux entrées claires :   

- **ProjetDinotofu Launcher** : lancement normal / Auto via le launcher de l'OS ;   
- **ProjetDinotofu Launcher Terminal version** : terminal forcé via le launcher de l'OS.   

Les `.cmd` et scripts internes peuvent exister dans le dossier du jeu, mais ils ne doivent pas créer 36 entrées visibles pour le joueur.   

## Versioning   

La source de vérité est :   

```text   
src/core/VersionInfo.cpp   
```   

Le manifeste d'exemple doit suivre la même version :   

```text   
release/manifest.example.json   
```   

## Contrôle avant partage   

Avant de publier ou envoyer une archive :   

```bash   
make clean   
make release-check   
```   

Le ZIP source local de développement ne doit pas contenir :   

- `build/`   
- `output/`   
- `.exe`   
- `.o`, `.d`, `.out`   
- `gui_debug/`   
- sauvegardes/comptes/personnages privés   
- `__pycache__/` ou `.pyc`   
- fichiers locaux de reprise de chat ou d'audit temporaire   

## Jalons majeurs   

- V2.0.03 : palier IG jouable/stabilisé.   
- V3.0.0 : premier chapitre d'histoire codé.   
- Multijoueur en ligne : future grosse version, beaucoup plus tard.   
