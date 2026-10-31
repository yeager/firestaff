# M11 Ownership Audit: Movement, Doors, and Environment

## Scope

Bounded parity audit against local ReDMCSB sources for these ownership areas only:

- movement ownership
- door/environment ownership
- sensor/environment interaction ownership

Primary comparison targets:

- Firestaff: `memory_movement_pc34_compat.c`, `memory_sensor_execution_pc34_compat.c`, `memory_tick_orchestrator_pc34_compat.c`, `m11_game_view.c`
- ReDMCSB: `MOVESENS.C`, `DUNGEON.C`, `COMMAND.C`

Working rule for this audit: when Firestaff differs from ReDMCSB/original, assume Firestaff is wrong unless there is strong evidence otherwise.

---

## Executive summary

Firestaff currently has **basic command-to-step ownership in compat**, but **observable environment behavior is still largely owned by custom M11 glue**.

The biggest parity gap is not rendering. It is ownership:

1. `memory_movement_pc34_compat.c` only owns a **thin wall/bounds step test**.
2. `memory_sensor_execution_pc34_compat.c` is **probe-grade**, not runtime-grade: it only executes teleport/text and only for `WALK_ON`.
3. `m11_game_view.c` still owns several behaviors that ReDMCSB keeps inside the movement/sensor runtime path:
   - stairs transitions
   - pit falling
   - teleporter chaining/application
   - front-door toggle/open-close behavior
   - post-move environment chaining
4. Sensor processing is **not part of the main compat movement runtime**. The main runtime path uses `F0702_MOVEMENT_TryMove_Compat` via the tick orchestrator, but does not route party movement through source-faithful sensor addition/removal processing comparable to `F0267_MOVE_GetMoveResult_CPSCE` + `F0276_SENSOR_ProcessThingAdditionOrRemoval`.

That means Firestaff movement may look functional, but ownership is still upside down relative to ReDMCSB: **M11 UI glue is deciding world behavior that should belong to compat/runtime movement and sensor code.**

---

## Source-faithful ownership baseline from ReDMCSB

### ReDMCSB ownership centers

| ReDMCSB area | Owns in original/runtime-faithful path |
|---|---|
| `COMMAND.C` | input/command dispatch only; it does not custom-script environment rules per viewport action |
| `MOVESENS.C:F0267_MOVE_GetMoveResult_CPSCE` | movement resolution, chained moves, teleport/pit consequences, party/group movement side effects |
| `MOVESENS.C:F0276_SENSOR_ProcessThingAdditionOrRemoval` | sensor triggering on leave/enter/add/remove events |
| `MOVESENS.C:F0270/F0271/F0275` | local sensor effects, rotation batching, click-on-wall sensor logic |
| `DUNGEON.C` | square/thing data, door definitions/attributes, fake-wall and square semantics helpers |

### Important implication

In the original ownership model, **movement and environment behavior are not UI helpers**. They are runtime rules executed inside movement/sensor processing. Firestaff currently violates that boundary in several places.

---

## Firestaff ownership map (POST passes 29–32, as of 2026-04-24)

| Firestaff file/function | Current role | Closest ReDMCSB ownership area | Current ownership verdict |
|---|---|---|---|
| `memory_movement_pc34_compat.c:F0700_MOVEMENT_TurnDirection_Compat` | turn math | `MOVESENS.C` direction math | already compat-owned |
| `memory_movement_pc34_compat.c:F0701_MOVEMENT_GetStepDelta_Compat` | relative step delta | `MOVESENS.C` direction/step logic | already compat-owned |
| `memory_tick_orchestrator_pc34_compat.c:cmd_to_move_action` | absolute command -> relative move action | `COMMAND.C`/movement bridge | already compat-owned |
| `memory_movement_pc34_compat.c:F0702_MOVEMENT_TryMove_Compat` | bounds + wall + **door state + fake wall + stairs** (pass 30) | broader subset of `F0267_MOVE_GetMoveResult_CPSCE` | **compat-owned** (post pass 30); animating intermediate door states 1..3 now owned by the timeline stepper (pass 38), so “door blocks step” at state 2 remains honest |
| `memory_movement_pc34_compat.c:F0705_MOVEMENT_ResolveStairsTransition_Compat` | stairs level change + bounds clamp (pass 30) | stairs branch of `F0267_MOVE_GetMoveResult_CPSCE` | **compat-owned** (post pass 30) |
| `memory_movement_pc34_compat.c:F0706_MOVEMENT_IsSquarePassable_Compat` | shared square passability (pass 30) | square-element helpers in `DUNGEON.C` | **compat-owned** (post pass 30); party-side behavior unchanged; creature-side now delegates via F0707 (pass 39) |
| `memory_movement_pc34_compat.c:F0707_MOVEMENT_IsSquarePassableForContext_Compat` | context-aware square passability (PARTY / CREATURE) (pass 39) | square-element helpers in `DUNGEON.C` + creature legality in `GROUP.C` / `F0264_MOVE_IsSquareAccessibleForCreature` | **compat-owned** (post pass 39); shares element/door decoding with F0706, rejects stairs in CREATURE context |
| `memory_movement_pc34_compat.c:F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat` | chained pit + teleporter post-move resolution (pass 29) | `F0267_MOVE_GetMoveResult_CPSCE` chained moves | **compat-owned** (post pass 29) |
| `memory_movement_pc34_compat.c:F0703_MOVEMENT_IdentifySensorsOnSquare_Compat` | find first sensor on square | small probe subset of `MOVESENS.C` sensor traversal | retained (still first-sensor-only) |
| `memory_sensor_execution_pc34_compat.c:F0710_SENSOR_Execute_Compat` | probe API for teleport/text `WALK_ON` effects | tiny subset of `F0276` sensor result path | probe-grade retained; production party route uses F0718/F0719 |
| `memory_sensor_execution_pc34_compat.c:F0717_SENSOR_EnumerateOnSquare_Compat` | enumerate ALL sensors on a square (pass 32) | thing-list walker inside `F0276_SENSOR_ProcessThingAdditionOrRemoval` | **compat-owned** (post pass 32) |
| `memory_sensor_execution_pc34_compat.c:F0718/F0719_SENSOR_ProcessPartyEnterLeave*_Compat` | process party enter/leave events over sensors on a square, including C001-C009 gates and local C10/deferred rotation receipts | `F0276_SENSOR_ProcessThingAdditionOrRemoval` | **compat-owned AND runtime-owned (partial)**; movement routes produce effects in source order, and M11 applies F0272 OnceOnly disabling, dispatches C10 XP and applies final F0271 CLEAR/TOGGLE rotation per effect batch. Party-route OnceOnly disabling is source-shaped; leader-only C10, once-only behavior on other event routes, other local effects and original-media evidence remain open. |
| `memory_sensor_execution_pc34_compat.c:F0276_SENSOR_ProcessObjectFloorChange_Compat` + M11 pickup/drop hooks | bounded DM1 PC34 floor object add/remove admission for C001/C002/C004/C007, with F0272 effect receipts after successful M11 chain mutation | object add/remove branch of `F0276_SENSOR_ProcessThingAdditionOrRemoval` | **partial runtime-owned**; keyboard and rendered-pile M11 pickup/drop routes execute the bounded floor pass. Wall cell branches, other object mutation routes, non-PC34 editions/platforms and complete object-specific consequences remain open. |
| `memory_door_action_pc34_compat.c:F0715_DOOR_ResolveToggleAction_Compat` | decide OPEN/CLOSE/DESTROYED target state (pass 31) | door actuator branch inside `MOVESENS.C` / `DUNGEON.C` | **compat-owned** (post pass 31) |
| `memory_door_action_pc34_compat.c:F0716_DOOR_RouteFrontCellClick_Compat` | categorise click on front cell (pass 31) | `F0275_SENSOR_IsTriggeredByClickOnWall` | **compat-owned** (post pass 31) |
| `memory_door_action_pc34_compat.c:F0714_DOOR_ResolveAnimationEffect_Compat` | resolve SET/CLEAR/TOGGLE vs current door state (pass 38) | TOGGLE branch inside `F0244_TIMELINE_ProcessEvent10_Square_Door` | **compat-owned** (post pass 38) |
| `memory_door_action_pc34_compat.c:F0713_DOOR_BuildAnimationEvent_Compat` | build `TIMELINE_EVENT_DOOR_ANIMATE` event (pass 38) | `F0238_TIMELINE_AddEvent_GetEventIndex_CPSE` call sites in TIMELINE.C | **compat-owned** (post pass 38) |
| `memory_door_action_pc34_compat.c:F0712_DOOR_StepAnimation_Compat` | walk one door-state step (SET 4→0 / CLEAR 0→4) (pass 38) | `F0241_TIMELINE_ProcessEvent1_DoorAnimation` step branch | **compat-owned** (post pass 38); party/creature hazard branches intentionally deferred |
| `memory_tick_orchestrator_pc34_compat.c:F0887_ORCH_DispatchTimelineEvents_Compat` (`TIMELINE_EVENT_DOOR_ANIMATE` case) | dispatch animating door step, emit DOOR_STATE + rattle, reschedule at gameTick+1 (pass 38) | `F0241_TIMELINE_ProcessEvent1_DoorAnimation` reschedule tail | **runtime-owned** (post pass 38) |
| `m11_game_view.c:m11_try_stairs_transition` | thin UI shim over F0705 (pass 30) | — | **reduced to UI delegation** (post pass 30); only owns status/log/inspect text + explored-bits reset |
| `m11_game_view.c:m11_apply_post_move_environment_from_compat` | thin UI shim over F0704 (pass 29) | — | **reduced to UI delegation** (post pass 29); only owns status/log/inspect text + explored-bits reset + champion fall-damage application |
| `m11_game_view.c:m11_check_pit_fall`, `m11_check_teleporter`, `m11_check_post_move_transitions` | former owners, now deprecated bodies | — | **no longer authoritative** (post pass 29); F0704 is the owner |
| `m11_game_view.c:m11_toggle_front_door` | thin UI shim over F0715 (pass 31) | — | **reduced to UI delegation** (post pass 31); owns only status/inspect text + synthetic tick notifications + world-byte write |
| `m11_game_view.c:m11_front_cell_is_door` | query helper; acceptable UI concern | — | viewport-local query, behavior-free |
| `m11_game_view.c:m11_square_walkable_for_creature` | thin delegation to F0707 with MOVEMENT_PASS_CTX_CREATURE (pass 39) | shared compat decoder via F0707 | **reduced to compat delegation** (post pass 39); no custom element/door switch remains |

---

## Biggest ownership mismatches

## 1) Compat movement owns too little; M11 owns too much

### Firestaff
`F0702_MOVEMENT_TryMove_Compat` currently:

- handles turn-only updates
- computes relative deltas
- blocks out-of-bounds
- blocks walls
- treats every other non-wall square as passable

It explicitly leaves this note:

- door open/closed checks are future work
- fake-wall logic is future work

### ReDMCSB baseline
`F0267_MOVE_GetMoveResult_CPSCE` is not a simple walkability check. It owns:

- adjacent movement vs teleport movement distinctions
- chained move handling
- pit/teleporter consequences
- party/group side effects
- sensor processing entry points
- projectile/group interaction consequences
- map transition consequences

### Audit verdict
**Current owner is wrong.** Firestaff compat movement is still a thin prefilter, while M11 glue implements the observable environment outcomes afterward.

---

## 2) Door behavior is still viewport-glue-owned

### Firestaff
`m11_toggle_front_door` directly mutates the low door-state bits in the square and emits synthetic door/sound events.

### ReDMCSB baseline
Door/environment state changes in the original runtime are part of dungeon/sensor/action processing, not a viewport-local direct square patch. `COMMAND.C` is command dispatch, not the final door authority.

### Why this matters
This keeps door ownership in the UI layer:

- state mutation policy is decided by the viewport
- door open/close semantics are reduced to `0 <-> 4`
- source-faithful interactions with actuators, sensors, door types, destruction, and side effects are bypassed

### Audit verdict
**Strong custom-glue ownership mismatch.** This is one of the highest-value ownership fixes.

---

## 3) Pit, teleporter, and stairs outcomes are custom post-move patches

### Firestaff
After the orchestrator moves the party, `m11_apply_tick` calls `m11_check_post_move_transitions`, which runs:

- `m11_check_pit_fall`
- `m11_check_teleporter`
- chained loops in `m11_check_post_move_transitions`
- separate `m11_try_stairs_transition` on explicit input

### ReDMCSB baseline
`F0267_MOVE_GetMoveResult_CPSCE` owns chained movement results directly, including teleporter/pit consequences. Sensor and environment processing are not bolted on afterward in the view layer.

### Specific parity risks in Firestaff
- pit open-state handling is explicitly non-faithful (`pitIsOpen = ... ? 1 : 1`)
- stairs use a simplified same-X/Y map-index shift rule
- teleporter application is driven by M11 helper logic rather than runtime movement ownership
- transition chain limit is a custom UI/runtime safeguard, not source-owned behavior

### Audit verdict
**Large custom-glue-owned mismatch.** This is the clearest place where visible world behavior still lives outside compat/runtime.

---

## 4) Sensor execution is not the runtime owner yet

### Firestaff
`F0703_MOVEMENT_IdentifySensorsOnSquare_Compat` + `F0710_SENSOR_Execute_Compat` remain probe-oriented. Party movement reaches the production F0718/F0719 route, but that route still covers only part of ReDMCSB's full sensor event/effect ownership.

Observed limits:

- the older F0710 probe API still surfaces a bounded effect result; production party movement uses F0718/F0719 and enumerates sensors in source order
- the party route handles C001-C009 admission, remote effects, text/teleport and C10 group-split Steal XP; CLEAR/TOGGLE local rotation is deferred until the end of each sensor batch
- OnceOnly disabling is implemented for party effects; its other event routes, leader-only C10 and other F0270 local effects remain unsupported
- full thing add/remove event coverage comparable to `F0276_SENSOR_ProcessThingAdditionOrRemoval` is incomplete; only bounded DM1 PC34 M11 keyboard/rendered-pile floor pickup/drop routes now invoke object admission, while wall-cell branches and other mutation paths remain open
- no click-on-wall routing comparable to `F0275_SENSOR_IsTriggeredByClickOnWall`
- remaining F0270 local-effect cases and non-party once-only routes are not source-complete; F0271 CLEAR/TOGGLE batching is wired for party WALK_ON

### ReDMCSB baseline
Sensor/environment interaction ownership is broad and event-driven, not a single-square single-sensor single-event adapter.

### Audit verdict
**Compat exists, but it is not yet the true owner.** Runtime ownership still effectively sits outside compat.

---

## 5) Fake-wall / door-state / square-semantics ownership source-lock

### Firestaff
Movement compat owns shared square passability through `F0706` / `F0707` and now matches the ReDMCSB edge cases: door states `0`, `1` (closed one-fourth), and `5` are passable; door states `2..4` block; fake walls are passable only with OPEN (`0x04`) or IMAGINARY (`0x01`) set.

### ReDMCSB baseline
Square semantics are source-owned in `CLIKMENU.C:F0366_COMMAND_ProcessTypes3To6_MoveParty`, including fake-wall handling and door-state consequences.

### Audit verdict
**Compat-owned for the square-semantics subset.** Remaining movement gaps are sensor/group side effects and runtime integration breadth, not the basic door/fake-wall square decoder.

---

## Ownership classification by area

### Movement ownership

- **Already compat-owned**
  - direction turning
  - relative move delta calculation
  - command-to-relative-move translation in orchestrator

- **Custom-glue-owned**
  - chained environment transitions after movement
  - stairs traversal behavior
  - pit fall application
  - teleporter application and chaining in M11

- **Unclear/mixed**
  - fake-wall semantics
  - door passability semantics
  - creature walkability vs party walkability consistency

### Door/environment ownership

- **Already compat-owned**
  - none that materially resolve runtime door behavior

- **Custom-glue-owned**
  - front-door toggle/open-close mutation
  - direct square-bit mutation for door state
  - sound/event emission around door toggles
  - stairs and pit outcome ownership in M11

- **Unclear/mixed**
  - door type-specific semantics vs simple open/closed bits
  - destroyed-door handling beyond simple special-case messaging

### Sensor/environment interaction ownership

- **Already compat-owned**
  - minimal probe-level sensor decode/serialization pieces

- **Custom-glue-owned**
  - actual post-move teleporter behavior in M11
  - wall-click / front-cell interaction behavior remains view-driven

- **Unclear/mixed**
  - multi-sensor ordering
  - sensor enter/leave/add/remove semantics
  - remaining local effect kinds and once-only mutation on non-party routes
  - click-on-wall actuator semantics

---

## Highest-value bounded next implementation slices

These are the best next passes to reduce custom ownership without attempting a full movement rewrite at once.

| Priority | Bounded slice | Why it matters | Expected ownership shift |
|---|---|---|---|
| 1 | Move pit + teleporter chain resolution out of `m11_game_view.c` and into compat runtime movement result processing | Biggest visible mismatch; currently M11 owns source runtime consequences | custom glue -> compat/runtime |
| 2 | Expand `F0702_MOVEMENT_TryMove_Compat` from wall-only step test into square-semantic movement resolution (doors, fake walls, passability outcomes) | Establish one owner for party movement legality | mixed -> compat/runtime |
| 3 | Introduce compat-side sensor addition/removal processing for party movement, modeled after `F0276_SENSOR_ProcessThingAdditionOrRemoval` | Restores original ownership boundary for environment triggers | custom glue/probe -> compat/runtime |
| 4 | Replace `m11_toggle_front_door` direct square mutation with compat/runtime-driven door action path | Removes one of the clearest UI-owned world mutations | custom glue -> compat/runtime |
| 5 | Add compat-side wall-click / front-cell sensor trigger routing modeled on `F0275_SENSOR_IsTriggeredByClickOnWall` | Needed for button/ornament/door-adjacent interaction ownership | custom UI -> compat/runtime |
| 6 | Unify creature and party square passability rules under shared compat helpers | Prevents divergent behavior between AI and party | mixed -> compat/runtime |

---

## Recommended next pass order

### Pass A — post-move environment migration
Implement compat-owned post-move resolution for:

- pit fall
- teleporter application
- chained teleporter/pit continuation
- resulting map/direction updates

Then remove or reduce:

- `m11_check_pit_fall`
- `m11_check_teleporter`
- `m11_check_post_move_transitions`

This is the cleanest first cut because it moves already-visible behavior without needing all sensor types at once.

### Pass B — movement legality migration
Expand compat movement to own:

- door passability
- fake-wall semantics
- shared square-semantic legality for party movement

This prevents M11 and creature helpers from continuing to encode separate rules.

### Pass C — sensor ownership migration
Bring in runtime-grade sensor processing for movement events:

- leave square
- enter square
- addition/removal semantics
- more than the first sensor
- local effect batching/rotation

This is the pass that changes ownership from “compat helper exists” to “compat actually owns environment reactions”.

### Pass D — front-door / wall interaction migration
Replace M11 direct world mutation with compat/runtime action handling.

This is high-value because it removes a very visible source of non-faithful UI authority.

---

## Practical conclusion

Firestaff is **not yet source-faithful in ownership** for movement/doors/environment, even where behavior may appear roughly correct on screen.

The current state is:

- **basic movement math:** compat-owned
- **actual world consequences of movement:** still heavily M11-owned
- **door/environment mutation:** still M11-owned
- **sensor/environment interaction runtime:** only partially compat-owned, mostly not integrated as the true owner

If the goal is post-analysis parity, the next work should not start with more viewport polish. It should start by **moving environment consequences out of `m11_game_view.c` and into compat/runtime movement + sensor processing.**

---

## Post-migration status (after passes 29–32)

As of commit `6062763` (pass 32), this audit's recommended next pass order (A → B → C → D, plus pass 32 sensor add/remove) is **fully landed as compat-side owners**, with three tracked runtime-integration deltas:

- **Pass A (post-move environment migration)** — **LANDED** as `F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat`; commit `74253a2`.
- **Pass B (movement legality migration)** — **LANDED** as extended `F0702_MOVEMENT_TryMove_Compat` + `F0705_MOVEMENT_ResolveStairsTransition_Compat` + `F0706_MOVEMENT_IsSquarePassable_Compat`; commit `8df70cc`.
- **Pass D (front-door / wall interaction migration)** — **LANDED** as `F0715_DOOR_ResolveToggleAction_Compat` + `F0716_DOOR_RouteFrontCellClick_Compat`; commit `cb6f88d`.
- **Pass C (sensor ownership migration)** — **LANDED (surface)** as `F0717_SENSOR_EnumerateOnSquare_Compat` + `F0718_SENSOR_ProcessPartyEnterLeave_Compat`; commit `6062763`.  **LANDED (runtime wiring)** as pass 37: `F0888_ORCH_ApplyPlayerInput_Compat` now invokes `F0718` for WALK_OFF at the pre-move square and WALK_ON at the post-resolve final square on every successful party move, surfacing each produced effect as an `EMIT_SENSOR_EFFECT` emission.  `V1_BLOCKERS.md` #1 is therefore resolved for emission-side wiring; world mutation for sensor-effect teleport remains bounded to the F0704 tile-type teleporter path.

Remaining ownership deltas now tracked as numbered V1 blockers rather than open audit items:

- Sensor enter/leave runtime wiring — `V1_BLOCKERS.md` #1.
- Animating door states 1..3 flattened to 0/4 — `V1_BLOCKERS.md` #2.
- Creature walkability not yet delegating to F0706 — **LANDED (pass 39)** as `F0707_MOVEMENT_IsSquarePassableForContext_Compat` + `m11_square_walkable_for_creature` delegation; `V1_BLOCKERS.md` #3 resolved.

Everything else in this audit is either landed or out of scope for V1 per `PASSLIST_29_36.md` §1.
