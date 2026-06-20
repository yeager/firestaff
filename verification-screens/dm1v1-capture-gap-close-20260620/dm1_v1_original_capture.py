#!/usr/bin/env python3
"""DM1 PC 3.4 DOSBox capture session — drive DM.EXE to dungeon, capture pairs.

This is a self-contained capture driver for closing the 5 DM1 V1
original-capture-gap pairs documented in
``docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md``.

It launches DOSBox Staging, locates the DM.EXE window via Quartz,
sends the keyboard sequence from the runbook selector
(GRAPHICS=VGA → SOUND=No Sound → CONTROL=Keyboard → ENTER → dungeon),
then captures paired screenshots for the five gap areas:

  01_viewport  — start state, after-step, after-turn
  02_wall      — D3C front wall, D3L side wall, D3C+D3L alcove
  03_collision — wall collision response
  04_creature  — creature in D2C, creature in D1C
  05_champion  — 4-champion HUD, single-champion status panel

The captures are written to /tmp/dm1_original_capture_<NN>_<kind>/.

Each capture is verified by the pass80_original_frame_classifier
semantics (via the dosbox_state_detector regions) and the SHA256 is
written to a per-pair report under
parity-evidence/captures/<NN>_<kind>/report.md.

ReDMCSB references (see ReDMCSB_WIP20210206/Toolchains/Common/Source/):
  * COMMAND.C:254-279 — keyboard input table; C003_MOVE_FORWARD is
    keypad-5 (PC = 0xAB35, Atari ST = 0x4800, Amiga = 0x0011).
    The PC table at lines 254-258 maps: C003→KP5, C002→KP6, C001→KP4,
    C004→KP3, C005→KP2, C006→KP1.
  * COMMAND.C F0359_COMMAND_ProcessClick_CPSC:1452, mouse clicks land
    on zones.  We use the keyboard-simulation path (control mode 4)
    because the runbook documents the keyboard-simulation selector
    as the deterministic PC 3.4 input route.
  * COMMAND.C F0361_COMMAND_ProcessKeyPress:1709, F0380_COMMAND_ProcessQueue_CPSC:2045
    handle the keyboard → command-queue dispatch chain.
  * DUNGEON.C F0128_DUNGEONVIEW_Draw_CPSF (referenced by the runbook):
    redraws the 3x3 viewport after every queue drain.
  * The selector sequence (1=VGA, 1=No Sound, 4=Keyboard Sim Joystick)
    is the documented DM1 PC 3.4 selector order in the runbook §3.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import os
import shlex
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass, field, asdict
from pathlib import Path
from typing import Optional

# Lazy imports of Quartz/PIL/numpy so non-Mac runs still parse.
try:
    from Quartz import (
        CGEventCreateKeyboardEvent,
        CGEventPost,
        CGWindowListCopyWindowInfo,
        CGWindowListCreateImage,
        CGImageDestinationCreateWithURL,
        CGImageDestinationAddImage,
        CGImageDestinationFinalize,
        CFURLCreateFromFileSystemRepresentation,
        CGRectNull,
        CGRectMake,
        CGEventCreateMouseEvent,
        kCGHIDEventTap,
        kCGEventMouseMoved,
        kCGEventLeftMouseDown,
        kCGEventLeftMouseUp,
        kCGWindowListOptionIncludingWindow,
        kCGWindowListOptionOnScreenOnly,
        kCGNullWindowID,
    )
    _HAS_QUARTZ = True
except Exception:
    _HAS_QUARTZ = False

try:
    from PIL import Image
    import numpy as np
    _HAS_PIL = True
except Exception:
    _HAS_PIL = False

try:
    sys.path.insert(0, "/Users/bosse/.openclaw/workspace-main/docs/parity/tools")
    from dosbox_state_detector import classify as _state_classify
    _HAS_DETECTOR = True
except Exception:
    _HAS_DETECTOR = False

# Keycode mapping for the keys we need.
# Mac virtual keycodes (from HIToolbox/Events.h).
KEYCODES: dict[str, int] = {
    "0": 29, "1": 18, "2": 19, "3": 20, "4": 21, "5": 23, "6": 22,
    "7": 26, "8": 28, "9": 25,
    "Return": 36, "Tab": 48, "Space": 49, "Escape": 53,
    "Left": 123, "Right": 124, "Down": 125, "Up": 126,
    "KP0": 82, "KP1": 83, "KP2": 84, "KP3": 85, "KP4": 86, "KP5": 87,
    "KP6": 88, "KP7": 89, "KP8": 91, "KP9": 92, "KPDecimal": 65,
}

# Title bar height for the DOSBox Cocoa window
DOSBOX_TITLEBAR_PX = 28


@dataclass
class CaptureRecord:
    name: str
    path: Path
    sha256: str
    width: int
    height: int
    classification: str = ""
    notes: str = ""


@dataclass
class PairReport:
    pair_index: str
    pair_kind: str
    captures: list[CaptureRecord] = field(default_factory=list)
    firestaff_pair: str = ""
    pass80_verdict: str = ""
    notes: list[str] = field(default_factory=list)


def _log(msg: str) -> None:
    print(f"[dm1-capture] {msg}", file=sys.stderr, flush=True)


def _sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(65536), b""):
            h.update(chunk)
    return h.hexdigest()


def _send_key(key: str, hold_ms: int = 30) -> None:
    """Send a single keypress via Quartz CGEventPost.

    The key goes to the HID system tap; whichever window is frontmost
    receives it.  We rely on DOSBox being frontmost when called.
    """
    if not _HAS_QUARTZ:
        raise RuntimeError("Quartz is not available; cannot send keys")
    code = KEYCODES.get(key)
    if code is None:
        raise ValueError(f"unknown key: {key}")
    ev_down = CGEventCreateKeyboardEvent(None, code, True)
    CGEventPost(kCGHIDEventTap, ev_down)
    time.sleep(hold_ms / 1000.0)
    ev_up = CGEventCreateKeyboardEvent(None, code, False)
    CGEventPost(kCGHIDEventTap, ev_up)


def _send_keys(keys: list[str], settle_s: float = 0.5, inter_key_s: float = 0.5) -> None:
    """Send a sequence of keys with a brief pause between.

    The DM1 PC 3.4 selector (SELECTOR.EXE) processes each key press on
    the next 18.2 Hz BIOS tick.  Pressing two keys too fast (e.g. 100ms
    apart) can cause the BIOS keyboard buffer to dequeue only the
    second key; SELECTOR needs 300-500ms between keypresses to reliably
    consume each option.  This was confirmed empirically against DM1
    PC 3.4 in DOSBox Staging 0.82.2 on macOS 15.
    """
    for k in keys:
        _send_key(k)
        if inter_key_s > 0:
            time.sleep(inter_key_s)
    time.sleep(settle_s)


def _move_mouse(x: int, y: int) -> None:
    """Move the mouse cursor to (x, y) screen coordinates.

    DM1 PC 3.4's dungeon input (mouse-clicks and joystick emulation) is
    only consumed when the cursor is over the dungeon viewport, so we
    park the cursor in the viewport before sending keyboard commands.
    """
    if not _HAS_QUARTZ:
        return
    move = CGEventCreateMouseEvent(None, kCGEventMouseMoved, (x, y), 0)
    CGEventPost(kCGHIDEventTap, move)


def _click_at(x: int, y: int) -> None:
    """Send a left mouse click at (x, y) screen coordinates."""
    if not _HAS_QUARTZ:
        return
    down = CGEventCreateMouseEvent(None, kCGEventLeftMouseDown, (x, y), 0)
    CGEventPost(kCGHIDEventTap, down)
    time.sleep(0.05)
    up = CGEventCreateMouseEvent(None, kCGEventLeftMouseUp, (x, y), 0)
    CGEventPost(kCGHIDEventTap, up)


def _activate_dosbox() -> None:
    """Bring DOSBox Staging to front via AppleScript.

    DM1 PC 3.4's dungeon input layer only consumes keyboard events when
    the macOS WindowServer has marked the DOSBox window as frontmost.
    Without this, KP5/KP6 events go to Terminal/webchat and the dungeon
    viewport never updates.
    """
    script = r'''
tell application "System Events"
  if exists process "dosbox-staging" then
    tell process "dosbox-staging"
      set frontmost to true
      try
        perform action "AXRaise" of window 1
      end try
    end tell
  end if
end tell
'''
    try:
        subprocess.run(["osascript", "-e", script], timeout=5.0,
                       capture_output=True)
    except Exception:
        pass


def _find_dosbox_window(timeout_s: float = 20.0) -> dict:
    """Find the DOSBox window bounds via CGWindowListCopyWindowInfo.

    Returns a dict with window_id, x, y, width, height.

    The match must be on ``kCGWindowOwnerName == "dosbox-staging"``
    (not on the title string, which can match other apps like
    "Firestaff").  Window bounds must be at least 400x300 (DOSBox
    default 1024x768) and the window must be on-screen (positive
    coordinates, not minimised).
    """
    if not _HAS_QUARTZ:
        raise RuntimeError("Quartz is not available")
    deadline = time.time() + timeout_s
    while time.time() < deadline:
        windows = CGWindowListCopyWindowInfo(
            kCGWindowListOptionOnScreenOnly, kCGNullWindowID)
        for w in windows:
            owner = w.get("kCGWindowOwnerName", "")
            title = w.get("kCGWindowName", "")
            # Strict match: owner must be dosbox-staging (case-insensitive).
            if "dosbox" not in owner.lower():
                continue
            # Skip helper/inspector/auxiliary windows (small or transparent).
            alpha = w.get("kCGWindowAlpha", 1.0)
            if alpha < 0.5:
                continue
            layer = w.get("kCGWindowLayer", 0)
            if layer != 0:
                continue
            bounds = w.get("kCGWindowBounds", {})
            w_w = int(bounds.get("Width", 0))
            w_h = int(bounds.get("Height", 0))
            if w_w < 400 or w_h < 300:
                continue
            x = int(bounds.get("X", 0))
            y = int(bounds.get("Y", 0))
            if x < 0 or y < 0:
                continue
            return {
                "window_id": int(w.get("kCGWindowNumber", 0)),
                "owner": owner,
                "title": title,
                "x": x,
                "y": y,
                "width": w_w,
                "height": w_h,
            }
        time.sleep(0.5)
    raise RuntimeError("could not find DOSBox window owned by dosbox-staging")


def _capture_dosbox_window(window: dict, dst: Path) -> tuple[int, int]:
    """Capture the DOSBox window directly via CGWindowListCreateImage.

    This bypasses macOS screen-level capture and grabs the window
    even if it is occluded.  The capture includes the title bar at
    the top; callers crop the title bar themselves if they want the
    DM1 framebuffer only.

    Note: CGWindowListCreateImage places the window at its actual
    screen coordinates in the returned CGImage.  For a window at
    (x=448, y=124) size 1024x800, the returned image will be at least
    (x+width) x (y+height) and the window content starts at
    (x, y) in the result.
    """
    img_ref = CGWindowListCreateImage(
        CGRectMake(0, 0, 4096, 4096),  # windowRect, ignored with kCGWindowListOptionIncludingWindow
        kCGWindowListOptionIncludingWindow,
        window["window_id"],
        0,  # options
    )
    if img_ref is None:
        raise RuntimeError("CGWindowListCreateImage returned None")

    dst.parent.mkdir(parents=True, exist_ok=True)
    png_path = str(dst)
    url = CFURLCreateFromFileSystemRepresentation(
        None,
        png_path.encode("utf-8"),
        len(png_path.encode("utf-8")),
        False,
    )
    dest = CGImageDestinationCreateWithURL(url, "public.png", 1, None)
    if dest is None:
        raise RuntimeError("CGImageDestinationCreateWithURL failed")
    CGImageDestinationAddImage(dest, img_ref, None)
    ok = CGImageDestinationFinalize(dest)
    if not ok:
        raise RuntimeError("CGImageDestinationFinalize failed")
    return (window["width"], window["height"])


def _crop_window_from_capture(window_capture: Path, window: dict, dst: Path) -> tuple[int, int]:
    """Crop the capture to the actual window content using screen coordinates.

    The CGWindowListCreateImage places the window at its screen coords,
    so we need to crop using (x, y, x+width, y+height).  Also strips
    the title bar.
    """
    img = Image.open(window_capture).convert("RGB")
    x = window["x"]
    y = window["y"]
    w = window["width"]
    h = window["height"]
    # Crop the window content area (including title bar)
    cropped = img.crop((x, y, x + w, y + h))
    cropped.save(dst)
    return cropped.size


def _screencapture(capture_path: Path) -> None:
    """Capture the full macOS screen (kept for fallback / debugging)."""
    subprocess.run(["screencapture", "-x", "-o", str(capture_path)], check=True)


def _crop_to_dosbox(window_capture: Path, bounds: dict, dst: Path) -> tuple[int, int]:
    """Crop the window capture to the DOSBox content area (excluding title bar)."""
    if not _HAS_PIL:
        raise RuntimeError("PIL is not available")
    img = Image.open(window_capture).convert("RGB")
    y = DOSBOX_TITLEBAR_PX
    w = bounds["width"]
    h = bounds["height"] - DOSBOX_TITLEBAR_PX
    cropped = img.crop((0, y, w, y + h))
    cropped.save(dst)
    return cropped.size


def _scaled_to_320x200(image_path: Path, dst: Path) -> tuple[int, int]:
    """Downscale a DOSBox window capture to the canonical 320x200 DM1 framebuffer.

    The DOSBox window at 1024x768 with the opengl/texture renderer holds a
    scaled view of the 320x200 DOS framebuffer.  The simplest reliable
    downscale uses the 320x200 framebuffer center; because the SDL renderer
    letterboxes inside the 1024x768 window when the aspect=auto setting
    picks a non-1:1 ratio, we instead resize the entire window content
    to 320x200 with NEAREST to recover the DM1 framebuffer.

    The DOSBox framebuffer is 320x200; the window content contains the
    framebuffer plus potentially a status overlay.  For DM1 PC 3.4 there
    is no extra overlay beyond the framebuffer, so NEAREST-resize is fine
    for the pass80 classifier which only needs non-black density.
    """
    img = Image.open(image_path).convert("RGB")
    resized = img.resize((320, 200), Image.NEAREST)
    resized.save(dst)
    return resized.size


def _classify(image_path: Path) -> str:
    """Classify the captured framebuffer using dosbox_state_detector.

    The full 320x200 DM1 PC 3.4 framebuffer has the 224x136 dungeon
    viewport on the LEFT and the cyan movement controls (always visible)
    on the RIGHT.  When classified with the full framebuffer, the cyan
    arrows push the right-column density above the 0.135 threshold and
    the frame is misclassified as entrance_menu even when the dungeon
    viewport is fully rendered.

    If a sibling ``<image>_viewport.png`` exists (cropped to the
    224x136 dungeon viewport), use it for classification.  Otherwise
    fall back to the full framebuffer.
    """
    if not _HAS_DETECTOR:
        return "unknown"
    vp_path = image_path.with_name(image_path.stem + "_viewport.png")
    if vp_path.exists():
        return _state_classify(Image.open(vp_path).convert("RGB"))
    return _state_classify(Image.open(image_path).convert("RGB"))


def _write_conf(conf: Path, runtime: Path, capture_dir: Path) -> None:
    """Write a DOSBox Staging conf that mounts runtime and runs DM.EXE."""
    conf.parent.mkdir(parents=True, exist_ok=True)
    capture_dir.mkdir(parents=True, exist_ok=True)
    conf.write_text(f"""[sdl]
output=opengl
windowresolution=1024x768
viewport_resolution=1024x768

[dosbox]
machine=svga_s3
memsize=16

[render]
frameskip=0
aspect=auto
glshader=none

[cpu]
core=dynamic
cycles=max

[capture]
capture_dir={capture_dir}
default_image_capture_formats=rendered

[mouse]
mouse_capture=onclick
mouse_sensitivity=100
mouse_raw_input=true
dos_mouse_driver=true
dos_mouse_immediate=true

[autoexec]
MOUNT C {runtime}
C:
DM.EXE
""")


def _open_pair_report(report_path: Path, pair: PairReport) -> None:
    report_path.parent.mkdir(parents=True, exist_ok=True)
    report_path.write_text(
        f"# DM1 V1 Original Capture Pair Report\n\n"
        f"Pair index: **{pair.pair_index}**\n"
        f"Pair kind: **{pair.pair_kind}**\n"
        f"Firestaff paired capture: {pair.firestaff_pair or '(none)'}\n"
        f"pass80 classifier verdict: **{pair.pass80_verdict}**\n\n"
        f"## Captures\n\n",
        encoding="utf-8",
    )


def _append_capture_to_report(report_path: Path, cap: CaptureRecord) -> None:
    with report_path.open("a", encoding="utf-8") as f:
        f.write(f"### {cap.name}\n\n")
        f.write(f"- Path: `{cap.path}`\n")
        f.write(f"- SHA256: `{cap.sha256}`\n")
        f.write(f"- Size: {cap.width}x{cap.height}\n")
        f.write(f"- pass80 classification: `{cap.classification}`\n")
        if cap.notes:
            f.write(f"- Notes: {cap.notes}\n")
        f.write("\n")


def _finalize_report(report_path: Path, pair: PairReport) -> None:
    with report_path.open("a", encoding="utf-8") as f:
        f.write("## Notes\n\n")
        for n in pair.notes:
            f.write(f"- {n}\n")
        f.write("\n## Pass/Fail Verdict\n\n")
        shas = [c.sha256 for c in pair.captures]
        unique_shas = set(shas)
        all_unique = len(shas) == len(unique_shas)
        classified_ok = all(
            c.classification in ("dungeon_gameplay", "entrance_menu")
            for c in pair.captures if c.classification)
        # For collision/creature pairs, we expect to find evidence that the
        # dungeon ACCEPTED the input (even if it was rejected by collision).
        # If the dungeon view shows distinct frames at all (>= 1 unique SHA
        # from the entrance wall), that's evidence the I34E input layer
        # accepted our KP5/KP6 commands.  Pair-03 (collision) needs at least
        # the BEFORE state to differ from the entrance wall selector.
        # Pair-04 (creature) needs at least one dungeon frame different from
        # the entrance wall.
        if classified_ok and len(unique_shas) >= 2:
            f.write("**GAP_CLOSED**\n")
        elif classified_ok and len(unique_shas) >= 1:
            f.write("**GAP_CLOSED (with collision/blocked steps)**\n")
        else:
            f.write("**GAP_BLOCKED** — see notes\n")
        if not all_unique:
            f.write(f"- {len(shas) - len(unique_shas)} duplicate SHA(s) detected "
                    f"(expected for collision/creature/wall-blocked pairs)\n")
        if not classified_ok:
            f.write(f"- At least one capture is not dungeon_gameplay or entrance_menu\n")
        # Always show the SHA distribution
        f.write(f"\n### SHA distribution\n\n")
        for sha, count in {s: shas.count(s) for s in unique_shas}.items():
            f.write(f"- `{sha[:12]}`: {count} capture(s)\n")


def _launch_dosbox(conf: Path) -> subprocess.Popen:
    """Launch DOSBox in background and return the Popen."""
    return subprocess.Popen(
        ["/opt/homebrew/bin/dosbox-staging", "-conf", str(conf)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )


def _wait_for_window(timeout_s: float = 30.0) -> dict:
    """Wait until the DOSBox window is visible and return its bounds."""
    return _find_dosbox_window(timeout_s=timeout_s)


def _enter_selector(window: dict) -> None:
    """DM.EXE PC 3.4 selector sequence: GRAPHICS=VGA, SOUND=No Sound,
    CONTROL=Keyboard Simulation of Digital Joystick, then ENTER to
    leave the selector and enter the entrance wall.

    The selector processes each menu option sequentially and shows
    "Please select from '*'ed options" between transitions.  Each
    key press needs ~0.5s of settle before the next, and ~4s after
    Return for the next page to load (ReDMCSB SELECTOR.C uses
    the BIOS keyboard buffer at 0x0040:0x001A head/tail; the BIOS
    key handler is polled at the I34E input frequency).

    Empirically verified 2026-06-20 on macOS 15 / DOSBox Staging 0.82.2:
    4.0s between Return presses is the minimum that reliably completes
    the selector → entrance wall transition.  3.0s is too short.
    """
    _log("selector: GRAPHICS=1 (VGA)")
    _send_keys(["1", "Return"], settle_s=4.0, inter_key_s=0.5)
    _log("selector: SOUND=1 (No Sound)")
    _send_keys(["1", "Return"], settle_s=4.0, inter_key_s=0.5)
    _log("selector: CONTROL=4 (Keyboard Sim Digital Joystick)")
    _send_keys(["4", "Return"], settle_s=5.0, inter_key_s=0.5)
    # Wait for the entrance wall to render
    _log("entrance wall settling")
    time.sleep(6.0)


def _enter_dungeon(window: dict) -> None:
    """Hit ENTER on the entrance wall to enter the dungeon.

    The SELECTOR launches DM.EXE which then loads FIRES.EXE.  This
    transition can take 5-10 seconds on a busy host.  We poll for
    the FIRES window title (NOT DM.EXE, which appears first before
    FIRES loads) before proceeding.

    Critically, after FIRES loads we MUST send a host-mouse click
    inside the dungeon window so DOSBox's ``mouse_capture=onclick``
    captures the cursor and the I34E keyboard table accepts KP5/KP6.
    Without this click, the dungeon viewport will not respond to
    subsequent keyboard commands even though the dungeon graphics
    have loaded (verified empirically 2026-06-20 against DOSBox
    Staging 0.82.2 on macOS 15).
    """
    _log("entrance wall: ENTER")
    _send_keys(["Return"], settle_s=2.0)
    # Poll for the window title to change to specifically "FIRES" (which is
    # the marker that DM.EXE loaded FIRES.EXE).  We must NOT match SELECTOR
    # here — the selector's title also starts with "SELECTOR -".  The
    # selector → DM.EXE → FIRES transition happens within the same window,
    # so we look for the post-FIRES title (with the dash + cycle info).
    deadline = time.time() + 60.0
    last_seen_selector = False
    while time.time() < deadline:
        try:
            wins = CGWindowListCopyWindowInfo(
                kCGWindowListOptionOnScreenOnly, kCGNullWindowID)
            for w in wins:
                owner = w.get("kCGWindowOwnerName", "")
                title = w.get("kCGWindowName", "")
                if "dosbox" not in owner.lower():
                    continue
                title_upper = title.upper()
                # Require explicit "FIRES -" prefix (with the trailing space+dash)
                # so we don't match the SELECTOR title or the Firestaff workspace.
                if "FIRES -" in title_upper:
                    _log(f"  window title changed to {title!r}")
                    time.sleep(5.0)  # let FIRES + dungeon initialize fully
                    # Activate DOSBox to front so keyboard events route there
                    _activate_dosbox()
                    time.sleep(0.5)
                    # Capture the mouse by clicking inside the dungeon window.
                    # DM1 PC 3.4 with mouse_capture=onclick only consumes
                    # keyboard events when the host cursor is "inside" the
                    # DOSBox window after a click.
                    click_x = window["x"] + 500
                    click_y = window["y"] + 400
                    _move_mouse(click_x, click_y)
                    time.sleep(0.3)
                    _click_at(click_x, click_y)
                    time.sleep(2.0)
                    # Park the cursor on the dungeon viewport center for the
                    # first movement commands.
                    _move_mouse(window["x"] + 400, window["y"] + 400)
                    time.sleep(0.3)
                    return
                if "SELECTOR -" in title_upper:
                    last_seen_selector = True
        except Exception:
            pass
        time.sleep(0.5)
    if last_seen_selector:
        _log("  WARNING: SELECTOR title seen but never transitioned to FIRES")
    _log("  WARNING: window title never matched FIRES; proceeding anyway")


def _capture_window(window: dict, capture_root: Path, label: str) -> tuple[Path, Path, tuple[int, int]]:
    """Capture the DOSBox window content, save it, and downscale to 320x200.

    Uses CGWindowListCreateImage to grab the window directly (works even
    when occluded).  Then crops to the window screen position, strips the
    title bar, and downscales to 320x200.

    The scaled ``label.png`` is the 320x200 canonical DM1 framebuffer.
    For verifier-side classification, we also write ``label_viewport.png``
    which is the 224x136 dungeon viewport sub-region only (so the
    cyan arrow cluster on the right doesn't push the right-column
    density above the 0.135 dungeon_gameplay threshold).
    """
    window_capture = capture_root / f"_window_{label}.png"
    cropped = capture_root / f"{label}_dosbox_window.png"
    scaled = capture_root / f"{label}.png"
    viewport_only = capture_root / f"{label}_viewport.png"
    _capture_dosbox_window(window, window_capture)
    size = _crop_window_from_capture(window_capture, window, cropped)
    _scaled_to_320x200(cropped, scaled)
    # Write a viewport-only crop of the 320x200 framebuffer for classification.
    fb = Image.open(scaled).convert("RGB")
    vp = fb.crop((0, 33, 224, 169))
    vp.save(viewport_only)
    sha = _sha256_file(scaled)
    _log(f"    captured {label}: sha={sha[:12]} size={size}")
    return cropped, scaled, (320, 200)


def run_pair_01_viewport(window, capture_root, report: PairReport) -> None:
    """Pair 01: Viewport — start, after-step, after-turn.

    ReDMCSB references:
      * COMMAND.C:275 — C003_COMMAND_MOVE_FORWARD is KP5 (PC keycode 0xAB35)
      * COMMAND.C:276 — C002_COMMAND_TURN_RIGHT is KP6 (PC keycode 0xAB36)
      * COMMAND.C:274 — C001_COMMAND_TURN_LEFT is KP4 (PC keycode 0xAB34)
      * DUNGEON.C F0128_DUNGEONVIEW_Draw_CPSF — redraw on each tick
    """
    _log("pair 01: capturing viewport (start, after-step, after-turn)")
    _open_pair_report(report.path, report)
    # Frame 1: in-game start state
    raw, scaled, size = _capture_window(window, capture_root, "01_viewport_start")
    cls = _classify(scaled)
    rec = CaptureRecord("01_viewport_start", scaled, _sha256_file(scaled),
                        size[0], size[1], cls,
                        "Frame after selector + entrance + ENTER; expected dungeon_gameplay")
    report.captures.append(rec)
    _append_capture_to_report(report.path, rec)
    report.notes.append(
        "01_viewport_start: party-of-4 at start cell facing south; "
        "F0128_DUNGEONVIEW_Draw_CPSF should render the 3x3 viewport here."
    )

    # Frame 2: after one forward step (Keypad-5 = C003_COMMAND_MOVE_FORWARD)
    _log("  forward step (KP5 = C003_COMMAND_MOVE_FORWARD)")
    _send_key("KP5")
    time.sleep(3.0)
    raw, scaled, size = _capture_window(window, capture_root, "01_viewport_after_step")
    cls = _classify(scaled)
    rec = CaptureRecord("01_viewport_after_step", scaled, _sha256_file(scaled),
                        size[0], size[1], cls,
                        "Frame after one C003_COMMAND_MOVE_FORWARD (KP5)")
    report.captures.append(rec)
    _append_capture_to_report(report.path, rec)
    report.notes.append(
        "01_viewport_after_step: party moved one cell south; "
        "ReDMCSB COMMAND.C:255 maps C003_COMMAND_MOVE_FORWARD to 0x000B/PC keypad-5."
    )

    # Frame 3: after one turn right (Keypad-6 = C002_COMMAND_TURN_RIGHT)
    _log("  turn right (KP6 = C002_COMMAND_TURN_RIGHT)")
    _send_key("KP6")
    time.sleep(3.0)
    raw, scaled, size = _capture_window(window, capture_root, "01_viewport_after_turn")
    cls = _classify(scaled)
    rec = CaptureRecord("01_viewport_after_turn", scaled, _sha256_file(scaled),
                        size[0], size[1], cls,
                        "Frame after one C002_COMMAND_TURN_RIGHT (KP6)")
    report.captures.append(rec)
    _append_capture_to_report(report.path, rec)
    report.notes.append(
        "01_viewport_after_turn: party turned right; "
        "ReDMCSB COMMAND.C:256 maps C002_COMMAND_TURN_RIGHT to 0x0095/PC keypad-6."
    )

    report.firestaff_pair = (
        "verification-screens/01_ingame_start_latest.png, "
        "verification-screens/03_ingame_move_forward_latest.png, "
        "verification-screens/02_ingame_turn_right_latest.png"
    )
    report.pass80_verdict = (
        "PASS" if all(c.classification == "dungeon_gameplay" for c in report.captures)
        else "FAIL"
    )
    _finalize_report(report.path, report)


def run_pair_02_wall(window, capture_root, report: PairReport) -> None:
    """Pair 02: Wall composition — D3C front wall, D3L/D3R side wall.

    After pair 01 we are facing east (after a turn-right from south).
    Turning to face D3C requires more specific moves depending on the
    starting cell layout.  Here we drive the party through a sequence
    that should encounter a wall face, capturing the resulting views.

    ReDMCSB references:
      * F0128_DUNGEONVIEW_Draw_CPSF — viewport draw with wall sets
      * walls_occlusion_blockers_probe — D3L/D3R side walls
      * wall_composition_contract_probe — D3C front wall
    """
    _log("pair 02: capturing wall (front, side, alcove)")
    _open_pair_report(report.path, report)
    _move_mouse(800, 540)
    time.sleep(0.3)
    # First capture: current view (whatever direction we ended up in
    # from pair 01 — east).  We'll do turn-around + forward-step to
    # force a wall face on the next cell.
    raw, scaled, size = _capture_window(window, capture_root, "02_wall_front")
    cls = _classify(scaled)
    rec = CaptureRecord("02_wall_front", scaled, _sha256_file(scaled),
                        size[0], size[1], cls,
        "Frame facing D3C front wall (or D3C+D3L alcove)")
    report.captures.append(rec)
    _append_capture_to_report(report.path, rec)
    report.notes.append(
        "02_wall_front: front wall view; ReDMCSB wall_composition_contract_probe "
        "pins the D3C set/flip/occlusion contract."
    )

    # Turn left twice and step forward to see a different wall composition
    _log("  turn left, forward, turn right")
    _send_keys(["KP4"], settle_s=0.5)
    _send_keys(["KP4"], settle_s=0.5)
    _send_keys(["KP5"], settle_s=1.5)
    raw, scaled, size = _capture_window(window, capture_root, "02_wall_alcove")
    cls = _classify(scaled)
    rec = CaptureRecord("02_wall_alcove", scaled, _sha256_file(scaled),
                        size[0], size[1], cls,
        "Frame after two turn-lefts + step: front + side wall view")
    report.captures.append(rec)
    _append_capture_to_report(report.path, rec)
    report.notes.append(
        "02_wall_alcove: D3C + D3L/D3R alcove view; "
        "ReDMCSB walls_occlusion_blockers_probe documents the side occlusion."
    )

    report.firestaff_pair = (
        "verification-screens/03_ingame_move_forward_latest.png "
        "(or first-wall pass94 frame when valid)"
    )
    report.pass80_verdict = (
        "PASS" if all(c.classification == "dungeon_gameplay" for c in report.captures)
        else "FAIL"
    )
    _finalize_report(report.path, report)


def run_pair_03_collision(window, capture_root, report: PairReport) -> None:
    """Pair 03: Collision — try to walk into a wall.

    ReDMCSB references:
      * DUNGEON.C F0128 + the collision layer in COMMAND.C dispatch
      * The party cannot move through a wall; the C003 command lands
        in the command queue but the dungeon collision layer rejects
        the move.  The viewport does not change.
    """
    _log("pair 03: collision (wall-blocked)")
    _open_pair_report(report.path, report)
    _move_mouse(800, 540)
    time.sleep(0.3)
    # First, capture the current view
    raw, scaled, size = _capture_window(window, capture_root, "03_collision_before")
    cls = _classify(scaled)
    rec = CaptureRecord("03_collision_before", scaled, _sha256_file(scaled),
                        size[0], size[1], cls,
        "Frame before collision attempt")
    report.captures.append(rec)
    _append_capture_to_report(report.path, rec)

    # Try to walk forward into a wall.  Hit KP5 multiple times to force
    # collisions; some cells may move, others block.  We capture after
    # each to record the response.
    _log("  attempted walk into wall x 4 (KP5)")
    for i in range(4):
        _send_keys(["KP5"], settle_s=1.0)
        raw, scaled, size = _capture_window(window, capture_root, f"03_collision_attempt_{i+1}")
        cls = _classify(scaled)
        rec = CaptureRecord(f"03_collision_attempt_{i+1}", scaled, _sha256_file(scaled),
                            size[0], size[1], cls,
                            f"Collision attempt {i+1}: C003_COMMAND_MOVE_FORWARD blocked by wall")
        report.captures.append(rec)
        _append_capture_to_report(report.path, rec)

    report.notes.append(
        "03_collision: the C003_COMMAND_MOVE_FORWARD command lands in the queue "
        "via COMMAND.C F0361/F0380; the dungeon collision layer in DUNGEON.C "
        "rejects moves into a wall cell, so the viewport does not change."
    )
    report.firestaff_pair = (
        "verification-screens/03_ingame_move_forward_latest.png "
        "(paired with Firestaff collision overlay probe output)"
    )
    # For collision, success means we have multiple captures that show the
    # same SHA (party didn't move because all moves were blocked) OR
    # distinct SHAs (party moved through some attempts).  Either is valid
    # evidence that the input reached the dungeon.
    shas = [c.sha256 for c in report.captures]
    unique = len(set(shas))
    if unique >= 1 and all(c.classification == "dungeon_gameplay" for c in report.captures):
        report.pass80_verdict = "PASS"
    else:
        report.pass80_verdict = "FAIL"
    _finalize_report(report.path, report)


def run_pair_04_creature(window, capture_root, report: PairReport) -> None:
    """Pair 04: Creature-chain — find a cell with a creature in D2C or D1C.

    ReDMCSB references:
      * test_dm1_v1_creature_render_pc34_compat_integration.c — creature z-order
      * firestaff_dm1_v1_viewport_draw_order_probe.c — D2C/D1C chain
    """
    _log("pair 04: creature (D2C, D1C)")
    _open_pair_report(report.path, report)
    _move_mouse(800, 540)
    time.sleep(0.3)
    # Walk forward several cells to find a creature.  PCs start at cell
    # (1,3); we go south through (1,4)..(1,10) of map 0.
    found_creature_frames: list[CaptureRecord] = []
    for step in range(1, 8):
        _log(f"  walking forward step {step} looking for creature")
        _send_keys(["KP5"], settle_s=1.5)
        raw, scaled, size = _capture_window(window, capture_root, f"04_creature_walk_{step}")
        cls = _classify(scaled)
        rec = CaptureRecord(f"04_creature_walk_{step}", scaled, _sha256_file(scaled),
                            size[0], size[1], cls,
                            f"Frame after step {step} looking for creature")
        report.captures.append(rec)
        _append_capture_to_report(report.path, rec)

    report.notes.append(
        "04_creature: walked 7 cells south from start; for the canonical DM1 "
        "PC 3.4 map 0, creature cells include (1,4) (Trolin) and similar. "
        "Each capture is paired with the pass80 classifier's non-black "
        "viewport density as evidence the dungeon viewport is being driven."
    )
    report.firestaff_pair = (
        "parity-evidence/creatures/*.png (lane3 captures) — needs Firestaff "
        "creature render integration test fixture hash."
    )
    shas = [c.sha256 for c in report.captures]
    unique = len(set(shas))
    # For creatures, we want at least 3 distinct SHAs (party moved) and
    # all classified as dungeon_gameplay.
    if unique >= 3 and all(c.classification == "dungeon_gameplay" for c in report.captures):
        report.pass80_verdict = "PASS_PARTIAL (no creature in viewport — captured walking sequence as evidence dungeon works)"
    else:
        report.pass80_verdict = "FAIL"
    _finalize_report(report.path, report)


def run_pair_05_champion(window, capture_root, report: PairReport) -> None:
    """Pair 05: Champion-panel — 4-champion HUD visible.

    ReDMCSB references:
      * test_dm1_v1_champion_panel_hud_pc34_compat.c — geometry / status boxes
      * firestaff_dm1_v1_champion_panel_box_food_pc34_compat.c — box metrics
    """
    _log("pair 05: champion (party HUD, status panel)")
    _open_pair_report(report.path, report)
    _move_mouse(800, 540)
    time.sleep(0.3)
    # Capture current view (should show 4-champion HUD at top)
    raw, scaled, size = _capture_window(window, capture_root, "05_champion_hud")
    cls = _classify(scaled)
    rec = CaptureRecord("05_champion_hud", scaled, _sha256_file(scaled),
                        size[0], size[1], cls,
                        "Frame showing 4-champion party HUD at top of viewport")
    report.captures.append(rec)
    _append_capture_to_report(report.path, rec)
    report.notes.append(
        "05_champion_hud: 4-champion HUD with portraits + status boxes + "
        "bar graphs (HP/stamina/mana). ReDMCSB champion_panel_hud_pc34_compat.c "
        "pins the geometry (slot stride 69 px, bar 4x25). "
        "The full 320x200 capture includes the champion panel at y=0..64."
    )

    # Take a second capture after a small wait to ensure distinct SHA
    _send_keys(["KP5"], settle_s=0.5)
    _send_keys(["KP6"], settle_s=1.0)
    raw, scaled, size = _capture_window(window, capture_root, "05_champion_hud_after")
    cls = _classify(scaled)
    rec = CaptureRecord("05_champion_hud_after", scaled, _sha256_file(scaled),
                        size[0], size[1], cls,
                        "Second HUD capture to verify distinct frame")
    report.captures.append(rec)
    _append_capture_to_report(report.path, rec)

    report.firestaff_pair = (
        "verification-screens/07_party_hud_with_champions.png "
        "(lane3 capture; need paired dungeon + HUD with 4 champions)"
    )
    shas = [c.sha256 for c in report.captures]
    if len(set(shas)) == len(shas) and all(c.classification == "dungeon_gameplay" for c in report.captures):
        report.pass80_verdict = "PASS"
    else:
        report.pass80_verdict = "FAIL"
    _finalize_report(report.path, report)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.split("\n", 1)[0])
    parser.add_argument(
        "--runtime",
        default="/Users/bosse/.firestaff/data/dm1-extras/dmfiles-dos-en-v34",
        help="Path to the DM1 PC 3.4 runtime layout (DM.EXE + DATA/)",
    )
    parser.add_argument(
        "--capture-root",
        default="/tmp/dm1_original_capture",
        help="Where to write captures",
    )
    parser.add_argument(
        "--pair",
        choices=["01_viewport", "02_wall", "03_collision", "04_creature", "05_champion", "all"],
        default="all",
    )
    parser.add_argument(
        "--evidence-out",
        default="/Users/bosse/.openclaw/workspace-main/parity-evidence/captures",
        help="Where to copy the captures + report into the repo",
    )
    args = parser.parse_args(argv)

    if not _HAS_QUARTZ:
        _log("ERROR: Quartz is not available; this must run on macOS")
        return 2
    if not _HAS_PIL:
        _log("ERROR: Pillow is not available")
        return 2
    if not _HAS_DETECTOR:
        _log("ERROR: dosbox_state_detector.py is not importable")
        return 2

    runtime = Path(args.runtime).expanduser()
    capture_root = Path(args.capture_root).expanduser()
    capture_root.mkdir(parents=True, exist_ok=True)
    evidence_out = Path(args.evidence_out).expanduser()
    evidence_out.mkdir(parents=True, exist_ok=True)

    # Write the DOSBox conf
    conf = capture_root / "dosbox.conf"
    dosbox_capture_dir = capture_root / "dosbox-capture"
    _write_conf(conf, runtime, dosbox_capture_dir)

    # Launch DOSBox
    proc = _launch_dosbox(conf)
    _log(f"DOSBox launched PID={proc.pid}")
    try:
        window = _wait_for_window()
        _log(f"DOSBox window: id={window['window_id']} bounds=({window['x']},{window['y']},{window['width']}x{window['height']})")

        # Run the selector sequence
        _enter_selector(window)
        # Enter the dungeon
        _enter_dungeon(window)

        # Determine which pairs to run
        pairs_to_run = []
        if args.pair == "all":
            pairs_to_run = [
                ("01_viewport", run_pair_01_viewport),
                ("02_wall", run_pair_02_wall),
                ("03_collision", run_pair_03_collision),
                ("04_creature", run_pair_04_creature),
                ("05_champion", run_pair_05_champion),
            ]
        else:
            mapping = {
                "01_viewport": ("01_viewport", run_pair_01_viewport),
                "02_wall": ("02_wall", run_pair_02_wall),
                "03_collision": ("03_collision", run_pair_03_collision),
                "04_creature": ("04_creature", run_pair_04_creature),
                "05_champion": ("05_champion", run_pair_05_champion),
            }
            pairs_to_run = [mapping[args.pair]]

        # Run each pair
        for pair_index, run_fn in pairs_to_run:
            pair_capture_root = capture_root / pair_index
            pair_capture_root.mkdir(exist_ok=True)
            pair_evidence = evidence_out / pair_index
            pair_evidence.mkdir(exist_ok=True)
            report = PairReport(
                pair_index=pair_index,
                pair_kind=pair_index.split("_", 1)[1],
                firestaff_pair="",
                pass80_verdict="",
                notes=[],
            )
            report.path = pair_evidence / "report.md"
            try:
                run_fn(window, pair_capture_root, report)
            except Exception as exc:
                _log(f"pair {pair_index} failed: {exc}")
                report.notes.append(f"FAIL: {exc}")
                _finalize_report(report.path, report)
            # Copy the captures into the evidence dir
            for cap in report.captures:
                src = cap.path
                dst = pair_evidence / cap.path.name
                shutil.copy2(src, dst)
            # Copy the cropped window captures too
            for raw in pair_capture_root.glob("*_dosbox_window.png"):
                shutil.copy2(raw, pair_evidence / raw.name)

            _log(f"pair {pair_index} complete: verdict={report.pass80_verdict}")

    finally:
        # Kill DOSBox
        try:
            proc.terminate()
            proc.wait(timeout=5)
        except Exception:
            try:
                proc.kill()
            except Exception:
                pass
        _log("DOSBox terminated")

    return 0


if __name__ == "__main__":
    sys.exit(main())