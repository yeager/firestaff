#!/usr/bin/env python3
"""Pass234: source-lock DM1 I34E viewport compose/present seams and keep runtime CS:IP blocked.

This pass deliberately refuses to promote static ReDMCSB source locations or the
FIRES.EXENEW MZ entry into runtime CS:IP. It records the exact source seams for
viewport_buffer_composed and viewport_present, verifies local debugger/toolchain
state, and documents the remaining FIRES.MAP/public-symbol/debugger-hit blocker.
"""
from __future__ import annotations

import argparse
import json
import shutil
import subprocess
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
TOOLCHAIN_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/IBM PC"
SYMBOL_MAP = ROOT / "data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json"
OUT_DIR = ROOT / "parity-evidence/verification/pass234_dm1_v1_viewport_runtime_hit_blocker"
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/pass234_dm1_v1_viewport_runtime_hit_blocker.md"

SEAMS: list[dict[str, Any]] = [
    {"id": "draw_uses_mutated_tuple", "file": "GAMELOOP.C", "function": "F0002_MAIN_GameLoop_CPSDF", "line_range": [55, 95], "needles": ["for (;;) { /*_Infinite loop_*/", "F0261_TIMELINE_Process_CPSEF();", "if (!G0300_B_PartyIsResting) {", "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);"], "observable": "main loop consumes G0308_i_PartyDirection/G0306_i_PartyMapX/G0307_i_PartyMapY in the F0128 draw call"},
    {"id": "viewport_buffer_composed", "file": "DUNVIEW.C", "function": "F0128_DUNGEONVIEW_Draw_CPSF", "line_range": [8336, 8611], "needles": ["if (G0297_B_DrawFloorAndCeilingRequested) {", "G0074_puc_Bitmap_Temporary = F0606_AllocateMemForGraphic", "G0076_B_UseFlippedWallAndFootprintsBitmaps = (P0184_i_MapX + P0185_i_MapY + P0183_i_Direction) & 0x0001", "F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport);", "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);", "F0607_FreeMemForGraphic(G0074_puc_Bitmap_Temporary);", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"], "observable": "I34E-family F0128 composes G0296_puc_Bitmap_Viewport from direction/X/Y before calling F0097"},
    {"id": "viewport_present", "file": "DRAWVIEW.C", "function": "F0097_DUNGEONVIEW_DrawViewport", "line_range": [709, 858], "needles": ["void F0097_DUNGEONVIEW_DrawViewport(", "switch (P0100_i_PaletteSwitchingRequestedState) {", "F0638_GetZone(C007_ZONE_VIEWPORT, L2414_ai_XYZ);", "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);"], "observable": "PC I34E-family F0097 resolves C007_ZONE_VIEWPORT and blits G0296_puc_Bitmap_Viewport via VIDRV_09_BlitViewPort"},
]

BLOCKER = {"id": "blocked/runtime-base-and-symbol-map-unavailable", "exact_missing_input": "Need FIRES.MAP/public symbols or another reproducible debugger binding from ReDMCSB source seams/globals to loaded original FIRES CS:IP/data addresses.", "required_formula": ["runtime_cs = (PSP + 0x10) + map_segment", "runtime_ip = map_offset"], "promotion_guardrail": "No static source line, MZ entrypoint, or EXENEW offset may be promoted without verified_runtime_hit evidence."}


def find_line(lines: list[str], needle: str, start: int, end: int) -> int | None:
    compact_needle = " ".join(needle.split())
    for idx in range(start - 1, min(end, len(lines))):
        if compact_needle in " ".join(lines[idx].split()):
            return idx + 1
    return None


def source_audit() -> list[dict[str, Any]]:
    out: list[dict[str, Any]] = []
    for seam in SEAMS:
        path = SOURCE_ROOT / seam["file"]
        start, end = seam["line_range"]
        if not path.is_file():
            out.append({**seam, "source_path": str(path), "needle_lines": {}, "missing_needles": seam["needles"], "ok": False})
            continue
        lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
        found: dict[str, int] = {}
        missing: list[str] = []
        for needle in seam["needles"]:
            line = find_line(lines, needle, start, end)
            if line is None:
                missing.append(needle)
            else:
                found[needle] = line
        out.append({"id": seam["id"], "source_path": str(path), "file": seam["file"], "function": seam["function"], "line_range": f"{seam['file']}:{start}-{end}", "needle_lines": found, "missing_needles": missing, "observable": seam["observable"], "ok": not missing})
    return out


def run_probe(cmd: list[str], timeout: int = 5) -> dict[str, Any]:
    try:
        proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, timeout=timeout)
        return {"cmd": cmd, "returncode": proc.returncode, "stdout_tail": proc.stdout[-3000:], "stderr_tail": proc.stderr[-3000:]}
    except Exception as exc:
        return {"cmd": cmd, "error": type(exc).__name__, "message": str(exc)}


def debugger_inventory() -> dict[str, Any]:
    tools = {name: shutil.which(name) for name in ["dosbox-debug", "dosbox-x", "dosbox", "wine", "objdump"]}
    probes: dict[str, Any] = {}
    if tools.get("dosbox-debug"):
        probes["dosbox_debug_help_probe"] = run_probe(["bash", "-lc", "TERM=xterm timeout 3 dosbox-debug -help 2>&1 | head -40"], timeout=6)
    if tools.get("dosbox-x"):
        probes["dosbox_x_version_probe"] = run_probe(["dosbox-x", "-version"], timeout=6)
    if tools.get("objdump"):
        probes["fires_exenew_probe"] = run_probe(["bash", "-lc", "find . ~/.openclaw/data/firestaff-original-games -iname FIRES.EXENEW -print -quit | xargs -r objdump -f"], timeout=6)
    return {"tools": tools, "toolchain_root_exists": TOOLCHAIN_ROOT.is_dir(), "symbol_map_exists": SYMBOL_MAP.is_file(), "probes": probes}


def symbol_map_guardrail() -> dict[str, Any]:
    if not SYMBOL_MAP.is_file():
        return {"exists": False, "entries": [], "has_verified_runtime_hit": False}
    doc = json.loads(SYMBOL_MAP.read_text(encoding="utf-8"))
    entries = doc.get("symbols") or doc.get("entries") or []
    if isinstance(entries, dict):
        entries = list(entries.values())
    checked = []
    for item in entries:
        if isinstance(item, dict):
            checked.append({"id": item.get("id") or item.get("name"), "verified_runtime_hit": bool(item.get("verified_runtime_hit"))})
    return {"exists": True, "entry_count": len(checked), "entries": checked, "has_verified_runtime_hit": any(i["verified_runtime_hit"] for i in checked)}


def write_report(manifest: dict[str, Any], report: Path) -> None:
    lines = ["# Pass234 — DM1 PC34 viewport runtime-hit blocker", "", f"Status: `{manifest['status']}`.", "", "Scope: source-lock viewport draw/present seams, inventory debugger availability, and keep runtime CS:IP promotion blocked until verified runtime hits exist.", "", "## Source seam audit", ""]
    for item in manifest["source_audit"]:
        mark = "PASS" if item["ok"] else "FAIL"
        lines.append(f"- {mark} `{item['id']}` — `{item['line_range']}` / `{item['function']}`; observable: {item['observable']}")
        for needle, line in item["needle_lines"].items():
            lines.append(f"  - line {line}: `{needle}`")
        for needle in item["missing_needles"]:
            lines.append(f"  - missing: `{needle}`")
    inv = manifest["debugger_inventory"]
    lines += ["", "## Runtime/debugger inventory", "", f"- dosbox-debug: `{inv['tools'].get('dosbox-debug')}`", f"- dosbox-x: `{inv['tools'].get('dosbox-x')}`", f"- symbol map exists: `{manifest['symbol_map_guardrail']['exists']}`", f"- verified runtime hits in symbol map: `{manifest['symbol_map_guardrail'].get('has_verified_runtime_hit')}`", "", "## Blocker", "", f"Exact blocker: `{manifest['blocker']['id']}` — {manifest['blocker']['exact_missing_input']}", "", "Required formula:"]
    for formula in manifest["blocker"]["required_formula"]:
        lines.append(f"- `{formula}`")
    lines += ["", f"Guardrail: {manifest['blocker']['promotion_guardrail']}", ""]
    report.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", type=Path, default=MANIFEST)
    parser.add_argument("--report", type=Path, default=REPORT)
    args = parser.parse_args()
    audit = source_audit()
    inv = debugger_inventory()
    guard = symbol_map_guardrail()
    status = "BLOCKED_RUNTIME_HITS_REQUIRED" if all(item["ok"] for item in audit) else "FAIL_SOURCE_AUDIT"
    manifest = {"schema": "pass234_dm1_v1_viewport_runtime_hit_blocker.v1", "status": status, "repo": str(ROOT), "source_root": str(SOURCE_ROOT), "source_audit": audit, "debugger_inventory": inv, "symbol_map_guardrail": guard, "blocker": BLOCKER, "artifact_policy": {"json_and_markdown_only": True, "no_screenshot_claim": True}}
    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest, args.report)
    print(json.dumps({"status": status, "manifest": str(args.manifest), "report": str(args.report), "source_ok": all(item["ok"] for item in audit)}, indent=2, sort_keys=True))
    return 0 if all(item["ok"] for item in audit) else 1


if __name__ == "__main__":
    raise SystemExit(main())
