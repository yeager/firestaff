# CSB V1 - GAP: Champion Implementation Gaps

**File:** `docs/csb_gap_champions.md`
**Audit:** Firestaff CSB V1 Audit Runner
**Date:** 2026-05-25
**Reference:** `docs/source-lock/csb_champions.md`

---

## Executive Summary

CSB champions are DM1 with 3 significant deltas:
- **NEOPHYTE rank** (skill level 0) - new rank below NOVICE
- **Reincarnation penalty** - CHANGE7_24: HP/MP/STA halved, other stats -1/8th (Luck exempt)
- **UI improvements** - left-click on champion bars opens inventory, expanded click zones

No new classes, no new skills, no new stat types.

---

## GAP 1: NEOPHYTE Skill Rank

**What source-lock says:**
- Chaos.cpp:515-530 - pnt4772[16] rank name table includes NEOPHYTE at index 0
- data.cpp:88 - neophyteSkills global defaults to false
- Recording.cpp:246 - neophyteSkills set true in neophyte mode replay
- Character.cpp:665 - allows skill level 0 (NEOPHYTE) as valid when neophyteSkills or D4W > 0
- Monster.cpp:321 - neophyteSkills affects door type check

**Implementation gap:**
No NEOPHYTE rank in current Firestaff champion system.
Need:
1. Add NEOPHYTE = 0 to SkillRank enum (current lowest is NOVICE = 1)
2. neophyteSkills global - controls whether skill level 0 is accepted as valid
3. Skill validation: if (neophyteSkills || skillLevel > 0) accept else reject
4. neophyteSkills = true when replaying recordings saved in neophyte mode
5. Update skill progression display/UI for 17 ranks (NEOPHYTE..MASTER)

**Source:** Chaos.cpp:515-530 · data.cpp:88 · Recording.cpp:246 · Character.cpp:665 · Monster.cpp:321

---

## GAP 2: Reincarnation Penalty System (CHANGE7_24)

**What source-lock says:**
- CSB:REVIVE.C CHANGE7_24: New reincarnation rules
- Character.cpp:14 - three new globals:
  - reincarnateAttributePenalty (default 2, max 16)
  - reincarnateStatPenalty (default 8, max 16)
  - randomPoints (default 3, max 25)
- Character.cpp:682-687 - globals loaded from save game data

| Stat | DM1 | CSB CHANGE7_24 |
|------|-----|----------------|
| Current Health | Preserved | Halved |
| Current Mana | Preserved | Halved |
| Current Stamina | Preserved | Halved |
| Max Health | Preserved | -1/8th of current value |
| Max Mana | Preserved | -1/8th of current value |
| Max Stamina | Preserved | -1/8th of current value |
| Strength/Max | Preserved | -1/8th |
| Dexterity/Max | Preserved | -1/8th |
| Wisdom/Max | Preserved | -1/8th |
| AntiFire/Max | Preserved | -1/8th |
| AntiMagic/Max | Preserved | -1/8th |
| Luck | Preserved | No change |

**Implementation gap:**
No reincarnation penalty in current Firestaff (DM1 preserves all stats).
Need:
1. Add three champion globals: reincarnateAttributePenalty, reincarnateStatPenalty, randomPoints
2. Load/save these in champion serialization (CSBGAME.DAT format)
3. Implement REVIVE.C CHANGE7_24 logic:
   - Halve current HP, MP, STA
   - Reduce max stats by floor(current * penalty / 8) for each stat
   - Luck completely exempt; respect minimums for all stats
4. randomPoints: on reincarnation, champion receives random bonus stat points

**Source:** CSB:REVIVE.C (CHANGE7_24) · Character.cpp:14,682-687 · csb_champions.md Part I.2

---

## GAP 3: Champion Transfer/Import (Hall of Champions Delta)

**What source-lock says:**
- DM1 Hall of Champions: dead champions stored and transferable between games
- CSB: CHANGE7_29 - new champion save format (CSBGAME.DAT vs DMSAVE.DAT)
- CSB adds reincarnateAttributePenalty, reincarnateStatPenalty, randomPoints to save format

**Implementation gap:**
Firestaff DM1 Hall of Champions imports champions with full stat preservation.
CSB requires:
1. CSBGAME.DAT format reader - different header layout than DMSAVE.DAT
2. Import champion from CSB save: parse new globals, apply to imported champion
3. Export champion to CSB save: serialize new globals
4. Hall of Champions must handle both DM1 (DMSAVE.DAT) and CSB (CSBGAME.DAT) formats

**Source:** CEDTINC8.C:101-118 · READWRIT.C · SAVEHEAD.C · LOADSAVE.C (CHANGE7_29)

---

## GAP 4: Left-Click Inventory Access (CHANGE7_28)

**What source-lock says:**
- DEFS.H:327-330 - new command constants:
  - C125_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_LEFT
  - C126_COMMAND_CLICK_ON_CHAMPION_ICON_TOP_RIGHT
  - C127_COMMAND_CLICK_ON_CHAMPION_ICON_BOTTOM_RIGHT
  - C128_COMMAND_CLICK_ON_CHAMPION_ICON_BOTTOM_LEFT
- COMMAND.C CHANGE7_28 - left-click on champion portrait/stat bars opens inventory
- DEFS.H:226 - C33_MOUSE_EVENT_LEAVE_CHAMPION_ICON_REGION mouse exit event

**Implementation gap:**
Current Firestaff requires right-click or menu nav for champion inventory.
Need:
1. Map champion icon quadrants to C125-C128 commands
2. COMMAND.C handler: on C125-C128, open champion inventory panel
3. DEFS.H:226 - mouse leave event for champion icon region (hover detection)
4. Remove or deprioritize right-click-only requirement for inventory access

**Source:** DEFS.H:226,327-330 · COMMAND.C (CHANGE7_28) · csb_champions.md Part II

---

## GAP 5: Champion Bug Fixes

**What source-lock says:**
| Bug | File | Description |
|-----|------|-------------|
| BUG0_37 | CHAMPION.C CHANGE7_25_FIX | Champion stat/combat bug |
| BUG0_46 | CHAMPION.C CHANGE8_07_FIX | Champion handling bug |
| BUG0_50 | CLIKMENU.C CHANGE8_10_FIX | Champion click handling |
| BUG0_51 | CLIKVIEW.C CHANGE8_11_FIX | Champion view click |

**Implementation gap:**
These are behavior corrections. Audit current Firestaff champion code:
1. BUG0_37 - champion stat/combat: check stat mutation edge cases
2. BUG0_46 - champion handling: check creation/deletion/loading paths
3. BUG0_50 - click menu: check champion context menu click handlers
4. BUG0_51 - click view: check champion view panel click handlers

**Source:** BugsAndChanges.htm (CHANGE7_25,8_07,8_10,8_11)

---

## Summary Table

| Gap | Severity | Description |
|-----|----------|-------------|
| NEOPHYTE rank (skill level 0) | HIGH | New rank below NOVICE; neophyteSkills global |
| Reincarnation penalty (CHANGE7_24) | HIGH | HP/MP/STA halved, -1/8th stats except Luck; 3 new globals |
| Champion transfer (CSB format) | HIGH | CSBGAME.DAT format; new globals in save; dual-format support |
| Left-click inventory (CHANGE7_28) | MEDIUM | C125-C128 commands; champion icon quadrant click |
| Champion icon mouse exit event | LOW | DEFS.H:226 C33 event for hover detection |
| BUG0_37,46,50,51 fixes | MEDIUM | Behavior corrections in champion code paths |

---

## Reference Sources

| Source | Content |
|--------|---------|
| docs/source-lock/csb_champions.md | Existing source-lock audit (primary) |
| CSBWin Chaos.cpp:515-530 | pnt4772[16] - NEOPHYTE rank name table |
| CSBWin data.cpp:88 | neophyteSkills global |
| CSBWin Recording.cpp:246 | neophyteSkills = true in neophyte mode |
| CSBWin Character.cpp:665,14,682-687 | neophyteSkills usage, penalty globals |
| CSB:REVIVE.C (CHANGE7_24) | Reincarnation rules |
| CEDTINC8.C:101-118 | CSBGAME.DAT vs DMSAVE.DAT routing |
| BugsAndChanges.htm | CHANGE7_24,25,28,29,8_07,8_10,8_11 |
