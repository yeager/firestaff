# DM1 V1 Champion Advancement — Source Lock

Source: ReDMCSB_WIP20210206/Toolchains/Common/Source/CHAMPION.C
Primary functions:
- F0303_CHAMPION_GetSkillLevel (line 736) — read skill level
- F0304_CHAMPION_AddSkillExperience (line 823) — add XP, level up

## Skill Level and Experience

### Skill Level Formula
Skill level is derived from accumulated experience:
- Loop: while Experience >= 500, Experience >>= 1 (halve)
- Level = number of times we halved until Experience < 500

Source: CHAMPION.C:766-767

### Experience System
Each skill has:
- Experience: permanent XP
- TemporaryExperience: temporary XP (decays over time)

When combat action is detected (>150 game ticks since last creature attack):
- For physical skills: base XP is computed, multiplied by map difficulty
- For skill level-ups: Vitality max increases by 0 or 1 (Priest) / (skill level % 2) (others)
- For skill level-ups: Antifire max may increase by 1 for even skill levels

Source: CHAMPION.C:753-906

### Level-Up Stat Bonuses

When skill level increases (F0304_CHAMPION_AddSkillExperience):

**Fighter class (C00_SKILL_FIGHTER):**
- Strength +Major, Dexterity +Minor (alternating per level)
- Major = 2, Minor = 1

**Ninja class (C01_SKILL_NINJA):**
- Strength +Minor, Dexterity +Major (alternating per level)
- Major = 2, Minor = 1

**Priest class (C02_SKILL_PRIEST):**
- Wisdom +Major or +Minor (alternating per level)
- Antifire +Random(3)

**Wizard class (C03_SKILL_WIZARD):**
- Wisdom +Minor or +Major (alternating per level)
- Antimagic +Random(4)

Source: CHAMPION.C:908-970

### Temporary Experience Decay

In the game loop (GAMELOOP.C), at regular intervals:
- TemporaryExperience decrements by 1 for all non-zero skills
- This represents skill rusting from disuse

Source: CHAMPION.C:2358-2359

### Map Difficulty Multiplier

When XP is awarded for combat actions, it is multiplied by the map difficulty:
- P0638_ui_Experience *= AL0915_ui_MapDifficulty

Source: CHAMPION.C:871

## No Character Levels

DM1 V1 does not have character levels. Champions do not level up in the RPG sense.
Advancement is purely skill-based, and skill levels derive directly from accumulated experience.
There is no separate experience pool, no level threshold, and no automatic stat jump.

## Statistic Ranges

Each statistic has 3 values: Maximum, Current, Minimum
- Maximum: ceiling (can be raised by leveling)
- Current: actual value (fluctuates with fatigue/hunger)
- Minimum: floor (cannot go below this; 30 for all except Luck=10)

Maximum values are raised by skill level-ups.
Current values can temporarily drop (e.g., stamina drain, poison) but are clamped to [Minimum, Maximum].
