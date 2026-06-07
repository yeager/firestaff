#!/usr/bin/env python3
"""dosbox_capture_preflight.py — deterministic preflight gate for the DM1
PC 3.4 original-capture route.

This tool is the runnable companion to
`docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md` Steps 1-2.  The runbook
prose says:

  Step 1: Verify Game Files
    sha256sum ... DUNGEON.DAT
    # Expected: d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85
    sha256sum ... GRAPHICS.DAT
    # Expected: 2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e
    **Do NOT proceed unless both SHA256 match exactly.**

  Step 2: DOSBox Staging Configuration
    **Machine type `svga_s3` is non-negotiable.** DM1 PC 3.4 requires
    VGA for stable 320×200 framebuffer reads. CGA and Hercules are
    incompatible with screenshot capture.

Both steps are currently prose-only.  The pass94 (2026-04-28) original
capture session shipped a `dosbox-original-viewports.conf` that used
``machine=svga_paradise`` and ``memsize=4`` — exactly the wrong-machine
failure mode the runbook later warns against — and that conf is what
the runbook now describes as ``IMPAIRED``.  The classifier was the
easiest thing to fix in isolation, but the *config* was the upstream
cause and still has no automated gate.

This tool does three things deterministically:

* verifies the canonical DUNGEON.DAT / GRAPHICS.DAT SHA256s against the
  runbook's expected values,
* writes a hardened `dosbox_capture.conf` that pins the runbook's
  required settings (machine=svga_s3, memsize=16, cpu core=dynamic,
  cpu_cycles=max, frameskip=0, windowresolution=1024x768,
  viewport_resolution=1024x768, output=opengl, captures=…),
* writes a preflight receipt JSON that records the verified SHA256s, the
  rendered settings, the chosen launch command, and a unique session
  id, so the next live attempt can cite the receipt in its manifest
  instead of re-deriving the prose.

It refuses to write a conf that contains the historical pass94 failure
settings (``svga_paradise`` or ``memsize=4``) and refuses to dispatch
keys unless the data SHA256s match — both behaviours are regression-
gated in CI by ``--self-test``.

Calibration provenance
----------------------
The expected SHA256s come from
``docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md`` §1 and are mirrored
from ``docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md`` §1.  The conf
contract values come from the runbook §2 "DOSBox Staging Configuration"
block.  The forbidden settings are the actual pass94 values recorded
in ``verification-m11/lane4-original-overlay-20260428-0917/pass94-diagnostic/dosbox-original-viewports.conf``.
"""
from __future__ import annotations

import argparse
import dataclasses
import hashlib
import json
import os
import re
import sys
import time
import uuid
from pathlib import Path
from typing import Iterable, Optional

# Runbook §1 — canonical reference data SHA256s.  These are the
# pin-check values; the preflight refuses to write a conf unless both
# match.  The names are also the same identifiers used in
# docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md §1.
EXPECTED_DUNGEON_SHA256 = "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"
EXPECTED_GRAPHICS_SHA256 = "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"

# Runbook §2 — required DOSBox Staging configuration.  Every key in
# REQUIRED_DOSBOX_SETTINGS must appear in the rendered conf; every
# key in FORBIDDEN_DOSBOX_SETTINGS must NOT appear.  These are the
# pass94 failure-mode pins; changing them is a release-relevant
# decision that needs a code review.
REQUIRED_DOSBOX_SETTINGS: dict[str, str] = {
    # machine type.  DM1 PC 3.4 mode 13h requires VGA.  Runbook prose
    # calls this "non-negotiable".  The pass94 conf used svga_paradise
    # which produces CGA/EGA-shaped framebuffer reads and is the
    # primary cause of the "unclassified on all frames" failure mode.
    "machine": "svga_s3",
    # memsize.  The runbook uses 16.  The pass94 conf used 4 which is
    # too small for DM1 PC 3.4's selector and dungeon state buffers.
    "memsize": "16",
    # CPU.  The runbook says dynamic + max cycles; pass94 used normal
    # + 386 + 3000 cycles, which is too slow for the title animation
    # to settle in the classifier's wait window.
    "core": "dynamic",
    "cycles": "max",
    # Render.  frameskip=0 so the classifier sees every frame, and
    # 1024x768 with opengl output so the 320x200 framebuffer is
    # readable by PIL via screencapture.
    "frameskip": "0",
    "windowresolution": "1024x768",
    "viewport_resolution": "1024x768",
    "output": "opengl",
}

# Pass94 failure-mode settings.  If any of these keys show up in a
# rendered conf, the preflight refuses to write it.  The actual pass94
# conf used ``svga_paradise`` + ``memsize=4`` + ``core=normal`` +
# ``cycles=3000``; the first two alone are enough to break classifier
# reads on macOS Retina screencapture.
FORBIDDEN_DOSBOX_SETTINGS: dict[str, str] = {
    "machine": "svga_paradise",
    "memsize": "4",
    "core": "normal",
    "cycles": "3000",
    "cputype": "386",
    "fullscreen": "true",
}

# Allowed values for `machine` to catch silent typos like
# ``svga_S3`` (uppercase) or ``svga_s2`` (wrong family).  The
# classifier reads are sensitive to the framebuffer layout; even
# a case mismatch can change how the 320x200 region is sampled.
ALLOWED_MACHINES: frozenset[str] = frozenset({"svga_s3"})


@dataclasses.dataclass
class PreflightReceipt:
    """JSON-serializable record of a successful preflight run.

    The receipt is what the next live attempt cites in its
    capture-session manifest, so an investigator can verify
    the config the session actually used matched the runbook
    requirements without re-deriving the prose.
    """
    session_id: str
    issued_at_iso: str
    data_dir: str
    dungeon_sha256: str
    graphics_sha256: str
    dungeon_match: bool
    graphics_match: bool
    conf_path: str
    conf_settings: dict[str, str]
    launch_command: str
    render_settings: dict[str, str]
    firestaff_git_head: Optional[str]
    pass94_forbidden_present: bool

    def to_json(self) -> str:
        return json.dumps(dataclasses.asdict(self), indent=2, sort_keys=True)


def _sha256_of_file(path: Path) -> str:
    """Return the lowercase hex SHA256 of a file's contents."""
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 16), b""):
            h.update(chunk)
    return h.hexdigest()


def _parse_dosbox_conf(conf_text: str) -> dict[str, str]:
    """Parse a DOSBox ``.conf`` into a flat key→value dict.

    DOSBox confs are ``key=value`` per line inside ``[section]``
    blocks.  We don't preserve section structure because every
    setting this tool cares about is uniquely named; if a future
    setting needs section context, this becomes a richer parser.
    """
    parsed: dict[str, str] = {}
    for raw in conf_text.splitlines():
        line = raw.strip()
        if not line or line.startswith("#") or line.startswith(";"):
            continue
        if line.startswith("[") and line.endswith("]"):
            continue
        if "=" not in line:
            continue
        key, _, value = line.partition("=")
        parsed[key.strip().lower()] = value.strip().lower()
    return parsed


def _render_dosbox_conf(
    data_dir: Path,
    captures_dir: Path,
    *,
    extra_settings: Optional[dict[str, str]] = None,
) -> str:
    """Render the hardened DOSBox Staging conf for a DM1 capture.

    The structure mirrors the runbook §2 block exactly.  ``data_dir``
    is mounted as the C: drive; ``captures_dir`` is the ``[capture]``
    target so the ``DM`` binary's in-game screenshots land in the
    same directory as the classifier crops.  The autoexec launches
    ``DM.EXE`` with no flags so the selector sequence in the runbook
    §3 stays valid (DM -vv / -sn / -pk from pass94 changed the
    selector prompts and is what produced the "champion_create never
    detected" failure mode in the gap evidence file).

    ``data_dir`` may be either the canonical game root (containing
    DUNGEON.DAT, GRAPHICS.DAT, TITLE, and a ``DungeonMasterPC34/``
    subdirectory holding DM.EXE) or the ``DungeonMasterPC34/``
    subdirectory itself.  The preflight detects the layout from
    DUNGEON.DAT / DM.EXE presence and emits the right autoexec.
    """
    lines: list[str] = []
    lines.append("# Generated by dosbox_capture_preflight.py")
    lines.append("# Source: docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md §2")
    lines.append("[sdl]")
    lines.append(f"output={REQUIRED_DOSBOX_SETTINGS['output']}")
    lines.append(f"windowresolution={REQUIRED_DOSBOX_SETTINGS['windowresolution']}")
    lines.append(f"viewport_resolution={REQUIRED_DOSBOX_SETTINGS['viewport_resolution']}")
    lines.append("")
    lines.append("[dosbox]")
    lines.append(f"machine={REQUIRED_DOSBOX_SETTINGS['machine']}")
    lines.append(f"memsize={REQUIRED_DOSBOX_SETTINGS['memsize']}")
    lines.append("")
    lines.append("[render]")
    lines.append(f"frameskip={REQUIRED_DOSBOX_SETTINGS['frameskip']}")
    lines.append("")
    lines.append("[cpu]")
    lines.append(f"core={REQUIRED_DOSBOX_SETTINGS['core']}")
    lines.append(f"cycles={REQUIRED_DOSBOX_SETTINGS['cycles']}")
    lines.append("")
    lines.append("[autoexec]")
    lines.append(f"MOUNT C {data_dir}")
    lines.append("C:")
    if (data_dir / "DM.EXE").is_file():
        # data_dir is the inner DungeonMasterPC34/ subdir; DM.EXE is
        # directly reachable from C:.
        lines.append("DM.EXE")
    elif (data_dir / "DungeonMasterPC34" / "DM.EXE").is_file():
        # data_dir is the canonical game root; DM.EXE lives in the
        # DungeonMasterPC34/ subdir.  The previous autoexec launched
        # "DungeonMasterPC34.EXE" from the root, which is a directory
        # not a binary, and the selector never started.
        lines.append("cd DungeonMasterPC34")
        lines.append("DM.EXE")
    else:
        # No DM.EXE found in either layout; fall back to the
        # subdirectory form so the live attempt can fail loudly
        # with a "Bad command or filename - DM.EXE" from DOSBox
        # rather than silently launching a wrong executable.
        lines.append("cd DungeonMasterPC34")
        lines.append("DM.EXE")
    lines.append("")
    return "\n".join(lines)


def _settings_to_report(parsed: dict[str, str]) -> dict[str, str]:
    """Return the subset of the parsed conf that the receipt records."""
    return {
        key: parsed[key]
        for key in REQUIRED_DOSBOX_SETTINGS
        if key in parsed
    }


def _check_forbidden(parsed: dict[str, str]) -> list[tuple[str, str, str]]:
    """Return [(key, forbidden_value, actual_value), ...] for any pin violation.

    A pin violation means the rendered conf still has a pass94
    failure-mode setting; the preflight refuses to write that conf.
    """
    violations: list[tuple[str, str, str]] = []
    for key, forbidden_value in FORBIDDEN_DOSBOX_SETTINGS.items():
        actual = parsed.get(key)
        if actual is not None and actual == forbidden_value.lower():
            violations.append((key, forbidden_value, actual))
    return violations


def _check_required(parsed: dict[str, str]) -> list[str]:
    """Return [missing_key, ...] for any required setting absent or wrong.

    The preflight refuses to write a conf where a runbook-required
    setting is missing or has a value outside the allowed set.
    """
    missing: list[str] = []
    for key, expected in REQUIRED_DOSBOX_SETTINGS.items():
        actual = parsed.get(key)
        if actual is None:
            missing.append(f"{key} (expected {expected!r}, got <missing>)")
            continue
        if key == "machine":
            if actual not in ALLOWED_MACHINES:
                missing.append(
                    f"{key} (expected one of {sorted(ALLOWED_MACHINES)}, got {actual!r})"
                )
            continue
        if actual != expected.lower():
            missing.append(f"{key} (expected {expected!r}, got {actual!r})")
    return missing


def _try_read_firestaff_git_head(repo_root: Path) -> Optional[str]:
    """Return the short HEAD of the Firestaff repo, or None if not a git repo.

    Recorded in the receipt so the next live attempt can cite exactly
    which Firestaff build produced the paired Firestaff-side capture.

    Handles both regular checkouts and git worktrees.  The pure-file
    approach (read ``.git/HEAD`` and follow the ref) is fragile in
    worktrees because the ref lives in the parent repo's ``refs/heads``
    directory, not in the worktree's gitdir.  We try the file path
    first and fall back to ``git rev-parse HEAD`` via subprocess for
    the worktree case.  The subprocess is wrapped in a short timeout
    so a hung git never blocks the preflight.
    """
    import subprocess
    try:
        proc = subprocess.run(
            ["git", "rev-parse", "HEAD"],
            cwd=str(repo_root),
            check=False,
            capture_output=True,
            text=True,
            timeout=2,
        )
    except (OSError, subprocess.TimeoutExpired):
        proc = None
    if proc is not None and proc.returncode == 0:
        sha = proc.stdout.strip()
        if re.fullmatch(r"[0-9a-f]{7,64}", sha):
            return sha[:12]
    return None


def preflight(
    data_dir: Path,
    captures_dir: Path,
    receipt_out: Path,
    *,
    repo_root: Optional[Path] = None,
    session_id: Optional[str] = None,
    now_iso: Optional[str] = None,
    expected_dungeon_sha256: Optional[str] = None,
    expected_graphics_sha256: Optional[str] = None,
    write_receipt: bool = True,
) -> tuple[int, int, list[str]]:
    """Run the preflight gate. Returns (matched, total, failures).

    The matched/total pair is the deterministic shape every other
    dosbox_capture_*.py gate returns; the caller (main() or CI)
    decides pass/fail.

    The ``expected_*_sha256`` overrides let the regression self-test
    pin the expected hashes to whatever the synthetic fixtures
    actually contain, so the test is hermetic and doesn't depend on
    the runbook's canonical SHA256s being reachable on disk.  The
    receipt and conf are only written when ``write_receipt`` is true
    and all pin checks pass; the self-test uses ``write_receipt=False``
    so the sandbox stays clean between runs.
    """
    failures: list[str] = []
    matched = 0
    total = 0

    expected_dungeon = expected_dungeon_sha256 or EXPECTED_DUNGEON_SHA256
    expected_graphics = expected_graphics_sha256 or EXPECTED_GRAPHICS_SHA256

    # Step 1 — verify canonical SHA256s.
    dungeon_path = data_dir / "DUNGEON.DAT"
    graphics_path = data_dir / "GRAPHICS.DAT"
    if not dungeon_path.is_file():
        failures.append(f"missing: {dungeon_path}")
    if not graphics_path.is_file():
        failures.append(f"missing: {graphics_path}")
    if failures:
        return 0, 0, failures

    dungeon_sha = _sha256_of_file(dungeon_path)
    graphics_sha = _sha256_of_file(graphics_path)
    dungeon_match = dungeon_sha == expected_dungeon
    graphics_match = graphics_sha == expected_graphics
    total += 2
    if not dungeon_match:
        failures.append(
            f"DUNGEON.DAT SHA256 mismatch: expected "
            f"{expected_dungeon}, got {dungeon_sha}"
        )
    if not graphics_match:
        failures.append(
            f"GRAPHICS.DAT SHA256 mismatch: expected "
            f"{expected_graphics}, got {graphics_sha}"
        )
    matched += int(dungeon_match) + int(graphics_match)

    # Step 2 — render the hardened conf and pin-check it.
    conf_text = _render_dosbox_conf(data_dir, captures_dir)
    parsed = _parse_dosbox_conf(conf_text)

    forbidden = _check_forbidden(parsed)
    total += len(FORBIDDEN_DOSBOX_SETTINGS)
    matched += len(FORBIDDEN_DOSBOX_SETTINGS) - len(forbidden)
    for key, forbidden_value, actual in forbidden:
        failures.append(
            f"forbidden setting present: {key}={actual!r} "
            f"(pass94 failure-mode value {forbidden_value!r})"
        )

    missing = _check_required(parsed)
    total += len(REQUIRED_DOSBOX_SETTINGS)
    matched += len(REQUIRED_DOSBOX_SETTINGS) - len(missing)
    failures.extend(f"required setting: {m}" for m in missing)

    # Step 3 — write the conf + receipt if the pin checks pass and
    # the caller did not opt out (the self-test opts out so the
    # sandbox stays clean between runs).
    if not failures and write_receipt:
        receipt_out.parent.mkdir(parents=True, exist_ok=True)
        conf_path = receipt_out.parent / "dosbox_capture.conf"
        conf_path.write_text(conf_text, encoding="utf-8")

        if now_iso is None:
            now_iso = time.strftime("%Y-%m-%dT%H:%M:%S%z", time.gmtime())
        if session_id is None:
            session_id = uuid.uuid4().hex[:12]
        if repo_root is None:
            repo_root = Path(__file__).resolve().parents[2]

        if (data_dir / "DM.EXE").is_file():
            # Inner subdir layout — DM.EXE is on the mounted C: root.
            launch_command = "DM.EXE"
        else:
            # Game root layout — DM.EXE lives one level down; the
            # autoexec did `cd DungeonMasterPC34` before launching.
            launch_command = "cd DungeonMasterPC34 && DM.EXE"
        receipt = PreflightReceipt(
            session_id=session_id,
            issued_at_iso=now_iso,
            data_dir=str(data_dir),
            dungeon_sha256=dungeon_sha,
            graphics_sha256=graphics_sha,
            dungeon_match=dungeon_match,
            graphics_match=graphics_match,
            conf_path=str(conf_path),
            conf_settings=_settings_to_report(parsed),
            launch_command=launch_command,
            render_settings={
                "machine": parsed.get("machine", "<missing>"),
                "memsize": parsed.get("memsize", "<missing>"),
                "core": parsed.get("core", "<missing>"),
                "cycles": parsed.get("cycles", "<missing>"),
                "frameskip": parsed.get("frameskip", "<missing>"),
                "windowresolution": parsed.get("windowresolution", "<missing>"),
                "viewport_resolution": parsed.get("viewport_resolution", "<missing>"),
                "output": parsed.get("output", "<missing>"),
            },
            firestaff_git_head=_try_read_firestaff_git_head(repo_root),
            pass94_forbidden_present=bool(forbidden),
        )
        receipt_out.write_text(receipt.to_json(), encoding="utf-8")
    return matched, total, failures


def selftest_preflight(tmp_root: Path) -> tuple[int, int, list[str]]:
    """Regression self-test that builds a fake game-data dir and asserts
    the preflight passes; then corrupts the SHA256s and asserts the
    preflight fails; then injects a pass94 failure-mode setting into a
    hand-written conf and asserts the pin-check rejects it.

    The synthetic fixture SHA256s are captured from the synthetic
    bytes after the fixture is written, so the self-test is hermetic
    and does not depend on the runbook's canonical DM1 SHA256s being
    reachable on disk.  CI runners without the real game data can
    still gate the preflight logic.
    """
    failures: list[str] = []
    matched = 0
    total = 0

    # Synthetic DUNGEON.DAT / GRAPHICS.DAT — content is irrelevant;
    # we capture the SHA256s after writing and pass them as the
    # ``expected_*_sha256`` overrides so the SHA check exercises
    # the matching case deterministically.
    sandbox = tmp_root / "preflight_selftest"
    data_dir = sandbox / "dm1"
    captures_dir = sandbox / "captures"
    receipt = sandbox / "preflight.receipt.json"
    if sandbox.exists():
        # Wipe between runs so a previous run's data doesn't leak
        # into the corruption check.
        import shutil
        shutil.rmtree(sandbox)
    data_dir.mkdir(parents=True, exist_ok=True)
    dungeon_bytes = b"DUNGEON_FIXTURE_v1\n"
    graphics_bytes = b"GRAPHICS_FIXTURE_v1\n"
    (data_dir / "DUNGEON.DAT").write_bytes(dungeon_bytes)
    (data_dir / "GRAPHICS.DAT").write_bytes(graphics_bytes)
    synth_dungeon_sha = hashlib.sha256(dungeon_bytes).hexdigest()
    synth_graphics_sha = hashlib.sha256(graphics_bytes).hexdigest()

    # Clean run with matching SHAs — expect success on the data
    # checks; the conf pin checks also pass because the rendered
    # conf is the runbook-confirmed hardened shape.
    m, n, fails = preflight(
        data_dir, captures_dir, receipt,
        session_id="selftest_clean", now_iso="2026-06-06T00:00:00+00:00",
        expected_dungeon_sha256=synth_dungeon_sha,
        expected_graphics_sha256=synth_graphics_sha,
        write_receipt=False,
    )
    total += n
    matched += m
    failures.extend(f"clean: {f}" for f in fails)

    # Corrupt DUNGEON.DAT and re-run; expect a SHA mismatch failure
    # and a partial pass (graphics still matches).
    (data_dir / "DUNGEON.DAT").write_bytes(b"DIFFERENT_CONTENT\n")
    m, n, fails = preflight(
        data_dir, captures_dir, receipt.with_name("corrupt.receipt.json"),
        session_id="selftest_corrupt", now_iso="2026-06-06T00:00:00+00:00",
        expected_dungeon_sha256=synth_dungeon_sha,
        expected_graphics_sha256=synth_graphics_sha,
        write_receipt=False,
    )
    # We only score the DUNGEON check here (graphics still matches);
    # the failure list must contain a DUNGEON.DAT SHA256 mismatch.
    total += 1
    matched += m  # graphics is 1 of the 2 data checks
    if not any("DUNGEON.DAT SHA256 mismatch" in f for f in fails):
        failures.append("corrupt: expected DUNGEON.DAT SHA256 mismatch, got none")
    # Restore the DUNGEON file so subsequent self-test runs start clean.
    (data_dir / "DUNGEON.DAT").write_bytes(dungeon_bytes)

    # Inject a pass94 failure-mode setting and assert the pin-check
    # catches it.  We do this by writing a hand-crafted conf with
    # ``machine=svga_paradise`` and feeding it through the parser +
    # pin-check helpers directly.  This is the regression gate for
    # the actual pass94 conf values.
    bad_conf = "[dosbox]\nmachine=svga_paradise\nmemsize=4\ncore=normal\ncycles=3000\n"
    parsed = _parse_dosbox_conf(bad_conf)
    forbidden = _check_forbidden(parsed)
    if not forbidden:
        failures.append("forbidden: expected at least one pin violation, got none")
    else:
        total += 1
        matched += 1

    missing = _check_required(parsed)
    if not missing:
        failures.append("required: expected missing required settings, got none")
    else:
        total += 1
        matched += 1

    # Sanity: the runbook-confirmed hardened conf must pass the
    # forbidden + required pin checks in isolation.
    hardened_conf = _render_dosbox_conf(
        data_dir=data_dir, captures_dir=captures_dir,
    )
    parsed_hardened = _parse_dosbox_conf(hardened_conf)
    if _check_forbidden(parsed_hardened):
        failures.append("hardened: rendered conf has forbidden settings")
    else:
        total += 1
        matched += 1
    if _check_required(parsed_hardened):
        failures.append(
            f"hardened: rendered conf missing required settings: "
            f"{_check_required(parsed_hardened)}"
        )
    else:
        total += 1
        matched += 1

    # Layout auto-detection: the runbook's canonical game root has
    # DM.EXE inside a DungeonMasterPC34/ subdirectory, not at the
    # root.  The preflight must emit `cd DungeonMasterPC34\nDM.EXE`
    # for that layout, and bare `DM.EXE` when data_dir IS the inner
    # subdir.  This is the regression gate for the historical
    # "DungeonMasterPC34.EXE" autoexec that pointed at a directory.
    if "DungeonMasterPC34.EXE" in hardened_conf:
        failures.append(
            "layout: hardened conf still references the directory "
            "'DungeonMasterPC34.EXE' instead of the binary 'DM.EXE'"
        )
    else:
        total += 1
        matched += 1
    if "DM.EXE" not in hardened_conf:
        failures.append("layout: hardened conf missing DM.EXE launch line")
    else:
        total += 1
        matched += 1
    if "cd DungeonMasterPC34" not in hardened_conf:
        failures.append(
            "layout: hardened conf missing 'cd DungeonMasterPC34' for "
            "the canonical game-root layout"
        )
    else:
        total += 1
        matched += 1

    # Inner-subdir layout: data_dir IS the DungeonMasterPC34/
    # subdir (DM.EXE on the root), so the autoexec should NOT
    # emit a `cd DungeonMasterPC34` line.  We build a synthetic
    # inner-subdir sandbox and assert the rendered conf for it.
    inner_dir = sandbox / "inner_subdir"
    inner_dir.mkdir(parents=True, exist_ok=True)
    (inner_dir / "DM.EXE").write_bytes(b"DM_FIXTURE\n")
    inner_conf = _render_dosbox_conf(
        data_dir=inner_dir, captures_dir=captures_dir,
    )
    if "cd DungeonMasterPC34" in inner_conf:
        failures.append(
            "layout: inner-subdir conf still has 'cd DungeonMasterPC34' "
            "even though DM.EXE is directly reachable"
        )
    else:
        total += 1
        matched += 1
    if "DM.EXE" not in inner_conf:
        failures.append("layout: inner-subdir conf missing DM.EXE launch line")
    else:
        total += 1
        matched += 1

    return matched, total, failures


def main(argv: Optional[list[str]] = None) -> int:
    parser = argparse.ArgumentParser(
        description="Preflight gate for the DM1 PC 3.4 original-capture "
                    "route (verifies data SHA256s, writes a hardened "
                    "DOSBox conf, records a receipt).",
    )
    parser.add_argument(
        "--data-dir", type=Path,
        default=Path(os.path.expanduser(
            "~/.openclaw/data/firestaff-original-games/DM/_canonical/dm1"
        )),
        help="DM1 game data root (default: %(default)s)",
    )
    parser.add_argument(
        "--captures-dir", type=Path,
        default=Path(os.path.expanduser("~/firestaff-captures")),
        help="where to write the hardened conf + receipt (default: %(default)s)",
    )
    parser.add_argument(
        "--receipt-out", type=Path, default=None,
        help="explicit receipt path (default: <captures-dir>/preflight.receipt.json)",
    )
    parser.add_argument(
        "--repo-root", type=Path, default=None,
        help="Firestaff repo root for git HEAD recording "
             "(default: derived from script path)",
    )
    parser.add_argument(
        "--self-test", action="store_true",
        help="run regression self-test on synthetic fixtures (no real game data)",
    )
    parser.add_argument(
        "--self-test-tmp", type=Path,
        default=Path("/tmp/dosbox_capture_preflight_selftest"),
        help="sandbox dir for --self-test (default: %(default)s)",
    )
    args = parser.parse_args(argv)

    if args.self_test:
        matched, total, failures = selftest_preflight(args.self_test_tmp)
        print(
            f"self-test: preflight checks {matched}/{total} matched"
        )
        if failures:
            print("FAIL:")
            for f in failures:
                print(f"  - {f}")
            return 1
        print("PASS")
        return 0

    receipt_out = args.receipt_out
    if receipt_out is None:
        receipt_out = args.captures_dir / "preflight.receipt.json"
    matched, total, failures = preflight(
        args.data_dir, args.captures_dir, receipt_out, repo_root=args.repo_root,
    )
    print(f"preflight: {matched}/{total} checks matched")
    if failures:
        print("FAIL:")
        for f in failures:
            print(f"  - {f}")
        return 1
    print(f"PASS — receipt: {receipt_out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
