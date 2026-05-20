# Web/WASM Toolchain Probe - 2026-05-20

Scope: advance the Web/WASM Platform item with a repeatable, CI-safe toolchain probe and N2 evidence. This does not close Web/WASM build verification; no Emscripten configure/build was completed on N2.

## Source revision

- Branch: `platform-web-wasm-probe-20260520-200136`
- Worktree: `/home/trv2/work/firestaff-worktrees/platform-web-wasm-probe-20260520-200136`
- Base: `origin/main` at `e2d6761c73cfe51785261d31fafb1ff80294cdda`
- Probe: `scripts/probes/run_firestaff_web_wasm_toolchain_probe.sh`
- CI guard: `.github/workflows/verify.yml` now runs the probe in a dedicated Ubuntu job without installing SDKs or publishing artifacts.

## What the probe checks

The probe is intentionally bounded:

- checks for `emcmake`, `emcc`, and `cmake`;
- skips with an explicit message when Emscripten is missing;
- when the toolchain exists, configures CMake through `emcmake` into `build/web-wasm-probe` by default;
- builds only the `firestaff`, `firestaff_m11_audio_probe`, and `firestaff_m11_phase_a_probe` smoke targets;
- makes no browser/runtime claim.

## N2 evidence

Host:

- N2 / `firestaff-worker`
- Ubuntu 24.04.4 LTS, Linux `6.8.0-110-generic`, `x86_64`
- CMake 3.28.3
- Ninja 1.11.1
- Host SDL3 3.2.24 from `pkg-config sdl3`

Command:

```sh
BUILD_DIR=/home/trv2/work/firestaff-builds/web-wasm-probe-20260520 ./scripts/probes/run_firestaff_web_wasm_toolchain_probe.sh
```

Result:

```text
WEB_WASM_TOOLCHAIN_PROBE: missing emcmake
WEB_WASM_TOOLCHAIN_PROBE: missing emcc
WEB_WASM_TOOLCHAIN_PROBE: cmake=/usr/bin/cmake
WEB_WASM_TOOLCHAIN_PROBE: SKIP - Emscripten toolchain is not available; no Web/WASM build claim is made.
```

## Current blocker

N2 does not have the Emscripten compiler wrappers (`emcmake`, `emcc`). Per the Platform TODO constraints, no SDK was installed in this pass. The Web/WASM item remains open until an Emscripten-backed configure/build and at least one browser/runtime smoke check are captured.

## CMake/readiness notes from inspection

`CMakeLists.txt` currently uses host SDL discovery (`find_package(PkgConfig REQUIRED)`, `find_package(SDL3 QUIET CONFIG)`, then `pkg_check_modules` fallback). No Emscripten-specific SDL port flags, HTML shell target, asset preload plan, or browser smoke test are wired yet. The new probe keeps that state visible without pretending a Web/WASM port is done.
