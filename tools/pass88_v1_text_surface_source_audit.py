#!/usr/bin/env python3
"""Pass 88: bounded DM1/V1 text-surface source audit.

This is an evidence-only audit. It cross-checks local ReDMCSB source facts,
local Firestaff helper seams, and original media presence for the text/label
rows that were still blanket-UNPROVEN in PARITY_MATRIX_DM1_V1.md.
It intentionally does not claim pixel parity and does not touch viewport,
HUD top-row, wall/item rendering, original overlay capture, or V2 assets.
"""

from __future__ import annotations

import hashlib
import json
import os
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
WORKSPACE = ROOT.parents[1]
SRC = WORKSPACE / "ReDMCSB_WIP20210206" / "Toolchains" / "Common" / "Source"
REF = Path.home() / ".openclaw" / "data"
GREATSTONE = REF / "firestaff-greatstone-atlas"
REDMCSB_CACHE = REF / "firestaff-redmcsb-source"
ORIGINAL_DM = REF / "firestaff-original-games" / "DM"
LOCAL_DM34 = ROOT / "verification-screens" / "dm1-dosbox-capture" / "DungeonMasterPC34"

CHECKS: list[dict[str, str]] = [
    {
        "id": "PASS88_SRC_MESSAGE_ROWS",
        "file": "DEFS.H",
        "pattern": r"#define\s+M532_MESSAGE_AREA_ROW_COUNT\s+4",
        "expect": "PC message area row count is four",
    },
    {
        "id": "PASS88_SRC_MESSAGE_ZONE",
        "file": "DEFS.H",
        "pattern": r"#define\s+C015_ZONE_MESSAGE_AREA\s+15",
        "expect": "C015 is the source message-area zone",
    },
    {
        "id": "PASS88_SRC_MESSAGE_PRINT_Y",
        "file": "TEXT.C",
        "pattern": r"G0358_i_MessageAreaCursorRow\s*\*\s*7\)\s*\+\s*177",
        "expect": "PC message text rows print at y=177+row*7",
    },
    {
        "id": "PASS88_SRC_SPELL_BACKDROP",
        "file": "DEFS.H",
        "pattern": r"#define\s+C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND\s+9",
        "expect": "spell backdrop is graphic 9",
    },
    {
        "id": "PASS88_SRC_SPELL_LINES",
        "file": "DEFS.H",
        "pattern": r"#define\s+C011_GRAPHIC_MENU_SPELL_AREA_LINES\s+11",
        "expect": "spell rune/symbol labels are graphic 11 line cells",
    },
    {
        "id": "PASS88_SRC_SPELL_DRAW_PATH",
        "file": "CASTER.C",
        "pattern": r"F0660_\(C009_GRAPHIC_MENU_SPELL_AREA_LINES,\s*C013_ZONE_SPELL_AREA",
        "expect": "CASTER.C blits C011 into C013 spell area",
    },
    {
        "id": "PASS88_SRC_FOOD_LABEL",
        "file": "DEFS.H",
        "pattern": r"#define\s+C030_GRAPHIC_FOOD_LABEL\s+30",
        "expect": "food label is graphic 30",
    },
    {
        "id": "PASS88_SRC_WATER_LABEL",
        "file": "DEFS.H",
        "pattern": r"#define\s+C031_GRAPHIC_WATER_LABEL\s+31",
        "expect": "water label is graphic 31",
    },
    {
        "id": "PASS88_SRC_POISON_LABEL",
        "file": "DEFS.H",
        "pattern": r"#define\s+C032_GRAPHIC_POISONED_LABEL\s+32",
        "expect": "poisoned label is graphic 32",
    },
    {
        "id": "PASS88_SRC_INVENTORY_LABEL_DRAW_PATH",
        "file": "PANEL.C",
        "pattern": r"C030_GRAPHIC_FOOD_LABEL[\s\S]+C031_GRAPHIC_WATER_LABEL[\s\S]+C032_GRAPHIC_POISONED_LABEL",
        "expect": "PANEL.C food/water/poison labels are source graphic blits, not text strings",
    },
    {
        "id": "PASS88_FS_MESSAGE_HELPER",
        "file": "m11_game_view.c",
        "pattern": r"int\s+M11_GameView_GetV1MessageAreaZoneId\(void\)[\s\S]+return\s+15;",
        "expect": "Firestaff exposes source C015 message-area zone id",
    },
    {
        "id": "PASS88_FS_MESSAGE_FILTER",
        "file": "m11_game_view.c",
        "pattern": r"m11_v1_strip_tick_prefix[\s\S]+m11_v1_message_is_player_facing",
        "expect": "Firestaff filters synthetic tick/debug text before C015 draw",
    },
    {
        "id": "PASS88_FS_SPELL_LABEL_C011",
        "file": "m11_game_view.c",
        "pattern": r"M11_SPELL_LABEL_CELL_W\s*=\s*14[\s\S]+M11_SPELL_LABEL_CELL_H\s*=\s*13[\s\S]+M11_GameView_GetV1SpellAreaLinesGraphicId",
        "expect": "Firestaff spell labels use the 14x13 C011 source cells",
    },
    {
        "id": "PASS88_FS_INVENTORY_LABEL_IDS",
        "file": "m11_game_view.c",
        "pattern": r"M11_GFX_FOOD_LABEL\s*=\s*30[\s\S]+M11_GFX_WATER_LABEL\s*=\s*31[\s\S]+M11_GFX_POISONED_LABEL\s*=\s*32",
        "expect": "Firestaff inventory label ids match source graphics 30/31/32",
    },
    {
        "id": "PASS88_FS_STATUS_NAME_ZONE",
        "file": "m11_game_view.c",
        "pattern": r"M11_GameView_GetV1StatusNameTextZoneId[\s\S]+return\s+163\s*\+\s*championSlot[\s\S]+M11_GameView_GetV1StatusNameTextZone",
        "expect": "Firestaff champion status name text is source-zone routed (C163..C166)",
    },
]


def read_text(base: Path, rel: str) -> str:
    return (base / rel).read_text(errors="replace")


def line_of(text: str, match_start: int) -> int:
    return text.count("\n", 0, match_start) + 1


def check_pattern(item: dict[str, str]) -> dict[str, object]:
    base = SRC if item["file"] in {"DEFS.H", "TEXT.C", "CASTER.C", "PANEL.C"} else ROOT
    path = base / item["file"]
    result: dict[str, object] = {
        "id": item["id"],
        "path": str(path),
        "expect": item["expect"],
    }
    if not path.exists():
        result.update(ok=False, error="missing file")
        return result
    text = read_text(base, item["file"])
    m = re.search(item["pattern"], text, re.MULTILINE)
    result["ok"] = bool(m)
    if m:
        result["line"] = line_of(text, m.start())
        snippet = text[m.start(): min(len(text), m.start() + 180)]
        result["snippet"] = " ".join(snippet.split())
    return result


def sha256(path: Path) -> str | None:
    if not path.exists():
        return None
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def media_manifest() -> dict[str, object]:
    entries = []
    for rel in ["TITLE", "DATA/GRAPHICS.DAT", "DATA/DUNGEON.DAT", "DATA/SONG.DAT"]:
        p = LOCAL_DM34 / rel
        entries.append({"path": str(p), "exists": p.exists(), "bytes": p.stat().st_size if p.exists() else None, "sha256": sha256(p)})
    archives = []
    for name in ["Dungeon-Master_DOS_EN.zip", "Game,Dungeon_Master,DOS,Software.7z"]:
        p = ORIGINAL_DM / name
        archives.append({"path": str(p), "exists": p.exists(), "bytes": p.stat().st_size if p.exists() else None, "sha256": sha256(p)})
    return {
        "redmcsb_cache": {"path": str(REDMCSB_CACHE), "exists": REDMCSB_CACHE.exists()},
        "greatstone_atlas": {"path": str(GREATSTONE), "exists": GREATSTONE.exists(), "files_json": (GREATSTONE / "index" / "files.json").exists()},
        "original_dm_archives": archives,
        "local_dm34_runtime_files": entries,
    }


def main() -> int:
    checks = [check_pattern(c) for c in CHECKS]
    ok = sum(1 for c in checks if c.get("ok"))
    out = {
        "audit": "pass88_v1_text_surface_source_audit",
        "scope": "DM1 V1 text/label rows only; no viewport/walls/items/HUD top-row/original-overlay/V2 work",
        "root": str(ROOT),
        "references": media_manifest(),
        "checks": checks,
        "summary": {"passed": ok, "total": len(checks), "failed": len(checks) - ok},
        "honesty": "Source/helper evidence only. Pixel parity, exact semantic route parity, wall plaque/scroll text capture parity, and original overlay parity are not claimed.",
    }
    print(json.dumps(out, indent=2, sort_keys=True))
    return 0 if ok == len(checks) else 1


if __name__ == "__main__":
    sys.exit(main())
