#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

VERSION="$(bash ./scripts/get_version.sh)"
OUTPUT_PATH="${1:-release_packages/dinotofu_backup_essentiel_${VERSION}.7z}"

if ! command -v 7z >/dev/null 2>&1; then
    echo "Erreur : 7z est requis pour créer le backup essentiel." >&2
    exit 1
fi

mkdir -p "$(dirname "${OUTPUT_PATH}")"
rm -f "${OUTPUT_PATH}"

# Important: only runtime/private SAVE DATA folders are excluded here.
# Code modules include/save and src/save MUST remain in the archive.
7z a -t7z -m0=lzma2 -mx=9 -ms=on "${OUTPUT_PATH}" . \
    -xr!.git \
    -xr!build \
    -xr!output \
    -xr!logs \
    -xr!release_packages \
    -xr!gui_debug \
    -xr!saves \
    -xr!accounts \
    -xr!characters \
    -xr!exported_accounts \
    -xr!import_accounts \
    -xr!__pycache__ \
    -xr!*.pyc \
    -xr!*.o \
    -xr!*.d \
    -xr!*.exe \
    -xr!*.out \
    -xr!*.log \
    -xr!*.tmp \
    -xr!*.png \
    -xr!*.jpg \
    -xr!*.jpeg \
    -xr!*.webp \
    -xr!*.ico \
    -xr!*.svg \
    -xr!reprise_chat_dinotofu_*

echo "Backup essentiel créé : ${OUTPUT_PATH}"
