#!/usr/bin/env python3
from __future__ import annotations
from datetime import datetime, timezone
import hashlib
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
PROMOTED_STATUS = "PASS513_DM1_V1_I34E_ROUTE_KEY_TRANSCRIPT_PROMOTABLE"
TRANSCRIPT_ENV = "FIRESTAFF_PASS513_TRANSCRIPT"

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
SCAFFOLD_RUNTIME_FIELDS = ["m527WasNonEmpty","f0361QueueSlot","g0434Before","g0434After","g2153BeforeEnqueue","g2153AfterEnqueue","g0433Before","g0433After","g2153BeforePop","g2153AfterPop","partyBeforeMap","partyBeforeX","partyBeforeY","partyBeforeDir","partyAfterMap","partyAfterX","partyAfterY","partyAfterDir","blockedOrNoopReason","f0128Direction","f0128MapX","f0128MapY","f0097Presented"]
REQUIRED_TRANSCRIPT_COUNTS = {"turnRows": 1, "successfulStepRows": 1, "blockedOrNoopRows": 1}

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
def as_int(value: Any) -> int | None:
    if isinstance(value, bool):
        return None
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        text = value.strip()
        if len(text) == 1:
            return ord(text)
        try:
            return int(text, 0)
        except ValueError:
            return None
    return None
def same_code(left: Any, right: Any) -> bool:
    return as_int(left) == as_int(right)
def int_delta(after: Any, before: Any, delta: int) -> bool:
    before_int = as_int(before)
    after_int = as_int(after)
    return before_int is not None and after_int is not None and after_int == before_int + delta
def truthy(value: Any) -> bool:
    if isinstance(value, bool):
        return value
    if isinstance(value, str):
        return value.strip().lower() in {"1", "true", "yes", "y", "presented"}
    return bool(value)
def capture_path(path_text: Any, transcript_path: Path) -> Path | None:
    if not isinstance(path_text, str) or not path_text.strip():
        return None
    path = Path(path_text)
    if path.is_absolute():
        return path
    for base in (ROOT, transcript_path.parent):
        candidate = base / path
        if candidate.exists():
            return candidate
    return ROOT / path
def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()
def command_for_code(code: Any) -> dict[str, str] | None:
    code_int = as_int(code)
    for row in ACCEPTED_KEY_ROWS:
        if as_int(row["normalizedKeyCode"]) == code_int:
            return row
    return None
def transcript_rows(payload: Any) -> list[dict[str, Any]]:
    if isinstance(payload, list):
        return payload
    if isinstance(payload, dict):
        for key in ("rows", "transcriptRows", "samples"):
            rows = payload.get(key)
            if isinstance(rows, list):
                return rows
    return []
def scaffold_missing_runtime_fields(payload: Any, rows: list[dict[str, Any]]) -> list[str]:
    missing: set[str] = set()
    if isinstance(payload, dict):
        schema = str(payload.get("schema", ""))
        status = str(payload.get("status", ""))
        if "scaffold" in schema.lower() or "SCAFFOLD_ONLY" in status:
            missing.update(SCAFFOLD_RUNTIME_FIELDS)
        for field in payload.get("missingOriginalRuntimeFields", []):
            if isinstance(field, str):
                missing.add(field)
    for row in rows:
        if not isinstance(row, dict):
            continue
        if row.get("scaffoldOnly"):
            missing.update(SCAFFOLD_RUNTIME_FIELDS)
        for field in row.get("missingOriginalRuntimeFields", []):
            if isinstance(field, str):
                missing.add(field)
    missing.discard("routeId")
    missing.discard("sampleIndex")
    missing.discard("inputSource")
    return sorted(missing)
def validate_transcript(path_text: str | None) -> dict[str, Any]:
    result: dict[str, Any] = {"provided": bool(path_text), "path": path_text, "ok": False, "status": "not_provided", "problems": [], "rowCount": 0, "scaffoldOnly": False, "missingRuntimeFields": [], "counts": {"turnRows": 0, "successfulStepRows": 0, "blockedOrNoopRows": 0}}
    if not path_text:
        return result
    path = Path(path_text)
    if not path.is_absolute():
        path = ROOT / path
    result["path"] = str(path)
    if not path.exists():
        result["status"] = "missing"
        result["problems"].append(f"transcript does not exist: {path}")
        return result
    try:
        payload = json.loads(read_text(path))
    except json.JSONDecodeError as exc:
        result["status"] = "invalid_json"
        result["problems"].append(f"transcript JSON parse failed: {exc}")
        return result
    rows = transcript_rows(payload)
    result["rowCount"] = len(rows)
    if not rows:
        result["problems"].append("transcript must be a JSON array or object with rows/transcriptRows/samples")
    scaffold_missing = scaffold_missing_runtime_fields(payload, rows)
    if scaffold_missing:
        result["scaffoldOnly"] = True
        result["status"] = "scaffold_only"
        result["missingRuntimeFields"] = scaffold_missing
        result["problems"].append("transcript is scaffold-only; replace with debugger-observed original PC/I34E runtime fields: " + ", ".join(scaffold_missing))
    seen_hashes: dict[str, list[int]] = {}
    for index, row in enumerate(rows):
        prefix = f"row[{index}]"
        if not isinstance(row, dict):
            result["problems"].append(f"{prefix}: row is not an object")
            continue
        row_scaffold_missing = scaffold_missing_runtime_fields({}, [row])
        if row_scaffold_missing:
            result["problems"].append(f"{prefix}: scaffold-only row lacks debugger-observed fields {row_scaffold_missing}")
            continue
        missing = [field for field in REQUIRED_TRANSCRIPT_FIELDS if field not in row]
        if missing:
            result["problems"].append(f"{prefix}: missing required fields {missing}")
            continue
        expected = command_for_code(row["normalizedKeyCode"])
        if expected is None:
            result["problems"].append(f"{prefix}: normalizedKeyCode is not an accepted I34E movement key: {row['normalizedKeyCode']!r}")
            continue
        if not truthy(row["m527WasNonEmpty"]):
            result["problems"].append(f"{prefix}: m527WasNonEmpty must prove keyboard buffer was non-empty before M528/F0540")
        if not same_code(row["m528Value"], row["normalizedKeyCode"]):
            result["problems"].append(f"{prefix}: m528Value must equal normalizedKeyCode after F0540 normalization")
        if row["f0361Command"] != expected["command"] or row["f0380Command"] != expected["command"]:
            result["problems"].append(f"{prefix}: F0361/F0380 command must both equal {expected['command']}")
        if expected["handler"] not in str(row["dispatchHandler"]):
            result["problems"].append(f"{prefix}: dispatchHandler must include {expected['handler']}")
        if not int_delta(row["g2153AfterEnqueue"], row["g2153BeforeEnqueue"], 1):
            result["problems"].append(f"{prefix}: G2153 enqueue count must increment by one")
        if not int_delta(row["g2153AfterPop"], row["g2153BeforePop"], -1):
            result["problems"].append(f"{prefix}: G2153 pop count must decrement by one")
        if as_int(row["g0434After"]) == as_int(row["g0434Before"]):
            result["problems"].append(f"{prefix}: G0434 last-index must change on F0361 queue write")
        if as_int(row["g0433After"]) == as_int(row["g0433Before"]):
            result["problems"].append(f"{prefix}: G0433 first-index must change on F0380 pop")
        if not truthy(row["f0097Presented"]):
            result["problems"].append(f"{prefix}: f0097Presented must be true after F0128/F0097 present boundary")
        if not (same_code(row["f0128Direction"], row["partyAfterDir"]) and as_int(row["f0128MapX"]) == as_int(row["partyAfterX"]) and as_int(row["f0128MapY"]) == as_int(row["partyAfterY"])):
            result["problems"].append(f"{prefix}: F0128 tuple must equal post-command party tuple")
        blocked_reason = str(row["blockedOrNoopReason"]).strip()
        same_square = (as_int(row["partyBeforeMap"]) == as_int(row["partyAfterMap"]) and as_int(row["partyBeforeX"]) == as_int(row["partyAfterX"]) and as_int(row["partyBeforeY"]) == as_int(row["partyAfterY"]))
        same_dir = as_int(row["partyBeforeDir"]) == as_int(row["partyAfterDir"])
        if expected["handler"] == "F0365":
            if not same_square or same_dir:
                result["problems"].append(f"{prefix}: F0365 turn row must keep map/X/Y and change direction")
            result["counts"]["turnRows"] += 1
        elif blocked_reason:
            if not same_square or not same_dir:
                result["problems"].append(f"{prefix}: blocked/no-op row must keep party tuple stable")
            result["counts"]["blockedOrNoopRows"] += 1
        else:
            if same_square and same_dir:
                result["problems"].append(f"{prefix}: successful F0366 row must change position or direction, or provide blockedOrNoopReason")
            result["counts"]["successfulStepRows"] += 1
        capture = capture_path(row["capturePath"], path)
        if capture is None or not capture.exists():
            result["problems"].append(f"{prefix}: capturePath does not exist: {row['capturePath']!r}")
        else:
            actual_sha = sha256_file(capture)
            if str(row["captureSha256"]).lower() != actual_sha:
                result["problems"].append(f"{prefix}: captureSha256 mismatch for {capture}: {actual_sha}")
            seen_hashes.setdefault(actual_sha, []).append(index)
    for name, minimum in REQUIRED_TRANSCRIPT_COUNTS.items():
        if result["counts"][name] < minimum:
            result["problems"].append(f"transcript needs at least {minimum} {name}, found {result['counts'][name]}")
    for digest, indexes in seen_hashes.items():
        if len(indexes) > 1:
            repeated_without_blocker = [i for i in indexes if not str(rows[i].get("blockedOrNoopReason", "")).strip()]
            if repeated_without_blocker:
                result["problems"].append(f"capture hash {digest} repeats in rows {indexes}; repeats require blockedOrNoopReason on every repeated row")
    result["ok"] = not result["problems"]
    result["status"] = "promotable" if result["ok"] else ("scaffold_only" if result["scaffoldOnly"] else "non_promotable")
    return result
def build_payload() -> dict[str, Any]:
    source = audit_source(); priors = prior_gate_rows(); candidates = transcript_candidates(); problems = []
    problems.extend(f"source lock failed {row['file']}:{row['lines']} missing={row['missing']}" for row in source if not row["ok"])
    problems.extend(f"prior gate failed {name}: {row['status']} != {row['expectedStatus']}" for name, row in priors.items() if not row["ok"])
    transcript = validate_transcript(os.environ.get(TRANSCRIPT_ENV))
    if transcript["provided"] and not transcript["ok"]:
        problems.extend(transcript["problems"])
    status = "FAIL_PASS513_DM1_V1_I34E_ROUTE_KEY_TRANSCRIPT_CONTRACT" if problems else (PROMOTED_STATUS if transcript["ok"] else STATUS)
    return {"schema": f"firestaff.parity.{PASS}.v2", "timestampUtc": datetime.now(timezone.utc).isoformat(), "status": status, "ok": not problems, "sourceRoot": str(RED), "scope": "DM1 V1 movement/forflyttning original PC/I34E keyboard-buffer route-key transcript acceptance contract", "sourceAudit": source, "priorGates": priors, "acceptedKeyRows": ACCEPTED_KEY_ROWS, "requiredTranscriptFields": REQUIRED_TRANSCRIPT_FIELDS, "minimumPromotableTranscript": {"counts": REQUIRED_TRANSCRIPT_COUNTS, "allRowsMustBind": ["M527 non-empty before M528 read", "M528/F0540 normalized value equals a COMMAND.C I34E movement table code", "F0361 writes that command into G0432 and increments G2153", "F0380 pops the same command and decrements G2153", "F0365 or F0366 is reached for that command", "F0128/F0097 consumes and presents the matching post-command party tuple before capture", "capturePath exists and captureSha256 matches the captured bytes"]}, "transcriptValidation": transcript, "transcriptValidationCommand": f"{TRANSCRIPT_ENV}=path/to/transcript.json python3 tools/verify_pass513_dm1_v1_i34e_route_key_transcript_contract.py", "rejectAsNonPromotable": REJECT_AS_NON_PROMOTABLE, "candidateTranscriptLikeArtifacts": candidates, "decision": "The remaining blocker is not another Firestaff movement implementation patch. It is a missing original PC/I34E route-key transcript with enough source-visible state to bind keyboard-buffer token, command queue delta, F0380 pop/dispatch, party tuple delta, and viewport present boundary.", "nonClaims": ["no DOSBox/FIRES/original runtime capture was launched by this verifier", "no original-vs-Firestaff pixel parity is claimed", "no runtime movement code is changed", "candidate transcript-like files are listed for triage only and are not promoted"], "problems": problems}
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
    lines += ["", "## Machine-checkable transcript validation", "", f"- Optional promotion gate: {payload['transcriptValidationCommand']}", f"- Provided: {payload['transcriptValidation']['provided']}", f"- Validation status: {payload['transcriptValidation']['status']}"]
    lines.extend(f"- Minimum {name}: {count}" for name, count in payload["minimumPromotableTranscript"]["counts"].items())
    lines.extend(f"- Binding: {item}" for item in payload["minimumPromotableTranscript"]["allRowsMustBind"])
    if payload["transcriptValidation"]["problems"]:
        lines += ["", "## Transcript validation problems", ""]
        lines.extend(f"- {problem}" for problem in payload["transcriptValidation"]["problems"])
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
