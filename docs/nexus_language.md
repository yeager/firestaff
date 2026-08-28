# Dungeon Master Nexus — language and text status

This document is a source-faithfully corrected status report. The earlier
version claimed Japanese-only content, Shift-JIS names, and completed text
rendering without proof from the local corpus.

## Verified in the local corpus

- The European English ISO contains `DMN_ABS.TXT`, `DMN_BIB.TXT`, and
  `DMN_CPY.TXT`; deras bytesidentitet verifieras av
  `scripts/verify_nexus_v1_asset_manifest.py`.
- `RLOWFIX.BIN` contains a verified `PLRD` resource with 20 records of 64
  bytes each. Each record retains six `TABL` indices and the raw glyph codes in
  `TABL`.
- `FONT256.S2D` provides 242 verified 8×8-byte CG tiles from real SCR regions.

## Not verified

The `TABL` codes are not yet bound to a Saturn font, a page, an attribute table,
or a VDP2 text consumer. Firestaff must therefore not convert them to guessed
ASCII, JIS, or Unicode names. `name_ascii` and legacy language/roster fields
may occur only in isolated fixture tests.

Nor can it be concluded that all game text is Japanese or English from a
single ISO metadata file. Language status must be bound per source, revision,
and text consumer.

## Sources and implementation

- DMWeb's file-format and data-file decoders are used for `PLRD`/`TABL`,
  `FONT256`, and the SCR regions.
- [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md) is
  the consolidated status.
- `src/nexus/nexus_v1_rlowfix_text.c` retains raw TEXT/TABL bytes.
- `src/nexus/nexus_v1_saturn_font.c` retains real CG tiles but marks glyph
  mapping as not ready.
- `src/nexus/nexus_v1_screen_text.c` rejects real SCR text until the mapping is
  source-bound. The generic indexed-text function is test/fixture material only
  and is not a Nexus text consumer.

No host text may replace a missing or unproven Saturn text surface.
