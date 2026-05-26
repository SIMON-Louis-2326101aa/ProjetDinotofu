#!/usr/bin/env bash
set -euo pipefail

# Prépare une release Dinotofu en local : bump version, commit, push.
# GitHub Actions créera ensuite le tag et la release automatiquement.
#
# Exemples :
#   ./scripts/release_push.sh patch
#   ./scripts/release_push.sh minor "Nouveau système de magie"
#   ./scripts/release_push.sh 1.33.00 "Début interface graphique"

BUMP_MODE="${1:-patch}"
COMMIT_MESSAGE="${2:-}"

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    echo "Ce script doit être lancé dans un dépôt Git." >&2
    exit 1
fi

python3 scripts/bump_version.py "${BUMP_MODE}"
VERSION="$(./scripts/get_version.sh)"

if [[ -z "${COMMIT_MESSAGE}" ]]; then
    COMMIT_MESSAGE="Patch Dinotofu ${VERSION}"
fi

git add .
if git diff --cached --quiet; then
    echo "Aucun changement à commiter."
    exit 0
fi

git commit -m "${COMMIT_MESSAGE}"
git push

cat <<TXT

Push terminé.
Si GitHub Actions est activé, le workflow va :
- lire la version ${VERSION},
- créer le tag v${VERSION} s'il n'existe pas,
- publier les ZIP Windows/Linux + installers dans GitHub Releases.
TXT
