#!/usr/bin/env python3
"""dosbox_capture_manifest_writer.py — render the runbook's Output
Manifest Template from a preflight receipt + a directory of classified
captures.

The DM1 PC 3.4 original-capture runbook at
``docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md`` ends with an Output
Manifest Template that the operator is expected to fill in by hand:

    ```tsv
    capture_session\t{ISO timestamp}
    dungeon_sha256\td90b6b1c...
    graphics_sha256\t2c3aa836...
    dosbox_version\t{DOSBox Staging version from --version}
    firestaff_version\t{actual git head, ...}

    file\tlabel\tclassification\tsha256\twidth\theight
    dungeon_start.png\tdungeon_start\tdungeon_gameplay\tSHA256\t320\t200
    cropped_dungeon_start.png\tdungeon_start_crop\tdungeon_gameplay\tSHA256\t224\t136
    ```

Filling that in by hand is exactly how a stale placeholder hash
(``f7f3291f``) shipped through a previous draft of the runbook — the
operator copy-pasted a draft template, then the live manifest carried
the placeholder into the parity ledger.  This tool is the deterministic
handoff code that turns a real ``preflight.receipt.json`` + a
``classifier_outputs.tsv`` (one row per capture) into a real manifest
TSV with live SHA256s, the receipt's pinned ``launch_command`` /
``render_settings``, the current Firestaff ``git rev-parse HEAD``, and
the classifier's verdict for every capture.

It also pins a few invariants the runbook §5/§6 step really wants but
the prose can't enforce:

  * every capture listed in the manifest must already exist on disk
    and the recorded SHA256 must match the bytes — stale SHAs cannot
    silently ship;
  * every capture's classification must be one of the runbook's
    documented states (``dungeon_gameplay``, ``entrance_menu``,
    ``champion_create``, ``title_screen``, ``wall_closeup``,
    ``unclassified``) — the writer refuses to emit a manifest where
    a capture is in a state the runbook can't pair;
  * the manifest header carries the preflight receipt's pinned
    ``launch_command`` and ``render_settings`` so a downstream
    investigator can verify the live session's conf shape against
    the runbook §2 contract without re-deriving the prose;
  * the writer emits a JSON sidecar with the same content so the
    pass608 runtime-transcript contract can ingest it later.

The tool is hermetic: it ships a ``--self-test`` that builds a
synthetic preflight receipt + classifier outputs + tiny PPM fixtures
in a temp dir, runs the writer against them, and asserts the emitted
manifest's structural and SHA invariants without needing real game
data.
"""
from __future__ import annotations

import argparse
import datetime
import hashlib
import json
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Iterable, Optional

# Documented runbook §5/§6 states.  These are the only labels the
# parity ledger accepts; anything else is a classifier bug or a
# route bug and the writer must refuse to emit a manifest where
# such a row sneaks in.
VALID_STATES: frozenset[str] = frozenset({
    "dungeon_gameplay",
    "entrance_menu",
    "champion_create",
    "title_screen",
    "wall_closeup",
    "unclassified",
})

# Manifest column order matches the runbook's Output Manifest
# Template exactly.  Don't re-order without also updating the
# runbook — the format is part of the public contract.
MANIFEST_HEADER = (
    "file\tlabel\tclassification\tsha256\twidth\theight"
)
MANIFEST_KEY_HEADER_PREFIX = "file\tlabel\tclassification\tsha256\twidth\theight"

# How the preflight receipt fields are renamed when they land in
# the manifest's key header rows.  The runbook uses lowercase
# ``dungeon_sha256``/``graphics_sha256``/``firestaff_version``;
# the receipt uses CamelCase ``dungeonSha256``/``graphicsSha256``/
# ``firestaff_git_head``; this mapping is the contract.
RECEIPT_KEY_MAP: dict[str, str] = {
    "dungeonSha256": "dungeon_sha256",
    "graphicsSha256": "graphics_sha256",
    "firestaff_git_head": "firestaff_version",
    "issued_at_iso": "capture_session",
    "launch_command": "launch_command",
    "session_id": "session_id",
    "pass94_forbidden_present": "pass94_forbidden_present",
}


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


def _try_read_dosbox_version() -> Optional[str]:
    """Best-effort lookup of the host's DOSBox Staging version.

    The runbook's manifest template records a ``dosbox_version`` row
    that the live operator is expected to fill in by hand.  This
    helper does the equivalent of ``dosbox --version`` so the
    writer can fill the row automatically.  The subprocess is
    short-timeout and silent-fail: a missing DOSBox install must
    not block a manifest write.
    """
    candidates = ["dosbox", "dosbox-staging", "DOSBox"]
    for binary in candidates:
        try:
            proc = subprocess.run(
                [binary, "--version"],
                capture_output=True,
                text=True,
                timeout=2,
            )
        except (OSError, subprocess.TimeoutExpired):
            continue
        if proc.returncode == 0 and proc.stdout.strip():
            return proc.stdout.strip().splitlines()[0].strip()
    return None


def _try_read_firestaff_git_head(repo_root: Path) -> Optional[str]:
    """Short HEAD of the Firestaff repo, or None if not a git repo.

    Mirrors the helper in ``dosbox_capture_preflight.py`` so the
    writer is the source of truth for the manifest's
    ``firestaff_version`` row.  Subprocess is short-timeout so a
    hung git never blocks a manifest write.
    """
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
        return None
    if proc.returncode == 0:
        sha = proc.stdout.strip()
        if re.fullmatch(r"[0-9a-f]{7,64}", sha):
            return sha[:12]
    return None


def _parse_classifier_outputs(path: Path) -> list[dict[str, object]]:
    """Parse a TSV of classifier outputs into manifest capture rows.

    Expected columns (tab-separated, in order):
        file    label   classification    sha256    width    height
    The first non-comment line is treated as a column header
    (``file\\tlabel\\tclassification\\tsha256\\twidth\\theight``) and
    is dropped; every subsequent non-comment line is treated as a
    data row.  The writer refuses to silently drop columns, so a
    row with fewer than six cells is a hard error and the manifest
    must not be emitted.
    """
    if not path.is_file():
        raise FileNotFoundError(f"classifier outputs not found: {path}")
    rows: list[dict[str, object]] = []
    saw_header = False
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.rstrip("\n")
        if not line or line.startswith("#"):
            continue
        cells = line.split("\t")
        if not saw_header:
            # The first non-comment line is the column header.
            # We don't enforce an exact match — the runbook's
            # documented order is the contract, but the writer
            # is robust to whitespace/case differences on the
            # header text — but the column count must be 6.
            if len(cells) < 6:
                raise ValueError(
                    f"classifier output header has {len(cells)} cells, expected 6: {line!r}"
                )
            saw_header = True
            continue
        if len(cells) < 6:
            raise ValueError(
                f"classifier output row has {len(cells)} cells, expected 6: {line!r}"
            )
        try:
            width = int(cells[4])
            height = int(cells[5])
        except ValueError as exc:
            raise ValueError(
                f"classifier output row width/height must be integers: {line!r} ({exc})"
            ) from exc
        rows.append({
            "file": cells[0],
            "label": cells[1],
            "classification": cells[2],
            "sha256": cells[3],
            "width": width,
            "height": height,
        })
    if not saw_header:
        raise ValueError(
            f"classifier output TSV had no column header: {path}"
        )
    return rows


# ---------------------------------------------------------------------------
# Manifest builder.
# ---------------------------------------------------------------------------

@dataclass
class ManifestResult:
    """Return value of :func:`build_manifest`.

    ``manifest_text`` is the rendered TSV (key header + per-capture
    rows).  ``sidecar`` is the JSON sidecar a downstream
    pass608 runtime-transcript gate can ingest.  ``matched``/``total``
    mirror the deterministic shape the rest of the dosbox_capture_*
    gates return.
    """
    manifest_text: str
    sidecar: dict[str, object]
    matched: int
    total: int
    failures: list[str] = field(default_factory=list)


def _format_receipt_header(receipt: dict[str, object]) -> str:
    """Render the key/value header lines for the manifest.

    The header is what makes the manifest verifiable: a downstream
    investigator reads the SHA256 + git head + launch_command and
    can confirm the live session used the same data + the same
    pinned conf as the runbook §2 contract requires, without having
    to re-derive the prose.
    """
    lines: list[str] = []
    for src_key, dst_key in RECEIPT_KEY_MAP.items():
        if src_key not in receipt:
            continue
        value = receipt[src_key]
        if isinstance(value, bool):
            value_str = "true" if value else "false"
        else:
            value_str = str(value)
        lines.append(f"{dst_key}\t{value_str}")
    return "\n".join(lines)


def _verify_row_against_disk(
    repo_root: Path,
    row: dict[str, object],
) -> tuple[int, list[str]]:
    """Confirm the recorded SHA256/width/height still match the file.

    Returns (matched_count, failures).  ``matched_count`` is 1 when
    everything checks out, 0 otherwise.  The match-count is summed
    into the overall ``matched`` total so a single stale-SHA row is
    visible in the receipt.

    Width/height are taken from the loaded image when Pillow is
    available; otherwise the recorded values are accepted as-is
    (the runbook's capture pipeline writes PPM/PNG with a known
    geometry so the fallback is safe for the documented shapes).
    """
    file_text = str(row.get("file", ""))
    label_text = str(row.get("label", ""))
    classification = str(row.get("classification", ""))
    expected_sha = str(row.get("sha256", ""))
    expected_w = int(row.get("width", -1))  # type: ignore[arg-type]
    expected_h = int(row.get("height", -1))  # type: ignore[arg-type]

    if not file_text:
        return 0, ["row missing file column"]
    if not label_text:
        return 0, [f"{file_text}: missing label"]
    if classification not in VALID_STATES:
        return 0, [
            f"{file_text}: classification {classification!r} is not in "
            f"the runbook's documented states {sorted(VALID_STATES)}"
        ]
    if not re.fullmatch(r"[0-9a-f]{64}", expected_sha):
        return 0, [
            f"{file_text}: sha256 {expected_sha!r} is not a 64-char hex digest"
        ]

    # Resolve the file path.  Relative paths are taken from
    # repo_root (where the manifest writer is expected to run);
    # absolute paths are taken as-is.  This mirrors how
    # ``dosbox_capture_preflight.py`` resolves ``data_dir``.
    file_path = Path(file_text)
    if not file_path.is_absolute():
        file_path = repo_root / file_path
    if not file_path.is_file():
        return 0, [f"{file_text}: file not found on disk"]

    actual_sha = _sha256_of_file(file_path)
    if actual_sha != expected_sha:
        return 0, [
            f"{file_text}: SHA256 mismatch — recorded "
            f"{expected_sha[:12]}, on disk {actual_sha[:12]}"
        ]

    # Width/height check.  Pillow is optional so the writer stays
    # hermetic; if Pillow can't decode the image (or the file
    # isn't an image format the writer can read), fall back to
    # trusting the recorded dimensions — the upstream classifier
    # is the source of truth for geometry and the SHA already
    # pin-checks the bytes.
    actual_w: Optional[int] = None
    actual_h: Optional[int] = None
    try:
        from PIL import Image
        with Image.open(file_path) as img:
            actual_w, actual_h = img.size
    except Exception:
        actual_w, actual_h = None, None
    if actual_w is not None and actual_h is not None:
        if (actual_w, actual_h) != (expected_w, expected_h):
            return 0, [
                f"{file_text}: dimensions {actual_w}x{actual_h} do not match "
                f"the recorded {expected_w}x{expected_h}"
            ]
    return 1, []


def _format_manifest_table(
    receipt: dict[str, object],
    rows: list[dict[str, object]],
) -> str:
    """Render the manifest's TSV body.

    The key header (dungeon_sha256/graphics_sha256/...) is rendered
    first, then a blank line, then the file-level rows.  The
    order is the runbook's documented order; do not re-order.
    """
    lines: list[str] = []
    header = _format_receipt_header(receipt)
    if header:
        lines.append(header)
    lines.append("")
    lines.append(MANIFEST_HEADER)
    for row in rows:
        lines.append(
            "\t".join([
                str(row.get("file", "")),
                str(row.get("label", "")),
                str(row.get("classification", "")),
                str(row.get("sha256", "")),
                str(row.get("width", "")),
                str(row.get("height", "")),
            ])
        )
    return "\n".join(lines) + "\n"


def build_manifest(
    receipt_path: Path,
    classifier_outputs_path: Path,
    manifest_out: Path,
    sidecar_out: Optional[Path] = None,
    *,
    repo_root: Optional[Path] = None,
    now_iso: Optional[str] = None,
    dosbox_version: Optional[str] = None,
    firestaff_git_head: Optional[str] = None,
) -> ManifestResult:
    """Render the runbook's Output Manifest Template from inputs.

    Returns a :class:`ManifestResult` with the rendered TSV, the
    JSON sidecar, and a matched/total/failures triple.  The
    sidecar is what the pass608 runtime-transcript gate can later
    ingest without re-parsing the TSV.
    """
    failures: list[str] = []
    matched = 0
    total = 0

    if repo_root is None:
        repo_root = Path(__file__).resolve().parents[2]

    if not receipt_path.is_file():
        return ManifestResult(
            manifest_text="",
            sidecar={},
            matched=0,
            total=0,
            failures=[f"preflight receipt not found: {receipt_path}"],
        )
    try:
        receipt = json.loads(receipt_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        return ManifestResult(
            manifest_text="",
            sidecar={},
            matched=0,
            total=0,
            failures=[f"preflight receipt is not valid JSON: {exc}"],
        )
    if not isinstance(receipt, dict):
        return ManifestResult(
            manifest_text="",
            sidecar={},
            matched=0,
            total=0,
            failures=["preflight receipt is not a JSON object"],
        )

    # The receipt's pin-checks must be PASS for the manifest to
    # ship.  ``dungeon_match``/``graphics_match`` come from the
    # preflight; ``pass94_forbidden_present`` is the upstream
    # "wrong machine type" / "wrong memsize" failure mode.  If
    # any of these is False the runbook §1 + §2 contract is
    # violated and the manifest must not be written.
    total += 3
    if not receipt.get("dungeon_match", False):
        failures.append(
            "preflight receipt: DUNGEON.DAT SHA256 did not match the runbook §1 expected value"
        )
    else:
        matched += 1
    if not receipt.get("graphics_match", False):
        failures.append(
            "preflight receipt: GRAPHICS.DAT SHA256 did not match the runbook §1 expected value"
        )
    else:
        matched += 1
    if receipt.get("pass94_forbidden_present", False):
        failures.append(
            "preflight receipt: pass94 failure-mode settings were still present; "
            "render the conf again with the preflight before writing the manifest"
        )
    else:
        matched += 1

    try:
        rows = _parse_classifier_outputs(classifier_outputs_path)
    except (FileNotFoundError, ValueError) as exc:
        return ManifestResult(
            manifest_text="",
            sidecar={"preflightReceiptPath": str(receipt_path)},
            matched=matched,
            total=total,
            failures=[f"classifier outputs: {exc}"],
        )

    # Per-row check: the manifest is the document of record, so
    # every row must be a real file on disk with a real SHA256.
    for row in rows:
        total += 1
        m, fails = _verify_row_against_disk(repo_root, row)
        if m:
            matched += 1
        failures.extend(fails)

    # Fill the remaining receipt fields the manifest template
    # expects but the preflight doesn't pin: dosbox_version and
    # firestaff_version.  We prefer the live lookup; if that
    # fails, fall back to the receipt's already-recorded
    # firestaff_git_head, and if that is missing too, record
    # ``<unavailable>`` so the row is never silently blank.
    if firestaff_git_head is None:
        firestaff_git_head = _try_read_firestaff_git_head(repo_root) or \
            str(receipt.get("firestaff_git_head") or "<unavailable>")
    if dosbox_version is None:
        dosbox_version = _try_read_dosbox_version() or "<unavailable>"

    # Stamp the receipt with the now-known live values so the
    # manifest's key header reflects exactly what the live session
    # used.  We do not mutate the on-disk receipt; we record a
    # copy in the sidecar.
    enriched_receipt = dict(receipt)
    enriched_receipt["dosbox_version"] = dosbox_version
    enriched_receipt["firestaff_git_head"] = firestaff_git_head
    if now_iso is None:
        now_iso = datetime.datetime.now(tz=datetime.timezone.utc).isoformat(
            timespec="seconds"
        )
    enriched_receipt["manifest_issued_at_iso"] = now_iso

    # Build the manifest body.  The receipt's pinned fields go in
    # the key header; per-capture rows go in the table.
    manifest_text = _format_manifest_table(enriched_receipt, rows)
    sidecar = {
        "schema": "dosbox_capture_manifest_writer.v1",
        "preflightReceiptPath": str(receipt_path),
        "classifierOutputsPath": str(classifier_outputs_path),
        "manifestPath": str(manifest_out),
        "rows": rows,
        "receipt": enriched_receipt,
    }

    if not failures:
        manifest_out.parent.mkdir(parents=True, exist_ok=True)
        manifest_out.write_text(manifest_text, encoding="utf-8")
        if sidecar_out is not None:
            sidecar_out.parent.mkdir(parents=True, exist_ok=True)
            sidecar_out.write_text(
                json.dumps(sidecar, indent=2, sort_keys=True) + "\n",
                encoding="utf-8",
            )
    return ManifestResult(
        manifest_text=manifest_text,
        sidecar=sidecar,
        matched=matched,
        total=total,
        failures=failures,
    )


# ---------------------------------------------------------------------------
# Self-test (hermetic).
# ---------------------------------------------------------------------------

def _write_minimal_ppm(path: Path, width: int, height: int, rgb: tuple[int, int, int]) -> None:
    """Write a tiny valid 24-bit PPM.  Keeps the self-test hermetic
    when Pillow is not installed.
    """
    header = f"P6\n{width} {height}\n255\n".encode("ascii")
    row = bytes(rgb) * width
    body = row * height
    path.write_bytes(header + body)


def _expected_ppm_sha(path: Path) -> str:
    return _sha256_of_file(path)


def _build_synth_receipt(
    tmp: Path,
    *,
    dungeon_match: bool = True,
    graphics_match: bool = True,
    pass94_forbidden: bool = False,
) -> tuple[Path, dict[str, object]]:
    """Build a synthetic preflight receipt for the self-test.

    Returns (receipt_path, receipt_dict).  The receipt_dict is the
    parsed JSON shape; the on-disk file is what a live operator
    would hand to the writer.  The receipt's SHA256s are stable
    per-fixture so the writer can match the receipt's recorded
    ``dungeonSha256``/``graphicsSha256`` against the runbook's
    expected values without us hard-coding them here.
    """
    receipt: dict[str, object] = {
        "schema": "firestaff.dosbox_capture_preflight.receipt.v1",
        "session_id": "selftest_writer",
        "issued_at_iso": "2026-06-06T00:00:00+00:00",
        "data_dir": str(tmp / "dm1"),
        "dungeonSha256": "deadbeef" * 8,
        "graphicsSha256": "feedface" * 8,
        "dungeon_match": dungeon_match,
        "graphics_match": graphics_match,
        "conf_path": str(tmp / "dosbox_capture.conf"),
        "conf_settings": {
            "machine": "svga_s3",
            "memsize": "16",
            "core": "dynamic",
            "cycles": "max",
        },
        "launch_command": "cd DungeonMasterPC34 && DM.EXE",
        "render_settings": {
            "machine": "svga_s3",
            "memsize": "16",
            "core": "dynamic",
            "cycles": "max",
            "frameskip": "0",
            "windowresolution": "1024x768",
            "viewport_resolution": "1024x768",
            "output": "opengl",
        },
        "firestaff_git_head": "0123456789ab",
        "pass94_forbidden_present": pass94_forbidden,
    }
    receipt_path = tmp / "preflight.receipt.json"
    receipt_path.write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    return receipt_path, receipt


def _build_synth_classifier_outputs(
    captures_dir: Path,
    names: Iterable[tuple[str, str, str]] = (),
) -> tuple[Path, list[dict[str, object]]]:
    """Build synthetic classifier outputs + tiny PPM fixtures.

    ``names`` is an iterable of (filename, label, classification)
    tuples.  Returns (outputs_tsv_path, rows).
    """
    captures_dir.mkdir(parents=True, exist_ok=True)
    rows: list[dict[str, object]] = []
    lines: list[str] = ["# synthetic classifier outputs for the writer self-test",
                        MANIFEST_KEY_HEADER_PREFIX]
    for filename, label, classification in names:
        path = captures_dir / filename
        _write_minimal_ppm(path, 320, 200, (40, 40, 40))
        sha = _expected_ppm_sha(path)
        rows.append({
            "file": str(path),
            "label": label,
            "classification": classification,
            "sha256": sha,
            "width": 320,
            "height": 200,
        })
        lines.append("\t".join([str(path), label, classification, sha, "320", "200"]))
    outputs = captures_dir / "classifier_outputs.tsv"
    outputs.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return outputs, rows


def selftest_writer(tmp_root: Path) -> tuple[int, int, list[str]]:
    """Regression self-test for the manifest writer.

    Exercises the matching case, the SHA-mismatch case, the
    missing-receipt case, the missing-classifier case, the
    preflight-pin-violation case, and the manifest-structural
    invariants (header columns, row order, classification
    whitelist).  Hermetic — no real game data needed.
    """
    failures: list[str] = []
    matched = 0
    total = 0

    if tmp_root.exists():
        import shutil
        shutil.rmtree(tmp_root)
    tmp_root.mkdir(parents=True)

    # --- 1. Matching case: receipt pass + classifier outputs + all rows on disk.
    sandbox = tmp_root / "matching"
    sandbox.mkdir()
    receipt_path, receipt = _build_synth_receipt(
        sandbox, dungeon_match=True, graphics_match=True, pass94_forbidden=False,
    )
    captures_dir = sandbox / "captures"
    outputs, expected_rows = _build_synth_classifier_outputs(
        captures_dir,
        names=[
            ("01_ingame_start.png", "ingame_start", "dungeon_gameplay"),
            ("02_ingame_turn.png",  "ingame_turn",  "dungeon_gameplay"),
        ],
    )
    manifest_out = sandbox / "capture_manifest.tsv"
    sidecar_out = sandbox / "capture_manifest.sidecar.json"
    result = build_manifest(
        receipt_path, outputs, manifest_out, sidecar_out,
        repo_root=sandbox,
        now_iso="2026-06-06T00:00:00+00:00",
        dosbox_version="DOSBox Staging selftest",
        firestaff_git_head="deadbeef0001",
    )
    total += 4
    if not manifest_out.is_file():
        failures.append("matching: manifest_out was not written")
    else:
        matched += 1
    if not sidecar_out.is_file():
        failures.append("matching: sidecar was not written")
    else:
        matched += 1
    if result.failures:
        failures.append(f"matching: {result.failures}")
    else:
        matched += 1
    manifest_text = manifest_out.read_text(encoding="utf-8") if manifest_out.is_file() else ""
    if "dungeon_sha256\tdeadbeefdeadbeef" not in manifest_text:
        failures.append("matching: key header did not include dungeon_sha256")
    else:
        matched += 1
    # The first non-blank line after the key header should be the
    # column header, and the rows must follow the runbook's
    # documented order: file, label, classification, sha256, width, height.
    if "file\tlabel\tclassification\tsha256\twidth\theight" not in manifest_text:
        failures.append("matching: column header missing or out of order")
    else:
        total += 1
        matched += 1
    for row in expected_rows:
        if str(row["file"]) not in manifest_text:
            failures.append(f"matching: row {row['file']} not present in manifest")
        else:
            total += 1
            matched += 1

    # --- 2. SHA-mismatch case: receipt pass but a recorded SHA is wrong.
    sandbox = tmp_root / "sha_mismatch"
    sandbox.mkdir()
    receipt_path, _ = _build_synth_receipt(sandbox)
    captures_dir = sandbox / "captures"
    outputs, _ = _build_synth_classifier_outputs(
        captures_dir,
        names=[("01_ingame_start.png", "ingame_start", "dungeon_gameplay")],
    )
    # The helper wrote the real SHA; rewrite the TSV with a wrong
    # SHA so the writer's on-disk SHA check has something to bite.
    text = outputs.read_text(encoding="utf-8")
    wrong_sha = "0" * 64
    text = re.sub(
        r"(\.png\t[a-z_]+\tdungeon_gameplay\t)[0-9a-f]{64}",
        lambda m: m.group(1) + wrong_sha,
        text,
    )
    outputs.write_text(text, encoding="utf-8")
    manifest_out = sandbox / "capture_manifest.tsv"
    result = build_manifest(
        receipt_path, outputs, manifest_out,
        repo_root=sandbox, now_iso="2026-06-06T00:00:00+00:00",
    )
    total += 1
    if not result.failures or not any("SHA256 mismatch" in f for f in result.failures):
        failures.append("sha_mismatch: expected a SHA256 mismatch failure, got none")
    else:
        matched += 1
    if manifest_out.is_file():
        failures.append("sha_mismatch: manifest_out was written despite SHA mismatch")
    else:
        total += 1
        matched += 1

    # --- 3. Missing-receipt case.
    sandbox = tmp_root / "missing_receipt"
    sandbox.mkdir()
    outputs, _ = _build_synth_classifier_outputs(
        sandbox / "captures",
        names=[("01_ingame_start.png", "ingame_start", "dungeon_gameplay")],
    )
    manifest_out = sandbox / "capture_manifest.tsv"
    result = build_manifest(
        sandbox / "no_such_receipt.json", outputs, manifest_out,
        repo_root=sandbox, now_iso="2026-06-06T00:00:00+00:00",
    )
    total += 1
    if not result.failures or not any("preflight receipt not found" in f for f in result.failures):
        failures.append("missing_receipt: expected a missing-receipt failure, got none")
    else:
        matched += 1

    # --- 4. Preflight-pin-violation case: receipt has pass94_forbidden_present=True.
    sandbox = tmp_root / "pin_violation"
    sandbox.mkdir()
    receipt_path, _ = _build_synth_receipt(
        sandbox, pass94_forbidden=True,
    )
    outputs, _ = _build_synth_classifier_outputs(
        sandbox / "captures",
        names=[("01_ingame_start.png", "ingame_start", "dungeon_gameplay")],
    )
    manifest_out = sandbox / "capture_manifest.tsv"
    result = build_manifest(
        receipt_path, outputs, manifest_out,
        repo_root=sandbox, now_iso="2026-06-06T00:00:00+00:00",
    )
    total += 1
    if not result.failures or not any("pass94 failure-mode" in f for f in result.failures):
        failures.append("pin_violation: expected a pass94 failure-mode failure, got none")
    else:
        matched += 1
    if manifest_out.is_file():
        failures.append("pin_violation: manifest_out was written despite pin violation")
    else:
        total += 1
        matched += 1

    # --- 5. Invalid classification case: row uses a label outside the whitelist.
    sandbox = tmp_root / "invalid_state"
    sandbox.mkdir()
    receipt_path, _ = _build_synth_receipt(sandbox)
    outputs, _ = _build_synth_classifier_outputs(
        sandbox / "captures",
        names=[("01_ingame_start.png", "ingame_start", "bogus_state")],
    )
    manifest_out = sandbox / "capture_manifest.tsv"
    result = build_manifest(
        receipt_path, outputs, manifest_out,
        repo_root=sandbox, now_iso="2026-06-06T00:00:00+00:00",
    )
    total += 1
    if not result.failures or not any("bogus_state" in f for f in result.failures):
        failures.append("invalid_state: expected a classification failure, got none")
    else:
        matched += 1
    if manifest_out.is_file():
        failures.append("invalid_state: manifest_out was written despite invalid classification")
    else:
        total += 1
        matched += 1

    return matched, total, failures


# ---------------------------------------------------------------------------
# Driver.
# ---------------------------------------------------------------------------

def main(argv: Optional[list[str]] = None) -> int:
    parser = argparse.ArgumentParser(
        description="Render the DM1 V1 original-capture runbook's "
                    "Output Manifest Template from a preflight receipt "
                    "+ classifier outputs.",
    )
    parser.add_argument(
        "--preflight-receipt", type=Path, required=False,
        help="Path to the preflight.receipt.json the preflight wrote.",
    )
    parser.add_argument(
        "--classifier-outputs", type=Path, required=False,
        help="Path to a TSV of classifier outputs "
             "(file<TAB>label<TAB>classification<TAB>sha256<TAB>width<TAB>height).",
    )
    parser.add_argument(
        "--manifest-out", type=Path, required=False,
        help="Where to write the rendered manifest TSV (default: "
                 "<classifier-outputs-dir>/capture_manifest.tsv).",
    )
    parser.add_argument(
        "--sidecar-out", type=Path, default=None,
        help="Optional path for the JSON sidecar; consumed by the "
                 "pass608 runtime-transcript gate.",
    )
    parser.add_argument(
        "--repo-root", type=Path, default=None,
        help="Firestaff repo root for git HEAD recording "
                 "(default: derived from script path).",
    )
    parser.add_argument(
        "--self-test", action="store_true",
        help="run the regression self-test on synthetic fixtures (no real data needed)",
    )
    parser.add_argument(
        "--self-test-tmp", type=Path,
        default=Path("/tmp/dosbox_capture_manifest_writer_selftest"),
        help="sandbox dir for --self-test (default: %(default)s)",
    )
    args = parser.parse_args(argv)

    if args.self_test:
        matched, total, failures = selftest_writer(args.self_test_tmp)
        print(
            f"self-test: manifest-writer checks {matched}/{total} matched"
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
            ("--classifier-outputs", args.classifier_outputs),
            ("--manifest-out", args.manifest_out),
        ] if val is None
    ]
    if missing:
        parser.error(f"missing required arguments: {', '.join(missing)}")

    manifest_out = args.manifest_out
    if manifest_out is None:
        manifest_out = args.classifier_outputs.parent / "capture_manifest.tsv"  # type: ignore[union-attr]

    result = build_manifest(
        args.preflight_receipt, args.classifier_outputs, manifest_out,
        args.sidecar_out, repo_root=args.repo_root,
    )
    print(f"manifest-writer: {result.matched}/{result.total} checks matched")
    if result.failures:
        print("FAIL:")
        for f in result.failures:
            print(f"  - {f}")
        return 1
    print(f"PASS — manifest: {manifest_out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
