# Pass499 — DM1 V1 wall occlusion runtime evidence gate

Status: `PASS499_DM1_V1_WALL_OCCLUSION_RUNTIME_EVIDENCE_GATE_LOCKED`

## Decision

Promote wall-occlusion runtime evidence only when pass496 source/spec matrix is ok, the compiled Firestaff wall-composition probe passes, and a runtime/capture lane reaches the F0128 to F0097 present boundary for the same viewport. This gate is source/probe coverage only.

## Source locks
- `dunview_f0128_far_to_near_then_present` ok=True refs=DUNVIEW.C:8318-8610: F0128 composes floor/ceiling and wall squares in far-to-near order, then hands the composed viewport to F0097.
- `drawview_f0097_present_boundary` ok=True refs=DRAWVIEW.C:709-858: F0097 marks the redraw request and reaches the PC34 viewport blit boundary for G0296.

## Firestaff coverage

- `wall_spec_table` ok=True line=346
- `wall_spec_accessor_count` ok=True line=940
- `wall_spec_accessor_by_square` ok=True line=951
- `door_front_occlusion_specs` ok=True line=166
- `wall_contract_probe_expected_matrix` ok=True line=30
- `wall_contract_probe_source_output` ok=True line=66
- `door_contract_probe_output` ok=True line=128
- `public_wall_spec_type` ok=True line=426
- `public_door_front_spec_type` ok=True line=303

## Gates

- compiled probe: `firestaff_dm1_v1_wall_composition_contract_probe`
- prior gates: `pass496_dm1_v1_wall_occlusion_spec_matrix, pass490_dm1_v1_wall_occlusion_merge_readiness, dm1_v1_viewport_3d_occlusion_metadata_gate`
- pass496 manifest rows: `15`

## Non-claims

- no new DOSBox debugger stop
- no original-vs-Firestaff pixel parity
- no promotion of static repeated screenshots
