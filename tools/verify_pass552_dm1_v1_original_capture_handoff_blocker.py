#!/usr/bin/env python3
from __future__ import annotations

import json
import shutil
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass552_dm1_v1_original_capture_handoff_blocker"
OUT = ROOT / "parity-evidence" / "verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"

SOURCE_SPECS = [
    {
        "id": "i34e_raw_key_normalization",
        "file": "IO2.C",
        "function": "F0540_INPUT_Crawcin",
        "sourceLineRange": [27, 61],
        "needles": [
            "L2944_ui_ = (*(G2162_IODriver->IODRV_00_GetKeyboardInput))();",
            "switch (L2944_ui_ - 0x1248)",
            "L2944_ui_ = 'L'",
            "L2944_ui_ = 'P'",
            "L2944_ui_ = 'K'",
            "L2944_ui_ = 'M'",
        ],
        "meaning": "PC/I34E shifted arrow scan codes normalize to the same command-key bytes used by the movement table.",
    },
    {
        "id": "i34e_movement_keyboard_table",
        "file": "COMMAND.C",
        "function": "G0459_as_Graphic561_SecondaryKeyboardInput_Movement",
        "sourceLineRange": [636, 685],
        "needles": [
            "{ C001_COMMAND_TURN_LEFT,     0x004B }",
            "{ C003_COMMAND_MOVE_FORWARD,  0x004C }",
            "{ C002_COMMAND_TURN_RIGHT,    0x004D }",
            "{ C006_COMMAND_MOVE_LEFT,     0x004F }",
            "{ C005_COMMAND_MOVE_BACKWARD, 0x0050 }",
            "{ C004_COMMAND_MOVE_RIGHT,    0x0051 }",
        ],
        "meaning": "The command transcript must prove K/L/M/O/P/Q bytes, not host-side route labels.",
    },
    {
        "id": "f0361_keyboard_enqueue",
        "file": "COMMAND.C",
        "function": "F0361_COMMAND_ProcessKeyPress",
        "sourceLineRange": [1709, 1813],
        "needles": [
            "if ((L1112_ps_KeyboardInput = G0443_ps_PrimaryKeyboardInput) == NULL)",
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "while (L1111_i_Command = L1112_ps_KeyboardInput->Command)",
            "if (P0728_KeyCode == L1112_ps_KeyboardInput->Code)",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G2153_i_QueuedCommandsCount++;",
        ],
        "meaning": "A fresh capture must show the keyboard byte entering F0361 and creating a queue slot/count delta.",
    },
    {
        "id": "gameloop_keyboard_before_f0380",
        "file": "GAMELOOP.C",
        "function": "F0002_MAIN_GameLoop_CPSDF",
        "sourceLineRange": [164, 219],
        "needles": [
            "while (M527_IsCharacterInKeyboardBuffer())",
            "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
            "F0380_COMMAND_ProcessQueue_CPSC();",
        ],
        "meaning": "The source route order is keyboard-buffer drain, then F0380 queue processing.",
    },
    {
        "id": "f0380_pop_dispatch",
        "file": "COMMAND.C",
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "sourceLineRange": [2045, 2156],
        "needles": [
            "if (G2153_i_QueuedCommandsCount == 0)",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G2153_i_QueuedCommandsCount--;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "meaning": "F0380 is only promotable when the queue count is positive and the pop/decrement/dispatch path is observed.",
    },
    {
        "id": "turn_step_party_handlers",
        "file": "CLIKMENU.C",
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "sourceLineRange": [142, 347],
        "needles": [
            "F0284_CHAMPION_SetPartyDirection",
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        ],
        "meaning": "After F0380 dispatch, the capture must bind turn/step handlers to party-tuple changes or blocked/no-op reason.",
    },
    {
        "id": "successful_step_tuple_commit",
        "file": "MOVESENS.C",
        "function": "F0267_MOVE_GetMoveResult_CPSCE",
        "sourceLineRange": [316, 450],
        "needles": [
            "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
        ],
        "meaning": "Successful forward/side/backward captures need the MOVESENS coordinate commit or a source-typed blocker.",
    },
    {
        "id": "draw_to_present_boundary",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "sourceLineRange": [8318, 8611],
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "meaning": "A frame/crop must be tied to F0128 consuming the party tuple and calling the present boundary.",
    },
    {
        "id": "pc34_viewport_present",
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "sourceLineRange": [709, 858],
        "needles": [
            "G0296_puc_Bitmap_Viewport",
            "F0638_GetZone(C007_ZONE_VIEWPORT",
            "VIDRV_09_BlitViewPort",
        ],
        "meaning": "Promotable original pixels require the PC/I34E viewport blit boundary, not just an earlier draw call.",
    },
]

ARTIFACTS = {
    "pass388": "parity-evidence/verification/pass388_dm1_v1_queue_producer_runtime/manifest.json",
    "pass514": "parity-evidence/verification/pass514_dm1_v1_i34e_runtime_transcript_capture_path/manifest.json",
    "pass548": "parity-evidence/verification/pass548_dm1_v1_original_overlay_capture_progress/manifest.json",
}


def norm(text: str) -> str:
    return " ".join(text.split())


def find_line(lines: list[str], needle: str, start: int | None = None, end: int | None = None) -> int | None:
    wanted = norm(needle)
    lo = 1 if start is None else start
    hi = len(lines) if end is None else end
    for idx in range(max(1, lo), min(len(lines), hi) + 1):
        if wanted in norm(lines[idx - 1]):
            return idx
    return None


def source_audit() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for spec in SOURCE_SPECS:
        path = RED / spec["file"]
        text = path.read_text(encoding="latin-1", errors="replace") if path.exists() else ""
        lines = text.splitlines()
        line_range = spec["sourceLineRange"]
        hits = {needle: find_line(lines, needle, line_range[0], line_range[1]) for needle in spec["needles"]}
        rows.append({
            "id": spec["id"],
            "file": spec["file"],
            "function": spec["function"],
            "path": str(path),
            "lineRange": line_range,
            "ok": path.exists() and all(v is not None for v in hits.values()) and line_range is not None,
            "lineHits": hits,
            "missing": [needle for needle, line in hits.items() if line is None],
            "meaning": spec["meaning"],
        })
    return rows


def load_json(rel: str) -> dict[str, Any]:
    path = ROOT / rel
    if not path.exists():
        return {"exists": False, "path": rel}
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        return {"exists": True, "path": rel, "parseError": str(exc)}
    return {"exists": True, "path": rel, "data": data}


def prereq() -> dict[str, Any]:
    tools = {name: shutil.which(name) for name in ["dosbox-debug", "Xvfb", "xdotool"]}
    return {
        "tools": tools,
        "missingTools": [name for name, value in tools.items() if not value],
        "originalStage": str(ORIG),
        "originalStageExists": ORIG.exists(),
        "dmExeExists": (ORIG / "DM.EXE").exists(),
        "n2ReferenceRoots": [
            str(Path.home() / ".openclaw/data/firestaff-greatstone-atlas"),
            str(Path.home() / ".openclaw/data/firestaff-redmcsb-source"),
            str(Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin"),
            str(Path.home() / ".openclaw/data/firestaff-csb-source/CSB"),
            str(Path.home() / ".openclaw/data/firestaff-original-games/DM"),
        ],
    }


def summarize(artifacts: dict[str, dict[str, Any]], source_ok: bool, pre: dict[str, Any]) -> tuple[str, str, dict[str, Any]]:
    p388 = artifacts["pass388"].get("data", {})
    p514 = artifacts["pass514"].get("data", {})
    p548 = artifacts["pass548"].get("data", {})
    p388_pred = p388.get("proofPredicates") or {}
    p514_pred = p514.get("proofPredicates") or {}
    p548_live = p548.get("liveAttempt") or {}
    predicates = {
        "sourceAuditOk": source_ok,
        "prerequisitesOk": not pre["missingTools"] and pre["dmExeExists"],
        "pass388RouteInputAfterArmingSucceeded": bool(p388_pred.get("routeInputAfterArmingSucceeded")),
        "pass388BreakpointsRetainedAndControlReached": bool(p388_pred.get("routeControlReachedAfterArmingWithBreakpointsRetained")),
        "pass388F0380Reached": bool(p388_pred.get("f0380Reached")),
        "pass388F0380QueuePositiveBeforePop": bool(p388_pred.get("f0380QueueCountPositiveImmediatelyBefore")),
        "pass388KeyboardProducerEntryHit": bool(p388_pred.get("keyboardProducerEntryHit")),
        "pass388QueueCountWriteObserved": bool(p388_pred.get("queueCountWriteObserved")),
        "pass388DispatchReached": bool(p388_pred.get("dispatchReached")),
        "pass514UnifiedCaptureAttempted": bool(p514_pred.get("captureAttemptedThisRun")),
        "pass548LiveTimedOut": bool(p548_live.get("timedOut")),
        "pass548Pass475SnapshotExists": bool((p548.get("pass475Snapshot") or {}).get("exists")),
    }
    if not predicates["sourceAuditOk"]:
        return "FAIL_PASS552_SOURCE_AUDIT_INCOMPLETE", "ReDMCSB source audit did not lock every required capture handoff anchor.", predicates
    if not predicates["prerequisitesOk"]:
        return "BLOCKED_PASS552_N2_RUNTIME_PREREQUISITE_MISSING", "N2 runtime prerequisites are missing, so no fresh runtime capture can be trusted.", predicates
    if (
        predicates["pass388BreakpointsRetainedAndControlReached"]
        and predicates["pass388RouteInputAfterArmingSucceeded"]
        and predicates["pass388F0380Reached"]
        and not predicates["pass388KeyboardProducerEntryHit"]
        and not predicates["pass388QueueCountWriteObserved"]
        and not predicates["pass388F0380QueuePositiveBeforePop"]
    ):
        return (
            "BLOCKED_PASS552_FRESH_CAPTURE_HANDOFF_STOPS_AT_EMPTY_F0380",
            "Fresh original capture handoff is blocked before pass548 overlay capture: pass388 proves the armed route reaches F0380, but it does not prove F0361 entry, G0432/G2153 enqueue, positive queue count before F0380 pop, or dispatch. pass548 therefore correctly remains non-promotable while its bounded live capture times out/no pass475 manifest is available.",
            predicates,
        )
    if predicates["pass548LiveTimedOut"] and not predicates["pass548Pass475SnapshotExists"]:
        return (
            "BLOCKED_PASS552_OVERLAY_CAPTURE_TIMEOUT_AFTER_INCOMPLETE_QUEUE_CHAIN",
            "The overlay capture lane still has no promotable original frame: pass548's bounded live capture timed out and no pass475 manifest exists.",
            predicates,
        )
    return "BLOCKED_PASS552_ORIGINAL_CAPTURE_HANDOFF_INCOMPLETE", "Original capture handoff remains incomplete; inspect predicates for the first missing runtime boundary.", predicates


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    source = source_audit()
    pre = prereq()
    artifacts = {name: load_json(rel) for name, rel in ARTIFACTS.items()}
    status, blocker, predicates = summarize(artifacts, all(row["ok"] for row in source), pre)
    run = lambda cmd: subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT).stdout.strip()
    manifest = {
        "schema": PASS + ".v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "blocker": blocker,
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"]),
        "head": run(["git", "rev-parse", "HEAD"]),
        "sourceRoot": str(RED),
        "sourceAudit": source,
        "prerequisites": pre,
        "inputArtifacts": {
            name: {
                "path": row.get("path"),
                "exists": row.get("exists"),
                "status": (row.get("data") or {}).get("status"),
                "blocker": (row.get("data") or {}).get("blocker"),
                "proofPredicates": (row.get("data") or {}).get("proofPredicates"),
                "liveAttempt": (row.get("data") or {}).get("liveAttempt") if name == "pass548" else None,
            }
            for name, row in artifacts.items()
        },
        "proofPredicates": predicates,
        "promotionRequirement": [
            "observe PC/I34E raw/normalized key value",
            "observe F0361 table match plus G0432 slot/G0434/G2153 enqueue delta",
            "observe F0380 positive count, pop/decrement, and F0365/F0366 dispatch",
            "observe party tuple before/after or a source-typed blocked/no-op reason",
            "observe F0128 tuple consumption and F0097/VIDRV present boundary",
            "then allow overlay/frame capture to promote pixels",
        ],
        "nextTightProbe": "Instrument F0540/M528 value plus F0361 entry and G0432/G2153 writes in the same armed window before rerunning pass548/pass475 overlay capture.",
        "nonClaims": [
            "no movement implementation changed",
            "no viewport rendering changed",
            "no original pixel/frame promoted",
            "no push",
        ],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass552 - DM1 V1 original capture handoff blocker",
        "",
        f"Status: {status}",
        "",
        "## Decision",
        "",
        blocker,
        "",
        "## ReDMCSB source audit",
        "",
    ]
    for row in source:
        rng = row["lineRange"] or ["?", "?"]
        lines.append(f"- {row['file']}:{rng[0]}-{rng[1]} / {row['function']} - {row['id']}")
    lines += [
        "",
        "## Runtime artifact inputs",
        "",
    ]
    for name, rel in ARTIFACTS.items():
        row = artifacts[name]
        data = row.get("data") or {}
        lines.append(f"- {name}: {rel} - {data.get('status', 'missing')}")
    lines += [
        "",
        "## Promotion rule",
        "",
        "Do not promote pass548/pass475 overlay pixels until one fresh runtime transcript observes the F0540/M528 key value, F0361 queue slot/count write, F0380 positive pop/decrement/dispatch, party tuple or typed blocker, F0128, and F0097/VIDRV boundary in order.",
        "",
        "## Evidence",
        "",
        f"- Manifest: parity-evidence/verification/{PASS}/manifest.json",
    ]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "blocker": blocker, "manifest": f"parity-evidence/verification/{PASS}/manifest.json", "report": f"parity-evidence/{PASS}.md"}, indent=2, sort_keys=True))
    return 0 if status.startswith("BLOCKED_PASS552") else 1


if __name__ == "__main__":
    raise SystemExit(main())
