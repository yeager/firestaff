#!/usr/bin/env python3
"""Verify pass350 DM1 V1 touch live dispatch gate artifacts."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MANIFEST = ROOT / "parity-evidence/verification/pass350_dm1_v1_touch_live_dispatch_gate/manifest.json"
EVIDENCE = ROOT / "parity-evidence/pass350_dm1_v1_touch_live_dispatch_gate.md"
PROBE = ROOT / "probes/m11/firestaff_m11_touch_live_dispatch_gate_probe.c"
CMAKE = ROOT / "CMakeLists.txt"
SOURCE_ROOT = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
EXPECTED_STATUS = "PASS_DM1_V1_TOUCH_LIVE_DISPATCH_GATE"


def fail(message: str) -> int:
    print(f"status=FAIL_PASS350_TOUCH_LIVE_DISPATCH_GATE reason={message}")
    return 1


def text(path: Path) -> str:
    try:
        return path.read_text()
    except OSError as exc:
        raise AssertionError(f"cannot read {path}: {exc}") from exc


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def run_probe() -> str:
    exe = ROOT / "build/firestaff_m11_touch_live_dispatch_gate_probe"
    if not exe.exists():
        subprocess.run(
            ["cmake", "--build", str(ROOT / "build"), "--target", "firestaff_m11_touch_live_dispatch_gate_probe", "-j2"],
            cwd=ROOT,
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
    return subprocess.run(
        [str(exe)],
        cwd=ROOT,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    ).stdout


def main() -> int:
    try:
        manifest = json.loads(text(MANIFEST))
        evidence = text(EVIDENCE)
        probe = text(PROBE)
        cmake = text(CMAKE)

        require(manifest.get("status") == EXPECTED_STATUS, "manifest status mismatch")
        require(manifest.get("ctest_name") == "pass350_dm1_v1_touch_live_dispatch_gate", "ctest name missing")
        require("pass350_dm1_v1_touch_live_dispatch_gate" in cmake, "CTest target missing from CMake")
        require("firestaff_m11_touch_live_dispatch_gate_probe" in cmake, "probe target missing from CMake")

        anchors = manifest.get("source_anchors", [])
        require(len(anchors) >= 8, "source anchors incomplete")
        for anchor in anchors:
            source_file = SOURCE_ROOT / anchor["file"]
            require(source_file.exists(), f"missing ReDMCSB source {source_file}")
            require(f"`{anchor['file']}:{anchor['lines']}`" in evidence or f"{anchor['file']}:{anchor['lines']}" in evidence,
                    f"evidence missing anchor {anchor['file']}:{anchor['lines']}")

        required_probe_tokens = [
            "TOUCHPOINTER_Compat_TranslateEvent",
            "TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue",
            "DM1_V1_InputCommandQueue_ProcessOnePc34Compat",
            "primary_left_beats_status_box_child",
            "secondary_left_movement_fallback",
            "secondary_right_screen_toggle",
            "scaled_primary_right_preserves_original_screen",
            "pending_replay_original_click_state",
            "TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT",
            "TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT",
        ]
        for token in required_probe_tokens:
            require(token in probe, f"probe missing token {token}")

        live_cases = manifest.get("live_cases", [])
        require(len(live_cases) == 6, "live case count mismatch")
        for case in live_cases:
            require(case["name"] in probe, f"probe missing live case {case['name']}")

        output = run_probe()
        for marker in manifest.get("required_probe_output_markers", []):
            require(marker in output, f"probe output missing {marker}")
        require("button=0x0001" in output and "button=0x0002" in output, "probe output missing original button masks")
        require("screen=25,11" in output and "screen=264,126" in output and "screen=25,9" in output,
                "probe output missing original/normalized screen coordinates")

        print(f"status={EXPECTED_STATUS}")
        print("sourceAnchors=%u liveCases=%u" % (len(anchors), len(live_cases)))
        print("probeMarkersOk=1")
        return 0
    except (AssertionError, subprocess.CalledProcessError, json.JSONDecodeError) as exc:
        return fail(str(exc))


if __name__ == "__main__":
    sys.exit(main())
