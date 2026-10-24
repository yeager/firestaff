# Nexus Build and Verification

## Build targets

Firestaff uses the repository's top-level CMake project and requires C11. The
Nexus sources are collected from `src/nexus/nexus_v1_*.c`; CMake then excludes
study and presentation modules that do not have an authenticated Saturn
runtime consumer. The production library also includes lifecycle-safe no-op
adapters and shared media helpers. Its public dependencies include `m` and
`firestaff_zip_extract`; SDL is used by the shared M11 host, not by the Nexus
format parsers themselves.

Configure and build the production library:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build --target firestaff_nexus
```

The normal Firestaff executable hosts Nexus through the shared M11 game view;
there is no separate retail-compatible Nexus executable. The CMake source
list is authoritative for production membership. A source file existing under
`src/nexus/` does not by itself mean that it is linked into the production
library or admitted as retail behavior.

## Tests and evidence

Build and run a focused test with:

```sh
cmake --build build --target test_nexus_v1_vdp2_runtime_tilemap
ctest --test-dir build --output-on-failure -R nexus_v1_vdp2_runtime_tilemap
```

Tests that consume operator-owned retail captures require their configured
external data paths. A pass without those paths may cover only a fixture or
skip-safe gate; it is not retail parity evidence. Use the Nexus data directory
under `~/.firestaff/data/nexus` and the capture instructions in
[`NEXUS_RUNTIME_CAPTURE.md`](NEXUS_RUNTIME_CAPTURE.md). Never replace available
retail input with generated game data.

The Nexus tests and probes cover different boundaries: byte-format decoders,
hash joins, runtime captures, and host-facing integration. A passing format
test does not prove that Saturn startup, menu, VDP ownership, viewport, HUD,
input, or gameplay behavior has been reproduced. Current parity status and
remaining evidence gaps are recorded in [`NEXUS_COMPLETION.md`](NEXUS_COMPLETION.md)
and [`NEXUS_STALE_CLAIM_AUDIT.md`](NEXUS_STALE_CLAIM_AUDIT.md).

## Retail build information

DMWeb's Nexus pages and the hash-verified retail disc corpus establish bounded
facts about `DM.BIN`, `LEV00.DGN` through `LEV15.DGN`, and documented data
formats. They do not establish the original executable name, complete boot
handoff, original compiler, linker script, or ROM/disc build command. The
authenticated corpus does not contain a `NEXUS.BIN` member. Do not infer a
retail executable path from that filename or from Firestaff's CMake targets.

Greatstone/SCK documents applicable Dungeon Master, Chaos Strikes Back, and
Dungeon Master II material. It is not a source for the Saturn Nexus retail
format or runtime.
