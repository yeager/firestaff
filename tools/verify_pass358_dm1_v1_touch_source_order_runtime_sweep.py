#!/usr/bin/env python3
"""Verify pass358 DM1 V1 touch source-order/runtime sweep artifacts."""
from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
MANIFEST = ROOT / "parity-evidence/verification/pass358_dm1_v1_touch_source_order_runtime_sweep/manifest.json"
EVIDENCE = ROOT / "parity-evidence/pass358_dm1_v1_touch_source_order_runtime_sweep.md"
EXPECTED_STATUS = "PASS_DM1_V1_TOUCH_SOURCE_ORDER_RUNTIME_SWEEP"


def fail(message: str) -> int:
    print(f"status=FAIL_PASS358_TOUCH_SOURCE_ORDER_RUNTIME_SWEEP reason={message}")
    return 1


def text(path: Path) -> str:
    return path.read_text(encoding="utf-8")



def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def require_all(blob: str, needles: list[str], context: str) -> None:
    compact = " ".join(blob.split())
    for needle in needles:
        require(" ".join(needle.split()) in compact, f"{context} missing {needle}")


def source_block(file_name: str, start: int, end: int) -> str:
    path = SOURCE_ROOT / file_name
    require(path.exists(), f"missing ReDMCSB source {path}")
    lines = path.read_text(encoding="latin-1").splitlines()
    return "\n".join(lines[start - 1:end])


def run_probe(exe_name: str) -> str:
    exe = ROOT / "build" / exe_name
    if not exe.exists():
        subprocess.run(["cmake", "--build", str(ROOT / "build"), "--target", exe_name, "-j2"], cwd=ROOT, check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    return subprocess.run([str(exe)], cwd=ROOT, check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True).stdout


def main() -> int:
    try:
        manifest = json.loads(text(MANIFEST))
        evidence = text(EVIDENCE)
        touch_matrix = text(ROOT / "src/shared/touch_click_zone_matrix_pc34_compat.c")
        touch_pointer = text(ROOT / "src/shared/touch_pointer_input_pc34_compat.c")
        touch_matrix_test = text(ROOT / "tests/test_touch_click_zone_matrix_pc34_compat_integration.c")
        touch_pointer_test = text(ROOT / "tests/test_touch_pointer_input_pc34_compat_integration.c")
        live_probe = text(ROOT / "probes/m11/firestaff_m11_touch_live_dispatch_gate_probe.c")
        cmake = text(ROOT / "CMakeLists.txt")

        require(manifest.get("status") == EXPECTED_STATUS, "manifest status mismatch")
        require("raw source dumps" not in evidence.lower(), "evidence should cite, not dump source")
        for anchor in manifest.get("source_anchors", []):
            marker = anchor["file"] + ":" + anchor["lines"]
            require(marker in evidence, f"evidence missing anchor {marker}")

        require_all(source_block("STARTUP2.C", 1179, 1182), [
            "G0441_ps_PrimaryMouseInput = G0447_as_Graphic561_PrimaryMouseInput_Interface",
            "G0442_ps_SecondaryMouseInput = G0448_as_Graphic561_SecondaryMouseInput_Movement",
        ], "STARTUP2.C:1179-1182")
        require_all(source_block("COMMAND.C", 375, 405), [
            "G0447_as_Graphic561_PrimaryMouseInput_Interface",
            "C012_COMMAND_CLICK_IN_CHAMPION_0_STATUS_BOX",
            "C100_COMMAND_CLICK_IN_SPELL_AREA",
            "C111_COMMAND_CLICK_IN_ACTION_AREA",
            "G0448_as_Graphic561_SecondaryMouseInput_Movement",
            "C003_COMMAND_MOVE_FORWARD",
            "C080_COMMAND_CLICK_IN_DUNGEON_VIEW",
            "C083_COMMAND_TOGGLE_INVENTORY_LEADER",
        ], "COMMAND.C:375-405")
        require_all(source_block("COMMAND.C", 412, 451), [
            "CM1_SCREEN_RELATIVE", "CM2_VIEWPORT_RELATIVE", "C070_COMMAND_CLICK_ON_MOUTH", "C071_COMMAND_CLICK_ON_EYE",
        ], "COMMAND.C:412-451")
        require_all(source_block("COMMAND.C", 498, 511), [
            "G0456_as_Graphic561_MouseInput_PanelChest", "G0457_as_Graphic561_MouseInput_PanelResurrectReincarnateCancel",
            "C160_COMMAND_CLICK_IN_PANEL_RESURRECT", "M664_ZONE_RESURRECT", "C162_COMMAND_CLICK_IN_PANEL_CANCEL",
        ], "COMMAND.C:498-511")
        require_all(source_block("COMMAND.C", 1379, 1449), [
            "F0358_COMMAND_GetCommandFromMouseInput_CPSC", "while (L1107_Command = P0721_ps_MouseInput->Command)",
            "P0724_i_ButtonsStatus & P0721_ps_MouseInput->Button", "P0721_ps_MouseInput++",
        ], "COMMAND.C:1379-1449")
        require_all(source_block("COMMAND.C", 1641, 1661), [
            "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput",
            "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput",
            "G0432_as_CommandQueue", ".X = P0725_i_X", ".Y = P0726_i_Y",
        ], "COMMAND.C:1641-1661")
        require_all(source_block("COMMAND.C", 1692, 1707), ["F0360_COMMAND_ProcessPendingClick", "G0436_B_PendingClickPresent", "G0437_i_PendingClickX", "G0439_i_PendingClickButtonsStatus"], "COMMAND.C:1692-1707")
        require_all(source_block("COMMAND.C", 2296, 2324), ["C083_COMMAND_TOGGLE_INVENTORY_LEADER", "C111_COMMAND_CLICK_IN_ACTION_AREA", "C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "F0377_COMMAND_ProcessType80_ClickInDungeonView"], "COMMAND.C:2296-2324")
        require_all(source_block("CLIKMENU.C", 519, 585), ["F0371_COMMAND_ProcessType111To115_ClickInActionArea_CPSE", "G0452_as_Graphic561_MouseInput_ActionAreaNames", "G0453_as_Graphic561_MouseInput_ActionAreaIcons"], "CLIKMENU.C:519-585")
        require_all(source_block("COORD.C", 1693, 1698), ["G2067_i_ViewportScreenX = 0", "G2068_i_ViewportScreenY = 33"], "COORD.C:1693-1698")
        require_all(source_block("DEFS.H", 202, 215), ["MOUSE_INPUT", "MASK0x0001_MOUSE_RIGHT_BUTTON", "MASK0x0002_MOUSE_LEFT_BUTTON"], "DEFS.H:202-215")
        require_all(source_block("DEFS.H", 3979, 3982), ["M664_ZONE_RESURRECT", "570", "M665_ZONE_REINCARNATE", "571", "M666_ZONE_CANCEL", "573"], "DEFS.H:3979-3982")

        require_all(touch_matrix, [
            "TOUCHCLICK_Compat_HitTestPrimaryThenSecondary",
            "kPrimaryInterfaceSourceOrder", "kSecondaryMovementSourceOrder",
            "TOUCHCLICK_Compat_MapViewportLocalPointToDispatch", "TOUCHCLICK_Compat_MapScaledScreenPointToDispatch",
        ], "src/shared/touch_click_zone_matrix_pc34_compat.c")
        require(touch_matrix.index("kPrimaryInterfaceSourceOrder") < touch_matrix.index("kSecondaryMovementSourceOrder"), "primary source order must appear before secondary order")
        require_all(touch_pointer, [
            "TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(event->x, event->y",
            "TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(screenX, screenY",
            "TOUCHCLICK_Compat_MapViewportLocalPointToDispatch",
            "DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat",
        ], "src/shared/touch_pointer_input_pc34_compat.c")
        require_all(touch_matrix_test, [
            "TOUCHCLICK_Compat_HitTestPrimaryThenSecondary(25, 11", "movement.forward", "inventory.toggle_leader",
            "TOUCHCLICK_Compat_MapViewportLocalPointToDispatch(104, 113", "TOUCHCLICK_Compat_MapScaledScreenPointToDispatch",
        ], "tests/test_touch_click_zone_matrix_pc34_compat_integration.c")
        require_all(touch_pointer_test, [
            "TOUCH_POINTER_SPACE_VIEWPORT_LOCAL_PC34_COMPAT", "DM1_V1_COMMAND_MOVE_FORWARD", "panel.cancel", "pendingClickPresent",
        ], "tests/test_touch_pointer_input_pc34_compat_integration.c")
        require_all(live_probe, [
            "primary_left_beats_status_box_child", "secondary_left_movement_fallback", "secondary_right_screen_toggle",
            "pending_replay_original_click_state", "TOUCHPOINTER_Compat_EnqueueEventToInputCommandQueue",
        ], "firestaff_m11_touch_live_dispatch_gate_probe.c")
        require("pass350_dm1_v1_touch_live_dispatch_gate" in cmake, "pass350 CTest missing")

        if (ROOT / "build").exists():
            pointer_output = run_probe("test_touch_pointer_input_pc34_compat_integration")
            live_output = run_probe("firestaff_m11_touch_live_dispatch_gate_probe")
            require("touchPointerInputInvariantOk=1" in pointer_output, "pointer probe invariant failed")
            require("pass350TouchLiveDispatchGateOk=1" in live_output, "live dispatch gate failed")

        print(f"status={EXPECTED_STATUS}")
        print("sourceAnchors=%u" % len(manifest.get("source_anchors", [])))
        print("firestaffArtifactsOk=1")
        return 0
    except (AssertionError, OSError, json.JSONDecodeError, subprocess.CalledProcessError) as exc:
        return fail(str(exc))


if __name__ == "__main__":
    sys.exit(main())
