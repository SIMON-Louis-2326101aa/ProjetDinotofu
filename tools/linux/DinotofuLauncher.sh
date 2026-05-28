#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_FILE="${SCRIPT_DIR}/dinotofu-installer.config.json"
REPO="${DINOTOFU_REPO:-}"
ASSET_PATTERN="${DINOTOFU_ASSET_PATTERN:-Dinotofu-Linux-v*.zip}"
INSTALL_DIR="$SCRIPT_DIR"
NO_UPDATE="false"
LAUNCH_MODE="auto"
INSTALL_DIR_FROM_ARG="false"

for arg in "$@"; do
    case "$arg" in
        --no-update) NO_UPDATE="true" ;;
        --terminal|--mode=terminal) LAUNCH_MODE="terminal" ;;
        --gui|--mode=gui) LAUNCH_MODE="gui" ;;
        --install-dir=*) INSTALL_DIR="${arg#--install-dir=}"; INSTALL_DIR_FROM_ARG="true" ;;
    esac
done

read_config_value() {
    local key="$1"
    [[ -f "$CONFIG_FILE" ]] || return 0
    python3 - "$CONFIG_FILE" "$key" <<'PY' 2>/dev/null || true
import json, sys
with open(sys.argv[1], encoding='utf-8') as f:
    data = json.load(f)
value = data.get(sys.argv[2], '')
print(value if value is not None else '')
PY
}

normalize_project_dir() {
    local path_text="$1"
    path_text="${path_text/#\~/$HOME}"
    path_text="${path_text%/}"
    if [[ "$(basename "$path_text")" == "ProjetDinotofu" ]]; then
        echo "$path_text"
    else
        echo "${path_text}/ProjetDinotofu"
    fi
}

if [[ -z "$REPO" ]]; then REPO="$(read_config_value repo)"; fi
configured_pattern="$(read_config_value assetPattern)"
[[ -z "$configured_pattern" ]] || ASSET_PATTERN="$configured_pattern"
if [[ "$INSTALL_DIR_FROM_ARG" == "true" ]]; then
    INSTALL_DIR="$(normalize_project_dir "$INSTALL_DIR")"
else
    INSTALL_DIR="$SCRIPT_DIR"
fi

normalize_version() {
    echo "${1#v}" | tr -d '[:space:]'
}

local_version="0.0.0"
if [[ -f "${INSTALL_DIR}/version.txt" ]]; then
    local_version="$(normalize_version "$(cat "${INSTALL_DIR}/version.txt")")"
fi

if [[ "$NO_UPDATE" != "true" && -n "$REPO" && "$REPO" != "TON_COMPTE/TON_REPO" && "$REPO" == */* ]] && command -v curl >/dev/null 2>&1 && command -v python3 >/dev/null 2>&1; then
    tmp_json="$(mktemp)"
    if curl -fsSL -H "User-Agent: DinotofuLauncher" "https://api.github.com/repos/${REPO}/releases/latest" -o "$tmp_json"; then
        remote_tag="$(python3 - "$tmp_json" <<'PY'
import json, sys
with open(sys.argv[1], encoding='utf-8') as f:
    print(json.load(f).get('tag_name',''))
PY
)"
        remote_version="$(normalize_version "$remote_tag")"
        if [[ -n "$remote_version" && "$remote_version" != "$local_version" ]]; then
            echo "Mise a jour disponible : ${local_version} -> ${remote_version}"
            if [[ -x "${INSTALL_DIR}/Installer-Dinotofu.sh" ]]; then
                "${INSTALL_DIR}/Installer-Dinotofu.sh" --skip-launch --no-prompt
            elif [[ -x "${INSTALL_DIR}/DinotofuInstaller.sh" ]]; then
                "${INSTALL_DIR}/DinotofuInstaller.sh" --skip-launch --no-prompt
            fi
        fi
    fi
    rm -f "$tmp_json"
fi

open_url_or_file() {
    local target="$1"
    if command -v xdg-open >/dev/null 2>&1; then
        xdg-open "$target" >/dev/null 2>&1 || true
    elif command -v gio >/dev/null 2>&1; then
        gio open "$target" >/dev/null 2>&1 || true
    elif command -v open >/dev/null 2>&1; then
        open "$target" >/dev/null 2>&1 || true
    else
        echo "Ouvre manuellement : $target"
    fi
}

start_gui_preview() {
    local gui_debug_dir="${INSTALL_DIR}/gui_debug"
    local gui_file="${INSTALL_DIR}/tools/gui/dinotofu_gui_experimental.html"
    local fallback_gui_file="${INSTALL_DIR}/tools/gui/dinotofu_gui_preview.html"
    local server_script="${INSTALL_DIR}/tools/gui/serve_gui_preview.py"
    local port="${DINOTOFU_GUI_PREVIEW_PORT:-8787}"

    if [[ ! -f "$gui_file" && -f "$fallback_gui_file" ]]; then
        gui_file="$fallback_gui_file"
    fi

    if [[ ! -f "$gui_file" ]]; then
        return 1
    fi

    mkdir -p "$gui_debug_dir"

    if command -v python3 >/dev/null 2>&1 && [[ -f "$server_script" ]]; then
        echo "Ouverture de l interface graphique experimentale : http://127.0.0.1:${port}/tools/gui/dinotofu_gui_experimental.html"
        nohup python3 "$server_script" --root "$INSTALL_DIR" --port "$port" --gui-debug-dir "$gui_debug_dir" >/dev/null 2>&1 &
        sleep 0.8
        open_url_or_file "http://127.0.0.1:${port}/tools/gui/dinotofu_gui_experimental.html"
    else
        echo "Python3 introuvable : ouverture du fichier HTML local. Le live peut etre limite par le navigateur."
        open_url_or_file "$gui_file"
    fi

    export DINOTOFU_GUI_DEBUG_DIR="$gui_debug_dir"
    export DINOTOFU_GUI_INPUT_MODE="1"
    export DINOTOFU_GUI_INPUT_FILE="$gui_debug_dir/pending_input.txt"
    export DINOTOFU_GUI_INPUT_QUEUE_DIR="$gui_debug_dir/input_queue"
    return 0
}

launch_terminal() {
    if [[ -x "${INSTALL_DIR}/output/Dinotofu" ]]; then
        cd "$INSTALL_DIR"
        exec "${INSTALL_DIR}/output/Dinotofu"
    fi
    if [[ -x "${INSTALL_DIR}/Dinotofu" ]]; then
        cd "$INSTALL_DIR"
        exec "${INSTALL_DIR}/Dinotofu"
    fi

    echo "Impossible de trouver l'executable terminal Dinotofu." >&2
    echo "Chemins attendus : ${INSTALL_DIR}/output/Dinotofu ou ${INSTALL_DIR}/Dinotofu" >&2
    exit 1
}

if [[ "$LAUNCH_MODE" != "terminal" ]]; then
    for candidate in \
        "${INSTALL_DIR}/output/DinotofuGUI" \
        "${INSTALL_DIR}/output/DinotofuGui" \
        "${INSTALL_DIR}/DinotofuGUI" \
        "${INSTALL_DIR}/DinotofuGui"; do
        if [[ -x "$candidate" ]]; then
            cd "$INSTALL_DIR"
            exec "$candidate"
        fi
    done

    if start_gui_preview; then
        launch_terminal
    fi

    if [[ "$LAUNCH_MODE" == "gui" ]]; then
        echo "Version graphique introuvable. Bascule vers la version terminale si elle existe." >&2
    fi
fi

launch_terminal
