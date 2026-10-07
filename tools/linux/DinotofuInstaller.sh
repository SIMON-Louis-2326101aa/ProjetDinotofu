#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ "$(basename "$SCRIPT_DIR")" == "linux" && "$(basename "$(dirname "$SCRIPT_DIR")")" == "tools" ]]; then
    SCRIPT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
fi
CONFIG_FILE="${SCRIPT_DIR}/dinotofu-installer.config.json"
REPO="${DINOTOFU_REPO:-}"
ASSET_PATTERN="${DINOTOFU_ASSET_PATTERN:-Dinotofu-Linux-v*.7z}"
INSTALL_DIR="${DINOTOFU_INSTALL_DIR:-}"
SKIP_LAUNCH="false"
NO_PROMPT="false"
FORCE_UPDATE="false"

for arg in "$@"; do
    case "$arg" in
        --skip-launch) SKIP_LAUNCH="true" ;;
        --no-prompt) NO_PROMPT="true" ;;
        --update) FORCE_UPDATE="true" ;;
        --repo=*) REPO="${arg#--repo=}" ;;
        --install-dir=*) INSTALL_DIR="${arg#--install-dir=}" ;;
        --asset-pattern=*) ASSET_PATTERN="${arg#--asset-pattern=}" ;;
    esac
done

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
    if [[ -f "${HOME}/.config/user-dirs.dirs" ]]; then
        local conf_dir
        conf_dir="$(sed -n 's/^XDG_DOWNLOAD_DIR="\(.*\)"/\1/p' "${HOME}/.config/user-dirs.dirs" 2>/dev/null || true)"
        conf_dir="${conf_dir/\$HOME/$HOME}"
        if [[ -n "$conf_dir" && -d "$conf_dir" ]]; then
            echo "$conf_dir"
            return
        fi
    fi
    for d in "${HOME}/Downloads" "${HOME}/Téléchargements" "${HOME}/Descargas" "${HOME}/Scaricati" "${HOME}/Загрузки" "${HOME}/Download"; do
        if [[ -d "$d" ]]; then
            echo "$d"
            return
        fi
    done
    echo "${HOME}/Downloads"
}

normalize_project_dir() {
    local path_text="$1"
    if [[ -z "$path_text" ]]; then
        path_text="${HOME}/ProjetDinotofu"
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

    echo "" >&2
    echo "Dossier d'installation : le jeu sera toujours installe dans un dossier nomme ProjetDinotofu." >&2
    echo "Par defaut : ${default_dir}" >&2
    echo "Tu peux entrer un autre dossier parent, par exemple /home/$USER/Jeux." >&2
    printf "Emplacement parent (Entree = defaut) : " >&2
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
# Legacy packages used a placeholder repo. Treat it as unconfigured so old installs can update again.
if [[ -z "$REPO" || "$REPO" == "TON_COMPTE/TON_REPO" || "$REPO" != */* ]]; then
    REPO="SIMON-Louis-2326101aa/ProjetDinotofu"
fi
if [[ "$ASSET_PATTERN" == "Dinotofu-Linux-v*.zip" || "$ASSET_PATTERN" == "Dinotofu-Linux-v*.7z" ]]; then
    configured_pattern="$(read_config_value assetPattern)"
    [[ -z "$configured_pattern" ]] || ASSET_PATTERN="$configured_pattern"
fi
if [[ -z "${DINOTOFU_INSTALL_DIR:-}" && -z "$INSTALL_DIR" ]]; then
    configured_install_dir="$(read_config_value installDir)"
    if [[ -n "$configured_install_dir" ]]; then
        INSTALL_DIR="$configured_install_dir"
    else
        INSTALL_DIR="${HOME}/ProjetDinotofu"
    fi
fi
INSTALL_DIR="$(ask_install_dir "$INSTALL_DIR")"

need_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Commande manquante : $1" >&2
        echo "Installe-la puis relance l'installer. Exemple Debian/Ubuntu : sudo apt install curl p7zip-full python3" >&2
        exit 1
    fi
}

need_command curl
need_command python3

extract_archive() {
    local archive="$1"
    local dest="$2"
    mkdir -p "$dest"
    if [[ "$archive" == *.7z ]]; then
        if command -v 7z >/dev/null 2>&1; then
            7z x -y -o"$dest" "$archive" >/dev/null
        elif command -v bsdtar >/dev/null 2>&1; then
            bsdtar -xf "$archive" -C "$dest"
        elif command -v tar >/dev/null 2>&1; then
            tar -xf "$archive" -C "$dest"
        else
            echo "Erreur : outil introuvable pour extraire le fichier .7z (7z, bsdtar ou tar avec libarchive)." >&2
            echo "Installe p7zip-full (ex: sudo apt install p7zip-full)" >&2
            exit 1
        fi
    else
        if command -v unzip >/dev/null 2>&1; then
            unzip -q "$archive" -d "$dest"
        elif command -v 7z >/dev/null 2>&1; then
            7z x -y -o"$dest" "$archive" >/dev/null
        elif command -v bsdtar >/dev/null 2>&1; then
            bsdtar -xf "$archive" -C "$dest"
        else
            tar -xf "$archive" -C "$dest"
        fi
    fi
}

find_local_release_archive() {
    local pattern="$1"
    local search_dirs=("$SCRIPT_DIR")
    local parent_dir
    parent_dir="$(dirname "$SCRIPT_DIR")"
    search_dirs+=("$parent_dir")
    local dl_parent
    dl_parent="$(default_download_parent)"
    if [[ -d "$dl_parent" ]]; then search_dirs+=("$dl_parent"); fi
    for d in "${HOME}/Downloads" "${HOME}/Téléchargements" "${HOME}/Descargas" "${HOME}/Scaricati" "${HOME}/Загрузки" "${HOME}/Download"; do
        if [[ -d "$d" ]]; then search_dirs+=("$d"); fi
    done

    local dir candidate
    for dir in "${search_dirs[@]}"; do
        [[ -d "$dir" ]] || continue
        candidate="$(find "$dir" -maxdepth 1 -type f \( -name "$pattern" -o -name 'Dinotofu-Linux-v*.7z' -o -name 'Dinotofu-Linux-v*.zip' \) ! -name '*Installer*' -printf '%T@ %p\n' 2>/dev/null | sort -nr | head -n 1 | cut -d' ' -f2-)"
        if [[ -n "$candidate" && -f "$candidate" ]]; then
            echo "$candidate"
            return 0
        fi
    done
    return 1
}

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT
RELEASE_JSON="${TMP_DIR}/latest.json"
ARCHIVE_PATH="${TMP_DIR}/dinotofu_pkg"
EXTRACT_DIR="${TMP_DIR}/extract"
BACKUP_DIR="${TMP_DIR}/save_backup"
mkdir -p "$EXTRACT_DIR" "$BACKUP_DIR"

# Detection : si on lance l'installer depuis un dossier du jeu dezippe ou un clone git
if [[ ! -f "${SCRIPT_DIR}/output/Dinotofu" && ! -f "${SCRIPT_DIR}/Dinotofu" && -f "${SCRIPT_DIR}/Makefile" ]] && command -v make >/dev/null 2>&1; then
    echo "==> Compilation locale de Dinotofu pour l'installation..."
    make -C "${SCRIPT_DIR}" >/dev/null 2>&1 || true
fi

local_game_found="false"
if [[ -f "${SCRIPT_DIR}/output/Dinotofu" || -f "${SCRIPT_DIR}/Dinotofu" ]] && [[ -d "${SCRIPT_DIR}/assets" || -d "${SCRIPT_DIR}/data/assets" ]]; then
    local_game_found="true"
fi

if [[ "$FORCE_UPDATE" == "true" ]]; then
    local_game_found="false"
elif [[ "$local_game_found" == "true" && -n "$REPO" && "$REPO" == */* ]] && command -v curl >/dev/null 2>&1; then
    local_pkg_version="0.00.00"
    if [[ -f "${SCRIPT_DIR}/version.txt" ]]; then
        local_pkg_version="$(normalize_version "$(cat "${SCRIPT_DIR}/version.txt")")"
    elif [[ -f "${SCRIPT_DIR}/scripts/get_version.sh" ]]; then
        local_pkg_version="$(normalize_version "$(bash "${SCRIPT_DIR}/scripts/get_version.sh" 2>/dev/null || echo "0.00.00")")"
    fi
    if curl -fsSL -H "User-Agent: DinotofuInstaller" "https://api.github.com/repos/${REPO}/releases/latest" -o "$RELEASE_JSON" 2>/dev/null; then
        remote_tag="$(python3 - "$RELEASE_JSON" <<'PY' 2>/dev/null || true
import json, sys
with open(sys.argv[1], encoding='utf-8') as f:
    print(json.load(f).get('tag_name',''))
PY
)"
        remote_ver="$(normalize_version "$remote_tag")"
        if [[ -n "$remote_ver" ]] && version_is_older "$local_pkg_version" "$remote_ver"; then
            echo "Version locale (${local_pkg_version}) plus ancienne que la release GitHub (${remote_ver})."
            echo "==> Telechargement automatique de la derniere version depuis GitHub..."
            local_game_found="false"
        fi
    fi
fi

if [[ "$local_game_found" == "true" && "$SCRIPT_DIR" == "$INSTALL_DIR" ]]; then
    echo "Dinotofu est deja dans son dossier d'execution : ${INSTALL_DIR}"
    echo "Configuration et creation des raccourcis..."
elif [[ "$local_game_found" == "true" ]]; then
    echo "==> Installation depuis le dossier local : ${SCRIPT_DIR} -> ${INSTALL_DIR}"
    if [[ -d "$INSTALL_DIR" ]]; then
        echo "==> Sauvegarde des donnees joueur"
        for p in assets/saves data/assets/saves saves accounts characters exported_accounts import_accounts; do
            if [[ -e "${INSTALL_DIR}/${p}" ]]; then
                mkdir -p "${BACKUP_DIR}/$(dirname "$p")"
                cp -a "${INSTALL_DIR}/${p}" "${BACKUP_DIR}/${p}"
            fi
        done
    fi
    mkdir -p "$INSTALL_DIR"
    for item in "${SCRIPT_DIR}"/*; do
        [[ -e "$item" ]] || continue
        base="$(basename "$item")"
        case "$base" in
            build|obj|output)
                continue
                ;;
        esac
        cp -a "$item" "$INSTALL_DIR/"
    done
    mkdir -p "${INSTALL_DIR}/output"
    if [[ -f "${SCRIPT_DIR}/output/Dinotofu" ]]; then
        cp -a "${SCRIPT_DIR}/output/Dinotofu" "${INSTALL_DIR}/output/Dinotofu"
        cp -a "${SCRIPT_DIR}/output/Dinotofu" "${INSTALL_DIR}/Dinotofu"
    fi
    if [[ -f "${INSTALL_DIR}/tools/linux/DinotofuLauncher.sh" ]]; then
        cp -a "${INSTALL_DIR}/tools/linux/DinotofuLauncher.sh" "${INSTALL_DIR}/DinotofuLauncher.sh"
    fi
    if [[ -f "${INSTALL_DIR}/tools/linux/Lancer-Dinotofu.sh" ]]; then
        cp -a "${INSTALL_DIR}/tools/linux/Lancer-Dinotofu.sh" "${INSTALL_DIR}/Lancer-Dinotofu.sh"
    fi
    if [[ -f "${INSTALL_DIR}/tools/linux/Lancer-Dinotofu-Terminal.sh" ]]; then
        cp -a "${INSTALL_DIR}/tools/linux/Lancer-Dinotofu-Terminal.sh" "${INSTALL_DIR}/Lancer-Dinotofu-Terminal.sh"
    fi
    if [[ -f "${INSTALL_DIR}/tools/linux/Installer-Dinotofu.sh" ]]; then
        cp -a "${INSTALL_DIR}/tools/linux/Installer-Dinotofu.sh" "${INSTALL_DIR}/Installer-Dinotofu.sh"
    fi
    if [[ -f "${SCRIPT_DIR}/scripts/get_version.sh" ]]; then
        current_v="$(bash "${SCRIPT_DIR}/scripts/get_version.sh" 2>/dev/null || echo "0.00.00")"
        echo "$current_v" > "${INSTALL_DIR}/version.txt"
    fi
    if [[ -d "$BACKUP_DIR" ]]; then
        cp -a "${BACKUP_DIR}/." "$INSTALL_DIR/" 2>/dev/null || true
    fi
else
    if [[ -z "$REPO" || "$REPO" != */* ]]; then
        LOCAL_ARCHIVE="$(find_local_release_archive "$ASSET_PATTERN" || true)"
        if [[ -z "$LOCAL_ARCHIVE" ]]; then
            echo "Repo GitHub non configure et aucune archive locale trouvee." >&2
            echo "DINOTOFU_REPO='SIMON-Louis-2326101aa/ProjetDinotofu' ./Installer-Dinotofu.sh" >&2
            exit 1
        fi
    fi

    echo "==> Recherche de la derniere release GitHub (${REPO})"
    LOCAL_ARCHIVE=""
    if [[ -n "$REPO" && "$REPO" == */* ]] && curl -fsSL -H "User-Agent: DinotofuInstaller" "https://api.github.com/repos/${REPO}/releases/latest" -o "$RELEASE_JSON"; then
        mapfile -t ASSET_INFO < <(python3 - "$RELEASE_JSON" "$ASSET_PATTERN" <<'PY' 2>/dev/null || true
import fnmatch, json, sys
with open(sys.argv[1], encoding='utf-8') as f:
    data = json.load(f)
pattern = sys.argv[2]
patterns = [pattern]
if pattern.endswith('.7z'):
    patterns.append(pattern[:-3] + '.zip')
elif pattern.endswith('.zip'):
    patterns.append(pattern[:-4] + '.7z')
patterns.extend(['Dinotofu-Linux-v*.7z', 'Dinotofu-Linux-v*.zip', 'Dinotofu-Linux*.7z', 'Dinotofu-Linux*.zip'])

for p in patterns:
    for asset in data.get('assets', []):
        if fnmatch.fnmatch(asset.get('name', ''), p):
            print(data.get('tag_name', ''))
            print(asset.get('name', ''))
            print(asset.get('browser_download_url', ''))
            sys.exit(0)

for asset in data.get('assets', []):
    name = asset.get('name', '')
    if 'Linux' in name and 'Installer' not in name and (name.endswith('.7z') or name.endswith('.zip')):
        print(data.get('tag_name', ''))
        print(name)
        print(asset.get('browser_download_url', ''))
        sys.exit(0)

sys.exit(2)
PY
)

        TAG_NAME="${ASSET_INFO[0]:-}"
        ASSET_NAME="${ASSET_INFO[1]:-}"
        ASSET_URL="${ASSET_INFO[2]:-}"
    else
        TAG_NAME=""
        ASSET_NAME=""
        ASSET_URL=""
    fi

    if [[ -z "$ASSET_URL" ]]; then
        LOCAL_ARCHIVE="$(find_local_release_archive "$ASSET_PATTERN" || true)"
        if [[ -z "$LOCAL_ARCHIVE" ]]; then
            echo "Aucun asset ne correspond a ${ASSET_PATTERN}, et aucune archive locale (.7z ou .zip) n'a ete trouvee." >&2
            echo "" >&2
            echo "Astuce : Si la mise a jour automatique ne fonctionne pas, tu peux telecharger directement l'archive sur :" >&2
            echo "https://github.com/${REPO}/releases/latest" >&2
            echo "puis la decompresser dans ton dossier de jeu." >&2
            exit 1
        fi
        TAG_NAME="$(basename "$LOCAL_ARCHIVE" | sed -E 's/.*-v([0-9]+\.[0-9]{2}\.[0-9]{2}).*/v\1/')"
        ASSET_NAME="$(basename "$LOCAL_ARCHIVE")"
        echo "GitHub non utilisé : archive locale trouvée dans le pack installer."
    else
        echo "Release trouvee : ${TAG_NAME}"
    fi

    echo "Fichier : ${ASSET_NAME}"
    echo "Installation finale : ${INSTALL_DIR}"
    echo "==> Telechargement / copie locale"
    if [[ -n "$LOCAL_ARCHIVE" ]]; then
        ARCHIVE_PATH="$LOCAL_ARCHIVE"
    else
        if [[ "$ASSET_NAME" == *.7z ]]; then
            ARCHIVE_PATH="${TMP_DIR}/dinotofu.7z"
        else
            ARCHIVE_PATH="${TMP_DIR}/dinotofu.zip"
        fi
        curl -L --progress-bar -H "User-Agent: DinotofuInstaller" "$ASSET_URL" -o "$ARCHIVE_PATH"
    fi

    if [[ -d "$INSTALL_DIR" ]]; then
        echo "==> Sauvegarde des donnees joueur"
        for p in assets/saves data/assets/saves saves accounts characters exported_accounts import_accounts; do
            if [[ -e "${INSTALL_DIR}/${p}" ]]; then
                mkdir -p "${BACKUP_DIR}/$(dirname "$p")"
                cp -a "${INSTALL_DIR}/${p}" "${BACKUP_DIR}/${p}"
            fi
        done
    fi

    echo "==> Installation dans ${INSTALL_DIR}"
    mkdir -p "$INSTALL_DIR"
    extract_archive "$ARCHIVE_PATH" "$EXTRACT_DIR"
    ROOT_DIR="$(find "$EXTRACT_DIR" -mindepth 1 -maxdepth 1 -type d | head -n 1)"
    if [[ -z "$ROOT_DIR" ]]; then
        ROOT_DIR="$EXTRACT_DIR"
    fi
    cp -a "${ROOT_DIR}/." "$INSTALL_DIR/"

    if [[ -d "$BACKUP_DIR" ]]; then
        cp -a "${BACKUP_DIR}/." "$INSTALL_DIR/" 2>/dev/null || true
    fi
fi

CREATE_DESKTOP_SHORTCUT="true"
if [[ "$NO_PROMPT" != "true" && -t 0 ]]; then
    echo ""
    read -r -p "Voulez-vous creer un raccourci sur le Bureau ? (O/n) [Defaut: O] : " sc_choice || true
    if [[ -n "$sc_choice" && ! "$sc_choice" =~ ^[oOyY]$ ]]; then
        CREATE_DESKTOP_SHORTCUT="false"
        echo "Creation du raccourci bureau ignoree a la demande de l'utilisateur."
    fi
fi

python3 - "${INSTALL_DIR}/dinotofu-installer.config.json" "${REPO}" "${ASSET_PATTERN}" "${INSTALL_DIR}" "${CREATE_DESKTOP_SHORTCUT}" <<'PYCONFIG'
import json
import sys
path, repo, asset_pattern, install_dir, create_sc = sys.argv[1:6]
with open(path, "w", encoding="utf-8") as handle:
    json.dump({
        "repo": repo,
        "assetPattern": asset_pattern,
        "installDir": install_dir,
        "createDesktopShortcut": create_sc.lower() == "true",
    }, handle, ensure_ascii=False, indent=2)
    handle.write("\n")
PYCONFIG

chmod +x "${INSTALL_DIR}/output/Dinotofu" "${INSTALL_DIR}/Dinotofu" 2>/dev/null || true
chmod +x "${INSTALL_DIR}/DinotofuLauncher.sh" 2>/dev/null || true
chmod +x "${INSTALL_DIR}/Lancer-Dinotofu.sh" 2>/dev/null || true
chmod +x "${INSTALL_DIR}/Lancer-Dinotofu-Terminal.sh" 2>/dev/null || true
chmod +x "${INSTALL_DIR}/Installer-Dinotofu.sh" "${INSTALL_DIR}/DinotofuInstaller.sh" 2>/dev/null || true

if [[ ! -f "${INSTALL_DIR}/version.txt" ]]; then
    ver="${TAG_NAME#v}"
    [[ -n "$ver" ]] || ver="$(cat "${SCRIPT_DIR}/version.txt" 2>/dev/null || echo "0.00.00")"
    echo "$ver" > "${INSTALL_DIR}/version.txt"
fi

echo "==> Configuration des raccourcis Linux"
mkdir -p "${HOME}/.local/share/applications"
GUI_ICON="${INSTALL_DIR}/assets/branding/dinotofu_launcher_graphical_512.png"
TERMINAL_ICON="${INSTALL_DIR}/assets/branding/dinotofu_launcher_terminal_512.png"
if [[ ! -f "$GUI_ICON" ]]; then GUI_ICON="${INSTALL_DIR}/data/assets/branding/dinotofu_launcher_graphical_512.png"; fi
if [[ ! -f "$TERMINAL_ICON" ]]; then TERMINAL_ICON="${INSTALL_DIR}/data/assets/branding/dinotofu_launcher_terminal_512.png"; fi
if [[ ! -f "$GUI_ICON" ]]; then GUI_ICON="${INSTALL_DIR}/assets/branding/dinotofu_site_logo_512.png"; fi
if [[ ! -f "$GUI_ICON" ]]; then GUI_ICON="${INSTALL_DIR}/data/assets/branding/dinotofu_site_logo_512.png"; fi
if [[ ! -f "$GUI_ICON" ]]; then GUI_ICON="${SCRIPT_DIR}/assets/branding/dinotofu_launcher_graphical_512.png"; fi

LOCAL_LAUNCHER_EXEC="${INSTALL_DIR}/Lancer-Dinotofu.sh"
if [[ ! -f "$LOCAL_LAUNCHER_EXEC" && -f "${INSTALL_DIR}/tools/linux/Lancer-Dinotofu.sh" ]]; then
    LOCAL_LAUNCHER_EXEC="${INSTALL_DIR}/tools/linux/Lancer-Dinotofu.sh"
elif [[ ! -f "$LOCAL_LAUNCHER_EXEC" && -f "${INSTALL_DIR}/DinotofuLauncher.sh" ]]; then
    LOCAL_LAUNCHER_EXEC="${INSTALL_DIR}/DinotofuLauncher.sh"
elif [[ ! -f "$LOCAL_LAUNCHER_EXEC" && -f "${INSTALL_DIR}/tools/linux/DinotofuLauncher.sh" ]]; then
    LOCAL_LAUNCHER_EXEC="${INSTALL_DIR}/tools/linux/DinotofuLauncher.sh"
fi

GUI_APP="${HOME}/.local/share/applications/projetdinotofu-launcher.desktop"
cat > "$GUI_APP" <<DESKTOP
[Desktop Entry]
Type=Application
Name=ProjetDinotofu Launcher
Comment=Lancer Dinotofu (Terminal par defaut - GUI en developpement)
Exec=${LOCAL_LAUNCHER_EXEC}
Path=$(dirname "${LOCAL_LAUNCHER_EXEC}")
Icon=${GUI_ICON}
Terminal=true
Categories=Game;
DESKTOP
chmod +x "$GUI_APP" || true
rm -f "${HOME}/.local/share/applications/projetdinotofu-launcher-terminal.desktop" 2>/dev/null || true

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

if [[ "$CREATE_DESKTOP_SHORTCUT" == "true" ]]; then
    desktop_dirs=()
    mapfile -t desktop_dirs < <(get_desktop_dirs || true)

    for desktop_dir in "${desktop_dirs[@]}"; do
        [[ -n "$desktop_dir" && -d "$desktop_dir" ]] || continue
        found_gui="false"
        while IFS= read -r candidate; do
            [[ -n "$candidate" ]] || continue
            base="$(basename "$candidate")"
            if [[ "$base" == "ProjetDinotofu Launcher Terminal version.desktop" || "$base" == *Terminal* || "$base" == *terminal* ]] || grep -qi "Lancer-Dinotofu-Terminal" "$candidate" 2>/dev/null; then
                rm -f "$candidate" 2>/dev/null || true
            elif [[ "$base" == "ProjetDinotofu Launcher.desktop" || ( "$base" == *Dinotofu*Launcher*.desktop && "$base" != *Terminal* ) ]] || grep -qi "Lancer-Dinotofu.sh" "$candidate" 2>/dev/null; then
                cp "$GUI_APP" "$candidate" || true
                chmod +x "$candidate" 2>/dev/null || true
                echo "Raccourci repare : $candidate"
                found_gui="true"
            fi
        done < <(find "$desktop_dir" -type f -name "*.desktop" 2>/dev/null)

        if [[ "$found_gui" != "true" ]]; then
            target="$desktop_dir/ProjetDinotofu Launcher.desktop"
            cp "$GUI_APP" "$target" || true
            chmod +x "$target" 2>/dev/null || true
            echo "Raccourci cree : $target"
        fi
    done
fi

echo "================================================="
echo " Dinotofu est installe dans : ${INSTALL_DIR}"
echo "================================================="
if [[ "$SKIP_LAUNCH" != "true" ]]; then
    if [[ -t 0 ]]; then
        echo ""
        read -r -s -n 1 -p "Appuie sur une touche pour lancer Dinotofu..." _ || true
        echo ""
    fi
    clear 2>/dev/null || tput clear 2>/dev/null || true
    exec "${INSTALL_DIR}/Lancer-Dinotofu.sh" --no-update
fi
