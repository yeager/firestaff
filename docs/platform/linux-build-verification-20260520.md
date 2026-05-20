# Linux Build Verification - 2026-05-20

This closes the TODO Platform slice for Linux build verification only. It does
not claim Windows, iOS, Android, Web/WASM, release publication, or full parity
suite completion.

## Source revision

- Branch: `worker/platform-opus46-20260520`
- Worktree: `/home/trv2/work/firestaff-worktrees/platform-opus46-20260520`
- Base: `origin/main`
- Commit verified before edits: `1c5fbef585c38d5de31f85d8816b75a2bf202ef0`
- CI source check: `.github/workflows/verify.yml` includes an `ubuntu-24.04`
  CMake build lane with Phase A and audio probes.

## Host environment

- Host: N2 / `firestaff-worker`
- OS: Ubuntu 24.04.4 LTS, Linux `6.8.0-110-generic`, `x86_64`
- Compiler: GCC 13.3.0 via `/usr/bin/cc`
- CMake: 3.28.3
- Ninja: 1.11.1
- SDL: SDL3 3.2.24 from `pkg-config sdl3`

## Commands and results

```sh
EVIDENCE_BUILD=/home/trv2/.openclaw/data/firestaff-platform-opus46-20260520/build-linux-verify-data

cmake -S . -B "$EVIDENCE_BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release
# PASS: configure completed and found SDL3.

cmake --build "$EVIDENCE_BUILD" --parallel 2
# PASS: 706/706 targets built, including firestaff and probe/test executables.

ctest --test-dir "$EVIDENCE_BUILD" -R '^(m11_phase_a|m11_audio|m11_launcher_smoke)$' --output-on-failure
# PASS: 3/3 tests passed.

BUILD_DIR="$EVIDENCE_BUILD" VERSION=2.4.0-platform-opus46 ./scripts/package_linux_preview.sh
# PASS: local preview packages built. No package was published.

dpkg-deb --info release/firestaff_2.4.0-platform-opus46_amd64.deb
# PASS: Debian package metadata reads correctly; package depends on libsdl3.

rpm -qpi release/firestaff-2.4.0-platform-opus46.x86_64.rpm
# PASS: RPM package metadata reads correctly; package requires libSDL3.so.0.
```

The generated build and local preview artifacts were moved out of the worktree
to `/home/trv2/.openclaw/data/firestaff-platform-opus46-20260520/`.

## Non-platform test note

A full `ctest --test-dir build-linux-verify --output-on-failure --parallel 2`
run was also attempted against the initial worktree build directory for
visibility. It finished with 316/347 passing and 31
failures in existing CSB/M12/DM1 parity/source-lock gates, including tests that
look for `build/test_dm1_v1_viewport_3d_pc34_compat` specifically rather than
the configured `build-linux-verify` directory. Those failures are outside this
Linux build-verification slice and were not committed as evidence changes.
