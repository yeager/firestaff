#!/usr/bin/env python3
"""Pass348 verifier: classify pass333 NumLock/keypad blocker after pass346."""
from __future__ import annotations

import json
import os
import shutil
import subprocess
from datetime import datetime, timezone
from pathlib import Path

PASS = "pass348_dm1_v1_numlock_keypad_blocker_closure"
ROOT = Path(__file__).resolve().parents[1]
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
DEFAULT_REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DEFAULT_BUILD = Path.home() / ".openclaw/data/firestaff-builds/pass348-verify"

SOURCE_LOCKS = [
    ("INPUT.C", 298, 430, ["F0543_INPUT_DeviceInterruptHandler", "IECLASS_RAWKEY", "G1044_B_MouseOrKeyboardInput"]),
    ("IO2.C", 5, 61, ["F0540_INPUT_Crawcin", "IODRV_00_GetKeyboardInput", "L2944_ui_ = 'L'", "L2944_ui_ = 'P'", "L2944_ui_ = 'K'", "L2944_ui_ = 'M'"]),
    ("COMMAND.C", 636, 685, ["G0459_as_Graphic561_SecondaryKeyboardInput_Movement", "MEDIA707_I34E_I34M", "0x004B", "0x004C", "0x004D", "0x004F", "0x0050", "0x0051"]),
    ("COMMAND.C", 1716, 1808, ["G0443_ps_PrimaryKeyboardInput", "G0444_ps_SecondaryKeyboardInput", "G2153_i_QueuedCommandsCount++"]),
    ("COMMAND.C", 2058, 2100, ["G0435_B_CommandQueueLocked", "G2153_i_QueuedCommandsCount == 0", "F0360_COMMAND_ProcessPendingClick"]),
    ("CLIKMENU.C", 142, 174, ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0284_CHAMPION_SetPartyDirection", "G0321_B_StopWaitingForPlayerInput"]),
    ("CLIKMENU.C", 180, 330, ["F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0357_COMMAND_DiscardAllInput"]),
]

PRODUCT_MARKERS = [
    ("src/engine/main_loop_m11.c", "if (strcmp(name, \"kp-1\") == 0 || strcmp(name, \"kp1\") == 0) return SDLK_KP_1"),
    ("src/engine/main_loop_m11.c", "if (strcmp(name, \"kp-6\") == 0 || strcmp(name, \"kp6\") == 0) return SDLK_KP_6"),
    ("src/engine/main_loop_m11.c", "case SDLK_KP_5:"),
    ("src/engine/main_loop_m11.c", "case SDLK_KP_1:"),
    ("src/engine/main_loop_m11.c", "case SDLK_KP_6:"),
    ("src/engine/m11_game_view.c", "m11_dm1_v1_pipeline_command_for_input"),
    ("src/engine/m11_game_view.c", "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat"),
    ("src/engine/m11_game_view.c", "No OS keypad/NumLock synthesis is involved"),
]

REPORT_MARKERS = [
    "RECLASSIFIED_PASS348_NUMLOCK_KEYPAD_BLOCKER_NARROWED_NOT_CLOSED",
    "RESIDUAL_PASS348_I34E_DOS_KEYBOARD_BUFFER_NOT_PROVEN",
    "INPUT.C",
    "IO2.C",
    "COMMAND.C",
    "CLIKMENU.C",
]


def run(cmd: list[str]) -> dict:
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"cmd": cmd, "returncode": p.returncode, "outputTail": p.stdout[-4000:]}


def window(path: Path, start: int, end: int) -> str:
    lines = path.read_text(errors="replace").splitlines()
    return "\n".join(lines[start - 1:end])


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    redmcsb = Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", str(DEFAULT_REDMCSB)))
    build_dir = Path(os.environ.get("FIRESTAFF_PASS348_BUILD_DIR", str(DEFAULT_BUILD)))
    checks = []

    report_text = REPORT.read_text(encoding="utf-8") if REPORT.exists() else ""
    checks.append({"kind": "report", "ok": REPORT.exists() and all(m in report_text for m in REPORT_MARKERS), "markers": REPORT_MARKERS})

    for filename, start, end, markers in SOURCE_LOCKS:
        path = redmcsb / filename
        text = window(path, start, end) if path.exists() else ""
        checks.append({"kind": "redmcsb_source_lock", "file": filename, "lines": f"{start}-{end}", "ok": path.exists() and all(m in text for m in markers), "markers": markers})

    for rel, marker in PRODUCT_MARKERS:
        text = (ROOT / rel).read_text(errors="replace") if (ROOT / rel).exists() else ""
        checks.append({"kind": "product_marker", "file": rel, "marker": marker, "ok": marker in text})

    if build_dir.exists():
        shutil.rmtree(build_dir)
    conf = run(["cmake", "-S", str(ROOT), "-B", str(build_dir)])
    checks.append({"kind": "cmake_configure", "ok": conf["returncode"] == 0, "result": conf})
    build = run(["cmake", "--build", str(build_dir), "--target", "test_dm1_v1_input_command_queue_pc34_compat", "-j1"])
    test_bin = build_dir / "test_dm1_v1_input_command_queue_pc34_compat"
    checks.append({"kind": "cmake_build_targets", "ok": build["returncode"] == 0 and test_bin.exists(), "result": build})
    if test_bin.exists():
        res = run([str(test_bin)])
        checks.append({"kind": "executable_gate", "name": test_bin.name, "ok": res["returncode"] == 0 and "sourceEvidence=" in res["outputTail"], "result": res})
    else:
        checks.append({"kind": "executable_gate", "name": test_bin.name, "ok": False, "missing": str(test_bin)})

    ok = all(c.get("ok") for c in checks)
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": "RECLASSIFIED_PASS348_NUMLOCK_KEYPAD_BLOCKER_NARROWED_NOT_CLOSED" if ok else "BLOCKED_PASS348_VERIFICATION_FAILED",
        "sourceRoot": str(redmcsb),
        "decision": {
            "pass333BlockerFullyClosed": False,
            "m11SdlNumlockKeypadRouteClosed": True,
            "i34eDosKeyboardBufferProven": False,
            "residual": "RESIDUAL_PASS348_I34E_DOS_KEYBOARD_BUFFER_NOT_PROVEN",
        },
        "sourceLocks": [{"file": f, "lines": f"{a}-{b}", "markers": m} for f, a, b, m in SOURCE_LOCKS],
        "productMarkers": [{"file": f, "marker": m} for f, m in PRODUCT_MARKERS],
        "checks": checks,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"status={manifest['status']}")
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
