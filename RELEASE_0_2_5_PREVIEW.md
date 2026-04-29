# Firestaff 0.2.5 Preview

Firestaff 0.2.5 is an early cross-platform preview release for macOS and Windows.

This is still an honest preview, not a parity-complete milestone. It packages the current DM1/V1 verification track with a stronger launcher, more source-backed evidence, and cross-platform build coverage.

## Highlights

- First preview release prepared for both macOS and Windows artifacts.
- ReDMCSB source-lock coverage has been closed across the current evidence map, with follow-up patch gaps inventoried for Christophe-friendly upstream work.
- DM1/V1 champion portrait and recruit-route evidence is now anchored to source identifiers such as `C007`, `C080`, `C127_SENSOR_WALL_CHAMPION_PORTRAIT`, and `F0280_CHAMPION_AddCandidateChampionToParty`.
- Startup launcher coverage has been hardened with M12 probes, settings smoke coverage, bounded input handling, and original-data status checks.
- Cross-platform CI now verifies Linux, macOS, and Windows determinism/build behavior before release.
- macOS packaging remains a self-contained `.app`/`.dmg` preview with bundled SDL3.
- Windows packaging provides a preview zip with `firestaff.exe`, SDL3 runtime DLL, README, and release notes.

## What this is

- A preview build for testers on macOS and Windows.
- A checkpoint for ongoing DM1/V1 original-faithful parity work.
- A source-backed evidence release meant to make the next compatibility patches smaller and easier to review.

## What this is not

- Not full Dungeon Master 1:1 parity yet.
- Not final viewport/HUD/inventory pixel parity.
- Not final TITLE cadence, audio cadence, or original handoff timing.
- Not complete CSB or DM2 runtime support.
- Not a polished end-user release.

## Known status

- DM1/V1 is materially closer, but viewport/world visuals, HUD top-row/status panel, inventory detail parity, and some original capture unblocks remain active work.
- CSB and DM2 are intentionally visible as future targets, not complete playable runtimes.
- The launcher is useful for preview testing but still evolving.

## Support

If you want to help fund the reverse-engineering, verification, capture tooling, packaging, and long parity passes behind Firestaff, you can sponsor the project here:

- https://github.com/sponsors/yeager
