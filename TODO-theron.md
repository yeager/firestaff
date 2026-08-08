# Firestaff TODO - THERON

_Auto-split from top-level TODO/DONE. Cross-cutting items remain in the top-level file._

## 2026-08-08 — nästa T900-bevis

- 🔧 Använd den sparade source-spawnkategorin när den autentiserade RNG-
  konsumenten fångas. Kategorin är nu provenance i live-poolen, men får inte
  driva HP, AI, attack eller generatorer innan `$4667`/`$5D64`/`$5D6A` är
  runtimebundna.

- 🔧 Råa itemrecords följer nu inventory genom pickup, drop och save/load.
  Bind den ursprungliga T900-konsumenten för equip/use/stack och validera
  dess state-skrivningar mot samma bytes innan någon regel aktiveras.

- 🔧 Kör den befintliga Mednafen/System Card-capturevägen med originalmedia
  för att ersätta `ram_consumer_2600=not_present`; utan den fångsten ska
  T700/T900-statistik, loot, AI och generatorlogik fortsatt neka mutation.

- 🔧 Bind den bevarade US-textcodonströmmen till originalets HuC6280
  textkonsument och kontrollkodtabell. Loadern får inte göra en hoststräng av
  `{...}`-värden innan den kedjan är fångad.

- 🔧 CDDA-intag och stream-handoff är verifierade mot den lokala original-RAR-
  korpusen. Bind fortfarande originalets spelhändelser till rätt CDDA/ADPCM-
  eller SFX-konsument innan ljud triggas från creature-, actuator- eller
  menylogik.

## Theron Authentic CD Trace Follow-up (2026-07-12)

2026-07-13 live stage-two correction: the authentic US-CUE/System Card capture
2026-07-15 post-`$3800` order gate: a future positive transcript must now
record the original Stage 3 `BRK $ff` IRQ2 return from `$3800` to `$3802`
before its later `$e009` dispatch. This proves ordering through the original
loader entry only. It does not classify the later sector, promote a grid,
or establish level, object, bitmap, palette, or transition semantics.

## Theron CUE IPL/Stage-Two Follow-up (2026-07-12)

The documented converted CUE layout now resolves only its explicit
that other selectors are CD commands or bind any later record to an object or
level; later loader execution evidence is still required.
physical MODE1 sectors are validated in JP/US media. Its payload role remains
218-unit manifest envelope, but its entries remain unclassified; do not treat

## Theron Track 02 Semantic Binding Follow-up (2026-07-11)

## Theron Original SRM Body Correlation Follow-up (2026-07-11)

Startup now exposes only fully gzip-trailer-authenticated unknown Save Disk
containers as opaque transfer candidates. They remain unavailable to Continue,
and failed SRM Continue leaves the world unchanged. The outstanding work is
still source-backed original body-layout correlation before any original SRM
can restore progression, party, or runtime state. Firestaff-native SRM export
also now publishes atomically without replacement, so it cannot overwrite a
staged original Save Disk artifact while the corpus remains unbound. The
direct SRM runtime handoff now requires all four hash-verified Track 02 media
surfaces and a selected real-media level bank before committing a restored
world; identity-only media rejects without mutation. Its structured receipt
now exposes the consumed media route mask, checksum, and selected level bank;
the remaining SRM blocker is still only original body-layout correlation.

## Theron's Quest

### Theron V1

- 🔧 2026-07-15 Track 02 post-Stage-2 `$e00f` service boundary: the same
  authentic 45-second boot receipt now covers direct non-System-Card calls to
  both System Card loader entries. Across two Stage-2 returns and 52 observed
  post-stage physical code pages, the only `$e00f` call is the already-known
  Stage-2 `$40a4 -> $e00f` setup, with `ff0000`/`ffff`/`ff` sentinel fields;
  the only `$e009` call remains `$3840` with the same invalid fields. No later
  direct game loader call to either entry and no game-owned `$1801` writer is
  observed. Indirect, block-transfer, or unobserved-route calls remain
  unclassified, so this is a boot-path boundary, not a universal absence
  claim. The next route still requires a non-sentinel caller correlated with
  a raw-sector receipt and verified return destination.

- 🔧 2026-07-15 Track 02 post-Stage-2 game-call boundary: an authentic
  45-second US CUE + System Card 3.0 capture accepts two real host RUN
  transitions, reaches two Stage-2 returns, and observes 61 physical code
  pages afterwards. It contains exactly one direct non-System-Card
  `$3840 -> $e009` call, but its record (`ff0000`), destination (`ffff`), and
  mode (`ff`) are all sentinel values; it is not followed by a game-owned
  `$1801` writer (only System Card `$e90d/$e92d/$e981` are observed). The
  candidate therefore remains rejected and cannot be treated as a later
  record or dungeon handoff. Next evidence must be a non-sentinel game call
  correlated with a subsequent raw-sector/SCSI receipt and a verified return
  destination.

- 🔧 2026-07-15 Track 02 live SCSI caller/destination boundary: a fresh
  authentic US CUE + System Card 3.0 capture records every `$1801` SCSI CDB
  byte with its HuC6280 caller, alongside each decoded READ(6) packet and raw
  sector binding. All 48 observed READ(6) packets, including later reads
  through generation 48 / LBA 4265, were issued by System Card `$e981`
  (command bytes) after `$e90d` selection; FIFO bytes were copied only by
  `$ea50` into System Card RAM `$1f:2256+`. No non-System-Card CD caller,
  dynamic `$e009`, or game-owned destination was observed, so none of those
  later records may enter the dungeon handoff. Next admissible evidence is a
  real game-code caller and destination after the System Card returns, tied
  to a hash-verified Track 02 sector and an original level/object consumer.

- 🔧 2026-07-14 Track 02 initial-level payload handoff: the one complete,
  trace-witnessed 2048-byte `$e009` payload is now copied atomically from the
  rehashed original MODE1 user-data sector into the runtime boot receipt.
  Record `0x0b52`, source coordinate `0x114`, destination `$3800`, byte
  count, and FNV-1a checksum must all agree; any change rejects the Soul Room
  route and cannot select a generated fallback. The payload remains opaque:
  its dungeon/object/tile/bitmap/palette grammar and a positive level
  transition still need original execution evidence.

- 🔧 2026-07-15 Track 02 level/object boundary: the authenticated original
  evidence is a game-owned post-`$3800` consumer that reads a separately
  hash-bound level/object record and proves its grammar.
  boundary: level envelope `[0x114,0x480)` and the remaining opaque bytes
  remains blocked. This proves media identity and record coordinates only;

  - Update 2026-07-20: the chain now generalizes the loader's per-byte
    consume/dispatch loop on original media, and evidence of where the
    loop terminates or dispatches into a record consumer.
    provenance only. Remaining: an authentic capture of the repeated

- 🔧 2026-07-11 Theron paired-CUE real-media follow-up: the hash scanner now
  accepts a CUE only when its one readable Track 01 AUDIO plus Track 02
  MODE1/2352 declaration canonically resolves to the independently
  hash-verified Track 02 payload. M12 passes that original CUE path to the
  launch profile, while an absent, malformed, renamed, or mismatched pair
  stays Track-02-only. No media is copied or synthesized. The bounded Track 01
  consumer now accepts only the CUE-declared WAV stem's local OGG counterpart
  and decodes it through optional Vorbis support; platforms without that
  decoder remain silent. Remaining work is user-staged JP/US title
  playback/capture evidence, not broader filename pairing or invented audio.

- 2026-07-27 Theron raw-CUE runtime launch regression: the current M11 path
  reaches the real startup route from the authentic USA MODE1/2352 CUE/BIN
  set (`f23601102138f87c33025877767ebf76`) and no longer relies on a direct
  Track-02-only probe. The focused runtime CTest advances title, stage, and
  Soul Room inputs under the dummy SDL driver, then requires
  `phase=theron-startup-2` and the original US asset identity. This proves
  startup admission and flow only; it does not promote unbound Track 02
  graphics, later dungeon records, or save semantics.

- 🔧 Track 02 graphics-format follow-up: the real hash-verified JP/US raw-BIN
  catalog found 1,522 strict HuC6260-shaped windows and 78 strict LE16
  stride-shaped windows across 2,022 exact matching nonzero MODE1 sectors.
  Its bounded detail list retained 64 records and overflowed 1,536; these are
  overlapping syntax matches, not independently proven palettes/tables. The
  catalog authorizes no decoder or runtime route. Exact media receipt:
  `docs/source-lock/tqr_v1_track02_graphics_format_real_media_2026-07-11.md`.
  Next evidence must trace one catalogued user-data offset through HuC6280 CD
  loader code to a VCE palette write or VDC VRAM destination, including the
  loaded byte count; only that can bind a candidate to graphics, a palette,
  or a compression routine.

- 🔧 2026-08-06 JP Stage-2 disassembly follow-up: the authentic JP Track 02
  BIN is now materialised as `~/.firestaff/data/theron/TQJP02.bin` and its
  IPL loader plus dynamic `$3800` payload receipt pass against record `0x4df`.
  The later static Stage-2 byte windows remain US-only because the JP image
  has region-specific bytes; do not widen those verifier gates until a JP
  disassembly identifies equivalent instruction/data spans and their callers.

- 🔧 2026-07-11 IPL-loader provenance update: original CUE sheets prove Track
  01 is CD-DA narration, while Track 02 is the MODE1 code track. The
  hash-gated JP/US Track 02 IPL information block at logical sector 1 selects
  record `0x0003a3`, load/entry `$4000`, and a 3-sector JP or 4-sector US
  executable. Both actual executables contain `JSR $e009` (System Card
  `CD_READ`) at CPU `$40cd`; the immediately verified setup selects local RAM
  `$3000` (`DH=$01`), not VRAM (`DH=$fe/$ff`). This is the first genuine
  loader/media linkage, but it does not bind the selected record, count,
  decompressor, palette, or graphics candidate. The next admissible step is
  bounded dataflow from this loader's record table through one complete read
  setup to a verified VDC/VCE destination; generated rendering remains
  fail-closed meanwhile.

  - Update 2026-07-21: L424B's callees and the $45A6 TII gap stream
    far-call targets, and L383E in the dynamic payload are future
    windows; the post-$3800 consumer chain remains capture-blocked.
    streams. Remaining: JP verification awaits staged JP media; the

- 🔧 2026-07-13 dynamic Track 02 RAM receipt: the instrumented original
  Mednafen route now requires a 32-byte FNV-1a receipt from System Card
  destination `$3800` immediately after the authenticated dynamic `CD_READ`
  returns. This proves record-to-RAM transfer but does not identify a Track 02
  source byte, decompressor, palette, VCE word, VDC transfer, level, or object
  family. Next evidence must tie that exact destination span to a hash-verified
  source sector and follow its bytes through one original VCE/VDC operation.

- 2026-07-16 update: the Track02 loader-intake chain now has a
  post-predecode-to-dungeon-level gate that preserves object/dungeon
  read-window topology only when it can also consume the source-locked initial
  level handoff for the same JP/US Track02 media. Missing raw media produces
  an explicit no-fallback blocker, and the positive branch remains conditional
  on `FIRESTAFF_THERON_TRACK02_RAW`. Remaining work is still real original
  loader/CD-read evidence that assigns a verified object-table or
  dungeon-record grammar before runtime/render admission.

- 2026-07-16 update: a grammar-admission barrier now consumes that
  dungeon-level topology receipt and preserves the original CD-read record,
  byte-window, hash, and topology evidence while explicitly requiring a future
  original object-table/dungeon-record grammar witness. It admits no grammar,
  decoder, runtime, rendering, fallback visual, or synthetic byte path.
  Remaining work is a real HuC6280/System Card trace that follows one of these
  exact windows into the original object or dungeon parser.

- 2026-07-16 update: the grammar boundary now also binds back to the
  read-table/layout-binding receipt, so a positive real-media path must
  preserve the exact original CD-read records, MODE1 user-data offsets,
  destinations, byte windows, copied-byte hashes, and topology hash before it
  can reach the grammar-witness-required blocker. Remaining work is still the
  original parser trace itself; this gate deliberately admits no object-table
  fields, dungeon-record grammar, runtime handoff, rendering, fallback visuals,
  or synthetic bytes.

- 2026-07-16 update: a parser-witness gate now admits object-table and
  dungeon-record grammar provenance only when supplied original trace facts
  prove that the original loader/parser consumed those exact preserved
  CD-read windows. Even that positive receipt keeps object fields, dungeon
  record fields, decoder semantics, runtime handoff, rendering, fallback
  visuals, and synthetic bytes blocked. Remaining work is to source such
  witness facts from a real HuC6280/System Card trace instead of a caller
  supplied receipt.

- 🔧 Phase 5 - Mechanics parity hardening: 50-assertion mechanics probe covers movement, click routes, doors, pits, teleporters, altar, combat, drops, and sounds. **2026-07-23 update (Lane E, cycle 11):** new `firestaff_theron_v1_mechanics_playability_probe` loads the authentic JP/US Track 02 Hall-of-Records level-0 grid and verifies movement, turning, wall blocking, and floor movement on the real 32×27 loader-accepted grid (36/36 PASS on staged TQUS02.bin + TQJP02.bin). **2026-08-06 update:** the real-data thing-data regression now discovers the supplied standard `~/.firestaff/data/theron/TQUS02.bin` path (or `FIRESTAFF_THERON_TRACK02_RAW`) before the legacy fixture path and verifies all seven dungeon object/text regions: AKUTUBA 228 ground refs/1021 items, DRATOR 249/969, FORMICIA 224/871, SARMON 226/1132, SHADODAN 264/980, THIEVES 255/988, DEMON 190/881. The loader also rejects non-sector-aligned raw input. Remaining work is broader real-asset gameplay traces for doors, pits, teleporters, altar, combat, drops, and sounds once those object semantics are source-locked.

- 2026-08-06 update: the real-data map, ground-reference and door/teleporter regressions now discover `FIRESTAFF_THERON_TRACK02_RAW` or standard `~/.firestaff/data/theron/TQUS02.bin` before the legacy fixture path. Against the supplied US BIN they verify all seven map groups, 4, 8, 5, 6, 3, 4 and 4 maps respectively; all seven ground-reference chains; and all seven door/teleporter tables. JP-specific map offsets remain a separate source-format gap and are not inferred from the US table.

- 2026-08-06 update: Track 02 raw-media intake now parses `FILE`, `TRACK`, and
  related CUE directives case-insensitively, matching the CUE format instead of
  depending on one editor's capitalization. A real-data regression builds a
  temporary CUE around the supplied `TQUS02.bin`, verifies the US pregap/index
  at raw sector 225, the authenticated BIN MD5, and trace preparation. The
  remaining intake gap is broader real CUE/BIN/ISO corpus coverage, not a
  generated fixture.

- 🔧 2026-08-06 Theron drop-placeholder removal: the old category-to-item
  resolver accepted synthetic item IDs and a host seed, then presented a
  guessed weapon, armour, consumable, scroll, or key as a real drop. The
  category table remains a verified item-name/category receipt, but no drop
  can be admitted until the original T900 consumer and selection record are
  decoded from Track 02. `theron_v1_drop_loot()` already fails closed at that
  boundary; the obsolete resolver and its positive fixture assertions are
  removed. Next evidence is a real T900 drop record plus its consumer.

- 🔧 2026-08-05 Theron production combat boundary: removed the inferred
  `theron_v1_compat.c` implementation from the `firestaff_theron` library.
  Production now uses the existing fail-closed adapter, so creature speed,
  AI, attack/defense formulas, spell combat, drops and sound IDs cannot be
  published from guessed records. Compatibility mechanics remain explicit in
  fixture/probe targets. The next replacement is still the authenticated
  Track 02 T500/T600/T900 consumer, not a new host-side table.

- 🔧 2026-08-05 Theron static consumer receipt: the authenticated US Track 19
  image now has a byte/MD5-locked regression for bank `$1f` `$243e–$24c3`.
  It proves the existing HuC6280 bitstream/register-map fragment against the
  real `TQUS19.iso` and explicitly records that the `$2600` consumer is absent
  from static ROM. The next step remains a real post-CD RAM capture with PC
  and source-LBA provenance; no RAM bytes or level/object semantics are
  inferred from this receipt.

- 🔧 Startup presentation hardening: stage/Soul Room render rows, enriched startup layout labels, and Track 02 descriptor-role receipt summaries are now test-visible; remaining work is real Track 02 startup art/audio decoding and pixel evidence instead of fallback text presentation.

  - 2026-07-08 update: Theron boot now owns the runtime dungeon/UI/V2-HUD/present render frame facade. M11 no longer calls `theron_vp_render_dungeon`, `theron_vp_render_ui`, V2 HUD render, or `theron_vp_present` directly in the Track 02 runtime path.

  - 2026-07-08 update: Theron boot now owns runtime ownership release for profile/world/viewport/assets, and M11 shutdown no longer frees those Track 02 objects directly.

- 🔧 Phase 7 - Save/import compatibility: round-trip, header-rejection, world-serialize-purchase-state, shop price-table regressions, and data-free cross-slot export/import are green. Remaining work is a real Track 02 save artifact import/export pass when such a save is available.

### Theron V2.0 / V2.1 / V2.2

- 🔧 Phase 2 - Enhanced asset pipeline: presentation-mode selection API + filter config + V2.1 EPX upscaler pipeline are wired (`theron_v2_texture_upscale_pc34.c` provides `theron_v2_epx_upscale` indexed→RGBA via PCE palette). The Theron V2.2 manifest parser remains available for fixture inspection, but production now requires `source_provenance="authenticated_track02"`; the existing procedural/gpt-image-2 pack is explicitly rejected as real data. Remaining: obtain source-owned Track 02 bitmap/material records and bind them before enabling V2.2 art.

- 🔧 **2026-06-27 Theron V2 Phase 3 HUD overlay:** `theron_v2_hud_overlay_pc34.c/.h` provides presentation-only HUD overlay for V2 modes on PC Engine 256x224 indexed framebuffer. Tests: `theron_v2_phase3_hud_overlay_probe` (40/40 PASS), `theron_v2_hud_overlay_pc34` (58/58 PASS), `theron_v2_hud_widget_assets_pc34` (105/105 PASS), `firestaff_theron_v2_hud_widget_assets_probe` (65/65 PASS). M11 runtime handoff and overlay seed gate are landed. **Remaining Phase 3 work:** (a) finish PBR top-bar / bottom-panel / action-strip bitmap assets under `~/.firestaff/assets/theron/hud/hud_widgets/` and `~/.firestaff/assets/theron/hud/hud_chrome/`, (b) author an example `~/.firestaff/assets/theron/hud/hud_widget_manifest.json` with `generator != "placeholder"` so the gate can promote to `PARTIAL`/`COMPLETE`, and (c) real-art visual verification + per-region pixel gates against real Track 02 captures.

- ❌ Phase 4 - Enhanced lighting/effects.

- ❌ Phase 5 - Smooth movement and viewport interpolation.

- 🔧 Phase 6 - Touch/controller ergonomics: **2026-06-29 initial Theron-specific input seed landed (presentation-only, data-free):** `theron_v2_touch_controller_affordance.c/.h` maps Theron V2 touch swipes, edge-strafe, D-pad, left-stick, and right-stick affordances onto the shared DM1-family C001-C006 command ids while rejecting every affordance when V2 presentation is off; `theron_v2_touch_runtime.c/.h` translates accepted affordances into `Dm1V1QueuedCommandPc34Compat` entries and adds a Theron 256x224 HUD-chrome exclusion gate for touch starts on the V2 top bar, champion mini-bars, and action strip while controller inputs bypass the framebuffer coordinate gate. New CTest `theron_v2_touch_controller_affordance` (267/267 PASS) and probe `theron_v2_touch_runtime_probe` (138/138 PASS) are data-free and source-locked against THQUEST.ASM T520/T560/T600 plus ReDMCSB DEFS.H:238-243, COMMAND.C:2045-2155, CLIKMENU.C:142/180, and GAMELOOP.C:164-219. Shared M11 SDL gamepad routing now exists; remaining Theron-specific work is a real touch-layout target-size audit across launcher/game views and real Track 02 runtime proof.

- ❌ Phase 7 - V2 verification suite.

## Theron Track 02 remaining evidence

- [ ] THERON-V1-TRACK02-LIVE-LOADER-CONSUMER: the latest replay against the
  authenticated US Track 02 ISO now gives a real HuC6280 loader witness
  (`$2286` `TIA` followed by 13 block transfers, 24 RTS and 24 post-RTS rows)
  plus 4,096 static-bank consumer reads and an executed `$2c54–$2c69`
  code-window receipt. The parser now accepts this richer real trace. It still
  has no `$2600` dynamic consumer bytes, no VDC VRAM/VCE snapshot, and no
  source-owned level/object field decisions, so visual runtime drawing and
  source-consumer correlation remain blocked. The interactive forcefield route
  now admits a source-only map/thing handoff from authenticated raw BIN data;
  it does not promote VDC/VCE pixels, host item semantics, or guessed field
  meanings. Next evidence is a capture that reaches the game-owned post-CD
  consumer and closes the VDC snapshot on clean exit.

- 2026-08-06 update: the Track 02 thing-category enum is now source-bound to
  the retail order used by DMBUILDER6 (`4=monster`, `5=weapon`, `6=clothing`,
  `7=scroll`, `8=potion`, `9=chest`, `10=misc`, `14=missile`, `15=cloud`).
  A real US Track 02 regression now checks all seven dungeon object-count
  tables and requires nonzero copied payload for every populated category.
  This is raw record provenance only; runtime item/monster publication and
  combat/render semantics remain closed until their consumers are bound.

- 2026-08-06 update: categories 4–10 now have a portable little-endian raw
  record decoder. It binds the two-byte next-reference prefix and the
  DMBUILDER field layouts for monsters, weapons, clothing, scrolls, potions,
  chests, and misc across every populated record in the real US corpus.
  Categories 14/15 now use the same source decoder for their six-/two-byte
  payloads; no item is published into the runtime object model yet.

- 2026-08-06 update: the full Track 02 dungeon loader now consumes those
  source-bound records and follows their authentic next-reference chains on
  all seven US dungeons. It reports decoded/unbound records separately and
  leaves `Theron_V1_Object` untouched for categories whose host owner is not
  proven. The remaining handoff is the original object-kind/item-index
  consumer, not raw media intake or chain traversal.

- 2026-08-06 update: each real Track 02 map header now survives the world
  handoff as an exact verified receipt (`x/y` offsets, opaque bytes, XP and
  door bytes, map id and creature count). These fields remain semantic
  read-only evidence; seed, spawn direction and object-kind publication stay
  closed pending the original consumers.

- 2026-08-06 update: the same world handoff now retains each real map's
  `creature_gfx_bank` and cumulative column thing-count from the Track 02 map
  directory. They remain raw level-record evidence; no creature graphics or
  object semantics are inferred from either field.

- 2026-08-06 update: every real category 4–10, 14 and 15 occurrence now carries
  both its exact raw bytes and the decoded source record (including missile and
  cloud payload fields) through the full-dungeon handoff. Host object-kind,
  inventory and projectile/cloud ownership remain deliberately unbound; no
  synthetic object is created.

- 2026-08-06 update: the real-data thing-record regression now covers both
  authenticated `TQUS02.bin` and `TQJP02.bin`. All seven Japanese dungeon
  blocks use their source-bound map/item offsets, retain 871–1 132 records per
  dungeon and decode every populated category without publishing a host
  object. Japanese text remains at zero until its codon consumer is proven;
  no translated or synthetic text is inserted.

## Theron Track 19 remaining evidence

- 2026-08-06 update: the authenticated 32x27 Track 19 startup-level record now
  survives the file-inventory handoff with its six raw header words, payload
  size/nonzero count and payload FNV-1a. This remains a source receipt only;
  tile, object and later-level semantics still require the original consumer.

- 2026-08-06 update: the real US and JP Track 19 startup envelope now has a
  bounded structural reader: big-endian 32×27 dimensions, six retained raw
  header words, and an 864-byte borrowed payload span are checked against the
  authenticated envelope hash. The payload remains opaque; tile/object
  ownership and later-level consumer semantics still require disassembly.

- 2026-07-15: Runtime level-bank selection now retains the authenticated
  startup bitmap's Track 02 MD5 and raw/user-data sector envelope. Remaining:
  obtain original loader/CD-read evidence that binds a post-startup bitmap or
  object-table record to a concrete runtime consumer. Do not infer palette,
  layout, object fields, or draw behavior from the retained startup envelope.

  - Update (latest): the positive decode vector can now feed a production M11
    Soul Room runtime-consumption receipt. The receipt selects Track 02 level
    0 through the live `Theron_RuntimeLevelMedia` Soul Room surface, verifies
    exact indexed-atlas route checksum/nonzero pixels/offsets against the
    decode vector, verifies 1:1 host placement and clipping, and permits M11
    host presentation only for that source-owned Soul Room surface. Generic
    dungeon draw, fallback visuals, scale changes, checksum drift, later-level
    graphics, and non-Soul Room routes remain blocked. Remaining: prove the
    original nonstartup dungeon graphics consumer and per-level render layout
    before promoting broader dungeon rendering or host uploads.

- Nexus Saturn memory-card intake remains opaque: the verified boundary accepts
  only an authenticated, hash-bound 8 KiB image with 16 x 512-byte blocks on
  an active title/champion route. Remaining work is an original-card corpus
  and capture proving the proprietary header, slot layout, checksums, and any
  state semantics; FNXS/native-save fallback remains prohibited.

- Nexus Mednafen capture remains operator-only: a dry-run manifest now binds
  VDP1 word layout, decoder, palette, pixel, or render admission from the
  current files.
  exact byte count, FNV, and SHA-256. Both byte streams remain uninterpreted;
  therefore remain capture-required: do not infer a source-to-command parser,

  - 2026-07-17 M11 presentation audit: the full-output admission is still an
    opaque evidence receipt. It authenticates one complete output byte range,
    its SHA-256/FNV, and later VDP1 command order, but deliberately publishes
    no indexed-pixel declaration, width, height, stride, CLUT/palette span,
    BGR/RGB ordering, transparency rule, or host placement. Both
    `graphics_permitted` and `decoder_promoted` remain zero. Do not connect
    this output to M11's indexed/palette surface, reuse WARNING.BIN's PP
    contract, or synthesize a title/menu image. A future original trace must
    attest all of those output-format facts before a byte-exact M11 consumer
    can be added.

- Nexus Structure1F multi-level capture remains no-draw: LEV00--LEV15 now
  original Saturn trace observations; mesh/face geometry and all
  pixel/palette semantics stay uninterpreted.
  remain missing.
  Remaining work is direct,

- The direct SLEV/SAL/MAP/SDDRVS discovery route now has the materialized
  English retail auxiliary corpus with positive hash/identity and bounded
  parser receipts. Retail-positive script/audio trace evidence, dispatch,
  decoding, and playback remain blocked. The direct SDDRVS dungeon
  admission also revalidates its direct file at consumption, but it still
  awaits authentic package/level/trace evidence before any script claim. The
  matching direct SAL/SLEV/MAP dungeon route now has the same identity-only
  rehash-on-consume guard; it does not establish a codec, event meaning,
  playback, or script semantics. The verified SAL `dsp01.EX` container
  preamble and bounded opaque payload interval are now retained only as
  provenance; descriptor/sample grammar and codec evidence remain open.
  Direct SNDLEV MAP provenance now also retains only its 24-byte header,
  bounded 8-byte rows, and terminator. M11 can bind one rehashed row to the
  active level/package/card/epoch, but selector/event semantics, codec proof,
  and playback remain unproven and blocked.

- Nexus SLEV task-body capture remains no-dispatch: every SLEV00--15 target
  requires matching admitted header/literal, raw-trace, and source-order
  receipts plus opaque external opcode and callback-owner labels. Remaining
  work is reviewed original-Saturn task-body grammar and callback ABI proof;
  no task opcode executes and no fallback script is admitted.
  The selected target can now enter M11 startup only through the matching
  direct SLEV/SAL/card/package/epoch receipt and exact SLEV FNV. That is an
  opaque source-order/trace admission only; authentic retail task-body and
  callback evidence is still required before any dispatch claim.

- Nexus SNDLEV/SAL capture planning remains playback-blocked: each unique
  audio, play sound, or draw. A real retail `NXSLSC01` capture and original
  command/driver semantics remain required.
  The payload remains opaque and non-retained; a real command grammar and

- Nexus PRS3 original-execution intake remains evidence-only: one independently
  authenticated V10 export must bind one MENU.BPK stream's complete SH-2 input
  reads, output fingerprint/range, and later VDP1 source command. Remaining
  work is reviewed opcode, pixel, and palette semantics; no decoder or graphics
  route is admitted.

  - 2026-07-22 capture-admission update: the final byte-admission stage now
    rehashes the supplied full MENU.BPK and DM.BIN bytes, derives the exact
    bounded MENU.BPK stream by the V10 offset/length, and requires FNV-1a plus
    SHA-256 agreement for those three source lanes before it accepts opaque
    output and VDP1 capture bytes. It also repeats the trace's strict final
    output-write -> VDP1-command ordering. This is not a PRS3 decoder, VDP1
    command parser, palette interpretation, pixel path, or draw permission.
    The remaining blocker is still an independently authenticated retail
    Mednafen/Saturn V10 export and its four real byte artifacts.

- Nexus PRS3 multi-capture review remains non-promoting: representative,
  independently authenticated MENU.BPK modes must agree on opaque bit-order
  and termination observations before a decoder candidate may be reviewed.
  Decoder, palette/pixel meaning, rendering, and fallback visuals remain off.

- Nexus Structure3 face/texturing capture remains capture-only: DGN face and
  Structure1F/2 provenance must agree with opaque material candidates and VDP1
  evidence. Pixel and mesh semantics remain unproven and no draw route opens.

- Nexus multi-level DGN capture remains opaque: LEV00--15 needs matched
  Structure1F, Structure2 placement, Structure3 face targets and ordered
  command/frame receipts. No decoder, mesh inference, or rendering is admitted.

- Nexus active dungeon route may report only capture-ready coverage when its
  loaded DGN identity matches the full multi-level adjudication receipt. Level,
  package, PRS3 trace FNV, or trace-size drift clears it. Decoder,
  mesh/texturing, and rendering remain unavailable.

- Nexus multi-level capture jobs remain operator-only planning data. A future
  Mednafen invocation must independently re-hash every staged retail asset and
  preserve the emitted job order; this planner never launches, captures, or
  interprets a trace.

- Nexus campaign asset intake is read-only and hash-first for explicitly staged
  loose files, ZIP members, and ISO/BIN/CUE members. Virtual container entries
  are never extracted or copied; unsupported containers remain blocked.

- Nexus Saturn memory-card startup intake remains opaque: authenticated 8 KiB
  card identity and selected route epoch may gate champion startup only. Save
  layout, FNXS fallback, and native-save semantics remain blocked.

- Nexus M12 card-startup selection consumes only exact opaque card/epoch
  readiness; native FNXS resume remains a separate route.

- Nexus Saturn-card discovery currently admits only one direct 8 KiB file;
  virtual ZIP/ISO/BIN/CUE identities are diagnostic-only and contents stay
  opaque; container launch remains blocked.

- Nexus champion startup accepts only an atomically bound direct card, package
  identity and current M11 route epoch; when the M11 PRS3 presentation receipt
  is present, it must share that exact package and epoch. Card bytes remain
  opaque and PRS3 remains no-draw.

- Nexus Structure1F records now retain parser-observed raw spans only; face,
  mesh, palette and texture semantics remain unproven and no-draw.

- Nexus Structure2 descriptor spans are source provenance only; codec, pixel
  and palette meaning remain blocked pending original evidence.

- Nexus Structure3 face spans are raw package provenance only; PRS3, palette,
  pixel and texture semantics remain blocked. The direct-source admission now
  also retains one hash-bound 40-byte entry header, its raw tag/count fields,
  and the three count-bounded 12-byte intervals only when the already admitted
  Structure3 target and ordinary source file still agree. This is framing, not
  a geometry, normal, material, texture, transform, or draw claim. The local
  retail LEV corpus is still absent, so positive corpus confirmation remains
  pending.

- Nexus Structure3 image/palette references are bounded source intervals only;
  codec and decoded surface admission remain blocked.

- Nexus MENU.BPK startup provenance now binds a selected PRS3 entry's bounded
  payload offset/length/FNV and header facts through an epoch- and
  package-bound M11 no-draw host receipt. Any engine-owned verified row may be
  selected, but its recognized mode byte, bounded opaque compressed body,
  declared output size, and body FNV must exactly match; unknown modes and
  declaration/span/FNV drift reject, including across launcher/card epoch
  transitions. PRS3 pixels, opcode grammar, and decoder promotion remain
  unavailable pending independent original-Saturn codec evidence.

- The legacy `nexus_v1_bpk_surface_class` synthetic fixture still asserts a
  synthetic PRS3 literal decoder and decoded material import. Its stored
  payload receipt now keeps the fallback-provenance bit closed, but it is
  incompatible with the current retail fail-closed PRS3 route and is not
  evidence for a Saturn codec; replace it with authenticated capture-backed
  expectations before treating it as a promotion test.

- 2026-07-17 DM1 original-save C-event package completed: F0435 now retains
  C2 `ActionIndex` and `PoisonEventCount`; F0802/F0796 preserve their bounded
  PC34 bytes. C25 and C29 exports require authenticated F0435 provenance,
  while C3/C4 snapshot drift, malformed poison width, synthetic C25/C29, and
  invalid source squares reject. The targeted original-save handoff suite is
  green; remaining work is external original-save corpus evidence.

- 2026-07-17 DM1 C2 PARTY_INFO follow-up completed: source byte 86
  `Event71Count_Invisibility` now materializes into both M10 invisibility
  owners and F0802 writes it back only as a bounded PC34 byte. The focused
  C71 path and full original-save handoff suite are green.

- 2026-07-17 DM2 DB14: the normal `QUERY_PICST_IT` `0x40`/neutral-mode branch
  now copies only authenticated native-size indexed IMG3 pixels under matching
  RAW4 clip and palette receipts. Flip, crop, nonzero offset, scaling, and
  every other blitmode remain fail-closed. Remaining: source-proven non-normal
  transform branches and live frame ordering.

- 2026-07-17 DM2 HUD SUMMARY_IMAGE: `c_gui_draw.cpp:926-942` now has a
  no-draw M11 receipt for exact `(1,vb_144,field)` HUD commands. It requires
  the source plan's decoded GDAT pixels, local palette, and RAW4 destination
  identity; tuple mismatch, absent palette, and stale destination reject.
  Remaining: source-proven HUD transform admission before any new draw path.

- 2026-07-17 DM2 HUD PICST transform: the exact `c_gui_draw.cpp:926-942`
  branch admits only source values `0..0x28`, retaining X scale `0x1f` for
  `0..0x0f` or `0x2f` otherwise and Y scale `0x35`. Out-of-range values,
  missing SUMMARY_IMAGE material, or stale destination reject; it remains
  source-gated for draw only where the resolved destination is the complete,
  exact scaled rect. Partial/unknown `QUERY_BLIT_RECT` clipping, flips, and
  every other HUD transform remain no-draw.

- 2026-07-17 DM2 pit viewport admission: `c_gui_vp.cpp:234-292`
  `DM2_DRAW_PIT_TILE` now has a bounded source receipt for cells 1..15. It
  binds `table1d6c70/90/a0/b0` selection, the live cell's `+8` state word,
  `DRAW_DUNGEON_GRAPHIC` light parameter, exact `(GRAPHICSSET,field)`
  SUMMARY_IMAGE, GFX256 raw material, decoded U4 bytes, and local palette.
  It remains `no_draw`: cell 0's `SET_GRAPHICS_FLIP_FROM_POSITION` and the
  selected `QUERY_BLIT_RECT` placement/clip chain are not yet proven.

  - 2026-07-17 latest cell consume state: normal (`blitmode=0`) cells 1, 3, 4,
    6, 7, 11, 12, 14 each admit their exact GRAPHICSSET field, RAW4 rect,
    independent B073/RAW7 palette receipt and ordered U4 handoff. HFLIP cells
    2, 5, 8, 13, 15 admit their exact fields/rects with source-locked reverse-X
    U4 row walk. Every other mirrored or unproven form remains fail-closed.

  - 2026-07-17 crop/chained-clip update: `QUERY_BLIT_RECT` source-coordinate mutation remains no-draw behind a source-locked PIT provenance receipt; root RAW4 does not prove crop or chaining.

- 2026-07-17 DM2 `DRAW_STAIRS_FRONT` primary GDAT material admission:
  `SKULLWIN/c_gui_vp.cpp:480-511` and `dm2data.cpp:289-310` now bind the
  successful `QUERY_GDAT_ENTRY_IF_LOADABLE` branch only: exact state-table
  lane, GRAPHICSSET SUMMARY_IMAGE/GFX256 raw bytes, decoded U4 indices, local
  palette, root RAW4 placement and the live DM2 composition/surface snapshot.
  It remains no-draw. The `QUERY_TEMP_PICST` fallback and the downstream
  B073/`QUERY_PICST_IT`/`DRAW_PICST` transform must be proven separately.

  - 2026-07-17 fallback update: the exact non-loadable `table1d6f7c` path at
    `c_gui_vp.cpp:514-527` now admits its own SUMMARY_IMAGE/GFX256 U4 and
    RAW4/M11 receipt plus `QUERY_TEMP_PICST(1,0x40,0x40,0,0,0,rect,-1,light,
    -1,8,graphicsset,field)` provenance. It remains no-draw because
    `query_32cb_0804` selects a live B073/field-7 palette transaction from
    `c_querydb.cpp:2415-2465`, which is not yet authenticated.

- 2026-07-17 DM2 `DRAW_STAIRS_SIDE` primary material admission:
  `SKULLWIN/c_gui_vp.cpp:540-565` and `dm2data.cpp:275-287` bind only cells
  1..8 with a defined `table1d6fdc/table1d6fee` state lane to authentic
  GRAPHICSSET SUMMARY_IMAGE/GFX256 U4 bytes, local palette, root RAW4 and M11
  owner surface. B073/`DRAW_PICST` remains no-draw pending a live palette and
  transform receipt.

  - 2026-07-17 transform provenance update: `SKULLWIN/c_image.cpp:450-475`
    now binds the side-stairs `DRAW_DUNGEON_GRAPHIC` delegation to blit mode 0,
    default normal scale and zero source offset; its source rects explicitly
    exclude the `0x2bc/0x2bd` offset special case. Material, RAW4 and M11
    identities must agree. The live `DM2_query_B073(image.palette,
    ddat.v1e12d2, alpha, -1, ...)` transaction remains unauthenticated, so
    the complete branch is intentionally no-draw.

  - 2026-07-17 live `DRAW_WALL` update: the receipt now binds one existing
    `QUERY_TEMP_PICST` wall command to the same recomputed material hash,
    M11 wall-composition identity and atomically identical owner snapshots.
    Only the source's `0x40` normal scale, RAW4 `0x2be + cell`, movement
    offset and source flip are recorded. This gate remains no-draw; it does
    not introduce a second wall renderer.

- 2026-07-17 DM2 `DRAW_WALL_TILE` admission: `SKULLWIN/c_gui_vp.cpp:6703-6741`
  and `dm2data.cpp:266-273,602-605` now bind every `table1d7012` cell branch
  to the existing authenticated wall/M11 identity. The receipt records the
  exact 0/1/2 delegated-call count and `table1d6afe` orientation; it remains
  no-draw because `DM2_guivp_32cb_15b8` has separate unbound GDAT transforms.

  - 2026-07-17 DRAW_TEMP_PICST admission (latest): three category-8/9
    `32cb_15b8` input receipts combine with loadable `0x0f` agreement into
    a no-draw consumption gate. It carries no destination or pixel information.

- 2026-07-17 DM2 `query_B073` input admission: `c_querydb.cpp:2506-2545`
  now requires authentic palette, live light, alpha/mask, colors/cache,
  RAW7, lookup and traversal identities in one no-draw receipt. No palette
  buffer or pixel result is produced.

  - 2026-07-17 DRAW_PICST M11 update (latest): the fully authenticated 8-bit
    BLITMODE0/PAL256/mask native executor now enters only through a DM2-owned
    M11 consumer that requires the exact live material handoff buffer/palette
    and owner generation. The complete B073-to-surface pipeline (palette
    borrowing, material pairing, QUERY_BLIT_RECT mode-1/global-clip/intersection,
    DRAW_PICST rect/surface-address/row-traversal/mask/palette-index/write) is
    source-proven. No legacy renderer or fallback path can reach this consumer.

  - 2026-07-17 DRAW_WALL latest state: the full B073 RAW7 interpreter
    (`c_gdatfile.cpp:1919-2003`, `c_querydb.cpp:2506-2668`) source-binds
    PAL16-to-PAL256 cache. Normal unflipped 0x40 U4-to-8 and BLITMODE1
    HFLIP branches consume the authenticated B073 cache. Scaling, vertical/
    chained flips, movement and scale changes remain fail-closed.

  - 2026-07-17 DRAW_DOOR latest state: stationary closed panel, horizontal
    split states 1..3, vertical intermediate states 1..3, right/left jamb
    frames (reverse-X / forward-X), and DOOR_FRAMES movement are all
    source-proven M11 consumers. Panel motion, scaling, flips and every
    incomplete table/material chain remain fail-closed.

- 2026-07-17 DM2 pit-roof viewport admission: `c_gui_vp.cpp:118-206` now
  source-gates cells 1..8 on the exact roof flag, `LOCATE_OTHER_LEVEL`
  success, remote tile type 2, and remote bit 0x08 before applying
  `table1d6c4c/5e/67`. The resulting GRAPHICSSET SUMMARY_IMAGE, GFX256 raw
  receipt, decoded U4 bytes and local palette remain `no_draw`; cell 0's
  position flip, the actual remote-map address walk, and `QUERY_BLIT_RECT`
  placement/clip still require separate evidence.

  - 2026-07-17 ordered-consume update (latest): the full PIT_ROOF pipeline is
    source-proven: B073 c_light/RAW4/alpha/blend, RAW7 table/traversal,
    QUERY_PICST_IT destination, surface-owner, composition-slot and
    material-buffer handoff. The source-owned hook now executes DRAW_PICST's
    authenticated normal-scale U4-to-8bpp masked rows including horizontal
    mirror. Every crop, scale, vertical/combined flip, changed source index,
    composition/surface drift, or incomplete receipt remains no-write.

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
  opening-door capture; compare a rebuilt installed app against v3.0.197
  before diagnosing or masking the old red-strip report.
  an invalid step-zero gap and retained the closed C004/C002/C003 page. The

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
  The manual does not replace the required original PC34 runtime capture or
  the M564 name/slot evidence.
  C00/C01 hand masks and backpack ownership remain source-backed. The F0033

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
  **2026-08-07 real possession-continuation gate:** the corpus regression now
  passes every genuinely decoded direct-root link, in source order, into the
  bounded `DM2_2066_062b` 10-bit continuation reader. The 135/135 real
  PC-DOS checks therefore cover both record-body consumption and the
  subsequent type-9/type-0xE continuation boundary. The receipt remains
  read-only; live record-pool, possession-index, timer and GAME_LOAD owners
  are still not connected.

- [ ] DM2 champion-mirror activation: the canonical PC G1 dungeon has 16
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
  **2026-08-13 delayed-owner audit:** when the exact half-step gate admits,
  the execution receipt now exposes six missing live-owner bits (hero load,
  wounds, walk speed, Aura-of-Speed, current pose and tick/countdown). The
  proven mask remains zero for caller-supplied compatibility snapshots; no
  interpolation or viewport offset is enabled.

- [ ] DM2 creature animation-frame ownership: `DM2_1c9a_0958` now carries
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
  **2026-08-13 empty-panel removal:** successful DM2 launch/resume and the
  generic DM2 launch-failure callback now return M12 to its ordinary main
  view instead of displaying a blank host message panel. The launch intent
  and structured failure receipt remain intact; M11 can therefore hand the
  next visible frame directly to the source-owned `SHOW_MENU_SCREEN` or
  dialogue path. The actual source failure dialogue producer is still open.

- [ ] DM2 runtime action/save text ownership: action, shop, movement and save
  **2026-08-13 pre-resolver correction:** DM2 quick-save and quick-load now
  enter the source-owned silent boundary before shared path resolution. This
  prevents path-length, directory and other generic host errors from leaking
  into the DM2 status channel. The original `DM2_GAME_SAVE_MENU`/GAME_LOAD
  producer is still not connected, so the item remains open.

- [ ] DM2 GDAT structure loader: `DM2_READ_GRAPHICS_STRUCTURE` remains
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
  **2026-08-07 party-wound correction:** the diagnostic `DM2_ATTACK_PARTY`
  seam now applies the source `DM2_MAX(1, per_hero_damage)` clamp before
  `WOUND_PLAYER`, matching `skhero.cpp:3365-3392`; a `base_damage=1` regression
  is green. The live champion/target/RNG/writeback chain remains absent.

- [ ] DM2 FM Towns English text consumption: a selected FM Towns Japanese CD
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

- **DM2 SKSAVE direct-root pool ownership:** The raw DB baseline and DB4–DB15
  clear phase are now followed by source `READ_RECORD_CHECKCODE` allocation
  into the authenticated c_record pools, including source list links,
  child-owner fields, type-9/type-0xE continuation writes, and a hash/count
  receipt. Remaining work is attaching the returned roots to champion/hand,
  possession-index and tile-chain owners; failed decode restores the cleared
  baseline and never publishes a session. The mounted workspace has no raw
  SKSAVE corpus, so this positive path remains compile/test-gated until one is
  supplied.

