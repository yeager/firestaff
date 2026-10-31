# Next Ten Native Work Items

Reviewed 2026-10-31. This is the current implementation queue after the
archive/startup regression batch recorded in the per-game `DONE-*.md` files.
It intentionally ranks source-owned work above synthetic presentation work.
Theron runtime and capture status is summarized in
[`THERON_CAPTURE_READINESS.md`](theron/THERON_CAPTURE_READINESS.md); this queue
contains only the next cross-game work items, not a complete Theron inventory.

1. **Nexus title-state capture bundle.** Capture one retail-JP revision that
   binds `TITLE.CG`, its active `MAPD` span, CRAM palette, VDP1/VDP2 register
   state, layer order, and timing. This is the admission prerequisite for a
   native title renderer.
2. **Nexus menu source/display join.** Ymir `--debug` on authentic retail media
   has already demonstrated Start opening the title menu, navigation to
   Continue, A confirming New Game, and a separate ShuttleMouse title button
   event. Bind that interaction to the menu/face display consumer and Firestaff
   startup state; the raw frames 13000–13039 remain non-interactive animation
   and must not be promoted as menu evidence.
3. **Nexus Structure 2/VDP1 material ownership.** Bind real texture, CLUT,
   raster, clipping, animation, and composition inputs to one captured
   consumer before enabling textured 3D output.
4. **Nexus dispatcher/event/save/audio contracts.** Add each native Saturn
   runtime behavior only after its original consumer and same-revision trace
   are hash-verified.
5. **Theron JP generated-row consumer join.** Replay the authenticated Drator
   event with the bounded `$C852` opcode-fetch hook, enabled by
   `THERON_CAPTURE_PCE_FAST_C852_FETCH_TRACE=1`. Its physical PC/MPR and shared
   event sequence can be correlated with the row-lifecycle and main-RAM reader
   traces. The optional main-RAM range filters physical data addresses, not
   instruction PCs. This evidence capture does not replace the native
   viewport's required map/pose-to-VDC/VCE/VRAM-to-frame join.
   The current receipt proves the row lifecycle, but no same-session capture
   shows `$C852` consuming that exact row; a hit in another physical bank does
   not close the join. Keep row fields opaque. Then continue the separate
   `$C1FC` active-map and movement question: its matched input-conditioned
   transfers may be teleporter destination copying and do not yet prove
   movement, collision or visible effect. See the
   [Theron capture readiness](theron/THERON_CAPTURE_READINESS.md). Before
   capture, satisfy the runtime gate: the complete clean-source patch chain
   passed three loops and the PCE Fast binary compiled and installed, but
   capture verification is blocked until a real SDL2 runtime is available;
   the current binary links `sdl2-compat`.
6. **Theron JP cold data handoff and presentation ownership.** Capture a cold
   Track 02 transition through its game-owned consumer, then bind bitmap,
   palette, text and audio state to the native presentation path. Save-state
   traces do not supply cold CD-to-RAM provenance.
7. **Theron JP/US save and later-dungeon matrix.** Verify each region
   separately with authentic progressed saves and level transitions. The
   available JP BRAM is empty; the emulator `.mc0` is a device snapshot, not
   an in-game save.
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
