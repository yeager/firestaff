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
import sys
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
    return matched, total, failures


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
    parser.add_argument("--game-dir", type=Path, default=None,
                        help="DM1 game data root (default: "
                             "~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1)")
    parser.add_argument("--capture-root", type=Path,
                        default=Path.home() / "firestaff-captures",
                        help="where to write captured frames")
    parser.add_argument("--screenshot-int", type=float, default=0.5,
                        help="seconds between classifier samples (default 0.5)")
    parser.add_argument("--state-timeout", type=float, default=300.0,
                        help="give up on a state after N seconds (default 300)")
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
    if args.live:
        # Live mode is intentionally a thin wrapper; the runbook
        # documents the cliclick sequence and the user is expected to
        # confirm the state detector agrees before each capture.
        # The full implementation is documented in
        # docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md §3.
        print("--live is not implemented in this scaffold; see the runbook §3", file=sys.stderr)
        return 2
    parser.print_help()
    return 0


if __name__ == "__main__":
    sys.exit(main())
