#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
VERSION=$(grep -E 'return "[0-9]+\.[0-9]+\.[0-9]+";' "${ROOT_DIR}/src/core/VersionInfo.cpp" | head -1 | sed -E 's/.*return "([0-9]+\.[0-9]+\.[0-9]+)";.*/\1/')

if [[ -z "${VERSION}" ]]; then
    echo "Impossible de trouver la version dans ${ROOT_DIR}/src/core/VersionInfo.cpp" >&2
    exit 1
fi

echo "${VERSION}"
