# Firestaff DONE — DM1

- 2026-09-06: Direct M10 F0412 practice checks stop consuming RNG at the
  first failure and support nine missing levels. Counted receipt APIs retain
  the legacy eight-probe interfaces without widening their buffer reads.
  Potion receipts use the same probes and do not draw potion power after a
  practice failure. The runtime regression pins first- and ninth-probe failure
  RNG states, no projectile/event creation, and the failed-potion ordering.
  Reintroducing eager probe consumption fails the exact-RNG assertion;
  restoring the early stop passes again.
  Nine focused spell/potion and DOS 3.4 media tests pass; these bounded checks
  do not replace original emulator failure captures.

- 2026-09-06: Live M10/M11 skill-level queries now share defined early/late
  accumulator arithmetic. F0849 permanent XP wraps instead of saturating;
  before/after level queries use the world edition policy. Exact known graphics
  hashes bind PC34 and Amiga 3.6 late semantics; direct archive startup hashes
  its actual graphics member when launcher metadata is absent. Boundary and
  live-world tests pass, including UBSan reproduction of the original overflow.
  Five DOS 3.4 media cases and six Atari/Amiga/F20 media cases pass with policy
  assertions (Atari EN/DE/FR, Amiga 2.0, F20 EN/JP). The Amiga HD-named test
  also selects 2.0, so it does not prove 3.6. See
  `docs/parity/DM1_SKILL_ACCUMULATOR_AUDIT.md` for remaining scope.

- 2026-09-06: F0186 fixed-drop sound selection no longer depends on a
  successful allocation. Exhausted dragon junk and Trolin weapon pools retain
  the source wooden/metallic sound respectively (GROUP.C:644), with no pool
  growth. The M11 runtime regression passes both cases.

- 2026-09-06: Fresh F0186 drops now have a three-case pit regression:
  open pits transfer to the lower map, while closed and imaginary pits retain
  the item. Exact cell, existing square ownership and raw/decoded compact-list
  metadata are checked. The five original DOS 3.4 projectile/media cases also
  invoke M11 F0186 on an unchanged original empty corridor and verify one
  correctly typed raw/decoded Trolin club. All six tests pass. This proves
  bounded pit behavior and original-pool integration, not emulator death parity.

- 2026-09-06: M11 fixed-possession materialization now delegates to the same
  allocation-owned F0186/F0267 operation as M10, replacing direct floor linking.
  Compact-list regressions follow ReDMCSB F0163 tail append order, preserving
  existing floor objects before generated and subsequently carried drops.
  Eighteen relevant tests pass, including five original DOS 3.4 archive tests.
  The original-media cases cover projectile runtime regressions, not emulator
  captures of fixed-drop consequences or complete cross-platform parity.

- 2026-09-06: The bounded M10 fixed-drop regression now verifies that a newly
  allocated worm round passes through an object-only teleporter onto a floor
  plate, retaining matching raw/decoded ownership and queuing exactly one
  PIT/SET event for tick 43 at (2,0). Normal timeline dispatch opens the pit
  at tick 43, consumes the event and retains the drop's raw/decoded ownership.
  Missing raw C10 backing is rejected before any sensor effect is published.
  All 13 explosion-group cases and the existing M11 fixed-possession runtime
  gate pass. This is a source-shaped RAM fixture, not an original emulator
  capture.

- 2026-09-06: DM1 M10/M11 fixed drops now reserve before cell RNG and
  publish before the next optional decision (ReDMCSB GROUP.C F0186:610-643).
  The bounded helper matrix covers 256 seed/capacity/cell cases; the behavior
  gate passes 2,489 checks, eight explosion cases pass (including zero/one-slot
  ownership), and the M11 runtime gate
  passes 138 checks including zero/one free-slot exhaustion. Evidence scope:
  `docs/parity/DM1_FIXED_DROP_ALLOCATION_ORDER.md`.

- 2026-09-06: M10 explosion death now has direct fixed-possession coverage,
  separate from the M11 helper gate. A two-worm group with six free C10
  records produces 2-6 worm rounds (DUNGEON.C G0252:543-547), with matching
  raw/decoded links and every allocated item reachable exactly once from
  the floor chain. The source-shaped fixture passes without a runtime fix.
  Exact optional-drop RNG, cell distribution and original-media pickup
  rendering remain open; this test asserts ownership and type/count bounds.

- 2026-09-06: Whole-group explosion death with a carried sharp dagger now
  has an ownership regression: the freed group has no possession head,
  the destination floor chain contains the dagger exactly once and its
  raw/decoded Next values agree. The bounded source-format fixture passes
  without a runtime change. Generated fixed possessions and real-media
  pickup/render pairing remain separate open work.

- 2026-09-06: Exhausted C15 smoke fixtures verify F0213:129-130 admission:
  when the fireball occupies the only source slot, lethal damage creates
  neither a host smoke entry nor a smoke event and preserves that fireball.
  Both one-survivor and all-dead cases pass, alongside the available-pool
  smoke lifecycle and nonlethal RNG tests (four tests total).

- 2026-09-06: C25 continuation skips unrelated co-located C15 records
  instead of rejecting the first different type/attack/fingerprint. The
  death-smoke lifecycle fixture failed before this correction and now
  verifies 190 -> 150 -> 110 -> 70 -> 30 -> freed, including raw attack
  bytes and Next=FFFF at retirement (PROJEXPL.C F0220:853-877).
  Nine tests pass, including five original DOS spell cases; executable built.
  This remains source-shaped lifecycle evidence, not original pixel capture.

- 2026-09-06: Shared F0191 death smoke uses source-first F0213 allocation
  (GROUP.C:916) instead of unconditionally creating a host-only effect.
  A three-slot C15 fixture failed before the fix and now verifies raw smoke
  type 40, half-square strength 190 and one fingerprinted C25 at tick+1.
  Nine tests pass, including five original DOS spells; executable rebuilt.
  This proves publication, not original smoke pixels or full expiration.

- 2026-09-06: A two-creature lethal explosion fixture verifies that F0191
  frees the raw/decoded C04 group, removes its active AI entry and unlinks
  the group from its square. This passes without another runtime change.
  The fixture does not provide free possession pools or source death-smoke
  capacity; drops and C15 smoke remain separate open verification work.

- 2026-09-06: Source explosion group damage now uses the shared world F0191
  transaction: descending slots, individual randomized damage and raw C04
  writeback. Its surviving active cells, directions and aspects are compacted
  with the live group. A two-worm fixture verifies exact HP and final RNG;
  a lethal-first-slot variant verifies the survivor. Raw HP and active cells
  failed before their respective corrections. Fifteen focused/regression
  tests pass, including five original DOS spells; executable rebuilt.
  Complete group death, drops and source death-smoke allocation remain open.

- 2026-09-06: Lethal half-square fixtures now assert exact surviving cell
  bytes in both decoded and raw C04: 08 when removing slot one, 0A when
  shifting slot one over slot zero. Both pass. F0190:892-904 preserves unused
  lower packed fields and does not automatically center a lone survivor;
  equality between two mirrors alone was insufficient to prove this rule.

- 2026-09-06: Projectile F0190 survivor removal shifts active packed
  directions and aspect bytes together with HP/cells (GROUP.C:892-899),
  before scheduling later behavior. The slot-zero lethal fixture now gives
  the survivor a distinct south-facing direction and aspect byte; it failed
  before the correction. Seven impact and seven broader tests pass, including
  five original DOS spell cases. Executable rebuilt. Other damage consumers,
  original ACTIVE_GROUP raw representation and capture pairing remain open.

- 2026-09-06: Projectile survivor compaction now synchronizes the normalized
  active AI cell mirror after F0190 damage. A lethal hit to slot zero in a
  two-worm group reproduced stale ai.groupCells before the fix; it now agrees
  with the surviving decoded group. Killing slot one alone did not expose
  the mismatch. Seven focused impact tests and seven broader tests (five
  original DOS spell cases plus two lifecycle checks) pass; executable built.
  Packed-direction compaction and original raw active-index parity remain open.

- 2026-09-06: Source C14 flight no longer treats C15 Fluxcage as a solid
  blocker. F0219 (PROJEXPL.C:687-764) has no F0221 call; F0221's callers
  at 997-1082 serve Lord Chaos/fusion. A raw C14/C15 Type=50 RAM crossing
  failed before this change and passes afterward. Twenty focused tests
  include five authentic DOS spells; F0221 lookup and source-owner metadata
  regressions also pass. Executable rebuilt. Those latter checks are not
  end-to-end fusion tests; original emulator capture pairing remains open.

- 2026-09-06: Extended the half-square projectile fixture to a lethal hit
  on the second creature. The survivor keeps its 1000 HP, raw and decoded
  group counts agree after removal, and the C14 carrier is freed. This
  bounded F0176/F0190 integration passes without a production change;
  GROUP.C:831-904 owns lethal removal and count reduction. It is not proof
  of original death-smoke pixels, possession drops or all platform deaths.

- 2026-09-06: Added source-shaped F0219 stair-boundary integration checks:
  corridor-to-stairs preserves the flying C14 on the same map, whereas
  stairs-to-stairs retires it without damaging the destination group.
  Both match PROJEXPL.C:723; no production correction was needed. All
  thirteen ownership/occupancy/geometry fixture cases pass. These bounded
  checks do not establish original-media or emulator stair capture parity.

- 2026-09-06: Native projectile destination digests preserve imaginary/open
  fake-wall bits (01/04), following PROJEXPL.C F0219:721-724. The omitted
  flags previously made open fake walls stop projectiles. A source-shaped
  RAM regression failed before the fix; open, imaginary and closed cases
  now pass. Sixteen focused tests include five authentic DOS spell tests;
  five shared movement/teleporter regressions also pass. Executable rebuilt.
  Original emulator comparisons and wider platform parity remain open.

- 2026-09-06: Source-backed C14 projectiles no longer collide with another
  projectile. PROJEXPL.C F0219:687-764 checks party, groups and geometry,
  not peer C14 records. A bounded two-record RAM fixture reproduced the
  erroneous despawn before the fix and now preserves both carriers while
  the first moves. Thirteen focused tests (five original DOS spell cases)
  and five shared movement/teleporter regressions pass; executable rebuilt.
  This is not an original emulator timing capture.

- 2026-09-06: Source projectile champion damage resolves the living roster
  member occupying the impact cell, following PROJEXPL.C F0217:509-514 and
  CHAMPION.C F0285, instead of treating the cell number as roster index.
  The party-landing regression failed before this correction and now proves
  unchanged HP on crossing followed by damage to champion zero in cell 3.
  Twelve focused tests pass, including five original DOS spell tests.

- 2026-09-06: Source C14 landing no longer resolves party/group collisions
  one event early. ReDMCSB PROJEXPL.C F0219:687-697,721-739 checks current
  occupants, moves and reschedules; the next event checks the landing cell.
  The native ownership fixture now asserts undamaged group and live C14
  after crossing, then removes its AI row and verifies the next-event hit
  and retained-weapon cleanup. Eleven focused tests pass, including five
  authentic DOS spell tests. Emulator timing comparison remains open.

- 2026-09-06: Projectile target selection now reuses the F0176 creature
  footprint rule (GROUP.C:88-103): only cells FF means centered, half-square
  creatures occupy facing-dependent pairs, and ordinals search last-first.
  Source-cell and movement preflight use the same selector. Four directions
  by four cells, single quarter-square and duplicate-cell cases pass; a
  bounded RAM integration hits the second worm without an active AI row.
  Fourteen distinct focused tests pass, including five authentic DOS spell
  tests and two shared CSB movement tests. These are source and native-media
  checks, not emulator capture or complete platform parity.

- 2026-09-06: Added native MEDIA009 spell composition for Atari/early
  Amiga, separate from the I34 C009-only route. Original C009 96x33 and
  C011 96x36 are required; source row offsets are 12/24, with Atari's
  interior-row copies preserved. Controls retain the selected caster and
  living tabs; legacy selected text stops at NUL. Closed panels clear.
  Five original-media tests passed (170.74 s, no skips): Atari EN/DE/FR,
  Amiga disk/HD, each with 24 independent whole-panel pixel comparisons
  across Original/V2.0/V2.1 plus closed-panel clearing. Existing source
  name/audio/scroll/object checks also passed. PC34 capture receipts no
  longer mislabel these legacy surfaces as I34 material. This is bounded
  source/data framebuffer evidence, not emulator or complete game parity.

- 2026-09-06: Rebuilt the executable and broader inventory/action/original
  object targets after restoring source HUD ownership in V2.0/V2.1. All
  three regressions pass (101.88 s, no skips), including original DOS
  inventory roundtrips and food-panel/C08 checks. This predates the separate
  in-progress bottom-anchored C009 background correction.

- 2026-09-06: Corrected caster-dependent spell tabs, living-slot iteration,
  selected-caster name drawing and source black-clear/XOR4 highlighting
  (SPELDRAW.C:87-94, VIDEODRV.C:3233-3237). Validated authentic item-696
  records against the retained layout reconstruction; name zones resolve
  to x235+14*caster, baseline48, not horizontal centering. DM1 V2.0/V2.1
  no longer select the generated V2.2 HUD. Independent original M653-bit
  comparisons pass all 24 Original/V2.0/V2.1 caster/sparse-party cases,
  plus the direct plan suite (0.21 s, no skips). Names and party state are
  bounded test fixtures. This does not prove full C009/C011 background,
  other-edition or emulator/display parity; broader regressions remain open.

- 2026-09-06: Rebuilt and reran all six selected original-media startup/
  object regressions after the shared XP and food-state changes: Atari
  English/German/French, Amiga disk/HD, and FM Towns (Japanese/English
  restart within its test). All pass in 165.76 s, with no failures or skips.
  This does not establish full startup animation or gameplay parity.

- 2026-09-06: Rebuilt the main executable and affected food/XP/melee/
  creature-projectile targets together. All eight selected regressions pass
  (8.25 s, zero failures or skips), including original package admission
  and original-dungeon Mon Light XP scaling. This is scoped regression
  evidence, not full game or all-platform parity.

- 2026-09-06: Added `m11_dm1_xp_real`: canonical original archive identity,
  fourteen map difficulties and native Mon Light rune/cast integration on
  difficulty 6. Independent source-derived expectations verify 2,556 XP
  normally and 5,112 after recent combat for Air and Wizard, a light effect
  and no duplicate mana debit (0.17 s, no skip). Party/RNG/timestamps are
  bounded RAM fixtures; no emulator or original-input capture is claimed.

- 2026-09-06: Audited ordinary defined PC 3.4 award reachability against
  ReDMCSB and independently streamed the original EN dungeon's fourteen
  map descriptors (maximum difficulty 6). Conservative melee/spell/throw/
  fear/action/heal/parry award bounds remain below 16-bit overflow even
  at format-maximum difficulty 15 with recent-combat doubling. See
  `parity-evidence/dm1-pc34-xp-award-reachability.md`. This is source and
  original-data evidence, not a fight capture; BUG0_81 and cumulative
  32-bit XP behavior remain outside the proven bounds.

- 2026-09-06: Preserved the active food panel and its pre-consumption
  displayed food during the admitted I34E pending command. Simulation food
  still changes immediately (PANEL.C F0349:1918); panel pixels update only
  after completion (1944-1949). Original-graphics/font regressions cover
  Original and Modern, mouth and UseItem, all four pending frames and
  cross-champion isolation (428 assertions, zero failures; CTest 0.58 s).
  These are source-backed framebuffer checks, not emulator captures.
  The rebuilt original DOS object corpus also passes (104.21 s, no skip):
  an allocated original food record preserves all four pending panel images,
  visibly updates the completed panel and queues exactly the original C08
  sample buffer after the final delay. The main executable was rebuilt.

- 2026-09-06: Blocked direct F5/F9 quicksave/load APIs while a food command
  is pending, preserving PANEL.C F0349 command ownership without changing
  save formats. Both consumption routes verify rejection and preservation
  of the pending command and committed food effect; the mouth regression
  passes (0.34 seconds). Before this guard, the rebuilt executable and six
  focused food/package/XP/melee tests passed (7.79 seconds, no skips).

- 2026-09-06: Verified automatic food-clock binding for the original
  top-level I34E ZIP after requiring matching GRAPHICS.DAT, DM.EXE and VGA
  identities/layout. Original object/C08 corpus passes (99.72 seconds).
  Standard VGA cadence is supported by original-driver disassembly and
  the observed mode13 emulator rate. Four original-byte package cases pass
  (0.91 seconds, no skips), including rejection of missing VGA, original EGA
  substituted for VGA, and sibling-directory executable/driver ownership.
  Complete audible/raster-phase parity remains separate verification work.

- 2026-09-06: Original DOS object corpus passes with deferred food/C08
  transport verification (109.10 seconds, no skip). The test consumes a
  real allocated food record, explicitly acknowledges each presented test
  frame, and checks zero early queueing, exactly one source-buffer enqueue
  after 36 edges and no duplicate enqueue. Startup rebinds SND3 to the same
  archive GRAPHICS.DAT used for graphics. This is bounded command-order and
  source-audio queue proof, not live raster timing or audible waveform proof.

- 2026-09-06: Built the main firestaff executable and rebuilt/reran mouth
  visual and action-runtime tests after adding opt-in food-command ordering:
  both pass (0.35/7.77 seconds). Source-edge fixtures verify pending-command
  sequencing, not live clock authentication or successful original C08
  playback. No normal launch enables the new clock; remaining integration
  is tracked in parity-evidence/dm1-consumption-timing-audit.md.

- 2026-09-06: Rebuilt the DOS PC3.4 original-object corpus test after the
  XP word-width and computed-level changes. It passes without skipping
  (101.01 seconds), reading original media in memory. This is object/runtime
  regression coverage, not a complete dungeon playthrough or timing capture.

- 2026-09-06: Extended direct F0303 threshold round trips through level 24
  (the positive signed-32-bit threshold range), plus M10/M11/F0848 runtime
  checks for levels 17-24. Added threshold-minus-one plus temporary XP=1
  cases, distinguishing ignored temporary XP from live queries. Both rebuilt
  skill suites pass. A bounded read-only caller audit found no newly unsafe
  array indexing or shifts from removing the caps; this is not an exhaustive
  safety claim or evidence for edition-specific overflow behavior.

- 2026-09-06: Removed artificial F0303 computed-level caps (live helper
  capped at 16, lifecycle at 17), following CHAMPION.C:765-770,822.
  Added non-overflow XP vectors 16,384,000 and 32,768,000, verifying levels
  17/18 through F0848, M11 and M10 F0888. The regression failed before
  the fix and passes afterward; the full action runtime test also passes.
  Display-name bounds remain separate. Edition-specific signedness and
  original-media high-XP reachability remain open verification work.

- 2026-09-06: Rebuilt original-media name/runtime tests after the F0304
  word-width correction. All six CTest cases pass without media skips:
  Atari EN/DE/FR, Amiga, Amiga HD and FM Towns (JP plus EN restart).
  Runtime: 179.81 seconds with two test workers. This establishes regression
  coverage for their asserted startup/object paths, not full visual parity
  or natural gameplay overflow reachability.

- 2026-09-06: Corrected F0849 award arithmetic to the unsigned 16-bit
  parameter/assignments in ReDMCSB CHAMPION.C F0304:834,866-889. Six
  explicit boundary cases cover input narrowing, multiplier wrap, recent
  doubling and temporary XP after a nonzero award wraps to zero. The new
  regression failed before the fix and passes afterward. These are source
  contract fixtures; natural gameplay reachability remains unproven.

- 2026-09-06: Rebuilt and reran the M11 skill-query, creature-projectile
  and Giggler-steal runtime suites after removing the kill-XP bridge: all
  three pass. These are scoped runtime regressions, not original-media
  captures or proof of full XP arithmetic parity.

- 2026-09-06: Removed the invented base-health/2 XP request from the DM1
  kill notification plan. PROJEXPL.C F0231:1533-1535 already awards
  damage-derived skill XP, with no second kill reward. Regression checks
  all 27 creature types with a living active champion: defeat logging stays
  enabled while bonus XP stays zero. Removed the obsolete M11 bonus helper
  and its notification call. The paired fatal/nonfatal action regression
  checks equal calculated damage and Fighter XP, zero remaining health and
  an actual EMIT_KILL_NOTIFY on the fatal route. Rebuilt melee-plan and full
  action tests pass. This is a bounded RAM fixture, not an emulator capture.

- 2026-09-06: Extended the C080 leader-hand dagger throw integration case
  to map ordinal zero with difficulty three. XP is independently derived
  as ((8 + 4 + (19 >> 2)) >> 1) * 3 from CHAMPION.C F0328/F0304;
  projectile energy/direction, inventory restoration, consumed tick and
  action-disable assertions remain enabled. The full action suite passes.

- 2026-09-06: FM Towns original-media startup checks now assert the
  PROJEXPL.C:5 attack-time sentinel in both Japanese startup and subsequent
  English restart. The names/font/audio test passes with both assertions.
  DOS PC 3.4 startup also asserts the sentinel, and its original-object
  corpus test passes (94 seconds); missing media was not skipped.

- 2026-09-06: Authentic Atari EN/DE/FR and Amiga/Amiga HD startup tests
  now assert the -200 attack-time sentinel immediately after M11 Start.
  All twenty death/leader/living-caster combinations pass with this check,
  proving propagation beyond default-world initialization for those media
  paths. This does not cover DOS/FM Towns startup or imported saves.

- 2026-09-06: Reproduced the zero initial attack clock and restored
  PROJEXPL.C:5's signed -200 bit pattern in F0859 initialization. Both
  direct no-party initialization and F0881 default-world initialization
  assert the sentinel. No serialized layout or save import was changed.

- 2026-09-06: Reproduced Giggler's live empty-inventory attack leaving the
  XP timestamp unchanged. Its F0207 entry now records the attempt before
  F0193, and cannot fall through to ordinary F0230 damage (GROUP.C:1691,
  1790-1793). Live creature checks pass 69/69 and Giggler checks 18/18.
  This does not establish full Giggler cadence/target-selection parity.

- 2026-09-06: Reproduced ordinary M11 melee leaving the XP attack clock
  at zero on tick 28. The live-creature path now records F0207 entry before
  target selection/damage (GROUP.C:1691), matching ranged attack timing.
  Creature runtime and the 1390-assertion action suite both pass after the
  fix. Initial state and Giggler-specific handling remain separately open.

- 2026-09-06: Corrected the Lord Chaos adjacent-retry test oracle from
  east to south. ReDMCSB BASE.C F0029:1765 advances seed 2 to 1988217957
  and 1387506636, giving two-bit values 0 and 3; TIMELINE.C F0252 maps
  the latter to Y+1. Active-state position and C37 destination now pass.
  No gameplay change was needed. Later cross-map teleporter cases fail,
  so the encompassing C006 suite is not yet verified.

- 2026-09-06: M10 C38 projectile test now supplies the complete bounded
  C04/ACTIVE_GROUP/free-C14 fixture and verifies F0207 attack time at tick
  300 (before the orchestrator's final increment). Its projectile receipt
  and timestamp checks pass. The enclosing C006 suite remains failing on
  two later Lord Chaos random-adjacent movement assertions; not a suite PASS.

- 2026-09-06: Reproduced missing live ranged attack timestamp (0 instead
  of tick 28) and connected the M11 F0207 entry to lifecycle XP time per
  GROUP.C:1691. Tests cover successful launch, failed scheduling retaining
  the timestamp, and rejected C04/position/sight routes leaving it unchanged.
  This is ranged M11 evidence; initial state and melee/M10 paths remain open.

- 2026-09-06: Restored the standalone lifecycle probe's repository-relative
  source/include paths and decompressor/FM Towns/reincarnation link inputs.
  Its executable and reports now stay in the selected output directory
  (default .codex-scratch/champion-lifecycle). Missing dungeon media now
  fails K3 instead of passing, and K3 explicitly distinguishes authentic
  header loading from its RAM party fixture. Verified authentic French DOS
  header acceptance and missing-file rejection; three separate legacy
  size/serialization assertions remain failing and are tracked in TODO.

- 2026-09-06: Reviewed F0304-dependent action XP assertions against ReDMCSB
  CHAMPION.C:866-895 and F0328:2170-2183, plus MENU.C F0407/F0412.
  The local action suite passes all 1390 assertions: hidden magic skills
  receive the recent-attack bonus; startup unsigned stale halving precedes
  recent doubling; separate throw awards round independently; parent skills
  receive no temporary XP. The throw-level fixture now starts at 492 so
  the inner eight-point compass award itself reaches 500 before the common
  action tail. This verifies the reviewed cases, not full game parity or
  live attack-timestamp propagation; remaining XP integration work is open.

- 2026-09-06: Original-media restorative casts now verify the exact F0412
  base-XP formula from the independently advanced first RNG sample, and
  exactly one EMIT_SPELL_EFFECT receipt for the correct caster/potion.
  All twenty high/low-skill cases pass (360 casts). This proves base XP
  handoff, not final F0304 map/freshness scaling; those gaps are tracked.

- 2026-09-06: Removed duplicate practice RNG consumption after the M11
  validated-cast/XP handoff. The original Atari test reproduced potion
  power 44 instead of source-expected 50 before the fix. A bounded RAM seed
  search now exercises successful skill-1 casts independently against
  F0412's wisdom threshold, practice draw count and potion power sample.
  All twenty forced-low-skill original-media cases pass; the default twenty
  cases now mix high/low skill and pass alongside three runtime regressions
  (23 tests). Media and flask contents are authentic; skill/RNG fixtures are
  RAM-only. Exact earned XP and the full failure matrix remain open.

- 2026-09-06: Replaced restorative CastSpell API calls with original C108
  mouse press/release. The independent F0027/F0412 oracle exposed an extra
  M10 XP random draw after M11 had already supplied XP; dispatch now skips
  that duplicate. Exact potion power, paid mana and retained spell controls
  pass in twenty original Atari/Amiga cases (360 casts), repeated with V2.1
  selected (720 total). Open Door and action-runtime regressions also pass.
  This covers high-skill input/state, not low-skill double validation or
  final Modern framebuffer parity; those remain open.

- 2026-09-06: DM1 top-row inventory open/switch/close now preserves the
  spell controls, selected caster and paid rune chain (PANEL.C F0355).
  Twenty original Atari/Amiga cases verify five inventory transitions,
  including both press and release, without mana or symbol mutation.
- 2026-09-06: Broader verification at b0043799a passed all thirteen tests:
  original DOS/Atari/Amiga object coverage, DOS PC3.4 native startup, and
  thirty-six CLI/menu launches across six original-media profiles and
  V1/V2.0/V2.1. Both FM Towns languages pass, with JDATA/JDM.EXP provenance
  checked for Japanese. These probes prove loaded-level/mode selection,
  not emulator-equivalent palette, cadence or final pixels.

- 2026-09-06: Corrected F0370/F0408 cast UI lifecycle. A lone power rune
  now reaches F0410's meaningless-spell feedback through mouse, keyboard
  and API, clearing symbols without a refund or practice RNG draw. Successful
  and source-feedback failed DM1 casts retain the caster's spell controls.
  Twenty original-media cases and four focused runtime tests pass (24 total).
  This is input/state evidence, not a final-frame visual parity claim.

- 2026-09-06: Corrected native DM1 F0399 rune charging and modulo-four
  SymbolStep, including fifth-rune replacement. F0412 now uses a separate
  paid-rune validation entry point, retaining legacy callers and serialized
  request layout. Twenty Atari/Amiga original-media cases enforce exact
  rune costs and selected-caster ownership for 360 restorative casts.
  Focused tests cover insufficient-mana rejection, no refund on recant/
  practice failure, and successful casting at zero remaining mana.
  Five additional runtime/legacy magic regressions pass; no emulator or
  final spell-panel pixel parity is claimed.

- 2026-09-06: Restored DM1 spell mouse routing with inventory open.
  PANEL.C:2445 changes the secondary list, not COMMAND.C's primary C100.
  Twenty original-media Atari/Amiga cases now create MON/EE/VI at all six
  powers through rune mouse press/release (360 casts); symbol ownership and
  unchanged non-caster mana are checked. Cast itself still uses the API.
  The strict opt-in rune mana/step oracle exposed separate failures tracked
  in TODO; this result is not mana timing or rendered spell-panel parity.

- 2026-09-06: Audited the compact spell workbench reachability and removed
  the ineffective DM1 caster change and its release claim. The enclosing
  draw guard excludes both DM1 and CSB. Passing inventory/skill tests did
  not prove a DM1 visual fix; the actual source spell renderer needs its
  own rendered evidence.

- 2026-09-06: Reran all twenty original-media inventory/death/consumption
  cases with presentationMode V2.1 using the explicit
  FIRESTAFF_VERIFY_UPSCALED_INVENTORY=1 test option. All pass. This exercises
  the source-layout input/state paths with that mode selected, not the
  separately composed Modern HUD or final upscaled framebuffer parity.

- 2026-09-06: Verified VI's ten-retry exhaustion using source RNG seed 321
  and wound bit 1 with a runtime-created Lo VI in an original flask. The
  wound remains and the final seed matches ten draws; all twenty cases pass.
  RNG state is controlled in RAM, not substituted potion data.

- 2026-09-06: Added failure-only Atari sound-load diagnostics recording
  errno and an independent read from resident original graphics bytes.
  Forty French Atari test repetitions passed without reproducing the
  intermittent failure. This is diagnostic coverage, not a stability fix.

- 2026-09-06: Extended VI's independent wound/final-seed checks to six
  individual wound bits through alternate UseItem; combined wounds retain
  mouth coverage. MON/EE also exercise both routes. Full twenty-case rerun
  passes; an earlier French original-audio setup failure remains tracked.

- 2026-09-06: Fixed VI's premature generation of unused wound-retry random
  values. Both callers now stop generating masks when F0349 would stop.
  Original-flask VI consumption reproduces the old seed mismatch and now
  matches independently computed wound bits and final RNG seed across six
  powers and twenty Atari/Amiga cases. MON/EE do not advance RNG on drinking.

- 2026-09-06: Added spell-created VI health recovery at all six power runes
  with wounds disabled, covering deficit and maximum clamp. The complete
  twenty-case MON/EE/VI matrix passes (360 cast/consume sequences). One
  earlier French Atari run exited without output; three isolated repetitions
  and the full rerun passed. The unexplained early exit remains open.

- 2026-09-06: All 240 MON/EE cast sequences additionally satisfy the
  independent MENU.C F0412 power bound: ordinal*40 through ordinal*40+15.
  This verifies scaling/range, not exact RNG draw identity or ordering.

- 2026-09-06: Extended original-flask spell/consumption coverage to MON
  and EE at all six power runes: 240 creation/consumption sequences across
  twenty Atari/Amiga cases pass. Checks include raw subtype conversion,
  stamina clamp, EE over-maximum diminishing gains and the 900 mana bound
  before reduction. Formula checks use runtime-generated potion power;
  independent RNG/power provenance remains open.

- 2026-09-06: Created MON through OpenSpellPanel/EnterRune/CastSpell (Lo Ya)
  in an original flask, then consumed it through mouth input. Twenty
  Atari/Amiga cases pass subtype creation, source-derived stamina recovery,
  maximum clamp and empty-flask conversion. Party skill/mana/placement are
  controlled RAM fixtures; potion type/power are produced only by runtime
  spell execution. This is not emulator or complete spell parity.

- 2026-09-06: Extended original antivenin tests to require accepted C08
  swallow transport and no generated-marker fallback for both consumption
  routes. All twenty Atari/Amiga cases pass; waveform/timing remains unproven.

- 2026-09-06: Rebuilt and reran the broader thirteen-test original-media
  suite after the consumable changes: PC3.4 native boot, six edition launch
  profiles through CLI/menu in v1/v20/v21, and six original object corpora
  all pass. No missing-media skips. These checks do not prove emulator
  pixel/timing parity or complete gameplay.

- 2026-09-06: Connected successful mouth and alternative consumption to the
  edition-owned C08 swallow sound transport. Twenty original Atari/Amiga
  cases pass, asserting accepted C08 for food and alternative water use and
  no sound request for an empty waterskin. This is transport acceptance,
  not emulator waveform/timing parity. Rebuilt after the RAM build was lost.

- 2026-09-06: Removed non-source skill XP from DM1 alternative potion use
  per PANEL.C F0349. Original antivenin reproduced the XP mutation before
  the fix; twenty Atari/Amiga cases now preserve all recipient lifecycle
  skill records through both consumption routes. Other games are unchanged.

- 2026-09-06: Verified alternative UseItem with original charged waterskins:
  decremented raw charges, water cap, empty rejection, recipient isolation
  and retained equipment object across five Atari/Amiga editions. Twenty
  original-media cases and the live-transaction unit test pass. Controlled
  equipment placement does not prove pickup/load or swallow-sound parity.

- 2026-09-06: Routed DM1 alternative junk consumption through the original
  inventory transaction instead of legacy placeholder amounts. Original-food
  tests also exposed missing raw-record release; both consumption routes now
  mark consumed food free per F0349. Twenty Atari/Amiga cases pass, including
  alternate food amount, recipient isolation, slot removal and raw release.
  Sound and full input/load coverage remain separate open work.

- 2026-09-06: Fixed alternative UseItem antivenin consumption to cancel
  the recipient's pending poison events and counter via F0323. Original
  Atari media reproduced the retained-counter failure before the fix;
  twenty Atari/Amiga cases now cover both mouth and UseItem routes while
  preserving another champion's poison event.

- 2026-09-06: Fixed ROS/KU/DANE/NETA statistic gains to use F0348's
  PC3.4/S1.2+ diminishing returns above 120 and 150 and cap at 170.
  Twenty-four explicit potion/boundary cases pass alongside existing
  consumable tests and twenty original-media regressions. Original
  ROS/KU/DANE/NETA records from five Atari/Amiga editions also pass mouse
  press/release consumption at statistics 151 and 169: reduced gain,
  cap, recipient isolation, empty-flask conversion and retained hand object.
  These RAM-controlled scenarios do not establish emulator or full-game parity.

- 2026-09-06: Expanded the existing live M11 lightning impact regression
  to both party and individual recipient shield. Both reach projectile
  consumption and the expected 86 HP through AdvanceProjectilesOnce;
  the full action/stamina runtime suite passes. This controlled corridor
  fixture does not replace original-media fire-impact parity evidence.

- 2026-09-06: Added independent F0307/F0313/F0321 damage expectations
  to shield-layer tests: fire 202 with body protection versus 239 without,
  magic 118 (body-scale bypass), normal 128. The RNG fire path consumes
  six slot-defense draws. New and two existing projectile tests pass;
  live original-media builder/impact verification remains open.

- 2026-09-06: M10/M11 combat snapshots now retain separate runtime
  body-shield and elemental-subtraction layers. F0733/F0733b use the body
  layer; legacy 76-byte serialization rejects unrepresentable split
  snapshots without writing. Integrated rebuild and 21 targeted tests
  pass. Actual projectile damage comparison remains open.

- 2026-09-06: Completed a static trace of YA shield into both combat
  snapshot builders and documented their conflated body/spell shield
  representation. The repair and damage proof remain open in TODO-dm1;
  this audit does not claim the combat defect is fixed.

- 2026-09-06: Original YA tests now advance every ordinary idle tick
  through C72 expiry while inventory remains open. All twenty cases
  preserve the shield until the due processing tick, then subtract only
  the potion delta from its recipient. Leader and party shields remain
  unchanged in both normal and above-50 diminishing-return cases.

- 2026-09-06: Fixed YA potion ownership: both M11 consumable bridges
  now read/write the recipient's champion shield, not party shield, and
  C72 records that recipient as Priority. Five original editions reproduced
  the wrong owner. Twenty cases now pass, verifying source power/duration,
  reduced gain above 50 defense, unchanged leader/party shields and C72
  owner. Actual expiry and damage-defense integration remain to be tested.

- 2026-09-06: Fixed original C10 antivenin leaving pending poison and
  its counter active after drinking. PANEL.C F0346 now reaches shared
  F0323 cancellation for the displayed inventory owner. All twenty
  original-media cases pass, preserving the other champion's dose,
  counter and pending event byte-for-byte. Events are controlled RAM
  fixtures; the antivenin Things are allocated original media records.

- 2026-09-06: Original allocated C15 water flasks now pass cross-owner
  mouse consumption on five Atari/Amiga editions. All twenty cases prove
  +1600 water to the displayed champion, unchanged leader water, same
  held Thing transformed to C20 empty flask and leader-owned final weight.
  Weight comparison shares F0140; potion status effects remain separate.

- 2026-09-06: Extended original waterskin cases to uncapped +800 water
  from -1024 and rejection of an empty skin while dehydrated. This removes
  the prior capped-only blind spot, where an accidental drink on empty
  could be hidden by the 2048 cap. All twenty cases pass.

- 2026-09-06: Twenty original-media cases now drain a genuinely allocated,
  charged waterskin through mouse press/release into the non-leader's
  inventory mouth. Verified the 2048 water cap, unchanged leader water,
  one charge per click, empty-skin rejection, retained held Thing and
  leader-owned changing weight. Expected weight still shares F0140;
  the capped fixture does not independently prove the uncapped +800 amount.

- 2026-09-06: Final champion death now runs through an actual due C75
  and AdvanceIdleTick in all twenty original-media regressions, rather
  than setting zero HP and calling the death probe. Verified zero HP,
  both party-dead flags, closed inventory, cleared caster input and removal
  of the old and newly rescheduled poison chains. The controlled event
  and one-HP setup are RAM-only; full emulator/death-screen parity remains
  unproven.

- 2026-09-06: The inventory-open regression now injects a C75 into the
  real-media world's timeline and uses AdvanceIdleTick, not direct damage.
  All twenty cases verify attack 128 deals two HP only to its owner,
  keeps inventory open, and queues attack 127 for the processing tick +36
  with the original C75 tag, champion and current map. The later direct
  death dispatch also removes that actual rescheduled event. This does
  not yet prove lethal poison dispatch or emulator visual parity.

- 2026-09-06: Fixed the DM1 inventory overlay incorrectly stopping idle
  simulation. GAMELOOP.C:81-128 changes rendering while inventory is open
  but still applies damage and advances game time. Five original-media
  cases reproduced the pause; all twenty death-owner regressions now pass
  with an ordinary idle tick advancing while the owner panel stays open.
  Other games' overlay policy is unchanged; full damage/timeline and
  Modern presentation sequences still need broader verification.

- 2026-09-06: Reproduced retained C75 events after death on all five
  Atari/Amiga editions. F0319 now removes the dead champion's poison
  events using stable queue compaction, following F0323. All twenty
  death-owner tests pass, checking unrelated payloads and equal-time
  ordering as well as the cleared poison counter. Pending events are
  controlled RAM-only fixtures on original game media, not emulator traces.

- 2026-09-06: Registered twenty original-media death tests covering
  independent leader/caster ownership on five Atari/Amiga editions. All
  pass, including preserved living-caster input and final-death spell
  clearance. Final death now preserves caster selection as F0319 does.
  The preceding 13-test full object/startup regression also passed before
  this final-death-only adjustment; the rebuilt adjustment passed the
  twenty targeted cases. No emulator pixel-parity claim is implied.

- 2026-09-06: F0319 now clears the dead champion's spell input and
  independently transfers magic-caster selection to a survivor, restoring
  that survivor's pending runes instead of retaining the dead UI buffer.
  Five original Atari/Amiga media probes pass in both leader-death and
  non-leader-death variants. This is a direct death-dispatch regression,
  not an emulator or full-timeline parity claim.

- 2026-09-06: Leader-death probes now retain a third distinct original
  weapon in the mouse hand and verify its identity/weight pass to the
  survivor while the dead champion's load becomes zero. All five editions
  pass both leader and non-leader death variants; weight values share F0140.

- 2026-09-06: Reproduced missing survivor direction alignment in five
  original-media leader-death probes. F0319 now applies F0368's direction
  rule to the replacement leader. Both default non-leader death and
  FIRESTAFF_VERIFY_LEADER_DEATH=1 variants pass for all five editions.
  Held-item transfer and full tick scheduling are not covered by this case.

- 2026-09-06: Reproduced stale load 2 after two-object death drops in all
  five Atari/Amiga editions. F0319 now refreshes carried load/hash after
  drop and leader fallback. All five regressions now reach load 0 and
  retain repeat-call stability. Dying-leader held-item transfer remains
  separate from this non-leader death test.

- 2026-09-06: Five original Atari/Amiga death tests now drop two distinct
  original weapons from source backpack 13 and ready hand 0. Floor-chain
  traversal verifies each exactly once and backpack before hand, matching
  G0057/F0318 and F0163's append semantics. All 30 slots and non-compact
  diagnostic layouts remain outside this ordering proof.

- 2026-09-06: Corrected F0318's source-slot/host-slot mismatch: translate
  G0057 C00..C29 through the inventory mapping before reading host slots.
  Build and five original-media death/ownership tests pass. Those currently
  drop one weapon; multi-item list ordering still needs dynamic proof.

- 2026-09-06: Five original Atari/Amiga death probes traverse the current
  square's Thing chain and find the dropped original weapon exactly once
  with the dead champion's cell bits. Bounded traversal rejects cycles.
  Head/link decoding shares the engine helpers; full inventory drop order
  and independent original-runtime comparison remain unverified.

- 2026-09-06: Five original-media death probes verify that the dead owner's
  ready hand is emptied and a second death-handler call leaves the world
  hash and dropped original weapon's next link unchanged. This checks
  repeat-call stability, not independent full floor-chain reachability.

- 2026-09-06: Five original-media tests now invoke the runtime death
  handler after setting the separate inventory owner to zero health.
  Panel closure, cleared owner, handled-death flag and unchanged living
  leader pass. One idle tick did not dispatch death in this fixture;
  the test uses ProbeCheckPartyDeath and does not prove tick timing,
  dropped-item chain integrity or resurrection behavior.

- 2026-09-06: Corrected F0319 inventory-close selection to use G0423's
  owner rather than the leader. Close the chest cache before dropping the
  dead owner's inventory and clear transient eye panels. Build and five
  existing ownership regressions pass; actual death/drop sequence coverage
  is still open and is not proved by these tests.

- 2026-09-06: Corrected the remaining inventory damage-overlay lookup to
  use the displayed inventory owner. Five original Atari/Amiga viewport
  comparisons pass with different leader/inventory damage timers. This
  does not establish independent C016 visual parity or damage timing.

- 2026-09-06: After enabling inventory/leader separation, all 13 broad
  regressions pass without skips: six original object/scroll corpora,
  PC3.4 application startup and six legacy presentation suites covering
  36 CLI/menu launches. This is regression coverage, not full-game parity.

- 2026-09-06: Promoted inventory/leader isolation into the default five
  original-media ownership tests. Normal mouse input opens, switches,
  closes and reopens both inventory owners; every press/release preserves
  the leader and held Thing. All five pass; no opt-in isolation flag remains.

- 2026-09-06: Enabled separate DM1 inventory selection in normal panel
  input. All five previously failing inventory/leader isolation probes now
  pass, as do the default five ownership regressions. Panel close clears
  selection; owner switching closes the old chest before assigning G0423.
  Broader startup/corpus and multi-owner close/reopen checks are pending.

- 2026-09-06: Migrated DM1 top-row redraw/ownership input records to the
  explicit inventory owner; leader name-color selection remains leader-owned.
  Five original-media regressions pass. Independent top-row highlight
  pixel evidence is still required when activating normal panel input.

- 2026-09-06: Five original Atari/Amiga tests now feed an allocated original
  food item to inventory champion 1 while champion 0 leads. Source G0242
  nutrition reaches only the recipient; leader identity, remaining weapon,
  empty mouse hand and final loads are checked after release. Water and
  potion-specific recipient/effect coverage remain open.

- 2026-09-06: Migrated mouth consumption's champion lookup to the explicit
  inventory owner, preserving leader-hand object selection. Build and five
  existing original-media ownership regressions pass. These tests do not
  exercise cross-owner consumption; that verification remains open.

- 2026-09-06: Migrated inventory main rendering and food/water rendering
  to the explicit owner. Five Atari/Amiga tests compare the 224x136 C017
  viewport for the same inventory owner with different leaders. All pass;
  the comparison shares the renderer and is not an emulator pixel oracle.

- 2026-09-06: Migrated empty-hand eye selection and statistics rendering
  to the explicit inventory owner. Five original Atari/Amiga tests pass
  cross-owner eye press/release and invalid-owner rejection. This verifies
  panel activation, not independent pixel comparison of another owner's
  statistics. Normal inventory opening remains on the migration backlog.

- 2026-09-06: Migrated action-hand scroll text selection to the explicit
  inventory owner while retaining leader-hand eye inspection. Five original
  Atari/Amiga tests decode all original scrolls from champion 1's action
  hand with champion 0 still leading. Expected text shares F0509: this
  verifies owner selection, not an independent text-decoder oracle.

- 2026-09-06: Action-hand chest opening and eye-release action-hand lookup
  now use the explicit inventory owner. Five original Atari/Amiga tests
  open a real chest on champion 1 without changing leader 0. The probe
  checks opening only; cross-owner eye release and close-time chain changes
  still need dedicated coverage before activating normal-input ownership.

- 2026-09-06: Migrated inventory slot transactions to the explicit owner
  accessor. Five Atari/Amiga tests exchange distinct original weapons
  between leader hand and another champion's ready hand in both directions,
  checking identities and separate loads on press/release. The owner is
  selected directly by the test; normal panel opening is not migrated yet.

- 2026-09-06: Added a separately validated DM1 inventory-owner ordinal and
  migrated inventory icon lookup. Five original-media tests verify another
  champion's item without changing the leader, invalid-owner rejection,
  and the explicit legacy fallback. Normal input does not set the ordinal
  yet; this is migration infrastructure, not the inventory-isolation fix.

- 2026-09-06: Added an opt-in original-media diagnostic for inventory/leader
  isolation and reproduced the index conflation on all five Atari/Amiga
  editions. Diagnosis is complete; the runtime correction remains in TODO-dm1.

- 2026-09-06: Rebuilt the application after leader fixes. PC3.4 native
  startup passes, plus 36 CLI/menu launches across Atari EN/DE/FR,
  Amiga EN and FM Towns EN/JP in Original/V2.0/V2.1. These establish
  authenticated dungeon handoff and retained presentation mode, not
  complete visual or emulator parity.

- 2026-09-06: Fixed missing direction alignment when selecting a different
  DM1 leader (CLIKCHAM.C F0368:67). All five Atari/Amiga regressions failed
  with deliberately divergent champion facing before the fix, then passed
  two-way keyboard and original-layout mouse selection afterward.

- 2026-09-06: After the leader-selection fixes, all six complete original
  object/scroll corpus tests pass: PC3.4, Atari EN/DE/FR and Amiga EN/HD.
  No media skips; this supplements the five targeted load regressions,
  not full gameplay or emulator parity.

- 2026-09-06: Fixed DM1 leader selection admitting zero-health champions,
  following CLIKCHAM.C F0368:55. Five original Atari/Amiga tests reproduced
  the keyboard defect before the fix and now verify both keyboard and
  mouse rejection without transferring held weight. Health is controlled
  in RAM; this does not verify the complete death/resurrection lifecycle.

- 2026-09-06: Fixed DM1 source-layout champion name clicks opening inventory
  instead of selecting the leader (CLIKCHAM.C F0367). Five Atari/Amiga
  original-media regressions now pass two-way mouse selection, checking
  immediate load transfer on press and stability on release, without
  opening inventory. Modern composed-HUD geometry remains separate.

- 2026-09-06: Reproduced stale held-object load after keyboard leader
  cycling in all five Atari/Amiga editions. Leader selection now publishes
  a refreshed load/hash immediately, following CLIKCHAM.C F0368. All five
  original-media tests pass a two-way keyboard leader switch before the
  floor drop, preserving the second champion's separate inventory weight.
  Direct mouse selection uses the same refresh but needs input-level coverage.

- 2026-09-06: Five original Atari/Amiga floor-drop load regressions now
  retain a distinct original weapon on a second champion. Normal leader
  pickup/drop/release preserves that champion's ready hand and load.
  This checks isolation, not leader switching or cross-champion exchanges.

- 2026-09-06: All five original Atari/Amiga floor-drop load tests now
  verify the explicit release event as well: load remains zero and the
  dropped Thing does not return to the hand. All five tests pass.

## 2026-09-06 — Carried-load transaction refresh

- Before publishing a DM1 transaction's world hash, recompute carried load
  from inventory and the active leader's held Thing using F0140. Open
  chests use live G0425 slots rather than stale linked contents. CSB's
  separately owned runtime is excluded. Invalid weight data prevents a
  partial load publication.
- The previously failing original Atari drop now reaches load 0. Five
  named `dm1_*_floor_drop_load_real` tests pass for Atari EN/DE/FR and
  Amiga EN2.0/HD, checking conserved slot-to-hand weight and its removal
  after a successful normal-input floor drop.
- Multi-champion, full weight-table and every-mutation coverage remain
  open; this corrects the reproduced defect, not all encumbrance parity.

- 2026-09-06: Added an opt-in original-media load diagnostic and reproduced
  stale load after normal Atari floor-drop input. This is diagnosis only,
  not a completed fix; reproduction and remaining work are in TODO-dm1.

- 2026-09-06: Audited original hand/slot load-update ownership against
  M11's direct hand bridge and recorded the missing dynamic verification
  in TODO-dm1. Existing identity tests do not establish load correctness.

- 2026-09-06: Documented the exact local legacy inventory test selection,
  media/skip requirements, oracle dependencies and public CI exclusion in
  `parity-evidence/dm1-legacy-inventory-verification-scope.md`.

## 2026-09-06 — Legacy occupied action-hand exchanges

- Each of the 606 allocated original objects now exchanges with a distinct
  allocated original weapon in the action hand, then exchanges back.
  Tests check both held and slot Thing identity after press and release.
- All five Atari/Amiga editions pass in Original/V2.1 together with the
  full existing all-slot corpus. Controlled resident placement is in RAM;
  no original archive is modified and no substitute objects are generated.
- Scope is occupied action-hand ownership, not occupied equipment-mask
  rejection, full drag gestures, rendering, load accounting or save parity.

## 2026-09-06 — Independent F0141 equipment-oracle indices

- The Atari/Amiga equipment oracle now decodes each object-info index
  directly from the normalized Thing bytes using DUNGEON.C F0141's
  category offsets, rather than calling the runtime index/subtype helper
  to choose the expected G0237 row. The runtime index is also compared
  explicitly to that independently computed value.
- All five original-media tests pass, including the complete all-slot
  placement/rejection corpus. Archive ingestion, dungeon normalization
  and slot coordinates remain shared dependencies; this does not prove
  complete original-runtime parity.

## 2026-09-06 — Legacy equipment-slot acceptance and rejection

- All 606 allocated original object records now visit all 30 inventory
  slots in Original/V2.1. Acceptance is computed from the selected
  archive's G0237 AllowedSlots word and DATA.C G0038:320–350 slot masks,
  independently of the runtime admission helper.
- Accepted placements are picked back up. Rejected placements must leave
  the original Thing in hand and the slot empty. Identity is checked after
  the press and after explicit release in both cases.
- All five Atari/Amiga editions pass: 61,580 input transactions each,
  307,900 total including rejected placements. Source geometry and
  object-to-info indexing remain shared; occupied swaps, drag sequences
  and complete original-runtime parity remain unproven.

## 2026-09-06 — Original legacy equipment-mask verification

- Read G0237 from each selected Atari/Amiga archive's original graphic
  559 into memory. Require exactly one complete table matching the
  source-defined Scroll/Chest/Mon Potion prefix; missing or ambiguous
  matches fail rather than selecting an arbitrary byte sequence.
- DEFS.H:1683–1688 defines the six-byte OBJECT_INFO layout. Its big-endian
  AllowedSlots word is compared against the runtime mask for each of the
  606 allocated original object records per edition. All five editions pass,
  together with the full hand/backpack transfer corpus.
- Object-to-info indexing remains shared. This verifies original mask
  values, not independent type decoding or body-slot input rejection.

## 2026-09-06 — Real-media read-error reporting

- Atari/Amiga media tests no longer convert every fopen failure into a
  missing-data skip. Only an unset/empty path or ENOENT returns 77;
  other open errors report failure and return 1.
- Rebuilt both binaries and verified missing-file and ENOTDIR paths.
  No game media was created, extracted or modified for these checks.

## 2026-09-06 — Legacy original-object hand/backpack corpus

- Extended the five original-media tests beyond scrolls to every allocated
  weapon, armour, scroll, potion, container and junk record (606 per
  edition). Unused records marked by a 0xffff next pointer are excluded.
- Each record visits the two hands and all 17 backpack slots in Original
  and V2.1 through pointer input. Held and resident Thing identity is
  checked after each placement/pickup press and explicit mouse release.
- Atari EN1.2/DE1.2/FR1.3 and Amiga EN2.0/HD each pass 46,056 transfer
  transactions: 230,280 total. Original archives remain unchanged.
- Scope: empty-slot transfers and release idempotence. This does not
  establish body-slot restrictions, occupied swaps, independent slot
  geometry, item pixel parity, save persistence or full gameplay parity.

## 2026-09-06 — Legacy scroll action-hand ownership

- Extended the five Atari/Amiga original-media tests to place and pick up
  each of the 35 original scrolls through action-hand pointer input before
  eye inspection. Both the held Thing and action-hand slot are checked
  after press and after the explicit left-button release API.
- All editions pass in Original/V2.1. This covers 700 transfer transactions
  and their releases without modifying the original archive. Other item
  classes, occupied exchanges and other slots still need equivalent
  original-media coverage on these platforms.

## 2026-09-06 — Legacy inventory border regression coverage

- The real-media Atari/Amiga checks now compare all 30 inventory slot
  borders against the original C033 32-pixel-stride source, cropped to
  18x18 as specified by CHAMDRAW.C F0291. Nontransparent border pixels
  are checked separately from the 16x16 resident icons.
- All five edition tests pass in Original/V2.1. The coordinate resolver
  remains shared; this checks source pixels/cropping, not independent
  layout geometry, transparent backdrop composition or whole-HUD parity.

## 2026-09-06 — Atari/Amiga inventory and scroll rendering

- Original C033 is stored as 32x18 pixels, whereas its visible box is
  18x18. The PC-only size gate had suppressed the inventory overlay.
  CHAMDRAW.C F0291:558–559,655–658 supplies the crop, source stride and
  C12 transparency. The legacy adapter now binds that original crop;
  the PC material gate remains strict and unchanged.
- Legacy scroll Y is a baseline, not a raster top. TEXT.C F0040:413,714
  subtracts four pixels before drawing, now reflected in the renderer.
- Tests initially failed at the undrawn panel on all five original-media
  editions. After correction, Atari EN1.2/DE1.2/FR1.3 and Amiga EN2.0/HD
  each pass 35 scrolls in Original/V2.1 (350 raster checks in total).
- The oracle uses original panel/font assets and independent baseline
  arithmetic. Line counting/font decoding are shared and transparent
  background pixels are retained from the actual frame. This is not a
  substitute for full same-state emulator comparison or full HUD parity.

## 2026-09-06 — PC3.4 scroll text anchor and baseline

- Read the original archive's C696 layout in memory and verified its CRC
  and GRAPHICS.DAT SHA-256 against the recorded source table.
- C560 resolves to (163,86); F0341/F0644 place the first raster row at
  89-floor(7*n/2). The old renderer was one pixel left and three pixels low.
- The independent coordinate oracle failed on all 35 original scrolls
  before the correction. Afterward the entire real-object corpus passes
  in Original/V2.1, together with the scroll-material gate.
- See `parity-evidence/dm1-pc34-scroll-text-position.md`. This proves the
  source/data calculation, not emulator-capture or other-platform parity.

## 2026-09-06 — Source-owned scroll cell background

- Corrected scroll rendering to copy six columns with white background,
  following PANEL.C F0340 and TEXT2.C F0644:133–143. The previous five-column
  transparent renderer contradicted the original PC3.4 no-transparency blit.
- A source-corrected original-media oracle failed on scrolls 10, 22 and 27
  before the fix. The full PC3.4 Original/V2.1 object corpus and targeted
  scroll-material gate pass afterward. Vertical layout and emulator parity
  remain separate verification requirements.

## 2026-09-06 — Non-vacuous scroll font check

- The real-scroll raster test now requires a loaded original font and a
  nonzero ink-pixel count for every tested scroll. Original/V2.1 and the
  complete original-object corpus pass with these guards.

## 2026-09-06 — Original scroll text raster placement

- The PC3.4 scroll eye corpus reconstructs the C023 panel and scroll-font
  ink with a separate character-remapping and pixel-placement loop. Its
  panel-area comparisons pass in Original and V2.1 alongside the full
  object corpus.
- Text layout and M653 pixel decoding remain shared with production. The
  comparison preserves actual pixels behind transparency, so it does not
  independently prove those pixels, font decoding or full emulator parity.

## 2026-09-06 — Original scroll panel border

- Every PC3.4 original scroll eye transaction now compares the visible
  two-pixel C023 outer border with the loaded original panel at its native
  destination. Original and V2.1 pass with the full object corpus.
- This is border/material evidence only: transparent pixels and the inner
  text area are excluded, and glyph parity remains open.

## 2026-09-06 — Original scroll text ownership

- Every original PC3.4 scroll tested through the eye now requires M11's
  decoded source text to match that scroll's own C02 reference with F0341's
  SCROLL/DECODE_EVEN_IF_INVISIBLE flags. Original and V2.1 pass with the
  full original-object corpus.
- This verifies owner/reference selection using the shared text decoder;
  it does not independently validate decompression, localization or glyph
  pixel parity.

## 2026-09-05 — Three-mode legacy launch matrix

- Added V2.0 to the existing Original/V2.1 original-media CLI/menu matrix.
  All 36 launches pass across Atari EN/DE/FR, Amiga EN and FM Towns EN/JP,
  retaining the requested presentation and reaching a loaded dungeon.
- Japanese FM Towns also asserts its own program/graphics fingerprints.
  These checks do not establish full gameplay, animation timing or pixel
  parity, and do not cover every historical edition of these platforms.

## 2026-09-05 — Japanese title media failure classification

- Missing archives skip explicitly, but unreadable existing archives or
  archives without a readable BIN now fail the JDM title test. Verified
  original media passes, an existing non-archive returns 1, and a missing
  path returns 77. No new game data or extracted files were created.

## 2026-09-05 — FM Towns launch and Japanese title evidence

- Rebuilt Firestaff passes the original ZIP CLI/menu English and Japanese
  launch/input matrix, including selected program/graphics hashes and CDDA
  track status. This is not an emulator timing or full-frame comparison.
- The original JDM title-receipt test passes. Its assertions now stay active
  under NDEBUG, and unavailable media returns CTest's skip code 77 instead
  of a false pass. CMake recognizes that skip explicitly.

## 2026-09-05 — Original chest owner transitions

- The original PC3.4 corpus now refreshes the same open chest after each
  pickup, requiring holes to survive, and switches to another chest while
  holding the first resident. It verifies the held Thing, old owner's full
  remaining chain and unchanged next owner's head after close.
- Original and V2.1 pass together with the complete original-object corpus.
  Test placements are restored in memory; save/resume remains out of scope.

- Bound the stripped Japanese FM Towns `JDM.EXP` title path to its unique
  disassembly fingerprints at `0xc428`, recovered rectangle/data owners, and
  exact C12/C13/C14 RGB6 records. The native title presenter now accepts the
  Japanese title only through the real in-memory ZIP/BIN/ISO receipt; it has
  no default/VGA palette fallback. See
  `parity-evidence/dm1_fmtowns_jdm_title_palette_binding.md`.

- Fixed the native English FM Towns title palette handoff. The authenticated
  EDM.EXP P3 load image now supplies the exact C12 PRESENTS and combined C13
  DUNGEON/C14 MASTER RGB6 transactions used at their original title-frame
  boundaries; stale host/PC palettes are no longer allowed. See
  `parity-evidence/dm1_v1_fmtowns_title_palette_transactions.md`.

## 2026-09-04 — F0125–F0127 targeted D0 transactions

- Replaced the class-wide terminal D0 callback pass with three explicitly
  targeted scheduler transactions in ReDMCSB order: D0L, D0R, then D0C.
  Each side square now completes its F0104/F0115/F0113 tail before the next
  square begins; D0C retains its split around F0115 and the final F0113 field.
- The implementation continues to use the mounted PC 3.4 GRAPHICS.DAT
  consumers. No generated bitmap, extracted runtime asset, or second replay
  was introduced.

## 2026-09-04 — Removed broad post-scheduler stairs ownership

- Deleted the remaining broad D3--D1 stairs fallback after square replay.
  The authenticated-plan failure path already returns before viewport
  background composition, so that branch was unreachable; retaining it gave
  F0104 stairs a second, source-invalid ownership path.
- D3--D1 stairs are now exclusively consumed from each owning square's
  authenticated foreground callback using mounted retail GRAPHICS.DAT data.

## 2026-09-04 — F0128-owned F0104 door frames

- Split native centre and side door-frame material out of F0111 into the
  scheduler's explicit F0104 door-frame callback phase. The live order is now
  F0108, DOORPASS1, frame, optional F0110, F0111 panel, DOORPASS2.
- F0111 no longer redraws frame pixels. Open doors retain their frames, and
  the same source plan covers normal side, centre, and D3L2/D3R2 routes. The
  real PC 3.4 HoC probe requires separate frame and panel receipts.

- Modern temporal post-processing now applies movement blur, phosphor
  persistence, and the next-frame history snapshot in one full-resolution
  traversal. The operation order and idle-frame history semantics are
  unchanged, while the all-effects path no longer performs three separate
  high-resolution memory passes per presented frame. Original mode never
  enters this optional filter path.

## 2026-09-04 — F0128-owned F0110 door-button routes

- Added an explicit F0110 scheduler step only to the four ReDMCSB call sites:
  F0117 D3R and the centre F0118/F0121/F0124 routes. The callback consumes it
  between the plan's frame and F0111 steps; D3L, D2L/D2R and D1L/D1R remain
  source-authentic no-button-call routes.
- Deleted the direct D3R and centre button replay. The real PC 3.4 HoC probe
  finds retail map 0 door `(1,2)` from party `(0,2)` facing east and proves
  one callback-owned F0108, DOORPASS1, F0110 and F0111 step in that order.

## 2026-09-04 — Door-front F0108 source boundary

- Split door-front floor ornaments out of the generic post-door foreground
  phase. Their authenticated scheduler step now rasterizes before
  `DOORPASS1`, followed by the source frame/F0111/DOORPASS2 transaction.
  Corridor, pit and stair F0108 steps remain in their ordinary plan-ordered
  foreground path; no primitive-class batch or second ornament draw exists.
- The real PC 3.4 HoC test finds an unmodified retail door and requires one
  pre-door F0108 callback receipt before accepting its DOORPASS1 and F0111
  receipts. F0113 remains owned by the same square after its final F0115
  consumer.

## 2026-09-04 — F0128-owned F0107 ornament projections

- Moved the authenticated 13-row D3--D1 wall-ornament family into the
  per-square callback. D3/D2 side routes consume their side-facing F0107
  projection followed by their front-facing projection; centre and D1 routes
  consume their sole projection. The three broad hand-written ornament
  replay calls are gone.
- Corrected the scheduler's F0107 cardinality from an unconditional pair to
  the exact ReDMCSB function bodies: two calls only for D3/D2 normal side
  squares, one for outer, centre, and D1 walls. The real PC 3.4 HoC gate now
  requires callback-owned F0107 material alongside wall, D0, and foreground
  work. F0676/F0677 outer walls and F0122/F0123 D1 side walls now also ignore
  the F0107 return exactly as source does, preventing a false alcove Thing
  pass on routes that immediately return. MEDIA720 D3L2/D3R2 now use the
  separately authenticated item-696 C1004 layout anchors; they do not index
  or infer nonexistent rows from the legacy 13-row G0205 source table.

## 2026-09-04 — F0128-owned F0111 door transactions

- Moved every D3--D1 F0111 door transaction from the hand-written viewport
  replay into the authenticated per-square scheduler callback. Center, side,
  and exceptional D3L2/D3R2 routes now consume their native frame, panel,
  ornament, mask, flip and opening-state material only when the owning plan
  span emits `F0111_DOOR`; no duplicate F0111 raster path remains.
- Preserved F0110 exactly where ReDMCSB places it before F0111, including the
  exceptional D3R button; it is now owned by its explicit scheduler step.
  The real PC 3.4 HoC runtime test searches the
  unmodified mounted dungeon for a door and now requires both `DOORPASS1` and
  F0111 callback receipts from that original square.

## 2026-09-04 — PC 3.4 SND3 event-table source closure

- Replaced the guessed priority, distance and period fields in the 35-entry
  DM1 sound table with the exact ReDMCSB I34E `DATA.C` MEDIA719/MEDIA712
  rows. This includes the distinct period for entrance-door event 3 and the
  source periods for party damage, War Cry, Blow Horn and movement sounds.
- The real-media gate reads `DATA/GRAPHICS.DAT` directly inside the supplied
  PC 3.4 ZIP, proves every event maps to its source `SND3` item, and decodes
  all 35 event aliases from the authentic 33-sample bank at 6000 Hz. Invalid
  host emission indices now fail closed instead of entering the audio route.
  No game-data member is extracted and no procedural sound is accepted as
  proof.

## 2026-09-03 — Real-media startup regression audit

- Re-ran the native direct-CLI and start-menu matrix against the staged
  original media. PC DOS 3.4; DOS English and French; Atari ST English,
  German, French and nested archive routes; Amiga HD and v2.0; and FM Towns
  all reached their intended native handoff in memory. The French RAR 2.0
  package retained its explicit unsupported-format diagnostic rather than a
  false missing-media result.

## 2026-08-30 — Amiga v2.0 original save-disk provenance

- The startup menu's Continue entry and direct CLI `--save` now accept the
  authenticated `ZIP → ZIP → ADF → DMGAMEG.DAT` path. M12 validates the
  actual Amiga F0435 envelope in RAM, and M11 routes the virtual path straight
  to the format-specific loader instead of attempting `fopen`. The direct
  Amiga boot probe resumes the real save at tick 292 with no media extraction.
- Added an in-memory receipt for the ordinary save disk retained inside the
  supplied Amiga v2.0 preservation ZIP. The selected ZIP → ZIP → ADF is
  SHA-256 `5679f789655ba3f53f6275137fc80f59eb798b03b88f801e260aed352b6709c9`
  and contains the original `DMGAMEG.DAT` and `DMGAMEG.BAK`, each 49,002
  bytes; no member is written to disk.
- Its primary header verifies as a ReDMCSB-compatible original save family
  (format 5, platform 3, dungeon 10) with a valid header checksum. This
  proves real saved-session material is present, while correctly keeping
  framebuffer/Copper-palette evidence capture-gated.

## 2026-08-30 — Native Amiga RGB4 palette producer

- Recovered the exact dynamic producer from the supplied English Amiga v2.0
  `dm` program without extracting the ADF: the real executable has one
  producer at `0x14306`, copies the caller source table to its work table,
  changes each RGB4 component by one or two, and invokes the Copper-list
  builder through the original `0x14434 → 0x14140` call.
- Added a native, source-gated eight-frame implementation. It accepts only
  verified caller-owned 16-word Amiga RGB4 tables, never invents a palette,
  and is regression checked against the original executable's in-memory
  control flow.

Reviewed 2026-08-29. Completed work only.

- The supplied Amiga 2.0 ZIP→ADF `graphics.dat` receipt now decodes an
  authenticated big-endian IMG1 record into original 4-bit palette indices
  in memory. The Amiga wrapper selects the legacy decoder's big-endian path
  explicitly, preventing an FM Towns/PC byte-order fallback.
- With that original ZIP→ZIP→ADF source present, the Amiga graphics test no
  longer builds a structurally valid replacement `GRAPHICS.DAT`. Positive
  decode and format coverage comes only from the authenticated ADF member;
  compact malformed-header checks remain solely as rejection boundaries.
- The supplied FM Towns ZIP now has a RAM-only CDDA payload receipt. It
  follows the source CUE's first audio index from the MODE1/2048 data region
  into the shared raw-audio BIN, validates every track interval, observes
  PCM in music tracks, and proves documented track 20 silence.
- The supplied FM Towns ZIP also supplies the TMenu input receipt directly:
  Firestaff follows ZIP→CUE→BIN→ISO9660 to `TMENU.EXP` in RAM and validates
  the Phar Lap header plus original poll, initialization, and TBIOS entry
  bytes used by the native input schema.
- The supplied DM1 Amiga 2.0 preservation chain now has a direct real-media
  graphics-format receipt. `test_dm1_v1_amiga_graphics_dat` reads the
  selected ZIP → ZIP → ADF `graphics.dat` member in RAM through the native
  AmigaDOS OFS reader, validates the actual 575-entry Amiga layout, and
  identifies it as the known English 2.0 format. It never copies game data
  to disk. Rendering/pixel comparison remains separate active work.
- The real Amiga ZIP → ZIP → ADF and Atari ZIP → ZIP → STX start-menu paths
  now publish their admitted source decoder (`IMG2` and `DMCSB1` respectively)
  only after the original graphics and dungeon pair have both bound.
- The authentic PC DOS 3.4 ZIP start-menu path likewise publishes its admitted
  `IMG3` source decoder only after its original graphics and dungeon pair bind.
- The supplied DOS-EN archive's nested lowercase `dungeon-master/dmaster/DATA`
  layout is now covered independently.  It binds the same authenticated PC
  3.4 graphics/dungeon pair and reaches native CLI, menu, and movement without
  unpacking the original archive.
- The manually unpacked authentic French PC DOS `EUDATA` route now receives the
  same direct start-menu `IMG3` handoff check; its unsupported RAR 2.0 wrapper
  remains a separate diagnostic boundary.
- The authentic German Atari ST 1.2 and French Atari ST 1.3 packages each now
  require their direct start-menu `DMCSB1` handoff before their existing native
  movement assertions run.
- The Amiga 2.0 preservation package now has its own real-media CTest. It
  verifies the exact ZIP → original ZIP → ADF selection, the `IMG2` handoff,
  and a post-menu native movement result rather than relying on the separate
  HD package's coverage.
- The supplied Atari preservation collection now selects only its `[!]`
  original member (`ZIP → ZIP → STX`) and reads its `GRAPHICS.DAT` and the
  release-specific `DUNGEON.DAT` identity entirely in RAM. Direct CLI, the
  startup menu and a native movement probe are covered by real-media CTests;
  cracked sibling images are never admitted.

- PC DOS 3.4 authentic archive startup reaches native DM1 runtime with a
  hash-verified real-media regression.
- M12 resolves the authenticated PC 3.4 data owner and production retains
  fail-closed behavior when required source data is absent.
- DM1 Amiga 2.0 English supplied as ZIP → ZIP → ADF is hash-verified and
  read entirely in RAM. Both direct CLI launch and the startup-menu route
  reach the bounded native runtime without extracting game data to disk.
- The supplied Amiga HD package (preservation ZIP → original HD ZIP → ADF)
  is admitted using the same authenticated Amiga 2.0 graphics/dungeon pair
  and reaches native runtime through direct CLI and the startup menu entirely
  in memory.
- Atari ST original media is source-locked and read in RAM: the supplied
  English 1.0a/1.2, German 1.2 and French 1.3 variants reach native runtime
  through direct CLI and start menu. The German 1.2 protected STX uses the
  verified image identity `0eff1c902ea155f19e4a177bb2ccac7d`, graphics hash
  `2bdc5f431f84c0ece738f54dbd787c3b` and dungeon hash
  `cea11d6e9f7e1698fc95329fe3fb0899`.
- The supplied FM Towns JA/EN archive is verified through both direct CLI
  and start menu, reaching `dm1-runtime` without media extraction.  The
  boot receipt requires the source-bound `TMENU.INF` selection and selected
  `EDM.EXP`/`JDM.EXP` MD5, rather than promoting generic DM1 movement as a
  native FM Towns handoff.
- The same original FM Towns ZIP now verifies the complete public input
  matrix from independent native sessions: forward, backward, turn left,
  turn right, both strafes, and action.  Every check follows
  ZIP → CUE → BIN → ISO9660 → TMENU → `EDM.EXP` in memory and asserts the
  observed initial runtime position, level ownership, and CDDA title track.
- The DM1 V2 movement/viewport regression now reads the canonical PC 3.4
  `DATA/DUNGEON.DAT` directly from its original ZIP in RAM. It no longer
  depends on an extracted corpus for its positive dungeon decode path, and
  the verified raw-map composition is used as the real-data evidence.
- The supplied Amiga HD preservation ZIP now verifies the complete first
  input matrix from independent original ZIP → ZIP → ADF sessions. Forward,
  backward, both turns, both strafes and action all retain the native runtime
  and observed source party state without a generated save or map.
- The English Atari ST v1.2 preservation chain now verifies the same complete
  first input matrix through fresh native ZIP → ZIP → STX sessions. The
  expected party states are observed from original media; no disk image or
  dungeon replacement is synthesized.
- The canonical PC DOS 3.4 ZIP now also verifies the complete initial input
  matrix through fresh native archive sessions. Its authenticated IMG3 title,
  dungeon and party state are retained in memory for every direction, strafe
  and action check; no extracted fixture or generated save is used.
- The complete PC34 `F0381_MENUS_PrintMessageAfterReplacements` producer set is
  now closed. ReDMCSB `MENU.C` has exactly two calls for `C005_ACTION_FLIP`;
  the bounded F0407 plan owns their exact HEADS/TAILS source strings, rejects
  values outside `M005_RANDOM(2)`, and passes the selected msgid through the
  DM1 translation domain into the cyan C015 message area with the original
  70-tick lifetime. Both outcomes, Swedish translations, the live M11 action
  tail, and the original PC 3.4 in-memory media identity are regression-gated.
- Original-mode C015 presentation is Unicode-capable without replacing its
  preservation font. ASCII cells still come from hash-identified PC34 M653;
  translated Latin glyphs are decoded once per UTF-8 codepoint into the same
  six-pixel TEXT.C advance. A real in-memory PC 3.4 ZIP regression combines
  the retail GRAPHICS.DAT/M653 owner with Swedish U+00C5 and proves its exact
  raster and single-cell placement on the bottom C015 row.
- Original-mode wall inscriptions now have the same bounded final-presentation
  boundary. F0168's packed TextString span, decoded glyph bytes, hashes and
  M648 raster receipt remain untouched and are always the fallback. A
  media-and-index-qualified gettext key may replace only the final readable
  text; supported UTF-8 Latin glyphs occupy centered 8x8 cells while ASCII
  continues to come from retail M648, using M648's own opaque palette index.
  The canonical PC 3.4 ZIP test visits all 56 visible inscriptions, then proves
  TextString 4 `HALL OF / CHAMPIONS` as Swedish `HJÄLTARNAS / SAL`, including
  the exact U+00C4 raster and retained original material receipt.
- The DM1 V2 side-by-side viewport comparison now accepts the decoded
  original PC 3.4 dungeon state and renders its actual initial composition
  into both lanes. The real-data viewport regression no longer promotes the
  old hard-coded entry fixture as positive composition evidence.
- `dm1_v2_side_by_side_seed_pc34` now uses that same native in-memory ZIP
  path for every positive lane, layout, RGBA export, and region comparison.
  It skips when the optional local PC 3.4 archive is absent instead of
  silently falling back to a hard-coded dungeon fixture.
- `dm1_v2_v1_v2_side_by_side_seed_pc34` now independently reads the original
  PC 3.4 ZIP member in RAM and verifies V1/V2 disabled-presentation framebuffer
  parity at the authentic entry coordinates in all four directions. It no
  longer uses the legacy entry-state fixture for positive pixel evidence. The
  fixture implementation and its data-free V2 screenshot probes were removed,
  so this family has no synthetic-dungeon fallback.
- The supplied authentic French DOS `DMSAVE.DAT` and `DMSAVE.BAK` (48,561
  bytes each; SHA-256 `494d081ee5175b2dccc900d5ea3f25230c8bb3b0f20828d311b8fc5bdfb82d21`
  and `a760234408bf27946b1586ecf396be72e648bd8f3d18abee90a18c2c7e94421f`)
  now pass a native backed-save roundtrip. Each source save is loaded through
  F0435 against its supplied original French `EUDATA`, staged through F0433,
  exported, reloaded through F0435, and checked for party, C03/C04 timeline,
  active-group and dungeon ownership preservation. No save fixture is used.
- DM1 PC 3.4 F0446 endgame cadence now follows `ENDGAME.C`: every ordered
  victory-message F0445 redraw owns its immediately following 780-tick wait,
  and the final 600-tick wait runs only after the last message. These waits
  freeze gameplay time. Victory music fails closed unless the selected
  installation's authentic `SONG.DAT` is admitted, and the victory path keeps
  restart disabled exactly as F0446 requires. A focused runtime test locks the
  event/delay order and the 780 + 780 + 600 total.
- The authenticated PC 3.4 F0444 victory presentation now has distinct
  source-ordered phases: champion mirrors/portraits wait for input, accepted
  input starts authentic SONG.DAT C3 and C006 `THE END`, the 300-tick hold
  freezes gameplay time, and the victory route then presents authentic C005
  credits. All phases own the full logical screen, and missing F0444 material
  or SONG.DAT fails closed instead of selecting synthetic presentation.
- The separate PC 3.4 F0444 restart request no longer escapes to a generic
  host relaunch. It invokes the native F0435-compatible quick-load consumer
  in place: a valid admitted save clears the terminal/death state and resumes
  the restored world, while a failed load clears the request and returns to
  C3/C006 `THE END` for the source 300-tick hold using retained authenticated
  endgame material. If that material is unavailable the failure path stops
  closed. Focused decisions cover load success, load failure/re-entry, and
  missing-material rejection.
- M11 DM1 status/readout localization now has an explicit extraction
  boundary. Six live pickup/drop/spell literals are marked as DM1-owned,
  included in the 578-entry template, and translated into Swedish. The CI
  gate checks catalog presence, nonblank Swedish coverage, and active-game
  domain dispatch so shared CSB/DM2/Nexus/Theron literals cannot inflate the
  DM1 completion number.
- The next explicit M11 localization group covers the source-locked DM1
  CLIKVIEW.C fountain path: `FOUNTAIN`, `DRANK WATER`, `CONTAINER FILLED`,
  and `FLASK FILLED WITH WATER`. All four runtime literals are marker-gated,
  Swedish-translated, and included in the now 581-entry DM1 template without
  importing shared-file strings owned only by another game.
- The shared required-file catalog retains the verified Amiga 2.0
  `DUNGEON.DAT` identity alongside the Atari additions.  The post-scan
  recovery now republishes a selected ZIP → ZIP → ADF source owner before
  calculating availability, so an Atari preservation scan cannot make a
  valid Amiga launch unavailable.
## DM1 PC 3.4 MEDIA720 outer-D3 wall ornaments

- Recovered all sixteen authentic D3L2/D3R2 layout records directly from
  real `GRAPHICS.DAT` item 696 and wired them into F0128's per-square F0107
  callbacks. The native route preserves record alignment types, C30/C14
  scaling, D3 palette mapping, C10 transparency, and D3R2 horizontal flip.
- Ruled out item 558 rather than guessing from D3L/D3R: the admitted PC 3.4
  file has SHA256 `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`;
  entry 558 starts at `0x25eed`, is 38 bytes compressed/decompressed, has a
  16x7 header, and hashes to
  `df28c5d26e9a7b87903ac817a21675c65ad52216e5deede4148f5286f4223e23`.
- Corrected Hall of Champions mirror admission for the source-owned outer
  D3 records. Internal view indices 13/14 now admit authentic C127 mirrors
  and consume base ornament C345 at the item-696 anchors; they are no longer
  suppressed as invalid aliases of G0205 rows 0/1. The real PC 3.4 archive
  regression covers all 15 matching mirror placements without fixtures.

## DM1 FM Towns Japanese native ZIP start

- Added an explicit `--dm1-fmtowns-ja` selection backed by the canonical
  FM Towns retail disc identities. The native in-memory route now selects
  TMENU -> `JDM.EXP`, reports its exact executable and Japanese GRAPHICS MD5,
  applies the recovered JDM title/palette owners, and reaches the runtime.
- Added the source-specific F20J `JDATA/DUNGEON.DAT` reader. The authentic
  33,931-byte Japanese file ends exactly after raw map data and has no F0434
  checksum trailer; all ordinary dungeon and save readers remain strictly
  checksum-validating. Real-media tests prove both the rejection boundary
  and successful 14-map Japanese materialization without extracting to disk.
# Local PC34 spell-panel background evidence (2026-09-06)

- Original Atari English, German and French archive tests passed with new
  source geometry assertions: C009 96x33 and C011 96x36. These establish
  required legacy bitmap dimensions, not the pending legacy compositor's
  pixel parity. Existing names, source sounds, scroll and object interaction
  checks in those three tests also passed without skips (111.14 seconds).
- Expanded successful-cast pixel/XP checks to six cases across Original,
  V2.0 and V2.1. The 38-test action/spell regression selection passed,
  including action stamina and source-route checks; two obsolete textual
  assertions were corrected to match I34 C009-only painting and VGA XOR4.
- The F0344/F0658 aggregate describes source-font rune zones rather than
  C011 bitmap copies. Its direct positive/negative test and original I34E
  archive gate both passed. This aggregate is not a full paint sequence
  and currently has no live production caller.
- The live effect, command ordering, input admission and capture lifecycle
  now bind C009 plus the original font, without a C011 companion. Original
  C101-C108 click-zone identities are unchanged. Legacy companion/count
  proofs are rejected in the focused negative tests.
- A successful Mon Oh Ir Ra cast now renders the same C013 pixels as the
  original-data panel at identical state. The original I34E test covers
  both recent-attack XP branches on separate source ticks. Its new render
  assertion exposed a cooldown mistakenly selecting an unopened action
  menu; MENU.C:778-797,2036-2039 supports retaining the icon path when
  G0506 is zero. Fourteen focused tests passed, including both original-media
  tests, without skips. This does not establish every cast/failure path.
- ReDMCSB CASTER.C:75-98 (I34) draws C009 without legacy C011 copies.
  The native painter now places the authentic 87x25 image at (233,50),
  retaining the full C013 click rectangle above it.
- `m11_dm1_spell_panel_real` passed against the original DOS 3.4 English
  archive: all pixels in C013 (87x33), four casters, sparse living slots,
  and Original/V2.0/V2.1 modes (24 cases), with original bitmap/font data.
  This proves the tested static panel, not spell-effect receipt integration,
  other editions, emulator parity or full-game completion. Publication is
  pending the integration and edition-routing work recorded in TODO-dm1.md.
# F0412 potion practice boundary regression (2026-09-06)

## F0209 wander movement gate

- Verified the prior-square admission rule with seeds 6 and 7: both select
  west, but only the zero two-bit prior-square sample admits returning
  west; nonzero falls back north. Both consume exactly one additional
  admission draw (GROUP.C:2164-2171). AI test: 2520 passed checks. Confirmed
  production dispatcher callers in the M10 orchestrator and M11 game view;
  this caller audit is not a full runtime movement-capture assertion.

- Corrected the ordinary C37 stationary look-around path to consume the
  admission draw before the smell-distance test, then a separate direction
  draw when last-move timing permits (GROUP.C:2217-2222). The seed-1
  regression previously failed facing and final-state checks; the rebuilt
  AI test passes all 2512 checks after correction. Negative reaction events
  and original emulator traces are not covered by this C37 fixture.

- Added four blocked-direction regressions for zero through three walls.
  A fixed source seed chooses west; fallback visits north, east and south
  in that order without extra direction draws (GROUP.C:2155,2199). All
  four cases retain the same final stream state. The rebuilt AI test now
  reports 2509 passed checks. Prior-square rejection, fully blocked groups
  and original-capture movement timing are outside these four cases.

- Corrected the inverted random-movement condition in the DM1 behavior
  dispatcher: GROUP.C:2153 moves on a nonzero one-bit sample. Two source-seed
  regressions (1: stay, 6: move) both fail before the correction and pass
  afterward. The complete creature behavior test reports 2493 passed checks,
  zero failures. This covers the decision in bounded RAM, not full C37
  scheduling, look-around RNG, CSB bridge behavior or emulator parity.

- Expanded post-fix verification: spell lookup, spell duration, potion
  power, spell-casting receipt and tick-source-ownership tests all pass
  after rebuilding their five targets. Added literal BASE.C seed-1 sample
  and second-state anchors to avoid relying solely on the runtime RNG helper
  when calculating the failure-path oracle.

- M10 now rejects unknown spell-table indices before consuming the XP RNG
  sample, following MENU.C F0412:1816-1826. Added a seed-preservation
  regression: it fails on the previous implementation and passes after
  moving the sample after lookup. Both focused F0412 tests and five DOS
  original-media C15 integration tests pass; unknown-command presentation
  is not claimed as original UI parity by this fixture.

- Audited M11_GameView_CastSpell: XP uses F0732(8), and paid validation
  uses F0732(128) with immediate first-failure return before flask admission.
  Added a low-skill M10 handoff regression with HAS_SPELL_XP: exactly one
  potion-power draw, no repeated XP/practice samples, unchanged supplied XP
  (420), and matching decoded/raw C08 mutation. Focused CTest passes.
  This is a source audit plus command-boundary fixture, not an original
  UI/input capture of the complete M11 path.

- Corrected M10 F0412 XP/practice samples from raw state to `state >> 8`,
  matching BASE.C F0027:1688-1695 and DEFS.H M003. The existing multiplier
  0xBB40E62D is correct for that game routine; CEDT002 is not its replacement.
  Both focused F0412 tests and all five original DOS C15 integration cases
  pass after rebuilding their targets. Updated stream expectations retain
  first/ninth failure, all-nine-pass flask gating, and raw potion mutation.

- Extended the runtime fixture with nine passing practice draws: absent
  flask consumes no potion sample or XP; a C08 flask in hand consumes one
  additional sample and updates decoded and raw potion type/power. Focused
  CTest passes. This verifies current-stream ordering and RAM mutation only;
  original execution trace comparisons remain in TODO.

- Corrected the failure-XP fixture's Wisdom Potion table index from 16
  (Stamina Potion) to 17. Added source-shaped receipt checks for nine
  successful threshold-equality probes (115), independent potion power
  masking (0xffff produces Mon power 255), and ninth-probe rejection at 116.
  Reference: ReDMCSB MENU.C F0412:1837,1853. The focused
  `test_dm1_v1_f0412_failure_xp_pc34_compat` passes. This is bounded
  source-contract evidence, not original emulator or complete RNG-stream parity.
