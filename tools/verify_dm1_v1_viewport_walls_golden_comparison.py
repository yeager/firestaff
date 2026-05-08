#!/usr/bin/env python3
"""Consolidate the DM1 V1 viewport/walls golden comparison source lock.

This gate is source-first.  It verifies the ReDMCSB contracts and current
Firestaff gates that must be true before a viewport/wall golden comparison can
be trusted: side-door/detail layering, draw order, row clipping, and available
original-evidence status.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
RED = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
DEFAULT_OUT = ROOT / "parity-evidence/verification/dm1_v1_viewport_walls_golden_comparison.json"
DEFAULT_REPORT = ROOT / "parity-evidence/dm1_v1_viewport_walls_golden_comparison.md"

EXISTING = {
    "movement_viewport_wall_golden": ROOT / "parity-evidence/verification/dm1_v1_movement_viewport_wall_golden.json",
    "pass177_draw_stack_verify_out": ROOT / "parity-evidence/verification/pass177_viewport_redmcsb_draw_stack_gate/verify.out",
    "pass179_original_capture_manifest": ROOT / "parity-evidence/verification/pass179_dm1_v1_original_evidence_capture_integration_gate/manifest.json",
    "pass207_original_movement_viewport_blocker_report": ROOT / "parity-evidence/pass207_dm1_v1_original_movement_viewport_blocker_gate.md",
}

SOURCE_CHECKS: list[dict[str, Any]] = [
    {
        "id": "viewport-base-and-far-to-near-square-order",
        "file": "DUNVIEW.C",
        "ranges": [(8318, 8542)],
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "if (G0297_B_DrawFloorAndCeilingRequested)",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0676_DrawD3L2(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0116_DUNGEONVIEW_DrawSquareD3L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0119_DUNGEONVIEW_DrawSquareD2L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0122_DUNGEONVIEW_DrawSquareD1L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0125_DUNGEONVIEW_DrawSquareD0L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
        ],
        "claim": "Viewport composition starts with source floor/ceiling and then draws far-to-near D3/D2/D1/D0 square functions.",
    },
    {
        "id": "left-side-door-detail-layering-d3-d2-d1",
        "file": "DUNVIEW.C",
        "ranges": [(6361, 6480), (6900, 7031), (7391, 7536)],
        "needles": [
            "STATICFUNCTION void F0116_DUNGEONVIEW_DrawSquareD3L",
            "case C17_ELEMENT_DOOR_FRONT:",
            "F0108_DUNGEONVIEW_DrawFloorOrnament(L0201_ai_SquareAspect[M558_FLOOR_ORNAMENT_ORDINAL], M588_VIEW_FLOOR_D3L);",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(L0201_ai_SquareAspect[M550_FIRST_THING], P0147_i_Direction, P0148_i_MapX, P0149_i_MapY, M601_VIEW_SQUARE_D3L, C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT);",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2120_DoorFrameLeftD3L, C718_ZONE_DOOR_FRAME_LEFT_D3L);",
            "F0111_DUNGEONVIEW_DrawDoor(L0201_ai_SquareAspect[M557_DOOR_THING_INDEX], L0201_ai_SquareAspect[M556_DOOR_STATE], G0693_ai_DoorNativeBitmapIndex_Front_D3LCR, C0_VIEW_DOOR_ORNAMENT_D3LCR, M624_ZONE_DOOR_D3L);",
            "L0200_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;",
            "STATICFUNCTION void F0119_DUNGEONVIEW_DrawSquareD2L",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2114_DoorFrameTopD2L, C729_ZONE_DOOR_FRAME_TOP_D2L);",
            "F0111_DUNGEONVIEW_DrawDoor(L0208_ai_SquareAspect[M557_DOOR_THING_INDEX], L0208_ai_SquareAspect[M556_DOOR_STATE], G0694_ai_DoorNativeBitmapIndex_Front_D2LCR, C1_VIEW_DOOR_ORNAMENT_D2LCR, M627_ZONE_DOOR_D2L);",
            "L0207_i_Order = C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT;",
            "STATICFUNCTION void F0122_DUNGEONVIEW_DrawSquareD1L",
            "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2111_DoorFrameTopD1L, C732_ZONE_DOOR_FRAME_TOP_D1L);",
            "F0111_DUNGEONVIEW_DrawDoor(L0214_ai_SquareAspect[M557_DOOR_THING_INDEX], L0214_ai_SquareAspect[M556_DOOR_STATE], G0695_ai_DoorNativeBitmapIndex_Front_D1LCR, C2_VIEW_DOOR_ORNAMENT_D1LCR, M630_ZONE_DOOR_D1L);",
            "L0213_i_Order = C0x0039_CELL_ORDER_DOORPASS2_FRONTRIGHT;",
        ],
        "claim": "Front side doors draw floor/detail pass 1, frame/door detail, then a second F0115 foreground pass with depth-specific door zones.",
    },
    {
        "id": "door-ornament-mask-layering-inside-door-bitmap",
        "file": "DUNVIEW.C",
        "ranges": [(4013, 4117), (4218, 4335)],
        "needles": [
            "STATICFUNCTION void F0109_DUNGEONVIEW_DrawDoorOrnament",
            "F0791_DUNGEONVIEW_DrawBitmapXX(AL0107_puc_Bitmap, G0074_puc_Bitmap_Temporary, C2000_ZONE_DOOR_ORNAMENT + G0103_as_CurrentMapDoorOrnamentsInfo[P0120_i_DoorOrnamentOrdinal].CoordinateSet * 3 + P0121_i_ViewDoorOrnamentIndex, 0, C09_COLOR_GOLD);",
            "STATICFUNCTION void F0111_DUNGEONVIEW_DrawDoor",
            "F0616_CopyBitmap(F0489_MEMORY_GetNativeBitmapOrGraphic(P0126_pi_DoorNativeBitmapIndices[AL0114_ui_DoorType = L0116_ps_Door->Type]), G0074_puc_Bitmap_Temporary);",
            "F0109_DUNGEONVIEW_DrawDoorOrnament(L0116_ps_Door->OrnamentOrdinal, P0128_i_ViewDoorOrnamentIndex);",
            "F0109_DUNGEONVIEW_DrawDoorOrnament(M000_INDEX_TO_ORDINAL(C16_DOOR_ORNAMENT_THIEVES_EYE_MASK), C2_VIEW_DOOR_ORNAMENT_D1LCR);",
            "F0109_DUNGEONVIEW_DrawDoorOrnament(M000_INDEX_TO_ORDINAL(C15_DOOR_ORNAMENT_DESTROYED_MASK), P0128_i_ViewDoorOrnamentIndex);",
            "F0791_DUNGEONVIEW_DrawBitmapXX(G0074_puc_Bitmap_Temporary, G0296_puc_Bitmap_Viewport, P2084_i_ZoneIndex, AL0114_ui_Flip, C10_COLOR_FLESH);",
        ],
        "claim": "Door ornaments, thieves-eye holes, and destroyed masks are composed into the temporary door bitmap before viewport blit.",
    },
    {
        "id": "open-cell-contents-deferred-draw-stack",
        "file": "DUNVIEW.C",
        "ranges": [(4547, 4582), (4820, 4855), (5690, 5701)],
        "needles": [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
            "if there is a projectile, explosion or creature, take note of it, but do not draw them yet",
            "draw each object found",
            "Draw one creature at the cell being processed",
            "Draw only projectiles at specified cell",
            "Draw only explosions at specified cell",
            "/* Draw objects */",
            "L0151_T_GroupThing = P0141_T_Thing",
            "L0186_B_SquareHasProjectile = C1_TRUE",
            "M612_GRAPHIC_FIRST_OBJECT",
            "M613_GRAPHIC_FIRST_PROJECTILE",
        ],
        "claim": "F0115 defers groups/projectiles/explosions while drawing objects first, then creatures, projectiles, explosions/fluxcages.",
    },
    {
        "id": "f0115-row-clipping-and-source-zone-families",
        "file": "DUNVIEW.C",
        "ranges": [(360, 380), (4547, 4582), (4800, 5090), (5658, 5688)],
        "needles": [
            "char G2028_ac_ViewSquareIndexTo[23] = { 11, -1, -1, 8, 9, 10, 5, 6, 7, -1, -1, 0, 1, 2, 3, 4, -1, -1, -1, -1, -1, -1, -1 };",
            "if ((AL0127_i_ThingType >= C05_THING_TYPE_WEAPON) && (AL0127_i_ThingType <= C10_THING_TYPE_JUNK)",
            "L2474_i_ZoneIndex = (C2500_ZONE_ | MASK0x8000_SHIFT_OBJECTS_AND_CREATURES) + (L2476_i_ * 4) + AL0126_i_ViewCell;",
            "if ((L2479_i_ = G2028_ac_ViewSquareIndexTo[P0145_i_ViewSquareIndex]) < 0)",
            "if ((L2475_i_ViewDepth == 3) && (AL0126_i_ViewCell <= C01_VIEW_CELL_FRONT_RIGHT))",
            "if ((L2475_i_ViewDepth == 0) && (AL0126_i_ViewCell >= C02_VIEW_CELL_BACK_RIGHT))",
            "L2474_i_ZoneIndex = C2900_ZONE_ + ((unsigned int16_t)L2479_i_ * 4) + AL0126_i_ViewCell;",
        ],
        "claim": "Object/projectile rows are selected through G2028 and rejected for source far/near edge cases before C2500/C2900 zone blits.",
    },
    {
        "id": "zone-family-definitions-for-comparison-seam",
        "file": "DEFS.H",
        "ranges": [(4228, 4260)],
        "needles": [
            "#define C2500_ZONE_",
            "#define C2900_ZONE_",
            "#define M624_ZONE_DOOR_D3L",
            "#define M627_ZONE_DOOR_D2L",
            "#define M630_ZONE_DOOR_D1L",
            "#define M631_ZONE_DOOR_D1C",
        ],
        "claim": "The comparison seam is source zone-index based: C2500/C2900 content rows and depth-specific M624/M627/M630/M631 door zones.",
    },
]


def read_lines(rel: str) -> list[str]:
    path = RED / rel
    if not path.is_file():
        raise AssertionError(f"missing ReDMCSB source file: {path}")
    return path.read_text(encoding="latin-1", errors="replace").splitlines()


def block(rel: str, ranges: list[tuple[int, int]]) -> str:
    ls = read_lines(rel)
    parts: list[str] = []
    for start, end in ranges:
        parts.extend(ls[start - 1 : end])
    return "\n".join(parts)


def compact(text: str) -> str:
    return " ".join(text.split())


def audit_source() -> list[dict[str, Any]]:
    out: list[dict[str, Any]] = []
    for check in SOURCE_CHECKS:
        text = block(check["file"], check["ranges"])
        ctext = compact(text)
        missing = [n for n in check["needles"] if compact(n) not in ctext]
        out.append({
            "id": check["id"],
            "ok": not missing,
            "citations": [f"{check['file']}:{a}-{b}" for a, b in check["ranges"]],
            "claim": check["claim"],
            "missing": missing,
            "needle_count": len(check["needles"]),
        })
    return out


def artifact_status() -> dict[str, Any]:
    files = {name: {"path": str(path), "exists": path.exists()} for name, path in EXISTING.items()}
    missing = [name for name, meta in files.items() if not meta["exists"]]
    if missing:
        raise AssertionError(f"missing required artifact(s): {missing}")

    golden = json.loads(EXISTING["movement_viewport_wall_golden"].read_text(encoding="utf-8"))
    pass179 = json.loads(EXISTING["pass179_original_capture_manifest"].read_text(encoding="utf-8"))
    draw_stack = EXISTING["pass177_draw_stack_verify_out"].read_text(encoding="utf-8", errors="replace")
    pass207 = EXISTING["pass207_original_movement_viewport_blocker_report"].read_text(encoding="utf-8", errors="replace")

    required_draw_stack = [
        "V1 viewport ReDMCSB draw-stack source gate passed",
        "ReDMCSB F0115 starts",
        "draw each object",
        "draw creature after objects",
        "draw projectiles after creatures",
        "draw explosions last",
    ]
    pass207_required = [
        "viewport-draw-uses-direction-and-map-coordinates",
        "viewport-present-requests-vblank-blit",
    ]
    pass207_status_ok = ("Status: `BLOCKED_MOVEMENT_VIEWPORT_ROUTE_NOT_PROMOTABLE`" in pass207) or ("Status: `SUPERSEDED_BY_PASS304_PASS308_STATE_ORACLE_PENDING`" in pass207)
    p179_art = pass179.get("existing_artifacts", {})
    capture = p179_art.get("capture_overlay_recovery", {})
    pass175 = p179_art.get("pass175_status", {})
    dbg = p179_art.get("debugger_address_map_boundary", {})
    mvw = p179_art.get("movement_viewport_wall_golden", {})

    return {
        "files": files,
        "movement_viewport_wall_golden": {
            "status": golden.get("status"),
            "entry": (golden.get("dm1_entry_asset_check") or {}).get("decoded"),
            "representative_case_count": len(((golden.get("golden") or {}).get("representativeCases") or [])),
            "assertions": (golden.get("golden") or {}).get("assertions", []),
            "limitations": (golden.get("source_asset_metadata") or {}).get("limitations", []),
        },
        "pass177_draw_stack": {"ok": not [n for n in required_draw_stack if n not in draw_stack], "missing": [n for n in required_draw_stack if n not in draw_stack]},
        "pass179_original_capture": {
            "status": pass179.get("status"),
            "capture_overlay_recovery": capture.get("status"),
            "pass175_runtime_debugger_gate": pass175.get("runtime_debugger_gate_classification"),
            "pass175_queue_probe": pass175.get("queue_probe_classification"),
            "remaining_blocker": pass175.get("remaining_blocker"),
            "debugger_boundary": dbg.get("required_boundary"),
            "movement_viewport_wall_status": mvw.get("status"),
        },
        "pass207_original_route": {"ok": pass207_status_ok and not [n for n in pass207_required if n not in pass207], "missing": (["accepted BLOCKED or SUPERSEDED pass207 status"] if not pass207_status_ok else []) + [n for n in pass207_required if n not in pass207], "status": "SUPERSEDED_BY_PASS304_PASS308_STATE_ORACLE_PENDING" if "Status: `SUPERSEDED_BY_PASS304_PASS308_STATE_ORACLE_PENDING`" in pass207 else "BLOCKED_MOVEMENT_VIEWPORT_ROUTE_NOT_PROMOTABLE"},
    }


def overall(source: list[dict[str, Any]], artifacts: dict[str, Any]) -> str:
    if any(not x["ok"] for x in source):
        return "FAIL_SOURCE_MISMATCH"
    if artifacts["movement_viewport_wall_golden"]["status"] != "PASS":
        return "FAIL_MOVEMENT_VIEWPORT_WALL_GOLDEN"
    if not artifacts["pass177_draw_stack"]["ok"]:
        return "FAIL_DRAW_STACK_ARTIFACT"
    if not artifacts["pass207_original_route"]["ok"]:
        return "FAIL_ORIGINAL_ROUTE_ARTIFACT"
    return "PASS_SOURCE_LOCKED_ORIGINAL_CAPTURE_BLOCKED"


def write_report(manifest: dict[str, Any], path: Path) -> None:
    art = manifest["artifacts"]
    lines = [
        "# DM1 V1 viewport/walls golden comparison source lock",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "This is a focused golden-comparison lock for viewport/walls only. It verifies ReDMCSB source facts and current Firestaff evidence gates; it does **not** claim original pixel parity.",
        "",
        "## ReDMCSB citations verified",
        "",
    ]
    for item in manifest["redmcsb_source_audit"]:
        state = "PASS" if item["ok"] else "FAIL"
        lines.append(f"- {state} `{item['id']}` — {', '.join(item['citations'])}: {item['claim']}")
    lines += [
        "",
        "## Existing evidence joined",
        "",
        f"- Movement/viewport/wall golden: `{art['movement_viewport_wall_golden']['status']}`; entry `{art['movement_viewport_wall_golden']['entry']}`; representative cases `{art['movement_viewport_wall_golden']['representative_case_count']}`.",
        f"- Draw stack artifact: `{'PASS' if art['pass177_draw_stack']['ok'] else 'FAIL'}`.",
        f"- Original capture integration: `{art['pass179_original_capture']['status']}`; capture route `{art['pass179_original_capture']['capture_overlay_recovery']}`.",
        f"- Original route follow-up: `{art['pass207_original_route']['status']}`.",
        "",
        "## Boundary",
        "",
        "- Source-locked: side-door/detail layering, far-to-near draw order, F0115 object/creature/projectile stack, and C2500/C2900 row clipping.",
        "- Original capture: available artifacts are blocker/supersession evidence only; pass304/pass308 route-label coverage still waits on state-oracle proof before pixel promotion.",
        "- No push, no <private-host> use, no original-vs-Firestaff pixel parity claim.",
        "",
    ]
    path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--report", type=Path, default=DEFAULT_REPORT)
    args = parser.parse_args()

    source = audit_source()
    artifacts = artifact_status()
    manifest = {
        "schema": "dm1_v1_viewport_walls_golden_comparison.v1",
        "status": overall(source, artifacts),
        "redmcsb_source_root": str(RED),
        "scope": "DM1 V1 viewport/walls golden comparison: side-door/detail layering, draw order, row clipping, original evidence status",
        "redmcsb_source_audit": source,
        "artifacts": artifacts,
        "non_claims": [
            "No original-vs-Firestaff pixel parity claim.",
            "No new DOSBox capture route.",
            "No <private-host>/<private-host-ip> use.",
            "No push.",
        ],
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest, args.report)
    print(json.dumps({
        "status": manifest["status"],
        "manifest": str(args.out),
        "report": str(args.report),
        "source_checks": len(source),
        "source_failures": [x["id"] for x in source if not x["ok"]],
        "original_route_status": artifacts["pass207_original_route"]["status"],
    }, indent=2, sort_keys=True))
    return 0 if manifest["status"].startswith("PASS") else 1


if __name__ == "__main__":
    raise SystemExit(main())
