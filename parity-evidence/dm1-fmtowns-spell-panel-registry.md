# FM Towns spell panel: source bitmap and registry evidence

Verified locally on 2026-09-06 by streaming the supplied original
`Dungeon-Master_FM-Towns_JA-EN.zip` BIN member in memory. No game files
were extracted or added to the repository. The first track is a
MODE1/2048 ISO9660 volume; both executable entries are in its root.

| Executable | Bytes | SHA-256 |
|---|---:|---|
| EDM.EXP | 310518 | c888470d39aa449eac85438b598158492d2c981008cc6b427c52f2c73001ecb6 |
| JDM.EXP | 290221 | 1db4f049def0eef52a20a4758a1ce3e204f9f5a9ec04a57eef1245fdeede0bae |

## Static linked registry

Phar Lap load-image addresses below exclude the 0x200 file header.
The unique root record sequence `(9,0,320,200),(1,1,0,0)` locates the
first block. Walking its next pointers visits 23 blocks in each image.
Each block has a little-endian `{u32 next,u16 first,u16 last}` header
followed by eight-byte `{u16 type,u16 parent,i16 a,i16 b}` records.

| Property | English | Japanese |
|---|---|---|
| First block, IDs 1–17 | 0x28e78 | 0x290ec |
| Spell block, IDs 220–264 | 0x27098 | 0x2730c |
| C012 | (9,2,87,33) | (9,2,87,33) |
| C013 | (3,12,319,74) | (3,12,319,82) |
| C010 | (9,2,87,45) | (9,2,87,72) |
| C011 | (2,10,319,77) | (2,10,319,85) |

All 45 spell-block records are byte-identical. Their ancestor C013 is
not: it moves the Japanese panel eight pixels down. Several other root
records also differ (7–11,14,16); matching child records cannot establish
matching complete layouts. This corrects the tempting but invalid
inference from the spell block alone.

## C009 placement and clipping

The original native loader reports C009 as **96×25** for FM Towns,
not the DOS I34 bitmap's 87×25. IMG2 stores actual width/height; the
decoder does not invent padding. ReDMCSB DEFS.H M100/M101 read those
header dimensions.

BASE.C F0660:1493–1500 delegates to COORD.C F0635. The latter reads
bitmap dimensions at 2307–2312, applies the bottom-right anchor at
2333–2338, and returns source offsets after parent clipping at
2363–2382. Consequently:

| Edition | Unclipped destination | C012 clipping | Source crop |
|---|---|---|---|
| F20E | (224,50), 96×25 | (233,50), 87×25 | (9,0), 87×25 |
| F20J | (224,58), 96×25 | (233,58), 87×25 | (9,0), 87×25 |

Discarding nine columns merely as presumed padding, or drawing the full
bitmap without the parent clip, is not the source operation.

## Verification scope

The late-panel real-media oracle uses original C009 and M653 bytes and
bounded RAM party/rune fixtures. English and Japanese ASCII/rune cases
are distinct from Japanese text: TEXT2.C:75–112 routes Japanese strings
through F0952. The oracle does not establish that Japanese-name path,
pointer/input alignment, original timing, or emulator capture parity.

Local regression run: `dm1_fmtowns_jp_names_real`,
`m11_dm1_spell_panel_real`, and `m11_dm1_xp_real` passed without skips
(3/3, 26.54 seconds). The FM Towns test exercises 24 EN and 24 JP
ASCII/rune cases across Original/V2.0/V2.1. Before the Japanese C011
clear correction, it failed at pixel (242,77): the DOS-position clear
overwrote the bottom six rows. ACTIDRAW.C F0387:323 clears C011, whose
Japanese top-right anchor resolves to (233,85), size 87x72.
Action cells and active-menu geometry remain separately unverified.

Related English registry disassembly: [region table](dm1_fmtowns_region_table.md).

## Follow-up: idle action-cell geometry

A second hash-verified in-memory registry walk found C088=(9,11,20,35)
in EDM and (9,11,20,62) in JDM. Both editions use
C089–C092=(1,88,22*slot,9) and C093–C096=(10,89+slot,0,0).
COORD.C:2177 onward centers type-10 children; the bitmap placement
subtracts half its extent with the source rounding convention.

| Geometry | English | Japanese |
|---|---|---|
| Cell, slot s | (233+22s,86), 20x35 | (233+22s,94), 20x62 |
| Centered 16x16 icon | (235+22s,96) | (235+22s,117) |

The Japanese icon displacement is 21 pixels, not eight: the parent
height also changes. These are recovered target coordinates, not a claim
that the current icon renderer or pointer consumers implement them.
