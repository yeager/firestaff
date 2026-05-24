# Pass406 — DM1 V1 movement legality completion gate

Status: `PASS406_DM1_V1_MOVEMENT_LEGALITY_COMPLETION_GATE_PROVEN`

## ReDMCSB-first source audit
- `CLIKMENU.C:180-351` / `F0366_COMMAND_ProcessTypes3To6_MoveParty` — computes relative target square, blocks walls, closed-enough doors, and closed real fake-walls before accepted move dispatch.
- `MOVESENS.C:316-999` / `F0267_MOVE_GetMoveResult_CPSCE` — accepted-move consequence path, including open non-imaginary pit level changes.
- `DUNGEON.C:1371-1421` / `F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement` — source relative-step coordinate update.
- `DUNGEON.C:1423-1479` / `F0151_DUNGEON_GetSquare` — source square fetch, including out-of-bounds-as-wall behavior.

## Firestaff executable guards
- `memory_movement_pc34_compat.c:408-493` / `F0702_MOVEMENT_TryMove_Compat` keeps the pre-step legality split.
- `memory_movement_pc34_compat.c:709-835` / `F0704_MOVEMENT_ResolvePostMoveEnvironment_Compat` keeps pit consequences post-step.
- `build/test_dm1_v1_movement_core_pc34_compat` covers closed/open/destroyed door states, fake-wall open/imaginary bits, wall blocks, and pit passability.

## Verdict
- Closes one completion-matrix movement source-lock gap: target-square legality is now an executable CTest gate, not just prose in the movement row.
- Scope guard: no representative original DOS overlay parity, complete movement side-effect parity, or direct-F0380 binary breakpoint proof is claimed.

Manifest: `parity-evidence/verification/pass406_dm1_v1_movement_legality_completion_gate/manifest.json`
