# CSB V1 - Champion Evolution Audit

## Source Paths
- CSB: /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src/Character.cpp, Champion.cpp
- CSB: /home/trv2/.openclaw/data/firestaff-csb-source/CSB/src/Magic.cpp, Statistics.cpp
- DM1: ReDMCSB CHAMPION.C, BugsAndChanges.htm

## Key CSB Champion Additions vs DM1

### 1. New Reincarnation Rules (CHANGE7_24_IMPROVEMENT)
- File: CSB:REVIVE.C (corresponding to DM1:REVIVE.C)
- DM1: Standard death/reincarnation with full stat preservation
- CSB CHANGE7_24: New reincarnation rules:
  - Health, Mana and Stamina values are halved
  - Current and maximum values of each statistic EXCEPT Luck
    are decreased by 1/8th of their value
  - No statistic goes below its minimum value
- **This is a significant champion evolution change**

### 2. New Saved Game Header Format (CHANGE7_29_IMPROVEMENT)
- Files: READWRIT.C, SAVEHEAD.C, LOADSAVE.C, DEFS.H
- DM1: Original save header format
- CSB: New header format for saved games
- CHANGE8_12_FIX: Related save/load fixes in READWRIT.C, LOADSAVE.C

### 3. Left Click on Champion Bars Opens Inventory (CHANGE7_28_IMPROVEMENT)
- Files: COMMAND.C, DEFS.H
- DM1: Required right-click or menu navigation
- CSB: Left click on champion bars opens their inventory directly
- UX improvement, not a stat/ability change

### 4. UI Panel Click Target Expansion
- CHANGE7_28: Champion portrait/stat bars now clickable for inventory access
- No new champion abilities, skills, or stats

### 5. Bug Fixes Affecting Champions
- CHANGE7_25_FIX: Fixed in CHAMPION.C (BUG0_37)
- CHANGE8_07_FIX: Fixed in CHAMPION.C (BUG0_46)
- CHANGE8_10_FIX: Fixed in CLIKMENU.C (BUG0_50) - click handling
- CHANGE8_11_FIX: Fixed in CLIKVIEW.C (BUG0_51) - view click handling

## Champion Stats/Abilities: No New Types

- No new skill types added in CSB
- No new statistic types
- No new champion classes or advancement changes
- Magic system: same spells as DM1 (grep C5_ATTACK_MAGIC sources same set)
- No new action types beyond what DM1 supported

## Character Files
- CSB Statistics.cpp defines NUM_MONSTER_TYPE = 27 (same as DM1)
- No champion-specific statistics changes

## Conclusion
CSB champion evolution is MINOR compared to DM1:
1. New reincarnation rules (halved HP/MP/STA, -1/8th other stats except Luck)
2. New save game header format
3. Left-click inventory access from champion bars
No new abilities, skills, or character advancement changes.
