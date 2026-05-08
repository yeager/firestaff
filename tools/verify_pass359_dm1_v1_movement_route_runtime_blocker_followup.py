#!/usr/bin/env python3
"""Pass359 verifier: classify DM1 V1 movement-route/runtime blocker after pass331/pass349/pass356/pass358.

This pass is evidence-only. It keeps the ReDMCSB source order as the primary
source of truth, then cross-checks current Firestaff/prior-pass artifacts so the
old route-key blocker cannot be confused with the still-open original FIRES
true-stop/presentation blocker.
"""
from __future__ import annotations

import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass359_dm1_v1_movement_route_runtime_blocker_followup"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
EXPECTED_STATUS = "PASS_DM1_V1_MOVEMENT_ROUTE_RUNTIME_BLOCKER_CLASSIFIED"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {"id":"startup_installs_dungeon_input_tables","file":"STARTUP2.C","lines":"1179-1183","function":"F0004_MAIN_ProcessEntranceCommand160_NewGame","needles":["G0441_ps_PrimaryMouseInput = G0447_as_Graphic561_PrimaryMouseInput_Interface;","G0442_ps_SecondaryMouseInput = G0448_as_Graphic561_SecondaryMouseInput_Movement;","G0443_ps_PrimaryKeyboardInput = G0458_as_Graphic561_PrimaryKeyboardInput_Interface;","G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement;","F0003_MAIN_ProcessNewPartyMap_CPSE(G0309_i_PartyMapIndex);"],"decision":"Dungeon runtime installs movement-capable mouse/keyboard tables before drawing the new party map."},
    {"id":"i34e_keyboard_movement_table","file":"COMMAND.C","lines":"636-685","function":"G0459_as_Graphic561_SecondaryKeyboardInput_Movement","needles":["KEYBOARD_INPUT G0459_as_Graphic561_SecondaryKeyboardInput_Movement[7]","#ifdef MEDIA707_I34E_I34M","{ C001_COMMAND_TURN_LEFT,     0x004B }","{ C003_COMMAND_MOVE_FORWARD,  0x004C }","{ C002_COMMAND_TURN_RIGHT,    0x004D }","{ C006_COMMAND_MOVE_LEFT,     0x004F }","{ C005_COMMAND_MOVE_BACKWARD, 0x0050 }","{ C004_COMMAND_MOVE_RIGHT,    0x0051 }"],"decision":"The PC34/I34E movement key table maps normalized movement keys to C001..C006."},
    {"id":"mouse_primary_then_secondary_route","file":"COMMAND.C","lines":"1641-1661","function":"F0359_COMMAND_ProcessClick_CPSC","needles":["L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput, P0725_i_X, P0726_i_Y, P0727_i_ButtonsStatus);","L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput, P0725_i_X, P0726_i_Y, P0727_i_ButtonsStatus);","G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command;","G0432_as_CommandQueue[L1108_i_CommandQueueIndex].X = P0725_i_X;","G0432_as_CommandQueue[L1108_i_CommandQueueIndex].Y = P0726_i_Y;"],"decision":"Mouse clicks resolve primary interface first, then secondary movement, and enqueue command/X/Y in source order."},
    {"id":"keyboard_route_to_queue","file":"COMMAND.C","lines":"1709-1813","function":"F0361_COMMAND_ProcessKeyPress","needles":["void F0361_COMMAND_ProcessKeyPress","G0443_ps_PrimaryKeyboardInput","G0444_ps_SecondaryKeyboardInput","G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command","G2153_i_QueuedCommandsCount++"],"decision":"Keyboard input is table-resolved into the same command queue used by mouse movement commands."},
    {"id":"queue_dequeue_dispatch","file":"COMMAND.C","lines":"2045-2156","function":"F0380_COMMAND_ProcessQueue_CPSC","needles":["void F0380_COMMAND_ProcessQueue_CPSC","G0435_B_CommandQueueLocked = C1_TRUE;","G2153_i_QueuedCommandsCount == 0","L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;","F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);","F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"],"decision":"F0380 is the canonical command queue consumer and movement dispatcher."},
    {"id":"turn_and_step_side_effects","file":"CLIKMENU.C","lines":"142-174,180-347","function":"F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty","needles":["F0365_COMMAND_ProcessTypes1To2_TurnParty","F0284_CHAMPION_SetPartyDirection","F0366_COMMAND_ProcessTypes3To6_MoveParty","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement","F0267_MOVE_GetMoveResult_CPSCE","G0310_i_DisabledMovementTicks"],"decision":"Accepted turns/steps mutate party facing/position and set the source movement cooldown."},
    {"id":"main_loop_draw_after_input","file":"GAMELOOP.C","lines":"80-90,164-168,215","function":"F0002_MAIN_GameLoop_CPSDF","needles":["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);","F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());","F0380_COMMAND_ProcessQueue_CPSC();"],"decision":"The main loop drains input into F0361/F0380 and uses F0128 as the redraw seam for the current party tuple."},
    {"id":"redraw_to_viewport_present","file":"DUNVIEW.C","lines":"8336-8611","function":"F0128_DUNGEONVIEW_Draw_CPSF","needles":["G0296_puc_Bitmap_Viewport","F0127_DUNGEONVIEW_DrawSquareD0C","F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"],"decision":"F0128 composes the dungeon viewport into G0296 and then calls F0097 for presentation."},
    {"id":"pc34_viewport_blit","file":"DRAWVIEW.C","lines":"709-858","function":"F0097_DUNGEONVIEW_DrawViewport","needles":["void F0097_DUNGEONVIEW_DrawViewport","F0638_GetZone(C007_ZONE_VIEWPORT","(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);"],"decision":"F0097 is the PC34 viewport-present seam through video-driver slot 9."},
]

PRIOR_ARTIFACTS = {
    "pass331": {"manifest":"parity-evidence/verification/pass331_dm1_v1_route_to_viewport_redraw_path/manifest.json","expected":"BLOCKED_PASS331_ROUTE_KEYS_NOT_COMMAND_QUEUE","role":"old original/DOS route-key blocker before the Firestaff launcher route was proven"},
    "pass349": {"manifest":"parity-evidence/verification/pass349_dm1_v1_full_launcher_keypad_runtime_route/manifest.json","expected":"FULL_LAUNCHER_KEYPAD_RUNTIME_ROUTE_PROVED","role":"full product launcher-to-live-movement proof for Firestaff M11"},
    "pass351": {"manifest":"parity-evidence/verification/pass351_dm1_v1_live_viewport_redraw_parity_sweep/manifest.json","expected":"PASS_DM1_V1_LIVE_VIEWPORT_REDRAW_PARITY_SWEEP","role":"source-locked live route to movement to viewport redraw sweep"},
    "pass352": {"manifest":"parity-evidence/verification/pass352_dm1_v1_movement_route_regression_matrix/manifest.json","expected":"PASS_DM1_V1_MOVEMENT_ROUTE_REGRESSION_MATRIX_CONSOLIDATED","role":"route-surface regression matrix after pass349/pass356 integration"},
    "pass357": {"manifest":"parity-evidence/verification/pass357_dm1_v1_original_runtime_true_stop_control_blocker/manifest.json","expected":"BLOCKED_PASS357_ORIGINAL_RUNTIME_TRUE_STOP_CONTROL_REQUIRED","role":"current original FIRES true-stop/presentation blocker"},
    "pass358": {"manifest":"parity-evidence/verification/pass358_dm1_v1_touch_source_order_runtime_sweep/manifest.json","expected":"PASS_DM1_V1_TOUCH_SOURCE_ORDER_RUNTIME_SWEEP","role":"touch source-order/runtime route guard"},
}

PRODUCT_MARKERS = [
    ("main_loop_m11.c", "kp4"),
    ("main_loop_m11.c", "SDLK_KP_4"),
    ("main_loop_m11.c", "M11_GameView_HandleInput(&gameView, input)"),
    ("m11_game_view.c", "DM1_V1_MovementPipeline_EnqueueCommandPc34Compat"),
    ("m11_game_view.c", "DM1_V1_MovementPipeline_ProcessOneTickPc34Compat"),
    ("touch_click_zone_matrix_pc34_compat.c", "TOUCHCLICK_Compat_HitTestPrimaryThenSecondary"),
    ("touch_pointer_input_pc34_compat.c", "DM1_V1_InputCommandQueue_EnqueueMouseCommandPc34Compat"),
]

def run(cmd: list[str]) -> dict[str, Any]:
    p = subprocess.run(cmd, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    return {"cmd": cmd, "returncode": p.returncode, "outputTail": p.stdout[-2000:]}

def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            start, end = [int(x) for x in part.split("-", 1)]
        else:
            start = end = int(part)
        chunks.append("\n".join(lines[start - 1:end]))
    return "\n".join(chunks)

def compact_contains(blob: str, needle: str) -> bool:
    return " ".join(needle.split()) in " ".join(blob.split())

def audit_sources() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for lock in SOURCE_LOCKS:
        path = REDMCSB / lock["file"]
        text = source_window(path, lock["lines"]) if path.exists() else ""
        missing = [n for n in lock["needles"] if not compact_contains(text, n)]
        rows.append({**lock, "path": str(path), "ok": path.exists() and not missing, "missing": missing})
    return rows

def read_status(rel: str) -> tuple[bool, str | None, dict[str, Any]]:
    path = ROOT / rel
    if not path.exists():
        return False, None, {}
    data = json.loads(path.read_text(encoding="utf-8"))
    return True, data.get("status"), data

def audit_priors() -> dict[str, Any]:
    rows: dict[str, Any] = {}
    for name, spec in PRIOR_ARTIFACTS.items():
        exists, status, data = read_status(spec["manifest"])
        if name == "pass351" and exists and status is None:
            vr = data.get("verifier_result", {})
            status = spec["expected"] if (vr.get("returncode") == 0 or vr.get("failed") == 0) else None
        rows[name] = {"manifest": spec["manifest"], "exists": exists, "status": status, "expected": spec["expected"], "role": spec["role"], "ok": exists and status == spec["expected"], "head": data.get("head")}
    return rows

def audit_product_markers() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for rel, marker in PRODUCT_MARKERS:
        path = ROOT / rel
        text = path.read_text(encoding="utf-8", errors="replace") if path.exists() else ""
        rows.append({"file": rel, "marker": marker, "ok": marker in text})
    return rows

def build_manifest() -> dict[str, Any]:
    source = audit_sources()
    priors = audit_priors()
    products = audit_product_markers()
    ok = all(r["ok"] for r in source) and all(r["ok"] for r in priors.values()) and all(r["ok"] for r in products)
    return {"schema": f"{PASS}.v1", "timestampUtc": datetime.now(timezone.utc).isoformat(), "status": EXPECTED_STATUS if ok else "FAIL_DM1_V1_MOVEMENT_ROUTE_RUNTIME_BLOCKER_CLASSIFICATION", "repo": str(ROOT), "branch": run(["git", "branch", "--show-current"])["outputTail"].strip(), "head": run(["git", "rev-parse", "HEAD"])["outputTail"].strip(), "sourceRoot": str(REDMCSB), "sourceAudit": source, "priorArtifactAudit": priors, "productMarkerAudit": products, "classification": {"firestaffMovementRoute": "unblocked/proved by pass349 plus pass351/pass352/pass358 guards", "retiredOrNarrowedBlockers": ["pass331 route-key blocker remains historical/original-DOS-route evidence, not the active Firestaff M11 route blocker", "pass333/pass335 keypad-symbol blockers are retired for Firestaff --script key:kp* routing by pass349/pass352"], "activeBlocker": "original FIRES strict true-stop sequence: F0128_DUNGEONVIEW_Draw_CPSF then F0097_DUNGEONVIEW_DrawViewport or VIDRV_09_BlitViewPort in one bounded controlled run", "mustNotPromote": ["BPLIST/setup echo", "static offsets alone", "screenshots without runtime code-stop sequence", "Firestaff-side route proof as original DOS pixel parity"]}, "notClaimed": ["new original DOSBox/FIRES debugger hit", "F0128->F0097 true-stop transcript", "original-vs-Firestaff pixel parity"]}

def write_report(manifest: dict[str, Any]) -> None:
    lines = ["# Pass359 — DM1 V1 movement-route/runtime blocker follow-up", "", f"Status: `{manifest['status']}`", "", "## Verdict", "", "The Firestaff M11 movement route is no longer the active blocker: pass349 proves full launcher-to-live movement, pass351/pass352 keep redraw and route regressions locked, and pass358 keeps touch source-order locked. The remaining blocker is narrower: original FIRES/DOS runtime control still needs a strict true-stop sequence from F0128 to F0097/VIDRV before any original-vs-Firestaff viewport comparator can be promoted.", "", "## ReDMCSB source audit", ""]
    for row in manifest["sourceAudit"]:
        lines.append(f"- `{row['file']}:{row['lines']}` — `{row['function']}` — {row['decision']} ok=`{row['ok']}`")
    lines += ["", "## Prior evidence classification", ""]
    for name, row in manifest["priorArtifactAudit"].items():
        lines.append(f"- `{name}`: `{row['status']}` (expected `{row['expected']}`) — {row['role']} ok=`{row['ok']}`")
    c = manifest["classification"]
    lines += ["", "## Classification", "", f"- Firestaff movement route: `{c['firestaffMovementRoute']}`", f"- Active blocker: `{c['activeBlocker']}`", "- Do not promote from: " + ", ".join(f"`{x}`" for x in c["mustNotPromote"]), "", "## Non-claims", ""]
    lines += [f"- {x}" for x in manifest["notClaimed"]]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    manifest = build_manifest()
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": manifest["status"], "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT))}, indent=2))
    return 0 if manifest["status"] == EXPECTED_STATUS else 1

if __name__ == "__main__":
    raise SystemExit(main())
