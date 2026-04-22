# Firestaff 0.1.0, macOS Preview

Firestaff 0.1.0 is the first public macOS preview of the project.

This is **not** a full DM1 parity release yet, and it is **not** a complete CSB / DM2 release. It is an early, honest preview of the current engine and launcher work.

## What this release is

- a macOS preview build
- a real launcher with game selection and startup settings
- a real DM1 engine slice backed by original data
- a verified engine core with deterministic probes still green
- an early product milestone that people can try, inspect, and give feedback on

## What works today

- launcher with DM1 / CSB / DM2 entries
- persistent startup settings
- startup-menu language groundwork
- DM1 asset validation via MD5
- real dungeon loading from `DUNGEON.DAT`
- real movement, facing, ticking, and interaction flow
- melee attack, item, spell, stairs, pits, teleporters, XP, save/load, and survival slices wired through the game core
- increasingly original-data-backed viewport rendering using `GRAPHICS.DAT`
- deterministic verification suite

## What this release is not

- not full DM1/V1 parity yet
- not complete CSB runtime support
- not complete DM2 runtime support
- not a final cross-platform release
- not polished end to end

## Platform

This release is for **macOS**.

Linux and Windows remain active targets, but this release should be presented as a macOS preview first.

## Original game data

Firestaff does **not** ship original assets.
You must provide your own legal original game files.

Recommended original-data search path:
- macOS/Linux: `~/.firestaff/originals/`
- Windows: `<install dir>\\originals`

Runtime data path in current builds may still also use:
- `~/.firestaff/data/`

## Recommended wording for GitHub release title

**Firestaff 0.1.0, macOS Preview**

## Recommended short release subtitle

**Early macOS preview of the Firestaff engine and launcher, with a real DM1 gameplay slice and verified core systems.**

## Recommended GitHub release body

Firestaff 0.1.0 is the first public macOS preview.

This build includes a real launcher, persistent startup settings, and a playable DM1 engine slice backed by original game data. The engine core is increasingly driven by real `GRAPHICS.DAT` and `DUNGEON.DAT` content, and the deterministic verification suite remains green.

This is still a preview release. DM1 original-presentation parity is not finished yet, and CSB / DM2 are not yet complete runtime targets. If you try this build, treat it as an early milestone: usable, inspectable, and intentionally honest about what is and is not done.

### Highlights
- launcher with game selection
- startup settings and language groundwork
- real DM1 dungeon loading
- movement, combat, items, spells, stairs, pits, teleporters, XP, save/load, and survival slices
- growing original-data-backed viewport rendering
- deterministic verification still green

### Not finished yet
- full DM1/V1 parity
- full CSB support
- full DM2 support
- final product polish

### Notes
- macOS preview release
- original retail game data required
- no original assets are distributed with Firestaff

## Recommended README status wording

If you want to align the README with this release, use language like:

> Firestaff is now available as an early **macOS preview** release, with a real launcher, a playable DM1 engine slice, and a growing verified core. It is not yet a full DM1 parity release, and CSB / DM2 runtime support is still in progress.
