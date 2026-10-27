# Firestaff v3.0.332

## Added

- `DM2 DOS introduction palette gate`: adds verification of the first visibly presented
  retail IBMIOP MVE frame after PAL8-to-RGB conversion, catching palette
  carry-over and RGB6/RGB8 conversion regressions before packaging.

## Changed

- `DM2 DOS startup verification`: changes the real-media check so the MVE
  handoff reaches SKULL's visible menu before the New Game input matrix runs.
