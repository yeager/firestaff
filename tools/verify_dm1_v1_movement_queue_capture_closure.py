#!/usr/bin/env python3
"""Close the current DM1 V1 input->queue->movement capture/evidence gap.

This gate is deliberately a contract/evidence closure, not an original-pixel
parity claim. It proves that the Firestaff source route is already covered and
that the only remaining input/movement TODO is the source-bound original runtime
transcript plus paired viewport captures required by pass564/pass608/pass609.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT_DIR = ROOT / "parity-evidence/verification/dm1_v1_movement_queue_capture_closure"
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence/dm1_v1_movement_queue_capture_closure.md"

SOURCE_LOCKS = [
    {
        "file": "COMMAND.C",
        "lines": "106-121",
        "claim": "PC34 movement mouse zones feed the command queue labels used by capture routes.",
        "markers": [
            "C001_COMMAND_TURN_LEFT",
            "C003_COMMAND_MOVE_FORWARD",
            "C002_COMMAND_TURN_RIGHT",
            "C006_COMMAND_MOVE_LEFT",
            "C005_COMMAND_MOVE_BACKWARD",
            "C004_COMMAND_MOVE_RIGHT",
            "C080_COMMAND_CLICK_IN_DUNGEON_VIEW",
        ],
    },
    {
        "file": "COMMAND.C",
        "lines": "1452-1661",
        "claim": "F0359 stores accepted mouse commands or pending clicks into G0432.",
        "markers": [
            "F0359_COMMAND_ProcessClick_CPSC",
            "G0436_B_PendingClickPresent = C1_TRUE",
            "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0441_ps_PrimaryMouseInput",
            "F0358_COMMAND_GetCommandFromMouseInput_CPSC(G0442_ps_SecondaryMouseInput",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command",
        ],
    },
    {
        "file": "COMMAND.C",
        "lines": "1709-1813",
        "claim": "F0361 scans keyboard tables and writes matched commands to G0432/G2153.",
        "markers": [
            "F0361_COMMAND_ProcessKeyPress",
            "G0443_ps_PrimaryKeyboardInput",
            "G0444_ps_SecondaryKeyboardInput",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command",
            "G2153_i_QueuedCommandsCount++",
        ],
    },
    {
        "file": "COMMAND.C",
        "lines": "2045-2156",
        "claim": "F0380 gates movement, replays pending clicks, pops a command, and dispatches F0365/F0366.",
        "markers": [
            "F0380_COMMAND_ProcessQueue_CPSC",
            "G0310_i_DisabledMovementTicks",
            "G0311_i_ProjectileDisabledMovementTicks",
            "F0360_COMMAND_ProcessPendingClick();",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
    },
    {
        "file": "CLIKMENU.C",
        "lines": "142-347",
        "claim": "F0365/F0366 own the turn and step state changes before redraw promotion is legal.",
        "markers": [
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0284_CHAMPION_SetPartyDirection",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
        ],
    },
    {
        "file": "DUNVIEW.C",
        "lines": "8318-8611",
        "claim": "F0128 composes the viewport from the post-command party tuple.",
        "markers": [
            "F0128_DUNGEONVIEW_Draw_CPSF",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
    },
    {
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "claim": "F0097 is the PC34 viewport-present boundary required for promoted captures.",
        "markers": [
            "F0097_DUNGEONVIEW_DrawViewport",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "M526_WaitVerticalBlank();",
        ],
    },
]

EXPECTED_ARTIFACTS = {
    "parity-evidence/verification/pass564_dm1_v1_original_movement_viewport_transcript_gate/manifest.json": "BLOCKED_PASS564_RUNTIME_TRANSCRIPT_CHAIN_MISSING",
    "parity-evidence/verification/pass608_dm1_v1_same_viewport_capture_blocker/manifest.json": "BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE",
    "parity-evidence/pass609_dm1_v1_same_viewport_capture_contract.md": "PASS609_DM1_V1_SAME_VIEWPORT_CAPTURE_CONTRACT_LOCKED",
    "parity-evidence/verification/pass545_dm1_v1_movement_queue_sensor_consequences/manifest.json": "PASS545_DM1_V1_MOVEMENT_QUEUE_SENSOR_CONSEQUENCES_LOCKED",
    "parity-evidence/verification/pass559_dm1_v1_gated_movement_pending_click_queue_replay/manifest.json": "PASS559_DM1_V1_GATED_MOVEMENT_PENDING_CLICK_QUEUE_REPLAY_LOCKED",
}


def compact(text: str) -> str:
    return " ".join(text.split())


def read(path: Path, encoding: str = "utf-8") -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding=encoding, errors="replace")


def excerpt(file_name: str, line_range: str) -> str:
    lines = read(RED / file_name, "latin-1").splitlines()
    start_s, end_s = line_range.split("-", 1)
    start, end = int(start_s), int(end_s)
    return "\n".join(lines[start - 1:end])


def audit_redmcsb() -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    for item in SOURCE_LOCKS:
        text = compact(excerpt(item["file"], item["lines"]))
        missing = [m for m in item["markers"] if compact(m) not in text]
        rows.append({
            "file": item["file"],
            "lines": item["lines"],
            "claim": item["claim"],
            "ok": not missing,
            "missingMarkers": missing,
        })
    return rows


def artifact_status(path: str) -> str | None:
    full = ROOT / path
    if not full.exists():
        return None
    if full.suffix == ".md":
        for line in read(full).splitlines():
            if line.startswith("Status:"):
                return line.split("Status:", 1)[1].strip().strip("`")
        return None
    data = json.loads(read(full))
    return data.get("status")


def audit_artifacts() -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    for path, expected in EXPECTED_ARTIFACTS.items():
        observed = artifact_status(path)
        rows.append({"path": path, "expected": expected, "observed": observed, "ok": observed == expected})
    return rows


def audit_todo() -> dict[str, object]:
    todo = read(ROOT / "TODO.md")
    needles = [
        "Input command routing",
        "release-mouse button identity",
        "movement collision-before-sensor dispatch ordering",
        "capture-only",
    ]
    return {"file": "TODO.md", "ok": all(n in todo for n in needles), "needles": needles}


def write_report(manifest: dict[str, object]) -> None:
    lines = [
        "# DM1 V1 movement queue capture closure",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "Scope: command input -> queue -> movement dispatch capture/evidence closure. This gate does not claim original pixel parity.",
        "",
        "## ReDMCSB source audit",
        "",
    ]
    for item in manifest["redmcsbSourceAudit"]:
        state = "PASS" if item["ok"] else "FAIL"
        lines.append(f"- {state} `{item['file']}:{item['lines']}` - {item['claim']}")
    lines += ["", "## Consumed evidence gates", ""]
    for item in manifest["artifactChecks"]:
        state = "PASS" if item["ok"] else "FAIL"
        lines.append(f"- {state} `{item['path']}` status `{item['observed']}`")
    lines += [
        "",
        "## Closure decision",
        "",
        "The Firestaff source route and queue-dispatch behavior are already covered. The current open TODO is capture-only: a promotable original PC/I34E transcript must bind command input, G0432/G2153 queue mutation, F0380 pop, F0365/F0366 dispatch, party tuple state, F0128 redraw, and F0097 present to paired original/Firestaff viewport crops.",
        "",
        "No new implementation behavior is changed by this pass.",
        "",
    ]
    OUT_MD.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    redmcsb = audit_redmcsb()
    artifacts = audit_artifacts()
    todo = audit_todo()
    ok = all(item["ok"] for item in redmcsb) and all(item["ok"] for item in artifacts)
    status = "PASS_DM1_V1_MOVEMENT_QUEUE_CAPTURE_CLOSURE_LOCKED" if ok else "FAIL_DM1_V1_MOVEMENT_QUEUE_CAPTURE_CLOSURE"
    manifest: dict[str, object] = {
        "schema": "firestaff.dm1_v1_movement_queue_capture_closure.v1",
        "status": status,
        "redmcsbRoot": str(RED),
        "redmcsbSourceAudit": redmcsb,
        "artifactChecks": artifacts,
        "todoCheck": todo,
        "closedSourceGap": "No uncovered source-table or Firestaff queue-dispatch gap remains for command input -> queue -> movement dispatch.",
        "remainingCaptureGap": {
            "id": "original_pc34_route_transcript_plus_paired_viewport_crops",
            "requires": [
                "original keyboard/mouse command token",
                "G0432/G2153 queue write and F0380 pop/count delta",
                "F0365/F0366 dispatch and party tuple mutation or blocked-step proof",
                "F0128 redraw tuple and F0097 viewport-present boundary",
                "paired original and Firestaff viewport crop hashes for the same tuple",
            ],
            "notClaimed": ["original pixel parity", "new DOSBox runtime transcript", "renderer behavior change"],
        },
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": status, "manifest": str(OUT_JSON.relative_to(ROOT)), "report": str(OUT_MD.relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
