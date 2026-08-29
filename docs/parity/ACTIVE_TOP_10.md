# Active Top 10 Native-Parity Work Items

Last reviewed: 2026-08-29

This is the current cross-game priority order. Each item is either actionable
with the supplied original media or explicitly marked **blocked by missing
evidence**. A fixture, a generated save, a guessed decoder, or an emulator
runtime dependency is not an acceptable substitute. Firestaff must continue
to read supplied game media in memory and run it natively.

| Priority | Work item | Scope | Acceptance evidence | State |
|---:|---|---|---|---|
| 1 | Capture the same-revision Nexus composed title/menu state | Nexus Saturn | One capture binds source bytes, CRAM, VDP1/VDP2 layers/priorities, timing and resulting pixels. | **Blocked by missing capture** |
| 2 | Reverse and implement Nexus PRS3 from real `MENU.BPK` bytes | Nexus Saturn | Executable/disassembly-derived format contract, real-archive decode receipts, bounds tests, then a native surface handoff. | Actionable |
| 3 | Capture Theron CD-to-RAM level-transition consumers | Theron JP and US | Authentic, revision-specific trace from Track 02 read through RAM consumer for at least one level transition per revision. | **Blocked by missing trace** |
| 4 | Obtain DM1 original runtime overlay pairs | DM1 PC 3.4 | Hash-locked original and Firestaff captures for movement, HUD and viewport states; pixel/behaviour regression. | **Blocked by missing captures** |
| 5 | Bind CSB DSA-bearing original saves | CSB Atari, Amiga, FM Towns | Provenance/hash receipts plus native timer, transaction and DSA regressions per admitted edition. | **Blocked by missing saves** |
| 6 | Build CSB representative visual/audio comparisons | CSB Atari, Amiga, FM Towns | Original HUD, viewport, title, door and audio captures with native comparison gates. | **Blocked by missing captures** |
| 7 | Complete DM2 retail save ownership | DM2 DOS, Amiga, FM Towns, Mac | Authentic `SKSAVE`/record-pool/relocation corpus, fail-closed parser and save/resume regression for every admitted platform. | **Blocked by missing saves** |
| 8 | Extend DM2 retail gameplay receipts | DM2 DOS, Amiga, FM Towns, Mac | Real-media dialog/input ordering, AI/drop, audio and save/resume paths captured and natively regression-tested. | Partly actionable; corpus limited |
| 9 | Prove DM1 FM Towns audio and input | DM1 FM Towns | BIN/CUE in-memory track selection, CD-audio timing receipts and FM Towns-specific input-route tests. The first real-CUE receipt is CTest-bound; track payload timing remains open. | Actionable with supplied media |
| 10 | Prove DM1 Amiga planar presentation | DM1 Amiga | Original ADF-to-native planar image decode and representative pixel receipt; no PC-byte fallback. | Actionable with supplied media |

## Recently closed before this queue

The current queue starts after these verified fixes, so they must not be
re-added as open work:

- DM1 canonical and nested DOS ZIP startup routes are exercised through CLI,
  start menu and first movement.
- Cracked-only DM1 Atari archive selection fails closed instead of selecting a
  nearby sibling.
- CSB accepts supplied preservation ZIPs in memory; protected STX logical
  sector order is preserved even when payload offsets are skewed.
- CSB's supplied French preservation ZIP reaches title, runtime, first
  movement and start-menu launch through the native STX reader.
- The supplied DM2 Macintosh First Chapter demo fails closed and cannot be
  mistaken for retail media.
- Game-data archives are no longer materialized on disk by Firestaff.
- Completion evidence records only the verified native-media scope above.

## Selection rule for the next queue

When an item closes, replace it with the highest-impact item that has real
media or capture evidence available. If the next item is evidence-blocked,
keep that state visible rather than fabricating a substitute. Per-game detail
and deferred data requests remain in `TODO-dm1.md`, `TODO-csb.md`,
`TODO-dm2.md`, `TODO-nexus.md`, and `TODO-theron.md`.
