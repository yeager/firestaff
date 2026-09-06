# Firestaff DONE — DM1

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
