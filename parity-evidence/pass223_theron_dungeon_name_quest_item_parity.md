# Pass 223 — Theron Dungeon Name and Quest Item Parity

## Summary

Replace synthetic dungeon names and quest item ordering with
binary-verified data from US Track 02 BIN.

## Source

- File: `TQUS02.bin` (MD5 `f23601102138f87c33025877767ebf76`)
- Format: BIN (2352 bytes/sector, 16-byte header + 2048 user data)

## Dungeon Creature Names (UD 0x2741EF)

7 entries, 8-byte stride (7-char ASCII + 0x01 separator):

| Dungeon | UD Offset  | Name    |
|---------|-----------|---------|
| 2       | 0x2741EF  | AKUTUBA |
| 3       | 0x2741F7  | DRATOR  |
| 4       | 0x2741FF  | FORMIC  |
| 5       | 0x274207  | SARMON  |
| 6       | 0x27420F  | SHADO   |
| 7       | 0x274217  | THIEF   |

Dungeon 1 (Hall of Records) name comes from the stage-select menu,
not this table.

## Quest Item Ordering (UD 0x27713D)

Retrieval messages "THERON has retrieved the X" appear in dungeon order:

| Index | UD Offset  | Item            |
|-------|-----------|-----------------|
| 0     | 0x277156  | Shield Defiant  |
| 1     | 0x277189  | Taza Boots      |
| 2     | 0x2771B8  | Taza Poleyn     |
| 3     | 0x2771E8  | Soulcage        |
| 4     | 0x277215  | Taza Armour     |
| 5     | 0x277245  | Tazahelm        |
| 6     | 0x277272  | Retaliator      |

## Object Action Names (UD 0x1DEEA5)

4 entries preceding the hand action table:

| Index | UD Offset  | Name      |
|-------|-----------|-----------|
| 0     | 0x1DEEA5  | BLOCK     |
| 1     | 0x1DEEAB  | CHOP      |
| 2     | 0x1DEEB2  | BLOW HORN |
| 3     | 0x1DEEBC  | FLIP      |

## Save/Delete File Cabinet Strings (UD 0x243081)

| Index | UD Offset  | String                                            |
|-------|-----------|---------------------------------------------------|
| 0     | 0x243081  | YOU DON'T HAVE ENOUGH SPACE IN THE FILE CABINET!  |
| 1     | 0x2430BE  | CHOOSE A FILE TO DELETE.                          |
| 2     | 0x2430DD  | SURE?                                             |
| 3     | 0x24311B  | THANK YOU.                                        |
| 4     | 0x24313A  | THIS GAME WILL NOT BE SAVED!                      |

## Action Name "X" Verification (UD 0x1DEF4E)

The single-character action name "X" at index 15 in the combat action
table is confirmed as a real binary value at UD 0x1DEF4E (byte 0x58),
not a placeholder. Context: `MELEE\x00X\x00INVOKE`.

## Track 19 Runtime Item Name Table (UD 0x0E9271)

69 items in MODE1/2048 ISO user data. Includes 14 dungeon-specific items
not present in Track 02's table: WATERSKIN, WATER, STORMRING, MACE,
TORSO PLATE, LEG PLATE, ARMET, FOOT PLATE, SOUL CAGE, SARMON'S BONES,
IRON KEY, SKELETON KEY, EMERALD KEY, LOCK PICKS.

## Versions

- v3.0.223: Dungeon names, quest item ordering, viewport wiring
- v3.0.224: Object action names, save/delete strings
- v3.0.225: Track 19 runtime item name table (69 items)
