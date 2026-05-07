# Pass322 — DM1 V1 movement state binding

Status: `MOVEMENT_STATE_BINDING_SOURCE_SYMBOL_LOCKED_RUNTIME_PROBE_DESIGNED`

## Verdict

Movement/core state is now bound source-first to the PC34 public-symbol runtime addresses. This pass deliberately does not claim a fresh DOSBox runtime hit; it records the bounded next probe and preserves the pass320 debugger-control blocker.

## Checks

- PASS `source_audit_ok`
- PASS `movemenu_absence_recorded`
- PASS `pass273_symbols_ok`
- PASS `queue_layout_derived`
- PASS `greatstone_pc34_map_data_seen`
- PASS `pass296_movement_tuple_proof_available`
- PASS `pass320_blocker_preserved`

## Source audit

- PASS `COMMAND.C:1-16` — command_queue_storage
- PASS `COMMAND.C:1734-1812` — keyboard_movement_to_queue
- PASS `COMMAND.C:100-115` — mouse_movement_zones
- PASS `COMMAND.C:2045-2156` — command_dequeue_dispatch
- PASS `DEFS.H:238-243` — movement_command_ids
- PASS `DEFS.H:229-233` — command_struct_layout
- PASS `CLIKMENU.C:142-173` — turn_state_write_path
- PASS `CHAMPION.C:93-130` — direction_setter
- PASS `CLIKMENU.C:180-328` — move_destination_compute
- PASS `MOVESENS.C:316-506` — movesens_party_xy_commit
- PASS `MOVESENS.C:550-556` — movesens_draw_while_falling
- PASS `GAMELOOP.C:80-90` — mainloop_consumes_current_tuple

## MOVEMENU.C

- `NOT_PRESENT_NONBLOCKING` — This ReDMCSB tree has no MOVEMENU.C; movement menu/input bindings are represented by COMMAND.C tables and CLIKMENU.C turn/move handlers, which this verifier audits explicitly.

## Runtime address bindings

### Globals
- PASS `G0432_as_CommandQueue` -> `2C20:3E7A`
- PASS `G0433_i_CommandQueueFirstIndex` -> `2C20:3EC8`
- PASS `G0434_i_CommandQueueLastIndex` -> `2C20:1F08`
- PASS `G0435_B_CommandQueueLocked` -> `2C20:1F0A`
- PASS `G0308_i_PartyDirection` -> `2C20:3C92`
- PASS `G0306_i_PartyMapX` -> `2C20:3C94`
- PASS `G0307_i_PartyMapY` -> `2C20:3CE0`
- PASS `G0310_i_DisabledMovementTicks` -> `2C20:3C9A`
- PASS `G0311_i_ProjectileDisabledMovementTicks` -> `2C20:3D28`
- PASS `G0312_i_LastProjectileDisabledMovementDirection` -> `2C20:3CE4`

### Functions
- PASS `F0380_COMMAND_ProcessQueue_CPSC` -> `22F4:0699`
- PASS `F0365_COMMAND_ProcessTypes1To2_TurnParty` -> `1EA4:010D`
- PASS `F0366_COMMAND_ProcessTypes3To6_MoveParty` -> `1EA4:01AA`
- PASS `F0267_MOVE_GetMoveResult_CPSCE` -> `1859:0516`
- PASS `F0128_DUNGEONVIEW_Draw_CPSF` -> `23AD:40FE`

## Queue layout

- slot 0: X `2C20:3E7A`, Y `2C20:3E7C`, Command `2C20:3E7E`
- slot 1: X `2C20:3E80`, Y `2C20:3E82`, Command `2C20:3E84`
- slot 2: X `2C20:3E86`, Y `2C20:3E88`, Command `2C20:3E8A`
- slot 3: X `2C20:3E8C`, Y `2C20:3E8E`, Command `2C20:3E90`
- slot 4: X `2C20:3E92`, Y `2C20:3E94`, Command `2C20:3E96`

## Greatstone

- Greatstone confirms PC 3.4 dungeon-map data availability, but not runtime command queue/party tuple binding; ReDMCSB/pass273 remain the binding authorities.

## Probe design

- Use pass273 runtime addresses, not guessed offsets.
- Set BPM on queue slot Command fields plus G0433/G0434/G0435, then BPM on G0308/G0306/G0307 and G0310/G0311/G0312.
- Drive controlled kp5/kp4/kp6 only after debugger-run readiness.
- On each true stop capture CPU plus MEMDUMP 2C20:3E7A 36 and MEMDUMP 2C20:3C88 96; ignore BPLIST/setup echoes as pass320 taught.
- Promote only if queue command write/dequeue, tuple mutation or disabled-tick state, and subsequent F0128/F0097 consumption are observed in order within <=60s.

## Next blocker

Debugger stop/control sequencing: pass320 did not recapture the post-F0128 gate after strict stop filtering, so live movement-state offset/table promotion must first get reliable true-stop control.
