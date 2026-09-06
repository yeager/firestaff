# Firestaff TODO — DM1

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
  Separately audit cumulative permanent-XP overflow; F0849 still saturates
  those 32-bit totals, which the award-word correction does not address.
  Include F0303 level-query signedness in that audit: CHAMPION.C:730-737
  selects signed long for early Atari/Amiga and FM Towns 2.0, but unsigned
  long for PC 3.4 and the MEDIA720 editions. F0848 currently uses signed
  int32_t addition (including hidden/base averaging) and clamps negative
  values. Do not apply one overflow/level-query policy to every edition;
  establish edition dispatch and avoid host signed-overflow undefined behavior.

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
- Resolve the C006 generated-group cross-map teleporter failures before
  promoting the complete suite. Six assertions fail: source chain, target
  linkage/next, active state, C37 location and buzz receipt. The fixture
  contains raw C01 (00 0c 22 a0 00 01), and instrumentation finds the correct
  target map1/2/1. F0262 then requests party-map ACTIVE_GROUP for a newly
  generated group before insertion creates it; the helper returns failure
  and its caller discards that result, retaining source coordinates.
  Compare GROUP.C F0185:542-543, MOVESENS.C F0267:439-440,524 and
  F0262:76-83 with original execution. DUNGEON.C:1276-1278,1303-1305
  also selects ACTIVE_GROUP solely by party-map equality, so neither a
  decoded fallback nor reordered insertion is justified without further
  evidence. Obtain a runtime capture for this exact generation case.
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
