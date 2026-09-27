#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

if ! command -v gh >/dev/null 2>&1; then
    echo "GitHub CLI (gh) est requis pour déclencher manuellement le workflow." >&2
    echo "Alternative : GitHub > Actions > Build and Publish Dinotofu Releases > Run workflow." >&2
    exit 1
fi

if ! gh auth status >/dev/null 2>&1; then
    echo "GitHub CLI n'est pas authentifié. Lance : gh auth login" >&2
    exit 1
fi

BRANCH="${1:-}"
if [[ -z "$BRANCH" ]]; then
    BRANCH="$(git branch --show-current 2>/dev/null || true)"
fi
if [[ -z "$BRANCH" ]]; then
    BRANCH="main"
fi

VERSION="$(bash ./scripts/get_version.sh)"
echo "Déclenchement manuel de la release Dinotofu V${VERSION} sur ${BRANCH}..."
gh workflow run release-dinotofu.yml --ref "$BRANCH" -f force_release=true

echo "Workflow demandé. Pour suivre son exécution :"
echo "  gh run watch"
