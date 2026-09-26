# Firestaff TODO — DM1

- Reproduce and correct the remaining reported retail HoC interaction/viewport
  set as one source-locked pass: pickup-to-inventory and object-bearing Eye
  scroll reads, Vi
  altar wall material, stairs occlusion, complete door
  composition, pressure-plate near/far visibility, wall-torch placement, and
  held-item striping/loss on inventory placement. A private original PC 3.4
  run now visibly covers the post-resurrection FOOD/WATER panel, an item in
  the action hand, and that item in a backpack slot. It is useful reference
  evidence for those three poses, but it does not prove a floor pickup or an
  Eye-scroll read. Existing raw-format pointer, F0128 scheduler, and
  individual material tests are necessary but do not prove these live
  combinations. Use an authenticated PC3.4 HoC route or a
  generated-in-original-runtime C13 save as the state driver; do not promote
  a hand-made save or substitute graphics from another platform. Capture the
  exact square/pose/input and compare source-order command receipts before
  changing a renderer or inventory transaction.

- Extend the validated original PC 3.4 same-state C127/C040 comparison through
  a real pickup. A prior private capture set with three bit-identical Hall
  frames must not be used as source-modal proof. The separate private retail
  route visibly contains a C040 panel followed by C007 inventory state. A
  same-state native C040 measurement is now recorded, but it does not prove
  C007 or item-transaction parity. The historic retail hybrid
  route is mouse-mode launch (`DM -vv -sn -pm`), but `(260,84)` is the
  Resume row, not a verified Enter selection. Current DOSBox-X/Xvfb runs
  reproduce the keyboard Return handoff into the no-party Hall frame while
  source-space mouse-button transitions remain unproven.
  keypad-Enter (`0x001c`), an approximately eight-second door animation, and
  the 28-command `KP5/KP1/KP2/KP6` sequence before the C127 click `(112,83)`.
  Use the visibly distinct source C040 geometry to compare material/cadence
  to the same-state native frame, and then extend it through a real inventory
  interaction. A dedicated all-mouse movement route remains desirable, but it
  must not replace the verified keypad sequence or be inferred from it. Keep
  raw captures operator-local under `.codex-scratch` and do not substitute a
  save, graphics, or a Firestaff-native result for this evidence.
  The authenticated route's recorded `(22,14)` click keeps the dungeon page
  visible in both the original capture and current native run; its historical
  `inventory` screenshot label is therefore not proof of a C017 transition.
  A later authenticated PC 3.4 route establishes C017 and a labelled
  empty-hand Eye hold after the C127/C040 recruit flow. The source Eye hit
  area `(20,53)` produces a verified held/released visual delta; the earlier
  `(24,77)` coordinate was a rejected cursor-only diagnostic. It does not
  establish a held object, an object-name/scroll read, or any
  pickup/placement result; those still require a real object-bearing route.
  A 2026-09-09 headless DOSBox 0.74 retry with
  the prior PC 3.4 stage reached the authentic Entrance screen but did not
  deliver its subsequent Hall keypad/mouse commands in either global or
  window-addressed X11 mode; its raw frames remain capture-health diagnostics
  only and must not be relabelled as C017. Reproduce this route in a DOSBox
  input environment that records the original command consumption before
  comparing C017 or pickup pixels.

- Add an original-media trace that covers F0205's G0395/G0396 half-pair owner
  through a same-tick map transition and slot reuse. The native transient
  source-slot implementation is covered by bounded C38 dispatch and lifecycle
  regressions, but has no authentic emulator trace yet.

- Validate the remaining attack-entry C32-C36 source decision tree. The raw
  active-group dispatcher already routes C32-C36 through F0179/F0208 and the
  clean group-timeline and creature-AI regression targets pass (2,609 checks);
  do not duplicate that logic in a second runtime path. Audit C32's lack of
  an individual slot without reproducing an out-of-bounds Aspect[-1] read.
  F0179 now carries its PC3.4 Couatl movement and Animated Armour ongoing-
  attack sound gates through committed reaction/fanout dispatch
  (GROUP.C:242, 270-280). F0183 map-entry initialization still has no
  result-emission handoff, and authentic live captures must validate sound
  priority/arbitration; do not replace these with a generic audio effect.
  Remaining integration requirements:
  - Audit remaining attack-entry transitions beyond C31-C37 with their
    own deletion/delay rules. Obtain turning-pair
    and emulator-trace coverage beyond the four already-facing slots.
  - Obtain original traces for approach entry and zero-tick chained dispatch.
    Preserve off-map handling and the separate priority gap.

- Extend native-save coverage from a fresh F0195/F0180 C37 admission to an
  authenticated original-media save-and-resume route. The G0377 regression now
  verifies the exact source-shaped C37 case, unique C04 ownership through the
  square chain, GLOBAL_DATA/ACTIVE_GROUP round-trip, and rejection of an
  unauthenticated C36 reaction. Other runtime-generated C29-C41 events still
  require an imported C3/C4 receipt; audit their original union ownership
  individually before widening export. The staged French DOS save corpus is
  absent here, so the focused regression does not establish original-media
  save/resume parity.

- Complete F0183/F0180 admission fidelity beyond the implemented F0179 aspect/RNG
  initialization and successful staged RNG publication. Obtain original traces
  beyond the bounded multi-group scan-order and partial-admission rollback tests.
  Check Couatl sound emission and edition-specific metadata beyond I34.
  F0180:333 event priority (255-MovementTicks) still needs integration.
  Timeline scheduling currently sorts only by
  time and preserves insertion order on ties; TIMELINE.C:143-151 requires
  time, descending original event type, descending priority, then source-slot
  order. The 44-byte serialized event has no dedicated priority/slot fields.
  Audit all event mappings and persistence before changing this shared queue;
  adding only a C37 priority would not establish original ordering.
  Do not treat the seeded two-event regression as
  proof of this complete admission sequence.

- Extend the new shared prior-square history to full original-runtime
  capture verification: compare consecutive live M11 wander moves and exact
  RNG with original execution and deferred
  removal/re-admission. Unadmitted legacy worlds still lack source history.
  Bounded C29/C37 two-move, staged map-transition rollback and lifecycle
  evidence is recorded in DONE-dm1.md; full original map-transition traces
  remain unverified.

- Extend F0412 RNG verification to M11's prevalidated XP/gate handoff and
  original execution traces. M10 XP/practice extraction is now corrected to
  BASE.C F0027's shifted sample; the shared multiplier matches BASE.C, not
  the distinct CEDT002 routine. The bounded fixtures and five original DOS
  media integrations do not establish complete cross-platform stream parity.

- Extend verification of newly allocated fixed drops through F0267's off-square source path.
  GROUP.C F0186:643 passes source X=-1; retain this distinction without relaxing
  source membership on `F0267_MOVE_MoveThingOnLoadedChain_Compat`.
  Pair the bounded direct-pit and teleporter/plate regressions with original
  captures; include multi-level falls and sensors at the landing square.
  Successful floor linking or event publication alone is not full move parity.
  Preserve failure ownership: the existing loaded-chain mover restores a
  failed destination link to its source, which is impossible for X=-1.
  A reserved-drop path must return the unlinked allocation to its caller
  without linking it to an invented source square. Audit destination sensor
  side effects on link failure before claiming transaction atomicity.
  Existing test owners to extend are `test_csb_v1_f0267_loaded_chain_pc34_compat`
  and `test_dm1_v1_f0267_local_sensor_rotation_pc34_compat`.
  Keep the complete reserve/RNG/publish operation shared by M10 and M11.
  Do not expose an unchecked
  public "fresh Thing" handle: the current ordinary-record validator accepts
  C05..C10 without checking raw ownership, and Next=END also describes existing
  floor/inventory tails. Keep source membership required on the existing public
  loaded-chain move API.

- Finish F0186 allocation/RNG ordering across runtime drop materializers.
  M10/M11/CSB use the streamed helper; CSB drops now share the F0190 caller RNG.
  Audit upstream RNG owners and nested sensor RNG effects beyond the verified
  fixed/carried-cell sequence and direct rollback observation.
  Extend M10 death-path RNG and reclamation evidence. See
  `docs/parity/DM1_FIXED_DROP_ALLOCATION_ORDER.md` for the confirmed mismatch
  and the required exhausted-pool regression matrix.

- Extend explosion F0191 verification to possession drops and original
  death-smoke captures across supported platforms.
  Nonlethal RNG/raw writeback, survivor compaction, whole-group retirement
  and source C15/C25 smoke publication are verified in bounded fixtures;
  they do not establish full death aftermath or emulator parity.

- Pair source-projectile Fluxcage passage with original emulator captures.
  The raw C14/C15 RAM regression and F0219 source audit do not establish
  full endgame or cross-platform parity. Extend real Lord Chaos/fusion
  coverage beyond F0221 lookup and source-owner metadata checks.

- Complete projectile occupancy parity for destination squares and active
  group overlays. The F0176 selector currently receives decoded group cells
  and uses the normalized representation documented in
  `docs/parity/DM1_ACTIVE_GROUP_CELL_OWNERSHIP.md`. Resolve raw C04 byte-5
  ownership before introducing an active-index dereference. It also receives
  the decoded primary direction; verify F0145/F0147 active-map overrides with original
  captures. Source C14 landing occupancy is now deferred to the next event,
  using its raw source-square lookup rather than active AI rows. Expand
  timing verification to party landings, teleporter exits and original
  emulator traces; audit legacy host-only destination collision separately.

- Extend F0304 reachability analysis beyond ordinary defined PC 3.4 paths.
  The source bounds in `parity-evidence/dm1-pc34-xp-award-reachability.md`
  rule out scaled-word overflow for the examined normal PC 3.4 awards,
  even at format-maximum difficulty. BUG0_81's uninitialized damage path
  needs original binary evidence, not a deterministic invented oracle.
  Extend the verified original-media Mon Light difficulty-6 integration to
  original input/presentation captures and other casts/attacks; primitive
  word-width tests are not naturally occurring original fights.
  Extend source-edition XP arithmetic verification to authentic Amiga 3.6
  startup and original boundary traces. Unknown identities retain the legacy
  signed default; add explicitly evidenced edition mappings rather than
  inferring signedness from graphics format. Audit remaining standalone legacy
  helpers and maintain common policy for all live query/award consumers. See
  `docs/parity/DM1_SKILL_ACCUMULATOR_AUDIT.md` for implementation and evidence.

- Extend rune lifecycle verification to original DOS/FM Towns media and
  rendered symbol rows after fourth-rune wrap/recant. Atari/Amiga input
  and debit tests do not establish visual parity for every platform.

- Extend spell-panel parity beyond the tested DOS/Atari/Amiga static
  panels and six successful DOS light-cast cases. Verify failure frames,
  other spell types, rune wrap/recant transitions and same-tick serial
  replacement; retain distinct edition-specific rendering and evidence.
  Add FM Towns spell pixel tests with authentic C009-size admission and
  edition registry evidence (English EDM.EXP regions already documented).
  Japanese caster names require TEXT2.C F0952, not M653 ASCII glyphs;
  verify JP registry bytes and that text path separately. Do not classify
  little-endian FM Towns as early Amiga merely due to its legacy loader.
  Original emulator/input captures and complete all-platform HUD parity
  remain unproven. V2.2 retains its explicitly permitted alternative art.

- Investigate the intermittent no-output early exit of the French Atari
  original-media test observed on September 6; it did not recur in three
  isolated repetitions or the full rerun. Do not treat retries as a fix.
- Verify all VI wound/power combinations; ten-retry exhaustion, individual
  wound bits on alternate UseItem and combined wounds on mouth are covered.
- Investigate intermittent French Atari SND1 source-index-1 rejection in
  original-audio setup, observed before the restorative tests. Full rerun
  passed; do not conflate this with a fixed failure or the no-output exit.

- Extend original-media combat XP integration coverage before publishing
  the XP batch. Paired fatal/nonfatal RAM tests do not replace an authentic
  fight capture. Extend startup sentinel proof to remaining editions and
  actual launcher UI routes; completed regression evidence is in DONE-dm1.md.
  English and German Atari ST 1.2, French Atari ST 1.3, and English Atari ST
  1.0a, 1.0b and 1.1 reach `dm1-runtime` through M12 with first-runtime
  receipts at `championCount=0`. All six Atari ST editions now follow the
  normal M12 route to authentic C127 ordinal 14, confirm its C040 choice, and
  accept a gameplay turn after the panel closes. Authentic-media checks cover
  English 1.0a, 1.0b, 1.1 and 1.2, German 1.2, and French 1.3; the PC 3.4
  choice and gameplay route is also verified. See DONE-dm1.md. The remaining
  gap below is proving the complete source-owned campaign-start transition,
  not Hall selection or immediate gameplay input.
- Verify the Atari ST source-owned new-game campaign-start transition beyond
  the current Champion Hall selection and first-turn checks on the normal M12
  route. German Atari ST 1.2 presents authentic
  source pixels through M12: ReDMCSB DEFS.H MEDIA020 binds floor/ceiling records
  75/76, and DATA.C's six Atari RGB3 dungeon palettes are installed in M11.
  A captured 320x200 first runtime frame has 17,773 nonblack pixels in six
  colours; this is a visible-frame smoke check, not visual parity. M12's first
  runtime receipt starts with `championCount=0`; its normal menu route then
  reaches the adjacent tile for C127 ordinal 14, opens its source candidate
  panel, confirms C040, and accepts a following gameplay turn on English v1.2,
  German v1.2, French v1.3 and English v1.0a, v1.0b and v1.1. The source-owned
  campaign-start transition remains open; these M12-to-M11 tests do not yet
  prove the complete F0441/F0435/F0462 new-game path or F0267 party-Thing
  placement onto the authenticated start square. World initialization already
  decodes the DUNGEON.DAT initial party location, so add a runtime assertion
  for the live map, coordinates, direction and Thing chain before claiming the
  campaign start complete. Do not invent champions or treat the partial floor
  view as complete visual parity.
  ReDMCSB STARTUP1.C:162-174 runs F0441, retries F0435, then calls F0462 and
  places the party when `G0298_B_NewGame` is set. `DUNGEON.FTL` is only used by
  LOADSAVE.C's optional custom-dungeon path; its absence from standard STX
  disks does not block built-in new-game startup. The authenticated English
  v1.2 STX used by launch tests has only
  BOOTER, SWOOSH.IMG, START.PRG, GRAPHICS.DAT, START.PAK and DUNGEON.DAT in
  its root; the supplied German v1.2 and French v1.3 STX roots likewise
  contain no `DUNGEON.FTL`. Clean v1.0a/v1.0b and v1.1 STX roots were also
  verified without that campaign file. Atari ST 1.0b and 1.1 have authenticated
  `DUNGEON.DAT` admission and now recruit C127 ordinal 14 through normal M12
  input after their zero-champion first-runtime receipt. The matched v1.0a STX
  does the same after its existing M12 profile's
  `ebccb5f99c4437adcb34d9228b57eb6a` hash was added to M11. The Atari v1.0
  software archive also contains an authentic MSA Automation Disk whose root has
  `DMGAME.DAT` and `DMGAME.BAK`
  (47,710 bytes each). Both pass the original-save header checksum and classify
  as FormatID 1 / `ORIGINAL_DM1`. The primary `DMGAME.DAT` also authenticates
  all five F0435 save parts: 128-byte GLOBAL_DATA, 60 x 16-byte active groups,
  a 3,328-byte four-champion PARTY, 463 x 10-byte events and a 926-byte
  timeline. Its source state is four champions on map 2 at (11,14), facing
  direction 3. The Atari tail starts at byte 10,484 and is 37,226 bytes. Its
  big-endian F0434 header reports 14 maps, ornament seed 99, 12,366 raw-map
  bytes, 1,750 text words, 1,968 square-first-thing words, and the original
  thing counts. The 14 Atari map descriptors yield 412 cumulative columns;
  using the original thing-record widths, those sections account for exactly
  37,226 bytes. This structurally authenticates the v1.0 F0434 stream; this
  early edition has no appended F0422 dungeon checksum. The Atari runtime
  runtime adapter is not implemented. The new read-only Atari F0435/F0434
  receipt authenticates all five primary-save parts, DM1 campaign identity,
  the saved party coordinate against its map dimensions, and the exact v1.0
  tail length. Passing the original save to
  admitted v1.2 STX yields the same zero-champion fresh start as no save. Keep
  this save as a verified source-part candidate, not a verified playable
  runtime; F0435/F0434/F0436 Atari save import remains separate from new-game
  campaign startup. Independent F0435 part checksum matches are 128-byte
  GLOBAL_DATA, 960-byte ACTIVE_GROUP, 3,328-byte PARTY, 4,630-byte EVENTS,
  and 926-byte TIMELINE (the Atari v1.0 EVENT record is 10 bytes). The
  authentic automation-disk MSA SHA256 is
  `b11ca8a124b574738a243fbeabc30567071e63e640b6289bc55315cc27c4d859`; its
  matched v1.0 retail STX SHA256 is
  `d9588480091d3aca753d86efedf0f2336871c817138a7eaf900d1afb4ed0f9ed`.
  Save hashes: DAT
  `728682a977fa49a8a3dd9e3afc77afb2a90e02604234edfc19b8495e937e0fa7`, BAK
  `ad009d608ae843ca3e014af2f5bb291ee9e76002824c5c1af6a12c0606aa3353`.
  The backup passes its header, GLOBAL_DATA, ACTIVE_GROUP, and TIMELINE
  checksums, but its PARTY and EVENTS checksums do not match; the Atari receipt
  rejects it at PARTY. Do not offer it as a playable fallback.
  ReDMCSB LOADSAVE.C F0435 reads FormatID 1 without platform or dungeon ID,
  authenticates the five parts, then reads the Atari F0434 tail; the legacy
  DM1 detection uses 14 maps and ornament seed 99. The existing Firestaff Amiga
  format-5 adapter is not interchangeable: Atari v1.0 uses different champion
  records and does not preserve the Amiga header metadata. Until the actual
  Atari tail, four champions and runtime queues all pass an Atari-specific
  transactional adapter, resume must fail closed rather than silently starting
  a fresh party.
- Resolve the C006 generated-group cross-map teleporter path before promoting
  it as full parity. The source fixture contains raw C01
  (`00 0c 22 a0 00 01`) and now reaches the correct target map 1 / 2,1,
  preserving the source chain, target linkage, inactive cross-map state and
  target buzz. The remaining local regression mismatch is narrower: two
  assertions still expect the legacy host `CREATURE_TICK` label for the
  scheduled C37 wander. The native timeline represents C37 as
  `CREATURE_REACTION` with `aux2=C37`, which is also what its dispatch and
  save conversion consume. Reconcile that test expectation only after an
  original trace confirms the event-type mapping; do not relabel the runtime
  merely to make an old test pass. Continue to compare GROUP.C F0185:542-543,
  MOVESENS.C F0267:439-440,524 and F0262:76-83 with original execution.
  DUNGEON.C:1276-1278,1303-1305 selects ACTIVE_GROUP solely by party-map
  equality, so neither a decoded fallback nor reordered insertion is
  justified without further evidence. Obtain a runtime capture for this exact
  generation case.
- When savegame work resumes, inspect the legacy lifecycle probe's A5/A6
  size expectations (208/872) and J4 all-fields-max serialization failure
  against magicMapRefresh and the serialization contract. It runs against
  original French DOS DUNGEON.DAT but is not a passing full probe. Keep
  this separate from runtime XP verification; serialization remains deferred.
- Extend F0412 evidence to final earned XP, low-wisdom failure trajectories,
  and non-potion low-skill effects. Successful restorative casts at skill 1
  now preserve the original practice/potion RNG sequence; the full failure
  and effect matrix is not yet proven.

- Establish edition-appropriate food-command clocks and presentation waits
  for paths not covered by the verified top-level I34E archive binding,
  including loose/nested packages and other platforms. See
  `parity-evidence/dm1-consumption-timing-audit.md`; source-edge and audio
  queue tests do not prove wall-clock or raster-phase parity.

- Verify swallow timing/output against emulator captures and load changes through actual
  pickup/equipment input and presentation feedback before claiming complete
  consumable parity. Extend the unchanged-skill assertion beyond antivenin.

- If early S1.0/S1.1 media are enabled, dispatch their distinct
  F0348 thresholds/caps rather than applying the PC3.4/S1.2+ rule.

- Verify the now-separated body/elemental shield layers through actual
  M10/M11 attacks with original YA and independently derived damage.
  See `parity-evidence/dm1-shield-damage-layer-audit.md`; primitive tests
  and consumable tests do not establish full combat parity.

- Verify original YA potion combat-defense integration;
  recipient shield gain, scheduled ownership and live C72 expiry now have
  real-media tests.

- Extend the inventory-open idle-tick regression to other damage sources,
  torch/status expiry and Modern presentation modes.

- Extend poison-triggered death coverage to cross-map events and death
  with multiple surviving champions; final death through the timeline is covered.

- Extend death spell-input coverage to actual timeline dispatch;
  verify final-death UI and redraw against
  the original. Savegames remain deferred.

- Extend cross-owner consumption to original stat/healing/shield potions.
  Original antivenin cancellation and other-owner preservation are covered.
  Capped/uncapped waterskin depletion, empty rejection, hand retention and
  leader held weight pass on five editions. Check potion recipient stats,
  consumed Thing removal, event ownership and release behavior.

- Complete the broader regression of the now-enabled inventory/leader
  separation: full original-media corpus, application startup, Modern input,
  chest-restoring eye release, top-row highlights, death/revival and any
  remaining direct activeChampionIndex consumers. Verify cross-owner
  scroll/chest close/reopen sequences through normal input, not just explicit
  owner fixtures. Save persistence remains deferred.

- Verify leader/load transitions through actual death and resurrection,
  including a party with no living champions; controlled zero-health
  selection rejection does not cover the lifecycle.

- Extend load verification beyond floor-drop and second-champion isolation:
  Modern composed-HUD leader changes, cross-champion exchanges, full and partially emptied
  open chests, mutation rollback and weight-changing consumption/spells.
  Audit mutation paths that do not publish through m11_refresh_hash and
  compare platform-specific weight values against original media. The
  current load regression shares the F0140 weight decoder with the engine.

- Extend Atari/Amiga original-object coverage to occupied equipment and
  backpack exchanges and complete drag sequences. Independently verify
  source slot geometry and
  archive-to-normalized-dungeon decoding; those remain shared with the
  current empty-slot admission oracle.

- Verify FM Towns scroll baseline/line placement against its own
  F0341/F0644 paths and original layout data. PC3.4 and the tested
  Atari/Amiga editions now have source/data raster evidence; same-state
  emulator captures remain necessary for independent full-render parity.

Reviewed 2026-08-29. Only open work is listed here.

- Deferred savegame gap: native quicksave does not serialize dungeon Thing
  tables; dm1_v1_original_save_pc34_handoff_adopt_runtime_world reuses the
  original dungeon when the loaded blob lacks one. A chest-0 pickup of 28f1
  correctly leaves head 28f0 before serialization, but resume restores head
  28f1 while the hand also retains 28f1. Preserve changed Thing ownership in
  a future save-format repair; current chest interaction tests do not prove
  full save/resume fidelity. This existing limitation is separate from the
  corrected live G0425 slot persistence and same-slot release handling.

- Verify DM1 chest shutdown paths and extend owner-transition evidence
  beyond the original PC3.4 corpus; save/resume remains separately deferred.

- Extend original-media inventory interaction verification to chest contents,
  scroll text/pixel equality and supported editions beyond PC3.4. Original
  PC3.4 scroll-eye panel routing and held-object preservation pass in both modes.
  The 611-record PC3.4 Original/V2.1 press/release corpus covers all 30 empty
  slots, a distinct admissible resident per occupied-slot swap, source-mask
  rejections and identity preservation, not complete
  inventory or pixel parity.

- Complete legacy gameplay sound parity. Atari now selects original SND1
  records through a source-locked event map; verify live event timing,
  arbitration and PSG output against original captures. FM Towns local
  events now read F20 unsigned PCM from retained media; verify distance
  attenuation, channel scheduling and original-driver output. Amiga local
  effects now read original signed PCM with the native period; verify
  PAL/NTSC selection, stereo distance volumes and two-pair channel arbitration.
  Consult SOUND.C MEDIA007/MEDIA413/MEDIA488 and each
  DATA.C sound table before reusing any CSB transport: CSB's later sound
  indices and sample maps are not interchangeable with DM1's. Preserve the
  current no-generated-marker behavior while adding authenticated sample
  and live dispatch tests. CDDA/title success does not prove effect playback.
  Capture original F0060/F0061 runtime RAM boundaries for Atari sound indices
  1, 12 and 16 (records 534, 544 and 546): the bounded SND1 decoder rejects
  these original EN/DE/FR streams as short. Record 534 declares 100 samples
  but its 37-byte nibble stream describes 99. Do not invent a final sample
  or read beyond the allocated record to make playback appear complete.
  Hatari 2.6.1 can mount the original English 1.2 ZIP/STX read-only and
  capture RAM/screens through a VBL breakpoint, but the ReDMCSB bundle's
  `tos.img` is EmuTOS (SHA256
  `5393932066f3199a6a653dfd1f1524bb52375ae0ad0831720743c2e015360a2b`).
  The ST/1 MiB run reaches an EmuTOS bus-error screen at PC `00e14aac`,
  not the game sound consumer. Do not admit that dump as gameplay evidence;
  establish a compatible reference boot before taking the SND1 trace.

- Compare original Atari pixel captures after the corrected F0497/F0496
  decoder. English, German and French original-media name/all-record checks
  and native CLI/menu/input regressions pass, but are not pixel comparisons.

- Complete Japanese FM Towns M564 catalog translations and rendered-glyph
  verification. All 199 names now pass original-JDATA framing/index/UTF-8
  checks, but these checks do not prove translated inventory/hand-label pixels.

- Complete l10n coverage for every player-facing string in Original and
  Modern/Custom: remaining item/action/spell names and dialog,
  remaining ending text, and every retail sensor/timeline/scroll string. Object and
  action names, F0410/F0381 messages, sensor/timeline text, scroll text, and
  wall inscriptions now cross the DM1-domain presentation boundary. The wall
  path is real-PC3.4 gated through F0168/M648 and has a Swedish Unicode raster
  proof; it is no longer open work. PANEL.C F0351 skill-rank, base-class and
  statistic labels now cross the same DM1-domain boundary in both the live eye
  readout and source-font inventory panel, with Swedish catalog coverage. The
  two authentic PC3.4 F0446 victory messages now also translate after their
  non-prose A/B ordering key is removed and before the message-area publish.
  remaining catalogs and call sites need
  equivalent coverage gates. Decode selected retail bytes first and retain
  them as the stable msgid/fallback; key ambiguous dungeon text by media
  identity plus text index. Never modify or replace original data.
- Validate the completed F0128 source-order executor against authenticated
  same-state original captures. HoC sensor/material-plan checks pass with
  French DOS original data (24 directional portraits), but do not render
  pixels. The PC3.4 live pointer sweep separately selects all 24 mirrors
  in Original and V2.1; host window scaling and emulator comparisons remain
  to be verified. Missing-media
  skips must not be counted as completed original-data verification.
  The live
  scheduler validates and dispatches every source step; D3--D1 wall, ornament,
  door, both F0115 partitions and foreground material are callback-owned at
  their square-local boundaries, and the hand-written scheduler-span and broad
  primitive replay paths are gone. No remaining implementation item in this
  section should reintroduce a parallel renderer.
  F0125--F0127 D0 F0104/F0112/F0113 primitives have been migrated into the
  callback and their separate replay deleted, retaining the F0127 split around
  its F0115 Thing consumers. F0127's complete D0C F0115 item, projectile and
  restarted explosion transaction is now callback-owned before F0113; the
  former direct passes, including the source-invalid post-field explosion
  placement, are deleted. F0125/F0126 now consume their D0L/D0R F0115
  creature rows and following F0113 field steps in the same callback stream;
  D0L, D0R and D0C are dispatched as three explicitly targeted callback
  transactions, so D0L's field tail completes before D0R and D0R completes
  before D0C primitives begin instead of using a class-wide D0 replay;
  the unreachable broad D3--D1 stairs fallback has also been deleted, leaving
  each authenticated F0104 stairs step solely owned by its square callback;
  G2028-negative item/projectile routes remain correctly suppressed. Their
  restarted C15 passes now preserve effect-list order, centered/cell identity,
  party-relative cell rotation, and the exact item-696 C3029/C3030 and
  C3061--C3064 anchors. Move each remaining authenticated F0104/F0107/
  F0108/F0111/F0113/F0115 consumer into that callback in source order, then
  delete its replay rather than running both paths. Center-square
  and side-square F0115 routes now consume their authentic one-based
  cell-order word, including the `DOORPASS1 → F0111 → DOORPASS2` partition
  for real floor items, creature groups and projectiles. The final occlusion
  replay now completes D3/D2 outer lanes and D3--D1 normal side lanes one
  source square at a time. D3L2/D3R2 now consume their previously missing
  `DOORPASS1` before F0676/F0677's door occluder; D2L2/D2R2 remain the
  source-authentic no-Thing routes. Normal side and center `DOORPASS1` is
  likewise callback-owned between each route's wall/ornament envelope and
  F0111 door. The callback-owned foreground phase then retains the exact
  `MAIN`/`DOORPASS2`/F0113 order together with F0104/F0108/F0112 material.
  Door-front F0108 is no longer part of that post-door tail: it has its own
  callback phase before `DOORPASS1`, matching the source
  `F0108 → DOORPASS1 → frame → F0110 → F0111 → DOORPASS2` transaction and preventing
  pressure plates from repainting a completed door.
  F0104 wall material for D3--D1 is now callback-owned at each square's first
  scheduler step; the former outer/side/centre wall replay calls are deleted.
  F0107's admitted 13-row wall-ornament family is now callback-owned one
  projection at a time, including D1C's source-owned C346/C026 champion
  mirror consumer, and the old broad and mirror replay calls are deleted. The
  D1C Thieves Eye wall restore is also callback-owned at F0124's square-tail
  boundary after the complete D1C transaction and before F0125; its former
  direct post-scheduler draw is gone.
  D3--D1 normal C15 explosions are now restarted after packed-cell material
  inside every owning F0115 callback, including both door partitions around
  F0111; the once-per-frame global replay is no longer called.
  MEDIA720 D3L2/D3R2 F0107 is now callback-owned from the authentic PC 3.4
  item-696 layout records `C1004 + set*15 + {0,1}`. These are centered/top/
  bottom-aligned anchors resolved with the source bitmap dimensions, not
  inferred G0205 rectangles. Item 558 was ruled out byte-for-byte: its real
  record at `0x25eed` is a 38-byte 16x7 image, not layout data.
  F0111 door frames/panels/ornaments/masks are now callback-owned at the
  plan's explicit door step. F0110 is likewise callback-owned only on the
  four source routes that contain it: exceptional D3R and centre D3C/D2C/D1C.
  The former direct side/centre button replay is deleted. F0104 door frames
  now rasterize through their own preceding callback phase, including open
  doors and D3L2/D3R2, while F0111 owns panel composition only. Replace remaining work only
  with the corresponding
  F0116--F0124 operation, never with a
  host-generated panel or a substitute asset.
  F0111 now resolves a DOOR Thing's Type bit through the current map's real
  `DoorSet0`/`DoorSet1` values, fixing iron/Ra panels on retail maps whose
  sets are not 0/1. F0111 now also consumes the animated Ra door's single
  `M004_RANDOM(4)`, composes the ordinary ornament, D1C Thieves Eye and
  destroyed mask into a complete native temporary door bitmap, applies the
  whole-bitmap horizontal/vertical flip, and only then clips the current
  opening state. The same transaction now owns all eight side-door F0111
  routes, including the exceptional D3L2/D3R2 slices; the old independent
  ornament and destroyed-mask viewport overlays are no longer dispatched.
  The pass1055 closed-D1C comparison remains candidate evidence only. Its
  original pass513 record does not bind F0128 map X/Y/direction or prove an
  F0097-presented frame, so the measured 429/8,448 panel-pixel delta cannot be
  used as a renderer oracle. Obtain an authenticated original same-pose
  debugger capture first, then close any proven differences and add
  route-specific captures for animated Ra, ornament, D1C Thieves Eye,
  destroyed-mask and opening-state variants; do not hide divergence behind a
  tolerance or nearest-neighbour pose match.
- Complete D3L2/D3R2 F0115 material consumers. The live object route now
  consumes the original layout-696 C2500 rows 3/4 through the decoded source
  Thing and GRAPHICS.DAT, including the depth-3 cell gate and C10 blit. The
  scheduler's F0676/F0677 rear/front partitions now own that real object
  pass; they do not borrow ordinary D3L/D3R pane geometry. For creatures,
  MEDIA720 F0115 maps `C14_VIEW_SQUARE_D3L2`/`C15_VIEW_SQUARE_D3R2` through
  `G2033[14]`/`G2033[15]` to raw C3200 rows 3/4 (`DUNVIEW.C:373, 5211,
  5613-5617`). The native raw-C3200 plan now consumes those exact rows and
  rejects blank source coordinates; the same F0676/F0677 consumer draws
  C2900 through its source row and original cell partition. The existing
  helper for ordinary D3/D2/D1 side rows remains separate. What remains is
  capture-backed pixel comparison for those uncommon creature/projectile
  routes, not a host pane substitute.
- Obtain authentic C13-save and original capture corpus for remaining HoC,
  top-row and action routes; bind each to the PC 3.4 runtime before promotion.
  The first required C13 scenario is now identified: the HoC revive alcove
  followed by the immediately lower stair square containing a monster. Capture
  the original route through both poses, preserving the save's dungeon-tail
  owner, active group and event state; do not replace that state with a
  hand-authored Firestaff save.
  The supplied French DOS save pair proves its own backed F0435/F0433 route,
  including direct CLI and start-menu resume, bounded movement, and turns on
  map 5 with its four saved champions, but does not substitute for these
  route-specific captures. A checksum-valid synthetic C13 seed without the
  matching source-owned bones Thing is deliberately rejected at F0435. A
  synthetic fixture may be used only to drive the original runtime to the
  required location and produce a new real-format save; it must never be
  promoted as preservation evidence itself. A usable C13 corpus must
  therefore preserve both the event and its dungeontail owner.
- Extend real-media parity beyond bounded Atari ST and Amiga routes to native
  end-to-end gameplay, input and presentation evidence. FM Towns now has a
  real ZIP CLI/start-menu receipt through TMENU → EDM/JDM, the authenticated
  18-frame English title plan, source-decoded PRESENTS/DUNGEON/MASTER palette
  transactions, CDDA title track, and its input matrix. The stripped JDM path
  now has its own unique disassembly fingerprint, recovered geometry/data
  owners, real-media C12/C13/C14 palette binding, and a native checksumless
  F20J JDATA dungeon handoff that reaches `levelLoaded=1` directly from the
  retail ZIP. Independent seven-command English/Japanese input matrices now
  pass against each edition's graphics and executable fingerprints. An
  authentic gameplay capture is still required before visual
  parity beyond those startup routes can be claimed.
- Bind an authenticated Amiga gameplay RGB4 table to the native renderer.
  The original v2.0 `dm` producer is implemented and source-gated: its
  in-memory 68000 receipt copies the saved table to a working table, adjusts
  each RGB4 component by one or two toward the target, and invokes the Copper
  builder eight times. It deliberately accepts no fabricated palette. What
  remains is a route-specific original gameplay table/capture that identifies
  the active source and target tables; do not substitute the PC VGA palette.
  The supplied ordinary Amiga save disk is now receipted as an authentic
  `DMGAMEG.DAT` session, but it is format-5/Amiga-compatible save material,
  not a Copper-list or framebuffer capture; it cannot alone identify the
  palette table active at a particular gameplay frame.
- Obtain an authentic nonzero-C1 Amiga format-5 `DMGAMEG.DAT` sample to
  exercise the implemented source-owned ACTIVE_GROUP adapter and additional
  C3/C4 event families. The admitted v2.0 save is a zero-active-group/C53
  session; it proves the full in-memory C1 ownership boundary but cannot by
  itself validate a live group or a non-C53 event route.
- The supplied French DOS ZIP → `dungeon_master.exe` SFX package uses a
  non-solid RAR 2.0 (`unp_ver=0x14`) stream. Native RAR2 decoding is
  intentionally out of scope; the launcher reports it as unsupported rather
  than misreporting the supplied original data as missing.
- Bind V2.2 presentation only to reviewed original material/pixels. Existing
  placeholder or procedural art remains fixture-only.

## Deferred original-data corpus

- Capture original PC 3.4 overlays and cadence for the now implemented F0444
  champion-screen input transition into C3 `THE END` and C005 credits. The
  separate non-victory F0435 restart success/failure branches are now wired
  without host relaunch, but still require an original runtime capture using
  an authentic loadable save and a rejected/corrupt save before pixel/cadence
  parity can be promoted. The live victory path correctly disables restart.

The missing C13 save/capture corpus is deferred rather than a release blocker
for the native media paths currently under active work. When an
operator-supplied corpus becomes available, add its hashes and provenance,
then promote only the routes it directly proves. A fixture-derived save may
exercise the original runtime, but cannot by itself close the preservation
evidence gap.
