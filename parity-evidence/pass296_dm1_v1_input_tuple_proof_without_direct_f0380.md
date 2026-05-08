# Pass296 — DM1 V1 input-to-party-tuple proof without direct F0380 BP

Status: `MOVEMENT_INPUT_TUPLE_PROOF_SOURCE_LOCKED_RUNTIME_SUPPORTED_DIRECT_F0380_BLOCKED_NOT_REQUIRED`

## Verdict

DM1 V1 movement proof status: source-locked and runtime-supported from controlled key to queue predicates to dispatch-equivalent source chain to party tuple mutation to F0128 draw consumption. Direct F0380 entry/body BP remains blocked and is explicitly not claimed or required by this gate.

## Proof chain

- PASS **controlled key** — source: `COMMAND.C:252-260 movement keyboard table; COMMAND.C:F0361`; runtime: pass278 route_posted_controlled_keys=true and pass293 controlled_keys_posted=true.
- PASS **queue predicates** — source: `COMMAND.C:F0361 writes G0432/G0434/G2153; COMMAND.C:F0380 reads/dequeues G0432/G0433`; runtime: pass278 g0432_write_seen=true and pass293 queue_or_index_bpm_seen=true.
- PASS **dispatch-equivalent/source chain** — source: `GAMELOOP.C:F0002 calls F0361 then F0380; COMMAND.C:F0380 dispatches F0365/F0366`; runtime: pass289 dispatch_equivalent_proven=true via pass278 runtime predicates.
- PASS **party tuple mutation** — source: `MOVESENS.C:F0267 writes G0306/G0307 and uses G0308 direction path`; runtime: pass278 party_tuple_mutation_seen=true with CPU/MEMDUMP after stops.
- PASS **F0128 draw consumption** — source: `GAMELOOP.C:88-90 passes G0308/G0306/G0307; DUNVIEW.C:F0128 receives P0183/P0184/P0185 and draws viewport`; runtime: pass278 f0128_draw_hit_seen=true.

## Source citations checked

- PASS `COMMAND.C:252-260` / `PC-34 movement keyboard table / F0361_COMMAND_ProcessKeyPress` — PC-34 controlled movement keys map to turn/move commands before enqueue.
- PASS `COMMAND.C:1734-1812` / `F0361_COMMAND_ProcessKeyPress` — F0361 locks the queue, selects the next slot, writes G0432, advances G0434, increments the guarded count, unlocks, then processes pending click replay.
- PASS `GAMELOOP.C:160-216` / `F0002_MAIN_GameLoop_CPSDF` — The active game loop scans keys with F0361 and then invokes F0380, creating the accepted caller seam when direct F0380 entry BP misses.
- PASS `COMMAND.C:2045-2156` / `F0380_COMMAND_ProcessQueue_CPSC` — F0380 source semantics dequeue G0432/G0433, apply the disabled-movement gate, advance/wrap the queue, replay pending click, then dispatch turn/move commands.
- PASS `MOVESENS.C:316-556` / `F0267_MOVE_GetMoveResult_CPSCE` — Successful movement writes the party tuple G0306/G0307 and calls direction/draw paths for movement consequences.
- PASS `GAMELOOP.C:88-90` / `F0002_MAIN_GameLoop_CPSDF -> F0128_DUNGEONVIEW_Draw_CPSF` — The main redraw passes the current party tuple G0308/G0306/G0307 into F0128.
- PASS `DUNVIEW.C:8318-8611` / `F0128_DUNGEONVIEW_Draw_CPSF` — F0128 receives direction/mapX/mapY arguments and finishes by drawing the dungeon viewport.

## Imported runtime support

- pass278 `BLOCKED_NO_PROVEN_RUNTIME_HOOK`: controlled keys, `G0432` queue write, party tuple mutation, CPU/MEMDUMP after stops, and `F0128` draw hit are true; direct F0380 dequeue hit is false.
- pass289 `DISPATCH_EQUIVALENT_PROVEN_DIRECT_F0380_BP_BLOCKED`: source chain and dispatch-equivalent runtime chain are true; direct F0380 body BP is false.
- pass293 `BLOCKED_SOURCE_MAP_DEBUGGER_WINDOW_FOR_DIRECT_F0380`: direct F0380 claim is false; queue/index watch support is true; exact blocker remains the direct entry BP/address window.

## Direct F0380 decision

Direct F0380 entry/body breakpoint proof is **blocked, not claimed, and not required** for this gate. The accepted proof is source-locked/runtime-supported through `GAMELOOP.C:F0002 -> COMMAND.C:F0380` dispatch-equivalence, with pass278 runtime evidence carrying queue write, tuple mutation, and draw consumption.

## Artifacts

- Manifest: `parity-evidence/verification/pass296_dm1_v1_input_tuple_proof_without_direct_f0380/manifest.json`
- Report: `parity-evidence/pass296_dm1_v1_input_tuple_proof_without_direct_f0380.md`
