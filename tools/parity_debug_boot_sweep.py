#!/usr/bin/env python3
"""Run real-data Firestaff debug boot probes by game and supported platform.

This reports selected media and startup receipts. It is not a gameplay or
parity score; a boot receipt proves only that the selected route reached its
reported startup state.
"""
from __future__ import annotations

import argparse
import datetime as dt
import json
import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Any

MATRIX: dict[str, tuple[tuple[str, str | None], ...]] = {
    "dm1": (("pc", None), ("amiga", None), ("atari-st", None), ("fm-towns", None)),
    "csb": (("amiga", None), ("atari-st", None), ("fm-towns", None)),
    "dm2": (("pc", None), ("amiga", None), ("fm-towns", None), ("mac", None)),
    "theron": (("pce", "jp"), ("pce", "us")),
    "nexus": (("saturn", None),),
}


def receipt_fields(stdout: str) -> dict[str, str]:
    line = next((line for line in stdout.splitlines()
                 if line.startswith("FIRESTAFF BOOT PROBE READY:")), "")
    fields = dict(re.findall(r"([A-Za-z][A-Za-z0-9]*)=([^ ]+)", line))
    data_dir = re.search(r"\bdataDir=(.*?)\s+frames=", line)
    if data_dir:
        fields["dataDir"] = data_dir.group(1)
    return fields


def run_probe(app: Path, data_dir: Path, game: str, platform: str,
              edition: str | None, timeout: int) -> dict[str, Any]:
    command = [str(app), "--game", game, "--platform", platform,
               "--data-dir", str(data_dir / game), "--debug", "--boot-probe"]
    if edition:
        command += ["--theron-native", edition]
    if not (data_dir / game).is_dir():
        return {"game": game, "platform": platform, "edition": edition,
                "status": "missing-data-directory", "command": command,
                "returnCode": None, "receipt": {}, "startupBlocker": None,
                "debugOutput": ""}
    env = os.environ.copy()
    env.setdefault("SDL_VIDEODRIVER", "dummy")
    try:
        proc = subprocess.run(command, text=True, stdout=subprocess.PIPE,
                              stderr=subprocess.STDOUT, check=False,
                              timeout=timeout, env=env)
        output = proc.stdout
        fields = receipt_fields(output)
        startup_block = re.search(
            r"NEXUS STARTUP RECEIPT: status=blocked .*?blocker=([^\s]+)", output)
        missing_media = ("SELECTED VERSION NOT FOUND" in output or
                         "game unavailable for --game" in output)
        if startup_block:
            status = "blocked"
        elif missing_media:
            status = "missing-media"
        elif proc.returncode != 0:
            status = "failed"
        elif not fields:
            status = "missing-boot-receipt"
        else:
            status = "boot-probe-ready"
        return {"game": game, "platform": platform, "edition": edition,
                "status": status, "command": command, "returnCode": proc.returncode,
                "receipt": fields,
                "startupBlocker": startup_block.group(1) if startup_block else None,
                "missingMedia": missing_media,
                "debugOutput": output}
    except subprocess.TimeoutExpired as exc:
        partial = exc.stdout or ""
        if isinstance(partial, bytes):
            partial = partial.decode("utf-8", errors="replace")
        return {"game": game, "platform": platform, "edition": edition,
                "status": "timeout", "command": command, "returnCode": None,
                "receipt": receipt_fields(partial), "startupBlocker": None,
                "debugOutput": partial}
    except OSError as exc:
        return {"game": game, "platform": platform, "edition": edition,
                "status": "launch-error", "command": command, "returnCode": None,
                "receipt": {}, "startupBlocker": None, "debugOutput": str(exc)}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--app", type=Path, required=True, help="Built Firestaff executable")
    parser.add_argument("--data-dir", type=Path,
                        default=Path.home() / ".firestaff" / "data",
                        help="Root with dm1, dm2, csb, theron, and nexus data directories")
    parser.add_argument("--game", choices=tuple(MATRIX) + ("all",), action="append",
                        help="Select games; repeatable (default: all)")
    parser.add_argument("--platform", action="append",
                        help="Filter by a platform token such as saturn or amiga; repeatable")
    parser.add_argument("--timeout", type=int, default=120,
                        help="Per-probe timeout in seconds (default: 120)")
    parser.add_argument("--dry-run", action="store_true", help="Print planned routes without launching")
    parser.add_argument("--output", type=Path, help="Write the full JSON report to this path")
    args = parser.parse_args()
    if args.timeout < 1:
        parser.error("--timeout must be at least 1 second")
    app = args.app.expanduser().resolve()
    data_dir = args.data_dir.expanduser().resolve()
    games_arg = args.game or ["all"]
    games = tuple(MATRIX) if "all" in games_arg else tuple(dict.fromkeys(games_arg))
    platform_filters = {p.casefold() for p in (args.platform or [])}
    routes = [(game, platform, edition) for game in games
              for platform, edition in MATRIX[game]
              if not platform_filters or platform.casefold() in platform_filters]
    if not routes:
        parser.error("the selected game/platform filters match no supported CLI route")
    if args.dry_run:
        for game, platform, edition in routes:
            suffix = f" edition={edition}" if edition else ""
            print(f"{game}/{platform}{suffix}")
        return 0

    report: dict[str, Any] = {
        "schema": "firestaff.parity_debug_boot_sweep.v1",
        "generatedAtUtc": dt.datetime.now(dt.timezone.utc).isoformat(),
        "app": str(app), "dataRoot": str(data_dir),
        "scope": "real-data debug boot receipts only; not a parity or completion claim",
        "runs": [],
    }
    failed = False
    for game, platform, edition in routes:
        result = run_probe(app, data_dir, game, platform, edition, args.timeout)
        report["runs"].append(result)
        receipt = result["receipt"]
        print("{}/{}{} status={} phase={} dataMd5={} blocker={}".format(
            game, platform, f"/{edition}" if edition else "", result["status"],
            receipt.get("phase", "-"), receipt.get("assetMd5", "-"),
            result.get("startupBlocker") or "-"))
        failed |= result["status"] != "boot-probe-ready"

    if args.output:
        output = args.output.expanduser().resolve()
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(json.dumps(report, indent=2, ensure_ascii=False) + "\n",
                          encoding="utf-8")
        print(f"report={output}")
    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
