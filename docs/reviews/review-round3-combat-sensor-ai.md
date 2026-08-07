# Firestaff Code Review — Round 3: Combat, Sensor/Trigger, Creature AI

## Combat System Bugs

### BUG-017: [critical] Poison system does instant damage instead of timed event chain
- File: src/dm1/dm1_v1_combat_pc34_compat.c:476-480
- ReDMCSB ref: CHAMPION.C F0322_CHAMPION_Poison (lines 1932-1960)
- Problem: Firestaff applies poison as immediate ATTACK_NORMAL damage: `dm1_champion_take_damage(s, targetChampIdx, poisonDmg, DM1_WOUND_NONE, DM1_ATTACK_NORMAL)`. ReDMCSB creates a C75_EVENT_POISON_CHAMPION timed event that fires every 36 game ticks with decreasing attack value (`--P0667_ui_Attack`). The champion also increments PoisonEventCount and the poison overlay is shown. Firestaff's approach means poison does all damage at once instead of over time.
- Fix: Implement poison event system — on initial poison, schedule a timed event at gameTime+36 with the full attack value. Each tick: apply max(1, attack >> 6) damage as ATTACK_NORMAL, decrement attack, reschedule if attack > 0.

### BUG-018: [major] Strength formula missing weight-load tier calculation
- File: src/dm1/dm1_v1_combat_pc34_compat.c:159-175
- ReDMCSB ref: CHAMPION.C F0312_CHAMPION_GetStrength (lines 1268-1295)
- Problem: Firestaff adds weapon strength directly: `str += ch->actionHandWeapon.strength`. ReDMCSB F0312 implements a 3-tier weight modifier:
  1. If objectWeight <= maxLoad/16: str += weight - 12
  2. If objectWeight <= threshold: str += (weight - maxLoad/16) >> 1
  3. Else: str -= (weight - threshold) << 1
  Heavy weapons should REDUCE strength when the champion can't handle them. Firestaff always adds, making heavy weapons universally good.
- Fix: Implement the 3-tier weight modifier from F0312 using dm1_combat_get_maximum_load_pc34().

### BUG-019: [major] Strength wound penalty checks wrong hand
- File: src/dm1/dm1_v1_combat_pc34_compat.c:185-187
- ReDMCSB ref: CHAMPION.C F0312 line 1300
- Problem: Firestaff always checks `DM1_WOUND_ACTION_HAND`. ReDMCSB checks which slot the weapon is in: `(P0651_i_SlotIndex == C00_SLOT_READY_HAND) ? MASK0x0001_WOUND_READY_HAND : MASK0x0002_WOUND_ACTION_HAND`. A weapon in the ready hand should check ready hand wound, not action hand.
- Fix: Pass slotIndex to dm1_champion_strength and check the correct wound mask.

### BUG-020: [major] Dexterity uses stored maxLoad instead of F0309 computed value
- File: src/dm1/dm1_v1_combat_pc34_compat.c:140-142
- ReDMCSB ref: CHAMPION.C F0311 line 1227
- Problem: Firestaff uses `ch->maxLoad` directly. ReDMCSB calls `F0309_CHAMPION_GetMaximumLoad()` which computes max load from current strength with stamina adjustment. The stored maxLoad may be stale.
- Fix: Replace `ch->maxLoad` with `dm1_combat_get_maximum_load_pc34(ch->strength)` or equivalent.

### BUG-021: [major] SELF attack missing ninja skill bonus to defense
- File: src/dm1/dm1_v1_combat_pc34_compat.c:291
- ReDMCSB ref: CHAMPION.C F0321 line ~1890 (I34E version)
- Problem: For ATTACK_SELF (champion using action on self), ReDMCSB adds ninja skill level to defense: `L0977_ui_Defense += F0303_CHAMPION_GetSkillLevel(P0662_i_ChampionIndex, C01_SKILL_NINJA)`. Firestaff only halves defense. This makes self-actions (like healing potions) riskier than intended for skilled ninjas.
- Fix: Add ninja skill bonus after the defense >>= 1 for ATTACK_SELF.

### BUG-022: [minor] Teleporter chain depth limit is 10, should be 1000
- File: src/dm1/dm1_v1_teleporter_pit_pc34_compat.c:10
- ReDMCSB ref: MOVESENS.C F0267 line ~720 (1000 iterations for teleporter, 100 for projectile)
- Problem: `s->chainDepthLimit = 10` — ReDMCSB uses 1000 for standard chains. Complex dungeon layouts with many linked teleporters could break.
- Fix: Set chainDepthLimit to 1000 (or separate limits: 1000 for party/creature, 100 for projectile).

### BUG-023: [major] Pit chain ignores starting level — always starts at level 0
- File: src/dm1/dm1_v1_teleporter_pit_pc34_compat.c:55
- Problem: `int cx = startX, cy = startY, currentLevel = 0` — the function doesn't receive the starting level. Pits on level 3 would search from level 0.
- Fix: Add startLevel parameter: `int m11_resolve_pit_chain(... int startLevel, ...)` and initialize `currentLevel = startLevel`.

### BUG-024: [minor] Weapon class boundaries wrong for THROW category
- File: src/dm1/dm1_v1_combat_pc34_compat.c:170-173
- ReDMCSB ref: CHAMPION.C F0312 lines 1287-1294
- Problem: Firestaff uses `cls != 0 && cls < 16` for THROW. ReDMCSB checks: `(Class != C000_CLASS_SWING_WEAPON) && (Class < C016_CLASS_FIRST_BOW)`. But Firestaff also checks `cls == 0 || cls == 2` for SWING above, then falls through. The logic adds THROW skill EVEN for SWING weapons (cls==0 passes the second check). ReDMCSB explicitly excludes SWING from THROW.
- Fix: The THROW block should be: `if (cls != 0 && cls < 16)` — BUT cls==2 (dagger/axes) already added SWING skill above. Need to ensure cls==2 doesn't also add THROW. Current code: cls==2 adds SWING (correct) then also adds THROW (wrong since cls!=0 && cls<16 is true for cls==2). Fix: `if (cls != 0 && cls != 2 && cls < 16)` or restructure to match ReDMCSB's exact if/else chain.

## Creature AI Bugs

### BUG-025: [major] F0817 always turns CW on 180° — should be random
- File: src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c:305
- ReDMCSB ref: GROUP.C F0205 line ~1150
- Problem: When creature faces opposite direction, code does `newDir = next_dir(direction)` (always CW). ReDMCSB uses `M017_NEXT((M006_RANDOM(65536) & 0x0002) + direction)` — the random bit selects CW or CCW. This makes creatures predictably turn right every time.
- Fix: Use RNG to pick CW or CCW: `newDir = (rng_random(2) ? next_dir(direction) : prev_dir(direction))`. Need to pass RNG state to F0817.

### BUG-026: [major] Approach walk-to-target uses party direction instead of target direction
- File: src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c:849-851
- Problem: When creature can't see party and walks to stored target position, code uses `ctx->currentGroupPrimaryDirToParty` and `ctx->currentGroupSecondaryDirToParty`. These are directions toward the party's CURRENT position, not toward activeGroup->targetMapX/Y. If party moved since the target was set, the creature walks the wrong way.
- Fix: Compute direction from current position to targetMapX/Y instead of using party-relative directions.

### BUG-027: [minor] Per-creature attack range check inverted
- File: src/dm1/dm1_v1_creature_ai_behavior_pc34_compat.c:990
- ReDMCSB ref: GROUP.C F0209 attack range gating
- Problem: `if (attackRange <= roll)` — this means creatures with LONGER range attack LESS often (need higher roll). ReDMCSB's logic at this point is more nuanced and uses the range as a probability booster, not limiter. Long-range creatures should be MORE likely to attack at range.
- Fix: Verify exact ReDMCSB condition at T0209044 and adjust the comparison.

## Sensor/Trigger System

### BUG-028: [minor] Sensor type C007 (FLOOR_CREATURE) rejects party but doesn't reject objects
- File: src/dm1/dm1_v1_sensor_trigger_pc34_compat.c:164-168
- ReDMCSB ref: MOVESENS.C F0276 case C007
- Problem: ReDMCSB checks `(L0767_i_ThingType > C04_THING_TYPE_GROUP) || (L0767_i_ThingType == CM1_THING_TYPE_PARTY) || L0773_B_SquareContainsGroup`. The "thingType > GROUP" check rejects objects AND projectiles AND any type beyond group. Firestaff only checks `thingType != CREATURE` which would accept objects.
- Fix: Check `thingType != DM1_TRIGGER_SOURCE_CREATURE || ctx->squareHasGroup` — reject everything that isn't a creature group.

## Verified Correct
- dm1_scaled_product — matches F0030
- dm1_stamina_adjusted — matches F0306
- dm1_stat_adjusted_attack — matches F0307
- dm1_wound_defense — core algorithm matches F0313
- dm1_champion_take_damage — F0321 core flow correct (wound loop, pending accumulation)
- dm1_apply_pending_damage — matches F0320
- dm1_damage_all_champions — matches F0324
- dm1_poison_adjusted_attack — matches F0192
- Sensor floor evaluation (types 0-6, 8-9) — correct per F0276
- Sensor wall evaluation (types 1-4, 11-13, 16-17, 127) — correct per F0275
- Sensor effect dispatch — matches F0272
- Creature AI wander/approach/flee — core state machine correct
- F0821 fear check — matches F0190
