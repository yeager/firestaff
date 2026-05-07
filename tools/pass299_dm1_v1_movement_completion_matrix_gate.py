#!/usr/bin/env python3
"""Pass299 DM1 V1 movement completion matrix gate.

This gate consumes pass296, checks the DM1 V1 parity matrix wording, and
emits an explicit blocker manifest. It deliberately keeps direct F0380 BP
blocked-but-nonblocking for the movement source/runtime-supported proof while
separating the remaining completion criteria.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PASS296 = ROOT / "parity-evidence/verification/pass296_dm1_v1_input_tuple_proof_without_direct_f0380/manifest.json"
MATRIX = ROOT / "docs/parity/PARITY_MATRIX_DM1_V1.md"
OUT = ROOT / "parity-evidence/verification/pass299_dm1_v1_movement_completion_matrix_gate"
REPORT = ROOT / "parity-evidence/pass299_dm1_v1_movement_completion_matrix_gate.md"

PASS296_STATUS = "MOVEMENT_INPUT_TUPLE_PROOF_SOURCE_LOCKED_RUNTIME_SUPPORTED_DIRECT_F0380_BLOCKED_NOT_REQUIRED"
SOURCE_CITATIONS = [
    {
        "file": "GAMELOOP.C",
        "function": "F0002_MAIN_GameLoop_CPSDF",
        "range": "80-90, 160-168, 215-219",
        "claim": "redraw consumes G0308/G0306/G0307; keyboard input is drained before F0380; the wait loop invokes F0380.",
    },
    {
        "file": "COMMAND.C",
        "function": "F0361_COMMAND_ProcessKeyPress / F0380_COMMAND_ProcessQueue_CPSC",
        "range": "252-260, 1734-1812, 2045-2156",
        "claim": "controlled movement keys map to commands, enqueue into G0432, dequeue/dispatch from F0380, and apply disabled-movement gates.",
    },
    {
        "file": "CLIKMENU.C",
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "range": "156-347",
        "claim": "turn commands set party direction; move commands compute relative destination, check blockers, and invoke movement result/timing paths.",
    },
    {
        "file": "MOVESENS.C",
        "function": "F0267_MOVE_GetMoveResult_CPSCE",
        "range": "316-565, 738-818",
        "claim": "successful movement commits G0306/G0307, handles consequences, records movement result/timing/scent, and runs sensor side effects.",
    },
    {
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "range": "8318-8611",
        "claim": "viewport draw receives direction/mapX/mapY arguments and presents the dungeon viewport.",
    },
]

REMAINING_BLOCKERS = [
    {
        "id": "original_party_control_ready_capture",
        "blocks": "original-runtime movement/HUD/viewport pixel parity",
        "missing": "A deterministic original PC route that reaches party-control-ready gameplay and yields semantically labelled movement before/after captures, not direct-start/no-party or menu/title frames.",
    },
    {
        "id": "direct_f0380_binary_trace_optional",
        "blocks": "only a future binary-level direct-F0380 hook claim, not the source/runtime-supported movement proof",
        "missing": "Decompressed stock FIRES memory map/FIRES.MAP or live disassembly breakpoints around the F0380 post-prologue/dequeue window; pass296 keeps this blocked and non-required.",
    },
    {
        "id": "full_movement_side_effect_coverage",
        "blocks": "promotion of the whole movement row to MATCHED",
        "missing": "Original-backed cases for the broader movement side effects: group/projectile interlocks, sensor/environment consequence breadth, timing/cooldown edge cases, and overlay comparison from matched runtime states.",
    },
]


def load_pass296() -> dict:
    data = json.loads(PASS296.read_text(encoding="utf-8"))
    return data


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    p296 = load_pass296()
    matrix_text = MATRIX.read_text(encoding="utf-8")

    checks = {
        "pass296_status_locked": p296.get("status") == PASS296_STATUS,
        "pass296_gate_passed": p296.get("proof_gate_passed") is True,
        "direct_f0380_nonblocking": p296.get("direct_f0380_decision", {}).get("required_for_this_gate") is False
        and p296.get("direct_f0380_decision", {}).get("blocked") is True,
        "matrix_has_status": PASS296_STATUS in matrix_text,
        "matrix_marks_direct_f0380_nonblocking": "no longer a blocker for the movement source/runtime-supported proof" in matrix_text,
        "matrix_lists_remaining_blockers": "Remaining movement blockers are tracked separately" in matrix_text,
        "source_citations_complete": all(c["file"] in matrix_text for c in SOURCE_CITATIONS),
        "remaining_blocker_manifest_complete": all(b.get("id") and b.get("blocks") and b.get("missing") for b in REMAINING_BLOCKERS),
    }
    ok = all(checks.values())
    status = "MOVEMENT_COMPLETION_MATRIX_UPDATED_DIRECT_F0380_NONBLOCKING_REMAINING_BLOCKERS_EXPLICIT" if ok else "BLOCKED"

    manifest = {
        "schema": "pass299_dm1_v1_movement_completion_matrix_gate.v1",
        "timestamp_utc": "2026-05-07T01:38:00Z",
        "status": status,
        "pass296_status": p296.get("status"),
        "direct_f0380_decision": p296.get("direct_f0380_decision"),
        "source_citations": SOURCE_CITATIONS,
        "remaining_movement_blockers": REMAINING_BLOCKERS,
        "checks": checks,
        "matrix": MATRIX.relative_to(ROOT).as_posix(),
        "pass296_manifest": PASS296.relative_to(ROOT).as_posix(),
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    citation_md = "\n".join(
        "- `{}:{}` / `{}` — {}".format(c["file"], c["range"], c["function"], c["claim"])
        for c in SOURCE_CITATIONS
    )
    blocker_md = "\n".join(
        "- **{}** — blocks: {}; missing: {}".format(b["id"], b["blocks"], b["missing"])
        for b in REMAINING_BLOCKERS
    )
    check_md = "\n".join("- {} `{}`".format("PASS" if v else "FAIL", k) for k, v in checks.items())
    report = f"""# Pass299 — DM1 V1 movement completion matrix gate

Status: `{status}`

## Verdict

The DM1 V1 movement matrix row now treats pass296 as the accepted source/runtime-supported proof for input→queue→dispatch-equivalent→party tuple→F0128 draw consumption. Direct F0380 entry/body BP remains blocked and unclaimed, but it is non-blocking for this proof scope.

## ReDMCSB-first citations

{citation_md}

## Remaining movement blockers

{blocker_md}

## Checks

{check_md}

## Artifacts

- Matrix: `docs/parity/PARITY_MATRIX_DM1_V1.md`
- Manifest: `parity-evidence/verification/pass299_dm1_v1_movement_completion_matrix_gate/manifest.json`
- Depends on pass296 manifest: `parity-evidence/verification/pass296_dm1_v1_input_tuple_proof_without_direct_f0380/manifest.json`
"""
    REPORT.write_text(report, encoding="utf-8")
    print(json.dumps({"status": status, "ok": ok, "manifest": str((OUT / "manifest.json").relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
