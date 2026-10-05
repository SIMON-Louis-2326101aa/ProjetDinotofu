# Dinotofu — publication GitHub simple   

## Ce qui déclenche une release   

Le workflow `.github/workflows/release-dinotofu.yml` se lance automatiquement à chaque `git push` sur `main` ou `master`.   

Il lit la version courante dans `src/core/VersionInfo.cpp` :   

- si `vX.YY.ZZ` n'existe pas encore, il construit et publie cette nouvelle release ;   
- si le tag existe mais que la release ou certains assets manquent, il répare la release ;   
- si la release de cette version est déjà complète, il ne republie rien ;   
- si tu pushes des changements sans augmenter la version, il est donc normal qu'aucune nouvelle release ne soit créée.   

Le tag n'est plus poussé avant la compilation. Les builds Windows/Linux utilisent le commit qui a déclenché le workflow et le tag/release est créé seulement après des builds réussis. Cela évite les tags vides et réduit les problèmes de permissions/protection de tags.   

## Workflow conseillé pour publier   

Depuis un terminal à la racine du dépôt :   

```bash   
python3 scripts/bump_version.py patch   
# Le script demande aussi si saveVersion et/ou le checkpoint obligatoire doivent changer.   

git add .   
git commit -m "Dinotofu Vx.yy.zz"   
git push   
```   

Ou en une commande :   

```bash   
./scripts/release_push.sh patch "Description courte de la mise à jour"   
```   

Après le push, ouvre l'onglet **Actions** de GitHub et regarde le workflow **Build and Publish Dinotofu Releases**.   

## Forcer/réparer une release manuellement   

Méthode GitHub :   

1. ouvre le dépôt GitHub ;   
2. ouvre **Actions** ;   
3. choisis **Build and Publish Dinotofu Releases** ;   
4. clique **Run workflow** ;   
5. choisis la branche contenant la version voulue ;   
6. mets `force_release` sur `true` pour reconstruire/remplacer les assets même si le tag existe déjà.   

Avec GitHub CLI :   

```bash   
./scripts/trigger_release.sh   
```   

ou directement :   

```bash   
gh workflow run release-dinotofu.yml --ref main -f force_release=true   
```   

## Assets publiés   

Chaque release publie quatre assets : deux petits packs **Installer** destinés aux joueurs, plus deux payloads techniques conservés pour les installateurs, launchers et mises à jour existants.   

- `INSTALLER-DINOTOFU-WINDOWS-vX.YY.ZZ.zip` : contient uniquement `INSTALLER-DINOTOFU.cmd` + `Documentation/` en `.txt` ;   
- `INSTALLER-DINOTOFU-LINUX-vX.YY.ZZ.7z` : contient uniquement `INSTALLER-DINOTOFU.sh` + `Documentation/` en `.txt` ;   
- `Dinotofu-Windows-vX.YY.ZZ-TECHNICAL-PAYLOAD.zip` : payload technique Windows téléchargé par l'installateur/updater ;   
- `Dinotofu-Linux-vX.YY.ZZ-TECHNICAL-PAYLOAD.7z` : payload technique Linux téléchargé par l'installateur/updater.   

Le joueur doit normalement télécharger **INSTALLER-DINOTOFU-...**. Les payloads complets restent publiés pour ne pas casser les installations et launchers déjà distribués.   

## Si rien ne démarre après le push   

Vérifie dans GitHub :   

- que tu as bien poussé sur `main` ou `master` ;   
- que la version a réellement changé ;   
- que **Actions** est activé pour le dépôt ;   
- que le workflow apparaît dans l'onglet Actions ;   
- qu'aucune règle du dépôt n'interdit l'exécution ou la création de releases/tags.   

Puis utilise le déclenchement manuel ci-dessus pour voir immédiatement les logs d'erreur.   

## Documentation des versions   

Les README restent courts. L'historique détaillé doit continuer à vivre dans :   

- `CHANGELOG.md` ;   
- `CHANGELOG_FR.md`.   
