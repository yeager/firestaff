# Firestaff v3.0.335

## User-facing changes

- `DM1 FM Towns startup palette`: corrects admission of the retail startup
  palette so that the native dungeon handoff does not retain an incompatible
  title palette.

- `CSB FM Towns Entrance composition`: restores the source-bound C28/C002/C003
  presentation transaction, preventing the broad red C004-only background
  when the Entrance doors are shown.

- `DM2 DOS MVE presentation`: fixes title-video startup when the host audio
  device is unavailable, while preserving source PCM delivery whenever an
  output device is present.

## Developer changes

- `Original-media capture gates`: update private DM1, CSB Atari, CSB Amiga and
  FM Towns capture harnesses to reject duplicate or misclassified frames
  before they can be used as parity evidence.

- `CSBWin DSA evidence`: clarify that no stock-original DSA save corpus is
  currently admitted, preventing documentation from representing an
  unverified save as original-game evidence.
