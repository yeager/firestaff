# Firestaff Code Review — Round 4: Game Loop, Event Timer, Food/Water, Skills

## Game Loop (dm1_v1_game_loop_pc34_compat.c)
**Verified correct:**
- VBlank tick simulation matches F0577
- Input wait timeout (G0318=10) correct for PC34
- Tick order matches F0002 (newMap → timeline → dungeonView → pointer → highlight → sound → damage → death → inputWait)
- Pause/resume matches G2586_TimerActive
- No bugs found. Clean implementation.

## Event Timer (dm1_v1_event_timer_pc34_compat.c)

### BUG-029: [major] Event merge scan iterates ALL slots instead of only active events
- File: src/dm1/dm1_v1_event_timer_pc34_compat.c:269-310
- ReDMCSB ref: TIMELINE.C F0238 (lines ~380-450)
- Problem: The merge logic for corridor/wall/door events scans `i = 0..maxEvents`, checking every slot including deleted ones (type == DM1_EVENT_NONE). ReDMCSB only scans the timeline array (active events). Scanning dead slots wastes time and could accidentally match stale data if a deleted event's non-type fields weren't zeroed.
- Fix: Only scan `queue->timeline[0..eventCount-1]` for merge candidates, not the raw events array.

### BUG-030: [minor] DOOR_DESTRUCTION merge doesn't check event time — only position
- File: src/dm1/dm1_v1_event_timer_pc34_compat.c:318-327
- ReDMCSB ref: TIMELINE.C F0238
- Problem: DOOR_DESTRUCTION deletes ANY door/door_animation event on the same square regardless of time. ReDMCSB checks both map AND time in its merge logic. This could prematurely delete future door events.
- Fix: Add time comparison: `DM1_MAP_TIME_TIME(ev.map_time) == DM1_MAP_TIME_TIME(existing->map_time)`.

## Food/Water (dm1_v1_food_water_pc34_compat.c)

### BUG-031: [critical] Food/water uses wall-clock milliseconds instead of game ticks
- File: src/dm1/dm1_v1_food_water_pc34_compat.c:20-45
- ReDMCSB ref: CHAMPION.C F0297_CHAMPION_DecrementStamina (ticked via GAMELOOP.C)
- Problem: Food/water decay is driven by `nowMs` (wall-clock time). ReDMCSB processes food/water/stamina decay per game tick in the main loop. Using wall-clock means: (1) decay continues when game is paused, (2) rate depends on real time not game time, (3) save/load breaks decay timing.
- Fix: Change to game-tick-based decay. Decrement food/water by fixed amount per tick, matching ReDMCSB F0297 which decrements stamina by 1 per tick and food/water by computed amount.

### BUG-032: [major] m11_fw_starve reduces food/water but they're already 0
- File: src/dm1/dm1_v1_food_water_pc34_compat.c:94-106
- Problem: When starved/thirsty flags are set (food==0 or water==0), the function subtracts MORE from already-zero values. The clamp to 0 prevents underflow but the function does nothing useful. In ReDMCSB, starvation applies HP damage via F0321 (combat damage), not further food reduction.
- Fix: Replace with HP damage per tick when starved/thirsty, matching ReDMCSB.

## Skill Experience (dm1_v1_skill_experience_pc34_compat.c)

### BUG-033: [major] Sub-skill count is wrong — should be 16, not 8
- File: src/dm1/dm1_v1_skill_experience_pc34_compat.c:44-48
- ReDMCSB ref: DEFS.H, CHAMPION.C F0303
- Problem: Code comments list 8 sub-skills but ReDMCSB has 16 sub-skills (Swing/Thrust/Club/Parry under Fighter, Steal/Fight/Throw/Shoot under Ninja, Identify/Heal/Influence/Defend under Priest, Fire/Air/Earth/Water under Wizard = 16 total). The skill index name array correctly has 20 entries (4 base + 16 sub), so the issue is only in the comment — but verify DM1_TOTAL_SKILL_COUNT == 20 in the header.
- Fix: Verify header constant matches; fix comment.

### BUG-034: [minor] Stamina base uses hardcoded 100 instead of actual champion maxStamina
- File: src/dm1/dm1_v1_skill_experience_pc34_compat.c:281-316
- ReDMCSB ref: CHAMPION.C F0304 lines 918-960
- Problem: `staminaBase = 100 >> 4` etc. uses 100 as placeholder. ReDMCSB uses `P0656_ps_Champion->Statistics[C2_STATISTIC_MANA][C0_MAXIMUM]` for the actual champion's max stamina. This means all champions get the same stamina bonus regardless of their actual stats.
- Fix: Pass champion maxStamina in ctx and use it instead of 100.

## Verified Correct
- Event timer heap (F0236 sift up/down) — correct binary min-heap
- Event ordering (F0234) — type_priority comparison matches
- Event serialization — LE format correct
- Skill level calculation (F0303) — XP threshold loop matches exactly
- Skill XP addition (F0304) — combat proximity bonus/penalty, map difficulty, temp XP all correct
- Level-up stat bonuses — per-class formulas match ReDMCSB
