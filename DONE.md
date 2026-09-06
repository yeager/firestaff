# Firestaff DONE — cross-game completed work

- 2026-09-06: DM1 Atari/Amiga runtime equipment masks match original
  graphic-559 G0237 words for 606 allocated objects in each of five
  editions; all source-mask and transfer checks pass.

- 2026-09-06: Atari/Amiga real-media verification skips only absent paths;
  archive-open errors now fail. Negative checks confirm ENOENT returns 77
  and ENOTDIR returns 1 for both test binaries.

- 2026-09-06: Five original DM1 Atari/Amiga editions pass 230,280
  place/pickup transactions across both hands and all 17 backpack slots
  in Original/V2.1, including identity checks after mouse release.

- 2026-09-06: Original DM1 Atari/Amiga scrolls now exercise action-hand
  placement and pickup, checking held/slot identity before and after mouse
  release in five editions and Original/V2.1; all tests pass.

- 2026-09-06: Original C033 border pixels now verify all 30 DM1 inventory
  slots across five Atari/Amiga editions in Original/V2.1; all tests pass.

- 2026-09-06: Fixed DM1 Atari/Amiga inventory admission of original padded
  C033 graphics and scroll baseline conversion. Five real-media editions
  pass 350 scroll/mode raster checks in Original/V2.1.

- 2026-09-06: Corrected DM1 PC3.4 scroll text position from original C696
  and F0341/F0644 evidence. All 35 original scrolls now pass the corrected
  raster oracle in Original/V2.1; other media coordinates are unchanged.

- 2026-09-06: Fixed DM1 scroll cells to use the original six-column white
  background; three original-scroll raster failures are resolved and the
  complete PC3.4 corpus passes in Original/V2.1.

- 2026-09-06: DM1 scroll raster evidence now explicitly requires loaded
  font data and nonzero glyph ink; the original-media corpus passes.

- 2026-09-06: DM1 original-scroll tests now compare panel/text raster
  placement in Original/V2.1 using original C023 and M653 materials;
  shared layout/font decoding and transparent backgrounds remain separate.

- 2026-09-06: DM1 PC3.4 scroll eye tests now check original C023 border
  pixels in Original/V2.1; the complete original-object corpus passes.

- 2026-09-06: DM1 original-scroll eye checks now assert source text equality
  against each scroll's C02 reference, not merely successful panel routing.

- 2026-09-05: DM1 legacy original-media launch matrix now covers V2.0 as
  well as Original/V2.1: all 36 CLI/menu launches across six editions pass.

- 2026-09-05: DM1 JDM title verification distinguishes absent optional media
  from invalid supplied media; positive, failure and skip paths are checked.

- 2026-09-05: DM1 FM Towns original EN/JP launch/input matrix passes;
  Japanese title-receipt assertions remain active in Release test builds,
  and unavailable media no longer counts as a successful title test.

- 2026-09-05: DM1 PC3.4 passes same-chest refresh and cross-chest owner
  transitions in Original/V2.1 alongside the full original-object corpus.

- 2026-09-05: Rebuilt Firestaff passes Atari/Amiga CSB CLI startup; original
  Amiga ZIP/ADF C025 admission now verifies exact dimensions and pixel range.

- 2026-09-05: Atari CSB chest comparisons now include all 10,512 panel-area
  pixels and the 848 transparency-key positions; three presentation modes
  pass against original materials and the pre-open viewport.

- 2026-09-05: Atari CSB chest composition now uses C025 and original icon
  atlas crops. Source-material pixel checks pass in Original, V2.0 and V2.1;
  transparency-background and emulator parity are not yet established.

- 2026-09-05: Original Atari CSB chest pickups pass in three presentation
  modes using C232-relative native input geometry. Chest rendering remains
  a separate open requirement.

- 2026-09-05: Atari CSB passes 22,797 original-object backpack drags across
  Original, V2.0 and V2.1, checking source/destination/leader-hand ownership.

- 2026-09-05: Atari CSB passes 40,230 occupied-slot exchange/rejection checks
  using distinct original objects and original C559 acceptance masks, across
  Original, V2.0 and V2.1, in addition to empty-slot checks.

- 2026-09-05: The independent original C559 inventory oracle also passes in
  V2.0 and V2.1. Rebuilt Firestaff passes the original Atari STX CLI startup
  and scripted native runtime-input regression after the mouse-release fix.

- 2026-09-05: Independently verified Atari CSB allowed-slot values for all
  447 allocated objects against original C559 bytes; Original mode passes
  the full 30-slot input sweep using those bytes as the acceptance oracle.

- 2026-09-05: Atari CSB original-object input coverage now spans all 30
  inventory slots, including equipment rejection: 40,230 checks pass across
  Original, V2.0 and V2.1. Independent object-mask decoding remains separate.

- 2026-09-05: Original Atari CSB backpack verification now covers 447
  allocated original objects across 17 slots in three presentation modes;
  all 22,797 pickup/replacement roundtrips pass.

- 2026-09-05: Fixed an original-media Atari CSB inventory regression where
  releasing the mouse undid pickup. The new original-weapon roundtrip failed
  before the fix and passes in Original, V2.0 and V2.1 afterward.

- 2026-09-05: Hardened the CSB M11 HUD regression: failure to start selected
  media is now a test failure, not a successful skip. Invalid presentation
  selection uses CTest's skip code. Original Atari MINI.DAT passes the
  rebuilt test in Original, V2.0 and V2.1.

- 2026-09-05: Original CSB FM Towns EN/JP tests also verify reopening an
  already-open chest with a hole and switching chest owners while holding
  an original resident. Both container lists and the held item survive in
  Original and V2.1; save/resume is outside this interaction check.

Reviewed 2026-08-25. This ledger contains completed, evidence-backed work
only. Active work is in `TODO.md` and `TODO-<game>.md`.

- 2026-09-05: Rebuilt Firestaff after the shared CSB chest-slot change and
  passed six original-media regressions: DM1 object names/full inventory
  corpus plus CSB Atari ST, Amiga, FM Towns English and Japanese startup
  paths. Startup success does not establish Atari/Amiga chest-click parity.

- 2026-09-05: CSB M11 now retains open G0425 slot positions while keeping
  runtime-linked container contents synchronized. Original F31 EN/JP tests
  pass pickup, same-slot release, permitted replacement and close-order
  checks for all 60 residents in Original and V2.1. Pre-placed equipment
  lacking the container mask remains correctly rejected on reinsertion;
  only those rejected test placements are reset. The eye-close regression
  also passes. Atari/Amiga geometry and native save-resume remain separate.

- 2026-09-05: Fixed CSB FM Towns chest pointer admission. Boot retains
  C537..C544 from the original item-696 C106/C101/C100 graph alongside the
  30 inventory boxes; input admits chest children only for an open chest,
  and same-slot release does not repeat the exchange. Both original EN/JP
  tests pick up all 60 residents across 14 containers with independent chain
  restoration. Continuous replacement and open-slot persistence remain open.

- 2026-09-05: The CSB FM Towns original-media M11 test now traverses
  GAME/Enter into the original MINI.DAT party/dungeon before inventory
  inspection. Both English and Japanese pass with one original champion,
  14 readable containers and 60 visible residents. This establishes media
  ownership for subsequent chest input tests, not slot-persistence parity.

- 2026-09-05: Rebuilt the Firestaff application with the live chest fixes.
  The original English DOS archive CLI/menu regression passes, including
  Original, V2.0 and V2.1 launch modes. Unreleased notes now distinguish the
  verified interaction repair from the deferred changed-dungeon save gap.

- 2026-09-05: Fixed two live DM1 chest interaction faults: same-slot release
  now resolves C101/G0456 without repeating the press exchange, and open
  G0425 slots retain holes until F0334 relinks on close. The original PC3.4
  corpus preserves all 43 chest residents through pickup/replacement/close
  in Original and V2.1, and the full 611-record inventory matrix passes.
  Eye-close and HoC regressions pass; snapshot preparation closes the chest
  and publishes its current chain. Full changed-dungeon save persistence
  remains an explicitly deferred, separately reproduced limitation.

- 2026-09-05: Original PC3.4 scroll records now exercise the live inventory
  eye route in Original and V2.1 after the full slot matrix. Each opens its
  own scroll panel, not a generic object dialog, renders through M11 and
  retains the held Thing after release. The combined corpus passes; this
  checks routing and ownership, not text/pixel equality with an emulator.

- 2026-09-05: The 611-record PC3.4 inventory matrix additionally seeds a
  distinct, slot-admissible original resident in every slot. Original and
  V2.1 swaps preserve both Thing identities in both directions; denied
  incoming objects preserve the resident and held Thing across press/release.
  The combined empty/occupied 30-slot matrix passes. This uses controlled
  in-memory placements, not a proof of floor/chest ownership or all item pairs.

- 2026-09-05: Expanded the 611-record PC3.4 mouse corpus to all 30 inventory
  slots in Original and V2.1 (36,660 object/slot/mode combinations). Source
  G0038 masks determine admission; permitted placements roundtrip and denied
  placements preserve the held Thing across press/release. All checks pass.
  Occupied-slot swaps and chest contents remain separate unfinished coverage.

- 2026-09-05: The original PC3.4 object corpus now exercises live action-hand
  placement and retrieval through mouse press/release for all 611 decoded
  weapon, armour, scroll, potion, container and junk records in Original and
  V2.1. All roundtrips preserve Thing identity and single-exchange ownership.
  Existing name/icon checks remain; no replacement game records are created.
  Missing media is explicitly skipped. This does not cover every inventory
  slot, chest interaction, or emulator-rendered pixel equivalence.

- 2026-09-05: The original PC3.4 archive's live HoC pointer sweep now runs
  in both Original and V2.1, requiring all 24 candidates rather than any
  nonempty subset. Both modes select all 24 through the rendered input path;
  the existing resurrection/reincarnation regression and side/depth viewport
  material sweep pass. The test now honors TMPDIR instead of hardcoding a
  temporary location. This does not establish emulator pixel parity.

- 2026-09-05: Both optional real-media HoC mirror tests now return CTest's
  explicit skip code when no data directory is selected, rather than a false
  pass. With the existing French DOS original files selected, the directional
  test checks 24 sensors and 24 distinct portraits; the material test also
  passes. These are sensor/material-plan checks, not rendered-pixel parity.

- 2026-09-05: CSB Amiga and FM Towns EN/JP runtime-transition tests now
  explicitly cover V1 and V2.1. Both modes retain Amiga's first UP movement
  and the Towns original MINI.DAT map/party seed through Game/Enter input.
  All three original-media scripts pass; complete rendering parity is open.

- 2026-09-05: CSB Atari ST original STX regression explicitly checks both
  V1 and V2.1 through CLI and menu to a loaded dungeon, retaining the
  requested presentation mode. Existing title, input and pointer-launch
  assertions pass in the same run; audiovisual parity remains separate.

- 2026-09-05: Atari audio rejection clears the previous accepted flag,
  hash, period and sample count. Regression tests cover invalid fingerprints,
  short SND1 streams and recovery on the next valid request; original DM1
  Atari EN/DE/FR audio corpus tests continue to pass.

- 2026-09-05: Original/Modern CLI/menu coverage extends to original Atari
  DE/FR and FM Towns JP (12 additional combinations). Japanese launches
  must retain both JDATA and JDM fingerprints, preventing an English
  fallback from satisfying the presentation-mode regression.

- 2026-09-05: The native Paula-volume PCM entry point supports all 0..64
  levels instead of rejecting everything except 64. Tests lock all 65 gains,
  including silence, without changing sample cadence; original DM1 Amiga
  2.0/HD transport regressions pass. Asymmetric stereo remains unimplemented.

- 2026-09-05: DM1 Amiga uses an explicit MEDIA413 sound table instead of
  deriving periods from the PC table. Water elemental attack record 571
  now uses source period 138 rather than PC's 112. Original 2.0/HD corpus
  checks validate the corrected sample cadence across all engine events.

- 2026-09-05: DM1 original Atari English, Amiga 2.0 and FM Towns English
  media pass explicit V1/V2.1 launches through CLI and menu (12 combinations).
  The new real-media tests check selected mode and loaded runtime rather
  than accepting a default-mode launch as evidence for both presentations.

- 2026-09-05: Original DOS DM1 archive boot checks now explicitly select
  V1, V2.0 and V2.1 through both CLI and menu, asserting the resulting mode,
  original graphics fingerprint and loaded runtime. This closes a test gap
  where a successful default-mode launch did not prove mode selection;
  it does not establish filter/rendering parity or other-platform coverage.

- 2026-09-05: Rebuilt CSB startup regressions pass with original Atari STX,
  Amiga and FM Towns EN/JP media after the audio fixes. These cover their
  existing CLI/runtime and, where included, menu/input assertions; they do
  not establish complete gameplay or emulator audiovisual parity.

- 2026-09-05: The data-free Amiga audio regression now locks byte-sample
  cadence and signed amplitude timing, catching the previous extra clock
  division independently of availability of original game media.

- 2026-09-05: Rebuilt CLI/menu/input regressions pass after the legacy audio
  changes: original DM1 DOS English, Atari EN/DE/FR, Amiga 2.0/HD and FM
  Towns EN/JP. The Towns check independently reloads both language programs
  for all seven directional/action commands. These are startup/input checks,
  not emulator pixel, audio-waveform or complete gameplay parity proofs.

- 2026-09-05: Legacy DM1 startup clears the PC SND3 bank without attempting
  to parse Atari/Amiga/FM Towns media as PC audio. Original-media startup
  tests assert an empty PC bank for these editions; the PC3.4 regression
  continues to require all original SND3 entries.

- 2026-09-05: F31 EN/JP audio corpus tests independently locate each of
  the 35 selected records in the original container and compare payload
  bytes. Oversized sample counts in private RAM copies are rejected for
  every event with empty output; original archives are never modified.

- 2026-09-05: CSB FM Towns reads the BE16 PCM sample count without requiring
  exactly two unused tail bytes. Original explosion record 675 has 3970
  samples in 3973 bytes and was incorrectly rejected. EN/JP original-media
  tests now cover all 35 sound events and compare every host output sample;
  declared samples must still fit entirely within the source record.

- 2026-09-05: DM1 Amiga local sound events select original PCM records,
  skipping the two-byte header as SOUND.C requires. Original 2.0/HD tests
  compare every resampled byte value and period across all 35 engine events.
  The common Amiga PCM transport no longer incorrectly halves the audio
  clock a second time. The current clock is NTSC; PAL selection, stereo
  attenuation and channel arbitration are not covered by this change.

- 2026-09-05: DM1 FM Towns local effects use retained F20 unsigned PCM,
  not the PC SND3 bank or F31 signed bytes. DATA.C MEDIA507 selects the
  22 original records; TOWNSIO.C supplies BE16 length, the 31936-sample
  limit and 5500 Hz cadence. EN/JP original-media checks compare every
  host output sample to the selected source record. No BIOS or extraction
  is required; original channel/timing/distance parity remains open.

- 2026-09-05: CSB FM Towns PCM host gain uses the original 1..127 driver
  domain instead of saturating it with PC's 1..3 divisor. Original EN/JP
  archive tests cover all 127 gain steps; direct local effects request 127.
  This verifies transport scaling, not distance-event or emulator parity.

- 2026-09-05: DM1 Atari gameplay dispatch selects original SND1/PSG instead
  of the PC SND3 bank. ReDMCSB event-index translation preserves missing
  Atari effects as silence and the entrance Timer-A period as 145. Original
  EN/DE/FR transport checks cover every engine sound index and reject the
  three known short streams without generated markers. Original-emulator
  timing, arbitration and electrical-output parity remain unproven.

- 2026-09-05: Original Atari EN/DE/FR audio characterization verifies all
  22 transport payloads byte-for-byte against the DM1 graphics reader.
  Nineteen decode within their record boundaries; three known short streams
  are explicitly tested for safe rejection. This is not playback parity.

- 2026-09-05: Rebuilt post-M653-fix startup regressions pass on original
  Amiga 2.0/HD and FM Towns EN/JP media, including CLI/menu handoffs and
  the English/Japanese input matrices. The legacy corpus test additionally
  decodes all 532 admitted images for each FM Towns language and Amiga 2.0.

- 2026-09-05: DM1 legacy/Atari startup binds the raw original M653 font from
  retained media bytes instead of attempting the PC3.4 file-state loader.
  All 768 bytes match original Atari EN/DE/FR, Amiga 2.0/HD and FM Towns JP
  records; the PC3.4 object/pickup regression also passes. FM Towns system
  Kanji glyphs are a separate route and are not supplied by this M653 fix.

- 2026-09-05: The Atari bitmap API now enforces the same source-record
  classification as the production asset loader. Text, sound, font and code
  records remain available through raw reads but cannot enter raster decode.
  EN/DE/FR original-media checks and the Atari container unit test pass.

- 2026-09-05: The Atari original-media gate now raster-decodes every one of
  the 532 admitted image records and checks every output pixel is 4bpp.
  English 1.2, German 1.2 and French 1.3 all pass, in addition to their 563
  raw-record and 199-name checks. This is decoder coverage, not a same-state
  original framebuffer comparison or proof of viewport composition.

- 2026-09-05: Amiga M564 regression now uses M12's authenticated edition
  selection and native archive handoff. All 199 original object-name indices
  pass for the 2.0 and HD ZIP→ZIP→ADF packages. This validates name bytes,
  not inventory glyph pixels or the outstanding gameplay palette capture.

- 2026-09-05: Extended the Atari name/all-record gate to hash-selected German
  1.2 and French 1.3 original disk containers. Both verify all 199 names and
  563 expanded lengths, alongside English 1.2. Rebuilt native CLI/menu/input
  regression scripts pass for all three editions after the decoder change.

- 2026-09-05: DM1 Atari raw graphics now use ReDMCSB F0497's dictionary
  convention and F0496 repetition output instead of the incompatible generic
  LZW end-code route. M564 binds from the retained original Atari bytes.
  The authentic English 1.2 archive verifies all 199 object-name indices and
  all 563 expanded record lengths; Atari container and STX unit tests pass.
  Expanded-length checks are not a pixel-parity claim.

- 2026-09-05: CSB's public hand-name accessor preserves UTF-8 boundaries
  when copying the already-localized cached label into a smaller UI buffer.
  The hand/no-DM1-fallback regression passes with additional empty-output,
  exact-fit multibyte-character and buffer-guard checks.

- 2026-09-05: DM1 translated/Japanese object labels clip only at complete
  UTF-8 boundaries after full-source catalog lookup. The real JDATA first
  weapon hand-label test checks every output capacity and its guard byte;
  all original Japanese names/actions and the PC3.4 pickup/cursor regression
  pass. Original non-UTF-8 fallback labels retain their existing byte encoding.

- 2026-09-05: DM1 Japanese FM Towns actions now consume the authenticated
  JDM.EXP load-image pool at `0x243bc`, instead of falling through to PC3.4
  English action names. The receipt retains original CP932 bytes; M11 converts
  to UTF-8 at catalog lookup. All 44 names match the reviewed JDM pool, all
  199 real Japanese object-name checks pass, and the original-disc English
  startup/menu-owner regression passes. This does not establish glyph parity.

- 2026-09-05: Fixed DM1 legacy M564 binding to read the already-owned original
  GRAPHICS bytes instead of reopening a diagnostic display path through the
  PC3.4 container decoder. Authenticated Japanese FM Towns now uses F0031's
  NUL framing and native CP932-to-UTF-8 keys with expansion capacity and
  overflow rejection. All 199 JDATA names match their original indices;
  real PC3.4 hand/pickup tests and both-endian raw-record bounds tests pass.

- 2026-09-05: Corrected the historical pass627 capture guidance: accept an
  authenticated same-state original reference independently of whether its
  pixels match Firestaff. Removed the unsupported assertion that no renderer
  changes could be needed. Source review also identified the open F20J M564
  name-framing defect, now recorded in the DM1 work list.

- 2026-09-05: Original-media startup/input regressions pass for DM1 DOS 3.4,
  Atari EN/DE/FR, Amiga 2.0/HD, FM Towns EN/JP, and CSB Atari/Amiga/FM Towns
  English. The FM Towns DM1 gate now independently checks all seven input
  commands against Japanese JDM/graphics fingerprints as well as English EDM.
  These are native boot and bounded input checks, not original pixel parity.

- 2026-09-05: Added 216 authentic FM Towns Japanese CSB catalog keys to
  every CSB locale, including 39 reviewed Swedish action translations.
  All 218 extracted keys pass the native PO loader lookup/fallback check;
  catalog regeneration and completion statistics are current.

- 2026-09-05: CSB object-name presentation obtains the complete original
  name before converting its encoding and looking it up in the catalog.
  Small UI buffers can no longer change the lookup key or split a UTF-8
  character during final clipping. The native engine builds and Japanese
  FM Towns CLI/menu startup passes with the original CD archive.

- 2026-09-05: Native CP932 conversion now binds CSB Japanese object/action
  names to UTF-8 catalog keys. The same decoder serves the source extractor,
  removing its iconv dependency. All 65,792 one/two-byte inputs match Python's
  standard CP932 codec; buffer/error tests pass, and the 218-message original
  Japanese FM Towns corpus is byte-identical to the prior iconv extraction.

- 2026-09-05: CSB source-text extraction now propagates CP932 conversion
  failure instead of silently omitting an entry and reporting success. Its
  UTF-8 buffer covers the runtime text bound's worst-case expansion. The
  authenticated FM Towns Japanese corpus remains byte-identical after the
  change, preserving the 218-message extraction evidence.

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
