# DM1 V1 - Magic Resistance and Defense
Audit date: 2026-05-25 | Source: ReDMCSB WIP20210206

---

## 1. Champion Magic Defense Statistics

DM1 V1 champions have two magic-related defense statistics:

| Statistic        | Effect                    | Source              |
|-----------------|---------------------------|---------------------|
| statisticAntifire | Reduces fire damage      | CHAMPION.C:1878     |
| statisticAntimagic | Reduces lightning damage | CHAMPION.C:1878   |

### Antifire
Source: memory_combat_pc34_compat.c:207-215
  adjusted = F0734_COMBAT_GetStatisticAdjustedAttack_Compat(
      champion->statisticAntifire, 255, rawDamage, &adjusted)

### Antimagic
Source: memory_combat_pc34_compat.c:211-215
  adjusted = F0734_COMBAT_GetStatisticAdjustedAttack_Compat(
      champion->statisticAntimagic, 255, adjusted, &tmp)

---

## 2. Spell Shield Defense (Party-Wide)

G0407_s_Party.SpellShieldDefense - party-wide magic shield from SHIELD spell.

### Adding shield defense (from SHIELD spell event):
Source: firestaff_pc34_flattened_amalgam.c:8478
  G0407_s_Party.SpellShieldDefense += event->B.Defense

### Applying shield defense to attacks:
Source: firestaff_pc34_flattened_amalgam.c:6144
  if (attackIsMagic) P0663_i_Attack -= G0407_s_Party.SpellShieldDefense

### Decay/removal:
Source: firestaff_pc34_flattened_amalgam.c:8475
  if (G0407_s_Party.SpellShieldDefense > 50) { ... }
  G0407_s_Party.SpellShieldDefense -= defense  // when event expires

---

## 3. Fire Shield Defense

FireShieldDefense is set by the FIRE SHIELD spell (Ful Bro Neta) and
applied against fire attacks.

Source: memory_combat_pc34_compat.c:224 - FireShieldDefense reference

KNOWN GAP: Fire/magic/psychic defense paths through SpellShieldDefense /
FireShieldDefense are not fully modelled in Phase 13. The code has a
placeholder:
  /* NEEDS DISASSEMBLY REVIEW: fire/magic/psychic defence paths
   * depend on SpellShieldDefense / FireShieldDefense globals [...]
   * For v1 we leave adjusted unchanged and DO NOT fabricate. */

---

## 4. Creature Magic Resistance

DM1 V1 creature resistance to magic is NOT a universal attribute table.
Magic defense for creatures is handled through armor/resistance values
stored per creature type in the dungeon data.

Source: dm1_v1_group_management_pc34_compat.c:13
  F0192: damage calculation respecting creature armor/resistance

### Poison resistance:
Source: dm1_v1_combat_pc34_compat.c:538
  GROUP.C: Adjusts poison attack by creature's poison resistance.

No global magic resistance flag found for creatures in current
source-locked code. Creature vulnerability to magic is implicit in
their health values and the projectile kinetic energy calculation.

---

## 5. Combat Formulas for Magic Damage

### Lightning damage to champions:
Source: CHAMPION.C:1878, memory_combat_pc34_compat.c
  rawDamage = f(projectileKE, distance)
  adjustedDamage = GetStatisticAdjustedAttack(Antimagic, 255, rawDamage)

### Fire damage to champions:
Source: memory_combat_pc34_compat.c:207
  rawDamage = f(fireSourcePower)
  adjustedDamage = GetStatisticAdjustedAttack(Antifire, 255, rawDamage)

---

## 6. Source Evidence

  firestaff_pc34_flattened_amalgam.c:4872  SpellShieldDefense decrement
  firestaff_pc34_flattened_amalgam.c:6144  SpellShieldDefense applied to attack
  firestaff_pc34_flattened_amalgam.c:8475  SpellShieldDefense decay threshold
  firestaff_pc34_flattened_amalgam.c:8478  SpellShieldDefense increment
  firestaff_pc34_core_amalgam.c:5256       SpellShieldDefense decrement (core)
  firestaff_pc34_core_amalgam.c:6025       Antifire bug comment (BUG0_41)
  firestaff_pc34_core_amalgam.c:6528       SpellShieldDefense attack reduction
  memory_combat_pc34_compat.c:207-215      Antifire/Antimagic stat application
  memory_champion_lifecycle_pc34_compat.c:316  partySpellShieldDefense decay
  dm1_v1_combat_pc34_compat.c:538          poison resistance adjustment
  dm1_v1_group_management_pc34_compat.c:13  creature armor/resistance F0192
  DM1_V1_PLAN.md Phase 3 item 13: Shield/light spells - GAP