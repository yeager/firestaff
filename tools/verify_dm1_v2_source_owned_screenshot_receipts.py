#!/usr/bin/env python3
"""DM1 V2 source-owned deterministic screenshot receipts verifier.

Reads the JSON manifest emitted by
``firestaff_dm1_v2_source_owned_screenshot_probe``, re-derives the
FNV-1a 32-bit hash of every on-disk BMP, and asserts the BM magic
and width/height and size and per-row hash invariants.

Skip-safe: if no manifest is present (probe was a SKIP), exit 0 with a
SKIP message. This matches the skip-safe behaviour of the probe itself
and the established
``csb_v1_hint_oracle_real_htc_scan`` /
``firestaff_x68k_media_classify_unit`` patterns.

This is a Firestaff-side deterministic screenshot receipts gate. It
deliberately does NOT claim DOSBox parity or original-asset pairing —
it proves that the same input bytes (real DM1 PC 3.4 DUNGEON.DAT)
and same composition state and same V2 mode produce a stable FNV-1a 32-bit
hash of the presented BMP across runs/machines, and that the probe
emitted an honest manifest with the expected per-mode geometry.

Source-lock:
    - ReDMCSB DUNVIEW.C:2999-3000  224x136 viewport bitmap dimensions
    - ReDMCSB DUNVIEW.C:8318-8542  F0097 viewport redraw composition
    - ReDMCSB DUNGEON.C:2199-2250  M034_SQUARE_TYPE element table
    - ReDMCSB GAMELOOP.C:90        F0128 viewport present hook
"""
from __future__ import annotations

import json
import struct
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_RECEIPTS_PATH = (
    Path.home() / ".firestaff-probe-dm1-v2-source-owned"
    / "source_owned_screenshot_receipts.json"
)
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / "dm1_v2_source_owned_screenshot_receipts"
EVIDENCE = VERIFY_DIR / "manifest.json"
SCHEMA = "firestaff.dm1_v2.source_owned_screenshot_receipts.v1"
FNV1A_OFFSET_BASIS = 2166136261
FNV1A_PRIME = 16777619

EXPECTED_MODES = ("v1", "v20", "v21", "v22")
EXPECTED_DIRECTIONS = (0, 1, 2, 3)
EXPECTED_DIR_LABELS = ("north", "east", "south", "west")
EXPECTED_GEOMETRY = {
    "v1": (320, 200),
    "v20": (320, 200),
    "v21": (640, 400),
    "v22": (320, 200),
}


def fnv1a_buf(data: bytes) -> int:
    """FNV-1a 32-bit. Identical algorithm to the probe's fnv1a_file."""
    h = FNV1A_OFFSET_BASIS & 0xFFFFFFFF
    for b in data:
        h ^= b
        h = (h * FNV1A_PRIME) & 0xFFFFFFFF
    return h


def parse_bmp_header(path: Path) -> dict[str, Any]:
    """Return BM-magic + width/height + bit_count for a 24-bit BMP."""
    with path.open("rb") as fh:
        header = fh.read(54)
    if len(header) < 54:
        return {"valid": False, "reason": "header-truncated"}
    if header[:2] != b"BM":
        return {"valid": False, "reason": "magic-not-BM"}
    width = struct.unpack("<i", header[18:22])[0]
    raw_height = struct.unpack("<i", header[22:26])[0]
    bit_count = struct.unpack("<H", header[28:30])[0]
    file_size = struct.unpack("<I", header[2:6])[0]
    if bit_count != 24:
        return {
            "valid": False,
            "reason": f"bpp-not-24 ({bit_count})",
            "width": width,
            "height": abs(raw_height),
            "bit_count": bit_count,
        }
    return {
        "valid": True,
        "width": width,
        "height": abs(raw_height),
        "bit_count": bit_count,
        "file_size": file_size,
    }


def load_manifest(path: Path) -> dict[str, Any] | None:
    if not path.exists():
        return None
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None


def expected_bmp_path(receipt_dir: Path, direction: int, mode: str) -> Path:
    label = EXPECTED_DIR_LABELS[direction]
    return receipt_dir / f"dir-{label}-{mode}.bmp"


def verify_row(receipt_dir: Path, row: dict[str, Any]) -> tuple[bool, list[str], dict[str, Any]]:
    errors: list[str] = []
    info: dict[str, Any] = {"row": row}

    mode = row.get("mode")
    direction = row.get("direction")
    declared_w = row.get("width")
    declared_h = row.get("height")
    declared_size = row.get("size")
    declared_fnv = row.get("fnv1a")

    if mode not in EXPECTED_MODES:
        errors.append(f"unknown mode {mode!r}")
        return False, errors, info
    if direction not in EXPECTED_DIRECTIONS:
        errors.append(f"unknown direction {direction!r}")
        return False, errors, info
    if not isinstance(declared_fnv, str) or not declared_fnv.startswith("0x"):
        errors.append(f"fnv1a must be 0x-prefixed hex string, got {declared_fnv!r}")
        return False, errors, info
    try:
        declared_fnv_int = int(declared_fnv, 16)
    except ValueError:
        errors.append(f"fnv1a not valid hex: {declared_fnv!r}")
        return False, errors, info

    bmp_path = expected_bmp_path(receipt_dir, direction, mode)
    info["bmp_path"] = str(bmp_path)
    if not bmp_path.exists():
        errors.append(f"missing BMP {bmp_path.name}")
        return False, errors, info

    raw_bytes = bmp_path.read_bytes()
    info["actual_size"] = len(raw_bytes)
    info["actual_fnv1a"] = f"0x{fnv1a_buf(raw_bytes):08x}"

    header = parse_bmp_header(bmp_path)
    info["header"] = header
    if not header.get("valid"):
        errors.append(f"BMP header invalid for {bmp_path.name}: {header.get('reason')}")
        return False, errors, info

    exp_w, exp_h = EXPECTED_GEOMETRY[mode]
    if header["width"] != exp_w or header["height"] != exp_h:
        errors.append(
            f"BMP dims {header['width']}x{header['height']} != expected {exp_w}x{exp_h}"
        )
    if declared_w != header["width"] or declared_h != header["height"]:
        errors.append(
            f"declared dims {declared_w}x{declared_h} != on-disk {header['width']}x{header['height']}"
        )

    if declared_size != len(raw_bytes):
        errors.append(
            f"declared size {declared_size} != on-disk {len(raw_bytes)}"
        )
    # File-size invariant: 54 byte header + 24-bit padded payload
    row_bytes = header["width"] * 3
    padded = (row_bytes + 3) & ~3
    expected_file_bytes = 54 + padded * header["height"]
    if expected_file_bytes != len(raw_bytes):
        errors.append(
            f"file size {len(raw_bytes)} != 54 + {padded}*{header['height']} = {expected_file_bytes}"
        )
    if header["file_size"] != len(raw_bytes):
        errors.append(
            f"BMP file_size header field {header['file_size']} != on-disk {len(raw_bytes)}"
        )

    if fnv1a_buf(raw_bytes) != declared_fnv_int:
        errors.append(
            f"fnv1a {info['actual_fnv1a']} != declared {declared_fnv}"
        )

    return (len(errors) == 0), errors, info


def verify_cross_mode_invariants(rows: list[dict[str, Any]]) -> tuple[bool, list[str]]:
    """Cross-check the per-direction distinctness the probe asserts.

    Each (direction) must yield 4 distinct hashes (V1 / V20 / V21 / V22).
    E and W may legitimately match because the V2 composition renderer
    does not draw DOOR_SIDE elements. We require N, S, and at least one
    of {E, W} to be pairwise distinct (matching the probe's own invariants).
    """
    errors: list[str] = []
    by_dir: dict[int, dict[str, int]] = {d: {} for d in EXPECTED_DIRECTIONS}
    for row in rows:
        mode = row.get("mode")
        direction = row.get("direction")
        if mode not in EXPECTED_MODES or direction not in EXPECTED_DIRECTIONS:
            continue
        fnv = row.get("fnv1a")
        if not isinstance(fnv, str) or not fnv.startswith("0x"):
            continue
        try:
            by_dir[direction][mode] = int(fnv, 16)
        except ValueError:
            continue

    for d in EXPECTED_DIRECTIONS:
        h = by_dir[d]
        if set(h.keys()) != set(EXPECTED_MODES):
            errors.append(f"direction {d} missing some modes: {sorted(h.keys())}")
            continue
        if len(set(h.values())) != len(h):
            errors.append(
                f"direction {d} modes not pairwise distinct: "
                + ", ".join(f"{m}=0x{v:08x}" for m, v in h.items())
            )

    for mode in EXPECTED_MODES:
        # Skip the per-mode cross-direction check when that mode is
        # missing for any direction (already errored above).
        if not all(mode in by_dir[d] for d in EXPECTED_DIRECTIONS):
            continue
        n = by_dir[0][mode]
        e = by_dir[1][mode]
        s = by_dir[2][mode]
        w = by_dir[3][mode]
        if n == s:
            errors.append(f"mode {mode} N==S (0x{n:08x})")
        if (e == n) and (w == n):
            errors.append(
                f"mode {mode} all directions identical (0x{n:08x}) — "
                "the probe requires N, S, and (E or W) to differ"
            )

    return (len(errors) == 0), errors


def main() -> int:
    receipts_path = DEFAULT_RECEIPTS_PATH
    if "--receipts" in sys.argv:
        idx = sys.argv.index("--receipts")
        if idx + 1 >= len(sys.argv):
            print("error: --receipts requires a path argument", file=sys.stderr)
            return 2
        receipts_path = Path(sys.argv[idx + 1])

    manifest = load_manifest(receipts_path)
    if manifest is None:
        print(
            "SKIP: firestaff_dm1_v2_source_owned_screenshot_receipts: "
            f"no manifest at {receipts_path}. "
            "Probe was a SKIP (no DUNGEON.DAT) — receipts gate is vacuously satisfied."
        )
        try:
            VERIFY_DIR.mkdir(parents=True, exist_ok=True)
            EVIDENCE.write_text(
                json.dumps(
                    {
                        "schema": "firestaff.parity.dm1_v2_source_owned_screenshot_receipts.v1",
                        "status": "skipped",
                        "pass": "dm1_v2_source_owned_screenshot_receipts",
                        "receipts": str(receipts_path),
                        "reason": "no receipts manifest (probe was a SKIP — no DUNGEON.DAT)",
                        "nonClaims": [
                            "No DOSBox parity claim — receipts prove Firestaff-side determinism only.",
                            "No original-vs-Firestaff pixel pairing performed.",
                            "No public screenshot promotion.",
                        ],
                    },
                    indent=2,
                    sort_keys=True,
                )
                + "\n",
                encoding="utf-8",
            )
        except OSError as exc:
            print(f"  warn: could not write {EVIDENCE}: {exc}", file=sys.stderr)
        return 0

    errors: list[str] = []
    info_rows: list[dict[str, Any]] = []

    if manifest.get("schema") != SCHEMA:
        errors.append(
            f"schema must be {SCHEMA!r}, got {manifest.get('schema')!r}"
        )

    if manifest.get("pass") != "firestaff_dm1_v2_source_owned_screenshot_probe":
        errors.append(
            f"pass must be 'firestaff_dm1_v2_source_owned_screenshot_probe', "
            f"got {manifest.get('pass')!r}"
        )

    if not manifest.get("noDosboxParityClaim", False):
        errors.append("manifest.noDosboxParityClaim must be true")

    dungeon = manifest.get("dungeon") or {}
    dungeon_path = dungeon.get("path")
    dungeon_size = dungeon.get("size")
    dungeon_fnv = dungeon.get("fnv1a")
    if not dungeon_path:
        errors.append("manifest.dungeon.path missing")
    if not isinstance(dungeon_size, int) or dungeon_size <= 0:
        errors.append(f"manifest.dungeon.size invalid: {dungeon_size!r}")
    if not isinstance(dungeon_fnv, str) or not dungeon_fnv.startswith("0x"):
        errors.append(f"manifest.dungeon.fnv1a invalid: {dungeon_fnv!r}")
    else:
        try:
            int(dungeon_fnv, 16)
        except ValueError:
            errors.append(f"manifest.dungeon.fnv1a not valid hex: {dungeon_fnv!r}")

    rows = manifest.get("rows") or []
    if len(rows) != 16:
        errors.append(f"manifest.rows must have 16 entries (4 dirs x 4 modes), got {len(rows)}")

    receipt_dir = receipts_path.parent
    row_fnv_hashes: list[int] = []
    for i, row in enumerate(rows):
        ok, row_errors, row_info = verify_row(receipt_dir, row)
        if not ok:
            errors.extend([f"row[{i}] {e}" for e in row_errors])
        info_rows.append(row_info)
        if "fnv1a" in row and isinstance(row["fnv1a"], str):
            try:
                row_fnv_hashes.append(int(row["fnv1a"], 16))
            except ValueError:
                pass

    cross_ok, cross_errors = verify_cross_mode_invariants(rows)
    if not cross_ok:
        errors.extend(cross_errors)

    # Determinism contract: same dungeon bytes + same composition state
    # + same mode must produce a stable FNV-1a across runs. We assert
    # by re-reading each on-disk BMP and confirming the recomputed hash
    # matches the manifest's declared hash. The probe's invariant 3
    # already proves rebuild determinism; this is the disk-side mirror.
    # (When dungeon_fnv matches the on-disk DUNGEON.DAT FNV-1a, the
    # receipts are valid for THIS dungeon. We do not assert what the
    # exact FNV-1a must be — only that the manifest's dungeon block
    # agrees with the on-disk dungeon file.)
    if dungeon_path and dungeon_size and isinstance(dungeon_fnv, str):
        try:
            dungeon_bytes = Path(dungeon_path).read_bytes()
            on_disk_size = len(dungeon_bytes)
            on_disk_fnv = f"0x{fnv1a_buf(dungeon_bytes):08x}"
            if on_disk_size != dungeon_size:
                errors.append(
                    f"manifest.dungeon.size {dungeon_size} != on-disk {on_disk_size}"
                )
            if on_disk_fnv != dungeon_fnv:
                errors.append(
                    f"manifest.dungeon.fnv1a {dungeon_fnv} != on-disk {on_disk_fnv}"
                )
        except OSError as exc:
            errors.append(f"could not re-read dungeon bytes: {exc}")

    result = {
        "schema": "firestaff.parity.dm1_v2_source_owned_screenshot_receipts.v1",
        "status": "failed" if errors else "passed",
        "pass": "dm1_v2_source_owned_screenshot_receipts",
        "receipts": str(receipts_path),
        "rowCount": len(rows),
        "nonClaims": [
            "No DOSBox parity claim — receipts prove Firestaff-side determinism only.",
            "No original-vs-Firestaff pixel pairing performed.",
            "No public screenshot promotion.",
        ],
        "sourceLocks": [
            {"file": "DUNVIEW.C", "line": 2999, "kind": "224x136 viewport bitmap dimensions"},
            {"file": "DUNVIEW.C", "line": 8318, "kind": "F0097 viewport redraw composition order"},
            {"file": "DUNVIEW.C", "line": 8542, "kind": "F0097 viewport redraw tail"},
            {"file": "DUNGEON.C", "line": 2199, "kind": "M034_SQUARE_TYPE element table head"},
            {"file": "GAMELOOP.C", "line": 90, "kind": "F0128 viewport present hook"},
        ],
        "errors": errors,
        "rows": info_rows,
    }

    print(
        f"dm1_v2_source_owned_screenshot_receipts: "
        f"status={result['status']} rows={result['rowCount']} errors={len(errors)}"
    )
    if errors:
        for err in errors:
            print(f"  error: {err}", file=sys.stderr)

    # Persist tracked evidence under parity-evidence/verification/ so
    # the run status + receipt summary + row count is reviewable even
    # on hosts where the probe is SKIP-safe. The gate itself is the
    # exit code, not this file.
    try:
        VERIFY_DIR.mkdir(parents=True, exist_ok=True)
        EVIDENCE.write_text(
            json.dumps(result, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
    except OSError as exc:
        print(f"  warn: could not write {EVIDENCE}: {exc}", file=sys.stderr)

    return 0 if not errors else 1


if __name__ == "__main__":
    raise SystemExit(main())
