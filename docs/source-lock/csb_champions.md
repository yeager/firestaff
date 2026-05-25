# CSB V1 — Champion Changes (Source-Lock Audit)

**Audit date:** 2026-05-25
**Sources:** ReDMCSB DEFS.H:757–768, C00–C03_SKILL_* · CHAMPION.C · CSBWin Character.cpp:14,665–686 · Chaos.cpp:515–530 · CSBWin Recording.cpp:246 · CSBWin data.cpp:88 · REVIVE.C · CLIKMENU.C · BugsAndChanges.htm

---

## Finding: No New Champion Classes or Skills in CSB

CSB carries forward DM1's 4 base classes and 20 skill system exactly:

| Class | Index | Hidden Skills |
|-------|-------|---------------|
| Fighter | C00 | Swing, Thrust, Club, Parry |
| Ninja | C01 | Steal, Fight, Throw, Shoot |
| Priest | C02 | Identify, Heal, Influence, Defend |
| Wizard | C03 | Fire, Air, Earth, Water |

Source: DEFS.H:757–768 · CHAMPION.C · dm1_v1_spell_casting_pc34_compat.h:22–36

---

## Part I: New Champion Mechanics in CSB

### 1. New Champion Ranks — "NEOPHYTE" Skill Level (NEW)

DM1 skill levels: NOVICE, APPRENTICE, JOURNEYMAN, CRAFTSMAN, ARTISAN, ADEPT, EXPERT, ` MASTER, a MASTER, ...
CSB adds a rank **below** NOVICE:

| Rank | Index | Notes |
|------|-------|-------|
| NEOPHYTE | 0 | **CSB-only** — new lowest rank |
| NOVICE | 1 | DM1's lowest rank |
| APPRENTICE | 2 | |
| ... | ... | |

Source: Chaos.cpp:515–530 · `pnt4772[16]` rank name table

`neophyteSkills` global (data.cpp:88):
- Default `false`
- Set `true` in Recording.cpp:246 when replaying a recording in "neophyte mode"
- Affects skill parsing in Character.cpp:665:
  `if (((D4W > 0) || neophyteSkills) && (D4W <= 17))` — allows skill level 0 (NEOPHYTE)
  as valid even in non-neophyte context

Also affects Monster.cpp:321 — door type check when `neophyteSkills == true`.

This allows CSB recordings saved in neophyte mode to be replayed and validated.

### 2. New Reincarnation Rules (CHANGE7_24_IMPROVEMENT) — Significant

CSB modifies death/reincarnation mechanics:

| Stat | DM1 Behavior | CSB CHANGE7_24 Behavior |
|------|-------------|------------------------|
| Current Health | Preserved | **Halved** |
| Current Mana | Preserved | **Halved** |
| Current Stamina | Preserved | **Halved** |
| Max Health | Preserved | **−1/8th of current value** |
| Max Mana | Preserved | **−1/8th of current value** |
| Max Stamina | Preserved | **−1/8th of current value** |
| Strength/Max | Preserved | **−1/8th** |
| Dexterity/Max | Preserved | **−1/8th** |
| Wisdom/Max | Preserved | **−1/8th** |
| AntiFire/Max | Preserved | **−1/8th** |
| AntiMagic/Max | Preserved | **−1/8th** |
| Luck/Max | Preserved | **No change (Luck exempted)** |
| All minimums | Respected | Respected |

Source: CSB:REVIVE.C (CHANGE7_24) · csb_champions.md (existing audit)

### 3. New Champion Character Definition Globals (CSB-only)

Three new globals in Character.cpp:14:
```c
ui8 reincarnateAttributePenalty,  // default 2, max 16 — attribute tier loss on reincarnate
     reincarnateStatPenalty,       // default 8, max 16 — stat point loss
     randomPoints;                 // default 3, max 25 — random bonus points
```
These are loaded from save game / champion definition data and control the reincarnation
penalty scaling. DM1 does not have these globals — it uses hardcoded values.

Source: Character.cpp:14,682–687

---

## Part II: UI / Interaction Changes

### Left Click on Champion Bars Opens Inventory (CHANGE7_28_IMPROVEMENT)

DM1: Required right-click or menu navigation to access champion inventory.
CSB: Left-click on champion portrait/stat bars opens their inventory directly.

| File | Change |
|------|--------|
| COMMAND.C | New click handler for champion bar click |
| DEFS.H | New command constants: C125–C128 for champion icon quadrants |

C125_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_LEFT
C126_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_RIGHT
C127_COMMAND_CLICK_ON_CHAMPION_ICON_BOTTOM_RIGHT
C128_COMMAND_CLICK_ON_CHAMPION_ICON_BOTTOM_LEFT

Source: COMMAND.C (CHANGE7_28) · DEFS.H:327–330

### Champion UI Zone Expansion

CSB adds more clickable zones for champion interaction in the panel.
DEFS.H:226 defines `C33_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION` — mouse exit event
for champion icon regions, allowing hover detection in CSB (not in DM1).

Source: DEFS.H:226,327–330

---

## Part III: Champion Bug Fixes

| Bug | File | Description |
|-----|------|-------------|
| BUG0_37 | CHAMPION.C (CHANGE7_25_FIX) | Champion stat/combat bug fix |
| BUG0_46 | CHAMPION.C (CHANGE8_07_FIX) | Champion handling bug fix |
| BUG0_50 | CLIKMENU.C (CHANGE8_10_FIX) | Champion click handling |
| BUG0_51 | CLIKVIEW.C (CHANGE8_11_FIX) | Champion view click |

Source: BugsAndChanges.htm

---

## Part IV: Save Game Champion Data (CHANGE7_29)

CSB uses a new champion/save game header format (`CSBGAME.DAT` vs `DMSAVE.DAT`).
The champion data serialization format includes the new `reincarnateAttributePenalty`,
`reincarnateStatPenalty`, and `randomPoints` fields.

Source: READWRIT.C, SAVEHEAD.C, LOADSAVE.C (CHANGE7_29)

---

## Summary

| Champion Feature | DM1 | CSB Delta | Type |
|-----------------|-----|-----------|------|
| Base classes | 4 (Fighter/Ninja/Priest/Wizard) | Identical | — |
| Skill system | 20 skills (C00–C19) | Identical | — |
| Skill ranks | NOVICE..MASTER | +NEOPHYTE rank (lowest) | New |
| Reincarnation | Full stat preservation | HP/MP/STA halved, −1/8th other stats except Luck | **Significant** |
| Reincarnation penalty scaling | Hardcoded | New globals: penalty/stat/randomPoints | New |
| Inventory access | Right-click | Left-click on champion bars | UI improvement |
| Champion icon regions | Basic | C33 mouse event for icon region exit | New |
| Champion click commands | — | C125–C128 quadrant click commands | New |
| Champion bug fixes | — | BUG0_37,46,50,51 fixes | Bug fixes |

**Verdict:** CSB champion evolution is limited to:
1. **New NEOPHYTE rank** in skill progression
2. **Significant reincarnation penalty** (halved HP/MP/STA, −1/8th other stats)
3. **UI improvements** (left-click inventory, expanded click zones)
4. Bug fixes

No new classes, no new skills, no new stat types, no level-up formula changes.

---

## Source Citations

| File | Lines | Content |
|------|-------|---------|
| ReDMCSB DEFS.H | 757–768 | Class/skill constants |
| ReDMCSB DEFS.H | 226,327–330 | Mouse event, champion click commands |
| CSBWin Chaos.cpp | 515–530 | pnt4772[16] — NEOPHYTE rank name |
| CSBWin data.cpp | 88 | `neophyteSkills = false` global |
| CSBWin Recording.cpp | 246 | `neophyteSkills = true` in neophyte mode |
| CSBWin Character.cpp | 14,665–687 | neophyteSkills usage, penalty globals |
| CSBWin Monster.cpp | 321 | neophyteSkills door check |
| CSBWin CLIKMENU.C | CHANGE8_10 | Bug0_50 fix |
| CSBWin CLIKVIEW.C | CHANGE8_11 | Bug0_51 fix |
| CSB:REVIVE.C | CHANGE7_24 | Reincarnation rules |
| BugsAndChanges.htm | CHANGE7_24,25,28,29,8_07,8_10,8_11 | All champion changes |