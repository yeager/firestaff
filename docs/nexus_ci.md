# Nexus V1 CI/CD Pipeline

## Overview

Nexus V1 is built as a static library (libfirestaff_nexus.a) compiled via CMake.
The CI pipeline in .github/workflows/ does not currently test Nexus specifically.
All CI jobs test DM1/M11 via probes and headless drivers.

---

## Current CI Workflows

### verify.yml

Runs on: ubuntu-24.04, macos-14, windows-2022

Jobs:
1. cmake-build: Configure and build with CMake, run firestaff_m11_phase_a_probe and firestaff_m11_audio_probe (smoke tests)
2. verify: Run M10 verify script (if GRAPHICS.DAT present), deterministic hash via headless driver, cross-platform determinism check
3. web-wasm-toolchain-probe: Check Web/WASM toolchain availability
4. warnings-check: -Wall -Wextra -Werror on all probes

**Nexus coverage:** NONE. verify.yml does not build or test Nexus.

### release.yml

Runs on: tag push (v*) or manual workflow_dispatch

Jobs:
1. macos-dmg: Build on macOS, package DMG/ZIP, upload artifact
2. windows-installer: Build on Windows (MSYS2/UCRT64), package ZIP/NSIS, upload artifact
3. linux-x86_64: Build on Ubuntu, package DEB/RPM, upload artifact
4. linux-arm64: Build on Ubuntu ARM64 (Steam Deck), package DEB/RPM, upload artifact

**Nexus coverage:** NONE. release.yml builds firestaff (DM1/CSB/DM2) only.

### pages.yml

Runs on: push to docs/ on main branch

**Nexus coverage:** NONE. Only deploys documentation pages.

---

## What Nexus CI Would Need

Nexus V1 CI requires a fundamentally different setup because:
1. No public disc image -- cannot ship original Sega Saturn data in CI
2. No compiled binary output -- only a static library
3. No regression tests -- 0 tests exist for Nexus

### Minimum CI for Nexus (Phase 0-1)

- cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
- cmake --build build --target firestaff_nexus
- test -f build/libfirestaff_nexus.a

This minimal job proves the library compiles on every push.

### After Phase 1 (Runtime Profile)

Add to verify.yml:
- Build full firestaff binary (links Nexus)
- Run: ./build/firestaff --profile nexus --data-dir tests/fixtures/ 2>&1
- Must start without loading DM1 assets (smoke test for profile isolation)

### After Phase 3 (World Model)

- Run: ./build/firestaff --profile nexus --seed 42 --ticks 100
- Capture world state hash
- Upload hash as artifact

### After Phase 7 (Verification Suite)

Add Nexus tests to CTest, run as part of verify.yml:
- ctest --test-dir build --output-on-failure -R nexus
- cppcheck --enable=all -std=c99 -I include src/nexus/

---

## Cross-Platform Determinism for Nexus

The verify.yml cross-platform-determinism job checks that firestaff produces identical hashes across Ubuntu/macOS/Windows for the same input script.

For Nexus, when integrated:
- Nexus game loop must produce deterministic hashes on all three platforms
- Same input script -> same world state hash
- Add Nexus to the hash comparison job once Nexus binary exists

Currently no such binary exists.

---

## Artifact Strategy

Nexus is a library, not a shippable binary. Artifacts are:

| Artifact | When | Storage |
|----------|------|---------|
| libfirestaff_nexus.a | Every build | Uploaded to GitHub Actions artifact |
| Nexus headers | Every build | Included in artifact |
| Nexus parity evidence | Phase 7+ | parity-evidence/nexus/ |

No DMG/EXE/DEB/RPM for Nexus until a proper game binary ships.

---

## CI Gaps for Nexus

1. No Nexus test target in CMakeLists.txt -- tests/ has no nexus_ tests, no CTest entries
2. No Nexus build target linked to any executable -- library not tested in context
3. No fixture or disc image in CI -- cannot run integration tests
4. No linting for Nexus source -- cppcheck/pylint not run on src/nexus/
5. No coverage measurement -- no coverage report for Nexus (0% coverage)

---

## Recommendation: Add Minimal Nexus Job to verify.yml

Add this job to .github/workflows/verify.yml:

nexus-compile-check:
  runs-on: ubuntu-24.04
  steps:
    - uses: actions/checkout@v4
    - name: Configure
      run: cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
    - name: Build Nexus library
      run: cmake --build build --target firestaff_nexus
    - name: Verify artifact
      run: test -f build/libfirestaff_nexus.a && echo OK

This proves the library compiles on every push. Expand when Phase 3+ is done and tests exist.