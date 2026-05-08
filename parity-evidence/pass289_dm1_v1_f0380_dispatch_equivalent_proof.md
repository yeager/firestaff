# Pass289 — DM1 V1 F0380 dispatch-equivalent proof

Status: `DISPATCH_EQUIVALENT_PROVEN_DIRECT_F0380_BP_BLOCKED`

## Verdict

- Source chain: `PROVEN`.
- Dispatch-equivalent runtime chain: `PROVEN`.
- Direct F0380 body breakpoint: `not seen`.
- Final claim: `dispatch-equivalent proven; direct F0380 BP remains blocked`.

## Required chain

- PASS `controlled_pc34_key_rows` — `COMMAND.C:252-260`: PC-34 movement keyboard input maps keypad/arrow controls to C001/C002 turns and C003..C006 movement commands.
- PASS `f0361_key_to_g0432_g0434_count` — `COMMAND.C:1734-1812`: F0361 scans keyboard tables, reserves next circular slot, writes command to G0432, advances G0434, and increments guarded G2153 count where present.
- PASS `f0380_dequeue_source_equivalent` — `COMMAND.C:2045-2156`: F0380 tests G0433/G0434 empty state, reads G0432[G0433], gates disabled movement, reads X/Y, decrements guarded G2153, advances/wraps G0433, unlocks/replays, then dispatches turns/moves.
- PASS `main_loop_calls_f0380` — `GAMELOOP.C:160-216`: The active game loop reads keyboard input through F0361 and then calls F0380_COMMAND_ProcessQueue_CPSC, giving the dispatch-equivalent caller seam when direct F0380 body BP misses.
- PASS `turn_dispatch_mutates_g0308` — `CHAMPION.C:117-130`: Turn dispatch mutates the party tuple by writing G0308_i_PartyDirection.
- PASS `move_dispatch_mutates_g0306_g0307` — `MOVESENS.C:316-556`: Move dispatch reaches F0267, where successful party movement writes G0306_i_PartyMapX/G0307_i_PartyMapY; pit/teleporter variants repeat tuple writes before draw/sensor consequences.
- PASS `f0128_consumes_tuple_from_gameloop` — `GAMELOOP.C:88-90`: The main loop redraw consumes the current party tuple by passing G0308/G0306/G0307 to F0128.
- PASS `dunview_f0128_argument_consumption` — `DUNVIEW.C:8318-8611`: DUNVIEW.C F0128 receives direction/mapX/mapY parameters and ends by drawing the dungeon viewport.

## Source-map justification

- Caller seam: GAMELOOP.C:160-216 reads keyboard input with F0361 and then calls F0380_COMMAND_ProcessQueue_CPSC at line 215.
- Queue/index/count: COMMAND.C:6-11 defines G0432 and circular G0433/G0434; G2153 exists, but for this PC-34 route the source proof is index authority plus guarded count updates.
- Dequeue equivalent: COMMAND.C:2045-2156 establishes that any observed G0432 write followed by G0306/G0307/G0308 mutation and later F0128 draw on the main-loop route traverses the F0380 dispatch semantics, even though BP 22F4:0699 itself did not stop.
- Draw consumption: GAMELOOP.C:88-90 passes G0308/G0306/G0307 into DUNVIEW.C F0128; DUNVIEW.C:8318-8611 consumes those args before viewport draw.

## Prior evidence inspection

- pass278: `BLOCKED_NO_PROVEN_RUNTIME_HOOK` from `parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json`.
- pass281: none found.
- pass284: `SOURCE_PROVEN_RUNTIME_F0380_BLOCKED` from `parity-evidence/verification/pass284_dm1_v1_f0380_dequeue_ordering_proof/manifest.json`; exact blocker was `No proven runtime F0380 dequeue breakpoint hit at 22F4:0699 in the imported pass278 transcript; do not claim runtime hook despite G0432 write, party tuple mutation, and F0128 draw hit.`.

Runtime predicates imported from pass278:

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

## Narrowed blocker

Direct F0380 body breakpoint `22F4:0699` still did not hit. Reproduce with the pass278 route and debugger setup in `parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json` / `parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/dosbox_debug_noise_reduced.clean.txt`. This pass therefore lands the smaller, honest result: source-locked dispatch-equivalent proof via `GAMELOOP.C:215 -> COMMAND.C:F0380`, plus runtime observation of controlled keys, `G0432`, tuple mutation, and `F0128`; no direct F0380 runtime hook claim.

## Artifacts

- Manifest: `parity-evidence/verification/pass289_dm1_v1_f0380_dispatch_equivalent_proof/manifest.json`
- Report: `parity-evidence/pass289_dm1_v1_f0380_dispatch_equivalent_proof.md`
