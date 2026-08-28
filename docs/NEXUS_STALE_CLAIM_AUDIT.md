# Nexus – audit of older claims

This is a documentation inventory, not a new runtime specification. The
current status is in
[`NEXUS_STRICT_FIDELITY_INVENTORY.md`](NEXUS_STRICT_FIDELITY_INVENTORY.md).
Older documents must not be used as proof that a Nexus route is implemented or
source-bound.

## Claims that are blocked

| Document family | Older claim | Correct status from the retail corpus |
|---|---|---|
| `nexus_content.md`, `nexus_overview.md` | DM1-derived roster, stats, and combat are verified Nexus runtime | PLRD has 20 real records; name, stats, and action consumers are unbound |
| `nexus_champions.md`, `nexus_creatures.md` | Hard-coded roster, creature stats, and portrait indexes are the game source | Hard-coded tables are fixture-/diagnostic-only; PLRD/FACE are source receipts |
| `nexus_data.md`, `nexus_graphics.md` | DGN/MNS geometry and textures can be drawn by the host rasterizer | Structure1B/2/3 and MNS are format proof; transform, palette, VDP1, and command order are missing |
| `nexus_intro.md`, `nexus_title.md` | The host rasterizer shows the original title | TITLE/LOGOBG/WARNING are decoded surfaces; VDP2 layers, timing, and placement are missing |
| `nexus_hud.md`, `nexus_inventory.md` | Generic HUD-/inventory widgets are Nexus presentation | STABG/ITEM/FACE data are receipts; Saturn's HUD and action consumer are not captured |
| `nexus_text.md`, `nexus_language.md` | FONT256/SJIS conversion is sufficient for visible text | RLOWFIX TEXT4/TABL and FONT012 are byte-bound; glyph consumer/page/attribute/VDP2 are missing |
| `nexus_audio_format.md`, `nexus_music.md`, `nexus_sound.md`, `nexus_sfx.md` | CD-DA/SAL/MAP can be played through host assumptions, or level pairs automatically choose tracks | CDDA tracks 2–9 and SAL/MAP are metadata; level→CDDA, SLEV/SDDRVS event→selector, and playback are capture-gated |
| `nexus_input.md`, `nexus_sensors.md`, `nexus_squares.md` | DM1-like input, sensor, and square semantics apply to Nexus | Only bounded source records are admitted; Saturn dispatch and state writes are unproven |
| `nexus_dungeon.md`, `nexus_content.md`, `nexus_math.md` | Nexus uses a 32×32 grid, or DGN geometry is entirely unparsed | Retail DGN has 64×64 Structure1B cells; Structure1B/2/3 intake exists, while transform/material/VDP1 consumer remains capture-gated |
| `nexus_menus.md`, `nexus_graphics.md` | Animated Saturn title, options/in-game menu, or the host polygon route is retail parity | Host state/input and bounded geometry exist as receipts; title/menu composition, VDP1/VDP2 ownership, and menu sequence remain unbound |
| `nexus_phase2_data_formats_H2321.md`, `nexus_test_coverage.md` | Historical test/stub matrices describe current production | Older plan/coverage documents do not override current CMake exclusion and no-draw gates |

## Production rule

No text, palette, portrait, HUD widget, item, mesh, viewport pixel, or sound
event may leave the diagnostic/receipt layer merely because an older document
calls it “implemented”. Promotion requires source-owned byte identity and an
authenticated Saturn/Mednafen capture of consumer, destination, and timing.
A missing capture means blank/no-draw or no-op, not a synthetic replacement.

DMWeb and Greatstone are used for byte and format rules. They do not by
themselves prove VDP1/VDP2 composition or runtime semantics.

See also `TODO.md`: `NEXUS-SATURN-PRESENTATION-HANDOFF`,
`NEXUS-MENU-SEQUENCE-CAPTURE` and `NEXUS-HUD-SATURN-DISPATCH-CAPTURE`.
