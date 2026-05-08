# Firestaff Completion Matrix

Last updated: 2026-05-08

This is a conservative 100-point parity-completion model. A point is counted only when the repo has source/runtime evidence for that criterion. `completionPercent` is therefore a verified evidence score, not optimism, effort spent, or pass count.

## Scoring criteria

| Criterion | Weight | Meaning |
|---|---:|---|
| `reference_inventory` | 10 | Original/source reference inventory exists and is source-locked for this target. |
| `definition_matrix` | 10 | Target-specific parity/completion matrix exists with clear acceptance labels. |
| `launch_smoke` | 10 | Target launches or renders through a repeatable repo gate/smoke path. |
| `core_input_movement` | 15 | Core input, movement/camera, command routing, and first viewport redraw path are source/runtime-gated. |
| `viewport_ui_render` | 20 | Viewport, HUD, panels, title/menu, palette, graphics decode, and UI render surfaces are source/runtime-gated. |
| `gameplay_systems` | 15 | Combat, creatures, items, spells, sensors, projectiles, save/load, lifecycle, and dungeon systems are original/source-gated. |
| `audio_timing` | 10 | Audio content, trigger cadence, animation/timing, input responsiveness, and music/SFX overlap are original/source-gated. |
| `original_overlay_regression` | 10 | Original runtime captures/overlays/regression gates prove pixel/behavior parity for representative states. |

## Current matrix

| Target | completionPercent | Points | Status | Primary blockers |
|---|---:|---:|---|---|
| DM1 V1 | 57% | 57/100 | `in_progress_verified_partial` | Original DOSBox/FIRES keyboard-buffer transcript for I34E route keys remains unclaimed, but no longer blocks Firestaff live route.<br>Representative original runtime movement/HUD/viewport overlay parity missing. |
| DM1 V2 | 27% | 27/100 | `in_progress_verified_partial` | V2 completion audit says 100% is not verifiable; phases 1–6 remain absent/incomplete. |
| CSB V1 | 18% | 18/100 | `inventory_only` | Need CSB V1 launch/render smoke and runtime/capture gates; definition matrix is now complete but evidence-only.<br>Do not infer CSB runtime/render/gameplay parity from DM1. |
| CSB V2 | 0% | 0/100 | `not_started` | No CSB V2 matrix or gates. |
| DM2 V1 | 7% | 7/100 | `inventory_only` | Need DM2-specific disassembly/runtime matrix; do not infer from DM1/CSB. |
| DM2 V2 | 0% | 0/100 | `not_started` | No DM2 V2 matrix or gates. |
| DM Nexus V1 | 0% | 0/100 | `not_started` | No DM Nexus V1 reference inventory, matrix, or gates. |
| DM Nexus V2 | 0% | 0/100 | `not_started` | No DM Nexus V2 reference inventory, matrix, or gates. |

## Detail

### DM1 V1

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 10/10 | DM1 PC 3.4 canonical anchors, ReDMCSB source and Greatstone references are present and repeatedly cited. |
| `definition_matrix` | 10/10 | docs/parity/PARITY_MATRIX_DM1_V1.md exists and carries DM1 V1 labels/status rows. |
| `launch_smoke` | 8/10 | M9/M10/M11 infrastructure and startup/menu/title gates exist, but current original route still has input/table blockers. |
| `core_input_movement` | 12/15 | pass296/pass299 plus pass372 source-lock Firestaff input/keypad route through C001..C006 command queue, movement timing/pipeline, and first viewport redraw; residual is original DOS keyboard-buffer transcript/overlay proof. |
| `viewport_ui_render` | 12/20 | Many source-backed UI/viewport/title/HUD/palette rows are MATCHED or narrowed; pass373 proves the live launcher movement route reaches the source-locked wall/door/occlusion redraw stack, and pass375 moves explosions into the ReDMCSB F0115 after-all-packed-cells deferred pass. Full pixel/content parity remains open. |
| `gameplay_systems` | 2/15 | Semantic suites exist but most gameplay rows remain UNPROVEN or need original-backed cases. |
| `audio_timing` | 3/10 | Audio assets/event mapping are partly source-backed; cadence/timing overlap remain blocked/unproven. |
| `original_overlay_regression` | 0/10 | Representative original runtime overlay parity is not yet proven; route/capture remains active blocker. |

### DM1 V2

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 8/10 | DM1 source/original inventory applies; V2 remains original-required but not separately complete. |
| `definition_matrix` | 6/10 | dm1_v2_completion_matrix/status artifacts exist but explicitly do not prove completion. |
| `launch_smoke` | 5/10 | Some V2 shell/scaffold gates exist; full launcher/preview smoke is unproven. |
| `core_input_movement` | 4/15 | Movement/camera scaffolds exist; source-locked V2 semantics incomplete. |
| `viewport_ui_render` | 4/20 | Viewport/wall/upscale scaffolds exist; full V2 render/material/UI gates absent. |
| `gameplay_systems` | 0/15 | No complete V2 gameplay-system parity gates. |
| `audio_timing` | 0/10 | No V2 audio/timing completion gates. |
| `original_overlay_regression` | 0/10 | No V2 representative original-overlay regression proof. |

### CSB V1

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 8/10 | CSB source/runtime archives and source-lock boundary guard exist. |
| `definition_matrix` | 10/10 | docs/parity/PARITY_MATRIX_CSB_V1.md defines the full CSB V1 DoD matrix with acceptance labels, required proof boundaries, and verifier-backed source anchors; parity surface matrix remains evidence-only. |
| `launch_smoke` | 0/10 | No complete CSB V1 launch/render smoke proof. |
| `core_input_movement` | 0/15 | No CSB V1 movement/input completion gates. |
| `viewport_ui_render` | 0/20 | No CSB V1 render/UI completion gates. |
| `gameplay_systems` | 0/15 | No CSB V1 gameplay completion gates. |
| `audio_timing` | 0/10 | No CSB V1 audio/timing completion gates. |
| `original_overlay_regression` | 0/10 | No CSB V1 overlay regression proof. |

### CSB V2

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 0/10 | No CSB V2 completion evidence/gate defined. |
| `definition_matrix` | 0/10 | No CSB V2 completion evidence/gate defined. |
| `launch_smoke` | 0/10 | No CSB V2 completion evidence/gate defined. |
| `core_input_movement` | 0/15 | No CSB V2 completion evidence/gate defined. |
| `viewport_ui_render` | 0/20 | No CSB V2 completion evidence/gate defined. |
| `gameplay_systems` | 0/15 | No CSB V2 completion evidence/gate defined. |
| `audio_timing` | 0/10 | No CSB V2 completion evidence/gate defined. |
| `original_overlay_regression` | 0/10 | No CSB V2 completion evidence/gate defined. |

### DM2 V1

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 7/10 | DM2 DOS archive and SKULL.ASM inventory exist; no ReDMCSB C source-lock equivalent. |
| `definition_matrix` | 0/10 | No DM2 V1 parity matrix. |
| `launch_smoke` | 0/10 | No DM2 V1 launch/render smoke proof. |
| `core_input_movement` | 0/15 | No DM2 V1 movement/input completion gates. |
| `viewport_ui_render` | 0/20 | No DM2 V1 render/UI completion gates. |
| `gameplay_systems` | 0/15 | No DM2 V1 gameplay completion gates. |
| `audio_timing` | 0/10 | No DM2 V1 audio/timing completion gates. |
| `original_overlay_regression` | 0/10 | No DM2 V1 overlay regression proof. |

### DM2 V2

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 0/10 | No DM2 V2 completion evidence/gate defined. |
| `definition_matrix` | 0/10 | No DM2 V2 completion evidence/gate defined. |
| `launch_smoke` | 0/10 | No DM2 V2 completion evidence/gate defined. |
| `core_input_movement` | 0/15 | No DM2 V2 completion evidence/gate defined. |
| `viewport_ui_render` | 0/20 | No DM2 V2 completion evidence/gate defined. |
| `gameplay_systems` | 0/15 | No DM2 V2 completion evidence/gate defined. |
| `audio_timing` | 0/10 | No DM2 V2 completion evidence/gate defined. |
| `original_overlay_regression` | 0/10 | No DM2 V2 completion evidence/gate defined. |

### DM Nexus V1

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 0/10 | No DM Nexus V1 completion evidence/gate defined. |
| `definition_matrix` | 0/10 | No DM Nexus V1 completion evidence/gate defined. |
| `launch_smoke` | 0/10 | No DM Nexus V1 completion evidence/gate defined. |
| `core_input_movement` | 0/15 | No DM Nexus V1 completion evidence/gate defined. |
| `viewport_ui_render` | 0/20 | No DM Nexus V1 completion evidence/gate defined. |
| `gameplay_systems` | 0/15 | No DM Nexus V1 completion evidence/gate defined. |
| `audio_timing` | 0/10 | No DM Nexus V1 completion evidence/gate defined. |
| `original_overlay_regression` | 0/10 | No DM Nexus V1 completion evidence/gate defined. |

### DM Nexus V2

| Criterion | Score | Evidence note |
|---|---:|---|
| `reference_inventory` | 0/10 | No DM Nexus V2 completion evidence/gate defined. |
| `definition_matrix` | 0/10 | No DM Nexus V2 completion evidence/gate defined. |
| `launch_smoke` | 0/10 | No DM Nexus V2 completion evidence/gate defined. |
| `core_input_movement` | 0/15 | No DM Nexus V2 completion evidence/gate defined. |
| `viewport_ui_render` | 0/20 | No DM Nexus V2 completion evidence/gate defined. |
| `gameplay_systems` | 0/15 | No DM Nexus V2 completion evidence/gate defined. |
| `audio_timing` | 0/10 | No DM Nexus V2 completion evidence/gate defined. |
| `original_overlay_regression` | 0/10 | No DM Nexus V2 completion evidence/gate defined. |
