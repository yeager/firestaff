# DM1 V1 - Champion Selection / Creation

## Source-Locked
ReDMCSB WIP20210206 - Toolchains/Common/Source/

---

## Champion Management (CEDT001.C)

Champion save/load functions:

| Function            | Line | Role                                |
|--------------------|------|-------------------------------------|
| F7001_SaveChampions | :132 | Write 4 champion CMP files to disk  |
| F7002_ReadCMP       | :182 | Read one CMP file from disk        |
| F7003_LoadPortrait  | :237 | Load and decompress portrait bitmap |
| F7004_LoadChampions | :292 | Load all 4 champions               |

---

## Champion Dialog System (CEDTDATA.C)

Skill-class names:
G0417_apc_BaseSkillNames[4] = { FIGHTER, NINJA, PRIEST, WIZARD }

Skill level names (15 levels):
G0428_apc_SkillLevelNames[15] = {
    NEOPHYTE, NOVICE, APPRENTICE, JOURNEYMAN,
    CRAFTSMAN, ARTISAN, ADEPT, EXPERT,
    MASTER, a MASTER, ... ARCHMASTER }

Stat labels:
- G7021_HEALTH  = HEALTH
- G7022_STAMINA = STAMINA
- G7023_MANA    = MANA

---

## Portrait Editing State (CEDT001.C globals)

- G7104_TextCursorVisible    - blinking cursor flag
- G7105_EditChampionNameOrTitleCharacterIndex - cursor column in name field
- G7106_EditChampionNameOrTitle - which field: 0=name, 1=title
- G7107_SelectedColorIndex - selected palette color for portrait
- G7108_BlinkingCursorTimer - cursor blink timing
- G7109_SelectedChampionIndex - which champion slot (0-3)
- G2329_s_UndoPortrait - undo buffer for portrait edits

---

## Selector Frontend Compatibility (selector_frontend_pc34_compat.c)

F8347_Compat expands compressed portrait graphics to bitmap via
IMG_Compat_ExpandToBitmapIfPresent.

Pass602b SELECTOR.C function citations:
- SELECTOR.C:91   F8316_I
- SELECTOR.C:154  F8323_W
- SELECTOR.C:202  F8328_G
- SELECTOR.C:539  F8353_W
- SELECTOR.C:545  F8354_A
- SELECTOR.C:569  F8356_A
- SELECTOR.C:597  F8358_D
- SELECTOR.C:609  F8359_G
- SELECTOR.C:710  F8363_G
- SELECTOR.C:716  F8364_D
- SELECTOR.C:759  F8365_L
- SELECTOR.C:789  F8366_L

---

## On-Disk Format

- ON_DISK_PORTRAIT G2248_as_OnDiskPortraits[4] - 4 champion portraits (CEDT001.C:27)
- BACKUP_PORTRAIT G2329_s_UndoPortrait - undo buffer (CEDT001.C:31)
- unsigned char* G2243_OnScreenPortraitBitmaps[4] - decompressed bitmaps (CEDT001.C:33)

---

## Summary

1. File picker - pick a .CMP champion file (CEDTDATA.C dialog)
2. Portrait + stats - display/edit portrait with 4 skill classes x 15 levels
3. Name/title edit - text input with blinking cursor (G7104-G7109)
4. Save/Load - F7001/F7004 write/read CMP files (CEDT001.C:132,292)
