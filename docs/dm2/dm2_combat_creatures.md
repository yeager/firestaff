# DM2 V1 — Creature Combat Attacks (Unique vs DM1)

## Source
- `skproject/SKWIN/defines.h:705-716` — AI_ATTACK_FLAGS
- `skproject/SKWIN/SkWinCore.cpp:415-437, 27038-27096` — creature attack routing
- `skproject/SKWIN/DME.h:1505-1560` — AIDefinition struct

---

## Overview: What DM2 Adds vs DM1

DM1 creatures used only **melee** and **ranged projectile** attacks. DM2 introduces a comprehensive spell-based attack system where creatures can cast spells directly.

---

## AI_ATTACK_FLAGS — DM2 Creature Attack Types

DM2 extends creature attacks with a 16-bit flag field:

| Flag | Value | Attack Type | Notes |
|---|---|---|---|
| `AI_ATTACK_FLAGS__MELEE` | 0x0001 | Standard physical attack | Same as DM1 |
| `AI_ATTACK_FLAGS__PUSH_BACK` | 0x0002 | Knock-back on hit | Spiked Wall/Floor Spikes (40) |
| `AI_ATTACK_FLAGS__STEAL` | 0x0004 | Steal item from champion | Giggler (26), Thicket Thief (27) |
| `AI_ATTACK_FLAGS__SHOOT` | 0x0008 | Throw projectile/weapon | Archer Guard (36) |
| `AI_ATTACK_FLAGS__FIREBALL` | 0x0010 | Cast fireball spell | Amplifier (51) — machine creature |
| `AI_ATTACK_FLAGS__DISPELL` | 0x0020 | Dispel champion enchantments | — |
| `AI_ATTACK_FLAGS__LIGHTNING` | 0x0040 | Cast lightning spell | — |
| `AI_ATTACK_FLAGS__POISON_CLOUD` | 0x0080 | AoE poison cloud | — |
| `AI_ATTACK_FLAGS__POISON_BOLT` | 0x0100 | Single-target poison bolt | — |
| `AI_ATTACK_FLAGS__POISON_BLOB` | 0x0200 | Contact poison blob | Giggler gas |
| `AI_ATTACK_FLAGS__PUSH_SPELL` | 0x0400 | Push spell (telekinesis) | — |
| `AI_ATTACK_FLAGS__PULL_SPELL` | 0x0800 | Pull spell (telekinesis) | — |

Source: `defines.h:705-716`

---

## Spell-Based Creature Attacks (DM2 New)

### Fireball (0x0010)
- **Creature:** Amplifier (AI index 51) — a magical machine object
- Patched in fixed mode: `dAITable[51].AttacksSpells |= AI_ATTACK_FLAGS__FIREBALL`
- Trigger: `OBJECT_EFFECT_FIREBALL` (SkWinCore.cpp:27051-27054)
- Amplifier must remain static or loses its moveable ability
- Used in Skullkeep dungeon

### Dispell (0x0020)
- Removes active enchantments from champions
- SkWinCore.cpp:27056-27058

### Lightning (0x0040)
- Electric damage projectile
- SkWinCore.cpp:27061-27063

### Poison Cloud (0x0080)
- Area-effect poison damage (like DM1's Gigglers)
- SkWinCore.cpp:27066-27068

### Poison Bolt (0x0100)
- Single-target poison projectile
- SkWinCore.cpp:27071-27073

### Poison Blob (0x0200)
- Contact poison on approach
- SkWinCore.cpp:27086-27088

### Push/Pull Spells (0x0400/0x0800)
- Telekinetic displacement of party members
- SkWinCore.cpp:27076-27082

---

## DM2-New Physical Attack Capabilities

### Steal (0x0004)
- Creatures can steal items from champions' inventory
- Applied to: Giggler (26), Thicket Thief (27)
- References: SkWinCore.cpp:419, 27046

### Shoot (0x0008)
- Creature throws weapons/items as projectiles
- Applied to: Archer Guard (36)
- SkWinCore.cpp:27096

### Push Back (0x0002)
- Physical knockback when creature attacks
- Applied to: Spiked Wall/Floor Spikes (40)
- SkWinCore.cpp:27038-27040

---

## DM2 Boss Creature Attacks

### Lord Dragoth (AI Index 30) — Final Boss
- Has `w0_a_a` flag (invisibility-related, bit 10)
- Has `flag4` (bit 4) and `flag3` (bit 3)
- Has `w0_5_5` (non-material/intangible)
- **Spawn Minion** — YELL sound (0x0E) triggers minion spawn
- Sound: SOUND_CREATURE_REFLECTOR (0x03), SOUND_CREATURE_XXX (0x06), SOUND_CREATURE_GET_HIT_2 (0x09)
- Has missile-absorbing capability (non-material)
- Source: SkWinCore.cpp:778

### Dragoth Minion (AI Index 34)
- Evil minion spawned by Lord Dragoth
- Evil version of ally minion system (indices 13-18)
- Source: SkWinCore.cpp:784

### Vexirk King (AI Index 55)
- Elite variant of Dark Vexirk (index 48)
- Has `flag4` (w0_4_4) bit
- King variant has boosted HP, AttackStrength, Defense

### Amplifier (AI Index 51) — Machine Boss
- Fireball spell attack
- Must remain static object (preserves IsStaticObject flag)
- Source: SkWinCore.cpp:249-252

---

## AIDefinition Struct — Combat-relevant Fields

```c
struct AIDefinition {  // 36 bytes
    Bit16u w0AIFlags;        // @0  — behavior/static/flying/invisible
    Bit8u  ArmorClass;      // @2  — armor class (defense rating)
    i8     b3;              // @3
    Bit16u BaseHP;          // @4  — initial hit points
    U8     AttackStrength;  // @6  — base physical damage
    U8     PoisonDamage;    // @7  — poison damage on hit
    U8     Defense;         // @8  — 255 = undestroyable
    X8     b9x;             // @9  — 0x40: pit ghost marker
    X16    w10;             // @10
    X16    w12;             // @12
    X16    AttacksSpells;   // @14 — AI_ATTACK_FLAGS (attack/spell flags)
    X16    w16;             // @16 — switch triggers
    X16    w18;             // @18
    U16    w20;             // @20
    Bit16u w22;             // @22
    Bit16u w24;             // @24 — resistance (fire/poison)
    X16    w26;             // @26
    U8     b28;             // @28
    Bit8u  Weight;          // @29 — push resistance, 255=immovable
    Bit16u w30;             // @30 — 0x0800: can turn missiles
    Bit16u w32;             // @32
    Bit8u  b34;             // @34
    Bit8u  b35;             // @35
}
```

### w0AIFlags Combat Bits

| Bit | Name | Description |
|---|---|---|
| 0 | `IsStaticObject` | Static object (blocks movement) |
| 1 | w0_1_1 | Unknown |
| 3 | w0_3_3 | Spectres and ghosts |
| 4 | w0_4_4 | Spectres/ghosts + vexirks |
| 5 | w0_5_5 | Non-material (intangible) |
| 8 | `PushWhenMoving` | Move and push back anything |
| 9 | `AbsorbsMissile` | Most creatures |
| 10 | w0_a_a | Related to invisibility (ghosts + dragoth) |

### w30: Missile Turning (0x0800)
- If set: creature reflects projectiles back at attacker
- Checked in SkWinCore.cpp:10479, 10561

---

## Comparison: DM1 vs DM2 Creature Attacks

| Feature | DM1 | DM2 |
|---|---|---|
| Physical attacks | Melee, shoot | Melee, shoot, push-back, steal |
| Spell attacks | None | Fireball, lightning, dispell, poison |
| AoE attacks | Poison gas (Gigglers) | Poison cloud, poison blob |
| Telekinetic | None | Push spell, pull spell |
| Boss special | Limited | Spawn minion (Dragoth) |
| Creature types | ~30 base | ~65+ with spell variants |
| Missile reflect | None | Via w30 flag |

---

## Minion System (DM2 New)

DM2 introduces a companion-like minion system with allied and evil variants:

| AI Index | Name | Type |
|---|---|---|
| 13 | Scout Minion | Ally |
| 14 | Attack Minion | Ally |
| 15 | Carry Minion | Ally |
| 16 | Fetch Minion | Ally |
| 17 | Guard Minion | Ally |
| 18 | U-Haul Minion | Ally |
| 34 | Dragoth Minion | Evil |
| 43 | Evil Attack Minion | Evil |
| 62 | Evil Attack Minion (alt) | Evil |

Spell-summoned minions (indices 29-31) use spell type 4 (SUMMON):
- ZO EW KU: Attack Minion
- ZO EW NETA: Guard Minion
- ZO EW ROS: U-Haul Minion

---

## Status

**SOURCE-LOCKED** — AI_ATTACK_FLAGS from defines.h:705-716, attack routing from SkWinCore.cpp:415-437 and 27038-27096, AIDefinition struct from DME.h:1505-1560.
