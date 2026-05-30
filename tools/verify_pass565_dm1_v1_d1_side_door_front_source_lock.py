#!/usr/bin/env python3
from pathlib import Path
import json
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
MANIFEST = ROOT / "parity-evidence/verification/pass565_dm1_v1_d1_side_door_front_source_lock/manifest.json"
REPORT = ROOT / "parity-evidence/pass565_dm1_v1_d1_side_door_front_source_lock.md"
STATUS = "PASS565_DM1_V1_D1_SIDE_DOOR_FRONT_SOURCE_LOCKED"
TEST_BINARY = ROOT / "build" / "test_dm1_v1_viewport_3d_pc34_compat"

SRC = [
    ("d1l-door-front-split", "DUNVIEW.C", "7492-7536", [
        "case C17_ELEMENT_DOOR_FRONT:",
        "F0108_DUNGEONVIEW_DrawFloorOrnament(L0214_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M594_VIEW_FLOOR_D1L);",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0214_ai_SquareAspect[M550_FIRST_THING], P0165_i_Direction, P0166_i_MapX, P0167_i_MapY, M607_VIEW_SQUARE_D1L, C0x0028_CELL_ORDER_DOORPASS1_BACKRIGHT);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2111_DoorFrameTopD1L, C732_ZONE_DOOR_FRAME_TOP_D1L);",
        "F0111_DUNGEONVIEW_DrawDoor(L0214_ai_SquareAspect[M557_DOOR_THING_INDEX], L0214_ai_SquareAspect[M556_DOOR_STATE], G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, C2_VIEW_DOOR_ORNAMENT_D1LCR, M630_ZONE_DOOR_D1L);",
        "L0213_i_Order = C0x0039_CELL_ORDER_DOORPASS2_FRONTRIGHT;",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0214_ai_SquareAspect[M550_FIRST_THING], P0165_i_Direction, P0166_i_MapX, P0167_i_MapY, M607_VIEW_SQUARE_D1L, L0213_i_Order);",
    ]),
    ("d1r-door-front-split", "DUNVIEW.C", "7660-7704", [
        "case C17_ELEMENT_DOOR_FRONT:",
        "F0108_DUNGEONVIEW_DrawFloorOrnament(L0216_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M596_VIEW_FLOOR_D1R);",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0216_ai_SquareAspect[M550_FIRST_THING], P0168_i_Direction, P0169_i_MapX, P0170_i_MapY, M608_VIEW_SQUARE_D1R, C0x0018_CELL_ORDER_DOORPASS1_BACKLEFT);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2110_DoorFrameTopD1R, C734_ZONE_DOOR_FRAME_TOP_D1R);",
        "F0111_DUNGEONVIEW_DrawDoor(L0216_ai_SquareAspect[M557_DOOR_THING_INDEX], L0216_ai_SquareAspect[M556_DOOR_STATE], G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, C2_VIEW_DOOR_ORNAMENT_D1LCR, M632_ZONE_DOOR_D1R);",
        "L0215_i_Order = C0x0049_CELL_ORDER_DOORPASS2_FRONTLEFT;",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0216_ai_SquareAspect[M550_FIRST_THING], P0168_i_Direction, P0169_i_MapX, P0170_i_MapY, M608_VIEW_SQUARE_D1R, L0215_i_Order);",
    ]),
]
LOCAL = [
    ("firestaff-d1-side-door-front-metadata", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "240-244", ["DM1_VIEW_SQUARE_D1L, 0x0028, 0x0039", "DM1_VIEW_SQUARE_D1R, 0x0018, 0x0049"]),
    ("firestaff-d1-side-door-front-runtime-test", ROOT / "tests/test_dm1_v1_viewport_3d_pc34_compat.c", "707-775", ["DM1_VIEW_SQUARE_D1L", "DM1_VIEW_SQUARE_D1R", "door_front_occlusion.count", "door_front_occlusion.d1l_side_door_front_spec"]),
    ("firestaff-d1-side-door-front-source-evidence", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "2115-2122", ["DUNVIEW.C:7493-7536 D1L door-front occlusion", "DUNVIEW.C:7661-7704 D1R mirrored door-front occlusion"]),
]
def read_span(path, span):
    a, b = [int(x) for x in span.split("-")]
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return a, "\n".join(path.read_text(encoding=encoding, errors="replace").splitlines()[a - 1:b])
def ordered_hits(base, text, needles):
    cursor = 0; hits = []; missing = []
    for needle in needles:
        pos = text.find(needle, cursor)
        if pos < 0: missing.append(needle)
        else:
            hits.append({"line": base + text.count("\n", 0, pos), "needle": needle})
            cursor = pos + len(needle)
    return hits, missing
def audit(rows):
    out = []
    for ident, name, span, needles in rows:
        path = RED / name if isinstance(name, str) else name
        base, text = read_span(path, span)
        hits, missing = ordered_hits(base, text, needles)
        out.append({"id": ident, "status": "PASS" if not missing else "FAIL", "sourceFile": path.name, "lines": span, "hits": hits, "missing": missing})
    return out
def run(cmd):
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {"command": cmd, "returncode": proc.returncode, "passed": proc.returncode == 0, "outputTail": "\n".join(proc.stdout.strip().splitlines()[-14:])}
def main(check_only=False):
    red = audit(SRC); local = audit(LOCAL)
    failed = [row["id"] for row in red + local if row["status"] != "PASS"]
    if check_only:
        print("PASS pass565 check-only" if not failed else "FAIL pass565 check-only: " + ",".join(failed))
        return 0 if not failed else 1
    runtime = run([str(TEST_BINARY)])
    self_check = run([sys.executable, str(Path(__file__).resolve()), "--check-only"])
    ok = not failed and runtime["passed"] and self_check["passed"]
    manifest = {"schema": "pass565_dm1_v1_d1_side_door_front_source_lock.v1", "status": "passed" if ok else "failed", "statusToken": STATUS if ok else "FAILED_PASS565_DM1_V1_D1_SIDE_DOOR_FRONT_SOURCE_LOCK", "redmcsbRoot": str(RED), "redmcsbChecks": red, "firestaffChecks": local, "verificationRuns": [runtime, self_check], "nonClaims": ["No movement/input/capture changes.", "No renderer pixel parity claim.", "No original DOS runtime capture claim.", "No DANNESBURK use."]}
    MANIFEST.parent.mkdir(parents=True, exist_ok=True); REPORT.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = ["# Pass565 DM1 V1 D1 side door-front source lock", "", "Status: " + manifest["status"], "", "Claim: D1L and mirrored D1R front-door branches use ReDMCSB two-pass door-front order: one rear side cell is drawn before the top frame and door, then one front side cell is drawn after the door.", "", "## Primary ReDMCSB Evidence"]
    for row in red:
        lines += ["", f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"] + [f"  - line {hit['line']}: {hit['needle']}" for hit in row["hits"]] + [f"  - missing: {miss}" for miss in row["missing"]]
    lines += ["", "## Firestaff Evidence"]
    for row in local:
        lines += ["", f"- {row['status']} {row['id']} ({row['sourceFile']}:{row['lines']})"] + [f"  - line {hit['line']}: {hit['needle']}" for hit in row["hits"]] + [f"  - missing: {miss}" for miss in row["missing"]]
    lines += ["", "## Verification"]
    for row in manifest["verificationRuns"]:
        lines += ["", f"- {' '.join(row['command'])}: rc={row['returncode']}", "~~~", row["outputTail"], "~~~"]
    lines += ["", "## Non-Claims", "", "- No input or movement code was changed.", "- No original DOS pixel parity is claimed.", "- DANNESBURK was not used."]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(manifest["statusToken"]); print("- wrote", MANIFEST.relative_to(ROOT)); print("- wrote", REPORT.relative_to(ROOT))
    return 0 if ok else 1
if __name__ == "__main__":
    raise SystemExit(main("--check-only" in sys.argv))
