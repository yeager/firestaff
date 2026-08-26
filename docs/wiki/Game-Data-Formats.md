# Game-data formats

Firestaff identifies original media by verified content rather than by a
particular folder or filename. It reads archive members directly from the
player-supplied container into bounded process memory; it does not extract or
persist game data to a Firestaff-owned directory. This page is the wiki
summary; the complete, maintained record catalogue is the
[game-data format reference](https://github.com/yeager/firestaff/blob/main/docs/GAME_DATA_FORMATS.md).

## Format status

- **Read** — Firestaff can inspect and validate the format.
- **Runtime-bound** — verified media is used by a live game path.
- **Opaque** — the format is retained or preflighted without publication into
  live game state.
- **Closed** — the route deliberately refuses to guess, convert or write data.

Read support alone does not imply a complete game route.

## Dungeon Master and Chaos Strikes Back

| Platform/media | Verified format boundary |
| --- | --- |
| PC 3.4 | GRAPHICS.DAT IMG3 records: packed 4-bit pixels, high nibble first, with row padding; DUNGEON.DAT provides the dungeon and linked thing records. |
| Amiga | IMG1 nibble-RLE graphics. CSB also uses platform program assets such as TITL.DAT, APPB.FTL, KAOS.FTL and BJELoad_R. |
| Atari ST | DMCSB1 catalogue with 563 Atari-LZW records decoded as big-endian IMG1. CSB uses ANIMATE.SCR, ANIMATE.DAT, ANIMATE.FTL, FTLCODE and CHAOS.FTL. The retail Save Disk is a 720 KiB MSA/FAT12 blank formatted medium; an empty root means no save, not missing game data. |
| FM Towns | 0x8001-wrapped IMG2 graphics, not PC IMG3. CSB's F31 chain includes RUN386.EXE, SWITCHTW.EXP, CHTWE.EXP or CHTWJ.EXP, and the Utility executables. |

CSB has no original DOS/PC 3.4 release. Its Amiga route is the default
original-media path; Atari ST and FM Towns have separate verified media
boundaries.

CSB campaign/bootstrap media such as MINI.DAT, MINIF.DAT and MINIG.DAT is
never treated as a generic user-save target. Atari and Amiga user slots are
CSBGAME*.DAT with native .BAK recovery. FM Towns F31 saves use a C5 header,
checksummed blocks, portraits and a dungeon tail. The current external EN/JP
save files are rejected before runtime admission because their valid headers
and parts contradict the two-map Prison tail's saved map index; they remain
preservation evidence while source-faithful F0433 writing stays closed. Legacy CSBWin
csbgame*.dat files are fully authenticated into private candidates, but their
dungeon state is not yet atomically published into a live session.

CSB for X68000 is intentionally unsupported. Any local HDM is preservation
reference only and must not select a data, startup or game-view route; see
[preservation status](../PRESERVATION_STATUS_2026-08-11.md).

## Dungeon Master II

GRAPHICS.DAT is a typed GDAT record graph, not a flat sprite sheet.
DUNGEON.DAT uses the G1 record layout, including per-database extensions.
DM2 sound entries are a two-byte header followed by unsigned 8-bit mono PCM
at 6000 Hz. FM Towns media uses its DATA/ tree; Amiga media includes CD.DAT
and SK00.MOD through SK09.MOD. SKSAVE is a platform-specific save boundary.

## DM Nexus

Nexus disc data is read from original CUE/BIN media. LEV*.DGN uses the DMDF
level structures, *.MNS provides static map data, and MENU.BPK plus FACE.BIN
use PRS3-compressed resources. Face palettes are BGR555. The SAL map prefix
remains opaque until its full layout is verified.

## Theron's Quest

Theron's Quest reads raw MODE1/2352 Track 02 sectors: each 2,352-byte sector
has a 2,048-byte user-data region beginning at offset 0x10. Track 19 and the
gzip-wrapped SRM save boundary have separate readers. Firestaff's own
FSTQPTY1 state is not presented as an original save format.

Use [Game data](Game-Data) for setup and scanning, then follow the per-game
technical pages for implementation detail:

- [DM1 technical reference](DM1-Technical-Reference)
- [CSB technical reference](CSB-Technical-Reference)
- [DM2 GDAT internals](DM2-GDAT-Internals)
- [Nexus DGN and PRS3 internals](Nexus-DGN-and-PRS3-Internals)
- [Theron's Quest Track 02 internals](Therons-Quest-Track02-Internals)
