# Pass276 — DM1 V1 two-loop BPM runtime proof

Status: `BLOCKED_NO_PROVEN_RUNTIME_HOOK`

## What changed from pass275

- Re-audited ReDMCSB command/movement/viewport seams before running.
- Replaced the synchronous xdotool route injector with a two-loop dosbox-debug driver.
- Route loop posts controlled `kp5`/`kp4`/`kp6` only after a debugger-run guard.
- Debugger loop captures `CPU`, `MEMDUMP 2C20:3E7A 80`, and `MEMDUMP 2C20:3C88 80` on stops and immediately continues via vt100 F5 (`Esc O t`).

## Evidence

- Manifest: `parity-evidence/verification/pass276_dm1_v1_two_loop_bpm_runtime_proof/manifest.json`
- Transcript: `parity-evidence/verification/pass276_dm1_v1_two_loop_bpm_runtime_proof/dosbox_debug_two_loop.clean.txt`
- Route log: `parity-evidence/verification/pass276_dm1_v1_two_loop_bpm_runtime_proof/route_two_loop_keylog.json`

## Proof predicates

```json
{
  "cpu_memdump_after_stops": true,
  "f0128_draw_hit_seen": false,
  "f0380_dequeue_hit_seen": false,
  "g0432_write_seen": true,
  "party_tuple_mutation_seen": true,
  "route_posted_controlled_keys": true
}
```

## Decision

The two-loop driver fixed the pass275 synchronous-input flaw and posted kp5/kp4/kp6 under the debugger-run guard, but the transcript still does not show F0380 dequeuing or F0128 consuming the mutated tuple as BP hits. Next step: reduce breakpoint noise or drive a longer in-game route after game input readiness so code BP hits are captured between queue write/mutation and draw.
