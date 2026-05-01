# Firestaff 0.2.9 macOS Preview

Firestaff 0.2.9 is a macOS preview build focused on restoring the expected DM1 startup path after recent launcher/title regressions.

## Highlights

- Restores the modern Firestaff launcher for V1 Original mode, including the branded logo and game artwork.
- Preserves the V1 original-faithful runtime after launch instead of making the launcher itself look like the old sparse placeholder menu.
- Routes DM1 menu launch through the launch-intent path again so the TITLE/entrance transition path is not bypassed.
- Keeps `FIRESTAFF_LEGACY_MENU=1` as the explicit old-menu escape hatch.

## Verification

- Built on macOS with CMake Release configuration.
- `m11_phase_a` passed.
- `m11_ingame_capture_smoke` passed.

## Preview caveats

- This is still a preview release, not final DM1 parity.
- DM1/V1 viewport, HUD, inventory, and original timing parity work remains active.
- CSB and DM2 remain future/runtime-expansion targets.

## Support

Sponsor ongoing Firestaff reverse-engineering and parity work:

- https://github.com/sponsors/yeager
