#!/usr/bin/env python3
"""Pass296 DM1 V1 movement input-to-tuple proof without direct F0380 BP.

This is a deterministic gate: it re-checks the source chain, imports the
runtime-supported predicates from pass278/pass289/pass293, and deliberately
marks the direct F0380 entry breakpoint as blocked-but-not-required.
"""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT = ROOT / "parity-evidence/verification/pass296_dm1_v1_input_tuple_proof_without_direct_f0380"
REPORT = ROOT / "parity-evidence/pass296_dm1_v1_input_tuple_proof_without_direct_f0380.md"

PASS278 = ROOT / "parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json"
PASS289 = ROOT / "parity-evidence/verification/pass289_dm1_v1_f0380_dispatch_equivalent_proof/manifest.json"
PASS293 = ROOT / "parity-evidence/verification/pass293_dm1_v1_direct_f0380_hook_address_window/manifest.json"

SOURCE_CHECKS = [
    {
        "id": "controlled_key_mapping",
        "file": "COMMAND.C",
        "function": "PC-34 movement keyboard table / F0361_COMMAND_ProcessKeyPress",
        "line_range": [252, 260],
        "claim": "PC-34 controlled movement keys map to turn/move commands before enqueue.",
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
        "id": "keypress_enqueue_predicates",
        "file": "COMMAND.C",
        "function": "F0361_COMMAND_ProcessKeyPress",
        "line_range": [1734, 1812],
        "claim": "F0361 locks the queue, selects the next slot, writes G0432, advances G0434, increments the guarded count, unlocks, then processes pending click replay.",
        "needles": [
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "G0434_i_CommandQueueLastIndex + 2",
            "G0433_i_CommandQueueFirstIndex",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G2153_i_QueuedCommandsCount++;",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
        ],
        "order": [
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G2153_i_QueuedCommandsCount++;",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
        ],
    },
    {
        "id": "gameloop_dispatch_equivalent_seam",
        "file": "GAMELOOP.C",
        "function": "F0002_MAIN_GameLoop_CPSDF",
        "line_range": [160, 216],
        "claim": "The active game loop scans keys with F0361 and then invokes F0380, creating the accepted caller seam when direct F0380 entry BP misses.",
        "needles": ["F0361_COMMAND_ProcessKeyPress", "F0380_COMMAND_ProcessQueue_CPSC();"],
        "order": ["F0361_COMMAND_ProcessKeyPress", "F0380_COMMAND_ProcessQueue_CPSC();"],
    },
    {
        "id": "f0380_dequeue_dispatch_source_chain",
        "file": "COMMAND.C",
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "line_range": [2045, 2156],
        "claim": "F0380 source semantics dequeue G0432/G0433, apply the disabled-movement gate, advance/wrap the queue, replay pending click, then dispatch turn/move commands.",
        "needles": [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "AL1159_i_CommandQueueIndex == G0433_i_CommandQueueFirstIndex",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G0310_i_DisabledMovementTicks",
            "G2153_i_QueuedCommandsCount--;",
            "++G0433_i_CommandQueueFirstIndex",
            "F0360_COMMAND_ProcessPendingClick();",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "order": [
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "++G0433_i_CommandQueueFirstIndex",
            "F0360_COMMAND_ProcessPendingClick();",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
    },
    {
        "id": "party_tuple_mutation",
        "file": "MOVESENS.C",
        "function": "F0267_MOVE_GetMoveResult_CPSCE",
        "line_range": [316, 556],
        "claim": "Successful movement writes the party tuple G0306/G0307 and calls direction/draw paths for movement consequences.",
        "needles": [
            "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE",
            "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
            "F0284_CHAMPION_SetPartyDirection",
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, P0560_i_DestinationMapX, P0561_i_DestinationMapY);",
        ],
    },
    {
        "id": "main_redraw_consumes_tuple",
        "file": "GAMELOOP.C",
        "function": "F0002_MAIN_GameLoop_CPSDF -> F0128_DUNGEONVIEW_Draw_CPSF",
        "line_range": [88, 90],
        "claim": "The main redraw passes the current party tuple G0308/G0306/G0307 into F0128.",
        "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);"],
    },
    {
        "id": "dunview_consumes_tuple_args",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "line_range": [8318, 8611],
        "claim": "F0128 receives direction/mapX/mapY arguments and finishes by drawing the dungeon viewport.",
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


def read_block(file_name: str, first: int, last: int) -> str:
    lines = (SOURCE_ROOT / file_name).read_text(encoding="latin-1", errors="replace").splitlines()
    return "\n".join(lines[first - 1 : min(last, len(lines))])


def check_source(spec: dict) -> dict:
    raw = read_block(spec["file"], *spec["line_range"])
    compact = norm(raw)
    missing = [needle for needle in spec.get("needles", []) if norm(needle) not in compact]
    order_missing = []
    pos = -1
    for needle in spec.get("order", []):
        idx = compact.find(norm(needle), pos + 1)
        if idx < 0:
            order_missing.append(needle)
        else:
            pos = idx
    return {
        "id": spec["id"],
        "file": spec["file"],
        "function": spec["function"],
        "line_range": spec["line_range"],
        "claim": spec["claim"],
        "ok": not missing and not order_missing,
        "missing": missing,
        "order_missing": order_missing,
    }


def load_manifest(path: Path) -> dict:
    data = json.loads(path.read_text(encoding="utf-8"))
    data["path"] = path.relative_to(ROOT).as_posix()
    return data


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    checks = [check_source(spec) for spec in SOURCE_CHECKS]
    p278 = load_manifest(PASS278)
    p289 = load_manifest(PASS289)
    p293 = load_manifest(PASS293)

    p278_pred = p278.get("proof_predicates", {})
    p293_pred = p293.get("proof_predicates", {})

    controlled_key_supported = p278_pred.get("route_posted_controlled_keys") is True and p293_pred.get("controlled_keys_posted") is True
    queue_predicates_supported = p278_pred.get("g0432_write_seen") is True and p293_pred.get("queue_or_index_bpm_seen") is True
    tuple_mutation_supported = p278_pred.get("party_tuple_mutation_seen") is True
    draw_consumption_supported = p278_pred.get("f0128_draw_hit_seen") is True
    after_stop_state_supported = p278_pred.get("cpu_memdump_after_stops") is True
    dispatch_equivalent_supported = p289.get("dispatch_equivalent_proven") is True and p289.get("runtime_equivalent_proven") is True
    direct_f0380_blocked = (
        p278_pred.get("f0380_dequeue_hit_seen") is False
        and p289.get("direct_f0380_body_bp_seen") is False
        and p293.get("direct_f0380_claim") is False
        and p293_pred.get("direct_f0380_entry_bp_hit") is False
    )
    source_chain_locked = all(check["ok"] for check in checks) and p289.get("source_chain_proven") is True

    proof_gate_passed = all([
        source_chain_locked,
        controlled_key_supported,
        queue_predicates_supported,
        dispatch_equivalent_supported,
        tuple_mutation_supported,
        draw_consumption_supported,
        after_stop_state_supported,
        direct_f0380_blocked,
    ])
    status = "MOVEMENT_INPUT_TUPLE_PROOF_SOURCE_LOCKED_RUNTIME_SUPPORTED_DIRECT_F0380_BLOCKED_NOT_REQUIRED" if proof_gate_passed else "BLOCKED"

    proof_chain = [
        {
            "step": "controlled key",
            "source": "COMMAND.C:252-260 movement keyboard table; COMMAND.C:F0361",
            "runtime_support": "pass278 route_posted_controlled_keys=true and pass293 controlled_keys_posted=true",
            "ok": controlled_key_supported and checks[0]["ok"],
        },
        {
            "step": "queue predicates",
            "source": "COMMAND.C:F0361 writes G0432/G0434/G2153; COMMAND.C:F0380 reads/dequeues G0432/G0433",
            "runtime_support": "pass278 g0432_write_seen=true and pass293 queue_or_index_bpm_seen=true",
            "ok": queue_predicates_supported and checks[1]["ok"] and checks[3]["ok"],
        },
        {
            "step": "dispatch-equivalent/source chain",
            "source": "GAMELOOP.C:F0002 calls F0361 then F0380; COMMAND.C:F0380 dispatches F0365/F0366",
            "runtime_support": "pass289 dispatch_equivalent_proven=true via pass278 runtime predicates",
            "ok": dispatch_equivalent_supported and checks[2]["ok"] and checks[3]["ok"],
        },
        {
            "step": "party tuple mutation",
            "source": "MOVESENS.C:F0267 writes G0306/G0307 and uses G0308 direction path",
            "runtime_support": "pass278 party_tuple_mutation_seen=true with CPU/MEMDUMP after stops",
            "ok": tuple_mutation_supported and after_stop_state_supported and checks[4]["ok"],
        },
        {
            "step": "F0128 draw consumption",
            "source": "GAMELOOP.C:88-90 passes G0308/G0306/G0307; DUNVIEW.C:F0128 receives P0183/P0184/P0185 and draws viewport",
            "runtime_support": "pass278 f0128_draw_hit_seen=true",
            "ok": draw_consumption_supported and checks[5]["ok"] and checks[6]["ok"],
        },
    ]

    direct_f0380_decision = {
        "required_for_this_gate": False,
        "blocked": direct_f0380_blocked,
        "pass278": p278.get("blocker"),
        "pass289": p289.get("exact_smaller_blocker"),
        "pass293": p293.get("exact_blocker"),
    }

    completion_wording = (
        "DM1 V1 movement proof status: source-locked and runtime-supported from controlled key "
        "to queue predicates to dispatch-equivalent source chain to party tuple mutation to F0128 draw consumption. "
        "Direct F0380 entry/body BP remains blocked and is explicitly not claimed or required by this gate."
        if proof_gate_passed else
        "DM1 V1 movement proof status: blocked; inspect manifest source_checks, proof_chain, and imported pass predicates."
    )

    manifest = {
        "schema": "pass296_dm1_v1_input_tuple_proof_without_direct_f0380.v1",
        "timestamp_utc": "2026-05-07T01:22:00Z",
        "status": status,
        "source_root": str(SOURCE_ROOT),
        "source_chain_locked": source_chain_locked,
        "runtime_support": {
            "controlled_key_supported": controlled_key_supported,
            "queue_predicates_supported": queue_predicates_supported,
            "dispatch_equivalent_supported": dispatch_equivalent_supported,
            "tuple_mutation_supported": tuple_mutation_supported,
            "draw_consumption_supported": draw_consumption_supported,
            "after_stop_state_supported": after_stop_state_supported,
        },
        "direct_f0380_decision": direct_f0380_decision,
        "proof_gate_passed": proof_gate_passed,
        "proof_chain": proof_chain,
        "source_checks": checks,
        "imported_manifests": {
            "pass278": {
                "path": p278["path"],
                "status": p278.get("status"),
                "proof_predicates": p278_pred,
                "transcript": p278.get("transcript"),
                "route_keylog": p278.get("route_keylog"),
            },
            "pass289": {
                "path": p289["path"],
                "status": p289.get("status"),
                "source_chain_proven": p289.get("source_chain_proven"),
                "runtime_equivalent_proven": p289.get("runtime_equivalent_proven"),
                "dispatch_equivalent_proven": p289.get("dispatch_equivalent_proven"),
                "direct_f0380_body_bp_seen": p289.get("direct_f0380_body_bp_seen"),
            },
            "pass293": {
                "path": p293["path"],
                "status": p293.get("status"),
                "proof_predicates": p293_pred,
                "direct_f0380_claim": p293.get("direct_f0380_claim"),
                "transcript": p293.get("runtime", {}).get("transcript"),
                "route_keylog": p293.get("runtime", {}).get("route_keylog"),
            },
        },
        "completion_wording": completion_wording,
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    chain_md = "\n".join(
        f"- {'PASS' if step['ok'] else 'FAIL'} **{step['step']}** — source: `{step['source']}`; runtime: {step['runtime_support']}."
        for step in proof_chain
    )
    checks_md = "\n".join(
        f"- {'PASS' if c['ok'] else 'FAIL'} `{c['file']}:{c['line_range'][0]}-{c['line_range'][1]}` / `{c['function']}` — {c['claim']}"
        for c in checks
    )
    report = f"""# Pass296 — DM1 V1 input-to-party-tuple proof without direct F0380 BP

Status: `{status}`

## Verdict

{completion_wording}

## Proof chain

{chain_md}

## Source citations checked

{checks_md}

## Imported runtime support

- pass278 `{p278.get("status")}`: controlled keys, `G0432` queue write, party tuple mutation, CPU/MEMDUMP after stops, and `F0128` draw hit are true; direct F0380 dequeue hit is false.
- pass289 `{p289.get("status")}`: source chain and dispatch-equivalent runtime chain are true; direct F0380 body BP is false.
- pass293 `{p293.get("status")}`: direct F0380 claim is false; queue/index watch support is true; exact blocker remains the direct entry BP/address window.

## Direct F0380 decision

Direct F0380 entry/body breakpoint proof is **blocked, not claimed, and not required** for this gate. The accepted proof is source-locked/runtime-supported through `GAMELOOP.C:F0002 -> COMMAND.C:F0380` dispatch-equivalence, with pass278 runtime evidence carrying queue write, tuple mutation, and draw consumption.

## Artifacts

- Manifest: `parity-evidence/verification/pass296_dm1_v1_input_tuple_proof_without_direct_f0380/manifest.json`
- Report: `parity-evidence/pass296_dm1_v1_input_tuple_proof_without_direct_f0380.md`
"""
    REPORT.write_text(report, encoding="utf-8")
    print(json.dumps({"status": status, "proof_gate_passed": proof_gate_passed, "manifest": str((OUT / "manifest.json").relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if proof_gate_passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
