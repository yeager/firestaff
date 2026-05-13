#!/usr/bin/env python3
"""Pass509: lock the DM1 V1 original overlay keyboard-buffer blocker."""
from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
VERIFY_DIR = ROOT / "parity-evidence/verification/pass509_dm1_v1_original_overlay_keyboard_buffer_blocker"
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/pass509_dm1_v1_original_overlay_keyboard_buffer_blocker.md"
STATUS = "PASS509_ORIGINAL_OVERLAY_KEYBOARD_BUFFER_BLOCKER_LOCKED"

PASS435 = ROOT / "parity-evidence/verification/pass435_dm1_v1_semantic_original_route_readiness_gate/manifest.json"
PASS497 = ROOT / "parity-evidence/verification/pass497_dm1_v1_original_capture_next_blocker/manifest.json"
PASS498 = ROOT / "parity-evidence/verification/pass498_dm1_v1_original_post_command_state_delta_boundary/manifest.json"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {"id": "game_loop_drains_keyboard_before_queue_dispatch", "file": "GAMELOOP.C", "lines": "164-219", "function": "F0002_MAIN_GameLoop_CPSDF", "needles": ["G0321_B_StopWaitingForPlayerInput = C0_FALSE;", "while (M527_IsCharacterInKeyboardBuffer())", "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());", "F0380_COMMAND_ProcessQueue_CPSC();", "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"], "claim": "route keys are drained into F0361 before F0380 and before the wait loop exits"},
    {"id": "discard_all_input_flushes_raw_keyboard_buffer", "file": "COMMAND.C", "lines": "1304-1326", "function": "F0357_COMMAND_DiscardAllInput", "needles": ["void F0357_COMMAND_DiscardAllInput", "while (M527_IsCharacterInKeyboardBuffer())", "M528_GetCharacterInKeyboardBuffer();"], "claim": "panel/rest/frozen transitions can flush pending keyboard input before it reaches the command queue"},
    {"id": "discard_all_input_preserves_only_specific_mouse_commands", "file": "COMMAND.C", "lines": "1327-1375", "function": "F0357_COMMAND_DiscardAllInput", "needles": ["G0435_B_CommandQueueLocked = C1_TRUE;", "C129_COMMAND_RELEASE_CHAMPION_ICON", "C254_COMMAND_STOP_PRESSING_EYE_MOUTH_WALL", "G0434_i_CommandQueueLastIndex = L2285_i_DestinationCommandQueueIndex;", "F0360_COMMAND_ProcessPendingClick();"], "claim": "discard keeps only release/eye-mouth queue entries, so packed keyboard tokens cannot be assumed to survive overlay boundaries"},
    {"id": "keyboard_queue_write_requires_active_table_and_capacity", "file": "COMMAND.C", "lines": "1734-1812", "function": "F0361_COMMAND_ProcessKeyPress", "needles": ["if ((L1112_ps_KeyboardInput = G0443_ps_PrimaryKeyboardInput) == NULL)", "G0435_B_CommandQueueLocked = C1_TRUE;", "if (G2153_i_QueuedCommandsCount < C5_UNKNOWN)", "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;", "G2153_i_QueuedCommandsCount++;", "F0360_COMMAND_ProcessPendingClick();"], "claim": "keyboard-driven overlay routes need evidence of F0361 queue write, not only host key labels"},
    {"id": "queue_pop_dispatch_boundary", "file": "COMMAND.C", "lines": "2045-2127", "function": "F0380_COMMAND_ProcessQueue_CPSC", "needles": ["void F0380_COMMAND_ProcessQueue_CPSC", "G0435_B_CommandQueueLocked = C1_TRUE;", "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;", "G2153_i_QueuedCommandsCount--;", "if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)", "F0360_COMMAND_ProcessPendingClick();"], "claim": "capture must be after a real F0380 pop/decrement/unlock path for the relevant route token"},
    {"id": "overlay_viewport_present_requires_f0128_f0097", "file": "DUNVIEW.C", "lines": "8318-8611", "function": "F0128_DUNGEONVIEW_Draw_CPSF", "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "P0183_i_Direction", "P0184_i_MapX", "P0185_i_MapY", "G0296_puc_Bitmap_Viewport", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"], "claim": "a route frame is overlay-eligible only when command-state reaches viewport compose/present"},
]


def norm(text: str) -> str:
    return " ".join(text.split())


def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    out: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            start, end = [int(x) for x in part.split("-", 1)]
        else:
            start = end = int(part)
        out.append("\n".join(lines[start - 1:end]))
    return "\n".join(out)


def source_row(lock: dict[str, Any]) -> dict[str, Any]:
    path = RED / lock["file"]
    text = source_window(path, lock["lines"]) if path.exists() else ""
    missing = [needle for needle in lock["needles"] if norm(needle) not in norm(text)]
    return {**lock, "path": str(path), "ok": path.exists() and not missing, "missing": missing}


def load(path: Path) -> dict[str, Any]:
    if not path.exists():
        return {"exists": False, "path": str(path.relative_to(ROOT))}
    return {"exists": True, "path": str(path.relative_to(ROOT)), **json.loads(path.read_text(encoding="utf-8"))}


def blocker_predicates(pass435: dict[str, Any], pass497: dict[str, Any], pass498: dict[str, Any]) -> dict[str, Any]:
    observed498 = pass498.get("observedBlockerState") or {}
    return {
        "pass435StillBlocksSemanticRoute": pass435.get("status") == "BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY",
        "pass435QuarantinesHistoricalArtifacts": bool((pass435.get("pass376_quarantine") or {}).get("quarantined")),
        "pass497LocksStateDeltaBlocker": pass497.get("status") == "PASS497_ORIGINAL_CAPTURE_NEXT_BLOCKER_LOCKED",
        "pass498NarrowsToPostCommandBoundary": pass498.get("status") == "PASS498_ORIGINAL_CAPTURE_BLOCKER_NARROWED_TO_POST_COMMAND_STATE_DELTA",
        "staticRepeatedGameplayFrameStillPresent": observed498.get("postEntryGameplayHashRepeated") is True,
        "filenameRouteLabelDriftStillPresent": int(observed498.get("filenameLabelDriftRows") or 0) > 0,
    }


def main() -> int:
    source = [source_row(lock) for lock in SOURCE_LOCKS]
    pass435 = load(PASS435)
    pass497 = load(PASS497)
    pass498 = load(PASS498)
    predicates = blocker_predicates(pass435, pass497, pass498)
    problems: list[str] = []
    problems.extend(f"source lock failed: {row[file]}:{row[lines]} {row[missing]}" for row in source if not row["ok"])
    problems.extend(f"required blocker predicate not observed: {name}" for name, ok in predicates.items() if not ok)

    payload: dict[str, Any] = {
        "schema": "firestaff.parity.pass509_dm1_v1_original_overlay_keyboard_buffer_blocker.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": STATUS if not problems else "FAIL_PASS509_ORIGINAL_OVERLAY_KEYBOARD_BUFFER_BLOCKER",
        "ok": not problems,
        "sourceRoot": str(RED),
        "sourceAudit": source,
        "inputs": {"pass435": pass435.get("path"), "pass497": pass497.get("path"), "pass498": pass498.get("path")},
        "observed": {"pass435Status": pass435.get("status"), "pass435Blockers": pass435.get("blockers"), "pass376Quarantine": pass435.get("pass376_quarantine"), "pass497Status": pass497.get("status"), "pass497NextBlocker": pass497.get("nextBlocker"), "pass498Status": pass498.get("status"), "pass498ObservedBlockerState": pass498.get("observedBlockerState")},
        "requiredObserved": predicates,
        "narrowedBlocker": "Original DM1 V1 overlay/capture remains blocked at the keyboard-buffer and overlay-boundary seam: a replacement route must prove every keyboard-driven overlay token survives F0357 buffer discard, writes through F0361 into G0432/G2153, is popped by F0380, and reaches the matching post-command F0128/F0097 present boundary. Host route labels, repeated 48ed gameplay frames, and duplicate pass376/pass487 artifacts are blocker evidence only.",
        "promotionPredicate": ["record a strict original-runtime stop or equivalent memory watch showing F0357 did not flush the route token before overlay capture", "for each keyboard token, show F0361 writes G0432_as_CommandQueue and increments G2153_i_QueuedCommandsCount", "show the same token is later popped by F0380 with G2153 decrement and first-index advance", "show the matching command reaches F0365/F0366 or the source-owned overlay handler before the shot label is captured", "show the following F0128/F0097 viewport present boundary or an explicit source-owned overlay present boundary for that same shot", "reject repeated raw/crop hashes unless source/runtime proves the command was intentionally blocked or no-op"],
        "nonClaims": ["no DOSBox launch by this verifier", "no original-vs-Firestaff pixel parity", "no promotion of pass376/pass487 screenshots", "no claim that Firestaff movement or viewport implementation is wrong"],
        "problems": problems,
    }

    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass509 - DM1 V1 original overlay keyboard-buffer blocker",
        "",
        "Status: " + str(payload["status"]),
        "",
        "## Decision",
        "",
        str(payload["narrowedBlocker"]),
        "",
        "## ReDMCSB source audit",
        "",
    ]
    for row in source:
        lines.append("- {}:{} / {} ok={} - {}".format(row["file"], row["lines"], row["function"], row["ok"], row["claim"]))
    obs498 = pass498.get("observedBlockerState") or {}
    lines += [
        "",
        "## Current blocker state",
        "",
        "- pass435 status: {}; blockers: {}".format(pass435.get("status"), pass435.get("blockers")),
        "- pass497 status: {}".format(pass497.get("status")),
        "- pass498 status: {}".format(pass498.get("status")),
        "- repeated gameplay frame still present: {}".format(obs498.get("postEntryGameplayHashRepeated")),
        "- filename/route-label drift rows: {}".format(obs498.get("filenameLabelDriftRows")),
        "",
        "## Promotion predicate for the next route",
        "",
    ]
    lines.extend(f"- {item}" for item in payload["promotionPredicate"])
    lines += ["", "## Non-claims", ""]
    lines.extend(f"- {item}" for item in payload["nonClaims"])
    lines += ["", "## Gate", "", "- python3 tools/verify_pass509_dm1_v1_original_overlay_keyboard_buffer_blocker.py", "", f"Manifest: {MANIFEST.relative_to(ROOT)}"]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(json.dumps({"status": payload["status"], "ok": payload["ok"], "problems": problems, "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT))}, indent=2))
    return 0 if payload["ok"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
