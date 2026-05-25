# DM1 V1 Champion Creation — Source Lock

Source: ReDMCSB_WIP20210206/Toolchains/Common/Source/REVIVE.C
Primary function: F0280_CHAMPION_AddCandidateChampionToParty (line 63)

## Champion Creation Flow

### 1. Entry Point
A candidate champion is added via a portrait sensor in the dungeon. When the party steps on the mirror square, MOVESENS.C:1502 calls F0280_CHAMPION_AddCandidateChampionToParty(portraitIndex).

### 2. Data Source — Champion Definition Text
The champion attributes are stored in a text string in the dungeon, parsed line-by-line:
- Line 1: Champion name (null-terminated string, variable length)
- Line 2: Champion title (variable length, stored as two parts separated by newline)
- Line 3: Single character M or F for gender
- Line 4: 4-digit encoded CurrentHealth (= MaximumHealth)
- Line 5: 4-digit encoded CurrentStamina (= MaximumStamina)
- Line 6: 4-digit encoded CurrentMana (= MaximumMana)
- Line 7: 7 pairs of 2-digit encoded statistics (Luck through Antifire)
- Line 8+: 16 skill levels as single uppercase letters (A through O)

Encoding: Each digit is stored as digit + A - 1 (so 1 -> A, 9 -> I, 0 -> J)
F0279_CHAMPION_GetDecodedValue reverses this.

Source: REVIVE.C:165-180

### 3. Starting Statistics

All 7 statistics have three values: [C0_MAXIMUM][C1_CURRENT][C2_MINIMUM]

| Stat        | Index                        | Minimum |
|-------------|------------------------------|---------|
| Luck        | C0_STATISTIC_LUCK            | 10      |
| Strength    | C1_STATISTIC_STRENGTH        | 30      |
| Dexterity   | C2_STATISTIC_DEXTERITY        | 30      |
| Wisdom      | C3_STATISTIC_WISDOM           | 30      |
| Vitality    | C4_STATISTIC_VITALITY         | 30      |
| Antimagic   | C5_STATISTIC_ANTIMAGIC        | 30      |
| Antifire    | C6_STATISTIC_ANTIFIRE         | 30      |

Source: DEFS.H:744-750, REVIVE.C:172-184

On creation: Current = Maximum = decoded value; Minimum = 30 (Luck=10)

### 4. Starting Vitals

- Health: set from decoded value, current = max
- Stamina: set from decoded value, current = max
- Mana: set from decoded value, current = max
- Food: 1500 + RANDOM(256) (range 1500-1755)
- Water: 1500 + RANDOM(256) (range 1500-1755)

Source: REVIVE.C:122-123, 165-169

### 5. Starting Skills

Skill levels are encoded as letters A-O:
- A -> Experience = 0 (level 0)
- B -> Experience = 125 (level 1)
- C -> Experience = 250 (level 2)
- ...
- O -> Experience = 1750 (level 14)

Formula: Experience = 125L << (letter - A) for letter > A
Formula: Experience = 0 for letter == A

Source: REVIVE.C:186-197, DEFS.H:757-768

16 skills (C04-C19): Swing, Thrust, Club, Parry, Steal, Fight, Throw, Shoot, Identify, Heal, Influence, Defend, Fire, Air, Earth, Water

4 base classes (C00-C03): Fighter, Ninja, Priest, Wizard
- Base class experience = sum of its 4 hidden skills >> 1 (averaged)

Source: REVIVE.C:199-270

### 6. Slots / Equipment

All 32 slots (C00_SLOT_READY_HAND through C31_SLOT_CHEST_2) initialized to C0xFFFF_THING_NONE.

Objects on the champion mirror square are auto-equipped:
- Weapon: placed in C01_SLOT_ACTION_HAND
- Armor: placed in appropriate slot (head/torso/legs/feet/neck)
- Scrolls/Potions: placed in C11_SLOT_POUCH_1, then C06_SLOT_POUCH_2
- Junk/Containers: placed in neck slot or backpack

Source: REVIVE.C:124-125, 289-362

### 7. Party Position

- Direction = current party direction
- Cell = first empty view cell in party formation
- Portrait copied from champion portraits bitmap

Source: REVIVE.C:111-119
