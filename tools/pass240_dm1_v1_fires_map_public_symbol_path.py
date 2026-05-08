#!/usr/bin/env python3
"""Pass240: concrete FIRES.MAP/TLINK path and verified N2-local negative."""
from __future__ import annotations

import json
import os
import shutil
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206"
COMMON = RED / "Toolchains/Common/Source"
IBM = RED / "Toolchains/IBM PC/Source"
BASE = RED / "Toolchains/IBM PC/Base/HARDDISK"
OUT_DIR = ROOT / "parity-evidence/verification/pass240_dm1_v1_fires_map_public_symbol_path"
REPORT = ROOT / "parity-evidence/pass240_dm1_v1_fires_map_public_symbol_path.md"
PASS236 = ROOT / "parity-evidence/verification/pass236_dm1_v1_fires_symbol_bridge_audit/manifest.json"

SEAMS = {
    "COMMAND.C": ["F0380_COMMAND_ProcessQueue_CPSC"],
    "CLIKMENU.C": ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"],
    "MOVESENS.C": ["F0267_MOVE_GetMoveResult_CPSCE"],
    "DUNVIEW.C": [
        "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap",
        "F0108_DUNGEONVIEW_DrawFloorOrnament",
        "F0109_DUNGEONVIEW_DrawDoorOrnament",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "F0128_DUNGEONVIEW_Draw_CPSF",
    ],
}
SEARCH_ROOTS = [
    RED / "Reference/ReDMCSB/I34E",
    RED / "Reference/Original/I34E",
    IBM,
    COMMON,
    Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1",
    Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34",
]
SYMBOL_SUFFIXES = {".MAP", ".SYM", ".LST"}


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8")) if path.exists() else {}


def grep_line(path: Path, needle: str) -> dict[str, Any]:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    for i, line in enumerate(lines, 1):
        if needle in line:
            return {"line": i, "text": line}
    return {"line": None, "text": None}


def source_audit() -> list[dict[str, Any]]:
    out: list[dict[str, Any]] = []
    for filename, needles in SEAMS.items():
        path = COMMON / filename
        text = path.read_text(encoding="latin-1", errors="replace") if path.exists() else ""
        lines = text.splitlines()
        for needle in needles:
            hit = next((i + 1 for i, line in enumerate(lines) if needle in line), None)
            out.append({"file": filename, "function": needle, "path": str(path), "line": hit, "ok": bool(hit)})
    return out


def link_audit() -> dict[str, Any]:
    mkii = IBM / "MKII.BAT"
    lnk = IBM / "I34E.LNK"
    tl = grep_line(mkii, r"\BUILD\I34E\FIRES.MAP")
    objects = [x.strip().rstrip("+") for x in lnk.read_text(encoding="latin-1", errors="replace").splitlines() if x.strip()]
    required = ["MOVESENS.OBJ", "CLIKMENU.OBJ", "COMMAND.OBJ", "DUNVIEW.OBJ"]
    return {
        "mkii_path": str(mkii),
        "i34e_lnk_path": str(lnk),
        "tlink_line": tl,
        "expected_map_path_dos": r"C:\BUILD\I34E\FIRES.MAP",
        "expected_map_path_dosbox_mount": r"<HARDDISK_MOUNT>/BUILD/I34E/FIRES.MAP",
        "tlink_command_path": r"TLINK.EXE /i /s \OBJECT\I34E\STATS.EXE\C0L.OBJ @\SOURCE\I34E.LNK \TCPP101\LIB\CL.LIB,\BUILD\I34E\FIRES.EXE,\BUILD\I34E\FIRES.MAP",
        "object_count": len(objects),
        "required_objects_present": {obj: obj in objects for obj in required},
        "objects": objects,
    }


def symbol_hits() -> list[dict[str, Any]]:
    hits: list[dict[str, Any]] = []
    for root in SEARCH_ROOTS:
        if not root.exists():
            continue
        for dirpath, dirnames, filenames in os.walk(root):
            dirnames[:] = [d for d in dirnames if d not in {".git", "node_modules", "__pycache__"}]
            for name in filenames:
                p = Path(dirpath) / name
                if p.suffix.upper() in SYMBOL_SUFFIXES or name.upper() in {"FIRES.MAP", "FIRES.SYM", "FIRES.LST"}:
                    hits.append({"path": str(p), "size": p.stat().st_size})
    return sorted(hits, key=lambda x: x["path"])


def tool_inventory() -> dict[str, Any]:
    names = ["TCC.EXE", "TLINK.EXE", "TASM.EXE", "LZEXE.EXE", "CL.LIB"]
    found: dict[str, list[str]] = {n: [] for n in names}
    for root in [BASE, IBM, COMMON]:
        if not root.exists():
            continue
        for dirpath, _, filenames in os.walk(root):
            for name in filenames:
                up = name.upper()
                if up in found:
                    found[up].append(str(Path(dirpath) / name))
    return {"native_path": {"dosbox-x": shutil.which("dosbox-x"), "dosbox-debug": shutil.which("dosbox-debug"), "wine": shutil.which("wine")}, "dos_tool_files": found}


def pass236_summary() -> dict[str, Any]:
    p = load_json(PASS236)
    ba = p.get("dosbox_build_attempt", {})
    return {
        "manifest": str(PASS236),
        "present": bool(p),
        "status": p.get("status"),
        "symbol_file_hits": len(p.get("symbol_file_hits", [])),
        "build_attempted": ba.get("attempted"),
        "returncode": ba.get("returncode"),
        "fires_map_size": ba.get("fires_map_size"),
        "dunview_obj_size": ba.get("dunview_obj_size"),
        "errors": ba.get("errors", [])[:8],
        "error_excerpt_path": ba.get("error_excerpt_path"),
    }


def write_report(m: dict[str, Any]) -> None:
    la = m["link_audit"]
    lines = [
        "# Pass240 — DM1 PC34 FIRES.MAP/public-symbol path",
        "",
        f"Status: `{m['status']}`",
        "",
        "## Concrete FIRES.MAP path",
        "",
        f"- ReDMCSB script: `{la['mkii_path']}` line `{la['tlink_line']['line']}`",
        f"- Link order: `{la['i34e_lnk_path']}` ({la['object_count']} objects)",
        f"- TLINK command: `{la['tlink_command_path']}`",
        f"- Expected output in DOS build: `{la['expected_map_path_dos']}`",
        f"- Expected output from a DOSBox-mounted HARDDISK tree: `{la['expected_map_path_dosbox_mount']}`",
        "",
        "## Verified negative on N2",
        "",
    ]
    if m["symbol_hits"]:
        for h in m["symbol_hits"]:
            lines.append(f"- Found symbol-ish file: `{h['path']}` ({h['size']} bytes)")
    else:
        lines.append("- No `*.MAP`, `*.SYM`, or `*.LST` artifact exists in focused N2-local ReDMCSB/original DM1 roots.")
    p236 = m["pass236"]
    lines += [
        f"- Prior bounded ReDMCSB DOSBox build attempt: status `{p236.get('status')}`, return `{p236.get('returncode')}`, `FIRES.MAP` size `{p236.get('fires_map_size')}`, `DUNVIEW.OBJ` size `{p236.get('dunview_obj_size')}`.",
        f"- Error excerpt: `{p236.get('error_excerpt_path')}`",
        "",
        "## Source seams audited",
        "",
    ]
    for s in m["source_audit"]:
        lines.append(f"- `{s['file']}` `{s['function']}` line `{s['line']}` ok `{s['ok']}`")
    lines += ["", "## Exact next blocker", "", m["exact_next_blocker"], ""]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    m = {
        "schema": "pass240_dm1_v1_fires_map_public_symbol_path.v1",
        "status": "VERIFIED_NEGATIVE_FIRES_MAP_PUBLIC_SYMBOLS_MISSING",
        "repo": str(ROOT),
        "source_audit": source_audit(),
        "link_audit": link_audit(),
        "symbol_search_roots": [str(p) for p in SEARCH_ROOTS],
        "symbol_hits": symbol_hits(),
        "tool_inventory": tool_inventory(),
        "pass236": pass236_summary(),
        "decision": "Do not promote pass237 candidate CS:IP rows to verified runtime hits.",
        "exact_next_blocker": "Produce an authentic ReDMCSB I34E FIRES.MAP via the MKII.BAT TLINK line, or provide equivalent public symbols/debugger-observed seam hits. Current N2-local DOSBox/Turbo-C attempt reaches DUNVIEW.C but emits no DUNVIEW.OBJ and no FIRES.MAP, so the immediate blocker is fixing that ReDMCSB/TCC DUNVIEW.C compile failure or obtaining a trusted FIRES.MAP from the same I34E object/link order.",
        "artifact_policy": {"text_only": True, "no_original_binaries_committed": True},
    }
    ok = all(s["ok"] for s in m["source_audit"]) and bool(m["link_audit"]["tlink_line"]["line"]) and not m["symbol_hits"]
    if not ok:
        m["status"] = "FAIL_PASS239_AUDIT_GUARD"
    (OUT_DIR / "manifest.json").write_text(json.dumps(m, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(m)
    print(json.dumps({"status": m["status"], "report": str(REPORT), "manifest": str(OUT_DIR / "manifest.json")}, indent=2, sort_keys=True))
    return 0 if ok else 1

if __name__ == "__main__":
    raise SystemExit(main())
