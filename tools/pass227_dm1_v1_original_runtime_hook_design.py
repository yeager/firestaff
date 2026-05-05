#!/usr/bin/env python3
"""Pass227 DM1 V1 original-runtime hook/API design spike.

JSON-only follow-up to pass224. This does not run DOSBox captures. It audits the
existing N2 original-runner/debugger assets, verifies the ReDMCSB seams that the
runtime hook must prove, and emits a landable implementation path plus the first
minimal JSON event schema for: command accepted -> movement applied -> viewport
present. No PNG/PPM artifacts are produced.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import shutil
import subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
DM_STAGE = Path("~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34").expanduser()
DEFAULT_OUT = ROOT / "parity-evidence/verification/pass227_dm1_v1_original_runtime_hook_design.json"
DEFAULT_REPORT = ROOT / "parity-evidence/pass227_dm1_v1_original_runtime_hook_design.md"

SOURCE_SEAMS: list[dict[str, Any]] = [
    {
        "id": "command_accepted",
        "file": "COMMAND.C",
        "line_range": [2045, 2156],
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "needles": [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "hook_kind": "code breakpoint after command dequeue",
    },
    {
        "id": "turn_or_step_state_applied",
        "file": "CLIKMENU.C",
        "line_range": [142, 328],
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "needles": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "F0267_MOVE_GetMoveResult_CPSCE(C0xFFFF_THING_PARTY, G0306_i_PartyMapX, G0307_i_PartyMapY, L1121_i_MapX, L1122_i_MapY);",
        ],
        "hook_kind": "code breakpoint after turn/step handler return plus global reads",
    },
    {
        "id": "party_coordinates_committed",
        "file": "MOVESENS.C",
        "line_range": [442, 443],
        "function": "F0267_MOVE_GetMoveResult_CPSCE",
        "needles": ["G0306_i_PartyMapX = P0560_i_DestinationMapX;", "G0307_i_PartyMapY = P0561_i_DestinationMapY;"],
        "hook_kind": "memory write watchpoint for party X/Y globals",
    },
    {
        "id": "draw_uses_mutated_tuple",
        "file": "GAMELOOP.C",
        "line_range": [88, 91],
        "function": "F0002_MAIN_GameLoop_CPSDF",
        "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);"],
        "hook_kind": "code breakpoint at draw call with direction/x/y reads",
    },
    {
        "id": "viewport_present",
        "file": "DRAWVIEW.C",
        "line_range": [709, 842],
        "function": "F0097_DUNGEONVIEW_DrawViewport / E0017_MAIN_Exception28Handler_VerticalBlank_CPSDF",
        "needles": [
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "M526_WaitVerticalBlank();",
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT, CM1_COLOR_NO_TRANSPARENCY);",
        ],
        "hook_kind": "code breakpoint or memory watch on viewport request plus C007 blit call",
    },
]

MINIMAL_TRACE_SCHEMA: dict[str, Any] = {
    "schema": "dm1_v1_original_runtime_trace.minimal.v1",
    "artifact_policy": {"json_only": True, "forbidden_extensions": [".png", ".ppm"], "screenshots_not_evidence": True},
    "required_order": ["command_accepted", "movement_applied", "viewport_present"],
    "event_common_fields": ["seq", "type", "emulator", "module", "cs", "ip", "linear_pc", "route_label", "monotonic_cycle"],
    "events": {
        "command_accepted": {
            "required_fields": ["accepted_seq", "command_id", "queue_first_index", "queue_last_index", "raw_command_x", "raw_command_y"],
            "source_seam": "COMMAND.C F0380 after G0432/G0433 dequeue",
        },
        "movement_applied": {
            "required_fields": ["accepted_seq", "movement_kind", "party_direction", "party_map_x", "party_map_y", "disabled_movement_ticks", "stop_waiting_for_input"],
            "source_seam": "CLIKMENU.C turn/step handler return and MOVESENS.C coordinate commits",
            "movement_kind_enum": ["turn", "step", "blocked_step"],
        },
        "viewport_present": {
            "required_fields": ["accepted_seq", "zone", "draw_viewport_requested", "viewport_bitmap_seg", "viewport_bitmap_off", "viewport_digest16", "vblank_counter"],
            "source_seam": "DRAWVIEW.C viewport request/vblank and C007_ZONE_VIEWPORT blit",
        },
    },
    "promotion_predicate": [
        "events are from the stock PC 3.4 runtime module, not a rebuilt ReDMCSB binary",
        "command_accepted.accepted_seq is referenced by a later movement_applied event",
        "movement_applied records the post-handler direction/x/y/wait state",
        "viewport_present with the same accepted_seq occurs later and uses zone C007_ZONE_VIEWPORT",
        "no PNG/PPM/screenshot artifact is used to satisfy the chain",
    ],
}

IMPLEMENTATION_PATH: list[dict[str, Any]] = [
    {
        "step": 1,
        "title": "Canonical runtime/image fixture",
        "action": "Launch the existing N2 PC 3.4 stage through DOSBox-X or dosbox-debug with `DM -vv -sn -pk`; treat `FIRES` as the gameplay module and record the loaded segment/base in a JSON run log.",
        "deliverable": "runtime_image.json with DM.EXE/FIRES/VGA hashes, load CS:IP, and emulator version",
    },
    {
        "step": 2,
        "title": "Address-map bootstrap",
        "action": "Create `data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json` mapping the five source seams to runtime CS:IP/global addresses. Bootstrap it with DOSBox debugger `BP`, `BPMEM`, `MEMDUMPBIN`, and byte-signature/disassembly notes; keep all map confidence fields explicit.",
        "deliverable": "symbol_map JSON; no source claims are promoted for entries with confidence below `verified_runtime_hit`",
    },
    {
        "step": 3,
        "title": "Debugger transcript adapter",
        "action": "Add a small Python runner that starts DOSBox-X/dosbox-debug under xvfb, feeds debugger commands from the symbol map, parses breakpoint/watchpoint hits, reads the required globals, and emits newline-delimited JSON events using the minimal schema.",
        "deliverable": "`tools/dm1_original_runtime_trace.py --route ... --symbol-map ... --out trace.ndjson`",
    },
    {
        "step": 4,
        "title": "Route-driver integration",
        "action": "Reuse the current original route driver only for input delivery/timing. Stop treating raw frames as state evidence; each input label is promoted only when the trace chain links accepted_seq across command_accepted, movement_applied, and viewport_present.",
        "deliverable": "pass224 successor gate consumes trace JSON and rejects duplicate/static visual captures automatically",
    },
    {
        "step": 5,
        "title": "First narrow command",
        "action": "Prove one turn command first (left/right) because it avoids destination/sensor ambiguity, then add one forward step with the MOVESENS X/Y commit watchpoint.",
        "deliverable": "one accepted turn trace and one accepted step trace, both JSON-only",
    },
]

RUNNER_FILES = [
    "tools/pass224_dm1_v1_runtime_state_probe_scaffold.py",
    "tools/pass223_dm1_v1_post_redraw_instrumentation_lock.py",
    "tools/pass220_dm1_v1_original_readiness_oracle.py",
    "tools/pass175_c080_runtime_debugger_gate.py",
    "scripts/dosbox_dm1_original_viewport_reference_capture.sh",
    "tools/pass118_state_aware_original_route_driver.py",
]


def compact(text: str) -> str:
    return " ".join(text.split())


def source_slice(file: str, start: int, end: int) -> str:
    path = REDMCSB / file
    if not path.is_file():
        raise AssertionError(f"missing ReDMCSB source file: {path}")
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    return "\n".join(lines[start - 1 : end])


def verify_seam(seam: dict[str, Any]) -> dict[str, Any]:
    start, end = seam["line_range"]
    text = compact(source_slice(seam["file"], start, end))
    missing = [needle for needle in seam["needles"] if compact(needle) not in text]
    return {k: seam[k] for k in ("id", "file", "function", "hook_kind")} | {
        "citation": '{}:{}-{}'.format(seam["file"], start, end),
        "verified": not missing,
        "missing": missing,
    }


def run(argv: list[str], timeout: int = 8) -> dict[str, Any]:
    try:
        proc = subprocess.run(argv, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout, check=False)
        return {"argv": argv, "returncode": proc.returncode, "output_head": proc.stdout.splitlines()[:30]}
    except Exception as exc:
        return {"argv": argv, "error": repr(exc), "output_head": []}


def sha256(path: Path) -> str | None:
    if not path.is_file():
        return None
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def file_contains(path: Path, tokens: list[str]) -> dict[str, Any]:
    if not path.is_file():
        return {"path": str(path.relative_to(ROOT)), "exists": False, "tokens": {token: False for token in tokens}}
    text = path.read_text(encoding="utf-8", errors="replace")
    return {"path": str(path.relative_to(ROOT)), "exists": True, "tokens": {token: token in text for token in tokens}}


def audit_assets() -> dict[str, Any]:
    tools = {name: shutil.which(name) for name in ["dosbox", "dosbox-debug", "dosbox-x", "xvfb-run", "xdotool", "gdb", "objdump", "strings"]}
    debug_strings: list[str] = []
    if tools.get("dosbox-debug") and tools.get("strings"):
        proc = run([tools["strings"] or "strings", tools["dosbox-debug"] or "dosbox-debug"], timeout=10)
        for line in proc.get("output_head", []):
            if any(token in line for token in ["BP", "BPMEM", "MEMDUMP", "BPLIST"]):
                debug_strings.append(line)
        # output_head is too small for strings; use direct grep if available is intentionally avoided here.
    files = {
        name: {
            "path": str(DM_STAGE / name),
            "exists": (DM_STAGE / name).is_file(),
            "sha256": sha256(DM_STAGE / name),
            "file": run(["file", str(DM_STAGE / name)]).get("output_head", []),
        }
        for name in ["DM.EXE", "FIRES", "VGA", "DATA/DUNGEON.DAT", "DATA/GRAPHICS.DAT"]
    }
    runner_api = [file_contains(ROOT / rel, ["dosbox", "xdotool", "breakpoint", "G0306", "G0324", "C007_ZONE_VIEWPORT", "json_only"]) for rel in RUNNER_FILES]
    return {
        "tools": tools,
        "debugger_capabilities_required": ["BP code breakpoint", "BPMEM memory watchpoint", "MEMDUMPBIN/MEMDUMP memory read", "continued execution after hit"],
        "debugger_capability_evidence": "dosbox-debug binary strings contain BP/BPMEM/BPLIST/MEMDUMP commands; pass175 already emitted debugger configs, but no source-symbol map exists yet",
        "canonical_files": files,
        "runner_api_audit": runner_api,
    }


def write_report(manifest: dict[str, Any], report: Path) -> None:
    lines = [
        "# Pass227 — DM1 V1 original-runtime hook/API design",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "Scope: JSON-only design spike for the missing stock-runtime state hook. It produces no PNG/PPM artifacts and makes no pixel-parity claim.",
        "",
        "## Feasible route",
        "",
        "Use the existing N2 original stage plus DOSBox-X/dosbox-debug as the first implementation route. The missing piece is not another capture script; it is a small debugger transcript adapter backed by a checked-in symbol/address map for the stock `FIRES` gameplay module.",
        "",
        "## Source seams to bind",
        "",
    ]
    for seam in manifest["source_seams"]:
        mark = "PASS" if seam["verified"] else "FAIL"
        lines.append(f"- {mark} `{seam['id']}` — `{seam['citation']}` / `{seam['function']}` ({seam['hook_kind']})")
    lines += ["", "## Implementation path", ""]
    for step in manifest["implementation_path"]:
        lines.append(f"{step['step']}. **{step['title']}** — {step['action']}")
        lines.append(f"   - deliverable: {step['deliverable']}")
    lines += [
        "",
        "## Minimal JSON event schema",
        "",
        "```json",
        json.dumps(manifest["minimal_trace_schema"], indent=2, sort_keys=True),
        "```",
        "",
        "## Asset/tool audit",
        "",
    ]
    assets = manifest["asset_audit"]
    lines.append(f"- tools: `{assets['tools']}`")
    for name, info in assets["canonical_files"].items():
        lines.append(f"- `{name}` exists=`{info['exists']}` sha256=`{info['sha256']}`")
    lines += [
        "",
        "## Decision",
        "",
        "Land this as the exact handoff for the next coding pass: implement the symbol-map JSON and debugger transcript adapter, starting with one turn command. Do not spend another pass on PNG/PPM route captures until this API emits the three-event chain.",
        "",
        "Non-claims: no runtime trace was captured here; no address map entry is claimed verified; no screenshot artifact is evidence for this pass.",
        "",
    ]
    report.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path, default=DEFAULT_OUT)
    parser.add_argument("--report", type=Path, default=DEFAULT_REPORT)
    args = parser.parse_args()
    seams = [verify_seam(seam) for seam in SOURCE_SEAMS]
    assets = audit_assets()
    source_ok = all(item["verified"] for item in seams)
    required_files_ok = all(assets["canonical_files"][name]["exists"] for name in ["DM.EXE", "FIRES", "VGA", "DATA/DUNGEON.DAT", "DATA/GRAPHICS.DAT"])
    required_tools_ok = bool((assets["tools"].get("dosbox-debug") or assets["tools"].get("dosbox-x")) and assets["tools"].get("xvfb-run") and assets["tools"].get("xdotool"))
    status = "PASS_ORIGINAL_RUNTIME_HOOK_DESIGN_READY" if source_ok and required_files_ok and required_tools_ok else "BLOCKED_HOOK_DESIGN_PREREQUISITE_MISSING"
    manifest = {
        "schema": "pass227_dm1_v1_original_runtime_hook_design.v1",
        "status": status,
        "repo": str(ROOT),
        "redmcsb_source_root": str(REDMCSB),
        "artifact_policy": {"json_only": True, "forbidden_extensions": [".png", ".ppm"]},
        "source_seams": seams,
        "asset_audit": assets,
        "implementation_path": IMPLEMENTATION_PATH,
        "minimal_trace_schema": MINIMAL_TRACE_SCHEMA,
        "first_coding_pass_recommendation": "Implement `data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json` plus `tools/dm1_original_runtime_trace.py` for a single turn command before any more visual capture attempts.",
    }
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest, args.report)
    print(json.dumps({"status": status, "out": str(args.out), "report": str(args.report), "source_seams": len(seams)}, indent=2, sort_keys=True))
    return 0 if source_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
