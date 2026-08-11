# Firestaff 3.0.89

- DM1: source-bound inventory panel material validation.
- DM2: FM Towns source `GAME_LOAD` inventory swapping now exchanges a real
  authenticated `c_hero::item` link with `LeaderPossession`; empty
  `OBJECT_NULL` remains fail-closed/normalized and invalid DB links roll back.
- CSB: transactional ActiveGroup retirement for level changes.
- CSB: FM Towns C06 `LOAD CHAMPIONS` now opens the authenticated nine-row
  `PORTRAIT` picker, applies source F7084 scrolling/hit boxes, and imports
  the selected real `.CMP` through F7002. New-disk media switching remains
  fail-closed until its native C06 transaction is recovered.
