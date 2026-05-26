#!/usr/bin/env bash
set -euo pipefail

VERSION=$(grep -R "return \"[0-9]\+\.[0-9]\+\.[0-9]\+\";" src/core/VersionInfo.cpp | head -1 | sed -E 's/.*return "([0-9]+\.[0-9]+\.[0-9]+)";.*/\1/')

if [[ -z "${VERSION}" ]]; then
    echo "Impossible de trouver la version dans src/core/VersionInfo.cpp" >&2
    exit 1
fi

echo "${VERSION}"
