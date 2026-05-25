# DM1 V1 - Spell Effects
Audit date: 2026-05-25 | Source: ReDMCSB WIP20210206

---

## 1. Spell Effect Categories

DM1 V1 spells fall into 5 effect categories based on attributes field
(decoded from G0487_as_Graphic560_Spells[25], MENU.C:50-76).

### Category: Projectile Spells
Travel through dungeon squares, checked for collision each tick.

| Spell            | Symbols     | Kind | Notes                   |
|------------------|-------------|------|-------------------------|
| FIREBALL         | Ful Ir      | 0xA802 | Kind=10, Type=0        |
| LIGHTNING BOLT   | Oh Kath Ra  | 0x7822 | Kind=7, Type=2         |
| POISON BOLT      | Des Ven    | 0x4062 | Kind=4, Type=6         |
| OPEN DOOR        | Zo         | 0x3C42 | Kind=3, Type=12        |

Projectile mechanics (CHAMPION.C:2073, F0327_CHAMPION_IsProjectileSpellCast):
- Kinetic Energy = bounded(21, (powerOrdinal + 2) * (4 + skillLevel*2), 255)
- Step Energy = 10 - min(8, MaximumMana >> 3)
- For Open Door: skillLevel doubles (used for breaking locks/barriers)

### Category: Light-Producing Spells

| Spell              | LightPower       | Duration (ticks)     |
|--------------------|------------------|----------------------|
| TORCH (Ful)        | 3-8 (by power)   | 2640-5200            |
| LIGHT (Oh Ir Ra)   | 3,5,7,9,11,13    | 10000-20240          |
| DARKNESS (Des Ir) | 2-7 (reduces)    | 98 ticks             |

- MAGNETIC FOOTPRINTS (Ya Bro Ros): shows party movement trail
- INVISIBILITY (Oh Ew Sar): reduces creature detection

Light system (Engine.htm documentation):
- MagicalLightAmount accumulates from all magical light sources
- F0257_TIMELINE_ProcessEvent70_Light handles light decay
- Light fades one step every 4 clock ticks until LightPower = 0
- Maximum brightness enforced at end-game (F0446_STARTEND_FuseSequence)

### Category: Shield/Defense Spells

| Spell          | Effect                         |
|----------------|--------------------------------|
| SHIELD         | Party-wide shield event, defense bonus |
| FIRE SHIELD    | Fire attack shield on champion |
| SHIELD POTION  | Single champion shield         |

### Category: Potion Spells (require empty flask)

| Spell                  | Effect              | Requires flask |
|------------------------|---------------------|----------------|
| STRENGTH POTION        | Temp STR boost      | Yes            |
| WISDOM POTION          | Temp WIS boost      | Yes            |
| VITALITY POTION        | Temp VIT boost      | Yes            |
| HEALTH POTION          | Heal missing HP     | Yes            |
| CURE POISON POTION     | Remove poison       | Yes            |
| DEXTERITY POTION       | Temp DEX boost      | Yes            |
| MANA POTION            | Restore mana        | Yes            |
| STAMINA POTION         | Restore stamina     | Yes            |
| POISON POTION          | Poison target       | Yes            |
| SHIELD POTION          | Shield defense      | Yes            |

Potion creation (MENU.C): flask consumed, potion item created.

### Category: Utility Spells

| Spell              | Effect                             |
|--------------------|------------------------------------|
| OPEN DOOR (Zo)     | Removes door from target square    |
| POISON CLOUD       | Area poison effect                 |
| THIEVE'S EYE       | Reveals hidden traps/features     |
| MAGIC FOOTPRINTS   | Shows party movement trail         |
| INVISIBILITY       | Reduces creature detection         |

### Category: Endgame

| Spell     | Effect                                               |
|-----------|------------------------------------------------------|
| ZOKATHRA  | Used with Firestaff Complete for Fuse action        |

---

## 2. Healing Cycle (S12E+)

Source: MENU.C:1512-1517

  cycles = min(CurrentMana / 2, MissingHealth / min(10, HealSkill))
  heal = min(MissingHealth, min(10, HealSkill)) * cycles
  manaCost = 2 * cycles
  XP = 2 + 2 * cycles

Health potions skip the cycle and apply directly.

---

## 3. Projectile Collision and Effect Application

Phase 3 gap (DM1_V1_PLAN.md): Projectile lifecycle and collision
detection with creatures/walls is listed as not yet implemented.

Source references for future implementation:
- CHAMPION.C:2073 - F0327 projectile spell detection
- MENU.C:F0412 case C2 - projectile spell resolution
- Collision checked each game tick against dungeon squares

---

## 4. Source Evidence

  MENU.C:50-76        spell table with attributes
  MENU.C:1811-1849    F0412 spell cast result handling
  CHAMPION.C:2073     F0327 projectile spell detection
  Engine.htm          Light spell mechanics (F0257_TIMELINE_ProcessEvent70_Light)
  DM1_V1_PLAN.md Phase 3 items 10-13: projectile lifecycle, spell area effects,
                   potion creation, shield/light spells - GAPS IDENTIFIED