# Dinotofu — automatisation GitHub simple   

Version actuelle : **V2.0.0**   
Dernière version de personnage encore acceptable : **V1.32.03**   

## Principe   

Quand le code est poussé sur GitHub, le workflow lit la version dans :   

```text   
src/core/VersionInfo.cpp   
```   

Il prépare ensuite les releases et assets autour du tag correspondant, par exemple :   

```text   
v2.0.0   
```   

## Ce que tu dois faire   

1. Modifier le code.   
2. Mettre à jour la version si la passe le mérite.   
3. Lancer les contrôles locaux.   
4. Commit/push.   
5. Laisser GitHub Actions générer les fichiers de release.   

## Commandes utiles   

```bash   
make clean   
make release-check   
```   

Pour créer une archive source propre :   

```bash   
./scripts/package_source_no_exe.sh   
```   

## Ce que tu donnes aux joueurs   

Donner plutôt les releases/installers Windows/Linux selon leur OS. Le ZIP source sert surtout au développement.   

## Règles importantes   

- Pas de sauvegardes privées dans les ZIP.   
- Pas d'exécutable dans le ZIP source.   
- Pas de long historique de versions dans les docs joueur.   
- Garder seulement la version actuelle, la compatibilité personnage, les gros jalons et les consignes utiles.   

## Jalons   

- V2.0.0 : palier IG jouable/stabilisé.   
- V3.0.0 : premier chapitre d'histoire.   
- Multijoueur en ligne : future grosse version beaucoup plus tard.   
