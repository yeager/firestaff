# Pass299 — DM1 V1 movement completion matrix gate

Status: `MOVEMENT_COMPLETION_MATRIX_UPDATED_DIRECT_F0380_NONBLOCKING_REMAINING_BLOCKERS_EXPLICIT`

## Verdict

The DM1 V1 movement matrix row now treats pass296 as the accepted source/runtime-supported proof for input→queue→dispatch-equivalent→party tuple→F0128 draw consumption. Direct F0380 entry/body BP remains blocked and unclaimed, but it is non-blocking for this proof scope.

## ReDMCSB-first citations

- `GAMELOOP.C:80-90, 160-168, 215-219` / `F0002_MAIN_GameLoop_CPSDF` — redraw consumes G0308/G0306/G0307; keyboard input is drained before F0380; the wait loop invokes F0380.
- `COMMAND.C:252-260, 1734-1812, 2045-2156` / `F0361_COMMAND_ProcessKeyPress / F0380_COMMAND_ProcessQueue_CPSC` — controlled movement keys map to commands, enqueue into G0432, dequeue/dispatch from F0380, and apply disabled-movement gates.
- `CLIKMENU.C:156-347` / `F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty` — turn commands set party direction; move commands compute relative destination, check blockers, and invoke movement result/timing paths.
- `MOVESENS.C:316-565, 738-818` / `F0267_MOVE_GetMoveResult_CPSCE` — successful movement commits G0306/G0307, handles consequences, records movement result/timing/scent, and runs sensor side effects.
- `DUNVIEW.C:8318-8611` / `F0128_DUNGEONVIEW_Draw_CPSF` — viewport draw receives direction/mapX/mapY arguments and presents the dungeon viewport.

## Remaining movement blockers

- **original_party_control_ready_capture** — blocks: original-runtime movement/HUD/viewport pixel parity; missing: A deterministic original PC route that reaches party-control-ready gameplay and yields semantically labelled movement before/after captures, not direct-start/no-party or menu/title frames.
- **direct_f0380_binary_trace_optional** — blocks: only a future binary-level direct-F0380 hook claim, not the source/runtime-supported movement proof; missing: Decompressed stock FIRES memory map/FIRES.MAP or live disassembly breakpoints around the F0380 post-prologue/dequeue window; pass296 keeps this blocked and non-required.
- **full_movement_side_effect_coverage** — blocks: promotion of the whole movement row to MATCHED; missing: Original-backed cases for the broader movement side effects: group/projectile interlocks, sensor/environment consequence breadth, timing/cooldown edge cases, and overlay comparison from matched runtime states.

## Checks

- PASS `pass296_status_locked`
- PASS `pass296_gate_passed`
- PASS `direct_f0380_nonblocking`
- PASS `matrix_has_status`
- PASS `matrix_marks_direct_f0380_nonblocking`
- PASS `matrix_lists_remaining_blockers`
- PASS `source_citations_complete`
- PASS `remaining_blocker_manifest_complete`

## Artifacts

- Matrix: `docs/parity/PARITY_MATRIX_DM1_V1.md`
- Manifest: `parity-evidence/verification/pass299_dm1_v1_movement_completion_matrix_gate/manifest.json`
- Depends on pass296 manifest: `parity-evidence/verification/pass296_dm1_v1_input_tuple_proof_without_direct_f0380/manifest.json`
