# DM1 source-locked symbol reachability survey (2026-08-09)

## What this is

A reachability census of the `F0NNN_*_Compat` symbols defined under `src/dm1`
and `src/memory`. It answers one question: which source-locked contracts have
no production caller, and of those, which have no test or probe exercising
them either.

It is an inventory, not a defect list. See the caveat below before treating
any entry as missing behaviour.

## Method

1. Collect every `F0NNN[a-z]?_*_Compat` definition in `src/dm1` and
   `src/memory`.
2. For each, count files under `src/` referencing it other than its own
   defining file. Zero means no production caller.
3. For each of those, count references under `tests/` and `probes/`.

Reproduce with the shell in this document's git history; no tooling is
shipped for it because the numbers are only meaningful alongside the caveat.

## Numbers

| Measure | Count |
| --- | ---: |
| Source-locked symbols defined | 570 |
| No production caller | 295 |
| ...but covered by a test or probe | 237 |
| ...with neither caller nor test ("dark") | 58 |

The 237 are validated contracts awaiting a consumer. That is a normal state
for this codebase: a pure decision function is landed and pinned first, and
wired into the runtime later.

## The caveat that governs this document

**A symbol having no caller does not mean the behaviour is missing.** During
this survey three of the most alarming-looking entries were checked directly
and all three are implemented, under different names, elsewhere:

| Dark symbol | Behaviour actually lives in |
| --- | --- |
| `F0836_LIFECYCLE_HandlePoisonTick_Compat` | `memory_tick_orchestrator_pc34_compat.c` (poisonDose reschedule and accrual) |
| `F0853_LIFECYCLE_AwardKillXP_Compat` | `memory_champion_lifecycle_pc34_compat.c`, `dm1_v1_melee_action_f0402_pc34_compat.c`, `m11_game_view.c` |
| `F0840_LIFECYCLE_HandleLightExpiry_Compat` | `memory_champion_lifecycle_pc34_compat.c`, `memory_magic_pc34_compat.c` |

This mirrors a mistake made earlier the same day: `M11_GFX_DAMAGE_TO_CHAMPION_SMALL`
was reported as an unimplemented feature purely because grep found only its
enum definition. The C015 damage banner is in fact implemented twice over,
under a different accessor. Grepping a symbol proves the symbol is unused. It
proves nothing about the feature.

So the working hypothesis for the 58 is **superseded or duplicate contract
surface**, not absent gameplay. Each entry needs its behaviour searched for
by name before any conclusion is drawn.

## Category breakdown of the 58

| Count | Prefix | Note |
| ---: | --- | --- |
| 14 | `SAVEGAME` | serialize/deserialize pairs (party, movement, sensor, timeline, combat, magic) |
| 8 | `CREATURE` | distance, visibility, freeze-life gate, counters, candidate scoring, emitters |
| 8 | `LIFECYCLE` | poison/shield/magic/light expiry, move timer, kill XP, timeline event, level-up |
| 7 | `RUNTIME` | fluxcage slot, disabled-sensor find/re-enable, light-decay and generator serialisers |
| 3 | `PROJECTILE` | impact attack, hit-champion action, door-destruction event |
| 2 | `TickStreamRecord` | serialise/deserialise |
| 2 | `MEMORY` | submenu mask classifiers |
| 2 | `DM1` | `F0812` first-possible-movement-dir, `F0815` melee-range |
| 9 | assorted `F0730_*` | projectile launcher-type predicates and cell helpers |
| 3 | other | `DUNGEON` escape replacement, `COMBAT` F0321 scale, `ORCH` run-until-condition |

The `SAVEGAME` cluster is the one worth a deliberate look: fourteen
serialise/deserialise functions with neither a caller nor a round-trip test
is the largest single untested block, and save-format code is where silent
divergence is most costly.

## Full list of the 58

  - `F0479_MEMORY_ClassifySubmenuBehaviorMask_Compat`
  - `F0479_MEMORY_ClassifySubmenuExitMask_Compat`
  - `F0508_DUNGEON_EscapeReplacement_Compat`
  - `F0730_IsExplosionProjectileLauncherType_Compat`
  - `F0730_IsProjectileLauncherType_Compat`
  - `F0730_IsSingleProjectileLauncherType_Compat`
  - `F0730_IsSquareObjectProjectileLauncherType_Compat`
  - `F0730_NextCell_Compat`
  - `F0730_NormalizeCell_Compat`
  - `F0730_OppositeCell_Compat`
  - `F0730_RecordProjectileLaunch_Compat`
  - `F0730_SelectSquareProjectileThing_Compat`
  - `F0739_COMBAT_ScaleChampionDamageF0321_Compat`
  - `F0776_SAVEGAME_DeserializeParty_Compat`
  - `F0776_SAVEGAME_SerializeParty_Compat`
  - `F0776a_SAVEGAME_MovementResultSerialize_Compat`
  - `F0776b_SAVEGAME_MovementResultDeserialize_Compat`
  - `F0777_SAVEGAME_DeserializeMovement_Compat`
  - `F0777_SAVEGAME_SerializeMovement_Compat`
  - `F0778_SAVEGAME_DeserializeSensor_Compat`
  - `F0778_SAVEGAME_SerializeSensor_Compat`
  - `F0779_SAVEGAME_DeserializeTimeline_Compat`
  - `F0779_SAVEGAME_SerializeTimeline_Compat`
  - `F0780_SAVEGAME_SerializeCombat_Compat`
  - `F0780b_SAVEGAME_DeserializeCombat_Compat`
  - `F0781_SAVEGAME_DeserializeMagic_Compat`
  - `F0781_SAVEGAME_SerializeMagic_Compat`
  - `F0790_CREATURE_GetManhattanDistance_Compat`
  - `F0791_CREATURE_IsDestinationVisible_Compat`
  - `F0794_CREATURE_ApplyFreezeLifeGate_Compat`
  - `F0795_CREATURE_DecrementCounters_Compat`
  - `F0797_CREATURE_ScoreCandidates_Compat`
  - `F0800_CREATURE_EmitCombatAction_Compat`
  - `F0802_CREATURE_EmitNextTickEvent_Compat`
  - `F0803_CREATURE_EmitSelfDamage_Compat`
  - `F0812_DM1_GROUP_GetFirstPossibleMovementDir_Compat`
  - `F0815_DM1_GROUP_IsMeleeRange_Compat`
  - `F0815_PROJECTILE_ComputeImpactAttack_Compat`
  - `F0818_PROJECTILE_BuildHitChampionAction_Compat`
  - `F0819_PROJECTILE_BuildDoorDestructionEvent_Compat`
  - `F0836_LIFECYCLE_HandlePoisonTick_Compat`
  - `F0837_LIFECYCLE_HandleShieldExpiry_Compat`
  - `F0838_LIFECYCLE_HandleMagicExpiry_Compat`
  - `F0840_LIFECYCLE_HandleLightExpiry_Compat`
  - `F0842_LIFECYCLE_UpdateMoveTimer_Compat`
  - `F0853_LIFECYCLE_AwardKillXP_Compat`
  - `F0854_LIFECYCLE_EmitTimelineEvent_Compat`
  - `F0856_LIFECYCLE_BuildLevelUpMarker_Compat`
  - `F0869_RUNTIME_IsFluxcageSlotLive_Compat`
  - `F0873_RUNTIME_FindDisabledSensorOnSquare_Compat`
  - `F0874_RUNTIME_ReEnableSensor_Compat`
  - `F0877_RUNTIME_LightDecayResultDeserialize_Compat`
  - `F0877_RUNTIME_LightDecayResultSerialize_Compat`
  - `F0879_RUNTIME_GeneratorReEnableResultDeserialize_Compat`
  - `F0879_RUNTIME_GeneratorReEnableResultSerialize_Compat`
  - `F0886_ORCH_RunUntilCondition_Compat`
  - `F0897c_TickStreamRecord_Deserialize_Compat`
  - `F0897c_TickStreamRecord_Serialize_Compat`

## What this document does not claim

- It does not claim any of these are bugs.
- It does not claim any gameplay behaviour is missing.
- It does not promote or retire any symbol.
