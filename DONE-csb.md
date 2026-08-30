# Firestaff DONE — CSB

Reviewed 2026-08-29. Completed work only.

- M11's CSB query-world handoff now consumes the exact verified
  `CSB_V1_DungeonData::raw_data` bytes retained by the selected boot reader.
  It no longer attempts to reopen an STX/ADF/archive locator as a loose
  `DUNGEON.DAT`. Real Atari STX and Amiga ZIP → ADF launcher regressions prove
  title/start-menu handoff, native mirror/candidate flow, and the first
  runtime frame with no extracted game-data file; FM Towns uses the same
  bounded-memory source contract.

- The supplied CSB Utility Disk's supported single-member LZMA2 7z profile
  is now decoded by Firestaff in bounded memory. Its Atari STX member and
  nested files, including `START.PRG`, can be read without `7z`, `bsdtar`, or
  another external tool at runtime and without materializing game data on
  disk. The reader validates both 7z header CRCs, the extracted member CRC,
  declared sizes, and the member name; unsupported 7z structures remain
  closed rather than falling back to an external extractor. The M12
  regression runs against the supplied archive with external archive tools
  disabled.
- The supplied Amiga ZIP→ADF `Graphics.DAT` now has an in-memory native
  runtime-family receipt: inventory/panel, pit and field, stairs, wall and
  floor ornaments, and doors all decode through the big-endian IMG1 consumer
  without a PC3.4 fallback or an extracted game-data file.
- The active CSB Amiga runtime-graphics CTest is now that ZIP → ADF → OFS
  receipt itself (`csb_v1_amiga_runtime_graphics_real`), rather than a
  separate loose-`GRAPHICS.DAT` test that skipped against the supplied media.
- The supplied CSB Amiga A-disk now has a direct native graphics-format
  receipt. `test_csb_v1_amiga_graphics_dat` reads `Graphics.DAT` from the
  original ZIP → ADF chain in RAM through the AmigaDOS OFS reader, validates
  its Amiga record table and known language/version identity, then decodes
  the source C017 inventory panel at its original 224×136 dimensions. This
  is source-format evidence only; composed-screen capture parity remains
  active work.
- Since this original ADF route is staged, its graphics test no longer
  constructs positive IMG1 or `Graphics.DAT` stand-ins. Positive parsing and
  decode evidence comes only from the admitted ADF member; malformed-header
  checks remain as fail-closed boundaries.
- The supplied Amiga A31 archive now has the same first-runtime input matrix
  evidence as the Atari and FM Towns releases. Each direction, strafe and
  action is launched in a fresh native ZIP → ADF session, with the original
  title owner and campaign state retained in memory; no generated save or
  replacement dungeon is used.
- The native Atari STX CLI route verifies original title startup, runtime and
  start-menu entry using the supplied campaign media. Its source-owned input
  regression now also covers backward movement, both turns, both strafes and
  action from independent original STX sessions, each retaining a nonzero
  native viewport receipt. The unchanged initial strafe/action position is a
  recorded source result, not synthetic content.
- The supplied French Atari preservation ZIP now follows its original
  `ZIP → STX` path in RAM.  Its protected sector descriptors retain their
  logical order even when capture offsets are skewed, so the verified shared
  `GRAPHICS.DAT`/`DUNGEON.DAT` pair reaches title, start menu, and first native
  movement without a replacement image or disk extraction.
- The M12/M11 Atari STX route now retains the hash-verified original
  `ANIMATE.SCR`/`ANIMATE.DAT` container when the selected runtime cache holds
  only `GRAPHICS.DAT`/`DUNGEON.DAT`; 50 Hz VBlank cadence, final FTLCODE
  handoff and first native HUD/viewport frame are exercised against that media.
  The completed 224×136 source-owned viewport publishes a nonzero FNV-1a
  receipt without being promoted as a PC F0128 runtime-session receipt.
- On that verified Atari route, native Enter/Accept now crosses the retained
  ANIM.C → FTLCODE handoff instead of being lost in the unrelated PC startup
  dispatcher. A requested CSB PC platform is explicitly rejected before any
  media or cache selection, because no original DOS/PC release exists.
- The Atari M12/M11 handoff regression now fails safely and precisely when a
  selected package cannot be opened, instead of cascading or crashing.
- The FM Towns F31 start-menu receipt now identifies its source-owned
  `TITLE.ANM` palette/frame handoff with a nonzero frame hash. An explicit
  F0435 user-save launch is kept distinct and proves the admitted C03
  executable handoff instead of claiming that it replayed the title.
- The current real-media launch matrix covers Atari STX, Amiga ZIP → ADF, and
  both English and Japanese FM Towns selection. Each route was exercised from
  its original source through title, normal start-menu launch, and the first
  native `UP` movement into the campaign. This is launch/runtime evidence,
  not a claim of complete campaign playthrough parity.
- Direct Amiga ZIP → ADF launches now identify A31M from `TITL.DAT` in the
  exact same virtual ADF as the selected `GRAPHICS.DAT`, rather than requiring
  an M12 cache leaf. The start menu publishes the source-owned `TITL.DAT`
  boundary and hash, while unrelated outer-archive or host files remain closed.
- Amiga A31E and A31M original ZIP → ADF media are read entirely in RAM.  The
  A31E direct C03 handoff verifies `APPB.FTL` and `BJELoad_R` through the same
  selected ADF as `GRAPHICS.DAT`, reaches `csb-entrance-0` with the original
  A31E hash, and does not create an asset-cache copy.  A31M's original
  `TITL.DAT`, `APPB.FTL` language page and `KAOS.FTL` continuation now use
  the same source locator and pass the real CLI and start-menu route into
  runtime after the old extracted cache is absent. The focused real-media
  regression uses virtual source locators rather than the legacy
  materialization API.
- The Atari R1 Hint Oracle now reads its hash-discovered `MINI.DAT` member
  directly into RAM before native GAMEBLOCK decoding.  It no longer writes an
  extracted Utility Disk save into an asset cache; a real STX CLI regression
  covers the direct `--csb-hint-oracle` route.
- CSB Utility Disk import now verifies the original archive member directly
  in bounded RAM.  UTIO.C sector 7 is read through the native STX transport
  reader when required, so the supplied Atari Utility Disk neither needs nor
  creates a transient ADF cache.
- The retired game-media disk-materialization switch now fails configuration
  and the media-admission source rejects direct activation.  Packed CSB
  formats therefore remain source-owned and in-memory only; a format without
  a native reader fails closed rather than creating a cache copy.
