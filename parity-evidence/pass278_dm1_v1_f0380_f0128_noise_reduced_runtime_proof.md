# Pass278 — DM1 V1 F0380/F0128 noise-reduced runtime proof

Status: `BLOCKED_NO_PROVEN_RUNTIME_HOOK`

## What this pass tried

- Started with a ReDMCSB source audit of `COMMAND.C`, `MOVESENS.C`, and `DUNVIEW.C`.
- Used pass273 runtime addresses and pass276 artifacts as inputs.
- Reduced debugger noise with staged windows: queue write watch -> queue-to-tuple watch -> F0380/F0128 code BP-only.
- Drove a longer controlled route after debugger-run readiness.

## Evidence

- Manifest: `parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json`
- Transcript: `parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/dosbox_debug_noise_reduced.clean.txt`
- Route log: `parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/route_noise_reduced_keylog.json`

## Proof predicates

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

Staged BPM disable/re-enable reduced queue/index noise and captured controlled-key G0432 writes, party tuple mutation, and an F0128 draw BP hit after switching to code-only mode; the exact remaining blocker is no proven F0380 dequeue BP hit at 22F4:0699. This is not a runtime-hook claim.
