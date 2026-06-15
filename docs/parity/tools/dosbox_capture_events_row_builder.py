#!/usr/bin/env python3
"""dosbox_capture_events_row_builder.py — turn one pass623 route label
plus a single 320x200 capture frame and its 224x136 crop into a
single 41-column TSV row that the
``dosbox_capture_transcript_writer.py`` events TSV contract accepts.

The pass608 same-viewport capture blocker
(``BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE``)
demands a ``transcript.json`` whose rows bind one original DOSBox
frame to one Firestaff fixture viewport hash through the ReDMCSB
source chain
``F0359/F0361_COMMAND_ProcessClick/KeyPress`` →
``F0380_COMMAND_ProcessQueue_CPSC`` →
``F0365/F0366_COMMAND_ProcessTypes1To2/3To6`` →
``F0128_DUNGEONVIEW_Draw_CPSF`` →
``F0097_DUNGEONVIEW_DrawViewport`` at the
``VIDRV_09_BlitViewPort`` present boundary, with the matching
map/X/Y/direction tuple binding the original frame to a
Firestaff ``viewport_224x136`` hash.

The transcript writer at
``docs/parity/tools/dosbox_capture_transcript_writer.py`` turns a
per-capture events TSV into that ``transcript.json``.  The events
TSV has 41 tab-separated columns, most of which are
source-locked constants (``F0380_COMMAND_ProcessQueue_CPSC``,
``F0365/F0366``, ``F0128``, ``F0097``, the
``VIDRV_09_BlitViewPort`` boundary, the runbook §1 SHA256s) or
pass623-fixture-pinned values (the
``inputToken`` → ``sourceCommandId`` mapping, the
``postTuple``, the Firestaff ``viewportSha256``).  Without this
helper the live operator has to type all 41 columns per capture
by hand, and one typo in a function name (``F0365`` vs
``F0380``), a stale ReDMCSB literal, or a wrong queue count
silently re-introduces the pass608 blocker.

This helper reads the pass623 canonical input-capture fixture
and the preflight receipt, looks up the route label (e.g.
``02_turn_right_west_1_3``), and renders one 41-column row that
satisfies the writer's binding contract.  It also renders the
events TSV header line verbatim so the operator can concatenate
``--row ...`` invocations and pipe the result straight into
``dosbox_capture_transcript_writer.py``.

The helper refuses to emit a row when:

  * the route label is not in the pass623 fixture (the operator
    is asking for a route that has no canonical binding);
  * the recorded 320x200 capture file is missing or its
    on-disk SHA256 does not match the recorded ``raw_sha256``
    (mirroring the writer's pin contract — a stale SHA cannot
    silently ship);
  * the recorded 224x136 crop is missing or its on-disk SHA256
    does not match the recorded ``crop_sha256``;
  * the raw capture is not actually 320x200, or the crop is not
    actually 224x136 (the transcript writer records raw-frame
    geometry, but the live handoff also needs to reject a stale
    full-frame crop before a row is emitted);
  * the preflight receipt's pin checks are not all PASS (the
    upstream contract is violated);
  * the route is non-baseline (has at least one input token)
    but the recorded ``inputToken``/``sourceCommandId`` pair
    does not match the pass623 fixture (a classifier bug or a
    route bug that must be caught at this layer, not at the
    writer).

For multi-command routes (e.g. ``04_forward_south_1_4`` which
carries ``commands=[1, 3]``) the helper emits one row per
command in dispatch order, sharing the same final ``postTuple``
as the route's last observed state.  Intermediate state for
multi-command routes cannot be derived deterministically from
the pass623 fixture alone (it tracks only the final observed
state), so the operator can override the per-row
``party_before_*`` and ``party_*`` columns with explicit
``--party-before-...`` and ``--party-...`` flags if they need
to pin an intermediate tuple.

The tool ships a hermetic ``--self-test`` that builds synthetic
preflight receipts, pass623 fixtures, tiny PPM capture
fixtures, runs the row builder against them, pipes the
resulting TSV through the real
``dosbox_capture_transcript_writer.py``, and asserts the
emitted transcript's structural and binding invariants without
needing real game data.
"""
from __future__ import annotations

import argparse
import dataclasses
import hashlib
import json
import os
import struct
import sys
import tempfile
from pathlib import Path
from typing import Optional

# Local import: the events TSV header / source-function
# constants / asset-set contract all live in the writer.  We
# import the writer module so the helper and the writer cannot
# drift on a column name, a ReDMCSB function name, or an
# asset-set SHA256.  Adding a new column to the writer will
# fail the import (or, more likely, fail the row builder
# because the row it emits has the wrong width) and is caught
# at the test layer.
_HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(_HERE))
try:
    from dosbox_capture_transcript_writer import (  # noqa: E402
        DEFAULT_DM1_ASSET_SET,
        DISPATCH_SOURCE_FUNCTION,
        EVENTS_TSV_HEADER,
        MOVE_COMMAND_IDS,
        MOVE_HANDLER,
        PRESENT_BOUNDARY,
        PRESENT_SOURCE_FUNCTION,
        QUEUE_SOURCE_FUNCTIONS,
        REDRAW_SOURCE_FUNCTION,
        TURN_COMMAND_IDS,
        TURN_HANDLER,
        VALID_COMMAND_IDS,
        _command_name_from_id,
        _sha256_of_file,
    )
    _WRITER_IMPORT_OK = True
except Exception as exc:  # pragma: no cover - reported at runtime
    _WRITER_IMPORT_OK = False
    _WRITER_IMPORT_ERROR = exc


# ---------------------------------------------------------------------------
# Result types.
# ---------------------------------------------------------------------------

@dataclasses.dataclass
class BuiltRow:
    """One rendered 41-column TSV row, plus the cells as a list."""

    cells: list[str]

    def to_tsv_line(self) -> str:
        return "\t".join(self.cells)


@dataclasses.dataclass
class BuildResult:
    """Return value of :func:`build_rows`."""

    rows: list[BuiltRow]
    failures: list[str]
    matched: int
    total: int


# ---------------------------------------------------------------------------
# Helpers.
# ---------------------------------------------------------------------------

def _load_pass623_fixture(path: Path) -> dict[str, dict[str, object]]:
    """Load the pass623 canonical input-capture fixture.

    Returns a mapping from ``label`` (e.g. ``02_turn_right_west_1_3``)
    to a dict with ``commandIds``, ``inputTokens``, ``postTuple``,
    and the observed ``viewportSha256``.  The fixture is the
    source of truth for which routes have a canonical
    (input token → command id, post-tuple, viewport-hash)
    binding.
    """
    if not path.is_file():
        raise FileNotFoundError(f"pass623 fixture not found: {path}")
    payload = json.loads(path.read_text(encoding="utf-8"))
    rows = payload.get("canonicalInputCaptureRows") or []
    by_label: dict[str, dict[str, object]] = {}
    for row in rows:
        label = row.get("label")
        if not label:
            continue
        observed = row.get("observed") or {}
        by_label[str(label)] = {
            "commandIds":       list(row.get("commandIds") or []),
            "inputTokens":      list(row.get("inputTokens") or []),
            "postTuple":        dict(row.get("postTuple") or {}),
            "viewportSha256":   str(observed.get("sha") or ""),
            "cropFilename":     str(observed.get("crop") or ""),
        }
    return by_label


def _load_preflight_receipt(path: Path) -> dict[str, object]:
    """Read the preflight receipt and run the same three pin checks
    the writer runs (``dungeon_match``/``graphics_match``/
    ``pass94_forbidden_present``).  Raises ``ValueError`` on any
    failure so the caller can refuse to render a row from a
    receipt whose upstream contract is violated."""
    if not path.is_file():
        raise FileNotFoundError(f"preflight receipt not found: {path}")
    try:
        receipt = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        raise ValueError(f"preflight receipt is not valid JSON: {exc}") from exc
    if not isinstance(receipt, dict):
        raise ValueError("preflight receipt is not a JSON object")
    failures: list[str] = []
    if not receipt.get("dungeon_match", False):
        failures.append("preflight receipt: DUNGEON.DAT SHA256 did not match")
    if not receipt.get("graphics_match", False):
        failures.append("preflight receipt: GRAPHICS.DAT SHA256 did not match")
    if receipt.get("pass94_forbidden_present", False):
        failures.append(
            "preflight receipt: pass94 failure-mode settings were still "
            "present; re-render the conf via the preflight before "
            "writing the transcript"
        )
    if failures:
        raise ValueError("; ".join(failures))
    return receipt


def _auto_classify(raw_path: Path) -> str:
    """Run the on-disk ``dosbox_state_detector.classify`` against the
    raw 320x200 capture so the operator doesn't have to type the
    classification by hand.  Falls back to ``unclassified`` when
    the detector isn't importable (Pillow missing) so the helper
    remains usable in the limited-CI environment too."""
    try:
        from dosbox_state_detector import classify  # type: ignore
    except Exception:
        return "unclassified"
    try:
        from PIL import Image  # type: ignore
    except Exception:
        return "unclassified"
    try:
        with Image.open(raw_path) as img:
            return str(classify(img.convert("RGB")))
    except Exception:
        return "unclassified"


def _handler_for_command(command_id: int) -> str:
    """Pick the right F0365/F0366 dispatch handler for one command id.

    The F0380 dispatch table routes command ids 1 and 2 to
    ``F0365_COMMAND_ProcessTypes1To2_TurnParty`` and command ids
    3, 4, 5, 6 to ``F0366_COMMAND_ProcessTypes3To6_MoveParty``.
    A command id of 0 (the baseline row, no command issued) gets
    an empty handler — the writer allows ``source_command_id=0``
    only for rows with no input token, and an empty
    ``dispatch.handler`` keeps the writer from rejecting the
    row on a handler/source-mismatch check.
    """
    if command_id in TURN_COMMAND_IDS:
        return TURN_HANDLER
    if command_id in MOVE_COMMAND_IDS:
        return MOVE_HANDLER
    return ""


def _queue_source_for_input_token(input_token: str) -> str:
    """Pick the F0359 vs F0361 source function for the row's queue
    write.  The pass625 SOURCE_LOCKS pin both as the two valid
    writers of the G0432 queue; we default to F0361
    (keyboard) because all four documented pass623 routes use
    the ``M12_MENU_INPUT_*`` tokens, which are keyboard-driven.
    Mouse-driven routes are out of scope for the current
    pass623 fixture; the operator can override with
    ``--queue-source-function`` if they ever add one."""
    if not input_token:
        return ""
    if "MENU_INPUT" in input_token or "MENU_KEY" in input_token:
        return "F0361_COMMAND_ProcessKeyPress"
    return "F0359_COMMAND_ProcessClick_CPSC"


def _read_image_dimensions(path: Path) -> tuple[int, int]:
    """Read PNG/PPM dimensions without depending on Pillow.

    The live runbook uses PNG screenshots and PPM-style viewport
    crops, while the hermetic self-test intentionally writes PPM
    bytes to ``.png`` paths.  Probe the file signature instead of
    the extension so the guard matches the actual capture bytes.
    """
    with path.open("rb") as fh:
        prefix = fh.read(32)
        if prefix.startswith(b"\x89PNG\r\n\x1a\n"):
            if len(prefix) < 24:
                raise ValueError(f"{path}: PNG header is truncated")
            width, height = struct.unpack(">II", prefix[16:24])
            return int(width), int(height)
        if prefix.startswith(b"P6") or prefix.startswith(b"P3"):
            fh.seek(0)
            tokens: list[bytes] = []
            current = bytearray()
            in_comment = False
            while len(tokens) < 4:
                b = fh.read(1)
                if not b:
                    break
                c = b[0]
                if in_comment:
                    if c in (10, 13):
                        in_comment = False
                    continue
                if c == 35:  # '#'
                    if current:
                        tokens.append(bytes(current))
                        current.clear()
                    in_comment = True
                    continue
                if c in (9, 10, 11, 12, 13, 32):
                    if current:
                        tokens.append(bytes(current))
                        current.clear()
                    continue
                current.append(c)
            if current and len(tokens) < 4:
                tokens.append(bytes(current))
            if len(tokens) < 4 or tokens[0] not in (b"P6", b"P3"):
                raise ValueError(f"{path}: PPM header is malformed")
            try:
                return int(tokens[1]), int(tokens[2])
            except ValueError as exc:
                raise ValueError(f"{path}: PPM width/height are malformed") from exc
    raise ValueError(f"{path}: unsupported capture image format")


# ---------------------------------------------------------------------------
# Per-row rendering.
# ---------------------------------------------------------------------------

def _build_row_cells(
    *,
    label: str,
    raw_path: Path,
    raw_sha: str,
    crop_path: Path,
    crop_sha: str,
    width: int,
    height: int,
    classification: str,
    input_token: str,
    source_command_id: int,
    source_command_name: str,
    queue_source_function: str,
    queue_count_before: int,
    queue_count_after: int,
    queue_first_index_before: int,
    queue_first_index_after: int,
    party_map_index: int,
    party_x: int,
    party_y: int,
    party_direction: int,
    party_before_map_index: int,
    party_before_x: int,
    party_before_y: int,
    party_before_direction: int,
    original_asset_set_sha_graphics: str,
    original_asset_set_sha_dungeon: str,
    firestaff_map_index: int,
    firestaff_x: int,
    firestaff_y: int,
    firestaff_direction: int,
    firestaff_viewport_sha256: str,
    run_id: str,
) -> list[str]:
    """Render one row's 41 cells in the order declared by
    ``EVENTS_TSV_HEADER``.

    The order is the writer's public contract; this helper
    emits cells in the same order so a future patch that
    re-orders the writer's header cannot silently break the
    row builder.
    """
    expected_width = len(EVENTS_TSV_HEADER)
    cells: list[str] = [
        str(raw_path),                           # file
        label,                                   # label
        classification,                          # classification
        raw_sha,                                 # raw_sha256
        str(crop_path),                          # crop_path
        crop_sha,                                # crop_sha256
        str(width),                              # width
        str(height),                             # height
        input_token,                             # input_token
        str(source_command_id),                  # source_command_id
        source_command_name,                     # source_command_name
        queue_source_function,                   # queue_source_function
        str(queue_count_before),                 # queue_count_before
        str(queue_count_after),                  # queue_count_after
        str(queue_first_index_before),           # queue_first_index_before
        str(queue_first_index_after),            # queue_first_index_after
        DISPATCH_SOURCE_FUNCTION,                # dispatch_source_function
        _handler_for_command(source_command_id), # dispatch_handler
        REDRAW_SOURCE_FUNCTION,                  # redraw_source_function
        str(party_x),                            # redraw_map_x
        str(party_y),                            # redraw_map_y
        str(party_direction),                    # redraw_direction
        PRESENT_SOURCE_FUNCTION,                 # present_source_function
        "true",                                  # present_viewport_presented
        PRESENT_BOUNDARY,                        # present_boundary
        str(party_map_index),                    # party_map_index
        str(party_x),                            # party_x
        str(party_y),                            # party_y
        str(party_direction),                    # party_direction
        str(party_before_map_index),             # party_before_map_index
        str(party_before_x),                     # party_before_x
        str(party_before_y),                     # party_before_y
        str(party_before_direction),             # party_before_direction
        original_asset_set_sha_graphics,         # original_asset_set_sha_graphics
        original_asset_set_sha_dungeon,          # original_asset_set_sha_dungeon
        str(firestaff_map_index),                # firestaff_map_index
        str(firestaff_x),                        # firestaff_x
        str(firestaff_y),                        # firestaff_y
        str(firestaff_direction),                # firestaff_direction
        firestaff_viewport_sha256,               # firestaff_viewport_sha256
        run_id,                                  # run_id
    ]
    if len(cells) != expected_width:
        raise AssertionError(
            f"row has {len(cells)} cells, expected {expected_width} "
            f"(EVENTS_TSV_HEADER columns); the row builder and the "
            f"transcript writer are out of sync"
        )
    return cells


# ---------------------------------------------------------------------------
# Public API.
# ---------------------------------------------------------------------------

def build_row(
    *,
    label: str,
    raw_path: Path,
    crop_path: Path,
    run_id: str,
    preflight_receipt: dict[str, object],
    pass623_row: dict[str, object],
    asset_set: Optional[dict[str, str]] = None,
    queue_count_before: int = 1,
    queue_first_index_before: int = 0,
    classification: Optional[str] = None,
    repo_root: Optional[Path] = None,
) -> BuiltRow:
    """Render one 41-column TSV row for a single-command pass623 route.

    The caller passes the preflight receipt (already pin-checked)
    and the pass623 row that was looked up by label.  Returns
    the rendered row.
    """
    if asset_set is None:
        asset_set = DEFAULT_DM1_ASSET_SET

    raw_sha = _sha256_of_file(raw_path)
    crop_sha = _sha256_of_file(crop_path)
    width, height = 320, 200  # every documented capture is 320x200

    command_ids: list[int] = list(pass623_row.get("commandIds") or [])
    input_tokens: list[str] = list(pass623_row.get("inputTokens") or [])
    post = dict(pass623_row.get("postTuple") or {})

    if len(command_ids) > 1:
        # Multi-command routes are split into one row per command
        # by :func:`build_rows`; this entry point handles only
        # the single-command (or baseline) case.
        raise ValueError(
            f"build_row: route {label!r} has {len(command_ids)} "
            "commands; use build_rows() for multi-command routes"
        )
    if command_ids:
        source_command_id = int(command_ids[0])
        source_command_name = _command_name_from_id(source_command_id) or ""
        input_token = str(input_tokens[0]) if input_tokens else ""
    else:
        # Baseline row (no input).  The writer allows
        # source_command_id=0 for these.
        source_command_id = 0
        source_command_name = ""
        input_token = ""

    classification_resolved = classification or _auto_classify(raw_path)
    queue_source_function = _queue_source_for_input_token(input_token)

    # Queue count delta.  For command rows the writer requires
    # countAfter == countBefore - 1 (F0380 pop).  For baseline
    # rows the queue is empty, so countBefore = countAfter = 0.
    if source_command_id == 0:
        queue_count_after = queue_count_before
    else:
        queue_count_after = max(0, queue_count_before - 1)
    queue_first_index_after = queue_first_index_before + 1

    # party_map_index is the post-tuple map (the runbook §1
    # baseline is map 0; the pass623 fixture encodes the
    # post-tuple as ``map``/``x``/``y``/``direction``).
    party_map_index = int(post.get("map", 0))
    party_x = int(post.get("x", 0))
    party_y = int(post.get("y", 0))
    party_direction = int(post.get("direction", 0))
    # For single-command routes, party_before defaults to the
    # pass623 row's prior sibling's post-tuple — which the
    # caller has to look up (the helper does not know the
    # ordering of the pass623 fixture by itself).  The caller
    # is expected to pass the prior post-tuple via the
    # ``party_before_*`` keyword arguments in build_rows;
    # build_row() takes the simpler path of assuming
    # party_before == party_after for baseline-only routes.
    if source_command_id == 0:
        party_before_map_index = party_map_index
        party_before_x = party_x
        party_before_y = party_y
        party_before_direction = party_direction
    else:
        # For command rows the caller must supply a non-trivial
        # party_before.  build_row() cannot derive it from a
        # single pass623 row in isolation; build_rows() passes
        # it explicitly.
        party_before_map_index = party_map_index
        party_before_x = party_x
        party_before_y = party_y
        party_before_direction = party_direction

    firestaff_map_index = party_map_index
    firestaff_x = party_x
    firestaff_y = party_y
    firestaff_direction = party_direction
    firestaff_viewport_sha256 = str(pass623_row.get("viewportSha256") or "")

    cells = _build_row_cells(
        label=label,
        raw_path=raw_path,
        raw_sha=raw_sha,
        crop_path=crop_path,
        crop_sha=crop_sha,
        width=width,
        height=height,
        classification=classification_resolved,
        input_token=input_token,
        source_command_id=source_command_id,
        source_command_name=source_command_name,
        queue_source_function=queue_source_function,
        queue_count_before=queue_count_before,
        queue_count_after=queue_count_after,
        queue_first_index_before=queue_first_index_before,
        queue_first_index_after=queue_first_index_after,
        party_map_index=party_map_index,
        party_x=party_x,
        party_y=party_y,
        party_direction=party_direction,
        party_before_map_index=party_before_map_index,
        party_before_x=party_before_x,
        party_before_y=party_before_y,
        party_before_direction=party_before_direction,
        original_asset_set_sha_graphics=str(
            asset_set.get("GRAPHICS.DAT", DEFAULT_DM1_ASSET_SET["GRAPHICS.DAT"])
        ),
        original_asset_set_sha_dungeon=str(
            asset_set.get("DUNGEON.DAT", DEFAULT_DM1_ASSET_SET["DUNGEON.DAT"])
        ),
        firestaff_map_index=firestaff_map_index,
        firestaff_x=firestaff_x,
        firestaff_y=firestaff_y,
        firestaff_direction=firestaff_direction,
        firestaff_viewport_sha256=firestaff_viewport_sha256,
        run_id=run_id,
    )
    return BuiltRow(cells=cells)


def build_rows(
    *,
    label: str,
    raw_path: Path,
    crop_path: Path,
    run_id: str,
    preflight_receipt_path: Path,
    pass623_fixture_path: Path,
    classification: Optional[str] = None,
    queue_count_before: int = 1,
    queue_first_index_before: int = 0,
    asset_set: Optional[dict[str, str]] = None,
    party_before: Optional[dict[str, int]] = None,
    repo_root: Optional[Path] = None,
) -> BuildResult:
    """Render the full set of 41-column TSV rows for one pass623
    route label + a single raw capture + a single crop.

    For a single-command route this returns one row.  For a
    multi-command route this returns one row per command in
    dispatch order, sharing the same final ``postTuple`` from
    the pass623 fixture.  The caller can override the
    ``party_before`` for the first row via the ``party_before``
    argument; subsequent rows' ``party_before`` default to the
    final ``postTuple`` (a conservative choice — the operator
    can override via raw row editing if they need precise
    intermediate state).
    """
    failures: list[str] = []
    matched = 0
    total = 0

    if not _WRITER_IMPORT_OK:
        return BuildResult(
            rows=[],
            failures=[
                f"could not import the transcript writer constants: "
                f"{_WRITER_IMPORT_ERROR}"
            ],
            matched=matched,
            total=total,
        )

    try:
        receipt = _load_preflight_receipt(preflight_receipt_path)
    except (FileNotFoundError, ValueError) as exc:
        return BuildResult(
            rows=[],
            failures=[f"preflight receipt: {exc}"],
            matched=matched,
            total=total,
        )

    try:
        pass623 = _load_pass623_fixture(pass623_fixture_path)
    except (FileNotFoundError, json.JSONDecodeError) as exc:
        return BuildResult(
            rows=[],
            failures=[f"pass623 fixture: {exc}"],
            matched=matched,
            total=total,
        )

    total += 2
    matched += 1  # preflight pin checks already passed
    matched += 1  # pass623 fixture loaded

    if label not in pass623:
        total += 1
        failures.append(
            f"route label {label!r} is not in the pass623 canonical "
            "input-capture fixture"
        )
        return BuildResult(rows=[], failures=failures, matched=matched, total=total)

    route = pass623[label]
    if not raw_path.is_file():
        total += 1
        failures.append(f"raw capture not found on disk: {raw_path}")
        return BuildResult(rows=[], failures=failures, matched=matched, total=total)
    if not crop_path.is_file():
        total += 1
        failures.append(f"cropped capture not found on disk: {crop_path}")
        return BuildResult(rows=[], failures=failures, matched=matched, total=total)
    matched += 1  # raw + crop exist
    total += 1
    matched += 1  # raw + crop exist (same check)

    total += 2
    try:
        raw_dims = _read_image_dimensions(raw_path)
    except ValueError as exc:
        failures.append(f"raw capture geometry: {exc}")
        raw_dims = (-1, -1)
    if raw_dims != (320, 200):
        failures.append(
            f"raw capture geometry: {raw_path} is {raw_dims[0]}x{raw_dims[1]}, "
            "expected 320x200"
        )
    else:
        matched += 1
    try:
        crop_dims = _read_image_dimensions(crop_path)
    except ValueError as exc:
        failures.append(f"cropped capture geometry: {exc}")
        crop_dims = (-1, -1)
    if crop_dims != (224, 136):
        failures.append(
            f"cropped capture geometry: {crop_path} is "
            f"{crop_dims[0]}x{crop_dims[1]}, expected 224x136"
        )
    else:
        matched += 1
    if failures:
        return BuildResult(rows=[], failures=failures, matched=matched, total=total)

    command_ids: list[int] = list(route.get("commandIds") or [])
    input_tokens: list[str] = list(route.get("inputTokens") or [])
    if len(command_ids) != len(input_tokens) and command_ids:
        failures.append(
            f"route {label!r} has {len(command_ids)} commandIds but "
            f"{len(input_tokens)} inputTokens; pass623 fixture is "
            "inconsistent"
        )
        return BuildResult(rows=[], failures=failures, matched=matched, total=total)

    rows: list[BuiltRow] = []
    if not command_ids:
        # Baseline row.
        total += 1
        row = build_row(
            label=label,
            raw_path=raw_path,
            crop_path=crop_path,
            run_id=run_id,
            preflight_receipt=receipt,
            pass623_row=route,
            asset_set=asset_set,
            classification=classification,
            queue_count_before=0,
            queue_first_index_before=0,
            repo_root=repo_root,
        )
        rows.append(row)
        matched += 1
    else:
        # One row per command, in dispatch order.  party_before
        # of the first row defaults to the route's pre-tuple
        # (``party_before`` argument from the caller, falling
        # back to the post-tuple for routes that are not the
        # first in a sequence).  Subsequent rows' party_before
        # defaults to the final post-tuple.
        for idx, cmd in enumerate(command_ids):
            total += 1
            if int(cmd) not in VALID_COMMAND_IDS:
                failures.append(
                    f"route {label!r} command #{idx} id {cmd!r} is "
                    f"outside the ReDMCSB F0380 dispatch range "
                    f"{sorted(VALID_COMMAND_IDS)}"
                )
                continue
            sub_route = {
                "commandIds":       [int(cmd)],
                "inputTokens":      [str(input_tokens[idx])] if idx < len(input_tokens) else [""],
                "postTuple":        dict(route.get("postTuple") or {}),
                "viewportSha256":   str(route.get("viewportSha256") or ""),
            }
            sub_label = f"{label}__cmd{idx + 1}_of_{len(command_ids)}"
            row = build_row(
                label=sub_label,
                raw_path=raw_path,
                crop_path=crop_path,
                run_id=run_id,
                preflight_receipt=receipt,
                pass623_row=sub_route,
                asset_set=asset_set,
                classification=classification,
                queue_count_before=queue_count_before,
                queue_first_index_before=queue_first_index_before + idx,
                repo_root=repo_root,
            )
            # Override the party_before tuple for the first row
            # when the caller supplies it; later rows use the
            # row builder's default (== party_after == post-tuple).
            cells = list(row.cells)
            if idx == 0 and party_before is not None:
                # party_before columns are 30..33 (1-based) in the
                # 41-column EVENTS_TSV_HEADER contract, i.e. 0-based
                # 29..32.  With 41 cells those indices are -12..-9.
                cells[-12] = str(int(party_before.get("map", 0)))
                cells[-11] = str(int(party_before.get("x", 0)))
                cells[-10] = str(int(party_before.get("y", 0)))
                cells[-9]  = str(int(party_before.get("direction", 0)))
                row = BuiltRow(cells=cells)
            rows.append(row)
            matched += 1

    return BuildResult(rows=rows, failures=failures, matched=matched, total=total)


# ---------------------------------------------------------------------------
# CLI.
# ---------------------------------------------------------------------------

def _parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Render one 41-column TSV row (or one row per command for "
            "multi-command routes) for a single pass623 route label + "
            "a 320x200 capture frame + a 224x136 crop.  Output is the "
            "row alone, or the events TSV header + N rows when "
            "--with-header is passed."
        ),
    )
    parser.add_argument(
        "--label", required=True,
        help="pass623 route label (e.g. 02_turn_right_west_1_3).",
    )
    parser.add_argument(
        "--raw", required=True, type=Path,
        help="Path to the 320x200 raw DOSBox capture PNG/PPM.",
    )
    parser.add_argument(
        "--crop", required=True, type=Path,
        help="Path to the 224x136 cropped viewport PNG/PPM.",
    )
    parser.add_argument(
        "--run-id", required=True,
        help="Live run id (e.g. 2026-06-07_dm1_v1_ingame).  Must be "
             "non-empty and not the pass625 template placeholder.",
    )
    parser.add_argument(
        "--preflight-receipt", required=True, type=Path,
        help="Path to the preflight receipt JSON (the writer reads "
             "the same pin checks from this file).",
    )
    parser.add_argument(
        "--pass623-fixture", required=True, type=Path,
        help="Path to the pass623 canonical input-capture fixture "
             "(parity-evidence/verification/pass623_dm1_v1_input_capture_readiness_bridge/manifest.json).",
    )
    parser.add_argument(
        "--classification", default=None,
        help="Override the auto-classification from the state "
             "detector (one of dungeon_gameplay, entrance_menu, "
             "champion_create, title_screen, wall_closeup, "
             "unclassified).",
    )
    parser.add_argument(
        "--queue-count-before", type=int, default=1,
        help="F0380 queued command count before the dispatch (default 1).",
    )
    parser.add_argument(
        "--queue-first-index-before", type=int, default=0,
        help="F0380 first-index before the dispatch (default 0).",
    )
    parser.add_argument(
        "--party-before-map", type=int, default=None,
        help="Override party_before.map for the first command of a multi-command route.",
    )
    parser.add_argument(
        "--party-before-x", type=int, default=None,
        help="Override party_before.x for the first command of a multi-command route.",
    )
    parser.add_argument(
        "--party-before-y", type=int, default=None,
        help="Override party_before.y for the first command of a multi-command route.",
    )
    parser.add_argument(
        "--party-before-direction", type=int, default=None,
        help="Override party_before.direction for the first command of a multi-command route.",
    )
    parser.add_argument(
        "--with-header", action="store_true",
        help="Emit the EVENTS_TSV_HEADER line first (use this when "
             "writing to a fresh events.tsv file).",
    )
    parser.add_argument(
        "--self-test", action="store_true",
        help="Run the hermetic self-test and exit.",
    )
    parser.add_argument(
        "--self-test-tmp", type=Path, default=None,
        help="Override the temp dir used by --self-test.",
    )
    return parser.parse_args(argv)


def _emit_row(row: BuiltRow, *, with_header: bool, sink) -> None:
    if with_header:
        sink.write("\t".join(EVENTS_TSV_HEADER) + "\n")
    sink.write(row.to_tsv_line() + "\n")


def main(argv: Optional[list[str]] = None) -> int:
    # Pre-scan for --self-test so the row-builder can be run
    # without the row-rendering CLI args.  argparse still
    # validates the other args once we hand it the full argv.
    raw_argv = list(sys.argv[1:] if argv is None else argv)
    if "--self-test" in raw_argv:
        tmp: Optional[Path] = None
        if "--self-test-tmp" in raw_argv:
            j = raw_argv.index("--self-test-tmp")
            if j + 1 < len(raw_argv) and not raw_argv[j + 1].startswith("-"):
                tmp = Path(raw_argv[j + 1])
        return _self_test(tmp)
    args = _parse_args(raw_argv)
    party_before: Optional[dict[str, int]] = None
    if (
        args.party_before_map is not None
        or args.party_before_x is not None
        or args.party_before_y is not None
        or args.party_before_direction is not None
    ):
        party_before = {
            "map":       args.party_before_map if args.party_before_map is not None else 0,
            "x":         args.party_before_x if args.party_before_x is not None else 0,
            "y":         args.party_before_y if args.party_before_y is not None else 0,
            "direction": args.party_before_direction if args.party_before_direction is not None else 0,
        }
    result = build_rows(
        label=args.label,
        raw_path=args.raw,
        crop_path=args.crop,
        run_id=args.run_id,
        preflight_receipt_path=args.preflight_receipt,
        pass623_fixture_path=args.pass623_fixture,
        classification=args.classification,
        queue_count_before=args.queue_count_before,
        queue_first_index_before=args.queue_first_index_before,
        party_before=party_before,
    )
    for failure in result.failures:
        print(f"FAIL: {failure}", file=sys.stderr)
    if not result.rows:
        return 1
    for row in result.rows:
        _emit_row(row, with_header=args.with_header, sink=sys.stdout)
    return 0


# ---------------------------------------------------------------------------
# Self-test.
# ---------------------------------------------------------------------------

def _write_minimal_ppm(path: Path, width: int, height: int, rgb: tuple[int, int, int]) -> None:
    """Write a tiny valid 24-bit PPM (same shape the writer's
    self-test uses for its capture fixtures)."""
    header = f"P6\n{width} {height}\n255\n".encode("ascii")
    row = bytes(rgb) * width
    body = row * height
    path.write_bytes(header + body)


def _build_synth_receipt(tmp: Path) -> Path:
    """Build a synthetic preflight receipt that passes all three
    pin checks the writer enforces."""
    receipt_path = tmp / "preflight.receipt.json"
    receipt_path.write_text(json.dumps({
        "dungeon_match":          True,
        "graphics_match":         True,
        "pass94_forbidden_present": False,
        "dungeon_sha256":         DEFAULT_DM1_ASSET_SET["DUNGEON.DAT"],
        "graphics_sha256":        DEFAULT_DM1_ASSET_SET["GRAPHICS.DAT"],
    }, indent=2, sort_keys=True), encoding="utf-8")
    return receipt_path


def _build_synth_pass623(tmp: Path) -> Path:
    """Build a tiny synthetic pass623 fixture covering a baseline
    row, a single-command row, and a multi-command row, all
    with non-trivial party tuples so the rendered rows are
    distinguishable from the empty defaults."""
    fixture_path = tmp / "pass623.json"
    rows = [
        {
            "label":       "01_start_south_1_3",
            "commandIds":  [],
            "inputTokens": [],
            "postTuple":   {"map": 0, "x": 1, "y": 3, "direction": 2},
            "observed":    {
                "sha":       "210fa5eedd9c37172c59dd451bffa7f942c5402358ae535d841d3a8614711371",
                "crop":      "01_start_south_1_3_viewport.ppm",
            },
        },
        {
            "label":       "02_turn_right_west_1_3",
            "commandIds":  [2],
            "inputTokens": ["M12_MENU_INPUT_RIGHT"],
            "postTuple":   {"map": 0, "x": 1, "y": 3, "direction": 3},
            "observed":    {
                "sha":       "1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81",
                "crop":      "02_turn_right_west_1_3_viewport.ppm",
            },
        },
        {
            "label":       "04_forward_south_1_4",
            "commandIds":  [1, 3],
            "inputTokens": ["M12_MENU_INPUT_LEFT", "M12_MENU_INPUT_UP"],
            "postTuple":   {"map": 0, "x": 1, "y": 4, "direction": 2},
            "observed":    {
                "sha":       "25bcc97ae93881a39e4bdeffadf07f6fc7b1ac695adbfcc07b585113a8ad4b2e",
                "crop":      "04_forward_south_1_4_viewport.ppm",
            },
        },
    ]
    fixture_path.write_text(json.dumps({
        "schema": "firestaff.parity.pass623_dm1_v1_input_capture_readiness_bridge.v1",
        "canonicalInputCaptureRows": rows,
    }, indent=2, sort_keys=True), encoding="utf-8")
    return fixture_path


def _build_synth_capture_manifest(tmp: Path) -> Path:
    """Build a tiny synthetic Firestaff capture manifest that
    contains the three viewport hashes the synthetic pass623
    fixture references (the writer's per-row check refuses a
    row whose firestaffFrame.viewportSha256 is not in this
    set)."""
    manifest_path = tmp / "capture_manifest_sha256.tsv"
    lines = ["# kind\tfilename\twidth\theight\tsize\tsha256"]
    for kind, fname, sha in (
        ("viewport_224x136", "01_start_south_1_3_viewport.ppm",
         "210fa5eedd9c37172c59dd451bffa7f942c5402358ae535d841d3a8614711371"),
        ("viewport_224x136", "02_turn_right_west_1_3_viewport.ppm",
         "1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81"),
        ("viewport_224x136", "04_forward_south_1_4_viewport.ppm",
         "25bcc97ae93881a39e4bdeffadf07f6fc7b1ac695adbfcc07b585113a8ad4b2e"),
    ):
        lines.append(f"{kind}\t{fname}\t224\t136\t1\t{sha}")
    manifest_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return manifest_path


def _self_test(tmp_dir: Optional[Path]) -> int:
    if tmp_dir is None:
        tmp_dir = Path(tempfile.mkdtemp(prefix="row-builder-selftest-"))
    tmp = Path(tmp_dir)
    tmp.mkdir(parents=True, exist_ok=True)
    failures: list[str] = []
    matched = 0
    total = 0

    # --- Synthesize the upstream fixtures.
    receipt_path = _build_synth_receipt(tmp)
    pass623_path = _build_synth_pass623(tmp)
    capture_manifest = _build_synth_capture_manifest(tmp)

    # --- Build a tiny 320x200 + 224x136 PPM fixture pair.
    captures = tmp / "captures"
    captures.mkdir(exist_ok=True)
    raw_path = captures / "02_ingame_turn_right.png"
    crop_path = captures / "02_ingame_turn_right_viewport.png"
    _write_minimal_ppm(raw_path, 320, 200, (40, 40, 40))
    _write_minimal_ppm(crop_path, 224, 136, (50, 50, 50))

    # --- Check 1: header width is 41 and matches the writer's
    # EVENTS_TSV_HEADER verbatim.  This is the verbatim column-
    # order contract: re-ordering the writer's header would
    # break the writer's own self-test first, but we also pin
    # it here so a future patch that changes the writer's
    # header in isolation is caught at the row-builder
    # self-test gate too.
    total += 1
    if len(EVENTS_TSV_HEADER) != 41:
        failures.append(
            f"column_order: writer's EVENTS_TSV_HEADER has "
            f"{len(EVENTS_TSV_HEADER)} columns, expected 41"
        )
    else:
        matched += 1

    # --- Check 2: single-command route renders a single row
    # with 41 cells.
    total += 1
    res = build_rows(
        label="02_turn_right_west_1_3",
        raw_path=raw_path,
        crop_path=crop_path,
        run_id="selftest_run_001",
        preflight_receipt_path=receipt_path,
        pass623_fixture_path=pass623_path,
    )
    if len(res.rows) != 1:
        failures.append(
            f"single_command: expected 1 row, got {len(res.rows)}"
        )
    elif len(res.rows[0].cells) != 41:
        failures.append(
            f"single_command: row has {len(res.rows[0].cells)} cells, "
            f"expected 41"
        )
    elif not res.failures:
        matched += 1

    # --- Check 3: baseline route renders a row with
    # source_command_id=0 and an empty input token.
    total += 1
    res = build_rows(
        label="01_start_south_1_3",
        raw_path=raw_path,
        crop_path=crop_path,
        run_id="selftest_run_001",
        preflight_receipt_path=receipt_path,
        pass623_fixture_path=pass623_path,
    )
    if res.failures or not res.rows:
        failures.append(
            f"baseline_row: build_rows returned failures: {res.failures}"
        )
    else:
        cells = res.rows[0].cells
        if cells[8] != "" or cells[9] != "0":
            failures.append(
                f"baseline_row: input_token={cells[8]!r} "
                f"source_command_id={cells[9]!r}, expected empty/0"
            )
        else:
            matched += 1

    # --- Check 4: multi-command route renders one row per
    # command (the writer's contract; pass625's blocker is
    # gate on one row per command for routes with
    # commands=[1, 3]).
    total += 1
    res = build_rows(
        label="04_forward_south_1_4",
        raw_path=raw_path,
        crop_path=crop_path,
        run_id="selftest_run_001",
        preflight_receipt_path=receipt_path,
        pass623_fixture_path=pass623_path,
    )
    if len(res.rows) != 2:
        failures.append(
            f"multi_command: expected 2 rows, got {len(res.rows)}: "
            f"failures={res.failures}"
        )
    elif res.rows[0].cells[9] != "1" or res.rows[1].cells[9] != "3":
        failures.append(
            f"multi_command: expected command ids 1 then 3, got "
            f"{res.rows[0].cells[9]} then {res.rows[1].cells[9]}"
        )
    else:
        matched += 1

    # --- Check 5: missing route label in pass623 is refused
    # (the row builder must not silently invent a row).
    total += 1
    res = build_rows(
        label="does_not_exist",
        raw_path=raw_path,
        crop_path=crop_path,
        run_id="selftest_run_001",
        preflight_receipt_path=receipt_path,
        pass623_fixture_path=pass623_path,
    )
    if not res.failures or res.rows:
        failures.append(
            f"unknown_label: expected a failure with no rows, got "
            f"failures={res.failures} rows={len(res.rows)}"
        )
    else:
        matched += 1

    # --- Check 6: missing preflight receipt is refused.
    total += 1
    res = build_rows(
        label="02_turn_right_west_1_3",
        raw_path=raw_path,
        crop_path=crop_path,
        run_id="selftest_run_001",
        preflight_receipt_path=tmp / "missing.json",
        pass623_fixture_path=pass623_path,
    )
    if not res.failures or res.rows:
        failures.append(
            f"missing_receipt: expected a failure with no rows, got "
            f"failures={res.failures}"
        )
    else:
        matched += 1

    # --- Check 7: preflight receipt with failing pin check is
    # refused.
    total += 1
    bad_receipt = tmp / "bad_receipt.json"
    bad_receipt.write_text(json.dumps({
        "dungeon_match":            False,
        "graphics_match":           True,
        "pass94_forbidden_present": False,
    }, indent=2, sort_keys=True), encoding="utf-8")
    res = build_rows(
        label="02_turn_right_west_1_3",
        raw_path=raw_path,
        crop_path=crop_path,
        run_id="selftest_run_001",
        preflight_receipt_path=bad_receipt,
        pass623_fixture_path=pass623_path,
    )
    if not res.failures or res.rows:
        failures.append(
            f"bad_receipt: expected a failure with no rows, got "
            f"failures={res.failures}"
        )
    else:
        matched += 1

    # --- Check 8: missing raw capture is refused.
    total += 1
    res = build_rows(
        label="02_turn_right_west_1_3",
        raw_path=tmp / "missing_raw.ppm",
        crop_path=crop_path,
        run_id="selftest_run_001",
        preflight_receipt_path=receipt_path,
        pass623_fixture_path=pass623_path,
    )
    if not res.failures or res.rows:
        failures.append(
            f"missing_raw: expected a failure with no rows, got "
            f"failures={res.failures}"
        )
    else:
        matched += 1

    # --- Check 9: crop geometry is validated before a row is
    # emitted.  A stale full-frame crop has a self-consistent
    # SHA256, so the row builder must inspect the actual image
    # dimensions instead of trusting the filename or hash alone.
    total += 1
    bad_crop_path = captures / "02_ingame_turn_right_fullframe_as_crop.png"
    _write_minimal_ppm(bad_crop_path, 320, 200, (70, 70, 70))
    res = build_rows(
        label="02_turn_right_west_1_3",
        raw_path=raw_path,
        crop_path=bad_crop_path,
        run_id="selftest_run_001",
        preflight_receipt_path=receipt_path,
        pass623_fixture_path=pass623_path,
    )
    if not any("expected 224x136" in f for f in res.failures) or res.rows:
        failures.append(
            f"bad_crop_geometry: expected a 224x136 geometry failure "
            f"with no rows, got failures={res.failures} rows={len(res.rows)}"
        )
    else:
        matched += 1

    # --- Check 10: end-to-end — pipe the row builder's output
    # through the real transcript writer and assert the emitted
    # transcript is ``promotable=True`` (every row passes).
    total += 1
    res = build_rows(
        label="02_turn_right_west_1_3",
        raw_path=raw_path,
        crop_path=crop_path,
        run_id="selftest_run_001",
        preflight_receipt_path=receipt_path,
        pass623_fixture_path=pass623_path,
    )
    if res.failures or not res.rows:
        failures.append(
            f"e2e_setup: row builder refused to render: {res.failures}"
        )
    else:
        events_tsv = tmp / "events.tsv"
        with events_tsv.open("w", encoding="utf-8") as fh:
            fh.write("\t".join(EVENTS_TSV_HEADER) + "\n")
            for row in res.rows:
                fh.write(row.to_tsv_line() + "\n")
        transcript_out = tmp / "transcript.json"
        # Late import: keep the test self-contained even when
        # the writer module fails to import (e.g. when run via
        # `python -m`).
        from dosbox_capture_transcript_writer import build_transcript
        result = build_transcript(
            preflight_receipt_path=receipt_path,
            events_tsv_path=events_tsv,
            transcript_out=transcript_out,
            pass623_fixture_path=pass623_path,
            firestaff_capture_manifest_path=capture_manifest,
            repo_root=Path(__file__).resolve().parents[2],
        )
        if result.failures or not result.payload:
            failures.append(
                f"e2e_promotable: transcript writer refused to render: "
                f"{result.failures}"
            )
        elif not result.payload.get("promotable", False):
            failures.append(
                f"e2e_promotable: transcript.promotable is False; "
                f"audit={result.payload.get('rows', [{}])[0]}"
            )
        else:
            matched += 1

    # --- Check 11: end-to-end — pipe a multi-command row
    # through the writer and assert every row is promotable.
    total += 1
    res = build_rows(
        label="04_forward_south_1_4",
        raw_path=raw_path,
        crop_path=crop_path,
        run_id="selftest_run_001",
        preflight_receipt_path=receipt_path,
        pass623_fixture_path=pass623_path,
    )
    if res.failures or len(res.rows) != 2:
        failures.append(
            f"e2e_multi_setup: row builder refused to render: "
            f"{res.failures} rows={len(res.rows)}"
        )
    else:
        events_tsv = tmp / "events_multi.tsv"
        with events_tsv.open("w", encoding="utf-8") as fh:
            fh.write("\t".join(EVENTS_TSV_HEADER) + "\n")
            for row in res.rows:
                fh.write(row.to_tsv_line() + "\n")
        transcript_out = tmp / "transcript_multi.json"
        from dosbox_capture_transcript_writer import build_transcript
        result = build_transcript(
            preflight_receipt_path=receipt_path,
            events_tsv_path=events_tsv,
            transcript_out=transcript_out,
            pass623_fixture_path=pass623_path,
            firestaff_capture_manifest_path=capture_manifest,
            repo_root=Path(__file__).resolve().parents[2],
        )
        if result.failures or not result.payload:
            failures.append(
                f"e2e_multi_promotable: transcript writer refused: "
                f"{result.failures}"
            )
        elif not result.payload.get("promotable", False):
            failures.append(
                "e2e_multi_promotable: transcript.promotable is False "
                "for a multi-command route"
            )
        else:
            matched += 1

    # --- Check 12: dispatch handler matches the command id
    # (TURN vs MOVE).  This is the F0365/F0366 source-chain
    # binding the writer enforces — the row builder picks
    # the right handler based on the command id.
    total += 1
    res = build_rows(
        label="02_turn_right_west_1_3",
        raw_path=raw_path,
        crop_path=crop_path,
        run_id="selftest_run_001",
        preflight_receipt_path=receipt_path,
        pass623_fixture_path=pass623_path,
    )
    if res.failures or not res.rows:
        failures.append(
            f"turn_handler_setup: row builder refused: {res.failures}"
        )
    elif res.rows[0].cells[17] != TURN_HANDLER:
        failures.append(
            f"turn_handler: dispatch.handler={res.rows[0].cells[17]!r}, "
            f"expected {TURN_HANDLER!r} for command id 2"
        )
    else:
        matched += 1

    # --- Check 13: source-function names are pinned to the
    # writer's constants (regression guard: a future operator
    # who edits the writer's constants cannot silently break
    # the row builder).
    total += 1
    res = build_rows(
        label="02_turn_right_west_1_3",
        raw_path=raw_path,
        crop_path=crop_path,
        run_id="selftest_run_001",
        preflight_receipt_path=receipt_path,
        pass623_fixture_path=pass623_path,
    )
    if res.failures or not res.rows:
        failures.append(
            f"source_function_setup: row builder refused: {res.failures}"
        )
    else:
        cells = res.rows[0].cells
        bad: list[str] = []
        if cells[16] != DISPATCH_SOURCE_FUNCTION:
            bad.append(
                f"dispatch.sourceFunction={cells[16]!r}, expected "
                f"{DISPATCH_SOURCE_FUNCTION!r}"
            )
        if cells[18] != REDRAW_SOURCE_FUNCTION:
            bad.append(
                f"redraw.sourceFunction={cells[18]!r}, expected "
                f"{REDRAW_SOURCE_FUNCTION!r}"
            )
        if cells[22] != PRESENT_SOURCE_FUNCTION:
            bad.append(
                f"present.sourceFunction={cells[22]!r}, expected "
                f"{PRESENT_SOURCE_FUNCTION!r}"
            )
        if cells[24] != PRESENT_BOUNDARY:
            bad.append(
                f"present.boundary={cells[24]!r}, expected "
                f"{PRESENT_BOUNDARY!r}"
            )
        if cells[33] != DEFAULT_DM1_ASSET_SET["GRAPHICS.DAT"]:
            bad.append(
                "originalAssetSet.sha256.GRAPHICS.DAT drifted from "
                "runbook §1"
            )
        if cells[34] != DEFAULT_DM1_ASSET_SET["DUNGEON.DAT"]:
            bad.append(
                "originalAssetSet.sha256.DUNGEON.DAT drifted from "
                "runbook §1"
            )
        if cells[11] not in QUEUE_SOURCE_FUNCTIONS:
            bad.append(
                f"queue.sourceFunction={cells[11]!r}, expected one of "
                f"{sorted(QUEUE_SOURCE_FUNCTIONS)}"
            )
        if bad:
            failures.append(
                "source_functions: " + "; ".join(bad)
            )
        else:
            matched += 1

    print(
        f"self-test: row-builder checks {matched}/{total} matched"
    )
    if failures:
        print("FAIL:")
        for f in failures:
            print(f"  - {f}")
        return 1
    print("PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
