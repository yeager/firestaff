# Dungeon Master Nexus — startup status

This is an evidence report for startup, not a specification for a completed
Nexus startup screen.

## Verified initialization order

`nexus_v1_init()` first selects a hash-verified extracted corpus when `DM.BIN`
and the authentic DGN corpus are available as real loose files. `LEV00.DGN` is
a title/entry resource; the first playable level is `LEV01.DGN`. A valid
co-located retail ISO can then be read as a supplementary source for exact ISO
members. If only ISO data is available, the ISO reader is used directly.

After source selection, the engine attempts to read real `RLOWFIX.BIN`, startup
surfaces, `FACE.BIN`, `FONT256.S2D`, sound metadata, and other source files.
Missing or unbound consumers must not be replaced with synthetic surfaces.
Title startup must therefore not report a game position until Saturn's authentic
`LEV01` position and direction have been captured and bound to the same runtime
source.

## Verified startup resources

| Resource | Status |
|---|---|
| `TITLE.CG` + `TITLE.BIN` | MAPD/TIBG decoding and five retail images verified for the canonical profile and documented English revision; VDP2 placement is missing |
| `LOGOBG.DG2` | PP pixels and a 256-entry BGR555 palette are verified; layer/timing is missing |
| `WARNING.BIN`, `GAMEOVER.BIN`, `STABG.BIN` | real surfaces are decoded and receive byte provenance; presentation is missing |
| `FACE.BIN` | 20 real 56×56 PRS3 portraits with source palettes; VDP1 destination is missing |
| `FONT256.S2D` | 242 CG tiles are retained in production source; Saturn page/attribute/glyph mapping is missing |
| `MENU.BPK` | 162 PRS3 surfaces decode to source-bound indexed bytes; menu ordering, CLUT, and VDP1/VDP2 are missing |
| Mednafen visual baseline | A clean J-BIOS capture against authentic retail shows black boot followed by the original intro FMV without input; menu/LEV01 identity remains missing |

## Runtime boundary

`firestaff_nexus` must not present a host-built title, champion menu,
portrait placement or a text footer as though it were Saturn output. M11 handoff
and viewport remain fail-closed until an authenticated Saturn capture binds
resource, palette, destination, and timing.

FONT256's authenticated Character Generator bytes now remain in the engine's
source object for the future Saturn consumer. This does not admit glyph coding,
page/PND attributes, layer placement, or text writing; `font_loaded` remains
closed until those parts are bound by the same runtime witness.

## Sources

- DMWeb Nexus file formats and DMN Data File Decoder.
- `src/nexus/nexus_v1_engine.c` and `src/nexus/nexus_v1_title.c`.
- `docs/NEXUS_RUNTIME_CAPTURE.md`.
- [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md).
