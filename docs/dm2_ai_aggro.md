# DM2 V1 Aggro System — Line-of-Sight, Range, Reset

## Source Evidence
- `skproject/SKULLWIN/c_creature.cpp` — `DM2_CREATURE_ATTACKS_PARTY`, `DM2_PROCEED_CCM`
- `skproject/SKULLWIN/c_creature.h` — `c_creature` struct (target: `w_18` x/y), attack range per weapon
- `skproject/SKULLWIN/c_ai.cpp` — `DM2_FIND_WALK_PATH`, `DM2_CREATURE_GO_THERE`
- `skproject/SKULLWIN/c_cloud.cpp` — cloud occlusion handling
- `src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c` — DM1 aggro: F0200, F0201, F0208, F0209
- `src/dm2/dm2_v1_combat.c` — ranged attack range penalty, LOS check

## DM1 Aggro System (ReDMCSB)
From ReDMCSB GROUP.C / MOVESENS.C:
- F0200_GROUP_GetDistanceToVisibleParty: Manhattan distance from group to party
- F0201_GROUP_GetSmelledPartyPrimaryDirectionOrdinal: Smell-based aggro (no LOS required)
- F0208: Wall/obstacle check during movement
- F0209 T0209085_SingleSquareMove: movement triggered by aggro

DM1 aggro is purely **distance-based** (Manhattan). No line-of-sight check. If party is within aggro range, creature moves toward them regardless of walls or visibility. Smell works through walls at longer range.

## DM2 Aggro: Distance + LOS + Attack Range

### Attack Range
From `dm2_v1_combat.c`:
```c
int dm2_v1_combat_resolve_attack(const DM2_V1_WeaponInfo *weapon,
    int attacker_strength, int target_defense, int distance)
{
    damage = weapon->base_damage + attacker_strength / 4;
    if (distance > weapon->range) return 0; /* out of range */
    range_penalty = (distance - 1) * damage / 10;
    damage -= range_penalty;
    damage -= target_defense;
    return damage > 0 ? damage : 0;
}
```
Range is per-weapon and per-creature-type. Creature attacks fail if distance > weapon range.

### Line-of-Sight
DM2 uses `DM2_FIND_WALK_PATH` (from c_ai.cpp) for pathfinding:
```c
RG1L = DM2_FIND_WALK_PATH(unsignedlong(s350.v1e0562.getxA()),
                          unsignedlong(s350.v1e0562.getyA()),
                          lcon(0x3), RG1W, ...);
```
This is a tile-based pathfinding check. If a walkable path exists, creature can navigate to party. If not, creature cannot reach party even if within distance.

### Cloud Occlusion
From c_cloud.cpp, clouds (spell effects) block movement and possibly line-of-sight:
- Cloud tiles occupy map squares
- DM2_FIND_WALK_PATH must navigate around cloud effects
- Creatures in clouds may have impaired aggro

### Ranged Attack Aggro
From `dm2_v1_combat.c`:
```c
int dm2_v1_combat_is_ranged(DM2_WeaponType type) {
    return type == DM2_WEAPON_CROSSBOW || type == DM2_WEAPON_GUN ||
           type == DM2_WEAPON_THROWN || type == DM2_WEAPON_BOMB;
}
```
Ranged creatures (crossbow, gun, thrown, bomb) can initiate attack from distance > 1. Their aggro range extends beyond melee range.

## Aggro Trigger Mechanism
In `DM2_PROCEED_CCM`, aggro logic is implicit:
1. Party within creature attack range -> `DM2_CREATURE_ATTACKS_PARTY` called (fallback action)
2. Creature mid-action (`b_1a` state) — no new aggro trigger until action completes
3. `DM2_THINK_CREATURE` evaluates target position each tick

The `w_18` field in `c_creature` holds creature target position (x/y). This is the aggro target — set when creature first detects party and used for pathfinding.

## DM1 vs DM2 Aggro Comparison
| Aspect | DM1 | DM2 |
|--------|-----|-----|
| Primary trigger | Manhattan distance | Distance + attack range |
| LOS check | None | Pathfinding-based (DM2_FIND_WALK_PATH) |
| Smell through walls | Yes (F0201) | Not clearly found in skproject |
| Ranged aggro | None (attack only at distance=1) | Yes (crossbow, gun, thrown, bomb) |
| Aggro reset | When party leaves aggro range | Implicit when target unreachable |
| Cloud occlusion | N/A | Yes (cloud tiles block pathfinding) |
| Attack range | Fixed per creature type | Per-weapon dynamic |
| Target tracking | None (re-evaluate each tick) | `w_18` holds persistent target coords |

## Aggro Range in DM2
No single global aggro range constant found in skproject. Instead:
1. **Per-weapon**: `weapon->range` from `DM2_V1_WeaponInfo`
2. **Per-creature-AI-spec**: AI spec flags determine valid actions (ranged vs melee)
3. **Pathfinding-dependent**: Even within weapon range, a wall blocks reaching target

## Open Questions
- No explicit AGGRO_RANGE constant found in skproject — likely in AI spec flags
- Smell sensor (DM1 F0201) has no clear DM2 equivalent — may have been removed
- Aggro reset behavior not yet reverse-engineered
