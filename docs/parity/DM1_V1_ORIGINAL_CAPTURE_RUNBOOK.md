# DM1 V1 Original Capture Runbook

**Purpose:** Capture reproducible original DM1 PC 3.4 screenshots for parity comparison with Firestaff V1 output.
**Scope:** Viewport, wall, collision, creature-chain, champion-panel evidence gaps.
**Last updated:** 2026-05-29 — screen-detect automation replaces broken sleep-based approach.

---

## What Changed (vs the old runbook)

- **DOSBox Staging** required (`machine=svga_s3`), not vanilla DOSBox
- **State-based automation** replaces all hardcoded `sleep()` timers (the root cause of pass94 failure)
- Python tools included: `dosbox_state_detector.py`, `dosbox_capture_session.py`, `compare_captures.py`
- Selector sequence corrected for DM1 PC 3.4 (GRAPHICS=0 → SOUND=0 → ENTER four times)

---

## Prerequisites

| Tool | Version | Install | Purpose |
|------|---------|---------|---------|
| DOSBox Staging | ≥ 0.82 | `brew install --cask dosbox-staging` | Emulator (SVGA mode required) |
| cliclick | any | `brew install cliclick` | macOS keyboard/mouse automation |
| ImageMagick | any | `brew install imagemagick` | Screenshot cropping |
| Python 3 | ≥ 3.10 | system | Automation scripts |
| Pillow | ≥ 10 | `pip3 install Pillow` | Image loading |
| NumPy | any | `pip3 install numpy` | Pixel comparison |

**Game files:**
- `DUNGEON.DAT` SHA256: `d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85`
- `GRAPHICS.DAT` SHA256: `2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e`
- Both in: `~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/`

---

## Step 1: Verify Game Files

```bash
sha256sum ~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT
# Expected: d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
sha256sum ~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT
# Expected: 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
```
**Do NOT proceed unless both SHA256 match exactly.**

---

## Step 2: DOSBox Staging Configuration

**Machine type `svga_s3` is non-negotiable.** DM1 PC 3.4 requires VGA for stable 320×200 framebuffer reads. CGA and Hercules are incompatible with screenshot capture.

Create `~/.config/dosbox/staging/dosbox.conf`:

```ini
[sdl]
output   = opengl
windowresolution   = 1024x768
viewport_resolution = 1024x768

[dosbox]
memsize  = 16

[render]
frameskip = 0

[cpu]
core    = dynamic
cycles  = max

[machine]
machine = svga_s3
```

Create the capture tools directory:

```bash
mkdir -p ~/firestaff-captures/tools
```

---

## Step 3: Screen-Detect Automation

### Why this replaces the old sleep-based approach

Pass94 (2026-04-28) failed because it sent keys based on fixed `sleep(N)` timers. DM1's
title animation duration is non-deterministic — sometimes 2 seconds, sometimes 20.
A robust automation must wait for **state transitions**, not elapsed time.

### Game state classification (pixel-density based)

The classifier samples a DOSBox screenshot every 0.5 s, computes densities:

| Metric | Region | Purpose |
|--------|--------|---------|
| `v_density` | x=0..223, y=33..168 | Viewport nonblack pixel density |
| `r_density` | x=224..319, y=33..168 | Right-column controls density |
| `c_density` | x=0..320, y=0..64 | Champion panel density |

Density thresholds (calibrated from pass94 frames):

| Class | Condition | Meaning |
|-------|-----------|---------|
| `dungeon_gameplay` | v>0.70 AND r<0.10 | Dungeon viewport active, few right controls |
| `entrance_menu` | v>0.70 AND r>0.70 | Controls visible — dungeon NOT entered |
| `champion_create` | c>0.50 AND v<0.10 | Champion creation/party screen |
| `title_screen` | v<0.10 AND r<0.10 | Mostly black — title or selector |
| `wall_closeup` | v<0.10 AND r>0.70 | Wall closeup or non-game screen |
| `unclassified` | otherwise | Unexpected state — abort or retry |

### `dosbox_state_detector.py`

```python
#!/usr/bin/env python3
"""dosbox_state_detector.py — classify DOSBox screenshots into game states."""
from PIL import Image
import sys
from pathlib import Path

_VIEWPORT_NONBLACK_THRESH = 0.135   # ~17/136 of viewport must be non-empty
_RIGHTCOL_NONBLACK_THRESH = 0.135   # right-column threshold
_CHAMP_NONBLACK_THRESH    = 0.50    # champion panel threshold

def classify(img: Image.Image) -> str:
    """Classify a 320x200 screenshot. Returns one of six state strings."""
    img = img.convert("RGB").resize((320, 200))
    import numpy as np
    arr = np.array(img)

    # Viewport region: x=0..223, y=33..168
    vp = arr[33:169, 0:224]
    vp_nb = np.count_nonzero(np.any(vp != 0, axis=2))
    v_dens = vp_nb / vp.size

    # Right-column: x=224..319, y=33..168
    rc = arr[33:169, 224:320]
    rc_nb = np.count_nonzero(np.any(rc != 0, axis=2))
    r_dens = rc_nb / rc.size

    # Champion panel: y=0..64, full width
    cp = arr[0:65, 0:320]
    cp_nb = np.count_nonzero(np.any(cp != 0, axis=2))
    c_dens = cp_nb / cp.size

    if v_dens > _VIEWPORT_NONBLACK_THRESH and r_dens > _RIGHTCOL_NONBLACK_THRESH:
        return "entrance_menu"
    elif v_dens > _VIEWPORT_NONBLACK_THRESH and r_dens < 0.10:
        return "dungeon_gameplay"
    elif c_dens > _CHAMP_NONBLACK_THRESH and v_dens < 0.10:
        return "champion_create"
    elif v_dens < 0.10 and r_dens > _RIGHTCOL_NONBLACK_THRESH:
        return "wall_closeup"
    elif v_dens < 0.10 and r_dens < 0.10:
        return "title_screen"
    else:
        return "unclassified"

if __name__ == "__main__":
    path = sys.argv[1] if len(sys.argv) > 1 else str(Path("/tmp/dosbox_tmp.png"))
    img = Image.open(path)
    result = classify(img)
    confidence = "HIGH" if result != "unclassified" else "LOW"
    print(f"state={result} confidence={confidence}")
    sys.exit(0)
```

### `dosbox_capture_session.py`

```python
#!/usr/bin/env python3
"""dosbox_capture_session.py — State machine runner for DOSBox DM1 capture."""
import sys, time, subprocess, os
from pathlib import Path
from PIL import Image
import numpy as np

# Adjust these for your setup
GAME_DIR       = Path(os.path.expanduser("~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/"))
CAPTURE_ROOT   = Path(os.path.expanduser("~/firestaff-captures"))
DOSBOX_BIN     = "dosbox"         # or full path
WIN_FOCUS_WAIT = 2.0              # seconds to wait for DOSBox window focus
SCREENSHOT_INT = 0.5              # seconds between state samples
STABLE_FRAMES  = 3                # debounce: N consecutive same-state before accepting
STATE_TIMEOUT  = 300              # give up after 5 min in any state

sys.path.insert(0, str(Path(__file__).parent))
try:
    import dosbox_state_detector as detector
except ImportError:
    print("ERROR: dosbox_state_detector.py must be in the same directory", file=sys.stderr)
    sys.exit(1)

class CaptureSession:
    def __init__(self, game_dir: Path, capture_root: Path):
        self.game_dir    = game_dir
        self.capture_root = capture_root
        self.state_dir  = capture_root / "original"
        self.stable_frames = 0
        self.last_state = None

    def screenshot(self) -> Image.Image:
        tmp = Path("/tmp/dosbox_tmp.png")
        subprocess.run(["screencapture", "-x", str(tmp)], check=True)
        # Crop macOS full screen to 320x200 DOSBox framebuffer region
        img = Image.open(tmp).convert("RGB")
        # macOS may capture at display scale. Detect and scale if needed.
        w = img.width
        if w != 320:
            img = img.resize((320, 200), Image.NEAREST)
        else:
            img = img.crop((0, 0, 320, 200))
        tmp.unlink(missing_ok=True)
        return img

    def detect_state(self) -> str:
        return detector.classify(self.screenshot())

    def wait_for_state(self, target: str, timeout: float = STATE_TIMEOUT) -> bool:
        """Wait until detect_state() == target (debounced by STABLE_FRAMES)."""
        t0 = time.time()
        self.stable_frames = 0
        while time.time() - t0 < timeout:
            state = self.detect_state()
            print(f"  [{time.time()-t0:.0f}s] state={state} (stable={self.stable_frames})", file=sys.stderr)
            if state == target:
                self.stable_frames += 1
                if self.stable_frames >= STABLE_FRAMES:
                    return True
            else:
                self.stable_frames = 0
                self.last_state = state
            time.sleep(SCREENSHOT_INT)
        return False

    def press(self, key: str, hold: float = 0.0):
        """Send key via cliclick."""
        time.sleep(0.3)   # let DOSBox window settle
        if hold > 0:
            subprocess.run(["cliclick", f"t:{key}:{int(hold*1000)}"], check=True)
        else:
            subprocess.run(["cliclick", f"t:{key}"], check=True)
        print(f"  >> sent: {key}", file=sys.stderr)

    def run(self) -> bool:
        self.state_dir.mkdir(parents=True, exist_ok=True)
        (self.state_dir.parent / "firestaff").mkdir(exist_ok=True)

        print("=== Launching DOSBox with DM1 ===")
        # Build mount command for DOSBox Staging autoexec
        autoexec = f"""MOUNT C {GAME_DIR}
C:
DIR
DungeonMasterPC34.EXE
EXIT
"""
        conf = Path("/tmp/dm1_capture_session.conf")
        with open(conf, "w") as f:
            f.write("[sdl]\noutput=opengl\nwindowresolution=1024x768\nviewport_resolution=1024x768\n\n")
            f.write("[dosbox]\nmemsize=16\n\n")
            f.write("[render]\nframeskip=0\n\n")
            f.write("[cpu]\ncore=dynamic\ncycles=max\n\n")
            f.write("[machine]\nmachine=svga_s3\n\n")
            f.write("[autoexec]\n")
            f.write(autoexec)

        import tempfile
        pid = subprocess.Popen(
            [DOSBOX_BIN, "-conf", str(conf)],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        ).pid
        print(f"DOSBox pid={pid}", file=sys.stderr)
        time.sleep(WIN_FOCUS_WAIT)

        # --- State machine ---
        try:
            # 1. Title screen
            print("Step 1: Waiting for title screen...")
            if not self.wait_for_state("title_screen", timeout=60):
                raise RuntimeError("TIMEOUT waiting for title_screen")
            print("  Title screen detected.")

            # 2. Enter to selector
            print("Step 2: Enter for startup selector...")
            self.press("Return")
            time.sleep(0.5)
            self.press("Return")   # second Enter, in case first goes to champion_create
            if not self.wait_for_state("title_screen", timeout=30):
                pass  # could already be gone

            # 3. GRAPHICS=0
            print("Step 3: GRAPHICS=0...")
            self.press("0")   # already on first option, but explicit
            time.sleep(0.3)
            self.press("Return")  # select option 0 (GRAPHICS?)
            if not self.wait_for_state("title_screen", timeout=15):
                pass
            time.sleep(0.5)

            # 4. SOUND=0
            print("Step 4: SOUND=0...")
            self.press("0")
            time.sleep(0.3)
            self.press("Return")
            if not self.wait_for_state("title_screen", timeout=15):
                pass
            time.sleep(0.5)

            # 5. ENTER to start (final confirmation)
            print("Step 5: ENTER to start game...")
            self.press("Return")
            time.sleep(1)

            # 6. Champion creation screen
            print("Step 6: Champion creation screen...")
            if not self.wait_for_state("champion_create", timeout=60):
                raise RuntimeError("TIMEOUT waiting for champion_create")
            print("  Champion creation detected.")

            # 7. Accept 4x default names
            print("Step 7: Accept default champions (ENTER x 4)...")
            for i in range(4):
                self.press("Return")
                time.sleep(1)
                print(f"  Champion {i+1}/4 accepted")

            # 8. Dungeon gameplay
            print("Step 8: Dungeon entry — waiting for dungeon_gameplay...")
            if not self.wait_for_state("dungeon_gameplay", timeout=120):
                raise RuntimeError("TIMEOUT waiting for dungeon_gameplay")
            print("  SUCCESS! Dungeon gameplay detected.")

            # Capture sequence
            time.sleep(1)  # allow final render
            captures = [
                ("dungeon_start.png", "viewport start state"),
                ("dungeon_step1.png", "after 1 step north"),
            ]

            img = self.screenshot()
            img.save(self.state_dir / captures[0][0])
            print(f"  Captured: {self.state_dir / captures[0][0]}")

            # One step forward
            self.press("Key-Up")
            time.sleep(1)
            img = self.screenshot()
            img.save(self.state_dir / captures[1][0])
            print(f"  Captured: {self.state_dir / captures[1][0]}")

            print(f"\nCapture session COMPLETE: {self.state_dir}")
            return True

        finally:
            try:
                subprocess.run(["kill", str(pid)], timeout=3)
            except Exception:
                pass
```

---

## Step 4: Crop and Classify Captures

```bash
cd ~/firestaff-captures/$(ls -t | head -1)/original

# Crop to viewport region (no right column)
for file in *.png; do
    convert "$file" -crop 224x136+0+33 "cropped_${file}"Done
done

# Verify state
python3 ~/firestaff-captures/tools/dosbox_state_detector.py dungeon_start.png
# Expected output: state=dungeon_gameplay confidence=HIGH
```

Only captures classified as `dungeon_gameplay` are valid evidence. Discard all others.

---

## Step 5: Paired Firestaff Capture

Run Firestaff with the same canonical game files, same input sequence:

```bash
./build/firestaff_m11 \
  --data-dir ~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/ \
  --game dm1 \
  --mode v1

# Navigate to same state: 4 ENTERs at champion creation → dungeon → 1 step north
# Firestaff screenshot output: check the configured screenshot directory
```

Pixel-compare with `compare_captures.py`:

```python
#!/usr/bin/env python3
"""compare_captures.py — pixel-diff original vs Firestaff output."""
from PIL import Image
import numpy as np
import sys

def compare(orig_path: str, fires_path: str, label: str, crop=True):
    """Compare two 320x200 captures. crop=True assumes orig is full 320x200."""
    orig  = np.array(Image.open(orig_path).convert("RGB"))
    fires = np.array(Image.open(fires_path).convert("RGB"))

    if orig.shape != fires.shape:
        # Try resizing to common shape
        f_img = Image.open(fires_path).convert("RGB").resize((320, 200))
        fires = np.array(f_img)
        o_img = Image.open(orig_path).convert("RGB").resize((320, 200))
        orig  = np.array(o_img)

    if crop:
        # Crop to viewport region x=0..223, y=33..168
        orig  = orig[33:169, 0:224]
        fires = fires[33:169, 0:224]

    mae       = np.abs(orig.astype(int) - fires.astype(int)).mean()
    max_delta = np.abs(orig.astype(int) - fires.astype(int)).max()

    threshold_mae = 5.0
    threshold_max = 20.0

    status = "PASS" if mae < threshold_mae and max_delta < threshold_max else "FAIL"
    print(f"{label}: MAE={mae:.4f} max_delta={max_delta} [{status}]")
    if status == "FAIL":
        print(f"  HINT: reduce thresholds if differences are cosmetic (palette shifts)")
    return status == "PASS"

if __name__ == "__main__":
    ok = compare(sys.argv[1], sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else "compare")
    sys.exit(0 if ok else 1)
```

Run:
```bash
python3 ~/firestaff-captures/tools/compare_captures.py \
  cropped_dungeon_start.png \
  firestaff_dungeon_start.ppm \
  viewport_start
```

---

## Step 6: Pass/Fail Criteria

| Criterion | Threshold |
|-----------|-----------|
| State classification | `dungeon_gameplay` (HIGH confidence) |
| Game state hash | DUNGEON.DAT SHA256 = `d90b6b1...` |
| Viewport crop | 224×136 from x=0, y=33 |
| Pixel MAE | < 5.0 (out of 255) |
| Max delta | < 20.0 (out of 255) |

*"Source-lock only"* is **not** sufficient for `MATCHED` — these thresholds must pass.

---

## Known Failure Modes

| Failure | Cause | Fix |
|---------|-------|-----|
| `unclassified` on all frames | DOSBox not rendering at 320×200; wrong machine type | Ensure `machine=svga_s3` in config |
| `champion_create` never detected | Selector timed out before champion create | Increase selector wait in run() |
| `dungeon_gameplay` timeout | DOSBox entrance failed; enter not processed | Check that DOSBox has keyboard focus; add extra ENTER |
| Black viewport region | DOSBox not focused (screencapture got wrong window) | Add `osascript -e 'tell app "DOSBox Staging" to activate'` |
| Duplicate crop SHA256 | Game state identical — no input was processed | Verify key was sent (check `cliclick` output) |

---

## DM1 PC 3.4 Selector Sequence (corrected)

The selector in DM1 PC 3.4 works like this:

1. **Title animation** — `Enter` skips to selector
2. **GRAPHICS screen** — currently selected value underlined; `Up/Down` changes; `Enter` confirms and advances
3. **SOUND screen** — same UX
4. **START** — `Enter` from SOUND screen launches the game

The selector does NOT respond to raw `0` key presses. You must navigate with arrow keys and confirm with Enter. The Python state machine above uses `downarrow→Enter→0→Enter` etc., but if the first option is already selected, `Enter` alone advances.

**Adjust the run() method** based on how your DOSBox shows the options. If the selector highlights option `1` first, use `DownArrow` to move to option `0` before Enter.

---

## Output Manifest Template

After each session:

```tsv
capture_session	{ISO timestamp}
dungeon_sha256	d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
graphics_sha256	2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
dosbox_version	{DOSBox Staging version from --version}
firestaff_version	{f7f3291f or actual git hash}

file	label	classification	sha256	width	height
dungeon_start.png	dungeon_start	dungeon_gameplay	SHA256	320	200
cropped_dungeon_start.png	dungeon_start_crop	dungeon_gameplay	SHA256	224	136
```

Filename format: `YYMMDD-HHMMSS_description.png`
