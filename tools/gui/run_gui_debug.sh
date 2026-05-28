#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "${ROOT_DIR}"

GUI_DIR="${ROOT_DIR}/gui_debug"
PORT="${DINOTOFU_GUI_PREVIEW_PORT:-8787}"

mkdir -p "${GUI_DIR}"
make -j"$(nproc 2>/dev/null || echo 2)"

SERVER_PID=""
if command -v python3 >/dev/null 2>&1; then
    python3 tools/gui/serve_gui_preview.py --root "${ROOT_DIR}" --port "${PORT}" --gui-debug-dir "${GUI_DIR}" &
    SERVER_PID="$!"
    trap 'if [ -n "${SERVER_PID}" ]; then kill "${SERVER_PID}" >/dev/null 2>&1 || true; fi' EXIT
    echo ""
    echo "Interface graphique experimentale : http://127.0.0.1:${PORT}/tools/gui/dinotofu_gui_experimental.html"
    echo "Le jeu va exporter ses derniers ecrans dans : ${GUI_DIR}"
    if command -v xdg-open >/dev/null 2>&1; then xdg-open "http://127.0.0.1:${PORT}/tools/gui/dinotofu_gui_experimental.html" >/dev/null 2>&1 || true; fi
    echo ""
else
    echo "Python3 introuvable. Ouvre tools/gui/dinotofu_gui_preview.html et charge les JSON manuellement."
fi

DINOTOFU_GUI_DEBUG_DIR="${GUI_DIR}" DINOTOFU_GUI_INPUT_MODE=1 DINOTOFU_GUI_INPUT_FILE="${GUI_DIR}/pending_input.txt" DINOTOFU_GUI_INPUT_QUEUE_DIR="${GUI_DIR}/input_queue" ./output/Dinotofu
