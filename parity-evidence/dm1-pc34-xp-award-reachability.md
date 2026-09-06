# DM1 PC 3.4 XP award reachability

## Scope and conclusion

Source audit, 2026-09-06. Ordinary, defined PC 3.4 combat and spell award
paths examined below stay below the unsigned 16-bit F0304 scaling limit.
These are conservative independent upper bounds, not assertions that each
maximum is simultaneously reachable. Direct primitive tests with oversized
awards verify word arithmetic, not an original fight that overflows it.

References below are ReDMCSB WIP20210206 files under
`reference/redmcsb-20210206/Toolchains/Common/Source/`.
This does not establish cumulative 32-bit XP behavior, other editions,
custom spell/action tables, or binary behavior of uninitialized variables.

## Source bounds

`CHAMPION.C` F0304:866-891 halves stale combat awards before applying
nonzero map difficulty and, for hidden skills, recent-combat doubling.
The map difficulty field is four bits. Thus multiplying by 15 and then 2
is a conservative scaling bound; stale halving can only reduce it.

| Award path | Input bound | With difficulty 15 and doubling | With original dungeon maximum 6 and doubling |
|---|---:|---:|---:|
| Initialized melee damage XP | 469 | 14,070 | 5,628 |
| DM1 spell XP | 427 | 12,810 | 5,124 |
| Throw | 75 | 2,250 | 900 |
| Fear | 45 | 1,350 | 540 |
| Ordinary action table | 40 | 1,200 | 480 |
| Heal action | 1,998 | 59,940 | 23,976 |
| Parry defense | 15 | 450 | 180 |

- Melee: `DEFS.H:1-4` defines the RNG bounds. `CHAMPION.C:1302` clamps
  computed strength to 0..100. `MENU.C:202-247` has maximum action damage
  factor 96. `PROJEXPL.C:1495-1505` therefore bounds the initialized
  pre-defense damage by 450 and the post-defense value by 481. The weak
  initialized branch at 1505-1517 is smaller (at most 38). Lines 1524-1528
  successively bound damage by 240, 482, 963, 240 and 244; critical damage
  at 1531-1532 is at most 498. The experience nibble is at most 15
  (`DEFS.H:1658`), so line 1535 awards at most `(498 * 15 >> 4) + 3 = 469`.
- Spells: DM1's table at `MENU.C:50-83` has maximum base requirement 4;
  later CSB-only entries are excluded. Power is 1..6. Line 1826 gives at
  most `7 + 10*16 + 5*4*8 + 10*10 = 427`. Failed practice reduces this.
- Throw: `CHAMPION.C:2172-2183` bounds the award by `12 + (255 >> 2)`;
  weapon kinetic energy is an unsigned byte (`DEFS.H:1532`).
- Fear: `MENU.C:949-965` awards at most 45. Ordinary action-table awards
  at 427-486 are at most 40. Parry at `PROJEXPL.C:1351` awards at most 15.
- Heal: `MENU.C:1523-1533` replaces the ordinary action award with
  `2 + 2*cycles`. In a living, valid original champion, maximum health is
  at most 999 (`CHAMPION.C:973-974`) and current health is at least 1.
  Each cycle heals at least one point, so at most 998 cycles give 1,998 XP.
  This deliberately loose bound does not require enough mana to realize it.

## Original dungeon evidence

The EN PC 3.4 archive's `DATA/DUNGEON.DAT` has MD5
`766450c940651fc021c92fe5d0d0b3a6`. Its fourteen map difficulties are:

`0, 1, 1, 2, 2, 2, 3, 3, 3, 4, 5, 5, 6, 6`.

Verified by streaming the original ZIP member, without disk extraction,
and inspecting its fourteen 16-byte descriptors after the 44-byte header.
Difficulty is the upper nibble of descriptor byte 13 (the high byte of C).
The native decoder is `memory_dungeon_dat_pc34_compat.c:949,963-988`.
Archive local-header backslashes differ from central-directory slashes;
the reader reports that spelling mismatch while reading the central member.

## Exception and next verification

`PROJEXPL.C:1493` explicitly documents BUG0_81: computed strength zero
jumps past initialization of `L0566_i_Damage`. Stack-dependent original
binary behavior cannot be assigned a deterministic synthetic oracle or
bounded using the initialized-path calculation above. An exact original
binary execution trace is needed if reproducing that bug is pursued.

Use valid high-power spell casts on an actual difficulty-6 map, comparing
recent and non-recent combat, for the next original-media scaling regression.
Do not manufacture an oversized normal award to claim natural overflow.
Actual fight/cast presentation and edition-specific cumulative XP overflow
remain separate open verification work.

## Native original-media regression

`m11_dm1_xp_real` now pins the canonical original archive MD5 and checks
all fourteen loaded map difficulties. A bounded RAM champion enters Mon
Oh Ir Ra through the native rune/cast APIs on original map 12 (difficulty
6). Seed 1 immediately before casting gives source RANDOM(8)=6, so the
independent constant oracle is 426 XP before scaling: 2,556 normally and
5,112 after a recent attack. Both selected Air and parent Wizard gains,
a positive light effect, and no second mana debit are checked. CTest passes
(0.17 seconds, no skip). Party, RNG and attack timestamps are test fixtures;
the dungeon is unchanged. This is native integration, not original emulator
input, spell rendering, or a demonstration that any conservative maximum
above is reachable.
