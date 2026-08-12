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
- A shared DM2 data-root scan admits both Mac versions in one pass. The first
  matching archive no longer hides its sibling, so the launcher can present
  the large retail and small demo entries together.
- The two versions have separate hash-paired boot receipts. Retail uses the
  39,411-byte big-endian dungeon; the demo uses its 6,535-byte truncated
  big-endian File_header/map data. Japanese and French Mac containers remain
  preservation inputs, not launchable variants.
- The Mac dungeon reader keeps the ZIP/HFS source bytes intact and carries the
  68k word order through the authenticated column, ground-stack and record-link
  accessors. Both English ZIPs now pass the source record-graph and start-pose
  gates. Retail map-wide GAME_LOAD passes all 44 authentic maps using the
  canonical File_header pool layout. The demo's authentic 16-entry champion
  roster uses per-mirror DYN4 selectors, but its File_header contains no
  tick-generator-family actuators; Firestaff records that as a valid
  zero-generator source projection. Its STARTEND path uses the real mirror at
  map 0,0,0 while retaining the source party pose at map 0,1,8. Both versions
  now pass separate real-media New Game gates without extracting the ZIP data.
- The English Macintosh input table is source-locked and tested for both
  versions: inventory keys, movement aliases, freeze/wake, Command-O/S/Q,
  entrance/credits Return, and the three wall-button columns. Actions
  represented by the existing command queue are enqueued; wall-button columns
  use the separate source-owned activation gate described below. Held
  keyboard/gamepad movement uses the same source boundary, with Mac-specific
  A/D/W/S/X/Z/C and keypad meanings; host autorepeat is not used as the game
  clock.
- In the M11/SDL route, an admitted `DM2_PLATFORM_MAC_EN` profile takes
  precedence over generic PC key aliases. Movement, champion/leader inventory,
  freeze, wake, save and quit are dispatched through the existing runtime
  boundaries. Mac wall-button columns now reach a
  source-owned activation gate: Firestaff consumes an authenticated
  renderer target, either the original `c_rwbb` door target or a target
  published from a visible wall tile's real DB3 mechanism. A complete real
  `PUSH_BUTTON_SWITCH` (`0x46`) chain may mutate only its direct DB0 door
  targets. Authenticated local-action DB3 chains for `0x17`, `0x18` and
  `0x1A` use the source actuator-list rotation path (including mixed chain
  members). Source `0x04` wall- and floor-mecha timers now enter the
  source-owned DB3/DB14 actuator walkers at runtime; target classes whose
  owners are not yet complete remain fail-closed. Remote Mac `0x17`/`0x1A`
  item admission and the small demo's authentic target class 7 still require
  their original GAME_LOAD owner before mutation.
  No chain is converted into a DM1 front-cell action.
- Retail HFS media now exposes the authentic raw `MooV` data forks and HFS
  Resource Manager forks in memory. The four present movies have source-owned
  `moov` resource payloads; Firestaff keeps the data fork, complete resource
  fork, and extracted resource separate:
  `Title.MooV` (2,403,013 bytes), `Swoosh.MooV` (463,528), `Credits.MooV`
  (5,601,948), and `Ending.MooV` (6,138,318). `Story.MooV` is absent from
  this image. Firestaff does not synthesize, flatten, or convert these files;
  Firestaff also builds a bounded in-memory QuickTime view (`moov` followed by
  `mdat`) for each present movie without changing either fork. The view keeps
  the authentic four-byte prefix present before `Swoosh.MooV`'s `mdat` out of
  the atom stream while preserving the source fork as the authority. The
  retail decoder gate covers all four present movies. M11 now presents the
  authentic Title movie at Mac startup and opens authentic Credits from the
  authenticated Credits rectangle; Mac Return/Enter closes that movie through
  the source input table. The demo has no Credits movie and remains on its
  authenticated static route. M11 now holds each decoded frame for the
  positive duration carried by its authentic QuickTime frame record; the
  real-media gate rejects zero-duration or non-monotonic frame timing instead
  of inventing a host cadence. Exact original presentation ownership remains
  separate.
- The small First Chapter demo is also admitted through the complete M11
  launch path: its static Mac startup surface accepts New Game and publishes
  the authentic source session from the truncated File_header and real
  16-entry mirror roster. This is separately gated from the retail movie
  route; the demo does not borrow retail QuickTime or title pixels.
- The retail image also retains the complete authentic resource forks for
  `Music` (662,956 bytes), `General.sounds` (134,562), and `Weapon.sounds`
  (50,651). These are source-bound Mac resources, not DOS HMP substitutes;
  a bounded format-1 `snd ` parser now validates their authentic resource
  counts and sample headers in place. The exact signed 8-bit sample range from
  resource 10001 is hash-checked and transported through the SDL host mixer at
  its source rate. The four present movies are also available through an
  optional FFmpeg-backed in-memory QuickTime decoder: Cinepak/Animation frames
  and each movie's PCM audio are delivered to the M11 surface/mixer without
  creating a converted movie file. Frame timing is now source-duration gated in
  M11. Complete Mac resource selection, gameplay timing, and MIDI/music
  scheduling remain separate runtime work.
- The retail HFS volume's real `Dungeon Master II` application is also kept as
  separate in-memory data/resource forks (`484,944` / `5,046,234` bytes).
  This preserves the source owner for the authentic `Midi`, `snd `, menu and
  event resources without flattening or extracting the application.

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
