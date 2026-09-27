# Dungeon Master Nexus — champion status

## Source-bound roster

The real European `RLOWFIX.BIN` file contains a `RES*` directory and a `PLRD`
resource with 20 records of 64 bytes each. Firestaff reads the numeric fields,
class/level fields, portrait ordinals, and equipment words actually stored in
PLRD. `FACE.BIN` is bound separately to 20 real portrait records.

The authenticated European retail `RLOWFIX.BIN` (SHA-256
`f2686bf3d6b971c5eaa613b2619b7e1bb7958a8045876296a83509f137be18b0`) stores
only the values `0`, `1`, and `2` in PLRD byte 23 across its 20 rows. The
authenticated `FACE.BIN` (SHA-256
`d733f50096098b5a2d15f2d355a89decd7b3777f82e515f60fee2e9ca4921e22`) contains
20 indexed records. Byte 23 is therefore retained as `portrait_type`, not
treated as an index into FACE.BIN. It does not establish which portrait
belongs to each champion row. Keep `portrait_index` unknown until a Saturn
consumer trace proves that join.

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
