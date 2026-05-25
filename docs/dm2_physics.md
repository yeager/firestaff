# DM2 V1 — Physics: Gravity, Projectiles, and Physical Systems in DM2

**Audit date:** 2026-05-25
**Sources:** SKULL.ASM, skproject SKWIN/SkWinCore.cpp, docs/dm2_actuators.md, docs/dm2_combat.md, docs/dm2_combat_creatures.md, docs/dm2_combat_magic.md, docs/dm2_combat_defense.md, docs/dm2_triggers.md, docs/dm2_timeline.md, docs/dm2_creatures_gfx.md, docs/dm2_items.md

---

## 1. Overview — Physics in DM1 vs DM2

DM1 had minimal physics:
- No gravity simulation
- Projectiles (crossbow bolts, thrown items) traveled instantly or in a simple arc
- No creature-to-creature physics (creatures didn't push each other)
- No dynamic collision between creatures

DM2 expands the physical simulation layer significantly:
- Projectile physics system (MISSILE_SHOOTER, WEAPON_SHOOTER, ITEM_SHOOTER actuators)
- Projectiles travel with fixed velocity until collision
- Flying item system (FLYING_ITEM_CATCHER, FLYING_ITEM_TELEPORTER)
- Creature spell projectiles (fireball, poison bolt, lightning, push/pull spells)
- No full gravity simulation (creatures don't fall between levels)

---

## 2. Projectile System

DM2 has a comprehensive projectile system. Projectiles are physical objects that travel from a source to a target.

### 2a. Actuator-Fired Projectiles

Three actuator types launch projectiles:

| Type Hex | Name | Description |
|----------|------|-------------|
| 0x08 | MISSILE_SHOOTER | Fires a magical/physical missile |
| 0x09 | WEAPON_SHOOTER | Fires a weapon projectile |
| 0x0E | ITEM_SHOOTER | Fires an item projectile |

These actuators are placed on floor cells. When triggered (via sensor), they launch a projectile in a configured direction.

Projectile behavior (from dm2_triggers.md):
- Projectiles have **fixed velocity** and travel until collision
- Velocity is hardcoded per projectile type (not configurable per instance)
- Projectile timing is actuator-specific (from shooter actuator type data)

### 2b. Weapon Projectiles

Player-thrown weapons and ranged weapons:
- Rocks, daggers (thrown)
- Crossbow bolts
- Gun shots
- Bombs (thrown explosive)

Weapon GDAT fields (from dm2_newfeatures.md and dm2_items.md):
- GDAT_ITEM_WEAPON_PROJECTILE_FLAG (0x05) - marks weapon as projectile
- Missile strength (0x09 / 0x0D field)
- Knock/hit obstacle sound (85 00 00 field)

### 2c. Magic Spell Projectiles

From dm2_combat_magic.md:
```
SPELL_TYPE_MISSILE (2) - fires magical projectile at target
```

Spell projectile types in AI_ATTACK_FLAGS (dm2_combat_creatures.md):
- FIREBALL - fire projectile
- POISON_BOLT - poison projectile
- POISON_CLOUD - area poison effect
- POISON_BLOB - blob poison projectile
- LIGHTNING - electric damage projectile
- PUSH_SPELL - force push projectile
- PULL_SPELL - force pull projectile

### 2d. Creature Projectile Attacks

From dm2_combat_creatures.md - AI_ATTACK_FLAGS in SkWinCore.cpp:
- `AI_ATTACK_FLAGS__SHOOT` (0x0008) - creature throws projectile/weapon (Archer Guard type 36)
- Electric damage projectile
- Single-target poison projectile
- Creature throws weapons/items as projectiles
- AbsorbsMissile defense - most creatures absorb projectiles instead of being hit
- ReflectMissiles defense - some creatures reflect projectiles back at attacker

### 2e. Scout Map

From dm2_quest.md: Scout map produces a large eye that scouts the area; reveals enemies, secret rooms, mana items, projectiles. The scout eye is a flying projectile with visibility.

---

## 3. Projectile Collision and Effects

Projectile collision detection:
- Projectile travels along a path until it hits a wall, creature, or party member
- On wall hit: projectile stops, knock/hit sound plays (85 00 00 in GDAT)
- On creature hit: damage applied, creature hit sound plays
- On party hit: champion takes damage

Defensive properties (from dm2_combat_defense.md):
- `AbsorbsMissile` - projectile is absorbed, no damage
- `ReflectsMissiles` - projectile redirected back at attacker (different from absorption)

Flying item handling:
- FLYING_ITEM_CATCHER (0x22) - floor actuator that catches projectile items
- FLYING_ITEM_TELEPORTER (0x23) - floor actuator that teleports projectile items

---

## 4. Ranged Combat Physics

DM2 ranged combat uses a distance-based physics model:

```
damage = weapon->base_damage + attacker_strength / 4;
if (distance > weapon->range) return 0;
range_penalty = (distance - 1) * damage / 10;  // -10% per extra tile beyond first
damage -= target_defense;
return damage > 0 ? damage : 0;
```

Key points:
- Damage falls off with distance (range_penalty = -10% per extra tile)
- Strength adds a bonus (attacker_strength / 4)
- Defense subtracts from damage
- Minimum damage = 0

Bombs (AoE weapons):
- Thrown to a target tile
- On impact: all creatures in blast radius take damage
- Bombs do not individually target creatures

---

## 5. Creature Spell Physics

Creatures can cast spells that produce physical effects:

- **FIREBALL**: projectile with fire damage, travels to target tile, explodes
- **POISON_BOLT**: single-target poison projectile
- **POISON_CLOUD**: area-of-effect poison at target location
- **POISON_BLOB**: blob-shaped poison projectile
- **LIGHTNING**: electric projectile
- **PUSH_SPELL**: projectile that pushes target creature away
- **PULL_SPELL**: projectile that pulls target creature closer

Push/pull spells are the closest DM2 gets to creature-creature physics interaction.

---

## 6. Gravity

DM2 does NOT simulate gravity in the traditional sense:
- Creatures do not fall between levels
- Falling pits (DM1 C02_ELEMENT_PIT) are a trigger/special square, not a physics simulation
- The closest to gravity is the PUSH_SPELL (which pushes a creature a few tiles)

Vertical movement between levels is handled purely by ladder/stairs actuators (ACTUATOR_TYPE_LADDER = 0x11), not by physics.

---

## 7. Item Physics

Items on the ground:
- Items sit on floor tiles
- FLYING_ITEM_CATCHER (0x22): floor actuator catches flying items
- FLYING_ITEM_TELEPORTER (0x23): teleports flying items
- ITEM_CAPTURE (0x47): floor actuator that removes items from floor
- ITEM_RECYCLER (0x40): floor actuator that removes items permanently

Items can be thrown as projectiles (ITEM_SHOOTER = 0x0E), traveling until collision.

---

## 8. Timer-Driven Physics

DM2 uses timers for physical processes:

From dm2_timeline.md:
- Projectile timing: hardcoded velocity, shooter actuator with type-specific data
- Projectiles had fixed velocities and traveled until collision
- Timers used for: blinking lights, recurring projectiles, self-resetting mechanisms

CONTINUE_TICK_GENERATOR drives the physics tick - the game loop processes projectile positions each tick.

---

## 9. Creature Creature Interaction

Creatures do not collide with each other in DM2. Their interaction is purely through:
- Combat (melee, ranged, spell attacks)
- AI pathfinding (c_ai routes around obstacles, but creatures don't push each other)
- Push/pull spells (creature displaced by spell force)

There is no fall damage, no creature falling into pits, no creature stacking.

---

## 10. Environmental Physics

Environmental physical effects in DM2:
- **Doors**: slide or swing open/closed based on actuator state
- **Pits**: floor squares that can be open/closed via triggers (C02_ELEMENT_PIT carried from DM1)
- **Teleporter floors**: instant teleport on step (no physics travel)
- **Ornate Animator** (0x2C): visual animation of wall ornaments (levers moving, etc.)

No environmental physics like pressure plates being physically depressed (beyond the trigger state change).

---

## 11. Comparison: Physics

| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Gravity simulation | None | None (vertical movement via ladders) |
| Projectile system | Simple (crossbow, thrown) | Comprehensive (3 actuator types + magic) |
| Projectile collision | Instant/hit-scan | Fixed velocity, travels until collision |
| Projectile types | Crossbow bolt, thrown | Missile, weapon, item, spell (7+ types) |
| Range penalty | None | -10% per extra tile |
| Strength bonus | None | attacker_strength / 4 |
| AoE weapons | None | Bombs (blast radius) |
| Push/pull physics | None | Push/pull spell projectiles |
| Creature collision | None | None (creatures don't push each other) |
| Item physics | Simple pickup | ITEM_SHOOTER, FLYING_ITEM_CATCHER/TELEPORTER |
| Creature spell attacks | None | Fireball, poison, lightning, push/pull |
| Absorb/reflect | None | AbsorbsMissile, ReflectsMissiles |
| Scout eye | None | Scout map produces flying visibility projectile |

---

## STATUS: AUDIT COMPLETE