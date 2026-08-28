# Dungeon Master Nexus — verified features and open gaps

This document distinguishes between real byte/format evidence and features
that still lack Saturn runtime binding.

## Verified data formats

- `LEV00.DGN`–`LEV15.DGN`: real DGN sources; the Structure1B census and
  Structure2 texture/palette bytes are verified in the local corpus.
- `.MNS`: 30 real DMDF containers and their TEXT/model descriptors are
  inventoried; the Structure3 face owner and VDP1 command order are unproven.
- `MENU.BPK`: 162 real PRS3 surfaces are decoded with DMWeb rules into a
  source-bound indexed-pixel receipt; CLUT, destination, and menu semantics
  are still missing.
- `STABG.BIN`, `SMAP00`–`SMAP15`, `ITEM.IBS`, `FACE.BIN`, and `STONE.BIN` have
  separate source-owned decoding and palette receipts.
- `SLEV*.BIN`, `SNDLEV*.SAL`/`.MAP`, and `SDDRVS.TSK` have bounded byte/entry
  receipts; script dispatch and sound events are not enabled.

## Features deliberately not claimed

Firestaff does not claim that its own software rasterizer is Saturn's VDP1
output, that the viewport perspective, four-square distance, creature
rendering, or VDP2 composition has parity, or that CD audio/SAL can play
correctly. Current host raster and gameplay modules are no-draw/fail-closed
where source-owned transform, pixel/palette, and runtime consumer are missing.

Nor are FMV decoding, SRAM format, controller semantics, shop actions, drops,
spells, combat, or text mapping verified merely because the corresponding files
exist.

## Priority next evidence

1. Authenticated startup/menu capture: VDP2 layers, CLUT, timing, and real
   menu sequence.
2. Authenticated VDP1 capture: DGN Structure3 faces, mesh, texture upload,
   and command coordinates.
3. Saturn input/HUD capture: `STABG.BIN` and runtime state over the viewport.
4. SLEV/SAL capture: event/action dispatch and MAP selector ownership.

See [NEXUS_STRICT_FIDELITY_INVENTORY.md](NEXUS_STRICT_FIDELITY_INVENTORY.md) and
[NEXUS_RUNTIME_CAPTURE.md](NEXUS_RUNTIME_CAPTURE.md) for the current admission
boundary. DMWeb and Greatstone are used as format references; they do not
replace an executed Saturn capture.
