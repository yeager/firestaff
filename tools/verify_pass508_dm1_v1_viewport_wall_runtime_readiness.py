#!/usr/bin/env python3
from __future__ import annotations
import hashlib, json, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
DM1 = Path("~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1").expanduser()
GREATSTONE = Path("~/.openclaw/data/firestaff-greatstone-atlas").expanduser()
CSBWIN = Path("~/.openclaw/data/firestaff-csbwin-source/CSBWin").expanduser()
OUT = ROOT / "parity-evidence/verification/pass508_dm1_v1_viewport_wall_runtime_readiness/manifest.json"
REPORT = ROOT / "parity-evidence/pass508_dm1_v1_viewport_wall_runtime_readiness.md"

SOURCE_CHECKS = [
    ("f0128_far_to_near_world_replay", "DUNVIEW.C", "8466-8542", "F0128_DUNGEONVIEW_Draw_CPSF", [
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(F0162_DUNGEON_GetSquareFirstObject(L0224_i_MapX, L0225_i_MapY), P0183_i_Direction, L0224_i_MapX, L0225_i_MapY, M598_VIEW_SQUARE_D4L",
        "F0116_DUNGEONVIEW_DrawSquareD3L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
        "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);"]),
    ("f0124_d1c_wall_alcove_exception", "DUNVIEW.C", "7784-7844", "F0124_DUNGEONVIEW_DrawSquareD1C", [
        "case C00_ELEMENT_WALL:",
        "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C04_WALL_D1C], C712_ZONE_WALL_D1C);",
        "if (F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0218_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M587_VIEW_WALL_D1C_FRONT))",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, C0x0000_CELL_ORDER_ALCOVE);"]),
    ("f0115_alcove_cell_filter", "DUNVIEW.C", "4800-4926", "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF", [
        "L0135_B_DrawAlcoveObjects = !(L0130_ul_RemainingViewCellOrdinalsToProcess = P0146_ui_OrderedViewCellOrdinals);",
        "AL0126_i_ViewCell = C04_VIEW_CELL_ALCOVE; /* Index of coordinates to draw objects in alcoves */",
        "L0139_i_Cell = M018_OPPOSITE(P0142_i_Direction); /* Alcove is on the opposite direction of the viewing direction */",
        "M011_CELL(P0141_T_Thing) == L0139_i_Cell",
        "L0142_B_UseAlcoveObjectImage = L0135_B_DrawAlcoveObjects && !L0138_i_ViewLane;"]),
    ("f0124_d1c_front_door_two_pass", "DUNVIEW.C", "7873-7938", "F0124_DUNGEONVIEW_DrawSquareD1C", [
        "case C17_ELEMENT_DOOR_FRONT:",
        "F0108_DUNGEONVIEW_DrawFloorOrnament",
        "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2112_DoorFrameTopD1LCR, C733_ZONE_DOOR_FRAME_TOP_D1C);",
        "F0110_DUNGEONVIEW_DrawDoorButton",
        "F0111_DUNGEONVIEW_DrawDoor",
        "L0217_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;",
        "T0124018:",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"]),
    ("drawview_present_boundary", "DRAWVIEW.C", "847-858", "F0097_DUNGEONVIEW_DrawViewport", [
        "F0638_GetZone(C007_ZONE_VIEWPORT, L2414_ai_XYZ);",
        "M768_BOX_LEFT(L2413_ai_Box) = M704_ZONE_LEFT(L2414_ai_XYZ);",
        "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);"]),
]
FIRE_CHECKS = [
    ("normal_renderer_batches_with_near_replay_guard", "m11_game_view.c", [
        "m11_draw_viewport_background(state", "m11_draw_dm1_floor_pits(state",
        "m11_draw_dm1_side_walls(state", "m11_draw_dm1_front_walls(state",
        "m11_draw_dm1_wall_ornaments(state", "m11_draw_dm1_center_doors(state",
        "m11_dm1_nearest_blocking_center_depth_index(cells)",
        "m11_draw_dm1_side_contents(state", "m11_draw_dm1_deferred_explosion_pass(state",
        "if (state->showDebugHUD)"]),
    ("wall_alcove_item_source_cell_gate", "m11_game_view.c", [
        "static void m11_draw_dm1_alcove_wall_items", "C0x0000_CELL_ORDER_ALCOVE",
        "C04_VIEW_CELL_ALCOVE", "M018_OPPOSITE(direction)",
        "cell->floorItemCells[ii] != alcoveCellRelativeToParty", "m11_draw_item_sprite(state"]),
]

def sha(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for b in iter(lambda: f.read(1048576), b""):
            h.update(b)
    return h.hexdigest()

def line_no(text: str, pos: int) -> int:
    return text.count("\n", 0, pos) + 1

def excerpt(text: str, spec: str) -> tuple[int, str]:
    a, b = [int(x) for x in spec.split("-", 1)]
    return a, "\n".join(text.splitlines()[a - 1:b])

def ordered(text: str, needles: list[str], line_base: int = 1):
    cursor = 0
    hits, missing = [], []
    for n in needles:
        p = text.find(n, cursor)
        if p < 0:
            missing.append(n)
        else:
            hits.append({"marker": n, "line": line_base + line_no(text, p) - 1})
            cursor = p + len(n)
    return hits, missing

def run(args: list[str]):
    p = subprocess.run(args, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"command": args, "returncode": p.returncode, "passed": p.returncode == 0, "outputTail": "\n".join(p.stdout.strip().splitlines()[-8:])}

def main() -> int:
    source = []
    for ident, name, lines, func, needles in SOURCE_CHECKS:
        path = RED / name
        text = path.read_text(encoding="latin-1", errors="replace")
        base, part = excerpt(text, lines)
        hits, missing = ordered(part, needles, base)
        source.append({"id": ident, "status": "PASS" if not missing else "FAIL", "sourceFile": name, "function": func, "lines": lines, "sha256": sha(path), "positions": hits, "missing": missing})
    fire = []
    for ident, name, needles in FIRE_CHECKS:
        path = ROOT / name
        text = path.read_text(encoding="utf-8", errors="replace")
        hits, missing = ordered(text, needles)
        fire.append({"id": ident, "status": "PASS" if not missing else "FAIL", "file": name, "positions": hits, "missing": missing})
    refs = []
    for ident, path, use in [
        ("dm1_pc34_graphics", DM1 / "GRAPHICS.DAT", "PC34 bitmap source"),
        ("dm1_pc34_dungeon", DM1 / "DUNGEON.DAT", "PC34 route/object source"),
        ("greatstone_index", GREATSTONE / "index/SUMMARY.md", "secondary local atlas index"),
        ("csbwin_cpp", CSBWIN / "CSBwin.cpp", "secondary lineage reference only"),
    ]:
        refs.append({"id": ident, "path": str(path), "exists": path.exists(), "sha256": sha(path) if path.is_file() else None, "use": use})
    gates = [
        run([sys.executable, "tools/verify_v1_viewport_alcove_wall_item_gate.py"]),
        run([sys.executable, "tools/verify_v1_viewport_d1c_doorpass_source_lock_gate.py"]),
        run([sys.executable, "tools/verify_pass500_dm1_v1_viewport_walls_blocker_cleanup_source_lock.py"]),
    ]
    failures = [x["id"] for x in source + fire if x["status"] != "PASS"]
    failures += ["secondary_" + x["id"] for x in refs if not x["exists"]]
    failures += ["gate_" + Path(x["command"][-1]).name for x in gates if not x["passed"]]
    status = "PASS_PASS508_DM1_V1_VIEWPORT_WALL_RUNTIME_READINESS" if not failures else "FAIL_PASS508_DM1_V1_VIEWPORT_WALL_RUNTIME_READINESS"
    manifest = {"schema": "pass508_dm1_v1_viewport_wall_runtime_readiness.v1", "status": status, "redmcsbSourceRoot": str(RED), "redmcsbSourceAudit": source, "firestaffAudit": fire, "secondaryReferences": refs, "gates": gates, "failures": failures, "blockers": ["No original DOSBox same-viewport capture was produced by this pass.", "No Firestaff-vs-original pixel comparator promotion is claimed.", "Firestaff normal V1 renderer remains guarded batched replay, not exact ReDMCSB F0128 per-square replay."]}
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = ["# Pass508 DM1 V1 viewport/wall runtime-readiness evidence", "", "Status: " + status, "", "## ReDMCSB anchors"]
    lines += [f"- {x['sourceFile']}:{x['lines']} {x['function']} status={x['status']}" for x in source]
    lines += ["", "## Firestaff readiness"]
    lines += [f"- {x['file']} {x['id']} status={x['status']}" for x in fire]
    lines += ["", "## Secondary local references"]
    lines += [f"- {x['id']} {x['path']} exists={x['exists']} sha256={x['sha256']}" for x in refs]
    lines += ["", "## Gates"]
    lines += [f"- {' '.join(x['command'])} -> rc={x['returncode']} passed={x['passed']}" for x in gates]
    lines += ["", "## Blockers"] + [f"- {x}" for x in manifest["blockers"]] + [""]
    REPORT.write_text("\n".join(lines), encoding="utf-8")
    print(status)
    print(f"- wrote {REPORT.relative_to(ROOT)}")
    print(f"- wrote {OUT.relative_to(ROOT)}")
    for f in failures:
        print("- " + f)
    return 0 if not failures else 1

if __name__ == "__main__":
    raise SystemExit(main())
