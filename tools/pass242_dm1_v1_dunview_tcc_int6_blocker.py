#!/usr/bin/env python3
"""Pass242: classify why the N2 ReDMCSB I34E build emits no DUNVIEW.OBJ/FIRES.MAP.

Text-only diagnostic over existing committed pass236 build evidence plus ReDMCSB
build metadata. It does not run DOSBox, preserve build products, or commit source
snippets/log dumps.
"""
from __future__ import annotations

import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206"
IBM = RED / "Toolchains/IBM PC/Source"
DOC = RED / "Documentation/Toolchains.htm"
PASS236 = ROOT / "parity-evidence/verification/pass236_dm1_v1_fires_symbol_bridge_audit/manifest.json"
PASS236_STDOUT = ROOT / "parity-evidence/verification/pass236_dm1_v1_fires_symbol_bridge_audit/dosbox_build_stdout_tail.txt"
PASS236_ERRORS = ROOT / "parity-evidence/verification/pass236_dm1_v1_fires_symbol_bridge_audit/mkii_log_error_excerpt.txt"
OUT_DIR = ROOT / "parity-evidence/verification/pass242_dm1_v1_dunview_tcc_int6_blocker"
REPORT = ROOT / "parity-evidence/pass242_dm1_v1_dunview_tcc_int6_blocker.md"


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def find_line(path: Path, needle: str) -> dict[str, Any]:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    for i, line in enumerate(lines, 1):
        if needle in line:
            return {"line": i, "text": line.strip()}
    return {"line": None, "text": None}


def count_pattern(path: Path, pattern: str) -> int:
    if not path.exists():
        return 0
    text = path.read_text(encoding="latin-1", errors="replace")
    return len(re.findall(pattern, text))


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    p236 = load_json(PASS236)
    ba = p236.get("dosbox_build_attempt", {})
    illegal_int6_count = count_pattern(PASS236_STDOUT, r"Illegal Unhandled Interrupt Called 6")
    dunview_error_lines = count_pattern(PASS236_ERRORS, r"Error dunview\.c")

    mkii_path_line = find_line(IBM / "MKII.BAT", r"SET PATH=\TCPP101\BIN;\TASM20;\LZEXE91")
    dunview_compile_line = find_line(IBM / "MKII.BAT", r"TCC.EXE -c -ml -O -Z    -DEXEID=74 -n\OBJECT\I34E\FIRES DUNVIEW.C")
    tlink_line = find_line(IBM / "MKII.BAT", r"\BUILD\I34E\FIRES.MAP")
    doc_tcpp_line = find_line(DOC, "Turbo C++ 1.01")
    doc_tc20_line = find_line(DOC, "Turbo C 2.01")

    guards = {
        "pass236_attempted": ba.get("attempted") is True,
        "dunview_obj_zero": ba.get("dunview_obj_size") == 0,
        "fires_map_zero": ba.get("fires_map_size") == 0,
        "illegal_int6_seen": illegal_int6_count > 0,
        "dunview_compile_errors_seen": dunview_error_lines > 0,
        "mkii_uses_tcpp101_path": bool(mkii_path_line["line"]),
        "mkii_dunview_compile_line_found": bool(dunview_compile_line["line"]),
        "mkii_tlink_map_line_found": bool(tlink_line["line"]),
        "redmcsb_docs_tcpp101_found": bool(doc_tcpp_line["line"]),
    }
    status = "PASS_DUNVIEW_TCC_INT6_BLOCKER_CLASSIFIED" if all(guards.values()) else "FAIL_DUNVIEW_TCC_INT6_BLOCKER_GUARD"

    manifest = {
        "schema": "pass242_dm1_v1_dunview_tcc_int6_blocker.v1",
        "status": status,
        "inputs": {
            "pass236_manifest": str(PASS236),
            "pass236_stdout_tail": str(PASS236_STDOUT),
            "pass236_error_excerpt": str(PASS236_ERRORS),
            "mkii_bat": str(IBM / "MKII.BAT"),
            "toolchains_doc": str(DOC),
        },
        "observed_build_state": {
            "returncode": ba.get("returncode"),
            "dunview_obj_size": ba.get("dunview_obj_size"),
            "fires_map_size": ba.get("fires_map_size"),
            "compiled_object_count": len(ba.get("compiled_objects", [])),
            "illegal_unhandled_interrupt_6_count_in_stdout_tail": illegal_int6_count,
            "dunview_error_line_count_in_mkii_excerpt": dunview_error_lines,
        },
        "build_metadata": {
            "mkii_path_line": mkii_path_line,
            "dunview_compile_line": dunview_compile_line,
            "tlink_map_line": tlink_line,
            "docs_turbo_cpp_101_line": doc_tcpp_line,
            "docs_turbo_c_201_line": doc_tc20_line,
        },
        "classification": "N2 DOSBox-X reaches the authentic MKII.BAT I34E DUNVIEW.C compile under TCPP101, but the DOS runtime repeatedly reports CPU illegal interrupt 6 while Turbo C++ emits DUNVIEW.C errors; DUNVIEW.OBJ remains zero bytes, so TLINK cannot produce FIRES.MAP.",
        "manual_followup_smoke": {
            "date_utc": "2026-05-06",
            "summary": "Direct N2 temp DUNVIEW.C compile attempts with TCPP101 under DOSBox-X using normal/simple CPU settings and with -O/-Z variants still left DUNVIEW.OBJ at 0 bytes and showed the same illegal interrupt 6 pattern. Temp build products were not committed.",
            "promotable": False,
        },
        "next_options": [
            "Use a non-DOSBox PC emulator/VM or real MS-DOS/FreeDOS environment for the Turbo C++ 1.01 compile.",
            "Patch/build DOSBox-X or use another emulator configuration that can run the TCPP101 DUNVIEW compile without INT6.",
            "Obtain a trusted FIRES.MAP from an authentic ReDMCSB I34E build with the same MKII.BAT/I34E.LNK order.",
        ],
        "artifact_policy": {"text_only": True, "no_original_binaries_committed": True, "no_source_or_log_dump": True},
        "guards": guards,
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass242 — DM1 V1 DUNVIEW/TCC INT6 build blocker",
        "",
        f"Status: `{status}`",
        "",
        "## Finding",
        "",
        "The N2 ReDMCSB I34E build is not merely missing a later TLINK/map step. It dies at the authentic `DUNVIEW.C` compile: `DUNVIEW.OBJ` is zero bytes, `FIRES.MAP` is zero bytes, and the DOSBox-X stdout tail contains repeated CPU illegal interrupt 6 reports while MKII.LOG records `dunview.c` errors.",
        "",
        "## Evidence",
        "",
        f"- `MKII.BAT` uses `TCPP101` for this route: line `{mkii_path_line['line']}`.",
        f"- `DUNVIEW.C` compile command: line `{dunview_compile_line['line']}`.",
        f"- `FIRES.MAP` TLINK output line: line `{tlink_line['line']}`.",
        f"- ReDMCSB toolchain doc names Turbo C++ 1.01 for PC: line `{doc_tcpp_line['line']}`.",
        f"- pass236 compiled object entries: `{len(ba.get('compiled_objects', []))}`; `DUNVIEW.OBJ` size `{ba.get('dunview_obj_size')}`; `FIRES.MAP` size `{ba.get('fires_map_size')}`.",
        f"- pass236 stdout-tail illegal interrupt 6 count: `{illegal_int6_count}`; MKII error excerpt `dunview.c` count: `{dunview_error_lines}`.",
        "",
        "## Exact blocker",
        "",
        "Produce an emulator/toolchain environment where Turbo C++ 1.01 can finish the `MKII.BAT` I34E `DUNVIEW.C` compile. Until `DUNVIEW.OBJ` is non-zero, the authentic TLINK line cannot emit `FIRES.MAP`, so the DM1 V1 public-symbol route remains blocked.",
        "",
        "## Next practical routes",
        "",
        "1. Try the same HARDDISK tree under a different DOS emulator/VM/real DOS instead of current DOSBox-X.",
        "2. Debug DOSBox-X INT6 behavior around the TCPP101 `DUNVIEW.C` compile.",
        "3. Obtain a trusted `FIRES.MAP` from an authentic ReDMCSB I34E build with the same `MKII.BAT`/`I34E.LNK` order.",
        "",
        "Evidence manifest: `parity-evidence/verification/pass242_dm1_v1_dunview_tcc_int6_blocker/manifest.json`.",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")
    print(json.dumps({"status": status, "report": str(REPORT), "manifest": str(OUT_DIR / "manifest.json")}, indent=2, sort_keys=True))
    return 0 if status.startswith("PASS_") else 1


if __name__ == "__main__":
    raise SystemExit(main())
