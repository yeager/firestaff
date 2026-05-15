#!/usr/bin/env python3
from pathlib import Path
import json
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
MANIFEST = ROOT / "parity-evidence/verification/pass560_dm1_v1_mirrored_door_front_source_lock/manifest.json"
REPORT = ROOT / "parity-evidence/pass560_dm1_v1_mirrored_door_front_source_lock.md"
STATUS = "PASS560_DM1_V1_MIRRORED_DOOR_FRONT_SOURCE_LOCKED"

SRC = [
    ("d3r-mirrored-door-front-split", "DUNVIEW.C", "6578-6602", [
        "case C17_ELEMENT_DOOR_FRONT:",
        "F0108_DUNGEONVIEW_DrawFloorOrnament(L0203_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M590_VIEW_FLOOR_D3R);",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0203_ai_SquareAspect[M550_FIRST_THING], P0150_i_Direction, P0151_i_MapX, P0152_i_MapY, M602_VIEW_SQUARE_D3R, C0x0128_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2122_, C720_ZONE_DOOR_FRAME_LEFT_D3R);",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2120_DoorFrameLeftD3L, C721_ZONE_DOOR_FRAME_RIGHT_D3R);",
        "F0110_DUNGEONVIEW_DrawDoorButton(M000_INDEX_TO_ORDINAL(C0_DOOR_BUTTON), C0_VIEW_DOOR_BUTTON_D3R);",
        "F0111_DUNGEONVIEW_DrawDoor(L0203_ai_SquareAspect[M557_DOOR_THING_INDEX], L0203_ai_SquareAspect[M556_DOOR_STATE], G0693_ai_DoorNativeBitmapIndex_Front_D3LCR, C0_VIEW_DOOR_ORNAMENT_D3LCR, M626_ZONE_DOOR_D3R);",
        "L0202_i_Order = C0x0439_CELL_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT;",
        "goto T0117018;",
    ]),
    ("d2l-door-front-split", "DUNVIEW.C", "6987-7004", [
        "case C17_ELEMENT_DOOR_FRONT:",
        "F0108_DUNGEONVIEW_DrawFloorOrnament(L0208_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M591_VIEW_FLOOR_D2L);",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0208_ai_SquareAspect[M550_FIRST_THING], P0156_i_Direction, P0157_i_MapX, P0158_i_MapY, M604_VIEW_SQUARE_D2L, C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2114_DoorFrameTopD2L, C729_ZONE_DOOR_FRAME_TOP_D2L);",
        "F0111_DUNGEONVIEW_DrawDoor(L0208_ai_SquareAspect[M557_DOOR_THING_INDEX], L0208_ai_SquareAspect[M556_DOOR_STATE], G0694_ai_DoorNativeBitmapIndex_Front_D2LCR, C1_VIEW_DOOR_ORNAMENT_D2LCR, M627_ZONE_DOOR_D2L);",
        "L0207_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;",
        "goto T0119020;",
    ]),
    ("d2r-mirrored-door-front-split", "DUNVIEW.C", "7180-7197", [
        "case C17_ELEMENT_DOOR_FRONT:",
        "F0108_DUNGEONVIEW_DrawFloorOrnament(L0210_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M593_VIEW_FLOOR_D2R);",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0210_ai_SquareAspect[M550_FIRST_THING], P0159_i_Direction, P0160_i_MapX, P0161_i_MapY, M605_VIEW_SQUARE_D2R, C0x0128_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2113_DoorFrameTopD2R, C731_ZONE_DOOR_FRAME_TOP_D2R);",
        "F0111_DUNGEONVIEW_DrawDoor(L0210_ai_SquareAspect[M557_DOOR_THING_INDEX], L0210_ai_SquareAspect[M556_DOOR_STATE], G0694_ai_DoorNativeBitmapIndex_Front_D2LCR, C1_VIEW_DOOR_ORNAMENT_D2LCR, M629_ZONE_DOOR_D2R);",
        "L0209_i_Order = C0x0439_CELL_ORDER_DOORPASS2_FRONTRIGHT_FRONTLEFT;",
        "goto T0120029;",
    ]),
]

LOCAL = [
    ("firestaff-mirrored-door-front-metadata", ROOT / "dm1_v1_viewport_3d_pc34_compat.c", "135-142", [
        "DM1_VIEW_SQUARE_D3R, 0x0128, 0x0439",
        "DUNVIEW.C:6579 floor ornament under mirrored rear pass",
        "DUNVIEW.C:6592-6593 optional button before door panel",
        "DM1_VIEW_SQUARE_D2L, 0x0218, 0x0349",
        "DUNVIEW.C:6988 floor ornament under rear pass",
        "DM1_VIEW_SQUARE_D2R, 0x0128, 0x0439",
        "DUNVIEW.C:7181 floor ornament under mirrored rear pass",
    ]),
    ("firestaff-mirrored-door-front-runtime-test", ROOT / "test_dm1_v1_viewport_3d_pc34_compat.c", "435-491", [
        "{ DM1_VIEW_SQUARE_D3R, \"6579\", \"6580\", \"6582\", \"6592\", \"6598\", \"6601\", 0x0128, 0x0439, {2, 1}, {3, 4} },",
        "{ DM1_VIEW_SQUARE_D2L, \"6988\", \"6989\", \"6991\", NULL,   \"7000\", \"7003\", 0x0218, 0x0349, {1, 2}, {4, 3} },",
        "{ DM1_VIEW_SQUARE_D2R, \"7181\", \"7182\", \"7184\", NULL,   \"7193\", \"7196\", 0x0128, 0x0439, {2, 1}, {3, 4} },",
        "check_int(\"door_front_occlusion.count\", (int)dm1_viewport_3d_door_front_occlusion_spec_count(), 7);",
        "rear.cells[0] == expected[i].rear_cells[0]",
        "front.cells[0] == expected[i].front_cells[0]",
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
        print("PASS pass560 check-only" if not failed else "FAIL pass560 check-only: " + ",".join(failed))
        return 0 if not failed else 1

    runtime = run([str(ROOT / "build-pass560" / "test_dm1_v1_viewport_3d_pc34_compat")])
    check_run = run([sys.executable, str(Path(__file__).resolve()), "--check-only"])
    ok = not failed and runtime["passed"] and check_run["passed"]
    manifest = {
        "schema": "pass560_dm1_v1_mirrored_door_front_source_lock.v1",
        "status": "passed" if ok else "failed",
        "statusToken": STATUS if ok else "FAILED_PASS560_DM1_V1_MIRRORED_DOOR_FRONT_SOURCE_LOCK",
        "redmcsbRoot": str(RED),
        "redmcsbChecks": red,
        "firestaffChecks": loc,
        "verificationRuns": [runtime, check_run],
        "nonClaims": [
            "No input or movement queue edits.",
            "No original DOS pixel parity claim.",
            "No DANNESBURK use.",
        ],
    }
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    REPORT.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass560 DM1 V1 mirrored door-front source lock",
        "",
        "Status: " + manifest["status"],
        "",
        "Claim: D3R, D2L, and D2R front-door branches use ReDMCSB's two-pass door-front order, including mirrored right-side cell orders for D3R/D2R.",
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
    lines += ["", "## Verification"]
    for row in manifest["verificationRuns"]:
        lines += ["", f"- {' '.join(row['command'])}: rc={row['returncode']}", "~~~", row["outputTail"], "~~~"]
    lines += ["", "## Non-Claims", "", "- No input or movement queue code was changed.", "- No original DOS pixel parity is claimed.", "- DANNESBURK was not used."]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(manifest["statusToken"])
    print("- wrote", MANIFEST.relative_to(ROOT))
    print("- wrote", REPORT.relative_to(ROOT))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main("--check-only" in sys.argv))
