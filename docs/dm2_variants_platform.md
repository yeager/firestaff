# DM2 V1 — Platform Variants

## DOS Version
- Publisher: Interplay
- Platform: DOS, 3.5" floppy disks (6 disks)
- Release: 1994
- Engine: Similar to DM1 (first-person dungeon crawler)
- Graphics: 3D rendered wireframe corridors, pre-rendered character sprites
- File: Dungeon-Master-II-Skullkeep_DOS_EN.zip

## Windows 3.1 Version
- Release: 1995
- Platform: Windows 3.1 with VP6 codec for video
- Video playback: FMV sequences using VP6 codec
- Same game content, different media handling
- File: (part of DOS release archive, Windows executable included)

## Amiga Version

- Dungeon Master II was released for Amiga in Europe as version 1.0 in
  English/French/German. It is a distinct 16-colour, big-endian DM2 port;
  it is neither a DOS data alias nor an unofficial conversion.
- The original retail media consists of six install floppies. It cannot run
  directly from those floppies: the supplied installer concatenates
  `dm2_arcsplit1` through `dm2_arcsplit6` into `DM2_archive.LZX`, then runs
  `unlzx` to create the hard-disk game tree. Firestaff must perform that
  original archive operation in memory before it can hash-verify an Amiga
  `GRAPHICS.DAT`/`DUNGEON.DAT` pair; it must never unpack the game data to
  disk or present the installer disks as playable.
- Amiga music is map-triggered MOD playback (`CD.DAT` and `SK00.MOD` through
  `SK09.MOD`), not the PC HMP route. The Amiga-specific keyboard, video,
  animation and MOD paths remain separate runtime work after media admission.

Source: [DMWeb's Amiga edition notes](http://dmweb.free.fr/games/dungeon-master-ii/editions/amiga/),
[Greatstone's DM2 version index](http://greatstone.free.fr/dm/g_dm2.html),
and the local archived copy in `docs/DMWEB_REFERENCE.md`.

## Macintosh Version

- Macintosh is a separate 68k release family with its own big-endian data,
  resource-fork/container and menu/audio conventions. It is not a DOS or FM
  Towns data alias.
- Firestaff admits both authentic US English ZIPs as separate versions:
  `mac-en-retail` (the large retail disc) and `mac-en-demo` (the smaller First
  Chapter demo). Both raw MODE1/2352 BINs are read in memory through the Apple
  Partition Map and HFS catalogue; the demo additionally walks its genuine
  StuffIt 2 `DMFiles` member in RAM. `DMFiles/Dungeon.dat`, `Graphics.dat`,
  and `md.dat` remain owned by the original ZIP and are never unpacked to the
  game-data directory.
- The two versions have separate hash-paired boot receipts. Retail uses the
  39,411-byte big-endian dungeon; the demo uses its 6,535-byte truncated
  big-endian File_header/map data. Japanese and French Mac containers remain
  preservation inputs, not launchable variants.
- The English Macintosh input table is source-locked and tested for both
  versions: inventory keys, movement aliases, freeze/wake, Command-O/S/Q,
  entrance/credits Return, and the three wall-button columns. Only actions
  represented by the existing command queue are enqueued; Mac-only actions
  remain explicit until their native dispatcher is implemented.
- In the M11/SDL route, an admitted `DM2_PLATFORM_MAC_EN` profile takes
  precedence over generic PC key aliases. Movement, champion/leader inventory,
  freeze, wake, save and quit are dispatched through the existing runtime
  boundaries. Wall-button actions remain explicit and fail closed until their
  original Mac wall-button owner is implemented.
- Retail HFS media now exposes the authentic raw `MooV` data forks and HFS
  Resource Manager forks in memory. The four present movies have source-owned
  `moov` resource payloads; Firestaff keeps the data fork, complete resource
  fork, and extracted resource separate:
  `Title.MooV` (2,403,013 bytes), `Swoosh.MooV` (463,528), `Credits.MooV`
  (5,601,948), and `Ending.MooV` (6,138,318). `Story.MooV` is absent from
  this image. Firestaff does not synthesize, flatten, or convert these files;
  Firestaff also builds a bounded in-memory QuickTime view (`moov` followed by
  `mdat`) for each present movie without changing either fork. This view is
  validated against the authentic retail boot profile; decoder, timing, audio,
  and presentation ownership remain separate.

X68000 is not part of the DM2 support matrix.

## PlayStation?
- DM2 was never released on PlayStation

## Source Reference
- skproject (https://github.com/gbsphenx/skproject) contains DOS source code
- SKULL.ASM is the main DOS executable/disk image file
- No separate source for Windows version - same codebase

## Conclusion
Amiga, DOS, FM Towns and Macintosh are separate original DM2 media/runtime
families in Firestaff's support matrix. The current runtime gates are
edition-specific: verified DOS and FM Towns paths are furthest along, while
Amiga and Macintosh require their remaining native runtime owners. X68000 and
PlayStation are outside the supported matrix.
