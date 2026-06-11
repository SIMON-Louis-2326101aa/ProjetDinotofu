#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

VERSION="$(./scripts/get_version.sh)"
PACKAGE_DIR="release_packages"
PACKAGE_NAME="dinotofu_source_${VERSION}.zip"
PACKAGE_PATH="${PACKAGE_DIR}/${PACKAGE_NAME}"

mkdir -p "${PACKAGE_DIR}"
rm -f "${PACKAGE_PATH}"

make clean >/dev/null 2>&1 || true

zip -r "${PACKAGE_PATH}" . \
    -x ".git/*" \
    -x "build/*" \
    -x "output/*" \
    -x "release_packages/*" \
    -x "gui_debug/*" \
    -x "assets/saves/*" \
    -x "saves/*" \
    -x "accounts/*" \
    -x "characters/*" \
    -x "exported_accounts/*" \
    -x "import_accounts/*" \
    -x "*.exe" \
    -x "*.o" \
    -x "*.d" \
    -x "*.out" \
    -x "Dinotofu" \
    -x "dinotofu" \
    -x "*.log" \
    -x "*.tmp" \
    -x "AUDIT_IG_RESTANTS_*.md" \
    -x "AUDIT_DINOTOFU_*.md" \
    -x "SUGGESTIONS_DINOTOFU_APRES_*.md" \
    -x "reprise_chat_dinotofu_*.txt" \
    -x "*/__pycache__/*" \
    -x "*.pyc"

echo "Archive source propre créée : ${PACKAGE_PATH}"
