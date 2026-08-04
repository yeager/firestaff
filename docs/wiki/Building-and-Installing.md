# Building and Installing

## Build from Source

### Requirements

- CMake 3.20+
- Ninja build system
- System C compiler (clang on macOS, gcc on Linux)
- SDL3 (release-3.2.14 or later)
- pkg-config

### macOS

```bash
brew install cmake ninja sdl3 pkg-config
cmake -S . -B build -DCMAKE_C_COMPILER=cc -G Ninja
ninja -C build
```

### Linux (Ubuntu/Debian)

```bash
sudo apt install build-essential cmake ninja-build pkg-config \
  libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev \
  libxfixes-dev libxss-dev libxtst-dev libwayland-dev \
  libxkbcommon-dev wayland-protocols

# SDL3 must be built from source (not yet in distro repos)
git clone --depth 1 --branch release-3.2.14 https://github.com/libsdl-org/SDL.git /tmp/SDL3
cmake -S /tmp/SDL3 -B /tmp/SDL3-build -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TESTS=OFF
cmake --build /tmp/SDL3-build --parallel
sudo cmake --install /tmp/SDL3-build
sudo ldconfig

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build
```

### Windows (MSYS2 UCRT64)

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-sdl3
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target firestaff --parallel
```

### Steam Deck

Use the Linux x86_64 packages from the release. Both a pacman `.pkg.tar.zst` and an AppImage are provided:

```bash
# pacman package
sudo pacman -U Firestaff-x.y.z-steamdeck-x86_64.pkg.tar.zst

# or AppImage
chmod +x Firestaff-x.y.z-steamdeck-x86_64.AppImage
./Firestaff-x.y.z-steamdeck-x86_64.AppImage
```

## Install from Release Packages

### macOS

Download the `.dmg` for your architecture (arm64 for Apple Silicon, x86_64 for Intel). Open the DMG and drag Firestaff to Applications. A `.zip` archive is also provided.

### Windows

Download and run the Inno Setup installer (`-windows-installer.exe`), or extract the portable `.zip`.

### Linux

```bash
# Debian/Ubuntu
sudo dpkg -i firestaff_x.y.z_amd64.deb

# Fedora/RHEL
sudo rpm -i firestaff-x.y.z.x86_64.rpm
```

ARM64 `.deb` and `.rpm` packages are also available.

### iOS (AltStore Classic Sideload)

The `.ipa` is ad-hoc signed and designed for sideloading via [AltStore Classic](https://altstore.io). No Apple Developer account is required.

1. Install AltStore Classic on your Mac or Windows PC following the [official guide](https://faq.altstore.io/getting-started/how-to-install-altstore).
2. Connect your iPhone/iPad via USB.
3. In AltStore, tap the `+` button and select the downloaded `Firestaff-x.y.z-ios-arm64.ipa`.
4. AltStore will install the app on your device.

**Requirements:**
- iOS 14.0 or later
- iPhone or iPad with arm64 processor
- AltStore Classic installed and configured
- The app must be refreshed every 7 days via AltStore (Apple limitation for free provisioning)

**Limitations:**
- Landscape orientation only (iPhone); all orientations supported on iPad
- Game data files must be supplied separately (see [Game Data](Game-Data))
- Touch input maps to the SDL3 touch backend

### Android

Download the `.apk` and install it directly:

1. On your Android device, enable "Install from unknown sources" in Settings > Security (or Settings > Apps > Special app access > Install unknown apps).
2. Transfer the `.apk` to your device or download it directly.
3. Tap the `.apk` file to install.

**Requirements:**
- Android 7.0 (API 24) or later
- arm64-v8a processor (virtually all modern Android devices)
- OpenGL ES 2.0 support

**Limitations:**
- Landscape orientation only
- Game data files must be supplied separately (see [Game Data](Game-Data))
- Debug-signed (sideload only, not from Google Play)

## Running Tests

```bash
ctest --test-dir build -j4
```

Run a subset:

```bash
ctest --test-dir build -R "viewport" -j4 --output-on-failure
```

There are approximately 3490 tests. Some viewport and boot tests require original game data files and will fail or timeout without them.
