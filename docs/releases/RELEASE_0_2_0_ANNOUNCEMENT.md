# Firestaff 0.2.0 announcement

Firestaff 0.2.0 is out as an early macOS preview.

This release is still an honest preview, not a fake "parity complete" milestone. But it is a stronger checkpoint than the earlier previews:

- V1 ownership migration has continued deeper into compat/runtime paths.
- Sensor runtime wiring, animating door states, and creature walkability have all landed as verified passes.
- Original-reference capture and overlay tooling now exists in-tree, which makes future parity claims less hand-wavy and more measurable.
- `SONG.DAT` format decoding groundwork is landed and probe-verified, even though runtime audio still uses placeholder playback today.
- macOS preview packaging continues as a self-contained app bundle with bundled SDL3.

What 0.2.0 is:
- a real macOS preview for testing current Firestaff progress
- a checkpoint for ongoing DM1/V1 parity work
- a more source-backed and better-instrumented build than earlier previews

What 0.2.0 is not:
- not full DM1 parity yet
- not finished V1 presentation
- not final audio
- not a polished cross-platform release

If you want to support the long, expensive, and occasionally ridiculous amount of reverse-engineering, verification, capture tooling, packaging, and token burn behind Firestaff, you can sponsor the project here:

- https://github.com/sponsors/yeager
