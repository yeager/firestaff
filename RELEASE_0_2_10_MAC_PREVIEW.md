# Firestaff 0.2.10 macOS Preview

Firestaff 0.2.10 is a macOS preview build focused on DM1 V1 parity evidence, cleaner public positioning, and continued CSB/DM2 source-lock work.

## Highlights

- Refreshes the public README with a clearer product pitch, roadmap, supported tracks, and project status.
- Adds additional DM1 V1 source-lock gates for viewport/world rendering, HUD champion status/name boxes, inventory toggle behaviour, and inventory panel-open routing.
- Adds V2 logical-catalog source-evidence binding for spell-area UI assets.
- Records CSB save/sample-save blockers with source-backed evidence instead of overclaiming runtime compatibility.
- Adds a DM2 INT21 trace artifact that confirms `GRAPHICS.DAT` file I/O and preserves the remaining register-frame blocker.
- Sanitizes deprecated remote provenance references in newly merged evidence.

## Verification

- Built on macOS with CMake Release configuration.
- Local CMake build completed successfully.
- Local runtime smoke tests passed:
  - `m11_phase_a`
  - `m11_game_view`
  - `m11_launcher_smoke`
  - `m11_ingame_capture_smoke`
- Full local `ctest` currently reports source-lock gate failures where tests require worker-local ReDMCSB/original-game reference paths that are not present on the Mac build host. Those gates are evidence checks, not app packaging failures.

## Preview caveats

- This is still a preview release, not final DM1 parity.
- DM1/V1 viewport, HUD, inventory, and original timing parity work remains active.
- CSB and DM2 remain source/runtime-expansion targets and are not claimed as complete playable support.

## Support

Sponsor ongoing Firestaff reverse-engineering and parity work:

- https://github.com/sponsors/yeager
