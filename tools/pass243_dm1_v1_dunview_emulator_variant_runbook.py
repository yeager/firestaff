#!/usr/bin/env python3
"""Pass243: record bounded DUNVIEW/TCPP101 emulator variants and next runbook.

This is a text-only verifier over N2-local source metadata plus the manual
variant matrix from the 2026-05-06 blocker-fix pass. It intentionally does not
preserve original-derived build products, DOSBox stdout floods, screenshots, or
FIRES.EXENEW.
"""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206"
IBM = RED / "Toolchains/IBM PC/Source"
COMMON = RED / "Toolchains/Common/Source"
OUT_DIR = ROOT / "parity-evidence/verification/pass243_dm1_v1_dunview_emulator_variant_runbook"
REPORT = ROOT / "parity-evidence/pass243_dm1_v1_dunview_emulator_variant_runbook.md"

VARIANTS: list[dict[str, Any]] = [
    {
        "name": "dosbox-x normal 386",
        "emulator": "dosbox-x 2024.03.01",
        "config": {"core": "normal", "cputype": "386", "cycles": "3000", "memsize": 32, "machine": "svga_s3"},
        "command": r"TCC.EXE -c -ml -O -Z -DEXEID=74 -n\\OBJECT\\I34E\\FIRES DUNVIEW.C",
        "return_code": 124,
        "timeout_seconds": 45,
        "dunview_obj_size": 0,
        "dunview_log_size": 278528,
        "first_diagnostic": "Warning dunview.c 2314: Possibly incorrect assignment in function F0096_DUNGEONVIEW_LoadCurrentMap",
        "result": "timeout with zero-byte/missing DUNVIEW.OBJ; DOSBox-X INT6 flood observed when stdout was not suppressed",
    },
    {
        "name": "dosbox-x normal 386_prefetch",
        "emulator": "dosbox-x 2024.03.01",
        "config": {"core": "normal", "cputype": "386_prefetch", "cycles": "3000", "memsize": 32, "machine": "svga_s3"},
        "command": r"TCC.EXE -c -ml -O -Z -DEXEID=74 -n\\OBJECT\\I34E\\FIRES DUNVIEW.C",
        "return_code": 124,
        "timeout_seconds": 45,
        "dunview_obj_size": 0,
        "dunview_log_size": 278528,
        "first_diagnostic": "Warning dunview.c 2314: Possibly incorrect assignment in function F0096_DUNGEONVIEW_LoadCurrentMap",
        "result": "timeout with zero-byte/missing DUNVIEW.OBJ; no improvement over normal 386",
    },
    {
        "name": "dosbox-x simple 386",
        "emulator": "dosbox-x 2024.03.01",
        "config": {"core": "simple", "cputype": "386", "cycles": "3000", "memsize": 32, "machine": "svga_s3"},
        "command": r"TCC.EXE -c -ml -O -Z -DEXEID=74 -n\\OBJECT\\I34E\\FIRES DUNVIEW.C",
        "return_code": 0,
        "timeout_seconds": 45,
        "dunview_obj_size": 0,
        "dunview_log_size": 280735,
        "first_diagnostic": "Warning dunview.c 2314: Possibly incorrect assignment in function F0096_DUNGEONVIEW_LoadCurrentMap",
        "error_lines_seen": [4791, 4827, 4838, 4845, 4849],
        "result": "compiler returned to DOSBox but emitted errors and no DUNVIEW.OBJ",
    },
    {
        "name": "dosbox-x dynamic auto",
        "emulator": "dosbox-x 2024.03.01",
        "config": {"core": "dynamic", "cputype": "auto", "cycles": "max", "memsize": 32, "machine": "svga_s3"},
        "command": r"TCC.EXE -c -ml -O -Z -DEXEID=74 -n\\OBJECT\\I34E\\FIRES DUNVIEW.C",
        "return_code": 0,
        "timeout_seconds": 45,
        "dunview_obj_size": 0,
        "dunview_log_size": 280735,
        "first_diagnostic": "Warning dunview.c 2314: Possibly incorrect assignment in function F0096_DUNGEONVIEW_LoadCurrentMap",
        "error_lines_seen": [4791, 4827, 4838, 4845, 4849],
        "result": "same compiler failure pattern as simple 386; no DUNVIEW.OBJ",
    },
    {
        "name": "classic dosbox normal 386",
        "emulator": "dosbox 0.74-3",
        "config": {"core": "normal", "cputype": "386", "cycles": "3000", "memsize": 32, "machine": "svga_s3"},
        "command": r"TCC.EXE -c -ml -O -Z -DEXEID=74 -n\\OBJECT\\I34E\\FIRES DUNVIEW.C",
        "return_code": 124,
        "timeout_seconds": 45,
        "dunview_obj_size": 0,
        "dunview_log_size": 1187,
        "first_diagnostic": "Warning dunview.c 2314: Possibly incorrect assignment in function F0096_DUNGEONVIEW_LoadCurrentMap",
        "result": "classic DOSBox did not complete the DUNVIEW compile inside the bounded window; no DUNVIEW.OBJ",
    },
]


def find_line(path: Path, needle: str) -> dict[str, Any]:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    for idx, line in enumerate(lines, 1):
        if needle in line:
            return {"line": idx, "text": line.strip()}
    return {"line": None, "text": None}


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    mkii = IBM / "MKII.BAT"
    dunview = COMMON / "DUNVIEW.C"
    source_lines = {
        "mkii_tcpp101_path": find_line(mkii, r"SET PATH=\TCPP101\BIN;\TASM20;\LZEXE91"),
        "mkii_dunview_compile": find_line(mkii, r"TCC.EXE -c -ml -O -Z    -DEXEID=74 -n\OBJECT\I34E\FIRES DUNVIEW.C"),
        "mkii_fires_map_tlink": find_line(mkii, r"\BUILD\I34E\FIRES.MAP"),
        "dunview_first_failing_assignment": find_line(dunview, "L0174_B_DrawCreaturesCompleted = L0186_B_SquareHasProjectile"),
    }
    guards = {
        "mkii_uses_tcpp101": bool(source_lines["mkii_tcpp101_path"]["line"]),
        "authentic_dunview_compile_line_found": bool(source_lines["mkii_dunview_compile"]["line"]),
        "authentic_tlink_map_line_found": bool(source_lines["mkii_fires_map_tlink"]["line"]),
        "variants_all_zero_dunview_obj": all(v["dunview_obj_size"] == 0 for v in VARIANTS),
        "variant_matrix_includes_dosbox_x_and_classic_dosbox": {v["emulator"].split()[0] for v in VARIANTS} == {"dosbox-x", "dosbox"},
    }
    status = "BLOCKED_DUNVIEW_TCPP101_DOSBOX_VARIANTS_EXHAUSTED" if all(guards.values()) else "FAIL_PASS243_GUARD"
    manifest = {
        "schema": "pass243_dm1_v1_dunview_emulator_variant_runbook.v1",
        "status": status,
        "source_lines": source_lines,
        "manual_variant_matrix_date": "2026-05-06",
        "variants": VARIANTS,
        "conclusion": "N2-local DOSBox-X CPU/core variants and classic DOSBox did not unblock the authentic TCPP101 DUNVIEW.C compile. DUNVIEW.OBJ remains zero/missing, so FIRES.MAP remains blocked.",
        "next_single_action": "Run the same HARDDISK/SOURCE tree in a non-DOSBox DOS environment (real DOS, 86Box/PCem, or QEMU+FreeDOS/MS-DOS with Turbo C++ 1.01 mounted) and preserve only DUNVIEW.OBJ/FIRES.MAP metadata if produced.",
        "artifact_policy": {"text_only_repo_artifacts": True, "no_original_binaries_committed": True, "temp_build_trees_removed": True},
        "guards": guards,
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass243 — DM1 V1 DUNVIEW emulator variant runbook",
        "",
        f"Status: `{status}`",
        "",
        "## Result",
        "",
        "No tested N2-local DOSBox variant produced `DUNVIEW.OBJ`; therefore the authentic `FIRES.MAP` route is still blocked before TLINK.",
        "",
        "## Source anchors",
        "",
        f"- `MKII.BAT` TCPP101 PATH line: `{source_lines["mkii_tcpp101_path"]["line"]}`.",
        f"- `MKII.BAT` authentic DUNVIEW compile line: `{source_lines["mkii_dunview_compile"]["line"]}`.",
        f"- `MKII.BAT` FIRES.MAP TLINK line: `{source_lines["mkii_fires_map_tlink"]["line"]}`.",
        f"- First repeated TCC error source line anchor in `DUNVIEW.C`: `{source_lines["dunview_first_failing_assignment"]["line"]}`.",
        "",
        "## Tested variants",
        "",
    ]
    for v in VARIANTS:
        lines.append(f"- `{v["name"]}`: rc `{v["return_code"]}`, `DUNVIEW.OBJ` `{v["dunview_obj_size"]}` bytes, log `{v["dunview_log_size"]}` bytes — {v["result"]}.")
    lines += [
        "",
        "## Next single action",
        "",
        manifest["next_single_action"],
        "",
        "Evidence manifest: `parity-evidence/verification/pass243_dm1_v1_dunview_emulator_variant_runbook/manifest.json`.",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")
    print(json.dumps({"status": status, "report": str(REPORT), "manifest": str(OUT_DIR / "manifest.json")}, indent=2, sort_keys=True))
    return 0 if status.startswith("BLOCKED_") else 1


if __name__ == "__main__":
    raise SystemExit(main())
