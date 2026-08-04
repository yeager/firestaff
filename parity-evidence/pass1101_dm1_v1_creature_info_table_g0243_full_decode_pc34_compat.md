# Pass 1101 — DM1 v1 Creature Info Table G0243 Full Struct Decode (PC 3.4)

## Source

FIRES.EXE (decompressed, 178,224 bytes). Table G0243 at DS:0x0995, stride 26
bytes, 27 entries. Follows pass 1100 (combat stats + attributes).

## Binary struct layout (26 bytes)

| Offset | Size | Field | Profile mapping |
|--------|------|-------|----------------|
| 0 | 1 | creatureType | creatureType |
| 1 | 1 | animationFrameIndex | (not in profile) |
| 2-3 | 2 | attributes | attributes |
| 4-5 | 2 | graphicCharacteristic | (not in profile) |
| 6 | 1 | movementTicks | movementTicks |
| 7 | 1 | attackTicks | attackTicks |
| 8 | 1 | baseDefense | baseDefense |
| 9 | 1 | baseHealth | baseHealth |
| 10 | 1 | baseAttack | baseAttack |
| 11 | 1 | poisonAttack | poisonAttack |
| 12 | 1 | dexterity | dexterity |
| 13 | 1 | padding (always 0) | — |
| 14 | 1 | sightRange (low nibble) | sightRange |
| 15 | 1 | smellRange (low nibble) | smellRange |
| 16-17 | 2 | properties | properties |
| 18-21 | 4 | (unknown — possibly wound/damage matrix) | — |
| 22-23 | 2 | woundProbabilities | woundProbabilities |
| 24 | 1 | attackType | attackType |
| 25 | 1 | padding (always 0) | — |

### Nibble encoding of bytes 14-15

Byte 14 encodes sightRange in its low nibble. The high nibble's purpose is
unconfirmed (possibly pursuit distance or hearing range).

Byte 15 encodes smellRange in its low nibble. The high nibble may encode
attackRange (verified against ReDMCSB Range comments for C01/C04/C05/C16).

## Fields corrected (67 total across 27 creatures)

### sightRange (13 corrections)

| Creature | Old | New |
|----------|-----|-----|
| C02 Giggler | 4 | 6 |
| C06 Screamer | 2 | 1 |
| C08 Ghost | 4 | 3 |
| C10 Mummy | 3 | 4 |
| C11 Black Flame | 4 | 2 |
| C14 Vexirk | 4 | 5 |
| C15 Magenta Worm | 3 | 1 |
| C18 Animated Armour | 3 | 5 |
| C19 Materializer | 5 | 8 |
| C20 Water Elemental | 3 | 1 |
| C23 Lord Chaos | 5 | 9 |
| C25 Lord Order | 5 | 9 |
| C26 Grey Lord | 4 | 9 |

### smellRange (19 corrections)

| Creature | Old | New |
|----------|-----|-----|
| C00 Giant Scorpion | 0 | 1 |
| C02 Giggler | 0 | 3 |
| C06 Screamer | 0 | 1 |
| C07 Rockpile | 0 | 4 |
| C08 Ghost | 0 | 4 |
| C10 Mummy | 4 | 2 |
| C11 Black Flame | 0 | 3 |
| C12 Skeleton | 4 | 0 |
| C13 Couatl | 4 | 3 |
| C14 Vexirk | 0 | 3 |
| C15 Magenta Worm | 0 | 10 |
| C16 Trolin | 4 | 3 |
| C20 Water Elemental | 0 | 3 |
| C22 Demon | 0 | 3 |
| C23 Lord Chaos | 0 | 3 |
| C24 Red Dragon | 0 | 6 |
| C25 Lord Order | 0 | 3 |
| C26 Grey Lord | 0 | 3 |

### attackType (9 corrections)

| Creature | Old | New | Notes |
|----------|-----|-----|-------|
| C00 Giant Scorpion | NORMAL | SHARP | Scorpion stinger |
| C06 Screamer | NORMAL | PSYCHIC | Sonic/psychic scream |
| C07 Rockpile | BLUNT | SHARP | Thrown rock shards |
| C09 Stone Golem | SHARP | BLUNT | Golem fists |
| C10 Mummy | NORMAL | BLUNT | Mummy strikes |
| C15 Magenta Worm | NORMAL | SHARP | Worm bite/fangs |
| C20 Water Elemental | NORMAL | BLUNT | Water impact |
| C21 Oitu | NORMAL | SHARP | Spider-like claws |
| C24 Red Dragon | FIRE | SHARP | Dragon claw melee |

### woundProbabilities (17 corrections)

All 17 creatures that had 0x0000 or 0x0222 defaults now have PC 3.4 binary
values. The woundProbabilities field is a nibble-packed wound slot distribution.

### properties (9 corrections)

Properties is a packed bitfield at offset 16-17. Nine creatures had values
diverging from the PC 3.4 binary.

## Remaining unknowns

Bytes 18-21 (4 bytes per creature) are not yet decoded. The high nibbles of
bytes 14 and 15 are partially understood (byte 15 high nibble likely encodes
attackRange) but not yet mapped to profile fields.

## Verification

Build succeeds. All DM1 creature/combat tests pass.
