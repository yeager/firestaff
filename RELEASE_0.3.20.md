## Firestaff v0.3.20

### Startup menu
- Shows a startup popup when no original retail game files are found.
- The popup tells the user to copy their retail files into the detected Firestaff data directory before launching.


Startup menu feature controls binary release.

### Added
- New startup menu control groups for Renderer, Input, Data, and Debug.
- Viewport style selector: Original, Expanded, Widescreen.
- Input mode selector: Auto, Keyboard+Mouse, Touch, Gamepad.
- Touch controls selector: Off, Minimal, Full, Large.
- Movement mode selector: Original, Fast, Smooth.
- Original data status row.
- Debug overlay selector: Off, Coords, Queue, Draw Order.
- Developer gates selector: Off, Quick, Full.
- Persistent config keys for the new menu controls.

### Packaging
- macOS DMG + app zip.
- Linux DEB + RPM.
- Windows zip + NSIS installer.

### Verified
- macOS Release build + smoke probes.
- GitHub Actions package builds for Linux and Windows.
- Package checksums included per platform.
