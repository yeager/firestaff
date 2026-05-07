#!/usr/bin/env python3
"""Pass316: verify the N2 direct F0380/F0097 runtime probe outcome.

This is intentionally conservative.  It records the exact code-only breakpoint
attempt after pass315, but promotes no new runtime seam unless the probe manifest
contains an explicit debugger hit for F0380 at 22F4:0699 or F0097 at 2809:1E31.
"""
from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PROBE_DIR = ROOT / "parity-evidence/verification/pass316_dm1_v1_f0380_f0097_direct_probe_attempt"
PROBE_MANIFEST = PROBE_DIR / "manifest.json"
PROBE_TRANSCRIPT = PROBE_DIR / "dosbox_debug_noise_reduced.clean.txt"
OUT_JSON = ROOT / "parity-evidence/verification/pass316_dm1_v1_f0380_f0097_direct_probe_attempt.json"
OUT_MD = ROOT / "parity-evidence/pass316_dm1_v1_f0380_f0097_direct_probe_attempt.md"

ADDR = {
    "F0380_COMMAND_ProcessQueue_CPSC": "22F4:0699",
    "F0128_DUNGEONVIEW_Draw_CPSF": "23AD:40FE",
    "F0097_DUNGEONVIEW_DrawViewport": "2809:1E31",
}

SOURCE_CHECKS: list[dict[str, Any]] = [
    {"id": "f0380_dequeue", "file": "COMMAND.C", "range": [2045, 2156], "needles": ["void F0380_COMMAND_ProcessQueue_CPSC", "G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command", "if (++G0433_i_CommandQueueFirstIndex > M529_COMMAND_QUEUE_SIZE)", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]},
    {"id": "f0097_viewport_present", "file": "DRAWVIEW.C", "range": [709, 858], "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "G0324_B_DrawViewportRequested = C1_TRUE", "F0638_GetZone(C007_ZONE_VIEWPORT", "VIDRV_09_BlitViewPort"]},
    {"id": "f0128_calls_f0097", "file": "DUNVIEW.C", "range": [8318, 8611], "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "G0296_puc_Bitmap_Viewport", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)"]},
    {"id": "mainloop_order", "file": "GAMELOOP.C", "range": [80, 215], "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)", "F0380_COMMAND_ProcessQueue_CPSC()"]},
    {"id": "turn_move_dispatch", "file": "CLIKMENU.C", "range": [142, 328], "needles": ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0267_MOVE_GetMoveResult_CPSCE"]},
    {"id": "move_result_tuple", "file": "MOVESENS.C", "range": [316, 556], "needles": ["BOOLEAN F0267_MOVE_GetMoveResult_CPSCE", "G0306_i_PartyMapX = P0560_i_DestinationMapX", "G0307_i_PartyMapY = P0561_i_DestinationMapY", "F0128_DUNGEONVIEW_Draw_CPSF"]},
]


def compact(s: str) -> str:
    return " ".join(s.split())


def audit_source() -> list[dict[str, Any]]:
    rows = []
    for chk in SOURCE_CHECKS:
        path = SOURCE_ROOT / chk["file"]
        lines = path.read_text(encoding="latin-1", errors="replace").splitlines() if path.exists() else []
        start, end = chk["range"]
        block = compact("\n".join(lines[start - 1 : min(end, len(lines))]))
        found, missing = {}, []
        for needle in chk["needles"]:
            c = compact(needle)
            line_no = None
            for idx in range(start - 1, min(end, len(lines))):
                if c in compact(lines[idx]):
                    line_no = idx + 1
                    break
            if line_no is None and c not in block:
                missing.append(needle)
            else:
                found[needle] = line_no
        rows.append({**chk, "ok": path.exists() and not missing, "found": found, "missing": missing, "source_path": str(path)})
    return rows


def bounded_excerpt(text: str) -> list[str]:
    keep = []
    pat = re.compile(r"(STOP|BP 22F4:0699|BP 2809:1E31|BP 23AD:40FE|Breakpoint list|\(Running\)F4:0699)", re.I)
    for line in text.splitlines():
        if pat.search(line):
            keep.append(line[:180])
        if len(keep) >= 40:
            break
    return keep


def main() -> int:
    probe = json.loads(PROBE_MANIFEST.read_text(encoding="utf-8"))
    transcript = PROBE_TRANSCRIPT.read_text(encoding="utf-8", errors="replace") if PROBE_TRANSCRIPT.exists() else ""
    hits = probe.get("debugger_hits_captured", [])
    kinds = {h.get("kind") for h in hits}
    f0380_hit = "f0380_bp" in kinds
    f0097_hit = "f0097_bp" in kinds
    f0128_hit = "f0128_bp" in kinds or probe.get("proof_predicates", {}).get("f0128_draw_hit_seen") is True
    bp_commands = [e.get("cmd") for e in probe.get("debugger_events", []) if e.get("event") == "debugger_cmd" and str(e.get("cmd", "")).startswith("BP ")]
    source = audit_source()
    setup_collision_seen = "BP 23AD:40FE_  int at 22F4:0699" in transcript and "00. BP 2809:1E31" in transcript and "01. BP 23AD:40FE" in transcript
    status = "PROMOTED_DIRECT_RUNTIME_HIT" if (f0380_hit or f0097_hit) and all(r["ok"] for r in source) else "BLOCKED_DIRECT_F0380_F0097_AFTER_CODE_ONLY_PROBE"
    manifest = {
        "schema": "pass316_dm1_v1_f0380_f0097_direct_probe_attempt.v1",
        "status": status,
        "attempted_command": "python3 tools/pass316_dm1_v1_f0380_f0097_direct_probe_attempt.py --seconds 75",
        "addresses": ADDR,
        "source_audit": source,
        "probe_manifest": str(PROBE_MANIFEST.relative_to(ROOT)),
        "debugger_bp_commands": bp_commands[:10],
        "debugger_hit_kinds": sorted(k for k in kinds if k),
        "direct_hits": {"f0380_22F4_0699": f0380_hit, "f0097_2809_1E31": f0097_hit, "f0128_23AD_40FE_stayed_seen": f0128_hit},
        "probe_predicates": probe.get("proof_predicates", {}),
        "bounded_sanitized_transcript_excerpt": bounded_excerpt(transcript),
        "narrowed_blocker": "Code-only BP probe armed 22F4:0699, 23AD:40FE, and 2809:1E31 under the pass278 DOSBox-debug/tmux/Xvfb/xdotool route. The run again captured only the existing F0128 hit. The F0380 BP command appears to collide during setup (22F4:0699 text appears while setting subsequent BPs, but BPLIST retains only 2809:1E31 and 23AD:40FE), so this is not promotable as a controlled dequeue hit. F0097 at 2809:1E31 did not stop before timeout.",
        "next_missing_input_or_tool": "Need a debugger sequencing method that arms BP 22F4:0699 after the initial game-loop/setup collision without losing route control, or a validated finer F0097/VIDRV_09_BlitViewPort offset beyond 2809:1E31 that stops after F0128. pass312 must remain sourced from pass278/pass315, not this blocked attempt.",
    }
    OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass316 — DM1 V1 F0380/F0097 direct runtime probe attempt",
        "",
        f"Status: `{status}`",
        "",
        "## Result",
        "",
        f"- Direct F0380 dequeue hit at `22F4:0699`: `{f0380_hit}`.",
        f"- Direct F0097 viewport-present hit at `2809:1E31`: `{f0097_hit}`.",
        f"- Existing F0128 runtime hit at `23AD:40FE` stayed visible: `{f0128_hit}`.",
        "- pass316 does not promote any new runtime seam.",
        "",
        "## Exact attempted command",
        "",
        "`python3 tools/pass316_dm1_v1_f0380_f0097_direct_probe_attempt.py --seconds 75`",
        "",
        "## Narrowed blocker",
        "",
        manifest["narrowed_blocker"],
        "",
        "Next missing input/tool: " + manifest["next_missing_input_or_tool"],
        "",
        "Manifest: `parity-evidence/verification/pass316_dm1_v1_f0380_f0097_direct_probe_attempt.json`.",
    ]
    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "f0380_hit": f0380_hit, "f0097_hit": f0097_hit, "f0128_hit": f0128_hit, "manifest": str(OUT_JSON)}, indent=2, sort_keys=True))
    return 0 if status.startswith("BLOCKED_") or status.startswith("PROMOTED_") else 1


if __name__ == "__main__":
    raise SystemExit(main())
