# Firestaff DONE — DM2

Reviewed 2026-08-29. Completed work only.

- The supplied DOS and Amiga retail ZIPs are now injected into the native
  CTest title, asset and New-Game receipts. The Amiga six-disk receipt walks
  ZIP→ADF→`dm2_arcsplit1`…`6`→LZX entirely in RAM; the Amiga M11/M12 title
  and New-Game paths and DOS M11 New-Game path cannot silently skip because
  their archive environment was unset.
- The supplied DOS retail ZIP now also drives the SKSAVE corpus regression:
  all four primary and four backup members retain virtual archive provenance,
  pass the original-state census, and are reread through receipt-bound RAM
  APIs rather than an extracted save directory.
- The supplied Macintosh retail BIN/CUE ZIP is now injected into the existing
  real-media CTest suite. It exercises native in-memory HFS forks, MooV
  resources, movie decoding/runtime, sound resources, pointer input,
  New-Game flow and wall-source census rather than silently skipping because
  the archive environment was unset.
- The real DOS English/French, Amiga, FM Towns, and Macintosh startup tests now
  each exercise the ordinary start-menu handoff separately from boot-probe
  mode. `FIRESTAFF_FAIL_IF_NO_LAUNCH` and `FIRESTAFF_EXIT_AFTER_LAUNCH` make a
  missing menu launch fail before the existing source-owned GDAT and movement
  assertions run.

- Native FM Towns ZIP/CUE/IMG intake reads original media in RAM, verifies the
  source-owned graphics/dungeon pair and preserves virtual source ownership.
- Real FM Towns M12/M11 startup, title and gameplay corpus checks pass with
  the authentic FM Towns archive and English DOS companion.
- The authentic Amiga installer archive now reaches title, original New Game,
  runtime and a visible native CHARSHEET inventory frame. Its 121×72 RAW4
  source is clipped to the original 119×70 destination using verified GDAT
  pixels and palette, rather than a substitute surface.
- The authentic PC-DOS ZIP now starts through both CLI and the start menu.
  M12 retains its verified `data/GRAPHICS.DAT` and `data/DUNGEON.DAT` virtual
  paths and the native DM2 boot owner reads them only in RAM.
- Generic ZIP hash discoveries remain diagnostic-only: they cannot claim a
  DM2 runtime route or redirect data into a cache. Only a supported,
  edition-specific archive owner may publish a native in-memory launch path.
- The authentic PC-DOS ZIP SKSAVE corpus is now also read directly in memory.
  The source scanner recognises all four `data/sksaveN.dat` primaries and four
  backups, records virtual archive paths and complete-file hashes, and can
  reread each receipt-bound payload without extracting a game-data member.
  The public slot scan, validity, and bounded-read APIs use the same virtual
  paths, so start-menu resume discovery does not require an unpacked save.
- The authentic Amiga installer now binds its native big-endian, 16-colour
  `INTERFACE_GENERAL/0` PalIRGB field 0 rather than PC field `0xfe`/PAL16.
  Its runtime HUD uses the source palette's physical-index receipt, matching
  the original 4-bit Amiga images without a fabricated local palette. The ZIP
  remains memory-owned through the native installer path and now produces an
  accepted M11 frame with real assets and no fallback drawing.
- The authentic Macintosh retail ZIP now keeps its normal 256-row
  `PalIRGB`/`dtPalette16` pair rather than being mistaken for the Amiga
  16-colour palette layout solely because both formats are big-endian. Its
  start-menu/title/New Game/movement route produces an accepted M11 frame with
  real assets and zero fallback draws, directly from the original ZIP in RAM.
- The authentic Macintosh *First Chapter* demo is explicitly fail-closed in
  the CLI regression suite.  An explicit demo archive is isolated from all
  sibling media and must never become a retail DM2 runtime owner.
- The DOS archive save matrix now verifies every supplied primary and backup
  `SKSAVE` through the native, read-only start-menu resume handoff. It treats
  frame acceptance as a separate presentation boundary, so a valid source
  load is not rejected merely because its initial saved pose has not yet
  supplied a renderable viewport transaction.
