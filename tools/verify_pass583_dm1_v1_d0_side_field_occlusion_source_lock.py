#!/usr/bin/env python3
from pathlib import Path
import json
import subprocess

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C").expanduser()
MANIFEST = ROOT / "parity-evidence/verification/pass583_dm1_v1_d0_side_field_occlusion_source_lock/manifest.json"
REPORT = ROOT / "parity-evidence/pass583_dm1_v1_d0_side_field_occlusion_source_lock.md"
STATUS = "PASS583_DM1_V1_D0_SIDE_FIELD_OCCLUSION_SOURCE_LOCKED"

SOURCE_CHECKS = [
    ("D0L", "7960-8062", [
        "STATICFUNCTION void F0125_DUNGEONVIEW_DrawSquareD0L(",
        "case C18_ELEMENT_STAIRS_SIDE:",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G0079_ai_StairsNativeBitmapIndices[C17_STAIRS_BITMAP_SIDE_D0L], C832_ZONE_STAIRS_SIDE_D0L);",
        "return;",
        "case C02_ELEMENT_PIT:",
        "C861_ZONE_FLOORPIT_D0L",
        "F0112_DUNGEONVIEW_DrawCeilingPit(C068_GRAPHIC_CEILING_PIT_D0L, C870_ZONE_CEILING_PIT_D0L",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0220_ai_SquareAspect[M550_FIRST_THING], P0174_i_Direction, P0175_i_MapX, P0176_i_MapY, M610_VIEW_SQUARE_D0L, C0x0002_CELL_ORDER_BACKRIGHT);",
        "case C00_ELEMENT_WALL:",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L);",
        "return;",
        "if (L0220_ai_SquareAspect[C0_ELEMENT] == C05_ELEMENT_TELEPORTER)",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M610_VIEW_SQUARE_D0L]], C716_ZONE_WALL_D0L);",
    ]),
    ("D0R", "8064-8162", [
        "STATICFUNCTION void F0126_DUNGEONVIEW_DrawSquareD0R(",
        "case C18_ELEMENT_STAIRS_SIDE:",
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G0079_ai_StairsNativeBitmapIndices[C17_STAIRS_BITMAP_SIDE_D0L], C833_ZONE_STAIRS_SIDE_D0R);",
        "return;",
        "case C02_ELEMENT_PIT:",
        "C863_ZONE_FLOORPIT_D0R",
        "F0112_DUNGEONVIEW_DrawCeilingPit(C068_GRAPHIC_CEILING_PIT_D0L, C872_ZONE_CEILING_PIT_D0R",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0221_ai_SquareAspect[M550_FIRST_THING], P0177_i_Direction, P0178_i_MapX, P0179_i_MapY, M611_VIEW_SQUARE_D0R, C0x0001_CELL_ORDER_BACKLEFT);",
        "case C00_ELEMENT_WALL:",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C00_WALL_D0R], C717_ZONE_WALL_D0R);",
        "return;",
        "if (L0221_ai_SquareAspect[C0_ELEMENT] == C05_ELEMENT_TELEPORTER)",
        "F0113_DUNGEONVIEW_DrawField(G0188_aauc_Graphic558_FieldAspects[G2035_ac_ViewSquareIndexToFieldAspectIndex[M611_VIEW_SQUARE_D0R]], C717_ZONE_WALL_D0R);",
    ]),
]

LOCAL_CHECKS = [
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DM1_VIEW_SQUARE_D0L, 0x0002"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DUNVIEW.C:7978-8062 D0L"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DM1_VIEW_SQUARE_D0R, 0x0001"),
    ("src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "DUNVIEW.C:8082-8162 D0R"),
    ("tests/test_dm1_v1_viewport_3d_pc34_compat.c", "floor_field_order_spec_count(), 15"),
    ("tests/test_dm1_v1_viewport_3d_pc34_compat.c", "floor_field_order.d0l_side_spec"),
    ("tests/test_dm1_v1_viewport_3d_pc34_compat.c", "floor_field_order.d0r_side_spec"),
]


def check_order(text, needles):
    pos = 0
    missing = []
    for needle in needles:
        found = text.find(needle, pos)
        if found < 0:
            missing.append(needle)
        else:
            pos = found + len(needle)
    return missing


def main():
    red_lines = RED.read_text(encoding="latin-1", errors="replace").splitlines()
    red_rows = []
    for ident, span, needles in SOURCE_CHECKS:
        first, last = (int(x) for x in span.split("-"))
        body = "\n".join(red_lines[first - 1:last])
        missing = check_order(body, needles)
        red_rows.append({"id": ident, "source": f"DUNVIEW.C:{span}", "status": "PASS" if not missing else "FAIL", "missing": missing})

    local_rows = []
    for rel, needle in LOCAL_CHECKS:
        text = (ROOT / rel).read_text(errors="replace")
        local_rows.append({"file": rel, "needle": needle, "status": "PASS" if needle in text else "FAIL"})

    test = subprocess.run([str(ROOT / "build" / "test_dm1_v1_viewport_3d_pc34_compat")], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    ok = all(row["status"] == "PASS" for row in red_rows + local_rows) and test.returncode == 0
    manifest = {
        "schema": "pass583_dm1_v1_d0_side_field_occlusion_source_lock.v1",
        "status": "passed" if ok else "failed",
        "statusToken": STATUS if ok else "FAILED_PASS583_DM1_V1_D0_SIDE_FIELD_OCCLUSION_SOURCE_LOCK",
        "redmcsbChecks": red_rows,
        "firestaffChecks": local_rows,
        "verificationRuns": [{
            "command": [str(ROOT / "build" / "test_dm1_v1_viewport_3d_pc34_compat")],
            "returncode": test.returncode,
            "passed": test.returncode == 0,
            "outputTail": "\n".join(test.stdout.splitlines()[-10:]),
        }],
        "nonClaims": [
            "no renderer pixel parity claim",
            "no input or movement queue changes",
            "no original DOS runtime capture claim",
            "no CSB/DM2/Nexus behavior claim",
        ],
    }
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    REPORT.write_text(
        "# Pass583 DM1 V1 D0 side field occlusion source lock\n\n"
        f"Status: {manifest['status']}\n\n"
        "Claim: D0L and D0R side squares now carry the same source-locked floor/field metadata as the nearer side lanes: stairs-side returns, pit falls through to ceiling/F0115, teleporter field draws after F0115, and wall cases return before F0115/field.\n\n"
        "Primary evidence: DUNVIEW.C:7960-8062 and DUNVIEW.C:8064-8162.\n",
        encoding="utf-8",
    )
    print(manifest["statusToken"])
    print("- wrote", MANIFEST.relative_to(ROOT))
    print("- wrote", REPORT.relative_to(ROOT))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
