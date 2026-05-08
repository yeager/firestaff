#!/usr/bin/env python3
"""Pass235: build the next DM1 PC34 movement->viewport runtime trace packet.

This pass is deliberately a blocker/driver handoff, not a promotion. It
combines the pass233 movement and pass234 viewport blockers into one ordered
trace packet, verifies the ReDMCSB source anchors that must be hit in the
original FIRES runtime, and emits the exact missing symbol/debugger evidence.
"""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
SYMBOL_MAP = ROOT / "data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json"
RUNTIME_IMAGE = ROOT / "data/original_runtime/dm1_pc34_i34e_runtime_image.v1.json"
PASS233 = ROOT / "parity-evidence/verification/pass233_dm1_v1_movement_runtime_hit_blocker/manifest.json"
PASS234 = ROOT / "parity-evidence/verification/pass234_dm1_v1_viewport_runtime_hit_blocker/manifest.json"
OUT_DIR = ROOT / "parity-evidence/verification/pass235_dm1_v1_movement_viewport_runtime_trace_packet"
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/pass235_dm1_v1_movement_viewport_runtime_trace_packet.md"

TRACE_EVENTS: list[dict[str, Any]] = [
    {"id":"command_accepted","file":"COMMAND.C","function":"F0380_COMMAND_ProcessQueue_CPSC","line_range":[2075,2156],"needles":{2095:"L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",2126:"G0435_B_CommandQueueLocked = C0_FALSE;",2151:"F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",2155:"F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"},"runtime_observable":"code breakpoint after queue unlock or on turn/move dispatch, proving accepted command id and X/Y came from G0432_as_CommandQueue"},
    {"id":"turn_or_step_state_applied","file":"CLIKMENU.C","function":"F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty","line_range":[156,328],"needles":{172:"F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE",269:"F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement(G0308_i_PartyDirection",278:"L1117_B_MovementBlocked = C0_FALSE;",317:"if (L1117_B_MovementBlocked) {",328:"F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);"},"runtime_observable":"code breakpoint after direction write for turns or after legal step calls F0267, with party dir/source/destination context captured"},
    {"id":"party_coordinates_committed","file":"MOVESENS.C","function":"F0267_MOVE_GetMoveResult_CPSCE","line_range":[438,506],"needles":{442:"G0306_i_PartyMapX = P0560_i_DestinationMapX;",443:"G0307_i_PartyMapY = P0561_i_DestinationMapY;",493:"if (P0557_T_Thing == C0xFFFF_THING_PARTY) {",494:"G0306_i_PartyMapX = P0560_i_DestinationMapX;",495:"G0307_i_PartyMapY = P0561_i_DestinationMapY;"},"runtime_observable":"data watchpoints on resolved G0306/G0307 addresses, tied back to the MOVESENS write site CS:IP"},
    {"id":"draw_uses_mutated_tuple","file":"GAMELOOP.C","function":"F0002_MAIN_GameLoop_CPSDF","line_range":[80,90],"needles":{84:"if (!G0300_B_PartyIsResting) {",88:"if (!G0423_i_InventoryChampionOrdinal) {",90:"F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);"},"runtime_observable":"code breakpoint at the F0128 call proving the draw consumes the post-command G0308/G0306/G0307 tuple"},
    {"id":"viewport_buffer_composed","file":"DUNVIEW.C","function":"F0128_DUNGEONVIEW_Draw_CPSF","line_range":[8336,8611],"needles":{8357:"G0076_B_UseFlippedWallAndFootprintsBitmaps = (P0184_i_MapX + P0185_i_MapY + P0183_i_Direction) & 0x0001",8367:"F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport);",8542:"F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",8610:"F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"},"runtime_observable":"code breakpoint after viewport composition before F0097, with P0183/P0184/P0185 and G0296 pointer captured"},
    {"id":"viewport_present","file":"DRAWVIEW.C","function":"F0097_DUNGEONVIEW_DrawViewport","line_range":[849,857],"needles":{850:"F0638_GetZone(C007_ZONE_VIEWPORT, L2414_ai_XYZ);",857:"(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);"},"runtime_observable":"code breakpoint on the PC34 viewport blit call proving composed G0296 reaches C007_ZONE_VIEWPORT presentation"},
]

BLOCKER_ID = "blocked/runtime-base-and-symbol-map-unavailable"


def load_json(path: Path) -> dict[str, Any] | None:
    if not path.is_file():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def source_line(path: Path, line_no: int) -> str | None:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    if 1 <= line_no <= len(lines):
        return " ".join(lines[line_no - 1].split())
    return None


def audit_event(event: dict[str, Any]) -> dict[str, Any]:
    path = SOURCE_ROOT / event["file"]
    found: dict[str, str] = {}
    missing: dict[str, str] = {}
    if not path.is_file():
        return {**event, "source_path": str(path), "ok": False, "found": found, "missing": {str(k): v for k, v in event["needles"].items()}, "error": "source file missing"}
    for line_no, needle in event["needles"].items():
        line = source_line(path, int(line_no))
        compact_needle = " ".join(needle.split())
        if line is not None and compact_needle in line:
            found[str(line_no)] = line
        else:
            missing[str(line_no)] = compact_needle
    return {**event, "source_path": str(path), "ok": not missing, "found": found, "missing": missing}


def symbol_guardrail() -> dict[str, Any]:
    symbol_map = load_json(SYMBOL_MAP)
    if symbol_map is None:
        return {"exists": False, "entry_count": 0, "promotable_entries": [], "missing_entries": [e["id"] for e in TRACE_EVENTS]}
    entries = symbol_map.get("entries") or []
    by_id = {entry.get("id"): entry for entry in entries if isinstance(entry, dict)}
    promotable = []
    missing_runtime = []
    for event in TRACE_EVENTS:
        entry = by_id.get(event["id"])
        if entry and entry.get("confidence") == "verified_runtime_hit" and (entry.get("runtime_cs_ip") or entry.get("global_addresses")):
            promotable.append(event["id"])
        else:
            missing_runtime.append(event["id"])
    return {"exists": True, "entry_count": len(entries), "promotable_entries": promotable, "missing_verified_runtime_hit_entries": missing_runtime, "promotion_rule": symbol_map.get("promotion_rule")}


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# Pass235 — DM1 PC34 movement→viewport runtime trace packet",
        "",
        f"Status: `{manifest['status']}`.",
        "",
        "This is the next trace packet after pass229/pass230/pass233/pass234. It does not promote any static source line or EXENEW offset; it only narrows the original-runtime hit list that must be captured before CS:IP/global addresses can enter the symbol map.",
        "",
        "## Ordered runtime-hit chain",
        "",
    ]
    for item in manifest["source_audit"]:
        mark = "PASS" if item["ok"] else "FAIL"
        lines.append(f"- {mark} `{item['id']}` — `{item['file']}:{item['line_range'][0]}-{item['line_range'][1]}` / `{item['function']}`")
        lines.append(f"  - runtime observable: {item['runtime_observable']}")
        for line_no, text in item["found"].items():
            lines.append(f"  - line {line_no}: `{text}`")
        for line_no, text in item["missing"].items():
            lines.append(f"  - missing line {line_no}: `{text}`")
    guard = manifest["symbol_map_guardrail"]
    lines += [
        "",
        "## Promotion guardrail",
        "",
        f"- symbol map: `{SYMBOL_MAP}`",
        f"- runtime image fixture: `{RUNTIME_IMAGE}`",
        f"- promotable entries now: `{guard.get('promotable_entries', [])}`",
        f"- entries still requiring verified runtime hits: `{guard.get('missing_verified_runtime_hit_entries', guard.get('missing_entries'))}`",
        "- required formula: `runtime_cs = (PSP + 0x10) + map_segment`; `runtime_ip = map_offset`",
        "",
        "## Blocker",
        "",
        f"Exact blocker: `{manifest['blocker']['id']}` — {manifest['blocker']['exact_missing_input']}",
        "",
        "Required debugger evidence: a reproducible FIRES.MAP/public-symbol table or debugger-observed bindings for the six events above, including PSP/load segment, runtime CS:IP, and G0306/G0307/G0308/G0296 data addresses where applicable.",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    source_audit = [audit_event(event) for event in TRACE_EVENTS]
    guard = symbol_guardrail()
    previous = {"pass233": load_json(PASS233), "pass234": load_json(PASS234)}
    source_ok = all(item["ok"] for item in source_audit)
    no_promotions = not guard.get("promotable_entries")
    status = "BLOCKED_RUNTIME_HITS_REQUIRED" if source_ok and no_promotions else "FAIL_TRACE_PACKET_GUARD"
    manifest = {"schema": "pass235_dm1_v1_movement_viewport_runtime_trace_packet.v1", "status": status, "source_root": str(SOURCE_ROOT), "source_audit": source_audit, "symbol_map_guardrail": guard, "previous_blocker_status": {name: doc.get("status") if doc else None for name, doc in previous.items()}, "blocker": {"id": BLOCKER_ID, "exact_missing_input": "Need FIRES.MAP/public symbols or another reproducible debugger binding from ReDMCSB source seams/globals to loaded original FIRES CS:IP/data addresses."}}
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": status, "source_ok": source_ok, "manifest": str(MANIFEST), "report": str(REPORT)}, indent=2, sort_keys=True))
    return 0 if status == "BLOCKED_RUNTIME_HITS_REQUIRED" else 1


if __name__ == "__main__":
    raise SystemExit(main())
