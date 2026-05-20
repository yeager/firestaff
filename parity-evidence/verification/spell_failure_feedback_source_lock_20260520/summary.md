# DM1 V1 spell failure feedback source lock (2026-05-20)

Status: implemented and verified pre-commit.

Source anchors:
- CLIKMENU.C:484-497 -> cast button routes to F0408_MENUS_GetClickOnSpellCastResult.
- MENU.C:1633-1663 -> F0408 clears Symbols[0], resets SymbolStep, and redraws F0397/F0398 for every result except C3 needs-flask.
- MENU.C:1817-1849 -> F0412 maps meaningless, needs-practice, and needs-flask failures through F0410 with C01/C00/C10; needs-flask returns C3.
- SPELFAIL.C:2-40,41-64,139-168,188-209 -> line feed, champion name prefix, and English failure message branches.
- MENUDRAW.C:47-120 -> available/champion spell symbol redraw routines.
- DEFS.H:2932-2941 -> failure and cast-result constants.

Implementation:
- Added DM1_SpellFailureFeedback metadata and F0408 cast-click cleanup predicate.
- Added a deterministic bridge/probe for message metadata and symbol clear/redraw policy.
- Kept needs-flask symbols intact, matching F0408 C3 behavior.
- Added focused tests for meaningless, needs-practice, needs-flask, and cleanup predicate behavior.

Verification pre-commit:
- cmake configure: PASS.
- clean target build: PASS.
- ctest dm1_v1_spell_casting_source_lock: PASS 1/1.
