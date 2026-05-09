#!/usr/bin/env python3
"""Executable completion matrix for DM1 V1 movement -> viewport/wall readiness.

This gate is intentionally source-first.  It re-checks the ReDMCSB anchors that
bind command queue, movement cooldowns, post-command redraw, far-to-near wall
replay, door/alcove exceptions, and F0115 thing layer handoff; then it chains the
narrow Firestaff source-lock probes that cover the same recent work lanes.
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOURCE = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))

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
        "id": "viewport_far_to_near_wall_replay",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "file": "DUNVIEW.C",
        "range": "8318-8610",
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
        "readyClaim": "objects/creatures/projectiles/explosions are replayed in the source F0115 layer pass",
    },
]

CHAINED_GATES: list[dict[str, Any]] = [
    {
        "id": "pass381_movement_viewport_walls_source_lock",
        "cmd": [sys.executable, "tools/verify_pass381_dm1_v1_movement_viewport_walls_source_lock.py", "--json"],
        "covers": "command queue -> movement/turn state -> viewport wall redraw and presentation source chain",
    },
    {
        "id": "pass402_movement_cooldown_order",
        "cmd": [sys.executable, "scripts/verify_pass402_dm1_v1_movement_cooldown_order.py"],
        "covers": "M11 live bridge preserves ReDMCSB cooldown aging before F0380 and no same-tick post-decrement",
    },
    {
        "id": "pass395_viewport_walls_source_runtime_lock",
        "cmd": [sys.executable, "scripts/verify_pass395_dm1_v1_viewport_walls_source_runtime_lock.py"],
        "covers": "wall replay, door two-pass, F0115 handoff, and post-command redraw metadata/runtime contract",
    },
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
            "source": "{}:{}".format(row["file"], row["range"]),
            "function": row["function"],
            "readyClaim": row["readyClaim"],
            "missing": missing,
        })
    return ok, results


def run_gate(entry: dict[str, Any], source: Path) -> dict[str, Any]:
    cmd = list(entry["cmd"])
    if entry["id"] == "pass381_movement_viewport_walls_source_lock":
        cmd.extend(["--source", str(source)])
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=60)
    return {
        "id": entry["id"],
        "passed": proc.returncode == 0,
        "cmd": cmd,
        "covers": entry["covers"],
        "returncode": proc.returncode,
        "outputTail": proc.stdout.splitlines()[-20:],
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    parser.add_argument("--skip-chained", action="store_true")
    parser.add_argument("--json", action="store_true")
    args = parser.parse_args()

    source_ok, source_results = verify_source_rows(args.source)
    chain_results: list[dict[str, Any]] = []
    chain_ok = True
    if not args.skip_chained:
        for entry in CHAINED_GATES:
            result = run_gate(entry, args.source)
            chain_ok = chain_ok and result["passed"]
            chain_results.append(result)

    ok = source_ok and chain_ok
    payload = {
        "gate": "dm1_v1_viewport_movement_completion_matrix",
        "sourceRoot": str(args.source),
        "passed": ok,
        "sourceRows": source_results,
        "chainedGates": chain_results,
    }
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        for row in source_results:
            print("{} source {} {} {}".format("PASS" if row["passed"] else "FAIL", row["id"], row["source"], row["function"]))
            print("  {}".format(row["readyClaim"]))
            for marker in row["missing"]:
                print("  missing/order: {}".format(marker))
        for row in chain_results:
            print("{} chained {} rc={}".format("PASS" if row["passed"] else "FAIL", row["id"], row["returncode"]))
            print("  {}".format(row["covers"]))
            if not row["passed"]:
                for line in row["outputTail"]:
                    print("  {}".format(line))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
