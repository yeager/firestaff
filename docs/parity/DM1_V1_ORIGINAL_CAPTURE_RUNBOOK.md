# DM1 V1 Original Capture Runbook

**Purpose:** Capture reproducible original DM1 PC 3.4 screenshots/transcripts that are
paired with Firestaff V1 output for pixel/parity verification.

**Scope:** Viewport, wall, collision, creature-chain, champion-panel cases.

**Prerequisites:**
- Original DM1 PC 3.4 English game files:
  - `DUNGEON.DAT` SHA256: `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
  - `GRAPHICS.DAT` SHA256: `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`
  - Both at: `~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/`
- DOSBox Staging (or DOSBox-X)
- `cliclick` (for macOS automated input): `brew install cliclick`
- ImageMagick (for cropping): `brew install imagemagick`
- Python 3 with `Pillow` (for pixel comparison)

---

## Step 1: Verify Canonical Game Files

```bash
# Verify DUNGEON.DAT
sha256sum ~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT
# Expected: d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85

# Verify GRAPHICS.DAT
sha256sum ~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT
# Expected: 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
```

Do NOT proceed unless both SHA256 values match exactly.

---

## Step 2: DOSBox Configuration

Create `dm1_original_capture.conf`:

```ini
[sdl]
output = opengl
windowresolution = 1024x768
viewport_resolution = 1024x768

[dosbox]
memsize = 16

[render]
frameskip = 0

[cpu]
core = dynamic
cycles = max
```

Mount the game directory. The canonical DM1 PC 3.4 is a 7z archive. Extract it first:

```bash
cd ~/.openclaw/data/firestaff-original-games/DM/
7z x -oDM1_pc34 "Game,Dungeon_Master,DOS,Software.7z"
# Or unzip the DOS_EN zip
unzip -o "Dungeon-Master_DOS_EN.zip" -d DM1_pc34
```

---

## Step 3: Input Sequence to Dungeon

The goal is to reach the first dungeon gameplay frame deterministically.

**Key insight from pass94 failure analysis:**
- Pass94 (2026-04-28) failed because the DOSBox input automation triggered
  `entrance_menu` and `wall_closeup` states instead of `dungeon_gameplay`.
- The Enter key sequence at the title/selector is unreliable for automated capture
  because the game blocks waiting for a keypress with no deterministic timeout.

**Reliable dungeon entry sequence:**

```bash
# 1. Start DOSBox with the game mounted
dosbox -conf dm1_original_capture.conf -c "MOUNT C ~/.openclaw/data/firestaff-original-games/DM/DM1_pc34" \
  -c "C:" -c "DIR" -c "DungeonMasterPC34.EXE" -c "EXIT"

# 2. AUTOMATED INPUT (macOS with cliclick):
# Wait for DOSBox window to be focused
sleep 2

# Title screen: press Enter to reach selector
cliclick t:Enter
sleep 3

# Selector: choose GRAPHICS=0, SOUND=0 or appropriate, then ENTER
# (depends on which selector screen appears)
cliclick t:Enter
sleep 1
cliclick t:0   # GRAPHICS option
sleep 0.5
cliclick t:Enter
sleep 0.5
cliclick t:0   # SOUND option
sleep 0.5
cliclick t:Enter
sleep 4       # Wait for title animation to finish

# Title: hold Enter to skip animation or wait for it to end
# Then press Enter to start game
cliclick t:Enter
sleep 3

# Champion creation: press Enter 4x to accept default names
cliclick t:Enter
sleep 0.5
cliclick t:Enter
sleep 0.5
cliclick t:Enter
sleep 0.5
cliclick t:Enter
sleep 5       # Dungeon should now be visible
```

**Problem:** This sequence is fragile. The title animation wait time is non-deterministic.
Pass63 `dosbox_dm1_title_input_pass63.sh` achieved title->selector handoff but the
subsequent Enter-to-dungeon timing was unreliable.

**Alternative approach (keyboard-based state machine):**
Use a Python script that:
1. Launches DOSBox with `-noautoexec`
2. Sends keys via `xdotool` (Linux) or `cliclick` (macOS)
3. Waits for screen-content transitions (by sampling DOSBox screenshot every 0.5s)
4. Stops sending keys once the dungeon viewport is detected (non-black pixels in
   the viewport region at y=33)

---

## Step 4: Capture Protocol

Once dungeon gameplay is confirmed (viewport region y=33 shows non-black pixels):

```bash
# Set up capture directory
CAPTURE_DIR=~/firestaff-captures/$(date +%Y%m%d-%H%M%S)
mkdir -p $CAPTURE_DIR/original $CAPTURE_DIR/firestaff

# Capture raw 320x200 original frame
# In DOSBox console:
#   screenshot png original_000.png
# Or use native DOSBox screenshot (usually mapped to Ctrl+F5)
```

### Capture 1: Viewport Start State
```bash
# Party at canonical start: map=0, x=1, y=3, dir=SOUTH
# Capture 320x200 raw
screencapture -x $CAPTURE_DIR/original/01_viewport_start_320x200.png
# Crop to viewport region (x=0, y=33, w=224, h=136)
convert $CAPTURE_DIR/original/01_viewport_start_320x200.png \
  -crop 224x136+0+33 $CAPTURE_DIR/original/01_viewport_start_224x136.png
```

### Capture 2: After One Forward Step
```bash
# In DOSBox, press Up-Arrow (forward)
cliclick t:Up
sleep 1
screencapture -x $CAPTURE_DIR/original/02_after_step_320x200.png
convert $CAPTURE_DIR/original/02_after_step_320x200.png \
  -crop 224x136+0+33 $CAPTURE_DIR/original/02_after_step_224x136.png
```

### Capture 3: After Turn-Right
```bash
cliclick t:Right
sleep 1
screencapture -x $CAPTURE_DIR/original/03_after_turn_320x200.png
convert $CAPTURE_DIR/original/03_after_turn_320x200.png \
  -crop 224x136+0+33 $CAPTURE_DIR/original/03_after_turn_224x136.png
```

### Capture 4: Champion Panel
```bash
# In DOSBox, press 'C' for champion panel (or click champion portrait)
cliclick t:c
sleep 1
screencapture -x $CAPTURE_DIR/original/04_champion_panel_320x200.png
# Crop champion panel region (depends on game version; for DM1 PC 3.4: y=0, x=0, w=320, h=65)
convert $CAPTURE_DIR/original/04_champion_panel_320x200.png \
  -crop 320x65+0+0 $CAPTURE_DIR/original/04_champion_panel_crop.png
```

---

## Step 5: Semantic Classification Gate

Before accepting any capture as evidence, run it through the classification gate:

```python
#!/usr/bin/env python3
"""classify_capture.py - classify a DOSBox 320x200 capture."""
import sys
from PIL import Image

def classify_viewport(img_path):
    """Classify a 320x200 capture."""
    img = Image.open(img_path)
    px = img.load()
    w, h = img.size  # should be 320x200

    # Check viewport region (y=33..168, x=0..223)
    viewport_nonblack = sum(
        1 for y in range(33, 169) for x in range(0, 224)
        if px[x, y] != (0, 0, 0)
    )
    # Check right-column controls (y=33..168, x=224..319)
    right_col_nonblack = sum(
        1 for y in range(33, 169) for x in range(224, 320)
        if px[x, y] != (0, 0, 0)
    )

    total_pixels = 224 * 136
    viewport_density = viewport_nonblack / total_pixels
    right_density = right_col_nonblack / (96 * 136)

    print(f"viewport nonblack: {viewport_nonblack}/{total_pixels} = {viewport_density:.3f}")
    print(f"right_col nonblack: {right_col_nonblack}/{96*136} = {right_density:.3f}")

    if viewport_density > 0.7 and right_density > 0.7:
        return "entrance_menu"  # Controls visible - dungeon NOT entered
    elif viewport_density > 0.7 and right_density < 0.1:
        return "dungeon_gameplay"  # Dungeon viewport, no/few right-column controls
    elif viewport_density < 0.1 and right_density < 0.1:
        return "title_or_menu"
    elif viewport_density < 0.1 and right_density > 0.7:
        return "wall_closeup"
    else:
        return "unclassified"

if __name__ == "__main__":
    path = sys.argv[1] if len(sys.argv) > 1 else "capture.png"
    result = classify_viewport(path)
    print(f"classification: {result}")
    sys.exit(0 if result == "dungeon_gameplay" else 1)
```

**CRITICAL:** Only use captures classified as `dungeon_gameplay` for parity evidence.
Discard anything classified as `entrance_menu`, `wall_closeup`, `title_or_menu`, or `unclassified`.

---

## Step 6: Firestaff Pairing

Run Firestaff from the same canonical game files with the same input sequence:

```bash
# Firestaff command (example - adjust for actual binary path)
./build/firestaff_m11 \
  --data-dir ~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/ \
  --game dm1 \
  --mode v1 \
  -vv

# Capture Firestaff output at the same game states
# Firestaff screenshots saved to:
#   $FIRESTAFF_SCREENSHOT_DIR/*.ppm (320x200 VGA palette)
```

Then compare:
```python
#!/usr/bin/env python3
"""compare_captures.py - pixel compare original vs Firestaff output."""
from PIL import Image
import sys
import numpy as np

def compare(original_png, firestaff_ppm, label):
    orig = np.array(Image.open(original_png).convert("RGB"))
    fires = np.array(Image.open(firestaff_ppm).convert("RGB"))

    # Both should be 320x200
    assert orig.shape == fires.shape == (200, 320, 3), f"Shape mismatch: {orig.shape} vs {fires.shape}"

    # MAE over all pixels
    mae = np.abs(orig.astype(int) - fires.astype(int)).mean()
    max_delta = np.abs(orig.astype(int) - fires.astype(int)).max()

    print(f"{label}: MAE={mae:.4f}, max_delta={max_delta}")

    # Strict gate: MAE < 2.0 and max_delta < 8
    if mae < 2.0 and max_delta < 8:
        print(f"  -> PASS (parity confirmed)")
        return True
    else:
        print(f"  -> FAIL (gap > threshold)")
        return False

if __name__ == "__main__":
    ok = compare(sys.argv[1], sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else "compare")
    sys.exit(0 if ok else 1)
```

---

## Step 7: Pass/Fail Criteria

| Criterion | Threshold |
|-----------|-----------|
| Semantic classification | All original captures must be `dungeon_gameplay` |
| Game state hash | DUNGEON.DAT SHA256 must be `d90b6b1...` |
| Capture dimensions | 320x200 raw, 224x136 viewport crop |
| Pixel parity MAE | < 2.0 (out of 255) |
| Pixel parity max delta | < 8 (out of 255) |

If any capture fails classification, discard it and re-run from Step 3 with adjusted timing.

---

## Step 8: Known Failure Modes

| Failure | Cause | Fix |
|---------|-------|-----|
| `entrance_menu` captures | Enter key sent before selector appears | Increase sleep before first Enter; use screen-content detection |
| `wall_closeup` captures | Enter pressed during title animation instead of after | Wait for title->selector transition; confirm selector screen |
| Duplicate frame SHA256 | Game state did not advance between captures | Verify input was actually received (check game state transcript) |
| `unclassified` captures | Unexpected game state or screenshot timing | Verify DOSBox is rendering at 320x200 VGA |
| Black captures | DOSBox screenshot taken before rendering completes | Add 1-frame wait (0.5s) after each input |

---

## Step 9: Output Manifest

After a successful capture session, produce a manifest:

```tsv
capture_session\t{ISO timestamp}
dungeon_sha256\t{d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85}
graphics_sha256\t{2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e}
dosbox_version\t{DOSBox version}
firestaff_version\t{Firestaff version or git hash}
classifier_version\tclassify_capture.py v1

file\tlabel\tclassification\tsha256\tw\th
01_viewport_start_320x200.png\tviewport_start\tdungeon_gameplay\t{sha256}\t320\t200
01_viewport_start_224x136.png\tviewport_start_crop\tdungeon_gameplay\t{sha256}\t224\t136
...
```
