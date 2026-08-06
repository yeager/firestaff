# DM1 FM Towns — technical guide for Firestaff agents

This page is the single entry point any agent should read before
touching DM1 FM Towns code. It consolidates the disc image format, the
executable structure, symbol coordinates recovered by disassembly,
CDDA layout, currently wired features and the concrete extraction
workflow. All addresses come from the hash-verified HMA-240 English
disc; the Japanese disc uses the same layout but a different
executable (`JDM.EXP`) whose structural map is recorded separately —
see the JDM section below.

### Companion parity-evidence files (deep-decode references)

- [`dm1_fmtowns_menu_p3_disassembly.md`](../../parity-evidence/dm1_fmtowns_menu_p3_disassembly.md) — DRAW_DMENU, DRAW_ICN_BUTTON,
  GET_LABEL, MOUSE_OFF/ON, SPC_BLOT, FILL_CSCREEN + TownsOS EGB
  primitive table.
- [`dm1_fmtowns_region_table.md`](../../parity-evidence/dm1_fmtowns_region_table.md) — GET_SCL_COORD / GET_RGN_COORD /
  GET_COORD, the 23-block region registry at `[0x28f08]`, byte-exact
  region records including regions 10 and 11.
- [`dm1_fmtowns_text_rasteriser.md`](../../parity-evidence/dm1_fmtowns_text_rasteriser.md) — DO_DRAW_CTEXT subtree,
  TEXT_SIZE font base, ASCII→glyph mapping, colourisation.
- [`dm1_fmtowns_jdm_structural_map.md`](../../parity-evidence/dm1_fmtowns_jdm_structural_map.md) — Japanese `JDM.EXP` P3
  header, Shift-JIS label pool, initial-EIP fingerprint match with
  EDM, EGB-trampoline recovery plan.
- [`dm1_fmtowns_jdm_symbol_recovery.md`](../../parity-evidence/dm1_fmtowns_jdm_symbol_recovery.md) — 19 JDM.EXP symbols
  recovered by masked byte-fingerprint (10 code + 8 EGB
  trampolines + 1 data pool); per-.OBJ non-uniform shifts.
- [`dm1_fmtowns_jdm_bss_triangulation.md`](../../parity-evidence/dm1_fmtowns_jdm_bss_triangulation.md) — 18 BSS scalars
  recovered by XREF triangulation + neighbor-delta derivation.
- [`dm1_fmtowns_font_asset.md`](../../parity-evidence/dm1_fmtowns_font_asset.md) — INIT_TEXT + GET_MY_DECODED
  decode; menu font = picture-library index 557, direct-to-buffer
  + no-header bits, 768-byte allocation.
- [`dm1_fmtowns_pic_library_format.md`](../../parity-evidence/dm1_fmtowns_pic_library_format.md) — `DATA/GRAPHICS.DAT`
  container (575 assets, 2 + 575×4-byte header + payload) plus
  DECODEGRAPHIC RLE decoder with byte-verified leaf-helper decodes
  and round-trip vectors.

## 1. Retail media

- Archive: `~/.firestaff/data/dm1/Dungeon Master (Japan) (En,Ja) (Rev
  1).7z` (or the same file without `(Rev 1)` for the earlier press).
  Both English and Japanese runtimes ship on the same disc.
- Layout: **21-file BIN + one CUE**. Track 01 is the data track,
  Tracks 02..20 are CDDA audio. The CUE names the multi-file layout;
  `firestaff_fmtowns_disc.c` parses the CUE and computes byte offsets.
- Data track: MODE1/2048 ISO 9660 volume `DUNGEON` (5,056,800 bytes).
- Audio tracks: raw 44.1 kHz signed-LE stereo, 2352-byte sectors.
- **Do not** assume a uniform sector size across the disc — the audio
  offset computation must add `data_track_end * 2048` for the data
  region and then `(audio_start - data_track_end) * 2352` for the
  audio region. This is why the CSB CDDA parser (uniform 2352) cannot
  be reused for DM1 and why `fmtowns_cue_parse_track_starts()` is used
  instead. See `test_dm1_v1_fmtowns_cd_audio.c` for the exact math.

### Track 01 file listing (ISO9660, `/DUNGEON/`)

| File            | Purpose                                             |
|-----------------|-----------------------------------------------------|
| `AUTOEXEC.BAT`  | Launches `\\CONTROL.EXE` → TownsOS shell            |
| `CONFIG.SYS`    | DOS/TownsOS config                                  |
| `CONTROL.EXE`   | TownsOS shell (owns `TMENU`)                        |
| `RUN386.EXE`    | Phar Lap DOS-extender                               |
| `TBIOS.SYS` / `TBIOS.BIN` | FM Towns TownsOS graphics library         |
| `IO.SYS`        | TownsOS I/O                                         |
| `DICUTY.COM`    | TownsOS dictionary utility                          |
| `OAK2USR.DIC` / `T_OAK2.EXE` | Japanese input method                  |
| `DRIVE_R.IMG`   | RAM disk image                                      |
| `TMENU.EXP` / `TMENU.ICN` / `TMENU.INF` | TownsOS file-browser launcher (owns the language-select menu, not the in-game menu) |
| `EDM.EXP`       | English game executable (Phar Lap P3, 310518 B)     |
| `JDM.EXP`       | Japanese game executable                            |
| `DATA/`         | English game data (`GRAPHICS.DAT`, `DUNGEON.DAT`, …) |
| `JDATA/`        | Japanese game data                                  |

The **hash-admitted pair** for the English runtime is:

- `GRAPHICS.DAT` MD5 same as the retained catalog entry — the fingerprint
  registry in `include/firestaff_game_data_fingerprint.h` covers both
  English and Japanese variants; only that registry admits a runtime.
- `DUNGEON.DAT` for FM Towns has its own two fingerprints (EN
  `3DC0A932…`, JP `FE098F70…`) — indices 113/114 (`FINGERPRINT_COUNT`
  = 115 in the header).

Materialization saves `FMTOWNS.BIN` and `FMTOWNS.CUE` into the
runtime cache directory (`m12_materialize_dm1_fmtowns_runtime_cache`
in `src/shared/asset_status_m12.c`) so the M11 CDDA dispatcher can
read raw audio bytes without re-opening the 7z at runtime.

## 2. `EDM.EXP` — Phar Lap 386 (P3) executable

Level-1 P3 image, 310518 bytes. All values below are from the actual
header via `dm1_v1_fmtowns_startup.c:validate_p3_header`.

| Field                 | Header offset | Value         |
|-----------------------|---------------|---------------|
| Magic                 | 0x00          | `"P3"`        |
| Level                 | 0x02          | 1             |
| Header size           | 0x04          | 0x180         |
| File size             | 0x06          | 0x46b4f       |
| Runtime params off/sz | 0x0c / 0x10   | 0x180 / 0x80  |
| Relocations off/sz    | 0x14 / 0x18   | 0x200 / 0     |
| Load image off/sz     | 0x26 / 0x2a   | 0x200 / 0x46941 |
| Symbol table off/sz   | 0x2e / 0x32   | 0x46b41 / 0x51b5 |
| Initial EIP           | 0x68          | 0x42a48       |
| Memory requirements   | 0x74          | 0x77684       |

**File offset of a runtime virtual address** =
`load_image_offset (0x200) + vaddr`. Every symbol value in the SYM1
table is a load-image virtual address.

### SYM1 symbol table (0x46b41, 0x51b5 bytes, 1174 entries)

- 4-byte magic `"SYM1"` at offset 0.
- Entry count word at file+0x1e (relative to SYM1 base).
- Records begin at cursor 0x22. Each record: `{u8 name_size, char
  name[name_size], u32 value, u16 flags}` — 6 bytes of tail past the
  name.
- Names are ASCII, up to 127 bytes, non-empty.

The receipt at `include/dm1_v1_fmtowns_startup.h` already exposes the
following recovered addresses on the English disc:

| Symbol             | Vaddr    | Kind |
|--------------------|----------|------|
| DO_TITLE_ANIMATION | 0xc3b0   | fn   |
| TITLE_PRESENTS     | 0x28f4a  | data |
| TITLE_DUNGEON      | 0x28f4c  | data |
| DRAW_DMENU         | 0x4620   | fn   |
| DYNAMENU           | 0x2418c  | data (8 B) |
| MENU_ICONS         | 0x2415c  | data (word) |
| CD_LEVEL_SONG      | 0x211d8  | fn   |

## 3. Menu drawing chain — decoded

Full disassembly with symbol resolution is in
[`parity-evidence/dm1_fmtowns_menu_p3_disassembly.md`](../../parity-evidence/dm1_fmtowns_menu_p3_disassembly.md).
Highlights so agents don't need to re-lift them:

- `DRAW_DMENU` (0x4620, 240 B) — mode-gates on MENU_ICONS-vs-MENU_OWNER.
  Icon mode loops `PARTY_SIZE` times through `DRAW_ICN_BUTTON` (0x44f0).
  Dynamic mode draws a coloured panel via `SPC_BLOT` (0x1ccec), a main
  label from a stride-319 table at `0x26019 + MENU_OWNER * 319`, and
  three button labels looked up through `GET_LABEL` (0x43e4).
- `GET_LABEL` walks `DYNA_BUTTONS` (0x24194) as a NUL-separated
  string table; `0xFF` sentinel returns `0x21d9c` (blank label).
- `DYNAMENU[+2]` / `DYNAMENU[+3]` are `0xFF`-flagged colour overrides
  for the panel colour (default `0x0B`, alternates `0x4D` / `0x4F`).
- `MOUSE_OFF` (0xdd38) / `MOUSE_ON` (0xdd18) are `cli`-guarded
  reference-counted wrappers around `MOS_DISP` (0x21a40) using
  `MSE_STATE` (0x25848) as the hide depth.

### DYNA_BUTTONS label pool (first 96 bytes, verified)

```
"N", "BLOCK", "CHOP", "X", "BLOW HORN", "FLIP", "PUNCH", "KICK",
"WAR CRY", "STAB", "CLIMB DOWN", "FREEZE LIFE", "HIT", "SWING",
"STAB", "THRU", …
```

These are the FM Towns action-menu button labels. Index 0 is the
`"N"` placeholder glyph; every subsequent index is a real verb.

## 3a. Region table — locked coordinates for menu geometry

`GET_RGN_COORD` (0x194fc) is a 1:1-scale tail-call into
`GET_SCL_COORD` (0x1942c), which walks a linked list of region blocks
whose head pointer is stored at `[0x28f08]` (statically initialised
to `0x28e78`). The lookup routine at `0x18db4` iterates blocks
matching `first_id ≤ id ≤ last_id` and returns the record at
`block+8 + (id-first_id)*8`.

Each record is 8 bytes: `type u16 | parent u16 | a i16 | b i16`.

- `type == 9`: `(a, b)` are `(width, height)` in pixels. The record
  is a **size** node.
- Other types: `(a, b)` are anchor coordinates whose meaning is
  selected by `GET_COORD` (0x18df0) jump tables at `0x18f14` /
  `0x18fe6`.
- `parent` walks up the tree; `GET_SCL_COORD` requires the parent
  to be a `type == 9` size node and returns
  `parent_size * scale / 10000` (scale defaults to 10000, i.e. 1:1
  when called via GET_RGN_COORD).

Byte-verified regions that back the current menu draw:

| Region | Record                             | Meaning                                     |
|--------|------------------------------------|---------------------------------------------|
| 10     | `type=9  parent=2  size=(87, 45)`  | SPC_BLOT dynamic-menu panel size            |
| 11     | `type=2  parent=10  anchor=(319,77)` | Menu clear-area anchor (inherits 87×45)  |

The full 23-block registry (994 records) is enumerated byte-exact in
[`parity-evidence/dm1_fmtowns_region_table.md`](../../parity-evidence/dm1_fmtowns_region_table.md);
consult it before adding any new region binding.

## 4. TownsOS EGB primitives — locked coordinates

`FILL_RECT` (0x1fccc) and `PIX_BLOT` (0x1fe7c) route into the FM Towns
TownsOS **EGB** graphics library. All calls take a persistent
`WORK` block at `0x318d8` and stage a 4-word rect through
`EGBPARA` at `0x360d8`. The runtime destination page is
`WRITE_PAGE` at `0x36170`.

| EGB primitive       | Vaddr    | Purpose                          |
|---------------------|----------|----------------------------------|
| EGB_RESOLUTIONRAM   | 0x40739  | Retarget EGB at a RAM raster     |
| EGB_VIEWPORT        | 0x407a0  | Clip rectangle                   |
| EGB_WRITEPAGE       | 0x407ec  | Select destination VRAM page     |
| EGB_COLOR           | 0x40836  | Set foreground/background colour |
| EGB_WRITEMODE       | 0x408a5  | 0 = plain copy, 6 = masked       |
| EGB_PAINTMODE       | 0x408ed  | 0x20 = solid fill                |
| EGB_PUTBLOCK        | 0x40bec  | Copy source raster to viewport   |
| EGB_RECTANGLE       | 0x40ee5  | Filled/outlined rectangle        |

Because these are documented TownsOS calls with published semantics,
the menu draw does not require reverse-engineering custom pixel code.
The bounded implementation task is a software EGB shim over the M11
framebuffer.

## 4a. Text rasteriser — DO_DRAW_CTEXT subtree

The menu draws all labels through the following call chain:

```
DO_FDRAW_CTEXT (0x1a8c0)   pad string to column count, NUL-terminate
  → DO_DRAW_CTEXT (0x1a804)  measure + place + rasterise
      → text_measure  (0x1a710)   glyph width sum via CHAR_X_SPC
      → text_place    (0x18df0)   same GET_COORD used by regions
      → DO_DRAW_TEXT  (0x1a664)   per-glyph loop
          → text_colourise (0x1a5a4)  1bpp → 4bpp packed
          → PIX_BLOT       (0x1fe7c)  EGB_PUTBLOCKCOLOR blit
```

**Font base.** Symbol `TEXT_SIZE` at vaddr `0x29344` is a dword
pointer to a 768-byte 1bpp source raster, populated at `INIT_TEXT`.
A pre-baked fg/bg-resolved 4bpp copy is kept at `[0x293e4]` (3072
bytes, EGB source stride `0x400`).

**Glyph geometry.**

| Quantity          | Symbol / value                          |
|-------------------|-----------------------------------------|
| Source pitch      | 8 pixels / char                         |
| Drawn width       | `CHAR_X_SPC`                            |
| Drawn height      | `CHAR_Y_SPC - CHAR_X_SIZE + 1`          |
| Per-char advance  | `CHAR_Y_HYT`                            |
| Static default    | 5×6 cell, 6-pixel advance               |
| Runtime source    | `TEXT_PIC` at `0x2934c`                 |

**ASCII → glyph mapping.** Direct: `source_x = char_byte * 8`. No
`-0x20` offset, no lookup table. Colour handling is done entirely
in the pre-blit `text_colourise` (1bpp source → 4bpp packed:
`bit7 → low nibble`, `bit6 → high nibble`, each nibble selects fg
or bg); the blit itself is `PIX_BLOT` with `colour = -1`, i.e. a
plain `EGB_PUTBLOCKCOLOR` copy.

The only piece not recoverable from the executable is the physical
2-D shape of the raster and the actual glyph bitmaps — those are
loaded at INIT_TEXT and remain a bounded next task.

## 5. CDDA layout — 19 audio tracks (2..20)

Runtime mapping is in
[`include/dm1_v1_fmtowns_cd_audio.h`](../../include/dm1_v1_fmtowns_cd_audio.h)
and `src/dm1/dm1_v1_fmtowns_cd_audio.c`.

| Track | Symbol / Role                                      |
|-------|----------------------------------------------------|
| 02    | `DM1_FMTOWNS_TRACK_TITLE` (title screen)           |
| 03    | `DM1_FMTOWNS_TRACK_HALL` (Hall of Champions)       |
| 04    | unused                                             |
| 05    | `DM1_FMTOWNS_TRACK_ENTRANCE_MAP6` (entrance + map 6) |
| 06–17 | per-map dungeon tracks (see `map_to_track[]`)      |
| 13    | `DM1_FMTOWNS_TRACK_GAME_OVER`                      |
| 18    | `DM1_FMTOWNS_TRACK_GAME_WON`                       |
| 19–20 | reserved / silence                                 |

`dm1_v1_fmtowns_cd_track_for_map(map_index)` maps dungeon map indices
to tracks; `dm1_v1_fmtowns_cd_track_for_event(evt)` maps title/HoC/
game-over/game-won events to tracks.

### Playback wiring (all live in `src/engine/main_loop_m11.c` and
`src/engine/m11_game_view.c`, gated on
`dm1FmtownsStartupReceiptValid`)

- **Title (track 2)** — `m11_open_requested_launch` calls
  `M11_GameView_PlayFmtownsCdda` before the FM Towns title animation.
- **HoC (track 3)** — `m11_publish_dm1_hoc_presented_capture_to_m12`
  dispatches when the HoC presented-capture receipt first goes ready.
- **Entrance (track 5)** — `m11_play_redmcsb_entrance_transition`
  dispatches before the door-open sequence, guarded against
  re-trigger via `dm1FmtownsCddaCurrentTrack`.
- **Map transitions** — stair and teleporter transitions invoke
  `dm1_v1_fmtowns_cd_track_for_map(toMapIndex)`.
- **Per-tick idle** — the music loop starts the current map's track
  when nothing is playing.
- **Game events** — game-over and game-won dispatch via
  `dm1_v1_fmtowns_cd_track_for_event(2)` / `(3)`.
- **Music toggle** — `m11_dm1_stop_fmtowns_cdda` when music is
  disabled.

Playback reads raw PCM bytes from the retained `FMTOWNS.BIN` at the
mixed-sector-computed byte offset and hands them to
`M11_Audio_PlayCdda` (16-bit signed LE stereo 44100 Hz).

## 6. Startup receipt — what is authenticated today

`dm1_v1_fmtowns_startup_receipt` validates before any FM Towns runtime
opens:

- Exact MD5s of `AUTOEXEC.BAT`, selected `EDM/JDM`, `TMENU.EXP`,
  `TMENU.ICN`, `TMENU.INF`.
- Menu-program symbol references to TMENU.INF / TMENU.ICN / TMENU.EXP
  (`menu_program_symbols_verified`).
- Game-program symbol references to `DO_TITLE_ANIMATION`,
  `TITLE_PRESENTS`, `TITLE_DUNGEON`, `DRAW_DMENU`, `DYNAMENU`,
  `MENU_ICONS`, `CD_LEVEL_SONG`
  (`game_program_symbols_verified`).
- Bounded Phar Lap P3 header for both menu and game
  (`menu_p3_header_verified`, `game_p3_header_verified`).
- English `EDM.EXP`: full SYM1 parse (1174 entries) with recorded
  addresses for the seven symbols listed above; the title-animation
  plan (GRAPHICS.DAT graphic index 1, PRESENTS/MASTER/zoom rects)
  bound from the P3 load image at 0xc3d1..0xc726.
- `TMENU.INF`: two 128-byte records selecting `\\JDM.EXP` and
  `\\EDM.EXP`.
- CD title/hall/entrance track ownership recorded on the receipt.

Japanese `JDM.EXP` is admitted at the header level but has no SYM1
table; its title animation and menu remain source-boundary-only.
See section 6a for the structural map that unblocks JDM recovery.

## 6a. `JDM.EXP` (Japanese runtime) — structural map

`JDM.EXP` is 290,221 bytes; SHA-256
`1db4f049…de0bae`; initial EIP `0x00042cb4`; load image at `0x200`
(size `0x46bad`); memory requirement `0x778f0`. **No SYM1 table**:
both header fields are 0 and the literal `"SYM1"` does not appear in
the file. No English identifier strings (`DRAW_DMENU`, `DYNAMENU`,
`EGB_`, …) appear either.

**Same source, Japanese resources.** The initial-EIP disassembly
matches EDM.EXP byte-for-byte for 62 of 64 bytes — differing only in
one call displacement (the Metaware / Phar Lap High-C run-time
stub). JDM is therefore the same source rebuilt with Japanese
resources rather than an independent build.

**Japanese label pool.** Located at vaddr `0x243bc` (`EDM.DYNA_BUTTONS
+ 0x228`). All 15 verb slots decode from Shift-JIS with a 1:1
mapping to the English pool:

| EDM label   | JDM (Shift-JIS)   |
|-------------|-------------------|
| BLOCK       | さえぎる          |
| CHOP        | 叩き切る          |
| BLOW HORN   | (recorded)        |
| WAR CRY     | ときの声          |
| …           | full list in evidence |

Larger dialog pool starts at `0x22000`; asset paths in the
executable use `Q:\JDATA\...` (versus EDM's `\DATA\`).

**EGB library still linked.** The trampoline signature
`0f a0 68 10 01 00` appears at multiple JDM call sites; only the
`EGB_*` symbol names are lost. Recovering them is a structural
byte-fingerprint pass against EDM.

**Recovery plan (recorded).** Structural byte-fingerprint match
against EDM.EXP → data-segment reference translation via string-
anchored deltas → EGB-trampoline pattern enumeration → bind the
Japanese label pool and `JDATA/` path fixup. Full recipe in
[`parity-evidence/dm1_fmtowns_jdm_structural_map.md`](../../parity-evidence/dm1_fmtowns_jdm_structural_map.md).

### JDM.EXP recovered symbol map

Two follow-up passes ship JDM-side vaddrs so a JDM-selected runtime
can reach the same features EDM has:

- **19 code + EGB-trampoline vaddrs** recovered by masked byte-
  fingerprint match (evidence:
  [`dm1_fmtowns_jdm_symbol_recovery.md`](../../parity-evidence/dm1_fmtowns_jdm_symbol_recovery.md)). All 8 EGB trampolines
  share a uniform `+0x26c` shift; menu / drawing / text code
  symbols shift per linked .OBJ (`+0x78`, `+0xb4`, `+0xc0`,
  `+0x1f0`, `+0x2c8` observed). Encoded in
  `include/dm1_v1_fmtowns_jdm_symbols.h`; lookup via
  `dm1_v1_fmtowns_jdm_symbol_vaddr_pc34("NAME")`.
- **18 BSS-scalar vaddrs** recovered by XREF triangulation and
  contiguous-block neighbor-delta derivation (evidence:
  [`dm1_fmtowns_jdm_bss_triangulation.md`](../../parity-evidence/dm1_fmtowns_jdm_bss_triangulation.md)). Block shifts:
  menu-owner `+0x228`, screen/icon `+0x264`, character-metrics
  `+0x276`, party state `+0x26c` — non-uniform confirms the
  same-source-different-link finding. Encoded in
  `include/dm1_v1_fmtowns_jdm_bss.h`; lookup via
  `dm1_v1_fmtowns_jdm_bss_vaddr_pc34("NAME")`.

## 6b. Font asset identity (INIT_TEXT / GET_MY_DECODED)

`INIT_TEXT` (EDM.EXP 0x1ae54) allocates a 768-byte buffer and calls
`GET_MY_DECODED(0xffffc22d, buf, 0, 0)`. Disassembling
`GET_MY_DECODED` (EDM.EXP 0x9f04) reveals the encoded id:

```
0xc22d  = 0x8000 (direct-to-buffer) | 0x4000 (skip size header)
                | 0x022d (picture-library index 557)
```

So the FM Towns DM1 menu font is **picture-library asset 557**,
decoded in *direct-to-caller-buffer, no-header* mode. The
GET_MY_DECODED grammar (bit 15 = direct, bit 14 = no-header,
bits 13..0 = index) is encoded in
`include/dm1_v1_fmtowns_font_asset.h`. Full evidence in
[`parity-evidence/dm1_fmtowns_font_asset.md`](../../parity-evidence/dm1_fmtowns_font_asset.md).

## 6c. Picture-library container — `DATA/GRAPHICS.DAT`

The FM Towns DM1 picture library is `DATA/GRAPHICS.DAT` on the
extracted disc. **This is a distinct file format from the DM1 PC 3.4
`GRAPHICS.DAT` LZW container** — different asset count, different
compression, different header. Do not reuse the PC34 loader here.

- **Total size**: 396,970 bytes
- **Header**: `u16 asset_count` followed by `asset_count × u32
  size` size table. Byte-verified `asset_count = 575`, so the
  header is `2 + 575 × 4 = 2,302` bytes.
- **Payload**: 394,668 bytes = Σ `size_table[i]` for i = 0..574
  (byte-exact match, no gaps or padding).
- **Asset span** for index N begins at `2 + 575*4 + Σ
  size_table[0..N-1]` and runs for `size_table[N]` bytes.

Encoded in `include/dm1_v1_fmtowns_pic_library.h`. The API returns
zero-copy views into a caller-owned buffer that already contains
`GRAPHICS.DAT`; no I/O is performed by the module itself. Full
evidence in
[`parity-evidence/dm1_fmtowns_pic_library_format.md`](../../parity-evidence/dm1_fmtowns_pic_library_format.md).

The menu font at index 557 is exactly 768 bytes and stored
uncompressed — it takes the DIRECT+NO_HDR path that bypasses
DECODEGRAPHIC entirely (raw span memcpy'd into the caller buffer).
`load_raw_asset_pc34` implements this path.

## 6d. DECODEGRAPHIC RLE decoder — round-trip verified

`DECODEGRAPHIC` (EDM.EXP 0x1f63c) turns a compressed asset span
into a nibble-packed 4bpp pixel matrix. Ported to Firestaff with
byte-verified disassembly of all four leaf helpers: `0x1f4c4`
(put_pixel), `0x1f518` (raw stream copy), `0x1f578` (nibble fill),
`0x1f5d8` (row copy).

**Per-asset header** (first 4 bytes of every asset span):

```
+0  u16 width_pixels
+2  u16 height_pixels
```

**Row stride**: `padded_width = (width + 0x1f) & ~0x1f` (32-pixel
alignment), `row_bytes = padded_width / 2` (2 pixels per byte).
When `width == padded_width` a fast-copy branch at 0x1f85f emits
the on-disk bytes verbatim; otherwise the RLE loop runs.

**RLE control-byte grammar**:

| bit 7 | bit 6 | count encoding                     |
|-------|-------|------------------------------------|
| 0     | –     | `(ctrl >> 4) + 1` pixels literal   |
| 1     | 0     | `next-byte + 1`                    |
| 1     | 1     | `next-two-bytes-BE + 1`            |

Bits 5..4 pick the mode: `00` raw stream copy (0x1f734), `01`
single-pixel fill using `ctrl & 0x0f` (0x1f775), `11` row-copy
with pixel nibble from stream (0x1f7f3).

**Round-trip verification** against the real
`DATA/GRAPHICS.DAT`: 347 of 347 RLE-branch assets round-trip
byte-exact — decoder consumes exactly `size_table[index]` source
bytes and emits exactly `padded_width/2 × height` destination
bytes for every asset. Two representative vectors in the test:

- Asset 6: 80 × 14 pixels, consumes 185 source bytes
- Asset 25: 144 × 73 pixels, consumes 565 source bytes

**API**: `dm1_v1_fmtowns_pic_library_decode_asset_pc34()` returns
the 4bpp matrix and its `(padded_width, height)` dimensions.
Test `test_real_graphics_dat_rle_roundtrip` is gated on the
`FIRESTAFF_DM1_FMTOWNS_GRAPHICS_DAT` env var and skips silently
when the file is absent — no game bytes are bundled.

## 7. What is wired, what is open

**Wired (real-data only, gated on the receipt):**

- Runtime cache materialization from the retained 7z including
  `FMTOWNS.BIN` + `FMTOWNS.CUE` retention.
- CDDA end-to-end: title, HoC, entrance, all 16 map transitions,
  ticks, events, music toggle.
- FM Towns title animation: 18-frame reverse zoom from
  `GRAPHICS.DAT` graphic 1 via `dm1_v1_fmtowns_title` compositor,
  driven by the receipt's title-plan geometry.
- Startup routes selected FM Towns editions around the PC34
  `SWSH → TITLE → ENTRANCE` transaction — no PC34 presentation
  fallback.

**Decoded and shipping as source-locked C** (available today —
just call the API, no re-lifting from the executable required):

| Layer                       | Module                                  |
|-----------------------------|-----------------------------------------|
| Region geometry             | `dm1_v1_fmtowns_menu_regions`           |
| DYNAMENU 8-byte record      | `dm1_v1_fmtowns_dynamenu`               |
| English 44-label pool       | `dm1_v1_fmtowns_dyna_buttons`           |
| Japanese 44-label pool      | `dm1_v1_fmtowns_dyna_buttons_ja`        |
| Text/screen/icon geometry   | `dm1_v1_fmtowns_text_geometry`          |
| Software EGB shim           | `dm1_v1_fmtowns_egb_shim` (fill / put)  |
| JDM code + EGB trampolines  | `dm1_v1_fmtowns_jdm_symbols` (19 syms)  |
| JDM BSS scalars             | `dm1_v1_fmtowns_jdm_bss` (18 scalars)   |
| Font asset identity         | `dm1_v1_fmtowns_font_asset`             |
| Picture library container   | `dm1_v1_fmtowns_pic_library` (575 ids)  |
| DECODEGRAPHIC RLE decoder   | `dm1_v1_fmtowns_pic_library` (347/347)  |

**What remains for the visible menu**: bind the ten modules above
into the M11 main loop. The bounded consumer skeleton is:

```c
/* Look up the region rectangle */
DM1_V1_FmtownsRegionRecord panel, anchor;
dm1_v1_fmtowns_region_menu_panel_pc34(&panel);
dm1_v1_fmtowns_region_menu_clear_area_pc34(&anchor);

/* Compose the panel-colour and slot indices from live state */
uint8_t rec[DM1_V1_FMTOWNS_DYNAMENU_BYTES] = {...};
uint8_t col = dm1_v1_fmtowns_dynamenu_panel_colour_pc34(rec);

/* Fill the panel via the software EGB shim */
dm1_v1_fmtowns_egb_fill_rect_pc34(fb, W, H, W,
    anchor.a - panel.a, anchor.b,
    anchor.a - 1,       anchor.b + panel.b - 1, col);

/* Draw three labels */
for (int i = 0; i < 3; ++i) {
    uint8_t ix = dm1_v1_fmtowns_dynamenu_slot_label_pc34(rec, i);
    const char *s = language == JP
        ? dm1_v1_fmtowns_dyna_button_label_ja_pc34(ix)
        : dm1_v1_fmtowns_dyna_button_label_pc34(ix);
    /* rasterise s at (anchor.a - panel.a + PAD, anchor.b + 20*i)
     * using the font raster loaded from GRAPHICS.DAT index 557 */
}
```

**Remaining bounded next steps** (all non-synthetic):

- Bind the picture-library decoder into the M11 asset pipeline so
  it opens `~/.firestaff/data/dm1/...` (materialised cache) rather
  than requiring the caller to supply the GRAPHICS.DAT bytes.
- Load font index 557 at boot into a persistent M11 slot; wire
  the text rasteriser subtree (section 4a) to sample from it.
- Compose live `DYNAMENU` records from the current champion's
  action-hand state and drive the shim.
- `TMENU` (TownsOS shell) interactive icon/layout rendering and
  mouse routes — separate scope, above the game executable.

**Genuinely blocked (require external evidence):**

- No item in the menu-draw chain remains genuinely blocked. Every
  constant and format reachable via disassembly of the
  hash-verified `EDM.EXP` / `JDM.EXP` / `DATA/GRAPHICS.DAT` is
  now shipping source-locked C with tests.

## 8. Extracting data during development

**Firestaff at runtime never unpacks the 7z.** Development extraction
uses standard host tools. From the repo root:

```bash
mkdir -p ~/scratch/fmtowns
cd ~/scratch/fmtowns
7z x -y ~/.firestaff/data/dm1/'Dungeon Master (Japan) (En,Ja) (Rev 1).7z' '*Track 01*'
# The Track 01 BIN is a plain ISO9660 image because it's MODE1/2048.
# Mount or bsdtar it to reach the files:
bsdtar -xf 'Dungeon Master (Japan) (En,Ja) (Rev 1) (Track 01).bin' -C extracted/
ls extracted/         # AUTOEXEC.BAT CONFIG.SYS EDM.EXP JDM.EXP TMENU.* DATA/ JDATA/ …
```

To parse the P3 header and dump the SYM1 name table:

```python
import struct
p = open("extracted/EDM.EXP", "rb").read()
r32 = lambda o: struct.unpack("<I", p[o:o+4])[0]
sym_off, sym_sz = r32(0x2e), r32(0x32)
load_off = r32(0x26)                    # 0x200 for EDM.EXP
table, cursor, syms = p[sym_off:sym_off+sym_sz], 0x22, {}
while cursor < sym_sz:
    ns = table[cursor]; cursor += 1
    if not (0 < ns <= 127) or cursor + ns + 6 > sym_sz: break
    name = table[cursor:cursor+ns].decode("ascii", errors="replace")
    val  = int.from_bytes(table[cursor+ns:cursor+ns+4], "little")
    syms[name] = val
    cursor += ns + 6
# To pull code for a symbol, read `p[load_off + syms[name] : … ]`.
```

Disassembling a symbol range with `capstone`:

```python
import capstone
md = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
start_v = syms["DRAW_DMENU"]
end_v   = syms["FIZZLE_SPELL"]           # next symbol above DRAW_DMENU
for i in md.disasm(p[load_off+start_v : load_off+end_v], start_v):
    print(f"{i.address:#8x}: {i.mnemonic:<7} {i.op_str}")
```

## 9. Rules that apply to all FM Towns DM1 work

- **Never synthesise pixels** when real data exists. The menu is
  blocked on decoded EGB shim work, not on a placeholder.
- **Never extract data at runtime** — Firestaff must open the
  materialized cache only. All extraction is a development-time step.
- **Never bypass the startup receipt.** Every playback and rendering
  path must gate on `dm1FmtownsStartupReceiptValid`.
- **Never restore the PC34 startup as a fallback** when an FM Towns
  edition is selected.
- **Never use the CSB uniform-2352 CDDA parser** for DM1 — the disc
  is mixed-sector. Use `fmtowns_cue_parse_track_starts` and compute
  the byte offset by hand (see `test_dm1_v1_fmtowns_cd_audio.c`).

## 10. Files to know

| Purpose                    | Path                                            |
|----------------------------|-------------------------------------------------|
| Startup receipt            | `include/dm1_v1_fmtowns_startup.h`, `src/dm1/dm1_v1_fmtowns_startup.c` |
| CDDA track table + lookup  | `include/dm1_v1_fmtowns_cd_audio.h`, `src/dm1/dm1_v1_fmtowns_cd_audio.c` |
| BIN/CUE + disc I/O         | `include/firestaff_fmtowns_disc.h`, `src/shared/firestaff_fmtowns_disc.c` |
| ISO9660 walker             | `src/dm1/dm1_v1_fmtowns_iso9660.c`              |
| Title compositor           | `include/dm1_v1_fmtowns_title.h`, `src/dm1/dm1_v1_fmtowns_title.c` |
| Runtime cache materializer | `src/shared/asset_status_m12.c` (`m12_materialize_dm1_fmtowns_runtime_cache`) |
| Runtime wiring             | `src/engine/main_loop_m11.c`, `src/engine/m11_game_view.c` |
| Fingerprints               | `include/firestaff_game_data_fingerprint.h`, `src/shared/firestaff_game_data_fingerprint.c` |
| Menu disassembly evidence  | `parity-evidence/dm1_fmtowns_menu_p3_disassembly.md` |
| Region table evidence      | `parity-evidence/dm1_fmtowns_region_table.md`   |
| Text rasteriser evidence   | `parity-evidence/dm1_fmtowns_text_rasteriser.md` |
| JDM.EXP structural map     | `parity-evidence/dm1_fmtowns_jdm_structural_map.md` |
| JDM.EXP symbol recovery    | `parity-evidence/dm1_fmtowns_jdm_symbol_recovery.md` |
| JDM.EXP BSS triangulation  | `parity-evidence/dm1_fmtowns_jdm_bss_triangulation.md` |
| Font asset identity        | `parity-evidence/dm1_fmtowns_font_asset.md`     |
| Picture library + RLE      | `parity-evidence/dm1_fmtowns_pic_library_format.md` |
| Menu region constants      | `include/dm1_v1_fmtowns_menu_regions.h`, `src/dm1/dm1_v1_fmtowns_menu_regions.c` |
| DYNAMENU record helpers    | `include/dm1_v1_fmtowns_dynamenu.h`, `src/dm1/dm1_v1_fmtowns_dynamenu.c` |
| English label pool         | `include/dm1_v1_fmtowns_dyna_buttons.h`, `src/dm1/dm1_v1_fmtowns_dyna_buttons.c` |
| Japanese label pool        | `include/dm1_v1_fmtowns_dyna_buttons_ja.h`, `src/dm1/dm1_v1_fmtowns_dyna_buttons_ja.c` |
| Text/screen/icon geometry  | `include/dm1_v1_fmtowns_text_geometry.h`, `src/dm1/dm1_v1_fmtowns_text_geometry.c` |
| Software EGB shim          | `include/dm1_v1_fmtowns_egb_shim.h`, `src/dm1/dm1_v1_fmtowns_egb_shim.c` |
| JDM symbol vaddr map       | `include/dm1_v1_fmtowns_jdm_symbols.h`, `src/dm1/dm1_v1_fmtowns_jdm_symbols.c` |
| JDM BSS vaddr map          | `include/dm1_v1_fmtowns_jdm_bss.h`, `src/dm1/dm1_v1_fmtowns_jdm_bss.c` |
| Font asset identity        | `include/dm1_v1_fmtowns_font_asset.h`, `src/dm1/dm1_v1_fmtowns_font_asset.c` |
| Picture library + RLE      | `include/dm1_v1_fmtowns_pic_library.h`, `src/dm1/dm1_v1_fmtowns_pic_library.c` |
| Tests                      | `tests/test_dm1_v1_fmtowns_cd_audio.c`, `tests/test_dm1_v1_fmtowns_title.c`, `tests/test_firestaff_fmtowns_disc.c` |

## 11. Cross-references

- DM2 FM Towns status and layout (also disc-image based):
  see the DM2 FM Towns notes tracked by the DM2 lane in TODO.md.
- CSB FM Towns CDDA runtime: `src/csb/csb_v1_fmtowns_cdda_*` — do
  not reuse for DM1 (uniform 2352 vs mixed sectors).
- ReDMCSB DM1 reference source and DMWeb/Greatstone documentation
  remain the authoritative outside references; the FM Towns disc's
  Phar Lap P3 layer is the only piece unique to this port.
