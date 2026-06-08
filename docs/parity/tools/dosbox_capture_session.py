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
  `unclassified` step instead of after a full session.
* Runs in `--plan` mode to dump the planned key sequence, expected
  state after each key, and timeout budget to stdout.  Useful for
  hand-running a session.
* Runs in `--live` mode to drive a real DOSBox Staging session.
  This is a thin wrapper over the documented behaviour and inherits
  the same `cliclick`-based key dispatch from the runbook.

The script intentionally does not depend on `cliclick` being installed
for the `--dry-run` and `--plan` modes.  The capture pipeline only
needs it for `--live` mode on macOS.
"""
from __future__ import annotations

import argparse
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


# The capture route plan.  This mirrors the runbook §3 state machine
# but uses the calibrated band 0.135 thresholds from the post-fix
# classifier.  Stable-frames requirement follows the runbook default.
DEFAULT_PLAN: list[PlanStep] = [
    PlanStep("title_screen",     "title_screen",     timeout_s=60.0),
    PlanStep("entrance_menu",    "entrance_menu",    keys=["Return", "Return"], timeout_s=30.0),
    PlanStep("graphics_select",  "entrance_menu",    keys=["0", "Return"],       timeout_s=15.0),
    PlanStep("sound_select",     "entrance_menu",    keys=["0", "Return"],       timeout_s=15.0),
    PlanStep("start_game",       "entrance_menu",    keys=["Return"],            timeout_s=15.0),
    PlanStep("champion_create",  "champion_create",  timeout_s=60.0),
    PlanStep("accept_champions", "champion_create",  keys=["Return"] * 4,        timeout_s=30.0),
    PlanStep("dungeon_gameplay", "dungeon_gameplay", timeout_s=120.0),
]

KEY_MAP = {
    "Return": "return",
    "Key-Up": "arrow-up",
    "Key-Down": "arrow-down",
    "Key-Left": "arrow-left",
    "Key-Right": "arrow-right",
}

DOSBOX_PROCESS_NAMES = ("DOSBox", "DOSBox Staging", "dosbox", "dosbox-staging")
DOSBOX_BIN_CANDIDATES = ("dosbox-staging", "dosbox")
BLACKOUT_NONBLACK_THRESH = 0.005


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
    for candidate in DOSBOX_BIN_CANDIDATES:
        found = shutil.which(candidate)
        if found:
            return found
    return None


def _is_executable_file(path: Path) -> bool:
    return path.is_file() and os.access(path, os.X_OK)


def _runtime_dir_failures(runtime_dir: Path) -> list[str]:
    """Return validation failures for the DOS runtime layout.

    The canonical hash root proved by preflight is not always the
    directory DOSBox can execute from.  The live route must mount the
    extracted PC 3.4 runtime subdir where DM.EXE can see its DATA/
    sibling; mounting the parent hash root can leave DOSBox at a menu
    or a black host capture even though the SHA preflight passed.
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
    if not (runtime_dir / "DATA").is_dir():
        failures.append(f"runtime dir missing DATA/ sibling: {runtime_dir}")
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
    print(
        "PASS live input validation: "
        f"dosbox_bin={dosbox_bin} runtime={runtime_dir} conf={conf}"
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
    if _should_abort_for_blackout(blackouts, "title_screen", 6):
        failures.append("blackout guard: aborted the legitimate mostly-black title step")
    with tempfile.TemporaryDirectory(prefix="dm1-live-runtime-") as tmp:
        tmp_root = Path(tmp)
        good = tmp_root / "DungeonMasterPC34"
        good.mkdir()
        (good / "DM.EXE").write_bytes(b"DM_FIXTURE\n")
        (good / "DATA").mkdir()
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


def _activate_dosbox() -> None:
    """Best-effort focus for macOS DOSBox Staging windows."""
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


def _screenshot_frame(capture_root: Path, label: str):
    if not _DETECTOR_OK:
        raise RuntimeError("Pillow and numpy are required for --live")
    from PIL import Image

    capture_root.mkdir(parents=True, exist_ok=True)
    raw = capture_root / "state-samples" / f"{label}.png"
    raw.parent.mkdir(parents=True, exist_ok=True)
    _activate_dosbox()
    bounds = _dosbox_window_bounds()
    if bounds is not None:
        x, y, w, h = bounds
        cmd = ["screencapture", "-x", "-R", f"{x},{y},{w},{h}", str(raw)]
    else:
        cmd = ["screencapture", "-x", str(raw)]
    subprocess.run(cmd, check=True)
    img = Image.open(raw).convert("RGB")
    resample = getattr(getattr(Image, "Resampling", Image), "NEAREST")
    return _crop_to_4x3(img).resize((320, 200), resample)


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


def _frame_quality(img, label: str, state: str) -> FrameQuality:
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
    )


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
    if target == "title_screen" or blackout_frame_limit <= 0:
        return False
    if len(recent) < blackout_frame_limit:
        return False
    window = recent[-blackout_frame_limit:]
    return all(item.blackout for item in window)


def _press_key(key: str) -> None:
    time.sleep(0.25)
    mapped = KEY_MAP.get(key)
    if mapped is not None:
        subprocess.run(["cliclick", f"kp:{mapped}"], check=True)
    elif len(key) == 1:
        subprocess.run(["cliclick", f"t:{key}"], check=True)
    else:
        raise ValueError(f"unsupported live key: {key}")


def _wait_for_state(capture_root: Path, target: str, timeout_s: float,
                    screenshot_int: float, stable_frames: int,
                    blackout_frame_limit: int) -> tuple[bool, str]:
    deadline = time.time() + timeout_s
    stable = 0
    last_state = "unknown"
    sample = 0
    recent_quality: list[FrameQuality] = []
    while time.time() < deadline:
        sample += 1
        label = f"{target}_{sample:04d}"
        img = _screenshot_frame(capture_root, label)
        state = classify(img)
        last_state = state
        quality = _frame_quality(img, label, state)
        _write_quality_log(capture_root, quality)
        recent_quality.append(quality)
        print(
            f"state={state} target={target} stable={stable}/{stable_frames} "
            f"full_nonblack={quality.full_nonblack:.4f} "
            f"viewport_nonblack={quality.viewport_nonblack:.4f} "
            f"rightcol_nonblack={quality.rightcol_nonblack:.4f} "
            f"blackout={int(quality.blackout)}",
            file=sys.stderr,
        )
        if _should_abort_for_blackout(recent_quality, target, blackout_frame_limit):
            print(
                "FAIL capture blackout: repeated all-black host frames for "
                f"target={target}; see {capture_root / 'state-samples' / 'quality.jsonl'}",
                file=sys.stderr,
            )
            return False, "capture_blackout"
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
    DM.EXE and DATA/ live together.
    """
    conf.parent.mkdir(parents=True, exist_ok=True)
    (capture_root / "dosbox-capture").mkdir(parents=True, exist_ok=True)
    conf.write_text(
        "\n".join([
            "[sdl]",
            "output=opengl",
            "windowresolution=1024x768",
            "viewport_resolution=1024x768",
            "",
            "[dosbox]",
            "machine=svga_s3",
            "memsize=16",
            "",
            "[render]",
            "frameskip=0",
            "",
            "[cpu]",
            "core=dynamic",
            "cycles=max",
            "",
            "[capture]",
            f"capture_dir={capture_root / 'dosbox-capture'}",
            "default_image_capture_formats=raw",
            "",
            "[autoexec]",
            f"MOUNT C {runtime_dir}",
            "C:",
            "DM.EXE",
            "",
        ]),
        encoding="utf-8",
    )


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
    else:
        conf = capture_root / "dosbox_capture.conf"
        if not conf.exists():
            print(
                "no runtime dir found and no preflight dosbox_capture.conf exists; "
                f"pass --runtime-dir or run preflight for {game_dir}",
                file=sys.stderr,
            )
            return 2

    original_dir = capture_root / "original"
    original_dir.mkdir(parents=True, exist_ok=True)
    print(f"launching DOSBox with {conf} via {dosbox_bin}", file=sys.stderr)
    proc = subprocess.Popen(
        [dosbox_bin, "-conf", str(conf)],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        env={**os.environ, "LANG": os.environ.get("LANG", "C")},
    )
    try:
        time.sleep(2.0)
        _activate_dosbox()
        for step in plan:
            for key in step.keys:
                print(f"send={key}", file=sys.stderr)
                _press_key(key)
            ok, last = _wait_for_state(
                capture_root,
                step.expected_state,
                min(step.timeout_s, args.state_timeout),
                args.screenshot_int,
                step.stable_frames,
                args.blackout_frame_limit,
            )
            if not ok:
                print(
                    f"FAIL live step={step.name} expected={step.expected_state} last={last}",
                    file=sys.stderr,
                )
                return 1

        start = _screenshot_frame(capture_root, "capture_01_ingame_start")
        start.save(original_dir / "01_ingame_start.png")
        _press_key("Key-Up")
        time.sleep(1.0)
        step = _screenshot_frame(capture_root, "capture_02_ingame_step_forward")
        step.save(original_dir / "02_ingame_step_forward.png")
        print(f"PASS live captures written: {original_dir}")
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
                        help="abort a non-title state after N consecutive all-black "
                             "host captures (default 6; set 0 to disable)")
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
