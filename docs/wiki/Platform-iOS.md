# Platform: iOS

## System Requirements

- iOS 14.0 or later
- iPhone or iPad with arm64 processor (iPhone 6s and later, all iPads with A9 or later)
- AltStore Classic for sideloading

## How Sideloading Works

Apple restricts app installation to the App Store by default. **Sideloading** means installing an app outside the App Store using an ad-hoc provisioning profile. Firestaff is distributed as a `.ipa` file (iOS App Package) that is ad-hoc signed, meaning it can be installed on any device through a sideloading tool — no Apple Developer account is required.

The main limitation of free sideloading is that apps expire after **7 days** and must be refreshed. AltStore Classic handles this automatically when your device is on the same Wi-Fi network as your computer.

## Install with AltStore Classic

[AltStore Classic](https://altstore.io) is the recommended sideloading tool. It runs a background helper on your Mac or Windows PC that re-signs the app every 7 days using your Apple ID's free provisioning profile.

### Step 1: Install AltStore Classic on your computer

**macOS:**

1. Download AltServer from [altstore.io](https://altstore.io).
2. Move AltServer to Applications and launch it.
3. AltServer appears as a diamond icon in the menu bar.
4. Sign in with your Apple ID when prompted.

**Windows:**

1. Download AltServer for Windows from [altstore.io](https://altstore.io).
2. Install and launch AltServer.
3. Install iCloud for Windows (from Apple, not the Microsoft Store version) if prompted.
4. Install iTunes for Windows (from Apple, not the Microsoft Store version) if prompted.
5. Sign in with your Apple ID when prompted.

### Step 2: Install AltStore on your device

1. Connect your iPhone/iPad to your computer via USB.
2. Trust the computer on your device if prompted.
3. On Mac: click the AltServer icon in the menu bar > **Install AltStore** > select your device.
4. On Windows: click the AltServer icon in the system tray > **Install AltStore** > select your device.
5. AltStore appears on your home screen. Open it and sign in with your Apple ID.

### Step 3: Install Firestaff

1. Download `Firestaff-x.y.z-ios-arm64.ipa` from the [GitHub Releases](https://github.com/yeager/firestaff/releases) page.
2. Transfer the `.ipa` file to your device (AirDrop, iCloud Drive, email attachment, or direct download in Safari).
3. Open the file with AltStore:
   - If downloaded in Safari: tap the file, then tap the share button, then **Open in AltStore**.
   - If in Files: long-press the `.ipa`, tap Share, then **AltStore**.
4. AltStore signs and installs the app. Firestaff appears on your home screen.

### Step 4: Keep the app alive (refresh every 7 days)

Apple's free provisioning profiles expire after 7 days. AltStore refreshes automatically if:

- Your iPhone/iPad is on the **same Wi-Fi network** as the computer running AltServer.
- AltServer is running on your computer.
- Background App Refresh is enabled for AltStore on your device.

If you miss the 7-day window, the app stops launching. Simply open AltStore while connected to your computer's network and it will re-sign automatically. No data is lost — your game saves persist through re-signing.

## Alternative Sideloading Methods

### SideStore (no computer required after setup)

[SideStore](https://sidestore.io) is a fork of AltStore that uses a WireGuard VPN to self-sign apps without a computer after initial setup. Follow SideStore's own documentation, then use it to install the `.ipa` the same way as AltStore.

### Apple Configurator 2 (Mac only)

1. Install Apple Configurator 2 from the Mac App Store.
2. Connect your device via USB.
3. Drag the `.ipa` onto your device in Apple Configurator 2.

This method does not handle re-signing. The app expires after 7 days unless you reinstall.

### Paid Apple Developer Account ($99/year)

With a paid developer account, apps are valid for **1 year** instead of 7 days, and you can install on up to 100 devices. Use Xcode or `ios-deploy` to install the `.ipa` with your developer certificate.

## Game Data Transfer

Firestaff stores game data in the app sandbox at:

```
On My iPhone (or iPad) > Firestaff > data/
```

### Using the Files App

1. Open the **Files** app on your device.
2. Navigate to **On My iPhone** (or **On My iPad**) > **Firestaff**.
3. Create a `data` folder if it doesn't exist (Firestaff creates it automatically on first launch).
4. Copy your game data files into the appropriate subdirectories (`dm1/data/`, `csb/data/`, etc.).

### Using iTunes / Finder File Sharing

1. Connect your device to your Mac via USB.
2. Open Finder (macOS Catalina+) or iTunes (older macOS / Windows).
3. Select your device, go to the **Files** tab.
4. Expand **Firestaff** and drag your game data files into the Documents folder.

### Using iCloud Drive

1. Place game data files in iCloud Drive on your computer.
2. On your device, open Files > iCloud Drive.
3. Copy the files to On My iPhone > Firestaff > data/.

## Display and Input

- **Orientation**: landscape only on iPhone. iPad supports all orientations.
- **Touch input**: tapping the screen maps to mouse clicks through SDL3's touch backend.
- **Game controllers**: MFi and Bluetooth controllers are supported via SDL3's gamepad API.
- **Presentation modes**: V1 Original (320×200 pixel-perfect), V2.0 Filtered, V2.1 Upscaled are all available.

## Troubleshooting

### "Untrusted Developer" alert

Go to Settings > General > VPN & Device Management, tap your Apple ID under Developer App, and tap **Trust**.

### App stops launching after 7 days

The free provisioning profile expired. Open AltStore while on the same Wi-Fi as your AltServer computer to refresh. Alternatively, reinstall the `.ipa`.

### App crashes on launch

Ensure you have iOS 14.0 or later. Check that Firestaff was signed correctly by AltStore (open AltStore > My Apps > verify Firestaff is listed and not expired).

### No game data found popup

Copy your original game files to the app's Documents folder via Files, iTunes, or iCloud Drive. See the [Game Data](Game-Data) page for the required files and directory structure.
