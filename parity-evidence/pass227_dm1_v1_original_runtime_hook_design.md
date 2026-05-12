# Pass227 — DM1 V1 original-runtime hook/API design

Status: `PASS_ORIGINAL_RUNTIME_HOOK_DESIGN_READY`

Scope: JSON-only design spike for the missing stock-runtime state hook. It produces no PNG/PPM artifacts and makes no pixel-parity claim.

## Feasible route

Use the existing N2 original stage plus DOSBox-X/dosbox-debug as the first implementation route. The missing piece is not another capture script; it is a small debugger transcript adapter backed by a checked-in symbol/address map for the stock `FIRES` gameplay module.

## Source seams to bind

- PASS `command_accepted` — `COMMAND.C:2045-2156` / `F0380_COMMAND_ProcessQueue_CPSC` (code breakpoint after command dequeue)
- PASS `turn_or_step_state_applied` — `CLIKMENU.C:142-328` / `F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty` (code breakpoint after turn/step handler return plus global reads)
- PASS `party_coordinates_committed` — `MOVESENS.C:442-443` / `F0267_MOVE_GetMoveResult_CPSCE` (memory write watchpoint for party X/Y globals)
- PASS `draw_uses_mutated_tuple` — `GAMELOOP.C:88-91` / `F0002_MAIN_GameLoop_CPSDF` (code breakpoint at draw call with direction/x/y reads)
- PASS `viewport_present` — `DRAWVIEW.C:709-842` / `F0097_DUNGEONVIEW_DrawViewport / E0017_MAIN_Exception28Handler_VerticalBlank_CPSDF` (code breakpoint or memory watch on viewport request plus C007 blit call)

## Implementation path

1. **Canonical runtime/image fixture** — Launch the existing N2 PC 3.4 stage through DOSBox-X or dosbox-debug with `DM -vv -sn -pk`; treat `FIRES` as the gameplay module and record the loaded segment/base in a JSON run log.
   - deliverable: runtime_image.json with DM.EXE/FIRES/VGA hashes, load CS:IP, and emulator version
2. **Address-map bootstrap** — Create `data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json` mapping the five source seams to runtime CS:IP/global addresses. Bootstrap it with DOSBox debugger `BP`, `BPMEM`, `MEMDUMPBIN`, and byte-signature/disassembly notes; keep all map confidence fields explicit.
   - deliverable: symbol_map JSON; no source claims are promoted for entries with confidence below `verified_runtime_hit`
3. **Debugger transcript adapter** — Add a small Python runner that starts DOSBox-X/dosbox-debug under xvfb, feeds debugger commands from the symbol map, parses breakpoint/watchpoint hits, reads the required globals, and emits newline-delimited JSON events using the minimal schema.
   - deliverable: `tools/dm1_original_runtime_trace.py --route ... --symbol-map ... --out trace.ndjson`
4. **Route-driver integration** — Reuse the current original route driver only for input delivery/timing. Stop treating raw frames as state evidence; each input label is promoted only when the trace chain links accepted_seq across command_accepted, movement_applied, and viewport_present.
   - deliverable: pass224 successor gate consumes trace JSON and rejects duplicate/static visual captures automatically
5. **First narrow command** — Prove one turn command first (left/right) because it avoids destination/sensor ambiguity, then add one forward step with the MOVESENS X/Y commit watchpoint.
   - deliverable: one accepted turn trace and one accepted step trace, both JSON-only

## Minimal JSON event schema

```json
{
  "artifact_policy": {
    "forbidden_extensions": [
      ".png",
      ".ppm"
    ],
    "json_only": true,
    "screenshots_not_evidence": true
  },
  "event_common_fields": [
    "seq",
    "type",
    "emulator",
    "module",
    "cs",
    "ip",
    "linear_pc",
    "route_label",
    "monotonic_cycle"
  ],
  "events": {
    "command_accepted": {
      "required_fields": [
        "accepted_seq",
        "command_id",
        "queue_first_index",
        "queue_last_index",
        "raw_command_x",
        "raw_command_y"
      ],
      "source_seam": "COMMAND.C F0380 after G0432/G0433 dequeue"
    },
    "movement_applied": {
      "movement_kind_enum": [
        "turn",
        "step",
        "blocked_step"
      ],
      "required_fields": [
        "accepted_seq",
        "movement_kind",
        "party_direction",
        "party_map_x",
        "party_map_y",
        "disabled_movement_ticks",
        "stop_waiting_for_input"
      ],
      "source_seam": "CLIKMENU.C turn/step handler return and MOVESENS.C coordinate commits"
    },
    "viewport_present": {
      "required_fields": [
        "accepted_seq",
        "zone",
        "draw_viewport_requested",
        "viewport_bitmap_seg",
        "viewport_bitmap_off",
        "viewport_digest16",
        "vblank_counter"
      ],
      "source_seam": "DRAWVIEW.C viewport request/vblank and C007_ZONE_VIEWPORT blit"
    }
  },
  "promotion_predicate": [
    "events are from the stock PC 3.4 runtime module, not a rebuilt ReDMCSB binary",
    "command_accepted.accepted_seq is referenced by a later movement_applied event",
    "movement_applied records the post-handler direction/x/y/wait state",
    "viewport_present with the same accepted_seq occurs later and uses zone C007_ZONE_VIEWPORT",
    "no PNG/PPM/screenshot artifact is used to satisfy the chain"
  ],
  "required_order": [
    "command_accepted",
    "movement_applied",
    "viewport_present"
  ],
  "schema": "dm1_v1_original_runtime_trace.minimal.v1"
}
```

## Asset/tool audit

- tools: `{'dosbox': '/usr/bin/dosbox', 'dosbox-debug': '/usr/bin/dosbox-debug', 'dosbox-x': '/usr/bin/dosbox-x', 'xvfb-run': '/usr/bin/xvfb-run', 'xdotool': '/usr/bin/xdotool', 'gdb': '/usr/bin/gdb', 'objdump': '/usr/bin/objdump', 'strings': '/usr/bin/strings'}`
- `DM.EXE` exists=`True` sha256=`4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4`
- `FIRES` exists=`True` sha256=`ebf84045c3edbce7690b826eadbea2e278fbb4c0a3cc19a470552586f37712eb`
- `VGA` exists=`True` sha256=`4d9815e777e135bf69e3575fea533128b6073ae8c6b5282c24529c606f95af3b`
- `DATA/DUNGEON.DAT` exists=`True` sha256=`d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
- `DATA/GRAPHICS.DAT` exists=`True` sha256=`2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`

## Decision

Land this as the exact handoff for the next coding pass: implement the symbol-map JSON and debugger transcript adapter, starting with one turn command. Do not spend another pass on PNG/PPM route captures until this API emits the three-event chain.

Non-claims: no runtime trace was captured here; no address map entry is claimed verified; no screenshot artifact is evidence for this pass.
