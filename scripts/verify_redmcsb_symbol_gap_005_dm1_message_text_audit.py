#!/usr/bin/env python3
"""Verify the source-proven DM1 message/text ownership audit batch."""

from __future__ import annotations

import csv
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
AUDIT = ROOT / "docs/reference/audits/REDMCSB_CONSTANT_GLOBAL_FULL_AUDIT.tsv"
PROVEN = {
    "G0355_B_ScrollMessageArea": ("scrollPending", "int", "Lifetime"),
    "G0356_puc_Bitmap_MessageAreaNewRow": (
        "message_area_new_row_bitmap",
        "const uint8_t *",
        "one update call",
    ),
    "G0358_i_MessageAreaCursorRow": ("cursorRow", "int", "Lifetime"),
    "G0359_i_MessageAreaCursorColumn": ("cursorColumn", "int", "Lifetime"),
    "G0360_al_MessageAreaRowExpirationTime": (
        "expirationTime",
        "four long",
        "Lifetime",
    ),
}
UNRESOLVED = {
    "G0353_ac_StringBuildBuffer": "semantic candidate (unverified)",
    "G0354_i_MessageAreaScrollingLineCount": "unmapped / no direct counterpart",
    "G0357_puc_InterfaceAndScrollsFont": "unmapped / no direct counterpart",
}
OUT_OF_SCOPE = ("C127", "C026", "C040", "F1172", "F1173", "F1174", "F1175", "F1176", "M11")


def main() -> int:
    with AUDIT.open(newline="", encoding="utf-8") as source:
        rows = {row["symbol"]: row for row in csv.DictReader(source, delimiter="\t")}

    for symbol, tokens in PROVEN.items():
        row = rows.get(symbol)
        if row is None:
            raise SystemExit(f"{symbol}: missing audit row")
        if row["firestaff_status"] != "storage ownership audited; behavioral parity unverified":
            raise SystemExit(f"{symbol}: unexpected audit status")
        text = f"{row['firestaff_mapping']} {row['evidence']}"
        if "full audit inventory" not in row["evidence"]:
            raise SystemExit(f"{symbol}: source inventory evidence is missing")
        normalized_text = text.lower()
        for token in tokens:
            if token.lower() not in normalized_text:
                raise SystemExit(f"{symbol}: missing evidence token {token!r}")
        for token in OUT_OF_SCOPE:
            if token in text:
                raise SystemExit(f"{symbol}: out-of-scope token {token!r}")

    for symbol, expected_status in UNRESOLVED.items():
        row = rows.get(symbol)
        if row is None or row["firestaff_status"] != expected_status:
            raise SystemExit(f"{symbol}: uncertain symbol must retain its prior disposition")

    print(f"verified {len(PROVEN)} REDMCSB-SYMBOL-GAP-005 DM1 message/text rows")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
