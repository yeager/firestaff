# Pass 1100 — DM1 v1 Creature Info Table G0243 PC 3.4 Binary Verification

## Source

FIRES.EXE (decompressed from LZEXE, 178,224 bytes). DS base paragraph 0x24ED
(entry code `MOV DX, 0x24ED`), physical DS offset 0x276D0. Table G0243 at
DS:0x0995 (physical file offset 0x28165), stride 26 bytes, 27 entries.

## Method

1. Decompressed FIRES.EXE using `unlzexe` to recover the raw game engine.
2. Located the creature info table by scanning for `IMUL` instructions with
   stride 26 (`0x1A`) — the table is referenced from 15+ functions.
3. Extracted all 27 × 26-byte entries and mapped byte offsets to profile fields:
   - Offset 0: creatureType (byte)
   - Offset 2–3: attributes (word, little-endian)
   - Offset 6: movementTicks (byte)
   - Offset 7: attackTicks (byte)
   - Offset 8: baseDefense (byte)
   - Offset 9: baseHealth (byte)
   - Offset 10: baseAttack (byte)
   - Offset 11: poisonAttack (byte)
   - Offset 12: dexterity (byte)
4. Cross-referenced each field against the profile table in
   `src/memory/memory_creature_ai_pc34_compat.c`.

## Findings

The existing profile table used Atari ST values from ReDMCSB. The PC 3.4 binary
has significantly different values. **87 combat stat fields** and **16 attributes
fields** were corrected across all 27 creatures.

### Combat stat corrections (15 creatures)

| Creature | Field | Old (Atari ST) | New (PC 3.4) |
|----------|-------|----------------|--------------|
| C00 Giant Scorpion | movTicks | 24 | 8 |
| C00 Giant Scorpion | atkTicks | 10 | 20 |
| C00 Giant Scorpion | attack | 40 | 150 |
| C00 Giant Scorpion | defense | 30 | 55 |
| C00 Giant Scorpion | health | 80 | 150 |
| C00 Giant Scorpion | dexterity | 40 | 55 |
| C00 Giant Scorpion | poison | 5 | 240 |
| C06 Screamer | movTicks | 32 | 120 |
| C06 Screamer | health | 40 | 165 |
| C07 Rockpile | movTicks | 220 | 185 |
| C07 Rockpile | attack | 35 | 40 |
| C07 Rockpile | defense | 150 | 170 |
| C09 Stone Golem | attack | 55 | 219 |
| C09 Stone Golem | defense | 70 | 240 |
| C11 Black Flame | movTicks | 14 | 255 |
| C11 Black Flame | attack | 65 | 105 |
| C15 Magenta Worm | attack | 35 | 45 |
| C15 Magenta Worm | defense | 55 | 72 |
| C15 Magenta Worm | poison | 30 | 35 |
| C18 Animated Armour | attack | 75 | 105 |
| C18 Animated Armour | defense | 110 | 140 |
| C20 Water Elemental | attack | 50 | 66 |
| C20 Water Elemental | defense | 60 | 75 |
| C20 Water Elemental | health | 120 | 144 |
| C24 Red Dragon | attack | 70 | 255 |
| C24 Red Dragon | health | 180 | 255 |
| C26 Grey Lord | movTicks | 12 | 255 |
| C26 Grey Lord | atkTicks | 22 | 255 |

(Table shows representative changes; 87 total fields corrected.)

### Attributes corrections (16 creatures)

All 27 creatures' attributes fields now use the exact hex value from the PC 3.4
binary at G0243 offset 2–3. Previously, many used symbolic mask expressions
derived from Atari ST ReDMCSB values. Key changes:

| Creature | Old | New (PC 3.4) | New flags decoded |
|----------|-----|--------------|-------------------|
| C01 Swamp Slime | 0x0004 | 0x0480 | KEEP_THROWN_SHARP |
| C03 Wizard Eye | 0x0434 | 0x04B4 | SIDE_ATTACK, ATTACK_ANY, LEVITATION, KEEP_THROWN |
| C05 Ruster | 0x000C | 0x0581 | SIDE_ATTACK, PREFER_BACK |
| C08 Ghost | 0x0060 | 0x5864 | SIDE_ATTACK, LEVITATION, NON_MATERIAL, SEE_INVISIBLE, NIGHT_VISION |
| C11 Black Flame | 0x0040 | 0x18C6 | SIDE_ATTACK, NON_MATERIAL, SEE_INVISIBLE, NIGHT_VISION |
| C13 Couatl | 0x002C | 0x14A2 | SIDE_ATTACK, LEVITATION, NIGHT_VISION |
| C14 Vexirk | 0x0020 | 0x05B8 | PREFER_BACK, LEVITATION |
| C17 Giant Wasp | 0x0420 | 0x04A0 | LEVITATION, KEEP_THROWN |
| C19 Materializer | 0x0860 | 0x4060 | LEVITATION, NON_MATERIAL |
| C20 Water Elemental | 0x0040 | 0x10DE | SIDE_ATTACK, PREFER_BACK, ATTACK_ANY, NON_MATERIAL, NIGHT_VISION |
| C21 Oitu | 0x0000 | 0x0082 | SIZE=1 (half-square) |
| C22 Demon | 0x0000 | 0x1480 | KEEP_THROWN, NIGHT_VISION |
| C23 Lord Chaos | 0x3830 | 0x78AA | PREFER_BACK, LEVITATION, SEE_INVISIBLE, NIGHT_VISION, ARCHENEMY |
| C25 Lord Order | 0x3830 | 0x78AA | (same as Lord Chaos) |
| C26 Grey Lord | 0x3830 | 0x78AA | (same as Lord Chaos) |

## Verification

Build succeeds. All creature/combat tests pass. The attributes field is read
via bitmask operations (`profile->attributes & CREATURE_ATTR_MASK_*`) so the
additional bits set in the PC 3.4 binary are correctly propagated.

## Conclusion

The creature info table now matches the PC 3.4 FIRES.EXE binary byte-for-byte
for all decoded fields (combat stats and attributes). The remaining 13 bytes
per entry (offsets 13–25) contain sightRange, smellRange, attackType, and
woundProbabilities which require further decoding.
