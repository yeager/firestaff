# DM1 V1 Combat: Creature Poison DOT Source Audit

Slice: source-lock creature poison damage over time.

ReDMCSB anchors:

- `PROJEXPL.C:1305-1414`, `F0230_GROUP_GetChampionDamage`: after `F0321_CHAMPION_AddPendingDamageAndWounds_GetDamage` returns damage, lines 1404-1408 request champion damage sound, require creature `PoisonAttack`, pass a 50% `M005_RANDOM(2)` gate, adjust poison through `F0307_CHAMPION_GetStatisticAdjustedAttack(..., C4_STATISTIC_VITALITY, PoisonAttack)`, then call `F0322_CHAMPION_Poison`.
- `CHAMPION.C:1106-1120`, `F0307_CHAMPION_GetStatisticAdjustedAttack`: computes `factor = 170 - current statistic`; if below 16 returns `attack >> 3`, otherwise returns `F0030_MAIN_GetScaledProduct(attack, 7, factor)`.
- `CHAMPION.C:1926-1963`, `F0322_CHAMPION_Poison`: immediately applies `max(1, attack >> 6)` as normal pending damage, marks champion statistics/panel dirty, decrements attack, and if attack remains non-zero schedules `C75_EVENT_POISON_CHAMPION` at `GameTime + 36` while incrementing `PoisonEventCount`.
- `TIMELINE.C:1991-1993`, `C75_EVENT_POISON_CHAMPION`: decrements the champion `PoisonEventCount` and calls `F0322_CHAMPION_Poison` with the event attack payload.
- `DEFS.H:8052-8057`: exported `F0322_CHAMPION_Poison` signature.

Firestaff change:

- `dm1_creature_attack_champion()` now routes creature poison through the ReDMCSB F0230 50% gate and Vitality adjustment instead of scheduling raw poison directly.
- `dm1_combat_start_poison_pc34()` mirrors F0322 immediate damage plus 36-tick follow-up scheduling for the existing single-chain compatibility state.
- `dm1_combat_tick_poison()` now mirrors C75 dispatch by decrementing the scheduled-event count before invoking the F0322 helper again.
