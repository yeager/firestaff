# Firestaff TODO — DM1

Reviewed 2026-08-29. Only open work is listed here.

- Obtain authentic C13-save and original capture corpus for remaining HoC,
  top-row and action routes; bind each to the PC 3.4 runtime before promotion.
  The supplied French DOS save pair proves its own backed F0435/F0433 route,
  including direct CLI and start-menu resume, bounded movement, and turns on
  map 5 with its four saved champions, but does not substitute for these
  route-specific captures.
- Extend real-media parity beyond bounded Atari ST, Amiga and FM Towns routes
  to native end-to-end gameplay, input and presentation evidence.
- Bind an authenticated Amiga gameplay RGB4 table to the native renderer.
  The original v2.0 `dm` producer is implemented and source-gated: its
  in-memory 68000 receipt copies the saved table to a working table, adjusts
  each RGB4 component by one or two toward the target, and invokes the Copper
  builder eight times. It deliberately accepts no fabricated palette. What
  remains is a route-specific original gameplay table/capture that identifies
  the active source and target tables; do not substitute the PC VGA palette.
  The supplied ordinary Amiga save disk is now receipted as an authentic
  `DMGAMEG.DAT` session, but it is format-5/Amiga-compatible save material,
  not a Copper-list or framebuffer capture; it cannot alone identify the
  palette table active at a particular gameplay frame.
- Materialize the already authenticated Amiga format-5 `DMGAMEG.DAT`/`.BAK`
  session into a native Amiga world adapter.  The read-only ZIP → ZIP → ADF
  route now verifies its header, all five F0435 encrypted parts (global data,
  active groups, party and timeline), the four 464-byte portraits, and the
  F0434 big-endian dungeon stream plus its original checksum.  It is not a
  PC34 envelope and must never be coerced through the PC34 importer.  Bind
  direct-CLI and start-menu resume only after that format-specific adapter
  owns the loaded world and presentation state.  The authenticated F0434
  tail now materializes atomically into an empty source-specific world while
  preserving the exact big-endian tail for a future Amiga serializer, proven
  against the supplied save disk without modifying its source bytes. C2's
  320-byte champion records and PARTY_INFO now have a source-specific receipt,
  and all five decrypted F0435 parts have a shared in-memory boundary. C1/C3/
  C4 validate the original event heap's active membership and materialize a
  native event queue (including the supplied save's WATCHDOG event). Applying
  the full authenticated session to M11 remains.
- The supplied French DOS ZIP → `dungeon_master.exe` SFX package uses a
  non-solid RAR 2.0 (`unp_ver=0x14`) stream. Native RAR2 decoding is
  intentionally out of scope; the launcher reports it as unsupported rather
  than misreporting the supplied original data as missing.
- Bind V2.2 presentation only to reviewed original material/pixels. Existing
  placeholder or procedural art remains fixture-only.

## Deferred original-data corpus

The missing C13 save/capture corpus is deferred rather than a release blocker
for the native media paths currently under active work.  When an
operator-supplied corpus becomes available, add its hashes and provenance,
then promote only the routes it directly proves.  Do not manufacture a save
or capture to close this section.
