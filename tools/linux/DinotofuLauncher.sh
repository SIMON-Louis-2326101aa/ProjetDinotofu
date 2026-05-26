#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_FILE="${SCRIPT_DIR}/dinotofu-installer.config.json"
REPO="${DINOTOFU_REPO:-}"
ASSET_PATTERN="${DINOTOFU_ASSET_PATTERN:-Dinotofu-Linux-v*.zip}"
INSTALL_DIR="$SCRIPT_DIR"
NO_UPDATE="false"
INSTALL_DIR_FROM_ARG="false"

for arg in "$@"; do
    case "$arg" in
        --no-update) NO_UPDATE="true" ;;
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

if [[ -x "${INSTALL_DIR}/output/Dinotofu" ]]; then
    cd "$INSTALL_DIR"
    exec "${INSTALL_DIR}/output/Dinotofu"
else
    echo "Impossible de trouver l'executable Dinotofu." >&2
    echo "Chemin attendu : ${INSTALL_DIR}/output/Dinotofu" >&2
    exit 1
fi
