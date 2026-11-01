# Next Ten Native Work Items

Reviewed 2026-11-01. This is the current implementation queue after the
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
5. **Theron US (Drator) generated-row consumer join.** The isolated cold
   Drator reference records 132 runtime `$C852` fetches at physical `$0DC852`;
   its first five 512-byte windows exactly match authenticated US Track 02 raw
   offset `$A3AD2`. The route reached `left-event` and `reentered-event`, and
   same-session writes/reads of generated-row offsets 6–8 plus a later ten-byte
   work-buffer copy are proven. The consumer observer reached its cap after
   these reads; the route subsequently reached reentry. The BIOS trace was
   explicitly truncated, and its streaming verifier correctly rejected the
   marker. A strict tooling verifier now matches constructed `$CBC6` and
   `$45C3` instruction windows against US retail bytes, with four and three
   possible raw offsets; capture both windows in a new authentic run to bind
   the live source occurrence. Then trace the work buffer downstream and
   establish whether `$C852`
   participates in this row lifecycle. Keep row bytes opaque. This reference
   capture does not replace the native viewport's map/pose-to-VDC/VCE/VRAM-to-frame join. Continue the separate `$C1FC` active-map and movement question:
   its matched input-conditioned transfers may be teleporter destination
   copying and do not prove movement, collision or visible effect. See the
   [Theron capture readiness](theron/THERON_CAPTURE_READINESS.md). The clean
   base Mednafen 1.32.1 patch chain and isolated real-SDL2 2.33.0 runtime check
   passed. The newer `$3879` window extension applies in three patch-only
   loops, with a separate PCE Fast-enabled pass. Its instrumented PCE Fast
   emulator also built against SDL2 2.32.10 and passed the isolated
   startup/runtime version check. An authentic JP state-only run exercised the
   patch but reached its shared 8,192-row cap without `$C852` or `$5039`; the
   ordered US `$C852`→`$C85E`→`$5039` path remains to be captured. See [Theron capture readiness](theron/THERON_CAPTURE_READINESS.md).
   No Drator-positioned save or state artifact is checked into the repository,
   so reproduce the route from hash-verified media in a fresh profile.
6. **Theron JP cold data handoff and presentation ownership.** Capture a cold
   Track 02 transition through its game-owned consumer. For the active map,
   preserve one sequence from authenticated descriptor/map source reads through
   System Card transfer and optional `$23AD` decoding, the active selector,
   and a tile read at the captured party pose; pair this with atomic
   VDC/VCE/VRAM and a no-input control. Save-state traces do not supply cold
   CD-to-RAM provenance. See the [active-map acceptance chain](../TODO-theron.md#2026-11-01--define-the-jp-active-map-capture-acceptance-chain).
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
