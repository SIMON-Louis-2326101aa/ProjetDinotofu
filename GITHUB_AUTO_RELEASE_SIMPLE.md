# Dinotofu — automatisation simple GitHub

## Comment GitHub crée les tags/releases tout seul ?

GitHub ne devine pas magiquement ton jeu. Il lit le fichier :

```text
.github/workflows/release-dinotofu.yml
```

Ce fichier dit à GitHub Actions :

1. quand un push arrive sur `main` ou `master`, lance le workflow ;
2. lis la version dans `src/core/VersionInfo.cpp` ;
3. construis le tag attendu, par exemple `v1.35.10` ;
4. si ce tag n'existe pas, crée-le ;
5. compile les versions Linux et Windows ;
6. prépare les packs installer ;
7. publie tout dans GitHub Releases.

La version dans le code est donc la source de vérité.

## Ce que tu dois configurer sur GitHub une seule fois

Sur ton repo GitHub :

1. Va dans `Settings > Actions > General`.
2. Vérifie que les actions sont autorisées.
3. Dans `Workflow permissions`, choisis `Read and write permissions`.
4. Garde le repo public si tu veux que tes amis téléchargent sans compte GitHub.

Après ça, tant que le fichier `.github/workflows/release-dinotofu.yml` est dans ton repo, GitHub s'occupe du reste.

## Méthode normale depuis ton PC

Pour publier un simple patch :

```bash
./scripts/release_push.sh patch
```

Pour une mise à jour moyenne :

```bash
./scripts/release_push.sh minor "Grosse amélioration du bestiaire"
```

Pour une grosse phase :

```bash
./scripts/release_push.sh major "Nouvelle phase Dinotofu"
```

Le script fait :

- changement de version ;
- `git add .` ;
- `git commit` ;
- `git push`.

Ensuite GitHub Actions crée automatiquement le tag et la release.

## Méthode Windows PowerShell

Depuis PowerShell, dans le dossier du repo :

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\windows\Publier-DinotofuPatch.ps1
```

Pour une version moyenne :

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\windows\Publier-DinotofuPatch.ps1 -Bump minor -Message "Grosse amélioration du jeu"
```

## Ce que tu donnes aux amis

Dans la page GitHub Release générée automatiquement :

- Windows : `DinotofuInstaller-Windows-vX.Y.Z.zip`
- Linux : `DinotofuInstaller-Linux-vX.Y.Z.zip`

Ils n'ont pas besoin de Visual Studio, Make, WSL ou Git.


## Vérifier que le ZIP du jeu reste propre

Avant d'envoyer un ZIP source ou de préparer une release locale, tu peux lancer :

```bash
make release-check
```

Ce contrôle vérifie notamment qu'il n'y a pas de vieux lanceur racine, pas de fichiers d’authentification locaux, pas de saves privées, pas de build/output et pas d'exécutable dans le projet source.
