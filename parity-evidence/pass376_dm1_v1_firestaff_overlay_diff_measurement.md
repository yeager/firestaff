# pass376_dm1_v1_firestaff_overlay_diff_measurement

Status: `PASS376_FIRESTAFF_OVERLAY_DIFF_MEASUREMENT_REPRODUCIBLE`

This is a measurement gate for the pass376 original <-> Firestaff overlay
diff artifacts under `parity-evidence/overlays/pass376_firestaff_pairing/`.
It is not a parity claim: the diff rows measure how far the latest
Firestaff capture set is from the PC 3.4 original-DOSBox capture set per
scene. Promotion to parity requires pass376 to confirm
`semantic_promotion_ok=true` on the original side.

## Source locks

| File | Lines | Function | Status |
|---|---|---|---|
| `DUNVIEW.C` | 8318-8611 | `F0128_DUNGEONVIEW_Draw_CPSF` | OK |
| `DRAWVIEW.C` | 709-858 | `F0097_DUNGEONVIEW_DrawViewport` | OK |
| `CLIKMENU.C` | 142-174,180-347 | `F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty` | OK |
| `COORD.C` | 1693-1724 | `` | OK |

## Scene measurements

| Scene | Differing / Total | Delta % | Firestaff SHA256 | Original SHA256 |
|---|---|---|---|---|
| `01_ingame_start_viewport_original_vs_firestaff` | 22149/30464 | 72.7055 | `68b4df029a8e34e3` | `c55c0311116bf2d3` |
| `02_ingame_turn_right_viewport_original_vs_firestaff` | 25009/30464 | 82.0936 | `f6446591628751ce` | `40140a2771d73c52` |
| `03_ingame_move_forward_viewport_original_vs_firestaff` | 23497/30464 | 77.1304 | `2d796ee923ac44b9` | `c55c0311116bf2d3` |
| `04_ingame_spell_panel_viewport_original_vs_firestaff` | 23497/30464 | 77.1304 | `2d796ee923ac44b9` | `c55c0311116bf2d3` |
| `05_ingame_after_cast_viewport_original_vs_firestaff` | 27108/30464 | 88.9837 | `2d796ee923ac44b9` | `40140a2771d73c52` |
| `06_ingame_inventory_panel_viewport_original_vs_firestaff` | 28480/30464 | 93.4874 | `bb1c86d2a93a7e30` | `c55c0311116bf2d3` |

Manifest: `parity-evidence/verification/pass376_dm1_v1_firestaff_overlay_diff_measurement/manifest.json`
