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

- REDMCSB-SYMBOL-GAP-005 — **C/G semantic ownership is not verified.**
  `docs/reference/audits/REDMCSB_CONSTANT_GLOBAL_FULL_AUDIT.tsv` covers all 866
  `Cxxx` constants and 2,074 named `Gxxxx` globals. Of those, 1,830 have only
  a semantic Firestaff candidate, 587 have no direct counterpart, and 523 are
  platform/toolchain-specific. Required work: prioritize the DM1/CSB runtime
  C/G rows by rendering, save/load, champion, dungeon, and timeline impact;
  then give each an owner, width/lifetime contract, and focused evidence.

- REDMCSB-SYMBOL-GAP-006 — **Runtime parameter ABI surfaces need contract
  review.** `docs/reference/audits/REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv` covers
  all 8,013 `A/L/M/P` symbols: 5,017 are local/module/auxiliary labels with no
  standalone port target, while 2,996 `Pxxx` entries remain runtime ABI
  surfaces. Required work: audit parameters by their enclosing high-impact
  DM1/CSB routine, verifying width, signedness, ownership, mutation, and
  cross-module call contracts before treating a Firestaff equivalent as exact.

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
  - 2026-07-14 update: the opt-in package handoff now loads an original
    `Dungeon.dat` into the runtime owner and ticks it before attempting the
    paired `csbgame*.dat`. The checked CSBWin corpus `csbgame2.dat`
    (`105104b30dde164e7000d388f251f3d6d3f83a56959f28f56220711d1e9f3a9e`)
    has no supported GAMEBLOCK1/Extended Features start and rejects without
    publishing party, timer, or DSA state. This is an observed unavailable
    route, not a substitute format; a checksum-valid DSA-bearing save remains
    required for positive DSA handoff proof.
  - 2026-07-14 follow-up: an accepted original package resume is now checked
    against the live runtime queue slot-by-slot. Every published event must
    retain its source timer index, full timer word, function, bytes, and
    queue-slot ownership; a missing or generic replacement event fails the
    real-package probe. The probe has no package fixture and skips without
    explicit original paths. Positive DSA proof still requires an authentic
    checksum-valid DSA-bearing save/dungeon pair.
  - 2026-07-14 follow-up: the opt-in Extended Features/DSA package probe now
    carries that ownership receipt through one live tick. Consumed source
    timers may leave the queue, but every surviving timeline event must still
    match one unique serialized TimerQueue slot and all TIMER fields. A host
    event without that source receipt fails the probe; no generated save,
    dungeon, DSA, or replacement event is used.
  - 2026-07-14 follow-up: CSBWin core export now also revalidates the retained
    TIMER/TimerQueue heap after matching every live source receipt. A live
    event that still matches its timer cannot make a stale or reordered
    serialized heap exportable; the route rejects before emitting bytes.
  - 2026-07-14 follow-up: the real-package Extended Features/DSA probe now
    tests post-tick core persistence only when the retained TIMER heap remains
    exportable. Production verification and a fresh core-only runtime resume
    must preserve the timer receipts, party pose, and game time while clearing
    extended DSA metadata. A fired or requeued timer that lacks a serializable
    CSBWin heap is explicitly unavailable; the probe never fabricates a
    replacement save or DSA tail.
  - 2026-07-14 follow-up: the general original-package handoff applies the
    same post-tick core writer/reader proof to every accepted `csbgame*.dat`,
    including saves without Extended Features. A verified core-only resume
    must retain the exact surviving timer queue, level, party pose, facing,
    and game time while owning no Extended Features/DSA state. A nonexportable
    post-tick heap remains an explicit unavailable result; no generic timer
    or synthetic core save is substituted.
  - 2026-07-14 follow-up: after a source-owned timer successor survives a
    live tick, Firestaff now rebuilds only the retained CSBWin `TimerQueue`
    ordering using `Timer.cpp::AdjustTimerQueue` ordering, and republishes
    event-to-slot receipts only when every live event still maps one-to-one to
    a complete original `TIMER` array. Consumed timers, generated events,
    duplicate slots, and malformed receipts remain unexportable. The opt-in
    real-package probe remains the required positive evidence for a requeued
    original save; no package, timer, dungeon, or DSA fixture was added.
  - 2026-07-16 follow-up: the CSBWin DSA/save-runtime corpus receipt now
    requires recognised CSBWin save filenames plus a verified Extended
    Features DSA section, game-info/level-index tail, and following GAMEBLOCK1
    header before declaring runtime handoff ready. This narrows the admission
    gate for a future authentic DSA-bearing corpus; it still does not provide
    the missing real save/dungeon pair.
  - 2026-07-14 follow-up: the opt-in Extended Features/DSA handoff now
    snapshots the supplied original `Dungeon.dat` and `csbgame*.dat` before
    production resume, then revalidates both complete-file size/FNV receipts
    after the live tick. A replaced package cannot inherit DSA-action,
    timer-slot, or core-resume evidence. The probe remains unavailable without an explicit
    checksum-valid DSA-bearing save/dungeon pair; it creates no substitute
    save, dungeon, timer, or DSA record.
  - 2026-07-14 follow-up: when a real resumed tick reaches the existing
    source-bounded `ProcessDSATimer5/6` pure-stack route, the runtime now
    retains a receipt for its original `TimerQueue` slot, `TIMER` index, and
    exact authenticated DSA action. The opt-in package probe verifies that
    receipt against the retained save-owned action table and reports the
    positive action path separately from an unavailable package. It neither
    predicts a timer nor synthesizes a DSA action; current local data has no
    checksum-valid extended save to exercise this positive branch.
  - 2026-07-16 follow-up: the DSA runtime-chain receipt is now a production
    CSB runtime API rather than probe-local evidence. It binds the
    Extended-Features DSA catalog, `DSALevelIndex`, live TimerQueue/event
    slots, and optional executed saved-timer DSA action into one fail-closed
    handoff receipt. The package and Extended-DSA probes both consume it and
    the Extended-DSA probe is registered with CTest; no synthetic save,
    dungeon, DSA, timer, or opcode payload was introduced. The positive
    external DSA-bearing corpus requirement remains open.
  - 2026-07-16 follow-up: the interpreter now has an explicit
    source-word-program verifier for the implemented core subset. It covers
    transfer dispatch and stack/runtime-hook STKOP admission before execution,
    and the Extended-DSA real-corpus probe treats a DSA package with no
    verified executable action as unavailable rather than inventing a script.
    Broader opcode support and positive external DSA-bearing corpus execution
    remain open.
  - 2026-07-16 follow-up: the verified transfer/stack-core interpreter now
    publishes a production runtime execution receipt only after transactional
    commit. The receipt binds operand-stack execution, local-state persistence,
    TimerQueue/event-selected action identity, and dungeon/save mutation flags
    to the exact authenticated `DSAAction`; unsupported opcodes and unverified
    actions still fail closed with no receipt. Positive external DSA-bearing
    corpus execution remains open.
  - 2026-07-16 follow-up: the same runtime receipt now classifies committed
    actions by verified CSBWin opcode family: conditionals, arithmetic,
    local/global variable access, timer-owned effects, and dungeon/save
    mutation callbacks. The Extended-DSA probe reports those family counts only
    from a real supplied corpus and checks the committed execution receipt when
    a saved TimerQueue DSA action fires. Positive external DSA-bearing corpus
    breadth and any not-yet-reviewed opcode family remain open.
  - 2026-07-16 follow-up: CSBWin MESSAGE/MESSAGE32/DESSAGE32 actions are now
    verified and executed as a source-shaped `QueueDSASwitchAction` scheduling
    family. The runtime route consumes the authenticated DSA command, validates
    real dungeon cell type for `M` messages, enqueues the selected live timer
    without inventing a saved TIMER slot, and lets the DSA chain receipt accept
    those new unmapped runtime timers while still validating every original
    saved TimerQueue slot exactly. Remaining work is authentic DSA corpus
    breadth and the still-unreviewed text/ex-pool, cancellation,
    party/champion, and broader dungeon mutation action surfaces.
  - 2026-07-16 follow-up: CSBWin COPYTELEPORTER/COPYTELEPORTER32 actions now
    share the production DSA VM and runtime-chain receipt. The real-dungeon
    callback walks the source Thing chain for DB1 teleporters, copies the
    teleporter payload without relinking `Next`, copies the source CELLFLAG
    byte, and reports committed source/destination locations only after
    rollback-guarded profile publication. This does not close broader door
    actuator timers, parameter-message EXPOOL text, or other unreviewed
    dungeon mutation opcodes without authentic corpus proof.
  - 2026-07-16 follow-up: `DSACMD_QUESTION` branch-transfer execution is now
    implemented as a real interpreter step. It consumes only source-owned
    branch operands, follows only the selected authenticated JUMP/GOSUB action
    chain, and reuses the same `last_transfer` state-persistence path as a
    top-level transfer. Positive external DSA-bearing corpus breadth remains
    open.
  - 2026-07-16 follow-up: the authenticated transfer VM now models nested
    CSBWin call frames instead of treating JUMP/GOSUB as receipt-only labels.
    JUMP stays in the current Execute frame, GOSUB records a continuation frame,
    and RETURN is observed when the selected state/column has no source
    program. Runtime receipts and the Extended-DSA probe now fail closed unless
    any executed transfer chain is return-counted and frame-balanced.
  - 2026-07-15 follow-up: the bounded authenticated
    `STKOP_ExperiencePlus` route now carries its source-owned CHARDESC skill
    mutation into both M11's party snapshot and the CSBWin save-summary
    writer. This is only the non-level-up `Magic.cpp::AddToSkill` path; no
    DSA fixture, synthetic skill level, or partial `LevelUp` result is used.

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
  - 2026-07-14 update: `CEDTINC6.C` F7055/F7056/F7057/F7058 now has a
    CSB-owned PC34 little-endian word port for the original save-part
    checksum/XOR and write-then-restore transaction. Empty and odd-length
    sections remain unavailable rather than acquiring a byte-wise fallback.
    This does not specify CSBWin GAMEBLOCK2, CHARDESC, ITEM16, EXPOOL, or DSA
    layouts, and does not establish a complete CSBWin resume.

  - 2026-07-14 update: `CEDTINC6.C` F7059/F7060 now has the PC34 modular
    16-bit accumulation of exact already-read or already-written dungeon-part
    bytes. The port owns no file transport and does not infer a dungeon part,
    CSBWin extension, timer, or DSA layout from that checksum.
  - 2026-07-14 update: `CEDTINC6.C` F7061/F0429 now checks exactly one
    512-byte PC34 header and deobfuscates its second half before returning the
    original checksum verdict, including on failure. It neither identifies a
    save format nor decodes CSBWin GAMEBLOCK2, CHARDESC, ITEM16, EXPOOL, DSA,
    timer, or runtime state.
  - 2026-07-14 update: `CEDTINC6.C` F7062/F0430 now prepares the exact
    obfuscated 512-byte PC34 header image from the source-owned 127-word RNG
    sequence and restores the caller's plaintext tail afterward. It supplies
    no RNG, file transport, format identification, CSBWin extension, DSA,
    timer, or runtime interpretation.
  - 2026-07-14 update: `CEDTINCA.C` F7063 now validates the source-owned
    22-part PC34 dungeon stream sequence against its trailing checksum word:
    header, maps, three table blocks, 16 ThingData pools, and RawMapData.
    The parts remain opaque; this neither allocates nor decodes DUNGEON_HEADER,
    MAP, ThingData, CSBWin, DSA, timer, or runtime layouts.
  - 2026-07-14 update: `CEDTINC8.C` now prepares the five original save
    parts in source order from header-owned keys: all checksums are calculated
    before any part is emitted, each emitted part is obfuscated, and caller
    plaintext is restored. Missing, empty, or odd parts have no partial output.
    This does not identify the five part layouts or decode CSBWin DSA/timers.
  - 2026-07-14 update: `CEDTINC8.C` plus F7060 now emits the fixed 22-part
    opaque dungeon byte stream and its final little-endian checksum word. A
    short output buffer emits nothing. This does not allocate or decode any
    dungeon, CSBWin, DSA, timer, or runtime layout.
  - 2026-07-14 update: `CEDTINCQ.C` F7064 now applies the PC34 fixed-field
    NUL padding for the 8-byte champion name and 20-byte title after load.
    Media-specific character conversion and all champion/CSBWin/DSA layouts
    remain outside this byte-field normalizer.
  - 2026-07-14 update: `CEDTINCS.C` F7065/F7066 now clears portrait pointer
    slots before saving and restores them in source sequential-buffer order
    after loading, only for `PORTRAITS_EXCLUDED`. This owns no champion record
    layout, portrait decode, CSBWin extension, DSA, timer, or save format.
  - 2026-07-14 update: `CEDT007.C` F7067/F7068 now owns the C31 portrait
    pointer get/set path for both source champion formats. Other champion info
    indices remain unimplemented rather than inferring a champion, CSBWin,
    DSA, timer, or save-record layout.
  - 2026-07-16 CSB audit update: F7055-F7058 and F7061-F7068 are now
    CMake-registered, CTest-verified, and closed in the ReDMCSB callable
    audit/disposition tables through the CSB-owned save/header/dungeon-stream
    and champion portrait/text helpers. Remaining work in this save area is
    still positive real CSBWin DSA/runtime corpus breadth and broader
    title/HUD/door capture, not these byte-transaction helpers.
  - 2026-07-16 CSB audit update: F7059/F7060 are now CMake-registered,
    CTest-verified, and closed in the audit/disposition tables as the
    source-owned dungeon-part byte checksum boundary. Remaining work here is
    still broader real CSBWin DSA/runtime corpus coverage, not this checksum.
  - 2026-07-16 CSB audit update: F7088-F7090 now have registered focused
    CTests and disposition rows. Remaining adjacent work is original
    media/save corpus interop for imported parties and broader CSB runtime
    handoff.
  - 2026-07-14 update: `CEDTINCR.C` F7088 now transfers the exact PC34
    `PORTRAITS_INCLUDED` to `PORTRAITS_EXCLUDED` route: four source portrait
    spans of 464 bytes are copied and then rebound through F7066. Other
    champion properties, allocation, champion records, CSBWin, DSA, timer,
    and save layouts remain unimplemented rather than inferred.
  - 2026-07-14 update: `CEDT008.C` F7089 and `CEDTINCI.C` F7090 now own the
    PC34 imported-party reset after that portrait transfer: source header
    tails, rotation, field reset, modifier callback and four-cell collision
    repair are explicit. Original per-media save bytes and runtime handoff
    still require the corpus proof tracked below.
- REDMCSB-CSB-GAP-004 — **Original CSB save bytes still require per-media
  corpus proof.** ReDMCSB `LOADSAVE.C` is selected through many `MEDIA*`
  branches and serializes platform-dependent portraits, music state, and
  allocation paths; `DEFS.H:503-517` enumerates multiple format/platform
  combinations. Firestaff risk: a PC-oriented import/export path may claim
  Atari ST, Amiga, PC-98, X68000, or FM-Towns byte compatibility without an
  observed save for that exact media branch. Independent evidence: one
  original save and round trip per claimed media/version, plus CSBWin only
  where its importer explicitly supports that media.
  - 2026-07-13 update: the native CSB runtime handoff now first requires the
    original `CSB_SAVE_HEADER` magic and then a successful `LOADSAVE.C F0435`
    runtime load against the already verified dungeon. It cannot fall through
    to CSBWin or roster import. The missing evidence remains external
    per-media original-save corpus, not a synthetic substitute.
  - 2026-07-14 update: the HINTLOAD CPSX path now has a source-defined PC34
    transport boundary for `F1910`, `F1913`, `F1914`, and F1918's initial
    `GLOBAL_DATA`, `ACTIVE_GROUP`, and `PARTY` reads. It consumes the exact
    512-byte `CSB_SAVE_HEADER`, extracts its original `Keys[0..2]` and
    `Checksums[0..2]` at `DEFS.H` offsets `0x138/0x158`, and rejects a bad
    header or part at HINTLOAD's original stage code. The caller must still
    supply each media's exact record span; EVENTS, TIMELINE, dungeon-tail
    allocation, platform identity, and all positive corpus proof remain open.
  - 2026-07-14 update: F0435's subsequent EVENTS and TIMELINE transport now
    consumes only caller-admitted original byte spans with header
    `Keys[3..4]`/`Checksums[3..4]`, and calls the required F0434 dungeon-tail
    loader only after both checks succeed. It does not infer EVENT records,
    heap semantics, tail allocation, DSA, or a runtime from fixtures. A real
    CSB corpus was not available locally; per-media positive evidence remains
    required before this boundary can admit a live save.
  - 2026-07-14 update: F0434 now has a sequential PC34 dungeon-tail byte
    boundary: the caller supplies its exact 22 original spans, each is read
    in F0434/F7063 order, and F0421's 16-bit byte accumulator must match the
    trailing source checksum. It does not infer headers, map/ThingData sizes,
    allocations, or runtime publication. A real CSB tail is still required to
    bind this transaction to an actual media layout.
  - 2026-07-14 update: the post-C4 F0651 free-list rebuild now scans only
    caller-admitted raw EVENT records, rewrites `EVENT_NONE` overlays in
    source order, and exposes the source first-free/largest-used result. It
    does not admit a C4 heap, event dispatcher, DSA program, or runtime from
    test data. A real CSB save still must establish the PC34 EVENT stride and
    bind this result to a live load transaction.
  - 2026-07-14 update: F0652 now source-merges C05..C10/C01/C02 records only
    through Firestaff's existing native EVENT/TIMELINE owner and F0237 delete
    transaction. It does not create a second queue, add or dispatch an event,
    or execute a DSA action. Real CSB media remains required before this
    helper can be selected by a live restored queue.
  - 2026-07-14 update: F0655/F0656 now provide the source-defined PC34
    bitmap-prefix copy/flip and F0635-resolved transparent viewport-blit
    boundary. Both accept only caller-owned real bitmap pointers, layout
    resolution, and F0132 renderer state; neither decodes a graphic, selects
    a layout, or creates a fallback event/DSA/runtime. Real CSB graphics and
    save corpus evidence is still required before this boundary is bound to a
    restored live viewport.
  - 2026-07-14 update: F0657/F0658 now route caller-owned bitmap indices
    through the original F0630 `STRUCT2` offsets and, for F0658, the F0635
    layout-relative offset before the PC34 F0132 viewport dispatch. Bitmap
    lookup, layout data, and renderer state remain external real-data owners;
    this adds no graphic decode, index fallback, synthetic event, DSA, or
    runtime selection. A real CSB graphics corpus still must bind the route
    to live HUD/viewport data.
  - 2026-07-14 update: F0662/F0663 now route caller-owned bitmap prefixes
    and palette-change bytes through the exact F0129 dimensions contract.
    F0662 stays in-place; F0663 copies only the source's four-byte dimensions
    prefix before the renderer dispatch. This does not validate, decode, or
    substitute palette/graphic data, and it does not infer F0661 derived-cache
    ownership, event, DSA, or runtime behavior. Real CSB palette and bitmap
    corpus evidence remains necessary for live HUD/viewport binding.
  - 2026-07-14 update: F0661 now follows the source-derived bitmap cache
    branch exactly: a cache hit returns the caller-owned derived bitmap
    unchanged; a miss obtains caller-owned native/derived bitmaps, writes the
    requested dimensions, dispatches F0129, then admits the derived index to
    the caller-owned cache. It creates no cache storage, image, palette,
    event, DSA, or runtime fallback. Real CSB cache/graphics corpus evidence
    is still required for a live HUD binding.
  - 2026-07-14 update: F0664 now owns the bounded source wall-click
    transaction over caller-owned live input and sound callbacks: the
    no-champion guard, closed imaginary-wall press/release state, left-button
    sample, pointer hide, ordinary wall thud, and input-wait stop flag. It
    does not infer mouse coordinates, dungeon cells, graphics, events, DSA,
    or a replacement sound route. Real CSB runtime input/audio binding is
    still required for end-to-end play evidence.
  - 2026-07-14 update: F0665 plus the PC F0362 zone gate now routes a
    caller-owned F0638 zone through source screen-update enable, copied
    highlighted zone, F0698 invert, enabled state, screen-update disable, and
    F0693 vertical-blank wait. An unresolved zone leaves state/video untouched.
    The adapter supplies no layout, framebuffer, menu event, DSA, or runtime
    substitute; real CSB menu/HUD zone and video binding remain required.
  - 2026-07-14 update: F0666 now has the explicit PC endgame handoff over
    caller-owned state/callbacks: hide the pointer until the original request
    counter is positive, close `GRAPHICS.DAT` twice, restore CPSX, then enter
    the caller-owned endgame jump boundary. It does not recreate credits,
    endgame bitmaps, media loading, or a replacement nonlocal transfer.
    Real CSB endgame runtime/capture evidence remains required.
  - 2026-07-14 update: F0670/F0671 now cover the source text helper pair
    used by save-path and out-of-memory UI routes: replace only the first
    caller-selected character with a caller-owned string, and format signed
    PC-long values as decimal text. Destination ownership/capacity remains
    with the caller; this adds no localization, allocation, dialog, or menu
    fallback. Real CSB UI binding remains required for end-to-end evidence.
  - 2026-07-14 update: F0672/F0673 now resolve the nine source-ordered,
    caller-owned mouse-input tables through the caller-owned F0638 zone
    callback. The command-none sentinel, `-2` viewport and `-3` centered
    viewport offsets, and inclusive X2/Y2 endpoints follow COMMAND.C. The
    bounded adapter adds no input layout, zone geometry, menu event, HUD,
    DSA, or runtime fallback; real CSB table/video binding remains required.
  - 2026-07-14 update: F0674 now preserves the PC viewport floor/ceiling
    copy path over a caller-owned bitmap cache: F0631 lookup, F0653 byte
    count, then an exact copy into caller-owned destination storage. The
    bounded adapter rejects undersized storage rather than partially copying;
    it supplies no graphic, cache, palette, viewport, HUD, or runtime fallback.
   - 2026-07-14 update: F0676/F0677 now execute the source D3L2/D3R2 branch
    ordering through caller-owned draw callbacks, including wall return, door
    passes, pit, teleporter field, and PC cell orders. No geometry, bitmap,
    dungeon, or render fallback is supplied.
  - 2026-07-14 update: F0678/F0679 now execute the PC D2L2/D2R2 wall or
    teleporter-only branch over caller-owned callbacks, preserving C05/C06
    swap, C707/C708 zones, field aspects, and wall early return. No geometry,
    bitmap, F0111/F0115, or runtime fallback is supplied.
  - 2026-07-14 update: F0685 now provides the PC IMG3 packed-nibble line fill
    over caller-owned destination storage for real image expansion paths. It
    allocates no bitmap and supplies no palette, image, or renderer fallback.
  - 2026-07-14 update: F0684 now preserves PC I34 viewport blit dispatch:
    zone dimensions, even source/destination strides, vertical/horizontal flip,
    and caller-owned opaque/transparent line primitives. No bitmap or display
    buffer is created by the adapter.
  - 2026-07-14 update: F0686 now preserves the PC IMG previous-line copy over
    caller-owned packed bitmap data, used by real IMG/IMG3 expansion. No image
    allocation, synthetic pixels, or decoder fallback is supplied.
  - 2026-07-14 update: F0687/F0688 now preserve PC IMG3 nibble and run-count
    decoding over caller-owned source bytes. No image material or decoder
    fallback is supplied.
  - 2026-07-14 update: F0689 now expands real caller-owned PC IMG3 records
    through their header, six-entry local palette, command stream, F0685, and
    F0686. The bounded adapter accepts the source even-stride branch only;
    padded-row variants remain explicitly unclaimed rather than synthesized.
  - 2026-07-14 update: successful F0689 PC IMG3 expansion can now flow through
    a caller-owned CSB asset-presentation callback. Truncated or unsupported
    records are not presented, and the adapter supplies no display layout,
    pixels, palette, or fallback.
  - 2026-07-14 update: F0690/F0691 now provide the PC 3.4 IMG3 screen path:
    source-backed compressed records are validated before each decoded row is
    delivered through the caller-owned F0690-style video sink. Command-6
    retained pixels, local palette entries, and the original 320-pixel line
    boundary are preserved; no clipping, generated graphics, or fallback is
    supplied.
  - 2026-07-15 update: CSB startup surfaces now consume the real
    LZW-decompressed PC34 IMG3 record through F0691 and bind only a complete,
    header-matching indexed result to title/entrance/HUD consumers. Unknown,
    truncated, and non-IMG3 records fail closed without a generated surface.
  - 2026-07-15 update: authenticated 320x200 title/entrance/HUD rasters can
    now reach a caller-owned PC34 packed page through F0692's original black
    fill and F0693 vertical-blank gate. Non-PC34 dimensions, palette indices
    outside 0..15, and missing VBlank delivery fail before presentation; no
    fallback raster is created.

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
  - 2026-07-14 update: the bounded PC 3.4 path now decodes canonical
    `GRAPHICS.DAT` C001-C005 with ReDMCSB LZW.C chunk-width semantics before
    IMAGE3 expansion. Real-data regressions lock TITLE.C PRESENTS/CHAOS/
    STRIKES palette/raster output and ENTRANCE.C C004/C002/C003 y=30 output.
    This evidence does not extend to other media branches.
  - 2026-07-16 update: the PC34 startup presenter now refuses to pack or
    VBlank-present a title/opening/HUD page unless it is the exact indexed
    raster named by the CSB host-surface receipt, including route and pixel
    hashes. This is a presenter-receipt hardening only; external original
    Mac/app capture remains required.

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

- REDMCSB-CSB-GAP-012 — **ReDMCSB has no complete PC-CSB boot-media
  specification.** Its common source describes game control flow, but it does
  not supply a canonical DOS loader, verified title/entrance asset directory,
  or frame/audio capture for each PC CSB disk variant. Firestaff risk: a
  title/PRESENTS/CHAOS/STRIKES sequence can be source-shaped yet select wrong
  bitmaps, palette latches, cadence, or audio. Required independent evidence:
  hash-indexed PC CSB media, decoded asset-offset receipts, and frame-numbered
  original startup captures; do not manufacture a missing startup frame.

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
  projectile families (M715, GRAPHICS.DAT 454/455; M716, 457/458; M717,
  460/461/462; M718, 463/464) resolve and composite decoded original indexed
  pixels through a CSB-owned F0115 lane, including the STARTUP2 D2/D3 palette
  rows, C10 transparency, flips, and 224x136 viewport clipping. M718's
  type-2 source row has only the 0/1 bitmap deltas, so 465 remains blocked.
  The first object family now resolves Chest (498), alcove Chest (499), and
  Scroll (500) through the PC34 G0237 -> G0209 -> M612 route. The resolver
  accepts F0115's zero-cell-order alcove predicate, while the open-square
  runtime collector remains correctly unable to assert that predicate. The
  complete native G0209 object band through GRAPHICS.DAT 583, including all
  armour and junk subtype rows. Giant Scorpion, Swamp Slime, Giggler,
  Screamer, Rockpile, and Ghost are the source-proven PC CSB group fronts:
  G0243 -> G0219 -> M618 maps types 0/1/2/6/7/8 to GRAPHICS.DAT
  584/588/590/603/605/607 through the F0115 C3200 occlusion band, with
  aspect-owned C13/C11/C4
  transparency and G0222/G0221 D2/D3 palette rows. Wizard Eye, Pain Rat, and Ruster (types 3-5) now use
  original PC graphics 594/596/600 only through the F0093 receipt: the live
  map order and full PC Graphic 558 aspect/replacement-set data must resolve
  before the CSB C3200 compositor consumes native D2/D3 remaps. Broader
  creature families/poses and real-session pixel capture remain open; do not
  bypass the receipt with a per-creature substitute palette.
  Greatstone `d_items.html` is retained only as supplemental IMG5
  extraction-format evidence for future object/aspect work; ReDMCSB/CSBWin
  remain authoritative and it does not widen the native object map. Continue
  the remaining creature families/poses and the positive object-as-projectile branch;
  real sessions still block rather than draw an icon or marker when source
  material is absent. The opt-in
  `csb_v1_f0115_first_projectile_real_asset_pc34_compat` regression consumes
  `FIRESTAFF_CSB_GRAPHICS_DAT` and is neutral without user media; the matching
  group-material regression is
  `csb_v1_f0115_first_group_real_asset_pc34_compat`.

## Chaos Strikes Back (CSB)

### CSB V1

- 🔧 Phase 2 - Dungeon data model: synthetic CSB dungeon loader/model probe exists, loader/free-cycle safety is covered, and the PC real-asset launch gate proves canonical CSB assets enter the runtime-owned dungeon singleton. Core PC runtime/input/movement slices are now CTest-registered for command chains, input-queue binding, one-step movement, rotation between steps, runtime tick accumulation, queue overflow, reincarnation penalty, projectile speed, Grey Lord combat, DECOMPDU, version-checker sensors, monster generator state, chaos cast cooldown/targeting, one DSA trigger step, save import path, save runtime boundary, Neophyte mode, and Zokathra spell handling. Remaining work is deeper end-to-end gameplay parity, real save compatibility artifacts, viewport/UI runtime evidence, and playability without DM1-only assumptions.
- 🔧 Phase 3 - Rendering parity hardening: D3/D2 wall tables, bitmap selection, grid routing, CSB-only D3L2/D3R2 and D2L2/D2R2 draw-order/frame gates, F0107 back-wall ornament routing, initial viewport gates, and the 2026-06-21 CSB-only viewport CTest slices are in place. The CTest rows now cover first CustomBackgrounds backdrop, room-slot backdrop-1, D1C F0108 floor/ceiling ornament, D1C F0115 thing pass, D3C F0107/F0108 first-backdrop composition, D3L/D3R sidewall backdrops, and D2C F0107 wall-ornament plus F0111 door-front layering without game data. **2026-06-26 CSB V1 PC real-asset ornament blit probe landed, hardened 2026-06-29:** `firestaff_csb_v1_pc_real_asset_ornament_blit_probe` parses the DMCSB1 BE GRAPHICS.DAT header from a real PC 3.4 CSB pair, drives the source-locked F0108 zone + C10_COLOR_FLESH transparency + F0115 thing-pass math against real bytes, bounds-checks the selected bitmap payload span, writes deterministic 320x200 PPM + SHA256 + JSON provenance manifest sidecars to `/tmp`, and now reads the manifest back to assert the schema, canonical MD5, capture SHA256, source anchors, tally fields, and non-claims are present (capture sha256 `5e489ae14354d791e12a9474bbb44027eaac1be8e1021491d9d88dcef8ba9de1`); CTest `csb_v1_pc_real_asset_ornament_blit` PASS 31/31 against `~/.firestaff/data/csb`, skip-safe on hosts without user-supplied PC CSB data. Remaining work is broader viewport/HUD captures and pixel parity evidence.
- 🔧 Runtime handoff: the old M12/M11 CSB launch-readiness blocker is retired. Hash-matched CSB assets now produce a valid launch intent, M11 hands CSB to `FS_GAME_CSB`, and `csb_v1_pc_real_asset_launch` proves canonical PC CSB `GRAPHICS.DAT`/`DUNGEON.DAT` scan, `csb_v1_boot_enter_game()`, dungeon ownership, source-locked start pose, Chaos magic init, one tick, and cleanup. The CTest runtime set also covers command chains, input-queue turn binding, queued movement, collision/no-step handling, movement-disabled gating, turn-between-step ordering, utility/import handoff, and runtime load/attribute formulas. **2026-06-28 PC 3.4 quickplay dungeon-handle probe landed:** new `firestaff_csb_v1_pc34_quickplay_dungeon_handle_probe` (CTest `csb_v1_pc34_quickplay_dungeon_handle`, labels `tier1;csb;quickplay;boot_handoff;dungeon_handle;rescan;skip_safe`) pins the handle-survival invariants (H1-H4: runtime owns the verified DUNGEON.DAT handle after enter_game, global singleton equals the runtime handle, current level is map 0, runtime starts at TITLE) plus the rescan-clearing invariants (H5-H7: failed rescan clears the runtime handle + global singleton + profile fields + blocks re-launch) and the re-launch invariants (H8: successful rescan into verified dir releases the previous handle before enter_game re-establishes a fresh one). Probe is skip-safe — when `FIRESTAFF_CSB_PC_DATA` (or `~/.firestaff/data/csb`) carries the canonical PC 3.4 EN pair it exercises the real-asset path end-to-end; on hosts without user-supplied CSB data it falls back to a synthetic-fixture path (1 level, 2x2 legacy-format dungeon) that still drives the production `csb_v1_boot_enter_game()` / `csb_v1_boot_scan_assets()` code paths so CI stays deterministic. **Bug found and fixed:** the previous `csb_v1_boot_scan_assets()` only cleared profile metadata on rescan, NOT the runtime-owned `dungeon_handle` or the `csb_v1_dungeon_get_current()` singleton — so a follow-up rescan that lost the CSB assets could leave the runtime still pointing at the previous heap-allocated dungeon. The rescan path now releases the handle and resets the singleton before the rescan-driven profile fields are populated, matching the same release contract as `csb_v1_boot_cleanup()`. Remaining work is richer CSB-specific viewport/HUD evidence, gameplay/save/audio parity, original capture/pixel parity, and end-to-end playability verification.
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
