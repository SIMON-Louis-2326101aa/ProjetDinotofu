#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

failures=0

fail() {
    echo "[ERREUR] $1" >&2
    failures=$((failures + 1))
}

warn() {
    echo "[INFO] $1"
}


# Version consistency across code, README and release metadata.
CURRENT_VERSION="$(./scripts/get_version.sh)"
[[ "${CURRENT_VERSION}" =~ ^[0-9]+\.[0-9]{2}\.[0-9]{2}$ ]] || fail "Version courante invalide : ${CURRENT_VERSION} (format attendu : x.yy.zz)"
grep -q "V${CURRENT_VERSION}" READMEFR.md || fail "READMEFR.md ne mentionne pas la version courante V${CURRENT_VERSION}."
grep -q "V${CURRENT_VERSION}" README.md || fail "README.md ne mentionne pas la version courante V${CURRENT_VERSION}."
[[ -f "PATCHNOTE_DINOTOFU.md" ]] || fail "PATCHNOTE_DINOTOFU.md (anglais) est manquant."
[[ -f "PATCHNOTE_DINOTOFU_FR.md" ]] || fail "PATCHNOTE_DINOTOFU_FR.md (français) est manquant."
grep -q "V${CURRENT_VERSION}" PATCHNOTE_DINOTOFU.md || fail "PATCHNOTE_DINOTOFU.md ne mentionne pas la version courante V${CURRENT_VERSION}."
grep -q "V${CURRENT_VERSION}" PATCHNOTE_DINOTOFU_FR.md || fail "PATCHNOTE_DINOTOFU_FR.md ne mentionne pas la version courante V${CURRENT_VERSION}."
grep -q "\"version\": \"${CURRENT_VERSION}\"" release/manifest.example.json || fail "release/manifest.example.json ne mentionne pas la version ${CURRENT_VERSION}."
grep -q "\"releaseTag\": \"v${CURRENT_VERSION}\"" release/manifest.example.json || fail "release/manifest.example.json ne mentionne pas le tag v${CURRENT_VERSION}."
warn "Version et manifeste OK : V${CURRENT_VERSION}."

# Markdown convention requested by the project owner: non-empty Markdown lines keep at least 3 trailing spaces.
if ! python3 <<'PY_MD'
from pathlib import Path
import sys
excluded = {'.git', 'build', 'output', 'release_packages', 'gui_debug'}
bad = []
for path in Path('.').rglob('*.md'):
    if any(part in excluded for part in path.parts):
        continue
    for index, line in enumerate(path.read_text(encoding='utf-8').splitlines(), start=1):
        if line and not line.endswith('   '):
            bad.append(f"{path}:{index}: ligne Markdown sans 3 espaces finaux")
for entry in bad:
    print(entry)
sys.exit(1 if bad else 0)
PY_MD
then
    fail "Convention Markdown non respectée."
fi
warn "Markdown OK : convention des 3 espaces vérifiée."

# Direct terminal I/O should stay in the display/fallback layer only.
if ! python3 <<'PY_IO'
from pathlib import Path
import re
import sys
allowed = {
    Path('src/core/Console.cpp'),
    Path('src/interface/TerminalInterface.cpp'),
    Path('src/interface/menu/common/MenuFrame.cpp'),
    Path('src/interface/menu/common/PagedMenu.cpp'),
    Path('src/interface/menu/common/MessageScreen.cpp'),
}
pattern = re.compile(r'std::cout|std::cerr|std::cin|cout <<|cerr <<|getline\(std::cin')
bad = []
for root in (Path('src'), Path('include')):
    for path in root.rglob('*'):
        if path.suffix not in {'.cpp', '.hpp'}:
            continue
        try:
            content = path.read_text(encoding='utf-8')
        except UnicodeDecodeError:
            content = path.read_text(errors='ignore')
        if pattern.search(content) and path not in allowed:
            bad.append(str(path))
for entry in bad:
    print(entry)
sys.exit(1 if bad else 0)
PY_IO
then
    fail "I/O terminal direct détecté hors socle autorisé."
fi
warn "I/O terminal OK : uniquement dans le socle autorisé."

# Old launcher / private helper files that should not be part of the project ZIP.
[[ ! -e "run_dinotofu.sh" ]] || fail "run_dinotofu.sh ne doit plus être à la racine."

# Local follow-up notes/audits are useful during development but must stay outside the source ZIP.
while IFS= read -r -d '' local_note; do
    fail "Fichier de suivi local interdit dans le ZIP source : ${local_note}"
done < <(find . -maxdepth 1 -type f \( -name 'AUDIT_IG_RESTANTS_*.md' -o -name 'reprise_chat_dinotofu_*.txt' \) -print0)
warn "Fichiers locaux de reprise/audit OK : absents du projet."

# Generated output folders.
[[ ! -d "build" ]] || fail "Le dossier build/ doit être nettoyé avant packaging."
[[ ! -d "output" ]] || fail "Le dossier output/ doit être nettoyé avant packaging source."
[[ ! -d "gui_debug" ]] || fail "Le dossier gui_debug/ est un dossier local de debug IG et ne doit pas être packagé."
while IFS= read -r -d '' pycache_dir; do
    fail "Cache Python detecte dans le projet : ${pycache_dir}"
done < <(find . -path './.git' -prune -o -type d -name '__pycache__' -print0)
warn "Dossiers générés et caches OK : absents."


# Launcher / installer files required for packaged releases.
for required_file in \
    tools/windows/DinotofuInstaller.ps1 \
    tools/windows/DinotofuLauncher.ps1 \
    tools/windows/Installer-Dinotofu.cmd \
    tools/windows/Lancer-Dinotofu.cmd \
    tools/windows/Lancer-Dinotofu.vbs \
    tools/windows/Lancer-Dinotofu-Terminal.cmd \
    tools/linux/DinotofuInstaller.sh \
    tools/linux/DinotofuLauncher.sh \
    tools/linux/Installer-Dinotofu.sh \
    tools/linux/Lancer-Dinotofu.sh \
    tools/linux/Lancer-Dinotofu-Terminal.sh; do
    [[ -f "${required_file}" ]] || fail "Fichier launcher/installer manquant : ${required_file}"
done

grep -q "ProjetDinotofu Launcher.lnk" tools/windows/DinotofuInstaller.ps1 || fail "Le raccourci Windows principal n'est pas declare correctement."
grep -q "ProjetDinotofu Launcher Terminal version.lnk" tools/windows/DinotofuInstaller.ps1 || fail "Le raccourci Windows terminal n'est pas declare correctement."
grep -q "Lancer-Dinotofu.vbs" tools/windows/DinotofuInstaller.ps1 || fail "Le raccourci Windows principal doit cibler Lancer-Dinotofu.vbs."
grep -q "Lancer-Dinotofu-Terminal.cmd" tools/windows/DinotofuInstaller.ps1 || fail "Le raccourci Windows terminal doit cibler Lancer-Dinotofu-Terminal.cmd."
grep -q -- "-Mode Auto" tools/windows/Lancer-Dinotofu.cmd || fail "Lancer-Dinotofu.cmd doit lancer le mode Auto."
grep -q -- "-Mode Terminal" tools/windows/Lancer-Dinotofu-Terminal.cmd || fail "Lancer-Dinotofu-Terminal.cmd doit lancer le mode Terminal."

if ! python3 <<'PY_INSTALLER'
from pathlib import Path
import json
import sys

errors = []
for name in [
    'tools/windows/dinotofu-installer.config.example.json',
    'tools/linux/dinotofu-installer.config.example.json',
]:
    path = Path(name)
    try:
        data = json.loads(path.read_text(encoding='utf-8'))
    except Exception as exc:
        errors.append(f"{name}: JSON invalide ({exc})")
        continue
    for key in ('repo', 'assetPattern', 'installDir'):
        if key not in data:
            errors.append(f"{name}: cle manquante {key}")

package_windows = Path('scripts/package_windows_release.sh').read_text(encoding='utf-8')
if 'write_installer_config_json()' not in package_windows or 'json.dump(config' not in package_windows:
    errors.append('scripts/package_windows_release.sh doit generer les configs JSON via Python/json.dump.')
if 'cat > "${STAGING_DIR}/dinotofu-installer.config.json" <<JSON' in package_windows:
    errors.append('scripts/package_windows_release.sh ne doit plus ecrire la config Windows via heredoc JSON non echappe.')
if 'r"%USERPROFILE%\\Downloads\\ProjetDinotofu"' not in package_windows:
    errors.append('scripts/package_windows_release.sh doit conserver le chemin Windows en raw string Python valide.')

workflow = Path('.github/workflows/release-dinotofu.yml').read_text(encoding='utf-8')
if 'package-source:' in workflow or ('no_' + 'exe') in workflow:
    errors.append('Le workflow GitHub ne doit plus publier de ZIP source dans les releases.')

for entry in errors:
    print(entry)
sys.exit(1 if errors else 0)
PY_INSTALLER
then
    fail "Configuration installateurs/release invalide."
fi
warn "Configs installateurs/release OK."
warn "Launchers/installateurs OK."

# Private save/account folders.
for private_path in saves accounts characters exported_accounts import_accounts; do
    [[ ! -e "${private_path}" ]] || fail "Dossier privé détecté : ${private_path}/"
done

if [[ -d "assets/saves" ]]; then
    while IFS= read -r -d '' save_file; do
        case "${save_file}" in
            */.gitkeep) ;;
            *) fail "Sauvegarde privée détectée dans assets/saves : ${save_file}" ;;
        esac
    done < <(find assets/saves -type f -print0)
fi
warn "Sauvegardes privées OK : aucune donnée joueur packagée."

# Generated binaries in source tree. Tools scripts are allowed, but executables are not.
while IFS= read -r -d '' forbidden; do
    fail "Fichier généré interdit dans le ZIP source : ${forbidden}"
done < <(find . \
    -path './.git' -prune -o \
    -path './release_packages' -prune -o \
    -type f \( \
        -name '*.exe' -o \
        -name '*.o' -o \
        -name '*.d' -o \
        -name '*.out' -o \
        -name '*.pyc' -o \
        -name 'Dinotofu' -o \
        -name 'dinotofu' \
    \) -print0)
warn "Binaires/objets générés OK : absents."

# Accidental local authentication material should never be committed.
while IFS= read -r -d '' auth_file; do
    fail "Fichier d’authentification local détecté dans le projet : ${auth_file}"
done < <(find . \
    -path './.git' -prune -o \
    -path './release_packages' -prune -o \
    -type f \( \
        -name 'id_ed25519*' -o \
        -name 'id_rsa*' -o \
        -name '*.pem' \
    \) -print0)
warn "Secrets locaux OK : absents."

if [[ ${failures} -gt 0 ]]; then
    echo "" >&2
    echo "Contrôle packaging échoué : ${failures} problème(s)." >&2
    exit 1
fi

warn "Contrôle packaging OK : aucun fichier interdit détecté."
