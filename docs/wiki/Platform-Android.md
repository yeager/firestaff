# Platform: Android

## System Requirements

- Android 7.0 Nougat (API 24) or later
- arm64-v8a processor (virtually all Android devices from 2017 onward)
- OpenGL ES 2.0 support

## Install from Release

Firestaff is distributed as a debug-signed `.apk` file. It is not available on the Google Play Store.

### Step 1: Enable sideloading

Android requires explicit permission to install apps from sources other than the Play Store.

**Android 8.0+ (Oreo and later):**

1. Open **Settings** > **Apps** > **Special app access** > **Install unknown apps**.
2. Select the app you'll use to open the APK (e.g., Chrome, Files, or a file manager).
3. Toggle **Allow from this source**.

**Android 7.x (Nougat):**

1. Open **Settings** > **Security**.
2. Enable **Unknown sources**.

### Step 2: Install the APK

1. Download `Firestaff-x.y.z-android-arm64.apk` from the [GitHub Releases](https://github.com/yeager/firestaff/releases) page.
2. Open the downloaded file. Android shows an installation prompt.
3. Tap **Install**.
4. Tap **Open** to launch, or find Firestaff in your app drawer.

### Updating

Download the new `.apk` and install it over the existing version. Your game data and saves are preserved. If installation fails due to a signature mismatch (different debug key), uninstall the old version first — this removes saves, so back up the data directory first.

## Game Data Transfer

Firestaff looks for game data at:

```
/sdcard/Documents/Firestaff/data/
```

This is the device's shared Documents folder, accessible from any file manager.

### Using a File Manager

1. Connect your Android device to your computer via USB. Select **File Transfer** (MTP) mode on the device.
2. On your computer, navigate to the device's **Documents** folder.
3. Create `Firestaff/data/` if it doesn't exist (Firestaff creates it on first launch).
4. Copy your game data files into the appropriate subdirectories (`dm1/data/`, `csb/data/`, etc.).

### Using a File Manager App on the Device

1. Download game data files to your device (e.g., via a cloud storage app or web download).
2. Use any file manager app (Files by Google, Solid Explorer, etc.).
3. Move the files to `Documents/Firestaff/data/` with the correct subdirectory structure.

### Using ADB (Android Debug Bridge)

```bash
adb push local-game-data/ /sdcard/Documents/Firestaff/data/
```

## Display and Input

- **Orientation**: landscape only.
- **Touch input**: tapping the screen maps to mouse clicks through SDL3's touch backend.
- **Game controllers**: Bluetooth and USB controllers are supported via SDL3's gamepad API.
- **Presentation modes**: V1 Original (320×200 pixel-perfect), V2.0 Filtered, V2.1 Upscaled are all available.

## Permissions

Firestaff requests no special Android permissions. Game data is accessed from the app's standard external storage area (`Documents/Firestaff/data/`), which is accessible without storage permissions on Android 10+ (scoped storage). On Android 7–9, the app accesses the shared `Documents` folder via the standard storage path.

## Troubleshooting

### "App not installed" error

- Ensure your device has an arm64-v8a processor. 32-bit (armeabi-v7a) devices are not supported.
- If updating, the new APK's debug signing key may differ from the installed version. Uninstall the old version first (back up your data directory).
- Check available storage space.

### No game data found popup

Copy your original game files to `/sdcard/Documents/Firestaff/data/`. See the [Game Data](Game-Data) page for the required files and directory structure.

### Black screen or crash on launch

- Ensure your device supports OpenGL ES 2.0 (Settings > About Phone > OpenGL ES version, or check via a GPU info app).
- Try force-stopping and relaunching the app.
- On some devices, battery optimization can kill background SDL3 threads. Exclude Firestaff from battery optimization in Settings > Apps > Firestaff > Battery.

### Touch controls feel unresponsive

Firestaff's touch input maps directly to SDL3's touch API. The original games were designed for mouse input, so precise tapping on small UI elements (especially inventory slots) may require practice. Using a Bluetooth mouse is an alternative.
