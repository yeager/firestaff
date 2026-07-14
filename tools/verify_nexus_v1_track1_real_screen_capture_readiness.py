#!/usr/bin/env python3
"""Nexus V1 Track 1 real-screen-capture readiness gate.

Runs the real-data path of
``firestaff_nexus_v1_track1_real_screen_capture_readiness_probe``
against each verified Nexus data root and records the BMP receipts the
probe wrote. The gate proves the DM.BIN/FONT256.S2D/MNS runtime handoff
reaches a real 24-bit BMP on disk; it does NOT promote screenshots,
copy image bytes into ``verification-screens/``, or rewrite public docs.

Source-lock target: ``docs/FIRESTAFF_GAP_LIST.md`` E1 Track 1
phase-launch row "remaining work" sub-row (b):
  "capture an actual Nexus screen using the real Track 1 assets
   before promoting this beyond phase-launch gated."

The current E1 row is gated at "EXTRACTED + VERIFIED + PHASE-LAUNCH
GATED". This gate moves the readiness receipt from "synth-only" to
"real DM.BIN/FONT256.S2D/MNS handoff drives a deterministic BMP on
disk". The README promotion decision is intentionally NOT made by this
gate; a separate eligibility contract would have to audit real Track 1
loader parity before any image bytes leave the operator-local output
directory.

Exit codes:
  0  PASS — every present data case produced a valid 320x200 BMP with
             > 200 non-black pixels, the probe reported PASS, and the
             two consecutive BMP hashes match.
  1  FAIL — at least one present case failed.
  0  SKIP — no Nexus data directories present (still exit 0 so CI
             can run the gate on hosts without user-supplied data).
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
import tempfile
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


PASS = "nexus_v1_track1_real_screen_capture_readiness"
ROOT = Path(__file__).resolve().parents[1]
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

DEFAULT_CASES = (
    {
        "id": "extracted_root",
        "label": "Nexus extracted Track 1 root",
        "path": Path.home() / ".firestaff" / "data" / "nexus",
    },
    {
        "id": "saturn_ja_track1_bin",
        "label": "Nexus saturn-ja Track 1 .bin",
        "path": Path.home() / ".firestaff" / "data" / "nexus-extras" / "saturn-ja",
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


def run(cmd: list[str], *, env: dict[str, str],
        replacements: dict[str, str], timeout: int = 30) -> dict[str, Any]:
    proc = subprocess.run(
        cmd,
        cwd=ROOT,
        env=env,
        capture_output=True,
        text=True,
        timeout=timeout,
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
    """Return geometry + non-black-pixel count for a 24-bit BMP."""
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        return {"valid": False, "width": 0, "height": 0, "non_black_pixels": 0}
    pixel_offset = int.from_bytes(data[10:14], "little", signed=False)
    width = int.from_bytes(data[18:22], "little", signed=True)
    raw_height = int.from_bytes(data[22:26], "little", signed=True)
    bit_count = int.from_bytes(data[28:30], "little", signed=False)
    height = abs(raw_height)
    if width <= 0 or height <= 0 or bit_count != 24:
        return {"valid": False, "width": width, "height": height,
                "non_black_pixels": 0}
    row_bytes = width * 3
    padded = (row_bytes + 3) & ~3
    pixels = data[pixel_offset:]
    non_black = 0
    total_rows = min(height, len(pixels) // padded)
    for y in range(total_rows):
        row = pixels[y * padded:y * padded + row_bytes]
        for i in range(0, len(row), 3):
            r, g, b = row[i], row[i + 1], row[i + 2]
            if r > 8 or g > 8 or b > 8:
                non_black += 1
    return {
        "valid": total_rows == height,
        "width": width,
        "height": height,
        "bit_count": bit_count,
        "non_black_pixels": non_black,
    }


_PROBE_SHA_RE = re.compile(r"\[INFO\]\s+\S*-data\s+capture\s+sha256:\s*([0-9a-f]{64})")


def parse_probe_sha256(stdout: str, stderr: str) -> str | None:
    """Extract the real-data capture SHA256 the probe prints on success."""
    combined = f"{stdout}\n{stderr}"
    for line in combined.splitlines():
        m = _PROBE_SHA_RE.search(line)
        if m:
            return m.group(1)
    return None


def run_case(probe: Path, case: dict[str, Any]) -> dict[str, Any]:
    data_dir = Path(case["path"])
    row: dict[str, Any] = {
        "id": case["id"],
        "label": case["label"],
        "status": "SKIP",
        "present": data_dir.exists(),
        "probe": None,
        "screenshots": [],
        "ok": False,
        "reason": None,
    }
    if not data_dir.exists():
        row["reason"] = "data directory missing"
        return row

    with tempfile.TemporaryDirectory(prefix=f"firestaff-{PASS}-{case['id']}-") as td:
        temp_root = Path(td)
        bmp_dir = temp_root / "bmp"
        replacements = {
            str(temp_root): "<temp>",
            str(data_dir): f"<{case['id']}-data-dir>",
            str(probe): "<probe-binary>",
        }
        env = os.environ.copy()
        env.setdefault("SDL_VIDEODRIVER", "dummy")

        # Run the probe twice in the same case so we can verify that
        # the produced BMPs are SHA256-deterministic across runs (the
        # probe prints the SHA256 it computed for both runs in its
        # stdout, so we don't need to re-hash the file from Python).
        proc = run([str(probe), str(data_dir), str(bmp_dir)],
                   env=env, replacements=replacements, timeout=30)
        row["probe_first_run"] = proc

        bmp_paths = sorted(bmp_dir.glob("firestaff-*.bmp"))
        for path in bmp_paths:
            try:
                info = bmp_info(path)
            except Exception as exc:  # pragma: no cover - defensive
                info = {"valid": False, "error": str(exc)}
            info.update({
                "name": path.name,
                "size": path.stat().st_size,
                "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
            })
            row["screenshots"].append(info)

        # Second run: only meaningful when at least one screenshot
        # appeared in the first run. We use a separate output dir so
        # the second-run BMPs get their own SHA256 sidecar.
        second_bmp_dir = temp_root / "bmp_second"
        proc_second = run([str(probe), str(data_dir), str(second_bmp_dir)],
                          env=env, replacements=replacements, timeout=30)
        row["probe_second_run"] = proc_second
        second_paths = sorted(second_bmp_dir.glob("firestaff-*.bmp"))
        second_rows = []
        for path in second_paths:
            try:
                info = bmp_info(path)
            except Exception as exc:  # pragma: no cover
                info = {"valid": False, "error": str(exc)}
            info.update({
                "name": path.name,
                "size": path.stat().st_size,
                "sha256": hashlib.sha256(path.read_bytes()).hexdigest(),
            })
            second_rows.append(info)
        row["screenshots_second_run"] = second_rows

        # The probe stamps files with the local wall-clock time so the
        # filenames differ between runs; we therefore compare
        # per-position SHA256 pairs (first vs first, second vs second).
        pairs = []
        for a, b in zip(row["screenshots"], row["screenshots_second_run"]):
            pairs.append({"first": a["sha256"], "second": b["sha256"]})
        row["sha_pairs"] = pairs
        row["sha_deterministic"] = (
            bool(pairs) and all(p["first"] == p["second"] for p in pairs)
        )

    # Pass/fail decision: probe returned 0, at least one valid 320x200
    # BMP with > 200 non-black pixels was produced, and the two
    # consecutive runs produced matching SHA256 pairs.
    valid_shots = [s for s in row["screenshots"]
                   if s.get("valid") and s.get("width") == 320
                   and s.get("height") == 200
                   and s.get("non_black_pixels", 0) > 200]
    probe_ok = row["probe_first_run"].get("ok") is True
    row["ok"] = (
        probe_ok
        and len(valid_shots) >= 1
        and row["sha_deterministic"]
        and parse_probe_sha256(row["probe_first_run"]["stdout"],
                               row["probe_first_run"]["stderr"]) is not None
    )
    row["status"] = "PASS" if row["ok"] else "FAIL"
    return row


def write_outputs(result: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n",
                        encoding="utf-8")
    rows = result.get("cases", [])
    lines = [
        "# Nexus V1 Track 1 real-screen-capture readiness",
        "",
        f"Status: `{result['status']}`",
        "",
        "This gate proves the DM.BIN/FONT256.S2D/MNS runtime handoff",
        "reaches a real 24-bit BMP on disk through",
        "`nexus_viewport_render` + `nexus_viewport_to_rgba` +",
        "`M11_Screenshot_CaptureRGBA` for every verified Nexus data root",
        "that is present on the host. It is a *readiness* receipt for",
        "the E1 Track 1 phase-launch row; it does not promote screenshots",
        "and it does not rewrite public docs.",
        "",
        "## Case Results",
        "",
        "| Case | Status | Probe | Valid BMPs | Non-black (first BMP) | SHA-deterministic |",
        "|---|---:|---:|---:|---:|---:|",
    ]
    for row in rows:
        first = (row.get("screenshots") or [{}])[0]
        lines.append(
            f"| {row['label']} | {row['status']} | "
            f"{'yes' if row.get('probe_first_run', {}).get('ok') else 'no'} | "
            f"`{len([s for s in row.get('screenshots', []) if s.get('valid') and s.get('width') == 320 and s.get('height') == 200 and s.get('non_black_pixels', 0) > 200])}` | "
            f"`{first.get('non_black_pixels', 0)}` | "
            f"{'yes' if row.get('sha_deterministic') else 'no'} |"
        )
    lines += [
        "",
        "## Public Screenshot Boundary",
        "",
        "- These receipts prove Firestaff can emit Nexus viewport BMP artifacts when DM.BIN/FONT256.S2D/MNS reach the V1 viewport render path.",
        "- BMPs are written to a per-case temporary directory and discarded when the gate exits; no image bytes are promoted.",
        "- No generated, mock, or synthetic image is promoted by this gate.",
        "- The probe's data-free path (no `--data-dir` supplied) still proves the synthetic viewport → RGBA → BMP contract, but is *not* a Track 1 capture.",
        "- README-eligible Nexus screenshots still need a separate eligibility gate that audits real Track 1 loader parity (DGN 3D geometry decode, text/glyph runtime binding, MNS model dispatch) before any image bytes leave the operator-local output directory.",
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
        help="Override/add a Nexus data case. May be supplied more than once.",
    )
    args = parser.parse_args()
    probe = args.build_dir / "firestaff_nexus_v1_track1_real_screen_capture_readiness_probe"
    cases = list(DEFAULT_CASES)
    for item in args.case:
        if "=" not in item:
            print(f"invalid --case value, expected ID=PATH: {item}",
                  file=sys.stderr)
            return 2
        cid, cpath = item.split("=", 1)
        cases.append({"id": cid, "label": cid.replace("_", " "),
                      "path": Path(cpath)})

    result: dict[str, Any] = {
        "schema": "firestaff.parity.nexus_v1_track1_real_screen_capture_readiness.v1",
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "status": "FAIL",
        "probe": rel_or_str(probe),
        "cases": [],
        "non_claims": [
            "No Nexus screenshot image is promoted by this gate.",
            "No generated or mock image is used.",
            "No claim of full Nexus V1 runtime playability, real Track 1 loader parity, or README-eligible screenshot.",
            "The probe's synthetic path proves the viewport → RGBA → BMP contract only; it is not a Track 1 capture.",
        ],
    }
    if not probe.exists():
        result["reason"] = f"missing probe binary: {rel_or_str(probe)}"
        write_outputs(result)
        print(f"FAIL {PASS}: missing probe binary", file=sys.stderr)
        return 1

    result["cases"] = [run_case(probe, case) for case in cases]
    present_rows = [row for row in result["cases"] if row.get("present")]
    if not present_rows:
        result["status"] = "SKIP"
        result["reason"] = "no Nexus data directories present"
        write_outputs(result)
        print(f"SKIP {PASS}: no Nexus data directories present")
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
