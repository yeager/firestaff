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

## Firestaff ownership map

| Firestaff file/function | Current role | Closest ReDMCSB ownership area | Current ownership verdict |
|---|---|---|---|
| `memory_movement_pc34_compat.c:F0700_MOVEMENT_TurnDirection_Compat` | turn math | `MOVESENS.C` direction math | already compat-owned |
| `memory_movement_pc34_compat.c:F0701_MOVEMENT_GetStepDelta_Compat` | relative step delta | `MOVESENS.C` direction/step logic | already compat-owned |
| `memory_tick_orchestrator_pc34_compat.c:cmd_to_move_action` | absolute command -> relative move action | `COMMAND.C`/movement bridge | already compat-owned |
| `memory_movement_pc34_compat.c:F0702_MOVEMENT_TryMove_Compat` | bounds + wall-only party move test | thin subset of `F0267_MOVE_GetMoveResult_CPSCE` | mixed, materially under-owned |
| `memory_movement_pc34_compat.c:F0703_MOVEMENT_IdentifySensorsOnSquare_Compat` | find first sensor on square | small probe subset of `MOVESENS.C` sensor traversal | mixed, probe-grade |
| `memory_sensor_execution_pc34_compat.c:F0710_SENSOR_Execute_Compat` | execute teleport/text for `WALK_ON` only | tiny subset of `F0276` sensor result path | mixed, probe-grade |
| `m11_game_view.c:m11_try_stairs_transition` | direct stairs level change | environment/movement consequence logic that should sit with runtime movement/square semantics | custom-glue-owned |
| `m11_game_view.c:m11_check_pit_fall` | direct pit fall + damage | `F0267_MOVE_GetMoveResult_CPSCE` chained movement/fall handling | custom-glue-owned |
| `m11_game_view.c:m11_check_teleporter` | direct teleporter lookup, apply move/rotation | `F0267_MOVE_GetMoveResult_CPSCE` + sensor/teleporter movement semantics | custom-glue-owned |
| `m11_game_view.c:m11_check_post_move_transitions` | chains pit/teleporter after move | `F0267_MOVE_GetMoveResult_CPSCE` chained moves | custom-glue-owned |
| `m11_game_view.c:m11_toggle_front_door` | direct door square mutation + synthetic emissions | door actuation via source-faithful sensor/action/runtime path, not viewport-owned mutation | custom-glue-owned |
| `m11_game_view.c:m11_front_cell_is_door` and door click handling | viewport-specific gating only | acceptable UI concern if it delegates behavior | mixed but acceptable only if behavior is delegated |
| `m11_game_view.c:m11_square_walkable_for_creature` | custom creature walkability semantics for doors/pits/fake walls/teleporters | square semantics should match runtime helpers | unclear/mixed |

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
`F0703_MOVEMENT_IdentifySensorsOnSquare_Compat` + `F0710_SENSOR_Execute_Compat` exist, but current usage is probe-oriented. Runtime integration is not the main movement path.

Observed limits:

- only first sensor is surfaced
- execution only honors `WALK_ON`
- only teleport and text are implemented
- unsupported sensor types are explicitly stubbed
- no source-faithful enter/leave/add/remove processing comparable to `F0276_SENSOR_ProcessThingAdditionOrRemoval`
- no click-on-wall routing comparable to `F0275_SENSOR_IsTriggeredByClickOnWall`
- no local-effect batching/rotation ownership comparable to `F0270/F0271`

### ReDMCSB baseline
Sensor/environment interaction ownership is broad and event-driven, not a single-square single-sensor single-event adapter.

### Audit verdict
**Compat exists, but it is not yet the true owner.** Runtime ownership still effectively sits outside compat.

---

## 5) Fake-wall / door-state / square-semantics ownership remains incomplete

### Firestaff
Movement compat still treats all non-wall squares as passable. Creature walkability in `m11_game_view.c` separately hardcodes corridor/pit/teleporter/fakewall as walkable and doors as open when low bits are zero.

### ReDMCSB baseline
Square semantics are source-owned and shared runtime rules, including fake-wall handling and door-state consequences.

### Audit verdict
**Mixed ownership with semantic drift risk.** Party and creature movement are already in danger of diverging because they do not share one source-faithful owner.

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
  - local effect batching/rotation
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
