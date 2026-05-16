#!/usr/bin/env python3
from pathlib import Path
import json
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
MANIFEST = ROOT / "parity-evidence/verification/pass570_dm1_v1_d2c_front_order_source_lock/manifest.json"
REPORT = ROOT / "parity-evidence/pass570_dm1_v1_d2c_front_order_source_lock.md"
STATUS = "PASS570_DM1_V1_D2C_FRONT_ORDER_SOURCE_LOCKED"
TEST_BINARY = ROOT / "build" / "test_dm1_v1_viewport_3d_pc34_compat"

SRC = [
    ("f0128-d2c-position", "DUNVIEW.C", "8510-8522", [
        "F0119_DUNGEONVIEW_DrawSquareD2L",
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
        "F0121_DUNGEONVIEW_DrawSquareD2C",
    ]),
    ("d2c-front-wall-alcove-return", "DUNVIEW.C", "7289-7312", [
        "case C00_ELEMENT_WALL:",
        "F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C09_WALL_D2C], C709_ZONE_WALL_D2C",
        "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF(L0212_ai_SquareAspect[M552_FRONT_WALL_ORNAMENT_ORDINAL], M583_VIEW_WALL_D2C_FRONT)",
        "L0211_i_Order = C0x0000_CELL_ORDER_ALCOVE;",
        "goto T0121016;",
        "return;",
    ]),
    ("d2c-door-front-two-pass-order", "DUNVIEW.C", "7313-7342", [
        "case C17_ELEMENT_DOOR_FRONT:",
        "F0108_DUNGEONVIEW_DrawFloorOrnament(L0212_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M592_VIEW_FLOOR_D2C);",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0212_ai_SquareAspect[M550_FIRST_THING], P0162_i_Direction, P0163_i_MapX, P0164_i_MapY, M603_VIEW_SQUARE_D2C, C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2115_DoorFrameTopD2LCR, C730_ZONE_DOOR_FRAME_TOP_D2C);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2118_DoorFrameLeftD2C, C724_ZONE_DOOR_FRAME_LEFT_D2C);",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2118_DoorFrameLeftD2C, C725_ZONE_DOOR_FRAME_RIGHT_D2C);",
        "F0110_DUNGEONVIEW_DrawDoorButton(M000_INDEX_TO_ORDINAL(C0_DOOR_BUTTON), C2_VIEW_DOOR_BUTTON_D2C);",
        "F0111_DUNGEONVIEW_DrawDoor(L0212_ai_SquareAspect[M557_DOOR_THING_INDEX], L0212_ai_SquareAspect[M556_DOOR_STATE], G0694_ai_DoorNativeBitmapIndex_Front_D2LCR, C1_VIEW_DOOR_ORNAMENT_D2LCR, M628_ZONE_DOOR_D2C);",
        "L0211_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;",
        "goto T0121016;",
    ]),
    ("d2c-open-pit-teleporter-tail", "DUNVIEW.C", "7353-7388", [
        "case C05_ELEMENT_TELEPORTER:",
        "case C01_ELEMENT_CORRIDOR:",
        "L0211_i_Order = C0x3421_CELL_ORDER_BACKLEFT_BACKRIGHT_FRONTLEFT_FRONTRIGHT;",
        "F0108_DUNGEONVIEW_DrawFloorOrnament(L0212_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M592_VIEW_FLOOR_D2C);",
        "F0112_DUNGEONVIEW_DrawCeilingPit(C065_GRAPHIC_CEILING_PIT_D2C, C865_ZONE_CEILING_PIT_D2C",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0212_ai_SquareAspect[M550_FIRST_THING], P0162_i_Direction, P0163_i_MapX, P0164_i_MapY, M603_VIEW_SQUARE_D2C, L0211_i_Order);",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M603_VIEW_SQUARE_D2C]], C709_ZONE_WALL_D2C);",
    ]),
    ("pc34-i34e-d2c-zones", "DEFS.H", "4049-4088", [
        "#define C709_ZONE_WALL_D2C",
        "#define C724_ZONE_DOOR_FRAME_LEFT_D2C",
        "#define C725_ZONE_DOOR_FRAME_RIGHT_D2C",
        "#define C730_ZONE_DOOR_FRAME_TOP_D2C",
    ]),
]

LOCAL = [
    ("firestaff-d2c-door-front-metadata", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "135-145", [
        "DM1_VIEW_SQUARE_D2C, 0x0218, 0x0349",
        "DUNVIEW.C:7314 floor ornament under rear pass",
        "DUNVIEW.C:7315 pass1 rear cells before frame",
        "DUNVIEW.C:7317-7333 top/side frame and button draw",
        "DUNVIEW.C:7332-7334 optional button before door panel",
        "DUNVIEW.C:7339 F0111 door bitmap/ornament",
        "DUNVIEW.C:7341 pass2 front cells after door",
    ]),
    ("firestaff-d2c-floor-field-metadata", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "235-242", [
        "DM1_VIEW_SQUARE_D2C, 0x3421",
        "DUNVIEW.C:7260-7288 stairs front bitmap before common floor/thing path",
        "DUNVIEW.C:7343-7353 pit bitmap before floor ornament",
        "DUNVIEW.C:7355-7357 order then F0108 floor ornament",
        "DUNVIEW.C:7367-7368 F0115 object/creature/projectile/explosion handoff",
        "DUNVIEW.C:7370-7388 teleporter field after F0115",
        "DUNVIEW.C:7289-7312 wall bitmap/ornament then return unless front alcove branches to F0115",
    ]),
    ("firestaff-d2c-wall-metadata", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "285-296", [
        "DM1_VIEW_SQUARE_D2C,  DM1_WALL_D2C,  DM1_WALL_D2C",
        "DM1_PC34_ZONE_WALL_D2C",
        "DUNVIEW.C:7299-7306",
        "DUNVIEW.C:7308-7312 front alcove branches to F0115, else return",
    ]),
    ("firestaff-d2c-zone-defines", ROOT / "include/dm1_v1_viewport_3d_pc34_compat.h", "392-415", [
        "#define DM1_PC34_ZONE_WALL_D2C",
        "#define DM1_PC34_ZONE_DOOR_FRAME_LEFT_D2C   724",
        "#define DM1_PC34_ZONE_DOOR_FRAME_RIGHT_D2C  725",
        "#define DM1_PC34_ZONE_DOOR_FRAME_TOP_D2C    730",
    ]),
    ("firestaff-d2c-runtime-test", ROOT / "tests/test_dm1_v1_viewport_3d_pc34_compat.c", "564-631", [
        "{ DM1_VIEW_SQUARE_D2C, \"7314\", \"7315\", \"7317\", \"7332\", \"7339\", \"7341\", 0x0218, 0x0349, {1, 2}, {4, 3} },",
        "check_int(\"door_front_occlusion.count\", (int)dm1_viewport_3d_door_front_occlusion_spec_count(), 11);",
        "rear.cells[0] == expected[i].rear_cells[0]",
        "front.cells[0] == expected[i].front_cells[0]",
    ]),
    ("firestaff-d2c-source-evidence", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "1105-1123", [
        "DUNVIEW.C:7260-7388 D2C stairs/pit/floor-ornament/ceiling-pit/F0115/teleporter-field order; wall returns before F0115",
        "DUNVIEW.C:7314-7341 D2C door-front occlusion: rear pass, frame/door, front pass",
        "DEFS.H:4082-4088 PC34/I34E D2C door-frame zones 724/725/730",
        "DUNVIEW.C:7289-7312 D2C front wall: wall zone, front ornament/alcove exception, else return before open-cell draw",
        "DUNVIEW.C:7353-7387 D2C open/pit/teleporter order: 0x3421 floor/ceiling/F0115, then field overlay",
    ]),
]


def body(path, span):
    a, b = [int(x) for x in span.split("-")]
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    text = path.read_text(encoding=encoding, errors="replace")
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


def audit(rows):
    out = []
    for ident, name, span, needles in rows:
        path = RED / name if isinstance(name, str) else name
        base, text = body(path, span)
        hits, missing = ordered(base, text, needles)
        out.append({
            "id": ident,
            "status": "PASS" if not missing else "FAIL",
            "sourceFile": path.name,
            "lines": span,
            "hits": hits,
            "missing": missing,
        })
    return out


def run(cmd):
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {
        "command": cmd,
        "returncode": proc.returncode,
        "passed": proc.returncode == 0,
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-14:]),
    }


def main(check=False):
    red, loc = audit(SRC), audit(LOCAL)
    failed = [row["id"] for row in red + loc if row["status"] != "PASS"]
    if check:
        print("PASS pass570 check-only" if not failed else "FAIL pass570 check-only: " + ",".join(failed))
        return 0 if not failed else 1

    runtime = run([str(TEST_BINARY)])
    check_run = run([sys.executable, str(Path(__file__).resolve()), "--check-only"])
    ok = not failed and runtime["passed"] and check_run["passed"]
    manifest = {
        "schema": "pass570_dm1_v1_d2c_front_order_source_lock.v1",
        "status": "passed" if ok else "failed",
        "statusToken": STATUS if ok else "FAILED_PASS570_DM1_V1_D2C_FRONT_ORDER_SOURCE_LOCK",
        "redmcsbRoot": str(RED),
        "redmcsbChecks": red,
        "firestaffChecks": loc,
        "verificationRuns": [runtime, check_run],
        "nonClaims": [
            "No renderer runtime behavior change.",
            "No input, movement queue, or capture code changed.",
            "No original DOS pixel parity claim.",
            "No CSB, DM2, or DANNESBURK use.",
        ],
    }
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass570 DM1 V1 D2C front-order source lock",
        "",
        "Status: " + manifest["status"],
        "",
        "Claim: ReDMCSB D2C is drawn after D2L/D2R and before D1. Its front wall returns unless the front ornament is an alcove, its front door uses rear-cell pass, frame/button/door, then front-cell pass, and its open/pit/teleporter tail draws floor/ceiling/F0115 before the teleporter field overlay.",
        "",
        "## Primary ReDMCSB Evidence",
    ]
    for row in red:
        lines += ["", f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"]
        lines += [f"  - line {hit['line']}: {hit['needle']}" for hit in row["hits"]]
        lines += [f"  - missing: {miss}" for miss in row["missing"]]
    lines += ["", "## Firestaff Evidence"]
    for row in loc:
        lines += ["", f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"]
        lines += [f"  - line {hit['line']}: {hit['needle']}" for hit in row["hits"]]
        lines += [f"  - missing: {miss}" for miss in row["missing"]]
    lines += ["", "## Verification"]
    for row in manifest["verificationRuns"]:
        lines += ["", f"- {' '.join(row['command'])}: rc={row['returncode']}", "~~~", row["outputTail"], "~~~"]
    lines += ["", "## Non-Claims", "", "- No renderer runtime behavior was changed.", "- No original DOS pixel parity is claimed.", "- No movement/input/capture behavior is changed.", "- DANNESBURK was not used."]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(manifest["statusToken"])
    print("- wrote", MANIFEST.relative_to(ROOT))
    print("- wrote", REPORT.relative_to(ROOT))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main("--check-only" in sys.argv))
