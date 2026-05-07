# Pass284 — DM1 V1 F0380 dequeue ordering proof

Status: `SOURCE_PROVEN_RUNTIME_F0380_BLOCKED`

## Verdict

- Source chain: `PROVEN`.
- Runtime chain: `NOT PROMOTED`.
- F0380 runtime BP hit: `not seen`.
- F0128 runtime BP hit: `seen`.

## Source-locked ordering

- PASS `controlled_key_rows` — `COMMAND.C:252-260`: PC movement keyboard rows map keypad/arrow controls to C001..C006 movement commands before F0361 scans them.
- PASS `controlled_key_write_to_g0432` — `COMMAND.C:1734-1812`: F0361 locks the queue, scans primary then secondary keyboard tables, writes matching commands into G0432, unlocks, then replays one pending click.
- PASS `f0380_index_dequeue_before_dispatch` — `COMMAND.C:2075-2127`: F0380 locks, checks empty/gated movement before dequeue, reads command/X/Y from first index, advances G0433, then unlocks and replays pending clicks before dispatch.
- PASS `f0380_dispatch_after_dequeue` — `COMMAND.C:2118-2156`: F0380 dispatches C001/C002 turns and C003..C006 steps only after dequeue/unlock/replay.
- PASS `turn_tuple_mutation` — `CHAMPION.C:117-130`: Turn commands mutate the party tuple by writing G0308_i_PartyDirection after rotating champion cells/directions.
- PASS `step_tuple_mutation` — `MOVESENS.C:438-444`: Successful step commands mutate the party tuple by writing G0306/G0307 from F0267 destination coordinates.
- PASS `main_loop_draw_consumes_tuple` — `GAMELOOP.C:88-90`: The game loop calls F0128 with the current G0308/G0306/G0307 tuple.
- PASS `f0128_consumes_draw_args_before_viewport` — `DUNVIEW.C:8318-8611`: F0128 receives direction/mapX/mapY parameters and requests the dungeon viewport from that draw path.

## Queue index/count resolution

DM1 V1 PC-34 resolves dequeue by circular first/last indices (`G0433_i_CommandQueueFirstIndex`, `G0434_i_CommandQueueLastIndex`). The `G2153_i_QueuedCommandsCount--` statement exists only under the later `MEDIA728_I34E/I35` guard; it is not the PC-34 authority. The PC-34 ordering is: read `G0432[G0433].Command`, gate movement before dequeue, read X/Y, advance/wrap `G0433`, unlock/replay pending click, then dispatch turn/step.

## Imported runtime predicates

From `parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json`:

```json
{
  "cpu_memdump_after_stops": true,
  "f0128_draw_hit_seen": true,
  "f0380_dequeue_hit_seen": false,
  "g0432_write_seen": true,
  "party_tuple_mutation_seen": true,
  "route_posted_controlled_keys": true
}
```

## Decision

Static ReDMCSB evidence proves the requested controlled-key to draw-consumption ordering. Existing runtime evidence still cannot promote the full chain: it has controlled key delivery, `G0432` write, party tuple mutation, and `F0128` draw hit, but no proven `F0380_COMMAND_ProcessQueue_CPSC` dequeue BP hit at `22F4:0699`.

## Artifacts

- Manifest: `parity-evidence/verification/pass284_dm1_v1_f0380_dequeue_ordering_proof/manifest.json`
- Imported pass278 transcript: `parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/dosbox_debug_noise_reduced.clean.txt`
