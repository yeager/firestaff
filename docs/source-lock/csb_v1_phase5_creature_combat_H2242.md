# CSB V1 Phase 5 — Creature Roster, AI Differences, Attacks/Projectiles, Drops, Sounds, and Combat Constants

**Phase:** 5 of CSB V1 implementation
**Date:** 2026-05-26
**Sources:** CSBWin Monster.cpp · Attack.cpp · Chaos.cpp · CSB.h · Data.h · Objects.h · ReDMCSB GROUP.C · DEFS.H · BugsAndChanges.htm

---

## Overview

Phase 5 source-locks the CSB creature and combat systems that diverge from DM1.
CSB shares the DM1 base creature roster (0x00–0x18, 0x1a) and combat infrastructure
(GROUP.C F0190–F0230, CHAMPION.C F0311–F0321) but adds:
- One new named creature: Grey Lord (0x1a)
- DSA-scriptable creature AI via two filter hooks (attack filter + movement filter)
- Extended projectile set including DispellMissile and ZoSpell
- CSB-specific drop tables and creature sounds
- Modified attack resolution (DSA can intercept and modify attack parameters)
- One new attack type (Chaos Bolt) beyond the DM1 base set

---

## Part I — Creature Roster

### Shared Roster (0x00–0x18, identical to DM1)

| Index | Hex | Name (DEFS.H) | Name (AsciiDump) | Notes |
|-------|-----|---------------|------------------|-------|
| 0 | 0x00 | C00_CREATURE_GIANT_SCORPION | Scorpion | Shared |
| 1 | 0x01 | C01_CREATURE_SWAMP_SLIME | Slime_Devil | Shared |
| 2 | 0x02 | C02_CREATURE_GIGGLER | Giggler | Shared; steal/flee behavior |
| 3 | 0x03 | C03_CREATURE_WIZARD_EYE | Flying_Eye | Shared; ranged spell attacks |
| 4 | 0x04 | C04_CREATURE_PAIN_RAT | Hellhound | Shared |
| 5 | 0x05 | C05_CREATURE_RUSTER | Ruster | Shared |
| 6 | 0x06 | C06_CREATURE_SCREAMER | Screamer | Shared |
| 7 | 0x07 | C07_CREATURE_ROCK_ROCKPILE | Rock_Pile | Shared |
| 8 | 0x08 | C08_CREATURE_GHOST_RIVE | Rive | Shared |
| 9 | 0x09 | C09_CREATURE_STONE_GOLEM | Stone_Golem | Shared |
| 10 | 0x0a | C10_CREATURE_MUMMY | Mummy | Shared |
| 11 | 0x0b | C11_CREATURE_BLACK_FLAME | Black_Flame | Shared |
| 12 | 0x0c | C12_CREATURE_SKELETON | Skeleton | Shared |
| 13 | 0x0d | C13_CREATURE_COUATL | Couatl | Shared |
| 14 | 0x0e | C14_CREATURE_VEXIRK | Vexirk | Shared; Chaos magic |
| 15 | 0x0f | C15_CREATURE_MAGENTA_WORM | Worm | Shared |
| 16 | 0x10 | C16_CREATURE_TROLIN_ANTMAN | Ant_Man | Shared |
| 17 | 0x11 | C17_CREATURE_GIANT_WASP | Muncher | Shared |
| 18 | 0x12 | C18_CREATURE_ANIMATED_ARMOUR | Deth_Knight | Shared |
| 19 | 0x13 | C19_CREATURE_MATERIALIZER_ZYTAZ | Zytaz | Shared; materializer |
| 20 | 0x14 | C20_CREATURE_WATER_ELEMENTAL | Water_Elemental | Shared |
| 21 | 0x15 | C21_CREATURE_OITU | Oitu | Shared |
| 22 | 0x16 | C22_CREATURE_DEMON | Demon | Shared |
| 23 | 0x17 | C23_CREATURE_LORD_CHAOS | Lord_Chaos | Shared; teleport, chaos magic |
| 24 | 0x18 | C24_CREATURE_RED_DRAGON | Dragon | Shared |
| 25 | 0x19 | — | "25" | **Unused placeholder**; no attack data, no graphic |
| 26 | 0x1a | C26_CREATURE_GREY_LORD | Grey_Lord | **CSB new boss**; C5_ATTACK_MAGIC |

**Source:** CSBWin AsciiDump.cpp:306–344 · CSBWin Monster.cpp:200 · DEFS.H:1339–1366 · Attack.cpp:2423

### MONSTERDESC Struct (CSB binary format, 26 bytes per creature)

Matches the dungeon.dat creature descriptor binary layout. Field layout from
CSB.h:2323–2375 and AsciiDump.cpp:1811–1855:

| Offset | Size | Field (CSB.h) | DM1 Equivalent (DEFS.H CREATURE_INFO) | Notes |
|--------|------|---------------|----------------------------------------|-------|
| 0 | 1 | `uByte0` | `CreatureAspectIndex` | Monster type index; bits 0-1 tested for ==1 |
| 1 | 1 | `attackSound` | `AttackSoundOrdinal` | Sound ordinal; 0 = no sound |
| 2 | 2 | `word2` | `Attributes` (lo) | Size, levitation, non-material, sees invisible, etc. |
| 4 | 2 | `word4` (MONSTERDESC_WORD4) | `GraphicInfo` | Side/back/attack graphic flags |
| 6 | 1 | `movementTicks06` | `MovementTicks` | Ticks per movement; 255 = immobile |
| 7 | 1 | `attackTicks07` | `AttackTicks` | Minimum ticks between attacks |
| 8 | 1 | `defense08` | `Defense` | Defense points |
| 9 | 1 | `baseHealth09` | `BaseHealth` | Base health |
| 10 | 1 | `attack10` | `Attack` | Attack power |
| 11 | 1 | `poisonAttack11` | `PoisonAttack` | Poison attack power |
| 12 | 1 | `dexterity12` | `Dexterity` | Dexterity (missile energy) |
| 13 | 1 | `unused13` | `aUnreferenced` | Padding (compiler-added on Atari ST) |
| 14 | 2 | `word14` | `Ranges` (lo) | Sight (bits 0-3), smell (bits 8-11), attack range (bits 12-15) |
| 16 | 2 | `word16` | `Properties` (lo) | Bravery (bits 4-7), experience/wariness |
| 18 | 2 | `word18` | `Resistances` (lo) | Resistance flags |
| 20 | 2 | `word20` | `AnimationTicks` (lo) | Non-attack/attack aspect timing |
| 22 | 4 | `uByte22[4]` | `WoundProbabilities` (bits 15-12, 11-8, 7-4, 3-0) | Per-body-part wound probabilities |
| 26 | — | (total) | — | — |

**Total: 26 bytes per MONSTERDESC record (26 × 27 = 702 bytes for full table).**

**Source:** CSB.h:2323–2375 · AsciiDump.cpp:1811–1855 · DEFS.H:1575–1594

---

## Part II — DSA Script System (CSB-only AI Layer)

CSB introduces two DSA (script) filter hooks that intercept and can modify
creature behavior at runtime. Neither exists in DM1.

### 1. Monster Attack Filter (EDT_SpecialLocations << 24 | ESL_MONSTERATTACKFILTER)

**CSBWin Monster.cpp:1134–1180:** After building `attackParameters` from
`MONSTERDESC` and party state, the game searches for a DSA object at the
script location key `(EDT_SpecialLocations<<24)|ESL_MONSTERATTACKFILTER`. If
found (actuator type 47), the filter receives a copy of `attackParameters`
and can modify any field before the attack proceeds.

**Filter Parameters (9 parameters):**
```
pDSAparameters[1+0] = attackParameters.monsterID        // monster RN integer
pDSAparameters[1+1] = attackParameters.monsterType    // MONSTERTYPE
pDSAparameters[1+2] = attackParameters.heroToDamage   // champion index (0-3)
pDSAparameters[1+3] = attackParameters.supressPoison   // poison suppression flag
pDSAparameters[1+4] = attackParameters.missileType     // RNVAL of missile
pDSAparameters[1+5] = attackParameters.missileRange    // calculated range
pDSAparameters[1+6] = attackParameters.missileDamage  // damage value
pDSAparameters[1+7] = attackParameters.missileDecayRate // step energy
pDSAparameters[1+8] = attackParameters.directionToParty // primary direction
pDSAparameters[0] = 9  // parameter count
```

**Re-entry:** After `ProcessDSAFilter()`, the modified `attackParameters`
are copied back (`memcpy(&attackParameters, pDSAparameters+1, sizeof(attackParameters))`).
The LoadLevel is restored to `currentLevel` to return to the dungeon context.

**Source:** Monster.cpp:1116–1180 · CSBWin DSA.cpp (ProcessDSAFilter)

### 2. Monster Movement Filter (per-level filterObj)

**CSBWin Monster.cpp:3176–3370:** Each dungeon level can have a DSA movement
filter object (`mmfloc[level].filterObj`) that intercepts group movement decisions.
The filter is consulted in `MonsterMovement()` (Monster.cpp:3240–3370).

**Filter Parameters (7 parameters):**
```
pDSAparameters[1+0] = d.LoadedLevel        // current dungeon level
pDSAparameters[1+1] = mapX                  // group map X
pDSAparameters[1+2] = mapY                  // group map Y
pDSAparameters[1+3] = monster.ConvertToInteger() // monster RN
pDSAparameters[1+4] = d.partyLevel          // party level
pDSAparameters[1+5] = d.partyX              // party X
pDSAparameters[1+6] = d.partyY              // party Y
pDSAparameters[0] = 7                       // parameter count
```

**Effect:** DSA movement filter can override or modify movement flags
(`mmr.flgs[0]`, `mmr.flgs[1]`) that control creature behavior.

**Source:** Monster.cpp:3222–3370 · mmfloc filter loading (Monster.cpp:3079–3176)

### DM1 Comparison

DM1 has no DSA filter system. Creature AI is purely data-driven via
`CREATURE_INFO` + `ACTIVE_GROUP` state + GROUP.C F0190–F0209 event dispatch.
Any CSB dungeon can use DSA scripts to create creature behaviors impossible in DM1.

---

## Part III — Attack and Projectile System

### Attack Parameter Structure (CSBWin Monster.cpp:923)

```c
struct ATTACK_PARAMETERS {
    i32 monsterID;              // monster RN as integer
    MONSTERTYPE monsterType;   // C00..C26 creature type
    i32 monsterIndex;          // index within group (0-3)
    i32 monsterLevel;          // dungeon level
    i32 monsterX;              // group X on map
    i32 monsterY;              // group Y on map
    i32 monsterPos;            // 2-bit cell position within square
    i32 directionToParty;      // primary direction (0-3)
    i32 distanceToParty;       // orthogonal distance
    bool monsterShouldLaunchMissile; // ranged attack flag
    bool monsterShouldSteal;    // Giggler steal flag
    i32 missileType;           // RNVAL of projectile thing
    i32 missileRange;          // computed range
    i32 missileDamage;         // damage (from Dexterity)
    i32 missileDecayRate;      // step energy (always 8 in CSB)
    i32 heroToDamage;          // target champion index (0-3)
    i32 missileOriginPosition; // cell position for missile origin
    i32 attackSoundOrdinal;    // from MONSTERDESC.attackSound
    i16 supressPoison;        // poison suppression (-1 = not suppressed)
};
```

### Projectile Type Resolution (Monster.cpp:1049–1080)

| Creature | Projectile (CSB) | DM1 Equivalent |
|----------|-------------------|----------------|
| Vexirk (mon_Vexirk) | 50% Fireball, else random {DispellMissile, Lightning, PoisonCloud, ZoSpell} | Fireball or OpenDoor |
| Lord Chaos (mon_LordChaos) | 50% Fireball, else random {DispellMissile, Lightning, PoisonCloud, ZoSpell} | Fireball or OpenDoor |
| Swamp Slime (mon_SlimeDevil) | Poison (`RNPoison`) | Poison (`RNPoison`) |
| Wizard Eye (mon_FlyingEye) | 7/8 Lightning, 1/8 ZoSpell | Lightning or OpenDoor |
| Zytaz (mon_Zytaz) | 50% PoisonCloud, 50% Fireball | Fireball only |
| Demon (mon_Demon) | Fireball | Fireball |
| Dragon (mon_Dragon) | Fireball | Fireball |

### CSB-Only Projectile Types

| Name | ReDMCSB Constant | Notes |
|------|------------------|-------|
| `RNDispellMissile` | — | CSB new; Dispell effect projectile |
| `RNZoSpell` | — | CSB new; Chaos magic missile |
| `RNPoisonBolt` | C006_EXPLOSION_POISON_BOLT | Shared; used in attack parameter |

**Source:** Monster.cpp:1049–1080 · Attack.cpp:1191–1192

### Missile Damage Calculation (Monster.cpp:1116–1124)

```c
attackParameters.missileRange = pmtDesc->attack10 / 4 + 1;  // base range
attackParameters.missileRange += STRandom(attackParameters.missileRange); // +random
attackParameters.missileRange += STRandom(attackParameters.missileRange); // +random again
attackParameters.missileDamage = pmtDesc->dexterity12;       // damage from DEX
attackParameters.missileDecayRate = 8;                        // step energy = 8
```

### Attack Sound Ordinal (Monster.cpp:1088–1090)

```c
attackParameters.attackSoundOrdinal = pmtDesc->attackSound;
```

### Target Selection (Monster.cpp:1086–1100)

```
if (pmtDesc->word2Bit4() == true) {
    // Random champion selection (including dead/revived)
    attackParameters.heroToDamage = STRandom0_3();
    for (D4W=0; D4W<4 && hero[heroToDamage].HP()==0; heroToDamage++, D4W++);
    if (D4W == 4) return false;  // all champions dead
} else {
    // Distance/position-based target selection (GetCharacterToDamage)
    attackParameters.heroToDamage = GetCharacterToDamage(monsterX, monsterY, missileOriginPosition);
}
```

### Defense Calculation (CSBWin Attack.cpp)

CSB uses the same defense resolution as DM1 (CHAMPION.C F0311), with
`ActionDefense + ShieldDefense` subtracted from incoming damage.

**Source:** CSBWin Attack.cpp · ReDMCSB CHAMPION.C F0311–F0321

---

## Part IV — Drop System (Fixed Possessions)

CSB uses the same `F0186_GROUP_DropCreatureFixedPossessions()` infrastructure
as DM1 (GROUP.C F0186) but with CSB-specific drop tables.

### Fixed Possessions Drop Tables (DEFS.H:5618–5626)

| Global | Creature | Count | Drop Types |
|--------|----------|-------|------------|
| G0245 | Skeleton (C12) | 3 | Weapon/Armour |
| G0246 | Stone Golem (C09) | 2 | Weapon/Armour |
| G0247 | Trolin/AntMan (C16) | 2 | Weapon/Armour |
| G0248 | Animated Armour (C18) | 7 | Full loadout |
| G0249 | Rock/RockPile (C07) | 5 | Junk |
| G0250 | Pain Rat/Hellhound (C04) | 3 | Junk |
| G0251 | Screamer (C06) | 3 | Junk |
| G0252 | Magenta Worm (C15) | 4 | Junk |
| G0253 | Red Dragon (C24) | 11 | Full loadout |

**DM1** has the same creature drop tables (G0245–G0253) in identical positions.
The actual drop items are dungeon-specific and resolved at runtime via
`GROUP.C F0186`/`F0187`/`F0188`.

### Drop Trigger Condition

```c
// GROUP.C:716 — DROP_FIXED_POSSESSIONS attribute check
if ((P0366_i_Mode >= C00_MODE_PLAY_IMMEDIATELY) &&
    M007_GET(G0243_as_Graphic559_CreatureInfo[L0368_ui_CreatureType].Attributes,
             MASK0x0200_DROP_FIXED_POSSESSIONS))
{
    F0186_GROUP_DropCreatureFixedPossessions(...);
}
```

**Source:** GROUP.C:716 · DEFS.H:5618–5626 · BugsAndChanges.htm (no CSB-specific changes to drop system)

---

## Part V — Sound System

### Attack Sound Lookup (CSB vs DM1)

CSB replaces `G0244_auc_Graphic559_CreatureAttackSounds[8]` (DM1, 8 entries) with
`G2003_aauc_CreatureSounds[18][2]` (CSB, 18 entries × 2 columns).

**Column 0:** Attack sound index
**Column 1:** Movement sound index

**DM1 G0244 (8 entries):** Used for attack sound ordinal → sound index mapping.
**CSB G2003 (18 entries × 2):** Extends with movement sound per creature,
and a richer attack sound table.

The 18 ordinals (1–18, since ordinal 0 = no sound) map to distinct attack
and movement sounds. The ordinal-to-sound mapping in DM1 is at DEFS.H:63–109
(sound constants C00..C16 for metallic thud, wooden thud, spell, etc.).

**Source:** GROUP.C:1803–1807 · DEFS.H:63–109 · CSBWin Sound.cpp (G2003 init)

### Sound Trigger Points

| Event | Sound | Source |
|-------|-------|--------|
| Creature attack (has attackSound) | `G0244_auc_Graphic559_CreatureAttackSounds[attackSoundOrdinal-1]` | GROUP.C:1803 |
| Creature attack (has attackSound) | `G2003_aauc_CreatureSounds[attackSoundOrdinal-1][C0_ATTACK_SOUND]` | GROUP.C:1807 |
| Creature movement (non-resting) | `G2003_aauc_CreatureSounds[attackSoundOrdinal-1][C1_MOVEMENT_SOUND]` | GROUP.C:1807 |
| Drop fixed possessions (weapon dropped) | `C00_SOUND_METALLIC_THUD` | GROUP.C:645 |
| Drop fixed possessions (no weapon) | `C04_SOUND_WOODEN_THUD_ATTACK_TROLIN_ANTMAN_STONE_GOLEM` | GROUP.C:645 |

**Source:** GROUP.C:645,1803–1807 · DEFS.H:63,66 · BugsAndChanges.htm (no CSB sound changes)

---

## Part VI — Combat Constants

### Attack Type Enum (DEFS.H:1673–1680)

| Value | Name | Creatures |
|-------|------|-----------|
| 0 | C0_ATTACK_NORMAL | Giggler, poison, stamina drain |
| 1 | C1_ATTACK_SLAYER | Slayer weapon |
| 2 | C2_ATTACK_FIRE | Fire-based attacks |
| 3 | C3_ATTACK_BLUNT | Non-explosion projectiles, Demon, Mummy, Ruster, Stone Golem, Swamp Slime, Trolin, Water Elemental |
| 4 | C4_ATTACK_MATERIAL_PROJECTILE | Material projectile weapons |
| 5 | C5_ATTACK_MAGIC | **Grey Lord** (NEW), Lord Chaos, Lord Order, Materializer/Zytaz, Vexirk, Wizard Eye |

**CSB:** Grey Lord (C26) is added to C5_ATTACK_MAGIC sources in DEFS.H:1679.
All other attack types are unchanged from DM1.

### Grey Lord Combat Behavior (Attack.cpp:2423)

Grey Lord is spawned by the `EndGame` sequence (Attack.cpp:2423):
```c
DB4A3->monsterType(mon_GreyLord);
```
This is part of the final game sequence, not a regular creature spawn.

**Grey Lord attack patterns** are defined in the EndGame DSA byte sequence
in Chaos.cpp (attack byte arrays near lines 791–804).

### Champion Combat Stats (CSBWin Character.cpp:5528 lines)

CSB champion combat uses the same CHAMPION.C F0311 damage pipeline as DM1.
The champion `ActionDefense`, `ShieldDefense`, and `FireShieldDefense`
fields are identical in layout and resolution.

**Source:** CSBWin Character.cpp · ReDMCSB CHAMPION.C F0311–F0321

### Attack Type → Defense Resolution

| Attack Type | Defense Formula | Notes |
|-------------|-----------------|-------|
| C0_ATTACK_NORMAL | ActionDefense + ShieldDefense | No wounds possible |
| C1_ATTACK_SLAYER | ActionDefense + ShieldDefense | Slayer bypass |
| C2_ATTACK_FIRE | FireShieldDefense | Fire shield check |
| C3_ATTACK_BLUNT | ActionDefense + ShieldDefense | Blunt weapons |
| C4_ATTACK_MATERIAL_PROJECTILE | ActionDefense + ShieldDefense | +50% vs type 4 |
| C5_ATTACK_MAGIC | SpellShieldDefense | Magic shield check |

**Source:** CHAMPION.C F0311–F0321 · DEFS.H:1673–1680

---

## Part VII — Grey Lord EndGame Sequence (Chaos.cpp)

The Grey Lord does not appear as a random encounter. It is spawned by
the CSB endgame sequence (Attack.cpp:2423) after a timed sequence of
explosions and text displays:

1. Fusion explosion (Fireball) at position
2. DispellMissile explosion
3. Grey Lord spawned (`mon_GreyLord` type)
4. All other monsters on the level deleted
5. Text displayed (decoded with `TranslateLanguage`)
6. Scrolling text sequence
7. Victory / credits

**Source:** Attack.cpp:2400–2500 · Chaos.cpp:791–804 (attack byte sequences)

---

## Part VIII — Implementation Checklist

| Item | Status | Evidence |
|------|--------|---------|
| MONSTERDESC struct (26 bytes, little-endian) | Skeleton only | CSB.h:2323–2375 |
| Grey Lord creature type (0x1a) | Skeleton only | csb_creatures.md |
| DSA Monster Attack Filter hook | Not implemented | Monster.cpp:1116–1180 |
| DSA Monster Movement Filter hook | Not implemented | Monster.cpp:3222–3370 |
| Projectile launch (mon_Vexirk, mon_LordChaos with ZoSpell, DispellMissile) | Not implemented | Monster.cpp:1049–1080 |
| Grey Lord endgame spawn | Not implemented | Attack.cpp:2423 |
| Sound system G2003_aauc_CreatureSounds[18][2] | Not implemented | GROUP.C:1807 |
| Attack parameters struct (ATTACK_PARAMETERS) | Not implemented | Monster.cpp:923 |
| Drop fixed possessions (G0245–G0253) | Not implemented | GROUP.C:716 |
| Defense resolution (CHAMPION.C pipeline) | Not implemented | Source-locked only |
| Combat constants (C0–C5 attack types) | Skeleton only | csb_combat.md |

---

## Source Citations

| File | Lines | Content |
|------|-------|---------|
| CSBWin CSB.h | 2323–2375 | MONSTERDESC struct, MONSTERDESC_WORD4 |
| CSBWin Data.h | 1093 | `MonsterDescriptor[27]` array declaration |
| CSBWin AsciiDump.cpp | 1811–1855 | DumpMonster — full field listing |
| CSBWin Monster.cpp | 111–200 | CreateMonster, ASSERT(monsterType < 27) |
| CSBWin Monster.cpp | 923–1200 | ATTACK_PARAMETERS struct, MonsterAttacks() |
| CSBWin Monster.cpp | 1049–1080 | Projectile type switch for Vexirk/LordChaos/Slime/Eye/Zytaz/Demon/Dragon |
| CSBWin Monster.cpp | 3079–3370 | DSA movement filter loading and execution |
| CSBWin Attack.cpp | 2423 | Grey Lord monsterType assignment in EndGame |
| CSBWin Chaos.cpp | 791–804 | Grey Lord attack byte sequences |
| ReDMCSB DEFS.H | 1575–1594 | CREATURE_INFO struct |
| ReDMCSB DEFS.H | 1339–1366 | C00–C26_CREATURE_* enum, C027_CREATURE_TYPE_COUNT |
| ReDMCSB DEFS.H | 5618–5626 | G0245–G0253 fixed possessions drop tables |
| ReDMCSB DEFS.H | 1673–1680 | C0–C5_ATTACK_* attack type enum |
| ReDMCSB GROUP.C | 716–750 | F0186_GROUP_DropCreatureFixedPossessions, MASK0x0200 check |
| ReDMCSB GROUP.C | 1803–1807 | Creature attack sound via G0244 and G2003 |
| ReDMCSB GROUP.C | 1645–1770 | F0207_GROUP_IsCreatureAttacking (projectile launch) |
| ReDMCSB GROUP.C | 769–932 | F0190_GROUP_GetDamageCreatureOutcome, F0191_GROUP_GetDamageAllCreaturesOutcome |
| ReDMCSB CHAMPION.C | (F0311-F0321) | Champion damage pipeline |
| BugsAndChanges.htm | CHANGE7_23, CHANGE8_02,06 | DSA filter additions, version changes |