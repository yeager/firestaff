#!/usr/bin/env python3
"""Verify the source-proven DM1 save/load and timeline audit batch."""

from __future__ import annotations

import csv
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
AUDIT = ROOT / "docs/reference/audits/REDMCSB_CONSTANT_GLOBAL_FULL_AUDIT.tsv"
EXPECTED = {
    "G0369_EventMaximumCount": ("maxEvents", "int", "256"),
    "G0370_ps_Events": ("DM1_EventQueue_V1.events", "events[256]", "16 bytes"),
    "G0371_pui_Timeline": ("DM1_EventQueue_V1.timeline", "timeline[256]", "2-byte"),
    "G0372_ui_EventCount": ("eventCount", "int", "signed 32-bit"),
    "G0373_ui_FirstUnusedEventIndex": ("firstUnusedIndex", "int", "signed 32-bit"),
    "G0525_l_GameID": ("header.game_id", "uint32_t", "4 source bytes"),
    "G0526_ui_DungeonID": ("header.dungeon_id", "uint16_t", "2 source bytes"),
    "G0527_i_Platform": ("header.platform", "int16_t", "2 source bytes"),
    "G0528_i_Format": ("header.format", "int16_t", "2 source bytes"),
    "G0534_ac_SaveHeaderAdditionalData": ("SaveLoadHeaderPc34.additional", "additional[134]", "134 bytes"),
}
FORBIDDEN_HOC_TOKENS = ("C127", "C026", "C040", "HoC")


def main() -> int:
    with AUDIT.open(newline="", encoding="utf-8") as source:
        rows = list(csv.DictReader(source, delimiter="\t"))

    selected = {row["symbol"]: row for row in rows if row["symbol"] in EXPECTED}
    if set(selected) != set(EXPECTED):
        missing = sorted(set(EXPECTED) - set(selected))
        raise SystemExit(f"missing audited symbols: {', '.join(missing)}")

    for symbol, required_tokens in EXPECTED.items():
        row = selected[symbol]
        if row["firestaff_status"] != "storage ownership audited; behavioral parity unverified":
            raise SystemExit(f"{symbol}: unexpected audit status")
        text = f"{row['firestaff_mapping']} {row['evidence']}"
        if "full audit inventory" not in row["evidence"]:
            raise SystemExit(f"{symbol}: source inventory evidence is missing")
        if "lifetime" not in row["evidence"].lower():
            raise SystemExit(f"{symbol}: lifetime contract is missing")
        for token in required_tokens:
            if token not in text:
                raise SystemExit(f"{symbol}: missing evidence token {token!r}")
        for token in FORBIDDEN_HOC_TOKENS:
            if token in text:
                raise SystemExit(f"{symbol}: HoC token {token!r} is out of scope")

    print(f"verified {len(EXPECTED)} REDMCSB-SYMBOL-GAP-005 DM1 save/timeline rows")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
