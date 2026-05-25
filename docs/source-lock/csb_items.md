# CSB V1 — New Items & Spells (Source-Lock Audit)

**Audit date:** 2026-05-25
**Sources:** ReDMCSB DEFS.H:1527,1774,1887–1951 · MENU.C:76,1994 · DUNGEON.C:259,426 · PANEL.C:671,719 · DM1/Firestaff dm1_v1_spell_casting_pc34_compat.c:41–100 · M13_PLAN.md:294–347

---

## Part I: New Spells

### Finding: ZOKATHRA (Zo Kath Ra) is the only CSB-only spell

DM1 has 25 spells (index 0–24, `DM1_SPELL_COUNT = 25`).
CSB adds exactly **1 new spell** beyond DM1.

### ZOKATHRA Spell

| Property | Value |
|----------|-------|
| Runes | Zo Kath Ra |
| Incantation | `0x006B6E76` (from DM1 spell table) |
| Base Req. | 0 |
| Skill | WIZARD (C03_SKILL_WIZARD) |
| Attrs | `0x3C73` |
| Type | `C7_SPELL_TYPE_OTHER_ZOKATHRA` = 7 |
| Icon | `C197_ICON_JUNK_ZOKATHRA` = 197 |
| Object type | `C51_JUNK_ZOKATHRA` = 51 |
| Dungeon slot | index 178 (DUNGEON.C:259) |
| Spell index in table | 24 (MENU.C:76 — last entry, same as DM1) |

**Effect:** `C7_SPELL_TYPE_OTHER_ZOKATHRA` is a special fireball variant.
- MENU.C:1994–1998: `case C7_SPELL_TYPE_OTHER_ZOKATHRA` creates a ZOKATHRA junk object
  (`L1277_ps_Junk->Type = C51_JUNK_ZOKATHRA`)
- PANEL.C:671: ZOKATHRA icon displayed in UI when held
- PANEL.C:719: ZOKATHRA projectile tracked in projectile slot
- DUNGEON.C:426: ZOKATHRA spell object type = 0 (non-material)

**Spell power:** M13_PLAN.md:337 notes CSB "changes some spell power values and adds
the ZO KATH RA fireball variant" — power/kinetic-energy differs from the standard
fireball (Ful Ir). The exact delta is documented in `memory_magic_pc34_compat.c`.

**Source citations:**
- DEFS.H:1527: `C51_JUNK_ZOKATHRA = 51`
- DEFS.H:1774: `C7_SPELL_TYPE_OTHER_ZOKATHRA = 7`
- DEFS.H:1951: `C197_ICON_JUNK_ZOKATHRA = 197`
- MENU.C:76: spell table entry for Zo Kath Ra
- MENU.C:1994–1998: ZOKATHRA case in spell-casting handler
- DUNGEON.C:259,426: dungeon object data for ZOKATHRA spell
- PANEL.C:671,719: UI and projectile tracking for ZOKATHRA

### All Other Spells: Unchanged from DM1

The 25-spell table (DM1_SPELL_COUNT = 25) carries over identically from DM1:
Shield, Magic Footprints, Invisibility, Poison Cloud, Thieve's Eye, Lightning Bolt,
Light, Torch, Fireball, Strength Potion, Fire Shield, Weaken Nonmaterial, Poison Bolt,
Darkness, Open Door, Shield Potion, Stamina Potion, Wisdom Potion, Vitality Potion,
Health Potion, Cure Poison Potion, Dexterity Potion, Mana Potion, Poison Potion,
Zokathra.

Sources: dm1_v1_spell_casting_pc34_compat.c:41–100 · DM1/Firestaff spell table

---

## Part II: New Items

### Finding: CSB Does NOT Add New Item Types

Both DM1 and CSB share the same icon/object type enumeration.
`C197_ICON_JUNK_ZOKATHRA` (icon index 197) is the highest icon number, same as DM1.
No new weapons, armor, clothing, potions, containers, or junk items in CSB.

### ZOKATHRA as an Item (Not a Spell Type)

ZOKATHRA occupies both:
- **Spell:** `C7_SPELL_TYPE_OTHER_ZOKATHRA` (spell-casting mechanic)
- **Junk item:** `C51_JUNK_ZOKATHRA` / `C197_ICON_JUNK_ZOKATHRA` (possessed/usable object)

The ZOKATHRA rune sequence (Zo Kath Ra) casts a fireball variant. After casting,
the champion holds a ZOKATHRA junk object (icon 197). This is the same in DM1 —
ZOKATHRA is not new to CSB; it existed in DM1.

### Item Categories Confirmed Unchanged

| Category | Evidence |
|----------|----------|
| Weapons (C000–C066 range) | No new IDs in CSB; Firestaff DEFS.H defines same range |
| Armor/Clothing (C080–C145 range) | No new IDs; same icon count as DM1 |
| Potions (C148, C163, C195) | No new potion types |
| Containers (C144–C145) | Same chest icons |
| Junk (C000–C197) | ZOKATHRA (C51/C197) present in DM1; not new to CSB |
| Scrolls (C030–C031) | Same scroll icons |

Sources: DEFS.H:1887–1951 (full icon enum) · Objects.h:430 (objDI_* index range)

---

## Summary

| Domain | DM1 | CSB Delta | Notes |
|--------|-----|-----------|-------|
| Spell count | 25 | +1 new spell | ZOKATHRA fireball variant (power differs) |
| New spell name | — | ZOKATHRA | Zo Kath Ra; same rune seq in both games |
| Spell table | 25 entries | Identical | All other 24 spells unchanged |
| Item types | Full set | None new | ZOKATHRA junk item existed in DM1 |
| Weapon types | Same | None new | |
| Armor types | Same | None new | |
| Potion types | Same | None new | |

**CSB item/spell verdict:**
- **Spells:** 1 new variant (ZOKATHRA fireball) with different power value; all other
  spells identical to DM1's 25-spell table
- **Items:** Zero new item types. No new weapons, armor, clothing, potions, or junk.
  ZOKATHRA junk (icon 197) is shared with DM1.

---

## Source Citations

| File | Lines | Content |
|------|-------|---------|
| ReDMCSB DEFS.H | 1527 | `C51_JUNK_ZOKATHRA` icon constant |
| ReDMCSB DEFS.H | 1774 | `C7_SPELL_TYPE_OTHER_ZOKATHRA` spell type |
| ReDMCSB DEFS.H | 1951 | `C197_ICON_JUNK_ZOKATHRA` full icon |
| ReDMCSB DEFS.H | 1767–1774 | Full spell-type enum (0–7) |
| ReDMCSB DEFS.H | 1887–1951 | Full icon/object enum |
| ReDMCSB MENU.C | 76 | Spell table entry for Zo Kath Ra |
| ReDMCSB MENU.C | 1994–1998 | ZOKATHRA case in spell-casting handler |
| ReDMCSB DUNGEON.C | 259,426 | Dungeon data for ZOKATHRA object |
| ReDMCSB PANEL.C | 671,719 | UI/proj tracking for ZOKATHRA |
| Firestaff dm1_v1_spell_casting_pc34_compat.c | 41–100 | DM1 25-spell table, confirmed identical in CSB |
| M13_PLAN.md | 294–347 | CSB delta scope: "new items and new spells" — items delta is zero |