# Preservation status 2026-08-12

This cross-game summary records what Firestaff can substantiate through source
code, tests and external original media. A passing parser or synthetic fixture
is not a claim of playability or pixel parity.

| Game | Preserved and verified | Open boundary |
|---|---|---|
| DM1 | PC DOS 3.4 V1 startup, input, viewport, HUD, combat and saves; separate Atari ST and FM Towns format paths | More paired original-vs-Firestaff captures and reviewed V2 material |
| CSB | Separate Amiga, Atari ST and FM Towns input and startup paths | X68000 is intentionally unsupported; remaining work covers DSA/save corpus and broader HUD/viewport parity on supported platforms |
| DM2 | DOS/FM Towns routes plus large and small English Macintosh routes from authentic ZIPs in RAM; Mac media, movies, sound, MIDI, input and New Game are verified | Full V1/SKSAVE ownership, Mac GAME_LOAD/Resume, dynamic pointer/drag ownership and continuous original-data gameplay |
| Nexus | Saturn DGN/DMDF, MNS, PRS3 and bounded SAL/MAP receipts | Visible material semantics, event/audio playback and a playable Saturn route |
| Theron's Quest | US/JP Track 02 identity, sector reading, level framing and authenticated capture chains | Game-owned Track 02 handoff, SRM contents, palette/bitmap ownership and positive gameplay capture |

## Format and disassembly principles

- ReDMCSB is the primary control-flow reference for DM1 and CSB. CSBWin is a
  separate reference for its own resources and save formats.
- DM2 follows skproject and its symbol audit. Nexus and Theron have no
  reconstructed source; their evidence must come from disassembly, raw media
  and capture receipts.
- Original archives, disk images, BIOS files, SRAM and raw emulator captures
  remain in the external user-owned collection. The repository contains code,
  hash metadata, small labelled fixtures and summarized receipts.

## CSB X68000

CSB for X68000 is unsupported. External HDM images may remain as preservation
references, but Firestaff provides no reader, cache, startup or emulation route
for them.

For supported-platform detail, see [preservation principles](wiki/Preservation.md),
[game-data formats](GAME_DATA_FORMATS.md), [CSB reference](REDMCSB_REFERENCE.md)
and the [gap list](FIRESTAFF_GAP_LIST.md).
