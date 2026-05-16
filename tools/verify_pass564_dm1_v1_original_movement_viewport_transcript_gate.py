#!/usr/bin/env python3
from __future__ import annotations

import json
import os
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass564_dm1_v1_original_movement_viewport_transcript_gate"
OUT = ROOT / "parity-evidence" / "verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS563 = ROOT / "parity-evidence/verification/pass563_dm1_v1_pc34_original_c254_boundary/manifest.json"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {
        "id": "game_loop_draws_tuple_before_wait_and_dispatches_queue_inside_wait",
        "file": "GAMELOOP.C",
        "function": "F0002_MAIN_GameLoop_CPSDF",
        "needles": [
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
            "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
            "while (M527_IsCharacterInKeyboardBuffer())",
            "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
        ],
        "claim": "The runtime transcript must bind input drain and F0380 dispatch to the next loop draw that consumes G0308/G0306/G0307.",
    },
    {
        "id": "pc34_keyboard_read_goes_through_c254_io_driver_slot0_slot1",
        "file": "IO2.C",
        "function": "F0540_INPUT_Crawcin / F0539_INPUT_Cconis",
        "needles": [
            "L2944_ui_ = (*(G2162_IODriver->IODRV_00_GetKeyboardInput))();",
            "switch (L2944_ui_ - 0x1248)",
            "L2944_ui_ = 'L';",
            "L2944_ui_ = 'P';",
            "L2944_ui_ = 'K';",
            "L2944_ui_ = 'M';",
            "return (*(G2162_IODriver->IODRV_01_IsKeyboardInputPresent))();",
        ],
        "claim": "A movement key is source-visible only after C254 slot0/slot1 are decoded and IO2 returns the normalized movement code.",
    },
    {
        "id": "f0361_enqueues_keyboard_command_and_count",
        "file": "COMMAND.C",
        "function": "F0361_COMMAND_ProcessKeyPress",
        "needles": [
            "void F0361_COMMAND_ProcessKeyPress",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G2153_i_QueuedCommandsCount++;",
            "F0360_COMMAND_ProcessPendingClick();",
        ],
        "claim": "The transcript must show the IO2 key reaches F0361 and writes G0432/G2153.",
    },
    {
        "id": "f0380_pops_queue_and_dispatches_turn_or_step",
        "file": "COMMAND.C",
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "needles": [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "if (G2153_i_QueuedCommandsCount == 0)",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G2153_i_QueuedCommandsCount--;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "claim": "The transcript must show a non-empty queue pop and source dispatch before any viewport capture is promoted.",
    },
    {
        "id": "turn_and_step_mutate_party_state",
        "file": "CLIKMENU.C",
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "needles": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(G0308_i_PartyDirection +",
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY",
            "G0306_i_PartyMapX = L1121_i_MapX;",
            "G0307_i_PartyMapY = L1122_i_MapY;",
        ],
        "claim": "Movement/turn parity requires source-visible party tuple mutation, not only queued input.",
    },
    {
        "id": "f0128_composes_viewport_and_calls_f0097_present",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "G0296_puc_Bitmap_Viewport",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "claim": "A promoted original frame must be after F0128 composes the viewport for the mutated tuple and hands it to F0097.",
    },
    {
        "id": "f0097_presents_viewport_zone",
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "F0704_();",
            "C007_ZONE_VIEWPORT",
            "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))",
            "F0705_();",
        ],
        "claim": "Viewport/wall capture needs the present-side F0097 boundary, not only F0128 entry.",
    },
]

REQUIRED_SEQUENCE = [
    "c254_vector_decoded",
    "io_driver_slot0_f8090_slot1_f8091",
    "io2_f0540_movement_key",
    "f0361_enqueue_g0432_g2153_increment",
    "f0380_pop_g2153_decrement",
    "f0365_or_f0366_dispatch",
    "party_tuple_mutated_or_confirmed_blocked",
    "f0128_draw_tuple",
    "f0097_viewport_present",
]


def run(cmd: list[str]) -> str:
    return subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False).stdout.strip()


def compact(text: str) -> str:
    return " ".join(text.split())


def find_line(path: Path, needle: str) -> int | None:
    if not path.exists():
        return None
    target = compact(needle)
    for idx, line in enumerate(path.read_text(encoding="latin-1", errors="replace").splitlines(), 1):
        if target in compact(line):
            return idx
    return None


def audit_source() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for spec in SOURCE_LOCKS:
        path = RED / spec["file"]
        hits = {needle: find_line(path, needle) for needle in spec["needles"]}
        present = [line for line in hits.values() if line is not None]
        rows.append({
            "id": spec["id"],
            "file": spec["file"],
            "path": str(path),
            "function": spec["function"],
            "ok": path.exists() and len(present) == len(hits),
            "lineRange": [min(present), max(present)] if present else None,
            "lineHits": hits,
            "missing": [needle for needle, line in hits.items() if line is None],
            "claim": spec["claim"],
        })
    return rows


def load_json(path: Path) -> dict[str, Any] | None:
    if not path.exists():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def load_transcript() -> dict[str, Any]:
    raw = os.environ.get("FIRESTAFF_PASS564_RUNTIME_TRANSCRIPT")
    if not raw:
        return {"provided": False, "ok": False, "status": "not_provided"}
    path = Path(raw)
    if not path.is_absolute():
        path = ROOT / path
    if not path.exists():
        return {"provided": True, "ok": False, "status": "missing", "path": str(path)}
    payload = json.loads(path.read_text(encoding="utf-8"))
    events = payload.get("events", [])
    names = [event.get("event") for event in events if isinstance(event, dict)]
    cursor = 0
    matched: list[str] = []
    for required in REQUIRED_SEQUENCE:
        try:
            idx = names.index(required, cursor)
        except ValueError:
            return {
                "provided": True,
                "ok": False,
                "status": "sequence_incomplete",
                "path": str(path),
                "matched": matched,
                "missingFrom": required,
                "requiredSequence": REQUIRED_SEQUENCE,
            }
        matched.append(required)
        cursor = idx + 1
    same_run = len({event.get("runId") for event in events if isinstance(event, dict) and event.get("event") in REQUIRED_SEQUENCE}) == 1
    return {
        "provided": True,
        "ok": same_run,
        "status": "loaded_promotable" if same_run else "events_not_same_run",
        "path": str(path),
        "matched": matched,
        "requiredSequence": REQUIRED_SEQUENCE,
    }


def classify(source: list[dict[str, Any]], pass563: dict[str, Any] | None, transcript: dict[str, Any]) -> tuple[str, str, dict[str, bool]]:
    predicates = {
        "sourceAuditOk": all(row["ok"] for row in source),
        "pass563Present": pass563 is not None,
        "pass563LocksC254Blocker": bool(pass563 and pass563.get("status") == "BLOCKED_PASS563_PC34_ORIGINAL_C254_SLOT_RUNTIME_PROBE_REQUIRED"),
        "runtimeTranscriptPromotable": transcript.get("ok") is True,
    }
    if not predicates["sourceAuditOk"]:
        return "FAIL_PASS564_REDMCSB_SOURCE_AUDIT_FAILED", "ReDMCSB source audit failed; do not interpret runtime captures.", predicates
    if transcript.get("ok") is True:
        return (
            "PASS564_DM1_V1_ORIGINAL_MOVEMENT_VIEWPORT_TRANSCRIPT_PROMOTABLE",
            "Supplied transcript proves the ordered C254/IO2/F0361/F0380/dispatch/F0128/F0097 chain in one run.",
            predicates,
        )
    return (
        "BLOCKED_PASS564_RUNTIME_TRANSCRIPT_CHAIN_MISSING",
        "Original DM1 V1 movement/viewport/wall capture remains blocked until one bounded canonical PC34 run records C254 slot decode, IO2 movement key, F0361 enqueue, F0380 pop, F0365/F0366 dispatch, party tuple mutation or intentional blocked-step proof, then F0128 draw and F0097 viewport present for the same route token.",
        predicates,
    )


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# Pass564 - DM1 V1 original movement viewport transcript gate",
        "",
        f"- Status: {manifest['status']}",
        f"- Manifest: parity-evidence/verification/{PASS}/manifest.json",
        "",
        "## Decision",
        "",
        manifest["decision"],
        "",
        "## Required runtime event order",
    ]
    lines.extend(f"- {event}" for event in REQUIRED_SEQUENCE)
    lines.extend(["", "## ReDMCSB locks"])
    for row in manifest["sourceAudit"]:
        rng = row["lineRange"] or ["?", "?"]
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['file']}:{rng[0]}-{rng[1]} {row['function']} - {row['claim']}")
    lines.extend([
        "",
        "## Inputs",
        "",
        f"- pass563: {manifest['pass563'].get('status') if manifest['pass563'] else 'missing'}",
        f"- runtime transcript: {manifest['runtimeTranscript'].get('status')}",
        "",
        "## Non-claims",
    ])
    lines.extend(f"- {item}" for item in manifest["nonClaims"])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    source = audit_source()
    pass563 = load_json(PASS563)
    transcript = load_transcript()
    status, decision, predicates = classify(source, pass563, transcript)
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "decision": decision,
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"]),
        "head": run(["git", "rev-parse", "HEAD"]),
        "sourceRoot": str(RED),
        "sourceAudit": source,
        "pass563": {"path": str(PASS563.relative_to(ROOT)), "status": pass563.get("status") if pass563 else None} if pass563 else None,
        "runtimeTranscript": transcript,
        "proofPredicates": predicates,
        "requiredRuntimeSequence": REQUIRED_SEQUENCE,
        "nonClaims": [
            "no DOSBox runtime transcript is fabricated by this gate",
            "no original screenshot or pixel parity is promoted without the required transcript",
            "no Firestaff movement or viewport runtime behavior is changed",
            "no DANNESBURK input touched",
            "no push",
        ],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(f"{status}: {decision}")
    return 0 if status.startswith("BLOCKED_PASS564_") or status.startswith("PASS564_") else 1


if __name__ == "__main__":
    raise SystemExit(main())
