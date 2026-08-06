# Nexus V1 CI/CD Pipeline

## Overview

Nexus V1 is built as a static library (libfirestaff_nexus.a) compiled via CMake.
The normal CMake matrix builds it on Ubuntu, macOS and Windows and hard-runs
the data-free `nexus_production_source_boundary` test. Original Nexus data and
the Saturn BIOS remain user-supplied and are never placed in CI.

---

## Current CI Workflows

### verify.yml

Runs on: ubuntu-24.04, macos-14, windows-2022

Jobs:
1. cmake-build: Configure and build with CMake, run firestaff_m11_phase_a_probe and firestaff_m11_audio_probe (smoke tests)
2. verify: Run M10 verify script (if GRAPHICS.DAT present), deterministic hash via headless driver, cross-platform determinism check
3. web-wasm-toolchain-probe: Check Web/WASM toolchain availability
4. warnings-check: -Wall -Wextra -Werror on all probes

**Nexus coverage:** production library build on all three platforms plus the
production-source boundary test. The broader CTest catalogue is also reported;
private real-media tests remain skipped when their corpus is absent.

### release.yml

Runs on: tag push (v*) or manual workflow_dispatch

Jobs:
1. macos-dmg: Build on macOS, package DMG/ZIP, upload artifact
2. windows-installer: Build on Windows (MSYS2/UCRT64), package ZIP/NSIS, upload artifact
3. linux-x86_64: Build on Ubuntu, package DEB/RPM, upload artifact
4. linux-arm64: Build on Ubuntu ARM64 (Steam Deck), package DEB/RPM, upload artifact

**Nexus coverage:** release builds inherit the production source boundary, but
packaging still targets the current DM1/CSB/DM2 products. Nexus is not claimed
as a finished packaged game.

### pages.yml

Runs on: push to docs/ on main branch

**Nexus coverage:** documentation only; no game data is published.

---

## What Nexus CI Would Need

Nexus V1 CI requires a fundamentally different setup because:
1. No public disc image -- cannot ship original Sega Saturn data in CI
2. No Saturn BIOS/capture artifact may be committed to CI
3. Real-media tests require the user's private corpus

### Current CI for Nexus (Phase 0-1)

- cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
- cmake --build build --target firestaff_nexus
- test -f build/libfirestaff_nexus.a
- ctest --test-dir build -R '^nexus_production_source_boundary$'

This minimal job proves the library compiles on every push.

### Remaining CI expansion after source-owned runtime handoff

Add a full Nexus launch smoke test only after Saturn startup and VDP
handoff are source-bound. An empty fixture directory must not be treated as
positive playability proof.

### After the source-owned world model is admitted

- Run: ./build/firestaff --profile nexus --seed 42 --ticks 100
- Capture world state hash
- Upload hash as artifact

### Current local verification suite

Nexus tests are already wired into CTest. On a machine with the user-owned
retail corpus, run:

    ctest --test-dir build --output-on-failure -R nexus

The important current gates cover production source boundaries, startup,
DGN/PRS3, explicit real-data MENU.BPK/STABG receipts, all-16-file SLEV
task-profile receipts, SLEV/SAL asset metadata and mechanics no-mutation.
The `*_real` variants bind `FIRESTAFF_NEXUS_DATA_DIR` and are skip-safe only
when the private corpus is absent. These prove bounded source handling, not
full playability or task/event semantics.

---

## Cross-Platform Determinism for Nexus

The verify.yml cross-platform-determinism job checks that firestaff produces identical hashes across Ubuntu/macOS/Windows for the same input script.

For Nexus, when integrated:
- Nexus game loop must produce deterministic hashes on all three platforms
- Same input script -> same world state hash
- Add Nexus to the hash comparison job once Nexus binary exists

The Nexus library is not yet a finished packaged game binary.

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

## Remaining CI Gaps for Nexus

1. CI cannot run the private retail corpus or the user's Saturn BIOS.
2. Original Saturn VDP1/VDP2 capture remains a local evidence requirement.
3. Coverage percentages are not a meaningful parity metric until blocked
   source-format tests are separated from admitted runtime behavior.
4. No full packaged Nexus product is claimed by the release workflow.

---

## Completed minimum Nexus CI boundary

The normal `cmake-build` matrix now performs the build and hard-runs
`nexus_production_source_boundary`. Broader real-data and original-Saturn
capture tests remain local by design.
