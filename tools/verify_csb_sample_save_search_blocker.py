#!/usr/bin/env python3
"""Inventory N2-local CSB saved-game sample coverage.

This verifier is intentionally non-invasive: it records whether an extracted,
curated CSBGAME*.DAT sample is present in the approved N2-local reference roots.
It exits 0 for both presence and absence so it can be used as a blocker/evidence
probe without breaking unrelated lanes.
"""
from __future__ import annotations

import json
import re
import subprocess
from pathlib import Path

HOME = Path.home()
ROOTS = [
    HOME / ".openclaw/data/firestaff-original-games/DM",
    HOME / ".openclaw/data/firestaff-original-games/DM/_extracted",
]
OUTPUT = Path("parity-evidence/verification/csb_sample_save_search_blocker.json")
CSB_SAMPLE_RE = re.compile(r"^CSBGAME(?:[0-9FG]|[0-9]+)?\.(?:DAT|BAK)$", re.IGNORECASE)
GENERAL_SAVE_RE = re.compile(r"(?:CSBGAME|DMGAME|DMSAVE|SKSAVE|SAVE|\.SAV$)", re.IGNORECASE)
IMAGE_SUFFIXES = {".adf", ".msa", ".st"}
ARCHIVE_SUFFIXES = {".zip"}


def rel(path: Path) -> str:
    try:
        return "~/" + str(path.relative_to(HOME))
    except ValueError:
        return str(path)


def iter_files():
    seen: set[Path] = set()
    for root in ROOTS:
        if not root.exists():
            continue
        for p in root.rglob("*"):
            if p.is_file() and p not in seen:
                seen.add(p)
                yield p


def archive_hits(path: Path) -> list[str]:
    try:
        proc = subprocess.run(["unzip", "-l", str(path)], text=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, timeout=10, check=False)
    except (OSError, subprocess.TimeoutExpired):
        return []
    hits: list[str] = []
    for line in proc.stdout.splitlines():
        if GENERAL_SAVE_RE.search(line):
            hits.append(line.strip())
    return hits[:20]


def image_string_hits(path: Path) -> list[str]:
    try:
        proc = subprocess.run(["strings", "-a", str(path)], text=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, timeout=15, check=False)
    except (OSError, subprocess.TimeoutExpired, UnicodeDecodeError):
        return []
    hits: list[str] = []
    for line in proc.stdout.splitlines():
        if GENERAL_SAVE_RE.search(line):
            hits.append(line.strip())
            if len(hits) >= 20:
                break
    return hits


def main() -> int:
    exact_extracted_samples = []
    exact_save_named_files = []
    archives = []
    images = []
    for p in sorted(iter_files(), key=lambda x: str(x).lower()):
        name = p.name
        if CSB_SAMPLE_RE.match(name):
            exact_extracted_samples.append({"path": rel(p), "size": p.stat().st_size})
        if GENERAL_SAVE_RE.search(name):
            exact_save_named_files.append({"path": rel(p), "size": p.stat().st_size})
        suffix = p.suffix.lower()
        if suffix in ARCHIVE_SUFFIXES:
            hits = archive_hits(p)
            if hits:
                archives.append({"path": rel(p), "hits": hits})
        elif suffix in IMAGE_SUFFIXES:
            hits = image_string_hits(p)
            if hits:
                images.append({"path": rel(p), "string_hits": hits})

    result = {
        "schema": "firestaff.csb_sample_save_search_blocker.v1",
        "pass": True,
        "verifier_semantics": "Inventory only; exits 0 whether a curated sample is present or absent.",
        "roots": [rel(r) for r in ROOTS],
        "curated_csbgame_dat_sample_present": bool(exact_extracted_samples),
        "exact_extracted_csbgame_samples": exact_extracted_samples,
        "save_named_filesystem_entries": exact_save_named_files,
        "archive_save_name_hits": archives,
        "disk_image_save_string_hits": images,
        "source_lock_compatibility_criteria": [
            "Use a real extracted CSBGAME*.DAT/BAK saved-game sample, not a game/utility executable disk string reference.",
            "Decoded save header must use C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK (DEFS.H:496-498; CEDTINCU.C:27-32).",
            "CSB_SAVE_HEADER.DungeonID must be C13_DUNGEON_CSB_GAME for CSB game saves (DEFS.H:482-494, 519-523; CEDTINCU.C:49-58; CEDTINCH.C:55-58).",
            "Platform/FormatID must pass the source gates for the targeted media (CEDTINCU.C:49-58; CEDTINCH.C:55-58).",
            "Save routing must map C13_DUNGEON_CSB_GAME or C12_DUNGEON_CSB_PRISON to M746_FILE_ID_SAVE_CSBGAME_DAT, not DMSAVE.DAT (CEDTINC8.C:101-118).",
        ],
        "blocker": None if exact_extracted_samples else "No extracted curated CSBGAME*.DAT/BAK saved-game sample is present under the approved N2 original-game roots.",
    }
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(json.dumps(result, indent=2, sort_keys=False) + "\n")
    if exact_extracted_samples:
        print(f"PASS csb sample-save inventory: {len(exact_extracted_samples)} exact extracted sample(s) present")
    else:
        print("PASS csb sample-save inventory: blocker recorded; no exact extracted CSBGAME*.DAT/BAK sample present")
    print(f"wrote {OUTPUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
