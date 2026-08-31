# CSB V1 Combat Audit

Audit date: 2026-08-31
Primary reference: local ReDMCSB WIP `PROJEXPL.C`, `GROUP.C`, `DUNGEON.C`, and
`DEFS.H`.

## Current implementation status

| CSB-specific rule | Firestaff boundary | Evidence | Status |
|---|---|---|---|
| CHANGE7_20 projectile speed | CSB scheduler uses `+1` even outside the party map; DM1 retains its source difference | `csb_v1_projectile_speed_pc34_compat` | Covered |
| Grey Lord presence | Grey Lord participates in the Lord-Chaos-equivalent proximity branch | `csb_v1_grey_lord_combat_pc34_compat` | Bounded behaviour verified |
| BUG0_69 teleport direction | direction is constrained before the group lookup | `csb_v1_lord_chaos_teleport_dir_pc34_compat`, `csb_v1_combat_bugfix_helpers_pc34_compat` | Covered |
| Grey Lord/Materializer teleporter eligibility | CSB helper now defaults to the CSB rule rather than DM1 comparison mode | `csb_v1_graphics_extras_pc34_compat` | Fixed 2026-08-31 |
| square-event/endgame raw boundary | CSB start/end raw event receipt | `csb_v1_f0426_f0445_startend_raw_pc34_compat` | Bounded route verified |

## Non-claims

No row above proves a full combat campaign, source-identical AI scheduling,
CSBGAME.DAT compatibility, or pixel equivalence. The next credible evidence is
a real-media combat capture containing Grey Lord behaviour and a save/reload
transition, replayed through the native Firestaff runtime. Synthetic fixtures
remain useful only for isolation; they cannot satisfy that parity claim.
