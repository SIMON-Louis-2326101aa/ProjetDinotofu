#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ "$(basename "$SCRIPT_DIR")" == "linux" && "$(basename "$(dirname "$SCRIPT_DIR")")" == "tools" ]]; then
    SCRIPT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
fi
CONFIG_FILE="${SCRIPT_DIR}/dinotofu-installer.config.json"
REPO="${DINOTOFU_REPO:-}"
ASSET_PATTERN="${DINOTOFU_ASSET_PATTERN:-Dinotofu-Linux-v*.7z}"
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
# Legacy packages used a placeholder repo. Treat it as unconfigured so old installs can update again.
if [[ -z "$REPO" || "$REPO" == "TON_COMPTE/TON_REPO" || "$REPO" != */* ]]; then
    REPO="SIMON-Louis-2326101aa/ProjetDinotofu"
fi
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

version_is_older() {
    local local_version_text="$1"
    local remote_version_text="$2"
    python3 - "$local_version_text" "$remote_version_text" <<'PY_VERSION_COMPARE' >/dev/null 2>&1
import re
import sys

def parse(value):
    value = value.strip().lstrip('vV')
    if not re.fullmatch(r'\d+(?:\.\d+){0,2}', value):
        return (0, 0, 0)
    parts = [int(part) for part in value.split('.')]
    while len(parts) < 3:
        parts.append(0)
    return tuple(parts[:3])

sys.exit(0 if parse(sys.argv[1]) < parse(sys.argv[2]) else 1)
PY_VERSION_COMPARE
}

local_version="0.00.00"
if [[ -f "${INSTALL_DIR}/version.txt" ]]; then
    local_version="$(normalize_version "$(cat "${INSTALL_DIR}/version.txt")")"
elif [[ -f "${INSTALL_DIR}/scripts/get_version.sh" ]]; then
    local_version="$(normalize_version "$(bash "${INSTALL_DIR}/scripts/get_version.sh" 2>/dev/null || echo "0.00.00")")"
fi

installed_runnable_exists() {
    local candidates=(
        "${INSTALL_DIR}/DinotofuGUI"
        "${INSTALL_DIR}/DinotofuGui"
        "${INSTALL_DIR}/output/DinotofuGUI"
        "${INSTALL_DIR}/output/DinotofuGui"
        "${INSTALL_DIR}/bin/DinotofuGUI"
        "${INSTALL_DIR}/bin/DinotofuGui"
        "${INSTALL_DIR}/Dinotofu"
        "${INSTALL_DIR}/output/Dinotofu"
        "${INSTALL_DIR}/bin/Dinotofu"
    )

    local candidate
    for candidate in "${candidates[@]}"; do
        if [[ -x "$candidate" || -f "$candidate" ]]; then
            return 0
        fi
    done

    return 1
}

run_installer_repair() {
    if [[ -x "${INSTALL_DIR}/Installer-Dinotofu.sh" ]]; then
        "${INSTALL_DIR}/Installer-Dinotofu.sh" --skip-launch --no-prompt
        return 0
    fi
    if [[ -x "${INSTALL_DIR}/DinotofuInstaller.sh" ]]; then
        "${INSTALL_DIR}/DinotofuInstaller.sh" --skip-launch --no-prompt
        return 0
    fi
    return 1
}


get_desktop_dirs() {
    local dirs=()
    if command -v xdg-user-dir >/dev/null 2>&1; then
        local xdg_desktop
        xdg_desktop="$(xdg-user-dir DESKTOP 2>/dev/null || true)"
        if [[ -n "$xdg_desktop" && -d "$xdg_desktop" ]]; then
            dirs+=("$xdg_desktop")
        fi
    fi
    if [[ -f "${HOME}/.config/user-dirs.dirs" ]]; then
        local conf_desktop
        conf_desktop="$(sed -n 's/^XDG_DESKTOP_DIR="\(.*\)"/\1/p' "${HOME}/.config/user-dirs.dirs" 2>/dev/null || true)"
        conf_desktop="${conf_desktop/\$HOME/$HOME}"
        if [[ -n "$conf_desktop" && -d "$conf_desktop" ]]; then
            dirs+=("$conf_desktop")
        fi
    fi
    for d in "${HOME}/Desktop" "${HOME}/Bureau" "${HOME}/Escritorio" "${HOME}/Schreibtisch" "${HOME}/Scrivania" "${HOME}/Bureaublad" "${HOME}/Skrivebord" "${HOME}/Рабочий стол"; do
        if [[ -d "$d" ]]; then
            dirs+=("$d")
        fi
    done
    if [[ ${#dirs[@]} -gt 0 ]]; then
        printf '%s\n' "${dirs[@]}" | awk '!seen[$0]++'
    fi
}

repair_linux_desktop_shortcuts() {
    local create_sc
    create_sc="$(read_config_value createDesktopShortcut)"
    if [[ "$create_sc" == "false" || "$create_sc" == "False" ]]; then
        return 0
    fi

    mkdir -p "${HOME}/.local/share/applications"
    local gui_icon="${INSTALL_DIR}/assets/branding/dinotofu_launcher_graphical_512.png"
    local terminal_icon="${INSTALL_DIR}/assets/branding/dinotofu_launcher_terminal_512.png"
    [[ -f "$gui_icon" ]] || gui_icon="${INSTALL_DIR}/data/assets/branding/dinotofu_launcher_graphical_512.png"
    [[ -f "$terminal_icon" ]] || terminal_icon="${INSTALL_DIR}/data/assets/branding/dinotofu_launcher_terminal_512.png"
    [[ -f "$gui_icon" ]] || gui_icon="${INSTALL_DIR}/assets/branding/dinotofu_site_logo_512.png"
    [[ -f "$gui_icon" ]] || gui_icon="${INSTALL_DIR}/data/assets/branding/dinotofu_site_logo_512.png"
    [[ -f "$gui_icon" ]] || gui_icon="${SCRIPT_DIR}/assets/branding/dinotofu_launcher_graphical_512.png"
    [[ -f "$terminal_icon" ]] || terminal_icon="$gui_icon"

    local launch_exec="${INSTALL_DIR}/Lancer-Dinotofu.sh"
    if [[ ! -x "$launch_exec" && -x "${INSTALL_DIR}/tools/linux/Lancer-Dinotofu.sh" ]]; then
        launch_exec="${INSTALL_DIR}/tools/linux/Lancer-Dinotofu.sh"
    elif [[ ! -x "$launch_exec" && -x "${INSTALL_DIR}/DinotofuLauncher.sh" ]]; then
        launch_exec="${INSTALL_DIR}/DinotofuLauncher.sh"
    elif [[ ! -x "$launch_exec" && -x "${INSTALL_DIR}/tools/linux/DinotofuLauncher.sh" ]]; then
        launch_exec="${INSTALL_DIR}/tools/linux/DinotofuLauncher.sh"
    elif [[ ! -x "$launch_exec" && -x "${SCRIPT_DIR}/tools/linux/Lancer-Dinotofu.sh" ]]; then
        launch_exec="${SCRIPT_DIR}/tools/linux/Lancer-Dinotofu.sh"
    elif [[ ! -x "$launch_exec" && -x "${SCRIPT_DIR}/tools/linux/DinotofuLauncher.sh" ]]; then
        launch_exec="${SCRIPT_DIR}/tools/linux/DinotofuLauncher.sh"
    fi

    local run_root="${INSTALL_DIR}"
    if [[ ! -d "$run_root" || "$(basename "$run_root")" == "linux" ]]; then
        run_root="${SCRIPT_DIR}"
    fi

    local gui_app="${HOME}/.local/share/applications/projetdinotofu-launcher.desktop"
    local terminal_app="${HOME}/.local/share/applications/projetdinotofu-launcher-terminal.desktop"

    cat > "$gui_app" <<DESKTOP
[Desktop Entry]
Type=Application
Name=ProjetDinotofu Launcher
Comment=Lancer Dinotofu (choix Interface Graphique ou Terminal)
Exec=${launch_exec}
Path=${run_root}
Icon=${gui_icon}
Terminal=true
Categories=Game;
DESKTOP
    chmod +x "$gui_app" 2>/dev/null || true
    rm -f "$terminal_app" 2>/dev/null || true

    local desktop_dirs=()
    mapfile -t desktop_dirs < <(get_desktop_dirs || true)

    for desktop_dir in "${desktop_dirs[@]}"; do
        [[ -n "$desktop_dir" && -d "$desktop_dir" ]] || continue
        local found_gui="false"
        while IFS= read -r candidate; do
            [[ -n "$candidate" ]] || continue
            base="$(basename "$candidate")"
            if [[ "$base" == "ProjetDinotofu Launcher Terminal version.desktop" || "$base" == *Terminal* || "$base" == *terminal* ]] || grep -qi "Lancer-Dinotofu-Terminal" "$candidate" 2>/dev/null; then
                rm -f "$candidate" 2>/dev/null || true
            elif [[ "$base" == "ProjetDinotofu Launcher.desktop" || ( "$base" == *Dinotofu*Launcher*.desktop && "$base" != *Terminal* ) ]] || grep -qi "Lancer-Dinotofu.sh" "$candidate" 2>/dev/null; then
                cp "$gui_app" "$candidate" || true
                chmod +x "$candidate" 2>/dev/null || true
                found_gui="true"
            fi
        done < <(find "$desktop_dir" -type f -name "*.desktop" 2>/dev/null)

        if [[ "$found_gui" != "true" ]]; then
            cp "$gui_app" "$desktop_dir/ProjetDinotofu Launcher.desktop" || true
            chmod +x "$desktop_dir/ProjetDinotofu Launcher.desktop" 2>/dev/null || true
        fi
    done
}

UPDATE_APPLIED="false"
if [[ "$NO_UPDATE" != "true" && -n "$REPO" && "$REPO" == */* ]] && command -v curl >/dev/null 2>&1 && command -v python3 >/dev/null 2>&1; then
    tmp_json="$(mktemp)"
    if curl -fsSL -H "User-Agent: DinotofuLauncher" "https://api.github.com/repos/${REPO}/releases/latest" -o "$tmp_json"; then
        remote_tag="$(python3 - "$tmp_json" <<'PY'
import json, sys
with open(sys.argv[1], encoding='utf-8') as f:
    print(json.load(f).get('tag_name',''))
PY
)"
        remote_version="$(normalize_version "$remote_tag")"
        if [[ -n "$remote_version" ]] && version_is_older "$local_version" "$remote_version"; then
            echo "Mise a jour obligatoire disponible : ${local_version} -> ${remote_version}"
            if run_installer_repair; then
                UPDATE_APPLIED="true"
            fi
        elif [[ -n "$remote_version" ]] && ! installed_runnable_exists; then
            echo "Installation incomplete : aucun executable local trouve malgre une version a jour. Reparation depuis la release GitHub."
            if run_installer_repair; then
                UPDATE_APPLIED="true"
            fi
        fi
    fi
    rm -f "$tmp_json"
fi

repair_linux_desktop_shortcuts

if [[ "$UPDATE_APPLIED" == "true" ]]; then
    echo ""
    echo "================================================="
    echo " Mise a jour terminee avec succes !"
    echo "================================================="
    echo ""
    if [[ -t 0 ]]; then
        read -r -s -n 1 -p "Appuie sur une touche pour lancer Dinotofu..." _ || true
        echo ""
    fi
    clear 2>/dev/null || tput clear 2>/dev/null || true
    if [[ -x "${INSTALL_DIR}/DinotofuLauncher.sh" ]]; then
        exec "${INSTALL_DIR}/DinotofuLauncher.sh" --no-update "--mode=${LAUNCH_MODE}"
    fi
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

stop_dinotofu_background_processes() {
    local target_dir="${1:-$INSTALL_DIR}"
    local debug_dir="${target_dir}/gui_debug"
    if [[ -f "${debug_dir}/server.pid" ]]; then
        local spid
        spid="$(cat "${debug_dir}/server.pid" 2>/dev/null || true)"
        if [[ -n "$spid" && "$spid" =~ ^[0-9]+$ ]]; then
            kill -TERM "$spid" 2>/dev/null || true
        fi
        rm -f "${debug_dir}/server.pid"
    fi
    if [[ -f "${debug_dir}/game.pid" ]]; then
        local gpid
        gpid="$(cat "${debug_dir}/game.pid" 2>/dev/null || true)"
        if [[ -n "$gpid" && "$gpid" =~ ^[0-9]+$ ]]; then
            kill -TERM "$gpid" 2>/dev/null || true
        fi
        rm -f "${debug_dir}/game.pid"
    fi
    pkill -f "serve_gui_preview.py.*${target_dir}" 2>/dev/null || true
    pkill -f "${target_dir}/(output/)?Dinotofu" 2>/dev/null || true
}

start_gui_preview() {
    local gui_debug_dir="${INSTALL_DIR}/gui_debug"
    local gui_root="${INSTALL_DIR}"
    if [[ -d "${INSTALL_DIR}/data/tools/gui" ]]; then
        gui_root="${INSTALL_DIR}/data"
    fi
    local gui_file="${gui_root}/tools/gui/dinotofu_gui_experimental.html"
    local fallback_gui_file="${gui_root}/tools/gui/dinotofu_gui_preview.html"
    local server_script="${gui_root}/tools/gui/serve_gui_preview.py"
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
        nohup python3 "$server_script" --root "$gui_root" --port "$port" --gui-debug-dir "$gui_debug_dir" >"$server_out" 2>"$server_err" &
        local server_pid=$!
        echo "$server_pid" > "${gui_debug_dir}/server.pid"
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
    if [[ -x "${SCRIPT_DIR}/output/Dinotofu" ]]; then
        printf '%s\n' "${SCRIPT_DIR}/output/Dinotofu"
        return 0
    fi
    if [[ -x "${SCRIPT_DIR}/Dinotofu" ]]; then
        printf '%s\n' "${SCRIPT_DIR}/Dinotofu"
        return 0
    fi
    if [[ -f "${SCRIPT_DIR}/Makefile" ]] && command -v make >/dev/null 2>&1; then
        echo "==> Binaire introuvable. Compilation locale de Dinotofu via make..." >&2
        if make -C "${SCRIPT_DIR}" >/dev/null 2>&1; then
            if [[ -x "${SCRIPT_DIR}/output/Dinotofu" ]]; then
                printf '%s\n' "${SCRIPT_DIR}/output/Dinotofu"
                return 0
            fi
        fi
    fi
    return 1
}

launch_terminal() {
    local executable
    if executable="$(find_terminal_executable)"; then
        local run_dir="$(dirname "$executable")"
        if [[ "$(basename "$run_dir")" == "output" ]]; then
            run_dir="$(dirname "$run_dir")"
        fi
        cd "$run_dir"
        trap 'exit 0' INT TERM
        "$executable" "$@" || true
        exit 0
    fi

    echo "Impossible de trouver l'executable terminal Dinotofu." >&2
    echo "Chemins attendus : ${INSTALL_DIR}/output/Dinotofu ou ${INSTALL_DIR}/Dinotofu" >&2
    if [[ -t 0 ]]; then
        read -r -p "Appuie sur Entree pour fermer..." _ || true
    fi
    exit 1
}

launch_hidden_gui_backend() {
    local executable
    if ! executable="$(find_terminal_executable)"; then
        return 1
    fi

    local run_dir="$(dirname "$executable")"
    if [[ "$(basename "$run_dir")" == "output" ]]; then
        run_dir="$(dirname "$run_dir")"
    fi
    cd "$run_dir"
    mkdir -p "${run_dir}/gui_debug"
    nohup "$executable" >"${run_dir}/gui_debug/game_stdout.log" 2>"${run_dir}/gui_debug/game_stderr.log" &
    local game_pid=$!
    echo "$game_pid" > "${run_dir}/gui_debug/game.pid"
    return 0
}

if [[ "$LAUNCH_MODE" == "auto" && -t 0 ]]; then
    echo ""
    echo "================================================="
    echo " Dinotofu - Choix du mode de lancement"
    echo "================================================="
    echo "  1. Interface Graphique (GUI / Navigateur web)"
    echo "  2. Mode Terminal (Classique dans la console)"
    echo "================================================="
    read -r -p "Choix [1 ou 2, Defaut = 1] : " user_choice || true
    if [[ "$user_choice" == "2" ]]; then
        LAUNCH_MODE="terminal"
    else
        LAUNCH_MODE="gui"
    fi
    echo ""
fi

if [[ "$LAUNCH_MODE" != "terminal" ]]; then
    for candidate in \
        "${INSTALL_DIR}/output/DinotofuGUI" \
        "${INSTALL_DIR}/output/DinotofuGui" \
        "${INSTALL_DIR}/DinotofuGUI" \
        "${INSTALL_DIR}/DinotofuGui"; do
        if [[ -x "$candidate" ]]; then
            cd "$INSTALL_DIR"
            trap 'exit 0' INT TERM
            "$candidate" "$@" || true
            exit 0
        fi
    done

    stop_dinotofu_background_processes "$INSTALL_DIR"
    if start_gui_preview; then
        if launch_hidden_gui_backend; then
            echo ""
            echo "================================================="
            echo " Dinotofu - Session Interface Graphique active"
            echo "================================================="
            echo "  Moteur de jeu Dinotofu actif en arriere-plan."
            echo ""
            echo "  Pour arreter le jeu et fermer la session :"
            echo "  Appuie sur Entree (ou fais Ctrl+C dans cette console)."
            echo "================================================="
            cleanup_gui_session() {
                trap - INT TERM EXIT
                echo ""
                echo "==> Arret des processus en arriere-plan..."
                stop_dinotofu_background_processes "$INSTALL_DIR"
                exit 0
            }
            trap cleanup_gui_session INT TERM EXIT
            if [[ -t 0 ]]; then
                read -r -p "Appuie sur Entree pour arreter Dinotofu : " _ || true
            else
                wait 2>/dev/null || true
            fi
            cleanup_gui_session
        else
            launch_terminal
            exit 0
        fi
    fi

    if [[ "$LAUNCH_MODE" == "gui" ]]; then
        echo "Version graphique introuvable. Bascule vers la version terminale si elle existe." >&2
    fi
fi

launch_terminal
