# Preservation and provenance

This page defines what Firestaff preserves, what can be claimed, and what
must remain outside the repository.

## Preservation rules

1. Keep original archives, disc images, CUE/BIN sets, extracted files and saves
   unchanged in an external, user-owned collection.
2. Record the source identity before decoding: game, platform, edition,
   language, container, byte length and SHA-256 where possible.
3. Never mix files from different editions. A matching filename is not enough;
   the loader must accept the complete hash-verified set.
4. Keep raw bytes and decoded interpretations separate. A decoder result is a
   hypothesis until its source span, format and round-trip or runtime check are
   recorded.
5. Do not commit copyrighted game data, saves, emulator memory dumps or private
   capture material to Firestaff.

## Evidence levels

| Level | Meaning | Suitable claim |
|---|---|---|
| Source reference | ReDMCSB, CSBWin, skproject, disassembly or DMWeb identifies a rule | The rule is documented or source-anchored |
| Byte verified | Hash-identified bytes parse and satisfy bounds/shape checks | The format or record boundary is verified |
| Runtime verified | The real-data route reaches a named runtime state | That route works for the tested corpus |
| Pixel/capture paired | Original and Firestaff frames or traces are paired with provenance | The named visual/timing comparison is verified |
| Playable | Startup, input, world, save/load and runtime route work together on original data | The game/platform is playable in the stated scope |

A parser test, synthetic fixture or source citation alone is not a playable or
pixel-parity claim.

## What belongs where

- **Repository:** source code, tests, source-lock documents, hash manifests,
  non-proprietary metadata, capture schemas and summaries.
- **External original-data store:** game archives, disc images, extracted
  `DATA/` trees, original saves and emulator media.
- **External capture store:** screenshots, frame dumps, audio captures, RAM
  traces and operator logs, each with a manifest and source identity.
- **Fixtures:** small deterministic inputs for parser, bounds and negative
  tests. Every fixture must be labelled fixture-only and must never be used as
  positive original-media evidence.

## Per-game preservation boundary

| Game | Preserve first | Current unresolved boundary |
|---|---|---|
| DM1 | PC34 `GRAPHICS.DAT`/`DUNGEON.DAT`, FM Towns Track 01, original saves and DOS captures | C13-bearing save corpus and broader original-vs-Firestaff capture pairs |
| CSB | Per-media graphics/dungeon pairs, Utility/CSBWin saves, DSA-bearing saves and captures | Positive CSBWin DSA corpus and wider HUD/viewport/save parity |
| DM2 | Complete DOS/FM Towns/Amiga media sets, GDAT/DUNGEON pairs and SKSAVE media | Full SKSAVE ownership, live V1 runtime, real HUD/material and broader gameplay evidence |
| Nexus | Complete Saturn CUE/BIN, `DM.BIN`, `LEV*.DGN`, `*.MNS`, `FACE.BIN`, `SLEV*.BIN`, `SNDLEV*.SAL/.MAP` | Saturn runtime/frame capture, material semantics and SAL/MAP event playback |
| Theron | Matching US/JP Track 02 BIN/ISO, CUE, Track 19 media and SRM/SRAM artifacts | Full Track 02 handoff, save body semantics, bitmap/palette ownership and JP capture |

The detailed gaps are in [missing functions by game](../MISSING_FUNCTIONS_BY_GAME.md)
and the synthetic-data inventory is in
[synthetic data by game](../SYNTHETIC_DATA_BY_GAME.md).

The current cross-game source, format and real-media boundary is maintained in
[Preservation status 2026-08-11](../PRESERVATION_STATUS_2026-08-11.md).

## Capture manifest minimum

Every promoted original capture should record:

- game, platform, edition and language;
- original-data hash or external provenance identifier;
- emulator/app version, display scale, window size and palette mode;
- input sequence and timing origin;
- frame/audio/RAM artifact hashes;
- the exact claim being tested and its non-claims;
- whether the result is source-only, original-only, paired or runtime-only.

If any of these are unknown, keep the artifact as exploratory evidence and do
not promote it to a parity receipt.

## Preservation workflow

1. Acquire and retain the original media in an external store.
2. Hash and classify it without renaming bytes or rewriting containers.
3. Extract only for analysis; retain the original container and extraction
   manifest.
4. Write a narrow source/data receipt with exact offsets or source anchors.
5. Add a focused test that fails closed for malformed, synthetic or mismatched
   input.
6. Capture the real route when a visual or timing claim is made.
7. Update the per-game status and release notes only for changes actually
   merged; do not imply that a receipt closes a larger open boundary.

## Reference map

- [Game-data setup](../DATA_SETUP.md)
- [Project status](../PROJECT_STATUS.md)
- [Verified hashes](../VERIFIED_HASHES.md)
- [DMWeb and Greatstone references](../DMWEB_REFERENCE.md)
- [Parity evidence](Parity-Evidence.md)
- [Reverse-engineering index](Reverse-Engineering-Index.md)
