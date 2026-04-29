# Pass172 — ReDMCSB patch gap inventory for Christophe

## Executive take

ReDMCSB is not “missing answers” for DM1/V1 parity; it is missing a clean patch queue around known documented bugs/anomalies and a few source facts that Firestaff has now independently gated. Highest value is to send small, source-local patches with repro/evidence, not a giant rewrite.

## Priority patch candidates

- **BUG0_71** (timing): fast-computer timing too short; title/entrance/endgame need enforced VBlank/delay parity — files: `ENDGAME.C`, `ENTRANCE.C`, `MOVESENS.C`, `TITLE.C`; hits: 4
- **BUG0_73** (input): mouse click command queue race with keyboard command can drop click — files: `COMMAND.C`; hits: 3
- **BUG0_75** (viewport): champion portrait ordinal not reset when no wall square in view — files: `DUNGEON.C`; hits: 1
- **BUG0_76** (viewport): single global inscription thing makes same text draw on multiple wall sides — files: `DUNGEON.C`; hits: 1
- **BUG0_86** (assets): champion portrait/resurrect/reincarnate graphics missing/garbage on constrained-memory PC/custom mirror maps — files: `DUNVIEW.C`; hits: 1
- **BUG0_35** (inventory): open chest icon not updated in action hand while inventory open — files: `CHAMDRAW.C`; hits: 2
- **BUG2_00** (inventory): scroll open/closed icon regression caused by chest fix — files: `CHAMDRAW.C`; hits: 2
- **BUG0_36** (hud): dead champion cell can erase another champion icon after load/rebirth — files: `CHAMDRAW.C`; hits: 1
- **BUG0_39** (inventory): leader Food/Water/Poisoned panel flashes while swapping scroll/chest — files: `CHAMPION.C`; hits: 1
- **BUG0_42** (hud): action icons not redrawn after unfreeze — files: `CHAMPION.C`; hits: 1
- **BUG0_43** (candidate): game may not end when last living champion killed while viewing candidate portrait — files: `CHAMPION.C`, `CLIKMENU.C`; hits: 4
- **BUG0_52** (input): keyboard commands while pressing eye/mouth can leave pointer/input/menu state broken — files: `COMMAND.C`; hits: 1
- **BUG0_53** (candidate): rest possible with candidate inventory open, can duplicate champion/items — files: `COMMAND.C`; hits: 1
- **BUG0_64** (viewport): floor ornaments drawn over open pits — files: `DUNVIEW.C`; hits: 11
- **BUG0_74** (viewport): creatures drawn with wrong colors through Thieves Eye wall hole — files: `DUNVIEW.C`; hits: 4
- **BUG0_83** (viewport): Thieves Eye hole moves with opening doors — files: `DUNVIEW.C`; hits: 1
- **BUG0_84** (endgame): ending screen palette may be uninitialized — files: `ENDGAME.C`; hits: 1
- **BUG0_85** (movement): empty party can move over group in custom dungeon and kill it — files: `CLIKMENU.C`; hits: 1

## Structural gaps / grep buckets

- `BUGX_XX`: unresolved/unnumbered source bug markers — hits: 43; files: `BASE.C`, `CEDT013.C`, `CEDTINC4.C`, `CEDTINCI.C`, `CHAMPION.C`, `CLIKMENU.C`, `CLIKVIEW.C`, `COMMAND.C`
- `ANOMALY_`: port/compiler/anomaly markers — hits: 222; files: `ANIM.C`, `ANIMSND.C`, `ANIMTOWN.C`, `BLITFILL.C`, `CASTER.C`, `CEDT001.C`, `CEDT007.C`, `CEDT012.C`
- `missing`: explicit missing support/code/comment markers — hits: 24; files: `CEDTINCI.C`, `CHAMDRAW.C`, `CHAMPION.C`, `COMMAND.C`, `COMPILE.H`, `DEFS.H`, `DUNVIEW.C`, `FIO1.C`
- `PC_FIX_CODE_SIZE`: PC code-size parity macro comments warn of additional/missing instructions — hits: 78; files: `CHAMDRAW.C`, `CHAMPION.C`, `CLIKMENU.C`, `CLIKVIEW.C`, `COMMAND.C`, `COMPILE.H`, `COORD.C`, `DUNGEON.C`
- `SU1E missing`: known missing media/version gate in declarations — hits: 1; files: `DEFS.H`

## Firestaff-backed patches we can prepare

- **DM1/V1 title+entrance timing** — ReDMCSB files: `TITLE.C`, `ENTRANCE.C`, `ENDGAME.C`; evidence: `pass160_redmcsb_source_lock`, `pass171_redmcsb_source_lock_closure`; patch shape: make BUG0_71 timing waits explicit/portable; document expected VBlank counts for PC-fast hosts
- **C007 viewport → C080 click routing** — ReDMCSB files: `COMMAND.C`, `CLIKVIEW.C`, `MOVESENS.C`; evidence: `pass161_c080_viewport_click_source`, `pass164_champion_portrait_click_source_path`; patch shape: add comments/tests or helper documenting C007/C080/front-wall sensor route into C127 portrait sensor
- **champion portrait geometry** — ReDMCSB files: `DUNVIEW.C`, `COORD.C`; evidence: `pass165_champion_portrait_click_geometry`; patch shape: document PC viewport origin + portrait bbox `{96,127,35,63}` → screen click center `(111,82)`
- **candidate champion recruit path** — ReDMCSB files: `MOVESENS.C`, `REVIVE.C`, `COMMAND.C`; evidence: `pass163_champion_recruit_source_path_lock`; patch shape: clarify that C160/C161 only works after C127/F0280 candidate state; avoid misleading panel-first route assumptions
- **object/status/action icon geometry** — ReDMCSB files: `CHAMDRAW.C`, `ACTIDRAW.C`, `PANEL.C`, `COORD.C`; evidence: `pass169_redmcsb_anchor_gap_resolution`, `pass170_source_mentioned_unresolved_batch`; patch shape: extract/label source constants for 16x16 object icons and status/action hand zones

## Recommendation

Start with 5 small Christophe-friendly patches:
1. Documentation/test note for DM1/V1 recruit route: C007→C080→C127→F0280 before C160/C161.
2. Portrait click geometry comment/table in `DUNVIEW.C`/`COORD.C`.
3. BUG0_71 timing notes for TITLE/ENTRANCE/ENDGAME with exact waits.
4. BUG0_73 input queue race explanation with a minimal safe fix candidate or at least a regression note.
5. Inventory icon correctness: BUG0_35 + BUG2_00 split so chest fix does not regress scroll icons.

Do **not** lead with huge gameplay-behavior changes. Lead with source comments, tests, and narrowly-scoped bug fixes where ReDMCSB already documents the bug.
