#!/usr/bin/env python3
"""Pass495: reconcile DM1 V1 F0365/F0366 runtime-stop evidence with static original-capture blockers."""
from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS = "pass495_dm1_v1_f0365_f0366_runtime_static_boundary"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
STATUS = "PASS495_F0365_F0366_RUNTIME_STOPS_PROVEN_ORIGINAL_STATIC_CAPTURE_STILL_BLOCKED"

SOURCE_REFS = [
    {"id": "game_loop_wait_wrap", "file": "GAMELOOP.C", "lines": "150-219", "function": "F0002_MAIN_GameLoop_CPSDF", "claim": "the game loop drains keyboard input, runs F0380, and exits the wait loop only after stop/tick state permits the next draw", "needles": ["while (M527_IsCharacterInKeyboardBuffer())", "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());", "F0380_COMMAND_ProcessQueue_CPSC();", "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"]},
    {"id": "pc34_movement_key_table", "file": "COMMAND.C", "lines": "636-685", "function": "G0459_as_Graphic561_SecondaryKeyboardInput_Movement", "claim": "PC34/I34E movement keys resolve to C001/C002 turn and C003..C006 movement command ids", "needles": ["G0459_as_Graphic561_SecondaryKeyboardInput_Movement", "MEDIA707_I34E_I34M", "C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT", "C006_COMMAND_MOVE_LEFT", "C005_COMMAND_MOVE_BACKWARD", "C004_COMMAND_MOVE_RIGHT"]},
    {"id": "keyboard_queue_write", "file": "COMMAND.C", "lines": "1709-1813", "function": "F0361_COMMAND_ProcessKeyPress", "claim": "keyboard input is table-resolved into G0432_as_CommandQueue before F0380 consumes it", "needles": ["void F0361_COMMAND_ProcessKeyPress", "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;", "G2153_i_QueuedCommandsCount++", "F0360_COMMAND_ProcessPendingClick();"]},
    {"id": "f0380_pop_load_dispatch", "file": "COMMAND.C", "lines": "2045-2156", "function": "F0380_COMMAND_ProcessQueue_CPSC", "claim": "F0380 gates movement cooldown, pop-loads one queued command, then dispatches turns to F0365 and steps to F0366", "needles": ["void F0380_COMMAND_ProcessQueue_CPSC", "if (G2153_i_QueuedCommandsCount == 0)", "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;", "G0310_i_DisabledMovementTicks", "G2153_i_QueuedCommandsCount--;", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"]},
    {"id": "f0365_turn_stop", "file": "CLIKMENU.C", "lines": "142-174", "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty", "claim": "accepted turns set stop-wait and mutate party direction through the source turn handler", "needles": ["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0276_SENSOR_ProcessThingAdditionOrRemoval", "F0284_CHAMPION_SetPartyDirection"]},
    {"id": "f0366_step_stop_or_block", "file": "CLIKMENU.C", "lines": "180-347", "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty", "claim": "accepted steps compute a relative destination, commit through F0267, and set movement cooldown; blocked steps discard input before the commit path", "needles": ["void F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0357_COMMAND_DiscardAllInput();", "F0267_MOVE_GetMoveResult_CPSCE", "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;"]},
]

REQUIRED_RUNTIME = {
    "postGameplayRuntimeRan": True,
    "f0361HitAfterArm": True,
    "g2153IncrementObserved": True,
    "f0380PopLoadAfterQueueWriteObserved": True,
    "g2153DecrementPopLoadObserved": True,
    "f0365OrF0366DispatchObserved": True,
    "sourceAuditOk": True,
}


def compact(text: str) -> str:
    return " ".join(text.split())


def source_text(file_name: str, ranges: str) -> str:
    path = RED / file_name
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    out: list[str] = []
    for part in ranges.split(","):
        lo, hi = [int(x) for x in part.split("-", 1)]
        out.extend(lines[lo - 1:hi])
    return "\n".join(out)


def audit_source(ref: dict) -> dict:
    text = compact(source_text(ref["file"], ref["lines"]))
    missing = [needle for needle in ref["needles"] if compact(needle) not in text]
    return {k: ref[k] for k in ("id", "file", "lines", "function", "claim")} | {"ok": not missing, "missing": missing}


def load_manifest(rel: str) -> dict:
    path = ROOT / rel
    if not path.exists():
        raise AssertionError(f"missing manifest {rel}")
    return json.loads(path.read_text(encoding="utf-8"))


def load_pass475_evidence() -> dict:
    manifest = ROOT / "parity-evidence/verification/pass475_dm1_v1_f0365_f0366_runtime_followup/manifest.json"
    if manifest.exists():
        data = json.loads(manifest.read_text(encoding="utf-8"))
        data["evidencePath"] = str(manifest.relative_to(ROOT))
        return data
    report = ROOT / "parity-evidence/pass475_dm1_v1_f0365_f0366_runtime_followup.md"
    text = report.read_text(encoding="utf-8")
    status = "PASS475_FIRESTAFF_DM1_V1_INPUT_TO_F0365_F0366_ROUTE_CLOSED"
    required = [
        status,
        "COMMAND.C:2045-2156",
        "CLIKMENU.C:142-174",
        "CLIKMENU.C:180-347",
        "Original stock FIRES keyboard-buffer/debugger hit",
    ]
    missing = [item for item in required if item not in text]
    return {"status": status if not missing else "PASS475_REPORT_INCOMPLETE", "evidencePath": str(report.relative_to(ROOT)), "missing": missing}


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    source = [audit_source(ref) for ref in SOURCE_REFS]
    source_ok = all(row["ok"] for row in source)

    pass475 = load_pass475_evidence()
    pass391 = load_manifest("parity-evidence/verification/pass391_dm1_v1_queued_command_dispatch/manifest.json")
    pass487 = load_manifest("parity-evidence/verification/pass487_dm1_v1_original_click_capture_blocker/manifest.json")

    proof = pass391.get("proofPredicates", {})
    runtime_ok = pass391.get("status") == "PASS391_KEYBOARD_QUEUE_TO_F0380_DISPATCH_PROVEN" and all(proof.get(k) is v for k, v in REQUIRED_RUNTIME.items())
    firestaff_route_ok = pass475.get("status") == "PASS475_FIRESTAFF_DM1_V1_INPUT_TO_F0365_F0366_ROUTE_CLOSED"

    blocker = pass487.get("blockerFindings", {})
    static_duplicate_blocker_ok = (
        pass487.get("status") == "PASS487_ORIGINAL_CLICK_ROUTE_REACHES_GAMEPLAY_STILL_LABEL_BLOCKED"
        and blocker.get("firstFrameStillEntranceMenu") is True
        and blocker.get("postEntryGameplayHashRepeated") is True
        and blocker.get("postEntryStaticNoStateDelta") is True
        and blocker.get("trueStopClassification") == "static_no_state_delta_after_entrance_not_movement_processor_stop"
    )

    problems: list[str] = []
    if not source_ok:
        problems.append("ReDMCSB source audit failed")
    if not firestaff_route_ok:
        problems.append("pass475 Firestaff route closure is absent or changed")
    if not runtime_ok:
        problems.append("pass391 runtime stop predicates no longer prove queue-to-dispatch")
    if not static_duplicate_blocker_ok:
        problems.append("pass487/pass494 static duplicate blocker is absent or changed")

    payload = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": STATUS,
        "ok": not problems,
        "sourceRoot": str(RED),
        "sourceAudit": source,
        "firestaffRouteEvidence": {"evidence": pass475.get("evidencePath"), "status": pass475.get("status"), "ok": firestaff_route_ok},
        "runtimeStopEvidence": {"manifest": "parity-evidence/verification/pass391_dm1_v1_queued_command_dispatch/manifest.json", "status": pass391.get("status"), "requiredPredicates": REQUIRED_RUNTIME, "observedPredicates": {k: proof.get(k) for k in REQUIRED_RUNTIME}, "f0380EntryBreakpointObservedAfterQueueWrite": proof.get("f0380EntryBreakpointObservedAfterQueueWrite"), "ok": runtime_ok, "interpretation": "F0380 entry itself is diagnostic here; the promoted predicate is the post-queue-write pop/load/decrement plus F0365/F0366 dispatch stop chain."},
        "staticDuplicateEvidence": {"manifest": "parity-evidence/verification/pass487_dm1_v1_original_click_capture_blocker/manifest.json", "status": pass487.get("status"), "firstFrameStillEntranceMenu": blocker.get("firstFrameStillEntranceMenu"), "postEntryGameplayHashRepeated": blocker.get("postEntryGameplayHashRepeated"), "postEntryStaticNoStateDelta": blocker.get("postEntryStaticNoStateDelta"), "trueStopClassification": blocker.get("trueStopClassification"), "ok": static_duplicate_blocker_ok},
        "decision": "Do not spend more Firestaff-route work on F0365/F0366; that seam is closed. The active blocker is original capture promotion: pass487/pass494 still classify the fresh click capture as entrance-first plus repeated static/no-state-delta post-entry frames, not source-visible movement state evidence.",
        "nextUnblocker": "Capture or trace a fresh original run where the labeled post-entry frames are non-duplicate and source-bound to F0380 pop/load plus F0365/F0366 dispatch/stop evidence, then rerun pass80/pass487-family promotion gates.",
        "nonClaims": ["no original-vs-Firestaff pixel parity promotion", "no claim that pass487 static post-entry frames prove movement processor stops", "no new DOSBox/FIRES runtime capture was performed by this pass"],
        "problems": problems,
    }
    MANIFEST.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    observed_required = ", ".join(k for k, v in REQUIRED_RUNTIME.items() if proof.get(k) is v)
    lines = [
        "# Pass495 — DM1 V1 F0365/F0366 runtime/static boundary",
        "",
        f"Status: `{STATUS}`",
        "",
        "## Decision",
        payload["decision"],
        "",
        "## ReDMCSB source audit",
    ]
    for row in source:
        lines.append(f"- `{row['file']}:{row['lines']}` / `{row['function']}` — ok={row['ok']}; {row['claim']}")
    lines += [
        "",
        "## Runtime stop evidence",
        f"- pass475 Firestaff route closure: `{firestaff_route_ok}` (`{pass475.get('status')}`)",
        f"- pass391 post-gameplay queue-to-dispatch stop chain: `{runtime_ok}` (`{pass391.get('status')}`)",
        f"- pass391 required predicates: `{observed_required}`",
        f"- diagnostic F0380 entry breakpoint observed after queue write: `{proof.get('f0380EntryBreakpointObservedAfterQueueWrite')}`; pop/load/decrement/dispatch predicates are the promoted runtime evidence.",
        "",
        "## Static duplicate evidence from pass494/pass487",
        f"- pass487 classifier blocker retained: `{static_duplicate_blocker_ok}` (`{pass487.get('status')}`)",
        f"- first frame still entrance/menu: `{blocker.get('firstFrameStillEntranceMenu')}`",
        f"- post-entry gameplay hash repeated: `{blocker.get('postEntryGameplayHashRepeated')}`",
        f"- true-stop classification: `{blocker.get('trueStopClassification')}`",
        "",
        "## Next unblocker",
        payload["nextUnblocker"],
        "",
        "## Non-claims",
    ]
    lines += [f"- {item}" for item in payload["nonClaims"]]
    if problems:
        lines += ["", "## Problems"] + [f"- {p}" for p in problems]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

    if problems:
        for problem in problems:
            print(problem)
        return 1
    print(f"{STATUS} manifest={MANIFEST}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
