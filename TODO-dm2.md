# Firestaff TODO — DM2

Reviewed 2026-08-25. Only open work is listed here.

- Complete source-owned record-pool, relocation and `SKSAVE` ownership using
  authentic saves; do not promote reduced state layouts as retail parity.
- Extend real-media gameplay evidence across DOS, Amiga, FM Towns and Mac for
  dialog/input ordering, creature AI/drop routes, audio and save/resume.
- Complete the Amiga `GRAPHICS.DAT` map-to-`GRAPHICSSET` viewport binding.
  The authentic installer passes boot, GAME_LOAD and movement, and binds the
  original GDAT plus DUNGEON.DAT, but its live first viewport frame currently
  has zero source-material blits. The atomic M11 frame gate correctly rejects
  it; do not weaken that gate or substitute PC/DOS artwork.
- Bind renderer/HUD V2.2 material, clipping and outdoor routes to original
  GDAT/capture evidence; synthetic V2.2 art is allowed only as a fixture.
