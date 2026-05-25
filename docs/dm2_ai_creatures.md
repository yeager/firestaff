# DM2 V1 Creature AI — State Machines & Behavior

## Source Evidence
- `skproject/SKULLWIN/c_ai.cpp` — `DM2_THINK_CREATURE`, `DM2_PROCEED_XACT_*`
- `skproject/SKULLWIN/c_creature.cpp` — `DM2_PROCEED_CCM`, `DM2_CREATURE_ATTACKS_PARTY`
- `skproject/SKULLWIN/c_creature.h` — `c_creature` class, `b_1a` (action pattern), `b_17` (pre-action pattern)
- `src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c` — DM1 reference

## DM2 vs DM1: Fundamental AI Differences

### DM1: Hardwired Sensor/Actor (ReDMCSB)
DM1 creature AI used a tight sensor-think-act loop locked to ReDMCSB GROUP.C / MOVESENS.C / TIMELINE.C:
- `F0209_GROUP_ProcessEvents29to41` — primary event dispatcher
- Pure directional movement toward party (smell/direction, not pathfinding)
- Attack range baked in per-creature type
- No state variable per se; behavior was a function of distance + creature type + random seed
- Single-square movement with quarter-square adjustment for larger creatures
- Flee behavior triggered by fear check after taking damage

### DM2: Action-Dispatch State Machine (CCM Pattern)
DM2 replaces the hardwired sequence with a **bytecode-like dispatch** through `DM2_PROCEED_CCM`:

The `b_1a` byte is the **primary state machine register** for the creature's current action. The `b_17` byte is a secondary context byte. `DM2_PROCEED_CCM` dispatches based on `b_1a` values:

- 0x00-0x01: WALK_NOW
- 0x01-0x02: attack logic
- 0x02-0x05: WALK_NOW (movement dispatch)
- 0x05-0x07: CCM06 / CCM0B / CCM0C (special actions)
- 0x09-0x0a: STEAL_FROM_CHAMPION (thief-type steals item)
- 0x0a-0x0d: CCM0B, CCM0C (merchant/shop behavior)
- 0x0d-0x0f: SHOOT_ITEM (ranged throw)
- 0x0f-0x13: KILL_ON_TIMER_POSITION
- 0x13-0x15: ROTATES_TARGET_CREATURE
- 0x15-0x17: CAST_SPELL
- 0x17-0x2b: merchant/combat idle
- 0x26-0x28: EXPLODE_OR_SUMMON
- 0x17-... : CREATURE_ATTACKS_PARTY (fallback)

This is a qualitative leap over DM1's inline distance checks — DM2 encodes **discrete behavioral states** as named command values.

### State Transition Mechanism
- `DM2_PROCEED_CCM` is called each game tick per creature
- No explicit "next state" field — state is encoded in `b_1a`
- The action handler writes the next `b_1a` command byte directly
- Creature can be mid-action and interrupted by a new party proximity event

## New DM2 Creature Action Types

| Action | Hex | Description | DM1 Equivalent |
|--------|-----|-------------|----------------|
| STEAL_FROM_CHAMPION | 0x09-0x0A | Thief-type steals item | None |
| SHOOT_ITEM | 0x0D-0x0F | Throws/picks up projectiles | None |
| CAST_SPELL | 0x27-0x28 | Monster spellcasting | None |
| EXPLODE_OR_SUMMON | 0x26-0x28 | Self-destruct or spawn minions | None |
| KILL_ON_TIMER_POSITION | 0x0F-0x13 | Delayed-position kill trigger | None |
| ROTATES_TARGET_CREATURE | 0x13-0x15 | Reorients another creature | None |
| CCM0B/CCM0C | 0x0A-0x0D | Non-combat interactive behavior | None |
| Merchant actions | 0x17-0x2B | Shop/treasure placement/take | None |

## Attack Implementation
- `DM2_CREATURE_ATTACKS_PARTY()` — loops over all 4 champions
- `DM2_ATTACK_CREATURE()` — resolves attack, accounts for attack range
- Range check: `distance > weapon->range` = no attack
- Ranged penalty: `-10% damage per extra tile` (crossbows, guns, bombs)
- No pathfinding-based attack — attacks are directional/adjacent only

## Key Structural Difference
DM1 encodes behavior **inline in distance checks**. DM2 encodes behavior as **named command bytes dispatched through a state machine** (`DM2_PROCEED_CCM`). This makes DM2 behavior data-driven — the creature database's AI spec flags determine which command byte patterns are valid for each creature type.
