#!/usr/bin/env python3
"""Validate V2 upscale/enhancement recipes without producing assets."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import PurePosixPath
from typing import Any

DEFAULT_PIPELINE_VERSION = "v2.1-upscale-scout"
ENHANCED_PIPELINE_VERSION = "v2.2-enhanced-4k1080"
SUPPORTED_PIPELINE_VERSIONS = {DEFAULT_PIPELINE_VERSION, ENHANCED_PIPELINE_VERSION}
CHECKSUM_STORE_ROOT = "assets-v2/store"
V22_PATH_PREFIX = "assets-v2/v2.2-enhanced/"

TEMPLATES: dict[str, dict[str, Any]] = {
    DEFAULT_PIPELINE_VERSION: {
        "pipelineVersion": DEFAULT_PIPELINE_VERSION,
        "assets": [
            {
                "logicalId": "fs.v2.ui.viewport-base",
                "sourceLogicalSize": {"width": 224, "height": 136},
                "masterSize": {"width": 2240, "height": 1360},
                "derivedSize": {"width": 1120, "height": 680},
                "masterPath": "assets-v2/ui/wave1/viewport-frame/masters/4k/fs-v2-viewport-base.master.png",
                "derivedPath": "assets-v2/ui/wave1/viewport-frame/exports/1080p/fs-v2-viewport-base.1080p.png",
                "provenance": "Planning example only; no asset file is present in this scout.",
            }
        ],
    },
    ENHANCED_PIPELINE_VERSION: {
        "pipelineVersion": ENHANCED_PIPELINE_VERSION,
        "checksumStoreRoot": CHECKSUM_STORE_ROOT,
        "assets": [
            {
                "logicalId": "fs.v2.2.ui.viewport-base.enhanced",
                "assetLineage": "v2.2-enhanced",
                "sourceLogicalSize": {"width": 224, "height": 136},
                "masterSize": {"width": 2240, "height": 1360},
                "derivedSize": {"width": 1120, "height": 680},
                "masterPath": "assets-v2/v2.2-enhanced/ui/viewport-base/masters/4k/fs-v2.2-viewport-base.master.png",
                "derivedPath": "assets-v2/v2.2-enhanced/ui/viewport-base/exports/1080p/fs-v2.2-viewport-base.1080p.png",
                "provenance": {
                    "sourceKind": "enhanced-clean-room-master",
                    "sourceNotes": "Planning example only; no asset file is present in this scaffold.",
                    "operator": "unassigned",
                    "toolchain": "unassigned",
                    "licenseNotes": "Must not include redistributed original DM1 asset bytes.",
                },
            }
        ],
    },
}


def positive_int(value: Any, field: str) -> int:
    if not isinstance(value, int) or value <= 0:
        raise ValueError(f"{field} must be a positive integer")
    return value


def dimensions(asset: dict[str, Any], key: str) -> tuple[int, int]:
    item = asset.get(key)
    if not isinstance(item, dict):
        raise ValueError(f"{key} must be an object")
    return (
        positive_int(item.get("width"), f"{key}.width"),
        positive_int(item.get("height"), f"{key}.height"),
    )


def safe_relative_path(value: Any, field: str) -> str:
    if not isinstance(value, str) or not value:
        raise ValueError(f"{field} must be a non-empty string")
    path = PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts:
        raise ValueError(f"{field} must be repository-relative and must not escape the checkout")
    return path.as_posix()


def planned_store_key(pipeline_version: str, logical_id: str, role: str, path: str) -> str:
    digest = hashlib.sha256(f"{pipeline_version}\0{logical_id}\0{role}\0{path}".encode("utf-8")).hexdigest()
    return f"sha256/{digest[:2]}/{digest}"


def require_v22_separation(asset: dict[str, Any], logical_id: str, master_path: str, derived_path: str) -> None:
    lineage = asset.get("assetLineage")
    if lineage != "v2.2-enhanced":
        raise ValueError(f"{logical_id}: assetLineage must be v2.2-enhanced")
    for field, path in (("masterPath", master_path), ("derivedPath", derived_path)):
        if not path.startswith(V22_PATH_PREFIX):
            raise ValueError(f"{logical_id}: {field} must live under {V22_PATH_PREFIX}")
        lowered = path.lower()
        if "v2.1" in lowered or "upscaled-original" in lowered or "/ui/wave1/" in lowered:
            raise ValueError(f"{logical_id}: {field} must not share V2.1 upscaled-original locations")

    provenance = asset.get("provenance")
    if not isinstance(provenance, dict):
        raise ValueError(f"{logical_id}: provenance must be an object for V2.2 Enhanced")
    required = ["sourceKind", "sourceNotes", "operator", "toolchain", "licenseNotes"]
    for key in required:
        value = provenance.get(key)
        if not isinstance(value, str) or not value.strip():
            raise ValueError(f"{logical_id}: provenance.{key} must be a non-empty string")
    if provenance["sourceKind"] in {"v2.1-upscaled-original", "dm1-original-extract"}:
        raise ValueError(f"{logical_id}: provenance.sourceKind must not identify a V2.1 upscaled original")


def validate_asset(asset: Any, pipeline_version: str) -> dict[str, Any]:
    if not isinstance(asset, dict):
        raise ValueError("asset entries must be objects")
    logical_id = asset.get("logicalId")
    if not isinstance(logical_id, str) or not logical_id:
        raise ValueError("logicalId must be a non-empty string")

    source_w, source_h = dimensions(asset, "sourceLogicalSize")
    master_w, master_h = dimensions(asset, "masterSize")
    derived_w, derived_h = dimensions(asset, "derivedSize")
    expected_master = (source_w * 10, source_h * 10)
    expected_derived = (master_w // 2, master_h // 2)

    if (master_w, master_h) != expected_master:
        raise ValueError(f"{logical_id}: masterSize must be exactly 10x sourceLogicalSize")
    if master_w % 2 or master_h % 2 or (derived_w, derived_h) != expected_derived:
        raise ValueError(f"{logical_id}: derivedSize must be exactly half of masterSize")

    master_path = safe_relative_path(asset.get("masterPath"), "masterPath")
    derived_path = safe_relative_path(asset.get("derivedPath"), "derivedPath")
    if pipeline_version == ENHANCED_PIPELINE_VERSION:
        require_v22_separation(asset, logical_id, master_path, derived_path)

    store_master = planned_store_key(pipeline_version, logical_id, "master", master_path)
    store_derived = planned_store_key(pipeline_version, logical_id, "derived", derived_path)
    result = {
        "logicalId": logical_id,
        "sourceLogicalSize": [source_w, source_h],
        "masterSize": [master_w, master_h],
        "derivedSize": [derived_w, derived_h],
        "wouldReadMaster": master_path,
        "wouldWriteDerived": derived_path,
        "wouldStore": {
            "master": store_master,
            "derived": store_derived,
        },
        "writesAssets": False,
    }
    if pipeline_version == ENHANCED_PIPELINE_VERSION:
        result["assetLineage"] = asset["assetLineage"]
        result["checksumStoreHandoff"] = {
            "storeRoot": CHECKSUM_STORE_ROOT,
            "masterObject": f"{CHECKSUM_STORE_ROOT}/{store_master}",
            "derivedObject": f"{CHECKSUM_STORE_ROOT}/{store_derived}",
        }
        result["provenance"] = asset["provenance"]
    return result


def load_recipe(path: str) -> dict[str, Any]:
    with open(path, "r", encoding="utf-8") as handle:
        recipe = json.load(handle)
    if not isinstance(recipe, dict):
        raise ValueError("recipe must be a JSON object")
    pipeline_version = recipe.get("pipelineVersion")
    if pipeline_version not in SUPPORTED_PIPELINE_VERSIONS:
        supported = ", ".join(sorted(SUPPORTED_PIPELINE_VERSIONS))
        raise ValueError(f"pipelineVersion must be one of: {supported}")
    if pipeline_version == ENHANCED_PIPELINE_VERSION:
        store_root = safe_relative_path(recipe.get("checksumStoreRoot"), "checksumStoreRoot")
        if store_root != CHECKSUM_STORE_ROOT:
            raise ValueError(f"checksumStoreRoot must be {CHECKSUM_STORE_ROOT}")
    assets = recipe.get("assets")
    if not isinstance(assets, list) or not assets:
        raise ValueError("assets must be a non-empty list")
    return recipe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--recipe", help="repository-relative JSON recipe to validate")
    parser.add_argument("--emit-template", action="store_true", help="print a sample recipe and exit")
    parser.add_argument(
        "--pipeline-version",
        default=DEFAULT_PIPELINE_VERSION,
        choices=sorted(SUPPORTED_PIPELINE_VERSIONS),
        help="template version to emit with --emit-template",
    )
    args = parser.parse_args()

    if args.emit_template:
        print(json.dumps(TEMPLATES[args.pipeline_version], indent=2, sort_keys=True))
        return
    if not args.recipe:
        parser.error("either --recipe or --emit-template is required")

    recipe_path = safe_relative_path(args.recipe, "recipe")
    recipe = load_recipe(recipe_path)
    pipeline_version = recipe["pipelineVersion"]
    plan = {
        "pipelineVersion": pipeline_version,
        "mode": "dry-run",
        "writesAssets": False,
        "assets": [validate_asset(asset, pipeline_version) for asset in recipe["assets"]],
    }
    print(json.dumps(plan, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
