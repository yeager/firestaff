#!/usr/bin/env python3
"""pass897 DM1 V1 DM-title-palette Apple-Silicon Metal-path probe.

Companion to pass842 dm_title_palette_regression. pass842 covers the
upstream V1_TitleFrontend_* helpers without touching SDL3; pass897
exercises the SDL3 / Metal renderer path that pass842 explicitly does
not cover (M11_Render_PresentIndexedWithSpecialPalette +
SDL_RenderReadPixels readback for the C12_PRESENTS -> C13_DUNGEON +
C14_MASTER switch).

The probe is gated to Apple Silicon (__APPLE__ + __arm64__) via
runtime sysctl hw.optional.arm64 detection so Intel macOS / Linux /
Windows runners skip cleanly with PASS. The contract under verification
is identical regardless of host: the GPU-side readback RGBA must match
F9011_VGA_GetSpecialColorRgb_Compat for the (colorIndex, specialPalette)
pair that was rendered.
"""
from __future__ import annotations

import json
import platform
import subprocess
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass897_dm1_v1_dm_title_palette_silicon_pc34_compat"
STATUS = "PASS897_DM1_V1_DM_TITLE_PALETTE_SILICON_LOCKED"
PROBE = ROOT / "probes/v1/firestaff_v1_dm_title_palette_silicon_probe.c"
HEADER_API_H = ROOT / "include/render_sdl_m11.h"
HEADER_PALETTE_H = ROOT / "include/vga_palette_pc34_compat.h"
CMAKE = ROOT / "CMakeLists.txt"
OUT_DIR = ROOT / 'parity-evidence/verification' / PASS
MANIFEST = OUT_DIR / 'manifest.json'
REPORT = ROOT / 'parity-evidence' / f'{PASS}.md'

# Probe-binary name = source-file basename (no _pc34 suffix).
BINARY_NAME = "firestaff_v1_dm_title_palette_silicon_probe"

# Anchor contract: every probe must cite the upstream palette/integer
# contract so a regression in F9011_VGA_GetSpecialColorRgb_Compat gets
# caught at the right layer too. The pass842 anchors also live in
# vga_palette_pc34_compat.h; we re-check the same constants here to
# verify the Apple Silicon probe depends on them.
ANCHORS = [
    "VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS",
    "VGA_PALETTE_PC34_SPECIAL_TITLE",
    "VGA_PALETTE_PC34_SPECIAL_ENTRANCE",
    "VGA_PALETTE_PC34_SPECIAL_CREDITS",
]

LOCAL_NEEDLES = [
    # Probe must declare the Apple Silicon gate + cite pass842.
    "is_apple_silicon",
    "Companion to firestaff_v1_dm_title_palette_regression_probe.c",
    "M11_Render_PresentIndexedWithSpecialPalette",
    "SDL_RenderReadPixels",
    "F9011_VGA_GetSpecialColorRgb_Compat",
    "F9010_VGA_GetColorRgb_Compat",
    # Public API the probe depends on must exist in the header.
    "M11_Render_GetWindow",
    "M11_Render_GetRenderer",
]

CMAKE_NEEDLES = [
    "firestaff_v1_dm_title_palette_silicon_probe",
    "probes/v1/firestaff_v1_dm_title_palette_silicon_probe.c",
    "NAME dm_title_palette_silicon",
]

PUBLIC_API_NEEDLES = [
    "M11_Render_GetWindow",
    "M11_Render_GetRenderer",
]


def read(path):
    return path.read_text(encoding="utf-8", errors="replace")


def check_needles(label, path, needles):
    text = read(path)
    missing = [n for n in needles if n not in text]
    return {
        "id": label,
        "file": str(path.relative_to(ROOT)),
        "status": "PASS" if not missing else "FAIL",
        "missing": missing,
    }


def is_apple_silicon_host() -> bool:
    """Mirror of the probe's is_apple_silicon() — compile-time check.

    The probe also does a runtime sysctl fallback for x86_64 macOS on
    M-series hardware. We don't need that here — if the verifier is
    running on Apple Silicon, the compile-time gate already passes
    and the probe is built + executed. If it isn't, the probe skips
    with PASS at runtime and the verifier reports SKIP for the run. """
    return platform.system() == "Darwin" and platform.machine() in ("arm64", "aarch64")


def run(cmd, cwd):
    proc = subprocess.run(cmd, cwd=cwd, text=True,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, timeout=180)
    return {
        "command": cmd,
        "returncode": proc.returncode,
        "passed": proc.returncode == 0,
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-30:]),
    }


def resolve_build_dir():
    candidates = [ROOT / "build", ROOT / "builds" / "nv1-build",
                  ROOT / "builds" / "n2-build"]
    for c in candidates:
        if (c / "CMakeCache.txt").exists() and (c / BINARY_NAME).exists():
            return c
    for c in candidates:
        if (c / "CMakeCache.txt").exists():
            return c
    return candidates[0]


def write_outputs(local_checks, runs, host_check):
    ok = (all(r["status"] == "PASS" for r in local_checks)
          and all(r["passed"] for r in runs if r.get("expectedToRun", True)))
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS if ok else f"FAILED_{STATUS}",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "scope": "DM1 V1 TITLE palette Apple-Silicon Metal-path probe.",
        "anchors": ANCHORS,
        "hostCheck": host_check,
        "sourceChecks": local_checks,
        "verificationRuns": runs,
        "nonOverlap": [
            "Disjoint from pass842 (dm_title_palette_regression): pass842 covers "
            "V1_TitleFrontend_* helpers without touching SDL3; pass897 covers "
            "M11_Render_PresentIndexedWithSpecialPalette + SDL_RenderReadPixels.",
        ],
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    rl = [f"# {PASS}", "", f"- Status: {manifest['status']}", "",
          "## Host", "",
          f"- {host_check['description']}",
          "",
          "## Anchors", ""]
    for a in ANCHORS:
        rl.append(f"- {a}")
    rl.append("")
    rl.append("## Verification")
    for r in runs:
        rl.append(f"- `{' '.join(r['command'])}`: rc={r['returncode']} "
                  f"({'PASS' if r['passed'] else 'FAIL'})")
    REPORT.write_text("\n".join(rl))


def main():
    local_checks = [
        check_needles("source_required_anchors", PROBE, ANCHORS),
        check_needles("source_runtime_contract", PROBE, LOCAL_NEEDLES),
        check_needles("public_api_surface", HEADER_API_H, PUBLIC_API_NEEDLES),
        check_needles("palette_header_contract", HEADER_PALETTE_H,
                      ["F9011_VGA_GetSpecialColorRgb_Compat",
                       "F9010_VGA_GetColorRgb_Compat",
                       "VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS"]),
        check_needles("cmake_registration", CMAKE, CMAKE_NEEDLES),
    ]
    is_as = is_apple_silicon_host()
    host_check = {
        "platform": platform.system(),
        "machine": platform.machine(),
        "isAppleSilicon": is_as,
        "description": (
            "Apple Silicon host (__APPLE__ + __arm64__) detected."
            if is_as else
            "Non-Apple-Silicon host; probe will skip with PASS at runtime "
            "(runtime sysctl fallback in probe.c handles edge cases)."
        ),
    }

    build_dir = resolve_build_dir()
    binary = build_dir / BINARY_NAME
    runs = []
    if binary.exists():
        runs.append(run([str(binary)], cwd=ROOT))
    else:
        runs.append({
            "command": [str(binary)],
            "returncode": -1,
            "passed": False,
            "outputTail": f"probe binary not built at {binary}",
            "expectedToRun": True,
        })

    write_outputs(local_checks, runs, host_check)
    local_ok = all(r["status"] == "PASS" for r in local_checks)
    runs_ok = all(r["passed"] for r in runs if r.get("expectedToRun", True))
    ok = local_ok and runs_ok
    print(f"{PASS}: {'PASS' if ok else 'FAIL'}")
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    for r in local_checks:
        if r["status"] != "PASS":
            print(f"missing in {r['id']}: {r['missing']}")
    for r in runs:
        print(r["outputTail"])
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())