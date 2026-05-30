#!/usr/bin/env python3
"""Pass510: source-lock DM1 V1 viewport wall parity flip/native restore."""
from __future__ import annotations

import hashlib
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
DM1 = Path("~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1").expanduser()
GREATSTONE = Path("~/.openclaw/data/firestaff-greatstone-atlas").expanduser()
CSBWIN = Path("~/.openclaw/data/firestaff-csbwin-source/CSBWin").expanduser()
CSB = Path("~/.openclaw/data/firestaff-csb-source/CSB").expanduser()

REPORT = ROOT / "parity-evidence/pass510_dm1_v1_viewport_wall_parity_flip_source_lock.md"
MANIFEST = ROOT / "parity-evidence/verification/pass510_dm1_v1_viewport_wall_parity_flip_source_lock/manifest.json"
STATUS = "PASS510_DM1_V1_VIEWPORT_WALL_PARITY_FLIP_SOURCE_LOCKED"

SOURCE_CHECKS = [
    {
        "id": "redmcsb_wallset_native_slots",
        "path": RED / "DUNVIEW.C",
        "lines": "183-200",
        "needles": [
            "int16_t G2107_WallSet[15]",
            "-17,  /* Wall D0R */",
            "-16,  /* Wall D0L */",
            "-9,   /* Wall D2R2 */",
            "-8,   /* Wall D2L2 */",
            "-4,   /* Wall D3R2 */",
            "-3,   /* Wall D3L2 */",
            "-5 }; /* Wall D3C */",
        ],
    },
    {
        "id": "redmcsb_flipped_wallset_pair_table",
        "path": RED / "DUNVIEW.C",
        "lines": "2427-2443",
        "needles": [
            "F1000_(G2107_WallSet[C14_WALL_D3C], G3048_WallSetFlipped[C14_WALL_D3C]);",
            "F1000_(G2107_WallSet[C01_WALL_D0L], G3048_WallSetFlipped[C00_WALL_D0R]);",
            "F1000_(G2107_WallSet[C00_WALL_D0R], G3048_WallSetFlipped[C01_WALL_D0L]);",
            "F1000_(G2107_WallSet[C12_WALL_D3R], G3048_WallSetFlipped[C13_WALL_D3L]);",
            "F1000_(G2107_WallSet[C13_WALL_D3L], G3048_WallSetFlipped[C12_WALL_D3R]);",
            "F1000_(G2107_WallSet[C05_WALL_D2R2], G3048_WallSetFlipped[C06_WALL_D2L2]);",
            "F1000_(G2107_WallSet[C06_WALL_D2L2], G3048_WallSetFlipped[C05_WALL_D2R2]);",
        ],
    },
    {
        "id": "redmcsb_f0128_flip_select_and_swap",
        "path": RED / "DUNVIEW.C",
        "lines": "8354-8414",
        "needles": [
            "G0076_B_UseFlippedWallAndFootprintsBitmaps = (P0184_i_MapX + P0185_i_MapY + P0183_i_Direction) & 0x0001",
            "F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport);",
            "F0792_DUNGEONVIEW_DrawBitmapYYY(G2108_Floor, C701_ZONE_VIEWPORT_FLOOR_AREA, G0076_B_UseFlippedWallAndFootprintsBitmaps = MASK0x0001_FLIP_HORIZONTAL);",
            "F0007_MAIN_CopyBytes(M772_CAST_PC(G3048_WallSetFlipped), M772_CAST_PC(G2107_WallSet), sizeof(G2107_WallSet));",
        ],
    },
    {
        "id": "redmcsb_f0128_restore_native_wallset",
        "path": RED / "DUNVIEW.C",
        "lines": "8543-8579",
        "needles": [
            "if (L0223_B_FlipWalls) {",
            "G3011_i_WallSet_Wall_D3C = G3060_i_WallSet_Wall_D3C;",
            "G3014_i_WallSet_Wall_D0L = G3069_i_WallSet_Wall_D0L;",
            "G3015_i_WallSet_Wall_D0R = G3070_i_WallSet_Wall_D0R;",
            "F0007_MAIN_CopyBytes(M772_CAST_PC(G3071_WallSetNotFlipped), M772_CAST_PC(G2107_WallSet), sizeof(G2107_WallSet));",
        ],
    },
    {
        "id": "redmcsb_center_walls_use_flip_flag",
        "path": RED / "DUNVIEW.C",
        "lines": "6697-6714",
        "needles": [
            "case C00_ELEMENT_WALL:",
            "F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C, G0076_B_UseFlippedWallAndFootprintsBitmaps);",
            "F0765_DUNGEONVIEW_DrawBitmapWithoutTransparency(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C);",
        ],
    },
]

FIRE_CHECKS = [
    {
        "id": "firestaff_party_tuple_flip_predicate",
        "path": ROOT / "src/engine/m11_game_view.c",
        "lines": "11507-11513",
        "needles": [
            "static int m11_dm1_use_flipped_walls(const M11_GameViewState* state)",
            "return (state->world.party.mapX +",
            "state->world.party.mapY +",
            "state->world.party.direction) & 1;",
        ],
    },
    {
        "id": "firestaff_wallset_variant_binding_before_draw",
        "path": ROOT / "src/engine/m11_game_view.c",
        "lines": "11505-11582",
        "needles": [
            "static unsigned int m11_wallset_graphic_index_for_state(const M11_GameViewState* state,",
            "wallSet = (int)state->world.dungeon->maps[state->world.party.mapIndex].wallSet;",
            "return (unsigned int)(M11_GFX_DM1_WALLSET_FIRST +",
            "wallSet * M11_GFX_DM1_WALLSET_COUNT +",
        ],
    },
    {
        "id": "firestaff_center_wall_flip_path",
        "path": ROOT / "src/engine/m11_game_view.c",
        "lines": "12131-12149",
        "needles": [
            "flipWalls = m11_dm1_use_flipped_walls(state);",
            "if (m11_viewport_cell_is_wall_like(&cells[depth][1]))",
            "if (flipWalls) {",
            "(void)m11_draw_dm1_wall_blit_flipped(state, framebuffer,",
            "} else {",
            "(void)m11_draw_dm1_front_wall_blit(state, framebuffer,",
        ],
    },
    {
        "id": "firestaff_side_wall_lr_swap_path",
        "path": ROOT / "src/engine/m11_game_view.c",
        "lines": "12971-13013",
        "needles": [
            "flipWalls = m11_dm1_use_flipped_walls(state);",
            "size_t partner = i ^ 1;",
            "M11_DM1WallFrontBlit swapped = kSideBlits[i];",
            "swapped.graphicIndex = kSideBlits[partner].graphicIndex;",
            "(void)m11_draw_dm1_wall_blit_flipped(state,",
            "} else {",
            "(void)m11_draw_dm1_wall_blit_with_transparency(state,",
        ],
    },
]


def sha(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def read(path: Path) -> str:
    return path.read_text(encoding="latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8", errors="replace")


def line_no(text: str, pos: int) -> int:
    return text.count("\n", 0, pos) + 1


def slice_text(text: str, spec: str) -> tuple[int, str]:
    start, end = [int(part) for part in spec.split("-", 1)]
    return start, "\n".join(text.splitlines()[start - 1:end])


def ordered_hits(text: str, needles: list[str], base_line: int = 1) -> tuple[list[dict[str, object]], list[str]]:
    cursor = 0
    hits: list[dict[str, object]] = []
    missing: list[str] = []
    for needle in needles:
        pos = text.find(needle, cursor)
        if pos < 0:
            missing.append(needle)
            continue
        hits.append({"needle": needle, "line": base_line + line_no(text, pos) - 1})
        cursor = pos + len(needle)
    return hits, missing


def audit_source(checks: list[dict[str, object]]) -> list[dict[str, object]]:
    rows = []
    for check in checks:
        path = Path(check["path"])
        text = read(path)
        if "lines" in check:
            base, body = slice_text(text, str(check["lines"]))
        else:
            base, body = 1, text
        hits, missing = ordered_hits(body, list(check["needles"]), base)
        rows.append({
            "id": check["id"],
            "path": str(path),
            "sourceFile": path.name,
            "lines": check.get("lines"),
            "status": "PASS" if not missing else "FAIL",
            "sha256": sha(path),
            "hits": hits,
            "missing": missing,
        })
    return rows


def run_gate(args: list[str]) -> dict[str, object]:
    proc = subprocess.run(args, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return {
        "command": args,
        "returncode": proc.returncode,
        "passed": proc.returncode == 0,
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-8:]),
    }


def references() -> list[dict[str, object]]:
    items = [
        ("dm1_pc34_graphics", DM1 / "GRAPHICS.DAT", "canonical PC34 wall bitmap source"),
        ("dm1_pc34_dungeon", DM1 / "DUNGEON.DAT", "canonical DM1 V1 dungeon/map wallset source"),
        ("greatstone_index", GREATSTONE / "index/SUMMARY.md", "local Greatstone atlas index"),
        ("csbwin_source", CSBWIN / "CSBwin.cpp", "local CSBWin lineage reference"),
        ("csb_lineage_source", CSB / "src" / "CSBwin.cpp", "local CSB lineage reference"),
    ]
    rows = []
    for ident, path, use in items:
        rows.append({
            "id": ident,
            "path": str(path),
            "exists": path.exists(),
            "sha256": sha(path) if path.is_file() else None,
            "use": use,
        })
    return rows


def write_report(manifest: dict[str, object]) -> None:
    lines = [
        "# Pass510 DM1 V1 viewport wall parity flip source lock",
        "",
        "Status: " + str(manifest["status"]),
        "",
        "## ReDMCSB anchors",
    ]
    for row in manifest["redmcsbSourceAudit"]:
        lines.append(f"- {row['sourceFile']}:{row['lines']} {row['id']} status={row['status']}")
    lines += ["", "## Firestaff anchors"]
    for row in manifest["firestaffAudit"]:
        first = row["hits"][0]["line"] if row["hits"] else "missing"
        lines.append(f"- {row['sourceFile']}:{first} {row['id']} status={row['status']}")
    lines += ["", "## Local references"]
    for row in manifest["secondaryReferences"]:
        lines.append(f"- {row['id']} {row['path']} exists={row['exists']} sha256={row['sha256']}")
    lines += ["", "## Gates"]
    for gate in manifest["gates"]:
        lines.append(f"- {' '.join(gate['command'])} -> rc={gate['returncode']} passed={gate['passed']}")
    lines += [
        "",
        "## Scope",
        "- Locks the source-visible wall parity/native flip path after pass509 startup binding.",
        "- Does not claim original-vs-Firestaff pixel parity or a new DOSBox runtime capture.",
        "- Does not touch movement/pass435 ownership.",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    red = audit_source(SOURCE_CHECKS)
    fire = audit_source(FIRE_CHECKS)
    refs = references()
    gates = [
        run_gate([sys.executable, "tools/verify_pass509_dm1_v1_wallset_startup_binding.py"]),
        run_gate([sys.executable, "tools/verify_pass508_dm1_v1_viewport_wall_runtime_readiness.py"]),
    ]
    failures = [row["id"] for row in red + fire if row["status"] != "PASS"]
    failures += ["missing_ref_" + row["id"] for row in refs if not row["exists"]]
    failures += ["gate_" + Path(gate["command"][-1]).stem for gate in gates if not gate["passed"]]
    manifest = {
        "schema": "firestaff.parity.pass510_dm1_v1_viewport_wall_parity_flip_source_lock.v1",
        "status": STATUS if not failures else "FAIL_PASS510_DM1_V1_VIEWPORT_WALL_PARITY_FLIP_SOURCE_LOCK",
        "ok": not failures,
        "redmcsbSourceRoot": str(RED),
        "redmcsbSourceAudit": red,
        "firestaffAudit": fire,
        "secondaryReferences": refs,
        "gates": gates,
        "failures": failures,
        "claims": [
            "ReDMCSB selects flipped wall/footprint bitmaps from the party map X/Y/direction parity at F0128 entry.",
            "ReDMCSB uses swapped L/R wallset entries during that frame and restores native wallset pointers before presentation cleanup.",
            "Firestaff's normal renderer has a matching party tuple predicate and separate center/side wall flip paths.",
        ],
        "nonClaims": [
            "no original DOSBox runtime capture",
            "no original-vs-Firestaff pixel parity",
            "no movement/pass435 ownership",
        ],
    }
    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(manifest["status"])
    print(f"- wrote {REPORT.relative_to(ROOT)}")
    print(f"- wrote {MANIFEST.relative_to(ROOT)}")
    for failure in failures:
        print(f"- {failure}")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
