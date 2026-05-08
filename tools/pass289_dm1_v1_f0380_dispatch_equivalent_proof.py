#!/usr/bin/env python3
"""Pass289 DM1 V1 F0380 dispatch-equivalent proof."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT = ROOT / "parity-evidence/verification/pass289_dm1_v1_f0380_dispatch_equivalent_proof"
REPORT = ROOT / "parity-evidence/pass289_dm1_v1_f0380_dispatch_equivalent_proof.md"
PASS278 = ROOT / "parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json"
PASS284 = ROOT / "parity-evidence/verification/pass284_dm1_v1_f0380_dequeue_ordering_proof/manifest.json"

SOURCE_CHECKS = [
    {
        "id": "controlled_pc34_key_rows",
        "file": "COMMAND.C",
        "line_range": [252, 260],
        "claim": "PC-34 movement keyboard input maps keypad/arrow controls to C001/C002 turns and C003..C006 movement commands.",
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
        "id": "f0361_key_to_g0432_g0434_count",
        "file": "COMMAND.C",
        "line_range": [1734, 1812],
        "claim": "F0361 scans keyboard tables, reserves next circular slot, writes command to G0432, advances G0434, and increments guarded G2153 count where present.",
        "needles": [
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "G0434_i_CommandQueueLastIndex + 2",
            "G0433_i_CommandQueueFirstIndex",
            "G0443_ps_PrimaryKeyboardInput",
            "G0444_ps_SecondaryKeyboardInput",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G2153_i_QueuedCommandsCount++;",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
        ],
        "order": [
            "G0443_ps_PrimaryKeyboardInput",
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "G0434_i_CommandQueueLastIndex + 2",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
        ],
    },
    {
        "id": "f0380_dequeue_source_equivalent",
        "file": "COMMAND.C",
        "line_range": [2045, 2156],
        "claim": "F0380 tests G0433/G0434 empty state, reads G0432[G0433], gates disabled movement, reads X/Y, decrements guarded G2153, advances/wraps G0433, unlocks/replays, then dispatches turns/moves.",
        "needles": [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "AL1159_i_CommandQueueIndex = G0434_i_CommandQueueLastIndex + 1",
            "AL1159_i_CommandQueueIndex == G0433_i_CommandQueueFirstIndex",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G0310_i_DisabledMovementTicks",
            "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
            "L1162_i_CommandY = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Y;",
            "G2153_i_QueuedCommandsCount--;",
            "++G0433_i_CommandQueueFirstIndex",
            "F0360_COMMAND_ProcessPendingClick();",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "order": [
            "AL1159_i_CommandQueueIndex = G0434_i_CommandQueueLastIndex + 1",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X;",
            "++G0433_i_CommandQueueFirstIndex",
            "F0360_COMMAND_ProcessPendingClick();",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        ],
    },
    {
        "id": "main_loop_calls_f0380",
        "file": "GAMELOOP.C",
        "line_range": [160, 216],
        "claim": "The active game loop reads keyboard input through F0361 and then calls F0380_COMMAND_ProcessQueue_CPSC, giving the dispatch-equivalent caller seam when direct F0380 body BP misses.",
        "needles": ["F0361_COMMAND_ProcessKeyPress", "F0380_COMMAND_ProcessQueue_CPSC();"],
        "order": ["F0361_COMMAND_ProcessKeyPress", "F0380_COMMAND_ProcessQueue_CPSC();"],
    },
    {
        "id": "turn_dispatch_mutates_g0308",
        "file": "CHAMPION.C",
        "line_range": [117, 130],
        "claim": "Turn dispatch mutates the party tuple by writing G0308_i_PartyDirection.",
        "needles": ["L0835_ps_Champion->Cell", "L0835_ps_Champion->Direction", "G0308_i_PartyDirection = P0600_i_Direction;"],
    },
    {
        "id": "move_dispatch_mutates_g0306_g0307",
        "file": "MOVESENS.C",
        "line_range": [316, 556],
        "claim": "Move dispatch reaches F0267, where successful party movement writes G0306_i_PartyMapX/G0307_i_PartyMapY; pit/teleporter variants repeat tuple writes before draw/sensor consequences.",
        "needles": [
            "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE",
            "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
            "F0284_CHAMPION_SetPartyDirection",
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, P0560_i_DestinationMapX, P0561_i_DestinationMapY);",
        ],
    },
    {
        "id": "f0128_consumes_tuple_from_gameloop",
        "file": "GAMELOOP.C",
        "line_range": [88, 90],
        "claim": "The main loop redraw consumes the current party tuple by passing G0308/G0306/G0307 to F0128.",
        "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);"],
    },
    {
        "id": "dunview_f0128_argument_consumption",
        "file": "DUNVIEW.C",
        "line_range": [8318, 8611],
        "claim": "DUNVIEW.C F0128 receives direction/mapX/mapY parameters and ends by drawing the dungeon viewport.",
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
    raw = block(SOURCE_ROOT / spec["file"], *spec["line_range"])
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
        "line_range": spec["line_range"],
        "claim": spec["claim"],
        "ok": not missing and not order_missing,
        "missing": missing,
        "order_missing": order_missing,
    }


def load_json(path: Path) -> dict:
    if not path.exists():
        return {"present": False, "path": str(path.relative_to(ROOT)), "reason": "missing"}
    data = json.loads(path.read_text(encoding="utf-8"))
    data["present"] = True
    data["path"] = str(path.relative_to(ROOT))
    return data


def pass281_probe() -> dict:
    matches = sorted(p.relative_to(ROOT).as_posix() for p in (ROOT / "parity-evidence").glob("**/*pass281*"))
    matches += sorted(p.relative_to(ROOT).as_posix() for p in (ROOT / "tools").glob("*pass281*"))
    return {"present": bool(matches), "matches": matches, "note": "No pass281 artifact exists in this worktree" if not matches else "pass281 artifacts present"}


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    checks = [check_one(spec) for spec in SOURCE_CHECKS]
    pass278 = load_json(PASS278)
    pass284 = load_json(PASS284)
    pass281 = pass281_probe()

    predicates = pass278.get("proof_predicates", {}) if pass278.get("present") else {}
    runtime_equivalent_proven = all(predicates.get(key) is True for key in [
        "route_posted_controlled_keys",
        "g0432_write_seen",
        "party_tuple_mutation_seen",
        "f0128_draw_hit_seen",
        "cpu_memdump_after_stops",
    ])
    direct_f0380_seen = predicates.get("f0380_dequeue_hit_seen") is True
    source_chain_proven = all(c["ok"] for c in checks)
    dispatch_equivalent_proven = source_chain_proven and runtime_equivalent_proven and not direct_f0380_seen
    status = "DISPATCH_EQUIVALENT_PROVEN_DIRECT_F0380_BP_BLOCKED" if dispatch_equivalent_proven else "BLOCKED"

    source_map_justification = {
        "caller_seam": "GAMELOOP.C:160-216 reads keyboard input with F0361 and then calls F0380_COMMAND_ProcessQueue_CPSC at line 215.",
        "queue_authority": "COMMAND.C:6-11 defines G0432 and circular G0433/G0434; G2153 exists, but for this PC-34 route the source proof is index authority plus guarded count updates.",
        "dequeue_equivalence": "COMMAND.C:2045-2156 establishes that any observed G0432 write followed by G0306/G0307/G0308 mutation and later F0128 draw on the main-loop route traverses the F0380 dispatch semantics, even though BP 22F4:0699 itself did not stop.",
        "draw_consumption": "GAMELOOP.C:88-90 passes G0308/G0306/G0307 into DUNVIEW.C F0128; DUNVIEW.C:8318-8611 consumes those args before viewport draw.",
    }

    manifest = {
        "schema": "pass289_dm1_v1_f0380_dispatch_equivalent_proof.v1",
        "timestamp_utc": "2026-05-07T01:05:44Z",
        "status": status,
        "source_root": str(SOURCE_ROOT),
        "source_chain_proven": source_chain_proven,
        "runtime_equivalent_proven": runtime_equivalent_proven,
        "direct_f0380_body_bp_seen": direct_f0380_seen,
        "dispatch_equivalent_proven": dispatch_equivalent_proven,
        "source_checks": checks,
        "source_map_justification": source_map_justification,
        "imported_pass278": {
            "present": pass278.get("present"),
            "path": pass278.get("path"),
            "status": pass278.get("status"),
            "addresses": pass278.get("addresses"),
            "proof_predicates": predicates,
            "transcript": pass278.get("transcript"),
            "route_keylog": pass278.get("route_keylog"),
            "blocker": pass278.get("blocker"),
        },
        "pass281_inspection": pass281,
        "imported_pass284": {
            "present": pass284.get("present"),
            "path": pass284.get("path"),
            "status": pass284.get("status"),
            "source_chain_proven": pass284.get("source_chain_proven"),
            "runtime_chain_proven": pass284.get("runtime_chain_proven"),
            "exact_remaining_blocker": pass284.get("exact_remaining_blocker"),
        },
        "exact_smaller_blocker": "Direct F0380 body breakpoint at pass278 address 22F4:0699 still does not hit; reproduce with pass278 route and breakpoints from its manifest/transcript. The dispatch-equivalent seam is GAMELOOP.C:215 -> COMMAND.C:F0380 source ordering, not a direct runtime BP claim.",
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    checks_md = "\n".join(
        f"- {'PASS' if c['ok'] else 'FAIL'} `{c['id']}` — `{c['file']}:{c['line_range'][0]}-{c['line_range'][1]}`: {c['claim']}"
        for c in checks
    )
    predicates_md = json.dumps(predicates, indent=2, sort_keys=True)
    pass281_md = "none found" if not pass281["matches"] else "\n".join(f"- `{m}`" for m in pass281["matches"])
    report = f"""# Pass289 — DM1 V1 F0380 dispatch-equivalent proof

Status: `{status}`

## Verdict

- Source chain: `{'PROVEN' if source_chain_proven else 'BLOCKED'}`.
- Dispatch-equivalent runtime chain: `{'PROVEN' if runtime_equivalent_proven else 'BLOCKED'}`.
- Direct F0380 body breakpoint: `{'seen' if direct_f0380_seen else 'not seen'}`.
- Final claim: `{'dispatch-equivalent proven; direct F0380 BP remains blocked' if dispatch_equivalent_proven else 'blocked; see manifest'}`.

## Required chain

{checks_md}

## Source-map justification

- Caller seam: {source_map_justification['caller_seam']}
- Queue/index/count: {source_map_justification['queue_authority']}
- Dequeue equivalent: {source_map_justification['dequeue_equivalence']}
- Draw consumption: {source_map_justification['draw_consumption']}

## Prior evidence inspection

- pass278: `{pass278.get('status')}` from `{pass278.get('path')}`.
- pass281: {pass281_md}.
- pass284: `{pass284.get('status')}` from `{pass284.get('path')}`; exact blocker was `{pass284.get('exact_remaining_blocker')}`.

Runtime predicates imported from pass278:

```json
{predicates_md}
```

## Narrowed blocker

Direct F0380 body breakpoint `22F4:0699` still did not hit. Reproduce with the pass278 route and debugger setup in `{pass278.get('path')}` / `{pass278.get('transcript')}`. This pass therefore lands the smaller, honest result: source-locked dispatch-equivalent proof via `GAMELOOP.C:215 -> COMMAND.C:F0380`, plus runtime observation of controlled keys, `G0432`, tuple mutation, and `F0128`; no direct F0380 runtime hook claim.

## Artifacts

- Manifest: `parity-evidence/verification/pass289_dm1_v1_f0380_dispatch_equivalent_proof/manifest.json`
- Report: `parity-evidence/pass289_dm1_v1_f0380_dispatch_equivalent_proof.md`
"""
    REPORT.write_text(report, encoding="utf-8")
    return 0 if status.startswith("DISPATCH_EQUIVALENT") else 1


if __name__ == "__main__":
    raise SystemExit(main())
