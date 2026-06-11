# Dinotofu — automatisation GitHub simple   

Ce document décrit comment publier Dinotofu depuis le dépôt source sans transformer les README en journal de versions.   

## Principe   

La version du jeu est lue depuis `src/core/VersionInfo.cpp`. Le manifeste de release doit rester aligné avec cette version dans `release/manifest.example.json`.   

Les notes détaillées vont dans `PATCHNOTE_DINOTOFU.md` en anglais et `PATCHNOTE_DINOTOFU_FR.md` en français. Les README restent centrés sur l'installation, la présentation du jeu et la note du développeur. Si GitHub Actions est activé sur le dépôt, un simple `git push` sur `main` ou `master` suffit : le workflow crée le tag de version et publie les ZIP Windows/Linux nécessaires.   

## Avant publication   

1. Vérifier la version dans le code et le manifeste.   
2. Compiler le projet.   
3. Lancer les contrôles de release.   
4. Nettoyer les dossiers générés.   
5. Créer le ZIP source propre.   
6. Faire `git push` sur `main` ou `master` et laisser GitHub Actions générer les assets Windows/Linux.   

## Commandes utiles   

```bash   
make -j4   
./output/Dinotofu --version   
./scripts/validate_release_tree.sh   
./scripts/package_source_clean.sh   
```   

## Publication avec simple git push   

Quand la version change dans `src/core/VersionInfo.cpp`, le push sur `main` ou `master` déclenche le workflow `.github/workflows/release-dinotofu.yml`.   

Le workflow :   

- lit la version du jeu ;   
- crée automatiquement le tag `vX.Y.Z` s'il n'existe pas ;   
- compile une release Linux ;   
- compile une release Windows via MinGW ;   
- publie ou répare la release GitHub ;   
- vérifie que les ZIP jouables Windows/Linux sont bien présents avant de considérer la release complète.   

Si une release existe mais ne contient pas les bons ZIP, elle est traitée comme incomplète et le workflow republie les assets nécessaires.   

## Règles importantes   

- Ne pas mettre d'historique de versions dans les README, guides ou fichiers de conception.   
- Utiliser `PATCHNOTE_DINOTOFU.md` pour les notes anglaises et `PATCHNOTE_DINOTOFU_FR.md` pour les notes françaises.   
- Ne jamais inclure `build/`, `output/`, `gui_debug/`, un exécutable ou un fichier de reprise local dans le ZIP source.   
- Garder la version actuelle visible dans les README, car les scripts de validation en ont besoin.   
