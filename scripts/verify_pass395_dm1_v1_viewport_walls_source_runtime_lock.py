#!/usr/bin/env python3
"""Verify pass395 DM1 V1 viewport wall source/runtime lock.

This is a source+runtime metadata probe: it checks ReDMCSB first for the wall
replay, occlusion, door two-pass, F0115 object/creature/projectile handoff, and
the command->next-redraw path; then it checks Firestaff's pc34 compat metadata
and C runtime test expose those same contracts.
"""
from __future__ import annotations

import argparse
import json
import subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOURCE = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
LOCAL_C = ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c"
LOCAL_H = ROOT / "include/dm1_v1_viewport_3d_pc34_compat.h"
LOCAL_TEST = ROOT / "tests/test_dm1_v1_viewport_3d_pc34_compat.c"

CHECKS: list[dict[str, Any]] = [
    {
        "id": "post-command-next-redraw-sequence",
        "files": [
            {"file": "COMMAND.C", "range": "2045-2156"},
            {"file": "GAMELOOP.C", "range": "55-90"},
            {"file": "DRAWVIEW.C", "range": "709-722"},
        ],
        "ordered": [
            "void F0380_COMMAND_ProcessQueue_CPSC(",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G2153_i_QueuedCommandsCount--;",
            "if ((L1160_i_Command == C002_COMMAND_TURN_RIGHT) || (L1160_i_Command == C001_COMMAND_TURN_LEFT)) {",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)) {",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
            "for (;;) { /*_Infinite loop_*/",
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "M526_WaitVerticalBlank();",
        ],
        "why": "An eligible queued command mutates turn/move state; the next GAMELOOP iteration redraws F0128 from the updated party tuple and waits for viewport presentation.",
    },
    {
        "id": "far-to-near-wall-square-replay-and-present",
        "file": "DUNVIEW.C",
        "range": "8318-8610",
        "ordered": [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0116_DUNGEONVIEW_DrawSquareD3L",
            "F0117_DUNGEONVIEW_DrawSquareD3R",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
            "F0119_DUNGEONVIEW_DrawSquareD2L",
            "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
            "F0121_DUNGEONVIEW_DrawSquareD2C",
            "F0122_DUNGEONVIEW_DrawSquareD1L",
            "F0123_DUNGEONVIEW_DrawSquareD1R",
            "F0124_DUNGEONVIEW_DrawSquareD1C",
            "F0125_DUNGEONVIEW_DrawSquareD0L",
            "F0126_DUNGEONVIEW_DrawSquareD0R",
            "F0127_DUNGEONVIEW_DrawSquareD0C",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "why": "Wall squares are replayed in source order far-to-near; near calls occlude prior pixels before final present.",
    },
    {
        "id": "wall-square-occlusion-and-alcove-exception",
        "file": "DUNVIEW.C",
        "range": "6421-7843",
        "contains": [
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L);",
            "L0200_i_Order = C0x0000_CELL_ORDER_ALCOVE;",
            "return;",
            "F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C04_WALL_D1C], C712_ZONE_WALL_D1C, G0076_B_UseFlippedWallAndFootprintsBitmaps);",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING]",
        ],
        "why": "Wall case normally returns after wall/ornament draw; front alcoves intentionally hand contained things to F0115.",
    },
    {
        "id": "door-front-two-pass-then-front-handoff",
        "file": "DUNVIEW.C",
        "range": "6440-6816",
        "contains": [
            "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
            "F0111_DUNGEONVIEW_DrawDoor(L0201_ai_SquareAspect[M557_DOOR_THING_INDEX]",
            "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
            "C0x0128_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT",
            "C0x0439_CELL_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT",
        ],
        "why": "Door-front cells draw rear contents, then door/frame, then front-side contents in a second F0115 pass.",
    },
    {
        "id": "f0115-object-creature-projectile-explosion-handoff",
        "file": "DUNVIEW.C",
        "range": "4547-5938",
        "ordered": [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(",
            "P0146_ui_OrderedViewCellOrdinals >>= 4;",
            "if ((AL0127_i_ThingType = M012_TYPE(P0141_T_Thing)) == C04_THING_TYPE_GROUP)",
            "if (AL0127_i_ThingType == C14_THING_TYPE_PROJECTILE)",
            "P0141_T_Thing = L0146_T_FirstThingToDraw; /* Restart processing list of things from the beginning.",
            "if (M012_TYPE(P0141_T_Thing) == C15_THING_TYPE_EXPLOSION)",
        ],
        "why": "F0115 is the source handoff for object/creature/projectile/explosion layers after each visible square decides its cell order.",
    },
]

LOCAL_NEEDLES = [
    (LOCAL_C, "s_wall_draw_specs"),
    (LOCAL_C, "front alcove branches to F0115"),
    (LOCAL_C, "s_thing_layers"),
    (LOCAL_C, "s_post_command_redraw"),
    (LOCAL_C, "COMMAND.C:2045-2156"),
    (LOCAL_C, "GAMELOOP.C:55-90"),
    (LOCAL_H, "DM1_ViewportPostCommandRedrawSpec"),
    (LOCAL_TEST, "test_post_command_redraw_contract"),
]


def read_range(source: Path, spec: dict[str, str]) -> tuple[str, str]:
    p = source / spec["file"]
    a, b = map(int, spec["range"].split("-"))
    lines = p.read_text(errors="replace").splitlines()
    return f"{spec['file']}:{spec['range']}", "\n".join(lines[a - 1 : b])


def ordered_missing(text: str, needles: list[str]) -> list[str]:
    pos = -1
    missing: list[str] = []
    for needle in needles:
        i = text.find(needle, pos + 1)
        if i == -1:
            missing.append(needle)
        else:
            pos = i
    return missing


def run(cmd: list[str]) -> dict[str, Any]:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
    return {"cmd": cmd, "returncode": proc.returncode, "passed": proc.returncode == 0, "output_tail": proc.stdout.splitlines()[-20:]}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    ap.add_argument("--run-runtime", action="store_true", help="also run built/source runtime probe if available")
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    results = []
    ok = True
    for check in CHECKS:
        specs = check.get("files") or [{"file": check["file"], "range": check["range"]}]
        slices = [read_range(args.source, s) for s in specs]
        combined = "\n".join(t for _, t in slices)
        missing: list[str] = []
        if "ordered" in check:
            missing.extend(ordered_missing(combined, check["ordered"]))
        if "contains" in check:
            missing.extend(n for n in check["contains"] if n not in combined)
        passed = not missing
        ok = ok and passed
        results.append({"id": check["id"], "passed": passed, "source": [name for name, _ in slices], "missing": missing, "why": check["why"]})

    for path, needle in LOCAL_NEEDLES:
        text = path.read_text(errors="replace")
        passed = needle in text
        ok = ok and passed
        results.append({"id": f"local:{path.name}:{needle}", "passed": passed, "source": [str(path.relative_to(ROOT))], "missing": [] if passed else [needle], "why": "Firestaff metadata/runtime probe exposes this source-locked contract."})

    runtime = None
    if args.run_runtime:
        runtime = run([str(ROOT / "build" / "test_dm1_v1_viewport_3d_pc34_compat")])
        ok = ok and runtime["passed"]

    payload = {"gate": "pass395_dm1_v1_viewport_walls_source_runtime_lock", "source_root": str(args.source), "passed": ok, "checks": results, "runtime": runtime}
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        for r in results:
            print(("PASS" if r["passed"] else "FAIL"), r["id"], "; ".join(r["source"]))
            print(" ", r["why"])
            for m in r["missing"]:
                print("  missing/order:", m)
        if runtime:
            print(("PASS" if runtime["passed"] else "FAIL"), "runtime", " ".join(runtime["cmd"]))
            for line in runtime["output_tail"]:
                print(" ", line)
    return 0 if ok else 1

if __name__ == "__main__":
    raise SystemExit(main())
