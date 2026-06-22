#!/usr/bin/env python3
"""Theron V1 runtime screenshot readiness gate.

Runs real Firestaff Theron launches when hash-verified Track 02 data is
available and records screenshot/probe receipts as metadata only. This is a
readiness gate for future README screenshots; it does not promote any image
bytes into public screenshot directories.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import subprocess
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS = "theron_v1_runtime_screenshot_readiness"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

DEFAULT_CASES = (
    {
        "id": "canonical_pcengine_root",
        "label": "Theron canonical PC Engine root",
        "path": Path.home() / ".firestaff" / "data" / "theron",
    },
    {
        "id": "jp_extras_track02_bin",
        "label": "Theron JP extras Track 02",
        "path": Path.home() / ".firestaff" / "data" / "theron-extras" / "japan",
    },
    {
        "id": "us_extras_track02_bin",
        "label": "Theron US extras Track 02",
        "path": Path.home() / ".firestaff" / "data" / "theron-extras" / "usa",
    },
)


def rel_or_str(path: Path) -> str:
    try:
        return str(path.relative_to(ROOT))
    except ValueError:
        return str(path)


def sanitize_text(text: str, replacements: dict[str, str]) -> str:
    out = text
    for raw, token in replacements.items():
        if raw:
            out = out.replace(raw, token)
    return out


def run(cmd: list[str], *, env: dict[str, str], replacements: dict[str, str]) -> dict[str, Any]:
    proc = subprocess.run(
        cmd,
        cwd=ROOT,
        env=env,
        capture_output=True,
        text=True,
        timeout=20,
    )
    redacted_cmd = [sanitize_text(part, replacements) for part in cmd]
    return {
        "cmd": redacted_cmd,
        "returncode": proc.returncode,
        "stdout": sanitize_text(proc.stdout.strip(), replacements),
        "stderr": sanitize_text(proc.stderr.strip(), replacements),
        "ok": proc.returncode == 0,
    }


def bmp_info(path: Path) -> dict[str, Any]:
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        return {"valid": False, "width": 0, "height": 0, "non_black_pixels": 0}
    pixel_offset = int.from_bytes(data[10:14], "little", signed=False)
    width = int.from_bytes(data[18:22], "little", signed=True)
    raw_height = int.from_bytes(data[22:26], "little", signed=True)
    bit_count = int.from_bytes(data[28:30], "little", signed=False)
    height = abs(raw_height)
    if width <= 0 or height <= 0 or bit_count != 24:
        return {"valid": False, "width": width, "height": height, "non_black_pixels": 0}
    row_bytes = width * 3
    padded = (row_bytes + 3) & ~3
    pixels = data[pixel_offset:]
    non_black = 0
    total_rows = min(height, len(pixels) // padded)
    for y in range(total_rows):
        row = pixels[y * padded:y * padded + row_bytes]
        for i in range(0, len(row), 3):
            if row[i:i + 3] != b"\x00\x00\x00":
                non_black += 1
    return {
        "valid": total_rows == height,
        "width": width,
        "height": height,
        "bit_count": bit_count,
        "non_black_pixels": non_black,
    }


def screenshot_rows(directory: Path) -> list[dict[str, Any]]:
    if not directory.exists():
        return []
    rows: list[dict[str, Any]] = []
    for path in sorted(directory.glob("*.bmp")):
        rows.append(
            {
                "name": path.name,
                "size": path.stat().st_size,
                "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
                **bmp_info(path),
            }
        )
    return rows


def run_case(firestaff: Path, case: dict[str, Any]) -> dict[str, Any]:
    data_dir = Path(case["path"])
    row: dict[str, Any] = {
        "id": case["id"],
        "label": case["label"],
        "status": "SKIP",
        "present": data_dir.exists(),
        "probe": None,
        "screenshots": [],
        "presented_screenshots": [],
        "marker_found": False,
        "ok": False,
    }
    if not data_dir.exists():
        row["reason"] = "data directory missing"
        return row

    with tempfile.TemporaryDirectory(prefix=f"firestaff-{PASS}-{case['id']}-") as td:
        temp_root = Path(td)
        temp_home = temp_root / "home"
        temp_home.mkdir(parents=True, exist_ok=True)
        probe_path = temp_root / "theron_runtime_probe.json"
        screenshot_dir = temp_root / "source_bmp"
        presented_dir = temp_root / "presented_bmp"
        replacements = {
            str(temp_root): "<temp>",
            str(data_dir): f"<{case['id']}-data-dir>",
            str(firestaff): "<build-firestaff>",
        }
        env = os.environ.copy()
        env["HOME"] = str(temp_home)
        env.setdefault("SDL_VIDEODRIVER", "dummy")
        env["FIRESTAFF_FAIL_IF_NO_LAUNCH"] = "1"
        env["FIRESTAFF_AUTOTEST_RUNTIME_PROBE_JSON"] = str(probe_path)
        env["FIRESTAFF_AUTOTEST_SCREENSHOT_DIR"] = str(screenshot_dir)
        env["FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR"] = str(presented_dir)
        cmd = [
            str(firestaff),
            "--game",
            "theron",
            "--data-dir",
            str(data_dir),
            "--duration",
            "1500",
        ]
        proc = run(cmd, env=env, replacements=replacements)
        row["command"] = proc
        combined = f"{proc['stdout']}\n{proc['stderr']}"
        row["marker_found"] = "TQR level load" in combined
        row["fallback_assets_used"] = "deterministic fallback assets" in combined
        if probe_path.exists():
            row["probe"] = json.loads(probe_path.read_text(encoding="utf-8"))
        row["screenshots"] = screenshot_rows(screenshot_dir)
        row["presented_screenshots"] = screenshot_rows(presented_dir)

    probe = row.get("probe") or {}
    source_shots = row.get("screenshots") or []
    presented_shots = row.get("presented_screenshots") or []
    source_ok = (
        len(source_shots) == 1
        and source_shots[0].get("valid")
        and source_shots[0].get("width") == 320
        and source_shots[0].get("height") == 200
        and source_shots[0].get("non_black_pixels", 0) > 200
    )
    presented_ok = (
        len(presented_shots) == 1
        and presented_shots[0].get("valid")
        and presented_shots[0].get("width", 0) > 0
        and presented_shots[0].get("height", 0) > 0
        and presented_shots[0].get("non_black_pixels", 0) > 200
    )
    row["ok"] = (
        bool(row.get("command", {}).get("ok"))
        and row["marker_found"]
        and probe.get("schema") == "firestaff_m11_autotest_runtime_probe.v1"
        and probe.get("launchedEver") == 1
        and probe.get("active") == 1
        and probe.get("sourceId") == "theron"
        and source_ok
        and presented_ok
    )
    row["status"] = "PASS" if row["ok"] else "FAIL"
    return row


def write_outputs(result: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    rows = result.get("cases", [])
    lines = [
        "# Theron V1 runtime screenshot readiness",
        "",
        f"Status: `{result['status']}`",
        "",
        "This gate runs real Firestaff Theron launches when hash-verified Track 02",
        "data is present. It records runtime probe fields plus BMP geometry and",
        "hash receipts only; it does not add screenshots to public docs.",
        "",
        "## Case Results",
        "",
        "| Case | Status | Boot marker | Runtime source | Fallback assets | Source BMP | Presented BMP |",
        "|---|---:|---:|---|---:|---:|---:|",
    ]
    for row in rows:
        probe = row.get("probe") or {}
        lines.append(
            f"| {row['label']} | {row['status']} | "
            f"{'yes' if row.get('marker_found') else 'no'} | "
            f"`{probe.get('sourceId', '')}` | "
            f"{'yes' if row.get('fallback_assets_used') else 'no'} | "
            f"`{len(row.get('screenshots', []))}` | "
            f"`{len(row.get('presented_screenshots', []))}` |"
        )
    lines += [
        "",
        "## Public Screenshot Boundary",
        "",
        "- These receipts prove Firestaff can emit Theron runtime screenshot artifacts when the Track 02 launch reaches M11.",
        "- Rows that use deterministic fallback assets are runtime/capture-path proof, not final art or source-bank proof.",
        "- No generated, mock, or synthetic image is promoted by this gate.",
        "- README-eligible Theron screenshots still need reviewed real runtime frames and stronger semantic Track 02 loader parity.",
        "",
        f"Manifest: `{rel_or_str(MANIFEST)}`",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build")
    parser.add_argument(
        "--case",
        action="append",
        default=[],
        metavar="ID=PATH",
        help="Override/add a Theron data case. May be supplied more than once.",
    )
    args = parser.parse_args()
    firestaff = args.build_dir / "firestaff"
    cases = list(DEFAULT_CASES)
    for item in args.case:
        if "=" not in item:
            print(f"invalid --case value, expected ID=PATH: {item}", file=sys.stderr)
            return 2
        cid, cpath = item.split("=", 1)
        cases.append({"id": cid, "label": cid.replace("_", " "), "path": Path(cpath)})

    result: dict[str, Any] = {
        "schema": "firestaff.parity.theron_v1_runtime_screenshot_readiness.v1",
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "status": "FAIL",
        "firestaff": rel_or_str(firestaff),
        "cases": [],
        "non_claims": [
            "No public Theron screenshot image is promoted.",
            "No generated or mock image is used.",
            "No claim of full Theron runtime playability or semantic Track 02 loader parity.",
        ],
    }
    if not firestaff.exists():
        result["reason"] = f"missing firestaff binary: {rel_or_str(firestaff)}"
        write_outputs(result)
        print(f"FAIL {PASS}: missing firestaff binary", file=sys.stderr)
        return 1

    result["cases"] = [run_case(firestaff, case) for case in cases]
    present_rows = [row for row in result["cases"] if row.get("present")]
    if not present_rows:
        result["status"] = "SKIP"
        result["reason"] = "no Theron data directories present"
        write_outputs(result)
        print(f"SKIP {PASS}: no Theron data directories present")
        return 0
    ok = all(row.get("ok") for row in present_rows)
    result["status"] = "PASS" if ok else "FAIL"
    write_outputs(result)
    if ok:
        print(f"PASS {PASS} cases={len(present_rows)}")
        return 0
    print(f"FAIL {PASS}", file=sys.stderr)
    print(json.dumps(result, indent=2)[:6000], file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
