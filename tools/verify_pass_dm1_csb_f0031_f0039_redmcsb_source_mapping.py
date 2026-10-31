#!/usr/bin/env python3
"""Check the bounded DM1 PC34 F0039 mapping against ReDMCSB source."""
from __future__ import annotations

import csv
import re
from pathlib import Path

from redmcsb_source import source_root


ROOT = Path(__file__).resolve().parents[1]
SOURCE = source_root(("OBJECT.C",))
OBJECT = SOURCE / "OBJECT.C"
IMPLEMENTATION = ROOT / "src/dm1/dm1_v1_object_slotbox_icon_f0039_pc34_compat.c"
SLOT_BOXES = ROOT / "src/dm1/dm1_v1_slot_boxes_pc34_compat.c"
TEST = ROOT / "tests/test_dm1_v1_object_slotbox_icon_f0039_pc34_compat.c"
M11 = ROOT / "src/engine/m11_game_view.c"
CSB_RUNTIME = ROOT / "src/csb/csb_v1_runtime_pc34_compat.c"
CSB_BOOT = ROOT / "src/csb/csb_v1_boot.c"
AUDIT = ROOT / "docs/reference/audits/REDMCSB_CALLABLE_SYMBOL_FULL_AUDIT.tsv"


def c_function_body(text: str, name: str) -> str:
    """Return a C function body while ignoring braces inside comments and literals."""
    match = re.search(rf"\\b{re.escape(name)}\\s*\\(", text)
    if not match:
        raise SystemExit(f"FAIL C function missing: {name}")
    opening = text.find("{", match.end())
    semicolon = text.find(";", match.end())
    if opening < 0 or (semicolon >= 0 and semicolon < opening):
        raise SystemExit(f"FAIL C function definition missing: {name}")

    depth = 1
    index = opening + 1
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if char == "/" and next_char == "/":
            newline = text.find("\\n", index + 2)
            index = len(text) if newline < 0 else newline + 1
            continue
        if char == "/" and next_char == "*":
            end = text.find("*/", index + 2)
            if end < 0:
                raise SystemExit(f"FAIL unterminated C comment in {name}")
            index = end + 2
            continue
        if char in ("\\\"", "'"):
            quote = char
            index += 1
            while index < len(text):
                if text[index] == "\\\\":
                    index += 2
                    continue
                if text[index] == quote:
                    index += 1
                    break
                index += 1
            continue
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1:index]
        index += 1
    raise SystemExit(f"FAIL unterminated C function body: {name}")


def main() -> int:
    lines = OBJECT.read_text(encoding="utf-8", errors="replace").splitlines()
    anchor = "int16_t F0039_OBJECT_GetIconIndexInSlotBox("
    try:
        start = next(i for i, line in enumerate(lines) if anchor in line)
    except StopIteration:
        raise SystemExit("FAIL ReDMCSB OBJECT.C F0039 definition missing")
    source_window = "\n".join(lines[start:start + 14])
    required = "return G0030_as_Graphic562_SlotBoxes[P0049_ui_SlotBoxIndex].IconIndex;"
    if required not in source_window:
        raise SystemExit("FAIL ReDMCSB F0039 indexed IconIndex return changed")

    implementation = IMPLEMENTATION.read_text(encoding="utf-8")
    slot_boxes = SLOT_BOXES.read_text(encoding="utf-8")
    test = TEST.read_text(encoding="utf-8")
    f0039_body = c_function_body(implementation, "F0039_OBJECT_GetIconIndexInSlotBox")
    if not re.fullmatch(
        r"\\s*return\\s+dm1_v1_slot_boxes_get_icon_index_pc34"
        r"\\s*\\(\\s*slotBoxIndex\\s*\\)\\s*;\\s*",
        f0039_body,
    ):
        raise SystemExit("FAIL Firestaff F0039 adapter return expression changed")
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

    # F0031 has three source encodings in the supported M11 lanes: MEDIA060
    # high-bit termination, MEDIA574 NUL termination, and MEDIA689's byte-1
    # display terminator inside NUL-terminated Japanese rows.
    f0031 = "\n".join(lines[24:119])
    for marker in (
        "void F0031_OBJECT_LoadNames(",
        "#ifdef MEDIA689_F31J_X31J_P31J",
        "if (*L0001_puc_ObjectNames == 1)",
        "#ifdef MEDIA060_",
        "while (!(*L0002_puc_Graphic & 0x80))",
        "#ifdef MEDIA574_",
        "while (*L0001_puc_ObjectNames++)",
    ):
        if marker not in f0031:
            raise SystemExit(f"FAIL ReDMCSB F0031 source behavior changed: {marker}")
    m11 = M11.read_text(encoding="utf-8")
    csb_runtime = CSB_RUNTIME.read_text(encoding="utf-8")
    csb_boot = CSB_BOOT.read_text(encoding="utf-8")
    for marker in (
        "static int m11_dm1_load_object_names_m564(",
        "M11_DM1_OBJECT_NAME_COUNT = 199",
        "highBitTerminated",
        "state->dm1ObjectNames[nameIndex][--written] = '\\0'",
    ):
        if marker not in m11:
            raise SystemExit(f"FAIL DM1 F0031 live loader behavior changed: {marker}")
    for marker in (
        "csb_v1_runtime_load_object_names_m564(",
        "csb_v1_runtime_load_object_names_m564_f31j(",
        "display_ended",
    ):
        if marker not in csb_runtime:
            raise SystemExit(f"FAIL CSB F0031 runtime parser behavior changed: {marker}")
    csb_name_loader = c_function_body(csb_boot, "csb_v1_boot_load_object_names_m564")
    if ("csb_v1_runtime_load_object_names_m564_f31j(" not in csb_name_loader or
            "csb_v1_runtime_load_object_names_m564(" not in csb_name_loader or
            "CSB_V1_VARIANT_FMTOWNS_JA" not in csb_name_loader):
        raise SystemExit("FAIL CSB M564 boot loader no longer dispatches to both parsers")
    if "csb_v1_boot_load_object_names_m564(profile)" not in csb_boot:
        raise SystemExit("FAIL CSB boot no longer binds source M564 names")
    name_rows = [row for row in rows if row["symbol"] == "F0031_OBJECT_LoadNames"]
    if (len(name_rows) != 1 or name_rows[0]["status"] != "IMPLEMENTED_NARROW" or
            "src/engine/m11_game_view.c" not in name_rows[0]["firestaff_mapping"] or
            "src/csb/" not in name_rows[0]["firestaff_mapping"] or
            "IIGS" not in name_rows[0]["notes"]):
        raise SystemExit("FAIL F0031 inventory row overstates or loses scoped mappings")

    # Keep the adjacent F0619 inventory row honest: the slot-table accessor
    # does not resolve a source zone rectangle or apply edition margins.
    f0619 = "\n".join(lines[364:393])
    for marker in (
        "void F0618_LoadZone(",
        "void F0619_GetSlotBoxBorderCoordinates(",
        "F0618_LoadZone(P2055_i_ZoneIndex, P2056_pi_XYZ);",
        "F0628_AddZoneMargin(P2056_pi_XYZ",
    ):
        if marker not in f0619:
            raise SystemExit(f"FAIL ReDMCSB F0619 source behavior changed: {marker}")
    declaration = re.compile(r"\bvoid\s+F0619_GetSlotBoxBorderCoordinates\s*\(")
    if declaration.search(IMPLEMENTATION.read_text(encoding="utf-8")) or declaration.search(SLOT_BOXES.read_text(encoding="utf-8")):
        raise SystemExit("FAIL Firestaff F0619 gap must be re-audited after implementation")
    missing_rows = [row for row in rows if row["symbol"] == "F0619_GetSlotBoxBorderCoordinates"]
    if len(missing_rows) != 1 or missing_rows[0]["status"] != "MISSING" or "F0635" not in missing_rows[0]["notes"]:
        raise SystemExit("FAIL F0619 source-backed missing-function disposition changed")

    print("PASS F0031 DM1/CSB M564 source mappings; F0039 PC34 mapping; F0619 missing gap")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
