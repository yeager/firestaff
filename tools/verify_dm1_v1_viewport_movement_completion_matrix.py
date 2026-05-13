#!/usr/bin/env python3
"""Executable aggregate for current DM1 V1 movement/viewport completion state.

This is deliberately evidence-backed, not optimistic.  It starts from ReDMCSB
WIP20210206 source slices, chains the concrete Firestaff gates that currently
own movement -> viewport rendering, and records the expected unresolved original
route blocker rather than treating a blocked report as completion.
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
DEFAULT_SOURCE = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
OUT_DIR = ROOT / "parity-evidence" / "verification" / "dm1_v1_viewport_movement_completion_matrix"
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / "dm1_v1_viewport_movement_completion_matrix.md"
BUILD_DIR = ROOT / "build"

SOURCE_ROWS: list[dict[str, Any]] = [
    {
        "id": "post_command_redraw_loop",
        "function": "GAMELOOP main input/redraw loop",
        "file": "GAMELOOP.C",
        "range": "55-90",
        "ordered": [
            "for (;;) { /*_Infinite loop_*/",
            "F0261_TIMELINE_Process_CPSEF();",
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
        ],
        "readyClaim": "main loop redraws viewport from the current party tuple before entering the input wait cycle",
    },
    {
        "id": "cooldown_age_before_f0380",
        "function": "GAMELOOP cooldown tick",
        "file": "GAMELOOP.C",
        "range": "150-155",
        "ordered": [
            "if (G0310_i_DisabledMovementTicks)",
            "G0310_i_DisabledMovementTicks--;",
            "if (G0311_i_ProjectileDisabledMovementTicks)",
            "G0311_i_ProjectileDisabledMovementTicks--;",
        ],
        "readyClaim": "old movement/projectile cooldowns age once per loop tick before queued input processing",
    },
    {
        "id": "queue_dispatch_turn_move_and_blocked_gates",
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "file": "COMMAND.C",
        "range": "2045-2156",
        "ordered": [
            "void F0380_COMMAND_ProcessQueue_CPSC(",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G0310_i_DisabledMovementTicks || (G0311_i_ProjectileDisabledMovementTicks",
            "G2153_i_QueuedCommandsCount--;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "readyClaim": "eligible queued turn/move commands dispatch through source handlers after movement-disabled checks",
    },
    {
        "id": "movement_legality_and_result_chain",
        "function": "F0366/F0267 movement blockers and side effects",
        "file": "CLIKMENU.C",
        "range": "180-351",
        "ordered": [
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "L1117_B_MovementBlocked",
            "F0357_COMMAND_DiscardAllInput();",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        ],
        "readyClaim": "wall/door/fakewall/group blockers return before movement side effects; successful moves enter F0267 timing/results",
    },
    {
        "id": "viewport_far_to_near_wall_replay",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "file": "DUNVIEW.C",
        "range": "8318-8618",
        "ordered": [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0116_DUNGEONVIEW_DrawSquareD3L",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
            "F0121_DUNGEONVIEW_DrawSquareD2C",
            "F0124_DUNGEONVIEW_DrawSquareD1C",
            "F0127_DUNGEONVIEW_DrawSquareD0C",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "readyClaim": "viewport squares replay source-order far-to-near and then request presentation",
    },
    {
        "id": "wall_door_alcove_occlusion_contract",
        "function": "F0116/F0117/F0118 wall and door branches",
        "file": "DUNVIEW.C",
        "range": "6421-6840",
        "ordered": [
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L);",
            "L0200_i_Order = C0x0000_CELL_ORDER_ALCOVE;",
            "return;",
            "case C17_ELEMENT_DOOR_FRONT:",
            "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
            "F0111_DUNGEONVIEW_DrawDoor(L0201_ai_SquareAspect[M557_DOOR_THING_INDEX]",
            "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        ],
        "readyClaim": "wall squares normally occlude/return, while alcoves and front doors explicitly hand contents to F0115",
    },
    {
        "id": "d2c_floor_field_and_front_door_order",
        "function": "F0121_DUNGEONVIEW_DrawSquareD2C",
        "file": "DUNVIEW.C",
        "range": "7244-7388",
        "ordered": [
            "STATICFUNCTION void F0121_DUNGEONVIEW_DrawSquareD2C",
            "case C17_ELEMENT_DOOR_FRONT:",
            "F0108_DUNGEONVIEW_DrawFloorOrnament",
            "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
            "F0111_DUNGEONVIEW_DrawDoor",
            "L0211_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
            "F0113_DUNGEONVIEW_DrawField",
        ],
        "readyClaim": "D2C keeps source floor/stairs/field/door/front-content ordering, including the recent D2C field/floor regression surface",
    },
    {
        "id": "f0115_thing_layer_handoff",
        "function": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "file": "DUNVIEW.C",
        "range": "4547-5938",
        "ordered": [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(",
            "P0146_ui_OrderedViewCellOrdinals >>= 4;",
            "C04_THING_TYPE_GROUP",
            "C14_THING_TYPE_PROJECTILE",
            "P0141_T_Thing = L0146_T_FirstThingToDraw; /* Restart processing list of things from the beginning.",
            "C15_THING_TYPE_EXPLOSION",
        ],
        "readyClaim": "objects/creatures/projectiles are processed per packed cell, while explosion handling restarts after the packed-cell pass",
    },
    {
        "id": "drawview_palette_and_present_cadence",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "file": "DRAWVIEW.C",
        "range": "709-900",
        "ordered": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "F0694_SetMultipleColorsInPalette(G2061_DungeonViewPaletteIndices[G0304_i_DungeonViewPaletteIndex]);",
            "F0638_GetZone(C007_ZONE_VIEWPORT, L2414_ai_XYZ);",
            "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",
        ],
        "readyClaim": "viewport present uses the single source dungeon palette index and vblank copy cadence rather than invented depth dimming",
    },
]

CHAINED_GATES: list[dict[str, Any]] = [
    {"id": "pass381_movement_viewport_walls_source_lock", "cmd": [sys.executable, "tools/verify_pass381_dm1_v1_movement_viewport_walls_source_lock.py", "--json"], "covers": "command queue -> movement/turn state -> viewport wall redraw and presentation source chain"},
    {"id": "pass423_input_command_movement_pipeline_source_lock", "cmd": [sys.executable, "tools/verify_pass423_dm1_v1_input_command_movement_pipeline_source_lock.py"], "covers": "PC34 input, queue, F0380, F0365/F0366 and command-core regressions"},
    {"id": "pass402_movement_cooldown_order", "cmd": [sys.executable, "scripts/verify_pass402_dm1_v1_movement_cooldown_order.py"], "covers": "cooldown ageing before F0380 and no same-tick post-decrement"},
    {"id": "pass406_movement_legality_completion_gate", "cmd": [sys.executable, "tools/verify_pass406_dm1_v1_movement_legality_completion_gate.py"], "covers": "party target-square legality, collision blockers, pits/teleporters/groups, and movement-result chain"},
    {"id": "pass406_game_loop_redraw_cadence", "cmd": [sys.executable, "tools/verify_pass406_dm1_v1_game_loop_redraw_cadence.py"], "covers": "game-loop redraw cadence, viewport dirty publication, draw/present/vblank ordering"},
    {"id": "pass395_viewport_walls_source_runtime_lock", "cmd": [sys.executable, "scripts/verify_pass395_dm1_v1_viewport_walls_source_runtime_lock.py"], "covers": "wall replay, door two-pass, F0115 handoff, and post-command redraw metadata/runtime contract"},
    {"id": "pass405_projectile_explosion_layer_occlusion", "cmd": [sys.executable, "tools/verify_pass405_dm1_v1_viewport_projectile_explosion_layer_occlusion.py"], "covers": "projectile/explosion layer split, deferred explosion pass, and center/side occlusion guards"},
    {"id": "viewport_square_collision_source_lock", "cmd": [sys.executable, "scripts/verify_dm1_v1_viewport_square_collision_source_lock.py"], "covers": "visible viewport cells are map-backed and agree with movement collision square state"},
    {"id": "viewport_field_zone_aspect_clip_gate", "cmd": [sys.executable, "tools/verify_v1_viewport_field_zone_aspect_clip_gate.py"], "covers": "field/teleporter zone/aspect clipping behind nearer blockers"},
    {"id": "viewport_palette_source_lock_gate", "cmd": [sys.executable, "tools/verify_v1_viewport_palette_source_lock_gate.py"], "covers": "single ReDMCSB dungeon palette cadence; rejects invented depth palette dimming"},
    {"id": "pass434_original_viewport_crop_readiness_gate", "cmd": [sys.executable, "tools/verify_pass434_dm1_v1_original_viewport_crop_readiness_gate.py"], "expectedStatus": "PASS_PASS434_ORIGINAL_VIEWPORT_CROP_READINESS", "covers": "original viewport crop/source readiness is available"},
]

EXPECTED_BLOCKER_GATES: list[dict[str, Any]] = [
    {"id": "pass435_semantic_original_route_readiness_gate", "cmd": [sys.executable, "tools/verify_pass435_dm1_v1_semantic_original_route_readiness_gate.py"], "expectedStatus": "BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY", "covers": "remaining original semantic route blocker: F0365/F0366 dispatch + six non-duplicate semantic route states not yet proven"},
]


REQUIRED_BUILD_TARGETS = [
    "test_dm1_v1_input_command_queue_pc34_compat",
    "test_dm1_v1_movement_command_core_pc34_compat",
    "test_dm1_v1_command_movement_sensor_timing_pc34_compat",
    "test_dm1_v1_movement_core_pc34_compat",
    "test_m11_v1_turning_presentation_pc34_compat",
    "test_dm1_v1_movement_pipeline_pc34_compat",
]


def compact(text: str) -> str:
    return " ".join(text.split())


def read_source_slice(source: Path, rel: str, line_range: str) -> str:
    start_s, end_s = line_range.split("-", 1)
    start, end = int(start_s), int(end_s)
    path = source / rel
    if not path.is_file():
        raise FileNotFoundError(path)
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    return "\n".join(lines[start - 1:end])


def verify_source_rows(source: Path) -> tuple[bool, list[dict[str, Any]]]:
    ok = True
    results: list[dict[str, Any]] = []
    for row in SOURCE_ROWS:
        missing: list[str] = []
        try:
            haystack = compact(read_source_slice(source, row["file"], row["range"]))
            pos = -1
            for marker in row["ordered"]:
                needle = compact(marker)
                found = haystack.find(needle, pos + 1)
                if found < 0:
                    missing.append(marker)
                else:
                    pos = found
        except FileNotFoundError as exc:
            missing.append(f"missing source file: {exc}")
        passed = not missing
        ok = ok and passed
        results.append({
            "id": row["id"],
            "passed": passed,
            "source": row["file"] + ":" + row["range"],
            "function": row["function"],
            "readyClaim": row["readyClaim"],
            "missing": missing,
        })
    return ok, results


def parse_status(output: str) -> str | None:
    try:
        first_json = output[output.index("{"):]
        return json.loads(first_json).get("status")
    except Exception:
        pass
    m = re.search(r"\b(?:status=)?((?:PASS|BLOCKED|FAIL)[A-Z0-9_]+)\b", output)
    return m.group(1) if m else None


def run_checked(cmd: list[str], timeout: int = 900) -> dict[str, Any]:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    return {
        "cmd": cmd,
        "passed": proc.returncode == 0,
        "returncode": proc.returncode,
        "outputTail": proc.stdout.splitlines()[-20:],
    }


def ensure_required_build() -> list[dict[str, Any]]:
    steps: list[dict[str, Any]] = []
    if not (BUILD_DIR / "CMakeCache.txt").is_file():
        steps.append(run_checked(["cmake", "-S", str(ROOT), "-B", str(BUILD_DIR)]))
        if not steps[-1]["passed"]:
            return steps
    steps.append(run_checked(["cmake", "--build", str(BUILD_DIR), "--target", *REQUIRED_BUILD_TARGETS, "-j2"]))
    return steps


def run_gate(entry: dict[str, Any], source: Path) -> dict[str, Any]:
    cmd = list(entry["cmd"])
    if entry["id"] == "pass381_movement_viewport_walls_source_lock":
        cmd.extend(["--source", str(source)])
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=180)
    status = parse_status(proc.stdout)
    expected = entry.get("expectedStatus")
    passed = proc.returncode == 0 and (expected is None or status == expected)
    return {
        "id": entry["id"],
        "passed": passed,
        "cmd": cmd,
        "covers": entry["covers"],
        "expectedStatus": expected,
        "status": status,
        "returncode": proc.returncode,
        "outputTail": proc.stdout.splitlines()[-20:],
    }


def write_report(payload: dict[str, Any]) -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# DM1 V1 viewport/movement completion aggregate",
        "",
        "Status: `" + ("PASS" if payload["passed"] else "FAIL") + "`",
        "Generated: `" + payload["timestampUtc"] + "`",
        "",
        "## ReDMCSB source audit",
    ]
    for row in payload["sourceRows"]:
        lines.append("- `" + ("PASS" if row["passed"] else "FAIL") + "` `" + row["id"] + "` — `" + row["source"] + "` `" + row["function"] + "`: " + row["readyClaim"])
        for missing in row.get("missing", []):
            lines.append(f"  - missing/order: `{missing}`")
    lines += ["", "## Executable gates"]
    for row in payload["chainedGates"]:
        status = " status `" + str(row["status"]) + "`" if row.get("status") else ""
        lines.append("- `" + ("PASS" if row["passed"] else "FAIL") + "` `" + row["id"] + "` rc=`" + str(row["returncode"]) + "`" + status + ": " + row["covers"])
        if not row["passed"]:
            lines.extend(f"  - {x}".rstrip() for x in row["outputTail"][-8:] if x.strip())
    lines += ["", "## Expected blockers"]
    for row in payload["expectedBlockerGates"]:
        lines.append("- `" + ("CONFIRMED" if row["passed"] else "UNEXPECTED") + "` `" + row["id"] + "` expected `" + str(row.get("expectedStatus")) + "` observed `" + str(row.get("status")) + "`: " + row["covers"])
        if row.get("outputTail"):
            for x in row["outputTail"][-8:]:
                lines.append(f"  - {x}")
    lines += [
        "",
        "## Decision",
        payload["decision"],
        "",
        f"Manifest: `{MANIFEST.relative_to(ROOT)}`",
    ]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--skip-chained", action="store_true")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    source_ok, source_results = verify_source_rows(args.source)
    build_steps: list[dict[str, Any]] = []
    chain_results: list[dict[str, Any]] = []
    blocker_results: list[dict[str, Any]] = []
    chain_ok = True
    blocker_ok = True
    if not args.skip_chained:
        build_steps = ensure_required_build()
        chain_ok = all(step["passed"] for step in build_steps)
        if not chain_ok:
            chain_results.extend({"id": "prepare_required_build", "covers": "configure/build movement targets required by chained gates", "expectedStatus": None, "status": None, **step} for step in build_steps)
        for entry in CHAINED_GATES:
            result = run_gate(entry, args.source)
            chain_ok = chain_ok and result["passed"]
            chain_results.append(result)
        for entry in EXPECTED_BLOCKER_GATES:
            result = run_gate(entry, args.source)
            blocker_ok = blocker_ok and result["passed"]
            blocker_results.append(result)

    ok = source_ok and chain_ok and blocker_ok
    payload = {
        "schema": "firestaff.dm1_v1_viewport_movement_completion_matrix.v2",
        "gate": "dm1_v1_viewport_movement_completion_matrix",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "sourceRoot": str(args.source),
        "passed": ok,
        "sourceRows": source_results,
        "buildSteps": build_steps,
        "chainedGates": chain_results,
        "expectedBlockerGates": blocker_results,
        "decision": "Current movement/viewport source-lock gates are green, pass434 crop readiness is green, and pass435 confirms the remaining blocker is original semantic route readiness; no original pixel/route parity is claimed." if ok else "One or more source rows, executable gates, or expected blocker classifications failed; do not update completion claims.",
    }
    write_report(payload)
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        for row in source_results:
            print(("PASS" if row["passed"] else "FAIL") + " source " + row["id"] + " " + row["source"] + " " + row["function"])
            print("  " + row["readyClaim"])
            for marker in row["missing"]:
                print(f"  missing/order: {marker}")
        for row in chain_results:
            status = " status=" + str(row["status"]) if row.get("status") else ""
            print(("PASS" if row["passed"] else "FAIL") + " chained " + row["id"] + " rc=" + str(row["returncode"]) + status)
            print("  " + row["covers"])
            if not row["passed"]:
                for line in row["outputTail"]:
                    print(f"  {line}")
        for row in blocker_results:
            print(("PASS" if row["passed"] else "FAIL") + " expected-blocker " + row["id"] + " observed=" + str(row.get("status")) + " expected=" + str(row.get("expectedStatus")))
            print("  " + row["covers"])
        print(f"report={REPORT.relative_to(ROOT)}")
        print(f"manifest={MANIFEST.relative_to(ROOT)}")
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
