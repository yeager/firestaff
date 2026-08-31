# CSB V1 Dungeon Audit

Audit date: 2026-08-31
Primary reference: local ReDMCSB WIP `DEFS.H`, `DUNGEON.C`, `MOVESENS.C`,
`GROUP.C`, `PROJEXPL.C`, and `DECOMPDU.C`.

## Current implementation status

| Surface | Native Firestaff boundary | Evidence | Status |
|---|---|---|---|
| CSB 24-level limit | `CSB_MAX_LEVELS == 24`, CSB world owns its level count | `csb_v1_decompdu_pc34_compat` | Bounded parser/world coverage |
| packed CSB dungeon data | source-faithful prefix decoder with bounds rejection | `csb_v1_decompdu_pc34_compat` | Covered for decoder inputs |
| end-game sensor raw route | raw F0426/F0445 start/end receipt gate | `csb_v1_f0426_f0445_startend_raw_pc34_compat` | Receipt verified; UI/capture parity open |
| version checker | CSB engine-version gate | `csb_v1_version_checker_pc34_compat` | Bounded gate verified |
| party/object/projectile teleporter chains | runtime movement chain and source rotation rules | `csb_v1_input_command_queue_binding`, `csb_v1_teleporter_rotation_runtime_pc34_compat`, `csb_v1_f0267_loaded_chain_pc34_compat` | Bounded runtime coverage |
| BUG0_69 direction guard | Lord Chaos direction is normalized before lookup | `csb_v1_lord_chaos_teleport_dir_pc34_compat`, `csb_v1_combat_bugfix_helpers_pc34_compat` | Covered |
| CSB projectile cadence | `+1` delay on all maps in CSB mode | `csb_v1_projectile_speed_pc34_compat` | Covered |

## Limits and next evidence

The old document incorrectly stated that the CSB loader was hard-coded to 14
levels and lacked DECOMPDU support. Both statements are obsolete. The current
coverage is deliberately bounded: it validates the parser and event mechanics,
not complete dungeon/campaign parity. Original-media captures must still prove
real CSB map traversal, endgame presentation, sensor sequences and save/reload
without substituting fixture data.
