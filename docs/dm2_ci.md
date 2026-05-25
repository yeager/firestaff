# DM2 V1 — CI/CD: GitHub Actions Workflow for DM2

## Overview

DM2 V1 does not yet have its own CI pipeline. The existing `verify.yml`
workflow focuses on M10 (DM1/CSB) with Phase A probes and audio probes that
do not cover DM2 systems. This document describes what a DM2-specific CI
workflow should look like and what needs to be added.

---

## 1. Existing CI Pipeline (verify.yml)

The current `verify.yml` has these relevant jobs:

| Job | What it does | DM2 coverage |
|-----|-------------|--------------|
| `cmake-build` | Builds on ubuntu/macos/windows, runs Phase A + Audio probes | NONE — probes test M10 only |
| `verify` | M10 verify + deterministic hash | NONE |
| `cross-platform-determinism` | Compares hashes across OS | NONE |
| `web-wasm-toolchain-probe` | Web toolchain availability | NONE |
| `warnings-check` | `-Wall -Wextra -Werror` on probes | PARTIAL — dm2_v1_*.c stubs compile |
| `pages-deploy` | Deploys `docs/` on main push | YES — this run |
| `release` | Builds DMG/EXE/DEB/RPM on tag | NONE — no DM2 binary to package |

**Key gap**: No job runs `firestaff_dm2`-specific tests. The DM2 static library
is built as part of the main CMake build but is never tested.

---

## 2. What a DM2 CI Workflow Should Include

### 2.1 Proposed DM2 CI Job Additions to verify.yml

```yaml
  dm2-dungeon-loader-test:
    runs-on: ubuntu-24.04
    steps:
      - uses: actions/checkout@v4
      - name: Install SDL3
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential cmake ninja-build git pkg-config \
            libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxfixes-dev \
            libxss-dev libxtst-dev libwayland-dev libxkbcommon-dev wayland-protocols
          git clone --depth 1 --branch release-3.2.14 https://github.com/libsdl-org/SDL.git /tmp/SDL3
          cmake -S /tmp/SDL3 -B /tmp/SDL3-build -G Ninja -DCMAKE_BUILD_TYPE=Release
          cmake --build /tmp/SDL3-build --parallel
          sudo cmake --install /tmp/SDL3-build
          sudo ldconfig
      - name: Configure & Build
        run: cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel
      - name: Run DM2 tests (when fixtures available)
        run: |
          # Placeholder until fixtures exist
          ctest --test-dir build --output-on-failure -R dm2 2>&1 || \
            echo "DM2 tests not yet written — skipping"
      - name: Assert DM2 library built
        run: test -f build/libfirestaff_dm2.a && echo "libfirestaff_dm2.a: OK" || (echo "::error::DM2 library not built" && exit 1)
```

### 2.2 DM2 Test Stage (when tests exist)

Once DM2 V1 tests are written (after Phase 1-7 implementation):

```yaml
  dm2-parity-tests:
    strategy:
      fail-fast: false
      matrix:
        os: [ubuntu-24.04, macos-14, windows-2022]
    runs-on: ${{ matrix.os }}
    steps:
      - uses: actions/checkout@v4
      - name: Build DM2
        run: cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel
      - name: Extract DM2 fixtures
        run: ./scripts/extract_dm2_fixtures.sh || echo "SKIP: Fixture extraction not ready"
      - name: Run DM2 CTest suite
        run: ctest --test-dir build --output-on-failure -R dm2_v1_
      - name: Run DM2 headless probe
        run: ./build/firestaff_dm2_phase_a_probe --game=dm2 || echo "DM2 probe not yet built"
```

### 2.3 DM2 Determinism Test

Once DM2 has a working game state:

```yaml
  dm2-determinism:
    needs: dm2-parity-tests
    runs-on: ubuntu-24.04
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: cmake -B build -G Ninja && cmake --build build
      - name: Deterministic DM2 world hash
        run: |
          ./run_firestaff_headless_driver.sh \
            tests/fixtures/dm2/minimal.DAT --game=dm2 --seed 42 --ticks 100 \
            2>&1 | tee dm2-hash.txt
          grep "final_hash:" dm2-hash.txt
      - uses: actions/upload-artifact@v4
        with:
          name: dm2-determinism-hash
          path: dm2-hash.txt
```

---

## 3. Fixture Availability Gates

The CI should gracefully skip DM2 tests when fixtures are missing:

```yaml
- name: Check for DM2 dungeon fixture
  id: fixtures
  run: |
    if [ -f tests/fixtures/dm2/dungeon_pc_en.DAT ]; then
      echo "available=true" >> $GITHUB_OUTPUT
    else
      echo "available=false" >> $GITHUB_OUTPUT
    fi

- name: Run DM2 dungeon loader tests
  if: steps.fixtures.outputs.available == 'true'
  run: ctest --test-dir build -R dm2_v1_dungeon
```

This prevents CI failures from blocking the repo while fixtures are being gathered.

---

## 4. Cross-Platform Build Matrix for DM2

DM2 V1 should be tested on the same matrix as M10:

| OS | Compiler | Generator | Notes |
|----|----------|-----------|-------|
| ubuntu-24.04 | gcc | Ninja | Primary CI |
| macos-14 | clang | Unix Makefiles | ARM64 Apple Silicon |
| windows-2022 | gcc (MSYS2/UCRT64) | Ninja | Win32/x64 |

All matrix entries should produce `libfirestaff_dm2.a` and compile without warnings.

---

## 5. Release Pipeline Additions

When DM2 V1 reaches playable status (Phase 8), the `release.yml` workflow
should add DM2 to the packaging:

```yaml
  linux-dm2:
    name: DM2 Linux x86_64
    needs: resolve-version
    runs-on: ubuntu-24.04
    steps:
      - uses: actions/checkout@v4
      - name: Build
        run: cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build build --parallel
      - name: Package
        run: VERSION="$FIRESTAFF_VERSION" GAME=dm2 ./scripts/package_linux_preview.sh
```

And `verify.yml` should gate main-branch protection on DM2 test results:

```yaml
- name: Assert DM2 library present
  run: test -f build/libfirestaff_dm2.a || (echo "::error::DM2 library missing" && exit 1)
```

---

## 6. Current CI Status for DM2

| CI Aspect | Status | Notes |
|-----------|--------|-------|
| CMake compiles dm2_v1_*.c | PASS (stubs compile) | No functional code to break |
| -Wall -Wextra on dm2_v1_*.c | PASS (stubs clean) | Stubs are trivially clean |
| DM2 test suite | NOT WRITTEN | 0 tests |
| DM2 headless probe | NOT WRITTEN | No binary to run |
| DM2 fixture extraction | NOT AUTOMATED | Manual step required |
| DM2 deterministic hash | NOT IMPLEMENTED | Game not functional |
| DM2 packaging (release.yml) | NOT ADDED | No binary to package |

**The first CI milestone for DM2** is the same as the first development
milestone: extract dungeon fixture, write dungeon loader tests, get those to green.

---

## 7. GitHub Actions Secrets / Runners

DM2-specific runners would be the same as M10 runners. No additional secrets
or runners required. The `run_firestaff_m10_verify.sh` script is M10-specific;
a DM2 equivalent (`run_firestaff_dm2_verify.sh`) would need to be written
when fixtures are available.
