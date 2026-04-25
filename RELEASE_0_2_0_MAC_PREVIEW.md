# Firestaff 0.2.0, macOS Preview

Firestaff 0.2.0 is an early macOS preview release.

This is still an honest preview, not a “parity complete” milestone. It is a much stronger checkpoint for the DM1/V1 original-faithful track and the modern launcher.

## Highlights

- Continued DM1/V1 parity work with source-backed compatibility passes.
- TITLE animation work now decodes, renders, and wires original `TITLE` data through the V1 frontend path.
- Native DOSBox/raw-title capture evidence now exists for original-runtime comparison work.
- Special credits/entrance VGA palette data is now exposed through a source-backed compat seam.
- Startup launcher is more robust:
  - safer input bounds
  - launch-path fixes
  - runtime fuzz coverage
  - Museum of Lore
  - game box-art presentation
  - CSB and DM2 shown as disabled/unsupported until their runtime support is ready
- Gamepad support now has a concrete implementation plan and regression strategy.
- macOS preview packaging remains a self-contained `.app` with bundled SDL3.

## What this is

- A macOS preview build for testing current Firestaff progress.
- A checkpoint for ongoing DM1/V1 parity work.
- A better-instrumented build with more original-reference evidence than earlier previews.

## What this is not

- Not full Dungeon Master 1:1 parity yet.
- Not finished V1 presentation parity.
- Not final audio timing/overlap parity.
- Not complete CSB or DM2 runtime support.
- Not a polished cross-platform release.

## Known status

- DM1/V1 is materially closer, but viewport/layout pixel parity, TITLE cadence/handoff timing, audio cadence/looping, and some original-bound behavior checks still remain.
- CSB and DM2 are intentionally visible but disabled in the launcher until support is real.
- The launcher is now safer and more useful, but still preview-grade.

## Support

If you want to help fund the reverse-engineering, verification, capture tooling, packaging, and long parity passes behind Firestaff, you can sponsor the project here:

- https://github.com/sponsors/yeager
