# CSB V1 — Champion Parity Audit

**Audit:** 2026-08-31
**Primary reference:** local ReDMCSB `Toolchains/Common/Source/REVIVE.C`,
`COMMAND.C`, `DEFS.H`, and `CHAMPION.C`
**Scope:** native Firestaff CSB runtime; original media is read in RAM.

## Result

The earlier version of this page was a historical gap list. It no longer
described the implementation: its four claimed high/medium severity champion
gaps are covered by the native runtime and regression suite. This page is
therefore a current audit, not a backlog.

| ReDMCSB behaviour | Firestaff boundary | Evidence | Status |
|---|---|---|---|
| NEOPHYTE rank and mode | `csb_v1_neophyte_mode_pc34_compat.c` and M11 rank display | `csb_v1_neophyte_mode_pc34_compat` | Covered |
| C125–C128 champion-icon clicks | M11 native Atari, Amiga and FM Towns top-row handlers | `csb_v1_m11_launcher_handoff_boundary`, `csb_v1_m11_prison_runtime_hud_pc34`, `csb_v1_fmtowns_m11_game_handoff` | Covered |
| CSBGAME transfer/import fields | `csb_v1_runtime_pc34_compat.c` party-state import/save path | `csb_v1_champion_transfer_field_gate_pc34_compat` | Covered |
| C161 reincarnation | runtime-owned `csb_v1_runtime_reincarnate_pending_mirror_candidate_source_compat` | real PC3.4 C161 route plus `csb_v1_reincarnation_media_branch_pc34_compat` | Covered |
| F0306 low-stamina arithmetic | CSB character/runtime paths use the first-operand formulation | `CHAMPION.C:F0306` compiler matrix; CSB ST/FMT/Amiga package families | Covered for native CSB media families |

## C161: source-specific behaviour

ReDMCSB `REVIVE.C:F0282` is conditional on the original package. It is not
valid to apply one generic rule to every CSB platform.

| Original package family | Vitals | Statistics | Random points |
|---|---|---|---|
| PC I34 | unchanged | unchanged | exactly 12 `M002_RANDOM(7)` increments |
| Atari ST S20/S21 | halve current and maximum HP, stamina and mana | strength through anti-fire become `max(minimum, current - (current >> 3))`; Luck is not reduced | exactly 12 increments, including Luck when selected |
| Amiga A31/A33/A35 and FM Towns F31 | quarter current and maximum HP, stamina and mana | same one-eighth non-Luck reduction | exactly 12 increments, including Luck when selected |

All branches clear skills before applying the increments and advance the live
`G0349`/F0027 RNG stream. The M11 C161 action reaches this runtime function
after the source C127 candidate, C040 panel, and rename state have been
admitted; it does not route through the older generic compatibility shim.

## Remaining validation limits

- The supplied media proves bounded native boot, title, entrance, HUD and
  source-owned candidate routes. A long real-play capture covering a player
  death, C127, C161 rename, save, and cold resume is still desirable for each
  Amiga and FM Towns release.
- ReDMCSB bug identifiers `BUG0_37`, `BUG0_46`, `BUG0_50`, and `BUG0_51` are
  not blanket-completion labels. Their relevant champion click and state
  routes are covered above; any newly observed source-differing case must be
  added with a source locator and a real-media reproducer where media matters.

No synthetic game data is accepted as campaign evidence.

## F0306 compiler-order scope

`CHAMPION.C:F0306` carries `BUGX_XX`: this expression depends on the original
compiler's operand order. ReDMCSB names Megamax (CSB Atari ST), High C (CSB
FM Towns), and THINK C 4.0 (CSB Amiga) as first-operand-first. Firestaff's
native CSB character and runtime paths preserve that formulation. DM1 is
different: its PC 3.4 and Amiga 2.x packages require second-operand-first,
and that selection is handled at DM1 native handoff rather than shared with
CSB. This is executable provenance, never a Modern-mode setting.
