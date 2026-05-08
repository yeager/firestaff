#!/usr/bin/env python3
"""Verify CSB V1 inventory render/grid anchors against local CSB lineage sources.

Evidence-only gate. It uses only N2-local CSB/CSBWin secondary lineage trees and
locks the inventory-screen rendering, item rendering, grid/layout, and viewport
return seam without enabling CSB runtime support.
"""
from __future__ import annotations

import argparse
from pathlib import Path

DEFAULT_CSBWIN = Path("/home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin")
DEFAULT_CSB = Path("/home/trv2/.openclaw/data/firestaff-csb-source/CSB/src")

CHECKS = [
    {
        "id": "csbwin-item-icon-render-target-and-atlas",
        "root": "csbwin",
        "file": "Character.cpp",
        "range": "31-76",
        "needles": [
            "void DrawItem(i32 squareNumber, OBJ_NAME_INDEX objectNameIndex)",
            "idA3 = &d.IconDisplay[squareNumber]",
            "LOCAL_8.w.x1 = idA3->pixelX",
            "LOCAL_8.w.x2 = (i16)(LOCAL_8.w.x1 + 15)",
            "LOCAL_8.w.y1 = idA3->pixelY",
            "LOCAL_8.w.y2 = (i16)(LOCAL_8.w.y1 + 15)",
            "A2 = (i8 *)GetBasicGraphicAddress(D5W + 42)",
            "if (squareNumber >= 8)",
            "LOCAL_12 = d.pViewportBMP",
            "D4W = 112",
            "LOCAL_12 = d.LogicalScreenBase",
            "D4W = 160",
            "TAG0088b2((ui8 *)A2",
            "(D6W & 15) << 4",
            "D6W & 0xff0",
        ],
    },
    {
        "id": "csb-src-item-icon-render-target-and-atlas",
        "root": "csb",
        "file": "Character.cpp",
        "range": "31-77",
        "needles": [
            "void DrawItem(i32 squareNumber, OBJ_NAME_INDEX objectNameIndex)",
            "idA3 = &d.IconDisplay[squareNumber]",
            "LOCAL_8.w.x1 = idA3->pixelX",
            "LOCAL_8.w.x2 = (i16)(LOCAL_8.w.x1 + 15)",
            "LOCAL_8.w.y1 = idA3->pixelY",
            "LOCAL_8.w.y2 = (i16)(LOCAL_8.w.y1 + 15)",
            "A2 = (i8 *)GetBasicGraphicAddress(D5W + 42)",
            "if (squareNumber >= 8)",
            "LOCAL_12 = d.pViewportBMP",
            "D4W = 112",
            "LOCAL_12 = d.LogicalScreenBase",
            "D4W = 160",
            "TAG0088b2((ui8 *)A2",
            "(D6W & 15) << 4",
            "D6W & 0xff0",
        ],
    },
    {
        "id": "csbwin-icon-graphic-16x16-atlas-copy",
        "root": "csbwin",
        "file": "Code11f52.cpp",
        "range": "82-115",
        "needles": [
            "void GetIconGraphic(OBJ_NAME_INDEX iconNum, ui8 *dest)",
            "D0W = d.Word612[D6W]",
            "A2 = (pnt)GetBasicGraphicAddress((--D6W) + 42)",
            "D7W = sw(objNID7 - d.Word612[D6W])",
            "D0W = (I16)(D7W & 0xff0)",
            "D1W = (I16)((D7W & 15) << 1)",
            "for (D6W=0; D6W<16; D6W++)",
            "A2 += 124",
        ],
    },
    {
        "id": "csb-src-icon-graphic-16x16-atlas-copy",
        "root": "csb",
        "file": "Code11f52.cpp",
        "range": "109-142",
        "needles": [
            "void GetIconGraphic(OBJ_NAME_INDEX iconNum, ui8 *dest)",
            "D0W = d.Word612[D6W]",
            "A2 = (pnt)GetBasicGraphicAddress((--D6W) + 42)",
            "D7W = sw(objNID7 - d.Word612[D6W])",
            "D0W = (I16)(D7W & 0xff0)",
            "D1W = (I16)((D7W & 15) << 1)",
            "for (D6W=0; D6W<16; D6W++)",
            "A2 += 124",
        ],
    },
    {
        "id": "csbwin-backpack-grid-viewport-slot-map",
        "root": "csbwin",
        "file": "Character.cpp",
        "range": "1099-1218",
        "needles": [
            "void DisplayBackpackItem(i32 chIdx, i32 itemNum)",
            "D5W = sw((d.SelectedCharacterOrdinal == (chIdx+1)) ? 1 : 0)",
            "if (D7W > 1) return",
            "squareNumber = 2*chIdx + D7W",
            "squareNumber = D7W + 8",
            "if (D7W >= 30)",
            "objD6 = d.rnChestContents[D7W-30]",
            "idA2 = &d.IconDisplay[squareNumber]",
            "LOCAL_18.w.x1 = sw(idA2->pixelX - 1)",
            "LOCAL_18.w.y1 = sw(idA2->pixelY - 1)",
            "LOCAL_24 = d.pViewportBMP",
            "LOCAL_26 = 112",
            "LOCAL_24 = d.LogicalScreenBase",
            "LOCAL_26 = 160",
            "DrawItem(squareNumber, objNID4)",
        ],
    },
    {
        "id": "csb-src-backpack-grid-viewport-slot-map",
        "root": "csb",
        "file": "Character.cpp",
        "range": "1086-1208",
        "needles": [
            "void DisplayBackpackItem(i32 chIdx, i32 itemNum)",
            "D5W = sw((d.SelectedCharacterOrdinal == (chIdx+1)) ? 1 : 0)",
            "if (D7W > 1) return",
            "squareNumber = 2*chIdx + D7W",
            "squareNumber = D7W + 8",
            "if (D7W >= 30)",
            "objD6 = d.rnChestContents[D7W-30]",
            "idA2 = &d.IconDisplay[squareNumber]",
            "LOCAL_18.w.x1 = sw(idA2->pixelX - 1)",
            "LOCAL_18.w.y1 = sw(idA2->pixelY - 1)",
            "LOCAL_24 = d.pViewportBMP",
            "LOCAL_26 = 112",
            "LOCAL_24 = d.LogicalScreenBase",
            "LOCAL_26 = 160",
            "DrawItem(squareNumber, objNID4)",
        ],
    },
    {
        "id": "csbwin-inventory-open-render-and-viewport-return",
        "root": "csbwin",
        "file": "CSBCode.cpp",
        "range": "7410-7511",
        "needles": [
            "void ShowHideInventory(i32 chIdx)",
            "if (d.PressingMouth != 0) return",
            "if (d.PressingEye != 0) return",
            "videoMode = VM_ADVENTURE",
            "RepackChest()",
            "DrawMovementButtons()",
            "FloorAndCeilingOnly()",
            "d.SelectedCharacterOrdinal = sw(chIdx + 1)",
            "videoMode = VM_INVENTORY",
            "TAG022a60(17, d.pViewportBMP)",
            "TextToViewport(5, 116",
            "TextToViewport(5, 124",
            "TextToViewport(5, 132",
            "for (itemNum=0; itemNum<30; itemNum++)",
            "DisplayBackpackItem(chIdx, itemNum)",
            "DrawCharacterState(chIdx)",
            "d.SecondaryButtonList = d.Buttons17760",
        ],
    },
    {
        "id": "csb-src-inventory-open-render-and-viewport-return",
        "root": "csb",
        "file": "CSBCode.cpp",
        "range": "7405-7507",
        "needles": [
            "void ShowHideInventory(i32 chIdx)",
            "if (d.PressingMouth != 0) return",
            "if (d.PressingEye != 0) return",
            "videoMode = VM_ADVENTURE",
            "RepackChest()",
            "DrawMovementButtons()",
            "FloorAndCeilingOnly()",
            "d.SelectedCharacterOrdinal = sw(chIdx + 1)",
            "videoMode = VM_INVENTORY",
            "TAG022a60(17, d.pViewportBMP)",
            "TextToViewport(5, 116",
            "TextToViewport(5, 124",
            "TextToViewport(5, 132",
            "for (itemNum=0; itemNum<30; itemNum++)",
            "DisplayBackpackItem(chIdx, itemNum)",
            "DrawCharacterState(chIdx)",
            "d.SecondaryButtonList = d.Buttons17760",
        ],
    },
    {
        "id": "csb-src-inventory-state-redraw-seam",
        "root": "csb",
        "file": "Character.cpp",
        "range": "1219-1490",
        "needles": [
            "void DrawCharacterState(i32 chIdx)",
            "inventoryOpen = ((chIdx+1) == d.SelectedCharacterOrdinal)",
            "CharacterPortraitToStatusBox(chIdx)",
            "TextToViewport(3, 7",
            "itemNum = inventoryOpen ? 5 : 1",
            "DisplayBackpackItem(chIdx, itemNum)",
            "TextToViewport(104, 132",
            "charFlags |= CHARFLAG_viewportChanged",
            "DisplayFoodWater()",
            "TAG0189d4()",
        ],
    },
    {
        "id": "csb-src-food-water-inventory-overlay",
        "root": "csb",
        "file": "Viewport.cpp",
        "range": "7240-7258",
        "needles": [
            "void DisplayFoodWater(void)",
            "pcA3 = &d.hero[d.SelectedCharacterOrdinal-1]",
            "RepackChest()",
            "BLT2Viewport(GetBasicGraphicAddress(20)",
            "BLT2Viewport(GetBasicGraphicAddress(30)",
            "BLT2Viewport(GetBasicGraphicAddress(31)",
            "DrawFoodWaterBar(pcA3->food, 69, 5)",
            "DrawFoodWaterBar(pcA3->water, 92, 14)",
        ],
    },
    {
        "id": "csbwin-clothing-click-state-mutation-redraw",
        "root": "csbwin",
        "file": "CSBCode.cpp",
        "range": "6919-6990",
        "needles": [
            "void HandleClothingClick(i32 button)",
            "D6W = (I16)(button & 1)",
            "D6W = sw(button-8)",
            "objD4 = d.rnChestContents[D6W-30]",
            "objD4 = d.CH16482[D7W].Possession(D6W)",
            "RemoveObjectFromHand()",
            "RemoveCharacterPossession(D7W, D6W)",
            "ObjectToCursor(objD4, 0)",
            "AddCharacterPossession(D7W, objD5, D6W)",
            "DrawCharacterState(D7W)",
        ],
    },
]


def line_slice(text: str, line_range: str) -> str:
    start, end = [int(part) for part in line_range.split("-", maxsplit=1)]
    return "\n".join(text.splitlines()[start - 1 : end])


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--csbwin-source", type=Path, default=DEFAULT_CSBWIN)
    parser.add_argument("--csb-source", type=Path, default=DEFAULT_CSB)
    args = parser.parse_args()

    roots = {"csbwin": args.csbwin_source, "csb": args.csb_source}
    failures: list[str] = []
    passes: list[str] = []

    for check in CHECKS:
        path = roots[check["root"]] / check["file"]
        if not path.exists():
            failures.append("FAIL {}: missing {}".format(check["id"], path))
            continue
        haystack = line_slice(path.read_text(errors="replace"), check["range"])
        missing = [needle for needle in check["needles"] if needle not in haystack]
        if missing:
            failures.append(
                "FAIL {}: {}:{} missing {}".format(check["id"], path, check["range"], "; ".join(repr(needle) for needle in missing))
            )
            continue
        passes.append("PASS {}: {}:{}".format(check["id"], path.name, check["range"]))

    print("[csb v1 inventory render/grid source lock]")
    for line in passes:
        print(line)
    for line in failures:
        print(line)
    if failures:
        return 1
    print(f"PASS {len(passes)} CSB V1 inventory render/grid source-lock checks")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
