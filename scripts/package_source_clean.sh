#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

VERSION="$(./scripts/get_version.sh)"
PACKAGE_DIR="release_packages"
PACKAGE_NAME="dinotofu_source_${VERSION}.7z"
PACKAGE_PATH="${PACKAGE_DIR}/${PACKAGE_NAME}"

mkdir -p "${PACKAGE_DIR}"
rm -f "${PACKAGE_PATH}"

make clean >/dev/null 2>&1 || true

7z a -t7z -m0=lzma2 -mx=9 -ms=on "${PACKAGE_PATH}" . \
    -xr!.git \
    -xr!build \
    -xr!output \
    -xr!release_packages \
    -xr!gui_debug \
    -xr!tools/gui/run_gui_debug.sh \
    -xr!saves \
    -xr!accounts \
    -xr!characters \
    -xr!exported_accounts \
    -xr!import_accounts \
    -xr!*.exe \
    -xr!*.o \
    -xr!*.d \
    -xr!*.out \
    -xr!Dinotofu \
    -xr!dinotofu \
    -xr!*.log \
    -xr!*.tmp \
    -xr!AUDIT_IG_RESTANTS_* \
    -xr!AUDIT_DINOTOFU_* \
    -xr!SUGGESTIONS_DINOTOFU_APRES_* \
    -xr!reprise_chat_dinotofu_* \
    -xr!__pycache__ \
    -xr!*.pyc

echo "Archive source propre créée : ${PACKAGE_PATH}"
