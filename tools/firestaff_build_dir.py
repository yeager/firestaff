#!/usr/bin/env python3
"""
firestaff_build_dir.py — shared helper for verification scripts
to find the Firestaff build directory without hardcoding a
single path.

Lookup order (first hit wins):
  1. FIRESTAFF_BUILD_DIR env var, if set and the path exists
  2. <root>/build           (in-tree single-config)
  3. <root>/build-*         (named in-tree single-config builds)
  4. <root>/builds          (in-tree multi-config, iterates)
  4. <root> parent walk for any CTestTestfile.cmake
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
    for child in sorted(root.glob("build-*")):
        if child.is_dir() and (child / sentinel).exists():
            return child
    builds = root / "builds"
    if builds.is_dir():
        for child in sorted(builds.iterdir()):
            if (child / sentinel).exists():
                return child
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


def resolve_build_executable(
    root: Path,
    name: str,
    fallback: Path,
    env_var: str = "FIRESTAFF_BUILD_DIR",
) -> Path:
    """Find a built executable, not merely the first configured build tree.

    Developers may keep a configured ``build/`` tree alongside a complete
    named build.  Source-lock checks must select the tree that actually owns
    their native test binary instead of failing on the empty configured tree.
    """
    env = os.environ.get(env_var)
    candidates: list[Path] = []
    if env:
        candidates.append(Path(env))
    configured = find_build_dir(root, env_var)
    if configured is not None:
        candidates.append(configured)
    candidates.extend(sorted(root.glob("build-*")))
    candidates.append(fallback)
    seen: set[Path] = set()
    for candidate in candidates:
        candidate = candidate.resolve()
        if candidate in seen:
            continue
        seen.add(candidate)
        executable = candidate / name
        if executable.is_file():
            return executable
    return fallback / name
