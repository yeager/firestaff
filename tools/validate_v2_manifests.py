#!/usr/bin/env python3
"""Validate Firestaff V2 asset manifest scaffolding without external deps.

This is intentionally conservative: it checks repository-local manifest shape,
path safety, duplicate ids, referenced spec files, and optional PNG dimensions.
It does not approve art direction or DM1 visual faithfulness.
"""

from __future__ import annotations

import argparse
import contextlib
import io
import json
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any, Iterable

try:
    from PIL import Image
except Exception:  # pragma: no cover - reported at runtime by --check-files
    Image = None  # type: ignore[assignment]

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_MANIFEST_GLOB = "assets-v2/manifests/*.manifest.json"
ALLOWED_PRODUCTION_CLASSES = {
    "preserve-scale-repaint",
    "redraw-native",
    "system-rebuild",
}
ALLOWED_STATUSES = {
    "planned",
    "stubbed",
    "in-progress",
    "approved",
    "shipped",
    "blocked",
    "rebuilt",
}
FILE_EXPECTED_STATUSES = {"in-progress", "approved", "shipped", "rebuilt"}


@dataclass
class Finding:
    level: str
    path: Path
    message: str

    def render(self) -> str:
        rel = self.path.relative_to(ROOT) if self.path.is_absolute() and self.path.is_relative_to(ROOT) else self.path
        return f"{self.level}: {rel}: {self.message}"


def is_repo_relative(value: str) -> bool:
    parts = PurePosixPath(value).parts
    return not value.startswith("/") and ".." not in parts and value != ""


def as_object(value: Any, label: str, findings: list[Finding], path: Path) -> dict[str, Any]:
    if not isinstance(value, dict):
        findings.append(Finding("ERROR", path, f"{label} must be an object"))
        return {}
    return value


def as_list(value: Any, label: str, findings: list[Finding], path: Path) -> list[Any]:
    if not isinstance(value, list):
        findings.append(Finding("ERROR", path, f"{label} must be an array"))
        return []
    return value


def require_string(obj: dict[str, Any], key: str, findings: list[Finding], path: Path, label: str) -> str | None:
    value = obj.get(key)
    if not isinstance(value, str) or value == "":
        findings.append(Finding("ERROR", path, f"{label}.{key} must be a non-empty string"))
        return None
    return value


def require_size(obj: dict[str, Any], key: str, findings: list[Finding], path: Path, label: str) -> tuple[int, int] | None:
    size = obj.get(key)
    if not isinstance(size, dict):
        findings.append(Finding("ERROR", path, f"{label}.{key} must be an object"))
        return None
    width = size.get("width")
    height = size.get("height")
    if not isinstance(width, int) or not isinstance(height, int) or width <= 0 or height <= 0:
        findings.append(Finding("ERROR", path, f"{label}.{key} must contain positive integer width/height"))
        return None
    return (width, height)


def png_dimensions(png: Path) -> tuple[int, int] | None:
    if Image is None:
        return None
    with Image.open(png) as image:
        return image.size


def find_png_with_size(directory: Path, expected: tuple[int, int]) -> list[Path]:
    if not directory.is_dir() or Image is None:
        return []
    matches: list[Path] = []
    for png in sorted(directory.glob("*.png")):
        try:
            if png_dimensions(png) == expected:
                matches.append(png)
        except Exception:
            continue
    return matches


def validate_paths(paths: dict[str, Any], findings: list[Finding], manifest_path: Path, asset_label: str) -> dict[str, Path]:
    resolved: dict[str, Path] = {}
    for key in ("masterDir", "derivedDir", "spec"):
        value = paths.get(key)
        if not isinstance(value, str) or not is_repo_relative(value):
            findings.append(Finding("ERROR", manifest_path, f"{asset_label}.paths.{key} must be repository-relative and non-empty"))
            continue
        resolved[key] = ROOT / value
    return resolved


def validate_manifest(path: Path, *, check_files: bool, strict_files: bool) -> tuple[list[Finding], dict[str, Any]]:
    findings: list[Finding] = []
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        return [Finding("ERROR", path, f"cannot parse JSON: {exc}")], {"assets": 0}

    manifest = as_object(data, "manifest", findings, path)
    for key in ("manifestVersion", "packId"):
        require_string(manifest, key, findings, path, "manifest")

    target_policy = as_object(manifest.get("targetPolicy"), "manifest.targetPolicy", findings, path)
    for key in ("masterResolution", "layoutSkeleton"):
        require_string(target_policy, key, findings, path, "manifest.targetPolicy")
    derived = as_list(target_policy.get("derivedResolutions"), "manifest.targetPolicy.derivedResolutions", findings, path)
    for idx, value in enumerate(derived):
        if not isinstance(value, str) or value == "":
            findings.append(Finding("ERROR", path, f"manifest.targetPolicy.derivedResolutions[{idx}] must be a non-empty string"))

    assets = as_list(manifest.get("assets"), "manifest.assets", findings, path)
    seen_ids: set[str] = set()
    file_checked = 0
    file_missing = 0
    downscale_checked = 0
    downscale_failed = 0

    for index, raw_asset in enumerate(assets):
        label = f"asset[{index}]"
        asset = as_object(raw_asset, label, findings, path)
        asset_id = require_string(asset, "id", findings, path, label)
        if asset_id:
            label = asset_id
            if asset_id in seen_ids:
                findings.append(Finding("ERROR", path, f"duplicate asset id: {asset_id}"))
            seen_ids.add(asset_id)
        for key in ("family", "role"):
            require_string(asset, key, findings, path, label)
        production = require_string(asset, "productionClass", findings, path, label)
        if production and production not in ALLOWED_PRODUCTION_CLASSES:
            findings.append(Finding("ERROR", path, f"{label}.productionClass is not allowed: {production}"))
        status = require_string(asset, "status", findings, path, label)
        if status and status not in ALLOWED_STATUSES:
            findings.append(Finding("ERROR", path, f"{label}.status is not allowed: {status}"))

        source_ref = as_object(asset.get("sourceReference"), f"{label}.sourceReference", findings, path)
        require_string(source_ref, "origin", findings, path, f"{label}.sourceReference")
        original_size = as_object(source_ref.get("originalSize"), f"{label}.sourceReference.originalSize", findings, path)
        require_size({"originalSize": original_size}, "originalSize", findings, path, f"{label}.sourceReference")

        sizes = as_object(asset.get("sizes"), f"{label}.sizes", findings, path)
        master_size = require_size(sizes, "master4k", findings, path, f"{label}.sizes")
        derived_size = require_size(sizes, "derived1080p", findings, path, f"{label}.sizes")
        if master_size and derived_size:
            downscale_checked += 1
            if master_size != (derived_size[0] * 2, derived_size[1] * 2):
                downscale_failed += 1
                findings.append(Finding(
                    "ERROR",
                    path,
                    f"{label}.sizes must keep exact 50% 4K-to-1080p downscale: master={master_size[0]}x{master_size[1]} derived={derived_size[0]}x{derived_size[1]}",
                ))
        paths = validate_paths(as_object(asset.get("paths"), f"{label}.paths", findings, path), findings, path, label)

        spec_path = paths.get("spec")
        if spec_path and not spec_path.is_file():
            findings.append(Finding("ERROR", path, f"{label}.paths.spec missing: {spec_path.relative_to(ROOT)}"))

        if check_files and status in FILE_EXPECTED_STATUSES and master_size and derived_size:
            if Image is None:
                findings.append(Finding("ERROR", path, "--check-files requires Pillow/PIL, but import failed"))
                continue
            for path_key, expected in (("masterDir", master_size), ("derivedDir", derived_size)):
                directory = paths.get(path_key)
                if not directory or not directory.is_dir():
                    findings.append(Finding("ERROR" if strict_files else "WARN", path, f"{label}.paths.{path_key} missing: {directory.relative_to(ROOT) if directory else path_key}"))
                    file_missing += 1
                    continue
                matches = find_png_with_size(directory, expected)
                file_checked += 1
                if not matches:
                    findings.append(Finding("ERROR" if strict_files else "WARN", path, f"{label} has no PNG in {directory.relative_to(ROOT)} with expected size {expected[0]}x{expected[1]}"))
                    file_missing += 1

    summary = {
        "packId": manifest.get("packId"),
        "assets": len(assets),
        "fileChecks": file_checked,
        "fileMissing": file_missing,
        "downscaleChecks": downscale_checked,
        "downscaleFailures": downscale_failed,
        "errors": sum(1 for f in findings if f.level == "ERROR"),
        "warnings": sum(1 for f in findings if f.level == "WARN"),
    }
    return findings, summary


def iter_manifest_paths(args: argparse.Namespace) -> Iterable[Path]:
    if args.manifest:
        for raw in args.manifest:
            yield (ROOT / raw) if not Path(raw).is_absolute() else Path(raw)
    else:
        yield from sorted(ROOT.glob(DEFAULT_MANIFEST_GLOB))


def selftest_manifest(*, pack_id: str, asset_id: str, master: tuple[int, int], derived: tuple[int, int]) -> dict[str, Any]:
    return {
        "manifestVersion": "2.0-test",
        "packId": pack_id,
        "targetPolicy": {
            "masterResolution": "4k",
            "derivedResolutions": ["1080p"],
            "layoutSkeleton": "fixed",
        },
        "assets": [
            {
                "id": asset_id,
                "family": "validator-selftest",
                "role": "regression-fixture",
                "productionClass": "redraw-native",
                "status": "planned",
                "sourceReference": {
                    "origin": "validator self-test synthetic fixture",
                    "originalSize": {"width": 320, "height": 200},
                },
                "sizes": {
                    "master4k": {"width": master[0], "height": master[1]},
                    "derived1080p": {"width": derived[0], "height": derived[1]},
                },
                "paths": {
                    "masterDir": "assets-v2/selftest/master",
                    "derivedDir": "assets-v2/selftest/derived",
                    "spec": "assets-v2/manifests/schema/README.md",
                },
            }
        ],
    }


def run_selftest() -> int:
    failures: list[str] = []
    with tempfile.TemporaryDirectory(prefix=".validate-v2-selftest-", dir=ROOT) as temp_raw:
        temp = Path(temp_raw)
        good = temp / "good.manifest.json"
        bad_scale = temp / "bad-scale.manifest.json"
        dup_a = temp / "duplicate-a.manifest.json"
        dup_b = temp / "duplicate-b.manifest.json"
        good.write_text(json.dumps(selftest_manifest(
            pack_id="selftest.good",
            asset_id="selftest-good",
            master=(3840, 2160),
            derived=(1920, 1080),
        )), encoding="utf-8")
        bad_scale.write_text(json.dumps(selftest_manifest(
            pack_id="selftest.bad-scale",
            asset_id="selftest-bad-scale",
            master=(3840, 2160),
            derived=(1919, 1080),
        )), encoding="utf-8")
        dup_a.write_text(json.dumps(selftest_manifest(
            pack_id="selftest.duplicate-pack",
            asset_id="selftest-duplicate-a",
            master=(3840, 2160),
            derived=(1920, 1080),
        )), encoding="utf-8")
        dup_b.write_text(json.dumps(selftest_manifest(
            pack_id="selftest.duplicate-pack",
            asset_id="selftest-duplicate-b",
            master=(3840, 2160),
            derived=(1920, 1080),
        )), encoding="utf-8")

        good_findings, _ = validate_manifest(good, check_files=False, strict_files=False)
        if any(f.level == "ERROR" for f in good_findings):
            failures.append("exact 3840x2160 -> 1920x1080 fixture unexpectedly failed")

        bad_findings, _ = validate_manifest(bad_scale, check_files=False, strict_files=False)
        if not any("exact 50% 4K-to-1080p downscale" in f.message for f in bad_findings):
            failures.append("non-50% 4K -> 1080p fixture did not report the downscale error")

        out = io.StringIO()
        err = io.StringIO()
        with contextlib.redirect_stdout(out), contextlib.redirect_stderr(err):
            duplicate_status = main([str(dup_a.relative_to(ROOT)), str(dup_b.relative_to(ROOT))])
        duplicate_output = out.getvalue() + err.getvalue()
        if duplicate_status != 1 or "duplicate packId" not in duplicate_output:
            failures.append("duplicate packId fixture did not fail with duplicate packId")

    if failures:
        for failure in failures:
            print(f"self-test failed: {failure}", file=sys.stderr)
        return 1
    print("validator self-test passed: exact 50% downscale accepted; non-50% downscale and duplicate packId rejected")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Validate Firestaff V2 asset manifest scaffolding.")
    parser.add_argument("manifest", nargs="*", help="manifest(s) to validate; defaults to assets-v2/manifests/*.manifest.json")
    parser.add_argument("--check-files", action="store_true", help="also check expected-size PNGs for in-progress/rebuilt/approved/shipped assets")
    parser.add_argument("--strict-files", action="store_true", help="treat missing expected-size PNGs as errors instead of warnings")
    parser.add_argument("--json", action="store_true", help="emit machine-readable summary JSON")
    parser.add_argument("--self-test", action="store_true", help="run lightweight synthetic regression tests for V2 manifest checks")
    args = parser.parse_args(argv)

    if args.self_test:
        return run_selftest()

    all_findings: list[Finding] = []
    summaries: list[dict[str, Any]] = []
    seen_pack_ids: dict[str, Path] = {}
    duplicate_pack_ids: list[dict[str, str]] = []
    manifest_paths = list(iter_manifest_paths(args))
    if not manifest_paths:
        print(f"no manifests matched {DEFAULT_MANIFEST_GLOB}", file=sys.stderr)
        return 2

    for manifest_path in manifest_paths:
        findings, summary = validate_manifest(manifest_path, check_files=args.check_files, strict_files=args.strict_files)
        pack_id = summary.get("packId")
        if isinstance(pack_id, str) and pack_id:
            first_path = seen_pack_ids.get(pack_id)
            if first_path is not None:
                duplicate_pack_ids.append({
                    "packId": pack_id,
                    "firstManifest": first_path.relative_to(ROOT).as_posix(),
                    "duplicateManifest": manifest_path.relative_to(ROOT).as_posix(),
                })
                all_findings.append(Finding(
                    "ERROR",
                    manifest_path,
                    f"duplicate packId {pack_id!r}; first seen in {first_path.relative_to(ROOT)}",
                ))
            else:
                seen_pack_ids[pack_id] = manifest_path
        all_findings.extend(findings)
        summaries.append(summary | {"manifest": manifest_path.relative_to(ROOT).as_posix()})

    errors = sum(1 for finding in all_findings if finding.level == "ERROR")
    warnings = sum(1 for finding in all_findings if finding.level == "WARN")
    if args.json:
        print(json.dumps({
            "manifests": summaries,
            "checks": {
                "duplicatePackIds": {
                    "duplicates": duplicate_pack_ids,
                    "failures": len(duplicate_pack_ids),
                },
                "downscalePolicy": {
                    "rule": "master4k == derived1080p * 2",
                    "checkedAssets": sum(int(summary.get("downscaleChecks", 0)) for summary in summaries),
                    "failures": sum(int(summary.get("downscaleFailures", 0)) for summary in summaries),
                },
            },
            "errors": errors,
            "warnings": warnings,
        }, indent=2, sort_keys=True))
    else:
        for finding in all_findings:
            print(finding.render(), file=sys.stderr if finding.level == "ERROR" else sys.stdout)
        asset_count = sum(int(summary.get("assets", 0)) for summary in summaries)
        print(f"validated {len(summaries)} V2 manifest(s), {asset_count} asset entries, {errors} error(s), {warnings} warning(s)")
    return 1 if errors else 0


if __name__ == "__main__":
    raise SystemExit(main())
