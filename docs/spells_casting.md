# DM1 V1 - Spell Casting Mechanics
Audit date: 2026-05-25 | Source: ReDMCSB WIP20210206

---

## 1. Mana (Spell Point) System

### Maximum Mana
  MaximumMana = max(1, Wisdom * 5 + (WizardLevel + PriestLevel) * 3)
Source: CHAMPION.C:2330-2355, F0297_CHAMPION_ProcessTick

### Mana Cost Formula
Each symbol costs mana to inscribe. Cost increases with power level.

Base cost (MENU.C:44-48, G0485):
  Step 0 Power:   1,  2,  3,  4,  5,  6
  Step 1 Element: 2,  3,  4,  5,  6,  7
  Step 2 Class:   4,  5,  6,  7,  7,  9
  Step 3 Align:    2,  2,  3,  4,  6,  7

Multiplier (MENU.C:49, G0486), indexed by first symbol minus 96:
  8, 12, 16, 20, 24, 28
Effective cost for steps 1-3:
  cost = (baseCost * multiplier[powerSymbolIndex]) >> 3

Step 0 cost is always the base cost (no multiplier applied).

### Mana Deduction
Source: SYMBOL.C:1222-1239, F0399_MENUS_AddChampionSymbol
- If manaCost <= champion.CurrentMana: deduct mana, store symbol
- If manaCost > champion.CurrentMana: reject symbol (no partial deduction)

### Mana Regeneration
Source: CHAMPION.C:2335-2355, F0297_CHAMPION_ProcessTick

  timeCriteria = ((gameTime & 0x0080) + ((gameTime & 0x0100) >> 2) + ((gameTime & 0x0040) << 2)) >> 2
  wizardPriestLevel = GetSkillLevel(champion, WIZARD) + GetSkillLevel(champion, PRIEST)

  if (CurrentMana < MaximumMana && timeCriteria < (wisdom + wizardPriestLevel)):
      manaGain = (MaximumMana / 40) + 1
      staminaCost = manaGain * max(7, 16 - wizardPriestLevel)
      DecrementStamina(champion, staminaCost)
      CurrentMana += min(manaGain, MaximumMana - CurrentMana)

Higher Wisdom + Wizard/Priest skill -> more frequent mana regen ticks.

---

## 2. Spell Casting Requirements

Source: MENU.C:1811-1849, F0412_MENUS_GetChampionSpellCastResult

A spell succeeds only if ALL of the following are true:

1. Champion is alive - dead champions cannot cast
2. CurrentMana >= total inscribed mana cost
3. Skill level >= baseRequiredSkillLevel (no temp XP bonus)
4. Object modifiers included - armor/weapon modifiers affect skill check
5. For flask/potion spells - champion has an empty flask in pack or action hand
6. For MAP spell - champion has Magic Map in action hand

### Skill Level Check
Source: CHAMPION.C:715-822, F0303_CHAMPION_GetSkillLevel
  effectiveSkill = baseSkill + objectBonus[slot0] + objectBonus[slot1] + ...

---

## 3. Casting Process (Step by Step)

### Step A: Select Magic Caster
Source: CASTER.C:10-, F0394_MENUS_SetMagicCasterAndDrawSpellArea
- Click a living champion to designate as magic caster
- Spell area redraws with that champion highlighted

### Step B: Inscribe Symbols
Source: SYMBOL.C:1222-1239, F0399_MENUS_AddChampionSymbol
- Click symbol buttons in sequence through 4 steps
- Each click: compute mana cost -> deduct if affordable -> store symbol -> advance step
- Step wraps (3->0) to allow multiple spells of same power symbol

### Step C: Delete Symbol
Source: SYMBOL.C:1240-1252, F0400_MENUS_DeleteChampionSymbol
- Click delete/backspace: retreat one step, clear last symbol
- Mana is NOT refunded - matches original behavior

### Step D: Look Up Spell
Source: MENU.C:1685-1705, F0409_MENUS_GetSpellFromSymbols
- Pack entered symbols into 4-byte int (MSB-first)
- If spell MSB != 0: compare full 4 bytes
- If spell MSB = 0: compare only lower 3 bytes (power symbol optional)

### Step E: Cast Spell
Source: MENU.C:1811-1849, F0412_MENUS_GetChampionSpellCastResult

  if (champion dead or absent)    -> C0: NEEDS MORE PRACTICE
  if (skill too low)              -> C0: NEEDS MORE PRACTICE WITH THIS SPELL
  if (CurrentMana < manaCost)   -> cast fails silently (symbols cleared)
  if (needs flask, none present) -> C3: NEEDS EMPTY FLASK IN HAND
  if (needs map, none present)    -> C1: NEEDS MAGIC MAP IN ACTION HAND
  if (spell not in table)        -> C01: MUMBLES A MEANINGLESS SPELL
  else                           -> cast succeeds, apply effect

### Step F: Experience Award
Source: MENU.C:1817-1849, F0412 experience formula

  required = baseRequiredSkillLevel + powerOrdinal
  exp = rng8 + (required << 4) + (((powerOrdinal - 1) * baseRequiredSkillLevel) << 3) + (required * required)

---

## 4. Projectile Spells - Kinetic Energy

Source: MENU.C:1811+, F0412 case C2_SPELL_KIND_PROJECTILE
  KE = bounded(21, (powerOrdinal + 2) * (4 + skillLevel * 2), 255)
For Open Door spell: skillLevel doubles.

---

## 5. Source Evidence

  SYMBOL.C:1222-1239  F0399_MENUS_AddChampionSymbol (mana cost, symbol storage)
  SYMBOL.C:1240-1252  F0400_MENUS_DeleteChampionSymbol (no mana refund)
  MENU.C:44           G0485 SymbolBaseManaCost[4][6]
  MENU.C:49           G0486 SymbolManaCostMultiplier[6]
  MENU.C:50-76        G0487 as Spells[25]
  MENU.C:1633-1663    F0408 clear symbols after cast attempt
  MENU.C:1685-1705    F0409 spell lookup from symbols
  MENU.C:1811-1849    F0412 spell cast result + requirements
  CHAMPION.C:2330-2355 mana regeneration tick