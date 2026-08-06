# Chaos Strikes Back FM Towns — technical guide for Firestaff agents

This is the FM Towns counterpart to [DM1's guide](DM1-FMTowns-Guide). Read
both before changing CSB FM Towns startup, cache, media or runtime code. The
games share Phar Lap and TownsOS conventions, but they do **not** share a
title program, CD layout, save body or presentation owner.

## Retail media and cache

The admitted CSB F31 disc contains both language trees:

| Tree | Game data | Game program | Bootstrap save |
|---|---|---|---|
| English | `CDATA/GRAPHICS.DAT`, `CDATA/DUNGEON.DAT` | `CHTWE.EXP` | `CDATA/MINI.DAT` |
| Japanese | `CJDATA/GRAPHICS.DAT`, `CJDATA/DUNGEON.DAT` | `CHTWJ.EXP` | `CJDATA/MINI.DAT` |

`src/shared/asset_status_m12.c` first verifies the selected hash pair, then
materializes the complete ISO inventory and the original `FMTOWNS.IMG` /
`FMTOWNS.CUE` pair. It must retain that pair for the original TD/TR CD-DA
commands; a derived track table or generated PCM is not an equivalent source.
The scanner reports each admitted CSB edition, rather than using the selected
cache pair as evidence for another platform.

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

This is deliberately not a partial live restore. A valid runtime handoff
must bind the F31 `CHAMPION_EXCLUDING_PORTRAIT` body, external portraits,
active groups, events and timeline together. Replacing only the dungeon or
only the party pose would create an invented mixed save state. Until those
owners are recovered, retain the receipt and source-backed entrance instead.

## Utility boundary

`UTILE.EXP` and `UTILJ.EXP` are separate C06_CEDT programs. Their P3
envelopes, six-label source pools, C09 icon palette, source-coordinate menu
boxes, F31 M653 font material and planar `.CMP` portrait decoder are
verified. That does not authorize an M11 editor: the original EGB screen
composition, file-picker, save/portrait transactions and F31J Shift-JIS
EGB glyph consumer remain unbound. Keep Utility modal until those specific
owners are evidenced.

## Files to know

| Purpose | Path |
|---|---|
| Game and MINI receipts | `include/csb_v1_fmtowns_game.h`, `src/csb/csb_v1_fmtowns_game.c` |
| Switch page and handoffs | `include/csb_v1_fmtowns_switch.h`, `src/csb/csb_v1_fmtowns_switch.c` |
| Disc, ISO and CD-DA reader | `include/csb_v1_fmtowns_cd.h`, `src/csb/csb_v1_fmtowns_cd.c` |
| Animation stream | `src/csb/csb_v1_fmtowns_animation.c` |
| Cache materializer | `src/shared/asset_status_m12.c` |
| M11 startup, entrance and transport | `src/engine/m11_game_view.c` |
| Japanese C06 text boundary | `parity-evidence/csb_fmtowns_f31j_text_owner.md` |
| Main remaining work | `TODO.md` (`CSB-FMTOWNS-RUNTIME-PARITY`) |

## Non-negotiable boundaries

- Never use PC 3.4 title, Utility, audio or save code as a FM Towns fallback.
- Never generate pixels, CD audio, menu labels or save/champion data when the
  verified F31 owner is missing.
- Keep the selected `CDATA`/`CJDATA` pair ahead of the opposite-language
  sidecar tree.
- Keep original data out of the repository. Tests use user-supplied,
  hash-admitted media and skip safely when it is unavailable.
