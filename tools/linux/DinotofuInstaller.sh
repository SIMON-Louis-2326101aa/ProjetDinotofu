#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONFIG_FILE="${SCRIPT_DIR}/dinotofu-installer.config.json"
REPO="${DINOTOFU_REPO:-}"
ASSET_PATTERN="${DINOTOFU_ASSET_PATTERN:-Dinotofu-Linux-v*.zip}"
INSTALL_DIR="${DINOTOFU_INSTALL_DIR:-}"
SKIP_LAUNCH="false"
NO_PROMPT="false"

for arg in "$@"; do
    case "$arg" in
        --skip-launch) SKIP_LAUNCH="true" ;;
        --no-prompt) NO_PROMPT="true" ;;
        --repo=*) REPO="${arg#--repo=}" ;;
        --install-dir=*) INSTALL_DIR="${arg#--install-dir=}" ;;
        --asset-pattern=*) ASSET_PATTERN="${arg#--asset-pattern=}" ;;
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

default_download_parent() {
    if command -v xdg-user-dir >/dev/null 2>&1; then
        local dir
        dir="$(xdg-user-dir DOWNLOAD 2>/dev/null || true)"
        if [[ -n "$dir" && -d "$dir" ]]; then
            echo "$dir"
            return
        fi
    fi
    if [[ -d "${HOME}/Downloads" ]]; then
        echo "${HOME}/Downloads"
    elif [[ -d "${HOME}/Téléchargements" ]]; then
        echo "${HOME}/Téléchargements"
    else
        echo "${HOME}/Downloads"
    fi
}

normalize_project_dir() {
    local path_text="$1"
    if [[ -z "$path_text" ]]; then
        path_text="$(default_download_parent)/ProjetDinotofu"
    fi
    path_text="${path_text/#\~/$HOME}"
    path_text="${path_text%/}"
    if [[ "$(basename "$path_text")" == "ProjetDinotofu" ]]; then
        echo "$path_text"
    else
        echo "${path_text}/ProjetDinotofu"
    fi
}

ask_install_dir() {
    local default_dir="$1"
    if [[ "$NO_PROMPT" == "true" || ! -t 0 ]]; then
        normalize_project_dir "$default_dir"
        return
    fi

    echo ""
    echo "Dossier d'installation : le jeu sera toujours installe dans un dossier nomme ProjetDinotofu."
    echo "Par defaut : ${default_dir}"
    echo "Tu peux entrer un autre dossier parent, par exemple /home/$USER/Jeux."
    printf "Emplacement parent (Entree = defaut) : "
    local answer=""
    read -r answer || true
    if [[ -z "$answer" ]]; then
        normalize_project_dir "$default_dir"
    else
        normalize_project_dir "$answer"
    fi
}

if [[ -z "$REPO" ]]; then
    REPO="$(read_config_value repo)"
fi
if [[ "$ASSET_PATTERN" == "Dinotofu-Linux-v*.zip" ]]; then
    configured_pattern="$(read_config_value assetPattern)"
    [[ -z "$configured_pattern" ]] || ASSET_PATTERN="$configured_pattern"
fi
if [[ -z "${DINOTOFU_INSTALL_DIR:-}" && -z "$INSTALL_DIR" ]]; then
    configured_install_dir="$(read_config_value installDir)"
    if [[ -n "$configured_install_dir" ]]; then
        INSTALL_DIR="$configured_install_dir"
    else
        INSTALL_DIR="$(default_download_parent)/ProjetDinotofu"
    fi
fi
INSTALL_DIR="$(ask_install_dir "$INSTALL_DIR")"

need_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Commande manquante : $1" >&2
        echo "Installe-la puis relance l'installer. Exemple Debian/Ubuntu : sudo apt install curl unzip python3" >&2
        exit 1
    fi
}

need_command curl
need_command unzip
need_command python3

if [[ -z "$REPO" || "$REPO" == "TON_COMPTE/TON_REPO" || "$REPO" != */* ]]; then
    echo "Repo GitHub non configure. Utilise un pack installer genere par GitHub Actions, ou lance :" >&2
    echo "DINOTOFU_REPO='tonPseudo/tonDepot' ./Installer-Dinotofu.sh" >&2
    exit 1
fi

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT
RELEASE_JSON="${TMP_DIR}/latest.json"
ZIP_PATH="${TMP_DIR}/dinotofu.zip"
EXTRACT_DIR="${TMP_DIR}/extract"
BACKUP_DIR="${TMP_DIR}/save_backup"
mkdir -p "$EXTRACT_DIR" "$BACKUP_DIR"

echo "==> Recherche de la derniere release GitHub (${REPO})"
curl -fsSL -H "User-Agent: DinotofuInstaller" "https://api.github.com/repos/${REPO}/releases/latest" -o "$RELEASE_JSON"

mapfile -t ASSET_INFO < <(python3 - "$RELEASE_JSON" "$ASSET_PATTERN" <<'PY'
import fnmatch, json, sys
with open(sys.argv[1], encoding='utf-8') as f:
    data = json.load(f)
pattern = sys.argv[2]
for asset in data.get('assets', []):
    if fnmatch.fnmatch(asset.get('name', ''), pattern):
        print(data.get('tag_name', ''))
        print(asset.get('name', ''))
        print(asset.get('browser_download_url', ''))
        sys.exit(0)
print('', file=sys.stderr)
sys.exit(2)
PY
)

TAG_NAME="${ASSET_INFO[0]:-}"
ASSET_NAME="${ASSET_INFO[1]:-}"
ASSET_URL="${ASSET_INFO[2]:-}"

if [[ -z "$ASSET_URL" ]]; then
    echo "Aucun asset ne correspond a ${ASSET_PATTERN}." >&2
    exit 1
fi

echo "Release trouvee : ${TAG_NAME}"
echo "Fichier : ${ASSET_NAME}"
echo "Installation finale : ${INSTALL_DIR}"
echo "==> Telechargement"
curl -L --progress-bar -H "User-Agent: DinotofuInstaller" "$ASSET_URL" -o "$ZIP_PATH"

if [[ -d "$INSTALL_DIR" ]]; then
    echo "==> Sauvegarde des donnees joueur"
    for p in assets/saves saves accounts characters exported_accounts import_accounts; do
        if [[ -e "${INSTALL_DIR}/${p}" ]]; then
            mkdir -p "${BACKUP_DIR}/$(dirname "$p")"
            cp -a "${INSTALL_DIR}/${p}" "${BACKUP_DIR}/${p}"
        fi
    done
fi

echo "==> Installation dans ${INSTALL_DIR}"
mkdir -p "$INSTALL_DIR"
unzip -q "$ZIP_PATH" -d "$EXTRACT_DIR"
ROOT_DIR="$(find "$EXTRACT_DIR" -mindepth 1 -maxdepth 1 -type d | head -n 1)"
if [[ -z "$ROOT_DIR" ]]; then
    echo "Archive invalide : aucun dossier racine trouve." >&2
    exit 1
fi
cp -a "${ROOT_DIR}/." "$INSTALL_DIR/"

if [[ -d "$BACKUP_DIR" ]]; then
    cp -a "${BACKUP_DIR}/." "$INSTALL_DIR/" 2>/dev/null || true
fi

cat > "${INSTALL_DIR}/dinotofu-installer.config.json" <<JSON
{
  "repo": "${REPO}",
  "assetPattern": "${ASSET_PATTERN}",
  "installDir": "${INSTALL_DIR}"
}
JSON

chmod +x "${INSTALL_DIR}/output/Dinotofu" 2>/dev/null || true
chmod +x "${INSTALL_DIR}/DinotofuLauncher.sh" 2>/dev/null || true
chmod +x "${INSTALL_DIR}/Lancer-Dinotofu.sh" 2>/dev/null || true
chmod +x "${INSTALL_DIR}/Installer-Dinotofu.sh" 2>/dev/null || true

if [[ ! -f "${INSTALL_DIR}/version.txt" ]]; then
    echo "${TAG_NAME#v}" > "${INSTALL_DIR}/version.txt"
fi

echo "==> Creation des raccourcis Linux"
mkdir -p "${HOME}/.local/share/applications"
cat > "${HOME}/.local/share/applications/dinotofu.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Dinotofu
Comment=Lancer Dinotofu
Exec=${INSTALL_DIR}/Lancer-Dinotofu.sh
Terminal=true
Categories=Game;
DESKTOP
chmod +x "${HOME}/.local/share/applications/dinotofu.desktop" || true
if [[ -d "${HOME}/Desktop" ]]; then
    cp "${HOME}/.local/share/applications/dinotofu.desktop" "${HOME}/Desktop/Dinotofu.desktop" || true
    chmod +x "${HOME}/Desktop/Dinotofu.desktop" || true
fi
if [[ -d "${HOME}/Bureau" ]]; then
    cp "${HOME}/.local/share/applications/dinotofu.desktop" "${HOME}/Bureau/Dinotofu.desktop" || true
    chmod +x "${HOME}/Bureau/Dinotofu.desktop" || true
fi

echo "Dinotofu est installe."
if [[ "$SKIP_LAUNCH" != "true" ]]; then
    exec "${INSTALL_DIR}/Lancer-Dinotofu.sh"
fi
