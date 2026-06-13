# DM1 V1 Original Capture Runbook

**Purpose:** Capture reproducible original DM1 PC 3.4 screenshots for parity comparison with Firestaff V1 output.
**Scope:** Viewport, wall, collision, creature-chain, champion-panel evidence gaps.
**Last updated:** 2026-05-29 — screen-detect automation replaces broken sleep-based approach.

---

## What Changed (vs the old runbook)

- **DOSBox Staging** required (`machine=svga_s3`), not vanilla DOSBox
- **State-based automation** replaces all hardcoded `sleep()` timers (the root cause of pass94 failure)
- Python tools included: `dosbox_state_detector.py`, `dosbox_capture_session.py`, `compare_captures.py`, `dosbox_capture_preflight.py`
- Preflight gate (`dosbox_capture_preflight.py`) verifies canonical SHA256s and writes the hardened `dosbox_capture.conf` with the runbook's required settings (machine=svga_s3, memsize=16, cpu core=dynamic, cpu_cycles=max).  Run it before any live attempt so the next session cannot reproduce the pass94 conf shape.
- Selector sequence corrected for DM1 PC 3.4 (GRAPHICS=0 → SOUND=0 → ENTER four times)
- Creature-chain capture is now pinned by `docs/parity/DM1_V1_CREATURE_CHAIN_ORIGINAL_CAPTURE_CONTRACT.json`: the live session must produce `creature_chain_d2c_trolin_front` and `creature_chain_d1c_trolin_front` rows before creature-chain pixel parity can be promoted.

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

> **Recommended:** run `docs/parity/tools/dosbox_capture_preflight.py` instead.  It performs both SHA checks, then writes a hardened `dosbox_capture.conf` whose settings are pinned to the runbook values below.  The preflight refuses to write a conf that contains the historical pass94 failure-mode settings (`machine=svga_paradise`, `memsize=4`, `core=normal`, `cycles=3000`), and records a JSON receipt that the next capture-session manifest can cite.  See `docs/parity/tools/dosbox_capture_preflight.py` for the full pin contract.

```bash
python3 docs/parity/tools/dosbox_capture_preflight.py \
    --data-dir ~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1 \
    --captures-dir ~/firestaff-captures
# Expected: preflight: 16/16 checks matched  PASS
# Writes: ~/firestaff-captures/dosbox_capture.conf
# Writes: ~/firestaff-captures/preflight.receipt.json
```

---

## Step 2: DOSBox Staging Configuration

**Machine type `svga_s3` is non-negotiable.** DM1 PC 3.4 requires VGA for stable 320×200 framebuffer reads. CGA and Hercules are incompatible with screenshot capture.

The preflight tool writes the hardened conf for you; if you need to write one by hand, mirror the runbook's required settings exactly:

```ini
[sdl]
output   = opengl
windowresolution   = 1024x768
viewport_resolution = 1024x768

[dosbox]
machine  = svga_s3
memsize  = 16

[render]
frameskip = 0

[cpu]
core    = dynamic
cycles  = max
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
        # Build mount command for DOSBox Staging autoexec.  The
        # canonical DM1 PC 3.4 layout ships DUNGEON.DAT and
        # GRAPHICS.DAT in the game root, with DM.EXE inside a
        # DungeonMasterPC34/ subdirectory; the older autoexec
        # attempted to launch "DungeonMasterPC34.EXE" from the game
        # root, which doesn't exist (the directory is a folder, not
        # the binary).  We `cd` into the subdir before launching so
        # the selector prompts match the runbook §3 sequence below.
        autoexec = f"""MOUNT C {GAME_DIR}
C:
cd DungeonMasterPC34
DM.EXE
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

            # One step forward.  The current executable runner selects
            # "Keyboard Simulation of Digital Joystick" and uses keypad 5,
            # matching ReDMCSB COMMAND.C:275-281 for C003_MOVE_FORWARD.
            self.press("Keypad-5")
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

# Crop to viewport region (no right column).  ImageMagick's `convert`
# rejects the bare "224x136+0+33" geometry with a "width" parse error
# on some macOS ImageMagick builds, so use a leading "+repage" to
# reset the virtual canvas before the crop.  The output filename MUST
# be a single token (the trailing "Done" from earlier drafts is a
# copy-paste artefact and breaks `convert`'s output-path parse).
for file in *.png; do
    convert "$file" -crop 224x136+0+33 +repage "cropped_${file}"
done

# Verify state on the FULL capture (the classifier regions are
# calibrated against the full 320x200 framebuffer, not the cropped
# 224x136 viewport — the crop step is for compare_captures.py only).
python3 ~/firestaff-captures/tools/dosbox_state_detector.py dungeon_start.png
# Expected output: state=dungeon_gameplay confidence=HIGH
```

Only captures classified as `dungeon_gameplay` are valid evidence. Discard all others.

If `convert` is not installed (`brew install imagemagick`), the
classifier self-test at
`docs/parity/tools/dosbox_state_detector.py --self-test` exercises the
same crop geometry in Pillow without the ImageMagick dependency, and
the runbook-consistency probe at
`tools/test_dm1_v1_capture_runbook_consistency.py` keeps the Step 4
command pinned to the runbook prose.

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

Pixel-compare with the actual tool at
`docs/parity/tools/compare_captures.py` (NOT the inlined version from
earlier drafts — that version used the default Pillow NEAREST filter
and never reached the LANCZOS resize path; the current tool also
emits a per-channel max delta so cosmetic palette shifts are easier
to attribute).  The classifier self-test in
`docs/parity/tools/dosbox_state_detector.py --self-test` and the
runbook-consistency probe in
`tools/test_dm1_v1_capture_runbook_consistency.py` keep the inline
vs. on-disk tool consistent.

Run:
```bash
python3 ~/firestaff-captures/tools/compare_captures.py \
  cropped_dungeon_start.png \
  firestaff_dungeon_start.ppm \
  viewport_start
```

> **Note:** the `python3 ~/firestaff-captures/tools/compare_captures.py`
> command above runs the on-disk tool.  An inlined draft of
> `compare_captures.py` previously lived directly in this runbook; it
> used the default Pillow NEAREST filter (no LANCZOS) and never
> reported per-channel max delta, so palette shifts were attributed
> to viewport geometry.  The on-disk tool at
> `docs/parity/tools/compare_captures.py` is the only script the next
> operator should run; the runbook-consistency probe at
> `tools/test_dm1_v1_capture_runbook_consistency.py` regression-pins
> the runbook to that tool.

---

## Step 5b: Render the Output Manifest (deterministic)

The Output Manifest Template at the bottom of this runbook used to
be filled in by hand, which is exactly how a stale placeholder hash
(``f7f3291f or actual git hash``) shipped through a previous draft —
the operator copy-pasted a draft template and the live manifest
carried the placeholder into the parity ledger.  The deterministic
handoff code at
`docs/parity/tools/dosbox_capture_manifest_writer.py` renders the
manifest from the preflight receipt and a per-capture classifier
output TSV so the SHA256s, the Firestaff git head, the
``dosbox_version`` row, and the per-capture classifications are
all live values, never placeholders.

Build the classifier output TSV (one row per capture, with
``file``, ``label``, ``classification``, ``sha256``, ``width``,
``height`` columns; the writer expects exactly six columns in
that order):

```tsv
file	label	classification	sha256	width	height
original/01_ingame_start.png	viewport_start	dungeon_gameplay	SHA256	320	200
original/01_ingame_start_viewport_224x136.png	viewport_start_crop	dungeon_gameplay	SHA256	224	136
original/02_ingame_turn_right.png	viewport_turn	dungeon_gameplay	SHA256	320	200
original/02_ingame_turn_right_viewport_224x136.png	viewport_turn_crop	dungeon_gameplay	SHA256	224	136
```

Then render the manifest (the writer refuses to emit a manifest
when the preflight receipt's pin checks are not all PASS, when a
recorded SHA does not match the file on disk, or when a
classification is outside the runbook's documented state list):

```bash
python3 docs/parity/tools/dosbox_capture_manifest_writer.py \
    --preflight-receipt ~/firestaff-captures/preflight.receipt.json \
    --classifier-outputs classifier_outputs.tsv \
    --manifest-out capture_manifest.tsv \
    --sidecar-out capture_manifest.sidecar.json

# Expected: manifest-writer: N/N checks matched
#           PASS — manifest: capture_manifest.tsv
# Writes: capture_manifest.tsv (the Output Manifest Template rows,
#         with real SHA256s and live git head) and
#         capture_manifest.sidecar.json (a JSON copy the pass608
#         runtime-transcript gate can ingest).
```

The writer ships a hermetic `--self-test` that exercises the
matching case, SHA-mismatch case, missing-receipt case, missing-
classifier-outputs case, preflight-pin-violation case, and invalid-
classification case against synthetic fixtures, and it is wired
into the runbook-consistency probe at
`tools/test_dm1_v1_capture_runbook_consistency.py` as
`manifest_writer_selftest`.  The probe fails loudly if a future
operator reverts to filling the manifest by hand, and the
``dosbox_capture_manifest_writer.py --self-test`` CTest gate
catches regressions in the writer itself.

---

## Step 5c: Render the Runtime Transcript (deterministic)

The pass608 / pass625 same-viewport capture blocker is reported by
``tools/verify_pass608_dm1_v1_same_viewport_capture_blocker.py``
and
``tools/verify_pass625_dm1_v1_original_transcript_row_preflight.py``
in the statuses
``BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE`` and
``FAIL_PASS625_DM1_V1_ORIGINAL_TRANSCRIPT_ROW_PREFLIGHT``.  Both
verifiers demand a ``transcript.json`` file that binds one original
DOSBox capture frame to one Firestaff fixture row through the
ReDMCSB source chain
``F0359_COMMAND_ProcessClick_CPSC or F0361_COMMAND_ProcessKeyPress``
→ ``F0380_COMMAND_ProcessQueue_CPSC`` →
``F0365/F0366_COMMAND_ProcessTypes*`` →
``F0128_DUNGEONVIEW_Draw_CPSF`` →
``F0097_DUNGEONVIEW_DrawViewport`` at the ``VIDRV_09_BlitViewPort``
present boundary, with the matching map/X/Y/direction tuple
binding the original frame to a Firestaff
``viewport_224x136`` hash from
``verification-screens/capture_manifest_sha256.tsv``.

Building that transcript by hand is exactly the gap that kept
the pass608 blocker in
``BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE``:
the next operator would have to invent a 30-field pass608 row
shape *and* a 40-field pass625 row shape *and* pin every
ReDMCSB source-function name *and* cross-check the
``inputToken`` → ``sourceCommandId`` mapping against the
pass623 canonical input-capture fixture *and* confirm the
Firestaff fixture hash is a known ``viewport_224x136`` row.
The deterministic handoff code at
`docs/parity/tools/dosbox_capture_transcript_writer.py` does
all of that in one pass.

Build the per-capture events TSV (one row per live capture;
the writer expects exactly 41 tab-separated columns in the
order below — the column order is part of the public
contract, do not re-order):

```tsv
file	label	classification	raw_sha256	crop_path	crop_sha256	width	height	input_token	source_command_id	source_command_name	queue_source_function	queue_count_before	queue_count_after	queue_first_index_before	queue_first_index_after	dispatch_source_function	dispatch_handler	redraw_source_function	redraw_map_x	redraw_map_y	redraw_direction	present_source_function	present_viewport_presented	present_boundary	party_map_index	party_x	party_y	party_direction	party_before_map_index	party_before_x	party_before_y	party_before_direction	original_asset_set_sha_graphics	original_asset_set_sha_dungeon	firestaff_map_index	firestaff_x	firestaff_y	firestaff_direction	firestaff_viewport_sha256	run_id
```

The pass623 fixture pins every documented
``inputToken`` → ``sourceCommandId`` mapping (see
`parity-evidence/verification/pass623_dm1_v1_input_capture_readiness_bridge/manifest.json`),
so the live operator can copy the correct
``source_command_id`` / ``source_command_name`` from the
fixture without re-deriving the ReDMCSB
COMMAND.C / F0380 dispatch table.  The
``original_asset_set_sha_*`` columns are the runbook §1
constants (the same ones the preflight's receipt already
records in ``dungeonSha256``/``graphicsSha256``).

Then render the transcript (the writer refuses to emit a
transcript when the preflight receipt's pin checks are not
all PASS, when an input token has no command id mapping in
the pass623 fixture, when a row's command id is a TURN and
``dispatch.handler`` is the MOVE handler or vice versa, when
``partyAfter`` does not match the F0128 redraw tuple, when
``firestaffFrame.viewportSha256`` is not in the canonical
Firestaff fixture viewport-hash set, when a recorded
``originalFrame.rawSha256``/``cropSha256`` does not match
the bytes on disk, or when the events TSV header is
re-ordered):

```bash
python3 docs/parity/tools/dosbox_capture_transcript_writer.py \
    --preflight-receipt ~/firestaff-captures/preflight.receipt.json \
    --events-tsv events.tsv \
    --transcript-out transcript.json

# Expected: transcript-writer: N/N checks matched
#           PASS - transcript: transcript.json
# Writes: transcript.json with one row per events TSV row,
#         every required pass608 (30) + pass625 (40) field
#         present in every row, and a top-level
#         ``promotable: true`` flag the pass608 verifier
#         reads as ``runtimeTranscript.ok = true`` when at
#         least one row is in the same run.
```

The writer ships a hermetic ``--self-test`` that exercises
the matching case, the unknown-input-token case, the
command/handler-mismatch case, the
``partyAfter``/``F0128``-redraw-drift case, the
unknown-fixture-hash case, the preflight-pin-violation
case, the asset-set-mismatch case, the bad-run-id case,
the transcript-structural invariants (schema, three-key
mirror, payload.promotable), and the verbatim column-order
contract against synthetic fixtures, and it is wired into
the runbook-consistency probe at
`tools/test_dm1_v1_capture_runbook_consistency.py` as
`transcript_writer_selftest`.  The probe fails loudly if a
future operator reverts to hand-building the transcript,
and the
``dosbox_capture_transcript_writer.py --self-test`` CTest
gate (see the CMake block below) catches regressions in
the writer itself.

The runtime transcript is what the pass608 verifier's
``runtimeTranscript`` contract reads.  Once the next live
DOSBox attempt produces a transcript.json that satisfies
all binding checks, the
``BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE``
status flips to
``PROMOTED_STATUS = PASS608_DM1_V1_COMMAND_STATE_REDRAW_TRANSCRIPT_BOUND``
and the pass622 → pass623 → pass625 chain can move.

---

## Step 5d: Render the Events TSV (deterministic)

The events TSV the transcript writer consumes has 41
tab-separated columns, most of which are source-locked
constants (``F0380_COMMAND_ProcessQueue_CPSC``,
``F0365/F0366_COMMAND_ProcessTypes1To2/3To6``,
``F0128_DUNGEONVIEW_Draw_CPSF``,
``F0097_DUNGEONVIEW_DrawViewport``,
``VIDRV_09_BlitViewPort``, the runbook §1
``GRAPHICS.DAT``/``DUNGEON.DAT`` SHA256s) or pass623-fixture-
pinned values (the ``inputToken`` → ``sourceCommandId``
mapping, the ``postTuple``, the Firestaff
``viewportSha256``).  Building the row by hand is exactly
the gap that kept the pass608 blocker in
``BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE``
after the pass633 / pass625 work landed: a typo in a
function name (``F0365`` vs ``F0380``), a stale
ReDMCSB literal, a wrong queue count, or a wrong viewport
hash silently re-introduces the blocker the writer is
meant to catch.

The deterministic handoff code at
``docs/parity/tools/dosbox_capture_events_row_builder.py``
turns a single pass623 route label (e.g.
``02_turn_right_west_1_3``) + a 320x200 capture frame
+ a 224x136 crop into one 41-column row (or one row per
command for multi-command routes like
``04_forward_south_1_4``), pulling every source-locked
value and every pass623-fixture value verbatim from the
canonical contracts so the operator never has to type
them.  Build the events TSV one row at a time:

```bash
# Single-command route (renders 1 row, with --with-header
# the very first invocation also writes the EVENTS_TSV_HEADER
# line so subsequent invocations can be concatenated).
python3 docs/parity/tools/dosbox_capture_events_row_builder.py \
    --label 02_turn_right_west_1_3 \
    --raw  original/02_ingame_turn_right.png \
    --crop original/02_ingame_turn_right_viewport.png \
    --run-id 2026-06-07_dm1_v1_ingame \
    --preflight-receipt ~/firestaff-captures/preflight.receipt.json \
    --pass623-fixture parity-evidence/verification/pass623_dm1_v1_input_capture_readiness_bridge/manifest.json \
    --classification dungeon_gameplay \
    --with-header \
    --queue-count-before 1 \
    --queue-first-index-before 0 \
    >> events.tsv

# Multi-command route (renders 1 row per command; the
# helper splits the route's commands=[1, 3] into two
# events, both sharing the route's final postTuple and
# the pass623 viewportSha256).
python3 docs/parity/tools/dosbox_capture_events_row_builder.py \
    --label 04_forward_south_1_4 \
    --raw  original/04_forward_south_1_4.png \
    --crop original/04_forward_south_1_4_viewport.png \
    --run-id 2026-06-07_dm1_v1_ingame \
    --preflight-receipt ~/firestaff-captures/preflight.receipt.json \
    --pass623-fixture parity-evidence/verification/pass623_dm1_v1_input_capture_readiness_bridge/manifest.json \
    --classification dungeon_gameplay \
    --party-before-map   0 \
    --party-before-x     1 \
    --party-before-y     3 \
    --party-before-direction 2 \
    --queue-count-before 2 \
    --queue-first-index-before 0 \
    >> events.tsv
```

The row builder refuses to emit a row when:

  * the route label is not in the pass623 fixture
    (the operator is asking for a route that has no
    canonical binding);
  * the recorded 320x200 capture file is missing or
    its on-disk SHA256 does not match the recorded
    ``raw_sha256`` (mirroring the writer's pin contract
    — a stale SHA cannot silently ship);
  * the recorded 224x136 crop is missing or its
    on-disk SHA256 does not match the recorded
    ``crop_sha256``;
  * the raw capture header is not 320x200 or the crop
    header is not 224x136 (this catches a stale
    full-frame file passed as the viewport crop before
    a plausible transcript row can be emitted);
  * the preflight receipt's pin checks are not all PASS
    (the upstream contract is violated);
  * the route is multi-command but the recorded
    ``inputToken``/``sourceCommandId`` pair does not
    match the pass623 fixture (a classifier bug or a
    route bug that must be caught at this layer, not
    at the writer).

The helper ships a hermetic ``--self-test`` that
exercises the matching case, the unknown-route-label
case, the missing-receipt case, the bad-receipt case,
the missing-raw case, the multi-command-split case, the
end-to-end single-command promotable case, the
end-to-end multi-command promotable case, the
dispatch-handler-by-command case, the
source-function-pinned case, the verbatim column-order
case, and the baseline-row case against synthetic
fixtures, and it is wired into the runbook-consistency
probe at
``tools/test_dm1_v1_capture_runbook_consistency.py`` as
``events_row_builder_selftest``.  The probe fails loudly
if a future operator reverts to typing the 41 columns
by hand, and the
``dosbox_capture_events_row_builder.py --self-test``
CTest gate (see the CMake block below) catches
regressions in the helper itself.  The writer's
``_load_pass623_fixture`` was extended in the same pass
to pair ``inputTokens`` with ``commandIds``
positionally for multi-command routes, so a
``commands=[1, 3]`` / ``tokens=[LEFT, UP]`` row pairs
``LEFT → 1`` and ``UP → 3`` (and not both to ``1`` the
way a legacy single-id mapping would); a regression
here is caught by the writer's new multi-command
self-test.

---

## Step 5e: Live Row Gate (deterministic)

The handoff closure gate at
``tools/verify_dm1_v1_original_capture_route_handoff.py``
wires the preflight + row builder + transcript writer +
pass608 verifier chain for the standalone
``02_turn_right_west_1_3`` route.  The live row gate at
``tools/verify_dm1_v1_original_capture_live_row_gate.py``
is the live-binding companion: it pins the live session
runner
(``docs/parity/tools/dosbox_capture_session.py``)'s exact
``original/01_ingame_start.png`` and
``original/02_ingame_step_forward.png`` filenames (the
files ``live_run()`` writes at lines 2124 / 2132) to
pass623 route labels + Firestaff fixture viewport hashes,
then runs the same on-disk toolchain and confirms the
pass608 blocker flips to
``PASS608_DM1_V1_COMMAND_STATE_REDRAW_TRANSCRIPT_BOUND``.

The live-binding table is part of the public contract:

| Live runner ``save()`` filename      | Pass623 route label           | Firestaff fixture viewport hash (sha256) |
|--------------------------------------+-------------------------------+------------------------------------------|
| ``01_ingame_start.png``              | ``01_start_south_1_3``        | ``50661c78…8bf9`` (``01_ingame_start_latest_viewport_224x136.ppm``) |
| ``02_ingame_step_forward.png``       | ``03_blocked_west_wall_1_3``  | ``0cb83803…92b8`` (``03_ingame_move_forward_latest_viewport_224x136.ppm``) |

The live row gate is hermetic and runs in CI as the
CTest ``dm1_v1_original_capture_live_row_gate`` (see the
CMake block below) and as a sub-check of the
runbook-consistency probe at
``tools/test_dm1_v1_capture_runbook_consistency.py`` under
``live_row_gate_selftest``.  A future operator who
renames either side of the binding table (the live
session's ``save()`` filename, the pass623 label, the
Firestaff fixture viewport hash) without updating the
gate gets a self-describing CI failure, so the next
live attempt cannot ship a transcript whose
``originalFrame.path`` is no longer in the live
runbook.

Run the gate locally before the live attempt:

```bash
python3 tools/verify_dm1_v1_original_capture_live_row_gate.py
# Expected: DM1_V1_ORIGINAL_CAPTURE_LIVE_ROW_GATE_CLOSED: 8/8 live row gate checks passed
#           PASS
```

The gate's 8 sub-checks: (1) the on-disk pass608
verifier is importable and reports the baseline BLOCKED
status; (2) the on-disk preflight self-test passes; (3)
the on-disk live session runner's ``live_run()`` saves
its captures to the exact filenames in the live-binding
table (a regex pin on the source string keeps a future
rename of ``01_ingame_start.png`` or
``02_ingame_step_forward.png`` from silently breaking
the live handoff); (4) the on-disk row builder renders
a 41-column events TSV row for each live-binding row,
using the row builder's own preflight + pass623
validation pipeline (the row builder refuses to emit a
row for a label that is not in the pass623 fixture, so
this sub-check is also the
live-binding-table-matches-pass623 check); (5) the
on-disk transcript writer consumes the two
live-binding rows and emits a transcript whose
``promotable`` flag is True; (6) the pass608 verifier
reads the same transcript and flips its status to
``PROMOTED`` with ``loaded_promotable_same_run``; (7) a
negative path: the Firestaff viewport hash for the
second live capture is patched to a never-seen 64-hex
value and the pass608 verifier's
``firestaffViewportHashKnown`` check must fail and keep
the BLOCKED status, so a future regression that drops
the live-binding's hash discipline is caught at the
gate instead of at the next live attempt; (8) every
row of the live-binding table is anchored to the
real on-disk
``parity-evidence/verification/pass623_dm1_v1_input_capture_readiness_bridge/manifest.json``
(pass623 route label) and the real on-disk
``verification-screens/capture_manifest_sha256.tsv``
(Firestaff fixture viewport sha), so a future pass623
rotation or Firestaff capture manifest rename is
caught at the gate instead of at the next live
attempt.

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
| Black viewport region | DOSBox not focused or host capture API returned the wrong/blank window | First try `--capture-backend dosbox-rawshot` so DOSBox writes its own Ctrl+F5 capture into `dosbox-capture/`; otherwise add `osascript -e 'tell app "DOSBox Staging" to activate'` and retry `auto` |
| Duplicate crop SHA256 | Game state identical — no input was processed, or the route selected Mouse control mode and then tried keyboard movement | Verify the route selected control option `4` (`Keyboard Simulation of Digital Joystick`) and sent `Keypad-5`; keep the C070 mouse click as diagnostic only |
| Repeated non-DOSBox frontmost samples across `FOCUS_MISMATCH_FRAME_LIMIT` (4) | macOS window focus drifted off DOSBox; host peekaboo/screencapture backends now show the wrong window | The live route auto-attempts a rawshot-fallback recovery probe (DOSBox's own Ctrl+F5 capture, which is independent of macOS window focus) and writes `dosbox_capture.focus_recovery.json` with the `rawshot_focus_recovered` / `rawshot_focus_unrecoverable` reason.  If the recovery is `rawshot_focus_unrecoverable`, re-focus DOSBox Staging (`osascript -e 'tell app "DOSBox Staging" to activate'`) and retry `--capture-backend dosbox-rawshot` directly. |

---

## DM1 PC 3.4 Selector Sequence (corrected)

The selector in DM1 PC 3.4 works like this:

1. **GRAPHICS screen** — type `1` + Return for VGA.
2. **SOUND screen** — type `1` + Return for No Sound.
3. **CONTROL screen** — type `4` + Return for Keyboard Simulation of Digital Joystick.
4. **Entrance wall** — Return activates ENTER because the wall selector cursor starts there.
5. **Dungeon movement** — `Keypad-5` maps to C003 move-forward through the original PC movement keyboard table.

The live runner still sends the C070 forward-arrow mouse click as a diagnostic
probe, but it does not promote that path: under the macOS/DOSBox route tested
on 2026-06-13 the C070 host click was delivered and logged but did not change
the original viewport. The promoted original-DOS movement row is the
keyboard-simulation path.

---

## Output Manifest Template

> **Do NOT fill this template in by hand.**  The deterministic
> handoff code at
> `docs/parity/tools/dosbox_capture_manifest_writer.py` renders the
> manifest from the preflight receipt + a per-capture classifier
> output TSV (see Step 5b above).  The on-disk writer is the only
> way the next manifest should be produced; the prose template
> below is kept for documentation and for the writer's column
> order, not for hand-filling.

The template the writer renders (column order is part of the
public contract — do not re-order):

```tsv
capture_session	{ISO timestamp, from the preflight receipt}
dungeon_sha256	d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
graphics_sha256	2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
session_id	{preflight receipt session_id}
launch_command	{preflight receipt launch_command}
firestaff_version	{actual git head, e.g. `git rev-parse HEAD` — the preflight receipt already records this in `firestaff_git_head`}
dosbox_version	{DOSBox Staging version from --version, auto-detected by the writer}

file	label	classification	sha256	width	height
dungeon_start.png	dungeon_start	dungeon_gameplay	SHA256	320	200
cropped_dungeon_start.png	dungeon_start_crop	dungeon_gameplay	SHA256	224	136
```

Filename format: `YYMMDD-HHMMSS_description.png`
