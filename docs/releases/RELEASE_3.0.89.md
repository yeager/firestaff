# Firestaff 3.0.89

- DM1: source-bound inventory panel material validation.
- DM2: FM Towns source `GAME_LOAD` inventory swapping now exchanges a real
  authenticated `c_hero::item` link with `LeaderPossession`; empty
  `OBJECT_NULL` remains fail-closed/normalized and invalid DB links roll back.
- DM2: FM Towns M11 now opens the authenticated CHARSHEET inventory frame,
  consumes its global 255-colour `PAL_IRGB` pixels with the source `RECT_1EE`
  RAW4 crop, routes native inventory pointer contexts to slot selection, and
  commits authenticated equipment-slot clicks through the source
  `c_hero::item[30]`/`LeaderPossession` exchange.
- CSB: transactional ActiveGroup retirement for level changes.
- CSB: FM Towns C06 `LOAD CHAMPIONS` now opens the authenticated nine-row
  `PORTRAIT` picker, applies source F7084 scrolling/hit boxes, and imports
  the selected real `.CMP` through F7002. New-disk media switching remains
  fail-closed until its native C06 transaction is recovered.
- CSB: FM Towns C06 now exposes a receipt-bound F7000 portrait-save filename
  mapping from the authentic English and Japanese utility executables
  (`2:\\#CHAMP_NAME#.CMP`). The M11 save dialog remains fail-closed until its
  native three-choice modal and record/write branches are bound.
