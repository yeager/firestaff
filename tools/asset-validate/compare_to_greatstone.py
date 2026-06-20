#!/usr/bin/env python3
"""Compare local game data files against the Firestaff verified-hash registry.

This is the general-purpose "is the data I have the data you expect" probe
that backs docs/PLATFORM_MATRIX.md. It supersedes tools/validate_dm1_pc34_provenance.py
by reading from a single source of truth (docs/VERIFIED_HASHES.md) and
covering all five supported games.

The registry entries are SHA-256 checksums of the original (unmodified) game
data files. Hashes were derived from greatstone-free-fr's sck tool
extractions (http://greatstone.free.fr/dm/) and Meynaf's ReDMCSB decompilation.

Why SHA-256 (and not MD5):
  - The docs/VERIFIED_HASHES.md file uses SHA-256.
  - src/shared/asset_find_by_hash.c uses MD5 for runtime discovery.
  - This probe uses SHA-256 to match the documentation. The two should
    converge; for now this is the most useful tool for human review.
  - See docs/PLATFORM_MATRIX.md "See also" section for context.

Usage:
    # Validate the canonical DM1 PC 3.4 file
    compare_to_greatstone.py ~/.firestaff/data/dm1/GRAPHICS.DAT

    # Validate every file in a data directory
    compare_to_greatstone.py ~/.firestaff/data/dm1/

    # Recursive scan of a Firestaff data root
    compare_to_greatstone.py ~/.firestaff/data/

    # List every game/version the registry knows about
    compare_to_greatstone.py --list

    # Validate against the registry but only print failures
    compare_to_greatstone.py --quiet ~/.firestaff/data/

Exit codes:
    0 - every file matched its expected hash (or was unknown but not corrupt)
    1 - at least one file failed hash check (corrupt or wrong version)
    2 - at least one file was missing when the registry expected it
    3 - input path doesn't exist
    4 - registry not found (docs/VERIFIED_HASHES.md missing)
"""
from __future__ import annotations

import argparse
import hashlib
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_REGISTRY = REPO_ROOT / "docs" / "VERIFIED_HASHES.md"


@dataclass(frozen=True)
class HashEntry:
    """One (game, filename, sha256, size_bytes) tuple from the registry."""

    game: str
    filename: str
    sha256: str
    size: int

    @property
    def expected_path(self) -> Path:
        return Path(self.game) / self.filename


def parse_registry(path: Path) -> dict[Path, HashEntry]:
    """Parse docs/VERIFIED_HASHES.md into a {relative_path -> HashEntry} map.

    Supports both the markdown table form ("| game | file | hash | size |")
    and the bullet form ("- `path/to/file` (N bytes): `hash`") so the script
    keeps working if the registry format evolves.
    """
    out: dict[Path, HashEntry] = {}
    text = path.read_text(encoding="utf-8")

    # Table rows: | game | filename | `hash` | size[,] |
    table_re = re.compile(
        r"^\|\s*(?P<game>[^|]+?)\s*\|\s*(?P<file>[^|]+?)\s*\|\s*`(?P<hash>[a-f0-9]{64})`\s*\|\s*(?P<size>[\d,]+)\s*\|",
        re.MULTILINE,
    )
    for m in table_re.finditer(text):
        game = m.group("game").strip()
        filename = m.group("file").strip()
        sha = m.group("hash")
        size = int(m.group("size").replace(",", ""))
        out[Path(game) / filename] = HashEntry(
            game=game, filename=filename, sha256=sha, size=size
        )

    # Bullet rows: - `game/file` (N bytes): `hash`
    bullet_re = re.compile(
        r"^-\s*`(?P<path>[^`]+?)`\s*\((?P<size>[\d,]+)\s*bytes?\)\s*:\s*`(?P<hash>[a-f0-9]{64})`",
        re.MULTILINE,
    )
    for m in bullet_re.finditer(text):
        relpath = Path(m.group("path").strip())
        sha = m.group("hash")
        size = int(m.group("size").replace(",", ""))
        if relpath in out:
            continue  # table row already covered it
        out[relpath] = HashEntry(
            game=str(relpath.parent), filename=relpath.name,
            sha256=sha, size=size,
        )
    return out


def sha256_file(path: Path, chunk_bytes: int = 1 << 20) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(chunk_bytes), b""):
            h.update(chunk)
    return h.hexdigest()


def resolve_candidate_path(path: Path) -> Path | None:
    """Map a user-supplied path to a relative registry key.

    A user might point at:
      ~/.firestaff/data/dm1/GRAPHICS.DAT     -> "dm1/GRAPHICS.DAT"
      ~/.firestaff/data/dm1/                 -> "dm1/" (directory scan)
      ~/DATA/GRAPHICS.DAT                    -> ?? (look at parent for game tag)
      /tmp/randomfile                        -> None
    """
    parts = path.parts
    # Find the first directory component that matches a known game tag
    known_games = {"dm1", "dm1-multilingual", "csb", "dm2", "theron", "nexus"}
    for i, part in enumerate(parts):
        if part in known_games:
            tail = Path(*parts[i + 1:])
            if path.is_dir():
                return Path(part) / ""
            return Path(part) / tail

    # If user gave a bare file path (no game tag in path), we can't map it
    # to a specific registry entry -- there could be many games with the
    # same filename (e.g. DUNGEON.DAT exists in dm1, csb, dm2). The user
    # must point at a path inside a game-tag directory OR pass the game
    # explicitly via --game.
    return None


def check_file(path: Path, expected: HashEntry) -> tuple[str, str, int, str]:
    """Compute sha256 of path, compare to expected. Return (status, msg)."""
    actual_size = path.stat().st_size
    actual_hash = sha256_file(path)
    if actual_size != expected.size:
        return (
            "FAIL", f"size mismatch: got {actual_size} bytes, expected {expected.size}",
            actual_size, actual_hash,
        )
    if actual_hash != expected.sha256:
        return (
            "FAIL", f"hash mismatch:\n         got  {actual_hash}\n         want {expected.sha256}",
            actual_size, actual_hash,
        )
    return ("OK", "", actual_size, actual_hash)


def scan_file(
    path: Path,
    registry: dict[Path, HashEntry],
    quiet: bool = False,
) -> int:
    """Check a single user file. Returns 0 (ok/unknown), 1 (corrupt), 2 (missing)."""
    rel = resolve_candidate_path(path)
    if rel is None or rel == Path():
        if not quiet:
            print(f"SKIP  {path}: cannot map to a known registry path")
        return 0
    expected = registry.get(rel)
    if expected is None:
        if not quiet:
            print(f"UNKNOWN  {path}: not in registry (not necessarily an error; "
                  f"the file may be from a version we don't have a hash for)")
        return 0
    status, msg, size, sha = check_file(path, expected)
    line = f"{status:6}  {path}"
    if status == "OK":
        if not quiet:
            print(f"{line} ({size} bytes, {sha[:12]}…)")
    else:
        print(f"{line}", file=sys.stderr)
        if msg:
            print(f"       {msg}", file=sys.stderr)
        return 1
    return 0


def scan_directory(
    root: Path,
    registry: dict[Path, HashEntry],
    quiet: bool = False,
) -> tuple[int, int, int]:
    """Walk root looking for files that match registry entries.

    Returns (ok, missing, corrupt) counts.
    """
    # Build a quick lookup: {filename -> [HashEntry, ...]} for matching
    by_name: dict[str, list[HashEntry]] = {}
    for entry in registry.values():
        by_name.setdefault(entry.filename, []).append(entry)

    # First, build a list of all files under root. Resolve symlinks
    # so we hash the real data, not the symlink itself.
    all_files: list[Path] = []
    for p in root.rglob("*"):
        if not p.is_file() and not p.is_symlink():
            continue
        try:
            real = p.resolve()
            if real.is_file():
                all_files.append(p)
        except OSError:
            # Broken symlink; skip
            continue

    # Find expected files in this scan
    found: dict[Path, Path] = {}  # registry_path -> filesystem_path
    for f in all_files:
        name = f.name
        if name not in by_name:
            continue
        try:
            rel = f.relative_to(root)
        except ValueError:
            rel = Path(name)
        rel_str = str(rel)
        # Pick the registry entry whose game directory is the first
        # path component of rel (so dm1-multilingual doesn't match
        # entries registered for dm1). This is the strictest match.
        candidates: list[HashEntry] = []
        for entry in by_name[name]:
            if rel.parts and rel.parts[0] == entry.game:
                candidates.append(entry)
            elif rel == entry.expected_path:
                candidates.append(entry)
            elif "/" + entry.game + "/" in "/" + rel_str + "/":
                candidates.append(entry)
        # If no strict match, fall back to the single registry entry
        # whose expected path is a suffix of the relative path.
        if not candidates and len(by_name[name]) == 1:
            candidates = by_name[name]
        for entry in candidates:
            found[entry.expected_path] = f
            break

    ok = missing = corrupt = 0
    checked_paths: set[Path] = set()

    # 1. Verify everything we found
    for regpath, fpath in found.items():
        checked_paths.add(regpath)
        expected = registry[regpath]
        status, msg, size, sha = check_file(fpath, expected)
        if status == "OK":
            ok += 1
            if not quiet:
                print(f"OK     {fpath} -> {regpath} ({size} bytes)")
        else:
            corrupt += 1
            print(f"FAIL   {fpath} -> {regpath}", file=sys.stderr)
            if msg:
                print(f"       {msg}", file=sys.stderr)

    # 2. Anything in the registry that we expected but didn't find
    scanned_games: set[str] = set()
    for p in all_files:
        for game in {"dm1", "dm1-multilingual", "csb", "dm2", "theron", "nexus"}:
            if f"/{game}/" in str(p) or str(p).endswith(f"/{game}"):
                scanned_games.add(game)
                break
    for regpath, entry in registry.items():
        if regpath in checked_paths:
            continue
        # Only flag missing if the game's directory is in the scanned tree
        if entry.game not in scanned_games:
            continue
        missing += 1
        if not quiet:
            print(f"MISS   expected {regpath} not found in {root}")

    return ok, missing, corrupt


def cmd_list(registry: dict[Path, HashEntry]) -> int:
    by_game: dict[str, list[HashEntry]] = {}
    for entry in registry.values():
        by_game.setdefault(entry.game, []).append(entry)
    for game in sorted(by_game):
        entries = sorted(by_game[game], key=lambda e: e.filename)
        print(f"== {game} ({len(entries)} file(s)) ==")
        for e in entries:
            print(f"  {e.filename:<24}  {e.size:>12,} bytes   sha256={e.sha256[:16]}…")
    return 0


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(
        description="Compare local game data files against Firestaff's verified-hash registry.",
    )
    p.add_argument("path", nargs="?", help="File or directory to validate. "
                                            "Omit if --list.")
    p.add_argument("--registry", type=Path, default=DEFAULT_REGISTRY,
                   help=f"Path to VERIFIED_HASHES.md (default: {DEFAULT_REGISTRY})")
    p.add_argument("--list", action="store_true",
                   help="List every game/file in the registry, then exit")
    p.add_argument("--quiet", action="store_true",
                   help="Only print failures (and missing files), not successes")
    p.add_argument("--no-color", action="store_true", help="(reserved)")
    args = p.parse_args(argv)

    if not args.registry.exists():
        print(f"registry not found: {args.registry}", file=sys.stderr)
        return 4

    registry = parse_registry(args.registry)
    if not registry:
        print(f"no entries found in registry: {args.registry}", file=sys.stderr)
        return 4

    if args.list:
        return cmd_list(registry)

    if not args.path:
        p.error("path is required unless --list is given")

    target = Path(args.path).expanduser()
    if not target.exists():
        print(f"path not found: {target}", file=sys.stderr)
        return 3

    if target.is_file():
        rc = scan_file(target, registry, args.quiet)
        return rc
    else:
        ok, missing, corrupt = scan_directory(target, registry, args.quiet)
        print()
        print(f"summary: ok={ok}  missing={missing}  corrupt={corrupt}", file=sys.stderr)
        if corrupt:
            return 1
        if missing:
            return 2
        return 0


if __name__ == "__main__":
    sys.exit(main())
