# Dinotofu — automatisation GitHub simple   

Version actuelle : **V2.01.04**   
Dernière version de personnage encore acceptable : **V2.01.03**   

## Principe   

Quand le code est poussé sur GitHub, le workflow lit la version dans :   

```text   
src/core/VersionInfo.cpp   
```   

Il prépare ensuite les releases et assets autour du tag correspondant, par exemple :   

```text   
v2.01.04   
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
./scripts/package_source_clean.sh   
```   

## Archives generees   

Les releases/installers Windows/Linux sont les seules archives de diffusion GitHub. Le ZIP source sert seulement au développement local.   

## Règles importantes   

- Pas de sauvegardes privées dans les ZIP.   
- Le ZIP source local reste propre et n’est pas publié comme asset GitHub.   
- Pas de long historique de versions dans les docs joueur.   
- Garder seulement la version actuelle, la compatibilité personnage, les gros jalons et les consignes utiles.   

## Jalons   

- V2.01.03 : persistance plus fine des légendes déjà lues et dialogues PNJ/guilde.   
- V3.00.00 : premier chapitre d'histoire.   
- Multijoueur en ligne : future grosse version beaucoup plus tard.   
