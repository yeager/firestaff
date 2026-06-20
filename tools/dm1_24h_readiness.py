#!/usr/bin/env python3
"""DM1 24h readiness roll-up.

This is a local orchestration gate for the active DM1 finish lane. It gathers
the narrow, already source-locked checks that matter most for the current
24-hour goal:

- DM1 data coverage is complete in coverage_by_game.py.
- The hash registry agrees with local data when local data exists.
- pass1056 keeps the pass1052/pass1054 viewport/wall pairing reproducible.
- pass1057 keeps the Amiga 2.2 English DUNGEONB.DAT sidecar locked.
- Optional build-dir probes keep the DM1 playable route, pass1055 collision
  semantic pair, pass1056/pass1057 CTests, and Phase A probe green.

The script writes a manifest/report so the roll-up can be cited without
claiming that the remaining original-capture gaps are solved.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS = "dm1_24h_readiness"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"


def run(cmd: list[str], *, timeout: int = 120, env: dict[str, str] | None = None) -> dict[str, Any]:
    proc = subprocess.run(
        cmd,
        cwd=ROOT,
        env=env,
        capture_output=True,
        text=True,
        timeout=timeout,
    )
    return {
        "cmd": cmd,
        "returncode": proc.returncode,
        "stdout": proc.stdout.strip(),
        "stderr": proc.stderr.strip(),
        "ok": proc.returncode == 0,
    }


def coverage_check(data_dir: Path) -> dict[str, Any]:
    proc = run(
        [
            sys.executable,
            str(ROOT / "tools/asset-validate/coverage_by_game.py"),
            "--data-dir",
            str(data_dir),
            "--json",
        ],
        timeout=60,
    )
    out: dict[str, Any] = {"ok": False, "command": proc}
    if not proc["ok"]:
        return out
    data = json.loads(proc["stdout"])
    dm1 = data.get("summary", {}).get("dm1", {})
    rows = [
        r
        for r in data.get("rows", [])
        if r.get("game") == "dm1"
    ]
    missing = [
        r
        for r in rows
        if r.get("status") != "READY"
    ]
    out.update(
        {
            "ok": dm1.get("total") == dm1.get("ready") and not missing,
            "summary": dm1,
            "missing_or_not_ready": missing,
        }
    )
    return out


def registry_check(data_dir: Path) -> dict[str, Any]:
    if not data_dir.exists():
        return {
            "ok": False,
            "skipped": True,
            "reason": f"data dir missing: {data_dir}",
        }
    proc = run(
        [
            sys.executable,
            str(ROOT / "tools/asset-validate/compare_to_greatstone.py"),
            str(data_dir),
            "--quiet",
        ],
        timeout=180,
    )
    return {"ok": proc["ok"], "command": proc}


def script_check(script: str) -> dict[str, Any]:
    return run([sys.executable, str(ROOT / script)], timeout=120)


def preserved_script_check(script: str, preserve: list[Path]) -> dict[str, Any]:
    before: dict[Path, bytes | None] = {}
    for path in preserve:
        before[path] = path.read_bytes() if path.exists() else None
    result = script_check(script)
    for path, data in before.items():
        if data is None:
            if path.exists():
                path.unlink()
        else:
            path.write_bytes(data)
    result["preserved_outputs"] = [str(p.relative_to(ROOT)) for p in preserve]
    return result


def ctest_check(build_dir: Path) -> dict[str, Any]:
    if not build_dir.exists():
        return {
            "ok": False,
            "skipped": True,
            "reason": f"build dir missing: {build_dir}",
        }
    regex = (
        "m11_phase_a|"
        "firestaff_dm1_v1_playable_route_probe|"
        "firestaff_dm1_v1_pass1055_closed_door_pair_probe"
    )
    env = os.environ.copy()
    env.setdefault("SDL_VIDEODRIVER", "dummy")
    proc = run(
        [
            "ctest",
            "--test-dir",
            str(build_dir),
            "-R",
            regex,
            "--output-on-failure",
        ],
        timeout=240,
        env=env,
    )
    return {"ok": proc["ok"], "regex": regex, "command": proc}


def write_outputs(result: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    checks = result["checks"]
    lines = [
        "# DM1 24h readiness roll-up",
        "",
        f"Status: `{result['status']}`",
        "",
        "This is an orchestration gate for the active DM1 finish lane. It does",
        "not claim the remaining original-capture gaps are solved; it keeps the",
        "current data, viewport/wall, collision, and smoke gates visible in one",
        "place while those gaps are being closed.",
        "",
        "## Checks",
        "",
        "| Check | Status | Notes |",
        "|---|---|---|",
    ]
    for name, check in checks.items():
        status = "PASS" if check.get("ok") else ("SKIP" if check.get("skipped") else "FAIL")
        note = check.get("reason", "")
        if name == "coverage" and check.get("summary"):
            dm1 = check["summary"]
            note = f"DM1 ready {dm1.get('ready')}/{dm1.get('total')}"
        elif name == "ctest" and check.get("regex"):
            note = f"regex `{check['regex']}`"
        lines.append(f"| `{name}` | `{status}` | {note} |")

    lines += [
        "",
        "## Non-claims",
        "",
        "- This is not a same-state original-to-Firestaff viewport promotion.",
        "- This is not a creature-chain original screenshot.",
        "- This is not a four-champion original HUD capture.",
        "- This is not a release gate; it is a local DM1 finish-lane roll-up.",
        "",
        f"Manifest: `{MANIFEST.relative_to(ROOT)}`",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--data-dir",
        type=Path,
        default=Path(os.environ.get("FIRESTAFF_DATA", Path.home() / ".firestaff" / "data")),
        help="Firestaff data root, default: ~/.firestaff/data or FIRESTAFF_DATA",
    )
    parser.add_argument(
        "--build-dir",
        type=Path,
        default=ROOT / "build",
        help="CMake build directory for optional ctest roll-up",
    )
    parser.add_argument(
        "--skip-ctest",
        action="store_true",
        help="Only run Python/data gates; do not invoke ctest",
    )
    args = parser.parse_args()

    checks: dict[str, Any] = {
        "coverage": coverage_check(args.data_dir),
        "registry": registry_check(args.data_dir),
        "pass1056_pairing": preserved_script_check(
            "tools/verify_pass1056_dm1_v1_pass1052_firestaff_pairing_gate.py",
            [
                ROOT / "parity-evidence/verification/pass1056_dm1_v1_pass1052_firestaff_pairing_gate/manifest.json",
                ROOT / "parity-evidence/pass1056_dm1_v1_pass1052_firestaff_pairing_gate.md",
            ],
        ),
        "pass1057_dungeonb": preserved_script_check(
            "tools/verify_pass1057_dm1_amiga22_dungeonb_asset_lock.py",
            [
                ROOT / "parity-evidence/verification/pass1057_dm1_amiga22_dungeonb_asset_lock/manifest.json",
                ROOT / "parity-evidence/pass1057_dm1_amiga22_dungeonb_asset_lock.md",
            ],
        ),
    }
    if args.skip_ctest:
        checks["ctest"] = {"ok": True, "skipped": True, "reason": "--skip-ctest"}
    else:
        checks["ctest"] = ctest_check(args.build_dir)

    ok = all(check.get("ok") for check in checks.values())
    result: dict[str, Any] = {
        "schema": "firestaff.parity.dm1_24h_readiness.v1",
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "status": "PASS" if ok else "FAIL",
        "data_dir": str(args.data_dir),
        "build_dir": str(args.build_dir),
        "checks": checks,
    }
    write_outputs(result)
    if ok:
        print(f"PASS {PASS}")
        return 0
    print(f"FAIL {PASS}", file=sys.stderr)
    print(json.dumps(result, indent=2)[:6000], file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
