#!/usr/bin/env python3
"""Pass430: prove DM1 V1 relative movement vector tables stay source-locked.

This audits the ReDMCSB movement-arrow forward/right tables and the F0150
relative-coordinate mutation, then checks Firestaff's isolated movement model
keeps the same command-action and cardinal direction vectors.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
OUT_DIR = ROOT / "parity-evidence" / "verification" / "pass430_dm1_v1_relative_step_vectors_source_lock"
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / "pass430_dm1_v1_relative_step_vectors_source_lock.md"


def read(path: Path, encoding: str = "latin-1") -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=encoding, errors="replace")


def compact(text: str) -> str:
    return " ".join(text.split())


def require(text: str, needle: str, label: str) -> None:
    if compact(needle) not in compact(text):
        raise AssertionError(f"missing {label}: {needle!r}")


def require_order(text: str, needles: list[str], label: str) -> None:
    flat = compact(text)
    pos = -1
    for needle in needles:
        hit = flat.find(compact(needle), pos + 1)
        if hit < 0:
            raise AssertionError(f"{label}: missing ordered needle {needle!r}")
        pos = hit


def line(text: str, needle: str) -> int:
    i = text.find(needle)
    if i < 0:
        raise AssertionError(f"missing line needle {needle!r}")
    return text.count("\n", 0, i) + 1


def extract_int_array(text: str, symbol: str) -> list[int]:
    m = re.search(rf"{re.escape(symbol)}\[4\]\s*=\s*\{{(?P<body>.*?)\}}", text, re.S)
    if not m:
        raise AssertionError(f"missing array initializer for {symbol}")
    values = [int(x) for x in re.findall(r"(?<![A-Za-z0-9_])-?\d+", m.group("body"))]
    if len(values) != 4:
        raise AssertionError(f"{symbol} expected 4 values, got {values}")
    return values


def source_audit() -> dict:
    clikmenu = read(RED / "CLIKMENU.C")
    dungeon = read(RED / "DUNGEON.C")

    fwd = extract_int_array(clikmenu, "G0465_ai_Graphic561_MovementArrowToStepForwardCount")
    right = extract_int_array(clikmenu, "G0466_ai_Graphic561_MovementArrowToStepRightCount")
    if fwd != [1, 0, -1, 0]:
        raise AssertionError(f"unexpected ReDMCSB forward table: {fwd}")
    if right != [0, 1, 0, -1]:
        raise AssertionError(f"unexpected ReDMCSB right table: {right}")

    require_order(clikmenu, [
        "AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD;",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(G0308_i_PartyDirection, G0465_ai_Graphic561_MovementArrowToStepForwardCount[AL1118_ui_MovementArrowIndex], G0466_ai_Graphic561_MovementArrowToStepRightCount[AL1118_ui_MovementArrowIndex], &L1121_i_MapX, &L1122_i_MapY);",
    ], "F0366 command-to-relative-vector route")
    require_order(dungeon, [
        "void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0254_i_StepsForwardCount",
        "*P0257_pi_MapY += G0234_ai_Graphic559_DirectionToStepNorthCount[P0253_i_Direction] * P0254_i_StepsForwardCount",
        "P0253_i_Direction += 1, P0253_i_Direction &= 3",
        "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0255_i_StepsRightCount",
        "*P0257_pi_MapY += G0234_ai_Graphic559_DirectionToStepNorthCount[P0253_i_Direction] * P0255_i_StepsRightCount",
    ], "F0150 forward then simulated-right coordinate mutation")

    return {
        "CLIKMENU.C:G0465_forward_counts": {"lines": [line(clikmenu, "G0465_ai_Graphic561_MovementArrowToStepForwardCount[4] ="), line(clikmenu, "0 }; /* Left */")], "values": fwd},
        "CLIKMENU.C:G0466_right_counts": {"lines": [line(clikmenu, "G0466_ai_Graphic561_MovementArrowToStepRightCount[4] ="), line(clikmenu, "-1 }; /* Left */")], "values": right},
        "CLIKMENU.C:F0366_command_index_to_F0150": [line(clikmenu, "AL1118_ui_MovementArrowIndex = P0735_ui_Command - C003_COMMAND_MOVE_FORWARD;"), line(clikmenu, "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement")],
        "DUNGEON.C:F0150_relative_coordinate_mutation": [line(dungeon, "void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement"), line(dungeon, "P0253_i_Direction += 1, P0253_i_Direction &= 3"), line(dungeon, "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0255_i_StepsRightCount")],
    }


def firestaff_audit() -> dict:
    movement = read(ROOT / "src/dm1/dm1_v1_movement_pc34_compat.c", "utf-8")
    core = read(ROOT / "src/dm1/dm1_v1_movement_command_core_pc34_compat.c", "utf-8")
    test = read(ROOT / "tests/test_dm1_v1_movement_core_pc34_compat.c", "utf-8")

    require(movement, "static const int16_t step_forward_count[4] = {  1,  0, -1,  0 };", "Firestaff forward-count table")
    require(movement, "static const int16_t step_right_count[4]   = {  0,  1,  0, -1 };", "Firestaff right-count table")
    require(movement, "static const int16_t dir_dx[4] = {  0,  1,  0, -1 };", "Firestaff east/west table")
    require(movement, "static const int16_t dir_dy[4] = { -1,  0,  1,  0 };", "Firestaff north/south table")
    require_order(movement, [
        "arrow_index = command - 3;",
        "fwd_count   = step_forward_count[arrow_index];",
        "right_count = step_right_count[arrow_index];",
        "abs_facing = state->facing;",
        "new_x = state->pos_x",
        "+ fwd_count   * dir_dx[abs_facing]",
        "+ right_count * dir_dx[normalize_dir(abs_facing + 1)];",
        "new_y = state->pos_y",
        "+ fwd_count   * dir_dy[abs_facing]",
        "+ right_count * dir_dy[normalize_dir(abs_facing + 1)];",
    ], "Firestaff relative-step arithmetic")
    require(core, "action = dm1_v1_command_to_move_action(outResult->queue.command);", "command core action index")
    require(core, "F0702_MOVEMENT_TryMove_Compat", "command core uses movement seam")
    for needle in [
        "north forward dx",
        "north right dx",
        "east backward dx",
        "south left dx",
        "forward key dispatches queued movement",
    ]:
        require(test, needle, f"movement vector probe {needle}")

    return {
        "src/dm1/dm1_v1_movement_pc34_compat.c": "static step_forward/right tables and cardinal dx/dy tables mirror ReDMCSB F0366/F0150 arithmetic",
        "src/dm1/dm1_v1_movement_command_core_pc34_compat.c": "dequeued C003..C006 commands are converted to the same zero-based movement-arrow index before movement resolution",
        "tests/test_dm1_v1_movement_core_pc34_compat.c": "covers forward/back/strafe vectors for north and forward for east",
    }


def main() -> int:
    source = source_audit()
    firestaff = firestaff_audit()
    manifest = {
        "schema": "firestaff.parity.pass430.dm1_v1_relative_step_vectors_source_lock.v1",
        "status": "PASS430_RELATIVE_STEP_VECTORS_SOURCE_LOCKED",
        "redmcsbRoot": str(RED),
        "sourceAudit": source,
        "firestaffAudit": firestaff,
        "claim": "DM1 V1 movement-arrow commands use ReDMCSB forward/right count vectors and F0150 forward-then-simulated-right coordinate mutation semantics.",
        "blocker": None,
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass430 — DM1 V1 relative step vectors source lock",
        "",
        "Status: `PASS430_RELATIVE_STEP_VECTORS_SOURCE_LOCKED`",
        "",
        "## ReDMCSB anchors",
    ]
    for name, detail in source.items():
        lines.append(f"- `{name}` — `{detail}`")
    lines += ["", "## Firestaff seams"]
    for name, detail in firestaff.items():
        lines.append(f"- `{name}` — {detail}")
    lines += ["", f"Manifest: `{OUT_JSON.relative_to(ROOT)}`", ""]
    OUT_MD.write_text("\n".join(lines), encoding="utf-8")
    print(f"pass430_dm1_v1_relative_step_vectors_source_lock=pass manifest={OUT_JSON.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
