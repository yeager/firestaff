#!/usr/bin/env python3
"""Live row gate for the DM1 V1 original-capture route.

The DM1 V1 original DOS capture route documented in
``docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md`` is blocked at the
pass608 same-viewport capture layer
(``BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE``).
The runbook ships a deterministic handoff toolchain that turns a
pass623 canonical input-capture fixture + a preflight receipt + a
320x200 capture frame + a 224x136 crop into a 41-column events
TSV row, then into a transcript.json that the pass608 verifier
reads as ``runtimeTranscript``.

The live DOSBox session runner
(``docs/parity/tools/dosbox_capture_session.py``) writes its two
captures to disk as
``<capture_root>/original/01_ingame_start.png`` and
``<capture_root>/original/02_ingame_step_forward.png`` (see
``live_run`` lines 2121-2132).  Those are the exact filenames the
next live attempt will produce — and yet they do not match the
pass623 fixture's route labels (``01_start_south_1_3`` /
``03_blocked_west_wall_1_3``) or the row builder's
``--label``/``--raw``/``--crop`` contract.  The
``verify_dm1_v1_original_capture_route_handoff.py`` gate proves the
``02_turn_right_west_1_3`` route flips the pass608 blocker, but it
does not exercise the live runner's exact filename conventions,
and a future rename of either side of the live-binding table can
silently ship a transcript whose original-frame path is no longer
in the live runbook.

This gate is the live-binding complement: it pins the live
session's exact filenames to a pass623 route label + a Firestaff
fixture viewport hash through the same on-disk
``preflight + row_builder + transcript_writer + pass608`` chain,
and asserts the resulting transcript flips the blocker to
``PROMOTED``.  The gate is fully hermetic: it builds synthetic
preflight receipts, pass623 fixtures, Firestaff capture manifests,
and 320x200 / 224x136 capture fixtures in a temp dir, runs the
real on-disk tools as subprocesses (no inlined logic, no module
re-implementations), and feeds the resulting ``transcript.json``
to the real
``verify_pass608_dm1_v1_same_viewport_capture_blocker.py`` to
confirm the binding contract is satisfied.

The live-binding table (the row that ships the live runbook to
the pass608 / pass625 verifier) is a constant in this gate:

  +--------------------------------+--------------------------------+--------------------------------+----------------+
  | Live runner ``save()`` filename | Pass623 route label            | Firestaff fixture viewport hash| Party-before    |
  +================================+================================+================================+================+
  | ``01_ingame_start.png``         | ``01_start_south_1_3``          | ``50661c78...``                | south, (1,3)    |
  +--------------------------------+--------------------------------+--------------------------------+----------------+
  | ``02_ingame_step_forward.png``  | ``03_blocked_west_wall_1_3``    | ``0cb83803...``                | west,  (1,3)    |
  +--------------------------------+--------------------------------+--------------------------------+----------------+

A future operator who renames either side of the binding table
without updating this gate will see the gate fail loudly (the row
builder will reject the new filename with a
``pass623 fixture does not contain label`` error or the pass608
verifier will refuse to promote the unknown Firestaff hash), and
the runbook-consistency probe at
``tools/test_dm1_v1_capture_runbook_consistency.py`` will catch
the same drift in a CI gate.

Checks (exit code 0 means the live row binding is wired into the
runbook-end handoff, exit code 1 means the next live attempt would
ship a transcript the pass608 verifier still rejects):

  * C1 - The on-disk ``verify_pass608_dm1_v1_same_viewport_capture_blocker.py``
    is importable and reports the baseline BLOCKED status (the
    gate's negative-control baseline).
  * C2 - The on-disk ``docs/parity/tools/dosbox_capture_preflight.py``
    is importable and its self-test passes (the receipt schema
    the row builder and the writer consume is the same schema the
    preflight emits).
  * C3 - The on-disk live session runner's ``live_run()`` saves its
    captures to the exact filenames in the live-binding table
    (a regex pin on the source string keeps a future rename of
    ``01_ingame_start.png`` or ``02_ingame_step_forward.png`` from
    silently breaking the live handoff).
  * C4 - The on-disk ``docs/parity/tools/dosbox_capture_events_row_builder.py``
    renders a 41-column events TSV row for each live-binding
    row, using the row builder's own preflight + pass623
    validation pipeline (the row builder refuses to emit a row
    for a label that is not in the pass623 fixture, so this
    subcheck is also the live-binding-table-matches-pass623
    check).
  * C5 - The on-disk ``docs/parity/tools/dosbox_capture_transcript_writer.py``
    consumes the two live-binding rows and emits a transcript
    whose ``promotable`` flag is True.
  * C6 - The pass608 verifier reads the same transcript and flips
    its status to
    ``PASS608_DM1_V1_COMMAND_STATE_REDRAW_TRANSCRIPT_BOUND``,
    its ``runtimeTranscript.status`` to
    ``loaded_promotable_same_run``, and clears the
    "no supplied transcript row satisfies the pass608
    command/state/redraw binding contract" / "original rows do
    not bind map/X/Y/direction" blockers.
  * C7 - A negative path: the Firestaff viewport hash for the
    second live capture (``02_ingame_step_forward`` →
    ``03_blocked_west_wall_1_3``) is patched to a never-seen
    64-hex value; the pass608 verifier's
    ``firestaffViewportHashKnown`` check must fail and keep the
    BLOCKED status.  This subcheck makes a future regression
    that drops the live-binding's hash discipline (e.g. a
    copied row that forgets to update the Firestaff viewport
    hash) visible at the gate instead of at the next live
    attempt.
  * C8 - Anchored binding: every row of ``LIVE_BINDING_TABLE``
    references a pass623 route label that is present in the
    real on-disk ``pass623_dm1_v1_input_capture_readiness_bridge/manifest.json``
    AND a Firestaff fixture viewport sha that is present in
    the real on-disk
    ``verification-screens/capture_manifest_sha256.tsv``.
    Without this anchor the hermetic C4-C7 chain can pass
    against a synthetic fixture even when the live binding
    has drifted away from the canonical fixtures, and the
    next live DOSBox attempt would discover the drift at
    row-builder time instead of at the gate.
"""
from __future__ import annotations

import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import zlib
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[1]
TOOLS_DIR = REPO_ROOT / "docs" / "parity" / "tools"
SESSION_RUNNER = TOOLS_DIR / "dosbox_capture_session.py"
PASS608_VERIFIER = REPO_ROOT / "tools" / "verify_pass608_dm1_v1_same_viewport_capture_blocker.py"
PREFLIGHT = TOOLS_DIR / "dosbox_capture_preflight.py"
ROW_BUILDER = TOOLS_DIR / "dosbox_capture_events_row_builder.py"
TRANSCRIPT_WRITER = TOOLS_DIR / "dosbox_capture_transcript_writer.py"
FIRESTAFF_CAPTURE_MANIFEST = REPO_ROOT / "verification-screens" / "capture_manifest_sha256.tsv"
FIRESTAFF_STATE_PROBE = REPO_ROOT / "verification-m11" / "capture-route-state-pass195" / "pass76_capture_route_state_probe.json"
REAL_PASS623_FIXTURE = (
    REPO_ROOT
    / "parity-evidence"
    / "verification"
    / "pass623_dm1_v1_input_capture_readiness_bridge"
    / "manifest.json"
)
PARITY_EVIDENCE = REPO_ROOT / "parity-evidence" / "verification"

STATUS = "DM1_V1_ORIGINAL_CAPTURE_LIVE_ROW_GATE_CLOSED"
BLOCKED_STATUS = "BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE"
PROMOTED_STATUS = "PASS608_DM1_V1_COMMAND_STATE_REDRAW_TRANSCRIPT_BOUND"

# Runbook §1 asset-set SHA256s (DUNGEON.DAT + GRAPHICS.DAT for the
# canonical DM1 PC 3.4 layout).  These are the same constants the
# preflight writes into the receipt and the writer embeds into
# transcript rows under ``originalAssetSet.sha256``.
CANONICAL_DUNGEON_SHA = "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"
CANONICAL_GRAPHICS_SHA = "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"

# Live-binding table.  Each row pins:
#   - the exact filename the live session writes
#     (see ``live_run`` lines 2121-2132 of
#     ``docs/parity/tools/dosbox_capture_session.py``);
#   - the pass623 route label that row is bound to
#     (the row builder refuses to emit a row for a label that
#     is not in the pass623 fixture, so this column is also
#     the live-binding-table-matches-pass623 contract);
#   - the Firestaff fixture viewport sha the transcript writer
#     needs in ``firestaffFrame.viewportSha256`` (the pass608
#     verifier's ``firestaffViewportHashKnown`` check accepts
#     only sha values that appear in
#     ``verification-screens/capture_manifest_sha256.tsv``);
#   - the party_before tuple the row builder needs to render
#     a queue-write / dispatch / redraw binding that the pass608
#     verifier's ``redrawMapX == partyAfterMapX`` /
#     ``redrawDirection == partyAfterDirection`` /
#     ``presentMapX == partyAfterMapX`` / etc. checks accept.
#
# A future operator who renames either side of this table
# without updating this gate will see C3 / C4 / C6 / C7 fail
# with a self-describing error message; the runbook-consistency
# probe at ``tools/test_dm1_v1_capture_runbook_consistency.py``
# catches the same drift in a CI gate.
LIVE_BINDING_TABLE: list[dict[str, Any]] = [
    {
        "live_filename": "01_ingame_start.png",
        "pass623_label": "01_start_south_1_3",
        "pass623_command_ids": [],
        "pass623_input_tokens": [],
        "firestaff_viewport_sha": "50661c78e2ece0839df9a71a62673aec2acaecefa5c122daf406fd34d94a8bf9",
        "firestaff_viewport_filename": "01_ingame_start_latest_viewport_224x136.ppm",
        "party_before": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 2},
        "party_after":  {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 2},
        "classification": "dungeon_gameplay",
        "queue_count_before": 0,
        "queue_first_index_before": 0,
    },
    {
        "live_filename": "02_ingame_step_forward.png",
        "pass623_label": "03_blocked_west_wall_1_3",
        "pass623_command_ids": [3],
        "pass623_input_tokens": ["M12_MENU_INPUT_UP"],
        "firestaff_viewport_sha": "0cb83803cd9cfbf3ff706998bc22a5a17926ec9dfd49c8a1fedbd4b376bf92b8",
        "firestaff_viewport_filename": "03_ingame_move_forward_latest_viewport_224x136.ppm",
        "party_before": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 3},
        "party_after":  {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 3},
        "classification": "dungeon_gameplay",
        "queue_count_before": 1,
        "queue_first_index_before": 0,
    },
]


# ---------------------------------------------------------------------------
# Minimal PNG writer for hermetic capture fixtures.
# ---------------------------------------------------------------------------

def _chunk(kind: bytes, payload: bytes) -> bytes:
    return (
        struct.pack(">I", len(payload)) + kind + payload +
        struct.pack(">I", zlib.crc32(kind + payload) & 0xFFFFFFFF)
    )


def write_png(path: Path, width: int, height: int, rgb: tuple[int, int, int]) -> None:
    """Write a minimal RGB PNG (no compression metadata, single IDAT)."""
    sig = b"\x89PNG\r\n\x1a\n"
    ihdr = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    raw_scanline = b"\x00" + bytes(rgb) * width
    idat = zlib.compress(raw_scanline * height, 9)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(
        sig + _chunk(b"IHDR", ihdr) + _chunk(b"IDAT", idat) + _chunk(b"IEND", b"")
    )


# ---------------------------------------------------------------------------
# Synthetic preflight receipt / pass623 fixture / capture manifest.
# ---------------------------------------------------------------------------

def _write_synth_receipt(path: Path) -> None:
    """Write a synthetic preflight receipt that matches the
    writer's three pin checks (``dungeon_match``,
    ``graphics_match``, ``pass94_forbidden_present``).  Same
    schema as
    ``docs/parity/tools/dosbox_capture_preflight.py``."""
    payload = {
        "schema": "dosbox_capture_preflight.v1",
        "session_id": "live_row_gate_synthetic",
        "captured_utc": "2026-06-08T06:00:00Z",
        "data_dir": "/synthetic/live/row-gate",
        "dungeon_match":   True,
        "graphics_match":  True,
        "title_match":     True,
        "dungeon_sha256":  CANONICAL_DUNGEON_SHA,
        "graphics_sha256": CANONICAL_GRAPHICS_SHA,
        "title_sha256":    "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
        "pass94_forbidden_present": False,
        "conf_path":       str(path.parent / "dosbox_capture.conf"),
        "launch_command":  "dosbox -conf dosbox_capture.conf",
        "firestaff_git_head": "live_row_gate_synthetic",
        "checks": [
            {"name": "dungeon_sha256", "ok": True, "expected": CANONICAL_DUNGEON_SHA, "actual": CANONICAL_DUNGEON_SHA},
            {"name": "graphics_sha256", "ok": True, "expected": CANONICAL_GRAPHICS_SHA, "actual": CANONICAL_GRAPHICS_SHA},
            {"name": "pass94_forbidden_settings_absent", "ok": True, "expected": False, "actual": False},
        ],
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True), encoding="utf-8")


def _write_synth_pass623(path: Path) -> None:
    """Write a synthetic pass623 fixture for both live-binding
    rows.  Same schema as
    ``parity-evidence/verification/pass623_dm1_v1_input_capture_readiness_bridge/manifest.json``."""
    canonical_rows: list[dict[str, Any]] = []
    for binding in LIVE_BINDING_TABLE:
        canonical_rows.append({
            "label": binding["pass623_label"],
            "commandIds": list(binding["pass623_command_ids"]),
            "inputTokens": list(binding["pass623_input_tokens"]),
            "postTuple": {
                "map":       binding["party_after"]["mapIndex"],
                "x":         binding["party_after"]["mapX"],
                "y":         binding["party_after"]["mapY"],
                "direction": binding["party_after"]["direction"],
            },
            "observed": {
                "label": binding["pass623_label"],
                "crop":  binding["firestaff_viewport_filename"],
                "sha":   binding["firestaff_viewport_sha"],
                "map":   binding["party_after"]["mapIndex"],
                "x":     binding["party_after"]["mapX"],
                "y":     binding["party_after"]["mapY"],
                "direction": binding["party_after"]["direction"],
            },
            "claim": f"synthetic pass623 row for live row gate ({binding['live_filename']})",
            "ok": True,
            "problems": [],
        })
    payload = {
        "schema": "pass623_input_capture_readiness_bridge.v1",
        "canonicalInputCaptureRows": canonical_rows,
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True), encoding="utf-8")


def _write_synth_capture_manifest(path: Path) -> None:
    """Write a synthetic Firestaff capture manifest.  Mirrors the
    column shape of
    ``verification-screens/capture_manifest_sha256.tsv`` (the
    pass608 verifier parses
    ``kind<TAB>filename<TAB>width<TAB>height<TAB>bytes<TAB>sha256``)
    and lists both live-binding viewport hashes so the pass608
    verifier's ``firestaffViewportHashKnown`` check accepts
    them."""
    lines = [
        "# synthetic Firestaff capture manifest for live row gate",
        "# columns: kind<TAB>filename<TAB>width<TAB>height<TAB>bytes<TAB>sha256",
    ]
    for binding in LIVE_BINDING_TABLE:
        lines.append(
            "viewport_224x136\t"
            f"{binding['firestaff_viewport_filename']}\t224\t136\t91407\t"
            f"{binding['firestaff_viewport_sha']}"
        )
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


# ---------------------------------------------------------------------------
# Helper: run pass608 verifier with a transcript path and parse
# its JSON manifest.  Mirrors the helper in
# ``verify_dm1_v1_original_capture_route_handoff.py``: the
# verifier writes a manifest under
# ``parity-evidence/verification/pass608_dm1_v1_same_viewport_capture_blocker/manifest.json``
# AND a human-readable report at
# ``parity-evidence/pass608_dm1_v1_same_viewport_capture_blocker.md``
# and prints a JSON envelope to stdout.  We re-parse the
# manifest so the gate does not depend on stdout ordering.
#
# Both verifier outputs are backed up before invocation and
# restored afterwards so the gate leaves the worktree clean.
# ---------------------------------------------------------------------------

def _run_pass608(transcript_path: Path) -> dict[str, Any]:
    manifest_path = PARITY_EVIDENCE / "pass608_dm1_v1_same_viewport_capture_blocker" / "manifest.json"
    report_path   = REPO_ROOT / "parity-evidence" / "pass608_dm1_v1_same_viewport_capture_blocker.md"
    manifest_backup: bytes | None = None
    report_backup:   bytes | None = None
    if manifest_path.is_file():
        manifest_backup = manifest_path.read_bytes()
    if report_path.is_file():
        report_backup = report_path.read_bytes()
    if manifest_path.exists():
        manifest_path.unlink()
    if report_path.exists():
        report_path.unlink()
    try:
        proc = subprocess.run(
            [sys.executable, str(PASS608_VERIFIER), "--transcript", str(transcript_path)],
            cwd=str(REPO_ROOT),
            capture_output=True,
            text=True,
            timeout=120,
        )
        if not manifest_path.exists():
            raise RuntimeError(
                f"pass608 verifier did not write its manifest "
                f"(exit={proc.returncode}, stdout={proc.stdout!r}, stderr={proc.stderr!r})"
            )
        return json.loads(manifest_path.read_text(encoding="utf-8"))
    finally:
        if manifest_backup is not None:
            manifest_path.write_bytes(manifest_backup)
        elif manifest_path.exists():
            manifest_path.unlink()
        if report_backup is not None:
            report_path.write_bytes(report_backup)
        elif report_path.exists():
            report_path.unlink()


# ---------------------------------------------------------------------------
# Helper: run the on-disk row builder + writer as subprocesses
# so a future tool rename or a syntax regression in one tool
# surfaces here as a real exit-code / parse error, not as an
# ImportError swallowed by the gate.
# ---------------------------------------------------------------------------

def _run_row_builder_for_binding(
    sandbox: Path,
    binding: dict[str, Any],
    raw_path: Path,
    crop_path: Path,
    receipt_path: Path,
    pass623_path: Path,
    run_id: str,
    write_header: bool,
) -> tuple[int, list[str], Path | None]:
    """Invoke the on-disk row builder for one live-binding row.
    Returns (matched, failures, events_tsv_path).  When
    ``write_header`` is True the very first invocation also
    writes the EVENTS_TSV_HEADER line so subsequent invocations
    can be concatenated."""
    events_tsv = sandbox / "events.tsv"
    cmd = [
        sys.executable, str(ROW_BUILDER),
        "--label",        binding["pass623_label"],
        "--raw",          str(raw_path),
        "--crop",         str(crop_path),
        "--run-id",       run_id,
        "--preflight-receipt", str(receipt_path),
        "--pass623-fixture",   str(pass623_path),
        "--classification",    binding["classification"],
        "--party-before-map",          str(binding["party_before"]["mapIndex"]),
        "--party-before-x",            str(binding["party_before"]["mapX"]),
        "--party-before-y",            str(binding["party_before"]["mapY"]),
        "--party-before-direction",    str(binding["party_before"]["direction"]),
        "--queue-count-before",        str(binding["queue_count_before"]),
        "--queue-first-index-before",  str(binding["queue_first_index_before"]),
    ]
    if write_header:
        cmd.append("--with-header")
    proc = subprocess.run(
        cmd,
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        timeout=60,
    )
    if proc.returncode != 0:
        return 0, [f"row-builder exit={proc.returncode} stderr={proc.stderr.strip()!r}"], None
    if not proc.stdout:
        return 0, [f"row-builder produced empty stdout (stderr={proc.stderr.strip()!r})"], None
    # Append to the events TSV (the row builder writes the
    # header + a single row when ``--with-header`` is passed, or
    # a single row otherwise; the very first invocation writes
    # the header, subsequent invocations append a row).
    mode = "a" if events_tsv.exists() else "w"
    with events_tsv.open(mode, encoding="utf-8") as fh:
        fh.write(proc.stdout)
    return 1, [], events_tsv


def _run_transcript_writer(
    receipt_path: Path,
    pass623_path: Path,
    capture_manifest_path: Path,
    events_tsv: Path,
    transcript_out: Path,
) -> tuple[int, list[str]]:
    """Invoke the on-disk transcript writer."""
    proc = subprocess.run(
        [
            sys.executable, str(TRANSCRIPT_WRITER),
            "--preflight-receipt",        str(receipt_path),
            "--events-tsv",               str(events_tsv),
            "--transcript-out",           str(transcript_out),
            "--pass623-fixture",          str(pass623_path),
            "--firestaff-capture-manifest", str(capture_manifest_path),
        ],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        timeout=60,
    )
    if proc.returncode != 0:
        return 0, [f"transcript-writer exit={proc.returncode} stderr={proc.stderr.strip()!r}"]
    if not transcript_out.is_file():
        return 0, [f"transcript-writer did not write {transcript_out}"]
    payload = json.loads(transcript_out.read_text(encoding="utf-8"))
    if not payload.get("promotable", False):
        return 0, [f"transcript.promotable is False: payload={payload}"]
    if not payload.get("rows"):
        return 0, [f"transcript has no rows: payload={payload}"]
    return 1, []


def _run_preflight_self_test() -> tuple[int, list[str]]:
    proc = subprocess.run(
        [sys.executable, str(PREFLIGHT), "--self-test"],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        timeout=120,
    )
    if proc.returncode != 0:
        return 0, [f"preflight self-test exit={proc.returncode} stderr={proc.stderr.strip()!r}"]
    return 1, []


# ---------------------------------------------------------------------------
# Check implementations.
# ---------------------------------------------------------------------------

def _check_c1_baseline_blocked() -> tuple[bool, str]:
    """C1: the pass608 verifier is importable and reports BLOCKED
    when no transcript is supplied (the gate's negative-control
    baseline)."""
    try:
        sys.path.insert(0, str(REPO_ROOT / "tools"))
        import verify_pass608_dm1_v1_same_viewport_capture_blocker as v
    except Exception as exc:  # pragma: no cover - import guard
        return False, f"could not import pass608 verifier: {exc}"
    if v.STATUS != BLOCKED_STATUS:
        return False, f"pass608.STATUS={v.STATUS!r}, expected {BLOCKED_STATUS!r}"
    if v.PROMOTED_STATUS != PROMOTED_STATUS:
        return False, f"pass608.PROMOTED_STATUS={v.PROMOTED_STATUS!r}, expected {PROMOTED_STATUS!r}"
    expected_path = str(PASS608_VERIFIER.resolve())
    actual_path = str(Path(v.__file__).resolve())
    if expected_path != actual_path:
        return False, f"pass608 module path mismatch: {actual_path!r} != {expected_path!r}"
    if not v.FIRESTAFF_CAPTURE_MANIFEST.is_file():
        return False, f"FIRESTAFF_CAPTURE_MANIFEST missing: {v.FIRESTAFF_CAPTURE_MANIFEST}"
    if not v.FIRESTAFF_STATE_PROBE.is_file():
        return False, f"FIRESTAFF_STATE_PROBE missing: {v.FIRESTAFF_STATE_PROBE}"
    return True, "pass608 verifier importable; baseline status matches BLOCKED"


def _check_c2_preflight_self_test() -> tuple[int, list[str]]:
    return _run_preflight_self_test()


def _check_c3_live_runner_filenames() -> tuple[bool, str]:
    """C3: the on-disk live session runner's ``live_run()`` saves
    its captures to the exact filenames in LIVE_BINDING_TABLE
    (a regex pin on the source string keeps a future rename of
    ``01_ingame_start.png`` or ``02_ingame_step_forward.png``
    from silently breaking the live handoff)."""
    if not SESSION_RUNNER.is_file():
        return False, f"session runner not found: {SESSION_RUNNER}"
    source = SESSION_RUNNER.read_text(encoding="utf-8")
    failures: list[str] = []
    for binding in LIVE_BINDING_TABLE:
        live_filename = binding["live_filename"]
        # Pin the exact ``save()`` / ``Path`` expression so a
        # future operator who changes the filename and forgets
        # to update the live-binding table gets a self-describing
        # failure message.
        pattern = rf"original_dir\s*/\s*[\"']{re.escape(live_filename)}[\"']"
        if not re.search(pattern, source):
            failures.append(
                f"live session runner does not save to original_dir / '{live_filename}'; "
                f"a future rename of the live capture filename would silently break the "
                f"live row gate.  Update LIVE_BINDING_TABLE and the row builder "
                f"invocation in {SESSION_RUNNER.relative_to(REPO_ROOT)}"
            )
    if failures:
        return False, "; ".join(failures)
    return True, (
        f"live session runner saves to {len(LIVE_BINDING_TABLE)} expected filenames: "
        + ", ".join(b["live_filename"] for b in LIVE_BINDING_TABLE)
    )


def _check_c4_c6_live_binding_flips(sandbox: Path) -> tuple[int, list[str]]:
    """C4-C6: render a hermetic events TSV for the live-binding
    table with the on-disk row builder, feed it through the
    on-disk transcript writer, and confirm the pass608 verifier
    flips the blocker to PROMOTED with
    ``loaded_promotable_same_run``."""
    failures: list[str] = []

    receipt_path    = sandbox / "preflight_receipt.json"
    pass623_path    = sandbox / "pass623.json"
    manifest_path   = sandbox / "capture_manifest.tsv"
    captures_dir    = sandbox / "original"
    transcript_path = sandbox / "transcript.json"

    _write_synth_receipt(receipt_path)
    _write_synth_pass623(pass623_path)
    _write_synth_capture_manifest(manifest_path)
    captures_dir.mkdir(parents=True, exist_ok=True)

    # The live session writes a 320x200 raw capture but no
    # 224x136 crop; the runbook's Step 4 ``convert -crop
    # 224x136+0+33 +repage`` step derives the crop from the
    # raw.  The live row gate writes both so the row builder's
    # geometry check is exercised end-to-end.
    rgb_per_index = [(40, 40, 40), (50, 50, 50), (60, 60, 60)]
    for idx, binding in enumerate(LIVE_BINDING_TABLE):
        rgb = rgb_per_index[idx % len(rgb_per_index)]
        raw_path  = captures_dir / binding["live_filename"]
        crop_path = captures_dir / binding["live_filename"].replace(".png", "_viewport.png")
        write_png(raw_path,  320, 200, rgb)
        write_png(crop_path, 224, 136, rgb)

    # C4 - row builder renders a 41-column row per binding.
    events_tsv = sandbox / "events.tsv"
    for idx, binding in enumerate(LIVE_BINDING_TABLE):
        raw_path  = captures_dir / binding["live_filename"]
        crop_path = captures_dir / binding["live_filename"].replace(".png", "_viewport.png")
        matched, fails, path = _run_row_builder_for_binding(
            sandbox=sandbox,
            binding=binding,
            raw_path=raw_path,
            crop_path=crop_path,
            receipt_path=receipt_path,
            pass623_path=pass623_path,
            run_id="live_row_gate_run_001",
            write_header=(idx == 0),
        )
        if matched != 1 or fails or path is None:
            return 0, [
                f"row-builder check failed for {binding['live_filename']} "
                f"(label={binding['pass623_label']}): matched={matched} fails={fails}"
            ]
    if not events_tsv.is_file():
        return 0, [f"row-builder did not produce {events_tsv}"]
    with events_tsv.open("r", encoding="utf-8") as fh:
        lines = [line.rstrip("\n") for line in fh if line.rstrip("\n")]
    if len(lines) < 1 + len(LIVE_BINDING_TABLE):
        return 0, [
            f"events TSV has {len(lines)} non-empty lines, expected "
            f">= 1 (header) + {len(LIVE_BINDING_TABLE)} (one per binding)"
        ]
    header_cells = lines[0].split("\t")
    if len(header_cells) != 41:
        return 0, [f"events TSV header has {len(header_cells)} columns, expected 41"]
    for idx, binding in enumerate(LIVE_BINDING_TABLE):
        row_cells = lines[1 + idx].split("\t")
        if len(row_cells) != 41:
            return 0, [
                f"events TSV row {idx} ({binding['live_filename']}) has "
                f"{len(row_cells)} columns, expected 41"
            ]
        # The row builder records the on-disk file path the
        # operator passed to ``--raw``; compare the basename
        # so the live row gate is path-independent.
        if Path(row_cells[0]).name != binding["live_filename"]:
            return 0, [
                f"events TSV row {idx} file={row_cells[0]!r}, expected "
                f"basename {binding['live_filename']!r}"
            ]
        if row_cells[6] != "320" or row_cells[7] != "200":
            return 0, [
                f"events TSV row {idx} width/height={row_cells[6]}x{row_cells[7]}, "
                f"expected 320x200"
            ]

    # C5 - transcript writer renders a promotable transcript.
    matched, fails = _run_transcript_writer(
        receipt_path=receipt_path,
        pass623_path=pass623_path,
        capture_manifest_path=manifest_path,
        events_tsv=events_tsv,
        transcript_out=transcript_path,
    )
    if matched != 1 or fails:
        return 0, [f"transcript-writer check failed: matched={matched} fails={fails}"]

    # C6 - pass608 verifier flips to PROMOTED.
    payload = _run_pass608(transcript_path)
    if payload.get("status") != PROMOTED_STATUS:
        failures.append(
            f"pass608 status={payload.get('status')!r}, expected {PROMOTED_STATUS!r}; "
            f"blockers={payload.get('blockers')!r}; "
            f"runtimeTranscript={payload.get('runtimeTranscript', {}).get('status')!r}"
        )
    rt = payload.get("runtimeTranscript", {})
    if rt.get("status") != "loaded_promotable_same_run":
        failures.append(
            f"runtimeTranscript.status={rt.get('status')!r}, expected 'loaded_promotable_same_run'"
        )
    if rt.get("promotableRowCount", 0) < 1:
        failures.append(
            f"runtimeTranscript.promotableRowCount={rt.get('promotableRowCount')}, expected >= 1"
        )
    for blocker in (
        "no supplied transcript row satisfies the pass608 command/state/redraw binding contract",
        "original rows do not bind map/X/Y/direction to F0380 -> F0365/F0366 -> F0128 -> F0097 for the same sampled frame",
    ):
        if blocker in (payload.get("blockers") or []):
            failures.append(f"blocker still present: {blocker!r}")
    if failures:
        return 0, failures
    return 1, []


def _check_c7_negative_path(sandbox: Path) -> tuple[int, list[str]]:
    """C7: a transcript whose second live-binding Firestaff
    viewport hash is NOT in the canonical manifest must NOT flip
    the pass608 blocker; the BLOCKED status must be preserved so
    a future regression that drops the
    ``firestaffViewportHashKnown`` check is caught here."""
    failures: list[str] = []

    # Use a separate sandbox for the negative path so the row
    # builder's stdout capture does not collide with the
    # positive path's events.tsv.
    neg_sandbox = sandbox / "neg"
    neg_sandbox.mkdir(parents=True, exist_ok=True)

    receipt_path    = neg_sandbox / "preflight_receipt.json"
    pass623_path    = neg_sandbox / "pass623.json"
    manifest_path   = neg_sandbox / "capture_manifest.tsv"
    captures_dir    = neg_sandbox / "original"
    transcript_path = neg_sandbox / "transcript.json"

    _write_synth_receipt(receipt_path)
    _write_synth_pass623(pass623_path)
    _write_synth_capture_manifest(manifest_path)
    captures_dir.mkdir(parents=True, exist_ok=True)
    rgb_per_index = [(40, 40, 40), (50, 50, 50), (60, 60, 60)]
    for idx, binding in enumerate(LIVE_BINDING_TABLE):
        rgb = rgb_per_index[idx % len(rgb_per_index)]
        raw_path  = captures_dir / binding["live_filename"]
        crop_path = captures_dir / binding["live_filename"].replace(".png", "_viewport.png")
        write_png(raw_path,  320, 200, rgb)
        write_png(crop_path, 224, 136, rgb)

    events_tsv = neg_sandbox / "events.tsv"
    for idx, binding in enumerate(LIVE_BINDING_TABLE):
        raw_path  = captures_dir / binding["live_filename"]
        crop_path = captures_dir / binding["live_filename"].replace(".png", "_viewport.png")
        matched, fails, path = _run_row_builder_for_binding(
            sandbox=neg_sandbox,
            binding=binding,
            raw_path=raw_path,
            crop_path=crop_path,
            receipt_path=receipt_path,
            pass623_path=pass623_path,
            run_id="live_row_gate_run_002",
            write_header=(idx == 0),
        )
        if matched != 1 or fails or path is None:
            return 0, [
                f"negative-path row-builder failed for {binding['live_filename']}: "
                f"matched={matched} fails={fails}"
            ]
    matched, fails = _run_transcript_writer(
        receipt_path=receipt_path,
        pass623_path=pass623_path,
        capture_manifest_path=manifest_path,
        events_tsv=events_tsv,
        transcript_out=transcript_path,
    )
    if matched != 1 or fails:
        return 0, [f"negative-path transcript-writer failed: matched={matched} fails={fails}"]

    # Patch the second row's Firestaff viewport hash to a
    # never-seen 64-hex value; the pass608 verifier's
    # ``firestaffViewportHashKnown`` check must fail and keep
    # the BLOCKED status.
    payload = json.loads(transcript_path.read_text(encoding="utf-8"))
    rows = payload.get("rows") or []
    if len(rows) < 2:
        return 0, ["negative-path transcript has fewer than 2 rows to patch"]
    target_row = rows[1]
    ff = target_row.setdefault("firestaffFrame", {})
    ff["viewportSha256"] = "f" * 64
    transcript_path.write_text(json.dumps(payload, indent=2, sort_keys=True), encoding="utf-8")

    out = _run_pass608(transcript_path)
    if out.get("status") != BLOCKED_STATUS:
        failures.append(
            f"negative-path pass608 status={out.get('status')!r}, expected {BLOCKED_STATUS!r}"
        )
    rt = out.get("runtimeTranscript", {})
    if rt.get("status") == "loaded_promotable_same_run":
        failures.append(
            "negative-path runtimeTranscript.status flipped to "
            "'loaded_promotable_same_run' even though second row's "
            "Firestaff viewport hash is not in the manifest"
        )
    if "no supplied transcript row satisfies the pass608 command/state/redraw binding contract" not in (out.get("blockers") or []):
        failures.append(
            "negative-path expected blocker 'no supplied transcript row satisfies "
            "the pass608 command/state/redraw binding contract' missing"
        )
    if failures:
        return 0, failures
    return 1, []


def _check_c8_binding_table_anchored_to_real_fixture() -> tuple[bool, str]:
    """C8: every row of LIVE_BINDING_TABLE must reference a
    pass623 route label that is present in the real on-disk
    ``pass623_dm1_v1_input_capture_readiness_bridge/manifest.json``
    AND a Firestaff fixture viewport sha that is present in the
    real on-disk
    ``verification-screens/capture_manifest_sha256.tsv``.
    Without this anchor the hermetic C4-C7 chain can pass
    against a synthetic fixture even when the live binding
    has drifted away from the canonical fixtures, and the
    next live DOSBox attempt would discover the drift at
    row-builder time (the row builder refuses to emit a row
    for a label that is not in the supplied pass623 fixture)
    instead of at the gate.  A future operator who renames a
    pass623 label, removes a row from the real pass623
    fixture, or rotates the Firestaff capture manifest
    without updating LIVE_BINDING_TABLE gets a self-describing
    failure message from this check."""
    failures: list[str] = []
    if not REAL_PASS623_FIXTURE.is_file():
        return False, f"real pass623 fixture missing: {REAL_PASS623_FIXTURE}"
    if not FIRESTAFF_CAPTURE_MANIFEST.is_file():
        return False, (
            f"real Firestaff capture manifest missing: {FIRESTAFF_CAPTURE_MANIFEST}"
        )
    try:
        pass623_payload = json.loads(REAL_PASS623_FIXTURE.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        return False, f"real pass623 fixture is not valid JSON: {exc}"
    pass623_labels = {
        str(row.get("label"))
        for row in (pass623_payload.get("canonicalInputCaptureRows") or [])
        if row.get("label")
    }
    if not pass623_labels:
        return False, (
            "real pass623 fixture has no canonicalInputCaptureRows; "
            "the gate cannot anchor LIVE_BINDING_TABLE"
        )

    firestaff_manifest_text = FIRESTAFF_CAPTURE_MANIFEST.read_text(encoding="utf-8")
    firestaff_shas: set[str] = set()
    for line in firestaff_manifest_text.splitlines():
        if not line.startswith("viewport_224x136"):
            continue
        parts = line.split("\t")
        if len(parts) >= 6:
            firestaff_shas.add(parts[5])
    if not firestaff_shas:
        return False, (
            f"real Firestaff capture manifest has no viewport_224x136 rows: "
            f"{FIRESTAFF_CAPTURE_MANIFEST}"
        )

    for binding in LIVE_BINDING_TABLE:
        if binding["pass623_label"] not in pass623_labels:
            failures.append(
                f"LIVE_BINDING_TABLE row live_filename={binding['live_filename']!r} "
                f"references pass623_label={binding['pass623_label']!r}, which is NOT "
                f"present in {REAL_PASS623_FIXTURE.relative_to(REPO_ROOT)} "
                f"(known labels: {sorted(pass623_labels)})"
            )
        if binding["firestaff_viewport_sha"] not in firestaff_shas:
            failures.append(
                f"LIVE_BINDING_TABLE row live_filename={binding['live_filename']!r} "
                f"references firestaff_viewport_sha={binding['firestaff_viewport_sha']!r}, "
                f"which is NOT present in {FIRESTAFF_CAPTURE_MANIFEST.relative_to(REPO_ROOT)}; "
                f"the pass608 verifier's firestaffViewportHashKnown check would reject "
                f"this row at the next live DOSBox attempt"
            )
    if failures:
        return False, "; ".join(failures)
    return True, (
        f"all {len(LIVE_BINDING_TABLE)} LIVE_BINDING_TABLE rows anchor to real pass623 "
        f"fixture ({len(pass623_labels)} known labels) and real Firestaff capture manifest "
        f"({len(firestaff_shas)} known viewport hashes)"
    )


# ---------------------------------------------------------------------------
# Main: drive all checks and report.
# ---------------------------------------------------------------------------

def main() -> int:
    failures: list[str] = []
    matched = 0
    total = 8

    sandbox = Path(tempfile.mkdtemp(prefix="live-row-gate-"))
    try:
        # C1
        ok, msg = _check_c1_baseline_blocked()
        if not ok:
            failures.append(f"c1_baseline_blocked: {msg}")
        else:
            matched += 1
            print(f"c1_baseline_blocked: {msg}  PASS")

        # C2
        m, fails = _check_c2_preflight_self_test()
        if m != 1 or fails:
            failures.append(f"c2_preflight_self_test: matched={m} fails={fails}")
        else:
            matched += 1
            print("c2_preflight_self_test: preflight self-test  PASS")

        # C3
        ok, msg = _check_c3_live_runner_filenames()
        if not ok:
            failures.append(f"c3_live_runner_filenames: {msg}")
        else:
            matched += 1
            print(f"c3_live_runner_filenames: {msg}  PASS")

        # C8 (anchored-binding check before the hermetic
        # C4-C7 chain so a drift in LIVE_BINDING_TABLE values
        # is caught before we burn time running the on-disk
        # toolchain against a binding the canonical fixtures
        # don't accept).
        ok, msg = _check_c8_binding_table_anchored_to_real_fixture()
        if not ok:
            failures.append(f"c8_binding_table_anchored_to_real_fixture: {msg}")
        else:
            matched += 1
            print(f"c8_binding_table_anchored_to_real_fixture: {msg}  PASS")

        # C4 + C5 + C6
        m, fails = _check_c4_c6_live_binding_flips(sandbox)
        if m != 1 or fails:
            failures.append(f"c4_c6_live_binding_flips: matched={m} fails={fails}")
        else:
            matched += 3
            print("c4_row_builder: row builder renders 41-column row per live binding  PASS")
            print("c5_transcript_writer: transcript.promotable is True  PASS")
            print("c6_pass608_promoted: pass608 status flipped to PROMOTED  PASS")

        # C7
        m, fails = _check_c7_negative_path(sandbox)
        if m != 1 or fails:
            failures.append(f"c7_negative_path: matched={m} fails={fails}")
        else:
            matched += 1
            print("c7_negative_path: unknown second-row hash keeps BLOCKED status  PASS")

    finally:
        shutil.rmtree(sandbox, ignore_errors=True)

    if failures:
        print(f"\n{STATUS}: {matched}/{total} live row gate checks failed")
        for fail in failures:
            print(f"  - {fail}")
        return 1
    print(f"\n{STATUS}: {matched}/{total} live row gate checks passed")
    print("PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
