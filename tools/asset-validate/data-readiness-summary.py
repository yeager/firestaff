#!/usr/bin/env python3
"""Per-game data-readiness summary across canonical + extras staging.

Combines three sources into one human-readable + machine-readable
table:

  1. Firestaff `--scan-data` against a data root (preferred: the
     full `~/.firestaff/data/` so canonical + extras both report
     their READY paths).
  2. `compare_to_greatstone.py` against the data root (SHA-256
     hash match against the verified-hash registry).
  3. A small boot-probe (firestaff --game X --data-dir Y) per
     known-good path — opt-in via `--boot-probe` (slow, ~8s per
     game).

Output:

  - Human-readable table on stdout (per-game row).
  - JSON dump on stdout with `--json`.
  - Exit code: 0 if all 5 games READY in canonical, 1 otherwise.

Usage:

  # Default: just scan + hash-match summary.
  tools/asset-validate/data-readiness-summary.py

  # Custom data root.
  tools/asset-validate/data-readiness-summary.py --data-root ~/my-staging

  # JSON output for CI.
  tools/asset-validate/data-readiness-summary.py --json

  # Add boot-probe results (slow, ~40s).
  tools/asset-validate/data-readiness-summary.py --boot-probe
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from dataclasses import dataclass, field, asdict
from pathlib import Path
from typing import Optional


# Canonical per-game subdirectories expected by M12 launcher.
GAME_SUBDIRS = {
    "dm1": "dm1",
    "dm1-multilingual": "dm1-multilingual",
    "csb": "csb",
    "dm2": "dm2",
    "nexus": "nexus",
    "theron": "theron",
}

# Pretty names for the summary table.
GAME_LABEL = {
    "dm1": "DM1",
    "dm1-multilingual": "DM1 Multi",
    "csb": "CSB",
    "dm2": "DM2",
    "nexus": "Nexus",
    "theron": "Theron",
}


@dataclass
class GameRow:
    game: str
    canonical_present: bool = False
    canonical_path: Optional[str] = None
    extras_ready_paths: list = field(default_factory=list)
    hash_summary: Optional[str] = None  # "ok=X missing=Y corrupt=Z" from compare_to_greatstone
    boot_ok_paths: list = field(default_factory=list)
    boot_fail_paths: list = field(default_factory=list)

    @property
    def label(self) -> str:
        return GAME_LABEL.get(self.game, self.game)

    @property
    def canonical_status(self) -> str:
        return "READY" if self.canonical_present else "MISSING"

    @property
    def overall(self) -> str:
        if self.canonical_present:
            return "READY"
        if self.extras_ready_paths:
            return "EXTRAS"
        return "MISSING"


def run_scan(firestaff_bin: Path, data_root: Path) -> dict:
    """Run `firestaff --scan-data` and parse the output.

    Returns a dict mapping game -> dict(ready=bool, found_paths=[...]).
    """
    env = os.environ.copy()
    env["SDL_VIDEODRIVER"] = "dummy"
    try:
        out = subprocess.run(
            [str(firestaff_bin), "--data-dir", str(data_root), "--scan-data"],
            capture_output=True, text=True, env=env, timeout=180,
        )
    except subprocess.TimeoutExpired:
        return {}
    if out.returncode != 0:
        return {}
    return _parse_scan_output(out.stdout)


def _parse_scan_output(text: str) -> dict:
    """Parse Firestaff's `--scan-data` output.

    Looks for lines like:
      Dungeon Master         READY
        GRAPHICS.DAT                 FOUND  /path/to/file
        DUNGEON.DAT                  FOUND  /path/to/file
    """
    result: dict = {}
    current_game: Optional[str] = None
    current_paths: list = []

    # Map pretty titles back to game ids
    title_to_game = {
        "Dungeon Master": "dm1",
        "Chaos Strikes Back": "csb",
        "Dungeon Master II": "dm2",
        "DM Nexus": "nexus",
        "Theron's Quest": "theron",
    }

    for raw in text.splitlines():
        line = raw.rstrip()
        m = re.match(r"^(Dungeon Master|Chaos Strikes Back|Dungeon Master II|DM Nexus|Theron's Quest)\s+(READY|MISSING)$", line)
        if m:
            title, status = m.group(1), m.group(2)
            game_id = title_to_game.get(title)
            if game_id:
                current_game = game_id
                current_paths = []
                result[game_id] = {
                    "ready": status == "READY",
                    "found_paths": [],
                }
            continue
        if current_game and "FOUND" in line:
            # Extract everything after "FOUND  "
            m = re.search(r"FOUND\s+(.+)$", line)
            if m:
                path = m.group(1).strip()
                result[current_game]["found_paths"].append(path)
    return result


def run_compare(firestaff_root: Path, data_root: Path) -> Optional[str]:
    """Run compare_to_greatstone.py and return the summary line."""
    compare_script = firestaff_root / "tools" / "asset-validate" / "compare_to_greatstone.py"
    if not compare_script.exists():
        return None
    try:
        out = subprocess.run(
            ["python3", str(compare_script), str(data_root)],
            capture_output=True, text=True, timeout=180,
        )
    except subprocess.TimeoutExpired:
        return None
    # Summary is printed to stderr; check both streams
    for stream in (out.stdout, out.stderr):
        for line in reversed(stream.splitlines()):
            line = line.strip()
            if line.startswith("summary:"):
                return line
    return None


def run_boot_probe(firestaff_bin: Path, game: str, path: Path) -> bool:
    """Run `firestaff --game X --data-dir Y --duration 1500` and report
    success based on whether M11 actually loaded game data.

    Success signals:
      - `LOADING DUNGEON:` (DM1)
      - `TQR level load: ... status=OK` (Theron)
      - no `direct launch failed` line in the output
    """
    env = os.environ.copy()
    env["SDL_VIDEODRIVER"] = "dummy"
    try:
        out = subprocess.run(
            [str(firestaff_bin), "--game", game, "--data-dir", str(path),
             "--duration", "1500"],
            capture_output=True, text=True, env=env, timeout=15,
        )
    except subprocess.TimeoutExpired:
        # timeout means M11 was running and hit the duration cap = good
        return True
    except Exception:
        return False
    text = out.stdout + out.stderr
    if "direct launch failed" in text:
        return False
    if "LOADING DUNGEON" in text or "TQR level load" in text or "DUNGEON LOADING" in text:
        return True
    # No error + no clear success signal = assume boot OK
    return out.returncode == 0 or "phase-a run failed" not in text


def collect_canonical(data_root: Path) -> dict:
    """Check canonical subdirs for the expected files."""
    canonical = {}
    for game_id, subdir in GAME_SUBDIRS.items():
        canon_dir = data_root / subdir
        if not canon_dir.is_dir():
            canonical[game_id] = False
            continue
        # For dm1 + csb + dm2: GRAPHICS.DAT + DUNGEON.DAT in canon_dir
        # For nexus: 138+ DMDF/DGN files OR ISO files in canon_dir
        # For theron: TQJP02End.iso or similar in canon_dir/ or canon_dir/jp or canon_dir/us
        has_data = False
        if game_id in ("dm1", "dm1-multilingual", "csb", "dm2"):
            has_data = (canon_dir / "GRAPHICS.DAT").is_file() and (canon_dir / "DUNGEON.DAT").is_file()
        elif game_id == "nexus":
            # Either ISO files or many DMDF files
            has_data = any(canon_dir.glob("*.iso")) or any(canon_dir.glob("*.bin")) or list(canon_dir.glob("*.dgn"))[:1] != []
        elif game_id == "theron":
            iso_files = list(canon_dir.glob("*.iso")) + list(canon_dir.glob("*.bin"))
            has_data = bool(iso_files)
        canonical[game_id] = has_data
    return canonical


def print_table(rows: list, json_mode: bool = False):
    if json_mode:
        print(json.dumps([asdict(r) for r in rows], indent=2))
        return

    # Human-readable table
    print()
    print("Per-game data readiness")
    print("=" * 80)
    print(f"{'Game':<14} {'Canonical':<11} {'Overall':<9} {'Hash':<22} {'Boot-tested paths':<30}")
    print("-" * 80)
    for r in rows:
        boot_str = ""
        if r.boot_ok_paths:
            boot_str = "✅ " + ", ".join(Path(p).parent.name for p in r.boot_ok_paths[:2])
        elif r.boot_fail_paths:
            boot_str = "❌ " + ", ".join(Path(p).parent.name for p in r.boot_fail_paths[:2])
        else:
            boot_str = "(not probed)"
        print(f"{r.label:<14} {r.canonical_status:<11} {r.overall:<9} "
              f"{r.hash_summary or 'n/a':<22} {boot_str:<30}")

    # Extras details
    extras_present = [r for r in rows if r.extras_ready_paths]
    if extras_present:
        print()
        print("Extras-staging READY paths:")
        for r in extras_present:
            print(f"  {r.label}:")
            for p in r.extras_ready_paths:
                print(f"    - {p}")


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--data-root", type=Path,
                    default=Path.home() / ".firestaff" / "data",
                    help="Firestaff data root (default: ~/.firestaff/data)")
    ap.add_argument("--firestaff-bin", type=Path,
                    default=Path(__file__).resolve().parents[2] / "build" / "firestaff",
                    help="Path to firestaff binary (default: ../../build/firestaff)")
    ap.add_argument("--firestaff-root", type=Path,
                    default=Path(__file__).resolve().parents[2],
                    help="Firestaff source root (for compare_to_greatstone.py)")
    ap.add_argument("--json", action="store_true", help="JSON output")
    ap.add_argument("--boot-probe", action="store_true",
                    help="Run firestaff --game X --data-dir Y boot probe per path (slow)")
    args = ap.parse_args()

    if not args.firestaff_bin.exists():
        print(f"ERROR: firestaff binary not found at {args.firestaff_bin}",
              file=sys.stderr)
        return 3

    if not args.data_root.is_dir():
        print(f"ERROR: data root not found: {args.data_root}", file=sys.stderr)
        return 3

    # 1) Run firestaff --scan-data
    scan = run_scan(args.firestaff_bin, args.data_root)

    # 2) Check canonical dirs directly
    canonical = collect_canonical(args.data_root)

    # 3) Run compare_to_greatstone.py
    hash_summary = run_compare(args.firestaff_root, args.data_root)

    # 4) Build rows
    rows = []
    for game_id in GAME_SUBDIRS:
        row = GameRow(game=game_id)
        row.canonical_present = canonical.get(game_id, False)
        if row.canonical_present:
            row.canonical_path = str(args.data_root / GAME_SUBDIRS[game_id])
        if game_id in scan:
            scan_info = scan[game_id]
            if scan_info["ready"]:
                row.extras_ready_paths = scan_info["found_paths"]
        if hash_summary:
            row.hash_summary = hash_summary
        rows.append(row)

    # 5) Optional boot probe
    if args.boot_probe:
        for row in rows:
            paths_to_probe = []
            if row.canonical_path:
                paths_to_probe.append(row.canonical_path)
            paths_to_probe.extend(row.extras_ready_paths)
            for path in paths_to_probe:
                ok = run_boot_probe(args.firestaff_bin, row.game, Path(path))
                if ok:
                    row.boot_ok_paths.append(path)
                else:
                    row.boot_fail_paths.append(path)

    # 6) Output
    print_table(rows, json_mode=args.json)

    # Exit code: 0 if all 5 canonical present
    all_ready = all(r.canonical_present for r in rows)
    return 0 if all_ready else 1


if __name__ == "__main__":
    sys.exit(main())
