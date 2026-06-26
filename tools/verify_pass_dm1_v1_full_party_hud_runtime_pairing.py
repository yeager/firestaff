#!/usr/bin/env python3
"""Verify pass_dm1_v1_full_party_hud_runtime_pairing_gate.

Companion to pass1071 (DM1 V1 champion-panel pairing readiness).
This gate records the Firestaff-side runtime + pixel evidence for
the two terminal party sizes that the existing
firestaff_dm1_v1_champion_panel_partial_party_pixel_probe only
touches tangentially:

  - full four-champion party HUD
  - single-champion status panel

It also records the two-way transition between them, the
two-champion and three-champion intermediate cases, and a
full4 -> two2 -> full4 round-trip so the M11 draw stack is
provably clearing inactive slots and repopulating them on demand
when the party size oscillates inside the same DM1 V1 game session.

Honesty boundary:
- This does not compare Firestaff against original DM1 PC 3.4 pixels.
- This does not claim full four-champion HUD or single-champion status
  panel parity.
- The emitted status is PASS when the probe exercises all terminal
  cases + both transitions + both intermediate cases against real
  Firestaff V1 assets, and SKIP_NO_DATA_DIR when no real DM1 V1 data
  is staged.
- The recorded panel-region FNV-1a64 fingerprints are forensic
  evidence only; they let us detect cross-run / cross-asset drift
  without claiming original DOS parity.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS = "pass_dm1_v1_full_party_hud_runtime_pairing_gate"
PROBE = "firestaff_dm1_v1_full_party_hud_runtime_pairing_probe"
PROBE_SRC = ROOT / f"probes/m11/{PROBE}.c"
DATA_DIR = Path(os.environ.get(
    "FIRESTAFF_DM1_WORKSPACE_DATA_DIR",
    str(Path.home() / ".firestaff/data/dm1")))

VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

PASS1071_REPORT = ROOT / "parity-evidence" / "pass1071_dm1_v1_champion_panel_pairing_readiness.md"
PASS1073_VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS

NEEDLES_SRC = [
    "full four-champion party HUD",
    "single-champion status panel",
    "CHAMDRAW.C F0292",
    "CHAMDRAW.C F0293",
    "CHAMDRAW.C F0291",
    "no original DOS claim",
    "full4",
    "single1",
    "single1->full4",
    "full4->single1",
    "two2",
    "three3",
    "panel-region FNV-1a",
    "DEFS.H:2157",
]

NEEDLES_CMAKE = [
    PROBE,
]

# Expected label prefixes emitted by the probe (per-case FNV-1a64
# fingerprints). The verifier parses the live values out of the probe
# stdout and records them under fingerprint_panel_fnv1a64_<label>.
EXPECTED_FINGERPRINT_LABELS = [
    "full4_panel_fnv1a64",
    "single1_panel_fnv1a64",
    "two2_panel_fnv1a64",
    "three3_panel_fnv1a64",
    "single1_to_full4_panel_fnv1a64",
    "full4_to_single1_panel_fnv1a64",
    "two2_to_full4_panel_fnv1a64",
]


def resolve_build_dir(binary_name: str) -> Path:
    candidates = [
        ROOT / "build",
        ROOT / "builds" / "nv1-build",
        ROOT / "builds" / "n2-build",
    ]
    # Add worktree-builds sibling location. Convention is
    # coding-worktrees-builds/<worktree-name>_build.
    worktree_root = ROOT.parent
    build_sibling = worktree_root.parent / "coding-worktrees-builds"
    if build_sibling.exists():
        try:
            stem = ROOT.name
            candidates.append(build_sibling / f"{stem}_build")
        except (OSError, ValueError):
            pass
        # Fallback: any *build directory whose CMakeCache source
        # matches ROOT, prioritizing those that contain the binary.
        try:
            for child in sorted(build_sibling.iterdir()):
                if not child.is_dir():
                    continue
                cache = child / "CMakeCache.txt"
                if not cache.exists():
                    continue
                try:
                    txt = cache.read_text(encoding="utf-8", errors="replace")
                except OSError:
                    continue
                marker = f"CMAKE_HOME_DIRECTORY:INTERNAL={ROOT}"
                if marker in txt and (child / binary_name).exists():
                    candidates.insert(0, child)
        except OSError:
            pass
    for c in candidates:
        if (c / "CMakeCache.txt").exists() and (c / binary_name).exists():
            return c
    for c in candidates:
        if (c / "CMakeCache.txt").exists():
            return c
    return candidates[0]


def check_needles(label: str, path: Path, needles: list[str]) -> list[str]:
    if not path.exists():
        return [f"missing {label}: {path}"]
    text = path.read_text(encoding="utf-8", errors="replace")
    missing = [n for n in needles if n not in text]
    if missing:
        return [f"{label} missing needles: {missing}"]
    return []


FP_RE = re.compile(
    r"INFO\s+(?P<label>[A-Za-z0-9_>]+)=0x(?P<hex>[0-9A-Fa-f]{16})")


def parse_fingerprints(stdout: str) -> dict[str, str]:
    out: dict[str, str] = {}
    for m in FP_RE.finditer(stdout):
        out[m.group("label")] = m.group("hex").lower()
    return out


def run_probe(build_dir: Path, data_dir: Path) -> dict[str, Any]:
    binary = build_dir / PROBE
    if not binary.exists():
        return {
            "ran": False,
            "returncode": None,
            "stdout": "",
            "stderr": f"binary not found: {binary}",
            "pass_count": 0,
            "fail_count": 0,
            "fingerprints": {},
        }
    proc = subprocess.run(
        [str(binary), str(data_dir)],
        capture_output=True,
        text=True,
        timeout=180,
    )
    stdout = proc.stdout
    stderr = proc.stderr
    pass_count = stdout.count("PASS ")
    fail_count = stdout.count("FAIL ")
    return {
        "ran": True,
        "returncode": proc.returncode,
        "stdout": stdout,
        "stderr": stderr,
        "pass_count": pass_count,
        "fail_count": fail_count,
        "fingerprints": parse_fingerprints(stdout),
    }


def classify(probe_result: dict[str, Any],
             data_dir_exists: bool,
             pass1071_report_exists: bool) -> tuple[str, str]:
    if not data_dir_exists:
        return ("SKIP_NO_DATA_DIR",
                "DM1 V1 data directory is not staged; probe was not run.")
    if not probe_result["ran"]:
        return ("FAIL_PROBE_BINARY_MISSING",
                "Probe binary was not built; check the CMake build.")
    rc = probe_result["returncode"]
    if rc == 2:
        return ("SKIP_NO_ASSET",
                "Probe returned SKIP (no GRAPHICS.DAT); runtime pairing "
                "evidence requires a real DM1 V1 data directory.")
    missing_fps = [
        lbl for lbl in EXPECTED_FINGERPRINT_LABELS
        if lbl not in probe_result["fingerprints"]
    ]
    if missing_fps:
        return ("FAIL_RUNTIME_PAIRING_FINGERPRINTS_MISSING",
                f"Probe did not emit all expected panel-region "
                f"FNV-1a64 fingerprints: {missing_fps}")
    if rc == 0 and probe_result["fail_count"] == 0:
        detail = ("Probe exercised full4 HUD, single1 status panel, "
                  "two-/three-champion intermediate cases, and all "
                  "transitions against real Firestaff V1 assets.")
        if not pass1071_report_exists:
            detail += (" pass1071 companion report is missing; the "
                       "existence-of-original-artifacts gate is not "
                       "co-staged.")
        return ("PASS_RUNTIME_PAIRING_EVIDENCE", detail)
    return ("FAIL_RUNTIME_PAIRING_ASSERTIONS",
            f"Probe exit={rc} passes={probe_result['pass_count']} "
            f"fails={probe_result['fail_count']}; see stderr for details.")


def write_outputs(result: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n",
                        encoding="utf-8")
    fp_table_lines = [
        "| Case | Panel-region FNV-1a64 |",
        "|---|---:|",
    ]
    for lbl in EXPECTED_FINGERPRINT_LABELS:
        fp = result["probe"]["fingerprints"].get(lbl, "<missing>")
        fp_table_lines.append(f"| `{lbl}` | `0x{fp}` |")

    lines = [
        f"# {PASS}",
        "",
        f"Status: `{result['status']}`",
        "",
        f"Detail: {result['detail']}",
        "",
        "## Probe",
        "",
        f"- Source: `{PROBE_SRC.relative_to(ROOT)}`",
        f"- Binary: `{result['binary_path']}`",
        f"- Data dir: `{result['data_dir']}`",
        f"- Return code: `{result['probe']['returncode']}`",
        f"- Pass count: `{result['probe']['pass_count']}`",
        f"- Fail count: `{result['probe']['fail_count']}`",
        "",
        "## Companion Pass",
        "",
        f"- pass1071 companion report: "
        f"`{PASS1071_REPORT.relative_to(ROOT)}` "
        f"(exists: `{result['pass1071_report_exists']}`)",
        "",
        "## Panel-Region Fingerprints",
        "",
        "These 64-bit FNV-1a hashes cover the 274x29 party panel "
        "region (C151..C154 status boxes side by side). They are "
        "forensic evidence only; they let us detect cross-run / "
        "cross-asset drift without claiming original DOS parity.",
        "",
    ]
    lines.extend(fp_table_lines)
    lines += [
        "",
        "## Honesty Boundary",
        "",
        "- No Firestaff-vs-original DM1 PC 3.4 pixel diff is performed.",
        "- No full four-champion HUD or single-champion status-panel",
        "  original pairing exists; pass1071 still records that blocker.",
        "- This verifier only records that the Firestaff M11 V1 draw "
        "  stack populates the full 4-champion HUD, populates the "
        "  single-champion status panel, populates the 2-/3-champion "
        "  intermediate cases, and actively clears / repopulates the "
        "  inactive slots during the single<->full and full<->two<->full",
        "  transitions.",
        "",
        f"Manifest: `{MANIFEST.relative_to(ROOT)}`",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    failures: list[str] = []
    failures.extend(check_needles("probe_source", PROBE_SRC, NEEDLES_SRC))
    cmake_text = (ROOT / "CMakeLists.txt").read_text(
        encoding="utf-8", errors="replace")
    cmake_missing = [n for n in NEEDLES_CMAKE if n not in cmake_text]
    if cmake_missing:
        failures.append(f"cmake_registration missing needles: {cmake_missing}")

    pass1071_report_exists = PASS1071_REPORT.exists()

    data_dir_exists = DATA_DIR.exists()
    build_dir = resolve_build_dir(PROBE)
    probe_result = run_probe(build_dir, DATA_DIR)
    status, detail = classify(probe_result, data_dir_exists,
                              pass1071_report_exists)

    result: dict[str, Any] = {
        "schema":
            "firestaff.parity.pass_dm1_v1_full_party_hud_runtime_pairing_gate.v1",
        "pass": PASS,
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "detail": detail,
        "parity_claim": "none",
        "claim_parity": False,
        "data_dir": str(DATA_DIR),
        "data_dir_exists": data_dir_exists,
        "binary_path": str(build_dir / PROBE),
        "pass1071_report": str(PASS1071_REPORT.relative_to(ROOT)),
        "pass1071_report_exists": pass1071_report_exists,
        "probe": {
            "ran": probe_result["ran"],
            "returncode": probe_result["returncode"],
            "pass_count": probe_result["pass_count"],
            "fail_count": probe_result["fail_count"],
            "fingerprints": probe_result["fingerprints"],
            "stdout_tail": probe_result["stdout"][-2000:],
            "stderr_tail": probe_result["stderr"][-2000:],
        },
        "static_checks_ok": not failures,
        "static_check_failures": failures,
    }
    write_outputs(result)

    if failures:
        print(f"FAIL {PASS}: static checks drifted")
        for f in failures:
            print(f"  {f}")
        print(f"manifest: {MANIFEST.relative_to(ROOT)}")
        return 1
    if status == "PASS_RUNTIME_PAIRING_EVIDENCE":
        print(f"PASS {PASS}: status={status}")
        print(f"  passes={probe_result['pass_count']} "
              f"fails={probe_result['fail_count']}")
        print(f"manifest: {MANIFEST.relative_to(ROOT)}")
        return 0
    if status.startswith("SKIP"):
        print(f"SKIP {PASS}: status={status}")
        print(f"  detail={detail}")
        print(f"manifest: {MANIFEST.relative_to(ROOT)}")
        return 0
    print(f"FAIL {PASS}: status={status}")
    print(f"  detail={detail}")
    print(f"manifest: {MANIFEST.relative_to(ROOT)}")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
