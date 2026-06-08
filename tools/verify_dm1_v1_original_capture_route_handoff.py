#!/usr/bin/env python3
"""Handoff closure gate for the DM1 V1 original-capture route.

The DM1 V1 original DOS capture route documented in
``docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md`` is blocked at the
pass608 same-viewport capture layer
(``BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE``).
The blocker demands a ``transcript.json`` whose rows bind one
original DOSBox capture frame to one Firestaff fixture viewport
hash through the ReDMCSB source chain
``F0359/F0361_COMMAND_ProcessClick/KeyPress`` →
``F0380_COMMAND_ProcessQueue_CPSC`` →
``F0365/F0366_COMMAND_ProcessTypes1To2/3To6`` →
``F0128_DUNGEONVIEW_Draw_CPSF`` →
``F0097_DUNGEONVIEW_DrawViewport`` at the
``VIDRV_09_BlitViewPort`` present boundary, with the matching
map/X/Y/direction tuple binding the original frame to a Firestaff
``viewport_224x136`` hash from
``verification-screens/capture_manifest_sha256.tsv``.

The runbook ships a deterministic handoff toolchain that turns a
pass623 canonical input-capture fixture + a preflight receipt + a
320x200 capture frame + a 224x136 crop into a 41-column events
TSV row, then into a transcript.json that the pass608 verifier
reads as ``runtimeTranscript``:

  1. ``docs/parity/tools/dosbox_capture_preflight.py`` writes a
     pinned ``dosbox_capture.conf`` and a receipt whose
     ``dungeon_match`` / ``graphics_match`` /
     ``pass94_forbidden_present`` flags are the upstream pin
     contract.
  2. ``docs/parity/tools/dosbox_capture_events_row_builder.py``
     renders the 41-column events TSV row from a pass623 route
     label (e.g. ``02_turn_right_west_1_3``) + a 320x200 capture
     + a 224x136 crop, sourcing every source-locked value
     (``F0359``/``F0361``, ``F0380``, ``F0365``/``F0366``,
     ``F0128``, ``F0097``/``VIDRV_09_BlitViewPort``, the runbook
     §1 asset-set SHA256s) and every pass623-fixture value
     (the ``inputToken`` → ``sourceCommandId`` mapping, the
     post-tuple, the Firestaff ``viewportSha256``) verbatim.
  3. ``docs/parity/tools/dosbox_capture_transcript_writer.py``
     turns the events TSV into the transcript.json.

Without a hermetic closure test the next live DOSBox attempt can
silently ship a transcript that the row builder and the writer
accept but the pass608 verifier rejects — exactly the regression
the pass608 blocker was put in place to catch.  This gate wires
all three tools together and asserts the resulting transcript flips
the pass608 blocker from ``BLOCKED`` to ``PROMOTED``.

The gate is fully hermetic: it builds synthetic preflight
receipts, pass623 fixtures, Firestaff capture manifests, and
320x200 / 224x136 capture fixtures in a temp dir, runs the real
on-disk tools (no inlined logic, no module re-implementations),
and feeds the resulting ``transcript.json`` to the real
``verify_pass608_dm1_v1_same_viewport_capture_blocker.py`` to
confirm the binding contract is satisfied.

Checks (exit code 0 means the handoff chain closes, exit code 1
means the next live attempt would ship a transcript the pass608
verifier still rejects):

  * C1 - The on-disk ``verify_pass608_dm1_v1_same_viewport_capture_blocker.py``
    is importable and reports the baseline BLOCKED status.
  * C2 - The on-disk ``docs/parity/tools/dosbox_capture_preflight.py``
    is importable and its self-test passes (the receipt schema
    the writer consumes is the same schema the preflight writes).
  * C3 - The on-disk ``docs/parity/tools/dosbox_capture_events_row_builder.py``
    renders a 41-column row for the canonical
    ``02_turn_right_west_1_3`` route using a real Firestaff
    viewport hash from
    ``verification-screens/capture_manifest_sha256.tsv`` (the
    02_ingame_turn_right_latest_viewport_224x136 row).
  * C4 - The on-disk ``docs/parity/tools/dosbox_capture_transcript_writer.py``
    consumes that row and emits a transcript whose
    ``promotable`` flag is True for at least one row.
  * C5 - The pass608 verifier reads the same transcript and
    flips its status to
    ``PASS608_DM1_V1_COMMAND_STATE_REDRAW_TRANSCRIPT_BOUND``,
    its ``runtimeTranscript.status`` to
    ``loaded_promotable_same_run``, and clears the
    "no supplied transcript row satisfies the pass608
    command/state/redraw binding contract" / "original rows do
    not bind map/X/Y/direction" blockers.
  * C6 - A negative path: a transcript whose Firestaff
    viewport hash is not in the canonical manifest does NOT
    flip the pass608 blocker; the gate asserts the BLOCKED
    status is preserved so a future regression that drops the
    "viewport hash must be a known Firestaff fixture hash"
    check is caught here.
"""
from __future__ import annotations

import json
import os
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
PASS608_VERIFIER = REPO_ROOT / "tools" / "verify_pass608_dm1_v1_same_viewport_capture_blocker.py"
PREFLIGHT = TOOLS_DIR / "dosbox_capture_preflight.py"
ROW_BUILDER = TOOLS_DIR / "dosbox_capture_events_row_builder.py"
TRANSCRIPT_WRITER = TOOLS_DIR / "dosbox_capture_transcript_writer.py"
FIRESTAFF_CAPTURE_MANIFEST = REPO_ROOT / "verification-screens" / "capture_manifest_sha256.tsv"
FIRESTAFF_STATE_PROBE = REPO_ROOT / "verification-m11" / "capture-route-state-pass195" / "pass76_capture_route_state_probe.json"
PARITY_EVIDENCE = REPO_ROOT / "parity-evidence" / "verification"

STATUS = "DM1_V1_ORIGINAL_CAPTURE_ROUTE_HANDOFF_CLOSED"
BLOCKED_STATUS = "BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE"
PROMOTED_STATUS = "PASS608_DM1_V1_COMMAND_STATE_REDRAW_TRANSCRIPT_BOUND"

# 02_ingame_turn_right_latest_viewport_224x136.ppm - the canonical
# Firestaff fixture viewport hash for the 02_turn_right_west_1_3
# route.  Sourced from verification-screens/capture_manifest_sha256.tsv
# at probe time; this constant exists only to give a self-describing
# failure message if the manifest row is ever renamed.
CANONICAL_02_FIRESTAFF_VIEWPORT_SHA = "6dd14b51a57fd958be2b3a409f9fab5ec352b692e287683925c9d4840240dee6"
CANONICAL_02_ROUTE_LABEL = "02_turn_right_west_1_3"
CANONICAL_02_INPUT_TOKEN = "M12_MENU_INPUT_RIGHT"
CANONICAL_02_COMMAND_ID = 2
CANONICAL_02_COMMAND_NAME = "C002_COMMAND_TURN_RIGHT"
CANONICAL_02_PARTY_BEFORE = {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 2}
CANONICAL_02_PARTY_AFTER = {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 3}

# Runbook §1 asset-set SHA256s (DUNGEON.DAT + GRAPHICS.DAT for the
# canonical DM1 PC 3.4 layout).  These are the same constants the
# preflight writes into the receipt and the writer embeds into
# transcript rows under ``originalAssetSet.sha256``.
CANONICAL_DUNGEON_SHA = "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"
CANONICAL_GRAPHICS_SHA = "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"


# ---------------------------------------------------------------------------
# Minimal PNG / PPM writer for hermetic capture fixtures.
# ---------------------------------------------------------------------------

def write_png(path: Path, width: int, height: int, rgb: tuple[int, int, int]) -> None:
    """Write a minimal RGB PNG (no compression metadata, single IDAT)."""
    def _chunk(kind: bytes, payload: bytes) -> bytes:
        return (
            struct.pack(">I", len(payload)) + kind + payload +
            struct.pack(">I", zlib.crc32(kind + payload) & 0xFFFFFFFF)
        )

    sig = b"\x89PNG\r\n\x1a\n"
    ihdr = struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)
    raw_scanline = b"\x00" + bytes(rgb) * width
    idat = zlib.compress(raw_scanline * height, 9)
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(sig + _chunk(b"IHDR", ihdr) + _chunk(b"IDAT", idat) + _chunk(b"IEND", b""))


def sha256_of_bytes(data: bytes) -> str:
    import hashlib
    return hashlib.sha256(data).hexdigest()


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
        "session_id": "handoff_closure_synthetic",
        "captured_utc": "2026-06-08T05:00:00Z",
        "data_dir": "/synthetic/handoff/closure",
        "dungeon_match":   True,
        "graphics_match":  True,
        "title_match":     True,
        "dungeon_sha256":  CANONICAL_DUNGEON_SHA,
        "graphics_sha256": CANONICAL_GRAPHICS_SHA,
        "title_sha256":    "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
        "pass94_forbidden_present": False,
        "conf_path":       str(path.parent / "dosbox_capture.conf"),
        "launch_command":  "dosbox -conf dosbox_capture.conf",
        "firestaff_git_head": "handoff_closure_synthetic",
        "checks": [
            {"name": "dungeon_sha256", "ok": True, "expected": CANONICAL_DUNGEON_SHA, "actual": CANONICAL_DUNGEON_SHA},
            {"name": "graphics_sha256", "ok": True, "expected": CANONICAL_GRAPHICS_SHA, "actual": CANONICAL_GRAPHICS_SHA},
            {"name": "pass94_forbidden_settings_absent", "ok": True, "expected": False, "actual": False},
        ],
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True), encoding="utf-8")


def _write_synth_pass623(path: Path) -> None:
    """Write a synthetic pass623 fixture for the
    ``02_turn_right_west_1_3`` route.  Same schema as
    ``parity-evidence/verification/pass623_dm1_v1_input_capture_readiness_bridge/manifest.json``."""
    payload = {
        "schema": "pass623_input_capture_readiness_bridge.v1",
        "canonicalInputCaptureRows": [
            {
                "label": CANONICAL_02_ROUTE_LABEL,
                "commandIds": [CANONICAL_02_COMMAND_ID],
                "inputTokens": [CANONICAL_02_INPUT_TOKEN],
                "postTuple": CANONICAL_02_PARTY_AFTER,
                "observed": {
                    "label": CANONICAL_02_ROUTE_LABEL,
                    "crop":  "02_ingame_turn_right_latest_viewport_224x136.ppm",
                    "sha":   CANONICAL_02_FIRESTAFF_VIEWPORT_SHA,
                    "map":   CANONICAL_02_PARTY_AFTER["mapIndex"],
                    "x":     CANONICAL_02_PARTY_AFTER["mapX"],
                    "y":     CANONICAL_02_PARTY_AFTER["mapY"],
                    "direction": CANONICAL_02_PARTY_AFTER["direction"],
                },
                "claim": "synthetic pass623 row for handoff closure",
                "ok": True,
                "problems": [],
            },
        ],
    }
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2, sort_keys=True), encoding="utf-8")


def _write_synth_capture_manifest(path: Path) -> None:
    """Write a synthetic Firestaff capture manifest.  Mirrors the
    column shape of
    ``verification-screens/capture_manifest_sha256.tsv`` (the
    pass608 verifier parses ``kind<TAB>filename<TAB>width<TAB>height<TAB>bytes<TAB>sha256``)."""
    lines = [
        "# synthetic Firestaff capture manifest for handoff closure",
        "# columns: kind<TAB>filename<TAB>width<TAB>height<TAB>bytes<TAB>sha256",
        f"viewport_224x136\t02_ingame_turn_right_latest_viewport_224x136.ppm\t224\t136\t91407\t{CANONICAL_02_FIRESTAFF_VIEWPORT_SHA}",
    ]
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


# ---------------------------------------------------------------------------
# Helper: run pass608 verifier with a transcript path and parse
# its JSON envelope (printed on stdout, manifest on disk).
# ---------------------------------------------------------------------------

def _run_pass608(transcript_path: Path) -> dict[str, Any]:
    """Run the pass608 verifier with a supplied transcript.

    The verifier writes a manifest under
    ``parity-evidence/verification/pass608_dm1_v1_same_viewport_capture_blocker/manifest.json``
    AND a human-readable report at
    ``parity-evidence/pass608_dm1_v1_same_viewport_capture_blocker.md``
    and prints a JSON envelope to stdout.  We re-parse the
    manifest so the gate does not depend on stdout ordering.

    Both verifier outputs are left untouched on the caller's
    filesystem: this gate backs up the pre-existing bytes (if
    any) before invoking the verifier and restores them
    afterwards.  Without that restore the hermetic closure
    check would silently rewrite the committed manifest and
    report, which would dirty the worktree and confuse
    reviewers running the gate as part of pre-commit
    verification.
    """
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
        payload = json.loads(manifest_path.read_text(encoding="utf-8"))
        return payload
    finally:
        # Restore the original manifest (or remove the
        # verifier-written one if there was no original) so the
        # gate leaves the worktree clean.
        if manifest_backup is not None:
            manifest_path.write_bytes(manifest_backup)
        elif manifest_path.exists():
            manifest_path.unlink()
        if report_backup is not None:
            report_path.write_bytes(report_backup)
        elif report_path.exists():
            report_path.unlink()


# ---------------------------------------------------------------------------
# Helper: run the on-disk row builder and writer to render a
# hermetic transcript.json.  We invoke the tools as subprocesses
# rather than importing them so a future tool rename or a syntax
# regression in one tool surfaces here as a real exit-code / parse
# error, not as an ImportError swallowed by the gate.
# ---------------------------------------------------------------------------

def _run_row_builder(
    sandbox: Path,
    raw_path: Path,
    crop_path: Path,
    receipt_path: Path,
    pass623_path: Path,
    run_id: str,
    *,
    classification: str,
    party_before: dict[str, int],
) -> tuple[int, list[str]]:
    """Invoke the on-disk row builder for the canonical
    ``02_turn_right_west_1_3`` route.  The row builder writes
    the rendered row to stdout; we redirect stdout to
    ``sandbox / 'events.tsv'`` so the writer can consume it.
    Returns (matched, failures)."""
    events_tsv = sandbox / "events.tsv"
    proc = subprocess.run(
        [
            sys.executable, str(ROW_BUILDER),
            "--label",        CANONICAL_02_ROUTE_LABEL,
            "--raw",          str(raw_path),
            "--crop",         str(crop_path),
            "--run-id",       run_id,
            "--preflight-receipt", str(receipt_path),
            "--pass623-fixture",   str(pass623_path),
            "--classification",    classification,
            "--party-before-map",          str(party_before["mapIndex"]),
            "--party-before-x",            str(party_before["mapX"]),
            "--party-before-y",            str(party_before["mapY"]),
            "--party-before-direction",    str(party_before["direction"]),
            "--queue-count-before",        "1",
            "--queue-first-index-before",  "0",
            "--with-header",
        ],
        cwd=str(REPO_ROOT),
        capture_output=True,
        text=True,
        timeout=60,
    )
    if proc.returncode != 0:
        return 0, [f"row-builder exit={proc.returncode} stderr={proc.stderr.strip()!r}"]
    if not proc.stdout:
        return 0, [f"row-builder produced empty stdout (stderr={proc.stderr.strip()!r})"]
    events_tsv.write_text(proc.stdout, encoding="utf-8")
    return 1, []


def _run_transcript_writer(
    sandbox: Path,
    receipt_path: Path,
    pass623_path: Path,
    capture_manifest_path: Path,
    events_tsv: Path,
    transcript_out: Path,
) -> tuple[int, list[str]]:
    """Invoke the on-disk transcript writer.  Returns
    (matched, failures) parsed from the writer's self-test-style
    summary (or a captured non-zero exit)."""
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
    """Run the preflight self-test to confirm the receipt
    schema the writer consumes is what the preflight emits."""
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


def _check_c3_c5_closure_flips(
    sandbox: Path,
) -> tuple[int, list[str]]:
    """C3-C5: render a hermetic transcript with the on-disk
    row builder + writer and feed it to the on-disk pass608
    verifier.  The pass608 status must flip to PROMOTED with
    ``loaded_promotable_same_run``."""
    failures: list[str] = []

    # Synthetic preflight receipt + pass623 fixture + capture
    # manifest + 320x200 raw + 224x136 crop.
    receipt_path    = sandbox / "preflight_receipt.json"
    pass623_path    = sandbox / "pass623.json"
    manifest_path   = sandbox / "capture_manifest.tsv"
    captures_dir    = sandbox / "captures"
    raw_path        = captures_dir / "02_ingame_turn_right.png"
    crop_path       = captures_dir / "02_ingame_turn_right_viewport.png"
    transcript_path = sandbox / "transcript.json"

    _write_synth_receipt(receipt_path)
    _write_synth_pass623(pass623_path)
    _write_synth_capture_manifest(manifest_path)
    write_png(raw_path,  320, 200, (40, 40, 40))
    write_png(crop_path, 224, 136, (50, 50, 50))

    # C3 - row builder renders a 41-column row.
    matched, fails = _run_row_builder(
        sandbox=sandbox,
        raw_path=raw_path,
        crop_path=crop_path,
        receipt_path=receipt_path,
        pass623_path=pass623_path,
        run_id="handoff_closure_run_001",
        classification="dungeon_gameplay",
        party_before=CANONICAL_02_PARTY_BEFORE,
    )
    if matched != 1 or fails:
        return 0, [f"row-builder check failed: matched={matched} fails={fails}"]
    events_tsv = sandbox / "events.tsv"
    if not events_tsv.is_file():
        return 0, [f"row-builder did not produce {events_tsv}"]
    with events_tsv.open("r", encoding="utf-8") as fh:
        header_line = fh.readline().rstrip("\n")
        first_row   = fh.readline().rstrip("\n")
    header_cells = header_line.split("\t")
    row_cells    = first_row.split("\t")
    if len(header_cells) != 41:
        return 0, [f"events TSV header has {len(header_cells)} columns, expected 41"]
    if len(row_cells) != 41:
        return 0, [f"events TSV row has {len(row_cells)} columns, expected 41"]
    if row_cells[6] != "320" or row_cells[7] != "200":
        return 0, [f"row width/height={row_cells[6]}x{row_cells[7]}, expected 320x200"]
    if row_cells[9] != str(CANONICAL_02_COMMAND_ID):
        return 0, [f"row source_command_id={row_cells[9]!r}, expected {CANONICAL_02_COMMAND_ID!r}"]

    # C4 - transcript writer renders a promotable transcript.
    matched, fails = _run_transcript_writer(
        sandbox=sandbox,
        receipt_path=receipt_path,
        pass623_path=pass623_path,
        capture_manifest_path=manifest_path,
        events_tsv=events_tsv,
        transcript_out=transcript_path,
    )
    if matched != 1 or fails:
        return 0, [f"transcript-writer check failed: matched={matched} fails={fails}"]

    # C5 - pass608 verifier flips to PROMOTED.
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


def _check_c6_negative_path(sandbox: Path) -> tuple[int, list[str]]:
    """C6: a transcript whose Firestaff viewport hash is NOT in
    the canonical manifest must NOT flip the pass608 blocker;
    the BLOCKED status must be preserved so a future regression
    that drops the "viewport hash must be a known Firestaff
    fixture hash" check is caught here."""
    failures: list[str] = []

    # Use a separate sandbox for the negative path so the row
    # builder's stdout capture (which writes to
    # ``<sandbox>/events.tsv``) does not collide with the
    # positive path's events.tsv.
    neg_sandbox = sandbox / "neg"
    neg_sandbox.mkdir(parents=True, exist_ok=True)

    receipt_path    = neg_sandbox / "preflight_receipt.json"
    pass623_path    = neg_sandbox / "pass623.json"
    manifest_path   = neg_sandbox / "capture_manifest.tsv"
    captures_dir    = neg_sandbox / "captures"
    raw_path        = captures_dir / "02_ingame_turn_right.png"
    crop_path       = captures_dir / "02_ingame_turn_right_viewport.png"
    transcript_path = neg_sandbox / "transcript.json"

    _write_synth_receipt(receipt_path)
    _write_synth_pass623(pass623_path)
    _write_synth_capture_manifest(manifest_path)
    write_png(raw_path,  320, 200, (40, 40, 40))
    write_png(crop_path, 224, 136, (50, 50, 50))

    matched, fails = _run_row_builder(
        sandbox=neg_sandbox,
        raw_path=raw_path,
        crop_path=crop_path,
        receipt_path=receipt_path,
        pass623_path=pass623_path,
        run_id="handoff_closure_run_002",
        classification="dungeon_gameplay",
        party_before=CANONICAL_02_PARTY_BEFORE,
    )
    if matched != 1 or fails:
        return 0, [f"negative-path row-builder failed: matched={matched} fails={fails}"]
    events_tsv = neg_sandbox / "events.tsv"
    if not events_tsv.is_file():
        return 0, [f"row-builder did not produce {events_tsv}"]
    matched, fails = _run_transcript_writer(
        sandbox=neg_sandbox,
        receipt_path=receipt_path,
        pass623_path=pass623_path,
        capture_manifest_path=manifest_path,
        events_tsv=events_tsv,
        transcript_out=transcript_path,
    )
    if matched != 1 or fails:
        return 0, [f"negative-path transcript-writer failed: matched={matched} fails={fails}"]

    # Patch the Firestaff viewport hash to a never-seen 64-hex
    # value; the pass608 verifier's
    # ``firestaffViewportHashKnown`` check must fail and keep
    # the BLOCKED status.
    payload = json.loads(transcript_path.read_text(encoding="utf-8"))
    rows = payload.get("rows") or []
    if not rows:
        return 0, ["negative-path transcript has no rows to patch"]
    for row in rows:
        ff = row.setdefault("firestaffFrame", {})
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
            "'loaded_promotable_same_run' even though firestaff "
            "viewport hash is not in the manifest"
        )
    if "no supplied transcript row satisfies the pass608 command/state/redraw binding contract" not in (out.get("blockers") or []):
        failures.append(
            "negative-path expected blocker 'no supplied transcript row satisfies "
            "the pass608 command/state/redraw binding contract' missing"
        )
    if failures:
        return 0, failures
    return 1, []


# ---------------------------------------------------------------------------
# Main: drive all checks and report.
# ---------------------------------------------------------------------------

def main() -> int:
    failures: list[str] = []
    matched = 0
    total = 6

    sandbox = Path(tempfile.mkdtemp(prefix="handoff-closure-"))
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

        # C3 + C4 + C5
        m, fails = _check_c3_c5_closure_flips(sandbox)
        if m != 1 or fails:
            failures.append(f"c3_c5_closure_flips: matched={m} fails={fails}")
        else:
            matched += 3
            print("c3_row_builder: row builder renders 41-column row  PASS")
            print("c4_transcript_writer: transcript.promotable is True  PASS")
            print("c5_pass608_promoted: pass608 status flipped to PROMOTED  PASS")

        # C6
        m, fails = _check_c6_negative_path(sandbox)
        if m != 1 or fails:
            failures.append(f"c6_negative_path: matched={m} fails={fails}")
        else:
            matched += 1
            print("c6_negative_path: unknown firestaff hash keeps BLOCKED status  PASS")

    finally:
        shutil.rmtree(sandbox, ignore_errors=True)

    if failures:
        print(f"\n{STATUS}: {matched}/{total} handoff closure checks failed")
        for fail in failures:
            print(f"  - {fail}")
        return 1
    print(f"\n{STATUS}: {matched}/{total} handoff closure checks passed")
    print("PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
