#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 ]]; then
    echo "Usage: $0 <destination> <Windows|Linux>" >&2
    exit 2
fi

DEST="$1"
PLATFORM="$2"
mkdir -p "$DEST"

cp READMEFR.md "$DEST/README_FR.txt"
cp README.md "$DEST/README_EN.txt"
cp CHANGELOG_FR.md "$DEST/CHANGELOG_FR.txt"
cp CHANGELOG.md "$DEST/CHANGELOG_EN.txt"
cp CHEATS_DINOTOFU.txt "$DEST/CHEATS_DINOTOFU.txt" 2>/dev/null || true

cat > "$DEST/INSTALLATION.txt" <<TXT
============================================================
 DINOTOFU - INSTALLATION ${PLATFORM}
============================================================

Le fichier d'installation se trouve a la racine du pack.
Il telecharge automatiquement la derniere archive de jeu compatible depuis GitHub,
puis installe/met a jour ProjetDinotofu en preservant les donnees joueur connues.

Les archives nommees Dinotofu-${PLATFORM}-vX.YY.ZZ.7z publiees sur GitHub sont les
payloads techniques utilises par l'installateur et l'updater. Pour une installation
normale, telecharge de preference le pack Installer-Dinotofu-${PLATFORM}-vX.YY.ZZ.7z.

En cas de grosse mise a jour de sauvegarde, le jeu peut imposer un checkpoint avec
backup de securite avant adaptation. Ne supprime pas manuellement les dossiers de
sauvegarde pendant une mise a jour.
TXT
