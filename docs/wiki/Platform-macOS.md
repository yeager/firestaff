# Platform: macOS

## System Requirements

- macOS 12 Monterey or later
- Apple Silicon (arm64) or Intel (x86_64) processor
- SDL3 (bundled in release packages)

## Install from Release

Two formats are provided for each architecture:

| File | Format |
|------|--------|
| `Firestaff-x.y.z-macos-arm64.dmg` | Disk image (Apple Silicon) |
| `Firestaff-x.y.z-macos-arm64.zip` | Portable archive (Apple Silicon) |
| `Firestaff-x.y.z-macos-x86_64.dmg` | Disk image (Intel) |
| `Firestaff-x.y.z-macos-x86_64.zip` | Portable archive (Intel) |

### DMG Installation

1. Download the `.dmg` for your architecture.
2. Open the DMG and drag **Firestaff** to your Applications folder.
3. On first launch, macOS may show a Gatekeeper warning since the app is not notarized. Right-click the app and choose **Open**, then confirm.

### ZIP Installation

1. Extract the `.zip` archive.
2. Move the `Firestaff.app` bundle to any location (Applications recommended).
3. Double-click to launch. Same Gatekeeper note as above.

## Build from Source

```bash
brew install cmake ninja sdl3 pkg-config
cmake -S . -B build -DCMAKE_C_COMPILER=cc -G Ninja
ninja -C build
```

## Game Data Location

```
~/.firestaff/data/
```

Place original game files here, organized by game subdirectory. See [Game Data](Game-Data) for the full directory layout. The data directory can be changed in Settings within the launcher.

## Bundled Tools

The macOS release includes three companion tools as standalone `.app` bundles inside the DMG:

- **Firestaff Artpack Studio** — create and edit V2.2 modern graphics artpacks
- **Firestaff Dungeon Studio** — dungeon viewer and editor
- **Firestaff Savegame Editor** — inspect and edit save files

## Troubleshooting

### Gatekeeper blocks the app

Right-click the app and select **Open**. macOS remembers the exception after the first launch.

### No audio output

Firestaff uses SDL3's audio backend. Ensure your default audio output device is configured in System Settings > Sound.

### Retina / HiDPI rendering

Firestaff renders at the game's native resolution (320×200 for DM1/CSB) and scales to your display. The V2.0 filtered and V2.1 upscaled presentation modes provide higher-quality scaling on Retina displays.
