#!/usr/bin/env python3
"""dosbox_capture_transcript_writer.py — render the pass608 / pass625
DM1 V1 original-runtime transcript from a per-capture event TSV.

The DM1 V1 same-viewport capture blocker is reported by
``tools/verify_pass608_dm1_v1_same_viewport_capture_blocker.py`` and
ends in the status
``BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE``.  The
upstream gates already pin every other half of the route:

  * ``docs/parity/tools/dosbox_capture_preflight.py`` writes a
    hardened ``dosbox_capture.conf`` and a JSON receipt whose
    ``dungeonSha256``/``graphicsSha256``/``launch_command``/
    ``render_settings`` are pinned to the runbook §1/§2 contract;
  * ``docs/parity/tools/dosbox_capture_manifest_writer.py`` (pass633)
    turns the preflight receipt + a per-capture classifier output
    TSV into the runbook's Output Manifest Template with live
    SHA256s, the receipt's pinned launch_command, and the
    current Firestaff ``git rev-parse HEAD``;
  * ``parity-evidence/verification/pass623_dm1_v1_input_capture_readiness_bridge/manifest.json``
    pins the canonical (input token, source command id, post-tuple,
    Firestaff viewport hash) tuple for every documented capture row;
  * ``tools/verify_pass625_dm1_v1_original_transcript_row_preflight.py``
    pins the exact 40-field transcript row schema the next live
    attempt must satisfy for the pass622 blocker to move.

What is still missing is the **deterministic handoff code that
turns a live DOSBox session into a transcript.json file the
pass608 verifier will accept as ``runtimeTranscript.ok=True``**.
The pass608 verifier's promote path is:

  1. The transcript JSON has ``rows`` (or ``transcriptRows`` /
     ``frameBindings``) entries.
  2. Every row satisfies the 30-field pass608 contract (the
     pass625 contract is a 40-field superset; emitting a row that
     satisfies pass625 implicitly satisfies pass608).
  3. Every row's ``firestaffFrame.viewportSha256`` is a known
     Firestaff fixture hash (from
     ``verification-screens/capture_manifest_sha256.tsv``).
  4. The row binds ``commandQueue.sourceFunction`` →
     ``dispatch.sourceFunction`` → ``dispatch.handler`` →
     ``redraw.sourceFunction`` → ``present.sourceFunction`` to
     the same map/X/Y/direction tuple, with the F0380 queue
     pop/count delta and a turn/move handler that matches the
     source command id.

This tool is the missing half: it reads a per-capture event TSV
(one row per live capture, in the same shape the
``dosbox_capture_manifest_writer.py`` TSV uses, but with the
input token / command id / party tuple / Firestaff fixture hash
columns added) and renders a transcript.json file that the
pass608 verifier will accept.  It refuses to emit a transcript
when:

  * the preflight receipt's pin checks are not all PASS
    (``dungeon_match``/``graphics_match``/``pass94_forbidden_present``)
    — the upstream contract is violated;
  * an input token has no source command id mapping in the
    pass623 canonical input-capture fixture (operators are
    typing a token the runbook has never documented);
  * a row's command id is 1 or 2 (turn) and ``dispatch.handler``
    is not ``F0365_COMMAND_ProcessTypes1To2_TurnParty`` (the
    source chain is broken);
  * a row's command id is 3, 4, 5, or 6 (move) and the handler
    is not ``F0366_COMMAND_ProcessTypes3To6_MoveParty`` (same);
  * the row's ``partyAfter`` tuple does not match the
    ``redraw.{mapX,mapY,direction}`` tuple (the F0128 redraw
    did not consume the post-dispatch state);
  * the row's ``firestaffFrame.viewportSha256`` is not in the
    canonical Firestaff fixture viewport-hash set (the operator
    is binding to a non-fixture row);
  * a recorded ``originalFrame.rawSha256`` /
    ``originalFrame.cropSha256`` does not match the bytes on
    disk (a stale SHA cannot silently ship, mirroring the
    pass633 writer's pin contract).

The tool is hermetic: it ships a ``--self-test`` that builds
synthetic preflight receipts, classifier outputs, tiny PPM
fixtures, and synthetic pass623 fixture + Firestaff viewport
manifests in a temp dir, runs the writer against them, and
asserts the emitted transcript's structural and binding
invariants without needing real game data.
"""
from __future__ import annotations

import argparse
import datetime
import hashlib
import json
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, Optional


# ---------------------------------------------------------------------------
# Constants — the public contract of the pass608 / pass625 transcript.
# ---------------------------------------------------------------------------

# pass608 field contract (30 fields, blocked-verifier shape).
PASS608_FIELDS: tuple[str, ...] = (
    "runId",
    "label",
    "originalFrame.path",
    "originalFrame.rawSha256",
    "originalFrame.cropSha256",
    "originalFrame.width",
    "originalFrame.height",
    "commandQueue.sourceFunction",
    "commandQueue.command",
    "commandQueue.countBefore",
    "commandQueue.countAfter",
    "commandQueue.firstIndexBefore",
    "commandQueue.firstIndexAfter",
    "dispatch.sourceFunction",
    "dispatch.handler",
    "partyAfter.mapIndex",
    "partyAfter.mapX",
    "partyAfter.mapY",
    "partyAfter.direction",
    "redraw.sourceFunction",
    "redraw.mapX",
    "redraw.mapY",
    "redraw.direction",
    "present.sourceFunction",
    "present.viewportPresented",
    "firestaffFrame.mapIndex",
    "firestaffFrame.mapX",
    "firestaffFrame.mapY",
    "firestaffFrame.direction",
    "firestaffFrame.viewportSha256",
)

# pass625 field contract (40 fields, superset of pass608).
PASS625_FIELDS: tuple[str, ...] = (
    "runId",
    "routeLabel",
    "originalAssetSet.sha256.GRAPHICS.DAT",
    "originalAssetSet.sha256.DUNGEON.DAT",
    "originalFrame.path",
    "originalFrame.rawSha256",
    "originalFrame.cropSha256",
    "originalFrame.width",
    "originalFrame.height",
    "input.source",
    "input.token",
    "input.sourceCommandId",
    "commandQueue.sourceFunction",
    "commandQueue.command",
    "commandQueue.countBefore",
    "commandQueue.countAfter",
    "commandQueue.firstIndexBefore",
    "commandQueue.firstIndexAfter",
    "dispatch.sourceFunction",
    "dispatch.handler",
    "partyBefore.mapIndex",
    "partyBefore.mapX",
    "partyBefore.mapY",
    "partyBefore.direction",
    "partyAfter.mapIndex",
    "partyAfter.mapX",
    "partyAfter.mapY",
    "partyAfter.direction",
    "redraw.sourceFunction",
    "redraw.mapX",
    "redraw.mapY",
    "redraw.direction",
    "present.sourceFunction",
    "present.viewportPresented",
    "present.boundary",
    "firestaffFrame.mapIndex",
    "firestaffFrame.mapX",
    "firestaffFrame.mapY",
    "firestaffFrame.direction",
    "firestaffFrame.viewportSha256",
)

# The union of pass608 and pass625 — what this writer emits.  A row
# satisfying this set satisfies pass608 (the blocker) implicitly
# because pass625's 40 fields are a strict superset of pass608's 30.
UNION_FIELDS: tuple[str, ...] = tuple(dict.fromkeys(PASS625_FIELDS))

# Column order for the per-capture event TSV.  Mirrors the
# ``dosbox_capture_manifest_writer.py`` TSV shape, plus the four
# input/command binding columns and the seven source-chain handler
# columns the runtime transcript needs.  The order is part of the
# public contract — see ``_EVENTS_TSV_HELP`` for documentation.
EVENTS_TSV_HEADER: tuple[str, ...] = (
    # file/label/classification (matches manifest writer TSV)
    "file",
    "label",
    "classification",
    # original frame geometry / hashes (matches manifest writer TSV)
    "raw_sha256",
    "crop_path",
    "crop_sha256",
    "width",
    "height",
    # input / command id binding (pass623 contract)
    "input_token",
    "source_command_id",
    "source_command_name",
    # source chain (F0359 / F0361 / F0380 / F0365 / F0366 / F0128 / F0097)
    "queue_source_function",
    "queue_count_before",
    "queue_count_after",
    "queue_first_index_before",
    "queue_first_index_after",
    "dispatch_source_function",
    "dispatch_handler",
    "redraw_source_function",
    "redraw_map_x",
    "redraw_map_y",
    "redraw_direction",
    "present_source_function",
    "present_viewport_presented",
    "present_boundary",
    # party tuple (pass625 needs before + after; pass608 needs after)
    "party_map_index",
    "party_x",
    "party_y",
    "party_direction",
    "party_before_map_index",
    "party_before_x",
    "party_before_y",
    "party_before_direction",
    # asset-set contract (pass625 needs the runbook §1 SHA256s)
    "original_asset_set_sha_graphics",
    "original_asset_set_sha_dungeon",
    # Firestaff fixture binding
    "firestaff_map_index",
    "firestaff_x",
    "firestaff_y",
    "firestaff_direction",
    "firestaff_viewport_sha256",
    # run id
    "run_id",
)

# Allowed dispatch handler values.  Anything else is a source-chain
# break: the F0380 pop must be followed by the right F0365/F0366
# handler for the command id it consumed.
TURN_HANDLER = "F0365_COMMAND_ProcessTypes1To2_TurnParty"
MOVE_HANDLER = "F0366_COMMAND_ProcessTypes3To6_MoveParty"
VALID_DISPATCH_HANDLERS: frozenset[str] = frozenset({TURN_HANDLER, MOVE_HANDLER})

# TURN command ids (C001=1, C002=2) and MOVE command ids
# (C003=3, C004=4, C005=5, C006=6) per the ReDMCSB
# COMMAND.C F0380_COMMAND_ProcessQueue_CPSC dispatch table.
TURN_COMMAND_IDS: frozenset[int] = frozenset({1, 2})
MOVE_COMMAND_IDS: frozenset[int] = frozenset({3, 4, 5, 6})
VALID_COMMAND_IDS: frozenset[int] = frozenset(TURN_COMMAND_IDS | MOVE_COMMAND_IDS)

# Source-function names that must land in the row.  These are
# the canonical ReDMCSB function names from
# tools/verify_pass625_dm1_v1_original_transcript_row_preflight.py
# and the pass608 source_lock list.
DISPATCH_SOURCE_FUNCTION = "F0380_COMMAND_ProcessQueue_CPSC"
REDRAW_SOURCE_FUNCTION = "F0128_DUNGEONVIEW_Draw_CPSF"
PRESENT_SOURCE_FUNCTION = "F0097_DUNGEONVIEW_DrawViewport"
PRESENT_BOUNDARY = "VIDRV_09_BlitViewPort"
# Mouse / keyboard queue writers that the pass625 SOURCE_LOCKS pin
# as the source of the G0432 queue write.
QUEUE_SOURCE_FUNCTIONS: frozenset[str] = frozenset({
    "F0359_COMMAND_ProcessClick_CPSC",
    "F0361_COMMAND_ProcessKeyPress",
})

# Output schema name.  Downstream verifiers (pass608, pass625)
# read rows under any of the keys ``rows``/``transcriptRows``/
# ``frameBindings``; we put the primary key first and mirror under
# the other two for back-compat.
SCHEMA = "firestaff.dosbox_capture_transcript_writer.v1"

# Runbook §1 canonical SHA256s.  These are the values the
# pass625 SOURCE_LOCKS / the runbook §1 / the pass608
# EXPECTED_ORIGINAL map pin as the DM1 PC 3.4 game-data
# contract; the writer must put them in the
# ``originalAssetSet.sha256.{GRAPHICS,DUNGEON}.DAT`` fields of
# every emitted row.  Tests can override via ``asset_set=``.
DEFAULT_DM1_ASSET_SET: dict[str, str] = {
    "GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "DUNGEON.DAT":  "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
}

# Friendly help string for the events TSV column order, shown in
# the CLI's --help and in the runbook.  Keeping it here next to
# the column tuple so the two cannot drift.
_EVENTS_TSV_HELP = (
    "Per-capture event TSV columns (tab-separated, in order):\n"
    "  file label classification raw_sha256 crop_path crop_sha256\n"
    "  width height\n"
    "  input_token source_command_id source_command_name\n"
    "  queue_source_function queue_count_before queue_count_after\n"
    "  queue_first_index_before queue_first_index_after\n"
    "  dispatch_source_function dispatch_handler\n"
    "  redraw_source_function redraw_map_x redraw_map_y redraw_direction\n"
    "  present_source_function present_viewport_presented present_boundary\n"
    "  party_map_index party_x party_y party_direction\n"
    "  party_before_map_index party_before_x party_before_y party_before_direction\n"
    "  original_asset_set_sha_graphics original_asset_set_sha_dungeon\n"
    "  firestaff_map_index firestaff_x firestaff_y firestaff_direction\n"
    "  firestaff_viewport_sha256\n"
    "  run_id"
)


# ---------------------------------------------------------------------------
# Helpers.
# ---------------------------------------------------------------------------

def _sha256_of_file(path: Path) -> str:
    """Lowercase hex SHA256 of a file's contents."""
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 16), b""):
            h.update(chunk)
    return h.hexdigest()


def _now_iso() -> str:
    return datetime.datetime.now(tz=datetime.timezone.utc).isoformat(timespec="seconds")


def _parse_int(value: str, *, field: str) -> int:
    """Parse an integer, raising a clear error if it is malformed."""
    try:
        return int(value)
    except ValueError as exc:
        raise ValueError(f"{field} must be an integer, got {value!r}: {exc}") from exc


def _parse_bool(value: str, *, field: str) -> bool:
    """Parse a truthy string (true/false/1/0/yes/no)."""
    norm = value.strip().lower()
    if norm in {"true", "1", "yes", "y"}:
        return True
    if norm in {"false", "0", "no", "n"}:
        return False
    raise ValueError(
        f"{field} must be a boolean (true/false/1/0/yes/no), got {value!r}"
    )


# ---------------------------------------------------------------------------
# Event TSV parsing.
# ---------------------------------------------------------------------------

def _parse_events_tsv(path: Path) -> list[dict[str, object]]:
    """Parse a per-capture event TSV into transcript row builders.

    The TSV must have a single header row whose columns are exactly
    ``EVENTS_TSV_HEADER`` (in that order).  The header is checked
    verbatim so a future operator who re-orders a column cannot
    silently emit a transcript whose fields are swapped.  Every
    subsequent non-comment, non-blank line becomes one row.

    The returned dicts hold the parsed column values verbatim; the
    cross-row binding validation (command id → handler, partyAfter
    ↔ redraw, viewport-hash fixture membership) happens in
    :func:`build_transcript`.
    """
    if not path.is_file():
        raise FileNotFoundError(f"events TSV not found: {path}")
    rows: list[dict[str, object]] = []
    saw_header = False
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.rstrip("\n")
        if not line or line.startswith("#"):
            continue
        cells = line.split("\t")
        if not saw_header:
            if tuple(cells) != EVENTS_TSV_HEADER:
                raise ValueError(
                    f"events TSV header does not match the documented column "
                    f"order.  Got {tuple(cells)}, expected {EVENTS_TSV_HEADER}.  "
                    "The column order is part of the public contract; do not "
                    "re-order or rename columns."
                )
            saw_header = True
            continue
        if len(cells) != len(EVENTS_TSV_HEADER):
            raise ValueError(
                f"events TSV row has {len(cells)} cells, expected "
                f"{len(EVENTS_TSV_HEADER)}: {line!r}"
            )
        rows.append({
            "file":                       cells[0],
            "label":                      cells[1],
            "classification":             cells[2],
            "raw_sha256":                 cells[3],
            "crop_path":                  cells[4],
            "crop_sha256":                cells[5],
            "width":                      _parse_int(cells[6],  field="width"),
            "height":                     _parse_int(cells[7],  field="height"),
            "input_token":                cells[8],
            "source_command_id":          _parse_int(cells[9],  field="source_command_id"),
            "source_command_name":        cells[10],
            "queue_source_function":      cells[11],
            "queue_count_before":         _parse_int(cells[12], field="queue_count_before"),
            "queue_count_after":          _parse_int(cells[13], field="queue_count_after"),
            "queue_first_index_before":   _parse_int(cells[14], field="queue_first_index_before"),
            "queue_first_index_after":    _parse_int(cells[15], field="queue_first_index_after"),
            "dispatch_source_function":   cells[16],
            "dispatch_handler":           cells[17],
            "redraw_source_function":     cells[18],
            "redraw_map_x":               _parse_int(cells[19], field="redraw_map_x"),
            "redraw_map_y":               _parse_int(cells[20], field="redraw_map_y"),
            "redraw_direction":           _parse_int(cells[21], field="redraw_direction"),
            "present_source_function":    cells[22],
            "present_viewport_presented": _parse_bool(cells[23], field="present_viewport_presented"),
            "present_boundary":           cells[24],
            "party_map_index":            _parse_int(cells[25], field="party_map_index"),
            "party_x":                    _parse_int(cells[26], field="party_x"),
            "party_y":                    _parse_int(cells[27], field="party_y"),
            "party_direction":            _parse_int(cells[28], field="party_direction"),
            "party_before_map_index":     _parse_int(cells[29], field="party_before_map_index"),
            "party_before_x":             _parse_int(cells[30], field="party_before_x"),
            "party_before_y":             _parse_int(cells[31], field="party_before_y"),
            "party_before_direction":     _parse_int(cells[32], field="party_before_direction"),
            "originalAssetSetShaGraphics":cells[33],
            "originalAssetSetShaDungeon": cells[34],
            "firestaff_map_index":        _parse_int(cells[35], field="firestaff_map_index"),
            "firestaff_x":                _parse_int(cells[36], field="firestaff_x"),
            "firestaff_y":                _parse_int(cells[37], field="firestaff_y"),
            "firestaff_direction":        _parse_int(cells[38], field="firestaff_direction"),
            "firestaff_viewport_sha256":  cells[39],
            "run_id":                     cells[40],
        })
    if not saw_header:
        raise ValueError(f"events TSV had no column header: {path}")
    return rows


# ---------------------------------------------------------------------------
# Fixture loaders.
# ---------------------------------------------------------------------------

def _load_pass623_fixture(path: Path) -> dict[str, dict[str, object]]:
    """Load the pass623 canonical input-capture fixture.

    Returns a mapping from ``inputToken`` to a dict with the keys
    ``commandId`` (the source command id for the token),
    ``commandName`` (Cxxx_COMMAND_xxx), and the expected
    ``postTuple``.  The fixture is the source of truth for
    which (input token → command id) mappings are valid; a row
    that names an input token the fixture has never seen is a
    classifier bug or a route bug, and the writer must refuse
    to emit a transcript where such a row sneaks in.

    For multi-command routes (rows where ``commandIds`` and
    ``inputTokens`` have the same length greater than one, e.g.
    ``04_forward_south_1_4`` with ``commands=[1, 3]`` and
    ``tokens=[M12_MENU_INPUT_LEFT, M12_MENU_INPUT_UP]``) the
    loader pairs the i-th token with the i-th commandId
    positionally, so the F0380 dispatch contract
    (token[i] -> command[i]) is preserved.  When the lengths
    disagree, the loader falls back to the legacy
    ``commandIds[0]`` mapping for every token (a real fixture
    would never do this; the runbook §5c row builder surfaces
    the inconsistency in its self-test).
    """
    if not path.is_file():
        raise FileNotFoundError(f"pass623 fixture not found: {path}")
    payload = json.loads(path.read_text(encoding="utf-8"))
    rows = payload.get("canonicalInputCaptureRows") or []
    by_token: dict[str, dict[str, object]] = {}
    for row in rows:
        command_ids = list(row.get("commandIds") or [])
        input_tokens = list(row.get("inputTokens") or [])
        if command_ids and input_tokens and len(command_ids) == len(input_tokens):
            # Positional pairing for multi-command routes.
            pairings: list[tuple[str, object | None]] = list(
                zip(input_tokens, command_ids)
            )
        else:
            # Legacy single-token mapping (one token, one command,
            # or a multi-token row whose commandIds list is the
            # first id only).  This matches the behaviour the
            # writer shipped with for the single-command rows
            # in the documented pass623 fixture.
            first_id = command_ids[0] if command_ids else None
            pairings = [(token, first_id) for token in input_tokens]
        for token, cmd_id in pairings:
            if token in by_token:
                by_token[token].setdefault("sharedWith", []).append(
                    row.get("label", "<unlabeled>")
                )
                continue
            by_token[token] = {
                "commandId":   cmd_id,
                "commandName": _command_name_from_id(cmd_id),
                "postTuple":   row.get("postTuple"),
                "rowLabel":    row.get("label"),
            }
    return by_token


def _command_name_from_id(command_id: object) -> str:
    """Render the source command name (Cxxx_COMMAND_xxx) from an id.

    Mirrors the COMMAND.C F0359/F0361 / F0380 dispatch table:
        1 -> C001_COMMAND_TURN_LEFT
        2 -> C002_COMMAND_TURN_RIGHT
        3 -> C003_COMMAND_MOVE_FORWARD
        4 -> C004_COMMAND_MOVE_RIGHT
        5 -> C005_COMMAND_MOVE_BACKWARD
        6 -> C006_COMMAND_MOVE_LEFT
    A None or out-of-range id maps to an empty string so the
    transcript never carries a misleading command name.
    """
    if not isinstance(command_id, int):
        return ""
    return {
        1: "C001_COMMAND_TURN_LEFT",
        2: "C002_COMMAND_TURN_RIGHT",
        3: "C003_COMMAND_MOVE_FORWARD",
        4: "C004_COMMAND_MOVE_RIGHT",
        5: "C005_COMMAND_MOVE_BACKWARD",
        6: "C006_COMMAND_MOVE_LEFT",
    }.get(command_id, "")


def _load_firestaff_viewport_hashes(path: Path) -> set[str]:
    """Read the canonical Firestaff fixture viewport-hash set.

    Mirrors the verifier's :func:`read_firestaff_capture_manifest`:
    only rows with ``kind == viewport_224x136`` count as a
    promotable Firestaff fixture hash.  A row whose
    ``firestaffFrame.viewportSha256`` is not in this set cannot
    bind to a known fixture and the writer must refuse to ship
    the transcript.
    """
    if not path.is_file():
        raise FileNotFoundError(f"capture manifest not found: {path}")
    out: set[str] = set()
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("#"):
            continue
        cells = line.split("\t")
        if len(cells) < 6:
            continue
        kind, _filename, _w, _h, _size, digest = cells[:6]
        if kind == "viewport_224x136" and re.fullmatch(r"[0-9a-f]{64}", digest):
            out.add(digest)
    return out


# ---------------------------------------------------------------------------
# Per-row validation.
# ---------------------------------------------------------------------------

@dataclass
class TranscriptResult:
    """Return value of :func:`build_transcript`."""
    payload: dict[str, object]
    matched: int
    total: int
    failures: list[str] = field(default_factory=list)


def _validate_row(
    repo_root: Path,
    row: dict[str, object],
    *,
    asset_set: dict[str, str],
    pass623: dict[str, dict[str, object]],
    firestaff_hashes: set[str],
) -> tuple[int, list[str]]:
    """Validate one parsed event row against the binding contract.

    Returns ``(matched_count, failures)``.  ``matched_count`` is
    1 when every check passes, 0 otherwise.  The check list is
    the same shape the pass608 verifier applies in
    :func:`validate_transcript_row`, plus the pass625
    ``routeLabel``/``input.*``/``originalAssetSet``/``present.boundary``
    extras the writer is contractually required to emit.
    """
    failures: list[str] = []
    matched = 1

    label = str(row.get("label", ""))
    file_text = str(row.get("file", ""))
    if not file_text:
        failures.append("row missing file column")
        return 0, failures
    if not label:
        failures.append(f"{file_text}: missing label")

    # File on disk + SHA256.
    file_path = Path(file_text)
    if not file_path.is_absolute():
        file_path = repo_root / file_path
    if not file_path.is_file():
        failures.append(f"{label or file_text}: original frame not found on disk")
        matched = 0
    else:
        actual_sha = _sha256_of_file(file_path)
        if actual_sha != row.get("raw_sha256"):
            failures.append(
                f"{label}: originalFrame.rawSha256 mismatch - recorded "
                f"{str(row.get('raw_sha256', ''))[:12]}, on disk {actual_sha[:12]}"
            )
            matched = 0

    # Crop on disk + SHA256.
    crop_path = Path(str(row.get("crop_path", "")))
    if not crop_path.is_absolute():
        crop_path = repo_root / crop_path
    if not crop_path.is_file():
        failures.append(f"{label}: original frame crop not found on disk")
        matched = 0
    else:
        actual_crop_sha = _sha256_of_file(crop_path)
        if actual_crop_sha != row.get("crop_sha256"):
            failures.append(
                f"{label}: originalFrame.cropSha256 mismatch - recorded "
                f"{str(row.get('crop_sha256', ''))[:12]}, on disk "
                f"{actual_crop_sha[:12]}"
            )
            matched = 0

    # Width / height sanity (every documented capture is 320x200).
    if (int(row.get("width", -1)), int(row.get("height", -1))) != (320, 200):
        failures.append(
            f"{label}: originalFrame dimensions {row.get('width')}x"
            f"{row.get('height')} are not 320x200"
        )
        matched = 0

    # Input token -> command id binding (pass623 fixture).
    token = str(row.get("input_token", ""))
    if token:
        if token not in pass623:
            failures.append(
                f"{label}: input.token {token!r} is not in the pass623 "
                "canonical input-capture fixture"
            )
            matched = 0
        else:
            expected_cmd = pass623[token].get("commandId")
            if expected_cmd != row.get("source_command_id"):
                failures.append(
                    f"{label}: input.token {token!r} maps to commandId "
                    f"{expected_cmd!r} in pass623, row carries "
                    f"{row.get('source_command_id')!r}"
                )
                matched = 0
    else:
        # A row with no input token is a baseline (e.g. the first
        # capture before any input).  pass608 accepts that, but
        # the row must still carry a valid command id (0 / unset
        # is treated as "no command issued").  We allow
        # source_command_id=0 here.
        if int(row.get("source_command_id", -1)) != 0:
            failures.append(
                f"{label}: row has no input.token but "
                f"source_command_id={row.get('source_command_id')!r} "
                "(baseline rows must have source_command_id=0)"
            )
            matched = 0

    # Command id sanity.
    cmd = int(row.get("source_command_id", -1))
    if cmd not in VALID_COMMAND_IDS and cmd != 0:
        failures.append(
            f"{label}: source_command_id {cmd!r} is outside the ReDMCSB "
            f"F0380 dispatch range {sorted(VALID_COMMAND_IDS)} (or 0 for "
            "baseline)"
        )
        matched = 0

    # Dispatch handler must match the command id (turn vs move).
    handler = str(row.get("dispatch_handler", ""))
    if cmd in TURN_COMMAND_IDS and handler != TURN_HANDLER:
        failures.append(
            f"{label}: command id {cmd} is a TURN command but "
            f"dispatch.handler is {handler!r}, expected {TURN_HANDLER!r}"
        )
        matched = 0
    elif cmd in MOVE_COMMAND_IDS and handler != MOVE_HANDLER:
        failures.append(
            f"{label}: command id {cmd} is a MOVE command but "
            f"dispatch.handler is {handler!r}, expected {MOVE_HANDLER!r}"
        )
        matched = 0
    if handler not in VALID_DISPATCH_HANDLERS and cmd != 0:
        failures.append(
            f"{label}: dispatch.handler {handler!r} is not in the "
            f"documented F0365/F0366 set {sorted(VALID_DISPATCH_HANDLERS)}"
        )
        matched = 0

    # Source-function names must match the ReDMCSB source-locked
    # function names.  The pass625 SOURCE_LOCKS pin these as
    # non-negotiable; a future operator who re-types the wrong
    # function name silently breaks the binding.
    if str(row.get("dispatch_source_function", "")) != DISPATCH_SOURCE_FUNCTION:
        failures.append(
            f"{label}: dispatch.sourceFunction must be "
            f"{DISPATCH_SOURCE_FUNCTION!r}, got "
            f"{row.get('dispatch_source_function')!r}"
        )
        matched = 0
    if str(row.get("redraw_source_function", "")) != REDRAW_SOURCE_FUNCTION:
        failures.append(
            f"{label}: redraw.sourceFunction must be "
            f"{REDRAW_SOURCE_FUNCTION!r}, got "
            f"{row.get('redraw_source_function')!r}"
        )
        matched = 0
    if str(row.get("present_source_function", "")) != PRESENT_SOURCE_FUNCTION:
        failures.append(
            f"{label}: present.sourceFunction must be "
            f"{PRESENT_SOURCE_FUNCTION!r}, got "
            f"{row.get('present_source_function')!r}"
        )
        matched = 0
    if str(row.get("present_boundary", "")) != PRESENT_BOUNDARY:
        failures.append(
            f"{label}: present.boundary must be {PRESENT_BOUNDARY!r}, got "
            f"{row.get('present_boundary')!r}"
        )
        matched = 0
    if not bool(row.get("present_viewport_presented", False)):
        failures.append(
            f"{label}: present.viewportPresented must be true (the "
            "transcript row is only valid after the VIDRV blit)"
        )
        matched = 0
    queue_src = str(row.get("queue_source_function", ""))
    if queue_src not in QUEUE_SOURCE_FUNCTIONS and cmd != 0:
        failures.append(
            f"{label}: commandQueue.sourceFunction must be one of "
            f"{sorted(QUEUE_SOURCE_FUNCTIONS)}, got {queue_src!r}"
        )
        matched = 0

    # F0380 queue decrement: countAfter = countBefore - 1 (the
    # canonical F0380 pop).  Anything else is a source-chain break.
    cb = int(row.get("queue_count_before", 0))
    ca = int(row.get("queue_count_after", 0))
    if cmd != 0 and ca != cb - 1:
        failures.append(
            f"{label}: F0380 queue pop requires countAfter == "
            f"countBefore - 1; got countBefore={cb}, countAfter={ca}"
        )
        matched = 0

    # PartyAfter <-> redraw tuple must match.  The F0128 redraw
    # consumes the post-dispatch (map/X/Y/direction) tuple; if
    # the row's partyAfter does not match the redraw tuple the
    # transcript cannot be promoted.
    if (int(row.get("party_x", -1)) != int(row.get("redraw_map_x", -1)) or
            int(row.get("party_y", -1)) != int(row.get("redraw_map_y", -1)) or
            int(row.get("party_direction", -1)) != int(row.get("redraw_direction", -1))):
        failures.append(
            f"{label}: partyAfter ({row.get('party_x')}, "
            f"{row.get('party_y')}, direction "
            f"{row.get('party_direction')}) does not match the F0128 "
            f"redraw tuple ({row.get('redraw_map_x')}, "
            f"{row.get('redraw_map_y')}, direction "
            f"{row.get('redraw_direction')})"
        )
        matched = 0

    # Firestaff fixture viewport hash must be in the canonical set.
    fs_sha = str(row.get("firestaff_viewport_sha256", ""))
    if not re.fullmatch(r"[0-9a-f]{64}", fs_sha):
        failures.append(
            f"{label}: firestaffFrame.viewportSha256 {fs_sha!r} is not a "
            "64-char hex digest"
        )
        matched = 0
    elif firestaff_hashes and fs_sha not in firestaff_hashes:
        failures.append(
            f"{label}: firestaffFrame.viewportSha256 {fs_sha[:12]} is not "
            "in the canonical Firestaff fixture viewport-hash set"
        )
        matched = 0

    # runId sanity - pass608 only promotes a transcript whose
    # promotable rows share one runId.  Empty / whitespace
    # runIds make the row un-promotable.
    run_id = str(row.get("run_id", "")).strip()
    if not run_id:
        failures.append(f"{label}: run_id is empty")
        matched = 0
    elif run_id == "<original-runtime-run-id>":
        # pass625 ships this literal as the template placeholder;
        # a real transcript must replace it.
        failures.append(
            f"{label}: run_id is the pass625 template placeholder "
            "'<original-runtime-run-id>'; replace it with the live "
            "runId from the capture session"
        )
        matched = 0

    # Asset-set contract: the runbook §1 SHA256s must be carried
    # in the row's originalAssetSet.sha256.{GRAPHICS,DUNGEON}.DAT
    # fields, and the values must match the recorded runbook §1
    # constants.  We pass the asset_set in as a parameter so the
    # self-test can exercise both the matching and the mismatch
    # cases against hermetic fixtures.
    if asset_set.get("GRAPHICS.DAT") != row.get("originalAssetSetShaGraphics"):
        failures.append(
            f"{label}: originalAssetSet.sha256.GRAPHICS.DAT does not "
            "match the runbook §1 expected value"
        )
        matched = 0
    if asset_set.get("DUNGEON.DAT") != row.get("originalAssetSetShaDungeon"):
        failures.append(
            f"{label}: originalAssetSet.sha256.DUNGEON.DAT does not "
            "match the runbook §1 expected value"
        )
        matched = 0

    return matched, failures


def _row_to_transcript(
    row: dict[str, object],
    *,
    asset_set: dict[str, str],
) -> dict[str, object]:
    """Render one parsed event row as a transcript JSON object.

    The output dict's keys are dotted (e.g.
    ``originalFrame.path``) so the pass608 and pass625
    field-contract probes can read them without traversing
    nested dicts.  All 40 union fields are present, even the
    ones the pass608 contract does not require (e.g.
    ``input.source``, ``partyBefore.*``) - a row satisfying
    pass625 implicitly satisfies pass608.
    """
    return {
        "runId":      str(row.get("run_id", "")),
        "label":      str(row.get("label", "")),
        "routeLabel": str(row.get("label", "")),
        "originalAssetSet": {
            "sha256": {
                "GRAPHICS.DAT": str(row.get("originalAssetSetShaGraphics", "")),
                "DUNGEON.DAT":  str(row.get("originalAssetSetShaDungeon", "")),
            },
        },
        "originalFrame": {
            "path":       str(row.get("file", "")),
            "rawSha256":  str(row.get("raw_sha256", "")),
            "cropSha256": str(row.get("crop_sha256", "")),
            "width":      int(row.get("width", 320)),
            "height":     int(row.get("height", 200)),
        },
        "input": {
            "source":            "original PC/I34E",
            "token":             str(row.get("input_token", "")),
            "sourceCommandId":   int(row.get("source_command_id", 0)),
            "sourceCommandName": str(row.get("source_command_name", "")),
        },
        "commandQueue": {
            "sourceFunction":   str(row.get("queue_source_function", "")),
            "command":          int(row.get("source_command_id", 0)),
            "countBefore":      int(row.get("queue_count_before", 0)),
            "countAfter":       int(row.get("queue_count_after", 0)),
            "firstIndexBefore": int(row.get("queue_first_index_before", 0)),
            "firstIndexAfter":  int(row.get("queue_first_index_after", 0)),
        },
        "dispatch": {
            "sourceFunction": str(row.get("dispatch_source_function", "")),
            "handler":        str(row.get("dispatch_handler", "")),
        },
        "partyBefore": {
            "mapIndex":  int(row.get("party_before_map_index", 0)),
            "mapX":      int(row.get("party_before_x", 0)),
            "mapY":      int(row.get("party_before_y", 0)),
            "direction": int(row.get("party_before_direction", 0)),
        },
        "partyAfter": {
            "mapIndex":  int(row.get("party_map_index", 0)),
            "mapX":      int(row.get("party_x", 0)),
            "mapY":      int(row.get("party_y", 0)),
            "direction": int(row.get("party_direction", 0)),
        },
        "redraw": {
            "sourceFunction": str(row.get("redraw_source_function", "")),
            "mapX":           int(row.get("redraw_map_x", 0)),
            "mapY":           int(row.get("redraw_map_y", 0)),
            "direction":      int(row.get("redraw_direction", 0)),
        },
        "present": {
            "sourceFunction":    str(row.get("present_source_function", "")),
            "viewportPresented": bool(row.get("present_viewport_presented", False)),
            "boundary":          str(row.get("present_boundary", "")),
        },
        "firestaffFrame": {
            "mapIndex":       int(row.get("firestaff_map_index", 0)),
            "mapX":           int(row.get("firestaff_x", 0)),
            "mapY":           int(row.get("firestaff_y", 0)),
            "direction":      int(row.get("firestaff_direction", 0)),
            "viewportSha256": str(row.get("firestaff_viewport_sha256", "")),
        },
    }


# ---------------------------------------------------------------------------
# Manifest / transcript builder.
# ---------------------------------------------------------------------------

def build_transcript(
    preflight_receipt_path: Path,
    events_tsv_path: Path,
    transcript_out: Path,
    *,
    pass623_fixture_path: Path,
    firestaff_capture_manifest_path: Path,
    asset_set: Optional[dict[str, str]] = None,
    repo_root: Optional[Path] = None,
) -> TranscriptResult:
    """Render the pass608 / pass625 transcript JSON from inputs.

    The shape of the returned payload is what the pass608 verifier
    reads as ``runtimeTranscript``: a JSON object with a ``rows``
    key (mirrored under ``transcriptRows`` and ``frameBindings``
    for back-compat), a ``schema`` key naming the writer version,
    and a small set of metadata fields the runbook §5c documents.
    """
    failures: list[str] = []
    matched = 0
    total = 0

    if repo_root is None:
        repo_root = Path(__file__).resolve().parents[2]
    if asset_set is None:
        asset_set = DEFAULT_DM1_ASSET_SET

    # --- Receipt pin checks (same contract as the manifest writer).
    if not preflight_receipt_path.is_file():
        return TranscriptResult(
            payload={},
            matched=0, total=0,
            failures=[f"preflight receipt not found: {preflight_receipt_path}"],
        )
    try:
        receipt = json.loads(preflight_receipt_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        return TranscriptResult(
            payload={},
            matched=0, total=0,
            failures=[f"preflight receipt is not valid JSON: {exc}"],
        )
    if not isinstance(receipt, dict):
        return TranscriptResult(
            payload={},
            matched=0, total=0,
            failures=["preflight receipt is not a JSON object"],
        )

    total += 3
    if not receipt.get("dungeon_match", False):
        failures.append("preflight receipt: DUNGEON.DAT SHA256 did not match")
    else:
        matched += 1
    if not receipt.get("graphics_match", False):
        failures.append("preflight receipt: GRAPHICS.DAT SHA256 did not match")
    else:
        matched += 1
    if receipt.get("pass94_forbidden_present", False):
        failures.append(
            "preflight receipt: pass94 failure-mode settings were still "
            "present; re-render the conf via the preflight before "
            "writing the transcript"
        )
    else:
        matched += 1

    # --- Pass623 + Firestaff fixture loaders.
    try:
        pass623 = _load_pass623_fixture(pass623_fixture_path)
    except (FileNotFoundError, json.JSONDecodeError) as exc:
        return TranscriptResult(
            payload={},
            matched=matched, total=total,
            failures=[f"pass623 fixture: {exc}"],
        )
    try:
        firestaff_hashes = _load_firestaff_viewport_hashes(
            firestaff_capture_manifest_path
        )
    except FileNotFoundError as exc:
        return TranscriptResult(
            payload={},
            matched=matched, total=total,
            failures=[f"firestaff capture manifest: {exc}"],
        )

    # --- Events TSV.
    try:
        events = _parse_events_tsv(events_tsv_path)
    except (FileNotFoundError, ValueError) as exc:
        return TranscriptResult(
            payload={"preflightReceiptPath": str(preflight_receipt_path)},
            matched=matched, total=total,
            failures=[f"events TSV: {exc}"],
        )

    # --- Per-row validation + rendering.
    rows: list[dict[str, object]] = []
    for event in events:
        total += 1
        # The events TSV carries its own originalAssetSetSha*
        # columns (the operator records the runbook §1 SHA256s
        # they actually captured from).  The writer checks those
        # values against the ``asset_set`` argument (which
        # defaults to the runbook §1 constants).  We do NOT
        # auto-inject the asset_set into the row; that would
        # turn the check into a tautology.  The redraw tuple
        # (redraw_map_x / redraw_map_y / redraw_direction) is
        # already explicit in the events TSV (the F0128 redraw
        # consumes the post-dispatch state and is its own
        # observable, separate from the dispatcher's stored
        # party tuple), so no derivation is needed here.
        enriched = dict(event)

        m, fails = _validate_row(
            repo_root, enriched,
            asset_set=asset_set,
            pass623=pass623,
            firestaff_hashes=firestaff_hashes,
        )
        if m:
            matched += 1
        # Always render the row so a downstream investigator can
        # see which rows failed which checks.  The transcript's
        # top-level ``promotable`` flag tells the consumer
        # whether every row passed.
        failures.extend(fails)
        rows.append(_row_to_transcript(enriched, asset_set=asset_set))

    payload = {
        "schema":      SCHEMA,
        "preflightReceiptPath": str(preflight_receipt_path),
        "pass623FixturePath":   str(pass623_fixture_path),
        "firestaffCaptureManifestPath": str(firestaff_capture_manifest_path),
        "eventsTsvPath":        str(events_tsv_path),
        "manifestPath":         str(transcript_out),
        "issuedAtIso":          _now_iso(),
        "rowCount":             len(rows),
        "rows":                 rows,
        # Back-compat: pass608 reads rows under any of these keys;
        # mirror the same list so older consumers keep working.
        "transcriptRows":       rows,
        "frameBindings":        rows,
        # Top-level promotability status.  pass608 only promotes
        # when every row passes; pass625 only checks the
        # structural / source-chain invariants and does not
        # require this flag, so it is informational.
        "promotable":           not failures,
    }

    if not failures:
        transcript_out.parent.mkdir(parents=True, exist_ok=True)
        transcript_out.write_text(
            json.dumps(payload, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
    return TranscriptResult(
        payload=payload, matched=matched, total=total, failures=failures,
    )


# ---------------------------------------------------------------------------
# Self-test (hermetic).
# ---------------------------------------------------------------------------

def _write_minimal_ppm(path: Path, width: int, height: int, rgb: tuple[int, int, int]) -> None:
    """Write a tiny valid 24-bit PPM."""
    header = f"P6\n{width} {height}\n255\n".encode("ascii")
    row = bytes(rgb) * width
    body = row * height
    path.write_bytes(header + body)


def _build_synth_pass623_fixture(
    tmp: Path,
    *,
    tokens: Iterable[tuple[str, int, str, dict[str, int]]] = (),
    multi_command_routes: Iterable[
        tuple[
            str,
            list[tuple[str, int, str]],
            dict[str, int],
            str,
        ]
    ] = (),
) -> Path:
    """Build a synthetic pass623 fixture for the self-test.

    ``tokens`` is an iterable of (inputToken, commandId,
    commandName, postTuple) tuples.  ``multi_command_routes``
    is an iterable of (label, [(token, cmdId, cmdName), ...],
    postTuple, observed_sha) tuples; the writer's
    pass623-fixture loader pairs the i-th token with the i-th
    commandId positionally, so a multi-command self-test row
    exercises the F0380 dispatch contract end-to-end.
    """
    rows: list[dict[str, object]] = []
    for token, cmd_id, _cmd_name, post in tokens:
        rows.append({
            "label": f"synth_{token}",
            "claim": "synthetic self-test row",
            "commandIds": [cmd_id],
            "inputTokens": [token],
            "postTuple": post,
            "observed": {
                "sha":       "f" * 64,
                "crop":      "synth.ppm",
                "map":       post.get("map", 0),
                "x":         post.get("x", 0),
                "y":         post.get("y", 0),
                "direction": post.get("direction", 0),
                "label":     f"synth_{token}",
            },
            "ok": True,
            "problems": [],
        })
    for label, pairs, post, observed_sha in multi_command_routes:
        rows.append({
            "label":      label,
            "claim":      "synthetic self-test multi-command row",
            "commandIds": [cmd for (_t, cmd, _n) in pairs],
            "inputTokens": [t for (t, _c, _n) in pairs],
            "postTuple":  post,
            "observed": {
                "sha":       observed_sha,
                "crop":      f"{label}.ppm",
                "map":       post.get("map", 0),
                "x":         post.get("x", 0),
                "y":         post.get("y", 0),
                "direction": post.get("direction", 0),
                "label":     label,
            },
            "ok": True,
            "problems": [],
        })
    fixture_path = tmp / "pass623_synth.json"
    fixture_path.write_text(
        json.dumps({"canonicalInputCaptureRows": rows}, indent=2) + "\n",
        encoding="utf-8",
    )
    return fixture_path


def _build_synth_capture_manifest(
    tmp: Path, *, viewport_sha: str = "f" * 64,
) -> Path:
    """Build a synthetic Firestaff capture manifest for the self-test.

    Writes the manifest in the same TSV shape as
    ``verification-screens/capture_manifest_sha256.tsv``.
    """
    manifest_path = tmp / "capture_manifest_sha256.tsv"
    lines = [
        "# synthetic Firestaff capture manifest for the transcript self-test",
        "# columns: kind<TAB>filename<TAB>width<TAB>height<TAB>bytes<TAB>sha256",
        f"fullframe_320x200\tsynth_full.ppm\t320\t200\t192000\t{viewport_sha}",
        f"viewport_224x136\tsynth_viewport.ppm\t224\t136\t91408\t{viewport_sha}",
    ]
    manifest_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return manifest_path


def _build_synth_receipt(
    tmp: Path,
    *,
    dungeon_match: bool = True,
    graphics_match: bool = True,
    pass94_forbidden: bool = False,
) -> Path:
    """Build a synthetic preflight receipt for the self-test."""
    receipt = {
        "schema": "firestaff.dosbox_capture_preflight.receipt.v1",
        "session_id": "selftest_transcript",
        "issued_at_iso": "2026-06-07T00:00:00+00:00",
        "data_dir": str(tmp / "dm1"),
        "dungeonSha256":  "d" * 64,
        "graphicsSha256": "a" * 64,
        "dungeon_match":  dungeon_match,
        "graphics_match": graphics_match,
        "conf_path": str(tmp / "dosbox_capture.conf"),
        "conf_settings": {
            "machine": "svga_s3", "memsize": "16",
            "core": "dynamic", "cycles": "max",
        },
        "launch_command": "cd DungeonMasterPC34 && DM.EXE",
        "render_settings": {
            "machine": "svga_s3", "memsize": "16",
            "core": "dynamic", "cycles": "max",
            "frameskip": "0", "windowresolution": "1024x768",
            "viewport_resolution": "1024x768", "output": "opengl",
        },
        "firestaff_git_head": "0123456789ab",
        "pass94_forbidden_present": pass94_forbidden,
    }
    path = tmp / "preflight.receipt.json"
    path.write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    return path


def _build_synth_events_tsv(
    captures_dir: Path,
    *,
    raw_filename: str = "01_ingame_start.png",
    crop_filename: str = "01_ingame_start_viewport.png",
    input_token: str = "M12_MENU_INPUT_RIGHT",
    source_command_id: int = 2,
    source_command_name: str = "C002_COMMAND_TURN_RIGHT",
    party_x: int = 1, party_y: int = 3, party_direction: int = 3,
    party_before_x: int = 1, party_before_y: int = 3,
    party_before_direction: int = 2,
    firestaff_x: int = 1, firestaff_y: int = 3,
    firestaff_direction: int = 3,
    firestaff_sha: str = "f" * 64,
    run_id: str = "selftest_run_001",
    queue_count_before: int = 1,
    queue_count_after: int = 0,
    dispatch_handler: str = TURN_HANDLER,
    bad_token: bool = False,
    bad_handler: bool = False,
) -> tuple[Path, dict[str, object]]:
    """Build a synthetic events TSV + tiny PPM fixtures for one row.

    Returns (tsv_path, parsed_row).  Various ``bad_*`` flags let
    the self-test exercise the matching case, the unknown-token
    case, the command/handler-mismatch case from a single helper.
    The party<->redraw drift case is built by a dedicated helper
    (:func:`_build_synth_events_tsv_with_redraw_drift`) so the
    helper here stays simple and the drift case exercises a
    realistic row shape (redraw tuple pinned to a different
    map/X/Y than the partyAfter tuple - the F0128 redraw
    consumed a different state than the dispatch left behind).
    """
    captures_dir.mkdir(parents=True, exist_ok=True)
    raw_path = captures_dir / raw_filename
    crop_path = captures_dir / crop_filename
    _write_minimal_ppm(raw_path, 320, 200, (40, 40, 40))
    _write_minimal_ppm(crop_path, 224, 136, (50, 50, 50))
    raw_sha = _sha256_of_file(raw_path)
    crop_sha = _sha256_of_file(crop_path)

    row = [
        str(raw_path),                                # file
        raw_filename.rsplit(".", 1)[0],               # label
        "dungeon_gameplay",                           # classification
        raw_sha,                                      # raw_sha256
        str(crop_path),                               # crop_path
        crop_sha,                                     # crop_sha256
        "320", "200",                                 # width, height
        "BOGUS_TOKEN" if bad_token else input_token,  # input_token
        str(source_command_id),                       # source_command_id
        source_command_name,                          # source_command_name
        "F0359_COMMAND_ProcessClick_CPSC",            # queue_source_function
        str(queue_count_before),                      # queue_count_before
        str(queue_count_after),                       # queue_count_after
        "0",                                          # queue_first_index_before
        "1",                                          # queue_first_index_after
        DISPATCH_SOURCE_FUNCTION,                     # dispatch_source_function
        MOVE_HANDLER if bad_handler else dispatch_handler,  # dispatch_handler
        REDRAW_SOURCE_FUNCTION,                       # redraw_source_function
        str(party_x),                                 # redraw_map_x
        str(party_y),                                 # redraw_map_y
        str(party_direction),                         # redraw_direction
        PRESENT_SOURCE_FUNCTION,                      # present_source_function
        "true",                                       # present_viewport_presented
        PRESENT_BOUNDARY,                             # present_boundary
        "0",                                          # party_map_index
        str(party_x),                                 # party_x
        str(party_y),                                 # party_y
        str(party_direction),                         # party_direction
        "0",                                          # party_before_map_index
        str(party_before_x),                          # party_before_x
        str(party_before_y),                          # party_before_y
        str(party_before_direction),                  # party_before_direction
        "a" * 64,                                     # original_asset_set_sha_graphics
        "d" * 64,                                     # original_asset_set_sha_dungeon
        "0",                                          # firestaff_map_index
        str(firestaff_x),                             # firestaff_x
        str(firestaff_y),                             # firestaff_y
        str(firestaff_direction),                     # firestaff_direction
        firestaff_sha,                                # firestaff_viewport_sha256
        run_id,                                       # run_id
    ]
    tsv = captures_dir / "events.tsv"
    tsv.write_text(
        "\t".join(EVENTS_TSV_HEADER) + "\n" + "\t".join(row) + "\n",
        encoding="utf-8",
    )
    return tsv, {
        "raw_sha":   raw_sha,
        "crop_sha":  crop_sha,
        "raw_path":  str(raw_path),
        "crop_path": str(crop_path),
    }


def _build_synth_events_tsv_with_redraw_drift(
    captures_dir: Path,
    *,
    raw_filename: str = "01_ingame_start.png",
    crop_filename: str = "01_ingame_start_viewport.png",
    input_token: str = "M12_MENU_INPUT_RIGHT",
    source_command_id: int = 2,
    source_command_name: str = "C002_COMMAND_TURN_RIGHT",
    party_x: int = 1, party_y: int = 3, party_direction: int = 3,
    party_before_x: int = 1, party_before_y: int = 3,
    party_before_direction: int = 2,
    redraw_x: int = 1, redraw_y: int = 4, redraw_direction: int = 3,
    firestaff_x: int = 1, firestaff_y: int = 3,
    firestaff_direction: int = 3,
    firestaff_sha: str = "f" * 64,
    run_id: str = "selftest_run_001",
) -> tuple[Path, dict[str, object]]:
    """Build a synthetic events TSV whose redraw tuple drifts away
    from the partyAfter tuple.  Used by the party_redraw_drift
    self-test case.  Identical to :func:`_build_synth_events_tsv`
    except ``redraw_x``/``redraw_y``/``redraw_direction`` are
    pinned to explicit values (default: y=4 instead of y=3) so
    the F0128 redraw tuple is not the same as the partyAfter
    tuple and the writer's binding check fires.
    """
    captures_dir.mkdir(parents=True, exist_ok=True)
    raw_path = captures_dir / raw_filename
    crop_path = captures_dir / crop_filename
    _write_minimal_ppm(raw_path, 320, 200, (40, 40, 40))
    _write_minimal_ppm(crop_path, 224, 136, (50, 50, 50))
    raw_sha = _sha256_of_file(raw_path)
    crop_sha = _sha256_of_file(crop_path)

    row = [
        str(raw_path),                                # file
        raw_filename.rsplit(".", 1)[0],               # label
        "dungeon_gameplay",                           # classification
        raw_sha,                                      # raw_sha256
        str(crop_path),                               # crop_path
        crop_sha,                                     # crop_sha256
        "320", "200",                                 # width, height
        input_token,                                  # input_token
        str(source_command_id),                       # source_command_id
        source_command_name,                          # source_command_name
        "F0359_COMMAND_ProcessClick_CPSC",            # queue_source_function
        "1", "0", "0", "1",                           # queue count + indices
        DISPATCH_SOURCE_FUNCTION,                     # dispatch_source_function
        TURN_HANDLER,                                 # dispatch_handler
        REDRAW_SOURCE_FUNCTION,                       # redraw_source_function
        str(redraw_x),                                # redraw_map_x (drifted)
        str(redraw_y),                                # redraw_map_y (drifted)
        str(redraw_direction),                        # redraw_direction (drifted)
        PRESENT_SOURCE_FUNCTION,                      # present_source_function
        "true",                                       # present_viewport_presented
        PRESENT_BOUNDARY,                             # present_boundary
        "0",                                          # party_map_index
        str(party_x),                                 # party_x
        str(party_y),                                 # party_y
        str(party_direction),                         # party_direction
        "0",                                          # party_before_map_index
        str(party_before_x),                          # party_before_x
        str(party_before_y),                          # party_before_y
        str(party_before_direction),                  # party_before_direction
        "a" * 64,                                     # original_asset_set_sha_graphics
        "d" * 64,                                     # original_asset_set_sha_dungeon
        "0",                                          # firestaff_map_index
        str(firestaff_x),                             # firestaff_x
        str(firestaff_y),                             # firestaff_y
        str(firestaff_direction),                     # firestaff_direction
        firestaff_sha,                                # firestaff_viewport_sha256
        run_id,                                       # run_id
    ]
    tsv = captures_dir / "events.tsv"
    tsv.write_text(
        "\t".join(EVENTS_TSV_HEADER) + "\n" + "\t".join(row) + "\n",
        encoding="utf-8",
    )
    return tsv, {
        "raw_sha":   raw_sha,
        "crop_sha":  crop_sha,
        "raw_path":  str(raw_path),
        "crop_path": str(crop_path),
    }


def selftest_writer(tmp_root: Path) -> tuple[int, int, list[str]]:
    """Regression self-test for the runtime transcript writer.

    Exercises the matching case, the unknown-input-token case,
    the command/handler-mismatch case, the
    partyAfter<->redraw-drift case, the unknown-fixture-hash
    case, the preflight-pin-violation case, the bad-SHA case,
    the bad-run-id case, and the transcript-structural
    invariants (field set, source-function names, dispatch
    handler names).  Hermetic - no real game data needed.
    """
    failures: list[str] = []
    matched = 0
    total = 0

    if tmp_root.exists():
        import shutil
        shutil.rmtree(tmp_root)
    tmp_root.mkdir(parents=True)

    asset_set = {
        "GRAPHICS.DAT": "a" * 64,
        "DUNGEON.DAT":  "d" * 64,
    }
    asset_set_mismatch = {
        "GRAPHICS.DAT": "0" * 64,
        "DUNGEON.DAT":  "1" * 64,
    }

    # --- 1. Matching case: every binding check passes.
    sandbox = tmp_root / "matching"
    sandbox.mkdir()
    receipt_path = _build_synth_receipt(sandbox)
    pass623 = _build_synth_pass623_fixture(
        sandbox,
        tokens=[
            ("M12_MENU_INPUT_RIGHT", 2, "C002_COMMAND_TURN_RIGHT",
             {"map": 0, "x": 1, "y": 3, "direction": 3}),
        ],
    )
    capture_manifest = _build_synth_capture_manifest(sandbox)
    captures_dir = sandbox / "captures"
    events_tsv, paths = _build_synth_events_tsv(captures_dir)

    transcript_out = sandbox / "transcript.json"
    result = build_transcript(
        receipt_path, events_tsv, transcript_out,
        pass623_fixture_path=pass623,
        firestaff_capture_manifest_path=capture_manifest,
        asset_set=asset_set,
        repo_root=sandbox,
    )
    total += 7
    if not transcript_out.is_file():
        failures.append("matching: transcript.json was not written")
    else:
        matched += 1
    if result.failures:
        failures.append(f"matching: {result.failures}")
    else:
        matched += 1
    payload = json.loads(transcript_out.read_text(encoding="utf-8")) if transcript_out.is_file() else {}
    rows = payload.get("rows") or []
    if len(rows) != 1:
        failures.append(
            f"matching: expected 1 row in transcript, got {len(rows)}"
        )
    else:
        matched += 1
    if rows and rows[0].get("dispatch", {}).get("handler") != TURN_HANDLER:
        failures.append(
            "matching: row dispatch.handler is "
            f"{rows[0].get('dispatch', {}).get('handler')!r}, expected "
            f"{TURN_HANDLER!r}"
        )
    else:
        matched += 1
    if rows and rows[0].get("originalFrame", {}).get("path") != paths["raw_path"]:
        failures.append(
            "matching: row originalFrame.path does not match the synthetic raw frame"
        )
    else:
        matched += 1
    if rows and rows[0].get("originalAssetSet", {}).get("sha256", {}).get("GRAPHICS.DAT") != "a" * 64:
        failures.append(
            "matching: row originalAssetSet.sha256.GRAPHICS.DAT does not "
            "carry the runbook §1 SHA256"
        )
    else:
        matched += 1
    if rows and rows[0].get("input", {}).get("source") != "original PC/I34E":
        failures.append(
            "matching: row input.source must be 'original PC/I34E' per the "
            "pass625 SOURCE_LOCKS contract"
        )
    else:
        matched += 1

    # --- 2. Unknown-input-token case: row uses a token the
    # pass623 fixture has never seen.
    sandbox = tmp_root / "unknown_token"
    sandbox.mkdir()
    receipt_path = _build_synth_receipt(sandbox)
    pass623 = _build_synth_pass623_fixture(
        sandbox,
        tokens=[
            ("M12_MENU_INPUT_RIGHT", 2, "C002_COMMAND_TURN_RIGHT",
             {"map": 0, "x": 1, "y": 3, "direction": 3}),
        ],
    )
    capture_manifest = _build_synth_capture_manifest(sandbox)
    captures_dir = sandbox / "captures"
    events_tsv, _ = _build_synth_events_tsv(
        captures_dir,
        input_token="BOGUS_TOKEN_NOT_IN_PASS623",
    )
    transcript_out = sandbox / "transcript.json"
    result = build_transcript(
        receipt_path, events_tsv, transcript_out,
        pass623_fixture_path=pass623,
        firestaff_capture_manifest_path=capture_manifest,
        asset_set=asset_set, repo_root=sandbox,
    )
    total += 2
    if not any("is not in the pass623" in f for f in result.failures):
        failures.append(
            "unknown_token: expected a pass623-fixture-missing failure, got none"
        )
    else:
        matched += 1
    if transcript_out.is_file():
        failures.append(
            "unknown_token: transcript was written despite an unknown token"
        )
    else:
        matched += 1

    # --- 3. Command/handler-mismatch case: command id is a TURN
    # but the row's dispatch.handler is the MOVE handler.  The
    # source chain is broken.
    sandbox = tmp_root / "handler_mismatch"
    sandbox.mkdir()
    receipt_path = _build_synth_receipt(sandbox)
    pass623 = _build_synth_pass623_fixture(
        sandbox,
        tokens=[
            ("M12_MENU_INPUT_RIGHT", 2, "C002_COMMAND_TURN_RIGHT",
             {"map": 0, "x": 1, "y": 3, "direction": 3}),
        ],
    )
    capture_manifest = _build_synth_capture_manifest(sandbox)
    captures_dir = sandbox / "captures"
    events_tsv, _ = _build_synth_events_tsv(
        captures_dir, bad_handler=True,
    )
    transcript_out = sandbox / "transcript.json"
    result = build_transcript(
        receipt_path, events_tsv, transcript_out,
        pass623_fixture_path=pass623,
        firestaff_capture_manifest_path=capture_manifest,
        asset_set=asset_set, repo_root=sandbox,
    )
    total += 2
    if not any("TURN command" in f for f in result.failures):
        failures.append(
            "handler_mismatch: expected a TURN/handler-mismatch failure, got none"
        )
    else:
        matched += 1
    if transcript_out.is_file():
        failures.append(
            "handler_mismatch: transcript was written despite a TURN/handler mismatch"
        )
    else:
        matched += 1

    # --- 4. partyAfter<->redraw-drift case: the redraw tuple
    # doesn't match the party tuple.
    sandbox = tmp_root / "party_redraw_drift"
    sandbox.mkdir()
    receipt_path = _build_synth_receipt(sandbox)
    pass623 = _build_synth_pass623_fixture(
        sandbox,
        tokens=[
            ("M12_MENU_INPUT_RIGHT", 2, "C002_COMMAND_TURN_RIGHT",
             {"map": 0, "x": 1, "y": 3, "direction": 3}),
        ],
    )
    capture_manifest = _build_synth_capture_manifest(sandbox)
    captures_dir = sandbox / "captures"
    events_tsv, _ = _build_synth_events_tsv_with_redraw_drift(
        captures_dir, redraw_y=4,
    )
    transcript_out = sandbox / "transcript.json"
    result = build_transcript(
        receipt_path, events_tsv, transcript_out,
        pass623_fixture_path=pass623,
        firestaff_capture_manifest_path=capture_manifest,
        asset_set=asset_set, repo_root=sandbox,
    )
    total += 2
    if not any("does not match the F0128" in f for f in result.failures):
        failures.append(
            "party_redraw_drift: expected an F0128 redraw-tuple failure, got none"
        )
    else:
        matched += 1
    if transcript_out.is_file():
        failures.append(
            "party_redraw_drift: transcript was written despite the redraw-tuple drift"
        )
    else:
        matched += 1

    # --- 5. Unknown-fixture-hash case: row binds to a viewport
    # SHA that is not in the canonical Firestaff fixture set.
    sandbox = tmp_root / "unknown_fixture"
    sandbox.mkdir()
    receipt_path = _build_synth_receipt(sandbox)
    pass623 = _build_synth_pass623_fixture(
        sandbox,
        tokens=[
            ("M12_MENU_INPUT_RIGHT", 2, "C002_COMMAND_TURN_RIGHT",
             {"map": 0, "x": 1, "y": 3, "direction": 3}),
        ],
    )
    # Manifest only contains sha "f"*64, not the row's "0"*64.
    capture_manifest = _build_synth_capture_manifest(sandbox)
    captures_dir = sandbox / "captures"
    events_tsv, _ = _build_synth_events_tsv(
        captures_dir, firestaff_sha="0" * 64,
    )
    transcript_out = sandbox / "transcript.json"
    result = build_transcript(
        receipt_path, events_tsv, transcript_out,
        pass623_fixture_path=pass623,
        firestaff_capture_manifest_path=capture_manifest,
        asset_set=asset_set, repo_root=sandbox,
    )
    total += 2
    if not any("not in the canonical Firestaff fixture" in f for f in result.failures):
        failures.append(
            "unknown_fixture: expected a Firestaff-fixture-membership failure, got none"
        )
    else:
        matched += 1
    if transcript_out.is_file():
        failures.append(
            "unknown_fixture: transcript was written despite a non-fixture hash"
        )
    else:
        matched += 1

    # --- 6. Preflight-pin-violation case: receipt reports
    # pass94_forbidden_present=True.  Writer must refuse to ship.
    sandbox = tmp_root / "pin_violation"
    sandbox.mkdir()
    receipt_path = _build_synth_receipt(
        sandbox, pass94_forbidden=True,
    )
    pass623 = _build_synth_pass623_fixture(
        sandbox,
        tokens=[
            ("M12_MENU_INPUT_RIGHT", 2, "C002_COMMAND_TURN_RIGHT",
             {"map": 0, "x": 1, "y": 3, "direction": 3}),
        ],
    )
    capture_manifest = _build_synth_capture_manifest(sandbox)
    captures_dir = sandbox / "captures"
    events_tsv, _ = _build_synth_events_tsv(captures_dir)
    transcript_out = sandbox / "transcript.json"
    result = build_transcript(
        receipt_path, events_tsv, transcript_out,
        pass623_fixture_path=pass623,
        firestaff_capture_manifest_path=capture_manifest,
        asset_set=asset_set, repo_root=sandbox,
    )
    total += 2
    if not any("pass94 failure-mode" in f for f in result.failures):
        failures.append(
            "pin_violation: expected a pass94 failure-mode failure, got none"
        )
    else:
        matched += 1
    if transcript_out.is_file():
        failures.append(
            "pin_violation: transcript was written despite a pin violation"
        )
    else:
        matched += 1

    # --- 7. Asset-set-mismatch case: writer's asset_set does
    # not match what the runbook §1 contract says.
    sandbox = tmp_root / "asset_mismatch"
    sandbox.mkdir()
    receipt_path = _build_synth_receipt(sandbox)
    pass623 = _build_synth_pass623_fixture(
        sandbox,
        tokens=[
            ("M12_MENU_INPUT_RIGHT", 2, "C002_COMMAND_TURN_RIGHT",
             {"map": 0, "x": 1, "y": 3, "direction": 3}),
        ],
    )
    capture_manifest = _build_synth_capture_manifest(sandbox)
    captures_dir = sandbox / "captures"
    events_tsv, _ = _build_synth_events_tsv(captures_dir)
    transcript_out = sandbox / "transcript.json"
    result = build_transcript(
        receipt_path, events_tsv, transcript_out,
        pass623_fixture_path=pass623,
        firestaff_capture_manifest_path=capture_manifest,
        asset_set=asset_set_mismatch, repo_root=sandbox,
    )
    total += 2
    if not any("originalAssetSet.sha256" in f for f in result.failures):
        failures.append(
            "asset_mismatch: expected an originalAssetSet failure, got none"
        )
    else:
        matched += 1
    if transcript_out.is_file():
        failures.append(
            "asset_mismatch: transcript was written despite an asset-set mismatch"
        )
    else:
        matched += 1

    # --- 8. Bad-run-id case: row uses the pass625 template
    # placeholder; the writer must refuse.
    sandbox = tmp_root / "bad_run_id"
    sandbox.mkdir()
    receipt_path = _build_synth_receipt(sandbox)
    pass623 = _build_synth_pass623_fixture(
        sandbox,
        tokens=[
            ("M12_MENU_INPUT_RIGHT", 2, "C002_COMMAND_TURN_RIGHT",
             {"map": 0, "x": 1, "y": 3, "direction": 3}),
        ],
    )
    capture_manifest = _build_synth_capture_manifest(sandbox)
    captures_dir = sandbox / "captures"
    events_tsv, _ = _build_synth_events_tsv(
        captures_dir, run_id="<original-runtime-run-id>",
    )
    transcript_out = sandbox / "transcript.json"
    result = build_transcript(
        receipt_path, events_tsv, transcript_out,
        pass623_fixture_path=pass623,
        firestaff_capture_manifest_path=capture_manifest,
        asset_set=asset_set, repo_root=sandbox,
    )
    total += 2
    if not any("pass625 template placeholder" in f for f in result.failures):
        failures.append(
            "bad_run_id: expected a placeholder-run-id failure, got none"
        )
    else:
        matched += 1
    if transcript_out.is_file():
        failures.append(
            "bad_run_id: transcript was written despite a placeholder run_id"
        )
    else:
        matched += 1

    # --- 9. Transcript-structural invariants: the emitted
    # payload has the schema name, mirrors rows under the three
    # pass608 keys, and renders one row whose 40-field union is
    # the pass625 contract.
    sandbox = tmp_root / "structural"
    sandbox.mkdir()
    receipt_path = _build_synth_receipt(sandbox)
    pass623 = _build_synth_pass623_fixture(
        sandbox,
        tokens=[
            ("M12_MENU_INPUT_RIGHT", 2, "C002_COMMAND_TURN_RIGHT",
             {"map": 0, "x": 1, "y": 3, "direction": 3}),
        ],
    )
    capture_manifest = _build_synth_capture_manifest(sandbox)
    captures_dir = sandbox / "captures"
    events_tsv, _ = _build_synth_events_tsv(captures_dir)
    transcript_out = sandbox / "transcript.json"
    result = build_transcript(
        receipt_path, events_tsv, transcript_out,
        pass623_fixture_path=pass623,
        firestaff_capture_manifest_path=capture_manifest,
        asset_set=asset_set, repo_root=sandbox,
    )
    total += 4
    payload = json.loads(transcript_out.read_text(encoding="utf-8")) if transcript_out.is_file() else {}
    if payload.get("schema") != SCHEMA:
        failures.append(
            f"structural: schema is {payload.get('schema')!r}, expected {SCHEMA!r}"
        )
    else:
        matched += 1
    if payload.get("rows") != payload.get("transcriptRows"):
        failures.append("structural: rows and transcriptRows diverged")
    else:
        matched += 1
    if payload.get("rows") != payload.get("frameBindings"):
        failures.append("structural: rows and frameBindings diverged")
    else:
        matched += 1
    if not payload.get("promotable"):
        failures.append("structural: matching case payload.promotable is False")
    else:
        matched += 1

    # --- 10. Column-order contract: events TSV header must be
    # exactly EVENTS_TSV_HEADER (verbatim).  A future operator
    # who re-orders a column cannot silently emit a transcript
    # whose fields are swapped.
    sandbox = tmp_root / "header_order"
    sandbox.mkdir()
    receipt_path = _build_synth_receipt(sandbox)
    pass623 = _build_synth_pass623_fixture(
        sandbox,
        tokens=[
            ("M12_MENU_INPUT_RIGHT", 2, "C002_COMMAND_TURN_RIGHT",
             {"map": 0, "x": 1, "y": 3, "direction": 3}),
        ],
    )
    capture_manifest = _build_synth_capture_manifest(sandbox)
    captures_dir = sandbox / "captures"
    captures_dir.mkdir(parents=True, exist_ok=True)
    # Write a header that swaps two columns.
    bad_tsv = captures_dir / "bad_header.tsv"
    swapped = list(EVENTS_TSV_HEADER)
    swapped[0], swapped[1] = swapped[1], swapped[0]
    swapped[2], swapped[3] = swapped[3], swapped[2]  # extra
    bad_tsv.write_text(
        "\t".join(swapped) + "\n" + "\t".join(["x"] * len(swapped)) + "\n",
        encoding="utf-8",
    )
    transcript_out = sandbox / "transcript.json"
    result = build_transcript(
        receipt_path, bad_tsv, transcript_out,
        pass623_fixture_path=pass623,
        firestaff_capture_manifest_path=capture_manifest,
        asset_set=asset_set, repo_root=sandbox,
    )
    total += 1
    if not any("events TSV header does not match" in f for f in result.failures):
        failures.append(
            "header_order: expected a header-order failure, got none"
        )
    else:
        matched += 1

    # --- 27. Multi-command route: the writer's pass623 loader
    # pairs inputTokens with commandIds positionally, so a row
    # whose route has commands=[1, 3] / tokens=[LEFT, UP] must
    # resolve LEFT -> 1 and UP -> 3 (and not both to 1, the
    # legacy single-id mapping).  The check below renders a
    # two-row events TSV (one row per command) against a
    # multi-command pass623 fixture and asserts the emitted
    # transcript is fully promotable — if the loader
    # regressed to the legacy single-id mapping, row 2 would
    # fail the inputToken->commandId check.
    sandbox = tmp_root / "multi_command"
    sandbox.mkdir()
    receipt_path = _build_synth_receipt(sandbox)
    multi_observed_sha = "ab" * 32
    pass623 = _build_synth_pass623_fixture(
        sandbox,
        multi_command_routes=[
            (
                "synth_multi_turn_then_forward",
                [
                    ("M12_MENU_INPUT_LEFT", 1, "C001_COMMAND_TURN_LEFT"),
                    ("M12_MENU_INPUT_UP",   3, "C003_COMMAND_MOVE_FORWARD"),
                ],
                {"map": 0, "x": 1, "y": 4, "direction": 1},
                multi_observed_sha,
            ),
        ],
    )
    capture_manifest = _build_synth_capture_manifest(
        sandbox, viewport_sha=multi_observed_sha,
    )
    captures_dir = sandbox / "captures"
    multi_events_a, _ = _build_synth_events_tsv(
        captures_dir,
        input_token="M12_MENU_INPUT_LEFT",
        source_command_id=1,
        source_command_name="C001_COMMAND_TURN_LEFT",
        party_x=1, party_y=4, party_direction=1,
        party_before_x=1, party_before_y=3, party_before_direction=2,
        firestaff_x=1, firestaff_y=4, firestaff_direction=1,
        firestaff_sha=multi_observed_sha,
        run_id="selftest_run_multi",
        queue_count_before=2,
        dispatch_handler=TURN_HANDLER,
    )
    multi_events_b, _ = _build_synth_events_tsv(
        captures_dir,
        raw_filename="synth_multi_turn_then_forward_b.png",
        crop_filename="synth_multi_turn_then_forward_b_viewport.png",
        input_token="M12_MENU_INPUT_UP",
        source_command_id=3,
        source_command_name="C003_COMMAND_MOVE_FORWARD",
        party_x=1, party_y=4, party_direction=1,
        party_before_x=1, party_before_y=3, party_before_direction=1,
        firestaff_x=1, firestaff_y=4, firestaff_direction=1,
        firestaff_sha=multi_observed_sha,
        run_id="selftest_run_multi",
        queue_count_before=1,
        dispatch_handler=MOVE_HANDLER,
    )
    # Concatenate the two single-row events TSVs into one
    # two-row events TSV (the writer reads one row per line
    # after the verbatim column-order header).
    multi_tsv = sandbox / "multi_events.tsv"
    with multi_tsv.open("w", encoding="utf-8") as fh:
        fh.write("\t".join(EVENTS_TSV_HEADER) + "\n")
        for one in (multi_events_a, multi_events_b):
            with one.open("r", encoding="utf-8") as src:
                src.readline()  # drop the header
                fh.write(src.read())
    multi_transcript_out = sandbox / "transcript_multi.json"
    multi_result = build_transcript(
        receipt_path, multi_tsv, multi_transcript_out,
        pass623_fixture_path=pass623,
        firestaff_capture_manifest_path=capture_manifest,
        asset_set=asset_set, repo_root=sandbox,
    )
    total += 3
    if multi_result.failures:
        failures.append(
            f"multi_command: transcript writer refused a multi-command "
            f"route: {multi_result.failures}"
        )
    else:
        matched += 1
    multi_payload = (
        json.loads(multi_transcript_out.read_text(encoding="utf-8"))
        if multi_transcript_out.is_file() else {}
    )
    multi_rows = multi_payload.get("rows") or []
    if len(multi_rows) != 2:
        failures.append(
            f"multi_command: expected 2 rows in transcript, got "
            f"{len(multi_rows)}"
        )
    else:
        matched += 1
    if multi_rows and not multi_payload.get("promotable", False):
        failures.append(
            "multi_command: transcript.promotable is False; the F0380 "
            "dispatch contract for multi-command routes is broken"
        )
    else:
        matched += 1

    return matched, total, failures


# ---------------------------------------------------------------------------
# Driver.
# ---------------------------------------------------------------------------

def main(argv: Optional[list[str]] = None) -> int:
    parser = argparse.ArgumentParser(
        description="Render the DM1 V1 pass608 / pass625 original-runtime "
                    "transcript from a preflight receipt + per-capture event "
                    "TSV + pass623 input-capture fixture + Firestaff capture "
                    "manifest.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=_EVENTS_TSV_HELP,
    )
    parser.add_argument(
        "--preflight-receipt", type=Path, required=False,
        help="Path to the preflight.receipt.json the preflight wrote.",
    )
    parser.add_argument(
        "--events-tsv", type=Path, required=False,
        help="Path to a TSV of per-capture events (see EVENTS TSV column order below).",
    )
    parser.add_argument(
        "--transcript-out", type=Path, required=False,
        help="Where to write the rendered transcript JSON (default: "
                 "<events-tsv-dir>/transcript.json).",
    )
    parser.add_argument(
        "--pass623-fixture", type=Path, required=False,
        help="Path to the pass623 canonical input-capture fixture "
                 "(default: parity-evidence/verification/pass623_dm1_v1_"
                 "input_capture_readiness_bridge/manifest.json).",
    )
    parser.add_argument(
        "--firestaff-capture-manifest", type=Path, required=False,
        help="Path to the Firestaff capture-manifest TSV whose "
                 "viewport_224x136 rows are the canonical fixture hashes "
                 "(default: verification-screens/capture_manifest_sha256.tsv).",
    )
    parser.add_argument(
        "--repo-root", type=Path, default=None,
        help="Firestaff repo root (default: derived from script path).",
    )
    parser.add_argument(
        "--self-test", action="store_true",
        help="run the regression self-test on synthetic fixtures (no real data needed)",
    )
    parser.add_argument(
        "--self-test-tmp", type=Path,
        default=Path("/tmp/dosbox_capture_transcript_writer_selftest"),
        help="sandbox dir for --self-test (default: %(default)s)",
    )
    args = parser.parse_args(argv)

    if args.self_test:
        matched, total, failures = selftest_writer(args.self_test_tmp)
        print(
            f"self-test: transcript-writer checks {matched}/{total} matched"
        )
        if failures:
            print("FAIL:")
            for f in failures:
                print(f"  - {f}")
            return 1
        print("PASS")
        return 0

    # Live mode: every required argument must be present.
    missing = [
        name for name, val in [
            ("--preflight-receipt", args.preflight_receipt),
            ("--events-tsv", args.events_tsv),
            ("--transcript-out", args.transcript_out),
        ] if val is None
    ]
    if missing:
        parser.error(f"missing required arguments: {', '.join(missing)}")

    transcript_out = args.transcript_out
    if transcript_out is None and args.events_tsv is not None:
        transcript_out = args.events_tsv.parent / "transcript.json"

    # Default fixture paths: live mode picks up the canonical
    # pass623 fixture + Firestaff capture manifest from the repo
    # root so the operator only has to wire the per-capture
    # events TSV.  Override via the corresponding CLI flag.
    repo_root = args.repo_root or Path(__file__).resolve().parents[2]
    pass623 = args.pass623_fixture or (
        repo_root
        / "parity-evidence"
        / "verification"
        / "pass623_dm1_v1_input_capture_readiness_bridge"
        / "manifest.json"
    )
    firestaff_manifest = args.firestaff_capture_manifest or (
        repo_root / "verification-screens" / "capture_manifest_sha256.tsv"
    )

    result = build_transcript(
        args.preflight_receipt, args.events_tsv, transcript_out,
        pass623_fixture_path=pass623,
        firestaff_capture_manifest_path=firestaff_manifest,
        repo_root=repo_root,
    )
    print(f"transcript-writer: {result.matched}/{result.total} checks matched")
    if result.failures:
        print("FAIL:")
        for f in result.failures:
            print(f"  - {f}")
        return 1
    print(f"PASS - transcript: {transcript_out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
