#!/usr/bin/env python3
# ------------------------------------------------------------------
# Pass 47b — ZONES layout recovery for DM1 PC 3.4 English (I34E)
# ------------------------------------------------------------------
# Extracts the LAYOUT_RANGE / LAYOUT_RECORD blob that is loaded at
# runtime by ReDMCSB F0640_LoadLayoutData() from GRAPHICS.DAT entry
# 696 (C696_GRAPHIC_LAYOUT, DEFS.H:2265).
#
# Provenance
# ----------
# * GRAPHICS.DAT SHA256:
#     2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
#   (recorded in extracted-graphics-v1/manifest.json).
# * Blob magic 0xFC0D at absolute offset 0x47a6f inside GRAPHICS.DAT.
#   This is the only coherent 0xFC0D occurrence in GRAPHICS.DAT.
# * Parser format documented in
#   ReDMCSB_WIP20210206 Toolchains/Common/Source/COORD.C
#   function F0639_LoadLayoutRanges (l. 2498) and
#   F0635_/F0637_GetProportionalZone (l. 2054+) which resolve zones.
# * LAYOUT_RECORD is four i16 words: (RecordType u16, ParentIndex i16,
#   Data1 i16, Data2 i16) -- 8 bytes per record.
# * LAYOUT_RANGE header is (NextRange ptr placeholder u16, FirstIndex
#   u16, LastIndex u16). On disk the NextRange ptr is absent; each
#   range on disk is just (FirstIndex, LastIndex, Records[]).
#   The magic 0xFC0D + range_count u16 + (first,last)*count prefix
#   appears once for the whole item; F0639_LoadLayoutRanges allocates
#   LAYOUT_RANGE nodes at runtime. See F0639 l. 2498-2535.
#
# Output
# ------
# 1. zones_h_reconstruction.json  -- machine-readable table of all
#    1133 records, preserving range boundaries.
# 2. Verification: record 101 (C101_ZONE_PANEL) must be
#    type=0, parent=100, d1=152, d2=89.
#
# This tool is pure-read. It writes one JSON file and prints a summary.
# Run:
#     python3 tools/extract_zones_layout_696.py \
#         ~/.openclaw/data/redmcsb-original/GRAPHICS.DAT \
#         zones_h_reconstruction.json
# ------------------------------------------------------------------
import hashlib
import json
import os
import struct
import sys


GRAPHICS_DAT_SHA256_EXPECTED = (
    "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"
)
BLOB_MAGIC = 0xFC0D
BLOB_ABSOLUTE_OFFSET = 0x47a6f  # confirmed unique 0xFC0D run in-file


def main(argv):
    src = argv[1] if len(argv) > 1 else os.path.expanduser(
        "~/.openclaw/data/redmcsb-original/GRAPHICS.DAT")
    dst = argv[2] if len(argv) > 2 else "zones_h_reconstruction.json"

    with open(src, "rb") as fh:
        raw = fh.read()
    digest = hashlib.sha256(raw).hexdigest()
    if digest != GRAPHICS_DAT_SHA256_EXPECTED:
        sys.stderr.write(
            "FATAL: GRAPHICS.DAT SHA256 mismatch.\n"
            f"  got     {digest}\n"
            f"  expect  {GRAPHICS_DAT_SHA256_EXPECTED}\n")
        return 2

    blob = raw[BLOB_ABSOLUTE_OFFSET:]
    magic, range_count = struct.unpack("<HH", blob[:4])
    if magic != BLOB_MAGIC:
        sys.stderr.write(f"FATAL: wrong magic {magic:#x}\n")
        return 2

    ranges = []
    for i in range(range_count):
        first, last = struct.unpack("<HH", blob[4 + i * 4:4 + i * 4 + 4])
        ranges.append({"first_index": first, "last_index": last})

    rp = 4 + range_count * 4
    records = {}
    for r in ranges:
        f = r["first_index"]
        l = r["last_index"]
        for idx in range(f, l + 1):
            rtype, parent, d1, d2 = struct.unpack("<Hhhh", blob[rp:rp + 8])
            records[idx] = {
                "type": rtype,
                "parent": parent,
                "d1": d1,
                "d2": d2,
            }
            rp += 8
    blob_end = rp
    blob_byte_count = blob_end

    # Sanity check against documented DEFS.H zones (hard-locked).
    must_match = {
        # (zone_index, expected_type, expected_parent, expected_d1, expected_d2)
        1: (9, 0, 320, 200),            # Screen dimensions
        3: (9, 0, 224, 136),            # Viewport bitmap dim (C000_DERIVED)
        7: (1, 3, 0, 33),               # ZONE_VIEWPORT positioned at y=33
        100: (9, 4, 144, 73),           # Panel bitmap 144x73 (C020_GRAPHIC_PANEL_EMPTY)
        101: (0, 100, 152, 89),         # C101_ZONE_PANEL center anchor at (152,89)
        150: (9, 0, 67, 29),            # Status-box frame bitmap
        151: (1, 150, 0, 0),            # C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS
        152: (1, 150, 69, 0),           # C152 champion 1 at +69
        153: (1, 150, 138, 0),          # C153 champion 2 at +138
        154: (1, 150, 207, 0),          # C154 champion 3 at +207
        187: (1, 183, 43, 0),           # C187_ZONE_CHAMPION_0_STATUS_BOX_BAR_GRAPHS
    }
    failures = []
    for idx, expect in must_match.items():
        if idx not in records:
            failures.append(f"zone {idx} missing")
            continue
        r = records[idx]
        got = (r["type"], r["parent"], r["d1"], r["d2"])
        if got != expect:
            failures.append(f"zone {idx} {got} != {expect}")
    if failures:
        sys.stderr.write("FATAL: zones layout sanity failed:\n  "
                         + "\n  ".join(failures) + "\n")
        return 3

    out = {
        "$provenance": {
            "source_file": "GRAPHICS.DAT (DM1 PC 3.4 English / I34E)",
            "source_sha256": GRAPHICS_DAT_SHA256_EXPECTED,
            "graphics_dat_entry_index": 696,
            "graphics_dat_entry_constant": "C696_GRAPHIC_LAYOUT",
            "blob_magic": "0xFC0D",
            "blob_absolute_offset": BLOB_ABSOLUTE_OFFSET,
            "blob_byte_count": blob_byte_count,
            "format_reference":
                "ReDMCSB_WIP20210206 Toolchains/Common/Source/COORD.C "
                "F0639_LoadLayoutRanges / F0635_ / F0637_GetProportionalZone",
            "defs_h_reference":
                "ReDMCSB_WIP20210206 Toolchains/Common/Source/DEFS.H "
                "zone constants C002..C701 (range 3748..4041)",
            "record_struct":
                "typedef struct { uint16 RecordType; int16 ParentRecordIndex;"
                " int16 Data1; int16 Data2; } LAYOUT_RECORD;  // 8 bytes",
            "note":
                "This is a verbatim dump of the original runtime layout "
                "table. It is NOT invented. Field meanings, record types, "
                "and resolver algorithm are documented by the cited "
                "ReDMCSB source files. Media variant targeted: I34E (DOS "
                "PC 3.4 English).",
        },
        "range_count": range_count,
        "ranges": ranges,
        "record_count": len(records),
        "records": {str(k): records[k] for k in sorted(records)},
    }
    with open(dst, "w") as fh:
        json.dump(out, fh, indent=2, sort_keys=False)
        fh.write("\n")
    print(f"OK -- {len(records)} records across {range_count} ranges "
          f"({blob_byte_count} bytes) -> {dst}")
    print("Sanity zones verified:", ", ".join(str(k) for k in must_match))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
