# Firestaff

An open-source engine port of **Dungeon Master** (1987) and **Chaos Strikes Back** (1989) for modern platforms.

**Status:** M10 complete — 20 of 20 engine-core phases passing deterministic verification. No UI/rendering yet (M11+).

## What it is

Firestaff is a deterministic, modular re-implementation of the FTL Games engine, designed to run on **macOS, Linux, and Windows** from day one. The entire engine is written in portable C with bit-identical cross-platform serialisation.

- Pure data-layer modules with invariant-based verification
- 500+ passing invariants across 20 engine phases
- Deterministic RNG, bit-identical save/load round-trip
- Headless driver for regression and replay verification
- Designed to read player-owned original `DUNGEON.DAT` files — no assets redistributed

## What it isn't (yet)

- Not playable in the traditional sense — UI/rendering/audio layer is M11+
- Does not distribute any FTL Games assets
- Does not include reverse-engineered source from other projects

## Architecture

The engine is split into 20 phases, each a separate compilable module with its own invariant probe:

| Phase | Module | What it does |
|-------|--------|--------------|
| 1-9 | Dungeon data layer | Tiles, text, doors, sensors, monsters, items |
| 10 | Movement + champions | Party position, champion state, inventory |
| 11 | Sensor execution | Pressure plates, triggers, actuator chains |
| 12 | Timeline scheduler | 16 event kinds with deterministic priority |
| 13 | Combat | Champion↔creature damage resolution |
| 14 | Magic | Spell system with runes and effects |
| 15 | Save/Load | CRC32 integrity, bit-identical round-trip |
| 16 | Monster AI | Creature behaviour state machine |
| 17 | Projectiles | Flight, collision, explosions |
| 18 | Champion lifecycle | Hunger, status effects, XP, rest |
| 19 | Runtime dynamics | Generators, light decay, fluxcage |
| 20 | Tick orchestrator | Main loop with deterministic harness |

Each phase emits a probe with dozens of invariants that must pass bit-identically on every compile.

## Building

Currently Unix-like (macOS, Linux). Windows via MSYS2/WSL during M10. Native Windows support lands in M11.

```sh
./run_firestaff_m10_verify.sh
```

This compiles all 20 phases and runs their invariant probes. Exits 0 on success; prints phase-by-phase PASS/FAIL summary.

### Headless driver

Run a deterministic game simulation from the command line:

```sh
./run_firestaff_headless_driver.sh /path/to/DUNGEON.DAT --seed 42 --ticks 100
```

Output includes final world hash, party state, and RNG seed. Two invocations with the same seed produce bit-identical output.

## Roadmap

- **M11** — SDL3 integration: rendering, audio, input (macOS/Linux/Windows)
- **M12** — Startup menu, i18n (sv/fr/de), asset validation via MD5
- **M13** — Chaos Strikes Back + Dungeon Master II integration

## Credits

Firestaff's development has been informed by **Christophe Fontanel's** extensive reverse-engineering work on [ReDMCSB](http://dmweb.free.fr/?q=community/redmcsb). His meticulous documentation of the original engine's bugs, quirks, and mechanics has been invaluable as a reference — though no ReDMCSB source code is included in Firestaff.

The original Dungeon Master and Chaos Strikes Back games were designed by **Doug Bell**, **Dennis Walker**, **Mike Newton**, **Andy Jaros**, and **Wayne Holder** at FTL Games. This project exists to preserve their work in a form playable on modern hardware.

## Licence

Firestaff is released under the **MIT Licence**. See [LICENSE](LICENSE).

This licence covers **only the engine code**. Dungeon Master and Chaos Strikes Back are © FTL Games / Software Heaven, Inc. You must own a legal copy of the original games (floppy disk, CD-ROM, or authorised digital release) to play them with Firestaff. No assets are distributed with this project.

## Contributing

Firestaff is in early development. Once M11 lands and the project is playable end-to-end, contribution guidelines will be published. For now, issues and discussion are welcome.

## Links

- Dungeon Master Encyclopaedia: [dmweb.free.fr](http://dmweb.free.fr/)
- ReDMCSB reference project: [dmweb.free.fr/?q=community/redmcsb](http://dmweb.free.fr/?q=community/redmcsb)
- Project tagline: *An open Dungeon Master engine — deterministic, modular, museum-grade*
