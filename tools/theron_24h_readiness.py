#!/usr/bin/env python3
"""Theron 24h readiness roll-up.

This is a local orchestration gate for the Theron's Quest 24-hour readiness
push. It gathers the strongest bounded checks that are already wired into
CTest without claiming that Theron is finished:

- Theron JP/US Track 02 coverage is READY in coverage_by_game.py.
- The Tier 1 strict boot probe includes the Theron JP canonical, JP extras,
  and US extras launch milestones and skips paths whose local data is absent.
- Theron V1 source-locked slices cover availability, progression, mechanics,
  save/load, rendering, direct-launch, M11 handoff, launcher scan reuse, and
  the Track 02 bank boundary.
- Theron V2 rows are treated as compatibility-boundary and presentation-slice
  checks only; they do not turn the README Custom/V2 status into a finished
  runtime claim.

The script writes a manifest/report so the roll-up can be cited while keeping
the remaining blockers explicit: exact dungeon-bank offsets, full dungeon
loader parity, real Track 02 .srm import/export evidence, broader real-route
runtime traces, original pixel/capture parity, and README-eligible screenshots.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS = "theron_24h_readiness"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

CTEST_REQUIRED_TESTS = (
    "tier1_strict_boot_probe",
    "theron_v1_availability",
    "theron_v1_dungeon_progression",
    "theron_v1_dungeon_progression_determinism_probe",
    "theron_v1_mechanics_champions_probe",
    "theron_v1_mechanics_hardening",
    "theron_v1_cross_route_mechanics",
    "theron_v1_teleporter_chain",
    "theron_v1_viewport_renderer",
    "theron_v1_tile_renderer",
    "theron_v1_rendering",
    "theron_v1_save_load",
    "theron_v1_save_header_rejection",
    "theron_v1_shop_price_table",
    "theron_v1_world_serialize_purchase_state",
    "theron_v1_direct_launch",
    "theron_v1_m11_direct_launch",
    "theron_v1_launcher_scan_reuse",
    "theron_v1_track02_bank",
    "theron_v2_phase_gate_pc34",
    "theron_v2_phase0_v1_compatibility_lock",
    "theron_v2_phase1_launch_profile_separation",
    "theron_v2_settings_pc34",
    "theron_v2_filter_config_pc34",
    "theron_v2_presentation_mode_pc34",
    "theron_v2_texture_upscale_pc34",
    "theron_v22_shapes_pc34",
    "theron_v22_inplace_draw_pc34",
    "theron_v22_modern_assets_pc34",
    "theron_v2_smooth_movement_pc34",
)

V1_TESTS = frozenset(
    name for name in CTEST_REQUIRED_TESTS
    if name == "tier1_strict_boot_probe" or name.startswith("theron_v1_")
)
V2_TESTS = frozenset(name for name in CTEST_REQUIRED_TESTS if name.startswith("theron_v2_") or name.startswith("theron_v22_"))


def ctest_required_regex() -> str:
    return "^(" + "|".join(re.escape(name) for name in CTEST_REQUIRED_TESTS) + ")$"


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
    try:
        data = json.loads(proc["stdout"])
    except json.JSONDecodeError as exc:
        out["reason"] = f"failed to parse coverage JSON: {exc}"
        return out

    theron = data.get("summary", {}).get("theron", {})
    rows = [r for r in data.get("rows", []) if r.get("game") == "theron"]
    not_ready = [r for r in rows if r.get("status") != "READY"]
    out.update(
        {
            "ok": theron.get("total") == theron.get("ready") and not not_ready,
            "summary": theron,
            "rows": rows,
            "missing_or_not_ready": not_ready,
        }
    )
    return out


def _ctest_inventory(build_dir: Path, regex: str) -> dict[str, Any]:
    inventory_proc = run(
        [
            "ctest",
            "--test-dir",
            str(build_dir),
            "--show-only=json-v1",
        ],
        timeout=60,
    )
    inventory: dict[str, Any] = {
        "ok": False,
        "command": inventory_proc,
        "required": list(CTEST_REQUIRED_TESTS),
        "missing": list(CTEST_REQUIRED_TESTS),
        "matched": [],
        "groups": {
            "v1": sorted(V1_TESTS),
            "v2_boundary": sorted(V2_TESTS),
        },
        "label_summary": {},
    }
    if not inventory_proc["ok"]:
        return inventory
    try:
        data = json.loads(inventory_proc["stdout"])
    except json.JSONDecodeError as exc:
        inventory["reason"] = f"failed to parse ctest JSON inventory: {exc}"
        return inventory

    tests = data.get("tests", [])
    inventory_proc_summary = dict(inventory_proc)
    inventory_proc_summary["stdout"] = f"<ctest json inventory: {len(tests)} tests>"
    inventory["command"] = inventory_proc_summary

    available: dict[str, dict[str, Any]] = {}
    for test in tests:
        name = str(test.get("name", ""))
        properties = test.get("properties", [])
        labels: list[str] = []
        for prop in properties:
            if prop.get("name") != "LABELS":
                continue
            value = prop.get("value", [])
            if isinstance(value, str):
                labels = [value]
            elif isinstance(value, list):
                labels = [str(v) for v in value]
        if name:
            available[name] = {"labels": labels}

    missing = [name for name in CTEST_REQUIRED_TESTS if name not in available]
    matched = sorted(name for name in available if re.search(regex, name))
    inventory.update(
        {
            "ok": not missing,
            "missing": missing,
            "matched": matched,
            "label_summary": {
                name: available.get(name, {}).get("labels", [])
                for name in CTEST_REQUIRED_TESTS
            },
        }
    )
    return inventory


def ctest_check(build_dir: Path, *, list_only: bool) -> dict[str, Any]:
    if not build_dir.exists():
        return {
            "ok": False,
            "skipped": True,
            "reason": f"build dir missing: {build_dir}",
        }
    regex = ctest_required_regex()
    inventory = _ctest_inventory(build_dir, regex)
    if not inventory["ok"]:
        return {"ok": False, "regex": regex, "inventory": inventory}
    if list_only:
        return {
            "ok": True,
            "skipped": True,
            "reason": "--list-only",
            "regex": regex,
            "inventory": inventory,
        }

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
        timeout=360,
        env=env,
    )
    no_tests = "No tests were found" in proc["stdout"] or "Total Tests: 0" in proc["stdout"]
    result = {
        "ok": proc["ok"] and not no_tests,
        "regex": regex,
        "inventory": inventory,
        "command": proc,
    }
    if no_tests:
        result["reason"] = "ctest selected zero tests"
    return result


def _status(check: dict[str, Any]) -> str:
    if check.get("ok"):
        return "PASS"
    if check.get("skipped"):
        return "SKIP"
    return "FAIL"


def write_outputs(result: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    checks = result["checks"]
    lines = [
        "# Theron 24h readiness roll-up",
        "",
        f"Status: `{result['status']}`",
        "",
        "This is an orchestration gate for the active Theron's Quest readiness",
        "push. It keeps the current Track 02 coverage, boot, V1 slice, and V2",
        "compatibility-boundary checks visible in one place without claiming",
        "finished runtime playability or original pixel parity.",
        "",
        "## Checks",
        "",
        "| Check | Status | Notes |",
        "|---|---|---|",
    ]
    for name, check in checks.items():
        status = _status(check)
        note = check.get("reason", "")
        if name == "coverage" and check.get("summary"):
            theron = check["summary"]
            note = f"Theron ready {theron.get('ready')}/{theron.get('total')}"
            rows = check.get("rows") or []
            variants = ", ".join(
                f"{r.get('platform')} {r.get('variant')}" for r in rows
            )
            if variants:
                note += f" ({variants})"
        elif name == "ctest" and check.get("regex"):
            inventory = check.get("inventory", {})
            if inventory.get("missing"):
                note = "missing " + ", ".join(f"`{n}`" for n in inventory["missing"])
            else:
                v1_count = len([n for n in inventory.get("matched", []) if n in V1_TESTS])
                v2_count = len([n for n in inventory.get("matched", []) if n in V2_TESTS])
                note = (
                    f"{len(inventory.get('matched', []))} required tests "
                    f"({v1_count} V1/boot, {v2_count} V2-boundary); regex `{check['regex']}`"
                )
                if check.get("skipped"):
                    note = f"{check.get('reason')}; " + note
        lines.append(f"| `{name}` | `{status}` | {note} |")

    lines += [
        "",
        "## Non-claims",
        "",
        "- This is not exact Track 02 dungeon-bank offset parity.",
        "- This is not full Theron dungeon-loader parity.",
        "- This is not a real `.srm` / Track 02 save import/export artifact pass.",
        "- This is not a broader real-route gameplay trace or playability claim.",
        "- This is not original pixel/capture parity or a README-eligible screenshot gate.",
        "- This is not a claim that Theron Custom/V2 is finished beyond compatibility-boundary and presentation-slice coverage.",
        "- This is not a release gate; it is a local Theron 24h readiness roll-up.",
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
        help="CMake build directory for ctest roll-up",
    )
    parser.add_argument(
        "--skip-ctest",
        action="store_true",
        help="Only run coverage; do not invoke ctest",
    )
    parser.add_argument(
        "--list-only",
        action="store_true",
        help="Inventory required CTest rows but do not run them",
    )
    args = parser.parse_args()

    checks: dict[str, Any] = {"coverage": coverage_check(args.data_dir)}
    if args.skip_ctest:
        checks["ctest"] = {"ok": True, "skipped": True, "reason": "--skip-ctest"}
    else:
        checks["ctest"] = ctest_check(args.build_dir, list_only=args.list_only)

    ok = all(check.get("ok") for check in checks.values())
    result: dict[str, Any] = {
        "schema": "firestaff.parity.theron_24h_readiness.v1",
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
