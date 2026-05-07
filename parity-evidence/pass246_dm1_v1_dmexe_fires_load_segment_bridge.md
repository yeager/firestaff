# Pass246 — DM.EXE FIRES load-segment bridge

Status: `PASS_DMEXE_FIRES_LOAD_SEGMENT_BRIDGED_NO_RUNTIME_HIT_PROMOTION`

## Result

- Actual DM.EXE-loader FIRES-side stop: `2B02:0038`.
- Matched FIRES.EXENEW static CS:IP: `23CF:0008` (`10` bytes).
- Derived actual FIRES load segment under `DM.EXE`: `0733`.

## Guardrail

This is a loader-segment bridge only. No `verified_runtime_hit` promotion and no command/movement/viewport chain proof.

Manifest: `<repo>/parity-evidence/verification/pass246_dm1_v1_dmexe_fires_load_segment_bridge/manifest.json`
