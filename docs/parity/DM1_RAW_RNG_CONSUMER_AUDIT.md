# Raw RNG consumer audit

Reviewed 2026-09-06 after the F0412 sample-extraction corrections.
Scope: production calls to `F0731_COMBAT_RngNextRaw_Compat`, not every
independent RNG implementation or every original execution path.

BASE.C F0027:1688-1695 defines the game generator as multiplication by
0xBB40E62D plus 11, returning `(state >> 8) & 0xffff`. DEFS.H M003 masks
that returned word. CEDT002's distinct implementation is not this oracle.

| Consumer | Observed behavior | Disposition |
| --- | --- | --- |
| F0732 combat wrapper | Shifts state by eight before modulo | Matches sample extraction |
| M10 F0412 XP | Shifts state by eight after successful spell lookup | Corrected; focused regressions pass |
| M10 F0412 practice | Shifts before low-seven-bit mask; stops on first failure | Corrected; first/ninth-failure and successful-gate regressions pass |
| M10 legacy attack marker | Uses raw low bits for explicitly synthetic marker damage | Not an original live melee path; do not cite as parity |
| M10 forced RNG advance | Discards output for explicit fuzzing input | No sample extraction contract |
| CSB C37 wandering | Reseeds local state from dungeon/time/position and masks raw low two bits | Open source-owner audit; changing the mask alone cannot establish original AI parity |

M11_GameView_CastSpell uses F0732 for XP; its paid validator uses F0732
for practice and exits on first failure. Its prevalidated command handoff
is covered by a bounded potion mutation/XP/draw-count test. These checks
do not replace original input captures, full stream tracing, or an audit
of separately implemented generators.
