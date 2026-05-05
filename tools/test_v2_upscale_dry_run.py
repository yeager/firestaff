#!/usr/bin/env python3
"""Unit tests for tools/v2_upscale_dry_run.py

Runs without external dependencies so clean worker nodes can execute.
"""
from __future__ import annotations

import json
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DRY_RUN_SCRIPT = REPO_ROOT / "tools" / "v2_upscale_dry_run.py"


def run_dry_run(*args: str) -> tuple[int, str, str]:
    """Return (returncode, stdout, stderr)."""
    result = subprocess.run(
        [sys.executable, str(DRY_RUN_SCRIPT), *args],
        capture_output=True,
        text=True,
        timeout=30,
        cwd=str(REPO_ROOT),
    )
    return result.returncode, result.stdout, result.stderr


def make_recipe_file(recipe: dict) -> str:
    """Write recipe to a temp JSON file inside REPO_ROOT and return a relative path."""
    tmp = tempfile.NamedTemporaryFile(mode="w", suffix=".json", delete=False, dir=str(REPO_ROOT))
    json.dump(recipe, tmp)
    tmp.close()
    return str(Path(tmp.name).relative_to(REPO_ROOT))


def test_emit_template_v21() -> None:
    rc, out, err = run_dry_run("--emit-template", "--pipeline-version", "v2.1-upscale-scout")
    assert rc == 0, f"emit-template v2.1 failed: {err}"
    recipe = json.loads(out)
    assert recipe["pipelineVersion"] == "v2.1-upscale-scout"
    assets = recipe["assets"]
    assert len(assets) == 1
    a = assets[0]
    src = a["sourceLogicalSize"]
    mst = a["masterSize"]
    drv = a["derivedSize"]
    # 10x rule
    assert mst["width"] == src["width"] * 10
    assert mst["height"] == src["height"] * 10
    # half rule
    assert drv["width"] == mst["width"] // 2
    assert drv["height"] == mst["height"] // 2


def test_emit_template_v22() -> None:
    rc, out, err = run_dry_run("--emit-template", "--pipeline-version", "v2.2-enhanced-4k1080")
    assert rc == 0, f"emit-template v2.2 failed: {err}"
    recipe = json.loads(out)
    assert recipe["pipelineVersion"] == "v2.2-enhanced-4k1080"
    assert "checksumStoreRoot" in recipe
    a = recipe["assets"][0]
    assert a["assetLineage"] == "v2.2-enhanced"
    assert isinstance(a["provenance"], dict)
    for key in ("sourceKind", "sourceNotes", "operator", "toolchain", "licenseNotes"):
        assert key in a["provenance"], f"provenance missing {key}"


def test_example_recipe() -> None:
    rc, out, err = run_dry_run("--recipe", "docs/v2/v2-upscale-recipe.example.json")
    assert rc == 0, f"example recipe failed: {err}"
    plan = json.loads(out)
    assert plan["mode"] == "dry-run"
    assert plan["writesAssets"] is False
    assert len(plan["assets"]) == 1
    plan_a = plan["assets"][0]
    assert plan_a["writesAssets"] is False
    # Checksum store keys would be populated
    assert "wouldStore" in plan_a
    assert "master" in plan_a["wouldStore"]
    assert "derived" in plan_a["wouldStore"]
    for role_key in ("master", "derived"):
        key = plan_a["wouldStore"][role_key]
        assert key.startswith("sha256/"), f"store key should start with sha256/: {key}"


def test_recipe_wrong_master_size() -> None:
    """Master size must be exactly 10x sourceLogicalSize."""
    recipe = {
        "pipelineVersion": "v2.1-upscale-scout",
        "assets": [
            {
                "logicalId": "fs.v2.ui.test",
                "sourceLogicalSize": {"width": 100, "height": 50},
                "masterSize": {"width": 900, "height": 500},  # wrong width
                "derivedSize": {"width": 450, "height": 250},
                "masterPath": "assets-v2/test/masters/m.png",
                "derivedPath": "assets-v2/test/exports/d.png",
            }
        ],
    }
    rel_path = make_recipe_file(recipe)
    try:
        rc, out, err = run_dry_run("--recipe", rel_path)
        assert rc != 0, "should have rejected wrong master size"
        assert "masterSize must be exactly 10x" in err.lower() or "mastersize" in err.lower()
    finally:
        Path(REPO_ROOT / rel_path).unlink(missing_ok=True)


def test_recipe_derived_not_half() -> None:
    """Derived size must be exactly half of master."""
    recipe = {
        "pipelineVersion": "v2.1-upscale-scout",
        "assets": [
            {
                "logicalId": "fs.v2.ui.test",
                "sourceLogicalSize": {"width": 100, "height": 50},
                "masterSize": {"width": 1000, "height": 500},
                "derivedSize": {"width": 400, "height": 250},  # 400 != 500
                "masterPath": "assets-v2/test/masters/m.png",
                "derivedPath": "assets-v2/test/exports/d.png",
            }
        ],
    }
    rel_path = make_recipe_file(recipe)
    try:
        rc, out, _ = run_dry_run("--recipe", rel_path)
        assert rc != 0, "should have rejected wrong derived size"
    finally:
        Path(REPO_ROOT / rel_path).unlink(missing_ok=True)


def test_recipe_absolute_master_path() -> None:
    """Master path must be repository-relative."""
    recipe = {
        "pipelineVersion": "v2.1-upscale-scout",
        "assets": [
            {
                "logicalId": "fs.v2.ui.test",
                "sourceLogicalSize": {"width": 10, "height": 10},
                "masterSize": {"width": 100, "height": 100},
                "derivedSize": {"width": 50, "height": 50},
                "masterPath": "/absolute/path/to/master.png",
                "derivedPath": "assets-v2/test/exports/d.png",
            }
        ],
    }
    rel_path = make_recipe_file(recipe)
    try:
        rc, out, err = run_dry_run("--recipe", rel_path)
        assert rc != 0, "should reject absolute path"
        assert "absolute" in err.lower() or "repository-relative" in err.lower()
    finally:
        Path(REPO_ROOT / rel_path).unlink(missing_ok=True)


def test_recipe_escape_via_dotdot() -> None:
    """Path with .. must be rejected."""
    recipe = {
        "pipelineVersion": "v2.1-upscale-scout",
        "assets": [
            {
                "logicalId": "fs.v2.ui.test",
                "sourceLogicalSize": {"width": 10, "height": 10},
                "masterSize": {"width": 100, "height": 100},
                "derivedSize": {"width": 50, "height": 50},
                "masterPath": "assets-v2/test/../../../etc/passwd",
                "derivedPath": "assets-v2/test/exports/d.png",
            }
        ],
    }
    rel_path = make_recipe_file(recipe)
    try:
        rc, out, err = run_dry_run("--recipe", rel_path)
        assert rc != 0, "should reject path with .."
    finally:
        Path(REPO_ROOT / rel_path).unlink(missing_ok=True)


def test_recipe_unknown_pipeline_version() -> None:
    """Only known pipeline versions are accepted."""
    recipe = {
        "pipelineVersion": "unknown-version",
        "assets": [
            {
                "logicalId": "fs.v2.ui.test",
                "sourceLogicalSize": {"width": 10, "height": 10},
                "masterSize": {"width": 100, "height": 100},
                "derivedSize": {"width": 50, "height": 50},
                "masterPath": "assets-v2/test/masters/m.png",
                "derivedPath": "assets-v2/test/exports/d.png",
            }
        ],
    }
    rel_path = make_recipe_file(recipe)
    try:
        rc, out, err = run_dry_run("--recipe", rel_path)
        assert rc != 0, "should reject unknown pipeline version"
    finally:
        Path(REPO_ROOT / rel_path).unlink(missing_ok=True)


def test_recipe_v22_requires_provenance_dict() -> None:
    """V2.2 Enhanced must have a provenance dict with required keys."""
    recipe = {
        "pipelineVersion": "v2.2-enhanced-4k1080",
        "checksumStoreRoot": "assets-v2/store",
        "assets": [
            {
                "logicalId": "fs.v2.2.ui.test.enhanced",
                "assetLineage": "v2.2-enhanced",
                "sourceLogicalSize": {"width": 10, "height": 10},
                "masterSize": {"width": 100, "height": 100},
                "derivedSize": {"width": 50, "height": 50},
                "masterPath": "assets-v2/v2.2-enhanced/ui/test/masters/4k/m.png",
                "derivedPath": "assets-v2/v2.2-enhanced/ui/test/exports/1080p/d.png",
                "provenance": "not-a-dict",
            }
        ],
    }
    rel_path = make_recipe_file(recipe)
    try:
        rc, out, err = run_dry_run("--recipe", rel_path)
        assert rc != 0, "should reject v2.2 recipe with non-dict provenance"
    finally:
        Path(REPO_ROOT / rel_path).unlink(missing_ok=True)


def test_recipe_v22_path_must_not_share_v21_location() -> None:
    """V2.2 Enhanced paths cannot live under /ui/wave1/."""
    recipe = {
        "pipelineVersion": "v2.2-enhanced-4k1080",
        "checksumStoreRoot": "assets-v2/store",
        "assets": [
            {
                "logicalId": "fs.v2.2.ui.test.enhanced",
                "assetLineage": "v2.2-enhanced",
                "sourceLogicalSize": {"width": 10, "height": 10},
                "masterSize": {"width": 100, "height": 100},
                "derivedSize": {"width": 50, "height": 50},
                "masterPath": "assets-v2/v2.2-enhanced/ui/wave1/test/masters/m.png",
                "derivedPath": "assets-v2/v2.2-enhanced/ui/test/exports/1080p/d.png",
                "provenance": {
                    "sourceKind": "enhanced-clean-room-master",
                    "sourceNotes": "test",
                    "operator": "test",
                    "toolchain": "test",
                    "licenseNotes": "test",
                },
            }
        ],
    }
    rel_path = make_recipe_file(recipe)
    try:
        rc, out, err = run_dry_run("--recipe", rel_path)
        assert rc != 0, "should reject v2.2 path that reuses wave1 location"
    finally:
        Path(REPO_ROOT / rel_path).unlink(missing_ok=True)


def test_recipe_v22_source_kind_must_not_be_v21() -> None:
    """V2.2 provenance.sourceKind must not be v2.1-upscaled-original."""
    recipe = {
        "pipelineVersion": "v2.2-enhanced-4k1080",
        "checksumStoreRoot": "assets-v2/store",
        "assets": [
            {
                "logicalId": "fs.v2.2.ui.test.enhanced",
                "assetLineage": "v2.2-enhanced",
                "sourceLogicalSize": {"width": 10, "height": 10},
                "masterSize": {"width": 100, "height": 100},
                "derivedSize": {"width": 50, "height": 50},
                "masterPath": "assets-v2/v2.2-enhanced/ui/test/masters/4k/m.png",
                "derivedPath": "assets-v2/v2.2-enhanced/ui/test/exports/1080p/d.png",
                "provenance": {
                    "sourceKind": "v2.1-upscaled-original",
                    "sourceNotes": "test",
                    "operator": "test",
                    "toolchain": "test",
                    "licenseNotes": "test",
                },
            }
        ],
    }
    rel_path = make_recipe_file(recipe)
    try:
        rc, out, err = run_dry_run("--recipe", rel_path)
        assert rc != 0, "should reject v2.2 recipe with forbidden sourceKind"
    finally:
        Path(REPO_ROOT / rel_path).unlink(missing_ok=True)


if __name__ == "__main__":
    tests = [v for k, v in sorted(globals().items()) if k.startswith("test_") and callable(v)]
    failed = 0
    for t in tests:
        name = t.__name__
        try:
            t()
            print(f"  PASS  {name}")
        except Exception as exc:
            print(f"  FAIL  {name}: {exc}")
            failed += 1
    print(f"\n{len(tests) - failed}/{len(tests)} passed")
    if failed:
        sys.exit(1)
