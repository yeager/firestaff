#!/usr/bin/env python3
"""Validate the Firestaff V2 shared logical asset ID catalog.

This repository-local acceptance gate avoids jsonschema so clean worker nodes can
run it. It validates catalog shape and cross-checks every catalog
existingManifestId against the current V2 manifest asset IDs.
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).resolve().parents[1]
CATALOG_PATH = REPO_ROOT / "assets-v2" / "catalog" / "logical-ids" / "shared-v2-logical-ids.json"
MANIFEST_DIR = REPO_ROOT / "assets-v2" / "manifests"

ALLOWED_GAMES = {"dm1", "csb", "dm2"}
ALLOWED_TRACKS = {"v2.0-original", "v2.1-upscaled", "v2.2-enhanced"}
ALLOWED_STATUS = {"planned", "stubbed", "in-progress", "blocked", "rebuilt"}
ALLOWED_BINDING_KIND = {"manifest-entry", "runtime-symbol", "audio-cue", "layout-slot", "catalog-only"}
REQUIRED_TOP_LEVEL = {"catalogVersion", "scope", "games", "tracks", "idPolicy", "categories"}
REQUIRED_ID_POLICY = {"namespace", "stability", "semantics"}
REQUIRED_CATEGORY = {"categoryId", "displayName", "ownership", "entries"}
REQUIRED_ENTRY = {"logicalId", "role", "sharedBy", "assetClass", "status", "binding"}
REQUIRED_BINDING = {"kind", "expectedBinding"}
LOGICAL_ID_RE = re.compile(r"^fs\.v2\.shared\.([a-z0-9-]+)(\.[a-z0-9-]+)+$")


class ValidationError(Exception):
    pass


def require(condition: bool, message: str) -> None:
    if not condition:
        raise ValidationError(message)


def require_string(value: Any, label: str) -> None:
    require(isinstance(value, str) and value.strip(), f"{label} must be a non-empty string")


def require_string_list(value: Any, label: str, *, allowed: set[str] | None = None, min_items: int = 1) -> list[str]:
    require(isinstance(value, list), f"{label} must be an array")
    require(len(value) >= min_items, f"{label} must contain at least {min_items} item(s)")
    out: list[str] = []
    for index, item in enumerate(value):
        item_label = f"{label}[{index}]"
        require_string(item, item_label)
        if allowed is not None:
            require(item in allowed, f"{item_label} is not allowed: {item}")
        out.append(item)
    require(len(out) == len(set(out)), f"{label} must not contain duplicates")
    return out


def load_manifest_assets(manifests: list[Path]) -> dict[str, dict[str, Any]]:
    assets_by_id: dict[str, dict[str, Any]] = {}
    for manifest in manifests:
        data = json.loads(manifest.read_text(encoding="utf-8"))
        require(isinstance(data, dict), f"{manifest}: manifest must be an object")
        assets = data.get("assets")
        require(isinstance(assets, list), f"{manifest}: assets must be an array")
        for index, asset in enumerate(assets):
            require(isinstance(asset, dict), f"{manifest}: assets[{index}] must be an object")
            asset_id = asset.get("id")
            require_string(asset_id, f"{manifest}: assets[{index}].id")
            # Existing V2 production manifests may intentionally re-list a shared
            # physical asset across family/rollup packs. The catalog gate only
            # needs membership and source-evidence checks, so duplicate IDs keep
            # the first asset object observed by deterministic manifest order.
            assets_by_id.setdefault(asset_id, asset)
    return assets_by_id


def manifest_asset_has_source_evidence(asset: dict[str, Any]) -> bool:
    source = asset.get("sourceReference")
    return isinstance(source, dict) and isinstance(source.get("sourceEvidence"), list) and bool(source["sourceEvidence"])


def validate_binding(binding: Any, label: str, manifest_assets: dict[str, dict[str, Any]]) -> str | None:
    require(isinstance(binding, dict), f"{label}.binding must be an object")
    require(REQUIRED_BINDING <= set(binding), f"{label}.binding missing {sorted(REQUIRED_BINDING - set(binding))}")
    allowed_keys = {"kind", "expectedBinding", "existingManifestId"}
    require(set(binding) <= allowed_keys, f"{label}.binding has unexpected keys {sorted(set(binding) - allowed_keys)}")
    kind = binding["kind"]
    require(kind in ALLOWED_BINDING_KIND, f"{label}.binding.kind is not allowed: {kind}")
    require_string(binding["expectedBinding"], f"{label}.binding.expectedBinding")
    existing = binding.get("existingManifestId")
    if existing is not None:
        require_string(existing, f"{label}.binding.existingManifestId")
        require(existing in manifest_assets, f"{label}.binding.existingManifestId not found in V2 manifests: {existing}")
    return existing


def validate_catalog(catalog_path: Path, manifest_assets: dict[str, dict[str, Any]], required_existing_bindings: set[str], required_bound_source_evidence: set[str]) -> tuple[int, int, int, int]:
    data = json.loads(catalog_path.read_text(encoding="utf-8"))
    require(isinstance(data, dict), f"{catalog_path}: catalog must be an object")
    require(REQUIRED_TOP_LEVEL <= set(data), f"{catalog_path}: missing {sorted(REQUIRED_TOP_LEVEL - set(data))}")
    allowed_top = REQUIRED_TOP_LEVEL | {"description"}
    require(set(data) <= allowed_top, f"{catalog_path}: unexpected top-level keys {sorted(set(data) - allowed_top)}")
    require_string(data["catalogVersion"], "catalogVersion")
    require_string(data["scope"], "scope")
    if "description" in data:
        require_string(data["description"], "description")
    games = set(require_string_list(data["games"], "games", allowed=ALLOWED_GAMES, min_items=3))
    require(games == ALLOWED_GAMES, f"games must include exactly {sorted(ALLOWED_GAMES)}")
    require_string_list(data["tracks"], "tracks", allowed=ALLOWED_TRACKS)

    id_policy = data["idPolicy"]
    require(isinstance(id_policy, dict), "idPolicy must be an object")
    require(REQUIRED_ID_POLICY <= set(id_policy), f"idPolicy missing {sorted(REQUIRED_ID_POLICY - set(id_policy))}")
    require(set(id_policy) <= REQUIRED_ID_POLICY, f"idPolicy has unexpected keys {sorted(set(id_policy) - REQUIRED_ID_POLICY)}")
    require_string(id_policy["namespace"], "idPolicy.namespace")
    require_string(id_policy["stability"], "idPolicy.stability")
    require_string_list(id_policy["semantics"], "idPolicy.semantics")

    categories = data["categories"]
    require(isinstance(categories, list) and categories, "categories must be a non-empty array")
    seen_categories: set[str] = set()
    seen_logical_ids: set[str] = set()
    existing_binding_count = 0
    source_evidence_binding_count = 0
    entry_count = 0
    for c_index, category in enumerate(categories):
        c_label = f"categories[{c_index}]"
        require(isinstance(category, dict), f"{c_label} must be an object")
        require(REQUIRED_CATEGORY <= set(category), f"{c_label} missing {sorted(REQUIRED_CATEGORY - set(category))}")
        allowed_category = REQUIRED_CATEGORY | {"notes"}
        require(set(category) <= allowed_category, f"{c_label} has unexpected keys {sorted(set(category) - allowed_category)}")
        category_id = category["categoryId"]
        require_string(category_id, f"{c_label}.categoryId")
        require(category_id not in seen_categories, f"duplicate categoryId: {category_id}")
        seen_categories.add(category_id)
        require_string(category["displayName"], f"{c_label}.displayName")
        require_string(category["ownership"], f"{c_label}.ownership")
        if "notes" in category:
            require_string_list(category["notes"], f"{c_label}.notes")
        entries = category["entries"]
        require(isinstance(entries, list) and entries, f"{c_label}.entries must be a non-empty array")
        for e_index, entry in enumerate(entries):
            e_label = f"{c_label}.entries[{e_index}]"
            require(isinstance(entry, dict), f"{e_label} must be an object")
            require(REQUIRED_ENTRY <= set(entry), f"{e_label} missing {sorted(REQUIRED_ENTRY - set(entry))}")
            allowed_entry = REQUIRED_ENTRY | {"notes"}
            require(set(entry) <= allowed_entry, f"{e_label} has unexpected keys {sorted(set(entry) - allowed_entry)}")
            logical_id = entry["logicalId"]
            require_string(logical_id, f"{e_label}.logicalId")
            match = LOGICAL_ID_RE.match(logical_id)
            require(match is not None, f"{e_label}.logicalId does not match shared namespace policy: {logical_id}")
            require(match.group(1) == category_id, f"{logical_id} category segment does not match categoryId {category_id}")
            require(logical_id not in seen_logical_ids, f"duplicate logicalId: {logical_id}")
            seen_logical_ids.add(logical_id)
            require_string(entry["role"], f"{e_label}.role")
            require_string_list(entry["sharedBy"], f"{e_label}.sharedBy", allowed=games)
            require_string(entry["assetClass"], f"{e_label}.assetClass")
            require(entry["status"] in ALLOWED_STATUS, f"{e_label}.status is not allowed: {entry['status']}")
            existing = validate_binding(entry["binding"], e_label, manifest_assets)
            if existing is not None:
                existing_binding_count += 1
                if manifest_asset_has_source_evidence(manifest_assets[existing]):
                    source_evidence_binding_count += 1
            if logical_id in required_existing_bindings:
                require(existing is not None, f"{logical_id} must declare binding.existingManifestId")
            if logical_id in required_bound_source_evidence:
                require(existing is not None, f"{logical_id} must declare binding.existingManifestId")
                require(manifest_asset_has_source_evidence(manifest_assets[existing]), f"{logical_id} bound manifest asset lacks sourceReference.sourceEvidence: {existing}")
            if "notes" in entry:
                require_string_list(entry["notes"], f"{e_label}.notes")
            entry_count += 1

    required_logical_ids = required_existing_bindings | required_bound_source_evidence
    missing_required = required_logical_ids - seen_logical_ids
    require(not missing_required, f"required logical IDs not found in catalog: {sorted(missing_required)}")
    return len(categories), entry_count, existing_binding_count, source_evidence_binding_count


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--catalog", type=Path, default=CATALOG_PATH, help="catalog JSON to validate")
    parser.add_argument("--manifest", action="append", type=Path, default=[], help="manifest JSON to include; defaults to all V2 manifests")
    parser.add_argument("--require-existing-manifest-binding", action="append", default=[], metavar="LOGICAL_ID", help="require this logical ID to bind to an existing V2 manifest asset ID")
    parser.add_argument("--require-bound-manifest-source-evidence", action="append", default=[], metavar="LOGICAL_ID", help="require this logical ID to bind to a manifest asset with sourceReference.sourceEvidence")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    manifests = args.manifest or sorted(MANIFEST_DIR.glob("firestaff-v2-*.manifest.json"))
    require(bool(manifests), "no V2 manifest files found")
    manifest_assets = load_manifest_assets(manifests)
    categories, entries, existing_bindings, source_evidence_bindings = validate_catalog(
        args.catalog,
        manifest_assets,
        set(args.require_existing_manifest_binding),
        set(args.require_bound_manifest_source_evidence),
    )
    print(f"validated V2 logical catalog: {categories} categories / {entries} logical IDs / {existing_bindings} manifest bindings / {source_evidence_bindings} source-evidence bindings")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, json.JSONDecodeError, ValidationError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise SystemExit(1)
