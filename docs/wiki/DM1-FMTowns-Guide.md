# DM1 FM Towns — technical guide for Firestaff agents

This page is the single entry point any agent should read before
touching DM1 FM Towns code. It consolidates the disc image format, the
executable structure, symbol coordinates recovered by disassembly,
CDDA layout, currently wired features and the concrete extraction
workflow. All addresses come from the hash-verified HMA-240 English
disc; the Japanese disc uses the same layout but a different
executable (`JDM.EXP`) whose symbol table has not been disassembled.

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

**Open (blocked on decoded work, not on synthesis):**

- `DRAW_DMENU` / `DYNAMENU` visible menu rendering. Consumption
  plan and disassembly are in the parity-evidence file linked above.
  The three bounded next steps: (a) lift `GET_SCL_COORD` (0x1942c)
  to recover the region table; (b) provide an EGB shim over the
  M11 framebuffer for `EGB_RECTANGLE` and `EGB_PUTBLOCK`; (c) decode
  `DO_DRAW_CTEXT` (0x1a804) for the font raster.
- `TMENU` interactive icon/layout rendering and mouse routes.
- Japanese `JDM.EXP` title animation and menu (no SYM1 → symbol
  addresses are not lifted).

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
| Tests                      | `tests/test_dm1_v1_fmtowns_cd_audio.c`, `tests/test_dm1_v1_fmtowns_title.c`, `tests/test_firestaff_fmtowns_disc.c` |

## 11. Cross-references

- DM2 FM Towns status and layout (also disc-image based):
  see the DM2 FM Towns notes tracked by the DM2 lane in TODO.md.
- CSB FM Towns CDDA runtime: `src/csb/csb_v1_fmtowns_cdda_*` — do
  not reuse for DM1 (uniform 2352 vs mixed sectors).
- ReDMCSB DM1 reference source and DMWeb/Greatstone documentation
  remain the authoritative outside references; the FM Towns disc's
  Phar Lap P3 layer is the only piece unique to this port.
