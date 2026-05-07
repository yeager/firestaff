# pass312_dm1_v1_original_runtime_state_oracle

- status: `PASS_STATE_ORACLE_SOURCE_RUNTIME_BOUND`
- pixels/screenshots: not promoted by this pass
- primary evidence: ReDMCSB source audit

## State-oracle decision

- party tuple/F0128 oracle: `True`
- comparator-input unblock: `True`
- pixel parity claim: `False`

## Source anchors

| seam | source line range | status |
|---|---|---|
| `main_loop_draws_current_tuple` | `GAMELOOP.C:80-90` | `PASS` |
| `main_loop_processes_command_queue` | `GAMELOOP.C:214-215` | `PASS` |
| `command_queue_dequeues_command` | `COMMAND.C:2045-2126` | `PASS` |
| `command_dispatch_turn_or_move` | `COMMAND.C:2150-2156` | `PASS` |
| `turn_mutates_party_direction` | `CLIKMENU.C:142-173` | `PASS` |
| `direction_setter_writes_g0308` | `CHAMPION.C:93-130` | `PASS` |
| `move_computes_destination_and_calls_move_result` | `CLIKMENU.C:180-329` | `PASS` |
| `move_result_writes_party_xy` | `MOVESENS.C:438-443` | `PASS` |
| `teleporter_case_updates_tuple_direction` | `MOVESENS.C:493-517` | `PASS` |
| `falling_case_draws_with_tuple` | `MOVESENS.C:550-556` | `PASS` |
| `f0128_accepts_direction_xy` | `DUNVIEW.C:8318-8324` | `PASS` |
| `f0128_uses_tuple_in_view_calculation` | `DUNVIEW.C:8356-8542` | `PASS` |
| `f0128_presents_viewport` | `DUNVIEW.C:8604-8610` | `PASS` |

## Runtime support

- pass278 status: `BLOCKED_NO_PROVEN_RUNTIME_HOOK`
- required predicates: `{'route_posted_controlled_keys': True, 'g0432_write_seen': True, 'party_tuple_mutation_seen': True, 'f0128_draw_hit_seen': True, 'cpu_memdump_after_stops': True}`
- direct F0380 breakpoint required here: `False`

## Capture coverage support

- pass308 status: `PASS_CAPTURE_EXECUTED_STATE_ORACLE_PENDING`
- coverage: `{'requiredLabelCoverage': True, 'requiredPromotionRowsGameplayOrWallCloseup': True}`

## Non-claims
- original-vs-Firestaff pixel parity
- direct F0380 binary breakpoint/dequeue hit
- tracked bitmap-byte publication
- screenshot promotion without comparator pass

## Next gate

run matched original-vs-Firestaff viewport comparator on promoted route labels and require non-duplicate, semantically matched outputs before any pixel-parity wording
