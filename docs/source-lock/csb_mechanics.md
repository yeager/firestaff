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

### Zokathra and edition-specific spell tables

Correction (2026-09-06): Zo Kath Ra is not a CSB-only fireball variant.
ReDMCSB `MENU.C:76` defines attributes `0x3C73` (other spell, Zokathra,
15 disabled ticks). `MENU.C:F0412:1994–2027` creates a C51 junk object
in an empty hand or on the party square in both DM1 and CSB. The former
50-versus-0 power helper and test had no authentic basis and were removed.

Atari CSB S20/S21 uses 25 spells; Amiga CSB A31/A33/A35 and FM Towns F31
use 29, adding four magic-map spells (`MENU.C:15–18,50–84`,
`DEFS.H:3224–3227`). Do not substitute CSBWin's 25-entry graphic
`0x230` table or spell-filter callbacks for these original owners.

Original casting is F0408/F0412: rune clearing except missing flask/map,
source RNG/practice XP, effect mutation, success XP and action disable.
Edition-specific effects/timing still require verification. See
[the item/spell source audit](csb_items.md) for the corrected scope.

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

### Champion Reincarnation Rules — CHANGED in CSB (Package-Specific)

DM1: Standard death/reincarnation — full stat preservation.
Atari ST CHANGE7_24 builds use the following rules.  `REVIVE.C:F0282` also
contains a PC branch with no vital/stat reduction and Amiga/FM Towns
`MEDIA629` branches that quarter vitals; all use twelve random statistic
increments after skill clearing.

| Stat | DM1 | CSB |
|------|-----|-----|
| Health | Full preservation | **Halved** |
| Mana | Full preservation | **Halved** |
| Stamina | Full preservation | **Halved** |
| Other stats (non-Luck) | Full preservation | **−1/8th of current value** |
| Luck | Full preservation | **No one-eighth penalty; may receive a random increment** |
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
| Magic-map spells | Four additional spells in later Amiga/FM Towns editions | Edition-specific content |
| Movement | None | — |
| Class system | Identical to DM1 | — |
| Champion reincarnation | Package-specific PC/ST/Amiga/FM Towns branch | Significant change |

**Key finding:** CSB's mechanical changes are minimal and targeted:
- One gameplay improvement (projectile speed normalization)
- One significant champion change (reincarnation penalty)
- One new creature with combat role (Grey Lord)
- Four magic-map spells in later Amiga/FM Towns editions; Zokathra is shared object creation
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
| ReDMCSB MENU.C | 76–84,1994–2027 | Shared Zokathra object creation; later magic-map spells |
| BugsAndChanges.htm | CHANGE7_20,21,22,23,24 | All CSB changes |
