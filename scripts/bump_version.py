#!/usr/bin/env python3
"""
Bump Dinotofu's game version without rewriting historical release references.

Interactive usage (recommended):
  python3 scripts/bump_version.py
  python3 scripts/bump_version.py patch
  python3 scripts/bump_version.py minor
  python3 scripts/bump_version.py major
  python3 scripts/bump_version.py 3.51.00

The interactive mode also asks whether to:
- change the internal save schema version (`saveVersion`);
- make the new game version a mandatory important-save checkpoint.

Automation flags:
  --save-schema keep|next|N
  --checkpoint keep|current|X.Y.Z
  --non-interactive

Examples:
  python3 scripts/bump_version.py patch --save-schema keep --checkpoint keep --non-interactive
  python3 scripts/bump_version.py minor --save-schema next --checkpoint current --non-interactive
"""
from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VERSION_FILE = ROOT / "src" / "core" / "VersionInfo.cpp"
SAVE_SCHEMA_FILE = ROOT / "include" / "save" / "SaveSchemaVersion.hpp"
VERSION_RE = re.compile(
    r'(std::string\s+VersionInfo::currentVersion\(\)\s*\{\s*return\s+")([0-9]+\.[0-9]+\.[0-9]+)("\s*;\s*\})',
    re.S,
)
CHECKPOINT_RE = re.compile(
    r'(std::string\s+VersionInfo::importantSaveUpdateVersion\(\)\s*\{\s*return\s+")([0-9]+\.[0-9]+\.[0-9]+)("\s*;\s*\})',
    re.S,
)
SAVE_SCHEMA_RE = re.compile(r'(inline\s+constexpr\s+int\s+Current\s*=\s*)([0-9]+)(\s*;)')


def parse_version(text: str) -> tuple[int, int, int]:
    cleaned = text.strip().lstrip("vV")
    parts = cleaned.split(".")
    if len(parts) != 3 or not all(p.isdigit() for p in parts):
        raise ValueError(f"Version invalide : {text!r} (format attendu : X.Y.Z ou X.YY.ZZ)")
    return int(parts[0]), int(parts[1]), int(parts[2])


def format_version(parts: tuple[int, int, int]) -> str:
    return f"{parts[0]}.{parts[1]:02d}.{parts[2]:02d}"


def ask_yes_no(question: str, default: bool = False) -> bool:
    suffix = " [O/n] " if default else " [o/N] "
    while True:
        try:
            answer = input(question + suffix).strip().lower()
        except EOFError:
            return default
        if not answer:
            return default
        if answer in {"o", "oui", "y", "yes"}:
            return True
        if answer in {"n", "non", "no"}:
            return False
        print("Réponds par o/oui ou n/non.")


def read_current_state() -> tuple[str, int, str, str]:
    version_source = VERSION_FILE.read_text(encoding="utf-8")
    version_match = VERSION_RE.search(version_source)
    checkpoint_match = CHECKPOINT_RE.search(version_source)
    if not version_match:
        raise RuntimeError("Impossible de trouver VersionInfo::currentVersion()")
    if not checkpoint_match:
        raise RuntimeError("Impossible de trouver VersionInfo::importantSaveUpdateVersion()")

    schema_source = SAVE_SCHEMA_FILE.read_text(encoding="utf-8")
    schema_match = SAVE_SCHEMA_RE.search(schema_source)
    if not schema_match:
        raise RuntimeError("Impossible de trouver SaveSchemaVersion::Current")

    return version_match.group(2), int(schema_match.group(2)), checkpoint_match.group(2), version_source


def compute_new_game_version(current: str, mode: str) -> str:
    major, minor, patch = parse_version(current)
    if mode == "patch":
        patch += 1
    elif mode == "minor":
        minor += 1
        patch = 0
    elif mode == "major":
        major += 1
        minor = 0
        patch = 0
    else:
        major, minor, patch = parse_version(mode)
    return format_version((major, minor, patch))


def resolve_save_schema(current: int, option: str | None, interactive: bool) -> int:
    if option is not None:
        normalized = option.strip().lower()
        if normalized == "keep":
            return current
        if normalized == "next":
            return current + 1
        if normalized.isdigit() and int(normalized) >= 1:
            return int(normalized)
        raise ValueError("--save-schema attend keep, next ou un entier >= 1")

    if not interactive:
        return current

    print(f"\nSchéma de sauvegarde interne actuel : {current}")
    print("À changer seulement si la structure persistée exige une migration explicite.")
    if not ask_yes_no("Changer aussi le schéma de sauvegarde ?", default=False):
        return current

    while True:
        raw = input(f"Nouveau saveVersion [Entrée = {current + 1}] : ").strip()
        if not raw:
            return current + 1
        if raw.isdigit() and int(raw) >= 1:
            return int(raw)
        print("Entre un entier >= 1.")


def resolve_checkpoint(current: str, new_game_version: str, option: str | None, interactive: bool) -> str:
    if option is not None:
        normalized = option.strip().lower()
        if normalized == "keep":
            return current
        if normalized == "current":
            return new_game_version
        return format_version(parse_version(normalized))

    if not interactive:
        return current

    print(f"\nCheckpoint de sauvegarde important actuel : V{current}")
    print("Un nouveau checkpoint force les anciennes sauvegardes à faire backup + rituel d'adaptation.")
    if ask_yes_no(f"Faire de V{new_game_version} un NOUVEAU checkpoint obligatoire ?", default=False):
        return new_game_version
    return current


def update_sync_files(new_ver: str) -> None:
    readme_rules = [
        (ROOT / "READMEFR.md", r'(- Version actuelle : \*\*V)[0-9]+\.[0-9]+\.[0-9]+(\*\*)'),
        (ROOT / "README.md", r'(- Current version: \*\*V)[0-9]+\.[0-9]+\.[0-9]+(\*\*)'),
    ]
    for readme, pattern_text in readme_rules:
        if not readme.exists():
            continue
        content = readme.read_text(encoding="utf-8")
        content, count = re.subn(pattern_text, rf'\g<1>{new_ver}\g<2>', content, count=1)
        if count == 0:
            raise RuntimeError(f"Ligne de version courante introuvable dans {readme.name}")
        readme.write_text(content, encoding="utf-8")

    changelog_sections = [
        (ROOT / "CHANGELOG.md", f"## V{new_ver} — Update notes   \n\n- Version synchronization placeholder. Replace with detailed release notes before publishing.   \n\n---   \n\n"),
        (ROOT / "CHANGELOG_FR.md", f"## V{new_ver} — Notes de mise à jour   \n\n- Synchronisation de version. Remplacer par les notes détaillées avant publication.   \n\n---   \n\n"),
    ]
    for changelog, new_section in changelog_sections:
        if not changelog.exists():
            continue
        content = changelog.read_text(encoding="utf-8")
        if f"V{new_ver}" not in content:
            pattern = re.compile(r'(## V[0-9]+\.[0-9]+\.[0-9]+)')
            if pattern.search(content):
                content = pattern.sub(lambda match: new_section + match.group(1), content, count=1)
            else:
                content += "\n\n" + new_section
            changelog.write_text(content, encoding="utf-8")

    for manifest in [ROOT / "release" / "manifest.example.json", ROOT / "assets" / "branding" / "branding_manifest.json"]:
        if not manifest.exists():
            continue
        content = manifest.read_text(encoding="utf-8")
        content, count = re.subn(
            r'("version"\s*:\s*")[0-9]+\.[0-9]+\.[0-9]+(")',
            rf'\g<1>{new_ver}\g<2>',
            content,
            count=1,
        )
        if count == 0:
            raise RuntimeError(f"Champ version introuvable dans {manifest}")
        if manifest.name == "manifest.example.json":
            content = re.sub(r'("releaseTag"\s*:\s*"v)[0-9]+\.[0-9]+\.[0-9]+(")', rf'\g<1>{new_ver}\g<2>', content, count=1)
            content = re.sub(r'((?:Installer-)?Dinotofu-(?:Windows|Linux)-v)[0-9]+\.[0-9]+\.[0-9]+(\.7z)', rf'\g<1>{new_ver}\g<2>', content)
        manifest.write_text(content, encoding="utf-8")


def update_checkpoint_docs(old_checkpoint: str, new_checkpoint: str) -> None:
    if old_checkpoint == new_checkpoint:
        return

    version_source = VERSION_FILE.read_text(encoding="utf-8")
    version_source, count = CHECKPOINT_RE.subn(rf'\g<1>{new_checkpoint}\g<3>', version_source, count=1)
    if count != 1:
        raise RuntimeError("Impossible de mettre à jour importantSaveUpdateVersion()")
    VERSION_FILE.write_text(version_source, encoding="utf-8")

    # Current-status docs may move with the checkpoint. Historical changelog entries must never move.
    for path in [ROOT / "README.md", ROOT / "READMEFR.md"]:
        if path.exists():
            content = path.read_text(encoding="utf-8")
            content = content.replace(f"V{old_checkpoint}", f"V{new_checkpoint}")
            path.write_text(content, encoding="utf-8")

    test_cpp = ROOT / "tests" / "ImportantSaveCheckpointTest.cpp"
    if test_cpp.exists():
        content = test_cpp.read_text(encoding="utf-8").replace(old_checkpoint, new_checkpoint)
        test_cpp.write_text(content, encoding="utf-8")

    test_sh = ROOT / "scripts" / "test_project.sh"
    if test_sh.exists():
        content = test_sh.read_text(encoding="utf-8").replace(old_checkpoint, new_checkpoint)
        test_sh.write_text(content, encoding="utf-8")


def update_save_schema(old_schema: int, new_schema: int) -> None:
    if old_schema == new_schema:
        return
    source = SAVE_SCHEMA_FILE.read_text(encoding="utf-8")
    source, count = SAVE_SCHEMA_RE.subn(rf'\g<1>{new_schema}\g<3>', source, count=1)
    if count != 1:
        raise RuntimeError("Impossible de mettre à jour SaveSchemaVersion::Current")
    SAVE_SCHEMA_FILE.write_text(source, encoding="utf-8")


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Synchronise la version Dinotofu et, si demandé, la version de sauvegarde.")
    parser.add_argument("mode", nargs="?", default="patch", help="patch, minor, major ou version explicite X.Y.Z")
    parser.add_argument("--save-schema", dest="save_schema", help="keep, next ou entier explicite")
    parser.add_argument("--checkpoint", help="keep, current ou version explicite X.Y.Z")
    parser.add_argument("--non-interactive", action="store_true", help="ne pose aucune question; conserve les valeurs non précisées")
    return parser


def main() -> int:
    args = build_parser().parse_args()
    try:
        current, current_schema, current_checkpoint, version_source = read_current_state()
        new_version = compute_new_game_version(current, args.mode.strip().lower())
        interactive = (not args.non_interactive) and sys.stdin.isatty()
        new_schema = resolve_save_schema(current_schema, args.save_schema, interactive)
        new_checkpoint = resolve_checkpoint(current_checkpoint, new_version, args.checkpoint, interactive)
    except (ValueError, RuntimeError) as exc:
        print(exc, file=sys.stderr)
        return 2

    print("\nRésumé du changement :")
    print(f"  Version du jeu       : {current} -> {new_version}")
    print(f"  Schéma de sauvegarde : {current_schema} -> {new_schema}")
    print(f"  Checkpoint important : V{current_checkpoint} -> V{new_checkpoint}")

    if interactive and not ask_yes_no("Appliquer ces changements ?", default=True):
        print("Annulé. Aucun fichier modifié.")
        return 0

    updated, count = VERSION_RE.subn(rf'\g<1>{new_version}\g<3>', version_source, count=1)
    if count != 1:
        print("Impossible de mettre à jour la version du jeu.", file=sys.stderr)
        return 1
    VERSION_FILE.write_text(updated, encoding="utf-8")

    update_save_schema(current_schema, new_schema)
    update_checkpoint_docs(current_checkpoint, new_checkpoint)
    update_sync_files(new_version)

    print("\nDinotofu mis à jour.")
    if new_schema != current_schema:
        print(f"ATTENTION : saveVersion est passé de {current_schema} à {new_schema}. Vérifie/ajoute la migration correspondante.")
    if new_checkpoint != current_checkpoint:
        print(f"ATTENTION : V{new_checkpoint} devient un nouveau checkpoint obligatoire de sauvegarde.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
