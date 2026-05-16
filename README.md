# Firestaff — Dungeon Master Collection

A faithful recreation and modern enhancement of the classic Dungeon Master series by FTL Games. Play Dungeon Master, Chaos Strikes Back, and Dungeon Master II with original accuracy or modern features — in 20 languages, on macOS, Windows, and Linux.

## Games

| Game | V1 Original | V2.1 Upscaled | V2.2 Enhanced |
|---|---|---|---|
| **Dungeon Master** | ✅ 62 modules | ✅ EPX 2x/4x | ✅ 20 features |
| **Chaos Strikes Back** | ✅ 12 modules | ✅ Viewport + DSA | ✅ 4 features |
| **Dungeon Master II: Skullkeep** | ✅ 10 modules | ✅ Indoor + outdoor | ✅ 4 features |
| **DM Nexus** | 🔜 Planned | — | — |

## Versions

### V1 — Original Faithful
Pixel-perfect recreation at native 320x200. All game logic source-locked to ReDMCSB with 100% function coverage across 40 source files. Original VGA 16-color palette. V1 tick rate preserved at 55ms / 18.2 Hz.

### V2.1 — Upscaled
Same game logic as V1, rendered at higher resolution using EPX/Scale2x — an edge-preserving pixel art scaler. 640x400 or 1280x800 output. No blurry bilinear.

### V2.2 — Enhanced
V1 game logic with visual and quality-of-life enhancements:

- **Smooth movement** — interpolated walk/turn/stairs with ease-out easing
- **Dynamic lighting** — per-tile propagation with torch flicker
- **Particle effects** — torch, spell, blood, weather
- **Camera shake** — trauma-based on damage/explosions
- **Minimap** — explored dungeon overview
- **Floating damage numbers** — combat feedback
- **Weather FX** — rain, fog, embers, drip per zone
- **Journal** — exploration/event log
- **Message log** — scrollable combat feed
- **Achievements, auto-save, screenshot, input remap, inventory sort, tooltips**

All animations use unified timing: exact V1 game speed, smooth visuals at display rate.

## Rendering Pipeline

Real bitmaps from original game files:

```
GRAPHICS.DAT -> parse 713 graphics
  -> 4bpp decompress -> DM1 VGA palette -> RGBA
  -> EPX upscale -> SDL3 present

DUNGEON.DAT -> square types per level
  -> view cone -> wall bitmap selection -> viewport blit
```

## 20 Languages

Auto-detects system language. Full UI for all 20:

| | | | | |
|---|---|---|---|---|
| English | Svenska | Deutsch | Français | Español |
| Italiano | Português | Nederlands | Polski | Čeština |
| Русский | 日本語 | 한국어 | 简体中文 | Dansk |
| Norsk | Suomi | Magyar | Türkçe | |

Swedish game text is a Firestaff original — includes all skill names, directions, items, actions, and dungeon inscriptions with correct ÅÄÖ.

## CLI

```bash
firestaff dm1 --v21 --fullscreen --lang sv
firestaff csb --v1 --scale 4
firestaff dm2 --v22 --load 3
firestaff --list-saves
```

## Start Menu

```
Main Menu
  > Play -> Game Select -> Game Mode
  > Settings -> Display | Video | Audio | Controls | Accessibility
  > Extras -> Museum | Bestiary | Spells | Maps | Items | Changelog | Gallery
  > Quit
```

54 settings across 5 tabs. Unavailable features shown grayed. Persists to INI.

## Extras

- **Bestiary** — 24 creatures with full stats
- **Spell Reference** — 14 spells with symbol sequences and mana costs
- **Map Viewer** — dungeon overview with fog-of-war
- **Item Encyclopedia** — 33 items across 7 categories
- **Screenshot Gallery** — browse captures

## CSB: Chaos Strikes Back

DSA bytecode interpreter for programmable puzzles. 16 opcodes, 256 flags, per-script stack. DM1 champion import from save files.

## DM2: Skullkeep

Outdoor exploration with day/night cycle and weather. Tech/magic hybrid crafting. NPC companions with loyalty and AI.

## Project Structure

```
src/
  dm1/       62 files    dm1v2/    36 files
  csb/       12 files    dm2/      10 files
  engine/    10 files    memory/   82 files
  shared/    80 files    ui/       12 files
  frontend/  18 files
tests/       304 tests
include/     310 headers
probes/      88 probes
```

927 source files, 188K lines of C.

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
ctest
```

CMake 3.20+, C99 compiler. SDL3 optional.

## Source References

- **ReDMCSB** — DM1/CSB reverse-engineered C source, 100% function parity
- **CSBWin** — BeipDev CSB Windows port
- **SKULL.ASM** — DM2 DOS disassembly, 522K lines
- **Greatstone** — DM1 PC-34 atlas

## Downloads

[GitHub Releases](https://github.com/yeager/firestaff/releases) — macOS DMG/zip, Windows zip/installer, Linux DEB/RPM.

## License

See [LICENSE](LICENSE).
