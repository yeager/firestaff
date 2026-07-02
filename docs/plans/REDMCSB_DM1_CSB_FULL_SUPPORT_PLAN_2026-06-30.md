# ReDMCSB DM1/CSB full support plan - 2026-06-30

## Goal

Firestaff is a clean-room rewrite with full DM1 and CSB support according to
ReDMCSB or CSBwin. ReDMCSB is the primary functional template for DM1 and CSB
behavior; CSBwin is an accepted functional template for CSBwin-owned save,
Utility, custom-resource, and runtime workflow boundaries where ReDMCSB
delegates or does not fully spell out the behavior. Firestaff must prove its
own implementation through source-lock gates and runtime probes. Original
captures are required for visual, pixel, audio-cadence, timing, and same-state
parity promotion.

Current status indexes:

- `docs/parity/DM1_V1_CURRENT_STATUS_INDEX.json`
- `docs/parity/CSB_V1_CURRENT_STATUS_INDEX.json`

## Shared rules

- Do not count DM1 evidence as CSB completion unless the behavior is explicitly
  source-locked as inherited and a CSB-specific gate accepts that inheritance.
- Do not count ReDMCSB source evidence as pixel/content `MATCHED`.
- Do not promote direct-start/no-party DM1 captures as party/HUD/spell/inventory
  references.
- Keep stale documents useful as provenance, but use the current status indexes
  to decide whether a row is active, superseded, or evidence-only.

## DM1 active path

Highest current blocker: semantic original-route capture.

Current truth:

- The DOSBox capture executor now produces raw screenshots and crops locally.
- The current six-shot route is still not party-control-ready:
  `blank_right_column_frames=6`, `control_classes_seen=[]`, and
  `party_control_ready=false`.
- The next fix is original runtime route semantics: reach/recruit a champion or
  otherwise enter a party-control-ready state before spell/inventory labels.

Next DM1 pass:

1. Use the pass113/pass162/pass173 evidence to derive a narrow Hall of Champions
   route candidate.
2. Capture a diagnostic route with full-frame labels around entrance, portrait
   click, candidate panel, C160/C161, and first party-control frame.
3. Require pass80 + pass113 to prove party-control-ready before reusing the
   six-shot spell/inventory fixture.
4. Only after semantic original capture is ready, generate same-state Firestaff
   pairs and promote individual rows.

## CSB active path

Highest current blocker: CSB-specific runtime/playability proof.

Current truth:

- `docs/FINAL_CSB_GAPS.md` says the bounded ReDMCSB functional gap list is
  closed or audit-only.
- `docs/parity/PARITY_MATRIX_CSB_V1.md` still blocks full support on
  CSB-specific runtime, input/movement, viewport/HUD overlay, save artifacts,
  audio/timing, and end-to-end playability evidence.
- CTest now wires and passes the CSB-owned runtime slices
  `csb_v1_movement_command_step_runtime_pc34_compat` and
  `csb_v1_command_chain_move_attack_cast_runtime_pc34_compat`, proving queued
  movement plus MOVE/ATTACK/CAST command-chain semantics against ReDMCSB
  source-lock anchors.
- The CTest-wired CSB runtime/save/Utility spine is now 9/9 green:
  `csb_v1_input_command_queue_binding`,
  `csb_v1_movement_command_rotation_between_steps_runtime_pc34_compat`,
  `csb_v1_runtime_champion_inventory_handoff_pc34_compat`,
  `csb_v1_runtime_tick_accumulator`,
  `csb_v1_save_import_path_pc34_compat`, and
  `csb_v1_utility_import_block_verify_pc34_compat` now run alongside the two
  command/movement gates above. The joined
  `csb_v1_boot_runtime_handoff` gate also proves verified profile ->
  runtime dungeon handle -> imported party -> leader/rotation state -> one
  deterministic tick -> Utility NEW_GAME handoff.
- The fast CSB runtime/viewport smoke suite is green and no longer pulls the
  broad `firestaff_m10` library into these probe builds:
  `csb_v1_pc_real_asset_launch`, `csb_v1_pc34_quickplay_dungeon_handle`,
  `csb_v1_first_viewport_frame`, `csb_v1_boot_runtime_handoff`, and
  `csb_v1_runtime_route_first_frame_movement_utility_gate`. It proves
  skip-safe PC real-asset scan/boot/tick, dungeon-handle survival and rescan
  cleanup, first M11 viewport-frame render entry, the composed
  runtime/Utility handoff, and one Utility `NEW_GAME` -> runtime -> repeated
  queued movement route with a wall-blocked forward step, post-route viewport
  render, and bounded save-prefix roundtrip. It is still not full playability,
  full save compatibility, original capture, or pixel parity.
- Therefore the next CSB work should not reopen old functional gap docs by
  default. It should add runtime evidence from CSB state.

Next CSB pass:

1. Run the current CSB completion/readiness verifiers and record any actual
   failing gate.
2. Extend the fast CTest-wired bounded runtime/viewport route into a wider
   real-data fixture from CSB state: launch -> CSB-owned runtime -> repeated
   movement/input interactions beyond the synthetic route -> CSBGAME.DAT save/reload or
   Utility/new-adventure persistence boundary -> viewport capture.
3. Keep every claim CSB-specific. Do not inherit DM1 runtime evidence unless the
   verifier explicitly says the CSB rule is inherited from ReDMCSB.
4. After runtime support is stable, add original capture/overlay receipts for
   viewport/HUD/UI surfaces.

## Completion bar

The goal is not complete until both DM1 and CSB have:

- ReDMCSB source-lock coverage for gameplay, UI state machines, save/load,
  timers, input routing, and compatibility quirks.
- Firestaff runtime probes proving those contracts in clean-room code.
- Real-data skip-safe gates for user-supplied original assets.
- Save/import/export roundtrip receipts where the original format is claimed.
- Original-to-Firestaff capture pairs for visual/content rows marked `MATCHED`.
- Audio/timing receipts for cadence and overlap rows marked complete.
- End-to-end playability proof for the supported target lanes.
