#!/usr/bin/env python3
"""Validate a V2.1 upscale recipe without producing assets."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import PurePosixPath
from typing import Any

PIPELINE_VERSION = "v2.1-upscale-scout"

TEMPLATE: dict[str, Any] = {
    "pipelineVersion": PIPELINE_VERSION,
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


def planned_store_key(logical_id: str, role: str, path: str) -> str:
    digest = hashlib.sha256(f"{PIPELINE_VERSION}\0{logical_id}\0{role}\0{path}".encode("utf-8")).hexdigest()
    return f"sha256/{digest[:2]}/{digest}"


def validate_asset(asset: Any) -> dict[str, Any]:
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

    return {
        "logicalId": logical_id,
        "sourceLogicalSize": [source_w, source_h],
        "masterSize": [master_w, master_h],
        "derivedSize": [derived_w, derived_h],
        "wouldReadMaster": master_path,
        "wouldWriteDerived": derived_path,
        "wouldStore": {
            "master": planned_store_key(logical_id, "master", master_path),
            "derived": planned_store_key(logical_id, "derived", derived_path),
        },
        "writesAssets": False,
    }


def load_recipe(path: str) -> dict[str, Any]:
    with open(path, "r", encoding="utf-8") as handle:
        recipe = json.load(handle)
    if not isinstance(recipe, dict):
        raise ValueError("recipe must be a JSON object")
    if recipe.get("pipelineVersion") != PIPELINE_VERSION:
        raise ValueError(f"pipelineVersion must be {PIPELINE_VERSION}")
    assets = recipe.get("assets")
    if not isinstance(assets, list) or not assets:
        raise ValueError("assets must be a non-empty list")
    return recipe


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--recipe", help="repository-relative JSON recipe to validate")
    parser.add_argument("--emit-template", action="store_true", help="print a sample recipe and exit")
    args = parser.parse_args()

    if args.emit_template:
        print(json.dumps(TEMPLATE, indent=2, sort_keys=True))
        return
    if not args.recipe:
        parser.error("either --recipe or --emit-template is required")

    recipe_path = safe_relative_path(args.recipe, "recipe")
    recipe = load_recipe(recipe_path)
    plan = {
        "pipelineVersion": PIPELINE_VERSION,
        "mode": "dry-run",
        "writesAssets": False,
        "assets": [validate_asset(asset) for asset in recipe["assets"]],
    }
    print(json.dumps(plan, indent=2, sort_keys=True))


if __name__ == "__main__":
    main()
