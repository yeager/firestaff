# Platform: Linux

## System Requirements

- Linux kernel 5.4 or later
- x86_64 or arm64 processor
- X11 or Wayland display server
- OpenGL 2.1 or later
- SDL3 (bundled in packages or built from source)

## Install from Release

| File | Architecture | Format |
|------|-------------|--------|
| `firestaff_x.y.z_amd64.deb` | x86_64 | Debian/Ubuntu package |
| `firestaff-x.y.z.x86_64.rpm` | x86_64 | Fedora/RHEL package |
| `firestaff_x.y.z_arm64.deb` | arm64 | Debian/Ubuntu package (Raspberry Pi, etc.) |
| `firestaff-x.y.z.aarch64.rpm` | arm64 | Fedora/RHEL package |

### Debian / Ubuntu

```bash
sudo dpkg -i firestaff_x.y.z_amd64.deb
```

### Fedora / RHEL

```bash
sudo rpm -i firestaff-x.y.z.x86_64.rpm
```

## Steam Deck

Firestaff provides dedicated Steam Deck packages (x86_64 only):

| File | Format |
|------|--------|
| `Firestaff-x.y.z-steamdeck-x86_64.pkg.tar.zst` | Pacman package |
| `Firestaff-x.y.z-steamdeck-x86_64.AppImage` | AppImage (no install needed) |

### Pacman Package

```bash
sudo pacman -U Firestaff-x.y.z-steamdeck-x86_64.pkg.tar.zst
```

### AppImage

```bash
chmod +x Firestaff-x.y.z-steamdeck-x86_64.AppImage
./Firestaff-x.y.z-steamdeck-x86_64.AppImage
```

The AppImage is self-contained and runs without installation. Add it as a non-Steam game in Steam's desktop mode to launch from Game Mode.

## Build from Source

```bash
# Install build dependencies (Ubuntu/Debian)
sudo apt install build-essential cmake ninja-build pkg-config \
  libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev \
  libxfixes-dev libxss-dev libxtst-dev libwayland-dev \
  libxkbcommon-dev wayland-protocols

# Build SDL3 from source (not yet in distro repos)
git clone --depth 1 --branch release-3.2.14 https://github.com/libsdl-org/SDL.git /tmp/SDL3
cmake -S /tmp/SDL3 -B /tmp/SDL3-build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TESTS=OFF
cmake --build /tmp/SDL3-build --parallel
sudo cmake --install /tmp/SDL3-build
sudo ldconfig

# Build Firestaff
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build
```

## Game Data Location

```
~/.firestaff/data/
```

You can also set the `FIRESTAFF_DATA` environment variable to override. The data directory can be changed in Settings within the launcher. See [Game Data](Game-Data) for the full directory layout.

## Bundled Tools

The Linux release includes three companion tools:

- **firestaff_artpack_studio** — create and edit V2.2 modern graphics artpacks
- **firestaff_dungeon_studio** — dungeon viewer and editor
- **firestaff_savegame_editor** — inspect and edit save files

## Troubleshooting

### No video output on Wayland

Set `SDL_VIDEODRIVER=wayland` or `SDL_VIDEODRIVER=x11` to force a specific backend.

### No audio on PipeWire

SDL3 supports PipeWire natively. If audio doesn't work, ensure `pipewire` and `pipewire-pulse` are installed and running.

### Steam Deck controls in Game Mode

Firestaff maps the Steam Deck's physical controls through SDL3's gamepad API. For best results, configure controls in Steam's controller settings for the non-Steam game entry.
