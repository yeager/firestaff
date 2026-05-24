# DM1 V1 — Magic System

## Source
ReDMCSB: `CASTER.C` (F0394–F0398), `MENU.C:44–76` (spell tables), `MENU.C:1633–2041` (F0408, F0412),
`SYMBOL.C:10–27`, `CHAMPION.C:2330–2355`, `DEFS.H` spell constants.

---

## 1. Spellcasting Overview — 4-Step Symbol Sequence

The player selects a champion as "magic caster" and enters 4 symbols in sequence:

| Step | Examples | Role |
|------|----------|------|
| 0 — Power | Lo, Um, On, Ee, Pal, Mon | Spell tier / power level |
| 1 — Element | Ya, Vi, Oh, Ful, Des, Zo | Most spells start here |
| 2 — Class | Ven, Ew, Kath, Ir, Bro, Gor | Effect sub-class |
| 3 — Alignment | Ku, Ros, Dain, Neta, Ra, Sar | Final refinement |

---

## 2. Spell Point (Mana) Consumption

**Base cost table** — `MENU.C:44–48`, `G0485_aauc_Graphic560_SymbolBaseManaCost[4][6]`:
```
{ 1,2,3,4,5,6 }   // Step 0 — Power
{ 2,3,4,5,6,7 }   // Step 1 — Element
{ 4,5,6,7,7,9 }   // Step 2 — Class
{ 2,2,3,4,6,7 }   // Step 3 — Alignment
```

**Multiplier** — `MENU.C:49`, `G0486_auc_Graphic560_SymbolManaCostMultiplier[6]`:
```
{ 8, 12, 16, 20, 24, 28 }  // indexed by first symbol - 96
effectiveCost = (baseCost * multiplier[firstSymbolIndex]) >> 3
```

**Deduction:** if `manaCost <= champion.CurrentMana`, then `CurrentMana -= manaCost`.

**25 spells** in DM1 V1 spell table (`MENU.C:50–76`, `G0487_as_Graphic560_Spells[25]`).
Each spell: Symbols (4-byte packed) + BaseRequiredSkillLevel + SkillIndex + Attributes.

---

## 3. Minimum Requirements to Cast

`F0412_MENUS_GetChampionSpellCastResult()` — `MENU.C:1811–1849`

Spell fails if:
1. Champion is dead or absent
2. `CurrentMana < manaCost`
3. Skill level (with object modifiers, no temp XP) < `baseRequiredSkillLevel`
4. Spell needs flask but champion has no flask in pack

---

## 4. Mana Regeneration

**Source: CHAMPION.C:2335–2355** — `F0297_CHAMPION_ProcessTick` (part of per-tick processing)

```
timeCriteria = ((gameTime & 0x0080) + ((gameTime & 0x0100) >> 2) + ((gameTime & 0x0040) << 2)) >> 2
wizardPriestLevel = GetSkillLevel(champion, WIZARD) + GetSkillLevel(champion, PRIEST)

if (CurrentMana < MaximumMana && timeCriteria < (wisdom + wizardPriestLevel)):
    manaGain = (MaximumMana / 40) + 1         // S12E+ (resting: * 2)
    staminaCost = manaGain * max(7, 16 - wizardPriestLevel)
    DecrementStamina(champion, staminaCost)
    CurrentMana += min(manaGain, MaximumMana - CurrentMana)
```

Higher Wisdom + Wizard/Priest skill → more frequent mana regen ticks.

---

## 5. Spell Effects

**Projectile spells** (`F0327_CHAMPION_IsProjectileSpellCast`, `CHAMPION.C:2073`):
- Fireball, Lightning Bolt, Poison Bolt launch from champion
- Kinetic energy = bounded(21, (powerSymbol + 2) * (4 + skillLevel*2), 255)
- Travel through dungeon, checked for collision each tick

**Non-projectile spells:**
| Spell | Effect |
|-------|--------|
| SHIELD (Ya Ir) | +shield defense event |
| LIGHT (Oh Ir Ra) | Light event, duration per power symbol |
| INVISIBILITY (Oh Ew Sar) | Reduces creature detection |
| FIRE SHIELD (Ful Bro Neta) | Fire attack shield |
| OPEN DOOR (Zo) | Removes door/trigger from target square |
| DARKNESS (Des Ir Sar) | Reduces local light |
| MAGIC FOOTPRINTS (Ya Bro Ros) | Shows party path |

**Heal cycles** (`MENU.C:1512–1517`, S12E+):
```
cycles = min(CurrentMana/2, MissingHealth / min(10, HealSkill))
heal = min(MissingHealth, min(10, HealSkill)) * cycles
manaCost = 2 * cycles
XP = 2 + 2 * cycles
```

---

## 6. Zokathra (Zo Kath Ra) — The Endgame Spell

Zo Kath Ra (spell index 24, DM1_SPELL_COUNT-1):
- Symbols: Zo + Kath + Ra (3 runes)
- BaseRequiredSkillLevel: 0 (anyone can attempt)
- Skill: WIZARD

Used as part of the Fuse action with Firestaff Complete. See endgame docs.

---

## 7. Firestaff Implementation

**File:** `src/dm1/dm1_v1_spell_casting_pc34_compat.c`
- `dm1_symbolBaseManaCost[4][6]` — verbatim from `MENU.C:44`
- `dm1_symbolManaCostMultiplier[6]` — verbatim from `MENU.C:49`
- `dm1_spells[25]` — all 25 DM1 V1 spells
- `dm1_spell_cast()` — implements F0412 spell resolution
- Symbol step tracking via `SymbolStep` (0..3 cycling)

**Gaps identified:** Spell effect on world (projectile collision, shield application,
light propagation) — listed as DM1_V1_PLAN.md Phase 3 items 10–13.

**Parity status:** FULL for spell table and symbol cost formula.
