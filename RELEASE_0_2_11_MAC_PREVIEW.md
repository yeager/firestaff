# Firestaff 0.2.11 macOS Preview

Firestaff 0.2.11 is a macOS preview build focused on launcher/game transition polish for the DM1 V1 preview path.

## Highlights

- Preserves maximized-window state across launcher and in-game transitions.
- Fixes V1 entrance/startup interactions so preview startup input gates behave more predictably.
- Returns cleanly to the launcher when quitting from the V1 entrance/game path.
- Carries forward the 0.2.10 DM1 V1 source-lock evidence work for viewport/world rendering, HUD status/name boxes, inventory routing, and related parity gates.

## Verification

- Built on macOS with CMake Release configuration.
- Local CMake build completed successfully.
- Local runtime smoke tests passed:
  - `firestaff_m11_phase_a_probe`
  - `firestaff_m11_audio_probe`
  - `firestaff_m12_startup_menu_probe --runtime-fuzz 200`
- macOS app bundle packaged with bundled SDL3 dylib via `@executable_path/../Frameworks/...`.
- App bundle ad-hoc codesign verification passed.
- DMG and ZIP SHA-256 checksums generated.

## Preview caveats

- This is still a preview release, not final DM1 parity.
- DM1/V1 viewport, HUD, inventory, input, and original timing parity work remains active.
- CSB and DM2 remain source/runtime-expansion targets and are not claimed as complete playable support.

## Support

Sponsor ongoing Firestaff reverse-engineering and parity work:

- https://github.com/sponsors/yeager
