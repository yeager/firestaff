# Firestaff TODO - CSB

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## ReDMCSB CSB Reference-Boundary Audit

### ReDMCSB Numbered-Symbol Audit (2026-07-14)

- REDMCSB-SYMBOL-GAP-004 — **Callable inventory audit: 870 routines have no
  Firestaff numeric reference.** `docs/reference/audits/REDMCSB_CALLABLE_SYMBOL_FULL_AUDIT.tsv`
  classifies all 2,137 `Exxx`/`Fxxxx`/`Rxxx`/`Sxxx` symbols: 1,031 have
  uncertain numeric Firestaff evidence, 870 have no exact numeric reference,
  and sixty are source-nonapplicable on the PC 3.4 route.
  Neither status establishes semantic parity; all 2,137 still require source
  behavior triage before they can be considered implemented. Required work:
  triage each missing callable routine by DM1/CSB runtime relevance, then
  implement or document a source-backed non-applicability decision with focused
  evidence. A comment or identifier match alone cannot close this gap.

  - 2026-07-22 update: `F0537`, `F0544`, `F1128`, and `F1172-F1176` now have
    explicit platform-boundary or narrow host mappings and four registered,
    passing CTest targets. The remaining callable inventory is still open.

  - 2026-08-09 update: Audit of the 870 routines confirmed ~26 game-critical
    functions are already present under different names in the Firestaff
    codebase (FILE.C I/O in src/shared/, MEMORY.C cache in src/memory/,
    SWSH.C/STRING.C are platform-specific or stdlib equivalents). The
    remaining routines are primarily platform shims and low-level DOS
    services that are non-applicable on the SDL3 host.

- REDMCSB-SYMBOL-GAP-006 — **Runtime parameter ABI surfaces need contract
  review.** `docs/reference/audits/REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv` covers
  all 8,013 `A/L/M/P` symbols: 5,017 are local/module/auxiliary labels with no
  standalone port target, while 2,996 `Pxxx` entries remain runtime ABI
  surfaces. Required work: audit parameters by their enclosing high-impact
  DM1/CSB routine, verifying width, signedness, ownership, mutation, and
  cross-module call contracts before treating a Firestaff equivalent as exact.

  - 2026-08-09 update: 3 ABI fixes landed (commit 0158bdd54): movement tick
    counters G0310/G0311 corrected to uint16_t, thing handle sentinel
    corrected to 0xFFFF. These are the highest-impact ABI surfaces
    identified in the audit.

- REDMCSB-SYMBOL-GAP-001 — **The complete numbered-symbol inventory is not a
  completion claim.** ReDMCSB has 13,090 unique numbered source symbols:
  `A` 4, `C` 866, `E` 6, `F` 2,104, `G` 2,074, `L` 4,616, `M` 397, `P` 2,996,
  `R` 8, and `S` 19. `Cxxx` denotes constants, not functions; only `F`, `E`,
  `R`, and `S` are callable families (2,137 total). Firestaff now tracks every
  symbol in `docs/reference/numbered_source_symbol_inventory.tsv`, but an exact source
  reference is deliberately only `referenced_not_verified`. Required work:
  complete per-routine behavioural mapping and original-data/runtime evidence
  before any family can be claimed fully implemented.

- REDMCSB-SYMBOL-GAP-002 — **Data-label families need a semantic map, not a
  name transplant.** `Gxxx`, `Lxxx`, and `Pxxx` are globals, locals, and
  parameters; `Mxxx` and `Axxx` are module/macro and auxiliary labels. C11
  Firestaff ownership may legitimately differ, but no complete audited map
  links them to their enclosing ReDMCSB function/data contract. Required work:
  record the owning Firestaff structure/function for each runtime-relevant
  label and verify aliasing, width, lifetime, and original-data boundaries.

- REDMCSB-SYMBOL-GAP-003 — **All platform/special boundaries require an
  explicit disposition.** The six `Exxx` exception handlers, eight `Rxxx`
  TOS/system routines, and nineteen `Sxxx` assembly/special routines cannot be
  accepted through a source-name match. Required work: mark each as portable
  equivalent, intentionally non-applicable, or source-backed implementation,
  with a focused test or a documented host-platform boundary.

- REDMCSB-CSB-GAP-001 — **CSBWin DSA is outside ReDMCSB's source domain.**
  ReDMCSB `Toolchains/Common/Source/` has no DSA, `EXPOOL`, `GAMEBLOCK2`, or
  `ITEM16` implementation; its original timer model is `TIMELINE.C`
  `F0261_TIMELINE_Process` and `DEFS.H` EVENT structures. Firestaff risk:
  treating a source-shaped original Cxx event as evidence for a CSBWin DSA
  callback invents selector, state, and opcode semantics. Independent source:
  CSBWin `DSA.cpp`, `data.cpp`, and `SaveGame.cpp`; required corpus: a
  checksum-valid CSBWin extended save containing DSA index/action records.

  - 2026-07-16 latest: the authenticated transfer VM now models nested
    CSBWin call frames (JUMP/GOSUB/RETURN), the DSA runtime-chain receipt is a
    production API binding Extended-Features DSA catalog/DSALevelIndex/live
    TimerQueue/event slots, the interpreter covers transfer dispatch and
    stack/runtime-hook STKOP admission with transactional commit receipts
    classifying opcode families, MESSAGE/MESSAGE32/DESSAGE32 and
    COPYTELEPORTER/COPYTELEPORTER32 actions execute through the production DSA
    VM, and `STKOP_ExperiencePlus` carries source-owned CHARDESC skill
    mutation into M11 and the CSBWin save-summary writer.
    Remaining: positive external DSA-bearing corpus execution breadth,
    unreviewed text/ex-pool, cancellation, party/champion, and broader
    dungeon mutation action surfaces.

- REDMCSB-CSB-GAP-002 — **CSBWin's restored timer queue is not ReDMCSB's
  timeline.** ReDMCSB `TIMELINE.C F0240/F0261` owns heap EVENT records, while
  CSBWin `CSBCode.cpp::ProcessTimers` dequeues `TIMER` entries and passes the
  original `timerObj6/8`, words, and queue index to `Timer.cpp`. Firestaff
  risk: C60/C61, C24, and DSA timers can have their object-word payload
  reinterpreted before CSBWin processing. Independent source: CSBWin
  `CSBCode.cpp`, `Timer.cpp`, `SaveGame.cpp`; required corpus: restored timer
  array plus its ordered queue, not merely a materialized EVENT list.

- REDMCSB-CSB-GAP-003 — **The ReDMCSB CSB save header is not a CSBWin save
  layout specification.** `DEFS.H:482-507` documents the 512-byte original
  `CSB_SAVE_HEADER`, five active parts, and original format/platform IDs. It
  does not specify CSBWin GAMEBLOCK2, CHARDESC tail, ITEM16, EXPOOL, or DSA
  extension bytes. Firestaff risk: accepting a header-shaped file as proof of
  a complete CSBWin resume, or writing unowned extension bytes. Independent
  source: CSBWin `SaveGame.cpp` and real CSBWin save corpus across versions.

  - 2026-07-16 latest: F7055-F7068, F7059/F7060, and F7088-F7090 are all
    CMake-registered, CTest-verified, and closed in the ReDMCSB callable
    audit/disposition tables. Covered: save-part checksum/XOR (F7055-F7058),
    header deobfuscation (F7061/F0429, F7062/F0430), dungeon-stream checksum
    (F7063), five-part save emission (CEDTINC8.C), champion name/title
    padding (F7064), portrait save/restore (F7065-F7068), portrait transfer
    (F7088), and imported-party reset (F7089/F7090).
    Remaining: positive real CSBWin DSA/runtime corpus breadth and broader
    title/HUD/door capture.

- REDMCSB-CSB-GAP-004 — **Original CSB save bytes still require per-media
  corpus proof.** ReDMCSB `LOADSAVE.C` is selected through many `MEDIA*`
  branches and serializes platform-dependent portraits, music state, and
  allocation paths; `DEFS.H:503-517` enumerates multiple format/platform
  combinations. Firestaff risk: a PC-oriented import/export path may claim
  Atari ST, Amiga, PC-98, X68000, or FM-Towns byte compatibility without an
  observed save for that exact media branch. Independent evidence: one
  original save and round trip per claimed media/version, plus CSBWin only
  where its importer explicitly supports that media.

  - 2026-07-15 latest: bounded PC34 adapters now cover the full save/load
    chain (F0435 HINTLOAD with header keys/checksums, F0434 dungeon-tail,
    F0651 free-list rebuild, F0652 event merge), viewport rendering (F0655/
    F0656 bitmap blit, F0657/F0658 bitmap index routing, F0661 derived-cache,
    F0662/F0663 palette/dimensions, F0664 wall-click, F0674 floor/ceiling
    copy, F0676-F0679 D3/D2 draw order, F0684 viewport blit dispatch),
    IMG3 decoding (F0685-F0691 nibble/run-count/expansion/screen path),
    UI helpers (F0665 zone gate, F0666 endgame, F0670-F0673 text/mouse-input),
    and startup presentation (F0692/F0693 packed page with VBlank gate).
    All are caller-owned bounded adapters creating no fallback data.
    Remaining: per-media original-save corpus for every claimed platform,
    real CSB graphics/save corpus for live HUD/viewport binding.

- REDMCSB-CSB-GAP-005 — **DSA timer action remapping cannot be inferred from
  ReDMCSB.** ReDMCSB has canonical original SET/CLEAR/TOGGLE EVENT behavior;
  CSBWin `DSA.cpp` can alter `timerTypeModifier`, and `Timer.cpp`
  `ProcessTT_FALSEWALL`, `ProcessTT_STONEROOM`, `ProcessTT_OPENROOM`,
  `ProcessTT_DOOR`, `ProcessTT_TELEPORTER`, and `ProcessTT_PITROOM` invoke
  DSA before their normal mutation. Firestaff risk: applying a canonical
  modifier after an imported type-47 actuator without its DSA owner changes
  a real dungeon. Independent source: CSBWin `DSA.cpp` and `Timer.cpp`;
  required corpus: DSA-bearing save/dungeon pair with the selected actuator.

- REDMCSB-CSB-GAP-006 — **Original graphics code does not prove every host
  presentation byte.** ReDMCSB contains media-conditional C and assembly
  paths, including `GRAPH21.C` fuzzy-sector/CPSE code and `VBLANK.C` interrupt
  handlers; startup and rendering data are selected by `MEDIA*` defines.
  Firestaff risk: deriving CSB title, entrance, palette, frame cadence, or
  copy-protection side effects from one compiled media path and applying them
  to another. Independent evidence: hash-identified original asset captures
  per claimed CSB media, with CSBWin `Swoosh.cpp`, `Graphics.cpp`, and
  `Viewport.cpp` used only for CSBWin behavior.

  - 2026-07-16 latest: PC34 path decodes canonical GRAPHICS.DAT C001-C005
    with LZW.C chunk-width semantics and locks TITLE/ENTRANCE raster output.
    The startup presenter refuses to pack/VBlank-present unless the raster
    matches the CSB host-surface receipt (route + pixel hashes). External
    original Mac/app capture remains required for non-PC media branches.

- REDMCSB-CSB-GAP-007 — **Platform service shims are opaque dispatches, not
  portable behavioral specifications.** `USIOSTUB.C` forwards mouse, input,
  and queue operations through library-vector jumps; `MEM1STUB.C`,
  `INT1STUB.C`, and `MUSCSTUB.C` do the same for memory, interrupt, and music
  services. Firestaff risk: assuming their call names prove event ordering,
  mouse sampling, or music semantics on a modern SDL host. Independent
  source/corpus: platform executable or trace capture for the target media;
  CSBWin `Mouse.cpp`, `Sound.cpp`, and `SoundMixer.cpp` for CSBWin-only paths.

  - 2026-07-14 update: ReDMCSB `AMIGINIT.C` F1081-F1084 are now recorded as
    four explicit PC34 host boundaries. The Amiga `NIL:` DOS handles and the
    C03 chip-memory `AMISTRUCT` allocation/free pair do not open files or
    allocate host memory; a target-media trace would still be required to
    claim platform-service behavior beyond this unavailable boundary.

- REDMCSB-CSB-GAP-008 — **Original known bugs are behavioral choices, not
  automatic Firestaff fixes.** ReDMCSB annotates surviving behavior such as
  `TIMELINE.C` BUG0_19 object-launcher exhaustion and `MENU.C` BUG0_54/55/56/
  77/79. Firestaff risk: silently "fixing" these changes original behavior,
  while reproducing them globally can corrupt modern/custom data. Independent
  evidence: `BugsAndChanges.htm`, version-tagged original runtime capture,
  and a corpus dungeon that reaches each branch. Each Firestaff choice must
  state emulate, guard, or reject rather than citing the annotation alone.

- REDMCSB-CSB-GAP-009 — **CSBWin custom-dungeon semantics need CSBWin, not
  ReDMCSB, as primary source.** ReDMCSB documents original CSB data and
  version changes, but not CSBWin's type-47 DSA scripts, extended EXPOOL
  records, trace database, or custom save continuation. Firestaff risk:
  claiming compatibility with DSA/custom CSB dungeons from original-CSB
  coverage alone. Independent source: CSBWin `DSA.cpp`, `data.cpp`,
  `SaveGame.cpp`, and a legal custom-dungeon/save corpus with raw hashes.

- REDMCSB-CSB-GAP-010 — **Assembly and media gates require observed failure
  behavior before any fallback is accepted.** `GRAPH21.C` shows original-CSB
  CPSE state (`G0068_i_CheckLastEvent22Time_CPSE`) tied to fuzzy-disk analysis;
  ReDMCSB exposes the code but Firestaff has no authentic floppy signal or
  sector timing. Firestaff risk: a synthetic success/failure state can alter
  memory/freeing or event-22 behavior. Independent evidence: archived
  original disk image plus emulator/real-machine trace; otherwise keep the
  branch explicitly unavailable rather than simulating protection state.

- REDMCSB-CSB-GAP-011 — **The ReDMCSB rebuild is not a binary oracle for
  original CSB releases.** `Documentation/Readme.htm` explains that the
  available Megamax 1.1 compiler and linker differ from FTL's unavailable
  toolchain, while the project is rooted in Atari ST reconstruction. Firestaff
  risk: treating source control flow or a rebuilt executable as proof of PC
  3.4 code layout, timing, checksums, or compiler-dependent expression
  behavior. Required independent evidence: hash-identified original release
  executable plus an emulator trace for every PC-specific binary claim.

- REDMCSB-CSB-GAP-013 — **Annotated original bugs are not a portability
  policy.** `Documentation/BugsAndChanges.htm` describes historical faults and
  version changes, but does not say whether a modern engine should emulate,
  guard, or reject malformed/custom data that reaches them. Firestaff risk:
  either silently changing an original route or copying memory corruption into
  a host runtime. Required evidence: a version-specific original reproduction
  and an explicit per-route policy with a regression test; absent that, retain
  bounds checks and report the route unavailable.

## CSB V1 Runtime Presentation Follow-up (2026-07-11)

The verified PC 3.4 entrance now hands the loaded dungeon directly to the V1
viewport and source V1 champion/control HUD lane, with no diagnostic maze,
status-screen, or runtime material-marker substitute. Remaining CSB work is
broader real-asset viewport/HUD pixel-parity capture across naturally
populated dungeon states.

- 2026-07-11 CSB F0115 material follow-up: the first four PC CSB native
  `FIRESTAFF_CSB_GRAPHICS_DAT` and is neutral without user media; the matching
  group-material regression is
  `csb_v1_f0115_first_group_real_asset_pc34_compat`.
  type-2 source row has only the 0/1 bitmap deltas, so 465 remains blocked.

## Chaos Strikes Back (CSB)

### FM Towns native save boundary

- **FM-TOWNS-BOOT-002 — language-private loose-tree handoff is closed.** A
  direct extraction containing both `CDATA` and `CJDATA` now carries the
  selected FM Towns language into the CSB boot owner. English binds
  `CHTWE.EXP`/`CDATA`; Japanese binds `CHTWJ.EXP`/`CJDATA`, with both required
  files admitted by their authentic hashes. The real-data M11 handoff test
  passes for both languages.

- **FM-TOWNS-RESUME-001 — retail MINI.DAT resume is closed.** The real
  English and Japanese `CDATA/CJDATA/MINI.DAT` bootstrap saves now pass the
  production M11 `savePath` boundary, restore the authenticated map-4 pose,
  champion/event state and dungeon tail, and enter GAMELOOP without replaying
  TITLE.ANM. This does not close arbitrary user `CSBGAME.DAT` compatibility or
  native writeback.

- **FM-TOWNS-SAVE-001 — arbitrary user-save compatibility remains open.** The
  external `fmtowns-save-corpus/CSBGAME.DAT` and `CSBGAME-JP.DAT` files are
  retained as unclassified candidates. Both pass the C5 header and five
  F7057-part checks, and F0434 consumes their appended F7063 streams from the
  same save-file handle (not from CDATA/CJDATA). Those tails parse as two-map
  Prison dungeons, but the saved global pose is not consistent with the
  authenticated tail: the English candidate declares map 4 at (22,18),
  outside its two maps. It remains fail-closed before a complete native F0435
  state receipt is produced. CSBGAME-JP.DAT is instead a valid F31J save: its
  language-private header, party envelope and 6,540-byte F7063 tail produce
  map 0 at (13,13) and are not interchangeable with F31E. The opt-in
  csb_v1_fmtowns_user_save_corpus regression supplies the licensed media plus
  those external candidate files, proves that F31E stays rejected, F31J
  materializes its authenticated state, and neither source file changes. The
  direct M11 F0433 path now resumes the valid F31J slot before writing it,
  verifies the source event-allocation envelope, preserves the language-owned
  tail and reopens the replacement through F0435. A MINI.DAT session cannot
  write across into that distinct F31J slot. It can also make a new canonical
  slot from the selected, hash-verified `MINI.DAT` bootstrap
  through the recovered F7052 key/header sequence. The first-save transaction
  is staging-only until the completed F0435-valid slot is atomically published;
  it neither accepts these inconsistent corpus candidates nor invents a save
  envelope. M11's F31 C140 route now maps F7052's M746 file id to the separate
  user-owned `saves/csb/fmtowns/CSBGAME.DAT` medium, never the scanned C03
  tree: its first save runs the verified MINI.DAT bootstrap and later saves
  reopen/write the same native slot. `MINI.DAT` remains the only retail
  bootstrap path, and no synthetic save is used.

- **FM-TOWNS-C06-SAVE-001 — selected portrait save and first GAME save are bound.**
  `CEDT001.C:F7001` now opens its source `GAME` / `PORTRAIT` / `CANCEL`
  dialog. The authenticated UTILE/UTILJ images retain the exact native
  `2:\#CHAMP_NAME#.CMP` mapping as a source receipt; `F7000` writes only the
  selected champion through that mapping to `.firestaff/portraits` on
  macOS/Linux and `INSTALLDIR\\portraits` on Windows; it never rewrites the
  scanned CD `PORTRAIT/*.CMP` catalogue. C06 `NEW DISK` now reopens that same
  medium through the native CMP admission path. `GAME` now recreates
  `CEDT001.C:F7001` choice 1 / `CEDTINC8.C:F7052`: it starts from the selected
  verified `MINI.DAT` state in an isolated runtime, copies C06's editor-owned
  champion records and four raw planar portrait blocks, and atomically publishes
  the first user-owned M746 `CSBGAME.DAT` only after the native F0435 reader can
  admit it. It never serializes a separately active C03 session. A later C06
  save reopens the authenticated slot, writes the same editor-owned fields via
  F0433/F7062, and retains the native `CSBGAME.BAK` rollback copy.

- **FM-TOWNS-C06-LOAD-001 — `F7002_ReadCMP` is now an authenticated import
  transaction.** Given an index returned by the admitted `PORTRAIT` catalogue,
  Firestaff rechecks the exact 508-byte file and its catalogue hash, then
  copies only that record's native 464-byte planar payload and 8/20-byte
  name/title into the selected party slot. It never accepts a host path,
  creates a row, or chooses a fallback. The catalog-bound selector now
  preserves the authenticated source-sorted 24-entry list, provides bounded
  previous/next movement, and delegates the selected row to this same import
  transaction. The real English catalogue covers catalog binding, movement,
  and import in `csb_v1_fmtowns_m11_game_handoff`. **2026-08-11 modal loop
  closed:** M11 first reproduces F7004's source `GAME` / `PORTRAIT` /
  `CANCEL` dialog, then enters F7083 only after its `PORTRAIT` choice, renders
  the admitted source raster, sends source-coordinate left-clicks to F7084,
  preserves scroll/cancel commands, and hands an accepted row to F7002. The
  real F31E handoff test proves open, raster, selection and import. No host
  path or synthetic row is accepted. C06 F31E name/title editing and its first
  `GAME` save are source-bound separately. **2026-08-11 F7004 CSB load closed:**
  the GAME choice now reopens the user-owned M746 `CSBGAME.DAT` through the
  native F0435/F7063 receipt and restores its party plus four raw planar
  portraits into C06. The selected file is rehashed before the portrait read,
  so the validated save parts cannot be paired with a swapped portrait span.
  The source's separate Dungeon Master-versus-CSB selection and Make New
  Adventure remain open; Firestaff's C06 path intentionally admits only CSB.
  In-game C140 native saving is bound separately through F7052.

### CSB V1

- 🔧 Phase 2 - Dungeon data model: synthetic CSB dungeon loader/model probe exists, loader/free-cycle safety is covered, and the PC real-asset launch gate proves canonical CSB assets enter the runtime-owned dungeon singleton. Core PC runtime/input/movement slices are now CTest-registered for command chains, input-queue binding, one-step movement, rotation between steps, runtime tick accumulation, queue overflow, reincarnation penalty, projectile speed, Grey Lord combat, DECOMPDU, version-checker sensors, monster generator state, chaos cast cooldown/targeting, one DSA trigger step, save import path, save runtime boundary, Neophyte mode, and Zokathra spell handling. Remaining work is deeper end-to-end gameplay parity, real save compatibility artifacts, viewport/UI runtime evidence, and playability without DM1-only assumptions.

- 🔧 Phase 3 - Rendering parity hardening: D3/D2 wall tables, bitmap selection, grid routing, CSB-only D3L2/D3R2 and D2L2/D2R2 draw-order/frame gates, F0107 back-wall ornament routing, initial viewport gates, and the 2026-06-21 CSB-only viewport CTest slices are in place. **2026-08-09 D0L/D0R + F0109 + F0110 gaps closed** (commit 7be579a7d): D0L/D0R side walls (F0125/F0126, view squares 9/10, 32x136 frames), F0109 door ornament rendering (D1/D2/D3 with scaling and palette remap), and F0110 door button rendering (D3R/D3C/D2C/D1C with coordinate sets G0207/G0208) are all implemented and source-locked. The CTest rows now cover first CustomBackgrounds backdrop, room-slot backdrop-1, D1C F0108 floor/ceiling ornament, D1C F0115 thing pass, D3C F0107/F0108 first-backdrop composition, D3L/D3R sidewall backdrops, and D2C F0107 wall-ornament plus F0111 door-front layering without game data. **2026-06-26 CSB V1 PC real-asset ornament blit probe landed, hardened 2026-06-29:** `firestaff_csb_v1_pc_real_asset_ornament_blit_probe` parses the DMCSB1 BE GRAPHICS.DAT header from a real PC 3.4 CSB pair, drives the source-locked F0108 zone + C10_COLOR_FLESH transparency + F0115 thing-pass math against real bytes, bounds-checks the selected bitmap payload span, writes deterministic 320x200 PPM + SHA256 + JSON provenance manifest sidecars to `/tmp`, and now reads the manifest back to assert the schema, canonical MD5, capture SHA256, source anchors, tally fields, and non-claims are present (capture sha256 `5e489ae14354d791e12a9474bbb44027eaac1be8e1021491d9d88dcef8ba9de1`); CTest `csb_v1_pc_real_asset_ornament_blit` PASS 31/31 against `~/.firestaff/data/csb`, skip-safe on hosts without user-supplied PC CSB data. Remaining work is broader viewport/HUD captures and pixel parity evidence.

- 🔧 Runtime handoff: the old M12/M11 CSB launch-readiness blocker is retired. Hash-matched CSB assets now produce a valid launch intent, M11 hands CSB to `FS_GAME_CSB`, and `csb_v1_pc_real_asset_launch` proves canonical PC CSB `GRAPHICS.DAT`/`DUNGEON.DAT` scan, `csb_v1_boot_enter_game()`, dungeon ownership, source-locked start pose, Chaos magic init, one tick, and cleanup. PC presentation/ornament probes require the exact PC 3.4 variant plus its canonical graphics/dungeon hashes; a verified FM Towns or other CSB edition at the default root is explicitly skipped rather than misreported as failed PC pixel parity. The CTest runtime set also covers command chains, input-queue turn binding, queued movement, collision/no-step handling, movement-disabled gating, turn-between-step ordering, utility/import handoff, and runtime load/attribute formulas. **2026-06-28 PC 3.4 quickplay dungeon-handle probe landed:** new `firestaff_csb_v1_pc34_quickplay_dungeon_handle_probe` (CTest `csb_v1_pc34_quickplay_dungeon_handle`, labels `tier1;csb;quickplay;boot_handoff;dungeon_handle;rescan;skip_safe`) pins the handle-survival invariants (H1-H4: runtime owns the verified DUNGEON.DAT handle after enter_game, global singleton equals the runtime handle, current level is map 0, runtime starts at TITLE) plus the rescan-clearing invariants (H5-H7: failed rescan clears the runtime handle + global singleton + profile fields + blocks re-launch) and the re-launch invariants (H8: successful rescan into verified dir releases the previous handle before enter_game re-establishes a fresh one). Probe is skip-safe — when `FIRESTAFF_CSB_PC_DATA` (or `~/.firestaff/data/csb`) carries the canonical PC 3.4 EN pair it exercises the real-asset path end-to-end; on hosts without user-supplied CSB data it falls back to a synthetic-fixture path (1 level, 2x2 legacy-format dungeon) that still drives the production `csb_v1_boot_enter_game()` / `csb_v1_boot_scan_assets()` code paths so CI stays deterministic. **Bug found and fixed:** the previous `csb_v1_boot_scan_assets()` only cleared profile metadata on rescan, NOT the runtime-owned `dungeon_handle` or the `csb_v1_dungeon_get_current()` singleton — so a follow-up rescan that lost the CSB assets could leave the runtime still pointing at the previous heap-allocated dungeon. The rescan path now releases the handle and resets the singleton before the rescan-driven profile fields are populated, matching the same release contract as `csb_v1_boot_cleanup()`. Remaining work is richer CSB-specific viewport/HUD evidence, gameplay/save/audio parity, original capture/pixel parity, and end-to-end playability verification.

### CSB V2.0 / V2.1 / V2.2

- 🔧 V2.2 F0128 projection gate (2026-07-29): the old modern-art renderer
  treated the CSB perspective viewport as nine opaque rectangles. Real-data
  capture proved that this paints horizontal bands over F0128. The runtime now
  retains the verified original F0128 frame in V2.2 while it publishes the
  selected material route; modern pixels remain blocked until each placement,
  clip mask, palette and draw order is consumed from an original F0128 receipt.
  Route provenance is now readable from the selected source pack as a strict
  `(category, id, sourceGraphicIndex, sourceDimensions)` record.
  2026-07-30: admitted D1/D2/D3 door commands now consume that exact record
  and C10 transparency inside the original F0128 draw loop. The V2.2 shape
  cache is prepared before F0128 starts, and the command-local replacement
  count reaches M11 without replaying modern pixels after source composition.
  Remaining: add equally source-locked placements, masks and material
  provenance for non-door F0128 families before allowing them to replace V1.

- 🔧 Phase 2 - Enhanced asset pipeline: presentation-mode selection API is wired (csb_v2_presentation_mode_set_m12, m12PresentationMode 0..3 → CSB_V2_PM_V1_FAITHFUL/V20_FILTERED/V21_UPSCALED/V22_MODERN). **2026-06-19 CSB V2.2 modern-asset module landed:** new `csb_v22_modern_assets_pc34.c/.h` (include + src) mirrors dm1_v2_modern_assets_pc34 with CSB-specific paths (`~/.firestaff/assets/csb/modern/`) and CSB source-locks (ReDMCSB LIGHT.C F0212 / DUNVIEW.C F0128 / PANEL.C F0354 / COMMAND.C:108-113,254-291 + CSBWin/Viewport.cpp:7290 + CSBWin/Chaos.cpp:60-69). Ctest `test_csb_v22_modern_assets_pc34` 33/33 (path resolution from dataDir, manifest validation missing/empty/partial, installed flag round-trip, epx warm flag round-trip, full fallback chain V1→V2.0→V2.1 cold/warm→V2.2 missing/installed, shape source name strings, missing placeholder 16x16 magenta, source evidence citation). **2026-06-19 CSB V2.2 first-cut asset pack landed:** `.openclaw/tmp/csb_v22_asset_author.py` procedural generator (5 PNGs: wall_dungeon/floor_prison/creature_chaos_fiend/panel_lord_order/champion_warrior_csb + modern_asset_manifest.json v1.0.0). CSB-specific palette accents: CHAOS_PURPLE (chaos magic), IRON_GREY (prison), LORD_GOLD (Lord Order). Smoke: `csb_v22_set_manifest_path(dataDir)` resolves correctly, `csb_v22_modern_assets_available()=1` end-to-end against real CSB data dir. **2026-06-21 CSB in-place render gate:** `csb_v22_shape_cache_update` now respects `csb_v2_presentation_mode_is_v22()`, `csb_v22_inplace_draw_init()` loads from `~/.firestaff/assets/csb/modern/`, and `csb_v22_inplace_render_pass()` uses the cache's 0..2 depth coordinates. New CTest `csb_v22_inplace_render_probe` PASS verifies a synthetic cache, V1 inactive cache, V22 bitmap lookup, 9 painted CSB cells, 4-direction sweep (36 cells), and source evidence. **2026-07-31 product binding correction:** the handcrafted V2.2 shape/material book is now probe-only; Firestaff links `csb_v22_shapes_runtime_gate.c`, which returns no material and therefore leaves V1/V2.1 pixels intact. The launcher-installed bit is also no longer an admission override: mode selection rechecks the finished-art/provenance gate. Remaining work: decode and review a real CSB material/pixel binding from PC 3.4 GRAPHICS.DAT before re-admitting any V2.2 art route; do not author PBR or procedural replacement art.

- 🔧 Phase 3 - Enhanced UI overlays: scaffolded (HUD compass/depth/gold/champion bars/action strip/chaos indicator, csb_v2_hud_overlay_pc34.h/.c, build+probe pass). Mode selection gate added in this pass (csb_v2_presentation_mode_is_v22() / is_v21() / is_v20() / is_v1()) so the HUD overlay can branch on the active mode.
