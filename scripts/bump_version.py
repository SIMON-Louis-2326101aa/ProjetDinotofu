#!/usr/bin/env python3
"""
Bump Dinotofu version across the project (VersionInfo.cpp, READMEs, patchnotes, manifests).

Usage:
  python3 scripts/bump_version.py           # Increments patch (+1, default)
  python3 scripts/bump_version.py patch     # Increments patch (+1)
  python3 scripts/bump_version.py minor     # Increments minor (+1, patch=0)
  python3 scripts/bump_version.py major     # Increments major (+1, minor=0, patch=0)
  python3 scripts/bump_version.py 4.0.0     # Sets explicit version 4.00.00
  python3 scripts/bump_version.py 3.50.01   # Sets explicit version 3.50.01
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VERSION_FILE = ROOT / "src" / "core" / "VersionInfo.cpp"
VERSION_RE = re.compile(r'(std::string\s+VersionInfo::currentVersion\(\)\s*\{\s*return\s+")([0-9]+\.[0-9]+\.[0-9]+)("\s*;\s*\})', re.S)


def parse_version(text: str) -> tuple[int, int, int]:
    cleaned = text.strip().lstrip("vV")
    parts = cleaned.split(".")
    if len(parts) != 3 or not all(p.isdigit() for p in parts):
        raise ValueError(f"Version invalide : {text!r} (format attendu : X.Y.Z ou X.YY.ZZ)")
    return int(parts[0]), int(parts[1]), int(parts[2])


def format_version(parts: tuple[int, int, int]) -> str:
    return f"{parts[0]}.{parts[1]:02d}.{parts[2]:02d}"


def update_sync_files(old_ver: str, new_ver: str) -> None:
    # 1. READMEFR.md
    readme_fr = ROOT / "READMEFR.md"
    if readme_fr.exists():
        content = readme_fr.read_text(encoding="utf-8")
        content = content.replace(f"**V{old_ver}**", f"**V{new_ver}**")
        readme_fr.write_text(content, encoding="utf-8")

    # 2. README.md
    readme_en = ROOT / "README.md"
    if readme_en.exists():
        content = readme_en.read_text(encoding="utf-8")
        content = content.replace(f"**V{old_ver}**", f"**V{new_ver}**")
        readme_en.write_text(content, encoding="utf-8")

    # 3. CHANGELOG.md
    changelog = ROOT / "CHANGELOG.md"
    if changelog.exists():
        content = changelog.read_text(encoding="utf-8")
        if f"V{new_ver}" not in content:
            pattern = re.compile(r'(## V[0-9]+\.[0-9]+\.[0-9]+)')
            new_section = (
                f"## V{new_ver} — Notes de mise à jour   \n\n"
                f"- Mises à jour et améliorations de Dinotofu.   \n\n"
                f"---   \n\n"
            )
            if pattern.search(content):
                content = pattern.sub(rf"{new_section}\g<1>", content, count=1)
            else:
                content += f"\n\n{new_section}"
            changelog.write_text(content, encoding="utf-8")

    # 4. PATCHNOTE_DINOTOFU_FR.md (si encore présent sur d'anciennes branches)
    pn_fr = ROOT / "PATCHNOTE_DINOTOFU_FR.md"
    if pn_fr.exists():
        content = pn_fr.read_text(encoding="utf-8")
        content = re.sub(rf"# V{re.escape(old_ver)}", f"# V{new_ver}", content)
        content = re.sub(rf"- Version passée en V{re.escape(old_ver)}\.", f"- Version passée en V{new_ver}.", content)
        pn_fr.write_text(content, encoding="utf-8")

    # 5. PATCHNOTE_DINOTOFU.md (si encore présent sur d'anciennes branches)
    pn_en = ROOT / "PATCHNOTE_DINOTOFU.md"
    if pn_en.exists():
        content = pn_en.read_text(encoding="utf-8")
        content = re.sub(rf"# V{re.escape(old_ver)}", f"# V{new_ver}", content)
        content = re.sub(rf"- Version updated to V{re.escape(old_ver)}\.", f"- Version updated to V{new_ver}.", content)
        pn_en.write_text(content, encoding="utf-8")

    # 5. release/manifest.example.json
    manifest = ROOT / "release" / "manifest.example.json"
    if manifest.exists():
        content = manifest.read_text(encoding="utf-8")
        content = content.replace(old_ver, new_ver)
        manifest.write_text(content, encoding="utf-8")

    # 6. assets/branding/branding_manifest.json
    branding = ROOT / "assets" / "branding" / "branding_manifest.json"
    if branding.exists():
        content = branding.read_text(encoding="utf-8")
        content = content.replace(old_ver, new_ver)
        branding.write_text(content, encoding="utf-8")


def main() -> int:
    if len(sys.argv) == 1:
        mode = "patch"
    elif len(sys.argv) == 2:
        mode = sys.argv[1].strip().lower()
    else:
        print(__doc__.strip(), file=sys.stderr)
        return 2

    source = VERSION_FILE.read_text(encoding="utf-8")
    match = VERSION_RE.search(source)
    if not match:
        print("Impossible de trouver VersionInfo::currentVersion() dans src/core/VersionInfo.cpp", file=sys.stderr)
        return 1

    current = match.group(2)
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
        try:
            major, minor, patch = parse_version(mode)
        except ValueError as exc:
            print(exc, file=sys.stderr)
            print(__doc__.strip(), file=sys.stderr)
            return 2

    new_version = format_version((major, minor, patch))
    updated = VERSION_RE.sub(rf'\g<1>{new_version}\g<3>', source, count=1)
    VERSION_FILE.write_text(updated, encoding="utf-8")

    update_sync_files(current, new_version)

    print(f"Dinotofu : {current} -> {new_version}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
