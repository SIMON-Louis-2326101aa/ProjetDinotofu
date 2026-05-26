# Dinotofu — GitHub Releases, installers et launcher automatique

Objectif : tu pushes ton code sur GitHub, GitHub Actions prépare les ZIP, crée le tag/release si la version a changé, et tes potes n'ont qu'un petit pack installer à lancer. Ils n'ont pas besoin de Visual Studio, Make, WSL ou d'un compilateur.

## Ce que contient cette version

```text
.github/workflows/release-dinotofu.yml
scripts/get_version.sh
scripts/package_source_no_exe.sh
scripts/package_linux_release.sh
scripts/package_linux_installer.sh
scripts/package_windows_release.sh
scripts/validate_release_tree.sh
tools/windows/DinotofuInstaller.ps1
tools/windows/DinotofuLauncher.ps1
tools/windows/Installer-Dinotofu.cmd
tools/windows/Lancer-Dinotofu.cmd
tools/windows/dinotofu-installer.config.example.json
tools/linux/DinotofuInstaller.sh
tools/linux/DinotofuLauncher.sh
tools/linux/Installer-Dinotofu.sh
tools/linux/Lancer-Dinotofu.sh
tools/linux/dinotofu-installer.config.example.json
```

## Fonctionnement automatique

Le workflow lit la version directement dans :

```text
src/core/VersionInfo.cpp
```

Si tu pushes sur `main` ou `master` et que la version du code est par exemple `1.35.10`, le workflow cherche le tag :

```text
v1.35.10
```

- Si le tag n'existe pas, il le crée automatiquement et publie la release.
- Si le tag existe déjà, il ne refait pas une release inutile.
- Depuis l'onglet Actions, tu peux lancer le workflow manuellement avec `force_release=true` si tu veux réécrire les fichiers de la release existante.

Donc le flux normal devient :

```bash
git add .
git commit -m "Patch Dinotofu 1.35.10"
git push
```

Et c'est tout, à condition d'avoir augmenté la version dans le code.

## Fichiers générés par la release

GitHub Actions fabrique automatiquement :

```text
dinotofu_source_X.Y.Z_no_exe.zip
Dinotofu-Linux-vX.Y.Z.zip
DinotofuInstaller-Linux-vX.Y.Z.zip
Dinotofu-Windows-vX.Y.Z.zip
DinotofuInstaller-Windows-vX.Y.Z.zip
```

## Ce que tu envoies à tes potes

### Pour Windows

Tu leur donnes seulement :

```text
DinotofuInstaller-Windows-vX.Y.Z.zip
```

Ils extraient le ZIP, puis lancent :

```text
Installer-Dinotofu.cmd
```

L'installer demande l'emplacement parent. Par défaut, il installe dans `Downloads/ProjetDinotofu`. Même si le joueur choisit un autre emplacement, le dossier final s'appelle toujours `ProjetDinotofu`. La version Windows n'utilise pas WSL : elle lance la release précompilée `Dinotofu.exe`.

### Pour Linux

Tu leur donnes seulement :

```text
DinotofuInstaller-Linux-vX.Y.Z.zip
```

Ils extraient le ZIP, ouvrent un terminal dans le dossier, puis lancent :

```bash
./Installer-Dinotofu.sh
```

L'installer demande l'emplacement parent. Par défaut, il installe dans `Downloads/ProjetDinotofu`. Même si le joueur choisit un autre emplacement, le dossier final s'appelle toujours `ProjetDinotofu`.

Le script Linux utilise généralement des outils déjà présents ou faciles à installer : `bash`, `curl`, `unzip`, `python3`.

## Mise à jour automatique

Après installation, les raccourcis lancent le launcher.

À chaque lancement, le launcher :

1. lit la version locale dans `version.txt` ;
2. regarde la dernière GitHub Release ;
3. télécharge la nouvelle version si elle existe ;
4. sauvegarde temporairement les sauvegardes/personnages ;
5. remplace les fichiers du jeu ;
6. restaure les données joueur ;
7. lance le jeu.

## Est-ce qu'il faut toucher GitHub ?

Oui, mais très peu.

À faire une seule fois :

1. Mettre le workflow dans le repo : `.github/workflows/release-dinotofu.yml`.
2. Vérifier que GitHub Actions est activé dans l'onglet `Actions`.
3. Vérifier que le repo a le droit d'écrire des releases : `Settings > Actions > General > Workflow permissions > Read and write permissions`.
4. Mettre le repo en public si tu veux que les installers téléchargent sans connexion GitHub.

Ensuite, au quotidien :

1. Tu modifies le code.
2. Tu augmentes la version dans `VersionInfo.cpp`.
3. Tu fais `git push` sur `main` ou `master`.
4. GitHub crée le tag/release automatiquement si ce numéro n'existait pas encore.

## Commandes développeur utiles

### ZIP source sans exécutable

```bash
make package-source
```

### Release Linux locale

```bash
make package-linux-release
```

### Pack installer Linux local

```bash
make package-linux-installer
```

### Release Windows locale depuis Linux/WSL avec MinGW

```bash
make package-windows-release
```

Pour cette commande locale, il faut `x86_64-w64-mingw32-g++`. Sur GitHub Actions, le workflow l'installe automatiquement avec `mingw-w64`.

## Notes importantes

- Le repo peut être public pour simplifier le téléchargement sans token.
- Si le repo est privé, l'installer public ne pourra pas télécharger les releases sans authentification GitHub.
- Les sauvegardes, comptes et personnages privés restent exclus des ZIP par les scripts et le `.gitignore`.
- Le ZIP source que je te renvoie dans ChatGPT reste sans exécutable, comme demandé.
- Si tu modifies le code sans augmenter la version, le workflow verra que le tag existe déjà et ne publiera pas une nouvelle release.

## Version encore plus automatique ajoutée en V1.32.36

Deux helpers ont été ajoutés pour éviter de faire les commandes Git à la main à chaque patch.

### Linux / WSL / Git Bash

```bash
./scripts/release_push.sh patch
```

Ce script augmente la version patch, commit et push. GitHub Actions crée ensuite le tag et la release automatiquement.

### Windows PowerShell

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\windows\Publier-DinotofuPatch.ps1
```

Ce script fait la même chose côté Windows.

Important : GitHub ne peut pas savoir seul si une modification mérite X, Y ou Z dans `X.Y.Z`. C'est encore toi qui décides si c'est `patch`, `minor` ou `major`, ou tu donnes directement une version comme `1.33.00`.


## Contrôle avant partage

Pour éviter de remettre par erreur des fichiers privés ou inutiles dans le ZIP source :

```bash
make release-check
```

Ce contrôle refuse notamment les exécutables, saves privées, anciens lanceurs racine et fichiers/outils authentification locale personnels.
