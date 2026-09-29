# Firestaff v3.0.349

## User-facing changes

- `Windows settings`: fix repeated configuration and JSON export saves by replacing
  existing files through the native file-replacement operation.

- `DM2 platforms`: remove DM2 PC-9821 support. Its launcher entry, CLI aliases, native media
  admission and runtime-specific paths are removed. An explicitly selected
  PC-9821 archive cannot silently select another edition from its directory.
- `Launcher dialogs`: move font, artpack, data-directory and custom-music
  dialog results to the main thread and prevent callbacks from accessing
  destroyed menus. Cancelled and oversized selections retain the prior value.
- `Quick Resume`: fix preference and remembered-save retention when disabled;
  recheck saves when enabled. Retain validated DM2 fallback saves when a
  manifest names a save that is unavailable on the current machine.
- `Startup and audio`: fix DM1 French DOS ZIP and CSB Atari ST/FM Towns
  launch routes; restore platform selection after Back, use the selected
  audio device, and apply DM2 volume and runtime pause settings.

## Developer changes

- `Desktop CI`: add launcher settings and dialog-lifetime regressions to desktop CI.
- `Platform policy`: add explicit rejection checks for removed PC-9821 aliases and media, and
  update supported-edition, fingerprint, music-routing and discovery tests.
- `Release metadata`: update version metadata, embedded changelog and source inventory synchronized
  with this release. Packages contain no original game data.

## Verification limits

- Original-media startup checks exercise source state and rendering receipts.
  SDL dummy output does not establish physical audio playback, MacBook Pro M5
  Retina input/display behavior, or original-versus-Firestaff pixel parity.
- Desktop packaging checks and mobile builds are distinct from gameplay
  verification on physical Windows, Linux, macOS, iOS and Android devices.
- PC-9801 preservation references do not enable gameplay. PC-9821 is excluded
  from supported platforms.
