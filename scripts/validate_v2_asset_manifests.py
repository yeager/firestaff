#!/usr/bin/env python3
"""Validate Firestaff V2 asset manifests with repository-local invariants.

This intentionally avoids external jsonschema dependencies so worker VMs can run
it in a clean checkout. It is stricter than JSON syntax, but narrower than a
full schema implementation: it checks the fields and production contracts the V2
asset pipeline depends on before art files are generated.
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[1]
MANIFEST_DIR = REPO_ROOT / "assets-v2" / "manifests"
ALLOWED_PRODUCTION_CLASSES = {"preserve-scale-repaint", "redraw-native", "system-rebuild"}
ALLOWED_STATUS = {"planned", "stubbed", "in-progress", "approved", "shipped", "blocked", "rebuilt"}
REQUIRED_TOP_LEVEL = {"manifestVersion", "packId", "targetPolicy", "assets"}
REQUIRED_TARGET_POLICY = {"masterResolution", "derivedResolutions", "layoutSkeleton"}
REQUIRED_ASSET = {"id", "family", "role", "productionClass", "sourceReference", "sizes", "paths", "status"}
REQUIRED_SOURCE_REF = {"origin", "originalSize"}
REQUIRED_SOURCE_EVIDENCE = {"file", "function", "lines", "assertion"}
REQUIRED_LINE_RANGE = {"start", "end"}
REQUIRED_SIZE = {"width", "height"}
REQUIRED_PATHS = {"masterDir", "derivedDir", "spec"}


class ValidationError(Exception):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValidationError(message)


def require_string(value: Any, label: str) -> None:
    require(isinstance(value, str) and value.strip(), f"{label} must be a non-empty string")


def require_size(value: Any, label: str) -> tuple[int, int]:
    require(isinstance(value, dict), f"{label} must be an object")
    require(REQUIRED_SIZE <= set(value), f"{label} missing {sorted(REQUIRED_SIZE - set(value))}")
    width = value["width"]
    height = value["height"]
    require(isinstance(width, int) and width > 0, f"{label}.width must be a positive integer")
    require(isinstance(height, int) and height > 0, f"{label}.height must be a positive integer")
    return width, height


def validate_source_evidence(value: Any, label: str) -> None:
    require(isinstance(value, list) and value, f"{label}.sourceEvidence must be a non-empty array")
    for index, entry in enumerate(value):
        entry_label = f"{label}.sourceEvidence[{index}]"
        require(isinstance(entry, dict), f"{entry_label} must be an object")
        require(REQUIRED_SOURCE_EVIDENCE <= set(entry), f"{entry_label} missing {sorted(REQUIRED_SOURCE_EVIDENCE - set(entry))}")
        require_string(entry["file"], f"{entry_label}.file")
        require_string(entry["function"], f"{entry_label}.function")
        require_string(entry["assertion"], f"{entry_label}.assertion")
        lines = entry["lines"]
        require(isinstance(lines, dict), f"{entry_label}.lines must be an object")
        require(REQUIRED_LINE_RANGE <= set(lines), f"{entry_label}.lines missing {sorted(REQUIRED_LINE_RANGE - set(lines))}")
        start = lines["start"]
        end = lines["end"]
        require(isinstance(start, int) and start > 0, f"{entry_label}.lines.start must be a positive integer")
        require(isinstance(end, int) and end >= start, f"{entry_label}.lines.end must be >= start")


def validate_asset(manifest_path: Path, asset: Any, seen_ids: set[str], *, strict_paths: bool, required_source_evidence_ids: set[str]) -> list[str]:
    require(isinstance(asset, dict), f"{manifest_path}: asset entry must be an object")
    label = f"{manifest_path}:{asset.get('id', '<missing id>')}"
    require(REQUIRED_ASSET <= set(asset), f"{label} missing {sorted(REQUIRED_ASSET - set(asset))}")

    asset_id = asset["id"]
    require_string(asset_id, f"{label}.id")
    require(asset_id not in seen_ids, f"duplicate asset id within manifest {manifest_path}: {asset_id}")
    seen_ids.add(asset_id)
    require_string(asset["family"], f"{label}.family")
    require_string(asset["role"], f"{label}.role")
    require(asset["productionClass"] in ALLOWED_PRODUCTION_CLASSES, f"{label}.productionClass is not allowed: {asset['productionClass']}")
    require(asset["status"] in ALLOWED_STATUS, f"{label}.status is not allowed: {asset['status']}")

    source = asset["sourceReference"]
    require(isinstance(source, dict), f"{label}.sourceReference must be an object")
    require(REQUIRED_SOURCE_REF <= set(source), f"{label}.sourceReference missing {sorted(REQUIRED_SOURCE_REF - set(source))}")
    require_string(source["origin"], f"{label}.sourceReference.origin")
    if "graphicsDatIndices" in source:
        indices = source["graphicsDatIndices"]
        require(isinstance(indices, list), f"{label}.sourceReference.graphicsDatIndices must be an array")
        require(all(isinstance(i, int) and i >= 0 for i in indices), f"{label}.sourceReference.graphicsDatIndices must contain non-negative integers")
    require_size(source["originalSize"], f"{label}.sourceReference.originalSize")
    if "sourceEvidence" in source:
        validate_source_evidence(source["sourceEvidence"], f"{label}.sourceReference")
    require(asset_id not in required_source_evidence_ids or "sourceEvidence" in source, f"{label}.sourceReference.sourceEvidence is required by --require-source-evidence")

    sizes = asset["sizes"]
    require(isinstance(sizes, dict), f"{label}.sizes must be an object")
    require({"master4k", "derived1080p"} <= set(sizes), f"{label}.sizes must include master4k and derived1080p")
    master = require_size(sizes["master4k"], f"{label}.sizes.master4k")
    derived = require_size(sizes["derived1080p"], f"{label}.sizes.derived1080p")
    require(master[0] == derived[0] * 2 and master[1] == derived[1] * 2, f"{label} violates exact 50% 4K-to-1080p downscale: master={master} derived={derived}")

    paths = asset["paths"]
    require(isinstance(paths, dict), f"{label}.paths must be an object")
    require(REQUIRED_PATHS <= set(paths), f"{label}.paths missing {sorted(REQUIRED_PATHS - set(paths))}")
    warnings: list[str] = []
    for key in ("masterDir", "derivedDir", "spec"):
        require_string(paths[key], f"{label}.paths.{key}")
        path_parts = Path(paths[key]).parts
        require(not Path(paths[key]).is_absolute(), f"{label}.paths.{key} must be repo-relative")
        require(".." not in path_parts, f"{label}.paths.{key} must not traverse upward")
    if strict_paths:
        for key in ("masterDir", "derivedDir"):
            path = REPO_ROOT / paths[key]
            require(path.is_dir(), f"{label}.paths.{key} does not exist: {paths[key]}")
        spec_path = REPO_ROOT / paths["spec"]
        require(spec_path.is_file(), f"{label}.paths.spec does not exist: {paths['spec']}")
    else:
        for key in ("masterDir", "derivedDir"):
            path = REPO_ROOT / paths[key]
            if not path.is_dir():
                warnings.append(f"{label}.paths.{key} missing scaffold directory: {paths[key]}")
        spec_path = REPO_ROOT / paths["spec"]
        if not spec_path.is_file():
            warnings.append(f"{label}.paths.spec missing scaffold spec: {paths['spec']}")

    notes = asset.get("notes")
    if notes is not None:
        require(isinstance(notes, list) and all(isinstance(n, str) and n.strip() for n in notes), f"{label}.notes must be an array of non-empty strings")
    return warnings


def validate_manifest(path: Path, seen_pack_ids: set[str], *, strict_paths: bool, required_source_evidence_ids: set[str]) -> tuple[int, list[str], set[str]]:
    data = json.loads(path.read_text(encoding="utf-8"))
    require(isinstance(data, dict), f"{path}: manifest must be an object")
    require(REQUIRED_TOP_LEVEL <= set(data), f"{path}: missing {sorted(REQUIRED_TOP_LEVEL - set(data))}")
    pack_id = data["packId"]
    require_string(data["manifestVersion"], f"{path}.manifestVersion")
    require_string(pack_id, f"{path}.packId")
    require(pack_id not in seen_pack_ids, f"duplicate V2 packId: {pack_id}")
    seen_pack_ids.add(pack_id)
    if "description" in data:
        require_string(data["description"], f"{path}.description")

    target = data["targetPolicy"]
    require(isinstance(target, dict), f"{path}.targetPolicy must be an object")
    require(REQUIRED_TARGET_POLICY <= set(target), f"{path}.targetPolicy missing {sorted(REQUIRED_TARGET_POLICY - set(target))}")
    require_string(target["masterResolution"], f"{path}.targetPolicy.masterResolution")
    derived = target["derivedResolutions"]
    require(isinstance(derived, list) and all(isinstance(item, str) and item.strip() for item in derived), f"{path}.targetPolicy.derivedResolutions must be an array of strings")
    require("1080p" in derived, f"{path}.targetPolicy.derivedResolutions must include 1080p")
    require_string(target["layoutSkeleton"], f"{path}.targetPolicy.layoutSkeleton")

    assets = data["assets"]
    require(isinstance(assets, list), f"{path}.assets must be an array")
    require(assets, f"{path}.assets must not be empty")
    warnings: list[str] = []
    seen_ids: set[str] = set()
    seen_required_ids: set[str] = set()
    for asset in assets:
        if isinstance(asset, dict) and asset.get("id") in required_source_evidence_ids:
            seen_required_ids.add(asset["id"])
        warnings.extend(validate_asset(path, asset, seen_ids, strict_paths=strict_paths, required_source_evidence_ids=required_source_evidence_ids))
    return len(assets), warnings, seen_required_ids


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("manifests", nargs="*", type=Path, help="manifest files to validate; defaults to assets-v2/manifests/firestaff-v2-*.manifest.json")
    parser.add_argument("--strict-paths", action="store_true", help="fail if manifest scaffold directories/specs are missing")
    parser.add_argument("--require-source-evidence", action="append", default=[], metavar="ASSET_ID", help="require sourceReference.sourceEvidence for the exact asset id; may be repeated")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    manifests = args.manifests or sorted(MANIFEST_DIR.glob("firestaff-v2-*.manifest.json"))
    require(bool(manifests), "no V2 manifest files found")
    seen_pack_ids: set[str] = set()
    total_assets = 0
    all_warnings: list[str] = []
    required_source_evidence_ids = set(args.require_source_evidence)
    found_required_source_evidence_ids: set[str] = set()
    for manifest in manifests:
        count, warnings, found_required = validate_manifest(manifest, seen_pack_ids, strict_paths=args.strict_paths, required_source_evidence_ids=required_source_evidence_ids)
        total_assets += count
        all_warnings.extend(warnings)
        found_required_source_evidence_ids.update(found_required)
    missing_required = required_source_evidence_ids - found_required_source_evidence_ids
    require(not missing_required, f"--require-source-evidence asset ids not found: {sorted(missing_required)}")
    for warning in all_warnings:
        print(f"warning: {warning}", file=sys.stderr)
    print(f"validated {len(manifests)} V2 manifests / {total_assets} assets")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, json.JSONDecodeError, ValidationError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise SystemExit(1)
