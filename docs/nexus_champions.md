# Dungeon Master Nexus — champion status

## Source-bound roster

The staged Japanese retail `RLOWFIX.BIN` file contains a `RES*` directory and
a `PLRD` resource with 20 records of 64 bytes each. Firestaff reads the
numeric fields, class/level fields, raw portrait-type byte, and equipment
words actually stored in PLRD. `FACE.BIN` is bound separately to 20 real
portrait records.

The authenticated Japanese 72,332-byte retail `RLOWFIX.BIN` revision (SHA-256
`f2686bf3d6b971c5eaa613b2619b7e1bb7958a8045876296a83509f137be18b0`) stores
only the values `0`, `1`, and `2` in PLRD byte 23 across its 20 rows. The
authenticated `FACE.BIN` (SHA-256
`d733f50096098b5a2d15f2d355a89decd7b3777f82e515f60fee2e9ca4921e22`) contains
20 indexed records. Byte 23 is therefore retained as `portrait_type`, not
treated as an index into FACE.BIN. It does not establish which portrait
belongs to each champion row. Keep `portrait_index` unknown until a Saturn
consumer trace proves that join.

Each PLRD row is 64 bytes, with a 40-byte tail after offset 24. The current
four-byte-stride equipment reads are therefore bounded to ten values; the
eleventh slot and any separate backpack offsets are not established by that
row layout and remain empty. Do not read across the next PLRD row or the
following `CRET` resource to populate runtime inventory.

The Japanese, English and French Track 1 `RLOWFIX.BIN` revisions are
recognized by exact MD5 identities, with Japanese and English CUE regressions
checking their source receipts and RES* envelopes. English and French
`TITLE.BIN` and the French `GAMEOVER.BIN` identities are also recorded. This
authenticates file identity and container structure only; it does not promote
regional title/menu rendering, text interpretation, or any PLRD-to-FACE
ordinal mapping.

The separately hash-verified English Saturn ISO revision
`e5cce2db884320541f91c22c1ec1ffac6efea30b2b7c3c206a442980f241a833`
(74,980 bytes) also decodes as a 14-entry RLOWFIX resource with 20 PLRD rows.
Its first champion's TABL index is `0x21` and its five stored codes are
`00c1 00cc 00c5 00d8 0005`; the trailing `0005` is not emitted as a name
glyph, so the retained row has four glyph codes. Its TEXT#0 table has 450
strings rather than 449. The real-media test now keys these structural
expectations by the exact European or English file hash; it preserves the raw
regional codes and does not translate or present them. This does not change
the unresolved Saturn text consumer or PLRD-to-FACE ordinal join.

The earlier eight-character table and the hard-coded 24-record roster list are
not Nexus source. They may only be used by explicit legacy fixture tests.
Twenty-four is storage capacity, not the verified number of retail champions.

## Names and text

PLRD points to `TABL` records. Firestaff retains both the indices and raw
16-bit glyph codes, but no Saturn TEXT/FONT256 consumer has yet demonstrated
how they become visible text. Production therefore does not publish names as
ASCII, Shift-JIS, katakana, or Swedish translations.

## What must not be inherited from DM1/DM2

DM1/DM2 sources must not fill in Nexus statistics, class semantics, combat,
spell costs, XP, drops, item use, food/water, alignment, or resurrection.
Such routes are blocked or fixture-isolated only until a Nexus disassembly or
capture binds them. A PLRD byte must not be assigned a DM1 meaning by itself.

## Sources

- DMWeb's Nexus file formats and `DMNDataFileDecoder.vbs` structures.
- `src/nexus/nexus_v1_champions.c` and
  `src/nexus/nexus_v1_rlowfix_text.c`.
- `tests/test_nexus_v1_champion_plrd.c` against the real RLOWFIX corpus.
- [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md).
