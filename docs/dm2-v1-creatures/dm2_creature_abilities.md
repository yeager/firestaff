# DM2 V1 — Creature Abilities vs DM1

**Source-locked to:** skproject/SKWIN/defines.h:705-716, SkWinCore.cpp:415-437, 27038-27096, DME.h:1505-1545

---

## 1. Extended Attack Flag System

DM2 introduces a rich set of creature attack ability flags (`AI_ATTACK_FLAGS__*`) that extend far beyond DM1's melee/ranged basic system.

### AI_ATTACK_FLAGS Values (defines.h:705-716)

| Flag | Value | Description |
|---|---|---|
| `AI_ATTACK_FLAGS__MELEE` | 0x0001 | Standard physical attack |
| `AI_ATTACK_FLAGS__PUSH_BACK` | 0x0002 | Knock-back on hit |
| `AI_ATTACK_FLAGS__STEAL` | 0x0004 | Steal from champion |
| `AI_ATTACK_FLAGS__SHOOT` | 0x0008 | Launch item/throw projectile |
| `AI_ATTACK_FLAGS__FIREBALL` | 0x0010 | Cast fireball spell |
| `AI_ATTACK_FLAGS__DISPELL` | 0x0020 | Dispel magic |
| `AI_ATTACK_FLAGS__LIGHTNING` | 0x0040 | Cast lightning |
| `AI_ATTACK_FLAGS__POISON_CLOUD` | 0x0080 | Poison cloud AoE |
| `AI_ATTACK_FLAGS__POISON_BOLT` | 0x0100 | Poison bolt projectile |
| `AI_ATTACK_FLAGS__POISON_BLOB` | 0x0200 | Poison blob |
| `AI_ATTACK_FLAGS__PUSH_SPELL` | 0x0400 | Push-back spell |
| `AI_ATTACK_FLAGS__PULL_SPELL` | 0x0800 | Pull spell |

Source: defines.h:705-716, SkWinCore.cpp:415-437

---

## 2. Spell-based Creature Attacks

DM2 creatures can cast spells directly — a major departure from DM1's purely physical attack system.

### Fireball (0x0010)
- Creature AI index 51 (AMPLIFIER) is patched to have fireball in fixed mode (SkWinCore.cpp:252)
- `dAITable[51].AttacksSpells |= AI_ATTACK_FLAGS__FIREBALL`
- SkWinCore.cpp:27051-27054: Fireball triggers `OBJECT_EFFECT_FIREBALL`
- Skullkeep dungeon uses this for the Amplifier machine creature

### Dispell (0x0020)
- Removes champion enchantments
- SkWinCore.cpp:27056-27058

### Lightning (0x0040)
- Electric damage projectile
- SkWinCore.cpp:27061-27063

### Poison Cloud (0x0080)
- Area-effect poison damage
- SkWinCore.cpp:27066-27068

### Poison Bolt (0x0100)
- Single-target poison projectile
- SkWinCore.cpp:27071-27073

### Poison Blob (0x0200)
- Contact poison blob (like DM1's Giggler gas)
- SkWinCore.cpp:27086-27088

### Push/Pull Spells (0x0400/0x0800)
- Telekinetic displacement of party members
- SkWinCore.cpp:27076-27082

---

## 3. Special Attack Capabilities

### Steal (0x0004)
- Creatures can steal items from champions
- Applied to Giggler (index 26), Thicket Thief (27)
- Similar to DM1 Giggler's stealing behavior
- SkWinCore.cpp:419, 27046

### Shoot (0x0008)
- Creature can throw weapons/items
- Archer Guard (index 36) uses this
- SkWinCore.cpp:27096: "Able to launch item / or maybe take item (axe, arrow?)"

### Push Back (0x0002)
- Physical knockback on hit
- Spiked Wall/Floor Spikes (40) uses this
- SkWinCore.cpp:27038-27040

---

## 4. AIDefinition Structure (DME.h:1505-1538)

The AIDefinition struct holds creature abilities:

```
struct AIDefinition { // 36 bytes
    Bit16u w0AIFlags;       // @0 — behavior flags (static, flying, invisible, etc.)
    Bit8u  ArmorClass;      // @2
    i8     b3;              // @3
    Bit16u BaseHP;          // @4 — initial HP
    U8     AttackStrength;  // @6
    U8     PoisonDamage;    // @7 — poison on hit
    U8     Defense;         // @8 — 255 = undestroyable
    X8     b9x;             // @9 — 0x40: pit ghost
    X16    w10;             // @10
    X16    w12;             // @12
    X16    AttacksSpells;   // @14 — attack/spell flags
    X16    w16;             // @16 — switch triggers
    X16    w18;             // @18
    U16    w20;             // @20
    Bit16u w22;             // @22
    Bit16u w24;             // @24 — resistance/fire/poison resistance
    X16    w26;             // @26
    U8     b28;             // @28
    Bit8u  Weight;           // @29 — push resistance, 255 = immovable
    Bit16u w30;             // @30 — 0x0800: can turn missiles
    Bit16u w32;             // @32
    Bit8u  b34;             // @34
    Bit8u  b35;             // @35
}
```

### w0AIFlags Bitfield (DME.h:1517-1560)

| Bit | Name | Description |
|---|---|---|
| 0 | `IsStaticObject` | Static object (1 = sk1c9a02c3 per Creature record) |
| 1 | w0_1_1 | Reflector? |
| 2 | w0_2_2 | Unknown |
| 3 | w0_3_3 | Spectres and ghosts |
| 4 | w0_4_4 | Spectres/ghosts + vexirks |
| 5 | w0_5_5 | Non-material (1 = intangible) |
| 6-7 | w0_6_7 | Worms and glops have both bits |
| 8 | `PushWhenMoving` | Move and push back anything on target |
| 9 | `AbsorbsMissile` | Most creatures have this |
| 10 | w0_a_a | Related to invisibility (ghosts + dragoth) |

### b9x Flag (DME.h notes)
- 0x40: Pit Ghost — marks the Pit Ghost creature type

Source: DME.h:1505-1560, defines.h:705-716

---

## 5. w30: Missile Turning Ability

AIDefinition.w30 at DME.h:1539:
- If bit 0x0800 is set: creature **can turn missiles** (reflects projectiles back)
- This is checked in SkWinCore.cpp:10479, 10561

---

## 6. Comparison: DM1 vs DM2

| Aspect | DM1 | DM2 |
|---|---|---|
| Attack types | Melee, Ranged (basic) | Melee + 9 spell types + Shoot + Steal |
| Spellcasting creatures | None | Fireball, Dispell, Lightning, Poison Cloud/Bolt/Blob, Push/Pull |
| Stealing | Giggler only | Giggler, Thicket Thief |
| Knockback | None (except Push Back flag) | Push Back physical + Push Spell |
| Missile reflection | None | w30 flag (0x0800) |
| Non-material/intangible | Ghosts (w0_3_3) | Ghosts + broader w0_5_5 |
| Poison damage | On hit poison | Separate PoisonDamage field + Poison Cloud/Bolt/Blob spells |
| Push resistance | None | Weight field (255 = immovable) |

---

## STATUS: SOURCE-LOCKED
