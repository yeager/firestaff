#!/usr/bin/env python3
"""Pass315: conservative DM1 V1 F0128 runtime-hit verifier.

Verifies the narrow runtime fact unblocked on N2: DOSBox-debug command control
can set a code breakpoint and the original DM1 PC34 runtime hit F0128 at
23AD:40FE. This deliberately does not claim the full F0380 dequeue or viewport
present chain.
"""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS278 = ROOT / "parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json"
SYMBOL_MAP = ROOT / "data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json"
OUT = ROOT / "parity-evidence/verification/pass315_dm1_v1_f0128_runtime_hit_verifier"
REPORT = ROOT / "parity-evidence/pass315_dm1_v1_f0128_runtime_hit_verifier.md"

SOURCE_CHECKS: list[dict[str, Any]] = [
    {"id": "gameloop_draw_call", "file": "GAMELOOP.C", "function": "F0002_MAIN_GameLoop_CPSDF", "line_range": [55, 95], "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);"]},
    {"id": "f0128_viewport_compose", "file": "DUNVIEW.C", "function": "F0128_DUNGEONVIEW_Draw_CPSF", "line_range": [8318, 8611], "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "G0296_puc_Bitmap_Viewport", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"]},
    {"id": "f0097_viewport_present", "file": "DRAWVIEW.C", "function": "F0097_DUNGEONVIEW_DrawViewport", "line_range": [709, 858], "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "C007_ZONE_VIEWPORT", "VIDRV_09_BlitViewPort"]},
    {"id": "f0380_command_dispatch", "file": "COMMAND.C", "function": "F0380_COMMAND_ProcessQueue_CPSC", "line_range": [2045, 2156], "needles": ["void F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]},
    {"id": "clickmenu_movement_dispatch", "file": "CLIKMENU.C", "function": "F0365/F0366 movement dispatch", "line_range": [142, 328], "needles": ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0267_MOVE_GetMoveResult_CPSCE"]},
    {"id": "champion_turn_state_write", "file": "CHAMPION.C", "function": "F0284_CHAMPION_SetPartyDirection", "line_range": [117, 130], "needles": ["G0308_i_PartyDirection = P0600_i_Direction;"]},
    {"id": "movesens_step_state_write", "file": "MOVESENS.C", "function": "F0267_MOVE_GetMoveResult_CPSCE", "line_range": [438, 444], "needles": ["G0306_i_PartyMapX = P0560_i_DestinationMapX;", "G0307_i_PartyMapY = P0561_i_DestinationMapY;"]},
]


def norm(s: str) -> str:
    return " ".join(s.split())


def audit_source() -> list[dict[str, Any]]:
    out = []
    for spec in SOURCE_CHECKS:
        path = SOURCE_ROOT / spec["file"]
        lines = path.read_text(encoding="latin-1", errors="replace").splitlines() if path.exists() else []
        start, end = spec["line_range"]
        block = "\n".join(lines[start - 1 : min(end, len(lines))])
        compact = norm(block)
        missing = [needle for needle in spec["needles"] if norm(needle) not in compact]
        anchors = {}
        for needle in spec["needles"]:
            n = norm(needle)
            for idx in range(start - 1, min(end, len(lines))):
                if n in norm(lines[idx]):
                    anchors[needle] = idx + 1
                    break
        out.append({**spec, "source_path": str(path), "ok": path.exists() and not missing, "missing": missing, "anchors": anchors})
    return out


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    audit = audit_source()
    p278 = json.loads(PASS278.read_text(encoding="utf-8"))
    smap = json.loads(SYMBOL_MAP.read_text(encoding="utf-8"))
    transcript_path = ROOT / p278.get("transcript", "")
    transcript = transcript_path.read_text(encoding="utf-8", errors="replace") if transcript_path.exists() else ""
    f0128_addr = p278.get("addresses", {}).get("F0128_DUNGEONVIEW_Draw_CPSF")
    hit_seen = p278.get("proof_predicates", {}).get("f0128_draw_hit_seen") is True and f"BP {f0128_addr}" in transcript
    debugger_control_seen = any(e.get("cmd") == f"BP {f0128_addr}" for e in p278.get("debugger_events", [])) and any(e.get("cmd") == "BPLIST" for e in p278.get("debugger_events", []))
    viewport_entry = next((e for e in smap.get("entries", []) if e.get("id") == "viewport_buffer_composed"), {})
    symbol_updated = viewport_entry.get("confidence") == "verified_runtime_hit" and viewport_entry.get("runtime_cs_ip") == f0128_addr
    f0380_blocked = p278.get("proof_predicates", {}).get("f0380_dequeue_hit_seen") is not True
    status = "F0128_RUNTIME_HIT_VERIFIED_F0380_REMAINS_BLOCKED" if all([all(a["ok"] for a in audit), hit_seen, debugger_control_seen, symbol_updated, f0380_blocked]) else "BLOCKED"
    manifest = {
        "schema": "pass315_dm1_v1_f0128_runtime_hit_verifier.v1",
        "status": status,
        "source_audit": audit,
        "runtime_evidence": {
            "pass278_manifest": str(PASS278.relative_to(ROOT)),
            "transcript": p278.get("transcript"),
            "f0128_runtime_cs_ip": f0128_addr,
            "hit_signature_required": f"BP {f0128_addr}",
            "hit_seen": hit_seen,
            "debugger_control_seen": debugger_control_seen,
            "proof_predicates": p278.get("proof_predicates", {}),
        },
        "symbol_map_check": {"path": str(SYMBOL_MAP.relative_to(ROOT)), "viewport_buffer_composed_updated": symbol_updated, "entry": viewport_entry},
        "remaining_blocker": "Direct F0380 dequeue BP at 22F4:0699 is still not observed; viewport_present/F0097 is still unpromoted. F0128 hit is a narrow viewport composition runtime fact, not a full command-to-present chain.",
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = ["# Pass315 — DM1 V1 F0128 runtime-hit verifier", "", f"Status: `{status}`", "", "## Runtime result", "", f"- Verified narrow F0128 breakpoint hit: `{hit_seen}` at `{f0128_addr}`.", f"- Debugger command control seen: `{debugger_control_seen}`.", f"- Symbol-map entry `viewport_buffer_composed` promoted: `{symbol_updated}`.", "- Scope guard: F0380 dequeue and F0097 viewport present remain blocked/unpromoted.", "", "## Source anchors", ""]
    for item in audit:
        lines.append("- {} {} lines {}-{} ok={}".format(item["file"], item["function"], item["line_range"][0], item["line_range"][1], item["ok"]))
        for needle, line in item["anchors"].items():
            lines.append(f"  - line {line}: `{needle}`")
    lines += ["", "Evidence manifest: `parity-evidence/verification/pass315_dm1_v1_f0128_runtime_hit_verifier/manifest.json`."]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "manifest": str(OUT / "manifest.json"), "report": str(REPORT)}, indent=2, sort_keys=True))
    return 0 if status.startswith("F0128_RUNTIME_HIT_VERIFIED") else 1


if __name__ == "__main__":
    raise SystemExit(main())
