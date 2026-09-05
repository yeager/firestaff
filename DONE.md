# Firestaff DONE — cross-game completed work

Reviewed 2026-08-25. This ledger contains completed, evidence-backed work
only. Active work is in `TODO.md` and `TODO-<game>.md`.

- 2026-09-05: The optional CSB source-text extraction tool now links CMake's
  Iconv target explicitly, resolving the missing macOS iconv symbols. Its
  Linux ARM64 build passes. Runtime game targets do not depend on Iconv.

- 2026-09-05: Savegame Editor bundling selects Python before compiling
  gettext catalogs. The complete Linux ARM64 bundle and its self-test pass.
  Translation calls outside f-string expressions preserve extraction with
  older gettext versions; the current full catalog check passes.
- 2026-09-05: Standalone dungeon-loader tests link the authentic FM Towns
  receipt implementation. The creature-map test and all 28 scroll text
  assertions pass locally after the CI linker failure was reproduced.

- DM1 D3--D1 ordinary explosion rendering now follows F0115 call ownership:
  every MAIN/DOORPASS callback restarts its C15 list after packed-cell
  material, so door rear explosions render before F0111 and front-pass
  explosions render afterward. PC 3.4 item-696 C3014/C3031 anchors clip real
  GRAPHICS.DAT material to the 224x136 viewport; the global replay is removed.

- DM1 F0124's post-transaction Thieves Eye D1C wall restore now executes
  through a dedicated D1C square-tail scheduler callback between D1C and
  F0125. This covers wall routes that correctly have no F0115 step and removes
  the last direct restore call outside the verified callback stream.

- DM1 F0125/F0126 now rasterize every ordinary D0L/D0R C15 record inside
  the owning F0115 callback. The runtime receipt retains source cell and
  centered state, rotates non-centered cells relative to party direction,
  and places real F0114 material at the PC 3.4 item-696 C3014/C3031 anchors;
  unsupported back-cell projections and missing source graphics fail closed.

- DM1 F0128 now consumes F0125/F0126 D0L/D0R F0115 creature material from
  authentic G2033 rows 11/12 and each following F0113 teleporter field in the
  same scheduler callback stream. Negative G2028 continues to suppress side
  items/projectiles; no host placement or substitute bitmap was introduced.

- DM1 F0128 now consumes F0127's complete D0C `F0115_MAIN` transaction from
  its verified `C0x0021` callback step. Real PC 3.4 floor-item, projectile and
  restarted C15 explosion consumers all finish before F0113; the former direct
  item/projectile calls and source-invalid post-field explosion call are gone.

- DM1 F0128 now invokes the real PC 3.4 D1C Hall-of-Champions C346/C026
  mirror transaction from its owning F0107 scheduler callback. The separate
  post-F0107 mirror call was deleted, while the existing fail-closed C127,
  backing and portrait receipts remain the only material authority.

- DM1 F0128 F0104 door frames now rasterize in their own scheduler callback
  phase before optional F0110 and F0111. Centre and side F0111 helpers no
  longer duplicate frame pixels; open-door and D3L2/D3R2 frames use the same
  original GRAPHICS.DAT-backed plans.

- DM1 F0128 F0110 door buttons now have one callback owner at their exact
  source boundary between frame and F0111. The scheduler emits F0110 only for
  F0117 D3R and centre F0118/F0121/F0124; the direct D3R and centre replay was
  deleted. A real PC 3.4 HoC door proves F0108, DOORPASS1, F0110 and F0111
  callback receipts without modifying the dungeon or extracting its media.

- DM1 F0128's complete D3--D1 `F0104_WALL_MATERIAL` family now rasterizes
  through the verified per-square callback at the owning outer, side or centre
  square. The three later wall replay call sites were deleted; all pixels still
  come from the mounted PC 3.4 `GRAPHICS.DAT` wall consumers.

- DM1 F0128's D3--D1 foreground tail is now scheduler-callback owned. For
  each exact target square the verified plan dispatches F0104 pit/stairs,
  F0108 floor ornament, F0112 ceiling pit, F0115 `MAIN`/`DOORPASS2`, and
  F0113 field operations after that square's F0111 door boundary. The former
  hand-written per-square span loop was deleted, leaving one operation owner
  and preserving `DOORPASS1 -> F0111 -> DOORPASS2`. A new receipt is asserted
  by the authentic PC 3.4 HoC ZIP regression; the native ZIP CLI boot and the
  asset-free no-fallback scheduler gate also pass.

- DM1 F0128 now rasterizes the terminal F0125/F0126/F0127 D0
  F0104/F0112/F0113 primitives from the verified scheduler execute callback;
  the former per-square D0 replay owner was removed. F0127 remains split at
  its authentic boundary so D0C floor items/projectiles precede its F0113
  field overlay. A frame-local receipt counts callback-owned source steps, and
  the real in-memory PC 3.4 HoC regression proves that the mounted archive
  executes this path while retaining four-direction, F0115 and mirror/HUD
  behavior.

- DM1 F0128's complete pre-F0111 F0115 `DOORPASS1` family now rasterizes
  from the verified scheduler execute callback at each existing square-local
  door boundary. The hand-written span scan was deleted; the callback consumes
  the scheduler's exact square and cell-order word for center, side and D3
  outer lanes. A callback receipt and a real PC 3.4 ZIP dungeon-door probe
  prove that an authentic door produces exactly one callback-owned rear
  partition before its F0111 panel.

- DM1 PC 3.4 HoC launch-to-mirror production route is covered without a test
  teleport. The real archive supplies the collision graph and C127 owner; BFS
  derives a route from retail tuple `(map 0, 1,3,South)`, public M11 input
  replays it, then the test draws and clicks the published C026 rectangle and
  completes C040/C160 through the pointer owner. This proves the earlier
  zero-valued entrance summary fields are not a gameplay-path failure.

- DM1 FM Towns native title cadence now follows the source-owned EDM.EXP
  schedule: only the 18 zoom frames wait one 60 Hz VBlank, followed by the
  separate two-VBlank return guard. Cumulative 16/17 ms host delays produce
  exactly 300 ms for the zoom and 333 ms overall instead of the former 374 ms
  drift. `dm1_v1_fmtowns_title` and the authentic in-memory FM Towns ZIP
  CLI/menu/TMENU-to-EDM/CDDA/input test both pass.
- DM1 ReDMCSB `SPELFAIL.C:F0410` failures now publish through the real
  `TEXT.C:F0051/F0047` C015 message state instead of remaining host telemetry.
  The source fragments are appended separately, use C04 cyan, synchronize the
  70-tick expiry to the cast tick, and retain exact practice, meaningless-spell
  and empty-flask text. The focused text-state and authenticated M653 C015
  consumer gates pass. `MENU.C:F0381` now also owns the visible FLIP heads/
  tails result, including the source newline, bounded `@p` replacement, C04
  colour and 70-tick lifetime; malformed replacement tokens fail closed.
- Normal play now loads all five isolated game PO domains (`dm1`, `csb`, `dm2`,
  `nexus`, `theron`) in addition to the startup-menu domain, using the language
  selected in the launcher instead of incorrectly re-reading only the host AUTO
  locale. DM1 retail-derived object/action names, sensor/timeline messages and
  scroll text are resolved only at their final presentation boundary, leaving
  original data untouched and using the exact decoded source string as fallback.
  F0410 uses portable named fields so translations can reorder champion and
  skill names; a focused real-catalog gate verifies lookup, expansion and exact
  fallback, while the original English C015 source-lock remains green.
- DM1 C015 storage/wrapping now advances by Unicode codepoint rather than UTF-8
  byte, and its Original renderer keeps M653 for ASCII while drawing supported
  Latin-extended translated glyphs through the built-in Unicode table. Unknown
  codepoints produce one replacement glyph rather than corrupt M653 atlas reads.
- CSB's synthetic, unwired 33-key catalog—including the nonexistent `CSB PC
  3.4` save/edition label—was replaced by 220 unique C699/M564 msgids extracted
  in memory from the supplied original Amiga archive. The extraction tool
  records the admitted GRAPHICS/DUNGEON hashes, rejects media without both
  authenticated tables, emits valid POT, and never writes extracted game data.
  Exact matching DM1 translations seed CSB locale entries without cross-domain
  runtime fallback; every unmatched entry retains its source-text fallback.
- CSB FM Towns source-text extraction now follows the explicit CHTWE/CHTWJ
  launch selection and reads M564 item 694 plus executable-owned DYNA_BUTTONS
  directly from retained CD members. English and Japanese both produce
  `msgfmt`-valid UTF-8 POT output; F31J follows its source Shift-JIS NUL/byte-1
  object-name rule instead of treating Japanese high bytes as row delimiters.
- DM1 PC 3.4 inventory source-lock verification now reads the canonical
  `GRAPHICS.DAT` member directly from the original ZIP, with no extracted
  test copy. Mouth-consumable fixtures carry matching raw C08/C10 records and
  the chest/scroll fixture carries a byte-accurate raw C09/C10 chain, so the
  hardened runtime cannot silently fall back to decoded test structs. The
  focused pickup/inventory/eye/mouth/drop group passes 15/15, including the
  formerly skipped real-media F0731/F0734 material gate.
- Native M12/M11 launcher and boot-probe infrastructure is in production and
  accepts original data without runtime emulator dependencies.
- Real-media focused regressions cover DM1 PC 3.4, DM2 FM Towns, CSB Atari
  STX, Theron JP CUE startup, and Nexus title-resource intake.
- CI enforces media hygiene, native builds and deterministic checks on Linux,
  macOS and Windows.
- DM1/CSB F0128 D3L2/D3R2 door composition consumes the authenticated rear
  Thing pass before the F0676/F0677 door occluder and the front Thing pass
  afterward; focused DM1, CSB and live HoC render gates pass.
- CSB product support and documentation expose only the original Atari ST,
  Amiga and FM Towns editions. Retired PC-labelled probes, targets and
  documentation rows are removed; the complete CSB-labelled regression was
  green (155/155) before removal of the redundant negative PC test, and the
  resulting focused launcher/runtime suite is green (8/8).
- High-resolution nearest-neighbour RGBA expansion now uses exact quotient /
  remainder coordinate stepping instead of per-pixel integer division. The
  output mapping matched the former formula over 1,974,784 tested coordinates;
  a 320x200-to-3840x2160 arithmetic microbenchmark improved by 1.38x, while
  a per-frame 256-entry RGBA lookup table also removes repeated palette
  resolution from every expanded output pixel. The DM1/CSB filter, resolution
  selector and runtime-popup tests remain green.
- CSB FM Towns no longer carries SWITCHTW's C26 menu palette into Entrance.
  The native handoff locates and admits C28_ENTRANCE_CSB (G8174) inside each
  already hash-verified CHTWE/CHTWJ executable and presents those six-bit DAC
  values. The supplied ZIP proves the real offsets as `0x35898` (English) and
  `0x35a78` (Japanese); both archive tests and the English SWITCHTW-to-C004 M11
  route pass without extracting game files.
- CSB FM Towns now also releases temporary C28 after the Prison doors and
  selects the authentic C00_LIGHT0--C05_LIGHT5 dungeon palette. All six
  contiguous G8151--G8156 COLOR_DEF rows are admitted from the same verified
  CHTWE/CHTWJ executable; real offsets are `0x35494` (English) and `0x35674`
  (Japanese). Both full M11 language routes prove that the first live dungeon
  frame publishes a LIGHT palette rather than retaining C28.
# CSB Atari source text receipt

- Bound the supplied S21E STX directly to its authentic 563-item DMCSB1
  GRAPHICS.DAT and decoded the 1848-byte item-556 M564 stream in memory. All
  199 source object-name rows authenticate. F0913 now decodes the complete
  145,418-byte START.PAK body without over-reading fourteen words, while the
  44-row G0490 action table is uniquely admitted from GRAPHICS.DAT C560 item
  560 at offset 0x174 as required by STARTUP2 F0750. The resulting 221 unique
  player strings come only from the selected STX; no extracted game-data file
  or cross-platform name table is used.
  The live Atari boot/runtime now uses that native IMG/LZW M564/C560 path as
  well: BLOCK, FUSE and object names survive STX boot-to-runtime without the
  invalid PC record-694/699 decoder or compiled DM1 fallbacks.
  FM Towns English and Japanese likewise bind live M564 from retained packed
  CDATA/CJDATA GRAPHICS.DAT item 694 and G0490 from hash-admitted CHTWE/CHTWJ
  DYNA_BUTTONS at `0x29f50`/`0x2a0ec`. All 44 rows and N/X sentinels are
  validated before use, with the edition's English high-bit or Shift-JIS
  decoder and no loose-file, PC-record or cross-language fallback.
  Amiga A31M proves the corresponding big-endian DMCSB2 M564/C699 binding
  through the real ZIP-to-ADF M12/TITL/APPB/KAOS/M11 handoff. A31E/A35E,
  where G0490 is compiled rather than a GRAPHICS.DAT record, accept it only
  from the exact hash-verified APPB.FTL in the same admitted disk context.

- DM1 C080 leader-hand throws now preserve the original screen coordinates
  passed by CLIKVIEW.C F0377 into F0375. Viewport-origin subtraction remains
  confined to the later clickable-box loop, restoring the complete F0329 to
  F0328 throw route. F0190 death smoke also uses source-owned F0887/C15 in a
  loaded real dungeon and a bounded F0821/timeline fallback only in test/probe
  worlds with no source explosion table. The action/stamina matrix improved
  from 1280/64 to 1384/0. Its stale throw, G0243 smoke, F0381 lifetime and
  F0401 fear-delay oracles were corrected directly from source arithmetic.
  CLIMB DOWN now applies MOVESENS.C's rope stamina cost without generic pit
  damage. Projectile F0213 impacts now publish their authentic C25 lifecycle,
  including real C15 ownership where available; poison C25/C75 ordering and
  harm-non-material killed-all behavior are source-verified. Nine focused
  source/raw-data regressions pass.
