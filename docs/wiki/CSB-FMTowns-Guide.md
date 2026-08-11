# Chaos Strikes Back FM Towns — technical guide for Firestaff agents

This is the FM Towns counterpart to [DM1's guide](DM1-FMTowns-Guide). Read
both before changing CSB FM Towns startup, cache, media or runtime code. The
games share Phar Lap and TownsOS conventions, but they do **not** share a
title program, CD layout, save body or presentation owner.

## Byte-verified DM1↔CSB shared payloads (as of 2026-08-07)

Cross-fingerprint recovery revealed CSB CHTWE.EXP embeds several
DM1 game data structures IDENTICALLY at per-game vaddrs:

| Payload | Bytes | DM1 vaddr | CSB vaddr | Firestaff alias module |
|---|---:|:---:|:---:|---|
| OICON descriptor | 1344 | 0x224db | 0x27f77 | `csb_v1_fmtowns_oicon_descriptor` |
| DYNA_BUTTONS pool | 500+ | 0x24194 | 0x29d50 | `csb_v1_fmtowns_dyna_buttons` |
| SPELL_COSTS | 32 | 0x24388 | 0x29f64 | `fmtowns_shared_tables_all_games` |
| SPELL_MULT | 8 | 0x243a0 | 0x29f7c | `fmtowns_shared_tables_all_games` |
| PLAYER_COLOR | 8 | 0x291b8 | 0x2d164 | `fmtowns_shared_tables_all_games` |
| ICON_PAL | 6 | 0x28f44 | 0x2cd8a | `fmtowns_shared_tables_all_games` |
| CHAR geometry | 14 | 0x26c8a | 0x2c94c | `fmtowns_geometry_all_games` |
| ICON geometry | 8 | 0x26c68 | 0x2c938 | `fmtowns_geometry_all_games` |
| 768-byte font raster | 768 | (asset 557) | file@0x50f1a | `fmtowns_font_raster_all_games` |
| Phar Lap 4-slot bridge | — | ✓ | ✓ | `fmtowns_pharlap_all_games` |
| Direct I/O 0x04E9 | — | ✓ | ✓ | `fmtowns_pharlap_all_games` |

CSB pic_library (`CDATA/GRAPHICS.DAT`, sig 0x8001) reuses DM1's
parser via `csb_v1_fmtowns_pic_library_open_ext_v1_pc34`.

CSB TMENU.EXP has its own SYM1 table (1724 entries) shipped via
`csb_v1_fmtowns_tmenu_sym1`. CHTWE.EXP itself has no SYM1
(stripped) — use the byte-fingerprint modules above.

## CSB-specific (NOT shared with DM1)

- CSB region table (independent menu layout)
- CSB CDDA track table (own soundtrack)
- CSB DOOR palette / animation-timings / STARTUP1 chain
- CSB SWITCHTW.EXP switch page, ANIMTW.EXP, UTILE.EXP

## Retail media and cache

The admitted CSB F31 disc contains both language trees:

| Tree | Game data | Game program | Bootstrap save |
|---|---|---|---|
| English | `CDATA/GRAPHICS.DAT`, `CDATA/DUNGEON.DAT` | `CHTWE.EXP` | `CDATA/MINI.DAT` |
| Japanese | `CJDATA/GRAPHICS.DAT`, `CJDATA/DUNGEON.DAT` | `CHTWJ.EXP` | `CJDATA/MINI.DAT` |

`src/shared/asset_status_m12.c` first verifies the selected hash pair, then
materializes the complete ISO inventory and the original `FMTOWNS.IMG` /
`FMTOWNS.CUE` pair when the source is the retail ZIP/RAR. It must retain that
pair for the original TD/TR CD-DA commands; a derived track table or generated
PCM is not an equivalent source. A manually extracted original tree is also
admitted only when both language packages, their two P3 programs and all six
registered hashes agree. Production retains the original disc root and M11
applies the selected F31 language before startup. The development/test
materializer creates a language-private cache with the selected flat
`GRAPHICS.DAT`/`DUNGEON.DAT`, the original nested `CDATA/MINI.DAT` or
`CJDATA/MINI.DAT`, root programs and original portraits, and removes stale
archive CUE/IMG files so loose media cannot borrow CDDA from a different
source. The scanner reports each admitted CSB edition, rather than using the
selected cache pair as evidence for another platform.

CSB uses its own 31-track Red Book layout. Do not reuse DM1's mixed
MODE1/2048-and-audio offset calculation or DM2's Towns music helper. The CSB
reader is `src/csb/csb_v1_fmtowns_cd.c`; M11's live transport boundary is in
`src/engine/m11_game_view.c`.

## Startup program graph

```
TITLE.ANM ──return──> SWITCHTW.EXP ──Game──> CHTWE.EXP / CHTWJ.EXP
                    ├─Story──> ANIMTW.EXP STORY.ANM ──return──┘
                    └─Utility──> UTILE.EXP / UTILJ.EXP
```

`TITLE.ANM`, `STORY.ANM` and `ENDING.ANM` are F2275/F8288 animation streams.
Their Timer-A waits, loop execution and TD/TR requests are source-owned.
`ENDING.ANM` holds its final frame when it returns; it does not route back to
the switch page. `SWITCHTW.EXP` owns the 320×200 language pages, palette,
four button streams and the selected handoff. `AUTOEXEC.BAT` establishes the
exit mapping. See ReDMCSB `NECIO.C`, `SWITCH.C`, `ANIMTOWN.C` and
`STARTUP2.C`.

The Game exit is a separate C03 program, not PC 3.4 TITLE.C:

- F31E admits only `CHTWE.EXP` (283,936 bytes; FNV-1a `3da136f6`).
- F31J admits only `CHTWJ.EXP` (284,416 bytes; FNV-1a `f937db45`).
- Both enter the C004 Prison wait from `STARTUP1.C` / `ENTRANCE.C F0807`.

`include/csb_v1_fmtowns_game.h` and `src/csb/csb_v1_fmtowns_game.c` carry
that admission receipt. M11 opens only the source-bound C004 entrance and
C002/C003 door sequence, then the verified C017/F0128 HUD/viewport session.
It must not replay the standalone title timeline after this transition.

## F31 MINI.DAT is a native save bootstrap

`MINI.DAT` is neither an Atari/Amiga GAMEBLOCK nor a substitute DUNGEON.DAT.
ReDMCSB `CEDTINCD.C F7051` validates its 512-byte C5 header through F7061,
then F7057-checks five save parts, reads four external portraits, and passes
the remaining dungeon tail to F7063.

| Receipt | English | Japanese |
|---|---:|---:|
| File size | 42,776 | 43,208 |
| FNV-1a | `494999c9` | `284799d1` |
| Header platform | F7 | F8 |
| Dungeon identity | C13 CSB Game | C13 CSB Game |
| Validated party pose | map 4, `(22,18)`, direction 2 | map 4, `(22,18)`, direction 2 |
| Game time | 82 | 88 |

The verified save body has native `GLOBAL_DATA`, active groups, champion and
party data, events and timeline parts. The tail begins after the four
464-byte external portraits. Its checksum is validated and can be copied by
`csb_v1_fmtowns_game_copy_verified_dungeon_tail()` into the ordinary real
CSB dungeon reader. MAP coordinates are bytes 6/7 (`OffsetMapX/Y`), not the
unreferenced bytes 4/5; this follows ReDMCSB `DEFS.H` and `DUNGEON.C F0154`.

The production M11 resume path binds those owners as one transaction. The
real English and Japanese `MINI.DAT` files pass the end-to-end
`test_csb_v1_fmtowns_mini_resume_real` gate: CHTWE/CHTWJ enters the saved
map-4 pose at `(22,18)`, direction 2, with one champion and no title replay.
This proves the shipped retail bootstrap save path. It does not prove an
arbitrary user `CSBGAME.DAT` until that file passes the same native F0435
header, five-part, portrait and dungeon-tail gates. The external
`CSBGAME.DAT` and `CSBGAME-JP.DAT` files are retained as unclassified
candidates. The current F7061/F7057 reader rejects them before a complete
native F0435 state receipt is produced, so they are not positive save evidence
and the F0433 write-back path remains closed. No generated save is used as a
substitute.

## Utility boundary

`UTILE.EXP` and `UTILJ.EXP` are separate C06_CEDT programs. Their P3
envelopes, six-label source pools, C09 icon palette, source-coordinate menu
boxes, F31 M653 font material and planar `.CMP` portrait decoder are
verified. For F31E this authorizes a deliberately small editor surface: the
verified C06 frame, champion selection, palette selection, planar 32×29
pixel drawing, connected-area fill, Revert and Undo. Each edit remains in
the admitted `MINI.DAT` portrait bytes. The undo image is one source-format
copy, and neither editing nor reverting writes a host or user save. The
source-owned portrait transactions also cover revalidated `F7002_ReadCMP`
import after a catalogue selection and the source `SAVE CHAMPIONS` dialog.
F7000's `PORTRAIT` choice preserves an admitted `.CMP` header, then writes
the selected champion's live name, title and planar payload to the separate
portrait medium: `~/.firestaff/portraits` on macOS/Linux or
`INSTALLDIR\\portraits` on Windows. The scanned CD catalogue stays read-only;
F7001's whole-game branch is still deliberately unavailable.

It does not authorize the rest of Utility. `F7004_LoadChampions` now presents
its native `GAME` / `PORTRAIT` / `CANCEL` choice before `PORTRAIT` opens the
source-owned `CEDT008`/`CEDT013` picker. The whole-game branch remains closed:
it belongs to the separately unbound `F7051_LoadGame` transaction. Make New
Adventure and name/title entry are also closed. F31J remains closed until the
native Shift-JIS glyph consumer is recovered. Keep all of those routes modal
until their specific owners are evidenced.

## Files to know

| Purpose | Path |
|---|---|
| Game and MINI receipts | `include/csb_v1_fmtowns_game.h`, `src/csb/csb_v1_fmtowns_game.c` |
| Switch page and handoffs | `include/csb_v1_fmtowns_switch.h`, `src/csb/csb_v1_fmtowns_switch.c` |
| Disc, ISO and CD-DA reader | `include/csb_v1_fmtowns_cd.h`, `src/csb/csb_v1_fmtowns_cd.c` |
| Animation stream | `src/csb/csb_v1_fmtowns_animation.c` |
| Cache materializer | `src/shared/asset_status_m12.c` |
| M11 startup, entrance and transport | `src/engine/m11_game_view.c` |
| C06 preservation boundary | `docs/source-lock/csb_v1_fmtowns_c06_utility_boundary.md` |
| Japanese C06 text boundary | `parity-evidence/csb_fmtowns_f31j_text_owner.md` |
| Main remaining work | `TODO.md` (`CSB-FMTOWNS-RUNTIME-PARITY`) |

## Non-negotiable boundaries

- Never use PC 3.4 title, Utility, audio or save code as a FM Towns fallback.
- Never generate pixels, CD audio, menu labels or save/champion data when the
  verified F31 owner is missing.
- Keep the selected `CDATA`/`CJDATA` pair ahead of the opposite-language
  sidecar tree.
- A development/test loose F31 cache must copy and hash the selected language's original
  `CDATA/MINI.DAT` or `CJDATA/MINI.DAT` path alongside its flat
  `GRAPHICS.DAT` and `DUNGEON.DAT`; title media alone is not a valid C03/C06
  startup owner.
- Keep original data out of the repository. Tests use user-supplied,
  hash-admitted media and skip safely when it is unavailable.
