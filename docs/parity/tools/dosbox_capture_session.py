#!/usr/bin/env python3
"""dosbox_capture_session.py — State machine runner for DOSBox DM1 capture.

This is the runnable companion to
`docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md` §3.  The runbook
describes the full state machine; this script provides an executable
scaffold that:

* Runs in `--dry-run` mode without DOSBox, walking the state machine
  transitions and asserting the classifier can pick up each one in
  order.  This is the regression guard for the pass94 failure mode
  where a working classifier would have caught the route bug at the
  `unclassified` step instead of after a full session.  The dry-run
  also exercises the focus-mismatch rawshot-fallback recovery gate
  (see ``_classify_rawshot_focus_recovery`` / ``_attempt_focus_recovery``)
  so a future patch that breaks the recovery classification or the
  ``dosbox_capture.focus_recovery.json`` schema is caught at the
  ``dm1_v1_original_capture_session_dry_run`` CTest gate instead of
  at the next live DOSBox attempt.
* Runs in `--plan` mode to dump the planned key sequence, expected
  state after each key, and timeout budget to stdout.  Useful for
  hand-running a session.
* Runs in `--live` mode to drive a real DOSBox Staging session.
  This is a thin wrapper over the documented behaviour and inherits
  the same `cliclick`-based key dispatch from the runbook.  When the
  focus-mismatch window fills, the live route triggers a
  ``dosbox-rawshot`` recovery probe (DOSBox's own Ctrl+F5 capture,
  which is independent of macOS window focus) and writes
  ``dosbox_capture.focus_recovery.json`` so a future operator can see
  whether the rawshot path saved the focus window or gave up.  The
  live abort receipt (``dosbox_capture.live_abort.json``) also
  carries a compact ``focus_recovery`` summary so the focus-recovery
  outcome is visible from the existing abort-receipt flow.

The script intentionally does not depend on `cliclick` being installed
for the `--dry-run` and `--plan` modes.  The capture pipeline only
needs it for `--live` mode on macOS.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass, field, asdict
from pathlib import Path
from typing import Callable, Optional

# Local import: the state detector is the contract for what counts as a
# successful state transition.  Keeping the import lazy so a missing
# Pillow install only blocks the live mode, not --dry-run.
try:
    from dosbox_state_detector import classify, _NONBLACK_THRESH  # noqa: F401
    _DETECTOR_OK = True
except Exception:  # pragma: no cover - reported at runtime
    _DETECTOR_OK = False


# Plan step: one keypress (or set of keypresses) followed by an
# expected state.  The state machine in run() walks the plan in order
# and asserts that the classifier eventually returns the expected
# state for each step.  In --live mode the plan is the keypress
# schedule; in --dry-run mode it is a sequence of synthetic frames
# that are fed into the classifier.
@dataclass
class PlanStep:
    name: str
    expected_state: str
    keys: list[str] = field(default_factory=list)
    timeout_s: float = 60.0
    stable_frames: int = 3
    # When True the live route does NOT gate this step on the 4-state
    # density classifier; it sends the step's keys, dwells ``settle_s``
    # for a deterministic capture, and proceeds.  This is for DM.EXE's
    # SELECTOR text menus (graphics/sound/input), which are not dungeon
    # framebuffers: their text density lands inconsistently across the
    # entrance_menu/title_screen buckets (graphics page ≈ entrance_menu,
    # sound page ≈ title_screen), so classifier gating there produced
    # false stalls even though the digit+Return navigation was working.
    # The dry-run still validates settle_only steps against their
    # ``expected_state`` fixture, so the state machine stays regression
    # tested without requiring a live classifier match mid-SELECTOR.
    settle_only: bool = False
    settle_s: float = 2.5


@dataclass
class FrameQuality:
    label: str
    state: str
    width: int
    height: int
    full_nonblack: float
    viewport_nonblack: float
    rightcol_nonblack: float
    champion_nonblack: float
    blackout: bool
    capture_backend: str = "unknown"
    capture_source: str = ""
    capture_source_sha256: str = ""
    normalized_rgb_sha256: str = ""
    host_active_app: str = ""
    dosbox_window_bounds: str = ""


@dataclass
class KeyDispatch:
    step: str
    key: str
    mapped: str
    host_active_app_before: str
    host_active_app_after: str
    dosbox_window_bounds_before: str
    dosbox_window_bounds_after: str
    timestamp_s: float
    ok: bool
    error: str = ""


# The capture route plan.  This mirrors the runbook §3 state machine
# but uses the calibrated band 0.135 thresholds from the post-fix
# classifier.  Stable-frames requirement follows the runbook default.
#
# Verified 2026-06-13 against a real DOSBox Staging 0.82.2 DM1 PC 3.4 boot
# (rendered Alt+F5 internal capture, see _trigger_dosbox_internal_screenshot):
#   * DM.EXE boots straight into the SELECTOR's *graphics* text menu
#     ("1.[*]VGA 2.[*]EGA 3.[ ]Tandy Q.[*]Quit") with no black title
#     screen first.  That selector frame classifies as ``entrance_menu``
#     (v≈0.38 high text density, r≈0.21), NOT ``title_screen``.  The old
#     plan's leading ``title_screen`` step is why every live run stalled
#     at step 1: it waited 60 s for a black screen that never comes.
#   * Each selector page (graphics, then sound, then input/control) is
#     advanced by typing a DIGIT and pressing Return.  A bare Return is
#     rejected with "Invalid selection."; the digit must be submitted
#     with Return.  The route picks VGA → No Sound → Keyboard Simulation
#     of Digital Joystick.  The latter is deliberate: the earlier Mouse
#     selection reaches ``dungeon_gameplay`` and accepts the entrance
#     click, but it leaves post-entry C003 keyboard movement ignored.
#   * After the three selector pages the "Dungeon Master" title art and
#     then the ENTER/RESUME/QUIT entrance wall appear.  With Keyboard
#     Simulation of Digital Joystick selected, the wall's selector cursor
#     starts on ENTER.  Once the live route launches DM.EXE through the
#     app binary itself instead of the Homebrew wrapper, Return reliably
#     activates it and avoids a host-mouse-capture dependency.  The dungeon
#     corridor viewport then classifies as ``dungeon_gameplay``
#     (v≈0.93, r≈0.12 < 0.135), which is the capture target.
# The entrance transition is the historically hard part of this route;
# everything up to and including it is now a deterministic,
# capture-verified sequence.
DEFAULT_PLAN: list[PlanStep] = [
    PlanStep("graphics_select",  "entrance_menu",    keys=["1", "Return"], settle_only=True, settle_s=3.0),
    PlanStep("sound_select",     "entrance_menu",    keys=["1", "Return"], settle_only=True, settle_s=3.0),
    PlanStep("input_select",     "entrance_menu",    keys=["4", "Return"], settle_only=True, settle_s=4.0),
    PlanStep("entrance_wall",    "entrance_menu",    settle_only=True, settle_s=4.0),
    PlanStep("enter_dungeon",    "dungeon_gameplay", keys=["Return"], timeout_s=60.0),
    PlanStep("dungeon_gameplay", "dungeon_gameplay", timeout_s=120.0),
]

KEY_MAP = {
    "Return": "return",
    "Key-Up": "arrow-up",
    "Key-Down": "arrow-down",
    "Key-Left": "arrow-left",
    "Key-Right": "arrow-right",
    "Keypad-5": "keypad-5",
}

# Pseudo-key for the one mouse click that crosses the DM entrance wall into
# the dungeon.  This is not a keyboard key: the live route maps it to a
# cliclick on the ENTER target on the right-hand stone wall.  Kept as a
# named pseudo-key so the plan, key-dispatch log, and self-tests can refer
# to it without special-casing raw coordinates.
ENTRANCE_ENTER_CLICK_KEY = "entrance_enter_click"
# Fractional position of the ENTER button inside the DOSBox *content* area
# (below the macOS title bar), measured from a real DM1 PC 3.4 entrance-wall
# capture (rendered framebuffer 799x599, ENTER center ~ (696, 160)).  Using
# fractions of the content rectangle keeps the click correct across window
# sizes and Retina scale factors.
ENTRANCE_ENTER_CLICK_FRAC = (696.0 / 799.0, 160.0 / 599.0)
# Height of the macOS window title bar (points) to skip when mapping a
# framebuffer-fractional target to an absolute on-screen click.
MACOS_WINDOW_TITLEBAR_H = 28

# Pseudo-key for the first in-dungeon movement proof.  ReDMCSB PC COMMAND.C
# lines 396-405 maps the movement arrow panel through screen-relative zones;
# line 398 maps C003_COMMAND_MOVE_FORWARD to C070_ZONE_MOVE_FORWARD.  The PC
# coordinate box is x=263..289, y=125..145, so this uses the center of that
# source zone in the normalized 320x200 framebuffer.  The live movement
# receipt records this source fact and proves the before/after viewport hash
# changed; the proprietary frames themselves stay in the operator-local
# capture root.
DUNGEON_MOVE_FORWARD_CLICK_KEY = "dungeon_move_forward_click"
DUNGEON_MOVE_FORWARD_CLICK_SCREEN_COORD = (276, 135)
DUNGEON_MOVE_FORWARD_CLICK_FRAC = (
    DUNGEON_MOVE_FORWARD_CLICK_SCREEN_COORD[0] / 320.0,
    DUNGEON_MOVE_FORWARD_CLICK_SCREEN_COORD[1] / 200.0,
)
DUNGEON_MOVE_FORWARD_SOURCE = (
    "ReDMCSB COMMAND.C:396-405 / line 398: "
    "C003_COMMAND_MOVE_FORWARD -> C070_ZONE_MOVE_FORWARD, "
    "screen-relative x=263..289 y=125..145"
)
DUNGEON_MOVE_FORWARD_KEYBOARD_KEY = "Keypad-5"
DUNGEON_MOVE_FORWARD_KEYBOARD_SOURCE = (
    "ReDMCSB COMMAND.C:275-281: "
    "C003_COMMAND_MOVE_FORWARD is also bound to numeric keypad 5 and "
    "Up Arrow (<CSI>A / <CSI>T) in the PC movement keyboard table"
)

DOSBOX_PROCESS_NAMES = ("dosbox", "dosbox-staging", "DOSBox Staging", "DOSBox")
DOSBOX_BIN_CANDIDATES = ("dosbox-staging", "dosbox")
MACOS_DOSBOX_STAGING_APP_BIN = Path("/Applications/DOSBox Staging.app/Contents/MacOS/dosbox")
# macOS application-bundle names that ``open -a NAME`` can activate to raise
# an already-running DOSBox window.  Empirically (DOSBox Staging 0.82.2 on
# macOS 15), launching the inner ``Contents/MacOS/dosbox`` binary directly via
# subprocess.Popen leaves the window behind Terminal: AppleScript
# ``set frontmost to true`` alone does NOT raise it (the frontmost process
# stays "Terminal"), so every cliclick/osascript keystroke is delivered to the
# wrong app and DOSBox never sees Ctrl+F5 or the route keys.  ``open -a`` does
# reliably activate the running instance without spawning a duplicate, which is
# the focus half of the original-capture blocker.
MACOS_DOSBOX_OPEN_APP_NAMES = ("DOSBox Staging",)
BLACKOUT_NONBLACK_THRESH = 0.005
RUNTIME_DATA_REQUIRED_FILES = ("GRAPHICS.DAT", "DUNGEON.DAT")
LIVE_INPUT_RECEIPT_SCHEMA = "firestaff.dosbox_capture_session.live_inputs.v1"
LIVE_ABORT_RECEIPT_SCHEMA = "firestaff.dosbox_capture_session.abort.v1"
LIVE_FOCUS_RECOVERY_RECEIPT_SCHEMA = "firestaff.dosbox_capture_session.focus_recovery.v1"
LIVE_MOVEMENT_RECEIPT_SCHEMA = "firestaff.dosbox_capture_session.in_dungeon_movement.v1"
FOCUS_MISMATCH_FRAME_LIMIT = 4
RAWSHOT_FOCUS_RECOVERY_REASONS = (
    "rawshot_focus_recovered",
    "rawshot_focus_unrecoverable",
    "no_focus_recovery_needed",
)
DOSBOX_INTERNAL_CAPTURE_GLOBS = ("*.png", "*.bmp", "*.raw")
LIVE_DOSBOX_STARTUP_SETTLE_S = 8.0
DosboxCaptureSignature = tuple[int, int]


class QuietRunTimeout:
    returncode = 124
    stdout = ""
    stderr = "timeout"


def _resolve_dosbox_bin(args: argparse.Namespace) -> str | None:
    """Return the DOSBox executable used by validation and live launch.

    The runbook requires DOSBox Staging, but local installs are split
    between a ``dosbox-staging`` executable, a ``dosbox`` shim, and
    explicit app-wrapper paths.  Resolving this once keeps
    ``--validate-live-inputs`` from passing a different launch shape
    than ``--live`` actually uses.
    """
    explicit = getattr(args, "dosbox_bin", None)
    if explicit:
        expanded = os.path.expanduser(str(explicit))
        if os.path.sep in expanded:
            return expanded if _is_executable_file(Path(expanded)) else None
        return shutil.which(expanded)
    env_bin = os.environ.get("DOSBOX_BIN")
    if env_bin:
        expanded = os.path.expanduser(env_bin)
        if os.path.sep in expanded:
            return expanded if _is_executable_file(Path(expanded)) else None
        found = shutil.which(expanded)
        if found:
            return found
    if _is_executable_file(MACOS_DOSBOX_STAGING_APP_BIN):
        return str(MACOS_DOSBOX_STAGING_APP_BIN)
    for candidate in DOSBOX_BIN_CANDIDATES:
        found = shutil.which(candidate)
        if found:
            return found
    return None


def _dosbox_conf_command(
        dosbox_bin: str,
        conf: Path,
        runtime_dir: Path | None = None) -> list[str]:
    """Return the argv needed to launch DOSBox with ``conf``.

    Homebrew ``dosbox-staging`` expects the long ``--conf`` option, while
    DOSBox-X and older app-wrapper launches still accept the historical
    ``-conf`` spelling used by earlier capture scripts.  The live runtime
    launch passes DM.EXE as DOSBox's PATH argument; DOSBox Staging mounts
    the executable's parent as C: and runs it without depending on
    wrapper-sensitive [autoexec] or ``-c`` command forwarding.
    """
    bin_text = str(dosbox_bin).casefold()
    conf_flag = "--conf" if (
        "dosbox-staging" in bin_text or "dosbox staging.app" in bin_text
    ) else "-conf"
    argv = [dosbox_bin, conf_flag, str(conf)]
    if runtime_dir is not None:
        argv.append(str(runtime_dir / "DM.EXE"))
    return argv


def _is_executable_file(path: Path) -> bool:
    return path.is_file() and os.access(path, os.X_OK)


def _has_case_insensitive_child(directory: Path, name: str) -> bool:
    """Return whether ``directory`` contains ``name`` under DOS semantics."""
    direct = directory / name
    if direct.is_file():
        return True
    if not directory.is_dir():
        return False
    target = name.casefold()
    try:
        return any(child.is_file() and child.name.casefold() == target
                   for child in directory.iterdir())
    except OSError:
        return False


def _runtime_dir_failures(runtime_dir: Path) -> list[str]:
    """Return validation failures for the DOS runtime layout.

    The canonical hash root proved by preflight is not always the
    directory DOSBox can execute from.  The live route must mount the
    extracted PC 3.4 runtime subdir where DM.EXE can see its DATA/
    sibling and the required DAT payloads inside it; mounting the
    parent hash root or an incomplete extraction can leave DOSBox at a
    menu or a black host capture even though the SHA preflight passed.
    """
    failures: list[str] = []
    if not runtime_dir.is_dir():
        return [f"runtime dir is not a directory: {runtime_dir}"]
    if not (runtime_dir / "DM.EXE").is_file():
        nested = runtime_dir / "DungeonMasterPC34" / "DM.EXE"
        if nested.is_file():
            failures.append(
                "--runtime-dir points at the parent game root; pass the "
                f"inner runtime dir instead: {nested.parent}"
            )
        else:
            failures.append(f"runtime dir missing DM.EXE: {runtime_dir}")
    data_dir = runtime_dir / "DATA"
    if not data_dir.is_dir():
        failures.append(f"runtime dir missing DATA/ sibling: {runtime_dir}")
    else:
        for filename in RUNTIME_DATA_REQUIRED_FILES:
            if not _has_case_insensitive_child(data_dir, filename):
                failures.append(
                    f"runtime DATA/ missing required {filename}: {data_dir}"
                )
    return failures


def validate_live_inputs(args: argparse.Namespace) -> int:
    """Validate the live route inputs without launching DOSBox."""
    capture_root = args.capture_root.expanduser()
    default_runtime_dir = Path(
        "~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
    ).expanduser()
    runtime_dir = (
        args.runtime_dir.expanduser()
        if args.runtime_dir is not None
        else default_runtime_dir
    )
    failures = _runtime_dir_failures(runtime_dir)
    dosbox_bin = _resolve_dosbox_bin(args)
    if dosbox_bin is None:
        failures.append(
            "DOSBox executable not found; pass --dosbox-bin, set DOSBOX_BIN, "
            "or put dosbox-staging/dosbox on PATH"
        )
    if failures:
        print("FAIL live input validation:")
        for failure in failures:
            print(f"  - {failure}")
        return 1
    conf = capture_root / "dosbox_capture.live.conf"
    _write_live_conf(conf, runtime_dir, capture_root)
    receipt = _write_live_input_receipt(
        capture_root=capture_root,
        runtime_dir=runtime_dir,
        dosbox_bin=dosbox_bin,
        conf=conf,
        capture_backend=getattr(args, "capture_backend", "auto"),
    )
    print(
        "PASS live input validation: "
        f"dosbox_bin={dosbox_bin} runtime={runtime_dir} "
        f"conf={conf} receipt={receipt}"
    )
    return 0


def dump_plan(plan: list[PlanStep], out: Path | None = None) -> None:
    """Render the plan to JSON (default) or stdout."""
    rendered = json.dumps(
        [asdict(step) for step in plan],
        indent=2,
    )
    if out is not None:
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(rendered, encoding="utf-8")
        print(f"wrote plan: {out}")
    else:
        print(rendered)


def dry_run(plan: list[PlanStep],
            sample_factory: Optional[Callable[[str], object]] = None
            ) -> tuple[int, int, list[str]]:
    """Walk the plan, asserting the classifier can pick up each step.

    ``sample_factory`` builds a synthetic PIL image for a given state
    name.  The default factory uses dosbox_state_detector's synth
    fixtures so this is regression-testable in CI without DOSBox.

    Returns (matched, total, failures).
    """
    if not _DETECTOR_OK:
        return 0, 0, ["Pillow is required for --dry-run"]
    if sample_factory is None:
        from dosbox_state_detector import selftest_synthetic_states
        fixtures = {label: img for label, _expected, img in selftest_synthetic_states()}
        def sample_factory(state: str):
            # Map the plan's expected state to a synthetic image.
            # The plan step name usually matches the state name, but
            # some intermediate plan steps (graphics_select, sound_select,
            # start_game) re-use the entrance_menu state.  Fall back to
            # the entrance_menu fixture in that case.
            return fixtures.get(state, fixtures.get("entrance_menu"))
    failures: list[str] = []
    matched = 0
    total = 0
    for step in plan:
        total += 1
        img = sample_factory(step.expected_state)
        if img is None:
            failures.append(f"{step.name}: no synthetic fixture for {step.expected_state}")
            continue
        actual = classify(img)
        if actual == step.expected_state:
            matched += 1
        else:
            failures.append(
                f"{step.name}: classifier returned {actual!r}, expected {step.expected_state!r}"
            )
    blackout_quality = _frame_quality(sample_factory("title_screen"), "blackout_fixture", "title_screen")
    if not blackout_quality.blackout:
        failures.append("blackout fixture: quality gate did not detect an all-black frame")
    dungeon_quality = _frame_quality(
        sample_factory("dungeon_gameplay"),
        "dungeon_fixture",
        "dungeon_gameplay",
    )
    if dungeon_quality.blackout:
        failures.append("dungeon fixture: quality gate misdetected a visible frame as blackout")
    blackouts = [blackout_quality] * 6
    if not _should_abort_for_blackout(blackouts, "entrance_menu", 6):
        failures.append("blackout guard: did not abort a non-title target after repeated black frames")
    if not _should_abort_for_blackout(blackouts, "title_screen", 6):
        failures.append("blackout guard: did not abort repeated all-black title captures")
    focus_mismatches = [
        FrameQuality(
            label=f"focus_mismatch_{i:04d}",
            state="unknown",
            width=320,
            height=200,
            full_nonblack=0.5,
            viewport_nonblack=0.5,
            rightcol_nonblack=0.5,
            champion_nonblack=0.5,
            blackout=False,
            capture_backend="fixture",
            capture_source="fixture:screen",
            host_active_app="Terminal",
            dosbox_window_bounds="",
        )
        for i in range(FOCUS_MISMATCH_FRAME_LIMIT)
    ]
    if not _should_abort_for_focus_mismatch(
            focus_mismatches, FOCUS_MISMATCH_FRAME_LIMIT):
        failures.append("focus guard: did not abort repeated non-DOSBox frontmost samples")
    focused_samples = list(focus_mismatches[:-1]) + [
        FrameQuality(
            label="focus_recovered",
            state="unknown",
            width=320,
            height=200,
            full_nonblack=0.5,
            viewport_nonblack=0.5,
            rightcol_nonblack=0.5,
            champion_nonblack=0.5,
            blackout=False,
            capture_backend="fixture",
            capture_source="fixture:screen",
            host_active_app="DOSBox Staging",
            dosbox_window_bounds="1,2,1024,768",
        )
    ]
    if _should_abort_for_focus_mismatch(
            focused_samples, FOCUS_MISMATCH_FRAME_LIMIT):
        failures.append("focus guard: aborted after DOSBox focus recovered")
    no_mismatch_window = [
        FrameQuality(
            label=f"focus_ok_{i:04d}",
            state="unknown",
            width=320,
            height=200,
            full_nonblack=0.5,
            viewport_nonblack=0.5,
            rightcol_nonblack=0.5,
            champion_nonblack=0.5,
            blackout=False,
            capture_backend="fixture",
            capture_source="fixture:screen",
            host_active_app="DOSBox Staging",
            dosbox_window_bounds="1,2,1024,768",
        )
        for i in range(FOCUS_MISMATCH_FRAME_LIMIT)
    ]
    if _classify_rawshot_focus_recovery(
            no_mismatch_window,
            rawshot_meta={"capture_backend": "dosbox-rawshot"},
            rawshot_visible=True,
    ) != "no_focus_recovery_needed":
        failures.append(
            "focus recovery classifier: did not return no_focus_recovery_needed "
            "for a clean DOSBox-frontmost window"
        )
    if _classify_rawshot_focus_recovery(
            focus_mismatches,
            rawshot_meta=None,
            rawshot_visible=False,
    ) != "rawshot_focus_unrecoverable":
        failures.append(
            "focus recovery classifier: did not return rawshot_focus_unrecoverable "
            "for a focus-mismatched window with no rawshot attempt"
        )
    if _classify_rawshot_focus_recovery(
            focus_mismatches,
            rawshot_meta={"capture_backend": "dosbox-rawshot"},
            rawshot_visible=False,
    ) != "rawshot_focus_unrecoverable":
        failures.append(
            "focus recovery classifier: did not return rawshot_focus_unrecoverable "
            "for a focus-mismatched window whose rawshot was blackout"
        )
    if _classify_rawshot_focus_recovery(
            focus_mismatches,
            rawshot_meta={"capture_backend": "peekaboo"},
            rawshot_visible=True,
    ) != "rawshot_focus_unrecoverable":
        failures.append(
            "focus recovery classifier: did not return rawshot_focus_unrecoverable "
            "when the rawshot_meta backend is not dosbox-rawshot"
        )
    if _classify_rawshot_focus_recovery(
            focus_mismatches,
            rawshot_meta={"capture_backend": "dosbox-rawshot"},
            rawshot_visible=True,
    ) != "rawshot_focus_recovered":
        failures.append(
            "focus recovery classifier: did not return rawshot_focus_recovered "
            "for a focus-mismatched window whose rawshot decoded as visible"
        )
    if "rawshot_focus_recovered" not in RAWSHOT_FOCUS_RECOVERY_REASONS:
        failures.append(
            "focus recovery reasons: rawshot_focus_recovered missing from "
            "RAWSHOT_FOCUS_RECOVERY_REASONS"
        )
    with tempfile.TemporaryDirectory(prefix="dm1-focus-recovery-") as focus_tmp:
        focus_root = Path(focus_tmp) / "capture-root"
        try:
            from PIL import Image, ImageDraw
            visible = Image.new("RGB", (320, 200), (0, 0, 0))
            draw = ImageDraw.Draw(visible)
            draw.rectangle((0, 33, 223, 168), fill=(96, 80, 48))

            def _visible_rawshot_probe(raw: Path):
                visible.save(raw)
                return visible, {
                    "capture_backend": "dosbox-rawshot",
                    "capture_source": str(focus_root / "dosbox-capture" / "fixture.raw"),
                    "capture_source_sha256": "deadbeef" * 8,
                    "host_active_app": "DOSBox Staging",
                    "dosbox_window_bounds": "1,2,1024,768",
                }

            def _identity_normalize(img):
                return img

            reason, quality, meta = _attempt_focus_recovery(
                focus_root, focus_mismatches,
                probe_factory=_visible_rawshot_probe,
                normalize_factory=_identity_normalize,
            )
            if reason != "rawshot_focus_recovered":
                failures.append(
                    "focus recovery attempt: did not return rawshot_focus_recovered "
                    f"for a visible rawshot probe (got {reason!r})"
                )
            if meta is None or meta.get("capture_backend") != "dosbox-rawshot":
                failures.append("focus recovery attempt: did not propagate rawshot_meta")
            if quality is None or quality.blackout:
                failures.append("focus recovery attempt: did not produce a non-blackout quality")
            receipt_path = _write_focus_recovery_receipt(
                focus_root, reason, focus_mismatches, quality, meta,
            )
            try:
                receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
            except Exception as exc:
                failures.append(f"focus recovery receipt: could not parse JSON: {exc}")
            else:
                if receipt.get("schema") != LIVE_FOCUS_RECOVERY_RECEIPT_SCHEMA:
                    failures.append("focus recovery receipt: schema mismatch")
                if receipt.get("reason") != "rawshot_focus_recovered":
                    failures.append("focus recovery receipt: reason mismatch")
                if not receipt.get("rawshot_attempted"):
                    failures.append("focus recovery receipt: rawshot_attempted flag missing")
                if not receipt.get("rawshot_visible"):
                    failures.append("focus recovery receipt: rawshot_visible flag missing")
                if receipt.get("trigger_quality") is None:
                    failures.append("focus recovery receipt: trigger_quality missing")
                if receipt.get("focus_mismatch_frame_limit") != FOCUS_MISMATCH_FRAME_LIMIT:
                    failures.append("focus recovery receipt: focus_mismatch_frame_limit mismatch")

            black = Image.new("RGB", (320, 200), (0, 0, 0))

            def _blackout_rawshot_probe(raw: Path):
                black.save(raw)
                return black, {
                    "capture_backend": "dosbox-rawshot",
                    "capture_source": str(focus_root / "dosbox-capture" / "black.raw"),
                    "host_active_app": "DOSBox Staging",
                    "dosbox_window_bounds": "1,2,1024,768",
                }

            reason, quality, meta = _attempt_focus_recovery(
                focus_root, focus_mismatches,
                probe_factory=_blackout_rawshot_probe,
                normalize_factory=_identity_normalize,
            )
            if reason != "rawshot_focus_unrecoverable":
                failures.append(
                    "focus recovery attempt: did not return rawshot_focus_unrecoverable "
                    f"for a blackout rawshot probe (got {reason!r})"
                )
            if meta is None or meta.get("capture_backend") != "dosbox-rawshot":
                failures.append("focus recovery attempt (blackout): did not propagate rawshot_meta")
            if quality is None or not quality.blackout:
                failures.append("focus recovery attempt (blackout): did not detect blackout")

            def _raise_rawshot_probe(raw: Path):
                raise RuntimeError("simulated Ctrl+F5 failure")

            reason, quality, meta = _attempt_focus_recovery(
                focus_root, focus_mismatches,
                probe_factory=_raise_rawshot_probe,
                normalize_factory=_identity_normalize,
            )
            if reason != "rawshot_focus_unrecoverable":
                failures.append(
                    "focus recovery attempt: did not return rawshot_focus_unrecoverable "
                    f"when the rawshot probe raised (got {reason!r})"
                )
            if meta is None or "capture_error" not in meta:
                failures.append(
                    "focus recovery attempt (raised): did not capture the probe error in meta"
                )

            reason, quality, meta = _attempt_focus_recovery(
                focus_root, no_mismatch_window,
                probe_factory=_visible_rawshot_probe,
                normalize_factory=_identity_normalize,
            )
            if reason != "no_focus_recovery_needed":
                failures.append(
                    "focus recovery attempt: did not return no_focus_recovery_needed "
                    f"for a clean DOSBox-frontmost window (got {reason!r})"
                )

        except Exception as exc:
            failures.append(f"focus recovery self-test: unexpected exception: {exc}")

    with tempfile.TemporaryDirectory(prefix="dm1-focus-recovery-abort-") as abort_tmp:
        abort_root = Path(abort_tmp) / "capture-root"
        try:
            summary = _focus_recovery_summary(abort_root)
            if summary is not None:
                failures.append("focus recovery summary: returned non-None for a missing receipt")
            from PIL import Image, ImageDraw
            visible = Image.new("RGB", (320, 200), (0, 0, 0))
            draw = ImageDraw.Draw(visible)
            draw.rectangle((0, 33, 223, 168), fill=(96, 80, 48))
            (abort_root / "state-samples").mkdir(parents=True, exist_ok=True)
            visible.save(str(abort_root / "state-samples" / "placeholder.png"))
            visible_quality = FrameQuality(
                label="placeholder",
                state="unknown",
                width=320,
                height=200,
                full_nonblack=0.5,
                viewport_nonblack=0.5,
                rightcol_nonblack=0.5,
                champion_nonblack=0.5,
                blackout=False,
                capture_backend="dosbox-rawshot",
                capture_source="fixture:rawshot",
                host_active_app="DOSBox Staging",
            )
            _write_focus_recovery_receipt(
                abort_root, "rawshot_focus_recovered", focus_mismatches,
                visible_quality, {"capture_backend": "dosbox-rawshot"},
            )
            summary = _focus_recovery_summary(abort_root)
            if summary is None:
                failures.append("focus recovery summary: returned None for a present receipt")
            elif summary.get("reason") != "rawshot_focus_recovered":
                failures.append("focus recovery summary: reason mismatch")
            elif not summary.get("rawshot_visible"):
                failures.append("focus recovery summary: rawshot_visible flag missing")
            step = PlanStep("entrance_menu", "entrance_menu", keys=["Return"], timeout_s=30.0)
            abort_path = _write_live_abort_receipt(
                abort_root, step, "capture_focus_mismatch", "capture_focus_mismatch",
            )
            abort_receipt = json.loads(abort_path.read_text(encoding="utf-8"))
            fr = abort_receipt.get("focus_recovery")
            if not isinstance(fr, dict) or fr.get("reason") != "rawshot_focus_recovered":
                failures.append("live abort receipt: focus_recovery summary not propagated")
        except Exception as exc:
            failures.append(f"focus recovery abort-receipt self-test: unexpected exception: {exc}")
    with tempfile.TemporaryDirectory(prefix="dm1-live-runtime-") as tmp:
        tmp_root = Path(tmp)
        good = tmp_root / "DungeonMasterPC34"
        good.mkdir()
        (good / "DM.EXE").write_bytes(b"DM_FIXTURE\n")
        good_data = good / "DATA"
        good_data.mkdir()
        (good_data / "GRAPHICS.DAT").write_bytes(b"GRAPHICS_FIXTURE\n")
        (good_data / "DUNGEON.DAT").write_bytes(b"DUNGEON_FIXTURE\n")
        if _runtime_dir_failures(good):
            failures.append("runtime layout guard: rejected valid DM.EXE + DATA/ layout")
        parent = tmp_root / "parent"
        nested = parent / "DungeonMasterPC34"
        nested.mkdir(parents=True)
        (nested / "DM.EXE").write_bytes(b"DM_FIXTURE\n")
        if not any("inner runtime dir" in f for f in _runtime_dir_failures(parent)):
            failures.append("runtime layout guard: did not flag parent game-root layout")
        missing_data = tmp_root / "missing_data"
        missing_data.mkdir()
        (missing_data / "DM.EXE").write_bytes(b"DM_FIXTURE\n")
        if not any("DATA/" in f for f in _runtime_dir_failures(missing_data)):
            failures.append("runtime layout guard: did not require DATA/ sibling")
        incomplete_data = tmp_root / "incomplete_data"
        incomplete_data.mkdir()
        (incomplete_data / "DM.EXE").write_bytes(b"DM_FIXTURE\n")
        (incomplete_data / "DATA").mkdir()
        if not any("GRAPHICS.DAT" in f for f in _runtime_dir_failures(incomplete_data)):
            failures.append("runtime layout guard: did not require DATA/GRAPHICS.DAT")
        lowercase_data = tmp_root / "lowercase_data"
        lowercase_data.mkdir()
        (lowercase_data / "DM.EXE").write_bytes(b"DM_FIXTURE\n")
        lowercase_data_dir = lowercase_data / "DATA"
        lowercase_data_dir.mkdir()
        (lowercase_data_dir / "graphics.dat").write_bytes(b"GRAPHICS_FIXTURE\n")
        (lowercase_data_dir / "dungeon.dat").write_bytes(b"DUNGEON_FIXTURE\n")
        if _runtime_dir_failures(lowercase_data):
            failures.append("runtime layout guard: rejected DOS-style lowercase DAT names")
        fake_bin = tmp_root / "fake-dosbox-staging"
        fake_bin.write_bytes(b"#!/bin/sh\nexit 0\n")
        fake_bin.chmod(0o755)
        if _resolve_dosbox_bin(argparse.Namespace(dosbox_bin=fake_bin)) != str(fake_bin):
            failures.append("dosbox binary resolver: rejected explicit executable path")
        nonexec_bin = tmp_root / "nonexec-dosbox-staging"
        nonexec_bin.write_bytes(b"#!/bin/sh\nexit 0\n")
        if _resolve_dosbox_bin(argparse.Namespace(dosbox_bin=nonexec_bin)) is not None:
            failures.append("dosbox binary resolver: accepted a non-executable path")
        missing_bin = tmp_root / "missing-dosbox"
        if _resolve_dosbox_bin(argparse.Namespace(dosbox_bin=missing_bin)) is not None:
            failures.append("dosbox binary resolver: accepted a missing explicit path")
        fake_staging_conf = tmp_root / "fake.conf"
        if _dosbox_conf_command(str(fake_bin), fake_staging_conf)[1] != "--conf":
            failures.append("dosbox launch argv: did not use --conf for dosbox-staging")
        fake_x_bin = tmp_root / "dosbox-x"
        if _dosbox_conf_command(str(fake_x_bin), fake_staging_conf)[1] != "-conf":
            failures.append("dosbox launch argv: did not preserve -conf for non-staging DOSBox")
        staging_launch = _dosbox_conf_command(str(fake_bin), fake_staging_conf, good)
        if staging_launch[-1] != str(good / "DM.EXE"):
            failures.append("dosbox launch argv: did not append runtime DM.EXE path")
        capture_root = tmp_root / "capture-root"
        conf = capture_root / "dosbox_capture.live.conf"
        _write_live_conf(conf, good, capture_root)
        receipt_path = _write_live_input_receipt(
            capture_root=capture_root,
            runtime_dir=good,
            dosbox_bin=str(fake_bin),
            conf=conf,
            capture_backend="auto",
        )
        try:
            receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
        except Exception as exc:
            failures.append(f"live input receipt: could not parse JSON: {exc}")
        else:
            if receipt.get("schema") != LIVE_INPUT_RECEIPT_SCHEMA:
                failures.append("live input receipt: schema mismatch")
            if receipt.get("runtime_dir") != str(good):
                failures.append("live input receipt: runtime_dir mismatch")
            if receipt.get("dosbox_bin") != str(fake_bin):
                failures.append("live input receipt: dosbox_bin mismatch")
            if receipt.get("conf_sha256") != _sha256_file(conf):
                failures.append("live input receipt: conf hash mismatch")
            if receipt.get("capture_backend") != "auto":
                failures.append("live input receipt: capture backend mismatch")
            required = receipt.get("runtime_required_files", {})
            if not required.get("GRAPHICS.DAT") or not required.get("DUNGEON.DAT"):
                failures.append("live input receipt: required DAT presence missing")
            pins = receipt.get("live_conf_pins", {})
            if pins.get("machine") != "svga_s3" or pins.get("memsize") != "16":
                failures.append("live input receipt: conf pin metadata mismatch")
        movement_point = _framebuffer_click_point(
            (100, 200, 1024, 768 + MACOS_WINDOW_TITLEBAR_H),
            DUNGEON_MOVE_FORWARD_CLICK_FRAC,
        )
        if movement_point is None:
            failures.append("dungeon movement click: framebuffer point did not resolve")
        movement_before = FrameQuality(
            label="capture_01_ingame_start",
            state="dungeon_gameplay",
            width=320,
            height=200,
            full_nonblack=0.5,
            viewport_nonblack=0.9,
            rightcol_nonblack=0.1,
            champion_nonblack=0.6,
            blackout=False,
            capture_backend="fixture",
            normalized_rgb_sha256="a" * 64,
        )
        movement_after = FrameQuality(
            label="capture_02_ingame_step_forward_0001",
            state="dungeon_gameplay",
            width=320,
            height=200,
            full_nonblack=0.6,
            viewport_nonblack=0.92,
            rightcol_nonblack=0.1,
            champion_nonblack=0.6,
            blackout=False,
            capture_backend="fixture",
            normalized_rgb_sha256="b" * 64,
        )
        movement_receipt_path = _write_live_movement_receipt(
            capture_root,
            before_quality=movement_before,
            after_quality=movement_after,
            before_viewport_sha256="c" * 64,
            after_viewport_sha256="d" * 64,
            before_path=capture_root / "original" / "01_ingame_start.png",
            after_path=capture_root / "original" / "02_ingame_step_forward.png",
            movement_changed=True,
            action_key=DUNGEON_MOVE_FORWARD_KEYBOARD_KEY,
            input_method="keyboard_keypad_5_after_c070_mouse_probe",
            source_anchor=DUNGEON_MOVE_FORWARD_KEYBOARD_SOURCE,
            action_attempts=[
                {
                    "action_key": DUNGEON_MOVE_FORWARD_CLICK_KEY,
                    "input_method": "absolute_mouse_click_with_mouse_capture_onclick",
                    "viewport_rgb_changed": False,
                },
                {
                    "action_key": DUNGEON_MOVE_FORWARD_KEYBOARD_KEY,
                    "input_method": "keyboard_key_code_keypad_5",
                    "viewport_rgb_changed": True,
                },
            ],
        )
        try:
            movement_receipt = json.loads(
                movement_receipt_path.read_text(encoding="utf-8")
            )
        except Exception as exc:
            failures.append(f"movement receipt: could not parse JSON: {exc}")
        else:
            if movement_receipt.get("schema") != LIVE_MOVEMENT_RECEIPT_SCHEMA:
                failures.append("movement receipt: schema mismatch")
            if movement_receipt.get("action_key") != DUNGEON_MOVE_FORWARD_KEYBOARD_KEY:
                failures.append("movement receipt: action key mismatch")
            if not movement_receipt.get("viewport_rgb_changed"):
                failures.append("movement receipt: viewport change flag missing")
            attempts = movement_receipt.get("action_attempts", [])
            if len(attempts) != 2:
                failures.append("movement receipt: action attempts missing")
            coord = movement_receipt.get("screen_coord_320x200", {})
            if coord.get("x") != 276 or coord.get("y") != 135:
                failures.append("movement receipt: source-zone coordinate mismatch")
        abort_quality = FrameQuality(
            label="entrance_menu_0006",
            state="title_screen",
            width=320,
            height=200,
            full_nonblack=0.0,
            viewport_nonblack=0.0,
            rightcol_nonblack=0.0,
            champion_nonblack=0.0,
            blackout=True,
        )
        key_dispatch = KeyDispatch(
            step="entrance_menu",
            key="Return",
            mapped="return",
            host_active_app_before="Terminal",
            host_active_app_after="DOSBox Staging",
            dosbox_window_bounds_before="",
            dosbox_window_bounds_after="1,2,1024,768",
            timestamp_s=123.0,
            ok=True,
        )
        _write_key_dispatch_log(capture_root, key_dispatch)
        parsed_key_dispatch = _last_key_dispatch_from_log(capture_root)
        if parsed_key_dispatch is None:
            failures.append("key dispatch log: parser did not return the last row")
        elif parsed_key_dispatch.key != "Return" or parsed_key_dispatch.mapped != "return":
            failures.append("key dispatch log: last-row key metadata mismatch")
        abort_path = _write_live_abort_receipt(
            capture_root=capture_root,
            step=PlanStep(
                "entrance_menu",
                "entrance_menu",
                keys=["Return"],
                timeout_s=30.0,
                stable_frames=3,
            ),
            last_state="capture_blackout",
            reason="capture_blackout",
            quality=abort_quality,
        )
        try:
            abort_receipt = json.loads(abort_path.read_text(encoding="utf-8"))
        except Exception as exc:
            failures.append(f"live abort receipt: could not parse JSON: {exc}")
        else:
            if abort_receipt.get("schema") != LIVE_ABORT_RECEIPT_SCHEMA:
                failures.append("live abort receipt: schema mismatch")
            if abort_receipt.get("step") != "entrance_menu":
                failures.append("live abort receipt: step mismatch")
            if abort_receipt.get("expected_state") != "entrance_menu":
                failures.append("live abort receipt: expected_state mismatch")
            if abort_receipt.get("reason") != "capture_blackout":
                failures.append("live abort receipt: reason mismatch")
            last_quality = abort_receipt.get("last_frame_quality", {})
            if not last_quality.get("blackout"):
                failures.append("live abort receipt: last frame quality missing blackout")
            if "quality_log" not in abort_receipt or "state_samples_dir" not in abort_receipt:
                failures.append("live abort receipt: diagnostic paths missing")
            if "key_dispatch_log" not in abort_receipt:
                failures.append("live abort receipt: key dispatch log path missing")
            last_key = abort_receipt.get("last_key_dispatch") or {}
            if last_key.get("step") != "entrance_menu" or last_key.get("key") != "Return":
                failures.append("live abort receipt: last key dispatch metadata missing")
        focus_abort_path = _write_live_abort_receipt(
            capture_root=capture_root,
            step=PlanStep(
                "focus_probe",
                "title_screen",
                timeout_s=5.0,
                stable_frames=3,
            ),
            last_state="capture_focus_mismatch",
            reason="capture_focus_mismatch",
            quality=focus_mismatches[-1],
        )
        try:
            focus_abort_receipt = json.loads(focus_abort_path.read_text(encoding="utf-8"))
        except Exception as exc:
            failures.append(f"focus abort receipt: could not parse JSON: {exc}")
        else:
            if focus_abort_receipt.get("reason") != "capture_focus_mismatch":
                failures.append("focus abort receipt: reason mismatch")
            focus_quality = focus_abort_receipt.get("last_frame_quality", {})
            if focus_quality.get("host_active_app") != "Terminal":
                failures.append("focus abort receipt: active-app diagnostic missing")
        from PIL import Image, ImageDraw
        black = Image.new("RGB", (320, 200), (0, 0, 0))
        visible = Image.new("RGB", (320, 200), (0, 0, 0))
        draw = ImageDraw.Draw(visible)
        draw.rectangle((0, 33, 223, 168), fill=(96, 80, 48))
        capture_root = tmp_root / "auto-backend-capture-root"
        original_capture_raw_frame = globals()["_capture_raw_frame"]
        original_capture_with_screencapture = globals()["_capture_with_screencapture"]
        try:
            def fake_capture_raw_frame(raw: Path, backend: str) -> dict[str, str]:
                if backend != "auto":
                    raise AssertionError("auto fallback fixture expected auto backend")
                black.save(raw)
                return {
                    "capture_backend": "peekaboo",
                    "capture_source": "peekaboo:window:DOSBox Staging",
                    "host_active_app": "DOSBox Staging",
                    "dosbox_window_bounds": "1,2,1024,768",
                }

            def fake_capture_with_screencapture(raw: Path) -> tuple[dict[str, str], bool]:
                visible.save(raw)
                return {
                    "capture_backend": "screencapture",
                    "capture_source": "screencapture:dosbox-window",
                    "host_active_app": "DOSBox Staging",
                    "dosbox_window_bounds": "1,2,1024,768",
                }, True

            globals()["_capture_raw_frame"] = fake_capture_raw_frame
            globals()["_capture_with_screencapture"] = fake_capture_with_screencapture
            fallback_img, fallback_meta = _screenshot_frame(
                capture_root,
                "auto_backend_blackout_fallback",
                "auto",
            )
        finally:
            globals()["_capture_raw_frame"] = original_capture_raw_frame
            globals()["_capture_with_screencapture"] = original_capture_with_screencapture
        if _is_capture_blackout(fallback_img):
            failures.append("auto capture backend: did not prefer visible alternate backend")
        if fallback_meta.get("capture_backend") != "screencapture":
            failures.append("auto capture backend: fallback backend metadata mismatch")
        if "auto-visible-after-peekaboo-blackout" not in fallback_meta.get("capture_source", ""):
            failures.append("auto capture backend: fallback provenance missing")
        capture_root = tmp_root / "auto-rawshot-fallback-capture-root"
        original_capture_raw_frame = globals()["_capture_raw_frame"]
        original_capture_with_screencapture = globals()["_capture_with_screencapture"]
        original_capture_with_dosbox_rawshot = globals()["_capture_with_dosbox_rawshot"]
        try:
            def fake_black_capture_raw_frame(raw: Path, backend: str) -> dict[str, str]:
                if backend != "auto":
                    raise AssertionError("rawshot fallback fixture expected auto backend")
                black.save(raw)
                return {
                    "capture_backend": "peekaboo",
                    "capture_source": "peekaboo:window:DOSBox Staging",
                    "host_active_app": "DOSBox Staging",
                    "dosbox_window_bounds": "1,2,1024,768",
                }

            def fake_black_screencapture(raw: Path) -> tuple[dict[str, str], bool]:
                black.save(raw)
                return {
                    "capture_backend": "screencapture",
                    "capture_source": "screencapture:dosbox-window",
                    "host_active_app": "DOSBox Staging",
                    "dosbox_window_bounds": "1,2,1024,768",
                }, True

            def fake_visible_rawshot(raw: Path) -> tuple[object, dict[str, str]]:
                visible.save(raw)
                return visible, {
                    "capture_backend": "dosbox-rawshot",
                    "capture_source": str(capture_root / "dosbox-capture" / "fixture.raw"),
                    "host_active_app": "DOSBox Staging",
                    "dosbox_window_bounds": "1,2,1024,768",
                }

            globals()["_capture_raw_frame"] = fake_black_capture_raw_frame
            globals()["_capture_with_screencapture"] = fake_black_screencapture
            globals()["_capture_with_dosbox_rawshot"] = fake_visible_rawshot
            rawshot_fallback_img, rawshot_fallback_meta = _screenshot_frame(
                capture_root,
                "auto_backend_rawshot_fallback",
                "auto",
            )
        finally:
            globals()["_capture_raw_frame"] = original_capture_raw_frame
            globals()["_capture_with_screencapture"] = original_capture_with_screencapture
            globals()["_capture_with_dosbox_rawshot"] = original_capture_with_dosbox_rawshot
        if _is_capture_blackout(rawshot_fallback_img):
            failures.append("auto capture backend: did not fall back to visible DOSBox rawshot")
        if rawshot_fallback_meta.get("capture_backend") != "dosbox-rawshot":
            failures.append("auto capture backend: rawshot fallback metadata mismatch")
        if "auto-visible-after-peekaboo-blackout" not in rawshot_fallback_meta.get("capture_source", ""):
            failures.append("auto capture backend: rawshot fallback provenance missing")
        rawshot_root = tmp_root / "rawshot-capture-root"
        rawshot_capture_dir = rawshot_root / "dosbox-capture"
        rawshot_capture_dir.mkdir(parents=True)
        existing_rawshot = rawshot_capture_dir / "old.raw"
        existing_rawshot.write_bytes(b"\x01" * (320 * 200))
        time.sleep(0.01)
        rawshot_source = rawshot_capture_dir / "new.raw"
        rawshot_source.write_bytes(
            b"\x00" * (320 * 33) +
            b"\x44" * (320 * 136) +
            b"\x00" * (320 * 31)
        )
        rawshot_target = rawshot_root / "state-samples" / "rawshot_fixture.png"
        rawshot_img, rawshot_meta = _load_latest_dosbox_capture(
            rawshot_target,
            {existing_rawshot: (existing_rawshot.stat().st_mtime_ns,
                                existing_rawshot.stat().st_size)},
        )
        if _is_capture_blackout(rawshot_img):
            failures.append("dosbox rawshot backend: latest .raw fixture decoded as blackout")
        if rawshot_meta.get("capture_backend") != "dosbox-rawshot":
            failures.append("dosbox rawshot backend: metadata backend mismatch")
        if rawshot_meta.get("capture_source") != str(rawshot_source):
            failures.append("dosbox rawshot backend: metadata source mismatch")
        if rawshot_meta.get("capture_source_size") != str(320 * 200):
            failures.append("dosbox rawshot backend: source size metadata missing")
        if rawshot_meta.get("capture_source_sha256") != _sha256_file(rawshot_source):
            failures.append("dosbox rawshot backend: source sha256 metadata mismatch")
        rewrite_capture_dir = tmp_root / "rawshot-rewrite-capture-root" / "dosbox-capture"
        rewrite_capture_dir.mkdir(parents=True)
        rewrite_source = rewrite_capture_dir / "rewritten.raw"
        rewrite_source.write_bytes(b"\x00" * (320 * 200))
        rewrite_known = _known_dosbox_capture_files(rewrite_capture_dir)
        rewrite_source.write_bytes(
            b"\x00" * (320 * 33) +
            b"\x66" * (320 * 136) +
            b"\x00" * (320 * 31)
        )
        rewrite_target = (
            tmp_root / "rawshot-rewrite-capture-root" /
            "state-samples" / "rawshot_rewrite_fixture.png"
        )
        rewrite_img, rewrite_meta = _load_latest_dosbox_capture(
            rewrite_target,
            rewrite_known,
        )
        if _is_capture_blackout(rewrite_img):
            failures.append("dosbox rawshot backend: rewritten .raw fixture decoded as blackout")
        if rewrite_meta.get("capture_source") != str(rewrite_source):
            failures.append("dosbox rawshot backend: rewritten source metadata mismatch")
        if rewrite_meta.get("capture_source_sha256") != _sha256_file(rewrite_source):
            failures.append("dosbox rawshot backend: rewritten source sha256 metadata mismatch")
        rewrite_quality = _frame_quality(
            rewrite_img,
            "rawshot_rewrite_fixture",
            "dungeon_gameplay",
            rewrite_meta,
        )
        if rewrite_quality.capture_source_sha256 != _sha256_file(rewrite_source):
            failures.append("frame quality: rawshot source sha256 was not propagated")
        if rewrite_quality.normalized_rgb_sha256 != hashlib.sha256(
                rewrite_img.convert("RGB").resize((320, 200)).tobytes()).hexdigest():
            failures.append("frame quality: normalized RGB sha256 mismatch")
    return matched, total, failures


def _run_quiet(argv: list[str], timeout: float = 10.0) -> subprocess.CompletedProcess:
    try:
        return subprocess.run(
            argv,
            capture_output=True,
            text=True,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired:
        return QuietRunTimeout()  # type: ignore[return-value]


def _open_activate_dosbox() -> bool:
    """Raise an already-running DOSBox window via ``open -a`` (macOS).

    ``open -a NAME`` activates the running application instance and brings
    its window to the front.  When the app is already running it does NOT
    spawn a duplicate process (verified against DOSBox Staging 0.82.2), so
    this is safe to call repeatedly inside the live route.  This is the
    reliable focus path that plain AppleScript ``set frontmost`` failed to
    achieve when DOSBox was launched as a bare ``Contents/MacOS/dosbox``
    subprocess.  Returns True if any ``open -a`` invocation succeeded.
    """
    if shutil.which("open") is None:
        return False
    for app_name in MACOS_DOSBOX_OPEN_APP_NAMES:
        proc = _run_quiet(["open", "-a", app_name], timeout=5.0)
        if getattr(proc, "returncode", 1) == 0:
            return True
    return False


def _activate_dosbox() -> None:
    """Best-effort focus for macOS DOSBox Staging windows.

    Tries ``open -a`` first (the only method observed to actually raise the
    DOSBox window above Terminal when DOSBox was started as a bare
    subprocess), then falls back to the AppleScript frontmost/AXRaise path
    so non-bundle DOSBox builds and headless self-tests still work.
    """
    _open_activate_dosbox()
    names = ", ".join(f'"{name}"' for name in DOSBOX_PROCESS_NAMES)
    script = r'''
tell application "System Events"
  repeat with appName in {%s}
    if exists process appName then
      tell process appName
        set frontmost to true
        try
          perform action "AXRaise" of window 1
        end try
      end tell
      return
    end if
  end repeat
end tell
''' % names
    _run_quiet(["osascript", "-e", script], timeout=5.0)


def _dosbox_window_bounds() -> tuple[int, int, int, int] | None:
    """Return the front DOSBox window bounds as x, y, w, h when available."""
    names = ", ".join(f'"{name}"' for name in DOSBOX_PROCESS_NAMES)
    script = r'''
tell application "System Events"
  repeat with appName in {%s}
    if exists process appName then
      tell process appName
        set frontmost to true
        set p to position of window 1
        set s to size of window 1
        return (item 1 of p as string) & "," & (item 2 of p as string) & "," & (item 1 of s as string) & "," & (item 2 of s as string)
      end tell
    end if
  end repeat
end tell
''' % names
    proc = _run_quiet(["osascript", "-e", script], timeout=5.0)
    if proc.returncode != 0:
        return None
    try:
        x, y, w, h = [int(float(part.strip())) for part in proc.stdout.strip().split(",")]
    except Exception:
        return None
    if w <= 0 or h <= 0:
        return None
    return x, y, w, h


def _frontmost_process_name() -> str:
    script = r'''
tell application "System Events"
  set frontApps to name of every process whose frontmost is true
  if (count of frontApps) is greater than 0 then
    return item 1 of frontApps
  end if
end tell
'''
    proc = _run_quiet(["osascript", "-e", script], timeout=5.0)
    if proc.returncode != 0:
        return ""
    return proc.stdout.strip()


def _bounds_text(bounds: tuple[int, int, int, int] | None) -> str:
    if bounds is None:
        return ""
    return ",".join(str(part) for part in bounds)


def _capture_with_screencapture(raw: Path) -> tuple[dict[str, str], bool]:
    _activate_dosbox()
    bounds = _dosbox_window_bounds()
    if bounds is not None:
        x, y, w, h = bounds
        cmd = ["screencapture", "-x", "-R", f"{x},{y},{w},{h}", str(raw)]
        source = "screencapture:dosbox-window"
    else:
        cmd = ["screencapture", "-x", str(raw)]
        source = "screencapture:screen"
    proc = _run_quiet(cmd, timeout=15.0)
    return {
        "capture_backend": "screencapture",
        "capture_source": source,
        "host_active_app": _frontmost_process_name(),
        "dosbox_window_bounds": _bounds_text(bounds),
    }, proc.returncode == 0 and raw.is_file()


def _capture_with_peekaboo(raw: Path) -> tuple[dict[str, str], bool]:
    if shutil.which("peekaboo") is None:
        return {
            "capture_backend": "peekaboo",
            "capture_source": "peekaboo:missing",
            "host_active_app": _frontmost_process_name(),
            "dosbox_window_bounds": _bounds_text(_dosbox_window_bounds()),
        }, False
    _activate_dosbox()
    bounds = _dosbox_window_bounds()
    for app_name in ("DOSBox Staging", "DOSBox"):
        cmd = [
            "peekaboo", "image",
            "--app", app_name,
            "--mode", "window",
            "--path", str(raw),
            "--json",
            "--no-remote",
        ]
        proc = _run_quiet(cmd, timeout=20.0)
        if proc.returncode == 0 and raw.is_file():
            return {
                "capture_backend": "peekaboo",
                "capture_source": f"peekaboo:window:{app_name}",
                "host_active_app": _frontmost_process_name(),
                "dosbox_window_bounds": _bounds_text(bounds),
            }, True
    cmd = [
        "peekaboo", "image",
        "--mode", "screen",
        "--path", str(raw),
        "--json",
        "--no-remote",
    ]
    proc = _run_quiet(cmd, timeout=20.0)
    return {
        "capture_backend": "peekaboo",
        "capture_source": "peekaboo:screen",
        "host_active_app": _frontmost_process_name(),
        "dosbox_window_bounds": _bounds_text(bounds),
    }, proc.returncode == 0 and raw.is_file()


def _dosbox_capture_dir(raw: Path) -> Path:
    return raw.parent.parent / "dosbox-capture"


def _dosbox_capture_signature(path: Path) -> DosboxCaptureSignature | None:
    try:
        st = path.stat()
    except OSError:
        return None
    if not path.is_file():
        return None
    return (st.st_mtime_ns, st.st_size)


def _known_dosbox_capture_files(capture_dir: Path) -> dict[Path, DosboxCaptureSignature]:
    known: dict[Path, DosboxCaptureSignature] = {}
    if not capture_dir.is_dir():
        return known
    for pattern in DOSBOX_INTERNAL_CAPTURE_GLOBS:
        for item in capture_dir.glob(pattern):
            sig = _dosbox_capture_signature(item)
            if sig is not None:
                known[item] = sig
    return known


def _trigger_dosbox_internal_screenshot() -> None:
    """Ask DOSBox to write its own capture artifact.

    This path bypasses macOS host-window capture entirely (no title bar,
    no notification banners), which is the clean-framebuffer half of the
    original-capture story.  The loader below accepts raw/PNG/BMP so the
    route state machine does not care which format DOSBox wrote.

    Keystroke choice (DOSBox Staging 0.82.2 on macOS 15, empirically
    verified): DOSBox's *default* screenshot binding is Ctrl+F5 (raw),
    but macOS swallows Ctrl+F5 as a system keyboard-navigation shortcut
    ("move focus to window toolbar"), so it never reaches DOSBox and no
    capture is written.  Alt+F5 (the *rendered* screenshot) is NOT
    intercepted by macOS and reliably reaches DOSBox, producing a clean
    chrome-free framebuffer image in ``capture_dir``.  Cmd+F5 is also
    swallowed.  We therefore drive the rendered screenshot via Alt+F5
    (key code 96 + option) on macOS.  Pair this with
    ``glshader=none``/``default_image_capture_formats=rendered`` in the
    live conf so the rendered capture has square pixels and no CRT
    shader overlay.
    """
    _activate_dosbox()
    script = r'''
tell application "System Events"
  key code 96 using {option down}
end tell
'''
    proc = _run_quiet(["osascript", "-e", script], timeout=5.0)
    if proc.returncode != 0:
        raise RuntimeError(
            "failed to trigger DOSBox rendered screenshot via Alt+F5: "
            f"{proc.stderr.strip() or proc.stdout.strip()}"
        )


def _decode_dosbox_raw_capture(source: Path):
    """Decode the raw 320x200 screenshot form written by DOSBox.

    The state detector only needs non-black density, so 64,000-byte raw
    VGA index dumps can be converted directly to an 8-bit grayscale
    image.  If a 768-byte RGB palette follows the index plane, use it.
    """
    if not _DETECTOR_OK:
        raise RuntimeError("Pillow and numpy are required for --live")
    from PIL import Image

    data = source.read_bytes()
    pixels = 320 * 200
    if len(data) == pixels:
        return Image.frombytes("L", (320, 200), data).convert("RGB")
    if len(data) >= pixels + 768:
        pal = data[pixels:pixels + 768]
        img = Image.frombytes("P", (320, 200), data[:pixels])
        img.putpalette(list(pal))
        return img.convert("RGB")
    raise RuntimeError(
        f"unsupported DOSBox rawshot size for {source}: "
        f"{len(data)} bytes, expected 64000 or 64768+"
    )


def _load_dosbox_capture_image(source: Path):
    if source.suffix.lower() == ".raw":
        return _decode_dosbox_raw_capture(source)
    if not _DETECTOR_OK:
        raise RuntimeError("Pillow and numpy are required for --live")
    from PIL import Image

    return Image.open(source).convert("RGB")


def _load_latest_dosbox_capture(
        raw: Path,
        known: dict[Path, DosboxCaptureSignature]) -> tuple[object, dict[str, str]]:
    capture_dir = _dosbox_capture_dir(raw)
    candidates: list[tuple[Path, DosboxCaptureSignature]] = []
    for pattern in DOSBOX_INTERNAL_CAPTURE_GLOBS:
        for item in capture_dir.glob(pattern):
            sig = _dosbox_capture_signature(item)
            if sig is not None and known.get(item) != sig:
                candidates.append((item, sig))
    if not candidates:
        raise RuntimeError(f"DOSBox rawshot did not create a capture in {capture_dir}")
    latest, latest_sig = max(candidates, key=lambda p: (p[1][0], p[0].name))
    img = _load_dosbox_capture_image(latest)
    raw.parent.mkdir(parents=True, exist_ok=True)
    img.save(raw)
    return img, {
        "capture_backend": "dosbox-rawshot",
        "capture_source": str(latest),
        "capture_source_mtime_ns": str(latest_sig[0]),
        "capture_source_size": str(latest_sig[1]),
        "capture_source_sha256": _sha256_file(latest),
        "host_active_app": _frontmost_process_name(),
        "dosbox_window_bounds": _bounds_text(_dosbox_window_bounds()),
    }


def _capture_with_dosbox_rawshot(raw: Path) -> tuple[object, dict[str, str]]:
    capture_dir = _dosbox_capture_dir(raw)
    capture_dir.mkdir(parents=True, exist_ok=True)
    known = _known_dosbox_capture_files(capture_dir)
    _trigger_dosbox_internal_screenshot()
    deadline = time.time() + 5.0
    last_error = ""
    while time.time() < deadline:
        try:
            return _load_latest_dosbox_capture(raw, known)
        except RuntimeError as exc:
            last_error = str(exc)
            time.sleep(0.1)
    raise RuntimeError(last_error or f"DOSBox rawshot did not create a capture in {capture_dir}")


def _capture_raw_frame(raw: Path, capture_backend: str) -> dict[str, str]:
    if capture_backend == "peekaboo":
        meta, ok = _capture_with_peekaboo(raw)
        if not ok:
            raise RuntimeError(f"Peekaboo failed to capture {raw}")
        return meta
    if capture_backend == "screencapture":
        meta, ok = _capture_with_screencapture(raw)
        if not ok:
            raise RuntimeError(f"screencapture failed to capture {raw}")
        return meta
    if capture_backend == "dosbox-rawshot":
        _img, meta = _capture_with_dosbox_rawshot(raw)
        return meta
    if capture_backend != "auto":
        raise ValueError(f"unsupported capture backend: {capture_backend}")
    if shutil.which("peekaboo") is not None:
        meta, ok = _capture_with_peekaboo(raw)
        if ok:
            meta["capture_backend"] = "peekaboo"
            return meta
    meta, ok = _capture_with_screencapture(raw)
    if not ok:
        raise RuntimeError(f"host screenshot failed to capture {raw}")
    meta["capture_backend"] = "screencapture"
    return meta


def _crop_to_4x3(img):
    """Crop a window screenshot down to its likely DOSBox framebuffer area."""
    target = 4.0 / 3.0
    w, h = img.size
    if w <= 0 or h <= 0:
        return img
    ratio = w / h
    if ratio > target:
        new_w = int(h * target)
        left = max(0, (w - new_w) // 2)
        return img.crop((left, 0, left + new_w, h))
    if ratio < target:
        new_h = int(w / target)
        top = max(0, (h - new_h) // 2)
        return img.crop((0, top, w, top + new_h))
    return img


def _normalize_capture_image(raw: Path):
    if not _DETECTOR_OK:
        raise RuntimeError("Pillow and numpy are required for --live")
    from PIL import Image

    img = Image.open(raw).convert("RGB")
    return _normalize_loaded_capture_image(img)


def _normalize_loaded_capture_image(img):
    if not _DETECTOR_OK:
        raise RuntimeError("Pillow and numpy are required for --live")
    from PIL import Image

    resample = getattr(getattr(Image, "Resampling", Image), "NEAREST")
    return _crop_to_4x3(img).resize((320, 200), resample)


def _is_capture_blackout(img) -> bool:
    quality = _frame_quality(img, "auto_backend_probe", "capture_probe")
    return quality.blackout


def _screenshot_frame(capture_root: Path, label: str, capture_backend: str):
    if not _DETECTOR_OK:
        raise RuntimeError("Pillow and numpy are required for --live")

    capture_root.mkdir(parents=True, exist_ok=True)
    raw = capture_root / "state-samples" / f"{label}.png"
    raw.parent.mkdir(parents=True, exist_ok=True)
    if capture_backend == "dosbox-rawshot":
        img, meta = _capture_with_dosbox_rawshot(raw)
        img = _normalize_loaded_capture_image(img)
    else:
        meta = _capture_raw_frame(raw, capture_backend)
        img = _normalize_capture_image(raw)
    if capture_backend == "auto" and _is_capture_blackout(img):
        alternate_backend = (
            "screencapture"
            if meta.get("capture_backend") == "peekaboo"
            else "peekaboo"
        )
        alternate_raw = raw.with_name(f"{raw.stem}.{alternate_backend}.png")
        fallback_errors: list[str] = []
        try:
            alternate_meta, alternate_ok = (
                _capture_with_screencapture(alternate_raw)
                if alternate_backend == "screencapture"
                else _capture_with_peekaboo(alternate_raw)
            )
            if alternate_ok and alternate_raw.is_file():
                alternate_img = _normalize_capture_image(alternate_raw)
                if not _is_capture_blackout(alternate_img):
                    alternate_meta["capture_backend"] = alternate_backend
                    alternate_meta["capture_source"] = (
                        alternate_meta.get("capture_source", "") +
                        f":auto-visible-after-{meta.get('capture_backend', 'unknown')}-blackout"
                    )
                    return alternate_img, alternate_meta
        except Exception as exc:
            fallback_errors.append(
                f"auto-alternate-{alternate_backend}-failed:{type(exc).__name__}"
            )
        rawshot_raw = raw.with_name(f"{raw.stem}.dosbox-rawshot.png")
        try:
            rawshot_img, rawshot_meta = _capture_with_dosbox_rawshot(rawshot_raw)
            rawshot_img = _normalize_loaded_capture_image(rawshot_img)
            if not _is_capture_blackout(rawshot_img):
                rawshot_meta["capture_source"] = (
                    rawshot_meta.get("capture_source", "") +
                    f":auto-visible-after-{meta.get('capture_backend', 'unknown')}-blackout"
                )
                return rawshot_img, rawshot_meta
            fallback_errors.append("auto-dosbox-rawshot-blackout")
        except Exception as exc:
            fallback_errors.append(
                f"auto-dosbox-rawshot-failed:{type(exc).__name__}"
            )
        if fallback_errors:
            meta["capture_source"] = (
                meta.get("capture_source", "") + ":" + ":".join(fallback_errors)
            )
    return img, meta


def _region_density_from_array(arr, x0: int, y0: int, x1: int, y1: int) -> float:
    import numpy as np
    region = arr[y0:y1, x0:x1]
    if region.size == 0:
        return 0.0
    n_pixels = region.shape[0] * region.shape[1]
    if n_pixels <= 0:
        return 0.0
    n_nonblack = int(np.count_nonzero(np.any(region != 0, axis=2)))
    return float(n_nonblack / n_pixels)


def _frame_quality(
        img,
        label: str,
        state: str,
        capture_meta: dict[str, str] | None = None) -> FrameQuality:
    """Measure whether a normalized frame is useful for route verification."""
    if not _DETECTOR_OK:
        raise RuntimeError("Pillow and numpy are required for frame-quality diagnostics")
    import numpy as np

    normalized = img.convert("RGB").resize((320, 200))
    arr = np.array(normalized)
    full = _region_density_from_array(arr, 0, 0, 320, 200)
    viewport = _region_density_from_array(arr, 0, 33, 224, 169)
    rightcol = _region_density_from_array(arr, 224, 33, 320, 169)
    champion = _region_density_from_array(arr, 0, 0, 320, 65)
    meta = capture_meta or {}
    return FrameQuality(
        label=label,
        state=state,
        width=normalized.size[0],
        height=normalized.size[1],
        full_nonblack=full,
        viewport_nonblack=viewport,
        rightcol_nonblack=rightcol,
        champion_nonblack=champion,
        blackout=full < BLACKOUT_NONBLACK_THRESH,
        capture_backend=meta.get("capture_backend", "fixture"),
        capture_source=meta.get("capture_source", ""),
        capture_source_sha256=meta.get("capture_source_sha256", ""),
        normalized_rgb_sha256=hashlib.sha256(arr.tobytes()).hexdigest(),
        host_active_app=meta.get("host_active_app", ""),
        dosbox_window_bounds=meta.get("dosbox_window_bounds", ""),
    )


def _viewport_rgb_sha256(img) -> str:
    normalized = img.convert("RGB").resize((320, 200))
    viewport = normalized.crop((0, 33, 224, 169))
    return hashlib.sha256(viewport.tobytes()).hexdigest()


def _write_quality_log(capture_root: Path, quality: FrameQuality) -> None:
    log_path = capture_root / "state-samples" / "quality.jsonl"
    log_path.parent.mkdir(parents=True, exist_ok=True)
    with log_path.open("a", encoding="utf-8") as fh:
        fh.write(json.dumps(asdict(quality), sort_keys=True))
        fh.write("\n")


def _should_abort_for_blackout(
        recent: list[FrameQuality],
        target: str,
        blackout_frame_limit: int) -> bool:
    if blackout_frame_limit <= 0:
        return False
    if len(recent) < blackout_frame_limit:
        return False
    window = recent[-blackout_frame_limit:]
    return all(item.blackout for item in window)


def _is_dosbox_process_name(name: str) -> bool:
    folded = name.casefold()
    return any(candidate.casefold() in folded for candidate in DOSBOX_PROCESS_NAMES)


def _is_focus_mismatch(quality: FrameQuality) -> bool:
    active = quality.host_active_app.strip()
    if not active:
        return False
    return not _is_dosbox_process_name(active)


def _should_abort_for_focus_mismatch(
        recent: list[FrameQuality],
        focus_frame_limit: int) -> bool:
    """Abort when host focus diagnostics prove keypresses target another app.

    Empty ``host_active_app`` means the OS focus probe was unavailable, so it
    remains diagnostic only.  A known non-DOSBox frontmost app across multiple
    samples is actionable: the live route's ``cliclick`` key dispatch cannot
    deterministically advance the DM1 state machine in that condition.
    """
    if focus_frame_limit <= 0:
        return False
    if len(recent) < focus_frame_limit:
        return False
    window = recent[-focus_frame_limit:]
    return all(_is_focus_mismatch(item) for item in window)


def _has_focus_mismatch_window(
        recent: list[FrameQuality],
        focus_frame_limit: int) -> bool:
    """Return True if a non-DOSBox frontmost app appears anywhere in the window.

    A single focus mismatch is the trigger to attempt a rawshot-fallback
    recovery, even when the focus-frame window is not yet full enough to
    abort.  This lets the live route ask DOSBox to write its own Ctrl+F5
    capture *before* the focus-mismatch window has filled, so a transient
    Terminal/Editor frontmost sample does not cost the session a full
    focus-recovery + rawshot cycle.
    """
    if focus_frame_limit <= 0 or not recent:
        return False
    window = recent[-focus_frame_limit:]
    return any(_is_focus_mismatch(item) for item in window)


def _classify_rawshot_focus_recovery(
        recent: list[FrameQuality],
        rawshot_meta: dict[str, str] | None,
        rawshot_visible: bool) -> str:
    """Decide whether a focus mismatch is recoverable via the rawshot path.

    The live route runs on macOS where the host peekaboo / screencapture
    backends can return a non-DOSBox window when the user clicks out of
    DOSBox.  The dosbox-rawshot backend triggers DOSBox's own Ctrl+F5
    capture so it is independent of macOS window focus, but it still has
    to (a) successfully trigger, and (b) decode into a non-black
    frame.  This helper turns the recent focus window, the rawshot
    capture metadata, and the rawshot-frame blackout flag into one of:

    * ``rawshot_focus_recovered`` - the focus window had at least one
      non-DOSBox frontmost sample AND the rawshot backend produced a
      non-black frame.  The live route can keep going.
    * ``rawshot_focus_unrecoverable`` - the focus window had at least
      one non-DOSBox frontmost sample AND the rawshot backend either
      did not run (no ``rawshot_meta``) or produced a black frame
      (``rawshot_visible`` is False).  The live route must abort with
      this reason.
    * ``no_focus_recovery_needed`` - the focus window had no non-DOSBox
      frontmost sample.  The live route has no focus issue to recover
      from, and the rawshot path is informational only.

    A future live attempt that silently keeps a focus-mismatched
    window alive (i.e. never escalates to dosbox-rawshot) is caught
    by the ``rawshot_focus_unrecoverable`` branch, which is the new
    rawshot-fallback gate for the focus-mismatch failure mode the
    runbook §"Known Failure Modes" already names.
    """
    if not _has_focus_mismatch_window(recent, FOCUS_MISMATCH_FRAME_LIMIT):
        return "no_focus_recovery_needed"
    if rawshot_meta is None:
        return "rawshot_focus_unrecoverable"
    if not rawshot_visible:
        return "rawshot_focus_unrecoverable"
    if rawshot_meta.get("capture_backend") != "dosbox-rawshot":
        return "rawshot_focus_unrecoverable"
    return "rawshot_focus_recovered"


def _attempt_focus_recovery(
        capture_root: Path,
        recent: list[FrameQuality],
        probe_factory: Optional[Callable[[Path], tuple[object, dict[str, str]]]] = None,
        normalize_factory: Optional[Callable[[object], object]] = None) -> tuple[str, FrameQuality | None, dict[str, str] | None]:
    """Run the rawshot-fallback recovery probe and classify the result.

    ``probe_factory`` triggers DOSBox's own Ctrl+F5 capture and decodes
    the latest ``dosbox-capture/`` artifact into a normalized image +
    metadata tuple.  ``normalize_factory`` is the same image
    normalization the live route uses (crop-to-4x3 + resize-to-320x200).
    Tests can pass fakes for both; the default call sites fall through
    to ``_capture_with_dosbox_rawshot`` and
    ``_normalize_loaded_capture_image``.

    The return tuple is ``(reason, rawshot_quality, rawshot_meta)``:
    ``reason`` is one of ``RAWSHOT_FOCUS_RECOVERY_REASONS``;
    ``rawshot_quality`` is the ``FrameQuality`` for the decoded
    rawshot, or ``None`` when the rawshot did not produce an image;
    ``rawshot_meta`` is the capture metadata the rawshot returned, or
    ``None`` when the rawshot was never attempted.  Callers can use the
    ``rawshot_meta`` and ``rawshot_quality`` to enrich the abort
    receipt when the recovery is unrecoverable.
    """
    capture_root.mkdir(parents=True, exist_ok=True)
    if probe_factory is None:
        probe_factory = _capture_with_dosbox_rawshot
    if normalize_factory is None:
        normalize_factory = _normalize_loaded_capture_image
    rawshot_meta: dict[str, str] | None = None
    rawshot_quality: FrameQuality | None = None
    try:
        probe_dir = capture_root / "state-samples"
        probe_dir.mkdir(parents=True, exist_ok=True)
        probe_target = probe_dir / "focus_recovery_rawshot_probe.png"
        img, meta = probe_factory(probe_target)
    except Exception as exc:  # pragma: no cover - host-only failure
        meta = {
            "capture_backend": "dosbox-rawshot",
            "capture_source": "",
            "capture_error": f"{type(exc).__name__}: {exc}",
            "host_active_app": _frontmost_process_name(),
            "dosbox_window_bounds": _bounds_text(_dosbox_window_bounds()),
        }
        rawshot_meta = meta
    else:
        rawshot_meta = dict(meta) if isinstance(meta, dict) else None
        try:
            normalized = normalize_factory(img)
            rawshot_quality = _frame_quality(
                normalized,
                "focus_recovery_rawshot_probe",
                "capture_focus_recovery",
                meta,
            )
        except Exception:  # pragma: no cover - normalize-only failure
            rawshot_quality = None
    rawshot_visible = bool(rawshot_quality is not None and not rawshot_quality.blackout)
    reason = _classify_rawshot_focus_recovery(
        recent,
        rawshot_meta,
        rawshot_visible,
    )
    return reason, rawshot_quality, rawshot_meta


def _framebuffer_click_point(
        bounds: tuple[int, int, int, int] | None,
        frac: tuple[float, float]) -> tuple[int, int] | None:
    """Map a normalized framebuffer target to an absolute screen point.

    ``bounds`` is the DOSBox window (x, y, w, h) in screen points.  The
    content area starts below the macOS title bar; the DM framebuffer is
    letterboxed 4:3 inside that content rectangle.  ``frac`` is x/y inside
    the normalized framebuffer.  Returns ``None`` when the window bounds are
    unavailable so the caller can fail loudly instead of clicking a guessed
    coordinate.
    """
    if bounds is None:
        return None
    x, y, w, h = bounds
    content_h = h - MACOS_WINDOW_TITLEBAR_H
    if w <= 0 or content_h <= 0:
        return None
    target = 4.0 / 3.0
    ratio = w / content_h
    if ratio > target:
        disp_w = content_h * target
        disp_h = float(content_h)
    else:
        disp_w = float(w)
        disp_h = w / target
    off_x = (w - disp_w) / 2.0
    off_y = (content_h - disp_h) / 2.0
    fx, fy = frac
    sx = x + off_x + fx * disp_w
    sy = y + MACOS_WINDOW_TITLEBAR_H + off_y + fy * disp_h
    return int(round(sx)), int(round(sy))


def _entrance_enter_click_point(
        bounds: tuple[int, int, int, int] | None) -> tuple[int, int] | None:
    """Map the ENTER target to an absolute on-screen click point."""
    return _framebuffer_click_point(bounds, ENTRANCE_ENTER_CLICK_FRAC)


def _press_entrance_enter_click() -> None:
    """Click the DM entrance ENTER target to cross into the dungeon.

    DOSBox captures the mouse on the first click in the window, so this
    issues a move + two clicks: the first click is consumed by mouse
    capture, the second actually presses ENTER.  Requires cliclick.
    """
    if shutil.which("cliclick") is None:
        raise RuntimeError("cliclick is required for the entrance ENTER click")
    _activate_dosbox()
    point = _entrance_enter_click_point(_dosbox_window_bounds())
    if point is None:
        raise RuntimeError(
            "cannot resolve DOSBox window bounds for the entrance ENTER click"
        )
    px, py = point
    subprocess.run(["cliclick", f"m:{px},{py}"], check=True)
    time.sleep(0.3)
    subprocess.run(["cliclick", f"c:{px},{py}"], check=True)
    time.sleep(0.4)
    subprocess.run(["cliclick", f"c:{px},{py}"], check=True)


def _press_dungeon_move_forward_click() -> None:
    """Click the original PC C070 forward-arrow movement zone once."""
    if shutil.which("cliclick") is None:
        raise RuntimeError("cliclick is required for the dungeon forward click")
    _activate_dosbox()
    point = _framebuffer_click_point(
        _dosbox_window_bounds(),
        DUNGEON_MOVE_FORWARD_CLICK_FRAC,
    )
    if point is None:
        raise RuntimeError(
            "cannot resolve DOSBox window bounds for the dungeon forward click"
        )
    px, py = point
    subprocess.run(["cliclick", f"m:{px},{py}"], check=True)
    time.sleep(0.2)
    subprocess.run(["cliclick", f"c:{px},{py}"], check=True)


# macOS virtual key codes for the route keys.  Delivered via AppleScript
# ``key code N`` (System Events), which is the ONLY keystroke path observed
# to actually reach a focused DOSBox Staging window on macOS 15.  cliclick's
# ``kp:return``/``t:1`` events were silently dropped by DOSBox even with the
# window frontmost (the SELECTOR echoed the typed digit but never consumed
# the Return), which is why every live SELECTOR page used to stall.
OSASCRIPT_KEY_CODES = {
    "Return": 36,
    "Key-Up": 126,
    "Key-Down": 125,
    "Key-Left": 123,
    "Key-Right": 124,
    "Keypad-5": 87,
    "0": 29, "1": 18, "2": 19, "3": 20, "4": 21,
    "5": 23, "6": 22, "7": 26, "8": 28, "9": 25,
}


def _osascript_key_code(code: int, modifiers: tuple[str, ...] = ()) -> None:
    if modifiers:
        mod = " using {" + ", ".join(f"{m} down" for m in modifiers) + "}"
    else:
        mod = ""
    script = f'tell application "System Events" to key code {code}{mod}'
    proc = _run_quiet(["osascript", "-e", script], timeout=5.0)
    if getattr(proc, "returncode", 1) != 0:
        raise RuntimeError(
            f"osascript key code {code} failed: "
            f"{proc.stderr.strip() or proc.stdout.strip()}"
        )


def _press_key(key: str) -> None:
    time.sleep(0.25)
    if key == ENTRANCE_ENTER_CLICK_KEY:
        _press_entrance_enter_click()
        return
    if key == DUNGEON_MOVE_FORWARD_CLICK_KEY:
        _press_dungeon_move_forward_click()
        return
    # Re-raise the DOSBox window before every keystroke.  Without this the
    # key event is delivered to whatever app happens to be frontmost
    # (observed: keystrokes silently dropped, frames byte-stable,
    # ``host_active_app`` momentarily empty), so the SELECTOR pages never
    # advanced even though the screenshot path later re-activated DOSBox.
    # ``_activate_dosbox`` is the proven ``open -a`` focus path and is cheap
    # to repeat (it does not spawn duplicate DOSBox instances).
    _activate_dosbox()
    code = OSASCRIPT_KEY_CODES.get(key)
    if code is not None:
        _osascript_key_code(code)
        return
    # Fallback for any single character not in the explicit key-code map.
    if len(key) == 1:
        proc = _run_quiet(
            ["osascript", "-e",
             f'tell application "System Events" to keystroke "{key}"'],
            timeout=5.0,
        )
        if getattr(proc, "returncode", 1) != 0:
            raise RuntimeError(f"osascript keystroke {key!r} failed")
        return
    raise ValueError(f"unsupported live key: {key}")


def _key_dispatch_log_path(capture_root: Path) -> Path:
    return capture_root / "state-samples" / "key_dispatch.jsonl"


def _write_key_dispatch_log(capture_root: Path, dispatch: KeyDispatch) -> None:
    log_path = _key_dispatch_log_path(capture_root)
    log_path.parent.mkdir(parents=True, exist_ok=True)
    with log_path.open("a", encoding="utf-8") as fh:
        fh.write(json.dumps(asdict(dispatch), sort_keys=True))
        fh.write("\n")


def _last_key_dispatch_from_log(capture_root: Path) -> KeyDispatch | None:
    log_path = _key_dispatch_log_path(capture_root)
    try:
        lines = [line.strip() for line in log_path.read_text(encoding="utf-8").splitlines()
                 if line.strip()]
    except OSError:
        return None
    if not lines:
        return None
    try:
        data = json.loads(lines[-1])
        return KeyDispatch(
            step=str(data.get("step", "")),
            key=str(data.get("key", "")),
            mapped=str(data.get("mapped", "")),
            host_active_app_before=str(data.get("host_active_app_before", "")),
            host_active_app_after=str(data.get("host_active_app_after", "")),
            dosbox_window_bounds_before=str(data.get("dosbox_window_bounds_before", "")),
            dosbox_window_bounds_after=str(data.get("dosbox_window_bounds_after", "")),
            timestamp_s=float(data.get("timestamp_s", 0.0)),
            ok=bool(data.get("ok", False)),
            error=str(data.get("error", "")),
        )
    except (TypeError, ValueError, json.JSONDecodeError):
        return None


def _dispatch_key_for_live_step(capture_root: Path, step_name: str, key: str) -> None:
    pseudo_map = {
        ENTRANCE_ENTER_CLICK_KEY: "mouse:left:entrance_enter",
        DUNGEON_MOVE_FORWARD_CLICK_KEY: "mouse:left:c070_move_forward",
    }
    mapped = KEY_MAP.get(key, pseudo_map.get(key, key if len(key) == 1 else ""))
    before_app = _frontmost_process_name()
    before_bounds = _bounds_text(_dosbox_window_bounds())
    dispatch = KeyDispatch(
        step=step_name,
        key=key,
        mapped=mapped,
        host_active_app_before=before_app,
        host_active_app_after="",
        dosbox_window_bounds_before=before_bounds,
        dosbox_window_bounds_after="",
        timestamp_s=time.time(),
        ok=False,
    )
    try:
        _press_key(key)
        dispatch.ok = True
    except Exception as exc:
        dispatch.error = f"{type(exc).__name__}: {exc}"
        raise
    finally:
        dispatch.host_active_app_after = _frontmost_process_name()
        dispatch.dosbox_window_bounds_after = _bounds_text(_dosbox_window_bounds())
        _write_key_dispatch_log(capture_root, dispatch)


def _wait_for_state(capture_root: Path, target: str, timeout_s: float,
                    screenshot_int: float, stable_frames: int,
                    blackout_frame_limit: int,
                    capture_backend: str) -> tuple[bool, str]:
    deadline = time.time() + timeout_s
    stable = 0
    last_state = "unknown"
    sample = 0
    recent_quality: list[FrameQuality] = []
    while time.time() < deadline:
        sample += 1
        label = f"{target}_{sample:04d}"
        img, capture_meta = _screenshot_frame(capture_root, label, capture_backend)
        state = classify(img)
        last_state = state
        quality = _frame_quality(img, label, state, capture_meta)
        _write_quality_log(capture_root, quality)
        recent_quality.append(quality)
        print(
            f"state={state} target={target} stable={stable}/{stable_frames} "
            f"full_nonblack={quality.full_nonblack:.4f} "
            f"viewport_nonblack={quality.viewport_nonblack:.4f} "
            f"rightcol_nonblack={quality.rightcol_nonblack:.4f} "
            f"blackout={int(quality.blackout)} "
            f"backend={quality.capture_backend} "
            f"source={quality.capture_source} "
            f"active={quality.host_active_app}",
            file=sys.stderr,
        )
        if _should_abort_for_blackout(recent_quality, target, blackout_frame_limit):
            print(
                "FAIL capture blackout: repeated all-black host frames for "
                f"target={target}; see {capture_root / 'state-samples' / 'quality.jsonl'}",
                file=sys.stderr,
            )
            return False, "capture_blackout"
        if _should_abort_for_focus_mismatch(
                recent_quality, FOCUS_MISMATCH_FRAME_LIMIT):
            recovery_reason, recovery_quality, recovery_meta = _attempt_focus_recovery(
                capture_root, recent_quality,
            )
            print(
                f"rawshot focus recovery probe: reason={recovery_reason} "
                f"visible={recovery_quality is not None and not recovery_quality.blackout} "
                f"backend={recovery_meta.get('capture_backend') if recovery_meta else 'n/a'}",
                file=sys.stderr,
            )
            if recovery_reason == "rawshot_focus_recovered":
                # Drop the focus-mismatch window so the live route can
                # keep advancing instead of immediately re-aborting on
                # the same samples.  The recovery_meta/recovery_quality
                # are written to the focus-recovery receipt below so a
                # future operator can see what saved the session.
                _write_focus_recovery_receipt(
                    capture_root,
                    reason=recovery_reason,
                    recent=recent_quality,
                    rawshot_quality=recovery_quality,
                    rawshot_meta=recovery_meta,
                )
                recent_quality = []
            else:
                print(
                    "FAIL capture focus: repeated host samples show a "
                    "non-DOSBox frontmost app while waiting for "
                    f"target={target}; rawshot recovery reason="
                    f"{recovery_reason}; see "
                    f"{capture_root / 'state-samples' / 'quality.jsonl'}",
                    file=sys.stderr,
                )
                _write_focus_recovery_receipt(
                    capture_root,
                    reason=recovery_reason,
                    recent=recent_quality,
                    rawshot_quality=recovery_quality,
                    rawshot_meta=recovery_meta,
                )
                return False, "capture_focus_mismatch"
        if state == target:
            stable += 1
            if stable >= stable_frames:
                return True, state
        else:
            stable = 0
        time.sleep(screenshot_int)
    return False, last_state


def _write_live_conf(conf: Path, runtime_dir: Path, capture_root: Path) -> None:
    """Write the conf used for live automation.

    The preflight receipt proves the canonical hash root.  For actually
    launching DM.EXE, DOSBox needs the original runtime layout where
    DM.EXE and DATA/ live together.  The live command line passes DM.EXE
    as DOSBox's PATH argument, so this conf only owns render/capture/input
    settings.
    """
    conf.parent.mkdir(parents=True, exist_ok=True)
    (capture_root / "dosbox-capture").mkdir(parents=True, exist_ok=True)
    conf.write_text(
        "\n".join([
            "[sdl]",
            "output=opengl",
            "windowresolution=1024x768",
            "viewport_resolution=1024x768",
            f"mapperfile={capture_root / 'dosbox_capture.mapper.map'}",
            "",
            "[dosbox]",
            "machine=svga_s3",
            "memsize=16",
            "",
            "[render]",
            "frameskip=0",
            "aspect=auto",
            # No CRT shader: the dosbox-rawshot backend drives DOSBox's own
            # rendered screenshot (Alt+F5) because macOS swallows the default
            # raw-screenshot Ctrl+F5 binding.  glshader=none keeps the
            # rendered capture a clean square-pixel framebuffer with no
            # scanline/curvature overlay, so the density classifier reads it
            # the same way it reads a raw VGA dump.
            "glshader=none",
            "",
            "[cpu]",
            "core=dynamic",
            "cycles=max",
            "",
            "[capture]",
            f"capture_dir={capture_root / 'dosbox-capture'}",
            # Request both rendered and raw: the live route triggers the
            # rendered screenshot (Alt+F5, the only screenshot key macOS
            # actually delivers), and the loader accepts raw/PNG/BMP so a
            # future operator who rebinds the raw key still works unchanged.
            "default_image_capture_formats=rendered raw",
            "",
            "[mouse]",
            # The entrance click is proven with DOSBox Staging's default
            # onclick capture path.  Keep it explicit so the live receipt
            # records the mouse mode instead of inheriting a global default.
            "mouse_capture=onclick",
            "mouse_sensitivity=100",
            "mouse_raw_input=true",
            "dos_mouse_driver=true",
            "dos_mouse_immediate=true",
            "",
        ]),
        encoding="utf-8",
    )


def _sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def _write_live_input_receipt(
        capture_root: Path,
        runtime_dir: Path,
        dosbox_bin: str,
        conf: Path,
        capture_backend: str) -> Path:
    """Write a deterministic receipt for no-launch validation/live launch.

    The preflight receipt proves the canonical asset hashes; this live
    receipt proves the host-side launch shape that has been blocking
    original captures: resolved DOSBox binary, extracted runtime
    layout, generated conf, and the capture directory DOSBox will use.
    """
    capture_root.mkdir(parents=True, exist_ok=True)
    required = {}
    data_dir = runtime_dir / "DATA"
    for filename in RUNTIME_DATA_REQUIRED_FILES:
        required[filename] = _has_case_insensitive_child(data_dir, filename)
    receipt = {
        "schema": LIVE_INPUT_RECEIPT_SCHEMA,
        "dosbox_bin": str(dosbox_bin),
        "runtime_dir": str(runtime_dir),
        "runtime_has_dm_exe": (runtime_dir / "DM.EXE").is_file(),
        "runtime_data_dir": str(data_dir),
        "runtime_required_files": required,
        "capture_root": str(capture_root),
        "capture_backend": capture_backend,
        "dosbox_capture_dir": str(capture_root / "dosbox-capture"),
        "conf_path": str(conf),
        "conf_sha256": _sha256_file(conf),
        "live_conf_pins": {
            "output": "opengl",
            "windowresolution": "1024x768",
            "viewport_resolution": "1024x768",
            "mapperfile": str(capture_root / "dosbox_capture.mapper.map"),
            "machine": "svga_s3",
            "memsize": "16",
            "frameskip": "0",
            "aspect": "auto",
            "scaler": "n/a (DOSBox Staging OpenGL/glshader path)",
            "glshader": "none",
            "core": "dynamic",
            "cycles": "max",
            "default_image_capture_formats": "rendered raw",
            "mouse_capture": "onclick",
            "mouse_sensitivity": "100",
            "mouse_raw_input": "true",
            "dos_mouse_driver": "true",
            "dos_mouse_immediate": "true",
        },
    }
    out = capture_root / "dosbox_capture.live_inputs.json"
    out.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n",
                   encoding="utf-8")
    return out


def _last_focus_mismatch_in_window(
        recent: list[FrameQuality],
        focus_frame_limit: int) -> FrameQuality | None:
    """Return the most recent focus-mismatched quality in the window.

    Used by ``_write_focus_recovery_receipt`` so the receipt points at
    the exact sample that triggered the rawshot-fallback probe (rather
    than a synthetic stand-in).  Returns ``None`` when the window has
    no focus-mismatched sample.
    """
    if focus_frame_limit <= 0 or not recent:
        return None
    window = recent[-focus_frame_limit:]
    for item in reversed(window):
        if _is_focus_mismatch(item):
            return item
    return None


def _write_focus_recovery_receipt(
        capture_root: Path,
        reason: str,
        recent: list[FrameQuality],
        rawshot_quality: FrameQuality | None,
        rawshot_meta: dict[str, str] | None) -> Path:
    """Write the focus-recovery decision receipt.

    The live abort receipt captures the *first* failing step.  This
    receipt captures the rawshot-fallback decision that the focus
    window triggered, so a future operator can answer two questions
    without re-reading the log:

    * did the rawshot probe actually run? (the
      ``rawshot_meta`` ``capture_backend`` / ``capture_source`` /
      ``capture_source_sha256`` fields, plus the
      ``rawshot_attempted`` boolean);
    * did the rawshot decode into a usable frame?
      (the ``rawshot_quality`` ``full_nonblack`` / ``viewport_nonblack``
      / ``blackout`` fields, plus the ``rawshot_visible`` boolean).

    The schema is versioned (``LIVE_FOCUS_RECOVERY_RECEIPT_SCHEMA``)
    so a future patch that changes the field set can be detected by
    a regression test that asserts the schema id rather than re-reading
    the JSON shape.
    """
    capture_root.mkdir(parents=True, exist_ok=True)
    if reason not in RAWSHOT_FOCUS_RECOVERY_REASONS:
        raise ValueError(
            f"unsupported focus recovery reason: {reason!r}; "
            f"expected one of {RAWSHOT_FOCUS_RECOVERY_REASONS!r}"
        )
    trigger = _last_focus_mismatch_in_window(recent, FOCUS_MISMATCH_FRAME_LIMIT)
    receipt = {
        "schema": LIVE_FOCUS_RECOVERY_RECEIPT_SCHEMA,
        "reason": reason,
        "focus_mismatch_frame_limit": FOCUS_MISMATCH_FRAME_LIMIT,
        "focus_window_size": min(len(recent), FOCUS_MISMATCH_FRAME_LIMIT),
        "trigger_quality": asdict(trigger) if trigger is not None else None,
        "rawshot_attempted": rawshot_meta is not None,
        "rawshot_visible": bool(
            rawshot_quality is not None and not rawshot_quality.blackout),
        "rawshot_quality": asdict(rawshot_quality) if rawshot_quality is not None else None,
        "rawshot_meta": dict(rawshot_meta) if rawshot_meta is not None else None,
    }
    out = capture_root / "dosbox_capture.focus_recovery.json"
    out.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n",
                   encoding="utf-8")
    return out


def _last_quality_from_log(capture_root: Path) -> FrameQuality | None:
    log_path = capture_root / "state-samples" / "quality.jsonl"
    try:
        lines = [line.strip() for line in log_path.read_text(encoding="utf-8").splitlines()
                 if line.strip()]
    except OSError:
        return None
    if not lines:
        return None
    try:
        data = json.loads(lines[-1])
        return FrameQuality(
            label=str(data.get("label", "")),
            state=str(data.get("state", "")),
            width=int(data.get("width", 0)),
            height=int(data.get("height", 0)),
            full_nonblack=float(data.get("full_nonblack", 0.0)),
            viewport_nonblack=float(data.get("viewport_nonblack", 0.0)),
            rightcol_nonblack=float(data.get("rightcol_nonblack", 0.0)),
            champion_nonblack=float(data.get("champion_nonblack", 0.0)),
            blackout=bool(data.get("blackout", False)),
            capture_backend=str(data.get("capture_backend", "unknown")),
            capture_source=str(data.get("capture_source", "")),
            capture_source_sha256=str(data.get("capture_source_sha256", "")),
            normalized_rgb_sha256=str(data.get("normalized_rgb_sha256", "")),
            host_active_app=str(data.get("host_active_app", "")),
            dosbox_window_bounds=str(data.get("dosbox_window_bounds", "")),
        )
    except (TypeError, ValueError, json.JSONDecodeError):
        return None


def _focus_recovery_summary(capture_root: Path) -> dict[str, object] | None:
    """Read the focus-recovery receipt and return a compact summary.

    Used by ``_write_live_abort_receipt`` so an operator looking at
    only the live abort receipt can still see whether the rawshot
    fallback saved the focus window (``rawshot_focus_recovered``) or
    gave up (``rawshot_focus_unrecoverable``).  Returns ``None`` when
    the focus-recovery receipt is absent, which is the steady-state
    case (no focus issue ever triggered the rawshot probe).
    """
    path = capture_root / "dosbox_capture.focus_recovery.json"
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None
    if not isinstance(data, dict):
        return None
    if data.get("schema") != LIVE_FOCUS_RECOVERY_RECEIPT_SCHEMA:
        return None
    return {
        "path": str(path),
        "reason": data.get("reason"),
        "rawshot_attempted": bool(data.get("rawshot_attempted")),
        "rawshot_visible": bool(data.get("rawshot_visible")),
        "focus_window_size": data.get("focus_window_size"),
    }


def _write_live_abort_receipt(
        capture_root: Path,
        step: PlanStep,
        last_state: str,
        reason: str,
        quality: FrameQuality | None = None) -> Path:
    """Write the machine-readable reason a live capture stopped.

    The live-input receipt records the launch shape.  This abort
    receipt records the first failing state-machine step, including the
    same frame-quality metrics printed to stderr, so a black-host-frame
    failure can be compared across attempts without re-reading logs.

    When the focus-recovery path produced a focus-recovery receipt
    (``dosbox_capture.focus_recovery.json``), the abort receipt also
    carries a compact ``focus_recovery`` summary so an operator can
    see at a glance whether the rawshot fallback saved the focus
    window or gave up.  The full receipt is at the path listed in
    ``focus_recovery.path``.
    """
    capture_root.mkdir(parents=True, exist_ok=True)
    if quality is None:
        quality = _last_quality_from_log(capture_root)
    key_dispatch = _last_key_dispatch_from_log(capture_root)
    focus_summary = _focus_recovery_summary(capture_root)
    receipt = {
        "schema": LIVE_ABORT_RECEIPT_SCHEMA,
        "step": step.name,
        "expected_state": step.expected_state,
        "last_state": last_state,
        "reason": reason,
        "timeout_s": step.timeout_s,
        "stable_frames_required": step.stable_frames,
        "keys": list(step.keys),
        "capture_root": str(capture_root),
        "state_samples_dir": str(capture_root / "state-samples"),
        "quality_log": str(capture_root / "state-samples" / "quality.jsonl"),
        "key_dispatch_log": str(_key_dispatch_log_path(capture_root)),
        "focus_recovery": focus_summary,
        "last_key_dispatch": asdict(key_dispatch) if key_dispatch is not None else None,
        "last_frame_quality": asdict(quality) if quality is not None else None,
    }
    out = capture_root / "dosbox_capture.live_abort.json"
    out.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n",
                   encoding="utf-8")
    return out


def _write_live_movement_receipt(
        capture_root: Path,
        before_quality: FrameQuality,
        after_quality: FrameQuality,
        before_viewport_sha256: str,
        after_viewport_sha256: str,
        before_path: Path,
        after_path: Path,
        movement_changed: bool,
        action_key: str,
        input_method: str,
        source_anchor: str,
        action_attempts: list[dict[str, object]]) -> Path:
    """Write the local hash/density proof for the in-dungeon movement action."""
    capture_root.mkdir(parents=True, exist_ok=True)
    key_dispatch = _last_key_dispatch_from_log(capture_root)
    receipt = {
        "schema": LIVE_MOVEMENT_RECEIPT_SCHEMA,
        "capture_root": str(capture_root),
        "action_key": action_key,
        "input_method": input_method,
        "source_anchor": source_anchor,
        "screen_coord_320x200": {
            "x": DUNGEON_MOVE_FORWARD_CLICK_SCREEN_COORD[0],
            "y": DUNGEON_MOVE_FORWARD_CLICK_SCREEN_COORD[1],
        },
        "framebuffer_fraction": {
            "x": DUNGEON_MOVE_FORWARD_CLICK_FRAC[0],
            "y": DUNGEON_MOVE_FORWARD_CLICK_FRAC[1],
        },
        "keyboard_key": DUNGEON_MOVE_FORWARD_KEYBOARD_KEY,
        "keyboard_source_anchor": DUNGEON_MOVE_FORWARD_KEYBOARD_SOURCE,
        "action_attempts": action_attempts,
        "before_frame": str(before_path),
        "after_frame": str(after_path),
        "before_quality": asdict(before_quality),
        "after_quality": asdict(after_quality),
        "before_viewport_rgb_sha256": before_viewport_sha256,
        "after_viewport_rgb_sha256": after_viewport_sha256,
        "normalized_rgb_changed": (
            before_quality.normalized_rgb_sha256
            != after_quality.normalized_rgb_sha256
        ),
        "viewport_rgb_changed": movement_changed,
        "last_key_dispatch": asdict(key_dispatch) if key_dispatch is not None else None,
        "non_claims": [
            "This is original-DOS movement-capture evidence, not a pixel-parity promotion.",
            "The C070 mouse probe is preserved separately from the keyboard fallback result.",
            "The proprietary game frames remain in the operator-local capture root.",
        ],
    }
    out = capture_root / "dosbox_capture.in_dungeon_movement.json"
    out.write_text(json.dumps(receipt, indent=2, sort_keys=True) + "\n",
                   encoding="utf-8")
    return out


def _capture_after_movement_until_distinct(
        capture_root: Path,
        before_viewport_sha256: str,
        capture_backend: str,
        timeout_s: float,
        screenshot_int: float) -> tuple[object, dict[str, str], FrameQuality, str, bool]:
    """Capture after a movement click until the viewport hash changes."""
    deadline = time.time() + timeout_s
    sample = 0
    last_img = None
    last_meta: dict[str, str] = {}
    last_quality: FrameQuality | None = None
    last_viewport_sha256 = before_viewport_sha256
    while time.time() < deadline:
        sample += 1
        label = f"capture_02_ingame_step_forward_{sample:04d}"
        img, capture_meta = _screenshot_frame(capture_root, label, capture_backend)
        state = classify(img)
        quality = _frame_quality(img, label, state, capture_meta)
        _write_quality_log(capture_root, quality)
        viewport_sha256 = _viewport_rgb_sha256(img)
        print(
            f"movement sample={sample} state={state} "
            f"viewport_sha_changed={int(viewport_sha256 != before_viewport_sha256)} "
            f"viewport_nonblack={quality.viewport_nonblack:.4f} "
            f"rightcol_nonblack={quality.rightcol_nonblack:.4f} "
            f"backend={quality.capture_backend}",
            file=sys.stderr,
        )
        last_img = img
        last_meta = capture_meta
        last_quality = quality
        last_viewport_sha256 = viewport_sha256
        if state == "dungeon_gameplay" and viewport_sha256 != before_viewport_sha256:
            return img, capture_meta, quality, viewport_sha256, True
        time.sleep(screenshot_int)
    if last_img is None or last_quality is None:
        raise RuntimeError("movement capture loop produced no samples")
    return last_img, last_meta, last_quality, last_viewport_sha256, False


def live_run(plan: list[PlanStep], args: argparse.Namespace) -> int:
    """Drive a real DOSBox Staging DM1 session and capture normalized frames."""
    if not _DETECTOR_OK:
        print("Pillow and numpy are required for --live", file=sys.stderr)
        return 2
    if shutil.which("cliclick") is None:
        print("cliclick is required for --live", file=sys.stderr)
        return 2
    dosbox_bin = _resolve_dosbox_bin(args)
    if dosbox_bin is None:
        print(
            "DOSBox executable not found; pass --dosbox-bin, set DOSBOX_BIN, "
            "or put dosbox-staging/dosbox on PATH",
            file=sys.stderr,
        )
        return 2

    capture_root = args.capture_root.expanduser()
    game_dir = (
        args.game_dir.expanduser()
        if args.game_dir is not None
        else Path("~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1").expanduser()
    )
    default_runtime_dir = Path(
        "~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
    ).expanduser()
    runtime_dir = (
        args.runtime_dir.expanduser()
        if args.runtime_dir is not None
        else default_runtime_dir if default_runtime_dir.is_dir()
        else None
    )
    if runtime_dir is not None:
        failures = _runtime_dir_failures(runtime_dir)
        if failures:
            for failure in failures:
                print(f"live input validation failed: {failure}", file=sys.stderr)
            return 2
        conf = capture_root / "dosbox_capture.live.conf"
        _write_live_conf(conf, runtime_dir, capture_root)
        receipt = _write_live_input_receipt(
            capture_root=capture_root,
            runtime_dir=runtime_dir,
            dosbox_bin=dosbox_bin,
            conf=conf,
            capture_backend=args.capture_backend,
        )
    else:
        conf = capture_root / "dosbox_capture.conf"
        if not conf.exists():
            print(
                "no runtime dir found and no preflight dosbox_capture.conf exists; "
                f"pass --runtime-dir or run preflight for {game_dir}",
                file=sys.stderr,
            )
            return 2
        receipt = None

    original_dir = capture_root / "original"
    original_dir.mkdir(parents=True, exist_ok=True)
    print(f"launching DOSBox with {conf} via {dosbox_bin}", file=sys.stderr)
    if receipt is not None:
        print(f"live input receipt: {receipt}", file=sys.stderr)
    proc = subprocess.Popen(
        _dosbox_conf_command(dosbox_bin, conf, runtime_dir),
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        env={**os.environ, "LANG": os.environ.get("LANG", "C")},
    )
    try:
        # DM.EXE can still be in the DOS startup batch on DOSBox Staging
        # 0.82.2 after the window appears.  Let the selector reach its first
        # menu before route keys are dispatched; otherwise 1/1/4 land at Z:\>.
        time.sleep(LIVE_DOSBOX_STARTUP_SETTLE_S)
        _activate_dosbox()
        for step in plan:
            for key in step.keys:
                print(f"send={key}", file=sys.stderr)
                _dispatch_key_for_live_step(capture_root, step.name, key)
            if step.settle_only:
                # DM.EXE SELECTOR text menus: do not gate on the dungeon
                # density classifier.  Dwell for a deterministic capture
                # and take one labelled screenshot so the state-samples
                # log still has evidence for this step, then proceed.
                time.sleep(step.settle_s)
                try:
                    img, capture_meta = _screenshot_frame(
                        capture_root, f"{step.name}_settle", args.capture_backend,
                    )
                    quality = _frame_quality(
                        img, f"{step.name}_settle", classify(img), capture_meta,
                    )
                    _write_quality_log(capture_root, quality)
                    print(
                        f"settle step={step.name} state={quality.state} "
                        f"full_nonblack={quality.full_nonblack:.4f} "
                        f"backend={quality.capture_backend}",
                        file=sys.stderr,
                    )
                except Exception as exc:  # pragma: no cover - host-only
                    print(
                        f"settle step={step.name} screenshot skipped: {exc}",
                        file=sys.stderr,
                    )
                continue
            ok, last = _wait_for_state(
                capture_root,
                step.expected_state,
                min(step.timeout_s, args.state_timeout),
                args.screenshot_int,
                step.stable_frames,
                args.blackout_frame_limit,
                args.capture_backend,
            )
            if not ok:
                abort = _write_live_abort_receipt(
                    capture_root=capture_root,
                    step=step,
                    last_state=last,
                    reason=last,
                )
                print(
                    f"FAIL live step={step.name} expected={step.expected_state} last={last}",
                    file=sys.stderr,
                )
                print(f"live abort receipt: {abort}", file=sys.stderr)
                return 1

        start, _meta = _screenshot_frame(
            capture_root,
            "capture_01_ingame_start",
            args.capture_backend,
        )
        start_path = original_dir / "01_ingame_start.png"
        start.save(start_path)
        start_quality = _frame_quality(
            start,
            "capture_01_ingame_start",
            classify(start),
            _meta,
        )
        _write_quality_log(capture_root, start_quality)
        start_viewport_sha256 = _viewport_rgb_sha256(start)

        action_attempts: list[dict[str, object]] = []

        print(f"send={DUNGEON_MOVE_FORWARD_CLICK_KEY}", file=sys.stderr)
        _dispatch_key_for_live_step(
            capture_root,
            "in_dungeon_move_forward_mouse_probe",
            DUNGEON_MOVE_FORWARD_CLICK_KEY,
        )
        step, _step_meta, step_quality, step_viewport_sha256, moved = (
            _capture_after_movement_until_distinct(
                capture_root,
                start_viewport_sha256,
                args.capture_backend,
                timeout_s=4.0,
                screenshot_int=args.screenshot_int,
            )
        )
        action_attempts.append({
            "action_key": DUNGEON_MOVE_FORWARD_CLICK_KEY,
            "input_method": "absolute_mouse_click_with_mouse_capture_onclick",
            "source_anchor": DUNGEON_MOVE_FORWARD_SOURCE,
            "after_viewport_rgb_sha256": step_viewport_sha256,
            "viewport_rgb_changed": moved,
        })
        final_action_key = DUNGEON_MOVE_FORWARD_CLICK_KEY
        final_input_method = "absolute_mouse_click_with_mouse_capture_onclick"
        final_source_anchor = DUNGEON_MOVE_FORWARD_SOURCE
        if not moved:
            print(
                "C070 mouse probe did not change viewport; "
                f"send={DUNGEON_MOVE_FORWARD_KEYBOARD_KEY}",
                file=sys.stderr,
            )
            _dispatch_key_for_live_step(
                capture_root,
                "in_dungeon_move_forward_keyboard_fallback",
                DUNGEON_MOVE_FORWARD_KEYBOARD_KEY,
            )
            step, _step_meta, step_quality, step_viewport_sha256, moved = (
                _capture_after_movement_until_distinct(
                    capture_root,
                    start_viewport_sha256,
                    args.capture_backend,
                    timeout_s=8.0,
                    screenshot_int=args.screenshot_int,
                )
            )
            action_attempts.append({
                "action_key": DUNGEON_MOVE_FORWARD_KEYBOARD_KEY,
                "input_method": "keyboard_key_code_keypad_5",
                "source_anchor": DUNGEON_MOVE_FORWARD_KEYBOARD_SOURCE,
                "after_viewport_rgb_sha256": step_viewport_sha256,
                "viewport_rgb_changed": moved,
            })
            final_action_key = DUNGEON_MOVE_FORWARD_KEYBOARD_KEY
            final_input_method = "keyboard_keypad_5_after_c070_mouse_probe"
            final_source_anchor = DUNGEON_MOVE_FORWARD_KEYBOARD_SOURCE
        step_path = original_dir / "02_ingame_step_forward.png"
        step.save(step_path)
        movement_receipt = _write_live_movement_receipt(
            capture_root,
            before_quality=start_quality,
            after_quality=step_quality,
            before_viewport_sha256=start_viewport_sha256,
            after_viewport_sha256=step_viewport_sha256,
            before_path=start_path,
            after_path=step_path,
            movement_changed=moved,
            action_key=final_action_key,
            input_method=final_input_method,
            source_anchor=final_source_anchor,
            action_attempts=action_attempts,
        )
        if not moved:
            print(
                "FAIL in-dungeon movement: neither the C070 forward-arrow "
                "mouse probe nor the source-locked Keypad-5 fallback changed "
                "the viewport hash; see "
                f"{movement_receipt}",
                file=sys.stderr,
            )
            return 1
        print(
            f"PASS live captures written: {original_dir} "
            f"movement_receipt={movement_receipt}"
        )
        return 0
    finally:
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait(timeout=5)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Run a DM1 PC 3.4 DOSBox capture session "
                    "(or --dry-run the state machine against synth fixtures).",
    )
    parser.add_argument("--plan", action="store_true",
                        help="print the planned key sequence as JSON to stdout and exit")
    parser.add_argument("--plan-out", type=Path, default=None,
                        help="write the planned key sequence to this JSON file")
    parser.add_argument("--dry-run", action="store_true",
                        help="walk the state machine using synthetic fixtures (no DOSBox)")
    parser.add_argument("--live", action="store_true",
                        help="drive a real DOSBox session via cliclick (macOS only)")
    parser.add_argument("--validate-live-inputs", action="store_true",
                        help="validate the live runtime layout and write the live conf "
                             "without launching DOSBox")
    parser.add_argument("--game-dir", type=Path, default=None,
                        help="DM1 game data root (default: "
                             "~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1)")
    parser.add_argument("--runtime-dir", type=Path, default=None,
                        help="DM1 DOS runtime dir containing DM.EXE and DATA/ "
                             "(default: local extracted PC 3.4 runtime when present)")
    parser.add_argument("--capture-root", type=Path,
                        default=Path.home() / "firestaff-captures",
                        help="where to write captured frames")
    parser.add_argument("--dosbox-bin", default=None,
                        help="DOSBox executable for --live/--validate-live-inputs "
                             "(default: DOSBOX_BIN, then dosbox-staging, then dosbox)")
    parser.add_argument("--screenshot-int", type=float, default=0.5,
                        help="seconds between classifier samples (default 0.5)")
    parser.add_argument("--state-timeout", type=float, default=300.0,
                        help="give up on a state after N seconds (default 300)")
    parser.add_argument("--blackout-frame-limit", type=int, default=6,
                        help="abort after N consecutive all-black host captures "
                             "(default 6; set 0 to disable)")
    parser.add_argument("--capture-backend",
                        choices=("auto", "peekaboo", "screencapture", "dosbox-rawshot"),
                        default="auto",
                        help="host screenshot backend for --live (default auto: "
                             "Peekaboo when available, else screencapture; "
                             "dosbox-rawshot uses DOSBox's internal rendered screenshot via Alt+F5)")
    args = parser.parse_args(argv)

    plan = DEFAULT_PLAN
    if args.plan or args.plan_out is not None:
        dump_plan(plan, args.plan_out)
        return 0
    if args.dry_run:
        matched, total, failures = dry_run(plan)
        print(f"dry-run: {matched}/{total} state machine transitions match the classifier")
        if failures:
            print("FAIL:")
            for f in failures:
                print(f"  - {f}")
            return 1
        print("PASS")
        return 0
    if args.validate_live_inputs:
        return validate_live_inputs(args)
    if args.live:
        return live_run(plan, args)
    parser.print_help()
    return 0


if __name__ == "__main__":
    sys.exit(main())
