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
        --auto|--mode=auto) LAUNCH_MODE="auto" ;;
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

local_version="0.00.00"
if [[ -f "${INSTALL_DIR}/version.txt" ]]; then
    local_version="$(normalize_version "$(cat "${INSTALL_DIR}/version.txt")")"
fi

UPDATE_APPLIED="false"
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
                UPDATE_APPLIED="true"
            elif [[ -x "${INSTALL_DIR}/DinotofuInstaller.sh" ]]; then
                "${INSTALL_DIR}/DinotofuInstaller.sh" --skip-launch --no-prompt
                UPDATE_APPLIED="true"
            fi
        fi
    fi
    rm -f "$tmp_json"
fi

if [[ "$UPDATE_APPLIED" == "true" && -x "${INSTALL_DIR}/DinotofuLauncher.sh" ]]; then
    echo "Redemarrage du launcher apres mise a jour."
    exec "${INSTALL_DIR}/DinotofuLauncher.sh" --no-update "--mode=${LAUNCH_MODE}"
fi


find_free_port() {
    local preferred="$1"
    python3 - "$preferred" <<'PYPORT' 2>/dev/null || echo "$preferred"
import socket
import sys
preferred = int(sys.argv[1])
for port in range(preferred, preferred + 20):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        try:
            sock.bind(("127.0.0.1", port))
        except OSError:
            continue
        print(port)
        raise SystemExit(0)
print(preferred)
PYPORT
}

wait_for_gui_server() {
    local port="$1"
    local attempts="${2:-32}"
    local url="http://127.0.0.1:${port}/gui/status"
    for _ in $(seq 1 "$attempts"); do
        if command -v python3 >/dev/null 2>&1; then
            if python3 - "$url" <<'PYWAIT' >/dev/null 2>&1
import sys
from urllib.request import urlopen
try:
    with urlopen(sys.argv[1], timeout=1) as response:
        sys.exit(0 if response.status < 500 else 1)
except Exception:
    sys.exit(1)
PYWAIT
            then
                return 0
            fi
        elif command -v curl >/dev/null 2>&1; then
            if curl -fsS --max-time 1 "$url" >/dev/null 2>&1; then
                return 0
            fi
        fi
        sleep 0.25
    done
    return 1
}

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
    if command -v python3 >/dev/null 2>&1; then
        port="$(find_free_port "$port")"
    fi

    if [[ ! -f "$gui_file" && -f "$fallback_gui_file" ]]; then
        gui_file="$fallback_gui_file"
    fi

    if [[ ! -f "$gui_file" ]]; then
        return 1
    fi

    mkdir -p "$gui_debug_dir"

    if command -v python3 >/dev/null 2>&1 && [[ -f "$server_script" ]]; then
        echo "Ouverture de l interface graphique experimentale : http://127.0.0.1:${port}/tools/gui/dinotofu_gui_experimental.html"
        local server_out="${gui_debug_dir}/server_stdout.log"
        local server_err="${gui_debug_dir}/server_stderr.log"
        rm -f "$server_out" "$server_err"
        nohup python3 "$server_script" --root "$INSTALL_DIR" --port "$port" --gui-debug-dir "$gui_debug_dir" >"$server_out" 2>"$server_err" &
        if wait_for_gui_server "$port" 32; then
            open_url_or_file "http://127.0.0.1:${port}/tools/gui/dinotofu_gui_experimental.html"
        else
            echo "Serveur IG local non joignable sur 127.0.0.1:${port}. Ouverture du fichier HTML local en secours." >&2
            echo "Logs serveur : ${server_out} / ${server_err}" >&2
            open_url_or_file "$gui_file"
        fi
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

find_terminal_executable() {
    if [[ -x "${INSTALL_DIR}/output/Dinotofu" ]]; then
        printf '%s\n' "${INSTALL_DIR}/output/Dinotofu"
        return 0
    fi
    if [[ -x "${INSTALL_DIR}/Dinotofu" ]]; then
        printf '%s\n' "${INSTALL_DIR}/Dinotofu"
        return 0
    fi
    return 1
}

launch_terminal() {
    local executable
    if executable="$(find_terminal_executable)"; then
        cd "$INSTALL_DIR"
        exec "$executable"
    fi

    echo "Impossible de trouver l'executable terminal Dinotofu." >&2
    echo "Chemins attendus : ${INSTALL_DIR}/output/Dinotofu ou ${INSTALL_DIR}/Dinotofu" >&2
    exit 1
}

launch_hidden_gui_backend() {
    local executable
    if ! executable="$(find_terminal_executable)"; then
        return 1
    fi

    cd "$INSTALL_DIR"
    nohup "$executable" >"${INSTALL_DIR}/gui_debug/game_stdout.log" 2>"${INSTALL_DIR}/gui_debug/game_stderr.log" &
    return 0
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
        launch_hidden_gui_backend || launch_terminal
        exit 0
    fi

    if [[ "$LAUNCH_MODE" == "gui" ]]; then
        echo "Version graphique introuvable. Bascule vers la version terminale si elle existe." >&2
    fi
fi

launch_terminal
