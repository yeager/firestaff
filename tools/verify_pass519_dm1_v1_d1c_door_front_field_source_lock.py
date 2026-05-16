#!/usr/bin/env python3
from pathlib import Path
import json, subprocess, sys

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
MANIFEST = ROOT / "parity-evidence/verification/pass519_dm1_v1_d1c_door_front_field_source_lock/manifest.json"
REPORT = ROOT / "parity-evidence/pass519_dm1_v1_d1c_door_front_field_source_lock.md"
STATUS = "PASS519_DM1_V1_D1C_DOOR_FRONT_FIELD_SOURCE_LOCKED"

SRC = [
    ("d1c-door-front-split", "DUNVIEW.C", "7873-7911", [
        "case C17_ELEMENT_DOOR_FRONT:",
        "F0108_DUNGEONVIEW_DrawFloorOrnament(L0218_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M595_VIEW_FLOOR_D1C);",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2112_DoorFrameTopD1LCR, C733_ZONE_DOOR_FRAME_TOP_D1C);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2117_DoorFrameLeftD1C, C726_ZONE_DOOR_FRAME_LEFT_D1C);",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2117_DoorFrameLeftD1C, C727_ZONE_DOOR_FRAME_RIGHT_D1C);",
        "F0110_DUNGEONVIEW_DrawDoorButton(M000_INDEX_TO_ORDINAL(C0_DOOR_BUTTON), C3_VIEW_DOOR_BUTTON_D1C);",
        "F0111_DUNGEONVIEW_DrawDoor(L0218_ai_SquareAspect[M557_DOOR_THING_INDEX], L0218_ai_SquareAspect[M556_DOOR_STATE], G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, C2_VIEW_DOOR_ORNAMENT_D1LCR, M631_ZONE_DOOR_D1C);",
        "L0217_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;",
        "goto T0124018;",
    ]),
    ("d1c-front-cells-then-field", "DUNVIEW.C", "7936-7955", [
        "T0124018:",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0218_ai_SquareAspect[M550_FIRST_THING], P0171_i_Direction, P0172_i_MapX, P0173_i_MapY, M606_VIEW_SQUARE_D1C, L0217_i_Order);",
        "if (L0218_ai_SquareAspect[C0_ELEMENT] == C05_ELEMENT_TELEPORTER) {",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M606_VIEW_SQUARE_D1C]], C712_ZONE_WALL_D1C);",
    ]),
    ("f0111-door-ornament-before-final-blit", "DUNVIEW.C", "4255-4334", [
        "L0116_ps_Door = (DOOR*)(G0284_apuc_ThingData[C00_THING_TYPE_DOOR]) + P0124_ui_DoorThingIndex;",
        "F0109_DUNGEONVIEW_DrawDoorOrnament(L0116_ps_Door->OrnamentOrdinal, P0128_i_ViewDoorOrnamentIndex);",
        "F0109_DUNGEONVIEW_DrawDoorOrnament(M000_INDEX_TO_ORDINAL(C16_DOOR_ORNAMENT_THIEVES_EYE_MASK), C2_VIEW_DOOR_ORNAMENT_D1LCR);",
        "if (P0125_ui_DoorState == C5_DOOR_STATE_DESTROYED) {",
        "F0109_DUNGEONVIEW_DrawDoorOrnament(M000_INDEX_TO_ORDINAL(C15_DOOR_ORNAMENT_DESTROYED_MASK), P0128_i_ViewDoorOrnamentIndex);",
        "F0791_DUNGEONVIEW_DrawBitmapXX(G0074_puc_Bitmap_Temporary, G0296_puc_Bitmap_Viewport, P2084_i_ZoneIndex, AL0114_ui_Flip, C10_COLOR_FLESH);",
    ]),
]

LOCAL = [
    ("firestaff-d1c-door-front-metadata", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "136-156", [
        "DM1_VIEW_SQUARE_D1C, 0x0218, 0x0349",
        "DUNVIEW.C:7874-7875 pass1 rear cells before frame",
        "DUNVIEW.C:7877-7902 top/side frame and button draw",
        "DUNVIEW.C:7905-7908 F0111 door bitmap/ornament",
        "DUNVIEW.C:7910-7937 pass2 front cells after door",
    ]),
    ("firestaff-d1c-runtime-test", ROOT / "tests/test_dm1_v1_viewport_3d_pc34_compat.c", "579-615", [
        "DM1_VIEW_SQUARE_D1C, \"7874\", \"7875\", \"7877\", \"7901\", \"7905\", \"7937\"",
        "check_int(\"door_front_occlusion.rear_order\", spec->rear_cell_order, expected[i].rear_order);",
        "check_int(\"door_front_occlusion.front_order\", spec->front_cell_order, expected[i].front_order);",
        "rear.cells[0] == expected[i].rear_cells[0]",
        "front.cells[0] == expected[i].front_cells[0]",
    ]),
]

def body(path, spec):
    a, b = [int(x) for x in spec.split("-")]
    text = path.read_text(encoding="latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8", errors="replace")
    return a, "\n".join(text.splitlines()[a - 1:b])

def ordered(base, text, needles):
    cursor, hits, missing = 0, [], []
    for needle in needles:
        pos = text.find(needle, cursor)
        if pos < 0:
            missing.append(needle)
        else:
            hits.append({"line": base + text.count("\n", 0, pos), "needle": needle})
            cursor = pos + len(needle)
    return hits, missing

def audit_source():
    rows = []
    for ident, name, span, needles in SRC:
        base, text = body(RED / name, span)
        hits, missing = ordered(base, text, needles)
        rows.append({"id": ident, "status": "PASS" if not missing else "FAIL", "sourceFile": name, "lines": span, "hits": hits, "missing": missing})
    return rows

def audit_local():
    rows = []
    for ident, path, span, needles in LOCAL:
        base, text = body(path, span)
        hits, missing = ordered(base, text, needles)
        rows.append({"id": ident, "status": "PASS" if not missing else "FAIL", "sourceFile": path.name, "lines": span, "hits": hits, "missing": missing})
    return rows

def run(cmd):
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"command": cmd, "returncode": proc.returncode, "passed": proc.returncode == 0, "outputTail": "\n".join(proc.stdout.strip().splitlines()[-12:])}

def main(check=False):
    red, loc = audit_source(), audit_local()
    failed = [row["id"] for row in red + loc if row["status"] != "PASS"]
    if check:
        print("PASS pass519 check-only" if not failed else "FAIL pass519 check-only: " + ",".join(failed))
        return 0 if not failed else 1
    runtime = run([str(ROOT / "build" / "test_dm1_v1_viewport_3d_pc34_compat")])
    check_run = run([sys.executable, str(Path(__file__).resolve()), "--check-only"])
    ok = not failed and runtime["passed"] and check_run["passed"]
    manifest = {"schema": "pass519_dm1_v1_d1c_door_front_field_source_lock.v1", "status": "passed" if ok else "failed", "statusToken": STATUS if ok else "FAILED_PASS519_DM1_V1_D1C_DOOR_FRONT_FIELD_SOURCE_LOCK", "redmcsbRoot": str(RED), "redmcsbChecks": red, "firestaffChecks": loc, "verificationRuns": [runtime, check_run], "nonClaims": ["No input or movement queue edits.", "No renderer runtime behavior change.", "No original DOS pixel parity claim.", "No DANNESBURK use."]}
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = ["# Pass519 DM1 V1 D1C door-front field source lock", "", "Status: " + manifest["status"], "", "Claim: D1C door-front renders rear cells before frame/button/door, composes door ornaments/masks before final door blit, draws front cells after the door, then leaves teleporter field as a final overlay after F0115.", "", "## Primary ReDMCSB Evidence"]
    for row in red:
        lines += ["", f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"]
        lines += [f"  - line {hit['line']}: {hit['needle']}" for hit in row["hits"]]
        lines += [f"  - missing: {miss}" for miss in row["missing"]]
    lines += ["", "## Firestaff Evidence"]
    for row in loc:
        lines += ["", f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"]
    lines += ["", "## Verification"]
    for row in manifest["verificationRuns"]:
        lines += ["", f"- {' '.join(row['command'])}: rc={row['returncode']}", "~~~", row["outputTail"], "~~~"]
    lines += ["", "## Non-Claims", "", "- No input or movement queue code was changed.", "- No renderer runtime behavior was changed.", "- No original DOS pixel parity is claimed.", "- DANNESBURK was not used."]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(manifest["statusToken"])
    print("- wrote", MANIFEST.relative_to(ROOT))
    print("- wrote", REPORT.relative_to(ROOT))
    return 0 if ok else 1

if __name__ == "__main__":
    raise SystemExit(main("--check-only" in sys.argv))
