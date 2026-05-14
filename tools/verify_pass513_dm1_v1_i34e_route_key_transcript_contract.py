#!/usr/bin/env python3
from __future__ import annotations
from datetime import datetime, timezone
import json, os
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass513_dm1_v1_i34e_route_key_transcript_contract"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")))
STATUS = "BLOCKED_PASS513_DM1_V1_I34E_ROUTE_KEY_TRANSCRIPT_REQUIRED"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {"id":"i34e_crawcin_normalizes_extended_arrows_to_dm_chars","file":"IO2.C","lines":"27-61","function":"F0540_INPUT_Crawcin","needles":["L2944_ui_ = (*(G2162_IODriver->IODRV_00_GetKeyboardInput))();","if (L2944_ui_ == 0x0C53)","if (L2944_ui_ == 0x0410)","switch (L2944_ui_ - 0x1248)","L2944_ui_ = 'L'","L2944_ui_ = 'P'","L2944_ui_ = 'K'","L2944_ui_ = 'M'","return L2944_ui_;"],"claim":"the transcript must record the concrete M528/F0540 value after PC/I34E arrow normalization, not a host-side route label"},
    {"id":"i34e_secondary_movement_table_accepts_only_k_l_m_o_p_q","file":"COMMAND.C","lines":"636-685","function":"G0459_as_Graphic561_SecondaryKeyboardInput_Movement","needles":["KEYBOARD_INPUT G0459_as_Graphic561_SecondaryKeyboardInput_Movement","MEDIA707_I34E_I34M","{ C001_COMMAND_TURN_LEFT,     0x004B }","{ C003_COMMAND_MOVE_FORWARD,  0x004C }","{ C002_COMMAND_TURN_RIGHT,    0x004D }","{ C006_COMMAND_MOVE_LEFT,     0x004F }","{ C005_COMMAND_MOVE_BACKWARD, 0x0050 }","{ C004_COMMAND_MOVE_RIGHT,    0x0051 }"],"claim":"F0361 can queue movement only after the drained key matches the active I34E secondary keyboard table"},
    {"id":"game_loop_drains_keyboard_before_f0380","file":"GAMELOOP.C","lines":"164-219","function":"F0002_MAIN_GameLoop_CPSDF","needles":["G0321_B_StopWaitingForPlayerInput = C0_FALSE;","while (M527_IsCharacterInKeyboardBuffer())","F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());","F0380_COMMAND_ProcessQueue_CPSC();","while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"],"claim":"the original route chain is keyboard-buffer drain, queue processing, then wait-loop boundary"},
    {"id":"f0361_queue_write_and_count_delta_required","file":"COMMAND.C","lines":"1734-1812","function":"F0361_COMMAND_ProcessKeyPress","needles":["if ((L1112_ps_KeyboardInput = G0443_ps_PrimaryKeyboardInput) == NULL)","G0435_B_CommandQueueLocked = C1_TRUE;","while (L1111_i_Command = L1112_ps_KeyboardInput->Command)","if (P0728_KeyCode == L1112_ps_KeyboardInput->Code)","if ((L1112_ps_KeyboardInput = G0444_ps_SecondaryKeyboardInput) == NULL)","G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;","G2153_i_QueuedCommandsCount++;","G0435_B_CommandQueueLocked = C0_FALSE;","F0360_COMMAND_ProcessPendingClick();"],"claim":"a transcript must include F0361's table match, queue slot write, last-index write, and G2153 increment"},
    {"id":"f0380_pop_count_delta_and_dispatch_required","file":"COMMAND.C","lines":"2075-2127,2150-2156","function":"F0380_COMMAND_ProcessQueue_CPSC","needles":["G0435_B_CommandQueueLocked = C1_TRUE;","if (G2153_i_QueuedCommandsCount == 0)","L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;","if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)","G2153_i_QueuedCommandsCount--;","if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)","G0435_B_CommandQueueLocked = C0_FALSE;","F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);","F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"],"claim":"the same command must be observed leaving F0380 with first-index/count delta and turn/move dispatch"},
    {"id":"turn_and_step_handlers_mutate_source_state","file":"CLIKMENU.C","lines":"142-174,237-270,293-347","function":"F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty","needles":["G0321_B_StopWaitingForPlayerInput = C1_TRUE;","F0284_CHAMPION_SetPartyDirection","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement","F0357_COMMAND_DiscardAllInput();","F0267_MOVE_GetMoveResult_CPSCE","G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;","G0311_i_ProjectileDisabledMovementTicks = 0;"],"claim":"post-dispatch evidence must distinguish turn mutation, blocked-step discard, and successful-step movement/cooldown side effects"},
    {"id":"successful_step_commits_tuple_and_last_movement_time","file":"MOVESENS.C","lines":"738-818","function":"F0267_MOVE_GetMoveResult_CPSCE","needles":["G0397_i_MoveResultMapX = P0560_i_DestinationMapX;","G0398_i_MoveResultMapY = P0561_i_DestinationMapY;","G0399_ui_MoveResultMapIndex = L0715_ui_MapIndexDestination;","F0317_CHAMPION_AddScentStrength","G0362_l_LastPartyMovementTime = G0313_ul_GameTime;","F0276_SENSOR_ProcessThingAdditionOrRemoval"],"claim":"successful movement transcripts must bind the source-committed tuple and timing/sensor side effects"},
    {"id":"post_command_viewport_present_boundary","file":"DUNVIEW.C","lines":"8318-8611","function":"F0128_DUNGEONVIEW_Draw_CPSF","needles":["void F0128_DUNGEONVIEW_Draw_CPSF","P0183_i_Direction","P0184_i_MapX","P0185_i_MapY","F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"],"claim":"viewport/HUD captures are promotable only after F0128 consumes the post-command direction/X/Y tuple"},
    {"id":"pc34_viewport_blit_boundary","file":"DRAWVIEW.C","lines":"709-858","function":"F0097_DUNGEONVIEW_DrawViewport","needles":["void F0097_DUNGEONVIEW_DrawViewport","G0296_puc_Bitmap_Viewport","F0638_GetZone(C007_ZONE_VIEWPORT","VIDRV_09_BlitViewPort"],"claim":"route-labeled screenshots must be tied to the PC34 viewport present/blit boundary"},
]
PRIOR_GATES = {
    "pass504_keyboard_buffer_state_delta_blocker": ("parity-evidence/verification/pass504_dm1_v1_keyboard_buffer_state_delta_blocker/manifest.json", "PASS504_KEYBOARD_BUFFER_STATE_DELTA_BLOCKER_LOCKED"),
    "pass509_original_overlay_keyboard_buffer_blocker": ("parity-evidence/verification/pass509_dm1_v1_original_overlay_keyboard_buffer_blocker/manifest.json", "PASS509_ORIGINAL_OVERLAY_KEYBOARD_BUFFER_BLOCKER_LOCKED"),
    "pass511_movement_original_route_contract": ("parity-evidence/verification/pass511_dm1_v1_movement_original_route_contract/manifest.json", "PASS511_DM1_V1_MOVEMENT_ORIGINAL_ROUTE_CONTRACT_LOCKED"),
    "pass512_movement_cross_reference_audit": ("parity-evidence/verification/pass512_dm1_v1_movement_cross_reference_audit/manifest.json", "BLOCKED_PASS512_DM1_V1_MOVEMENT_CROSS_REFERENCE_AUDIT_GREATSTONE_DETAIL_PAGES_MISSING"),
}
REQUIRED_TRANSCRIPT_FIELDS = ["routeId","sampleIndex","inputSource","rawKeyCode","normalizedKeyCode","m527WasNonEmpty","m528Value","f0361Table","f0361Command","f0361QueueSlot","g0434Before","g0434After","g2153BeforeEnqueue","g2153AfterEnqueue","f0380Command","g0433Before","g0433After","g2153BeforePop","g2153AfterPop","dispatchHandler","partyBeforeMap","partyBeforeX","partyBeforeY","partyBeforeDir","partyAfterMap","partyAfterX","partyAfterY","partyAfterDir","blockedOrNoopReason","f0128Direction","f0128MapX","f0128MapY","f0097Presented","capturePath","captureSha256"]
ACCEPTED_KEY_ROWS = [
    {"normalizedKeyCode":"0x004B","ascii":"K","command":"C001_COMMAND_TURN_LEFT","handler":"F0365"},
    {"normalizedKeyCode":"0x004C","ascii":"L","command":"C003_COMMAND_MOVE_FORWARD","handler":"F0366"},
    {"normalizedKeyCode":"0x004D","ascii":"M","command":"C002_COMMAND_TURN_RIGHT","handler":"F0365"},
    {"normalizedKeyCode":"0x004F","ascii":"O","command":"C006_COMMAND_MOVE_LEFT","handler":"F0366"},
    {"normalizedKeyCode":"0x0050","ascii":"P","command":"C005_COMMAND_MOVE_BACKWARD","handler":"F0366"},
    {"normalizedKeyCode":"0x0051","ascii":"Q","command":"C004_COMMAND_MOVE_RIGHT","handler":"F0366"},
]
REJECT_AS_NON_PROMOTABLE = ["route labels without rawKeyCode plus normalizedKeyCode plus M528 value","F0361 entry/exit records without G0432 slot, G0434 delta, and G2153 increment","F0380 records where G2153 is zero, command is gated by movement cooldown, or no matching pop/decrement occurs","state-delta screenshots lacking the preceding F0365/F0366 handler and F0128/F0097 boundary","repeated capture hashes unless the transcript proves a source-owned blocked/no-op route"]

def compact(text: str) -> str: return " ".join(text.split())
def read_text(path: Path, encoding: str = "utf-8") -> str:
    if not path.exists(): raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=encoding, errors="replace")
def source_window(path: Path, spec: str) -> str:
    lines = read_text(path, "latin-1").splitlines(); out = []
    for part in spec.split(","):
        start, end = [int(x) for x in part.split("-", 1)]
        out.extend(lines[start - 1:end])
    return "\n".join(out)
def audit_source() -> list[dict[str, Any]]:
    rows = []
    for lock in SOURCE_LOCKS:
        path = RED / lock["file"]; text = source_window(path, lock["lines"]) if path.exists() else ""; flat = compact(text)
        missing = [needle for needle in lock["needles"] if compact(needle) not in flat]
        row = dict(lock); row["path"] = str(path); row["ok"] = path.exists() and not missing; row["missing"] = missing; row.pop("needles", None); rows.append(row)
    return rows
def prior_gate_rows() -> dict[str, dict[str, Any]]:
    rows = {}
    for name, (rel, expected) in PRIOR_GATES.items():
        path = ROOT / rel; status = json.loads(read_text(path)).get("status") if path.exists() else None
        rows[name] = {"path": rel, "expectedStatus": expected, "status": status, "ok": status == expected}
    return rows
def transcript_candidates() -> list[dict[str, Any]]:
    roots = [ROOT / "parity-evidence", ROOT / "verification-screens"]; hints = ("pass513","i34e","route-key","route_key","keyboard-buffer","keyboard_buffer","f0380"); rows = []
    for root in roots:
        if not root.exists(): continue
        for path in root.rglob("*"):
            if not path.is_file(): continue
            rel = path.relative_to(ROOT); text = str(rel).lower()
            if PASS in text or not any(hint in text for hint in hints): continue
            rows.append({"path": str(rel), "size": path.stat().st_size})
    return sorted(rows, key=lambda item: item["path"])
def build_payload() -> dict[str, Any]:
    source = audit_source(); priors = prior_gate_rows(); candidates = transcript_candidates(); problems = []
    problems.extend(f"source lock failed {row['file']}:{row['lines']} missing={row['missing']}" for row in source if not row["ok"])
    problems.extend(f"prior gate failed {name}: {row['status']} != {row['expectedStatus']}" for name, row in priors.items() if not row["ok"])
    return {"schema": f"firestaff.parity.{PASS}.v1", "timestampUtc": datetime.now(timezone.utc).isoformat(), "status": STATUS if not problems else "FAIL_PASS513_DM1_V1_I34E_ROUTE_KEY_TRANSCRIPT_CONTRACT", "ok": not problems, "sourceRoot": str(RED), "scope": "DM1 V1 movement/forflyttning original PC/I34E keyboard-buffer route-key transcript acceptance contract", "sourceAudit": source, "priorGates": priors, "acceptedKeyRows": ACCEPTED_KEY_ROWS, "requiredTranscriptFields": REQUIRED_TRANSCRIPT_FIELDS, "minimumPromotableTranscript": {"turnRows": 1, "successfulStepRows": 1, "blockedOrNoopRows": 1, "allRowsMustBind": ["M527 non-empty before M528 read", "M528/F0540 normalized value equals a COMMAND.C I34E movement table code", "F0361 writes that command into G0432 and increments G2153", "F0380 pops the same command and decrements G2153", "F0365 or F0366 is reached for that command", "F0128/F0097 consumes and presents the matching post-command party tuple before capture"]}, "rejectAsNonPromotable": REJECT_AS_NON_PROMOTABLE, "candidateTranscriptLikeArtifacts": candidates, "decision": "The remaining blocker is not another Firestaff movement implementation patch. It is a missing original PC/I34E route-key transcript with enough source-visible state to bind keyboard-buffer token, command queue delta, F0380 pop/dispatch, party tuple delta, and viewport present boundary.", "nonClaims": ["no DOSBox/FIRES/original runtime capture was launched by this verifier", "no original-vs-Firestaff pixel parity is claimed", "no runtime movement code is changed", "candidate transcript-like files are listed for triage only and are not promoted"], "problems": problems}
def write_report(payload: dict[str, Any]) -> None:
    lines = ["# Pass513 - DM1 V1 I34E route-key transcript contract", "", "Status: " + payload["status"], "", "## Decision", "", payload["decision"], "", "## ReDMCSB source audit", ""]
    for row in payload["sourceAudit"]:
        state = "PASS" if row["ok"] else "FAIL"; lines.append(f"- {state} {row['file']}:{row['lines']} / {row['function']} - {row['claim']}")
    lines += ["", "## Required prior gates", ""]
    for name, row in payload["priorGates"].items():
        state = "PASS" if row["ok"] else "FAIL"; lines.append(f"- {state} {name}: {row['status']}")
    lines += ["", "## Accepted I34E key rows", ""]
    for row in payload["acceptedKeyRows"]: lines.append(f"- {row['normalizedKeyCode']} ({row['ascii']}) -> {row['command']} -> {row['handler']}")
    lines += ["", "## Required transcript fields", ""]; lines.extend(f"- {field}" for field in payload["requiredTranscriptFields"])
    lines += ["", "## Reject as non-promotable", ""]; lines.extend(f"- {item}" for item in payload["rejectAsNonPromotable"])
    lines += ["", "## Candidate transcript-like artifacts", ""]
    if payload["candidateTranscriptLikeArtifacts"]:
        lines.extend(f"- {row['path']} size={row['size']}" for row in payload["candidateTranscriptLikeArtifacts"])
    else:
        lines.append("- none found")
    lines += ["", "## Non-claims", ""]; lines.extend(f"- {item}" for item in payload["nonClaims"])
    lines += ["", "## Gate", "", "- python3 tools/verify_pass513_dm1_v1_i34e_route_key_transcript_contract.py", "", f"Manifest: {MANIFEST.relative_to(ROOT)}"]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True); payload = build_payload()
    MANIFEST.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8"); write_report(payload)
    print(json.dumps({"status": payload["status"], "ok": payload["ok"], "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT)), "problems": payload["problems"]}, indent=2, sort_keys=True))
    return 0 if payload["ok"] else 1
if __name__ == "__main__": raise SystemExit(main())
