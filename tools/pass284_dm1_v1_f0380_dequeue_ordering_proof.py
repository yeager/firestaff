#!/usr/bin/env python3
"""Pass284 DM1 V1 command queue dequeue ordering proof."""
from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT = ROOT / "parity-evidence/verification/pass284_dm1_v1_f0380_dequeue_ordering_proof"
REPORT = ROOT / "parity-evidence/pass284_dm1_v1_f0380_dequeue_ordering_proof.md"
PASS278 = ROOT / "parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json"

SOURCE_CHECKS = [
    {
        "id": "controlled_key_rows",
        "file": "COMMAND.C",
        "line_range": [252, 260],
        "claim": "PC movement keyboard rows map keypad/arrow controls to C001..C006 movement commands before F0361 scans them.",
        "needles": [
            "G0459_as_Graphic561_SecondaryKeyboardInput_Movement",
            "C001_COMMAND_TURN_LEFT",
            "C003_COMMAND_MOVE_FORWARD",
            "C002_COMMAND_TURN_RIGHT",
            "C006_COMMAND_MOVE_LEFT",
            "C005_COMMAND_MOVE_BACKWARD",
            "C004_COMMAND_MOVE_RIGHT",
        ],
    },
    {
        "id": "controlled_key_write_to_g0432",
        "file": "COMMAND.C",
        "line_range": [1734, 1812],
        "claim": "F0361 locks the queue, scans primary then secondary keyboard tables, writes matching commands into G0432, unlocks, then replays one pending click.",
        "needles": [
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "G0443_ps_PrimaryKeyboardInput",
            "G0444_ps_SecondaryKeyboardInput",
            "P0728_KeyCode == L1112_ps_KeyboardInput->Code",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
        ],
        "order": [
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "P0728_KeyCode == L1112_ps_KeyboardInput->Code",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
        ],
    },
    {
        "id": "f0380_index_dequeue_before_dispatch",
        "file": "COMMAND.C",
        "line_range": [2075, 2127],
        "claim": "F0380 locks, checks empty/gated movement before dequeue, reads command/X/Y from first index, advances G0433, then unlocks and replays pending clicks before dispatch.",
        "needles": [
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "AL1159_i_CommandQueueIndex = G0434_i_CommandQueueLastIndex + 1",
            "AL1159_i_CommandQueueIndex == G0433_i_CommandQueueFirstIndex",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G0310_i_DisabledMovementTicks",
            "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
            "L1162_i_CommandY = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Y;",
            "++G0433_i_CommandQueueFirstIndex",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
        ],
        "order": [
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G0310_i_DisabledMovementTicks",
            "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
            "L1162_i_CommandY = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Y;",
            "++G0433_i_CommandQueueFirstIndex",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
        ],
    },
    {
        "id": "f0380_dispatch_after_dequeue",
        "file": "COMMAND.C",
        "line_range": [2118, 2156],
        "claim": "F0380 dispatches C001/C002 turns and C003..C006 steps only after dequeue/unlock/replay.",
        "needles": [
            "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "order": [
            "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        ],
    },
    {
        "id": "turn_tuple_mutation",
        "file": "CHAMPION.C",
        "line_range": [117, 130],
        "claim": "Turn commands mutate the party tuple by writing G0308_i_PartyDirection after rotating champion cells/directions.",
        "needles": [
            "if (P0600_i_Direction == G0308_i_PartyDirection)",
            "L0835_ps_Champion->Cell = M021_NORMALIZE",
            "L0835_ps_Champion->Direction = M021_NORMALIZE",
            "G0308_i_PartyDirection = P0600_i_Direction;",
        ],
    },
    {
        "id": "step_tuple_mutation",
        "file": "MOVESENS.C",
        "line_range": [438, 444],
        "claim": "Successful step commands mutate the party tuple by writing G0306/G0307 from F0267 destination coordinates.",
        "needles": [
            "if (P0560_i_DestinationMapX >= 0)",
            "if (P0557_T_Thing == C0xFFFF_THING_PARTY)",
            "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
        ],
    },
    {
        "id": "main_loop_draw_consumes_tuple",
        "file": "GAMELOOP.C",
        "line_range": [88, 90],
        "claim": "The game loop calls F0128 with the current G0308/G0306/G0307 tuple.",
        "needles": [
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
        ],
    },
    {
        "id": "f0128_consumes_draw_args_before_viewport",
        "file": "DUNVIEW.C",
        "line_range": [8318, 8611],
        "claim": "F0128 receives direction/mapX/mapY parameters and requests the dungeon viewport from that draw path.",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "REGISTER int16_t P0183_i_Direction",
            "REGISTER int16_t P0184_i_MapX",
            "REGISTER int16_t P0185_i_MapY",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "order": [
            "REGISTER int16_t P0183_i_Direction",
            "REGISTER int16_t P0184_i_MapX",
            "REGISTER int16_t P0185_i_MapY",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
    },
]


def norm(text: str) -> str:
    return " ".join(text.split())


def block(path: Path, a: int, b: int) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    return "\n".join(lines[a - 1 : min(b, len(lines))])


def check_one(spec: dict) -> dict:
    path = SOURCE_ROOT / spec["file"]
    raw = block(path, *spec["line_range"])
    compact = norm(raw)
    missing = [needle for needle in spec.get("needles", []) if norm(needle) not in compact]
    order_missing = []
    order_ok = True
    pos = -1
    for needle in spec.get("order", []):
        idx = compact.find(norm(needle), pos + 1)
        if idx < 0:
            order_missing.append(needle)
            order_ok = False
        else:
            pos = idx
    return {
        "id": spec["id"],
        "file": spec["file"],
        "line_range": spec["line_range"],
        "claim": spec["claim"],
        "ok": not missing and order_ok,
        "missing": missing,
        "order_missing": order_missing,
    }


def load_pass278() -> dict:
    if not PASS278.exists():
        return {"present": False, "reason": f"missing {PASS278}"}
    data = json.loads(PASS278.read_text(encoding="utf-8"))
    return {
        "present": True,
        "manifest": str(PASS278.relative_to(ROOT)),
        "status": data.get("status"),
        "addresses": data.get("addresses", {}),
        "proof_predicates": data.get("proof_predicates", {}),
        "blocker": data.get("blocker"),
        "transcript": data.get("transcript"),
        "route_keylog": data.get("route_keylog"),
    }


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    checks = [check_one(spec) for spec in SOURCE_CHECKS]
    pass278 = load_pass278()
    runtime_pred = pass278.get("proof_predicates", {}) if pass278.get("present") else {}
    source_chain_proven = all(c["ok"] for c in checks)
    runtime_chain_proven = bool(runtime_pred) and all(runtime_pred.values())
    f0380_runtime_seen = bool(runtime_pred.get("f0380_dequeue_hit_seen"))
    f0128_runtime_seen = bool(runtime_pred.get("f0128_draw_hit_seen"))
    status = "SOURCE_PROVEN_RUNTIME_F0380_BLOCKED"
    if source_chain_proven and runtime_chain_proven:
        status = "PROVEN_SOURCE_AND_RUNTIME_CHAIN"
    elif not source_chain_proven:
        status = "BLOCKED_SOURCE_AUDIT_FAILED"

    manifest = {
        "schema": "pass284_dm1_v1_f0380_dequeue_ordering_proof.v1",
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "source_root": str(SOURCE_ROOT),
        "source_chain_proven": source_chain_proven,
        "runtime_chain_proven": runtime_chain_proven,
        "f0380_runtime_seen": f0380_runtime_seen,
        "f0128_runtime_seen": f0128_runtime_seen,
        "source_checks": checks,
        "queue_index_count_resolution": {
            "dm1_v1_pc34_queue_model": "first/last circular indices G0433/G0434; empty when (G0434+1 wrapped) == G0433",
            "dequeue_order": "read G0432[G0433].Command, apply movement-disabled gate before dequeue, then read X/Y, optionally decrement G2153 only on MEDIA728, increment/wrap G0433, unlock/replay, dispatch",
            "count_note": "G2153_i_QueuedCommandsCount decrement is guarded by MEDIA728_I34E/I35 and is not the PC-34 DM1 V1 queue authority; the PC-34 source path is resolved by G0433/G0434 index ordering.",
        },
        "imported_runtime_evidence": pass278,
        "decision": "Static ReDMCSB source proves controlled key -> G0432 write -> index-based dequeue ordering -> tuple mutation -> F0128 consumption. Existing runtime evidence proves controlled keys, G0432 write, tuple mutation, and F0128 hit, but disproves promotion of the full runtime chain because no F0380 dequeue BP hit is present.",
        "exact_remaining_blocker": None if runtime_chain_proven else "No proven runtime F0380 dequeue breakpoint hit at 22F4:0699 in the imported pass278 transcript; do not claim runtime hook despite G0432 write, party tuple mutation, and F0128 draw hit.",
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    pred_json = json.dumps(runtime_pred, indent=2, sort_keys=True)
    check_lines = "\n".join(
        f"- {'PASS' if c['ok'] else 'FAIL'} `{c['id']}` — `{c['file']}:{c['line_range'][0]}-{c['line_range'][1]}`: {c['claim']}"
        for c in checks
    )
    report_manifest = pass278.get("manifest") if pass278.get("present") else "missing pass278 manifest"
    report_transcript = pass278.get("transcript") if pass278.get("present") else "n/a"
    REPORT.write_text(
        f"""# Pass284 — DM1 V1 F0380 dequeue ordering proof

Status: `{status}`

## Verdict

- Source chain: `{'PROVEN' if source_chain_proven else 'BLOCKED'}`.
- Runtime chain: `{'PROVEN' if runtime_chain_proven else 'NOT PROMOTED'}`.
- F0380 runtime BP hit: `{'seen' if f0380_runtime_seen else 'not seen'}`.
- F0128 runtime BP hit: `{'seen' if f0128_runtime_seen else 'not seen'}`.

## Source-locked ordering

{check_lines}

## Queue index/count resolution

DM1 V1 PC-34 resolves dequeue by circular first/last indices (`G0433_i_CommandQueueFirstIndex`, `G0434_i_CommandQueueLastIndex`). The `G2153_i_QueuedCommandsCount--` statement exists only under the later `MEDIA728_I34E/I35` guard; it is not the PC-34 authority. The PC-34 ordering is: read `G0432[G0433].Command`, gate movement before dequeue, read X/Y, advance/wrap `G0433`, unlock/replay pending click, then dispatch turn/step.

## Imported runtime predicates

From `{report_manifest}`:

```json
{pred_json}
```

## Decision

Static ReDMCSB evidence proves the requested controlled-key to draw-consumption ordering. Existing runtime evidence still cannot promote the full chain: it has controlled key delivery, `G0432` write, party tuple mutation, and `F0128` draw hit, but no proven `F0380_COMMAND_ProcessQueue_CPSC` dequeue BP hit at `22F4:0699`.

## Artifacts

- Manifest: `parity-evidence/verification/pass284_dm1_v1_f0380_dequeue_ordering_proof/manifest.json`
- Imported pass278 transcript: `{report_transcript}`
""",
        encoding="utf-8",
    )
    print(json.dumps({"status": status, "manifest": str(OUT / "manifest.json"), "report": str(REPORT)}, indent=2))
    return 0 if source_chain_proven else 1


if __name__ == "__main__":
    raise SystemExit(main())
