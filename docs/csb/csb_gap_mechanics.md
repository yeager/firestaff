# CSB V1 Mechanics Audit

Audit date: 2026-08-31
Primary reference: local ReDMCSB WIP `Toolchains/Common/Source`
Scope: native Firestaff CSB V1 mechanics. Game archives are consumed in
memory; no emulator or extracted-game-data dependency is implied.

## Result

This page replaces the May 2026 speculative gap list. The items below have
the implementation/evidence status shown below. Passing a unit or bounded runtime test
does **not** claim a complete original-media playthrough or visual parity.

| ReDMCSB delta | Firestaff implementation | Regression evidence | Status |
|---|---|---|---|
| 24-level CSB world | `CSB_MAX_LEVELS`, CSB dungeon world and loader | `csb_v1_decompdu_pc34_compat` | Bounded implementation verified |
| compressed dungeon stream | `csb_v1_decompdu_pc34_compat.c` bit-packed decoder with explicit bounds checks | `csb_v1_decompdu_pc34_compat` | Bounded implementation verified |
| ZOKATHRA object creation | Source requires C51 junk allocation, not a fireball or DM1 no-op | ReDMCSB `MENU.C:1994–2027`; false unused power helper/test removed | CSB live cast implementation and capture remain open |
| Grey Lord | CSB Grey Lord proximity/attack classification helper | `csb_v1_grey_lord_combat_pc34_compat` | Bounded implementation verified |
| CHANGE7_24 reincarnation | source-family vitals/stat/RNG path | `csb_v1_champion_per_stat_parity_pc34_compat`, `csb_v1_reincarnation_media_branch_pc34_compat` | Covered at runtime boundary |
| CHANGE7_20 projectile cadence | CSB normalization keeps `+1` on every map | `csb_v1_projectile_speed_pc34_compat` | Scheduler verified |
| BUG0_69 teleporter access | CSB defaults to the CSB expansion: Lord Chaos, Lord Order, Grey Lord and Materializer | `csb_v1_graphics_extras_pc34_compat`, `csb_v1_lord_chaos_teleport_dir_pc34_compat` | Fixed 2026-08-31 |

## Source notes

- ReDMCSB `CHAMPION.C:F0306` is compiler/package-sensitive. CSB Atari ST,
  FM Towns and Amiga take first-operand-first arithmetic; this is selected at
  native handoff, not exposed as a presentation option.
- `PROJEXPL.C:F0219` distinguishes DM1's non-party-map delay from CSB's
  normalization. The Firestaff scheduler retains that difference.
- `GROUP.C` CHANGE7_19 / BUG0_69 requires a valid direction before indexing.
  The Firestaff direction gate is tested separately from the CSB teleporter
  eligibility rule.

## Remaining evidence limits

The supplied Atari, Amiga and FM Towns archives now pass boot and launcher
handoff checks. Still required for broad mechanics completion are authentic
long-route captures that include a CSB Grey Lord encounter, ZOKATHRA cast,
teleporter traversal, save/cold-resume, and end-game transition. Those captures
must be derived from real media; no synthetic campaign data can earn the
remaining parity credit.
