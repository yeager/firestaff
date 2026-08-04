# Platform: Windows

## System Requirements

- Windows 10 or later (x86_64)
- OpenGL 2.1 or later (virtually all GPUs)
- SDL3 (bundled in release packages)

## Install from Release

| File | Format |
|------|--------|
| `Firestaff-x.y.z-windows-installer.exe` | Inno Setup installer |
| `Firestaff-x.y.z-windows.zip` | Portable archive |

### Installer

1. Download `Firestaff-x.y.z-windows-installer.exe`.
2. Run the installer. Windows SmartScreen may show a warning since the binary is not EV-signed — click **More info** > **Run anyway**.
3. Follow the wizard. Default install location is `C:\Program Files\Firestaff`.
4. Launch from the Start Menu or desktop shortcut.

### Portable ZIP

1. Extract `Firestaff-x.y.z-windows.zip` to any folder.
2. Run `firestaff.exe` directly.

## Build from Source (MSYS2 UCRT64)

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-sdl3
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --target firestaff --parallel
```

## Game Data Location

```
%USERPROFILE%\.firestaff\data\
```

This typically resolves to `C:\Users\YourName\.firestaff\data\`. Place original game files here, organized by game subdirectory. See [Game Data](Game-Data) for the full directory layout. The data directory can be changed in Settings within the launcher.

## Bundled Tools

The Windows release includes three companion tools:

- **firestaff_artpack_studio.exe** — create and edit V2.2 modern graphics artpacks
- **firestaff_dungeon_studio.exe** — dungeon viewer and editor
- **firestaff_savegame_editor.exe** — inspect and edit save files

## Troubleshooting

### SmartScreen blocks the installer

Click **More info** then **Run anyway**. This happens because the binary is not code-signed with an EV certificate.

### Missing DLLs

The installer and portable ZIP bundle all required DLLs including SDL3. If you built from source, ensure `SDL3.dll` is in the same directory as `firestaff.exe` or on your system PATH.

### Controller not detected

Firestaff uses SDL3's gamepad API. Connect your controller before launching. SDL3 supports Xbox, PlayStation, Switch Pro, and generic HID controllers.
