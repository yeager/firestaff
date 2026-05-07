#!/usr/bin/env python3
"""Pass312 DM1 V1 original runtime party tuple/F0128 state-oracle source lock.

This gate intentionally uses ReDMCSB source as primary evidence and the existing
N2 original-runtime debugger/capture manifests as bounded runtime support. It
promotes only the state-oracle blocker; it does not promote pixels.
"""
from __future__ import annotations

import argparse
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

PASS = "pass312_dm1_v1_original_runtime_state_oracle"
ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT_JSON = ROOT / f"parity-evidence/verification/{PASS}.json"
OUT_MD = ROOT / f"parity-evidence/{PASS}.md"
PASS278 = ROOT / "parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json"
PASS308 = ROOT / "parity-evidence/verification/pass308_original_capture_execution_manifest.json"

SOURCE_CHECKS: list[dict[str, Any]] = [
    {"id": "main_loop_draws_current_tuple", "file": "GAMELOOP.C", "line_range": [80, 90], "needles": ["!G0300_B_PartyIsResting", "!G0423_i_InventoryChampionOrdinal", "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)"]},
    {"id": "main_loop_processes_command_queue", "file": "GAMELOOP.C", "line_range": [214, 215], "needles": ["F0380_COMMAND_ProcessQueue_CPSC()"]},
    {"id": "command_queue_dequeues_command", "file": "COMMAND.C", "line_range": [2045, 2126], "needles": ["void F0380_COMMAND_ProcessQueue_CPSC", "G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command", "L1161_i_CommandX = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].X", "if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)"]},
    {"id": "command_dispatch_turn_or_move", "file": "COMMAND.C", "line_range": [2150, 2156], "needles": ["F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command)", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command)"]},
    {"id": "turn_mutates_party_direction", "file": "CLIKMENU.C", "line_range": [142, 173], "needles": ["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(G0308_i_PartyDirection + ((P0734_i_Command == C002_COMMAND_TURN_RIGHT) ? 1 : 3)))"]},
    {"id": "direction_setter_writes_g0308", "file": "CHAMPION.C", "line_range": [93, 130], "needles": ["void F0284_CHAMPION_SetPartyDirection", "if (P0600_i_Direction == G0308_i_PartyDirection)", "G0308_i_PartyDirection = P0600_i_Direction"]},
    {"id": "move_computes_destination_and_calls_move_result", "file": "CLIKMENU.C", "line_range": [180, 329], "needles": ["void F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY"]},
    {"id": "move_result_writes_party_xy", "file": "MOVESENS.C", "line_range": [438, 443], "needles": ["if (P0557_T_Thing == C0xFFFF_THING_PARTY)", "G0306_i_PartyMapX = P0560_i_DestinationMapX", "G0307_i_PartyMapY = P0561_i_DestinationMapY"]},
    {"id": "teleporter_case_updates_tuple_direction", "file": "MOVESENS.C", "line_range": [493, 517], "needles": ["G0306_i_PartyMapX = P0560_i_DestinationMapX", "G0307_i_PartyMapY = P0561_i_DestinationMapY", "F0284_CHAMPION_SetPartyDirection"]},
    {"id": "falling_case_draws_with_tuple", "file": "MOVESENS.C", "line_range": [550, 556], "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, P0560_i_DestinationMapX, P0561_i_DestinationMapY)"]},
    {"id": "f0128_accepts_direction_xy", "file": "DUNVIEW.C", "line_range": [8318, 8324], "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "P0183_i_Direction", "P0184_i_MapX", "P0185_i_MapY"]},
    {"id": "f0128_uses_tuple_in_view_calculation", "file": "DUNVIEW.C", "line_range": [8356, 8542], "needles": ["(P0184_i_MapX + P0185_i_MapY + P0183_i_Direction)", "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY)"]},
    {"id": "f0128_presents_viewport", "file": "DUNVIEW.C", "line_range": [8604, 8610], "needles": ["F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)"]},
]


def compact(text: str) -> str:
    return " ".join(text.split())


def audit_source() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for check in SOURCE_CHECKS:
        path = SOURCE_ROOT / check["file"]
        lines = path.read_text(encoding="latin-1", errors="replace").splitlines() if path.exists() else []
        start, end = check["line_range"]
        block = compact("\n".join(lines[start - 1 : min(end, len(lines))]))
        missing = [needle for needle in check["needles"] if compact(needle) not in block]
        rows.append({
            "id": check["id"],
            "file": check["file"],
            "functionOrSeam": check["id"],
            "lineRange": check["line_range"],
            "ok": path.exists() and not missing,
            "missingNeedles": missing,
        })
    return rows


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def build() -> dict[str, Any]:
    source = audit_source()
    p278 = load_json(PASS278)
    p308 = load_json(PASS308)
    predicates = p278.get("proof_predicates", {})
    runtime_required = {
        "route_posted_controlled_keys": predicates.get("route_posted_controlled_keys") is True,
        "g0432_write_seen": predicates.get("g0432_write_seen") is True,
        "party_tuple_mutation_seen": predicates.get("party_tuple_mutation_seen") is True,
        "f0128_draw_hit_seen": predicates.get("f0128_draw_hit_seen") is True,
        "cpu_memdump_after_stops": predicates.get("cpu_memdump_after_stops") is True,
    }
    capture_coverage = p308.get("coverage", {})
    state_oracle_ok = all(r["ok"] for r in source) and all(runtime_required.values()) and capture_coverage.get("requiredLabelCoverage") is True and capture_coverage.get("requiredPromotionRowsGameplayOrWallCloseup") is True
    return {
        "pass": PASS,
        "status": "PASS_STATE_ORACLE_SOURCE_RUNTIME_BOUND" if state_oracle_ok else "BLOCKED_STATE_ORACLE_INCOMPLETE",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "scope": "DM1 V1 original-runtime state oracle for command input -> queue write -> party tuple mutation -> F0128 viewport draw consumption. No pixel parity or bitmap publication is claimed.",
        "primaryEvidence": "ReDMCSB source audit under ~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
        "sourceAudit": source,
        "runtimeSupport": {
            "manifest": "parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json",
            "status": p278.get("status"),
            "requiredPredicates": runtime_required,
            "directF0380BreakpointSeen": predicates.get("f0380_dequeue_hit_seen") is True,
            "directF0380BreakpointRequiredForThisOracle": False,
            "reasonDirectF0380IsNonBlocking": "ReDMCSB source proves F0380 queue dequeue and turn/move dispatch; pass278 independently proves controlled original-runtime queue writes, party tuple mutation, and F0128 draw hit. A direct F0380 BP remains useful for a stronger binary-hook claim but is not required for this source-first state oracle.",
        },
        "captureCoverageSupport": {
            "manifest": "parity-evidence/verification/pass308_original_capture_execution_manifest.json",
            "coverage": capture_coverage,
            "status": p308.get("status"),
        },
        "promotionDecision": {
            "partyTupleF0128StateOracle": state_oracle_ok,
            "unblocksOriginalCapturePromotionToComparatorInputs": state_oracle_ok,
            "pixelParityClaim": False,
            "nextRequiredGate": "run matched original-vs-Firestaff viewport comparator on promoted route labels and require non-duplicate, semantically matched outputs before any pixel-parity wording",
        },
        "notClaimed": ["original-vs-Firestaff pixel parity", "direct F0380 binary breakpoint/dequeue hit", "tracked bitmap-byte publication", "screenshot promotion without comparator pass"],
    }


def render_md(m: dict[str, Any]) -> str:
    lines = [
        f"# {PASS}",
        "",
        f"- status: `{m['status']}`",
        "- pixels/screenshots: not promoted by this pass",
        "- primary evidence: ReDMCSB source audit",
        "",
        "## State-oracle decision",
        "",
        f"- party tuple/F0128 oracle: `{m['promotionDecision']['partyTupleF0128StateOracle']}`",
        f"- comparator-input unblock: `{m['promotionDecision']['unblocksOriginalCapturePromotionToComparatorInputs']}`",
        f"- pixel parity claim: `{m['promotionDecision']['pixelParityClaim']}`",
        "",
        "## Source anchors",
        "",
        "| seam | source line range | status |",
        "|---|---|---|",
    ]
    for row in m["sourceAudit"]:
        rng = f"{row['file']}:{row['lineRange'][0]}-{row['lineRange'][1]}"
        lines.append(f"| `{row['id']}` | `{rng}` | `{'PASS' if row['ok'] else 'BLOCKED'}` |")
    lines += [
        "",
        "## Runtime support",
        "",
        f"- pass278 status: `{m['runtimeSupport']['status']}`",
        f"- required predicates: `{m['runtimeSupport']['requiredPredicates']}`",
        f"- direct F0380 breakpoint required here: `{m['runtimeSupport']['directF0380BreakpointRequiredForThisOracle']}`",
        "",
        "## Capture coverage support",
        "",
        f"- pass308 status: `{m['captureCoverageSupport']['status']}`",
        f"- coverage: `{m['captureCoverageSupport']['coverage']}`",
        "",
        "## Non-claims",
    ]
    lines += [f"- {x}" for x in m["notClaimed"]]
    lines += ["", "## Next gate", "", m["promotionDecision"]["nextRequiredGate"]]
    return "\n".join(lines) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--write", action="store_true")
    args = ap.parse_args()
    m = build()
    if args.write:
        OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
        OUT_JSON.write_text(json.dumps(m, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        OUT_MD.write_text(render_md(m), encoding="utf-8")
    print(f"{PASS}: {m['status']} oracle={m['promotionDecision']['partyTupleF0128StateOracle']}")
    return 0 if m["status"].startswith("PASS_") else 1


if __name__ == "__main__":
    raise SystemExit(main())
