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

Le fichier d'installation et les lanceurs se trouvent a la racine du dossier.

Pour jouer directement sans installer :
- Windows : double-clique sur Lancer-Dinotofu.cmd
- Linux   : execute ./Lancer-Dinotofu.sh

Le lanceur vous demande au demarrage de choisir :
  1. Interface Graphique (GUI / Navigateur web)
  2. Mode Terminal (Classique dans la console)

Pour installer le jeu dans votre profil et configurer un raccourci Bureau :
- Windows : execute Installer-Dinotofu.cmd
- Linux   : execute ./Installer-Dinotofu.sh

L'installateur vous demande si vous voulez creer un raccourci sur le Bureau.
En cas de mise a jour, vos donnees joueur (sauvegardes, comptes) sont preservees.
TXT
