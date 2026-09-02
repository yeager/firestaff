# Firestaff TODO — DM1

Reviewed 2026-08-29. Only open work is listed here.

- Complete the F0128 renderer as one source-order executor. Center-square
  and side-square F0115 routes now consume their authentic one-based
  cell-order word, including the `DOORPASS1 → F0111 → DOORPASS2` partition
  for real floor items, creature groups and projectiles. The final occlusion
  replay now completes D3/D2 outer lanes and D3--D1 normal side lanes one
  source square at a time; `DOORPASS1` is replayed between each route's
  wall/ornament envelope and F0111 door, while the foreground tail retains
  `MAIN`/`DOORPASS2`/F0113 order. The primary structural pass is still
  composed in bounded batches before that replay. Replace the remaining
  batches only with the corresponding F0116--F0124 operation, never with a
  host-generated panel or a substitute asset.
- Add F0112 ceiling-pit to the live per-square F0128 executor. The current
  scheduler models F0104/F0107/F0108/F0111/F0113/F0115, but
  `F0116`--`F0127` also call F0112 before their F0115 tail for the applicable
  open, pit and stair routes (for example `DUNVIEW.C:7359-7365` and
  `7929-7935`). The source helper now maps the real D2/D1/D0 bitmap frames
  for both MEDIA508 and MEDIA720 explicitly: their numeric graphic/zone ids
  overlap, so the media profile is mandatory and auto-detection must not be
  extended past the legacy D2 contract. M11 still lacks the decoded
  level-above pit state that F0112 consults, so it has not bound that material
  into the live square transaction. Do not promote an invented ceiling panel;
  thread that source state, then bind the original GRAPHICS.DAT material and
  clipping zone through preflight.
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
  18-frame title plan, CDDA title track, and its input matrix, but still needs
  an authentic gameplay capture before visual parity beyond that route can be
  claimed.
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

The missing C13 save/capture corpus is deferred rather than a release blocker
for the native media paths currently under active work. When an
operator-supplied corpus becomes available, add its hashes and provenance,
then promote only the routes it directly proves. A fixture-derived save may
exercise the original runtime, but cannot by itself close the preservation
evidence gap.
