#!/usr/bin/env python3
"""Lock the DM1 V2 journal, minimap and tooltip compatibility policy."""

from __future__ import annotations

import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = (
    Path.home()
    / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
ARCHIVE = Path.home() / ".firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"
DMWEB = ROOT / "reference/dmweb-community-docs/html/community/documentation/dungeon-master-and-chaos-strikes-back/graphics.dat-item-562.html"
GREATSTONE = ROOT / "tests/fixtures/greatstone_db_data_paths/index.json"
EVIDENCE = ROOT / "parity-evidence/verification/dm1_v2_synthetic_ui_overlays_source_lock.json"

errors: list[str] = []

for filename, needles in {
    "GAMELOOP.C": ["F0128_DUNGEONVIEW_Draw_CPSF"],
    "DUNVIEW.C": ["void F0128_DUNGEONVIEW_Draw_CPSF", "F0097_DUNGEONVIEW_DrawViewport"],
    "CHAMPION.C": ["F0047_TEXT_MESSAGEAREA_PrintMessage"],
}.items():
    path = SOURCE / filename
    if not path.exists():
        errors.append(f"missing ReDMCSB source {path}")
        continue
    text = path.read_text(encoding="utf-8", errors="replace")
    for needle in needles:
        if needle not in text:
            errors.append(f"{filename}: missing {needle!r}")

if SOURCE.exists():
    source_text = "\n".join(
        path.read_text(encoding="utf-8", errors="replace")
        for path in SOURCE.glob("*.C")
    ).lower()
    for term in ("journal", "automap", "minimap", "tooltip"):
        if term in source_text:
            errors.append(f"ReDMCSB source unexpectedly contains {term!r}")

if not ARCHIVE.exists():
    errors.append(f"missing PC34 archive {ARCHIVE}")
else:
    listing = subprocess.run(
        ["unzip", "-Z1", str(ARCHIVE)], check=False, capture_output=True, text=True
    )
    if listing.returncode != 0:
        errors.append("could not list PC34 archive")
    else:
        members = set(listing.stdout.splitlines())
        for required in ("DATA/GRAPHICS.DAT", "DATA/DUNGEON.DAT", "DM.EXE"):
            if required not in members:
                errors.append(f"PC34 archive missing {required}")

if not DMWEB.exists():
    errors.append(f"missing DMWeb reference {DMWEB}")
else:
    dmweb_text = DMWEB.read_text(encoding="utf-8", errors="replace")
    for needle in ("Masks to print text", "rectangle-to-clear-held-object-name"):
        if needle not in dmweb_text:
            errors.append(f"DMWeb item 562 missing {needle!r}")

if not GREATSTONE.exists():
    errors.append(f"missing Greatstone fixture {GREATSTONE}")
else:
    greatstone_paths = json.loads(GREATSTONE.read_text(encoding="utf-8")).get("paths", [])
    pc34_graphics = [
        entry for entry in greatstone_paths
        if entry.get("path") == "db_data/dm_pc_34/graphics.dat/graphics.dat.html"
    ]
    if len(pc34_graphics) != 1 or pc34_graphics[0].get("title") != "mapfile extracted items":
        errors.append("Greatstone fixture lacks the DM1 PC34 GRAPHICS.DAT mapfile record")

required = {
    "src/dm1v2/dm1_v2_journal_pc34.c": ["retains no", "return NULL;", "return false;"],
    "src/dm1v2/dm1_v2_journal.c": ["must not turn source", "(void)rgba;"],
    "src/dm1v2/dm1_v2_minimap_pc34.c": ["has no map cache", "(void)framebuffer;", "return false;"],
    "src/dm1v2/dm1_v2_minimap.c": ["do not construct a map", "(void)rgba;"],
    "src/dm1v2/dm1_v2_tooltip_pc34.c": ["no PC34 owner", "return false;", "no host glyph"],
    "tests/test_dm1_v2_synthetic_ui_overlays_pc34.c": ["memcmp(framebuffer, before", "v2_journal_save", "v2_minimap_is_explored"],
}
for relative, needles in required.items():
    path = ROOT / relative
    if not path.exists():
        errors.append(f"missing Firestaff file {relative}")
        continue
    text = path.read_text(encoding="utf-8")
    for needle in needles:
        if needle not in text:
            errors.append(f"{relative}: missing {needle!r}")

forbidden = {
    "src/dm1v2/dm1_v2_journal_pc34.c": ["g_v2_journal", "fopen(", "fwrite("],
    "src/dm1v2/dm1_v2_journal.c": ["memmove(", "type_color", "Semi-transparent overlay"],
    "src/dm1v2/dm1_v2_minimap_pc34.c": ["M11_V2_MINIMAP_COLORS", "v2_minimap_plot_pixel", "arrow_color"],
    "src/dm1v2/dm1_v2_minimap.c": ["Alpha blend", "0xFF00FF00", "cell_size ="],
    "src/dm1v2/dm1_v2_tooltip_pc34.c": ["FONT_4x5", "g_tooltip", "fade_alpha"],
}
for relative, needles in forbidden.items():
    text = (ROOT / relative).read_text(encoding="utf-8")
    for needle in needles:
        if needle in text:
            errors.append(f"{relative} retains synthetic UI data: {needle!r}")

result = {
    "status": "failed" if errors else "passed",
    "scope": "DM1 V2 journal, minimap and tooltip compatibility surfaces",
    "redmcsbSourceRoot": str(SOURCE),
    "pc34Archive": str(ARCHIVE),
    "dmwebReference": str(DMWEB.relative_to(ROOT)),
    "greatstoneReference": "tests/fixtures/greatstone_db_data_paths/index.json: verified dm_pc_34 GRAPHICS.DAT mapfile record; no journal/minimap/tooltip material record",
    "sourceOwnedRoutes": [
        "GAMELOOP.C:F0128_DUNGEONVIEW_Draw_CPSF",
        "DUNVIEW.C:F0128_DUNGEONVIEW_Draw_CPSF/F0097_DUNGEONVIEW_DrawViewport",
        "CHAMPION.C:F0047_TEXT_MESSAGEAREA_PrintMessage",
    ],
    "forbiddenSyntheticState": forbidden,
    "errors": errors,
}
EVIDENCE.parent.mkdir(parents=True, exist_ok=True)
EVIDENCE.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

if errors:
    print("dm1_v2_synthetic_ui_overlays_source_lock=FAIL")
    print("\n".join(errors))
    sys.exit(1)

print(f"dm1_v2_synthetic_ui_overlays_source_lock=OK evidence={EVIDENCE.relative_to(ROOT)}")
