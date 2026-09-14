#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "${ROOT_DIR}"

if [[ ! -x "./output/Dinotofu" ]]; then
    echo "Exécutable output/Dinotofu manquant, compilation en cours..."
    make -j"$(nproc 2>/dev/null || echo 2)"
fi

mkdir -p gui_debug

echo "Démarrage du serveur local d'aperçu IG sur le port 8787..."
python3 tools/gui/serve_gui_preview.py --root . --port 8787 &
SERVER_PID=$!

cleanup() {
    echo ""
    echo "Arrêt du serveur d'aperçu IG (PID ${SERVER_PID})..."
    kill "${SERVER_PID}" 2>/dev/null || true
    wait "${SERVER_PID}" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

sleep 0.5

PREVIEW_URL="http://127.0.0.1:8787/tools/gui/dinotofu_gui_experimental.html"

echo "============================================================"
echo " Dinotofu - Interface Graphique Expérimentale"
echo "============================================================"
echo " URL d'accès : ${PREVIEW_URL}"
echo " Ouvrez cette adresse dans votre navigateur."
echo " Les commandes et snapshots sont synchronisés avec le jeu."
echo "============================================================"
echo ""

if command -v wslview >/dev/null 2>&1; then
    wslview "${PREVIEW_URL}" >/dev/null 2>&1 &
elif [[ -n "${DISPLAY:-}" || -n "${WAYLAND_DISPLAY:-}" ]]; then
    if command -v xdg-open >/dev/null 2>&1; then
        xdg-open "${PREVIEW_URL}" >/dev/null 2>&1 &
    fi
fi

DINOTOFU_GUI_DEBUG_DIR=gui_debug DINOTOFU_GUI_INPUT_MODE=1 ./output/Dinotofu
