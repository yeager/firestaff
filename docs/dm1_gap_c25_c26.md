# GAP C25/C26: Lord Order and Grey Lord Projectile Fallback

## Status
**RESOLVED — explicit safe fallback for ReDMCSB BUG0_13**

This gap was stale. Firestaff now defines both late creature IDs and gives
them explicit `F0823_DM1_GROUP_ResolveProjectileAttack_Compat` switch cases.
The implementation deliberately preserves the safe PC34 behavior by launching
a Fireball instead of using an uninitialized projectile thing.

## Affected Creatures

- `DM1_CREATURE_TYPE_LORD_CHAOS` (C23, index 23) — explicit boss handler
- `DM1_CREATURE_TYPE_LORD_ORDER` (C25, index 25) — explicit safe fallback
- `DM1_CREATURE_TYPE_GREY_LORD` (C26, index 26) — explicit safe fallback

## Source Evidence

- ReDMCSB `DEFS.H` lines 1364-1365 define `C25_CREATURE_LORD_ORDER`
  and `C26_CREATURE_GREY_LORD`.
- ReDMCSB `DEFS.H` line 1679 classifies Grey Lord, Lord Order, Lord Chaos,
  Materializer, Vexirk, and Wizard Eye under magic attack type C5.
- ReDMCSB `GROUP.C` line 1763 documents BUG0_13: Lord Order and Grey Lord
  can cast spells because their attack range is greater than 1, but original
  code defines no projectile type for them. If a custom dungeon places them,
  the original can create a projectile from an uninitialized thing value.

## Firestaff Behavior

`src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c` names both C25/C26 cases
inside the F0823 projectile switch and routes them to
`DM1_PROJECTILE_THING_FIREBALL`.

That is not a claim that the DOS source explicitly chose Fireball. It is a
source-cited hardening of BUG0_13: original dungeons do not contain groups of
these types, so normal DM1 play is unaffected, while custom dungeons get
deterministic behavior instead of undefined memory.

## Verification

pass1064 adds `test_lord_order_grey_lord_projectile_safe_fallback()` to
`tests/test_dm1_v1_creature_ai_behavior_pc34_compat.c`.

The test drives both C25 Lord Order and C26 Grey Lord through F0823 with
ranged caster metadata and asserts:

- resolver returns success;
- `shouldLaunch == 1`;
- projectile thing is `DM1_PROJECTILE_THING_FIREBALL`;
- spell sound fallback remains active;
- direction and attack payload still come from the source-mode context.

CTest:

```bash
ctest --test-dir build -R '^dm1_v1_creature_ai_behavior_source_lock$' --output-on-failure
```

## Remaining Work

None for DM1 finish scope. A future DOS runtime capture can document the
crash/undefined-projectile behavior directly, but Firestaff should keep the
deterministic fallback for modded/custom dungeons.
