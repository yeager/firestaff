#!/usr/bin/env python3
"""Pass273: bind DM1 PC34/I34E FIRES globals to the preserved ReDMCSB public map.

This pass follows pass270's exact unblocker: preserve/build/find I34E DM.MAP/public
symbols before attempting debugger BPMs. It does not claim a stock runtime hook;
it only converts trusted map public symbols plus the pass246 load-segment lineage into
bounded debugger addresses for the next live run.
"""
from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
MAP_ROOT = (Path.home() / ".openclaw/data/redmcsb-n2-build-probe")
FIRES_MAP = MAP_ROOT / "ibm-pc-i34e-fires/HARDDISK/BUILD/I34E/FIRES.MAP"
DM_MAP = MAP_ROOT / "ibm-pc-i34e-dm/HARDDISK/BUILD/I34E/DM.MAP"
PC_H = SOURCE_ROOT / "PC.H"
OUT_DIR = ROOT / "parity-evidence/verification/pass273_dm1_v1_fires_public_symbol_unblock"
REPORT = ROOT / "parity-evidence/pass273_dm1_v1_fires_public_symbol_unblock.md"
LOAD_SEG = 0x0733

SOURCE_AUDIT = [
    ("COMMAND.C", 1, 16, ["COMMAND G0432_as_CommandQueue", "G0433_i_CommandQueueFirstIndex", "G0434_i_CommandQueueLastIndex", "G0435_B_CommandQueueLocked"]),
    ("COMMAND.C", 1734, 1812, ["G0443_ps_PrimaryKeyboardInput", "G0432_as_CommandQueue", "G0434_i_CommandQueueLastIndex"]),
    ("COMMAND.C", 2045, 2156, ["F0380_COMMAND_ProcessQueue_CPSC", "G0432_as_CommandQueue", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]),
    ("CLIKMENU.C", 142, 328, ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0308_i_PartyDirection", "F0267_MOVE_GetMoveResult_CPSCE"]),
    ("GAMELOOP.C", 55, 91, ["F0261_TIMELINE_Process_CPSEF", "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)"]),
    ("MOVESENS.C", 316, 556, ["F0267_MOVE_GetMoveResult_CPSCE", "G0306_i_PartyMapX = P0560_i_DestinationMapX", "G0307_i_PartyMapY = P0561_i_DestinationMapY", "F0284_CHAMPION_SetPartyDirection"]),
    ("DUNVIEW.C", 8318, 8611, ["F0128_DUNGEONVIEW_Draw_CPSF", "G0296_puc_Bitmap_Viewport", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)"]),
]

GLOBAL_TARGETS = [
    "G0296_puc_Bitmap_Viewport",
    "G0297_B_DrawFloorAndCeilingRequested",
    "G0305_ui_PartyChampionCount",
    "G0308_i_PartyDirection",
    "G0306_i_PartyMapX",
    "G0307_i_PartyMapY",
    "G0309_i_PartyMapIndex",
    "G0310_i_DisabledMovementTicks",
    "G0311_i_ProjectileDisabledMovementTicks",
    "G0312_i_LastProjectileDisabledMovementDirection",
    "G0321_B_StopWaitingForPlayerInput",
    "G0432_as_CommandQueue",
    "G0433_i_CommandQueueFirstIndex",
    "G0434_i_CommandQueueLastIndex",
    "G0435_B_CommandQueueLocked",
    "G0436_B_PendingClickPresent",
]

FUNCTION_TARGETS = {
    "F0359_COMMAND_ProcessClick_CPSC": "COMMAND.C mouse/click queue writer",
    "F0380_COMMAND_ProcessQueue_CPSC": "COMMAND.C command dequeue/dispatch",
    "F0365_COMMAND_ProcessTypes1To2_TurnParty": "CLIKMENU.C turn handler",
    "F0366_COMMAND_ProcessTypes3To6_MoveParty": "CLIKMENU.C move handler",
    "F0284_CHAMPION_SetPartyDirection": "CHAMPION.C direction write",
    "F0267_MOVE_GetMoveResult_CPSCE": "MOVESENS.C movement result / coordinate writes",
    "F0128_DUNGEONVIEW_Draw_CPSF": "DUNVIEW.C viewport compose",
    "F0097_DUNGEONVIEW_DrawViewport": "DRAWVIEW.C viewport present request/blit",
}


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def fmt(seg: int, off: int) -> str:
    return f"{seg & 0xffff:04X}:{off & 0xffff:04X}"


def parse_csip(value: str) -> tuple[int, int]:
    seg, off = value.split(":", 1)
    return int(seg, 16), int(off, 16)


def runtime_addr(link_csip: str) -> str:
    seg, off = parse_csip(link_csip)
    return fmt(seg + LOAD_SEG, off)


def load_pc_aliases() -> dict[str, str]:
    aliases: dict[str, str] = {}
    rx = re.compile(r"^#define\s+(G\d+_[A-Za-z0-9_]+)\s+(\S+)")
    for line in PC_H.read_text(errors="replace").splitlines():
        m = rx.match(line.strip())
        if m:
            aliases[m.group(1)] = m.group(2)
    return aliases


def load_map_symbols(path: Path) -> dict[str, list[dict[str, Any]]]:
    rx = re.compile(r"^\s*([0-9A-F]{4}:[0-9A-F]{4})\s+(idle\s+)?_(\S+)\s*$")
    out: dict[str, list[dict[str, Any]]] = {}
    for line_no, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
        m = rx.match(line.rstrip("\r"))
        if not m:
            continue
        name = m.group(3)
        out.setdefault(name, []).append({"link_csip": m.group(1), "idle": bool(m.group(2)), "line": line_no})
    return out


def source_window() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for file_name, start, end, needles in SOURCE_AUDIT:
        path = SOURCE_ROOT / file_name
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        window = "\n".join(lines[start - 1:end])
        missing = [n for n in needles if n not in window]
        rows.append({"file": file_name, "range": [start, end], "ok": not missing, "missing": missing})
    return rows


def map_lookup(symbols: dict[str, list[dict[str, Any]]], name: str) -> dict[str, Any] | None:
    hits = symbols.get(name)
    if not hits and len(name) > 31:
        matches = [key for key in symbols if key.startswith(name[:31])]
        if len(matches) == 1:
            name = matches[0]
            hits = symbols.get(name)
    if not hits:
        return None
    first_by_addr: dict[str, dict[str, Any]] = {}
    for hit in hits:
        first_by_addr.setdefault(hit["link_csip"], hit)
    vals = list(first_by_addr.values())
    if len(vals) != 1:
        return {"name": name, "ambiguous_hits": vals}
    hit = vals[0]
    return {"name": name, **hit, "runtime_with_load_seg_0733": runtime_addr(hit["link_csip"])}


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for required in (FIRES_MAP, DM_MAP, PC_H):
        if not required.exists():
            raise SystemExit(f"missing required input: {required}")
    aliases = load_pc_aliases()
    symbols = load_map_symbols(FIRES_MAP)

    globals_out = []
    for source_name in GLOBAL_TARGETS:
        alias = aliases.get(source_name, source_name)
        rec = {"source_name": source_name, "pc_public_symbol": alias}
        hit = map_lookup(symbols, alias) or map_lookup(symbols, source_name.upper())
        if hit:
            rec.update(hit)
        else:
            rec["status"] = "missing_from_fires_map"
        globals_out.append(rec)

    functions_out = []
    for source_name, meaning in FUNCTION_TARGETS.items():
        map_name = source_name.upper()[:31]
        rec = {"source_name": source_name, "map_symbol_prefix": map_name, "meaning": meaning}
        matches = [name for name in symbols if name.startswith(map_name)]
        if matches:
            hit = map_lookup(symbols, sorted(matches, key=len)[0])
            rec.update(hit or {"status": "ambiguous_or_missing"})
        else:
            rec["status"] = "missing_from_fires_map"
        functions_out.append(rec)

    manifest = {
        "schema": "pass273_dm1_v1_fires_public_symbol_unblock.v1",
        "status": "UNBLOCKED_PUBLIC_SYMBOLS_FOUND_NO_RUNTIME_HOOK",
        "binary_policy": "No original/decompressed executable copied or committed; this pass records text map paths, hashes, and derived addresses only.",
        "load_segment_from_pass246_lineage": f"{LOAD_SEG:04X}",
        "inputs": {
            "fires_map": {"path": str(FIRES_MAP), "sha256": sha256(FIRES_MAP), "bytes": FIRES_MAP.stat().st_size},
            "dm_map": {"path": str(DM_MAP), "sha256": sha256(DM_MAP), "bytes": DM_MAP.stat().st_size},
            "pc_h": str(PC_H),
        },
        "source_audit": source_window(),
        "globals": globals_out,
        "functions": functions_out,
        "pass270_correction": "The preserved FIRES.MAP shows e.g. F0267 at 1126:0516, F0365 at 1771:010D, and F0366 at 1771:01AA. Adding load segment 0733 yields 1859:0516, 1EA4:010D, and 1EA4:01AA respectively, matching prior pass263/pass270 candidates. Therefore pass270's extra +0733 runtime column was a double-add for these map-backed candidates.",
        "debugger_next_step": [
            "Launch stock PC34/I34E under dosbox-debug and verify FIRES load segment 0733 in that run.",
            "Set BP at runtime F0380/F0365/F0366/F0267/F0128 addresses from this manifest.",
            "Set BPM writes on runtime G0432/G0433/G0434/G0308/G0306/G0307 addresses, then press kp5/kp4/kp6 in a controlled in-game run.",
            "Promote only after transcript proves command queue write/dequeue plus G0308 or G0306/G0307 mutation and subsequent F0128 draw consumption.",
        ],
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")

    def table(rows: list[dict[str, Any]], cols: list[str]) -> str:
        lines = ["| " + " | ".join(cols) + " |", "| " + " | ".join(["---"] * len(cols)) + " |"]
        for row in rows:
            lines.append("| " + " | ".join(str(row.get(c, "")) for c in cols) + " |")
        return "\n".join(lines)

    source_lines = [f"- `{r['file']}:{r['range'][0]}-{r['range'][1]}` ok={r['ok']} missing={r['missing']}" for r in manifest["source_audit"]]
    glob_rows = [{"source": g["source_name"], "public": g["pc_public_symbol"], "link": g.get("link_csip", "missing"), "runtime": g.get("runtime_with_load_seg_0733", "missing"), "line": g.get("line", "")} for g in globals_out]
    func_rows = [{"source": f["source_name"], "link": f.get("link_csip", "missing"), "runtime": f.get("runtime_with_load_seg_0733", "missing"), "meaning": f.get("meaning", "")} for f in functions_out]
    report = f"""# pass273 — DM1 V1 FIRES public-symbol unblock

Date: 2026-05-06
Worktree: `{ROOT.name}`
Status: `UNBLOCKED_PUBLIC_SYMBOLS_FOUND_NO_RUNTIME_HOOK`

## ReDMCSB source audit first

Primary source root: `{SOURCE_ROOT}`.

{chr(10).join(source_lines)}

## Public map inputs found on N2

- FIRES.MAP: `{FIRES_MAP}` ({FIRES_MAP.stat().st_size} bytes, sha256 `{sha256(FIRES_MAP)}`)
- DM.MAP: `{DM_MAP}` ({DM_MAP.stat().st_size} bytes, sha256 `{sha256(DM_MAP)}`)
- Binary policy: no original/decompressed executable copied or committed; this pass records map paths, hashes, and derived addresses only.

## Global address bindings from FIRES.MAP + PC.H aliases

Load segment used from pass246 lineage: `{LOAD_SEG:04X}`.

{table(glob_rows, ['source', 'public', 'link', 'runtime', 'line'])}

## Code seam bindings from FIRES.MAP

{table(func_rows, ['source', 'link', 'runtime', 'meaning'])}

## pass270 correction

The preserved `FIRES.MAP` shows `F0267` at `1126:0516`, `F0365` at `1771:010D`, and `F0366` at `1771:01AA`. Adding load segment `0733` yields `1859:0516`, `1EA4:010D`, and `1EA4:01AA`, matching prior candidate addresses. So pass270's additional runtime `+0733` column was a double-add for these map-backed candidates.

## Blocker status / exact next step

The public-symbol route is now unblocked: use the runtime addresses above for the next stock DOS debugger run. This pass still does **not** claim a runtime hook. The required next proof is a dosbox-debug transcript showing controlled `kp5`/`kp4`/`kp6` input writing `G0432`, dequeuing via `F0380`, mutating `G0308` or `G0306/G0307`, and then consuming that tuple in `F0128`.

Manifest: `parity-evidence/verification/pass273_dm1_v1_fires_public_symbol_unblock/manifest.json`
"""
    REPORT.write_text(report)
    print(json.dumps({"status": manifest["status"], "report": str(REPORT), "manifest": str(OUT_DIR / "manifest.json")}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
