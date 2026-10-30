# Dungeon Master Nexus — overview with evidence boundaries

Dungeon Master Nexus is a Sega Saturn game with real DGN levels, DMDF/MNS
models, PRS3/UI media, Saturn font data, per-level SLEV/SAL/MAP files, and CD
media. Firestaff's Nexus work is a source-bound bring-up; it has not been
proven that a host rasterizer or DM1-derived gameplay model has parity with the
original SH-2/VDP1/VDP2 runtime.

## Verified local corpus

The user-provided European English ISO and the loose files in
`.firestaff/data/nexus` are hash-verified. The corpus includes 137 manifest
members: 131 loose files and six authenticated ISO members. The mixed runtime
reader uses verified loose files first and the ISO only as a supplementary
source for missing exact members.

## Source status

| Area | Verified | Remaining |
|---|---|---|
| Startup | TITLE/MAPD, LOGOBG, WARNING/GAMEOVER/STABG, FACE and FONT bytes are admitted from retail data. Japanese title-session traces bind the `TITLE.CG` upload, MAPD palette and NBG0 bitmap producer. Japanese and English Ymir cold boots reach the native menu; the English screenshot replay is pixel-exact with 126 informative retail-byte matches among 137 textured draws. A Japanese cold-boot prompt capture binds one 921-byte NBG0 span to `TITLE.BIN` TITL record 2 and command 171 to DGT2/PP record 21; the selected-layer reconstruction matches all 286,720 screenshot samples within eight levels. | The full title bitmap, other VDP1 commands, timing, text mapping, complete title composition and title-to-menu transition remain unbound; emulator evidence does not make Nexus playable through Firestaff. |
| Menu | `MENU.BPK` decodes all 162 real PRS3 surfaces. A separate authentic Japanese menu frame binds five visible VDP1 marker/label commands (171–175) to unique `TITLE.BIN` DGT2/PP records and matching CRAM palettes. Captured VDP1 FBS + NBG0 + those five commands match all 286,720 doubled screenshot samples within eight levels; the C capture-only compositor exactly matches the independent native RGB reference. | The five draws are not evidence that `MENU.BPK` supplies them: these selected sources are in `TITLE.BIN`. The complete NBG0 bitmap, menu transitions, input hitboxes and Firestaff production rendering remain gated. |
| HUD | STABG tiles/palette and DM.BIN hit rectangles | input/VDP consumer and runtime-state binding |
| Viewport | DGN Structure1B/2 and MNS/DMDF census | Structure3 face owner, mesh transform, VDP1 command/texture use |
| Runtime | SLEV/SAL/MAP bounded receipts | event/action dispatch and sound selector |

## Sources

- [DMWeb Nexus file-format pages](http://dmweb.free.fr/community/documentation/dungeon-master-nexus/file-formats/) and the [DMN Data File Decoder](http://dmweb.free.fr/community/tools/dungeon-master-nexus-data-file-decoder/) document game-file formats and decoder behavior.
- [Greatstone Swoosh Construction Kit](http://greatstone.free.fr/dm/) is a cross-version extraction and format reference for authentic DM-family assets; use it for asset/format cross-checks, not as evidence of Saturn VDP register behavior or runtime presentation.
- The reviewed page inventory and scope notes are in [DMWeb and Greatstone Reference](../DMWEB_REFERENCE.md).
- [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md).
- [NEXUS_RUNTIME_CAPTURE.md](NEXUS_RUNTIME_CAPTURE.md).

All unproven routes are fail-closed. No synthetic title, roster, HUD, viewport
texture, sound sample, or gameplay semantic may conceal missing Saturn
evidence.
