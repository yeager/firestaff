# Firestaff TODO — DM1

Reviewed 2026-08-29. Only open work is listed here.

- Complete legacy gameplay sound parity. Atari now selects original SND1
  records through a source-locked event map; verify live event timing,
  arbitration and PSG output against original captures. FM Towns local
  events now read F20 unsigned PCM from retained media; verify distance
  attenuation, channel scheduling and original-driver output. Amiga
  still calls the PC SND3 bank; startup also binds that bank through
  `graphicsDatPath`, which is only a display label for some retained legacy
  buffers. Amiga SND2/Paula needs its original maps, timing
  and volume semantics. Consult SOUND.C MEDIA007/MEDIA413/MEDIA488 and each
  DATA.C sound table before reusing any CSB transport: CSB's later sound
  indices and sample maps are not interchangeable with DM1's. Preserve the
  current no-generated-marker behavior while adding authenticated sample
  and live dispatch tests. CDDA/title success does not prove effect playback.
  Capture original F0060/F0061 runtime RAM boundaries for Atari sound indices
  1, 12 and 16 (records 534, 544 and 546): the bounded SND1 decoder rejects
  these original EN/DE/FR streams as short. Record 534 declares 100 samples
  but its 37-byte nibble stream describes 99. Do not invent a final sample
  or read beyond the allocated record to make playback appear complete.

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
  same-state original captures when those captures become available. The live
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
