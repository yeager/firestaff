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
| Startup | TITLE/MAPD, LOGOBG, WARNING/GAMEOVER/STABG, FACE, and FONT bytes | Saturn VDP2 layers, timing, text mapping, and destination |
| Menu | MENU.BPK/BPPK, 162 PRS3 surfaces, and pixel decoding | CLUT, VDP1 upload, placement, and menu sequence |
| HUD | STABG tiles/palette and DM.BIN hit rectangles | input/VDP consumer and runtime-state binding |
| Viewport | DGN Structure1B/2 and MNS/DMDF census | Structure3 face owner, mesh transform, VDP1 command/texture use |
| Runtime | SLEV/SAL/MAP bounded receipts | event/action dispatch and sound selector |

## Sources

- DMWeb Nexus file-format pages and DMN Data File Decoder.
- Greatstone for DM-format comparisons and byte-format references.
- [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md).
- [NEXUS_RUNTIME_CAPTURE.md](NEXUS_RUNTIME_CAPTURE.md).

All unproven routes are fail-closed. No synthetic title, roster, HUD, viewport
texture, sound sample, or gameplay semantic may conceal missing Saturn
evidence.
