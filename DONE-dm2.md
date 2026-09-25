# Firestaff DONE — DM2

## 2026-09-25 — French DOS runtime regression timeout

- Increased the French DOS CTest allowance to cover both fresh sessions in the
  regression: the M12 source-menu runtime handoff and the direct CLI movement
  probe. With the original French ZIP supplied, the menu reaches the native
  runtime receipt and the CLI reaches `party=1,7,0`; the shell regression
  passes when allowed to finish. The previous 45-second CTest cap expired
  before that second result, despite no functional failure.

## 2026-09-24 — Original DOS save Quick Resume

- Fixed the explicit `--menu --game dm2 --save <path>` route: the M12 row now
  selects Quick Resume rather than consuming Enter as a fresh-game selection
  and dropping the supplied save. The real DOS archive regression passes both
  direct `--boot-probe` resume and ordinary M12 Quick Resume for all eight
  `SKSAVE0–3` primary/backup files, checking each saved map and party pose.
  The archive remains unchanged; full save/write ownership remains open.

## 2026-09-24 — DOS start-menu runtime handoff

- The authentic DOS English ZIP now has a normal M12→M11 regression through
  the source MVE intro and New-Game event. It checks the first source-owned
  runtime receipt for the retail initial party and requires a visible 320×200
  presented frame. The wait keeps the source menu keys from being consumed by
  the movie owner. Direct `--boot-probe` coverage remains a separate path.
  The French DOS route is recorded below; remaining DM2 editions/platforms
  still need equivalent first-runtime evidence.

## 2026-09-24 — French DOS start-menu runtime

- The authentic French DOS ZIP now follows the ordinary M12 → M11 path through
  the retail MVE and New-Game inputs to the native runtime receipt at the
  original party pose `(map=0,x=1,y=8,direction=0,count=1)`. The focused test
  keeps direct boot-probe movement separate and confirms the French GDAT
  identity there. This extends the DOS startup evidence only; it does not
  claim French DOS presentation parity.

## 2026-09-24 — Amiga start-menu runtime handoff

- The authentic Amiga CLI regression now advances the original SWSH.DAT and
  TITL.DAT streams through their 1,345 source-owned 50 Hz ticks, then clicks
  the original GDAT New Game rectangle through the normal SDL input mapper.
  It checks the loaded retail party at `(map=0,x=1,y=8,direction=0,count=1)`
  and requires a visible 320×200 runtime capture. The direct boot-probe input
  matrix remains a separate check. The Amiga big-endian RAW4 decoding fix
  restores source wall-rectangle geometry for GDAT v5, allowing the initial
  indoor frame and authentic movement matrix to use real assets without
  fallback draws. Same-state comparison with the original Amiga renderer and
  broader gameplay/render parity remain open in `TODO-dm2.md`.

## 2026-09-16 — FM Towns New Game real-media revalidation

- Rebuilt and ran the full M11 FM Towns real-media gameplay regression against
  the admitted HME-242 ZIP. It reaches the source SKULL menu after the title,
  dispatches New Game through the decoded source rectangle, confirms the
  prepared champion through the source mirror route, and commits the runtime
  session without a PC-English companion archive. The same run verifies
  authentic pit, stairs and DB1 transitions plus the real DB4/F9 creature
  timer path. FM Towns does not borrow the DOS Enter shortcut at this title;
  its source-owned New Game pointer route is required. All game data remains
  archive-backed and in memory.

## 2026-09-08 — FM Towns IMG2/IMG6 palette binding

- Corrected the FM Towns HME-242 image-palette route. Its `0x8004` GDAT
  stores variable-length IMG2/IMG6 C4 streams, not PC IMG3 records with a
  trailing `QUERY_GDAT_IMAGE_LOCALPAL` table. The old generic decoder used
  the final compressed bytes as palette values and emitted invalid physical
  indices such as 179 and 181 into the 16-colour dungeon framebuffer. FM
  Towns image pixels now keep their source-owned direct `0..15` physical
  indices; the active GRAPHICSSET palette remains the RGB owner. The genuine
  zipped HME-242 New-Game regression verifies a fully admitted source frame
  contains no index above 15. No game media is extracted, generated, or
  substituted.
- Corrected the matching FM Towns CHARSHEET admission and presentation
  gates. They had required the PC 255-colour summary even though the selected
  original panel is an IMG2/IMG6 four-bit surface. The live source route now
  opens, draws, accepts authenticated pointer contexts, and closes the real
  inventory page after New Game.

## 2026-09-03 — Real-media startup regression audit

- Re-ran the native real-media startup matrix for DOS English and French,
  Macintosh, Amiga and FM Towns. Every archived DOS `SKSAVE` primary and
  backup slot resumed through direct CLI and the start menu in RAM; all four
  platform owners selected their own retail source rather than a fallback.

Reviewed 2026-08-29. Completed work only.

- FM Towns and Amiga startup menu/credits palettes now preserve the exact
  eight-bit RGB components in their authenticated GDAT `dtPalIRGB` rows.
  The DOS-only `DM2_CONVERT_DRIVERPALETTE` six-bit VGA conversion is no
  longer applied to those platform routes. Real-media tests compare all 256
  presented entries against each selected ZIP's raw palette and prove the
  old conversion changed at least one source component.

- Macintosh retail `Cmd-O` now reaches a native **Open Game** owner rather
  than being dropped or treated as Back/quickload. The modal lists only
  complete original `SKSave` candidates, two per page, from the configured
  original-save source. Before display and again before loading it performs a
  read-only ordered corpus census; the selected candidate's source path,
  byte order, payload and state hashes must still match before the runtime
  imports it. The chosen member stays in RAM and enters the existing
  source-owned `GAME_LOAD` import path. A missing, incomplete or changed
  corpus fails closed; no default slot, generated save, extraction or fallback
  candidate is used.

- The DOS, Amiga, FM Towns, and Macintosh retail start-menu routes each now
  create a fresh native New-Game session for every observed input command:
  forward/backward movement, left/right turn, both strafes and action.  Every
  path retains its original archive in RAM and checks the resulting party
  pose, accepted real GDAT frame and zero core fallback draws.  FM Towns and
  Macintosh keep their source-specific title and mirror-confirmation order.

- The packed PC-DOS `SKSAVE1` M11 spell regression now uses the retail ZIP
  directly in RAM. It proves that an authentic rejected `YA FUL IR` Fireball
  is consumed with the original rune-tail rule and that a source-admitted
  spell can then commit through M11. Read-only hero mana/wizardry receipts
  select the real cast; no save bytes, stats, or runes are fabricated.

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
  resources and in-memory container admission, sound resources, pointer input,
  New-Game flow and wall-source census rather than silently skipping because
  the archive environment was unset.
- The Macintosh SDL bridge now preserves Return's source-owned modal action:
  `NEW_GAME` at the entrance and `CLOSE_CREDITS` in credits both reach the
  existing M11 Accept owner, while gameplay Return remains `WAKE`. This fixes
  a dropped keyboard route without inventing a selection or save; the retail
  movie, New-Game/runtime and Macintosh input-table regressions cover it.
- The native Macintosh QuickTime reader now validates the original rebased
  `moov`/`mdat` sample tables in RAM for every retail MooV: `Title.MooV`
  (Cinepak/`twos`), `Swoosh.MooV` (QuickTime Animation/`raw `), and
  `Credits.MooV`/`Ending.MooV` (Cinepak/`raw `). It derives every admitted
  sample span from original `stsc`/`stsz`/`stco` tables, with no extraction,
  placeholder frames, or host codec involved.
- `Swoosh.MooV` now has a dependency-free native playback lane: the original
  16-bit QuickTime Animation (`rle `) samples and `raw ` PCM packets are read
  directly from the admitted sample spans. Twelve authentic consecutive frames
  and their timing/audio handoff pass from the packed retail ZIP in RAM.
- All four supplied retail Macintosh movies now decode natively in memory:
  the 24-bit Cinepak (`cvid`) vector/codebook stream and signed `twos` PCM
  used by `Title.MooV`, the 16-bit Animation RLE/`raw ` lane in `Swoosh.MooV`,
  and Cinepak/`raw ` for `Credits.MooV` and `Ending.MooV`. The full authentic
  four-movie regression advances twelve frames from every stream with source
  timing and audio; the old FFmpeg configuration is no longer part of the
  Firestaff build or runtime.
- The M11 real-media route now drives the verified native Title film to its
  source completion, opens and closes Credits through its authentic input
  event, then enters the retained GAME_LOAD candidate. It must click the
  original 224×136 mirror viewport before the source DB3 mirror selection
  publishes the New Game STARTEND session; this preserves the two-stage
  source sequence instead of treating Enter as a synthetic champion choice.
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
  remains memory-owned through the native installer path. A current clean
  local probe contradicts the earlier frame-acceptance claim: movement works,
  but the initial M11 dungeon frame is rejected. See `TODO-dm2.md` for the
  reproduced receipt and required follow-up.
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
- DM2 Macintosh real-media tests now select retail and First Chapter archives
  by their locked SHA-256 identities across the original and duplicate-suffixed
  filenames. This keeps retail boot coverage on the full archive and the
  negative admission test on the demo when both share a basename.
