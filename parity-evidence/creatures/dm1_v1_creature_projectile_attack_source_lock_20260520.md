# DM1 V1 Creature Projectile Attack Source Lock

Date: 2026-05-20
Scope: one narrow source-backed slice of the open TODO item `Creature projectile attacks (Vexirk spells, Dragon fire)`.

## ReDMCSB Source Evidence

- `GROUP.C:1645-1770`, `F0207_GROUP_IsCreatureAttacking`: computes source target cell from active-group cells and primary direction to party; gates ranged attacks with attack range > 1 and `(distance > 1 || random(2))`; selects projectile special thing by creature type; computes kinetic energy from creature attack with two random rolls; calls `F0212_PROJECTILE_Create(..., targetCell, primaryDirection, bounded(20, kineticEnergy, 255), creatureDexterity, 8)`.
- `PROJEXPL.C:43-92`, `F0212_PROJECTILE_Create`: projectile creation consumes special thing, map, cell, direction, kinetic energy, attack, and step energy; champion/creature-created projectiles schedule `C48_EVENT_MOVE_PROJECTILE_IGNORE_IMPACTS` first movement.
- `DEFS.H:421-428`: special EXPLOSION thing values used as projectile-associated things: fireball `0xFF80`, slime `0xFF81`, lightning `0xFF82`, harm-non-material `0xFF83`, open-door `0xFF84`, poison-cloud `0xFF87`.
- `BASE.C:1661-1671`, `F0026_MAIN_GetBoundedValue`: clamps the projectile kinetic energy to the inclusive source bounds.

## Firestaff Binding

- `include/dm1_v1_creature_ai_behavior_pc34_compat.h`: adds the source projectile thing constants and `DM1CreatureProjectileAttack_Compat` payload.
- `src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c`: adds `F0823_DM1_GROUP_ResolveProjectileAttack_Compat`, mirroring the ReDMCSB F0207 launch payload and exposing it through per-creature attack dispatch.
- `tests/test_dm1_v1_creature_ai_behavior_pc34_compat.c`: adds deterministic gates for red-dragon fireball launch payload, Vexirk alternate spell table coverage, and dispatch payload propagation.

## Locked Behavior

- Red Dragon/Demon/default ranged projectile payload resolves to fireball.
- Vexirk/Lord Chaos projectile table can choose fireball or harm-non-material/lightning/poison-cloud/open-door through the source random branches.
- Projectile launch direction is the current primary direction to the party.
- Projectile launch cell follows the active-group cell transform, including the single-centered `0xFF` random-cell path.
- Projectile kinetic energy is bounded to `20..255`, attack is creature dexterity, and step energy is the source constant `8`.

## Remaining Gap

This commit does not insert the projectile into the live world/timeline. Runtime insertion must still bridge the payload into the M11/phase-17 projectile list and prove first-move ignore-impact behavior against live dungeon state.
