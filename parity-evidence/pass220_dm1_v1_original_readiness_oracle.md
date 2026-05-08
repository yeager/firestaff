# Pass220 — DM1 V1 original post-command readiness oracle

Status: `BLOCKED_MISSING_POST_COMMAND_REDRAW_OBSERVABLE`

Scope: N2 original-runner movement capture readiness. This is a strict oracle/gate, not a new PNG/PPM capture and not a pixel-parity claim.

## ReDMCSB readiness seam

- PASS `command-queue-dispatches-turn-step` — `F0380_COMMAND_ProcessQueue_CPSC` at COMMAND.C:2045-2156: Movement captures must be after queued input reaches source turn/step dispatch, not after xdotool delivery alone.
- PASS `turn-handler-mutates-direction-and-releases-wait` — `F0365_COMMAND_ProcessTypes1To2_TurnParty` at CLIKMENU.C:142-179: Turn shots need a post-turn state mutation and source wait-loop release observable.
- PASS `step-handler-resolves-map-move-and-cooldown` — `F0366_COMMAND_ProcessTypes3To6_MoveParty` at CLIKMENU.C:180-347: Step shots need source destination resolution, move-result handling, cooldown, and wait-loop release.
- PASS `move-result-updates-party-map-time-and-sensors` — `F0267_MOVE_GetMoveResult_CPSCE` at MOVESENS.C:442-818, MOVESENS.C:1553-1793: A move command is state-ready only once source movement can mutate party X/Y/time and process sensors.
- PASS `game-loop-redraws-mutated-party-state-before-wait` — `F0002_MAIN_GameLoop_CPSDF` at GAMELOOP.C:35-97, GAMELOOP.C:215-219: Screenshot must be after the loop redraws from current direction/X/Y, not during pre-command wait/menu churn.
- PASS `dungeon-view-draw-uses-direction-and-map-coordinates` — `F0128_DUNGEONVIEW_Draw_CPSF` at DUNVIEW.C:8318-8616: Post-redraw means source dungeon view draw consumed direction/X/Y and requested viewport presentation.
- PASS `viewport-present-vblank-blit-seam` — `F0097_DUNGEONVIEW_DrawViewport` at DRAWVIEW.C:709-724, DRAWVIEW.C:840-858: A raw frame is promotable only after the viewport request/vblank blit seam can have presented it.

## Oracle rule

A screenshot is party-control/post-command ready only when all of these hold:

1. a prior `gate_confirmed_gameplay` frame has been classified as `dungeon_gameplay`;
2. the shot label is bound to a movement/turn command in the driver log before capture;
3. the shot itself is classified as `dungeon_gameplay`;
4. the raw SHA differs from the previous shot, proving at least one fresh presented frame after the command;
5. pass80 duplicate-frame strictness is clean for the movement sequence.

This visual oracle is intentionally weaker than a future memory oracle for `G0308_i_PartyDirection`, `G0306_i_PartyMapX`, `G0307_i_PartyMapY`, `G0321_B_StopWaitingForPlayerInput`, and `G0305_ui_PartyChampionCount`, but it blocks the known pass211/pass212 false positives.

## Attempt audit

- attempt dir: `<firestaff-repo>-pass220-readiness/verification-screens/pass212-n2-state-aware-movement-probe`
- classifier JSON: `<firestaff-repo>-pass220-readiness/verification-screens/pass212-n2-state-aware-movement-probe/pass80_movement_six_class_gate.json`
- classifier pass: `False`
- class counts: `{'dungeon_gameplay': 6}`
- duplicate SHA counts: `{'48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397': 6}`
- manifest rows: `6`
- driver log exists: `False`

| # | label | class | sha12 | key | key before capture | fresh vs previous |
|---|-------|-------|-------|-----|--------------------|-------------------|
| 1 | `gate_confirmed_gameplay` | `dungeon_gameplay` | `48ed3743ab6a` | `` | `None` | `None` |
| 2 | `move_up_after_gate` | `dungeon_gameplay` | `48ed3743ab6a` | `Up` | `False` | `False` |
| 3 | `turn_right_after_gate` | `dungeon_gameplay` | `48ed3743ab6a` | `Right` | `False` | `False` |
| 4 | `move_left_after_gate` | `dungeon_gameplay` | `48ed3743ab6a` | `Left` | `False` | `False` |
| 5 | `spell_or_party_key_after_gate` | `dungeon_gameplay` | `48ed3743ab6a` | `` | `None` | `False` |
| 6 | `inventory_or_party_key_after_gate` | `dungeon_gameplay` | `48ed3743ab6a` | `` | `None` | `False` |

## Decision

Blocked: the current captures are gameplay-classified but do not expose post-command redraw/state readiness.

- shot 2 `move_up_after_gate` has no logged `Up` command before capture
- shot 2 `move_up_after_gate` repeats the previous raw SHA; no post-command redraw/state observable
- shot 3 `turn_right_after_gate` has no logged `Right` command before capture
- shot 3 `turn_right_after_gate` repeats the previous raw SHA; no post-command redraw/state observable
- shot 4 `move_left_after_gate` has no logged `Left` command before capture
- shot 4 `move_left_after_gate` repeats the previous raw SHA; no post-command redraw/state observable
- pass80 duplicate-frame gate failed; repeated raw SHA values cannot prove post-command redraw

Non-claims: no <private-host> use, no push, no PNG/PPM committed, no original-vs-Firestaff pixel parity claim.
