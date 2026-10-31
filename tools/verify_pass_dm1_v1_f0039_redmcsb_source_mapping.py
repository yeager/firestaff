#!/usr/bin/env python3
"""Check the bounded DM1 PC34 F0039 mapping against ReDMCSB source."""
from __future__ import annotations

import csv
from pathlib import Path

from redmcsb_source import source_root


ROOT = Path(__file__).resolve().parents[1]
SOURCE = source_root(("OBJECT.C",))
OBJECT = SOURCE / "OBJECT.C"
IMPLEMENTATION = ROOT / "src/dm1/dm1_v1_object_slotbox_icon_f0039_pc34_compat.c"
SLOT_BOXES = ROOT / "src/dm1/dm1_v1_slot_boxes_pc34_compat.c"
TEST = ROOT / "tests/test_dm1_v1_object_slotbox_icon_f0039_pc34_compat.c"
AUDIT = ROOT / "docs/reference/audits/REDMCSB_CALLABLE_SYMBOL_FULL_AUDIT.tsv"


def main() -> int:
    lines = OBJECT.read_text(errors="replace").splitlines()
    anchor = "int16_t F0039_OBJECT_GetIconIndexInSlotBox("
    try:
        start = next(i for i, line in enumerate(lines) if anchor in line)
    except StopIteration:
        raise SystemExit("FAIL ReDMCSB OBJECT.C F0039 definition missing")
    source_window = "\n".join(lines[start:start + 14])
    required = "return G0030_as_Graphic562_SlotBoxes[P0049_ui_SlotBoxIndex].IconIndex;"
    if required not in source_window:
        raise SystemExit("FAIL ReDMCSB F0039 indexed IconIndex return changed")

    implementation = IMPLEMENTATION.read_text()
    slot_boxes = SLOT_BOXES.read_text()
    test = TEST.read_text()
    if "dm1_v1_slot_boxes_get_icon_index_pc34(slotBoxIndex)" not in implementation:
        raise SystemExit("FAIL Firestaff F0039 adapter no longer delegates to G0030 owner")
    if "return s_g0030[slot_box_index].iconIndex;" not in slot_boxes:
        raise SystemExit("FAIL Firestaff G0030 accessor no longer returns IconIndex")
    if "dm1_v1_object_get_icon_index_in_slotbox_f0039_pc34" not in test:
        raise SystemExit("FAIL focused F0039 adapter regression missing")

    with AUDIT.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle, delimiter="\t"))
    matches = [row for row in rows if row["symbol"] == "F0039_OBJECT_GetIconIndexInSlotBox"]
    if len(matches) != 1:
        raise SystemExit("FAIL F0039 audit inventory row missing or duplicated")
    row = matches[0]
    if row["status"] != "IMPLEMENTED_NARROW" or "PC34" not in row["notes"] or "other edition/platform runtime parity remain open" not in row["notes"]:
        raise SystemExit("FAIL F0039 disposition overstates its PC34 scope")

    print("PASS F0039 ReDMCSB source mapping (PC34 scope)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
