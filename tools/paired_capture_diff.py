#!/usr/bin/env python3
"""Validate paired runtime captures and emit source-state-bound image diffs.

The tool accepts a JSON manifest that binds each original/reference frame and
Firestaff frame to the same canonical game state. It verifies image hashes,
state fields, and geometry before measuring explicitly named screen regions.
The resulting metrics are evidence only; a nonzero pixel difference is not a
parity verdict.

All input paths are resolved relative to the manifest. Output must be outside
the Git worktree or inside a Git-ignored directory so original game imagery is
not accidentally added to the repository.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Any

from PIL import Image

SCHEMA = "firestaff.parity.paired_capture.v2"
MAX_FRAME_DIMENSION = 8192


class ManifestError(ValueError):
    """Raised when the manifest cannot support a trustworthy comparison."""


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def _required_text(value: Any, label: str) -> str:
    if not isinstance(value, str) or not value.strip():
        raise ManifestError(f"{label} must be a non-empty string")
    return value.strip()


def _safe_filename_component(value: Any, label: str) -> str:
    name = _required_text(value, label)
    if value != name:
        raise ManifestError(f"{label} must not have leading or trailing whitespace")
    if name in {".", ".."} or not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]*", name):
        raise ManifestError(
            f"{label} must be a filename-safe identifier using letters, digits, dot, "
            "underscore, or hyphen"
        )
    return name


def _safe_output_file(out_dir: Path, filename: str) -> Path:
    output_root = out_dir.resolve()
    output_path = (output_root / filename).resolve()
    try:
        output_path.relative_to(output_root)
    except ValueError as exc:
        raise ManifestError(f"output file resolves outside the output directory: {output_path}") from exc
    return output_path


def _resolve_input(base: Path, entry: Any, label: str) -> Path:
    if not isinstance(entry, dict):
        raise ManifestError(f"{label} must be an object with path and sha256")
    raw_path = _required_text(entry.get("path"), f"{label}.path")
    expected_hash = _required_text(entry.get("sha256"), f"{label}.sha256").lower()
    if len(expected_hash) != 64 or any(c not in "0123456789abcdef" for c in expected_hash):
        raise ManifestError(f"{label}.sha256 must be a SHA-256 hex digest")
    path = (base / raw_path).resolve()
    if not path.is_file():
        raise ManifestError(f"{label} file does not exist: {path}")
    actual_hash = sha256_file(path)
    if actual_hash != expected_hash:
        raise ManifestError(
            f"{label} SHA-256 mismatch: expected {expected_hash}, got {actual_hash}"
        )
    return path


def _git_root(path: Path) -> Path | None:
    try:
        result = subprocess.run(
            ["git", "-C", str(path), "rev-parse", "--show-toplevel"],
            check=True,
            capture_output=True,
            text=True,
        )
    except (OSError, subprocess.CalledProcessError):
        return None
    return Path(result.stdout.strip()).resolve()


def _safe_output_dir(path: Path) -> Path:
    output = path.resolve()
    probe = output
    while not probe.exists() and probe != probe.parent:
        probe = probe.parent
    root = _git_root(probe)
    if root is None:
        return output
    try:
        relative = output.relative_to(root)
    except ValueError:
        return output
    ignored = subprocess.run(
        ["git", "-C", str(root), "check-ignore", "-q", "--no-index", str(relative)],
        check=False,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    ).returncode == 0
    if not ignored:
        raise ManifestError(
            f"output directory is inside the Git worktree and is not ignored: {output}"
        )
    return output


def _state_value(state: dict[str, Any], key: str) -> Any:
    value: Any = state
    for part in key.split("."):
        if not isinstance(value, dict) or part not in value:
            raise ManifestError(f"state field is missing: {key}")
        value = value[part]
    return value


def _json_pointer(document: Any, pointer: str, label: str) -> Any:
    if pointer == "":
        return document
    if not pointer.startswith("/"):
        raise ManifestError(f"{label} must be a JSON Pointer")
    value = document
    for raw_part in pointer[1:].split("/"):
        part = raw_part.replace("~1", "/").replace("~0", "~")
        if isinstance(value, dict) and part in value:
            value = value[part]
        elif isinstance(value, list):
            try:
                value = value[int(part)]
            except (ValueError, IndexError) as exc:
                raise ManifestError(f"{label} does not resolve: {pointer}") from exc
        else:
            raise ManifestError(f"{label} does not resolve: {pointer}")
    return value


def _validate_state_evidence(base: Path, side: dict[str, Any], label: str,
                             state_keys: list[str], capture_id: str,
                             image_sha256: str) -> dict[str, Any]:
    state = side.get("state")
    evidence_ref = side.get("stateEvidence")
    if not isinstance(state, dict) or not isinstance(evidence_ref, dict):
        raise ManifestError(f"{label} needs state and stateEvidence objects")
    evidence_path = _resolve_input(base, evidence_ref, f"{label}.stateEvidence")
    bindings = evidence_ref.get("bindings")
    if not isinstance(bindings, dict):
        raise ManifestError(f"{label}.stateEvidence.bindings must map state keys to JSON Pointers")
    try:
        document = json.loads(evidence_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ManifestError(f"cannot read state evidence JSON {evidence_path}: {exc}") from exc
    observed: dict[str, Any] = {}
    for key, expected in (("captureId", capture_id), ("imageSha256", image_sha256)):
        pointer = bindings.get(key)
        if not isinstance(pointer, str):
            raise ManifestError(f"{label}.stateEvidence has no JSON Pointer for {key}")
        actual = _json_pointer(document, pointer, f"{label}.stateEvidence.{key}")
        if actual != expected:
            raise ManifestError(
                f"{label} state evidence mismatch at {key}: "
                f"manifest={expected!r}, evidence={actual!r}"
            )
    for key in state_keys:
        pointer = bindings.get(key)
        if not isinstance(pointer, str):
            raise ManifestError(f"{label}.stateEvidence has no JSON Pointer for {key}")
        claimed = _state_value(state, key)
        actual = _json_pointer(document, pointer, f"{label}.stateEvidence.{key}")
        if claimed != actual:
            raise ManifestError(
                f"{label} state evidence mismatch at {key}: "
                f"manifest={claimed!r}, evidence={actual!r}"
            )
        observed[key] = actual
    return {"path": str(evidence_path), "sha256": sha256_file(evidence_path), "state": observed}


def _validate_region(region: Any, image_size: tuple[int, int], index: int) -> dict[str, Any]:
    label = f"regions[{index}]"
    if not isinstance(region, dict):
        raise ManifestError(f"{label} must be an object")
    name = _safe_filename_component(region.get("name"), f"{label}.name")
    bounds = [region.get(field) for field in ("x", "y", "width", "height")]
    if any(not isinstance(value, int) or isinstance(value, bool) for value in bounds):
        raise ManifestError(f"{label} x, y, width, and height must be integers")
    x, y, width, height = bounds
    if x < 0 or y < 0 or width <= 0 or height <= 0:
        raise ManifestError(f"{label} has invalid bounds: {bounds}")
    if x + width > image_size[0] or y + height > image_size[1]:
        raise ManifestError(f"{label} exceeds canonical frame bounds {image_size}: {bounds}")
    return {"name": name, "x": x, "y": y, "width": width, "height": height}


def _diff_region(reference: Image.Image, firestaff: Image.Image,
                 region: dict[str, Any], tolerance: int, out_path: Path) -> dict[str, Any]:
    box = (
        region["x"], region["y"],
        region["x"] + region["width"], region["y"] + region["height"],
    )
    ref_crop = reference.crop(box)
    fs_crop = firestaff.crop(box)
    ref_pixels = ref_crop.load()
    fs_pixels = fs_crop.load()
    width, height = ref_crop.size
    mask = Image.new("RGB", ref_crop.size, (255, 255, 255))
    mask_pixels = mask.load()
    differing = 0
    for y in range(height):
        for x in range(width):
            left = ref_pixels[x, y]
            right = fs_pixels[x, y]
            if any(abs(int(a) - int(b)) > tolerance for a, b in zip(left, right)):
                differing += 1
                mask_pixels[x, y] = (255, 0, 0)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    mask.save(out_path, format="PNG", optimize=True)
    total = width * height
    return {
        **region,
        "differingPixels": differing,
        "totalPixels": total,
        "differencePercent": round(differing * 100.0 / total, 6),
        "maskPath": str(out_path),
    }


def compare_manifest(manifest_path: Path, out_dir: Path,
                     tolerance: int = 0) -> dict[str, Any]:
    if tolerance < 0 or tolerance > 255:
        raise ManifestError("tolerance must be between 0 and 255")
    manifest_path = manifest_path.resolve()
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ManifestError(f"cannot read JSON manifest {manifest_path}: {exc}") from exc
    if not isinstance(manifest, dict) or manifest.get("schema") != SCHEMA:
        raise ManifestError(f"manifest schema must be {SCHEMA!r}")
    game_id = _required_text(manifest.get("gameId"), "gameId").lower()
    if game_id not in {"dm1", "csb", "dm2", "theron", "nexus"}:
        raise ManifestError("gameId must be dm1, csb, dm2, theron, or nexus")
    platform_id = _required_text(manifest.get("platformId"), "platformId")
    reference_build_id = _required_text(manifest.get("referenceBuildId"), "referenceBuildId")
    build_id = _required_text(manifest.get("firestaffBuildId"), "firestaffBuildId")
    state_keys = manifest.get("comparisonStateKeys")
    if not isinstance(state_keys, list) or not state_keys:
        raise ManifestError("comparisonStateKeys must be a non-empty list")
    if any(not isinstance(key, str) or not key.strip() for key in state_keys):
        raise ManifestError("comparisonStateKeys entries must be non-empty strings")
    if len(set(state_keys)) != len(state_keys):
        raise ManifestError("comparisonStateKeys entries must be unique")
    regions = manifest.get("regions")
    if not isinstance(regions, list) or not regions:
        raise ManifestError("regions must be a non-empty list")
    region_names = [
        _safe_filename_component(region.get("name"), f"regions[{index}].name")
        if isinstance(region, dict) else ""
        for index, region in enumerate(regions)
    ]
    if len(set(region_names)) != len(region_names):
        raise ManifestError("region names must be unique")
    captures = manifest.get("captures")
    if not isinstance(captures, list) or not captures:
        raise ManifestError("captures must be a non-empty list")
    frame_size = manifest.get("frameSize")
    if (not isinstance(frame_size, list) or len(frame_size) != 2 or
            any(not isinstance(value, int) or isinstance(value, bool) or
                value <= 0 or value > MAX_FRAME_DIMENSION for value in frame_size)):
        raise ManifestError(
            f"frameSize must be [width, height] with dimensions from 1 to {MAX_FRAME_DIMENSION}"
        )
    canonical_size = (frame_size[0], frame_size[1])
    out_dir = _safe_output_dir(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)
    report: dict[str, Any] = {
        "schema": "firestaff.parity.paired_capture_report.v1",
        "status": "PAIR_READY_METRIC_ONLY",
        "claim": "metrics only; this report does not claim behavioral or visual parity",
        "gameId": game_id,
        "platformId": platform_id,
        "referenceBuildId": reference_build_id,
        "firestaffBuildId": build_id,
        "manifestPath": str(manifest_path),
        "comparisonStateKeys": state_keys,
        "canonicalFrameSize": list(canonical_size),
        "channelTolerance": tolerance,
        "captures": [],
    }
    capture_ids: set[str] = set()
    for capture_index, capture in enumerate(captures):
        label = f"captures[{capture_index}]"
        if not isinstance(capture, dict):
            raise ManifestError(f"{label} must be an object")
        capture_id = _safe_filename_component(capture.get("id"), f"{label}.id")
        if capture_id in capture_ids:
            raise ManifestError(f"duplicate capture id: {capture_id}")
        capture_ids.add(capture_id)
        reference_path = _resolve_input(
            manifest_path.parent, capture.get("reference"), f"{label}.reference"
        )
        firestaff_path = _resolve_input(
            manifest_path.parent, capture.get("firestaff"), f"{label}.firestaff"
        )
        reference_sha256 = sha256_file(reference_path)
        firestaff_sha256 = sha256_file(firestaff_path)
        reference_state_evidence = _validate_state_evidence(
            manifest_path.parent, capture["reference"], f"{label}.reference", state_keys,
            capture_id, reference_sha256,
        )
        firestaff_state_evidence = _validate_state_evidence(
            manifest_path.parent, capture["firestaff"], f"{label}.firestaff", state_keys,
            capture_id, firestaff_sha256,
        )
        state_values: dict[str, Any] = {}
        for key in state_keys:
            reference_value = reference_state_evidence["state"][key]
            firestaff_value = firestaff_state_evidence["state"][key]
            if reference_value != firestaff_value:
                raise ManifestError(
                    f"{capture_id}: source-state mismatch at {key}: "
                    f"reference={reference_value!r}, firestaff={firestaff_value!r}"
                )
            state_values[key] = reference_value
        with Image.open(reference_path) as image:
            reference_image = image.convert("RGB")
        with Image.open(firestaff_path) as image:
            firestaff_image = image.convert("RGB")
        if reference_image.size != canonical_size or firestaff_image.size != canonical_size:
            raise ManifestError(
                f"{capture_id}: both images must match frameSize {canonical_size}; "
                f"reference={reference_image.size}, firestaff={firestaff_image.size}"
            )
        normalized_regions = [
            _validate_region(region, canonical_size, index)
            for index, region in enumerate(regions)
        ]
        region_reports = []
        for region_index, region in enumerate(normalized_regions):
            mask_name = (
                f"capture{capture_index:04d}.region{region_index:04d}."
                f"{capture_id}.{region['name']}.mask.png"
            )
            region_reports.append(
                _diff_region(reference_image, firestaff_image, region, tolerance,
                             _safe_output_file(out_dir, mask_name))
            )
        report["captures"].append({
            "id": capture_id,
            "state": state_values,
            "reference": {"path": str(reference_path), "sha256": reference_sha256},
            "firestaff": {"path": str(firestaff_path), "sha256": firestaff_sha256},
            "referenceStateEvidence": {
                "path": reference_state_evidence["path"],
                "sha256": reference_state_evidence["sha256"],
            },
            "firestaffStateEvidence": {
                "path": firestaff_state_evidence["path"],
                "sha256": firestaff_state_evidence["sha256"],
            },
            "regions": region_reports,
        })
    report_path = _safe_output_file(out_dir, "paired_capture_report.json")
    report["reportPath"] = str(report_path)
    report_path.write_text(json.dumps(report, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return report


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--manifest", required=True, type=Path,
                        help="paired capture JSON manifest")
    parser.add_argument("--out-dir", required=True, type=Path,
                        help="diff masks and report; must be outside Git or ignored")
    parser.add_argument("--tolerance", type=int, default=0,
                        help="per-channel tolerance, 0 through 255 (default: exact pixels)")
    args = parser.parse_args(argv)
    try:
        report = compare_manifest(args.manifest, args.out_dir, args.tolerance)
    except ManifestError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2
    print(f"{report['status']}: {len(report['captures'])} paired capture(s)")
    for capture in report["captures"]:
        for region in capture["regions"]:
            print(
                f"{capture['id']} {region['name']}: "
                f"{region['differingPixels']}/{region['totalPixels']} pixels differ "
                f"({region['differencePercent']:.4f}%)"
            )
    print(f"report: {report['reportPath']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
