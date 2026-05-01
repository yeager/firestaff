#!/usr/bin/env python3
"""Consolidate DM1 V1 original-evidence/capture blockers into one gate.

This is intentionally a source-first N2 gate, not a visual parity claim. It
re-checks the original ReDMCSB contracts that matter for movement, viewport
state, wall/content draw order, and click/command delivery; then it joins the
existing capture/debugger evidence so future work has one reproducible lane
instead of scattered pass175/pass177/pass127 references.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
CANONICAL_DM1 = Path("/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1")
OUT_DIR = ROOT / "parity-evidence/verification/pass179_dm1_v1_original_evidence_capture_integration_gate"
REPORT = ROOT / "parity-evidence/pass179_dm1_v1_original_evidence_capture_integration_gate.md"

SOURCE_CHECKS: list[dict[str, Any]] = [
    {"id":"new-game-party-state","file":"LOADSAVE.C","ranges":[(1940,1945)],"needles":["if (G0298_B_NewGame)","G0306_i_PartyMapX = (AL1353_i_InitialPartyLocation = G0278_ps_DungeonHeader->InitialPartyLocation) & 0x001F;","G0307_i_PartyMapY = (AL1353_i_InitialPartyLocation >>= 5) & 0x001F;","G0308_i_PartyDirection = (AL1353_i_InitialPartyLocation >> 5) & 0x0003;","G0309_i_PartyMapIndex = 0;"],"claim":"Original new-game viewport state is decoded from DUNGEON_HEADER.InitialPartyLocation and starts on map 0."},
    {"id":"movement-vectors-relative-step","file":"DUNGEON.C","ranges":[(35,44),(1371,1391)],"needles":["G0233_ai_Graphic559_DirectionToStepEastCount","G0234_ai_Graphic559_DirectionToStepNorthCount","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement","P0253_i_Direction += 1, P0253_i_Direction &= 3; /* Simulate turning right */","*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0254_i_StepsForwardCount","*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0255_i_StepsRightCount"],"claim":"Every relative party/viewport probe must rotate the facing direction first, then apply the original east/north vectors."},
    {"id":"movement-legality-and-cooldown","file":"CLIKMENU.C","ranges":[(156,173),(237,347)],"needles":["F0284_CHAMPION_SetPartyDirection","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement","L1116_i_SquareType == C00_ELEMENT_WALL","L1117_B_MovementBlocked = M036_DOOR_STATE(AL1115_ui_Square);","MASK0x0004_FAKEWALL_OPEN","MASK0x0001_FAKEWALL_IMAGINARY","G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;","G0311_i_ProjectileDisabledMovementTicks = 0;","G0321_B_StopWaitingForPlayerInput = C1_TRUE;"],"claim":"Party turn/step commands mutate state only through source movement legality and then stop the wait loop for redraw."},
    {"id":"main-loop-draw-command-cadence","file":"GAMELOOP.C","ranges":[(90,90),(150,155),(215,219)],"needles":["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);","G0310_i_DisabledMovementTicks--","G0311_i_ProjectileDisabledMovementTicks--","F0380_COMMAND_ProcessQueue_CPSC();","while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"],"claim":"Capture timing must respect draw-from-current-party-state, command queue processing, and cooldown decrement cadence."},
    {"id":"viewport-state-draw-request","file":"DUNVIEW.C","ranges":[(8318,8616)],"needles":["void F0128_DUNGEONVIEW_Draw_CPSF","P0183_i_Direction","P0184_i_MapX","P0185_i_MapY","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement","F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"],"claim":"Dungeon-view cells are derived from the updated party direction/map coordinates before presentation."},
    {"id":"viewport-floor-ceiling-base","file":"DUNVIEW.C","ranges":[(2962,3003)],"needles":["void F0098_DUNGEONVIEW_DrawFloorAndCeiling","F0008_MAIN_ClearBytes(G0086_puc_Bitmap_ViewportBlackArea","F0007_MAIN_CopyBytes(G0085_puc_Bitmap_Ceiling, G0296_puc_Bitmap_Viewport","F0007_MAIN_CopyBytes(G0084_puc_Bitmap_Floor","G0297_B_DrawFloorAndCeilingRequested = C0_FALSE;"],"claim":"Viewport draw order starts from black/ceiling/floor source bitmaps, not from a captured previous frame."},
    {"id":"wall-door-blits-into-viewport","file":"DUNVIEW.C","ranges":[(3048,3110)],"needles":["void F0100_DUNGEONVIEW_DrawWallSetBitmap","F0132_VIDEO_Blit(P0105_puc_Bitmap, G0296_puc_Bitmap_Viewport","void F0102_DUNGEONVIEW_DrawDoorBitmap","F0132_VIDEO_Blit(G0074_puc_Bitmap_Temporary, G0296_puc_Bitmap_Viewport","void F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally"],"claim":"Wall/door assets share the same viewport bitmap target and must be compared as composited viewport content."},
    {"id":"object-creature-projectile-draw-stack","file":"DUNVIEW.C","ranges":[(4547,5113),(5311,5316),(5693,5700)],"needles":["STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF","draw objects in an alcove on a wall square","Draw one creature at the cell being processed","Draw only projectiles at specified cell","Draw only explosions at specified cell","AL0127_i_ThingType >= C05_THING_TYPE_WEAPON","AL0127_i_ThingType <= C10_THING_TYPE_JUNK","L0135_B_DrawAlcoveObjects","C04_THING_TYPE_GROUP","C14_THING_TYPE_PROJECTILE","C15_THING_TYPE_EXPLOSION"],"claim":"Open-cell visual evidence must preserve the original object, creature, projectile/effect deferral order."},
    {"id":"viewport-present-seam","file":"DRAWVIEW.C","ranges":[(709,724),(840,858)],"needles":["void F0097_DUNGEONVIEW_DrawViewport","G0324_B_DrawViewportRequested = C1_TRUE","M526_WaitVerticalBlank();","F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT","VIDRV_09_BlitViewPort"],"claim":"The comparison seam is the presented 224x136 viewport zone after the viewport draw request/vblank path."},
    {"id":"click-queue-c080-boundary","file":"COMMAND.C","ranges":[(1,16),(1452,1662),(2045,2127),(2322,2324)],"needles":["G0432_as_CommandQueue","G0433_i_CommandQueueFirstIndex","G0434_i_CommandQueueLastIndex","F0359_COMMAND_ProcessClick_CPSC","G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex].Command","F0380_COMMAND_ProcessQueue_CPSC","F0377_COMMAND_ProcessType80_ClickInDungeonView"],"claim":"Pass175 original-runtime work must bind addresses at enqueue, dequeue, C080 dispatch, and candidate transition; visual no-delta alone is not evidence."},
]

REQUIRED_EXISTING = {
    "overlay_source_lock_tool": ROOT / "tools/verify_original_overlay_capture_source_lock.py",
    "movement_viewport_wall_golden_tool": ROOT / "tools/verify_dm1_v1_movement_viewport_wall_golden.py",
    "pass175_runtime_debugger_gate_tool": ROOT / "tools/pass175_c080_runtime_debugger_gate.py",
    "pass175_queue_probe_tool": ROOT / "tools/pass175_original_queue_breakpoint_probe.py",
    "pass177_draw_stack_verify_out": ROOT / "parity-evidence/verification/pass177_viewport_redmcsb_draw_stack_gate/verify.out",
    "movement_viewport_wall_golden_json": ROOT / "parity-evidence/verification/dm1_v1_movement_viewport_wall_golden.json",
    "pass175_runtime_debugger_manifest": ROOT / "parity-evidence/verification/pass175_c080_runtime_debugger_gate/manifest.json",
    "pass175_queue_manifest": ROOT / "parity-evidence/verification/pass175_original_queue_breakpoint_probe/manifest.json",
}

def read_lines(path: Path) -> list[str]:
    if not path.is_file(): raise AssertionError(f"missing file: {path}")
    return path.read_text(encoding="latin-1", errors="replace").splitlines()

def block(file: str, ranges: list[tuple[int,int]]) -> str:
    ls = read_lines(REDMCSB / file); out: list[str] = []
    for start, end in ranges: out.extend(ls[start-1:end])
    return "\n".join(out)

def compact_contains(text: str, needle: str) -> bool:
    return " ".join(needle.split()) in " ".join(text.split())

def source_audit() -> list[dict[str, Any]]:
    results = []
    for check in SOURCE_CHECKS:
        text = block(check["file"], check["ranges"])
        missing = [n for n in check["needles"] if not compact_contains(text, n)]
        results.append({"id":check["id"],"citations":[f"{check['file']}:{a}-{b}" for a,b in check["ranges"]],"claim":check["claim"],"ok":not missing,"missing":missing,"needle_count":len(check["needles"])})
    return results

def existing_artifact_audit() -> dict[str, Any]:
    files = {name:{"path":str(path),"exists":path.exists()} for name,path in REQUIRED_EXISTING.items()}
    for name,item in files.items():
        if not item["exists"]: raise AssertionError(f"missing required existing artifact: {name}: {item['path']}")
    pass175_runtime = json.loads(REQUIRED_EXISTING["pass175_runtime_debugger_manifest"].read_text())
    pass175_queue = json.loads(REQUIRED_EXISTING["pass175_queue_manifest"].read_text())
    golden = json.loads(REQUIRED_EXISTING["movement_viewport_wall_golden_json"].read_text())
    draw_stack_text = REQUIRED_EXISTING["pass177_draw_stack_verify_out"].read_text(errors="replace")
    required_draw_stack = ["V1 viewport ReDMCSB draw-stack source gate passed","ReDMCSB F0115 starts","draw each object","draw creature after objects","draw projectiles after creatures","draw explosions last","Firestaff contents layer 0 floor ornaments","Firestaff contents layer 1 floor items","Firestaff contents layer 2 creatures","Firestaff contents layer 3 projectiles/effects"]
    missing_draw_stack = [n for n in required_draw_stack if n not in draw_stack_text]
    runtime_class = pass175_runtime.get("classification")
    queue_class = pass175_queue.get("classification")
    entry = (golden.get("dm1_entry_asset_check") or {}).get("decoded") or {}
    return {
        "files": files,
        "capture_overlay_recovery": {"status":"tooling-recovered-source-locked-not-overlay-ready","evidence":["tools/verify_original_overlay_capture_source_lock.py","scripts/dosbox_dm1_original_viewport_reference_capture.sh","tools/pass80_original_frame_classifier.py","tools/pass112_original_semantic_route_audit.py"],"boundary":"source/tool route is present; semantic original capture must classify as expected route before pixel overlay parity claims"},
        "pass175_status": {"runtime_debugger_gate_classification":runtime_class,"queue_probe_classification":queue_class,"retired_firestaff_blocker":queue_class=="retired/firestaff-source-locked-c080-gate-passed-original-debugger-symbol-binding-blocked","remaining_blocker":pass175_runtime.get("exact_remaining_blocker") or pass175_queue.get("next_blocker"),"breakpoint_gate_count":len(pass175_runtime.get("breakpoint_gates", [])),"breakpoint_gates":pass175_runtime.get("breakpoint_gates", [])},
        "debugger_address_map_boundary": {"classification":"blocked/address-map-required" if runtime_class=="blocked/address-map-required" else runtime_class,"required_boundary":"map original LZEXE/real-mode addresses for F0359 enqueue, F0380 dequeue, F0377 C080 handler, and F0280 candidate transition","forbidden_claim":"do not infer original-runtime success from static no-delta frames or Firestaff-only C080 tests"},
        "movement_viewport_wall_golden": {"status":golden.get("status"),"entry_state":entry,"assertions":(golden.get("golden") or {}).get("assertions", []),"representative_case_count":len((golden.get("golden") or {}).get("representativeCases", [])),"citation":(golden.get("golden") or {}).get("citation")},
        "pass177_draw_stack": {"ok":not missing_draw_stack,"missing":missing_draw_stack,"verify_out":str(REQUIRED_EXISTING["pass177_draw_stack_verify_out"])},
    }

def overall_status(source_results: list[dict[str, Any]], artifacts: dict[str, Any]) -> str:
    if any(not x["ok"] for x in source_results): return "FAIL_SOURCE_MISMATCH"
    if not artifacts["pass177_draw_stack"]["ok"]: return "FAIL_DRAW_STACK_ARTIFACT"
    if artifacts["movement_viewport_wall_golden"]["status"] != "PASS": return "FAIL_GOLDEN_MISSING"
    return "PASS_SOURCE_LOCKED_ORIGINAL_RUNTIME_BLOCKED_ON_ADDRESS_MAP"

def write_report(manifest: dict[str, Any], report: Path) -> None:
    art = manifest["existing_artifacts"]
    lines = [
        "# Pass179 DM1 V1 original evidence/capture integration gate","",f"Status: `{manifest['status']}`","",
        "This is the merge lane for the current big original-faithful blocker cluster. It is deliberately source-first: it verifies ReDMCSB movement/viewport/draw/click contracts and joins the existing capture/debugger artifacts, but it does **not** claim original pixel parity.","",
        "## What is now consolidated","",
        f"- Capture-overlay recovery: `{art['capture_overlay_recovery']['status']}` via `tools/verify_original_overlay_capture_source_lock.py` and classifier/route tools.",
        f"- Pass175 status: runtime gate `{art['pass175_status']['runtime_debugger_gate_classification']}`; queue probe `{art['pass175_status']['queue_probe_classification']}`.",
        f"- Debugger/address-map boundary: {art['debugger_address_map_boundary']['required_boundary']}.",
        f"- Movement/viewport/walls: `{art['movement_viewport_wall_golden']['status']}` with entry state `{art['movement_viewport_wall_golden']['entry_state']}` and {len(art['movement_viewport_wall_golden']['assertions'])} golden assertions.","",
        "## ReDMCSB source citations verified","",
    ]
    for item in manifest["redmcsb_source_audit"]:
        status = "PASS" if item["ok"] else "FAIL"
        lines.append(f"- {status} `{item['id']}` — {', '.join(item['citations'])}: {item['claim']}")
    lines += ["","## Remaining original-runtime blockers","",
        "1. Build or obtain an original DM.EXE address/symbol map for the pass175 gates: F0359 enqueue, F0380 dequeue, F0377 C080 handler, and F0280 candidate transition.",
        "2. Produce a semantic-ready 320x200 original route capture after party/champion control is proven; source/tool recovery alone is not an overlay reference.",
        "3. Only then run original-vs-Firestaff viewport/HUD/inventory pixel overlays from the source-locked movement state.","",
        "## Non-claims","","- No DANNESBURK/192.168.2.126 use.","- No push.","- No original pixel parity claim.","- No claim that DOSBox has reached F0359/F0380/F0377/F0280 until the address-map gate is solved.",""]
    report.write_text("\n".join(lines), encoding="utf-8")

def main() -> int:
    p = argparse.ArgumentParser(); p.add_argument("--out-dir", type=Path, default=OUT_DIR); p.add_argument("--report", type=Path, default=REPORT); args = p.parse_args()
    source_results = source_audit(); artifacts = existing_artifact_audit(); status = overall_status(source_results, artifacts)
    manifest = {"schema":"pass179_dm1_v1_original_evidence_capture_integration_gate.v1","status":status,"worker":"N2 / firestaff-worker / trv2@192.168.3.121","repo":str(ROOT),"redmcsb_source_root":str(REDMCSB),"canonical_dm1_root":str(CANONICAL_DM1),"forbidden_hosts":["DANNESBURK","192.168.2.126"],"redmcsb_source_audit":source_results,"existing_artifacts":artifacts}
    args.out_dir.mkdir(parents=True, exist_ok=True); (args.out_dir/"manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True)+"\n", encoding="utf-8"); write_report(manifest, args.report)
    print(json.dumps({"status":status,"manifest":str(args.out_dir/"manifest.json"),"report":str(args.report),"source_checks":len(source_results),"source_failures":[x["id"] for x in source_results if not x["ok"]],"pass175_remaining_blocker":artifacts["pass175_status"]["remaining_blocker"]}, indent=2, sort_keys=True))
    return 0 if status.startswith("PASS") else 1

if __name__ == "__main__":
    raise SystemExit(main())
