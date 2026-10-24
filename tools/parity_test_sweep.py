#!/usr/bin/env python3
"""Discover and run Firestaff's existing CTest gates by game.

This is an execution aid, not a parity score. A passing test proves only the
contract that test checks. Tests with no game label are associated only when
their CTest name has an explicit game prefix; shared tests are never credited
to every game by default.
"""
from __future__ import annotations

import argparse
import datetime as dt
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Any

GAMES = ("dm1", "dm2", "csb", "theron", "nexus")
ALIASES = {"dm1": ("dm1",), "dm2": ("dm2",), "csb": ("csb",),
           "theron": ("theron", "tqr"), "nexus": ("nexus",)}
PLATFORM_ALIASES = {
    # Keep this game-specific: for example PC34 compatibility tests are not
    # evidence of an original DOS release for CSB, Nexus, or Theron.
    "dm1": {
        "amiga": ("amiga",), "atari-st": ("atari-st", "atari"),
        "dos-pc": ("dos-pc", "dos", "pc34", "pc"),
        "fm-towns": ("fm-towns", "fmtowns", "towns"),
        "pc-98": ("pc-98", "pc98"), "x68000": ("x68000",),
    },
    "dm2": {
        "amiga": ("amiga",), "dos-pc": ("dos-pc", "dos", "pc34", "pc"),
        "fm-towns": ("fm-towns", "fmtowns", "towns"),
        "macintosh": ("macintosh", "mac"), "pc-98": ("pc-98", "pc98"),
        "sega-cd": ("sega-cd", "segacd"),
    },
    "csb": {
        "amiga": ("amiga",), "atari-st": ("atari-st", "atari"),
        "fm-towns": ("fm-towns", "fmtowns", "towns"),
        "pc-98": ("pc-98", "pc98"), "x68000": ("x68000",),
    },
    "theron": {"pc-engine": ("pc-engine", "pce", "track02")},
    "nexus": {"saturn": ("saturn",)},
}


def run(command: list[str], *, cwd: Path | None = None) -> subprocess.CompletedProcess[str]:
    return subprocess.run(command, cwd=cwd, text=True, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, check=False)


def load_inventory(build_dir: Path) -> list[dict[str, Any]]:
    proc = run(["ctest", "--test-dir", str(build_dir), "--show-only=json-v1"])
    if proc.returncode:
        raise RuntimeError(f"CTest inventory failed ({proc.returncode}):\n{proc.stdout}")
    try:
        return json.loads(proc.stdout)["tests"]
    except (json.JSONDecodeError, KeyError) as exc:
        raise RuntimeError(f"CTest returned invalid JSON inventory: {exc}") from exc


def labels(test: dict[str, Any]) -> set[str]:
    for prop in test.get("properties", []):
        if prop.get("name") == "LABELS":
            return {str(v).casefold() for v in prop.get("value", [])}
    return set()


def belongs(test: dict[str, Any], game: str, include_shared: bool) -> bool:
    name = str(test.get("name", "")).casefold()
    labs = labels(test)
    aliases = ALIASES[game]
    if any(re.search(rf"(^|[^a-z0-9]){re.escape(a)}([^a-z0-9]|$)", label) for label in labs for a in aliases):
        return True
    # Legacy CTest entries often lack labels. Only explicit name prefixes are
    # accepted; broad substring matching would misattribute shared tests.
    if any(re.match(rf"^{re.escape(a)}(?:_|-|$)", name) for a in aliases):
        return True
    return include_shared and "cross-cutting" in labs


def platform_evidence(tests: list[dict[str, Any]], game: str) -> dict[str, int]:
    """Count test names/labels that explicitly identify a known platform."""
    counts: dict[str, int] = {}
    for test in tests:
        name = str(test.get("name", "")).casefold()
        labs = labels(test)
        for platform, aliases in PLATFORM_ALIASES[game].items():
            if any(alias in labs or re.search(
                    rf"(^|[^a-z0-9]){re.escape(alias)}([^a-z0-9]|$)", name)
                    for alias in aliases):
                counts[platform] = counts.get(platform, 0) + 1
    return dict(sorted(counts.items()))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=Path("build"),
                        help="Configured CMake build directory (default: build)")
    parser.add_argument("--game", choices=GAMES + ("all",), action="append",
                        help="Game sweep; repeat to select several (default: all)")
    parser.add_argument("--platform", help="Also filter selected tests by a CTest label/name token")
    parser.add_argument("--list", action="store_true", help="List selected CTest tests without running")
    parser.add_argument("--list-platforms", action="store_true",
                        help="Report platform identifiers explicitly present in game test names/labels")
    parser.add_argument("--include-shared", action="store_true",
                        help="Include tests explicitly labeled cross-cutting")
    parser.add_argument("--results", type=Path,
                        help="Write machine-readable JSON results to this path")
    parser.add_argument("--parallel", type=int, default=1,
                        help="CTest parallel jobs per game (default: 1)")
    parser.add_argument("--limit-per-game", type=int,
                        help="Run only the first N discovered tests per game (labels result partial)")
    args = parser.parse_args()
    if args.parallel < 1:
        parser.error("--parallel must be at least 1")
    if args.limit_per_game is not None and args.limit_per_game < 1:
        parser.error("--limit-per-game must be at least 1")
    build_dir = args.build_dir.expanduser().resolve()
    selected_games = args.game or ["all"]
    games = list(GAMES) if "all" in selected_games else list(dict.fromkeys(selected_games))
    try:
        inventory = load_inventory(build_dir)
    except RuntimeError as exc:
        print(str(exc), file=sys.stderr)
        return 2
    if not inventory:
        print(f"CTest registered no tests in {build_dir}; configure and build Firestaff first.",
              file=sys.stderr)
        return 2

    report: dict[str, Any] = {
        "schema": "firestaff.parity_test_sweep.v1",
        "generatedAtUtc": dt.datetime.now(dt.timezone.utc).isoformat(),
        "buildDirectory": str(build_dir),
        "scope": "CTest execution coverage only; not a game parity or completion claim",
        "games": {},
    }
    failed = False
    for game in games:
        tests = [t for t in inventory if belongs(t, game, args.include_shared)]
        platform_counts = platform_evidence(tests, game)
        if args.platform:
            needle = args.platform.casefold()
            tests = [t for t in tests if needle in str(t.get("name", "")).casefold()
                     or any(needle in label for label in labels(t))]
        names = [str(t.get("name", "")) for t in tests]
        if args.list or args.list_platforms:
            print(f"[{game}] discovered={len(names)}")
            if args.list:
                for name in names:
                    print(f"  {name}")
            report["games"][game] = {"discovered": len(names), "tests": names, "status": "listed"}
            report["games"][game]["platformIdentifiers"] = platform_counts
            print(f"[{game}] platform-identifiers=" + json.dumps(platform_counts, sort_keys=True))
            continue
        if not names:
            print(f"[{game}] discovered=0 status=NO_TESTS_MATCHED")
            report["games"][game] = {"discovered": 0, "tests": [], "status": "no-tests-matched"}
            failed = True
            continue
        selected_names = names[:args.limit_per_game] if args.limit_per_game else names
        # CTest uses its own regex dialect. CTest test names currently use
        # alphanumerics, underscore, and hyphen, which are safe in an ERE
        # alternation outside a character class. Fail closed on future names
        # requiring dialect-specific escaping.
        if any(not re.fullmatch(r"[A-Za-z0-9_-]+", name) for name in selected_names):
            print(f"[{game}] unsupported character in CTest name; refusing unsafe regex filter",
                  file=sys.stderr)
            return 2
        command = ["ctest", "--test-dir", str(build_dir), "--output-on-failure",
                   "--parallel", str(args.parallel), "-R", "^(" + "|".join(selected_names) + ")$"]
        proc = run(command)
        ran = re.search(r"\btests failed out of (\d+)", proc.stdout)
        matched_none = "No tests were found" in proc.stdout or not ran or int(ran.group(1)) == 0
        has_skips = "***Skipped" in proc.stdout or "The following tests did not run:" in proc.stdout
        if proc.returncode != 0 or matched_none:
            status = "failed"
        elif has_skips:
            status = "partial-passed-with-skips" if len(selected_names) < len(names) else "passed-with-skips"
        else:
            status = "partial-passed" if len(selected_names) < len(names) else "passed"
        print(f"[{game}] discovered={len(names)} selected={len(selected_names)} status={status} exit={proc.returncode}")
        print(proc.stdout, end="" if proc.stdout.endswith("\n") else "\n")
        report["games"][game] = {"discovered": len(names), "selected": len(selected_names),
                                 "tests": selected_names, "platformIdentifiers": platform_counts,
                                 "status": status, "exitCode": proc.returncode,
                                 "ctestOutput": proc.stdout}
        failed |= status == "failed"

    if args.results:
        output = args.results.expanduser().resolve()
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
        print(f"results={output}")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
