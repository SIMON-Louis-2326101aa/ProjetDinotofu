#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

failures=0
fail() {
    echo "[ERREUR] $1" >&2
    failures=$((failures + 1))
}
info() {
    echo "[INFO] $1"
}

required_assets=(
    assets/branding/dinotofu_logo.png
    assets/branding/dinotofu_logo_512.png
    assets/branding/dinotofu_site_logo.png
    assets/branding/dinotofu_site_logo_512.png
    assets/branding/dinotofu_banner.png
    assets/branding/dinotofu_banner_web.png
    assets/branding/dinotofu_intro_cover.png
    assets/branding/dinotofu_intro_cover_web.png
    assets/branding/dinotofu_launcher_graphical.png
    assets/branding/dinotofu_launcher_graphical_512.png
    assets/branding/dinotofu_launcher_graphical.ico
    assets/branding/dinotofu_launcher_terminal.png
    assets/branding/dinotofu_launcher_terminal_512.png
    assets/branding/dinotofu_launcher_terminal.ico
    assets/branding/dinotofu.ico
    assets/branding/branding_manifest.json
    assets/branding/README_BRANDING.md
)

for asset in "${required_assets[@]}"; do
    if [[ ! -s "${asset}" ]]; then
        fail "Asset branding manquant ou vide : ${asset}"
    fi
done

current_version="$(./scripts/get_version.sh)"
grep -q "\"version\": \"${current_version}\"" assets/branding/branding_manifest.json || fail "branding_manifest.json n'est pas synchronisé avec V${current_version}."

# GUI must use the official branding files and keep the banner as a visual background only.
grep -q "assets/branding/dinotofu.ico" tools/gui/dinotofu_gui_experimental.html || fail "L'IG ne référence pas le favicon Dinotofu."
grep -q "assets/branding/dinotofu_site_logo_512.png" tools/gui/dinotofu_gui_experimental.html || fail "L'IG ne référence pas le logo site 512 dans l'en-tête."
grep -q "assets/branding/dinotofu_banner_web.png" tools/gui/dinotofu_gui_experimental.html || fail "L'IG ne référence pas la bannière web."
grep -q "Les textes restent la source complète" tools/gui/dinotofu_gui_experimental.html || fail "L'IG doit rappeler que les textes restent la source complète."

# Windows shortcuts must point to distinct launcher icons and distinct launch targets.
grep -q "dinotofu_launcher_graphical.ico" tools/windows/DinotofuInstaller.ps1 || fail "L'installateur Windows ne référence pas l'icône graphique."
grep -q "dinotofu_launcher_terminal.ico" tools/windows/DinotofuInstaller.ps1 || fail "L'installateur Windows ne référence pas l'icône terminale."
grep -q "ProjetDinotofu Launcher" tools/windows/DinotofuInstaller.ps1 || fail "Le raccourci Windows graphique n'est pas déclaré."
grep -q "ProjetDinotofu Launcher Terminal version" tools/windows/DinotofuInstaller.ps1 || fail "Le raccourci Windows terminal n'est pas déclaré."
grep -q "Repair-DinotofuDesktopShortcuts" tools/windows/DinotofuLauncher.ps1 || fail "Le launcher Windows ne répare pas les raccourcis après mise à jour."

# Linux .desktop entries must point to distinct 512 PNG icons and distinct launch targets.
grep -q "dinotofu_launcher_graphical_512.png" tools/linux/DinotofuInstaller.sh || fail "L'installateur Linux ne référence pas l'icône graphique 512."
grep -q "dinotofu_launcher_terminal_512.png" tools/linux/DinotofuInstaller.sh || fail "L'installateur Linux ne référence pas l'icône terminale 512."
grep -q "ProjetDinotofu Launcher" tools/linux/DinotofuInstaller.sh || fail "Le raccourci Linux graphique n'est pas géré par l'installateur."
grep -q "ProjetDinotofu Launcher Terminal version" tools/linux/DinotofuInstaller.sh || fail "Le raccourci Linux terminal n'est pas géré par l'installateur."
grep -q "repair_linux_desktop_shortcuts" tools/linux/DinotofuLauncher.sh || fail "Le launcher Linux ne répare pas les raccourcis après mise à jour."

if [[ ${failures} -gt 0 ]]; then
    echo "" >&2
    echo "Contrôle branding échoué : ${failures} problème(s)." >&2
    exit 1
fi

info "Branding OK : assets, IG et raccourcis Windows/Linux vérifiés."
