# Continuous integration

**Last reviewed: 2026-08-06.** CI is a verification gate for the whole
five-game tree; it does not require copyrighted game data to be committed.

Firestaff's GitHub Actions workflow is defined in
[`.github/workflows/verify.yml`](../.github/workflows/verify.yml). It runs on
pushes to `main` and on pull requests.

## What the workflow checks

- strict `-Wall -Wextra -Werror` compilation;
- asset hygiene, hash validation and PO-layout checks without copyrighted game
  data;
- CMake configure/build, CTest, the headless Phase A probe and the audio probe
  on Ubuntu, macOS and Windows;
- deterministic world-hash comparison across the build platforms.

The workflow uses a concurrency group for `main` and cancels an older run when
a newer commit arrives. That is expected: always inspect the newest run for
the current `main` SHA before treating a cancelled run as a failure.

## Recent failure signatures

### Lefthook installer fails during asset hygiene

The installer follows the Go version declared by the current Lefthook module.
The workflow therefore pins the matching Go toolchain instead of relying on an
older runner default. This job must install Lefthook and complete `lefthook
ci` before the run is considered healthy.

### `dm2_v1_decode_img9` is undefined during a CMake test link

This means a test target uses the DM2 GDAT loader without linking the IMG9
decoder translation unit. The fix belongs in the target's CMake source list;
do not add a stub implementation or weaken linker checks.

### macOS Homebrew reports an untrusted tap

The SDL3 install step may print a Homebrew trust warning for an unrelated tap
while still installing SDL3 successfully. The actionable result is the return
status of `brew install sdl3` and the following CMake build. Do not trust or
untap unrelated taps in the repository workflow.

## Local reproduction

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
ctest --test-dir build --output-on-failure
SDL_VIDEODRIVER=dummy ./build/firestaff_m11_phase_a_probe
```

For a remote run, use the Actions page or:

```bash
gh run list --repo yeager/firestaff --branch main
gh run view <run-id> --repo yeager/firestaff --log-failed
```

The repository must not contain original game data. CI intentionally runs the
asset checks with an empty data directory.

Documentation changes should also update the cross-game status in
[`PROJECT_STATUS.md`](PROJECT_STATUS.md) and the page map in
[`DOCUMENTATION_INDEX.md`](DOCUMENTATION_INDEX.md) when a public claim changes.
