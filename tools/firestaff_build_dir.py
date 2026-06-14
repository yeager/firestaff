#!/usr/bin/env python3
"""
firestaff_build_dir.py — shared helper for verification scripts
to find the Firestaff build directory without hardcoding a
single path.

Lookup order (first hit wins):
  1. FIRESTAFF_BUILD_DIR env var, if set and the path exists
  2. <root>/build           (in-tree single-config)
  3. <root>/builds          (in-tree multi-config, iterates)
  4. /tmp/firestaff-blockers-build-current
  5. <root> parent walk for any CTestTestfile.cmake
"""
from __future__ import annotations

import os
from pathlib import Path
from typing import Optional


def find_build_dir(
    root: Path,
    env_var: str = "FIRESTAFF_BUILD_DIR",
    sentinel: str = "CTestTestfile.cmake",
) -> Optional[Path]:
    """Return the first existing build directory under the
    Firestaff root, or None.  Caller decides what to do with
    the None (typically a hardcoded fallback to <root>/build
    for backward compat)."""
    env = os.environ.get(env_var)
    if env:
        p = Path(env)
        if (p / sentinel).exists():
            return p
    p = root / "build"
    if (p / sentinel).exists():
        return p
    builds = root / "builds"
    if builds.is_dir():
        for child in sorted(builds.iterdir()):
            if (child / sentinel).exists():
                return child
    p = Path("/tmp/firestaff-blockers-build-current")
    if (p / sentinel).exists():
        return p
    for parent in root.parents:
        candidate = parent / "build"
        if (candidate / sentinel).exists():
            return candidate
        builds_p = parent / "builds"
        if builds_p.is_dir():
            for child in sorted(builds_p.iterdir()):
                if (child / sentinel).exists():
                    return child
    return None


def resolve_build_dir(
    root: Path,
    fallback: Path,
    env_var: str = "FIRESTAFF_BUILD_DIR",
) -> Path:
    """Same as find_build_dir but always returns a Path, falling
    back to `fallback` if nothing matches.  `fallback` is
    typically root / "build"."""
    found = find_build_dir(root, env_var)
    if found is not None:
        return found
    return fallback
