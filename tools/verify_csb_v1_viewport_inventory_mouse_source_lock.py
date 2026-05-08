#!/usr/bin/env python3
"""Verify CSB V1 viewport/inventory mouse seam against local CSB lineage sources."""
from __future__ import annotations

import argparse
from pathlib import Path

DEFAULT_CSBWIN = Path("/home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin")
DEFAULT_CSB = Path("/home/trv2/.openclaw/data/firestaff-csb-source/CSB/src")

CHECKS = [
    {
        "id": "csbwin-viewport-front-cell-and-door-click",
        "root": "csbwin",
        "file": "Mouse.cpp",
        "range": "1031-1086",
        "needles": [
            "void HandleClickInViewport(i32 clickX, i32 clickY)",
            "touchedX = d.partyX;",
            "touchedY = d.partyY;",
            "D1W = d.DeltaX[D1W];",
            "touchedX = sw(touchedX + D1W); // Cell touched X",
            "D1W = d.DeltaY[d.partyFacing];",
            "touchedY = sw(touchedY + D1W); // Cell touched Y",
            "if (d.CellTypeJustAhead == roomDOORFACING)",
            "if (d.HandChar == -1)",
            "D0W = TestInRectangle((ui8 *)&d.ViewportObjectButtons[5],",
            "CreateTimer(TT_DOOR,",
        ],
        "why": "CSBWin routes viewport clicks through the front-cell coordinate and door-button path before generic wall/object handling.",
    },
    {
        "id": "csbwin-empty-hand-wall-object-buttons",
        "root": "csbwin",
        "file": "Mouse.cpp",
        "range": "1096-1160",
        "needles": [
            "D0W = d.EmptyHanded;",
            "for (D5W=0; D5W<6; D5W++)",
            "D0W = TestInRectangle((ui8 *)&d.ViewportObjectButtons[D5W],",
            "if (D5W == 5)",
            "TouchWallF1();",
            "TAG01a148(D5W);",
            "RecordFile_Record(clickX, clickY, 80);",
        ],
        "why": "Empty-hand viewport clicks prefer source object-button hitboxes, with button 5 as wall/touch and 0-4 as object-button routes.",
    },
    {
        "id": "csbwin-object-in-hand-drop-fill-touch",
        "root": "csbwin",
        "file": "Mouse.cpp",
        "range": "1162-1228",
        "needles": [
            "ASSERT(RememberToPutObjectInHand == -1,\"objInHand\");",
            "LOCAL_2 = d.objectInHand;",
            "D0W = TestInRectangle(d.DropAreas+4*D5W, clickX, clickY);",
            "DropObject(D5W); // D5=position",
            "D0W = TestInRectangle((ui8 *)&d.ViewportObjectButtons[5],",
            "if (d.FacingAlcove != 0)",
            "DropObject(4); // 4 = position",
            "if (d.FacingWaterFountain != 0)",
            "TouchWallF1();",
        ],
        "why": "Object-in-hand viewport clicks are source-ordered as drop areas, alcove drop, fountain fill, then wall touch.",
    },
    {
        "id": "csbwin-inventory-eye-mouth-viewport-dispatch",
        "root": "csbwin",
        "file": "Mouse.cpp",
        "range": "1600-1668",
        "needles": [
            "if ((D6W>=28)&&(D6W<66))",
            "HandleClothingClick(D6W-20);",
            "if ((D6W>=7) && (D6W<=11))",
            "ShowHideInventory(D7W);",
            "if (D6W == 83)",
            "ShowHideInventory(d.HandChar);",
            "if (D6W == 70)",
            "FeedCharacter(_10_);",
            "if (D6W == 71)",
            "ClickOnEye(_4_);",
            "if (D6W == 80) // The Viewport while adventuring",
            "if (d.ClockRunning != 0) HandleClickInViewport(clickX, clickY);",
        ],
        "why": "Inventory/clothing, mouth, eye, and viewport clicks are distinct dispatch codes; viewport handling must not absorb inventory routes.",
    },
    {
        "id": "csb-lineage-fountain-empty-hand-delta",
        "root": "csb",
        "file": "Mouse.cpp",
        "range": "1148-1165",
        "needles": [
            "if (D5W == 5)",
            "if (d.FacingWaterFountain) {",
            "pc = &d.hero[d.HandChar]; // selected character",
            "pc->water = sw(LIMIT(-1023,pc->water+1600,2048));",
            "QueueSound(8, d.partyX, d.partyY, 0); // gulp sound for the fountain",
            "if (d.FacingAlcove == 0)",
            "TouchWallF1();",
            "TAG01a148(D5W);",
        ],
        "why": "The CSB src lineage adds empty-hand fountain drinking on viewport button 5 before the ordinary TouchWallF1 path.",
    },
    {
        "id": "csb-lineage-inventory-eye-mouth-viewport-dispatch",
        "root": "csb",
        "file": "Mouse.cpp",
        "range": "1771-1839",
        "needles": [
            "if ((D6W>=28)&&(D6W<66))",
            "HandleClothingClick(D6W-20);",
            "if ((D6W>=7) && (D6W<=11))",
            "ShowHideInventory(D7W);",
            "if (D6W == 83)",
            "ShowHideInventory(d.HandChar);",
            "if (D6W == 70)",
            "FeedCharacter(_10_);",
            "if (D6W == 71)",
            "ClickOnEye(_4_);",
            "if (D6W == 80) // The Viewport while adventuring",
            "if (d.ClockRunning != 0) HandleClickInViewport(clickX, clickY);",
        ],
        "why": "CSB src preserves the same inventory/mouth/eye/viewport dispatch partition as the CSBWin mirror.",
    },
]


def line_slice(text: str, line_range: str) -> str:
    start, end = [int(part) for part in line_range.split("-")]
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
        check_id = check["id"]
        line_range = check["range"]
        why = check["why"]
        path = roots[check["root"]] / check["file"]
        if not path.exists():
            failures.append(f"FAIL {check_id}: missing {path}")
            continue
        text = path.read_text(errors="replace")
        haystack = line_slice(text, line_range)
        missing = [needle for needle in check["needles"] if needle not in haystack]
        if missing:
            failures.append(
                f"FAIL {check_id}: {path}:{line_range} missing "
                + "; ".join(repr(m) for m in missing)
            )
        else:
            passes.append(
                f"PASS {check_id}: {path.name}:{line_range} - {why}"
            )

    print("[csb v1 viewport/inventory mouse source lock]")
    for line in passes:
        print(line)
    for line in failures:
        print(line)
    if failures:
        return 1
    print(f"PASS {len(passes)} CSB V1 viewport/inventory mouse source-lock checks")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
