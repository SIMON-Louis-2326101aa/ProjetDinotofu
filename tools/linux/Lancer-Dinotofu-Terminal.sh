#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
trap 'exit 0' INT TERM
"${SCRIPT_DIR}/DinotofuLauncher.sh" --terminal "$@" || true
exit 0
