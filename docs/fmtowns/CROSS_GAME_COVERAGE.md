# FM Towns cross-game recovery coverage

Session 2026-08-11 comprehensive tally of Firestaff's byte-verified
FM Towns real-data recovery across DM1, CSB, and DM2.

Latest verification (2026-08-11): the complete checkout builds successfully;
the authentic CSB FM Towns handoff suite passes 6/6 executed tests (one
explicitly skipped switch-only gate), and the authentic DM2 FM Towns M12,
title, and NEW GAME/gameplay suite passes 3/3. These are opt-in tests and read
the user-owned archives in place.

The CSB direct loose-tree handoff now carries the selected language through
M11: `FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE=en` binds `CDATA/CHTWE.EXP`, while
`ja` binds `CJDATA/CHTWJ.EXP`. A shared extraction can no longer silently boot
the first directory found by the filesystem scan; each selected graphics and
dungeon pair must match its authentic FM Towns hashes.

Packed-media runtime coverage is now closed for CSB as well as DM2. CSB can
start M11 directly from the authentic ZIP: the original IMG member and the
selected `GRAPHICS.DAT`, `DUNGEON.DAT`, `CHTWE/CHTWJ.EXP`, `MINI.DAT`, and
`TITLE.ANM` members stay in bounded RAM. Firestaff creates no loose runtime
tree and does not rewrite the source archive. The real-data gates
`csb_v1_fmtowns_archive_launch_real` and `csb_v1_fmtowns_packed_m11_real`
cover this path.

The CSB M11 handoff follows the CSB materializer's original `FMTOWNS.IMG`
name for F0743 CUE-track dispatch; DM1's separate materializer continues to
use its own `FMTOWNS.BIN` name.

The CSB switch-only gate was also run directly against the materialized
`SWITCHTW.EXP` on 2026-08-10: 18/18 assertions passed. The skip above is only
the default CTest invocation without the optional switch-path environment
variable; it does not indicate a parser failure.

## Discs used

| Game | Disc archive | Track 01 sha256 (via header strip) |
|---|---|---|
| DM1 | Dungeon Master (Japan) (En,Ja) (Rev 1).7z | c888470d.. (EDM.EXP) |
| CSB | Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip | 08cceb0c.. (CDATA/GRAPHICS.DAT) |
| DM2 | Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip | 634e7004.. (DATA/GRAPHICS.DAT) |

Full per-file hashes in `docs/fmtowns/all_games_real_data_hashes.json`.

## Cross-game byte-identical payloads

Data structures that appear byte-identical across two or three games
at per-game virtual addresses:

| Payload | Size | DM1 vaddr | CSB vaddr | DM2 vaddr | Games |
|---|---:|:---:|:---:|:---:|:---:|
| Menu font raster | 768 | (via asset 557) | 0x50f1a in GRAPHICS.DAT | 0x2f5a3 in GRAPHICS.DAT | 3 |
| CHAR geometry (7 words) | 14 | 0x26c8a | 0x2c94c | 0x1f6 | 3 |
| ICON geometry (4 words) | 8 | 0x26c68 | 0x2c938 | 0x1de | 3 |
| SPELL_COSTS | 32 | 0x24388 | 0x29f64 | 0x3bb0 | 3 |
| SPELL_MULT | 8 | 0x243a0 | 0x29f7c | 0x3bc8 | 3 |
| Phar Lap fs: 4-slot bridge | — | ✓ | ✓ | ✓ | 3 |
| Direct I/O port 0x04E9 | — | ✓ | ✓ | ✓ | 3 |
| OICON descriptor | 1344 | 0x224db | 0x27f77 | — | 2 (DM1+CSB) |
| PLAYER_COLOR | 8 | 0x291b8 | 0x2d164 | — | 2 |
| ICON_PAL | 6 | 0x28f44 | 0x2cd8a | — | 2 |
| DYNA_BUTTONS pool | ≥500 | 0x24194 | 0x29d50 | — | 2 |

## Per-game unique payloads

Data specific to each game that other games do NOT share:

- **DM1 EDM.EXP SYM1 table** (1174 entries) — unique to DM1.
- **CSB TMENU.EXP SYM1 table** (1724 entries) — unique to CSB launcher.
- **DM1 region registry** (23 blocks / 994 records) — unique DM1 menu layout.
- **DM1 LEVEL_SONGS** — DM1-specific CDDA mapping.
- **DM1 DOOR_PAL, DM_MUSIC** — DM1-specific chrome.
- **DM1 tmenu_input schema** (TMENU.EXP event queue) — DM1 launcher.
- **CSB CHTWJ.EXP, SWITCHTW.EXP, ANIMTW.EXP, UTILE.EXP** — CSB-specific binaries with independent code but no SYM1 stripped from release.
- **DM2 SKULL.EXP** — DM2 game code, no SYM1, no shared menu-render
  data (different action-label set).
- **DM2 GRAPHICS.DAT format** (extended v4 raw-size table with an ENT1
  directory in raw item 0) — DM2's own container variant.

## Shipping Firestaff modules

Every module below is source-locked with a real-data round-trip test:

### Byte-verified constants + accessors

- `dm1_v1_fmtowns_text_geometry` (CHAR/ICON macros — same for all games)
- `dm1_v1_fmtowns_icon_geometry` (ICON constants — same for all games)
- `dm1_v1_fmtowns_icon_category` (LOAD_ICON threshold table)
- `dm1_v1_fmtowns_oicon_descriptor` (1344-byte OICON records)
- `dm1_v1_fmtowns_dyna_buttons` (English action label pool)
- `dm1_v1_fmtowns_font_asset` (asset 557 identity)
- `dm1_v1_fmtowns_font_rasteriser` (6x128 raster decoder)
- `dm1_v1_fmtowns_menu_regions` (23-block region registry)
- `dm1_v1_fmtowns_menu_bss` (12 vaddrs + PLAYER schema)
- `dm1_v1_fmtowns_egb_rect` (region-tree walker)
- `dm1_v1_fmtowns_menu_render` (end-to-end composer)
- `dm1_v1_fmtowns_music_tables` (LEVEL_SONGS, SPELL_COSTS, etc.)
- `dm1_v1_fmtowns_edm_sym1` (1174 named symbols)
- `dm1_v1_fmtowns_snd_api` (36 SND_* function vaddrs)
- `dm1_v1_fmtowns_tmenu_input` (TMENU event schema)
- `dm1_v1_fmtowns_tbios_id` (8 TBIOS version fingerprints)
- `dm1_v1_fmtowns_pharlap_bridge` (4-slot layout)
- `dm1_v1_fmtowns_direct_io` (5 direct I/O ports)

### Cross-game aliases

- `csb_v1_fmtowns_oicon_descriptor` (aliases DM1's OICON via vaddr)
- `csb_v1_fmtowns_dyna_buttons` (aliases DM1's label pool via vaddr)
- `csb_v1_fmtowns_tmenu_sym1` (1724 CSB-specific launcher symbols)

### Shared cross-game infrastructure

- `fmtowns_pharlap_all_games` (11 binary profiles, 8-game direct I/O)
- `fmtowns_graphics_dat_format` (legacy/ext_v1/ext_v4 classifier)
- `fmtowns_geometry_all_games` (per-game CHAR/ICON vaddrs)
- `fmtowns_shared_tables_all_games` (SPELL/PLAYER/ICON per-game vaddrs)
- `fmtowns_font_raster_all_games` (per-game font raster locations)
- `dm2_v1_fmtowns_graphics_dat_ext_walker` (source-verified ext-v4/v5 raw
  table and ENT1 directory traversal)

### Documentation + evidence

- `docs/fmtowns/all_games_real_data_hashes.json`
- `docs/fmtowns/pharlap_call_sites.json` (502 call sites, 11 binaries)
- `docs/fmtowns/TOWNSOS_BIOS_INTEGRATION.md` (Tsugaru + FMT_F20.ROM)
- `docs/fmtowns/CROSS_GAME_INVENTORY.md` (earlier inventory)
- `docs/dm1/fmtowns_real_data_hashes.json` (DM1-specific hashes)
- `parity-evidence/dm1_fmtowns_region_table_full.md` (994 records)

## Completeness

- **DM1 FM Towns**: 100% of extractable byte-verified real data
  now shipped. Every runtime table, symbol, and I/O surface a
  hosted-BIOS integration would need is source-locked.
- **CSB FM Towns**: The authenticated CHTWE/CHTWJ, SWITCHTW, MINI.DAT,
  portrait, dungeon-tail, utility, and source HUD routes are wired. The
  real English and Japanese `MINI.DAT` files resume through M11 into the
  saved map-4 state without replaying the title. C06 `SAVE CHAMPIONS` now
  writes only existing, authenticated `.CMP` records from the real
  `PORTRAIT` catalogue: the native header is preserved and only the
  receipt-bound planar payload changes. `F7002_ReadCMP` also imports a
  revalidated selected catalogue entry into the selected party slot, copying
  only its source name/title and planar payload. The catalog-bound selector
  preserves the authenticated source order, provides bounded previous/next
  movement, and delegates the selected row to that same import transaction.
  Its C06 F7083/F7084 source-coordinate file-list state, nine-row raster,
  bounded scroll commands, and real `PORTRAIT` catalogue binding are now
  implemented. The remaining utility gap is wiring that state into C06's
  modal event pump and returning the selected row to F7002. Arbitrary
  `CSBGAME.DAT` load/write/resume remains fail-closed until its
  exact source transactions are proven. The
  external `fmtowns-save-corpus/CSBGAME.DAT` and `CSBGAME-JP.DAT` files are
  retained as candidates, not used as synthetic substitutes.
- **DM2 FM Towns**: The authentic HME-242 CD path, TWANIM/TITLE/SWOOSH/END,
  M12 handoff, English companion overlay, title input, NEW GAME, active
  gameplay movement, viewport frame, native HUD plan, all 16 authentic
  CHAMPIONS portrait types, and the eight source action-hand selection events
  (116..123) are verified by the real-media M11/M12 regressions. The click
  owner reads the authentic 640x400 FM-Towns RAW4 rectangles and converts
  pointer input to Firestaff's 320x200 presentation surface. The three source
  action-panel command events now reach the CMDSTR-backed runtime owner;
  unsupported or unavailable item/action records still fail closed. The boot
  receipt also retains the complete authenticated 264-record `SKULL.EXP`
  MOUSE_INPUT span (1584 bytes, FNV-1a `0x1500c4c9`) and exposes raw
  event/flag/rect/mask candidates by source ordinal. This is evidence for
  context-specific UI owners, not a global hit-test: Towns reuses rectangle
  IDs across branches.
  The first context-bound consumer now identifies route ordinal 117 as the
  source `hand_panel.action_1` branch, while event 0x70/112 closes that panel
  through M11. Other candidate branches remain unavailable until their live
  UI state owners are bound.
  The native inventory census resolves 129/166 source contexts; ordinals
  47-49, 52-83, 99, and 110 have no matching Towns RAW4 geometry and remain
  fail-closed.
  After authenticated `GAME_LOAD`, the source inventory bridge can now swap a
  real `c_hero::item[30]` link with `LeaderPossession`, with record-pool
  validation, `OBJECT_NULL` handling, read-back verification, and rollback.
  M11 also consumes the real `INTERFACE_CHARSHEET/0/dtImage/1` frame through
  the global 255-colour `PAL_IRGB` route and its authenticated `RECT_1EE`
  RAW4 source crop. Authenticated Towns inventory contexts route pointer
  input to M11 for panel and slot selection. This is not a claim of complete
  inventory parity: source text, item movement/equip commits, and
  non-equipment owners remain unavailable and fail closed.
  The native viewport event 0x50/rect 0x0007 reaches the DM2 c_rwbb target
  resolver and cannot fall through to DM1's C080 handler. Inventory/dialogue
  pointer ownership and map-dependent viewport mutations remain fail-closed
  gates until their source owners are bound.
  Direct launch also accepts the authenticated
  loose `fmtowns_iso` tree. Native startup works from the exact loose
  TWANIM/SKULL/TITLE/SWOOSH/END members even without a ZIP; when the original
  HME-242 ZIP is beside that tree, it remains the CDDA owner. The corpus
  contains no FM Towns SKSAVE artifact, so save/resume remains fail-closed;
  DOS SKSAVE files are not used as a Towns substitute.

## What is intentionally NOT shipped

- **Full extended-format GRAPHICS.DAT coverage** for CSB (0x8001) and DM2
  (0x8004). The DM2 raw table, ENT1 directory, and authenticated FM Towns
  IMG2/U4 hand-action decode are shipped; broader per-record presentation
  coverage remains game-specific work.
- **TownsOS BIOS runtime execution**. Requires Tsugaru integration
  or hosted TBIOS shim (see integration doc).
- **JDM Shift-JIS glyph bitmaps**. Byte-verified 2026-08-07: the
  Japanese Rev 1 disc contains NO game-owned Shift-JIS bitmap.
  JDATA/GRAPHICS.DAT asset 557 (768 bytes) is byte-identical to
  the English DATA/GRAPHICS.DAT asset 557 — the Japanese release
  reuses the same ASCII font raster for Latin characters and
  delegates all kanji glyphs to TownsOS TBIOS at runtime. See
  `dm1_v1_fmtowns_jdm_font.{h,c}` for the source-locked resolution
  contract: ASCII via the shared rasteriser, Shift-JIS pairs via a
  caller-supplied TBIOS callback (fail-closed if none).
- **Atari ST and Amiga variants**. Out of scope per user directive.
