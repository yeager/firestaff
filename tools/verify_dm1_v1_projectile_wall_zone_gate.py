#!/usr/bin/env python3
"""Verify DM1 V1 projectile wall-zone movement/viewport source lock."""
from __future__ import annotations

import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
OUT = ROOT / "parity-evidence/verification/pass566_dm1_v1_projectile_wall_zone_gate/manifest.json"

CHECKS = [
    {"id": "projectile_wall_travel_blocker", "file": "PROJEXPL.C", "function": "F0219_PROJECTILE_ProcessEvents48To49", "lines": "717-727", "needles": ["L0522_B_ProjectileMovesToOtherSquare", "F0151_DUNGEON_GetSquare", "C00_ELEMENT_WALL", "C06_ELEMENT_FAKEWALL", "MASK0x0001_FAKEWALL_IMAGINARY | MASK0x0004_FAKEWALL_OPEN", "C03_ELEMENT_STAIRS", "F0217_PROJECTILE_HasImpactOccured(M034_SQUARE_TYPE(AL0516_ui_Square)"], "claim": "cross-square projectiles and thrown objects test wall-equivalent destination squares before F0267 moves the projectile thing"},
    {"id": "projectile_move_commit_after_blocker", "file": "PROJEXPL.C", "function": "F0219_PROJECTILE_ProcessEvents48To49", "lines": "729-745", "needles": ["L0515_T_ProjectileThingNewCell = M015_THING_WITH_NEW_CELL", "F0267_MOVE_GetMoveResult_CPSCE", "G0397_i_MoveResultMapX", "C04_ELEMENT_DOOR", "F0217_PROJECTILE_HasImpactOccured(C04_ELEMENT_DOOR"], "claim": "only non-wall resolved projectiles reach F0267 dungeon movement; same-square closed doors are handled separately"},
    {"id": "party_group_projectile_impact_precheck", "file": "MOVESENS.C", "function": "F0267_MOVE_GetMoveResult_CPSCE", "lines": "432-435", "needles": ["If moving the party or a creature on the party map from a dungeon square then check for a projectile impact", "F0266_MOVE_IsKilledByProjectileImpact", "return C1_TRUE"], "claim": "dungeon movement also checks projectile impacts for party/group movers before committing movement"},
    {"id": "viewport_projectile_zone_map", "file": "DUNVIEW.C", "function": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF", "lines": "370-373,5645-5683,5710-5715,5881-5883", "needles": ["G2028_ac_ViewSquareIndexTo", "T0115129_DrawProjectiles", "if ((L2479_i_ = G2028_ac_ViewSquareIndexTo[P0145_i_ViewSquareIndex]) < 0)", "if ((L2475_i_ViewDepth == 3) && (AL0126_i_ViewCell <= C01_VIEW_CELL_FRONT_RIGHT))", "if ((L2475_i_ViewDepth == 0) && (AL0126_i_ViewCell >= C02_VIEW_CELL_BACK_RIGHT))", "L2474_i_ZoneIndex = C2900_ZONE_ + ((unsigned int16_t)L2479_i_ * 4) + AL0126_i_ViewCell", "AL0150_ui_ProjectileScaleIndex = (L2475_i_ViewDepth << 1) - (AL0126_i_ViewCell >> 1)", "F0791_DUNGEONVIEW_DrawBitmapXX"], "claim": "visible projectiles are drawn only through F0115's row/cell zone map after cell clipping"},
    {"id": "viewport_wall_case_return_or_alcove", "file": "DUNVIEW.C", "function": "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF", "lines": "6697-6720,6811-6816", "needles": ["case C00_ELEMENT_WALL", "F0792_DUNGEONVIEW_DrawBitmapYYY", "F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF", "goto T0118028", "return;", "T0118028:", "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF"], "claim": "a plain wall case returns before the F0115 thing/projectile layer; only a front alcove branches into F0115"},
]

LOCAL_NEEDLES = [
    ("memory_projectile_pc34_compat.c", "PROJECTILE_BLOCKER_WALL"),
    ("memory_projectile_pc34_compat.c", "dispatch = PROJECTILE_RESULT_HIT_WALL"),
    ("dm1_v1_viewport_3d_pc34_compat.c", "dm1_viewport_3d_projectile_visible_after_wall_case"),
    ("dm1_v1_viewport_3d_pc34_compat.c", "return front_alcove && wall->front_alcove_reveals_contents;"),
    ("test_dm1_v1_viewport_3d_pc34_compat.c", "test_projectile_wall_zone_movement_visibility_gate"),
    ("test_dm1_v1_viewport_3d_pc34_compat.c", "PROJECTILE_SUBTYPE_KINETIC_ARROW"),
    ("test_dm1_v1_viewport_3d_pc34_compat.c", "projectile_wall_zone.wall_hit_result"),
    ("test_dm1_v1_viewport_3d_pc34_compat.c", "projectile_wall_zone.plain_wall_hides_projectile"),
    ("test_dm1_v1_viewport_3d_pc34_compat.c", "projectile_wall_zone.front_alcove_reveals_projectile_layer"),
]

def read_lines(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1").splitlines()
    chunks = []
    for part in spec.split(","):
        start, end = (int(x) for x in part.split("-", 1)) if "-" in part else (int(part), int(part))
        chunks.extend(lines[start - 1:end])
    return "\n".join(chunks)

def main() -> int:
    ok = True
    results = []
    for check in CHECKS:
        text = read_lines(RED / check["file"], check["lines"])
        missing = [n for n in check["needles"] if n not in text]
        passed = not missing
        ok = ok and passed
        results.append({**check, "ok": passed, "missing": missing})
    local = []
    for rel, needle in LOCAL_NEEDLES:
        passed = needle in (ROOT / rel).read_text(encoding="utf-8")
        ok = ok and passed
        local.append({"file": rel, "needle": needle, "ok": passed})
    payload = {"schema": "pass566_dm1_v1_projectile_wall_zone_gate.v1", "status": "PASS" if ok else "FAIL", "redmcsb_source_root": str(RED), "source_audit": results, "local_needles": local, "non_claims": ["does not read or hash DUNGEON.DAT/GRAPHICS.DAT", "does not claim pixel parity", "does not launch original DOS runtime"]}
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    if ok:
        print("PASS pass566_dm1_v1_projectile_wall_zone_gate")
        for r in results:
            print(f"- {r['file']}:{r['lines']} {r['function']} {r['claim']}")
        print(f"- manifest {OUT.relative_to(ROOT)}")
        return 0
    print(json.dumps(payload, indent=2, sort_keys=True), file=sys.stderr)
    return 1

if __name__ == "__main__":
    raise SystemExit(main())
