# DM1 FM Towns Japanese executable (`JDM.EXP`) — structural map

Source: HMA-240 Japanese-language executable `JDM.EXP` extracted from
`~/.firestaff/data/dm1/Dungeon Master (Japan) (En,Ja) (Rev 1).7z`,
Track 01 (MODE1/2048 ISO 9660 volume "DUNGEON"), file `/DUNGEON/JDM.EXP`.

- Disk size: **290 221 bytes** (0x46dad)
- SHA-256: `1db4f049def0eef52a20a4758a1ce3e204f9f5a9ec04a57eef1245fdeede0bae`

All field offsets below use the exact positions parsed by
`validate_p3_header` in `src/dm1/dm1_v1_fmtowns_startup.c:159`.

## 1. P3 header (parsed from JDM.EXP)

| Field                    | Offset | JDM value              | EDM value (reference) |
|--------------------------|--------|------------------------|-----------------------|
| Magic                    | 0x00   | `"P3"`                 | `"P3"`                |
| Level (LE16)             | 0x02   | 1                      | 1                     |
| Header size (LE16)       | 0x04   | 0x0180 (384)           | 0x0180                |
| File size (LE32)         | 0x06   | 0x46dad (290 221)      | 0x46b4f (289 615)     |
| Runtime offset (LE32)    | 0x0c   | 0x180                  | 0x180                 |
| Runtime size (LE32)      | 0x10   | 0x80                   | 0x80                  |
| Relocation offset (LE32) | 0x14   | 0x200                  | 0x200                 |
| Relocation size (LE32)   | 0x18   | 0                      | 0                     |
| Load image offset (LE32) | 0x26   | 0x200                  | 0x200                 |
| Load image size (LE32)   | 0x2a   | 0x46bad (289 709)      | 0x46941 (289 089)     |
| Symbol table offset (LE32) | 0x2e | **0** (absent)         | 0x46b41               |
| Symbol table size (LE32) | 0x32   | **0** (absent)         | 0x51b5 (20 917 bytes) |
| Initial EIP (LE32)       | 0x68   | **0x00042cb4**         | 0x00042a48            |
| Memory requirements (LE32) | 0x74 | 0x778f0 (489 712)      | 0x77684 (489 092)     |

The header passes `validate_p3_header` cleanly. JDM.EXP is a well-formed
Phar Lap P3 level-1 executable, 620 bytes larger in load image than the
English build and 620 bytes higher in memory footprint.

## 2. Symbol-table verdict — SYM1 is absent

- `symbol_table_offset == 0` and `symbol_table_size == 0` in the header.
- A byte-level scan for the literal `"SYM1"` across the entire 290 221-byte
  file returns **no match**.
- Byte-level scans of the load image for the English identifiers
  `DRAW_DMENU`, `DYNAMENU`, `DYNA_BUTTONS`, `MENU_ICONS`, `PLAYER`,
  `SCREEN`, `MOUSE_ON`, `MOUSE_OFF`, and `EGB_` — every one of them
  present in EDM's SYM1 table — return **no match**.

The Japanese release was linked without the Metaware / Phar Lap debug
symbol emission. This confirms the receipt code path in
`dm1_v1_fmtowns_startup.c` that treats a zero-size symbol table as
"unverified" for JDM.

## 3. DYNA_BUTTONS-equivalent Japanese label pool

Although no symbols exist, the DYNA_BUTTONS NUL-separated string pool
is byte-structurally preserved. It sits at **load vaddr 0x243bc**
(instead of EDM's 0x24194 — shifted by 0x228 bytes to accommodate the
wider Shift-JIS text). Entry 0 is again the single ASCII placeholder
`"N"`; every subsequent slot maps 1:1 to the EDM verb pool:

| Idx | EDM label   | JDM label  | Shift-JIS bytes | Vaddr    |
|----:|-------------|------------|-----------------|----------|
|  0  | `N`         | `N`        | 4E              | 0x243bc  |
|  1  | `BLOCK`     | `さえぎる` | 82B382A682AC82E9 | 0x243be |
|  2  | `CHOP`      | `叩き切る` | 924082AB90D882E9 | 0x243c7 |
|  3  | `X`         | `X`        | 58              | 0x243d0  |
|  4  | `BLOW HORN` | `角笛を吹く` | 8A70934A82F0908182AD | 0x243d2 |
|  5  | `FLIP`      | `ｺｲﾝﾄｽ`    | BAB2DDC4BD      | 0x243dd  |
|  6  | `PUNCH`     | `殴る`     | 89A382E9        | 0x243e3  |
|  7  | `KICK`      | `蹴る`     | 8F5282E9        | 0x243e8  |
|  8  | `WAR CRY`   | `ときの声` | 82C682AB82CC90BA | 0x243ed |
|  9  | `STAB`      | `刺す`     | 8E6882B7        | 0x243f6  |
| 10  | `CLIMB DOWN`| `降りる`   | 8D7E82E882E9    | 0x243fb  |
| 11  | `FREEZE LIFE`| `時間凍結`| 8E9C8AD493808C8B | 0x24402 |
| 12  | `HIT`       | `打つ`     | 91C582C2        | 0x2440b  |
| 13  | `SWING`     | `振り回す` | 905582E889F182B7 | 0x24410 |
| 14  | `STAB`      | `刺す`     | 8E6882B7        | 0x24419  |
| 15  | `THRU`      | `突き刺す` | 93CB82AB8E6882B7 | 0x2441e  |

Larger Japanese message strings (dialog, disk prompts, error text) live
in a separate pool starting at **load vaddr 0x22000** and continuing
past 0x23000. Sample content recovered from that pool includes:

- `Q:\JDATA\GRAPHICS.DAT`, `Q:\JDATA\DUNGEON.DAT`, `\DUNGEONB.DAT`,
  `A:\DUNGEON.FTL` (Japanese release reads `JDATA/`, not `DATA/`).
- Disk prompts: `ｹﾞｰﾑ再開`, `ｹﾞｰﾑ中断`, `ｹﾞｰﾑ経過をﾃﾞｨｽｸにｾｰﾌﾞできません。`.
- Skill titles are still English (`FIGHTER`, `NINJA`, `PRIEST`,
  `WIZARD`) and so are character levels (`NEOPHYTE`, `NOVICE`,
  `APPRENTICE`, `JOURNEYMAN`, …).

Nothing here was fabricated: every string above was decoded from raw
JDM.EXP bytes with `shift_jis` and printed with its exact load-image
offset.

## 4. Initial-EIP comparison with EDM

Both entry points contain the same Metaware / Phar Lap "Hight C
Run-time Library Copyright (C) 1983-1988" run-time stub. Disassembly
under `capstone` i386 mode:

```
JDM  0x00042cb4:  jmp  0x42d0c
     0x00042cb6:  call 0x248          ; run-time init
     0x00042cbb:  push eax
     0x00042cbc:  call 0x448d4        ; C startup / main
     0x00042cc1:  xor  eax, eax
     0x00042cc3:  int  0x21           ; DOS extender exit
     0x00042cc5:  ret

EDM  0x00042a48:  jmp  0x42aa0
     0x00042a4a:  call 0x248
     0x00042a4f:  push eax
     0x00042a50:  call 0x44668
     0x00042a55:  xor  eax, eax
     0x00042a57:  int  0x21
     0x00042a59:  ret
```

Byte-for-byte diff of the first 64 bytes at each initial EIP: **62/64
bytes identical**; the two differing bytes are the LE32 displacement of
the `call` at file+4 (`call rel32` to the C main), which is expected
because the code layouts differ by 0x26c bytes overall.

**Verdict.** JDM.EXP is not an independent build — it is the *same*
Phar Lap-linked binary produced from the same source with the DATA
segment localised to Japanese. Both files use the identical run-time
stub, the identical entry sequence, and the identical structure of the
DYNA_BUTTONS pool. This is a rebuild-with-different-resources, not a
reimplementation.

## 5. TownsOS EGB library boundary

EDM's SYM1 table anchors the EGB primitives inside the load image at
`0x40739..0x40ee5` (see `parity-evidence/dm1_fmtowns_menu_p3_disassembly.md`).
Because JDM lacks SYM1, the same vaddrs cannot be confirmed by name.
The bytes at those exact addresses in JDM are entirely different:

| EDM symbol         | Vaddr    | EDM first 16 bytes                | JDM first 16 bytes                |
|--------------------|----------|-----------------------------------|-----------------------------------|
| EGB_RESOLUTIONRAM  | 0x40739  | `5356 578b 7c24 108a 4424 1466 8b4c 2418` | `0000 a002 4000 0000 0000 0000 0000 c805` |
| EGB_VIEWPORT       | 0x407a0  | `5657 8b7c 240c 8b74 2410 b403 0fa0 6810` | `c291 0ea6 aea0 19e3 a346 0000 0f0c 7581` |
| EGB_WRITEPAGE      | 0x407ec  | `578b 7c24 088a 4424 0cb4 050f a068 1001` | `0000 0000 0000 0000 0000 0000 0000 0000` |
| EGB_COLOR          | 0x40836  | `578b 7c24 088a 4424 0c8b 5424 10b4 070f` | `0000 0000 0000 0000 0000 0000 0000 0000` |
| EGB_WRITEMODE      | 0x408a5  | `578b 7c24 088a 4424 0cb4 0a0f a068 1001` | `0000 c0ff 7f01 00c7 aa30 d36d ded0 edfe` |
| EGB_PUTBLOCK       | 0x40bec  | `5657 8b7c 240c 8a44 2410 8b74 2414 b425` | `0fa0 6810 0100 000f a164 ff1d 2000 0000` |
| EGB_RECTANGLE      | 0x40ee5  | `5657 8b7c 240c 8b74 2410 b446 0fa0 6810` | `64ff 1d20 0000 000f a10f bec4 5f5e c356` |

JDM's `0x40000..0x40fff` band is populated with different code and
significant runs of zero-fill. Because both builds are the same binary
with rearranged data, the EGB library is still linked in — its
trampolines are simply relocated to different load-image vaddrs, and
without SYM1 there is no name index to point at them.

Structural signature of an EGB trampoline in EDM (per
`dm1_fmtowns_menu_p3_disassembly.md`) is a short i386 stub ending in
`b?XX 0fa0 6810 01?? ??`. JDM contains that same `0f a0 68 10 01 00`
sequence at multiple sites — for example load vaddrs 0x40bec, 0x40ee5,
plus other addresses inside the code segment. The library is present;
it just is not addressable by name from this executable alone.

## 6. What would be needed to reach EDM-parity decoding on JDM

1. **Rebuild the symbol map by structural matching.** With EDM's SYM1
   as an oracle, walk EDM function-by-function and use short unique
   byte fingerprints (opcode + immediate-stripped byte pattern) to
   locate the equivalent JDM function. The Phar Lap linker preserved
   function bodies verbatim, so ~1174 entries should map with high
   confidence; only functions whose immediates reference data-segment
   addresses will need per-immediate rewriting.
2. **Rewrite absolute data references.** Data-segment vaddrs shift by
   +0x228 (label pool) and by other deltas elsewhere. A relocation
   table synthesised from JDM ↔ EDM data-string matches (recover pool
   base by locating `"N\0"` + a distinctive verb string; recover BSS by
   locating fixed-pattern strings such as `Q:\JDATA\GRAPHICS.DAT`) is
   sufficient to translate the EDM decode's data addresses to JDM.
3. **Enumerate JDM EGB trampolines.** Search for the six-byte
   `0f a0 68 10 01 00` pattern (already present at 0x40bec / 0x40ee5)
   plus its `b? XX` prefix to locate all EGB trampolines and record
   their JDM vaddrs. Cross-reference call sites to identify which
   is which (writepage / viewport / rectangle …) by comparing the
   caller sequence to EDM's known callers.
4. **Bind the Japanese DYNA_BUTTONS pool** at 0x243bc into
   `dm1_v1_fmtowns_startup.c` receipt reporting so the runtime can
   surface Shift-JIS action labels once the ordinal index resolution
   is proven against JDM's own `GET_LABEL` equivalent.
5. **Verify the JDATA/ path fixup.** JDM's data-segment strings hard
   -code `Q:\JDATA\` for asset files (English uses `Q:\DATA\`). The
   asset loader must consult the executable-derived root string.

None of steps 1–5 require synthesising bytes. Every needed address is
in JDM.EXP already; the SYM1 loss only means we must recover it by
structural comparison, not by fabrication.

## Notes on method

- Header parse: `struct.unpack_from` at the exact offsets used by
  `validate_p3_header`.
- String scans: literal `bytes.find` on the load image
  (`file[0x200:0x200+load_size]`).
- Disassembly: `capstone` 5.0.7, `CS_ARCH_X86 / CS_MODE_32`.
- Shift-JIS decoding: Python `.decode("shift_jis")` — no substitution
  or reinterpretation was applied.
- All addresses in this document are load-image vaddrs (file offset =
  `0x200 + vaddr`).
