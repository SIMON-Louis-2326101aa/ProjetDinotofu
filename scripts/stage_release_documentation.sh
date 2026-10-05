#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 ]]; then
    echo "Usage: $0 <destination> <Windows|Linux> [payload|installer]" >&2
    exit 2
fi

DEST="$1"
PLATFORM="$2"
PACKAGE_KIND="${3:-payload}"
mkdir -p "$DEST"

cp READMEFR.md "$DEST/README_FR.txt"
cp README.md "$DEST/README_EN.txt"
cp CHANGELOG_FR.md "$DEST/CHANGELOG_FR.txt"
cp CHANGELOG.md "$DEST/CHANGELOG_EN.txt"
cp CHEATS_DINOTOFU.txt "$DEST/CHEATS_DINOTOFU.txt" 2>/dev/null || true

if [[ "$PACKAGE_KIND" == "installer" ]]; then
    cat > "$DEST/LISEZ-MOI.txt" <<TXT
============================================================
 DINOTOFU - PACK INSTALLATEUR ${PLATFORM}
============================================================

Ce pack ne contient PAS le jeu complet.
Il contient uniquement l'installateur et cette documentation texte.

L'installateur recupere ensuite le payload technique correspondant a la
derniere release GitHub et installe/met a jour ProjetDinotofu en preservant
les donnees joueur prevues par l'installateur.

Pour le moment, le mode TERMINAL est le mode de lancement par defaut.
L'interface graphique est conservee uniquement pour le developpement et sera
refaite plus tard.
TXT

    cat > "$DEST/INSTALLATION.txt" <<TXT
============================================================
 DINOTOFU - INSTALLATION ${PLATFORM}
============================================================

${PLATFORM} : lance le fichier INSTALLER-DINOTOFU situe a la racine de ce pack.

Le jeu lui-meme n'est volontairement pas inclus dans ce pack installateur.
Une connexion Internet est necessaire pour recuperer la release technique,
sauf si un payload compatible est deja disponible localement et detecte par
l'installateur.

Mode de jeu par defaut apres installation : Terminal.
Interface graphique : en cours de developpement, non recommandee pour jouer.
TXT
else
    cat > "$DEST/LISEZ-MOI.txt" <<TXT
============================================================
 DINOTOFU - PAYLOAD TECHNIQUE ${PLATFORM}
============================================================

Cette archive est le payload technique utilise par les installateurs et les
anciens launchers/updaters. Pour une nouvelle installation, telecharge plutot
l'archive dont le nom commence par INSTALLER-DINOTOFU.
TXT

    cat > "$DEST/INSTALLATION.txt" <<TXT
============================================================
 DINOTOFU - PAYLOAD TECHNIQUE ${PLATFORM}
============================================================

Cette archive reste publiee pour la compatibilite des installations existantes.
Pour une nouvelle installation, utilise le pack INSTALLER-DINOTOFU correspondant a ton systeme.

Le mode TERMINAL est maintenant le mode de lancement par defaut.
L'interface graphique est encore en cours de developpement.
TXT
fi
