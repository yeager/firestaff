#!/usr/bin/env python3
"""Source-lock DM1 V1 PC34 viewport wall/occlusion metadata.

This gate keeps the lightweight dm1_v1_viewport_3d_pc34_compat wall metadata
honest against the local ReDMCSB DUNVIEW.C PC34/I34E branches.  It verifies each
known wall square has:

* native wall-set entry;
* parity-flipped source entry and horizontal flip call where applicable;
* PC34 viewport zone;
* the wall-case return or front-alcove exception used as the occlusion contract.
"""
from __future__ import annotations

from dataclasses import dataclass
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
LOCAL = ROOT / "dm1_v1_viewport_3d_pc34_compat.c"
DEFAULT_REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C"
REDMCSB = Path(os.environ.get("FIRESTAFF_REDMCSB_DUNVIEW", DEFAULT_REDMCSB))


@dataclass(frozen=True)
class WallCase:
    square: str
    native: str
    parity: str
    zone: str
    source_lines: str
    redmcsb_range: tuple[int, int]
    redmcsb_needles: tuple[str, ...]
    local_needles: tuple[str, ...]


CASES: tuple[WallCase, ...] = (
    WallCase("D3L2", "C11_WALL_D3L2", "C10_WALL_D3R2", "C702_ZONE_WALL_D3L2", "DUNVIEW.C:6254-6260", (6254, 6264), (
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C10_WALL_D3R2], C702_ZONE_WALL_D3L2);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C11_WALL_D3L2], C702_ZONE_WALL_D3L2);",
        "return;",
    ), ("DM1_VIEW_SQUARE_D3L2, DM1_WALL_D3L2, DM1_WALL_D3R2", "DUNVIEW.C:6254-6260", "DUNVIEW.C:6263-6264 wall ornament then return")),
    WallCase("D3R2", "C10_WALL_D3R2", "C11_WALL_D3L2", "C703_ZONE_WALL_D3R2", "DUNVIEW.C:6321-6327", (6321, 6331), (
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C11_WALL_D3L2], C703_ZONE_WALL_D3R2);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C10_WALL_D3R2], C703_ZONE_WALL_D3R2);",
        "return;",
    ), ("DM1_VIEW_SQUARE_D3R2, DM1_WALL_D3R2, DM1_WALL_D3L2", "DUNVIEW.C:6321-6327", "DUNVIEW.C:6330-6331 wall ornament then return")),
    WallCase("D3L", "C13_WALL_D3L", "C12_WALL_D3R", "C705_ZONE_WALL_D3L", "DUNVIEW.C:6421-6427", (6421, 6437), (
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C12_WALL_D3R], C705_ZONE_WALL_D3L);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C13_WALL_D3L], C705_ZONE_WALL_D3L);",
        "C0x0000_CELL_ORDER_ALCOVE",
        "return;",
    ), ("DM1_VIEW_SQUARE_D3L,  DM1_WALL_D3L,  DM1_WALL_D3R", "DUNVIEW.C:6421-6427", "front alcove branches to F0115, else return")),
    WallCase("D3R", "C12_WALL_D3R", "C13_WALL_D3L", "C706_ZONE_WALL_D3R", "DUNVIEW.C:6554-6564", (6554, 6573), (
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C13_WALL_D3L], C706_ZONE_WALL_D3R);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C12_WALL_D3R], C706_ZONE_WALL_D3R);",
        "C0x0000_CELL_ORDER_ALCOVE",
        "return;",
    ), ("DM1_VIEW_SQUARE_D3R,  DM1_WALL_D3R,  DM1_WALL_D3L", "DUNVIEW.C:6554-6564", "front alcove branches to F0115, else return")),
    WallCase("D3C", "C14_WALL_D3C", "C14_WALL_D3C", "C704_ZONE_WALL_D3C", "DUNVIEW.C:6707-6714", (6707, 6720), (
        "F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C14_WALL_D3C], C704_ZONE_WALL_D3C, G0076_B_UseFlippedWallAndFootprintsBitmaps);",
        "C0x0000_CELL_ORDER_ALCOVE",
        "return;",
    ), ("DM1_VIEW_SQUARE_D3C,  DM1_WALL_D3C,  DM1_WALL_D3C", "DUNVIEW.C:6707-6714", "front alcove branches to F0115, else return")),
    WallCase("D2L2", "C06_WALL_D2L2", "C05_WALL_D2R2", "C707_ZONE_WALL_D2L2", "DUNVIEW.C:6849-6858", (6849, 6862), (
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C05_WALL_D2R2], C707_ZONE_WALL_D2L2);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C06_WALL_D2L2]",
        "return;",
    ), ("DM1_VIEW_SQUARE_D2L2, DM1_WALL_D2L2, DM1_WALL_D2R2", "DUNVIEW.C:6849-6858", "DUNVIEW.C:6848-6862 wall case returns")),
    WallCase("D2R2", "C05_WALL_D2R2", "C06_WALL_D2L2", "C708_ZONE_WALL_D2R2", "DUNVIEW.C:6880-6889", (6880, 6893), (
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C06_WALL_D2L2], C708_ZONE_WALL_D2R2);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C05_WALL_D2R2]",
        "return;",
    ), ("DM1_VIEW_SQUARE_D2R2, DM1_WALL_D2R2, DM1_WALL_D2L2", "DUNVIEW.C:6880-6889", "DUNVIEW.C:6882-6893 wall case returns")),
    WallCase("D2L", "C08_WALL_D2L", "C07_WALL_D2R", "C710_ZONE_WALL_D2L", "DUNVIEW.C:6954-6964", (6954, 6973), (
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C07_WALL_D2R], C710_ZONE_WALL_D2L);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C08_WALL_D2L], C710_ZONE_WALL_D2L);",
        "C0x0000_CELL_ORDER_ALCOVE",
        "return;",
    ), ("DM1_VIEW_SQUARE_D2L,  DM1_WALL_D2L,  DM1_WALL_D2R", "DUNVIEW.C:6954-6964", "front alcove branches to F0115, else return")),
    WallCase("D2R", "C07_WALL_D2R", "C08_WALL_D2L", "C711_ZONE_WALL_D2R", "DUNVIEW.C:7105-7115", (7105, 7166), (
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C08_WALL_D2L], C711_ZONE_WALL_D2R);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C07_WALL_D2R], C711_ZONE_WALL_D2R);",
        "C0x0000_CELL_ORDER_ALCOVE",
        "return;",
    ), ("DM1_VIEW_SQUARE_D2R,  DM1_WALL_D2R,  DM1_WALL_D2L", "DUNVIEW.C:7105-7115", "DUNVIEW.C:7119-7123 front alcove branch; DUNVIEW.C:7166 blocker return")),
    WallCase("D2C", "C09_WALL_D2C", "C09_WALL_D2C", "C709_ZONE_WALL_D2C", "DUNVIEW.C:7299-7306", (7299, 7312), (
        "F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C09_WALL_D2C], C709_ZONE_WALL_D2C, G0076_B_UseFlippedWallAndFootprintsBitmaps);",
        "C0x0000_CELL_ORDER_ALCOVE",
        "return;",
    ), ("DM1_VIEW_SQUARE_D2C,  DM1_WALL_D2C,  DM1_WALL_D2C", "DUNVIEW.C:7299-7306", "front alcove branches to F0115, else return")),
    WallCase("D1L", "C03_WALL_D1L", "C02_WALL_D1R", "C713_ZONE_WALL_D1L", "DUNVIEW.C:7445-7455", (7445, 7460), (
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C02_WALL_D1R], C713_ZONE_WALL_D1L);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C03_WALL_D1L], C713_ZONE_WALL_D1L);",
        "return;",
    ), ("DM1_VIEW_SQUARE_D1L,  DM1_WALL_D1L,  DM1_WALL_D1R", "DUNVIEW.C:7445-7455", "DUNVIEW.C:7459-7460 side ornament then return")),
    WallCase("D1R", "C02_WALL_D1R", "C03_WALL_D1L", "C714_ZONE_WALL_D1R", "DUNVIEW.C:7613-7623", (7613, 7628), (
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C03_WALL_D1L], C714_ZONE_WALL_D1R);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C02_WALL_D1R], C714_ZONE_WALL_D1R);",
        "return;",
    ), ("DM1_VIEW_SQUARE_D1R,  DM1_WALL_D1R,  DM1_WALL_D1L", "DUNVIEW.C:7613-7623", "DUNVIEW.C:7627-7628 side ornament then return")),
    WallCase("D1C", "C04_WALL_D1C", "C04_WALL_D1C", "C712_ZONE_WALL_D1C", "DUNVIEW.C:7833-7840", (7833, 7843), (
        "F0792_DUNGEONVIEW_DrawBitmapYYY(G2107_WallSet[C04_WALL_D1C], C712_ZONE_WALL_D1C, G0076_B_UseFlippedWallAndFootprintsBitmaps);",
        "C0x0000_CELL_ORDER_ALCOVE",
        "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
    ), ("DM1_VIEW_SQUARE_D1C,  DM1_WALL_D1C,  DM1_WALL_D1C", "DUNVIEW.C:7833-7840", "DUNVIEW.C:7842-7843 front alcove draws F0115; no side cells behind D1C")),
    WallCase("D0L", "C01_WALL_D0L", "C00_WALL_D0R", "C716_ZONE_WALL_D0L", "DUNVIEW.C:8016-8033", (8016, 8038), (
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C00_WALL_D0R], C716_ZONE_WALL_D0L);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C01_WALL_D0L], C716_ZONE_WALL_D0L);",
        "return;",
    ), ("DM1_VIEW_SQUARE_D0L,  DM1_WALL_D0L,  DM1_WALL_D0R", "DUNVIEW.C:8016-8033", "DUNVIEW.C:8036-8038 wall case returns")),
    WallCase("D0R", "C00_WALL_D0R", "C01_WALL_D0L", "C717_ZONE_WALL_D0R", "DUNVIEW.C:8126-8139", (8126, 8144), (
        "F0105_DUNGEONVIEW_DrawFloorPitOrStairsBitmapFlippedHorizontally(G2107_WallSet[C01_WALL_D0L], C717_ZONE_WALL_D0R);",
        "F0104_DUNGEONVIEW_DrawFloorPitOrStairsBitmap(G2107_WallSet[C00_WALL_D0R], C717_ZONE_WALL_D0R);",
        "return;",
    ), ("DM1_VIEW_SQUARE_D0R,  DM1_WALL_D0R,  DM1_WALL_D0L", "DUNVIEW.C:8126-8139", "DUNVIEW.C:8142-8144 wall case returns")),
)


def line_slice(text: str, bounds: tuple[int, int]) -> str:
    start, end = bounds
    return "\n".join(text.splitlines()[start - 1 : end])


def main() -> int:
    local = LOCAL.read_text(encoding="utf-8")
    redmcsb = REDMCSB.read_text(encoding="latin-1")
    failures: list[str] = []

    if local.count("{ DM1_VIEW_SQUARE_") < len(CASES):
        failures.append("local wall draw spec table has fewer entries than expected")

    for case in CASES:
        excerpt = line_slice(redmcsb, case.redmcsb_range)
        for needle in case.redmcsb_needles:
            if needle not in excerpt:
                failures.append(f"{case.square} missing ReDMCSB needle in lines {case.redmcsb_range[0]}-{case.redmcsb_range[1]}: {needle}")
        for needle in case.local_needles:
            if needle not in local:
                failures.append(f"{case.square} missing local metadata needle: {needle}")

    if failures:
        print("FAIL dm1-v1-viewport-3d-occlusion-metadata-gate")
        for failure in failures:
            print(f"- {failure}")
        return 1

    print("PASS dm1-v1-viewport-3d-occlusion-metadata-gate")
    print(f"redmcsb={REDMCSB}")
    for case in CASES:
        start, end = case.redmcsb_range
        print(f"- {case.square}: {case.native}->{case.parity} {case.zone} {case.source_lines} occlusion lines {start}-{end}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except OSError as exc:
        print(f"FAIL dm1-v1-viewport-3d-occlusion-metadata-gate: {exc}", file=sys.stderr)
        raise SystemExit(1)
