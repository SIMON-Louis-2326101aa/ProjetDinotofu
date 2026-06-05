# Dinotofu — automatisation GitHub simple   

Ce document décrit comment préparer une publication GitHub propre sans transformer les README en journal de versions.   

## Principe   

La version du jeu est lue depuis `src/core/VersionInfo.cpp`. Le manifeste de release doit rester aligné avec cette version dans `release/manifest.example.json`.   

Les notes détaillées vont dans `PATCHNOTE_DINOTOFU.md`. Les README restent centrés sur l'installation, la présentation du jeu et la note du développeur.   

## Avant publication   

1. Vérifier la version dans le code et le manifeste.   
2. Compiler le projet.   
3. Lancer les contrôles de release.   
4. Nettoyer les dossiers générés.   
5. Créer le ZIP source propre.   
6. Publier sur GitHub avec les assets Windows/Linux prévus.   

## Commandes utiles   

```bash   
make -j4   
./output/Dinotofu --version   
./scripts/validate_release_tree.sh   
./scripts/package_source_clean.sh   
```   

## Règles importantes   

- Ne pas mettre d'historique de versions dans les README, guides ou fichiers de conception.   
- Utiliser `PATCHNOTE_DINOTOFU.md` pour les notes de patch.   
- Ne jamais inclure `build/`, `output/`, `gui_debug/`, un exécutable ou un fichier de reprise local dans le ZIP source.   
- Garder la version actuelle visible dans les README, car les scripts de validation en ont besoin.   
