#!/usr/bin/env python3
"""Pass241: bind FIRES.EXENEW static CS:IP candidates to captured runtime CS:IP candidates.

This pass is deliberately narrower than a runtime-hit promotion.  It uses only
text manifests already in the repo:

* pass235 captured the DOSBox runtime entry/load segment for the decompressed
  FIRES.EXENEW fixture.
* pass237 derived candidate-only static CS:IP anchors from FIRES.EXENEW static
  disassembly and ReDMCSB source seams.

The result is a reproducible, non-manual bridge to numeric runtime CS:IP
*candidates* suitable for DOSBox breakpoints.  It does not mark any symbol-map
entry as verified_runtime_hit.
"""
from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS235 = ROOT / "parity-evidence/verification/pass235_dm1_v1_dosbox_debugger_workflow/manifest.json"
PASS237 = ROOT / "parity-evidence/verification/pass237_dm1_v1_fires_static_csip_crosswalk/manifest.json"
SYMBOL_MAP = ROOT / "data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json"
OUT_DIR = ROOT / "parity-evidence/verification/pass241_dm1_v1_runtime_csip_candidate_bridge"
REPORT = ROOT / "parity-evidence/pass241_dm1_v1_runtime_csip_candidate_bridge.md"

EXPECTED_SHA256 = "fc79ac65046e3d96c189ac3dd20ad40bacb8debee2cd1c7d2c33ca2d8f82fe94"

SYMBOL_MAP_IDS = {
    "command_accepted": "command_accepted",
    "turn_types_1_to_2": "turn_or_step_state_applied",
    "move_types_3_to_6": "turn_or_step_state_applied",
    "move_get_move_result": "party_coordinates_committed",
    "viewport_game_loop_draw_call_site": "draw_uses_mutated_tuple",
}

REQUIRED_CHAIN = {
    "command": {"command_accepted"},
    "movement": {"turn_types_1_to_2", "move_types_3_to_6", "move_get_move_result"},
    "viewport": {"viewport_game_loop_draw_call_site"},
}


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def parse_csip(csip: str) -> tuple[int, int]:
    if not re.fullmatch(r"[0-9A-Fa-f]{4}:[0-9A-Fa-f]{4}", csip or ""):
        raise ValueError(f"bad CS:IP: {csip!r}")
    cs, ip = csip.split(":")
    return int(cs, 16), int(ip, 16)


def fmt_csip(cs: int, ip: int) -> str:
    if not (0 <= cs <= 0xFFFF and 0 <= ip <= 0xFFFF):
        raise ValueError(f"CS:IP out of 16-bit range: {cs:x}:{ip:x}")
    return f"{cs:04x}:{ip:04x}"


def runtime_candidate(load_segment: int, static_cs_ip: str) -> str:
    static_cs, static_ip = parse_csip(static_cs_ip)
    return fmt_csip(load_segment + static_cs, static_ip)


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    pass235 = load_json(PASS235)
    pass237 = load_json(PASS237)
    symbol_map = load_json(SYMBOL_MAP)

    entry = pass235.get("entry_capture", {})
    entry_cs_ip = entry.get("entry_runtime_cs_ip")
    entry_cs, entry_ip = parse_csip(entry_cs_ip)
    psp_segment = entry.get("psp_segment_inferred")
    inferred_load = int(psp_segment, 16) + 0x10 if psp_segment else None

    source_audit = pass235.get("source_audit", [])
    source_audit_seams = [
        {
            "id": item.get("id"),
            "file": item.get("file"),
            "function": item.get("function"),
            "line_range": item.get("line_range"),
            "ok": item.get("ok") is True,
        }
        for item in source_audit
        if item.get("id") in {"command_accepted", "turn_handler", "move_handler", "move_result", "draw_uses_mutated_tuple", "viewport_buffer_composed", "viewport_present"}
    ]

    bridged_symbol_ids = set(SYMBOL_MAP_IDS.values())
    promoted_bridged_entries = [
        e.get("id")
        for e in symbol_map.get("entries", [])
        if e.get("id") in bridged_symbol_ids and e.get("confidence") == "verified_runtime_hit"
    ]

    guards = {
        "pass235_source_audit_ok": bool(source_audit_seams) and all(item["ok"] for item in source_audit_seams),
        "pass235_entry_capture_ok": entry.get("ok") is True,
        "pass235_entry_ip_zero": entry_ip == 0,
        "pass235_load_segment_matches_psp_plus_10": inferred_load == entry_cs,
        "pass235_fires_sha256_ok": entry.get("fires_exenew", {}).get("sha256") == EXPECTED_SHA256 and entry.get("fires_exenew", {}).get("sha256_matches_expected") is True,
        "pass237_static_status_ok": pass237.get("status") == "CANDIDATE_ONLY_RUNTIME_HITS_REQUIRED",
        "pass237_fires_sha256_ok": pass237.get("fires_input", {}).get("sha256") == EXPECTED_SHA256,
        "symbol_map_bridged_candidate_entries_unpromoted": not promoted_bridged_entries,
    }

    rows: list[dict[str, Any]] = []
    for candidate in pass237.get("candidates", []):
        static_cs_ip = candidate.get("candidate_static_cs_ip")
        cid = candidate.get("id")
        rows.append({
            "id": cid,
            "symbol_map_entry_id": SYMBOL_MAP_IDS.get(cid),
            "source_function": candidate.get("source_function"),
            "source_file": candidate.get("source_file"),
            "static_cs_ip": static_cs_ip,
            "runtime_cs_ip_candidate": runtime_candidate(entry_cs, static_cs_ip),
            "load_segment": f"{entry_cs:04x}",
            "confidence": candidate.get("confidence"),
            "classification": "runtime_candidate_only_not_verified_hit",
            "promotion_blocker": candidate.get("blocker_to_promote"),
        })

    present = {row["id"] for row in rows}
    chain_ok = {name: bool(ids & present) for name, ids in REQUIRED_CHAIN.items()}
    status = "PASS_RUNTIME_CSIP_CANDIDATES_BRIDGED_NO_PROMOTIONS" if all(guards.values()) and all(chain_ok.values()) else "FAIL_RUNTIME_CSIP_CANDIDATE_BRIDGE_GUARD"

    manifest = {
        "schema": "pass241_dm1_v1_runtime_csip_candidate_bridge.v1",
        "status": status,
        "inputs": {
            "pass235_manifest": str(PASS235),
            "pass237_manifest": str(PASS237),
            "symbol_map": str(SYMBOL_MAP),
        },
        "source_audit_seams": source_audit_seams,
        "load_segment_source": {
            "entry_runtime_cs_ip": entry_cs_ip,
            "psp_segment_inferred": psp_segment,
            "load_segment": f"{entry_cs:04x}",
            "formula": "runtime_cs = captured_load_segment + static_cs; runtime_ip = static_ip",
            "capture_kind": "pass235 automated DOSBox-debug entry capture for decompressed FIRES.EXENEW fixture",
        },
        "guards": guards,
        "chain_coverage": chain_ok,
        "runtime_csip_candidates": rows,
        "promotion_rule": "These rows are numeric breakpoint candidates only. Keep bridged data/original_runtime symbol-map entries unpromoted until a debugger-observed seam hit proves the command/movement/viewport state transition.",
        "existing_unrelated_promotions_ignored": [
            e.get("id")
            for e in symbol_map.get("entries", [])
            if e.get("confidence") == "verified_runtime_hit" and e.get("id") not in bridged_symbol_ids
        ],
        "artifact_policy": {"text_only": True, "no_original_binaries_committed": True, "no_source_or_log_dump": True},
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass241 — DM1 V1 runtime CS:IP candidate bridge",
        "",
        f"Status: `{status}`",
        "",
        "## Result",
        "",
        "Pass235 already captured FIRES.EXENEW runtime entry/load segment in DOSBox. Pass237 already produced static disassembly candidates tied to ReDMCSB source seams. This pass binds those two text artifacts into numeric DOSBox breakpoint candidates without promoting any runtime hit.",
        "",
        f"- Captured load segment: `{entry_cs:04x}` from entry `{entry_cs_ip}`; PSP `{psp_segment}` confirms PSP+0x10.",
        f"- Formula: `runtime_cs = 0x{entry_cs:04x} + static_cs`; `runtime_ip = static_ip`.",
        f"- FIRES.EXENEW sha256 guard: `{EXPECTED_SHA256}`.",
        "",
        "## ReDMCSB source seam audit",
        "",
    ]
    for seam in source_audit_seams:
        lines.append("- {id}: {file} {line_range} / {function} — ok={ok}.".format(**seam))
    lines.extend([
        "",
        "## Candidate bridge",
        "",
    ])
    for row in rows:
        lines.append("- {id} -> {symbol_map_entry_id}: static {static_cs_ip} => runtime candidate {runtime_cs_ip_candidate} ({source_function}).".format(**row))
    lines.extend([
        "",
        "## Guardrail",
        "",
        "No bridged candidate entry in `data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json` is promoted by this pass. These candidates are enough for a reproducible non-manual breakpoint bridge, but not enough to claim `verified_runtime_hit` until the debugger actually stops on the bridged seam with state evidence.",
        "",
        "Evidence manifest: `parity-evidence/verification/pass241_dm1_v1_runtime_csip_candidate_bridge/manifest.json`.",
        "",
    ])
    REPORT.write_text("\n".join(lines), encoding="utf-8")
    print(json.dumps({"status": status, "candidate_count": len(rows), "report": str(REPORT), "manifest": str(OUT_DIR / "manifest.json")}, indent=2, sort_keys=True))
    return 0 if status.startswith("PASS_") else 1


if __name__ == "__main__":
    raise SystemExit(main())
