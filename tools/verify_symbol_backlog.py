#!/usr/bin/env python3
"""Smoke-check the symbol backlog queue used by Firestaff subagents."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BACKLOG = ROOT / "tools" / "symbol_backlog.py"


def run(args: list[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        args,
        cwd=ROOT,
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )


def require_jsonl(args: list[str], expected_reference: str) -> None:
    result = run([sys.executable, str(BACKLOG), *args, "--format", "jsonl", "--limit", "5"])
    rows = [json.loads(line) for line in result.stdout.splitlines() if line.strip()]
    if not rows:
        raise AssertionError(f"empty JSONL queue for {' '.join(args)}")
    for row in rows:
        if row.get("reference") != expected_reference:
            raise AssertionError(f"unexpected reference in queue: {row}")
        if not row.get("symbol") or not row.get("status"):
            raise AssertionError(f"incomplete backlog row: {row}")


def main() -> int:
    run([sys.executable, "-m", "py_compile", str(BACKLOG)])
    summary = run([sys.executable, str(BACKLOG), "--limit", "0", "--json"])
    payload = json.loads(summary.stdout)
    if payload["summary"].get("totalOpen", 0) <= 0:
        raise AssertionError("symbol backlog unexpectedly empty")
    require_jsonl(["--reference", "ReDMCSB"], "ReDMCSB")
    require_jsonl(["--reference", "skproject", "--game", "DM2"], "skproject")
    print("symbol backlog verifier: ok")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
