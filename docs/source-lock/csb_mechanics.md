# CSB V1 — Mechanical Changes (Source-Lock Audit)

**Audit date:** 2026-05-25
**Sources:** ReDMCSB: PROJEXPL.C, GROUP.C, DUNGEON.C, CASTER.C, BugsAndChanges.htm · CSBWin: Magic.cpp, Attack.cpp, Character.cpp · DEFS.H:757–768

---

## Part I: Combat

### Projectile Speed Normalization — `PROJEXPL.C` (CHANGE7_20_IMPROVEMENT)

| Setting | DM1 | CSB |
|---------|-----|-----|
| Projectile speed on party map | Full | Full |
| Projectile speed on other maps | **Slower (bug)** | **Full speed (fixed)** |

This is the only non-bug-fix combat gameplay change in CSB.
Source: PROJEXPL.C (CHANGE7_20_IMPROVEMENT) · csb_combat.md

### Grey Lord Combat Behavior (New Creature)

Grey Lord (0x1a) is a new C5_ATTACK_MAGIC creature:
- Attack.cpp:2423 — monster type assignment
- `IsLordChaosHere()` widened to include Grey Lord proximity checks
- Chaos.cpp — dedicated attack byte sequences

Grey Lord uses existing attack infrastructure (C5_ATTACK_MAGIC category shared with
Lord Chaos, Lord Order, Zytaz, Vexirk, FlyingEye). The new content is the creature itself,
not the attack type system.

Source: Attack.cpp:2423 · Chaos.cpp · DEFS.H:1679 · csb_creatures.md

### Group AI / Creature Behavior Fixes

| Bug | File | Change |
|-----|------|--------|
| BUG0_69: Group movement/teleporter | GROUP.C (CHANGE7_19_FIX) | Fixed Lord Chaos allowed-map checks |
| BUG0_09: Dungeon square event | DUNGEON.C (CHANGE7_17_FIX) | Fixed spawn/trigger |
| BUG0_10: Dungeon square event | DUNGEON.C (CHANGE7_18_FIX) | Fixed spawn/trigger |

Source: GROUP.C (CHANGE7_19) · DUNGEON.C (CHANGE7_17/18)

### Save Game Combat State

- CHANGE7_29: New saved game header format (CSBGAME.DAT vs DMSAVE.DAT)
- CHANGE8_12_FIX: Save/load fixes affecting combat state

Source: CEDTINC8.C:101–118 · BugsAndChanges.htm

---

## Part II: Magic

### Spell Power Changes (DM1 vs CSB delta)

M13_PLAN.md:337: *"CSB changes some spell power values"* for shared spell names.
Specifically the ZOKATHRA (Zo Kath Ra) fireball variant has a different kinetic energy
/power calculation vs the standard Fireball (Ful Ir) spell.

No new spell types. The 25-spell table is identical between DM1 and CSB.
The CSB-specific delta is the power value for ZOKATHRA's fireball effect.

Source: M13_PLAN.md:337 · memory_magic_pc34_compat.c (CSB spell variants gate)

### ZOKATHRA Spell — Fireball Variant

| Property | Value |
|----------|-------|
| Runes | Zo Kath Ra |
| Type | `C7_SPELL_TYPE_OTHER_ZOKATHRA` = 7 |
| Effect | Fireball variant with CSB-specific power |
| Icon | `C197_ICON_JUNK_ZOKATHRA` = 197 |
| CSB power delta | Different from DM1 Fireball (Ful Ir) |

Source: DEFS.H:1774 (C7_SPELL_TYPE_OTHER_ZOKATHRA) · MENU.C:1994 · M13_PLAN.md:337

### Magic System — Unchanged from DM1

- Same 25-spell table (DM1_SPELL_COUNT = 25)
- Same 4 schools (Fire/Air/Earth/Water), 4 classes (Fighter/Ninja/Priest/Wizard)
- Same casting mechanism (Incantation2Spell → CastSpell → spell type handler)
- Same mana/stat requirements
- Same spell filtering (`CallSpellFilter`, `SPELL_PARAMETERS`)

Source: Magic.cpp:844–954 · CSB.h:1727–1739 (SPELL struct) · dm1_v1_spell_casting_pc34_compat.c

---

## Part III: Movement

### No Movement System Changes

Both DM1 and CSB use identical movement mechanics:
- 8-directional movement (forward step, backward step, turning)
- Collision detection (walls, doors, objects)
- Sensor triggering (party movement sensors)
- Teleporter behavior (same logic in DUNGEON.C)

No changes to:
- Movement speed
- Turn rate
- Step size
- Cell normalization

Source: ReDMCSB DUNGEON.C · MOVESENS.C (version-check sensor is new, movement logic unchanged)

---

## Part IV: Champion Advancement

### Class System — Identical to DM1

CSB carries forward DM1's 4 base classes without modification:

| Class | Index | Skills |
|-------|-------|--------|
| Fighter | C00 | Swing, Thrust, Club, Parry |
| Ninja | C01 | Steal, Fight, Throw, Shoot |
| Priest | C02 | Identify, Heal, Influence, Defend |
| Wizard | C03 | Fire, Air, Earth, Water |

Source: DEFS.H:757–768 · CHAMPION.C · csb_champions.md

### Level-Up Stat Gains — Identical to DM1

No changes to stat gain formulas for any class.
Fighter: Strength +2 major / +1 minor alternation.
Ninja: Dexterity +2 major / +1 minor alternation.
Priest: Wisdom +1/+2, AntiFire +0–2.
Wizard: Wisdom +1/+2, AntiMagic +0–3.
MaxHP = mastery + random(0..mastery/2). MaxMP = mastery. MaxStamina = base/16–32 depending on class.

Source: Magic.cpp:LevelUp() function · CHAMPION.C · csb_champions.md

### Champion Reincarnation Rules — CHANGED in CSB (CHANGE7_24_IMPROVEMENT)

DM1: Standard death/reincarnation — full stat preservation.
CSB CHANGE7_24: New reincarnation rules:

| Stat | DM1 | CSB |
|------|-----|-----|
| Health | Full preservation | **Halved** |
| Mana | Full preservation | **Halved** |
| Stamina | Full preservation | **Halved** |
| Other stats (non-Luck) | Full preservation | **−1/8th of current value** |
| Luck | Full preservation | **No penalty** |
| Minimums | Respected | Respected (no stat goes below minimum) |

Source: CSB:REVIVE.C (CHANGE7_24) · csb_champions.md

---

## Summary of Mechanical Changes

| System | Change | Type |
|--------|--------|------|
| Projectile speed on non-party maps | Fixed to full speed | Bug fix |
| Grey Lord combat | New C5_ATTACK_MAGIC creature | New content |
| Group AI (teleporter/map) | BUG0_69 fix | Bug fix |
| Dungeon event processing | BUG0_09, BUG0_10 fix | Bug fix |
| ZOKATHRA spell power | Different value vs DM1 | Variant delta |
| Movement | None | — |
| Class system | Identical to DM1 | — |
| Champion reincarnation | HP/MP/STA halved, −1/8th stats except Luck | Significant change |

**Key finding:** CSB's mechanical changes are minimal and targeted:
- One gameplay improvement (projectile speed normalization)
- One significant champion change (reincarnation penalty)
- One new creature with combat role (Grey Lord)
- One spell power variant (ZOKATHRA)
- Bug fixes for group movement and dungeon events

---

## Source Citations

| File | Lines | Content |
|------|-------|---------|
| ReDMCSB PROJEXPL.C | CHANGE7_20 | Projectile speed normalization |
| ReDMCSB GROUP.C | CHANGE7_19 | Group/teleporter fix |
| ReDMCSB DUNGEON.C | CHANGE7_17/18 | Dungeon event fixes |
| ReDMCSB DEFS.H | 757–768 | Class/skill constants |
| ReDMCSB DEFS.H | 1679 | C5_ATTACK_MAGIC sources |
| CSBWin Attack.cpp | 2423 | Grey Lord combat |
| CSBWin Magic.cpp | 1090–1400 | CastSpell, LevelUp, ZOKATHRA |
| CSBWin Character.cpp | 5528 | Character handling |
| CSB:REVIVE.C | CHANGE7_24 | Reincarnation change |
| M13_PLAN.md | 337,346 | ZOKATHRA power variant |
| BugsAndChanges.htm | CHANGE7_20,21,22,23,24 | All CSB changes |