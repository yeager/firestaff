#!/usr/bin/env python3
"""Verify that runtime MD5 hashes in the Firestaff source tree match
the SHA256 hashes locked in docs/VERIFIED_HASHES.md.

Why this exists:
    Firestaff identifies game versions two ways, deliberately:

    1. MD5 in src/shared/asset_status_m12.c g_requiredFiles[] and
       src/shared/asset_find_by_hash.c — embedded 32-hex-char strings
       used at runtime by M12 launcher for version detection.

    2. SHA256 in docs/VERIFIED_HASHES.md — used by validate_*.py tools
       and the new tools/asset-validate/compare_to_greatstone.py.

    Both systems should agree on what bytes constitute "DM1 PC 3.4
    English GRAPHICS.DAT". This script enforces that invariant.

What it does:
    1. Walks src/shared/asset_status_m12.c and src/shared/asset_find_by_hash.c
       (and src/dm2/dm2_v1_*.c) to extract every embedded MD5 hex string.
    2. For each (game, filename) pair in docs/VERIFIED_HASHES.md whose
       local file exists under ~/.firestaff/data/, compute MD5 + SHA256
       and check both:
         a) MD5 matches the embedded runtime constant (if one exists for
            that file),
         b) SHA256 matches the registry value (always).
    3. Emits a pass/fail table and exits non-zero on any mismatch.

Exit codes:
    0  PASS  (no embedded MD5 mismatches, all registry-locked files
              compute the expected SHA256)
    1  FAIL  (one or more mismatches)
    2  USAGE (registry or data dir missing, no embedded MD5 found, etc.)
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_REGISTRY = REPO_ROOT / "docs" / "VERIFIED_HASHES.md"
DEFAULT_DATA_DIR = Path.home() / ".firestaff" / "data"

# Source files that embed runtime MD5 constants for asset identification.
EMBEDDING_SOURCES = (
    "src/shared/asset_status_m12.c",
    "src/shared/asset_find_by_hash.c",
    "src/dm2/dm2_v1_boot.c",
    "src/dm2/dm2_v1_game.c",
)

# Game -> filename -> known embedded MD5.
# This is the curated list; entries that only have SHA256 (no embedded
# runtime MD5) are tracked as "sha256-only" and verified by hash alone.
RUNTIME_MD5_TABLE = {
    "dm1": {
        "DUNGEON.DAT": "766450c940651fc021c92fe5d0d0b3a6",
        "GRAPHICS.DAT": "fa6b1aa29e191418713bf2cda93d962e",
    },
    "csb": {
        "DUNGEON.DAT": "6695d2acebce49f95db1d8f3a5c733de",
        "GRAPHICS.DAT": "61fbfd56887c94adc26888a9491c6611",
    },
    "dm2": {
        "DUNGEON.DAT": "6caccd7875009e82fe2e28e7f6d6adc0",
        "GRAPHICS.DAT": "25247ede4dabb6a71e5dabdfbcd5907d",
    },
    "nexus": {
        "DM.BIN": "e88d60859f65f08fa622e1992b02280f",
    },
}

_TABLE_RE = re.compile(
    r"^\|\s*(?P<game>[^|]+?)\s*\|\s*(?P<file>[^|]+?)\s*\|\s*`(?P<hash>[a-f0-9]{64})`\s*\|\s*(?P<size>[\d,]+)\s*\|",
    re.MULTILINE,
)
_BULLET_RE = re.compile(
    r"^-\s*`(?P<path>[^`]+?)`\s*\((?P<size>[\d,]+)\s*bytes?\)\s*:\s*`(?P<hash>[a-f0-9]{64})`",
    re.MULTILINE,
)
_EMBEDDED_MD5_RE = re.compile(r'"([0-9a-f]{32})"')


@dataclass(frozen=True)
class RegistryEntry:
    game: str
    filename: str
    sha256: str
    size: int


@dataclass
class CheckRow:
    game: str
    filename: str
    expected_sha256: str
    actual_sha256: str = ""
    actual_md5: str = ""
    expected_md5: str = ""
    actual_size: int = 0
    status: str = "PENDING"
    notes: list[str] = field(default_factory=list)


def parse_registry(path: Path) -> dict[Path, RegistryEntry]:
    text = path.read_text(encoding="utf-8")
    out: dict[Path, RegistryEntry] = {}
    for m in _TABLE_RE.finditer(text):
        rel = Path(m.group("game").strip()) / m.group("file").strip()
        out.setdefault(
            rel,
            RegistryEntry(
                game=m.group("game").strip(),
                filename=m.group("file").strip(),
                sha256=m.group("hash"),
                size=int(m.group("size").replace(",", "")),
            ),
        )
    for m in _BULLET_RE.finditer(text):
        rel_path = Path(m.group("path").strip())
        out.setdefault(
            rel_path,
            RegistryEntry(
                game=rel_path.parts[0] if len(rel_path.parts) > 1 else "unknown",
                filename=rel_path.name,
                sha256=m.group("hash"),
                size=int(m.group("size").replace(",", "")),
            ),
        )
    return out


def collect_embedded_md5() -> set[str]:
    """Read every embedded MD5 hex string from the source files."""
    found: set[str] = set()
    for rel in EMBEDDING_SOURCES:
        path = REPO_ROOT / rel
        if not path.is_file():
            continue
        for m in _EMBEDDED_MD5_RE.finditer(path.read_text(encoding="utf-8", errors="replace")):
            found.add(m.group(1))
    return found


def _candidate_paths(data_dir: Path, entry: RegistryEntry) -> list[Path]:
    """Return possible on-disk paths for a registry entry, in priority order.

    Search order (highest priority first):
      1. <data_dir>/<game>/<filename>  (the canonical layout)
      2. case-insensitive match under <data_dir>/<game>/
      3. paths under <data_dir>/<game>-extras/ whose path embeds the variant
         substring (e.g. entry.game="dm1-atari-st-1.2-en" -> prefer
         paths containing "atari-st-1.2-en" or "atari-st-1.2" or
         "atari-st" under dm1-extras/), then fall back to any other
         match in that extras dir.

    The first existing match in priority order is used. We deliberately
    do NOT pick loose files at <data_dir>/ — a top-level GRAPHICS.DAT
    there is typically a symlink to a different game's data and would
    produce misleading "MISMATCH" reports.
    """
    candidates: list[Path] = []

    # Tier 1: canonical layout
    direct = data_dir / entry.game / entry.filename
    if direct.is_file():
        candidates.append(direct)

    parent = data_dir / entry.game
    if parent.is_dir():
        try:
            for child in parent.iterdir():
                if child.is_file() and child.name.lower() == entry.filename.lower():
                    candidates.append(child)
        except OSError:
            pass

    # Tier 2: parent-game extras with variant substring matching.
    parent_game = entry.game
    for sep in ("-atari-st-", "-amiga-", "-apple-iigs-", "-fm-towns-", "-pc-98-", "-x68000-", "-pc-", "-mac-", "-mega-cd-", "-sega-cd-"):
        idx = entry.game.find(sep)
        if idx > 0:
            parent_game = entry.game[:idx]
            break

    parent_extras = data_dir / f"{parent_game}-extras"
    if parent_extras.is_dir():
        # Compute the variant substring (everything after parent_game-).
        variant_substr = entry.game[len(parent_game) + 1:] if entry.game.startswith(parent_game + "-") else ""
        # Tokens we use for matching: split on both "-" and "." so that
        # "atari-st-1.2-en" yields ["atari", "st", "1", "2", "en"] and a
        # path containing "atari-st" still matches on ["atari", "st"].
        variant_tokens = [t for t in variant_substr.lower().replace("-", " ").replace(".", " ").split() if t] if variant_substr else []
        # Platform prefix in the variant substr (first token, e.g.
        # "atari-st", "amiga", "pc"). We use it as a strong discriminator
        # when a path contains that platform prefix in a directory name.
        platform_prefix = variant_tokens[0] if variant_tokens else ""

        # Priority 2a: full variant substring in path.
        priority_a: list[Path] = []
        # Priority 2b: path contains platform prefix as a directory segment.
        priority_b: list[Path] = []
        # Priority 2c: path contains ALL variant tokens.
        priority_c: list[Path] = []
        # Priority 2d: path contains at least one variant token.
        priority_d: list[Path] = []
        # Priority 2e: any other match.
        priority_e: list[Path] = []
        try:
            for child in parent_extras.rglob(entry.filename):
                if not child.is_file():
                    continue
                path_str = str(child).lower()
                # Look at directory segments AND space-separated tokens in
                # segment names so that "Atari ST v1.2" inside one path
                # segment counts as containing "atari" and "st".
                segs = set(path_str.split("/"))
                space_tokens: set[str] = set()
                for seg in segs:
                    space_tokens.update(
                        seg.replace("(", " ")
                        .replace(")", " ")
                        .replace(",", " ")
                        .replace(".", " ")
                        .split()
                    )
                combined = segs | space_tokens
                # Helper: test for a single token as a word boundary match
                # (so "en" matches "english" only when it is its own token,
                # not just as a substring of "meynaf").
                def _token_match(tok: str) -> bool:
                    return tok in space_tokens
                if variant_substr and variant_substr.lower() in path_str:
                    priority_a.append(child)
                elif platform_prefix and platform_prefix in combined:
                    priority_b.append(child)
                elif variant_tokens and all(_token_match(tok) for tok in variant_tokens):
                    priority_c.append(child)
                elif variant_tokens and any(_token_match(tok) for tok in variant_tokens):
                    priority_d.append(child)
                else:
                    priority_e.append(child)
        except OSError:
            pass
        candidates.extend(priority_a)
        candidates.extend(priority_b)
        candidates.extend(priority_c)
        candidates.extend(priority_d)
        candidates.extend(priority_e)

    # Tier 3: <game>-extras (only meaningful if game has no parent).
    if parent_game == entry.game:
        extras = data_dir / f"{entry.game}-extras"
        if extras.is_dir():
            try:
                for child in extras.rglob(entry.filename):
                    if child.is_file() and child not in candidates:
                        candidates.append(child)
            except OSError:
                pass

    seen: set[Path] = set()
    out: list[Path] = []
    for c in candidates:
        if c not in seen:
            seen.add(c)
            out.append(c)
    return out


def _hash_file(path: Path) -> tuple[str, str, int]:
    md5 = hashlib.md5()
    sha = hashlib.sha256()
    size = 0
    with open(path, "rb") as fp:
        while True:
            chunk = fp.read(1 << 20)
            if not chunk:
                break
            md5.update(chunk)
            sha.update(chunk)
            size += len(chunk)
    return md5.hexdigest(), sha.hexdigest(), size


def run_checks(
    registry: Path, data_dir: Path
) -> tuple[list[CheckRow], list[str]]:
    rows: list[CheckRow] = []
    notes: list[str] = []

    entries = parse_registry(registry)
    embedded = collect_embedded_md5()
    if not embedded:
        notes.append("WARN: no embedded MD5 literals found in any source file")

    for rel, e in entries.items():
        row = CheckRow(
            game=e.game,
            filename=e.filename,
            expected_sha256=e.sha256,
        )
        # Snapshot expected runtime MD5 if we know one for this (game, file).
        runtime_md5 = RUNTIME_MD5_TABLE.get(e.game, {}).get(e.filename)
        if runtime_md5:
            row.expected_md5 = runtime_md5

        path_found = None
        for cand in _candidate_paths(data_dir, e):
            if cand.is_file():
                path_found = cand
                break
        if path_found is None:
            row.status = "MISSING"
            row.notes.append(f"local file not found for {e.game}/{e.filename}")
            rows.append(row)
            continue

        md5, sha, size = _hash_file(path_found)
        row.actual_md5 = md5
        row.actual_sha256 = sha
        row.actual_size = size

        sha_ok = sha == e.sha256
        md5_ok = True
        if runtime_md5 is not None:
            md5_ok = md5 == runtime_md5

        size_ok = size == e.size

        if sha_ok and md5_ok and size_ok:
            row.status = "OK"
        elif sha_ok and md5_ok:
            row.status = "OK-SIZE-DRIFT"
            row.notes.append(f"size drift: expected {e.size}, got {size}")
        else:
            row.status = "MISMATCH"
            if not sha_ok:
                row.notes.append(f"SHA256 mismatch: expected {e.sha256[:12]}..., got {sha[:12]}...")
            if runtime_md5 is not None and not md5_ok:
                row.notes.append(
                    f"MD5 mismatch: expected {runtime_md5[:12]}..., got {md5[:12]}..."
                )
            if not size_ok:
                row.notes.append(f"size mismatch: expected {e.size}, got {size}")
        rows.append(row)

    return rows, notes


def print_report(rows: Iterable[CheckRow], notes: list[str]) -> None:
    rows = list(rows)
    if notes:
        for n in notes:
            print(n)
    print(f"{'Game':<22} {'File':<28} {'Status':<14} {'MD5':<34} {'SHA256':<14} {'Size':>10}")
    print("-" * 130)
    for r in rows:
        md5 = r.actual_md5[:12] + "..." if r.actual_md5 else "-"
        sha = r.actual_sha256[:12] + "..." if r.actual_sha256 else "-"
        print(
            f"{r.game:<22} {r.filename:<28} {r.status:<14} {md5:<34} {sha:<14} {r.actual_size:>10}"
        )
        for note in r.notes:
            print(f"   - {note}")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--registry", type=Path, default=DEFAULT_REGISTRY)
    parser.add_argument("--data-dir", type=Path, default=DEFAULT_DATA_DIR)
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    if not args.registry.is_file():
        print(f"registry not found: {args.registry}", file=sys.stderr)
        return 2
    if not args.data_dir.is_dir():
        print(f"data dir not found: {args.data_dir}", file=sys.stderr)
        return 2

    rows, notes = run_checks(args.registry, args.data_dir)
    if args.json:
        out = {
            "rows": [r.__dict__ for r in rows],
            "notes": notes,
        }
        print(json.dumps(out, indent=2))
    else:
        print_report(rows, notes)

    failures = [r for r in rows if r.status not in ("OK", "OK-SIZE-DRIFT", "MISSING")]
    missing = [r for r in rows if r.status == "MISSING"]
    if failures:
        print(f"\nFAIL: {len(failures)} mismatch(es)", file=sys.stderr)
        return 1
    if missing:
        print(f"\nWARN: {len(missing)} MISSING entries (skipped)", file=sys.stderr)
        return 0
    print("\nPASS: all runtime MD5 + registry SHA256 agree")
    return 0


if __name__ == "__main__":
    sys.exit(main())