# Firestaff

<p align="center">
  <img src="assets/branding/firestaff-logo.png" alt="Firestaff logo" width="420">
</p>

<p align="center">
  <a href="https://github.com/yeager/firestaff/actions/workflows/verify.yml"><img alt="CI" src="https://github.com/yeager/firestaff/actions/workflows/verify.yml/badge.svg"></a>
  <a href="LICENSE"><img alt="License: MIT" src="https://img.shields.io/badge/License-MIT-blue.svg"></a>
  <img alt="Platforms" src="https://img.shields.io/badge/platforms-macOS%20%7C%20Linux%20%7C%20Windows-lightgrey">
  <a href="https://github.com/sponsors/yeager"><img alt="GitHub Sponsors" src="https://img.shields.io/badge/GitHub%20Sponsors-support%20Firestaff-pink"></a>
</p>

<p align="center">
  <strong>A modern, portable, source-faithful engine for Dungeon Master, Chaos Strikes Back, and Dungeon Master II.</strong>
</p>

Firestaff is an open-source engine project for the classic FTL dungeon crawlers. It preserves the feel and mechanics of the originals while making the engine portable, inspectable, testable, and ready for future enhanced presentations.

The project has three product tracks:

- **V1 — Original**: faithful presentation and behaviour for the original games, with a clean shared launcher.
- **V2 — Enhanced 2D**: higher-resolution 2D UI and art, wider aspect ratios, and richer presentation on top of the original rules.
- **V3 — Modern / 3D**: a later reinterpretation track once the original engine baseline is trustworthy.

Firestaff does **not** ship original game assets. You must own legal copies of the original games.

## Why Firestaff exists

Dungeon Master deserves more than a quick compatibility wrapper. Firestaff aims to make the original engine live again as a maintainable, portable codebase:

- real game data, not approximations
- deterministic systems that can be verified pass by pass
- original quirks preserved when they are part of the game
- clean boundaries between launcher, engine, assets, rendering, and future enhanced modes
- honest status reporting: supported means verified, not guessed

The goal is museum-grade preservation with a practical path toward modern builds.

## Current status

Firestaff is currently an early preview. The DM1 path is the priority and already includes a real in-game slice backed by original data. CSB and DM2 are being source-locked and integrated carefully rather than claimed early.

Working today:

- cross-platform launcher for DM1, CSB, and DM2 entries
- modern startup menu with persistent settings
- original-data detection with checksum-backed status indicators
- real DM1 dungeon loading from `DUNGEON.DAT`
- live movement, facing, ticking, melee, item handling, spells, stairs, pits, teleporters, survival systems, save/load, and champion state slices
- increasingly source-faithful DM1 viewport, HUD, inventory, and interaction routing
- asset-backed rendering from original `GRAPHICS.DAT` data
- deterministic verification gates for engine behaviour and parity claims
- opt-in V2 asset/catalog pipeline for enhanced 2D presentation

Still in progress:

- full DM1 V1 visual and timing parity
- complete CSB runtime integration
- complete DM2 runtime integration
- final audio path and presentation polish
- final release packaging across all supported platforms

## Preview art

Firestaff includes finished launcher creature cards for the current preview set.

<p align="center">
  <img src="assets/cards/creatures/red-dragon.png" alt="Red Dragon" width="180">
  <img src="assets/cards/creatures/skeleton.png" alt="Skeleton" width="180">
  <img src="assets/cards/creatures/stone-golem.png" alt="Stone Golem" width="180">
  <img src="assets/cards/creatures/giant-scorpion.png" alt="Giant Scorpion" width="180">
  <img src="assets/cards/creatures/mummy.png" alt="Mummy" width="180">
</p>

Current finished launcher cards:

- Red Dragon
- Skeleton
- Stone Golem
- Giant Scorpion
- Mummy

More creature cards are planned as the enhanced presentation grows.

## Running Firestaff

Put your legal original game files in the standard originals directory, then launch Firestaff and pick the game/version in the startup menu.

Default originals locations:

- macOS / Linux: `~/.firestaff/originals/`
- Windows: `<installation-directory>\originals`

A green status indicator means the selected version was found and matched. Missing or mismatched originals stay visible, but launch is blocked until the required files are present.

Typical local run:

```sh
./firestaff --data-dir "$HOME/.firestaff/data"
```

Basic in-game controls:

- `Enter` — inspect / confirm
- `Space` — act or wait
- `Tab` — cycle active champion
- `Esc` — return to launcher
- mouse / touch-style clicks — interact with viewport, party cards, and UI zones where supported

## Roadmap

### V1 — Original

The priority track. V1 is where Firestaff earns trust.

Focus areas:

- original-facing DM1 presentation and behaviour
- source-faithful viewport, HUD, inventory, spell, dialog, and endgame coverage
- precise handling of original bugs and platform/version differences
- CSB runtime support after DM1 parity gates are stable
- DM2 integration after its file/runtime boundaries are fully understood

### V2 — Enhanced 2D

A richer 2D presentation built on the verified V1 foundation.

Planned direction:

- high-resolution UI and art
- wide and 4K layouts
- cleaner readability without changing the game rules underneath
- shared logical asset catalog and manifest validation
- modernized presentation while keeping the Dungeon Master feel

### V3 — Modern / 3D

A later, freer reinterpretation once preservation is solid.

Planned direction:

- modern rendering
- 3D presentation experiments
- redesigns that are allowed to differ from the original because V1 remains the faithful baseline

## Design principles

- **Preserve first, enhance second**
- **Use real original data whenever possible**
- **Verify behaviour before claiming support**
- **Keep deterministic systems deterministic**
- **Treat original quirks as evidence, not inconvenience**
- **Build in small green slices**

## Credits

Firestaff's development has been informed by **Christophe Fontanel's** reverse-engineering work on [ReDMCSB](http://dmweb.free.fr/community/redmcsb/). His documentation of the original engine's bugs, quirks, and mechanics has been invaluable as a reference. No ReDMCSB source code is included in Firestaff.

The original Dungeon Master and Chaos Strikes Back games were designed by **Doug Bell**, **Dennis Walker**, **Mike Newton**, **Andy Jaros**, and **Wayne Holder** at FTL Games.

## Licence

Firestaff is released under the **MIT Licence**. See [LICENSE](LICENSE).

This licence covers only the Firestaff engine code. Dungeon Master and Chaos Strikes Back are © FTL Games / Software Heaven, Inc. No original assets are distributed with this project.

## Support development

If you want to help fund the preservation work, parity passes, packaging, and release effort behind Firestaff, you can sponsor the project here:

- GitHub Sponsors: **https://github.com/sponsors/yeager**

## Contributing

Issues and discussion are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md) for repository policy and contribution guidance.

## Links

- Dungeon Master Encyclopaedia: [dmweb.free.fr](http://dmweb.free.fr/)
- ReDMCSB reference project: [dmweb.free.fr/community/redmcsb/](http://dmweb.free.fr/community/redmcsb/)
- Project tagline: *An open Dungeon Master engine — deterministic, modular, museum-grade*
