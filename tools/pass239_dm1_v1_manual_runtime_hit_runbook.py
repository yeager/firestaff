#!/usr/bin/env python3
"""Pass239: executable manual runbook for DM1 PC34 DOSBox runtime hits.

This is intentionally text-only. It does not claim runtime hits; it makes the
human/debugger path executable by validating local prerequisites, source seams,
and candidate CS:IP rows, then emitting a copy/paste runbook plus optional
breakpoint commands once the operator has the program load segment.
"""
from __future__ import annotations

import argparse
import json
import re
import shutil
import subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS237 = ROOT / "parity-evidence/verification/pass237_dm1_v1_fires_static_csip_crosswalk/manifest.json"
SYMBOL_MAP = ROOT / "data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json"
RUNTIME_IMAGE = ROOT / "data/original_runtime/dm1_pc34_i34e_runtime_image.v1.json"
OUT_DIR = ROOT / "parity-evidence/verification/pass239_dm1_v1_manual_runtime_hit_runbook"
REPORT = ROOT / "parity-evidence/pass239_dm1_v1_manual_runtime_hit_runbook.md"

SEAM_CHECKS = [
    ("command_accepted", "COMMAND.C", 2045, 2156, ["F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]),
    ("turn_types_1_to_2", "CLIKMENU.C", 142, 179, ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0308_i_PartyDirection", "F0284_CHAMPION_SetPartyDirection"]),
    ("move_types_3_to_6", "CLIKMENU.C", 180, 347, ["F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0267_MOVE_GetMoveResult_CPSCE", "G0306_i_PartyMapX", "G0307_i_PartyMapY"]),
    ("move_get_move_result", "MOVESENS.C", 316, 850, ["F0267_MOVE_GetMoveResult_CPSCE", "C0xFFFF_THING_PARTY", "C05_ELEMENT_TELEPORTER"]),
    ("viewport_game_loop_draw_call_site", "GAMELOOP.C", 35, 95, ["F0002_MAIN_GameLoop_CPSDF", "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)"]),
    ("viewport_buffer_composed", "DUNVIEW.C", 8318, 8611, ["F0128_DUNGEONVIEW_Draw_CPSF", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)"]),
    ("viewport_present", "DRAWVIEW.C", 709, 858, ["F0097_DUNGEONVIEW_DrawViewport", "VIDRV_09_BlitViewPort"]),
]

ROUTE = [
    "Start from a stock DM1 PC34 directory outside this repo.",
    "Copy the verified decompressed fixture as FIRES.EXE only in that scratch directory; never commit FIRES/FIRES.EXENEW.",
    "Run /usr/bin/dosbox-debug from a real terminal/console. Do not use stdin-only SSH automation for this pass.",
    "At the debugger prompt, set the loader breakpoint: BPINT 21 4B; BPLIST; F5.",
    "At DOS C:\\>, run FIRES.EXE. When INT 21 AH=4B breaks, note the parent PSP from registers if visible, then continue/step until the child entry line shows CS:0000 with the decompressed FIRES first instruction.",
    "Record that child-entry CS as program_load_segment. Because FIRES.EXENEW has entry 0000:0000, PSP = program_load_segment - 0x10.",
    "Compute each runtime breakpoint as runtime_cs = program_load_segment + static_cs and runtime_ip = static_ip.",
    "Set the movement/viewport breakpoints printed below, press F5, enter/reach a playable viewport, then issue one movement command.",
    "For each hit record: exact CS:IP, register dump, 8-16 nearby disassembly lines, command value/state if visible, and whether the sequence proves command accepted -> movement resolver -> viewport draw.",
    "Do not edit data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json to verified_runtime_hit unless a debugger-observed hit at the computed CS:IP is captured.",
]


def compact(s: str) -> str:
    return " ".join(s.split())


def load_json(p: Path) -> dict[str, Any]:
    return json.loads(p.read_text())


def parse_seg(value: str | None) -> int | None:
    if value is None:
        return None
    v = value.strip().lower().replace("0x", "")
    if not re.fullmatch(r"[0-9a-f]{1,4}", v):
        raise SystemExit(f"invalid --load-segment {value!r}; expected 1-4 hex digits")
    return int(v, 16)


def add_seg(base: int, csip: str) -> str:
    cs, ip = csip.split(":")
    return f"{(base + int(cs, 16)) & 0xffff:04X}:{int(ip,16):04X}"


def validate_sources() -> list[dict[str, Any]]:
    out: list[dict[str, Any]] = []
    for seam_id, filename, start, end, needles in SEAM_CHECKS:
        p = SOURCE_ROOT / filename
        text = p.read_text(encoding="latin-1", errors="replace").splitlines() if p.exists() else []
        window = "\n".join(text[start - 1 : min(end, len(text))])
        missing = [n for n in needles if compact(n) not in compact(window)]
        out.append({"id": seam_id, "file": filename, "line_range": [start, end], "path": str(p), "ok": p.exists() and not missing, "missing": missing})
    return out


def debugger_capabilities() -> dict[str, Any]:
    exe = shutil.which("dosbox-debug")
    caps = {"dosbox-debug": exe, "has_bpint": False, "has_bplist": False, "has_bp": False, "has_memdump": False}
    if exe:
        s = subprocess.run(["strings", exe], text=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, timeout=10).stdout
        caps.update({"has_bpint": "BPINT" in s, "has_bplist": "BPLIST" in s, "has_bp": "BP     [segment]:[offset]" in s or "BP " in s, "has_memdump": "MEMDUMP" in s})
    return caps


def candidate_rows(load_segment: int | None) -> list[dict[str, Any]]:
    candidates = load_json(PASS237)["candidates"]
    rows = []
    for c in candidates:
        static = c["candidate_static_cs_ip"]
        cs, ip = static.split(":")
        row = {
            "id": c["id"],
            "source_file": c.get("source_file"),
            "source_function": c.get("source_function"),
            "static_cs_ip": static,
            "runtime_formula": f"{{program_load_segment + 0x{cs}}}:0x{ip}",
            "bp_template": f"BP {{LOAD+0x{cs}}}:{ip}",
            "classification": c.get("classification"),
            "confidence": c.get("confidence"),
        }
        if load_segment is not None:
            row["runtime_cs_ip"] = add_seg(load_segment, static)
            row["bp_command"] = f"BP {row['runtime_cs_ip']}"
        rows.append(row)
    return rows


def write_outputs(manifest: dict[str, Any]) -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    bp_file = OUT_DIR / ("breakpoints.txt" if manifest.get("load_segment_hex") else "breakpoints.template.txt")
    key = "bp_command" if manifest.get("load_segment_hex") else "bp_template"
    bp_file.write_text("\n".join(r[key] for r in manifest["candidate_breakpoints"] if key in r) + "\n")

    lines = [
        "# Pass239 — DM1 PC34 manual runtime-hit runbook",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "## What this makes executable",
        "",
        "A human can now enter the DOSBox debugger, set `BPINT 21 4B`, derive the child program load segment, compute numeric CS:IP breakpoints, and bind movement/viewport source seams to observed debugger hits.",
        "",
        "## Exact runbook",
        "",
    ]
    lines += [f"{i}. {step}" for i, step in enumerate(ROUTE, 1)]
    lines += ["", "## Candidate breakpoints", ""]
    for r in manifest["candidate_breakpoints"]:
        cmd = r.get("bp_command", r["bp_template"])
        lines.append(f"- `{r['id']}` — `{r['source_file']}` / `{r['source_function']}` — static `{r['static_cs_ip']}` -> `{cmd}`; still `{r['classification']}` until hit.")
    caps = manifest["debugger_capabilities"]
    ri = manifest["runtime_image"]["decompressed_fires"]
    lines += ["", "## Validation", ""]
    lines.append(f"- dosbox-debug: `{caps.get('dosbox-debug')}`; BPINT=`{caps.get('has_bpint')}`, BPLIST=`{caps.get('has_bplist')}`, MEMDUMP=`{caps.get('has_memdump')}`")
    lines.append(f"- FIRES.EXENEW SHA256: `{ri['sha256']}`; entry `{ri['entry_cs_ip']}`")
    if manifest.get("load_segment_hex"):
        lines.append(f"- Captured/example program_load_segment: `{manifest['load_segment_hex']}`; inferred PSP: `{manifest['psp_segment_hex']}`")
    lines.append(f"- ReDMCSB source seam checks passed: `{all(x['ok'] for x in manifest['source_audit'])}`")
    lines += ["", "## Guardrail", "", manifest["promotion_guardrail"], "", f"Manifest: `{OUT_DIR / 'manifest.json'}`"]
    REPORT.write_text("\n".join(lines) + "\n")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--load-segment", help="child program load segment captured at FIRES.EXE CS:0000, e.g. 01ED")
    ap.add_argument("--self-test", action="store_true")
    ns = ap.parse_args()
    load = parse_seg(ns.load_segment)
    symbol = load_json(SYMBOL_MAP)
    promoted = [e for e in symbol.get("entries", []) if e.get("confidence") == "verified_runtime_hit"]
    if promoted:
        raise SystemExit(f"refusing to write unguarded runbook; symbol map already has verified_runtime_hit entries: {[e.get('id') for e in promoted]}")
    manifest = {
        "schema": "pass239_dm1_v1_manual_runtime_hit_runbook.v1",
        "status": "PASS_MANUAL_RUNTIME_HIT_PATH_EXECUTABLE_NO_PROMOTIONS",
        "load_segment_hex": f"{load:04X}" if load is not None else None,
        "psp_segment_hex": f"{(load - 0x10) & 0xffff:04X}" if load is not None else None,
        "psp_segment_formula": "PSP = program_load_segment - 0x10 (FIRES.EXENEW entry is 0000:0000)",
        "loader_breakpoint": "BPINT 21 4B",
        "runbook": ROUTE,
        "candidate_breakpoints": candidate_rows(load),
        "source_audit": validate_sources(),
        "debugger_capabilities": debugger_capabilities(),
        "runtime_image": load_json(RUNTIME_IMAGE),
        "inputs": {"pass237_manifest": str(PASS237), "symbol_map": str(SYMBOL_MAP), "runtime_image": str(RUNTIME_IMAGE), "source_root": str(SOURCE_ROOT)},
        "promotion_guardrail": "No candidate row is a verified_runtime_hit. Promote only after a real DOSBox debugger stop at the computed runtime CS:IP with registers/disassembly/state evidence for the movement-to-viewport chain.",
    }
    if not all(x["ok"] for x in manifest["source_audit"]):
        manifest["status"] = "FAIL_SOURCE_SEAM_AUDIT"
    caps = manifest["debugger_capabilities"]
    if not (caps.get("dosbox-debug") and caps.get("has_bpint") and caps.get("has_bplist")):
        manifest["status"] = "FAIL_DEBUGGER_CAPABILITY_AUDIT"
    write_outputs(manifest)
    print(json.dumps({"status": manifest["status"], "manifest": str(OUT_DIR / "manifest.json"), "report": str(REPORT), "breakpoints": str(OUT_DIR / ("breakpoints.txt" if load is not None else "breakpoints.template.txt"))}, indent=2, sort_keys=True))
    return 0 if manifest["status"].startswith("PASS_") else 1


if __name__ == "__main__":
    raise SystemExit(main())
