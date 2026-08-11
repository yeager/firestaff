# FM Towns cross-game inventory

Snapshot of every FM Towns module currently shipped in Firestaff,
grouped by the game it belongs to plus the shared infrastructure.
Use this to find prior art before writing a new module — several
patterns are already implemented for one game and directly reusable
for another.

Last refreshed 2026-08-11 with 93+ session commits. For the
authoritative byte-verified cross-game coverage matrix, see
[`CROSS_GAME_COVERAGE.md`](CROSS_GAME_COVERAGE.md). This file
provides a per-module inventory grouped by game.

## Save-media audit (2026-08-10)

The local FM Towns corpus was checked without modifying any source image:

- CSB FM Towns archives expose the original CD image only; no additional
  `CSBGAME.DAT`/`CSBGAME.BAK` member was found. The available
  `CSBGAME.DAT` and `CSBGAME-JP.DAT` are retained as external, unclassified
  candidates. The current F7061/F7057 reader rejects both before a valid
  native save-tail receipt is produced; neither is positive Towns save
  evidence.
- DM2 FM Towns archives expose the original Victor CD image only; no
  `SKSAVE*` member or Towns save disk was found. The `SKSAVE0..3` files in the
  DOS archive are a different platform and are not used as substitutes.
- An external DOSBox capture is available under the user's `Downloads/dm2`
  directory: four primary and four backup `sksave0..3` files, each with the
  authentic 42-byte DOS SKSAVE envelope. The read-only corpus test admits all
  eight files for DOS raw-prefix/state inspection (`269` checks pass), but
  this evidence does not close the FM Towns save/resume gate. Its companion
  `graphics.dat` is the PC-English asset (`25247ede4dabb6a71e5dabdfbcd5907d`),
  not the FM Towns `GRAPHICS.DAT` (`027ff3b8ddc2c4c4cdda7ada0b0bc46c`).
- `Downloads/DMSAVE.DAT` is not a CSB or DM2 FM Towns save candidate and was
  not admitted into either corpus.

These absences keep native save/resume gates fail-closed; they are not
permission to generate replacement saves.

## DM1 (19 headers, most complete)

Menu-rendering stack (source-locked to EDM.EXP + JDM.EXP):

| Module | Purpose |
|---|---|
| `dm1_v1_fmtowns_menu_regions` | Byte-verified panel size/anchor rectangles (regions 10, 11) |
| `dm1_v1_fmtowns_dynamenu` | 8-byte DYNAMENU record layout, panel-colour sentinels |
| `dm1_v1_fmtowns_dyna_buttons` | English DYNA_BUTTONS label pool |
| `dm1_v1_fmtowns_dyna_buttons_ja` | Japanese Shift-JIS DYNA_BUTTONS-equivalent pool |
| `dm1_v1_fmtowns_text_geometry` | CHAR_X_SIZE=5, CHAR_Y_SIZE=6, CHAR_X_WID=6 constants |
| `dm1_v1_fmtowns_egb_shim` | Bounded TownsOS EGB primitive shim (rect fill) |
| `dm1_v1_fmtowns_font_asset` | Byte-fingerprint identity for the 768-byte menu font raster |
| `dm1_v1_fmtowns_font_rasteriser` | Round-trip-verified 6×128 raster → framebuffer blitter |
| `dm1_v1_fmtowns_menu_render` | Consumer that binds all of the above into an M11 paint |

Asset/disc infrastructure:

| Module | Purpose |
|---|---|
| `dm1_v1_fmtowns_graphics_dat` | GRAPHICS.DAT admission gate |
| `dm1_v1_fmtowns_dungeon_dat` | DUNGEON.DAT classifier |
| `dm1_v1_fmtowns_pic_library` | GRAPHICS.DAT record-table parser |
| `dm1_v1_fmtowns_pic_library_loader` | File-backed loader for the picture library |
| `dm1_v1_fmtowns_iso9660` | ISO 9660 data-track file extractor |
| `dm1_v1_fmtowns_cd_audio` | CD audio track mapping |

Runtime/startup:

| Module | Purpose |
|---|---|
| `dm1_v1_fmtowns_startup` | Original FM Towns launcher boundary receipt |
| `dm1_v1_fmtowns_title` | HMA-240 EDM.EXP DO_TITLE_ANIMATION 320×200 source |
| `dm1_v1_fmtowns_jdm_symbols` | JDM.EXP symbol vaddr map (byte-fingerprint recovered) |
| `dm1_v1_fmtowns_jdm_bss` | JDM.EXP BSS scalar vaddr map |

## DM2 (6 headers)

| Module | Purpose |
|---|---|
| `dm2_v1_fmtowns_disc` | Victor HME-242 disc image extractor |
| `dm2_v1_fmtowns_graphics_dat` | GRAPHICS.DAT admission gate |
| `dm2_v1_fmtowns_cd_dat` | CD.DAT parser |
| `dm2_v1_fmtowns_cdda_music` | HMP → CDDA track mapping |
| `dm2_v1_fmtowns_music_lookup` | End-to-end dungeon-map → CDDA track lookup |
| `dm2_v1_fmtowns_anim_stream` | Read-only TWANIM stream admission |

## CSB (6 headers)

| Module | Purpose |
|---|---|
| `csb_v1_fmtowns_cd` | CD image parser |
| `csb_v1_fmtowns_graphics_dat` | GRAPHICS.DAT classifier + item decoder |
| `csb_v1_fmtowns_anm` | ANM animation format parser |
| `csb_v1_fmtowns_portrait` | Champion portrait .CMP decoder |
| `csb_v1_fmtowns_game` | C06 CEDT006.C loop, authenticated CMP catalogue, source-owned portrait load, and portrait save |
| `csb_v1_fmtowns_switch` | SWITCHTW.EXP switch-menu resource identity |

## Shared (3 headers)

| Module | Purpose |
|---|---|
| `firestaff_fmtowns_disc` | Shared disc image reader for DM1 and DM2 |
| `firestaff_fmtowns_cd_audio_track_receipt` | Cross-game CD audio track receipt |
| `firestaff_fmtowns_cd_classify` | CD image classification |

## Cross-game reuse opportunities

Prior art in one game that DM1/DM2/CSB should audit before writing
fresh code:

1. **`dm2_v1_fmtowns_music_lookup`** (dungeon-map → CDDA track) — DM1
   has `dm1_v1_fmtowns_cd_audio` which does the same mapping. Compare
   patterns to keep semantics aligned; DM2's per-map dispatcher may
   be a cleaner shape.

2. **`csb_v1_fmtowns_switch`** (SWITCHTW.EXP menu resource identity) —
   mirrors the pattern DM1's `dm1_v1_fmtowns_jdm_symbols` uses for
   JDM.EXP. If SWITCHTW.EXP menu render lands in CSB, it can reuse
   DM1's menu-render harness verbatim (region-lookup + DYNA_BUTTONS
   pool + font rasteriser are all game-agnostic).

3. **`csb_v1_fmtowns_portrait`** (.CMP decoder) — DM1 also renders
   champion portraits from GRAPHICS.DAT but currently through the
   generic asset loader, not a CMP-specific decoder. If DM1 ever
   needs face-plate parity, CSB's CMP decoder is the reference.

4. **`dm2_v1_fmtowns_anim_stream`** (TWANIM) — DM1's title animation
   uses HMA-240 (also documented in `dm1_v1_fmtowns_title`). Both
   are TownsOS bespoke animation formats; DM2's stream admission
   pattern is worth mirroring for DM1 if title frame streaming
   graduates from single-frame receipt to full playback.

5. **Shared `firestaff_fmtowns_disc`** — already covers DM1 + DM2.
   CSB should migrate to it if not already; CSB's `csb_v1_fmtowns_cd`
   should be audited against the shared reader for behaviour parity.

6. **DM1's `dm1_v1_fmtowns_font_rasteriser`** — the 6×128 raster
   layout (right-aligned 5-bit glyph, MSB first) is documented and
   round-trip verified. If DM2/CSB ship additional bitmap-font
   consumers, they can reuse the rasteriser directly.

## What DM1 still lacks vs peers

- **CMP portrait decoder** (CSB has it, DM1 doesn't).
- **Dedicated animation stream parser** (DM2 has TWANIM admission,
  DM1's title is a single 320×200 frame receipt).

## What DM2/CSB lack vs DM1

- **Font rasteriser + menu render harness** — DM1 is the only game
  that has decoded the byte-verified font raster and shipped a
  region → panel → label paint pipeline. DM2 and CSB menu draws
  still fail closed pending their own equivalent.
- **JDM.EXP-equivalent symbol maps** — DM1 has byte-fingerprint
  recovered vaddr maps for both text-segment symbols and BSS
  scalars. DM2 (JDM2.EXP?) and CSB (JCSB.EXP?) do not.
