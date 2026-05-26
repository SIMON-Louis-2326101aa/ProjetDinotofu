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

# Old launcher / private helper files that should not be part of the project ZIP.
[[ ! -e "run_dinotofu.sh" ]] || fail "run_dinotofu.sh ne doit plus être à la racine."

# Generated output folders.
[[ ! -d "build" ]] || fail "Le dossier build/ doit être nettoyé avant packaging."
[[ ! -d "output" ]] || fail "Le dossier output/ doit être nettoyé avant packaging source."

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
        -name 'Dinotofu' -o \
        -name 'dinotofu' \
    \) -print0)

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

if [[ ${failures} -gt 0 ]]; then
    echo "" >&2
    echo "Contrôle packaging échoué : ${failures} problème(s)." >&2
    exit 1
fi

warn "Contrôle packaging OK : aucun fichier interdit détecté."
