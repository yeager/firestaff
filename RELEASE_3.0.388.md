# Firestaff v3.0.388

## User-facing changes

- `Startup scan progress`: update every game-data scan phase to use the selected language throughout startup.

## Developer changes

- `Default game-data discovery`: verify that the no-game launcher discovers installed DM1, CSB and DM2 data from the default per-game data root and remains in the start menu.
- `DM1, CSB and DM2 startup coverage`: add original-media CLI and M12 menu retests across tested platforms, including DM1 PC34, Atari ST, Amiga 3.6 and FM Towns; CSB Atari STX, Amiga and FM Towns; and DM2 DOS and FM Towns.
- `Startup verification limits`: use SDL dummy output for original-media CLI and menu checks; physical display, input, audio output and complete visual parity remain unverified.
