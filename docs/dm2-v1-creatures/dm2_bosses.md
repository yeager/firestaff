# DM2 V1 — Boss Creatures

**Source-locked to:** skproject/SKWIN/SkWinCore.cpp:741-810 (getAIName), DME.h:1505-1560, defines.h:705-716, SkWinCore.cpp:252, 27051-27054

---

## 1. Lord Dragoth — The Dragon / Final Boss

### Identity
- **AI Index:** 30 (SkWinCore.cpp:778)
- **Name:** LORD DRAGOTH
- **Type:** Primary antagonist, final boss, dragon-like creature

### Key Attributes (from AIDefinition struct)
- `w0_a_a` (bit 10): set — "related to invisibility" (ghosts + dragoth)
- `flag4` (bit 4): set — spectres/ghosts + vexirks have it
- `flag3` (bit 3): set — spectres and ghosts have it
- Likely has `w0_5_5` (non-material) set — dragging intangible form

### Sound Associations
- `SOUND_CREATURE_REFLECTOR (0x03)` — Dragoth
- `SOUND_CREATURE_XXX (0x06)` — Rocky, Dragoth (Hard Hit?)
- `SOUND_CREATURE_GET_HIT_2 (0x09)` — Dragoth (Hit)
- `SOUND_CREATURE_ACTIVATE_TRIGGER (0x0A)` — Giggler, Dragoth (Hit?)
- `SOUND_CREATURE_YELL (0x0E)` — Thorn Demon, Dragoth (Spawn Minion) — Dragoth can spawn minions!

### Abilities
- **Spawn Minion:** YELL sound associated with "Spawn Minion" — Dragoth spawns minion creatures
- Non-material/intangible form
- w0_a_a invisibility-related ability
- Likely has missile-absorbing capability

### Dragon Door
From dm2_newfeatures.md:
- Second clan door type (0x0A) used for Skullkeep Dragon door
- Animated mirrored door flag (field 20 00 00)
- Color key fields for see-through door effects

### Dragon Significance
- The Skullkeep fortress is guarded by a dragon protecting a legendary artifact
- Lord Dragoth is the central antagonist of DM2's story
- Appears as the final boss encounter before the artifact

Source: SkWinCore.cpp:778, defines.h:350-361, DME.h:1517-1560

---

## 2. Dragoth Minion — Evil Subordinate

### Identity
- **AI Index:** 34 (SkWinCore.cpp:784)
- **Name:** DRAGOTH MINION (EVIL)
- **Type:** Minion spawned by Lord Dragoth

### Role
- Serves Lord Dragoth
- Evil version of ally minion system (indices 13-18)
- Appears as part of Dragoth's "spawn minion" ability

---

## 3. Vexirk King — Vexirk Boss

### Identity
- **AI Index:** 55 (SkWinCore.cpp:808)
- **Name:** VEXIRK KING (VEXIRK)
- **Type:** Elite Vexirk leader

### Vexirk Race Overview
- Index 48: DARK VEXIRK (VEXIRK) — standard Vexirk
- Index 55: VEXIRK KING — elite Vexirk
- Both share `flag4` (w0_4_4) bit — spectres/ghosts + vexirks

### Abilities
- Likely has unique attack pattern
- King variant would have boosted stats (HP, AttackStrength, Defense)
- Associated with Vexirk King sound triggers

---

## 4. Amplifier — Magical Object Boss?

### Identity
- **AI Index:** 51 (SkWinCore.cpp:803)
- **Name:** AMPLIFIER (MACHINE)
- **Type:** Magical machine creature

### Special Ability: Fireball
The Amplifier is patched with FIREBALL attack:
```cpp
// SkWinCore.cpp:249-252
if (SkCodeParam::bUseFixedMode)
{
    dAITable[51].AttacksSpells |= AI_ATTACK_FLAGS__FIREBALL;
    // Amplifier must remain static object, or it loses its moveable ability.
}
```
- Fireball spell (0x0010) via `OBJECT_EFFECT_FIREBALL`
- Skullkeep dungeon uses this creature with fireball attack
- Must remain static (IsStaticObject flag preserved)
- Not a traditional boss, but a dangerous magical object

---

## 5. Evil Attack Minion — Elite Minion Variants

### Identity
- **AI Index:** 43, 62 (SkWinCore.cpp:794, 810) — both named "EVIL ATTACK MINION (EVIL)"
- Same AI index for two different entries — likely duplicate or map-specific

### Role
- Evil version of Attack Minion (index 14)
- Enemy equivalent to ally companion system
- Higher threat than standard ally minions

---

## 6. Boss Encounter Locations

### Skullkeep Fortress (Main DM2 Dungeon)
- Lord Dragoth guards the final artifact
- The "Dragon Door" (second clan door type 0x0A) leads to Dragoth's chamber
- Animated mirrored door with color key effects

### Vexirk King
- Appears in Vexirk-controlled areas
- Vexirk race inhabit specific dungeon zones

### Amplifier
- Located in Skullkeep dungeon
- Fireball-creating magical machine

---

## 7. Companion/Boss Hierarchy

DM2 introduces a multi-tier creature hierarchy:

| Tier | Type | Examples |
|---|---|---|
| Ally Minions | Friendly companions | Scout, Attack, Carry, Fetch, Guard, U-Haul |
| Neutral/Object | Environment/machines | Reflector, Power Crystal, Amplifier |
| Enemy Minions | Evil subordinates | Dragoth Minion, Evil Guard, Evil Attack |
| Elite Enemies | Strong individual monsters | Vexirk King, Thorn Demon, Treant |
| Boss | Primary antagonist | Lord Dragoth |

---

## 8. Comparison: DM1 Bosses vs DM2

DM1 (from ReDMCSB) boss/unique creatures: Grond (heavy weapon boss), Golem (golem type). DM2 adds:
- Named dragon boss (Lord Dragoth) with minion-spawning ability
- Elite race leader (Vexirk King)
- Magical machine with spell-casting (Amplifier)
- Hierarchical minion system (ally vs evil variants)

---

## STATUS: SOURCE-LOCKED
