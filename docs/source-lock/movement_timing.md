# JOB 5: Source-lock DM1 V1 — Movement Timing/Pace

## Per-Step Cooldown

Ref: CLIKMENU.C:330-346 — after successful step:
```
AL1115_ui_Ticks = 1;
for each living champion:
    AL1115_ui_Ticks = F0025_MAIN_GetMaximumValue(
        AL1115_ui_Ticks,
        F0310_CHAMPION_GetMovementTicks(L1119_ps_Champion));
G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;
G0311_i_ProjectileDisabledMovementTicks = 0;
```

Party step cost = max movement ticks across all living champions.

## Per-Champion Movement Tick Formula

Ref: CHAMPION.C:1180-1215 — F0310_CHAMPION_GetMovementTicks:

```
if (Load < MaximumLoad) {           // BUG0_72: should be <=
    ticks = 2;
    if ((Load * 5) > (MaximumLoad * 5)) {  // load > 62.5% of max
        ticks++;                          // ticks = 3
    }
    wound_ticks = 1;
} else {
    ticks = 4 + ((Load - MaximumLoad) * 4 / MaximumLoad);  // 4..7+ when overloaded
    wound_ticks = 2;
}
if (feet_wounded)    ticks += wound_ticks;
if (boots_of_speed)  ticks--;           // minimum 1 tick even with boots
```

Summary table:
- Unloaded (<62.5% max): 2 ticks
- Loaded (>62.5% max, not at limit): 3 ticks
- At maximum load exactly (BUG0_72: slow path): 4+ ticks (overloaded formula)
- Overloaded: 4 to 7+ ticks depending on overload amount
- With wounded feet: +1 or +2 additional ticks
- With Boots of Speed: -1 tick (minimum 1)

## Cooldown Decrement

Ref: GAMELOOP.C:150-155 — each game loop tick:
```
if (G0310_i_DisabledMovementTicks)
    G0310_i_DisabledMovementTicks--;
if (G0311_i_ProjectileDisabledMovementTicks)
    G0311_i_ProjectileDisabledMovementTicks--;
```

Both cooldowns decrement by 1 per game tick (not per vertical blank).

## Input Wait Window

Ref: GAMELOOP.C:47-50:
- PC-34/I34E: G0318_i_WaitForInputMaximumVerticalBlankCount = 12
- Other versions: 10

Ref: GAMELOOP.C:164-219 — main loop drains keyboard input, processes one queue slot, then loops while !G0321_B_StopWaitingForPlayerInput or !game_time_ticking.

VBlank waits do NOT decrement G0310/G0311 (IO.C:772-778 and 1015-1019 advance the VBlank counter but do not touch movement cooldowns).

## Stamina Cost Per Step

Ref: CLIKMENU.C:237-248:
```
F0325_CHAMPION_DecrementStamina(champion_index,
    ((load * 3) / F0309_CHAMPION_GetMaximumLoad(champion)) + 1);
```
Per-step stamina cost: floor(load/maxLoad * 3) + 1.

## Scent Recording

Ref: MOVESENS.C:752-775 — after successful move:
- If party changed map or square (has living champions):
  G0362_l_LastPartyMovementTime = G0313_ul_GameTime
- Scent trail is updated with game tick of last movement

## Firestaff Implementation

Source: src/dm1/dm1_v1_movement_timing_pc34_compat.c
- DM1_V1_MovementTiming_ComputeChampionTicksPc34Compat — mirrors F0310
- DM1_V1_MovementTiming_ComputePartyStepTicksPc34Compat — max across living champions
- DM1_V1_MovementTiming_ApplySuccessfulStepPc34Compat — sets G0310 and scent time
- DM1_V1_MovementTiming_DecrementCooldownsPc34Compat — called per tick, decrements both
- DM1_V1_MovementTiming_InputWaitMaxVBlanksPc34Compat — returns 12 for PC-34

Source citations: CLIKMENU.C:237-248,330-346; CHAMPION.C:1180-1215; GAMELOOP.C:47-50,150-155,164-219; MOVESENS.C:752-775.

## Status

PASS — Movement pace is the max of F0310_CHAMPION_GetMovementTicks across all living champions, stored in G0310 and decremented once per game tick. Unloaded champions move in 2 ticks; overloaded or wounded champions slower. Input window is 12 VBlanks for PC-34. VBlank wait does NOT decrement cooldowns. ReDMCSB fully traced.
