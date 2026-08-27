# DM2 V1 — Platform Variants

## Current Firestaff support

Firestaff's DM2 runtime is playable from authenticated DOS, Amiga, FM Towns
and Macintosh source data. The shared M12 data root keeps those editions
separate: DOS may resolve through its verified `data` symlink, while Amiga,
FM Towns and Mac retain their original archive owners. Focused M11 real-media
regressions verify New Game and active runtime paths for all four families.

The remaining items in this page are parity boundaries, not a requirement to
substitute one edition's files for another: native non-DOS saves, positive DOS
WIELD/death-drop evidence and broader platform-specific UI/audio owners remain
tracked in `TODO-dm2.md`.

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
- Firestaff admits the authentic US English retail ZIP as `mac-en-retail`.
  Its raw MODE1/2352 BIN is read in memory through the Apple Partition Map and
  HFS catalogue. `DMFiles/Dungeon.dat`, `Graphics.dat`, and `md.dat` remain
  owned by the original ZIP and are never unpacked to the game-data directory.
- The retail version has a hash-paired boot receipt and uses the 39,411-byte
  big-endian dungeon. The Japanese Mac multi-track archive is now
  readable through its authentic late Apple_HFS partition and
  case-preserving catalogue as a media-only gate; it remains a preservation
  input, not a launchable variant, until its Japanese graphics/dungeon pair
  and runtime are source-locked. French Mac remains preservation-only.
- The Mac dungeon reader keeps the ZIP/HFS source bytes intact and carries the
  68k word order through the authenticated column, ground-stack and record-link
  accessors. The retail ZIP passes the source record-graph and start-pose
  gates. Retail map-wide GAME_LOAD passes all 44 authentic maps using the
  canonical File_header pool layout without extracting the ZIP data.
- The English Macintosh input table is source-locked and tested for both
  versions: inventory keys, movement aliases, freeze/wake, Command-O/S/Q,
  entrance/credits Return, and the three wall-button columns. Actions
  represented by the existing command queue are enqueued; wall-button columns
  use the separate source-owned activation gate described below. Held
  keyboard/gamepad movement uses the same source boundary, with Mac-specific
  A/D/W/S/X/Z/C and keypad meanings; host autorepeat is not used as the game
  clock.
- The authenticated GAME_LOAD sound owner is wired for both English
  editions. Immediate gameplay SFX resolve the source `xsndptr2` binding
  (`w_00`) to its real GDAT raw sample (`w_05`), decode the original Mac
  `Graphics.dat` payload in memory and hand it to the verified playback
  backend. Missing source rows or an unavailable backend stay fail-closed;
  no DOS sample, `snd ` MIDI instrument resource or synthetic PCM is used.
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
  members). Source `0x04` wall- and floor-mecha timers remain fail-closed
  until the complete DB3/DB14/DB0 GAME_LOAD transaction is owned atomically;
  Firestaff must not derive mutable targets from raw timer bytes. Remote Mac
  `0x17`/`0x1A`
  item admission and the small demo's authentic target class 7 still require
  their original GAME_LOAD owner before mutation.
  No chain is converted into a DM1 front-cell action.
- Both English Mac editions now also admit the authentic 16-colour
  `INTERFACE_CHARSHEET/0/dtImage/1` inventory frame. The M11 inventory toggle
  consumes its original local palette and `RECT_1EE`/RAW4 placement directly
  from the selected Mac `Graphics.dat`, without extracting or redrawing the
  panel. Inventory item-slot selection, drag/drop and the remaining native
  modal event owner are still fail-closed until their Mac source owner is
  recovered.
- Dynamic creature admission now reads the authenticated FB/FC/FD animation
  tables from the selected Mac `Graphics.dat` through the same source-owned
  GAF lookup used by the other admitted editions. Each authenticated Mac
  image may legitimately lack a row on a particular source path; that case
  preserves the source creature/timer owner with no animation rather than
  borrowing the other edition's table or inventing a frame. The fallback is
  limited to the two authenticated English Mac dungeon sizes, and both
  editions are covered by the real-media census gate.
- The authenticated GAME_LOAD party snapshot is now copied read-only into
  the M11 presentation mirror. This enables the English Mac F1-F4 champion
  inventory commands to select a real party champion and open/close the Mac
  CHARSHEET route on both editions. The snapshot copy does not create heroes
  or invent missing item records; native item-slot and drag/drop event
  ownership remains a separate gate.
- Keyboard/gamepad confirmation of the selected slot is now an accessibility
  path into the same authenticated ObjectID/leader-possession exchange for
  both English editions. A missing or rejected source slot remains unavailable;
  Firestaff does not create an item. Native Mac pointer and drag/drop input is
  still blocked until the dynamic Control/Event owner from the retained Mac
  application is bound. The Mac `CODE(3)` path does not use the PC or FM Towns
  expanded-rect table, so those coordinates are not reused.
- Mac retail save parsing preserves the source big-endian word order for the
  42-byte save header, dungeon prefix, column/ground links and initial
  `s_savegamebuffer` scalars. This is a read-only admission boundary only:
  `.firestaff/data/dm2` contains no authentic Mac save to verify, and Resume
  remains closed until the complete Mac `DM2_GAME_LOAD` record/possession
  owner is implemented. The corpus scanner now carries an explicit
  big-endian body-order receipt for a future authentic Mac corpus; it never
  guesses from the host or filename. No synthetic save is used.
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
  the source input table. M11 now holds each decoded frame for the
  positive duration carried by its authentic QuickTime frame record; the
  real-media gate rejects zero-duration or non-monotonic frame timing instead
  of inventing a host cadence. Exact original presentation ownership remains
  separate.
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
  M11. The 28 authentic application `Midi` resources (IDs 1000-1027) are
  parsed in place as SMF and selected by the Mac `md.dat` map route. Native
  CoreMIDI scheduling parity and the remaining `snd ` timing classes remain
  separate runtime work.
- The retail HFS volume's real `Dungeon Master II` application is also kept as
  separate in-memory data/resource forks (`484,944` / `5,046,234` bytes).
  This preserves the source owner for the authentic `Midi`, `snd `, menu and
  event resources without flattening or extracting the application. During
  gameplay, the Mac runtime advances an authenticated MIDI map cue on the
  same 55 ms source tick used by the DM2 runtime after a cue is selected; it
  does not use a host frame delta, and the scheduler consumes each event once
  as the source playhead crosses it instead of replaying the elapsed prefix
  on every host tick. The retained application resource map is also
  source-censused: it contains `CNTL` resources 130 and 131 (32 bytes each),
  alongside the `MENU`, `DITL`, and `DLOG` families. These are authentic
  dialog/control resources, not permission to invent gameplay hit rectangles;
  native CHARSHEET pointer/drag ownership still depends on the dynamic
  `Control`/event records published by `CODE(3)`/`CODE(11)`. CoreMIDI
  device/timing parity and that dynamic pointer owner remain open gates. This
  is a verified media/resource boundary, not a claim of complete Macintosh
  playability.

X68000 is not part of the DM2 support matrix.

## PlayStation?
- DM2 was never released on PlayStation

## Source Reference
- skproject (https://github.com/gbsphenx/skproject) contains DOS source code
- SKULL.ASM is the main DOS executable/disk image file
- No separate source for Windows version - same codebase

## Conclusion
Amiga, DOS, FM Towns and Macintosh are separate original DM2 media/runtime
families in Firestaff's support matrix, and all four now reach a playable
source-owned runtime. X68000 and PlayStation are outside the supported matrix.
