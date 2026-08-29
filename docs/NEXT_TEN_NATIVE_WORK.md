# Next Ten Native Work Items

Reviewed 2026-08-29. This is the current implementation queue after the
archive/startup regression batch recorded in the per-game `DONE-*.md` files.
It intentionally ranks source-owned work above synthetic presentation work.

1. **Nexus title-state capture bundle.** Capture one retail-JP revision that
   binds `TITLE.CG`, its active `MAPD` span, CRAM palette, VDP1/VDP2 register
   state, layer order, and timing. This is the admission prerequisite for a
   native title renderer.
2. **Nexus interactive title-to-menu trace.** Capture actual Start/A input
   handling after the title state; the existing frames 13000–13039 are
   non-interactive animation and must not be promoted as menu evidence.
3. **Nexus Structure 2/VDP1 material ownership.** Bind real texture, CLUT,
   raster, clipping, animation, and composition inputs to one captured
   consumer before enabling textured 3D output.
4. **Nexus dispatcher/event/save/audio contracts.** Add each native Saturn
   runtime behavior only after its original consumer and same-revision trace
   are hash-verified.
5. **Theron JP CD-to-RAM transition trace.** Capture a real level transition
   through the consumer that reads the verified Track 02 data; do not infer
   it from the US image.
6. **Theron JP presentation ownership.** Capture bitmap, palette, text and
   audio binding needed for native surfaces; fallback visuals remain closed.
7. **Theron JP/US save and later-dungeon matrix.** Verify each region
   separately with authentic saves and level transitions.
8. **CSB authenticated DSA saves.** Add checksum-verified save corpora for
   Atari ST, Amiga, and FM Towns before extending timers and transactions.
9. **CSB original capture comparisons.** Bind title, HUD, viewport, doors and
   audio to real captures for every supported edition; CSBWin remains a
   reference, never a PC route.
10. **DM2 complete save/record ownership.** Extend the source-owned
    `SKSAVE` record pool, relocations and resume state from authentic saves,
    then bind renderer/HUD and outdoor behavior to original GDAT captures.

## Queue rules

- Do not manufacture saves, screenshots, VDP traces, palettes, or gameplay
  assets merely to advance an item.
- Development emulators and disassemblers may collect evidence, but no item
  may introduce an emulator, BIOS, firmware, extracted game-data cache, or
  other third-party runtime dependency into Firestaff.
- When an item is completed, move only its completed statement to the
  relevant `DONE-<game>.md`; leave the remaining work in `TODO-<game>.md`.
