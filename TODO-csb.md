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
## CSB completed save-contract verification

- [x] 2026-07-28 Parse Atari ST CSB `ANIMATE.SCR` and its `ANIMATE.DAT`
  DMCSB1 container as the documented big-endian formats. The loader now
  honors DMCSB1's separate compressed/decompressed-size tables and preserves
  uncompressed items verbatim; the local original proves its 87-item layout.
  `ANIMATE.SCR` bounds-checks all 30 known
  big-endian instruction stream. Firestaff now bounds-checks all 30 known
  opcodes, their fixed word parameters, and the required Stop terminator.
  The local original 1,802-byte script from the Atari ST hard-disk package
  parses successfully. Original `ANIMATE.DAT` P4B1 palettes and 320x200 IMG1
  title frames are now consumed into RGBA after an exact script-to-item-family
  validation (80 loads, no foreign item family). The original command stream
  now has a source-validated semantic trace for loops, fades, waits, expands,
  blits, presents and sounds. Its final live screen now resolves to original
  IMG1 item 75 with original P4B1 item 21 solely through script state.
  Verified loose and stored-ZIP discovery identifies the exact original
  `ANIMATE.SCR`/`ANIMATE.DAT` pair by hash and refuses cross-package pairs;
  archive entries materialize into a hash-revalidated cache before rendering.
  The launcher-facing Atari route now discovers, materializes and renders the
  final source-selected frame from one data root.
  The two source `Set-screen` presentations are now individually available
  with the palette active at each original VBlank. The framebuffer player
  follows `ANIM.C`'s source-box clip, destination box, transparent colour,
  loop and item-attribute operations before M11 consumes the indexed frame.
  The host handoff now has the original indexed pixels and P4B1 palette;
  integration must preserve those 16 indices through M11 presentation. The
  same data-root route now exposes that indexed presentation in one call.
  Atari ST 2.0/2.1 M11 startup now consumes that first original `Set-screen`
  page directly. M11 now owns the Atari 50 Hz VBlank clock from the source
  55 ms startup cadence and caches the real replayed framebuffer per VBlank.
  The bounded player executes the local script's IMG1 expand/copy,
  source-rectangle clipping, display-coordinate and transparent blit
  operations, including the verified changed image between the two
  `Set-screen` pages. ReDMCSB `ANIM.C` and
  `PALETTE.C` now also drive Atari `FadeToPalette` timing: each source delay
  waits `delay + 1` VBlanks before the target P4B1 palette is committed.
  The direct Atari hard-disk launch now bypasses the incompatible PC34
  TITLE.C session and holds the original animation active in M11. Remaining
  Atari completion now uses ANIM.C's final VBlank as the FTLCODE program
  boundary and enters the already hash-verified CSB runtime from the same
  package; do not substitute the PC34 title/Entrance renderer for this route.
  Remaining Atari work is real app capture.
  A direct selected `.7z`/`.zip`/`.iso` is now an authoritative hash-search
  root, so a launch may not fall back to a sibling PC installation merely
  because the selected release is archived.

- [x] 2026-07-28 Consume the original Atari ST animation sounds. `ANIM.C`
  opcode 12 now retains the script-selected SND1 item, Timer-A period and
  source VBlank in the verified trace. M11 copies only those payloads from
  the same hash-verified `ANIMATE.DAT` root and plays them at their recorded
  source VBlanks. The SDL transport decodes `SOUND.C F0060`'s packed
  amplitude stream and applies the original F0061 three-channel loud table;
  it never substitutes a marker, PC34 SWSH sample or generated cue. The
  staged original proves item 86 then item 85, both at period 112. Remaining
  Atari work is real app capture with audible-device verification.

- [x] 2026-07-28 Verify the Atari ST end-to-end boot handoff with the local
  original hard-disk package. The direct `--game csb --data-dir` boot probe
  reached the original `animate-scr` phase at frame zero and, after 1,000
  frames, reported `startupActive=0`, `levelLoaded=1`, map 0, party 9,0,2,
  and runtime tick 259. This proves the ANIM.C final-VBlank handoff enters
  the package-owned CSB runtime without the PC34 TITLE/ENTRANCE route. Real
  interactive app-window and audible-device capture remain open.

- [x] 2026-07-28 Connect CSB V2 smooth-runtime to the production M11 CSB
  handoff. Enhanced modes now initialize and bind to the live V1 profile,
  consume V1 runtime ticks, advance at display cadence, and cleanly unbind on
  shutdown. The launch handoff also retains the V2.2 material gate's resolved
  mode instead of restoring an unadmitted V2.2 request.

- [x] 2026-07-28 Replace the fixed CSB new-game pose with the verified
  `DUNGEON_HEADER.InitialPartyLocation` route. ReDMCSB `LOADSAVE.C` F0435
  reads x, y and direction from the loaded header; Firestaff now retains and
  consumes that packed field in both boot paths. The local original PC
  `DUNGEON.DAT` resolves to map 0, `(9,0)`, direction 2. Native saves remain
  authoritative for resume positions.

- [x] 2026-07-28 Align the M11 quicksave/resume regression with native v12
  CSB package ownership. A standalone loader receives the original
  hash-verified package identity before validating the quicksave, so the test
  proves the package fence and the party/tick/projectile round-trip without
  weakening the production save boundary.

- [x] 2026-07-28 Reauthenticate the CSBWin `EDT_Skins` save-tail fixture with
  its FNV receipt. The phase-7 renderer now proves verified saved skin columns
  override dungeon defaults while a stale tail remains rejected.

- [x] 2026-07-28 Restore CSBWin `DSAINDEX::ReadTracing` state during both
  report-backed and file-backed CSBWin resume. A verified EXPOOL tracing
  record (`0x05070000`) now becomes runtime-owned DSA bitmap state, while an
  absent record remains valid and malformed/truncated EXPOOL data fails
  closed. The remaining CSBWin save task is a positive operator-owned corpus,
  not this previously disconnected authenticated field.
- 🔧 Nexus loot provenance: the former inferred DM1 drop tables were removed
  from production. Identify the actual Saturn creature/item drop records from
  DMWeb or an authenticated Nexus capture before re-enabling item or gold
  drops; no synthetic fallback is permitted.
- 🔧 Nexus item provenance: identify and bind the Saturn item-definition
  records before re-enabling inventory names, combat stats, or HUD item text;
  the former DM1 catalog remains reference-only and no synthetic replacement
  is permitted. The real ITEM.IBS → DGN Structure1F floor-item handoff is
  now regression-covered on retail LEV01 (eight declared records); this does
  not prove names, actions, combat stats, or HUD text.
- 🔧 Nexus door-state provenance: bind SDDRVS.TSK/retail door state and its
  open transition before allowing movement through type-8 squares. Missing
  registration now fails closed in both passability and square-event routes;
  the remaining gap is binding the registered door's state transition and
  animation to SDDRVS.TSK/retail capture rather than the bounded local state.
  Projectile consumers also stop at an unbound type-8 square instead of
  inheriting the generic non-wall passability rule.
- 🔧 Nexus FONT256 text binding: retail DMWeb map/page header facts are now
  exposed. The remaining gap is proving how page tilemap entries become
  runtime character codes before enabling text rendering. The engine retains
  the 242 real CG tiles diagnostically but keeps `font_loaded=0` until that
  mapping is source-bound.
- 🔧 Nexus HUD runtime binding: `DM.BIN` `yam\\menuctrl.c` geometry at
  `0x376D0` is now copied into the M11 startup handoff as separate source
  provenance (80 layout entries and 40 hit rectangles) and is verified against
  the European retail corpus. `startup_hud_ready` remains false: element
  surfaces, palette, FONT256 text and VDP1/VDP2 destinations still require one
  authenticated Saturn capture, so parser output cannot authorize HUD pixels.
  2026-08-06 audit: the V2 HUD runtime integration test now asserts the
  current no-draw invariant even under its force-active test hook; it no
  longer treats synthetic overlay pixels as evidence of a usable HUD route.
- 🔧 Nexus HUD click-route binding: the real `DM.BIN` ring-menu rectangle
  section at `0x38000` now parses and matches all 40 entries. Replace the
  removed screen-coordinate adapter only after an epoch/package-bound
  mounted-table handoff and an authenticated Saturn screen-input capture
  exist; parser output alone never enables interaction.
- 2026-08-06: Nexus boot validation now binds the champion-data diagnostic to
  the real hash-verified European `RLOWFIX.BIN` RES* archive (PLRD/CRET),
  replacing the obsolete `CHAMPIONS.DAT` placeholder check. The real-corpus
  boot hash regression confirms no false missing-champion diagnostic.
- 🔧 Nexus champion follow-up: PLRD equipment ordinals now bind to the real
  `ITEM.IBS` declaration category/weight lane. Bind the remaining ITEM.IBS
  text/action/combat semantics before exposing item names, attack, defense or
  key flags; the compatibility fixture still belongs only to isolated tests.
- 🔧 Nexus viewport material admission: the DGN host now fails closed for
  out-of-range or incomplete MNS/BPK/Structure2 surfaces. The remaining gap is
  still the authenticated Saturn PRS3 palette and VDP1 placement route; real
  MENU.BPK PRS3 output is retained as a deterministic diagnostic pixel hash,
  but no replacement texture or procedural viewport may be added.
- 🔧 Nexus startup menu pixels: M11 no longer paints the planner's procedural
  save/champion fill- and outline-rectangles. Bind `MENU.BPK`/`STABG.BIN`
  surfaces, text glyphs, palette and VDP1 placement from one authenticated
  Saturn capture before restoring any menu chrome or selection frame.
- 🔧 Nexus MENU.BPK regression root: `test_nexus_v1_bppk` now prefers the
  configured `FIRESTAFF_NEXUS_DATA_DIR` over `$HOME/.firestaff/data/nexus`,
  so the real external corpus is exercised even when HOME points elsewhere.
  The same root contract now covers the legacy startup/menu probes for
  `FONT256.S2D`, `FACE.BIN`, `TITLE.CG`/RES* and `STABG.BIN`; continue applying
  it to remaining probes before treating a local skip as evidence of a missing
  asset. The legacy DGN/LOGOBG/raw-binary and SAL/MAP corpus probes now use the
  same external-root-first contract as well; their decoded facts remain
  diagnostics and do not authorize guessed pixels, sound playback or event
  semantics.
- 🔧 Nexus startup title pixels: the M11 `BOOT_TITLE_FRAME` executor is now
  capture-gated like the title-background route. Bind the real `TITLE.CG` /
  `TITLE.BIN` composition and VDP1/VDP2 placement from an authenticated Saturn
  capture before enabling any title framebuffer write.
- 🔧 Nexus startup portrait pixels: `FACE.BIN` remains a verified asset
  receipt, but M11 no longer places it in the planner's guessed portrait
  rectangles. Prove the champion-index and Saturn VDP destination before
  enabling roster portraits in the startup menu.
- 🔧 Nexus ITEM.IBS gameplay semantics: live ITEM.IBS declarations no longer
  reinterpret carry-location bits as consumable flags, and the old fixed-ID
  DM1 potion/armor/melee routes are blocked for real Nexus data. Bind the
  Saturn action dispatcher and combat target/power semantics from DM.BIN/
  capture before enabling item use or player melee again.
- 🔧 Nexus CD/SFX handoff: a missing Red Book track now remains selection-only
  and is never submitted to the host callback as an empty path. Keep CD audio
  and SAL/MAP event playback blocked until the Saturn capture/decoder contract
  proves the source bytes, event selector, and host presentation together.
- 🔧 Nexus SAL diagnostics: the real DataID-0 tone-bank parser remains useful
  for bounded metadata/candidate inspection, but `SAL_BANK` runtime decode is
  now explicitly unsupported until Saturn sample-consumption and SDDRVS trace
  evidence proves the host PCM interpretation. Do not promote
  `sal_decode_ready` or a guessed selector into playback readiness.
- 2026-08-06: M11 Nexus save/champion pointer input now requires the exact
  route capture and package-bound input matrix. The retained fixed startup
  rectangles remain isolated compatibility geometry; they can no longer
  mutate live startup state without Saturn menu/input evidence.
- 2026-08-06: DGN scene planning no longer promotes the first Structure3
  model when no Structure1F owner row is visible. Real adjacent-cell facts
  remain retained, but mesh planning now blocks until the source-owned
  Structure1F → Structure1A → Structure3 face chain is present.
- 2026-08-06: Structure1F scene selection now additionally requires the
  source-recorded Structure1A owner cell to be party/forward/left/right of
  the active camera. A valid owner elsewhere in the level is no longer
  treated as the visible face; VDP1/raster semantics remain capture-gated.
# Theron V2 HUD widget pixels remain blocked in production: the manifest parser is fixture-only and the runtime now fails closed until all seven slots resolve to decoded Track 02 source assets.

- 🔧 CSB V2.2 artpack follow-up: the hand-authored per-cell asset-id catalog is
  contract-test-only; production retains just the F0128 source-provenance
  admissions. A reviewed PC 3.4 GRAPHICS.DAT pixel binding is still required
  before any modern art is admitted.

- 🔧 CSB V2.2 artpack follow-up: both mode selection and F0128 cache blits now
  reject a launcher flag or readable RGBA cache until the complete
  PC 3.4 source-material/provenance gate passes. The remaining work is a
  reviewed original GRAPHICS.DAT extraction and pixel binding; no generated
  cache or PBR substitute may be admitted.

- 🔧 CSB startup follow-up: M11's retired synthetic startup receipt probe is
  contract-only. Real package-owned startup receipts remain the sole active
  proof route; interactive app capture against the original corpus is still
  needed.

- 🔧 CSB Utility Disk CMP follow-up: production accepts CMP bytes only as a
  portrait/name/title overlay for an already authenticated champion. A
  positive original CMP-plus-save corpus is still needed before exposing that
  combined import route in the launcher.

- 🔧 CSB creature-drop follow-up: the old no-op fixed-possession API and
  no-context DSA stubs are contract-only. Bind original dungeon placement and
  the imported DSA interpreter before enabling either live creature drops or
  DSA filters.

- 🔧 CSB hidden-graphics follow-up: only the real source-loader is available
  in production. Bind a verified original GRAPHICS.DAT hidden-item corpus to
  a visible owner before promoting those records into a runtime presentation.

- 🔧 CSB Atari ST graphics follow-up: the production DMCSB1 reader accepts
  only user-supplied Atari ST data. Bind verified original animation/image
  records to the startup presentation before promoting this container reader
  beyond its current source-data loading role.

- 🔧 CSB Mac app-capture follow-up: an interactive capture of the installed
  PC 3.4 package reached real PRESENTS and closed Entrance, but its
  door-opening frame showed an opaque red centre strip. Treat that as a
  material/compositing regression in the older installed app until the current
  production build reproduces it and the C004/C002/C003 source ownership can
  be traced; do not hide it with a generated fill. 2026-07-31 current-build
  follow-up: the real PC3.4 package capture exposed a separate stale-frame
  defect at the end of the 20-VBlank pre-open wait. The source state reached
  an invalid step-zero gap and retained the closed C004/C002/C003 page. The
  sequence now enters F0807 step 1 atomically as that wait ends; real
  pointer-driven V1 captures prove that both indexed and presented opening
  frames differ from the preceding closed-door frames. The older installed
  app still needs its own current-build comparison for the red strip.
  **2026-07-31 installed-app recheck:** `/Applications/Firestaff.app` is
  v3.0.195. With the real hash-recognised PC3.4 package it captures the four
  title/Entrance palette phases, but its `--boot-probe` route reports
  `startupActive=0` before the scripted Prison click and never reaches
  `csb-entrance-opening-1`. It therefore cannot be used as a positive F0807
  opening-door capture; compare a rebuilt installed app against v3.0.197
  before diagnosing or masking the old red-strip report.
- [ ] DM1-HOC-OBJECTS-001 Capture the corrected live PC34 HoC wall-torch
  material and holder composition against the original GRAPHICS.DAT. The
  source mapping is now corrected to ReDMCSB I34E `G0194` (DUNVIEW.C:932-1007)
  and the exact `G0198`/`G0199` palette/depth route remains source-bound; close
  only after a real app capture proves the torch and holder pixels at each
  visible depth. No synthetic black ornament is admitted. Invalid global
  ornament indices outside the 60-entry G0194 table now fail closed; the
  real capture is still required. Runtime now distinguishes the synthetic
  final local inscription slot from real global ornament 0, so a real
  ornament-0 torch/holder cannot enter the inscription path.
  - 2026-08-06 fallback audit: the remaining legacy wall/door/floor helper
    paths now fail closed unless the authentic per-map ornament table and
    decoded pixel buffer are present. They cannot manufacture a global
    ornament index or draw a dimension-only slot. This is code-side cleanup;
    the real Mac/window torch-and-holder capture is still open.
  - 2026-08-06 viewport-coordinate audit: the live M11 F0128 iterator uses
    normalized D3 outer-wall offsets `-1/+1`, while the raw F0115 D3L2/D3R2
    source contract also exposes `-2/+2` aliases. The C127 mirror admission
    now accepts both representations and keeps the real C346 backing material
    for `viewWallIndex` 0/1. Real PC34 all-cell coverage passes; Mac/window
    pixel capture remains open.
- [ ] DM1-HOC-OBJECTS-002 Capture a real PC34 HoC pickup/placement round trip
  for weapon, potion, scroll, container and junk objects. Confirm M564 names,
  C00/C01 hand masks and backpack ownership remain source-backed. The F0033
  icon resolver now matches the PC34 ReDMCSB bit layout for weapon charge,
  torch charge and scroll Closed state. Pickup now selects the rendered pile
  top and enforces F0302/G0038 AllowedSlots before moving an object into a
  hand, pouch or backpack. Invalid decoded subtypes now fail closed instead
  of silently rendering subtype-0 artwork. Mouse-up now also consumes the
  separate C211..C218 status-hand route, so dragging into another champion's
  hand no longer stops at the leader hand. Real Mac pickup/placement capture
  is still required. The supplied `Dungeon-Master_Misc_DOS_EN_TXT-doc.zip`
  (`Dungeon Master.txt`, chapter 1) is an additional behavioral reference:
  champion mirrors are opened with left click; the pointer must change into
  the held object's hand-shaped cursor; objects must be placeable in valid
  hand, sheath, pouch and backpack slots; and placing onto an occupied slot
  swaps the objects. It also states that some distant dungeon objects are not
  reachable, so capture must distinguish reachability from a missing object.
  The manual does not replace the required original PC34 runtime capture or
  the M564 name/slot evidence.
- 2026-08-06 source-runtime verification: the real PC3.4 alcove test now
  completes pickup-to-placement for Thing 5196 (graphic 511), preserving the
  source `AllowedSlots=0x40` mask and placing it in legal quiver slot C519.
  M564 name-table validity remains intact after placement. Remaining scope is
  real macOS/window capture plus the requested weapon, potion, scroll,
  container and junk corpus; do not reopen the source route without a failing
  real-data case.
  - 2026-08-06 source-identity hardening: the live DM1 F0115 floor and
    F0121/F0124 alcove consumers now require the raw PC34 `THING` record before
    resolving subtype or drawing an icon. Candidate viewport metadata can no
    longer manufacture a plausible but incorrect object when the source chain
    is incomplete; the real floor-item and alcove pickup/place tests still pass.
- 2026-08-06 update: the active legacy stairs helper now rejects dimension-only
  cache entries unless the authentic GRAPHICS.DAT surface is decoded
  (`loaded` and `pixels` are both present). This prevents an invalid stair
  cache record from reporting a successful draw and covering the source wall
  or floor. Real Mac capture of each visible stair depth is still required.
- 2026-08-06 update: the active DM1 zone-blit, door-ornament, destroyed-door,
  Thieves' Eye, and door-button consumers now use the same decoded-surface
  gate. Dimension-only cache records cannot reach `BlitRegion`/`BlitScaled`
  in those F0102/F0110/F0111/F0113 routes. The real PC34 sweep remains the
  authoritative data check; packaged Mac capture is still required.
- 2026-08-06 update: the remaining CSB Atari planar viewport path now also
  requires `loaded && pixels` for ceiling, floor and wall records, and the
  DM1 HoC C027/C040 inventory-input checks reject the same dimension-only
  cache records. This closes the last source-consumer cases found in the
  current M11 load scan; no capture requirement is being marked complete.
- 2026-08-06 update: the DM1 action/spell utility-panel admission now also
  requires decoded C010/C009 pixel payloads, not only loaded flags and native
  dimensions. A dimension-only cache record can no longer suppress the real
  source-owned panel route while leaving the action/spell strip empty.
- [ ] DM1-HOC-OBJECTS-003 Capture the live held-object cursor on the host window
  after pickup and during movement. The source framebuffer now invalidates on
  pointer motion and hides the host arrow while G4055 is occupied; close only
  after a real Mac capture proves the object-shaped pointer remains visible at
  the mapped pointer position. 2026-08-06 source-side proof: the real-data
  `test_m11_dm1_real_object_names` now verifies 169 non-zero F0702 pixels for
  `EYE OF TIME`; only the packaged macOS/window capture remains.
- 2026-08-06 source-runtime hardening: authenticated DM1 V1 F0287 bar graphs
  now ignore `FIRESTAFF_V1_BAR_GRAPHS=0` and never re-enable the retired
  horizontal host bars. The switch remains available for non-source/debug and
  V2 compatibility sessions. Real object-corpus and held-cursor tests pass;
  packaged Mac capture remains governed by the open capture items above.

- 2026-08-06 CI follow-up: the CSB V2 touch/controller test now has its
  source-required PC34 VGA palette module. Continue watching the main build
  matrix; this closes only the missing-link regression, not a presentation
  parity claim.
- Theron teleporter resolution now rejects unresolved object-ID links and
  cycles; restore positive legacy links only when backed by an authenticated
  Track 02/T900 record corpus.

- [ ] THERON-V1-TRACK02-JP-LEVEL-DATA: promote the authenticated Japanese
  Track 02 level-bank receipt into runtime only after the HuC6280 graphics
  consumer and tile/map ownership are disassembled. The JP raw BIN now has
  its own verified seven-block offsets and metadata table; this remains a
  source-bound runtime byte/hash/meta receipt, not a synthetic or inferred
  tile binding. The runtime copy is now lifetime-safe; decompression and
  tile/map/object ownership remain gated.
  The real JP dungeon-map table is now bound separately; object-record and
  graphics-consumer handoff remains gated. The direct `TQJP19.iso` projection
  now has its own seven authenticated level-block offsets and EOF-bounded
  hashes; no BIN offset is reused as a container guess.
  The runtime level receipt now accepts both direct ISO variants as well as
  the two raw BIN variants, while retaining only byte-range/hash metadata.
  2026-08-06 update: the runtime copy now also retains the source-owned
  `LE16(+2)-5` resource length and exact user-data end offset of the framed
  bitstream, so a future HuC6280 decoder cannot accidentally consume trailing
  bytes from the next level span. This is still a framing receipt only;
  decompression and tile/map/object publication remain gated.
  Decompression and tile/map/object publication remain gated.

- [ ] THERON-V1-TRACK02-VRAM-CONSUMER: bind the real VDC BAT/tile and VCE
  palette snapshot to the source-owned square/material/UI consumer. An
  instrumented Mednafen replay now emits exact 64 KiB VRAM and 1 KiB VCE
  snapshots; the production viewport can explicitly mount that pair through
  `FIRESTAFF_THERON_VRAM_SNAPSHOT` and `FIRESTAFF_THERON_VCE_SNAPSHOT`, and
  the real-capture regression verifies non-zero BAT/tile data, 154 tile/palette
  pairs and 512 palette entries. This remains a screen-space capture binding:
  `$2600` source-LBA joins, object/level records, square-to-tile semantics,
  and production dungeon/UI admission remain blocked until the HuC6280
  consumer is disassembled and tied to Track 02. The current instrumented
  build uses SDL 2.32.70 through `sdl2-compat` with dummy video, so it does
  not claim native Quartz/SDL2 capture parity.

- [ ] THERON-V1-HUC6280-RAM-CONSUMER: the real US/JP bank-$1f static support
  fragment at `$243e` is now byte-verified in both retail ISO projections.
  It proves the bounded bit/byte helper, bank-switch table and forward/reverse
  byte paths, but it is not the post-CD `$2600` RAM-loaded consumer. Capture a
  source-owned RAM instruction window around `$2400–$2800` with executing PCs
  before promoting decompression, tiles, maps, objects or HUD pixels.

- 🔧 DM2 HUD follow-up: M11 now leaves the accepted V1 runtime frame as the
  sole production HUD owner. The retired V2 compatibility blit used a static
  GDAT plan without SKProject's live GUI/session inputs, so it cannot return
  until complete per-command, party and champion-state receipts drive the
  original UI route. Diagnostic V2 HUD modules remain non-production only.

- [ ] DM2 SKSAVE runtime restoration: the corpus reader now follows the
  source-sized 60-byte `s_savegamebuffer` and its continuous timer bitstream,
  but still stops at the `DM2_READ_SKSAVE_DUNGEON` record-link boundary. Keep
  Continue and slot admission disabled until that complete source-owned
  record/object/possession restoration path is live; do not substitute D2RS
  fixtures, a test-session fixture, or inferred session state. The M12/M11
  handoff regression now deliberately
  has no synthetic quick-resume branch; the supplied PC-DOS SKSave corpus is
  covered only by its read-only real-data gate.
  **2026-08-06 audit:** the existing
  callback-only `DM2_READ_RECORD_CHECKCODE` transcript cannot be promoted as
  a raw-save reader: SKProject `sksvgame.cpp:880-881` selects the DB4 record
  SUPPRESS mask through `DM2_QUERY_CREATURE_AI_SPEC_FLAGS` (the authenticated
  `CREATURES[type].word(5) → v1d296c` chain, with optional GDAT override).
  `c_querydb.cpp::DM2_QUERY_GDAT_CREATURE_WORD_VALUE` returns scalar zero
  when that word is absent, so all eight supplied PC-DOS direct-root streams
  decode through the genuine `v1d296c[0]` row. The raw absence remains
  asserted for type 54 twice and type 127 once; it is not replaced with a
  GDAT mapping.
  The real-data creature-animation probe now opens the canonical DOS spelling
  `GRAPHICS.DAT` after the lowercase attempt fails, so this conclusion comes
  from parsed mounted media rather than an accidental filename-only skip.
  Its remaining source dependencies
  also include the c_record allocator/append graph, container-moneybox mask
  swap, timer links, `DM2_2066_062b` possession continuation and source
  item-bonus pass. Bind those real owners before retrying corpus promotion.
  **2026-08-07 audit:** added a source-faithful bounded decoder for the
  `DM2_2066_062b` 10-bit possession continuation stream, including its
  `0x1000`/`0x2400` link markers and fail-closed underflow behavior. It is
  callback-only until the authenticated `savegamep3` owner and record pool
  are connected; Continue and slot admission remain disabled.
  **2026-08-13 continuation-type correction:** `DM2_2066_062b` consumes its
  10-bit stream only for record types 9 and `0xE`; types 0 through 8 are the
  source's empty branch and consume no bits. The reader and its ordering
  regression now retain that boundary, preventing a type-5 link from shifting
  the later type-9/`0xE` continuations. This remains callback-only and does
  not admit a partial save.
  **2026-08-06 tile-link correction:** the bounded
  `DM2_READ_SKSAVE_DUNGEON` reader now retains each decoded tile-chain root
  through `set_tile_record_link`, matching `sksvgame.cpp:1390-1399`. This fixes
  a discarded ownership edge but does not promote the incomplete save into a
  resumable runtime session. **2026-08-13 moneybox-mask progress:** the
  callback reader and writer now switch DB10 child records to the source
  `v1d64c3` mask only while traversing a source-owned moneybox, matching
  `sksvgame.cpp:923-928` and the `DM2_IS_CONTAINER_MONEYBOX` predicate in
  `skgdtqdb.cpp:822`. A synthetic chain regression and the supplied real
  PC-DOS SKSave corpus pass; the c_record runtime graph, timer links,
  `DM2_2066_062b` possession continuation and item-bonus pass remain open.
  **2026-08-07 item-bonus sub-audit:** `DM2_RETRIEVE_ITEM_BONUS` now matches
  SKProject `bitem.cpp:22-44` for the non-equipped high-byte sign-bit filter
  and conditional contexts, with real PC-DOS GRAPHICS/DUNGEON name receipts
  still passing. This corrects the shared helper only; hero stat/light,
  weight, timer and `DM2_PROCESS_ITEM_BONUS` mutation owners remain gated.
  **2026-08-07 tile-byte correction:** the loader now passes each masked tile
  through a source-faithful preserve-read path and publishes it through an
  authenticated `set_tile` callback. SKProject `sksvgame.cpp:1277` mutates
  the live `t_tile`; the old temporary zero byte discarded unmasked tile bits.
  The regression restores a real type-2 tile from a SUPPRESS stream while
  retaining its source tile bits. The full record/object/timer restoration
  transaction remains gated.
  **2026-08-07 timer-link progress:** `DM2_READ_RECORD_CHECKCODE` now exposes
  source-owned DB14 `w_06 → c_tim::setA` and DB15 timer-match
  `index → c_tim::setB` through an explicit callback, matching
  `sksvgame.cpp:963-973` and `989-996`. The callback is optional for corpus
  readers and no timer array or index is fabricated; live timer-owner binding
  and complete GAME_LOAD admission remain gated.
  **2026-08-07 special-timer progress:** the bounded reader now mirrors
  `DM2_2066_197c` for timer types `0x3c`/`0x3d`: it requires the source
  `savegamew7` gate, reads one non-following record chain with `wvalueB`
  initialized to `OBJECT_END_MARKER`, and publishes that link only after the
  callback read succeeds. The timer array, record pool and GAME_LOAD owner
  are still external, so Continue admission remains gated.
  **2026-08-13 map-span progress:** the original raw-dungeon receipt now
  retains every source map's width, height, relative tile offset and
  authenticated raw tile-span hash. A new per-map receipt revalidates the
  exact span against the mounted `SKSave0-3` bytes, giving the future
  `READ_SKSAVE_DUNGEON` owner source geometry without reconstructing a map
  from DUNGEON.DAT or inventing tile/object links. Complete record-graph,
  possession, timer and runtime-session admission remain open.
  **2026-08-13 raw c_map capacity correction:** the mounted PC-DOS SKSAVE
  corpus serializes 44 `File_header::nMaps` entries even though the standalone
  G1 dungeon has 28. `DM2_V1_MAX_LEVELS` now follows the source six-bit map
  field, so all eight raw prefixes enter the byte-square c_map model with
  their authenticated dimensions and tile bounds. This establishes only the
  raw-dungeon container; record graph, possession, timer and Continue
  admission remain closed.
  **2026-08-13 AI-row correction:** `QUERY_GDAT_ENTRY_DATA_INDEX` returns
  zero for a missing `dtWordValue`; `QUERY_CREATURE_AI_SPEC_FROM_TYPE` uses
  that value as the real `table1d296c` index. The mounted PC-DOS corpus omits
  CREATURES word `0x05` for types 54 and 127, so both now use authenticated
  source row zero. All eight supplied direct-root streams decode; this only
  removes an incorrect corpus blocker and does not admit CCM or Continue.
  **2026-08-07 real possession-continuation gate:** the corpus regression now
  passes every genuinely decoded direct-root link, in source order, into the
  bounded `DM2_2066_062b` 10-bit continuation reader. The 135/135 real
  PC-DOS checks therefore cover both record-body consumption and the
  subsequent type-9/type-0xE continuation boundary. The receipt remains
  read-only; live record-pool, possession-index, timer and GAME_LOAD owners
  are still not connected.

- [ ] DM2 champion-mirror activation: the canonical PC G1 dungeon has 16
  source-addressed DB3 `Actuator::Type() == 0x7e` marker roots. Their raw
  `Actuator::Data()` values are retained. **2026-08-06 inventory correction:**
  all sixteen are the original `0x1ff`, not static hero IDs. SKProject
  `c_loadlevel.cpp:604-611` truncates that value to `0xff` and queues
  `DM2_MARK_DYN_LOAD(0x16ffffff)` before `c_hero.cpp` reaches
  `REVIVE_PLAYER`; direct `CHAMPIONS/255` text/stat rows are absent from the
  on-disk GDAT. The read-only first DYN4 selection pass is now verified
  against all 277 original category-0x16 rows (including 21 sound rows). Its
  source-independent part now materialises deduplicated raw blocks in RAM
  with the original length/payload/raw-index layout. The initial source sound
  state is also bound: an empty `DM2_SOUND7` queue and clear `v1e13fe[2]`
  admit the remaining sound records, while missing or failed state defers
  them. The hash-verified PC boot now joins its 16 G1 mirror roots to selector
  `0x16ffffff` and retains the complete 96-block/149,670-byte DYN4 selection
  inside the boot-owned graphics lifetime. Live champion selection remains
  blocked until event preconditions, possession transfer and session-state
  updates are connected. Do not treat the boot receipt as a playable New Game
  path. FM Towns remains separate: the selected HME-242 corpus has 134 DB3
  extension roots outside the currently decoded Towns record owner, so its
  mirror-to-DYN4 join stays closed instead of borrowing the PC continuation.
  `DM2_SOUND9` now retains the original unbound `w_00/w_05 == -1` state.
  `DM2_482b_0684` now binds that queue only against the same materialised
  DYN4 selection and preserves the source's pool-capacity stop. The remaining
  owner is the real `sndptr4` descriptor pool, sample format/header handling,
  decode lifetime and playback scheduling; do not admit playback from the
  binding receipt alone. **2026-08-13 activation-boundary progress:** the
  selection seam now requires the committed G1 mirror census to bind the
  source DB3 marker, raw actuator data and `0x16ffffff` dynamic-load key
  before returning its still-fail-closed result. It does not create a hero;
  `REVIVE_PLAYER`, possession transfer and session mutation remain open.
  **2026-08-13 signed-type parity:** the lifecycle receipt now preserves
  SKProject's signed `i8` hero type when the source marker byte is `0xff`
  (published as `-1` to `REVIVE_PLAYER`), while retaining the unsigned
  `0x16ffffff` DYN4 selector. The real 16-marker and 96-block PC-DOS probes
  remain green; this does not open party mutation or possession transfer.
  **2026-08-07 corpus update:** the three champion/DYN4 real-data probes now
  accept only an explicit `GRAPHICS.DAT` argument or
  `FIRESTAFF_DM2_DATA_DIR`; a selected unreadable corpus fails verification
  instead of being reported as an unavailable local fixture.
  **2026-08-13 selector-boundary progress:** the PC boot join now requires
  the exact source `0x16ffffff` champion DYN4 resource ID in addition to
  agreement across every authenticated G1 marker; unrelated selectors remain
  fail-closed.
  **2026-08-13 lifecycle-boundary progress:** the selection seam now also
  requires the addressed marker's source direction and source-derived dynamic
  selector before reporting a mirror binding. It still does not mutate a party
  or call `REVIVE_PLAYER` without the live GDAT hero-stat owner.
  **2026-08-13 raw-marker correction:** selection now independently requires
  the canonical PC G1 DB3 actuator value `0x1ff`, not merely its derived
  `0x16ffffff` selector. A mismatched raw marker with the right selector is
  rejected; party mutation and possession transfer remain open.
  **2026-08-07 real-data recheck:** the external worktree reproduced the full
  PC-DOS chain: 16/16 mirror roots, 96 DYN4 blocks, 149,670 selected bytes,
  payload hash `0xa0af7eca`, receipt hash `0x8ae00cc1`, and the M11 boot
  receipt. No static or synthetic champion row was admitted.
  **2026-08-07 record-owner progress:** the owned c_record pools now resolve
  validated PC G1 DB3/DB4 continuation ObjectIDs using the original 10-bit
  index and record stride. The real champion-mirror regression resolves all
  16 marker roots through that owner; hero creation, possession transfer and
  session mutation remain open.
  **2026-08-13 source-bound transaction progress:** the lifecycle seam now
  exposes a source-bound `SELECT_CHAMPION` transaction that requires the
  authenticated marker identity and every live mutation owner before it can
  commit. Its callback order follows `c_hero.cpp:1052-1200` (creation-map
  switch, signed `REVIVE_PLAYER`, first-party leader, tile possession
  transfer, champion-strip refresh, map restore, weight recompute). The
  mounted PC mirror receipt now drives a positive callback-order regression;
  production GAME_LOAD/session wiring and source hero-stat ownership remain
  open, so this does not yet claim playable champion selection.

- [ ] DM2 delayed movement ownership: `PERFORM_MOVE`'s real
  `glbIsPlayerMoving` path retains the old party pose and derives its countdown
  from live champion loads, wounds, walk-speed modifiers and global Aura of
  Speed. The active V1 runtime deliberately renders only the settled source
  pose until those G1 hero/inventory/spell-state owners are restored; do not
  reintroduce a host-defined one-frame floor/ceiling offset. The execution
  receipt now reports this as an unbound delayed pose instead of falsely
  claiming that interpolation entered; the source-owned pose/countdown
  owner and live real-data runtime handoff remain open.
  **2026-08-13 owner-chain progress:** the exact `3 * weight / max_load + 1`
  `DM2_ADJUST_STAMINA` amount is now retained per hero slot in the execution
  receipt instead of being discarded. It remains a receipt only; applying the
  writeback and publishing `glbIsPlayerMoving` still require the live c_hero,
  possession and tick owners.
  **2026-08-13 source-gate progress:** the execution receipt now implements
  SKProject `v4/skgame.cpp:2364-2372`'s exact half-step admission: no active
  prior movement, forward movement, backward movement subject to double-step/
  stairs, or an active table-to-move. It records `walk_delay >> 1` only when
  those source inputs are explicitly supplied; it still does not create a
  viewport offset or publish a live delayed pose without the original owner.
  **2026-08-07 UseAltic parity correction:** the half-step gate now requires
  `bEnableDoubleStepMove` for both forward (`xx == 3`) and backward (`xx == 5`)
  movement, while retaining the independent `glbTableToMove` escape. The
  previous forward-unconditional branch could enter a delayed pose with the
  source flag disabled; the live pose/countdown owner remains open.
  **2026-08-07 receipt-boundary correction:** `delayed_pose_unbound` is now
  reported only after the complete source half-step gate admits
  `glbIsPlayerMoving`; a walk-delay value alone no longer claims that a delayed
  pose was requested. The live pose/countdown owner remains open.
  **2026-08-07 global-state correction:** `DM2_CALC_PLAYER_WALK_DELAY` now
  receives `savegames1.b_04` once from the execution request, matching the
  SKProject global Aura-of-Speed owner; a per-hero compatibility byte can no
  longer fabricate a party-wide speed state. Live c_hero/session binding and
  countdown publication remain open.
  **2026-08-13 delayed-owner audit:** when the exact half-step gate admits,
  the execution receipt now exposes six missing live-owner bits (hero load,
  wounds, walk speed, Aura-of-Speed, current pose and tick/countdown). The
  proven mask remains zero for caller-supplied compatibility snapshots; no
  interpolation or viewport offset is enabled.

- [ ] DM2 creature animation-frame ownership: `DM2_1c9a_0958` now carries
  the source's exact `CreatureAnimationFrame::w0` bit-14 query through an
  explicit callback, but no live DB4 record → AI-info → animation-frame
  runtime owner is connected yet. Keep creature animation-state consumers
  fail-closed until that source traversal is bound; do not infer record or
  animation offsets from a fixture. **2026-08-13 boundary correction:** the
  real G1 DB4 material receipt now retains `info_slot`, `w8/iAnimSeq` and
  `w10/iAnimInfo` from the exact source record and includes them in its
  identity. Runtime no longer promotes the V5 FB/FC/FD route with fabricated
  command `0` and frame `0xffff`; the live CAII command and complete
  `DM2_1c9a_0958` traversal remain required before animation state can drive a
  player-facing frame. **2026-08-07 source-data progress:** added a bounded
  `DM2_1c9a_0958` GDAT `0xfc` fetch that applies `DM2_query_4E26(w2)` and
  rejects missing/out-of-range rows. The real PC-DOS `GRAPHICS.DAT` FB/FC/FD
  probe passes; the record/CAII owner is still required before runtime
  promotion.
  **2026-08-07 cursor-owner progress:** the bounded `DM2_1c9a_0958` GDAT
  query now binds `DM2_query_1c9a_02c3` to the actual DB4 `+8` cursor for
  static AI rows and rejects live DB4 rows until their real CAII slot
  (`record byte@5`, 34-byte stride, `+8`) is supplied. The mounted G1
  DUNGEON/GRAPHICS corpus proves 10 source DB4 roots through the static
  owner; no caller-filled animation base is admitted. Live CAII allocation,
  command/frame mutation and runtime animation publication remain open.
  **2026-08-13 FD-route progress:** the source-owned FD selector is now
  bounded to the real `CREATURES/type/dtRaw7/0xfd` rows and indexes the
  authenticated cursor `iAnimInfo` with the source four-way face byte, as in
  `v4/skcrture.cpp:1967-1978`. The mounted PC-DOS FB/FC/FD corpus passes this
  selector. A positive dynamic CAII runtime consumer and image-field
  publication are still open; no static DB4 map-chip row is promoted into a
  fabricated live animation.
  **2026-08-07 V5 step parity:** `CREATURE_STEP_ANIMATION_V5` starts from
  the source-owned `iAnimInfo` and advances only while FC `seqnext` is not
  `0x0f`; the selector no longer pre-increments the frame and therefore does
  not skip the real terminal frame. The mounted PC-DOS FB/FC/FD regression
  now calls the selector again from the admitted frame and requires the
  source terminal state to remain stable. Live CAII ownership and runtime
  publication remain open.
  **2026-08-13 cursor publication:** the production DB4→F9 handoff now
  carries the source record's `b5/w8/w10` cursor (`info_slot`, `iAnimSeq`,
  `iAnimInfo`) through `DM2_CreatureSprite`, the viewport render plan and the
  runtime render receipt. This prevents the source cursor from being
  discarded, while the live CAII owner and V5 frame publication remain
  explicitly unbound.
  **2026-08-13 0958-owner progress:** the exact DB4 cursor now also performs
  the source `DM2_query_1c9a_02c3`/`DM2_query_4E26` 0xfc read during boot
  materialization. Static AI rows retain the real `frame_bit14`, query index
  and blended value through the viewport/runtime receipts; dynamic rows retain
  an explicit CAII block. No command-0 or `0xffff` frame is promoted.

- 2026-08-06: PC-DOS startup's decoded `TITLE/0/4` surface is now named and
  receipted as an original GDAT image route, not a fallback. It remains the
  verified alternative only when `SHOW_MENU_SCREEN` has no source raw-screen
  record; generated menu text or rectangles remain forbidden.

- 2026-08-06: DM2's cross-platform CMake build now has its immediate Windows
  and macOS linkage faults corrected. Re-run the GitHub build matrix after the
  verified main push; retain the usual platform-specific test coverage.

- [ ] DM2 startup status-panel ownership: host-authored English status,
  inspector and log labels are now absent from the live menu path. Bind an
  actual `c_gui_draw`/dialogue status producer before displaying startup or
  resume feedback; the structured action and load results remain available to
  drive control flow without visible replacement text.
  **2026-08-06 boundary correction:** removed the remaining boot-failure
  labels (`DM2 ASSETS MISSING`, `DM2 ASSETS UNVERIFIED`, and related launch
  strings), the M11 DM2 startup/resume stderr substitutions, and the
  direct-start boot-profile stdout summary. Failure and resume receipts now
  retain structured results while leaving the visible status channel empty
  until the original GUI producer is bound. **2026-08-13 generic-failure
  cleanup:** the remaining `main_loop_m11.c` DM2 branch no longer injects
  `DM2 LOAD FAILED` or data-path instructions into the message popup; it keeps
  the structured launch-failure transition and clears all three visible
  lines. The incomplete source-owned status producer remains open.
  **2026-08-07 M12 boundary correction:** DM2 quick-resume and successful
  launch handoff no longer populate the shared launcher message view with
  synthetic `RESUMING SAVE`, title, `READY TO LAUNCH` or escape text. The
  launch intent remains intact, while the real DM2 bitmap load dialogue
  (`skguidrw.cpp:80-94`) remains the only permitted visible status owner.
  **2026-08-13 empty-panel removal:** successful DM2 launch/resume and the
  generic DM2 launch-failure callback now return M12 to its ordinary main
  view instead of displaying a blank host message panel. The launch intent
  and structured failure receipt remain intact; M11 can therefore hand the
  next visible frame directly to the source-owned `SHOW_MENU_SCREEN` or
  dialogue path. The actual source failure dialogue producer is still open.
- [ ] DM2 runtime action/save text ownership: action, shop, movement and save
  receipts are structurally silent until their matching original GUI/dialogue
  producer is connected. Bind that source-owned producer before rendering any
  player-facing feedback. **2026-08-06 inventory boundary correction:** the
  two M11 DM2 inventory rejection paths no longer publish the host-authored
  `DM2 INVENTORY GDAT REQUIRED` label; they preserve the source ObjectID and
  silently reject until `CHANGE_VIEWPORT_TO_INVENTORY` owns the real
  CHAMPIONS/INTERFACE_GENERAL surface and click route. **2026-08-07 action
  boundary correction:** removed the remaining M11-only `DM2` inspect-title
  fallback when a partial action receipt is returned; M11 now requires the
  source dialogue producer to provide the complete title/text pair.
  **2026-08-13 boundary correction:** M11 quick-save/load stderr strings are
  now suppressed for `M11_GAME_SOURCE_DM2_BOOT`; save/load operations still
  return their structured result, but no host-authored action/save text is
  exposed until the original DM2 GUI producer is bound.
  **2026-08-13 generic-loader correction:** `M11_GameView_QuickLoad` and its
  shared path helper now reject the DM2 route before the generic DM1 envelope
  reader can emit `SAVE HEADER INVALID`, `QUICKSAVE RESTORED`, or another
  host status. DM2 BACK and front-cell action receipts also clear the generic
  inspect/status channel; source-owned control receipts remain available.
  **2026-08-13 pre-resolver correction:** DM2 quick-save and quick-load now
  enter the source-owned silent boundary before shared path resolution. This
  prevents path-length, directory and other generic host errors from leaking
  into the DM2 status channel. The original `DM2_GAME_SAVE_MENU`/GAME_LOAD
  producer is still not connected, so the item remains open.
- [ ] DM2 GDAT structure loader: `DM2_READ_GRAPHICS_STRUCTURE` remains
  unavailable until the source's complete GRAPHICS.DAT transaction is ported:
  header validation, ULP offsets, ENT1, optional underlay data and image
  allocator setup. The compatibility seam no longer returns a synthetic valid
  receipt from caller-filled fields. **2026-08-06 update:** the boot-owned
  real GDAT loader now retains the original `dtWordValue(0,0,0)` setup word
  and its bit-5 sound / bit-6 image-cache decisions (`0x3e8` or `0x1f`),
  alongside the raw-offset, ENT1 and underlay receipts. This is source data
  from the mounted v4/v5 file, never a host default; the remaining work is
  wiring the source allocator lifetime and secondary-file transaction.
  **2026-08-13 progress:** the callback seam now consumes the real source
  header and ULP span, validates the `0x8000 | version`/entry-count contract,
  checks the cumulative ULP raw boundary against the mounted file size, and
  closes the source file on every success/failure path. ENT1, underlay and
  allocator admission remain gated. **2026-08-13 progress:** the same
  callback receipt now follows SKProject's little-/big-endian header, size
  word and ULP decoding contract, with a regression fixture for 68k byte
  order and the mounted PC-DOS GRAPHICS.DAT still passing. ENT1, underlay,
  allocator lifetime and the secondary-file transaction remain gated.
  **2026-08-07 allocator progress:** the validated source ULP words now remain
  owned by `DM2_V1_GdatFileState` after the file close and require explicit
  release, matching the source table lifetime. A second structure transaction
  cannot orphan the first table. ENT1 admission and the secondary-file
  transaction remain gated.
  **2026-08-07 LOAD_ENT1 progress:** raw entry 0 is now retained from the
  mounted GRAPHICS.DAT transaction and checked against SKProject's `0x8001`
  signature, independent raw-entry byte order, and T/I/D/S/F/G/P descriptors.
  The authenticated PC-DOS file proves 11,854 raw entries, seven groups and
  packed stride 8; the raw entry is released with the ULP/allocator lifetime.
  BUILD_GDAT_ENTRY_DATA, underlay admission and optional GRAPHIC2.DAT remain
  gated.
  **2026-08-07 source-layout correction:** the callback now follows
  SKProject `v4/skcore.cpp:15043-15103`/`v5/bgdat.cpp:1067-1095`: v4/v5 read
  the four-byte first ENT1 size at offset 4, load only entries-1 ULP words,
  and create ULP[0] as zero; v2 retains the all-word table variant. The real
  PC-DOS file now verifies as 5/5624 entries, first ENT1 size `0x17284`, raw
  base/end `11254/8639757`. ENT1 materialization, allocator lifetime and the
  secondary-file transaction remain gated.
  **2026-08-07 allocator-table progress:** the structure transaction now
  retains the source-initialized `w_table2`-equivalent table (`entries * 2`
  bytes, every word `0xffff`) alongside ULP and releases both together. The
  real PC-DOS GRAPHICS.DAT receipt verifies the full `5624 * 2` table. The
  later `LOAD_ENT1` population, decoded allocator tables and secondary-file
  transaction remain gated.
  **2026-08-07 field-layout progress:** retained ENT1 state now preserves each
  source-derived T/I/D/S/F/G/P byte offset and field size, rejects duplicate
  descriptors, and proves alternate descriptor order plus the mounted PC-DOS
  layout. The complete BUILD_GDAT_ENTRY_DATA transaction, underlay admission
  and secondary-file transaction remain gated.
  **2026-08-07 transaction-boundary progress:** the callback port now rolls
  back a failed primary or optional GRAPHIC2.DAT open, closes the already-open
  primary handle on the secondary failure path, and rejects close-counter
  underflow. This keeps the source GRAPHICS_DATA_OPEN transaction reusable
  after an I/O error without admitting any synthetic GDAT state. The complete
  BUILD_GDAT_ENTRY_DATA transaction, underlay admission and secondary-file
  transaction remain gated.
  **2026-08-07 ENT1 materialization progress:** the authenticated raw ENT1
  payload can now be materialized into source-shaped `T/I/D/S/F/G/P` rows,
  using SKProject's descriptor offsets and big-endian field-value semantics
  even for the little-endian DOS container header. The mounted PC-DOS
  GRAPHICS.DAT verifies all 11,854 rows and their nonzero receipt hash.
  BUILD_GDAT_ENTRY_DATA category/index allocation, decoded raw images,
  underlay admission and the optional GRAPHIC2.DAT transaction remain gated.
  **2026-08-07 source-table progress:** the verified ENT1 rows now feed the
  source `BUILD_GDAT_ENTRY_DATA` pass through a context-safe adapter. The real
  PC-DOS table builds 11,854 entries, maximum category 26 and 247 source
  subcategory slots with a stable row receipt hash. The returned category
  tables remain caller-owned diagnostic/source state; runtime graphics-cache,
  raw-image and underlay admission are not implied.
  **2026-08-07 raw-entry progress:** the retained source ULP table now drives
  a bounded raw-entry reader. It resolves index 0 from the authenticated
  first-ENT1 length and later indices from source ULP continuation lengths,
  checks the cumulative raw boundary and returns a source payload hash. The
  real PC-DOS regression reads raw entries 0 and 1; image decoding, allocator
  cache lifetime, underlay and GRAPHIC2.DAT admission remain gated.
  **2026-08-07 underlay progress:** a source-owned materializer now resolves
  the exact `dtRaw8/0/0` ENT1 row, reads its real four-byte image-to-underlay
  table through the ULP raw-entry reader, validates source raw-index bounds
  and sorted order, and returns payload/pair hashes. The mounted PC-DOS v5
  corpus has no such source row, so its regression stays fail-closed; no
  empty or synthetic underlay table is admitted. Positive underlay-corpus
  wiring and decoded overlay/cache ownership remain gated.
- **2026-08-07 save-dungeon parity correction:** the isolated
  `DM2_STORE_EXTRA_DUNGEON_DATA` teleporter gate now matches SKProject's
  `current_map > target_map` backward-reference skip; the complete raw-dungeon
  record allocator and runtime restore owner remain gated.
- [ ] DM2 combat source contract: a creature Defense GDAT row alone cannot
  author a player attack. Keep `dm2_v1_combat_resolve_attack_on_creature()`
  blocked until `DM2_ENGAGE_COMMAND`/`CALC_PLAYER_ATTACK_DAMAGE` has the
  live champion hand and CMDSTR action, target record, difficulty/light,
  item words, source RNG, and skill/stamina/poison writeback. The old host
  formula must not publish real damage, kills or combat feedback.
  **2026-08-06 cleanup:** removed the disabled partial-defense bridge; the
  production boundary rejects restoring its computed creature damage.
  **2026-08-13 progress:** the verified CREATURES→v1d296c AIDefinition
  defense byte is now retained in the combat receipt when a real provider is
  bound. This is evidence only; damage, kills and feedback remain rejected
  until the full `DM2_CALC_PLAYER_ATTACK_DAMAGE` owner chain is connected.
  **2026-08-13 boundary audit:** the source-shaped damage, wound and party
  player-attack and wound helpers are direct-regression seams only; no `src/`
  product source calls them, and the production-boundary verifier now locks
  that condition. This
  does not close the item: the live champion/CMDSTR/target/RNG/writeback owner
  chain is still absent.
  **2026-08-13 source-owner audit progress:** the receipt now reports all
  seven missing source owners separately from the one proven target-Defense
  byte; the real PC-DOS GDAT regression asserts this split, while damage and
  kills remain zero and the host formula remains unwired.
  **2026-08-07 source-roll correction:** the diagnostic hit-check seam now
  consumes five RNG bits (`rand_hit & 0x1f`), matching SKProject's
  `DM2_CALC_PLAYER_ATTACK_DAMAGE` source contract. A `0x0f`/`0x10` boundary
  regression is green; this does not admit the helper or any damage, kill or
  combat-feedback publication into the production runtime.
  **2026-08-07 party-wound correction:** the diagnostic `DM2_ATTACK_PARTY`
  seam now applies the source `DM2_MAX(1, per_hero_damage)` clamp before
  `WOUND_PLAYER`, matching `skhero.cpp:3365-3392`; a `base_damage=1` regression
  is green. The live champion/target/RNG/writeback chain remains absent.
- [ ] DM2 FM Towns English text consumption: a selected FM Towns Japanese CD
  can now carry an explicit, hash-verified PC-English GDAT text companion in
  RAM. The companion now admits both a direct user file and a selected
  `archive.zip::data/graphics.dat` member through bounded RAM-only ZIP
  extraction and the canonical PC-English MD5; it is never unpacked to disk.
  `c_dialog.cpp::DM2_dialog_OPEN_DIALOG_PANEL` now consumes the companion for
  its two original `DIALOG_BOXES/0x81/dtText/0..1` labels, retaining the
  native panel, raw4 rectangles, palette and source font. M11 delegates that
  active source command to the DM2 viewport renderer, so the English labels
  are now actually drawn through the original raw font and palette path.
  Bind the companion to each remaining original GUI/dialogue text call before
  claiming complete English UI; missing companion data blocks English FM
  Towns rather than substituting text.
  Both `c_gfx_str.cpp::DM2_QUERY_GDAT_TEXT` and the QueryDB relay now accept
  only explicit decoded companion callbacks and otherwise consume the selected
  GDAT unchanged; remaining work is wiring that callback to every remaining
  live M11 GUI/dialogue owner. **2026-08-06 coverage update:** the real-media M12
  regression and the FM Towns English start gate both walk every non-empty text
  entry in the selected Japanese GDAT and require a non-empty entry with the
  same `(category,index,field)` key in the authenticated PC-English companion.
  The check covers both a direct file and the original DOS ZIP member, entirely
  in RAM. It proves the companion corpus covers the selected CD's text keys;
  it does not claim that unbound GUI/dialogue consumers already render them.
  **2026-08-06 input audit:** the PC `D7 80 1C 00` Enter row is absent from
  the authenticated `SKULL.EXP` load image, so generic M12 keyboard tokens
  remain correctly blocked for FM Towns. Decode the native P3 input route
  before enabling it; do not import the PC table.
  **2026-08-06 END handoff:** the selected source `0xE0` Quit rectangle now
  replays the authenticated HME-242 `END` stream, including its FO/NE loops
  and per-frame PL palette selection, before returning to the launcher.
  This completes only the AUTOEXEC exit transition. Native P3 keyboard input,
  GAME_LOAD, save-resume and the remaining GUI text consumers stay open.
  **2026-08-13 callback progress:** the authenticated companion is now also
  exposed through a range-checked runtime callback matching generic
  `DM2_QUERY_GDAT_TEXT` consumers; real-media coverage verifies both
  `DIALOG_BOXES/0x81` labels through that bridge. The callback does not create
  text or enable any unbound GUI owner; native event/dialogue routing remains
  gated.
  **2026-08-13 live-owner census:** the current production M11 tree has one
  DM2 GDAT-text consumer, `m11_draw_dm2_save_dialogue_panel`, and it already
  enters through `dm2_v1_boot_dialogue_open_panel_host_command`, which passes
  the authenticated FM Towns callback into the source panel receipt. The
  source-shaped `0AAF`, QueryDB, GfxStr and generic GUI draw consumers have no
  M11 call site and remain excluded from product archives. The production
  placeholder verifier now rejects a future direct M11 call to those owners.
  This is an ownership audit, not completion of native event/dialogue routing;
  keep the item open until those source consumers are genuinely live and use
  the companion callback.
  **2026-08-13 owner-census hardening:** the production verifier now requires
  exactly one M11 boot-panel text call (the save-dialogue consumer), plus one
  definition and one render call for that consumer. Any new GUI/text route
  must be source-bound before it can enter the product tree; the native
  `c_dialog`/`c_gfx_str` event owners are still absent.
  **2026-08-13 direct-launch parity:** `firestaff --game dm2 --fm-towns`
  now accepts `--dm2-english-companion <PC-English GRAPHICS.DAT>`, forwarding
  that explicit path through the same M12→M11 launch receipt as the menu.
  The boot layer still verifies its canonical hash and keeps it in RAM; the
  option does not broaden text-consumer admission or unpack game data.

- 2026-08-06: the full 30-file retail MNS corpus now decodes without silent
  texture/MOTN truncation (VEXIRK=64 TEXT descriptors, D_GOLD=11 MOTN
  tables). Remaining work is original Saturn/VDP1 capture and source-locked
  face/mesh texture placement; parser success is not viewport proof.
- 2026-08-06: the MNS pose/texture helper is now excluded from the production
  Nexus library because its fixed-point Taylor trig and BGR555 conversion
  have no Saturn execution/capture receipt and no production caller. The
  real-data decoder test still compiles it explicitly; restore a production
  mesh route only after VDP1/VDP2 capture proves rotation, CLUT and draw order.

- 2026-08-06: DGN Structure2 texture decode now resolves DMWeb's real
  `Palette offset = 0` reuse rule by prior Palette ID association. The
  hash-verified LEV00-LEV15 corpus decodes 1,678 descriptors (1,553 indexed4,
  125 direct555). Remaining gap is Saturn VDP1 upload/CLUT and Structure3
  face-to-texture/draw-order capture; do not promote this byte proof to pixels.
- 2026-08-06: Nexus spell lookup remains available from the real DM.BIN table,
  but `nexus_v1_cast_spell()` is now side-effect free and returns `-1` until a
  Saturn dispatcher capture binds mana commit, effect/target routing, RNG and
  SLEV/SFX publication. The previous host mana/damage mutation was synthetic.
- **NEXUS-EVENT-DGN-OWNER-CAPTURE:** Real DGN Structure1F/Structure1B bytes
  remain retained as source evidence, but the runtime no longer promotes
  apparent door/teleporter/pit/stairs records into live registries. The
  verified corpus does not prove that low DGN bits select DM1-like events,
  nor that `SDDRVS.TSK` dispatches them. Original-Saturn capture must bind
  event owner, selector order, destination fields, and state transitions.
- **NEXUS-UI-EVENT-DISPATCH-CAPTURE:** Retail `nexus_mechanics_dispatch_event()`
  now rejects host UI events for ISO/extracted data until the Saturn SLEV/SDDRVS
  producer, queue and state-write contract is captured. The source-less fixture
  lane remains available for isolated tests. Bind the original event route before
  admitting automap, inventory, save, leader, throw or drop mutations.
- **NEXUS-LEVEL-TRANSITION-CAPTURE:** The public level-transition helper now
  rejects ISO/extracted transitions until the Saturn SLEV/SDDRVS owner is
  captured; the tick gate alone was insufficient because callers could invoke
  the helper directly. Bind the original transition producer, destination
  fields and level-load timing before enabling retail level changes.
- **NEXUS-BPK-NO-DRAW-REGRESSION:** The bounded PRS3 presentation receipt must
  continue to admit exact retail-shaped rows only as opaque no-draw evidence;
  decoder drift, payload/hash drift, unknown modes and malformed spans must
  remain rejected before M11. The previously inverted matching-row assertion
  is corrected and the focused BPK/M11/Saturn-card gates are green.
- **NEXUS-WORLD-SCRIPT-CLAIM-QUARANTINE:** The linked native world/save state
  now labels its event, timer, hash and provisional action vocabulary as
  Firestaff-native/test state rather than recovered SDDRVS/SLEV semantics.
  Keep the actual SLEV task body, callback owner, event selector and dispatch
  capture-gated; do not promote the compatibility enum into Saturn opcodes.
- **NEXUS-SAVE-ROUNDTRIP-STACK:** The manager-level native save round-trip is
  now verified with heap-owned test state; keep the serialized world contract
  unchanged while extending real Saturn-card save provenance separately.
-  - 2026-08-06 Nexus PRS3 capture-schema correction: the real retail
    `MENU.BPK` MD5 admission constant was stale (`c277...`) while the
    verified corpus and boot profile use `a6f2272a4f6cb3c6b3b33012bc5b15ed`.
    Update the capture-sidecar evidence only; Saturn authentication and
    runtime texture upload remain blocked until independent VDP1 capture.
-  - 2026-08-06 Nexus production-source boundary now has a CTest verifier.
  It keeps synthetic V2 HUD/renderer modules and unproven text/MNS
  presentation paths out of `firestaff_nexus` during future source-list edits.
-  - 2026-08-06: `.github/workflows/verify.yml` now hard-runs that data-free
  production-source boundary after the cross-platform Nexus library build.
  Real retail-media and Saturn-capture tests remain local by design.
2026-08-06 regional capture follow-up: the same private CUE normalization
now accepts the archive's Japanese `TQJP02.iso` alias and binds the complete
sibling `TQJP02End.iso` only after the authenticated JP ISO MD5 matches
`397039af02d50d15c70b74088eb8a1cb`. The new generic `THERON_CUE` variable
retains `THERON_US_CUE` compatibility. A fresh JP consumer capture remains
required before semantic promotion.
- **CSB-AMIGA-LIVE-AUDIO:** M11 now transports the selected authentic Amiga
  `GRAPHICS.DAT` sample bytes through the F0709 period calculation
  (`ioa_Period = 72800 / SOUND_DATA.Period`) rather than falling back to the
  PC3.4 PIT/marker route. The remaining Amiga work is source-captured
  audio.device voice allocation, left/right volume arbitration and overlap
  behavior; do not infer those from PC3.4's distance-volume model.
- 2026-08-07: An authentic European Mednafen capture now records a 48-word
  SH-2 code window around the VDP1 source writer at runtime PC `0x06013098`
  while it writes `0x47c00`. The routine contains a real branch to
  `0x06012f52`, but relocated/decompressed code is not yet joined to an
  authenticated DM.BIN/TM.BIN source span. Keep VDP1/VDP2 composition and
  production draw admission blocked until that identity and command/CLUT
  contract are proven.
- 2026-08-07: The authentic high-RAM load trace shows 3,080 writes into the
  `0x06013000..0x06013fff` code corridor from runtime loader PC `0x2368`.
  This is a BIOS/runtime-loader receipt only; the trace does not yet expose
  the CD source read or identify the retail member that supplied the bytes.
  Keep the VDP1 source join and production composition blocked.
- 2026-08-07: The Saturn-CDB hook now traces the real `cdb.cpp` data-sector
  path. The current bounded run reaches only BIOS LBA `0..16` (1,024 reads);
  no `DM.BIN`/`TM.BIN`/other retail member has been joined yet. Continue with
  a capture route that reaches the authenticated game startup window; do not
  promote the BIOS sector receipt to SLEV/SAL or VDP1 source evidence.
- 2026-08-07: The corrected input ordering now reaches the authenticated
  French Nexus startup window. A 50,000-read CDB trace joins `DM.BIN`,
  `TM.BIN`, `ITEM.IBS`, `MENU.BPK`, `SLEV00.BIN`, `SDDRVS.TSK`, DGN and SAL
  spans to the retail ISO; the new analyzer reports this as LBA provenance
  only. The same run records 3,080 runtime-loader writes and one raw frame,
  but no VDP1 writer trace. Keep PRS3 pixel consumers, VDP1/VDP2 composition,
  HUD/viewport, SLEV/SAL/SDDRVS semantics and SFX playback blocked pending a
  live producer/consumer join to those authenticated bytes.
- 2026-08-07: The source-read/source-write witness now joins complete 4 KiB
  runtime code chunks to `TM.BIN` (`0x74f3000`) and `DM.BIN` (`0x5e000`) in
  the retail ISO, while the same bounded startup capture observes VDP1
  writer PC `0x06013098` writing `0x47c00` and eight active raw frames. Keep
  face/mesh/texture decode, VDP2 tilemap/CLUT placement, HUD/viewport
  composition and SLEV/SAL/SDDRVS playback blocked until the live writer and
  consumer contract itself is decoded and source-owned.
- **DM2 SKSAVE direct-root pool ownership:** The raw DB baseline and DB4–DB15
  clear phase are now followed by source `READ_RECORD_CHECKCODE` allocation
  into the authenticated c_record pools, including source list links,
  child-owner fields, type-9/type-0xE continuation writes, and a hash/count
  receipt. Remaining work is attaching the returned roots to champion/hand,
  possession-index and tile-chain owners; failed decode restores the cleared
  baseline and never publishes a session. The mounted workspace has no raw
  SKSAVE corpus, so this positive path remains compile/test-gated until one is
  supplied.

# DM2 PC-DOS File_header continuation and champion activation (2026-08-07)

- [ ] Derive the PC-DOS record/map continuation after the 44-entry
  `File_header` from an original-loader trace. The former 28-map pseudo-header
  accidentally produced 16 champion mirrors and a DYN4 selection; it is not
  valid evidence and must not be restored. Champion selection remains gated
  until the real DB3/DB4 ownership and marker route are independently proven.
