"""Resolve an operator-supplied ReDMCSB Common/Source tree consistently.

ReDMCSB is an audit reference, not a Firestaff runtime or vendored source
dependency.  Verification tools may use the workspace reference when it is
present, or an explicitly configured external checkout.  They must never
manufacture a replacement source tree just to satisfy a source-lock test.
"""
from __future__ import annotations

import os
from pathlib import Path
from typing import Iterable, Optional


_WORKSPACE = Path(__file__).resolve().parents[1]
_LEGACY = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/"
           "ReDMCSB_WIP20210206/Toolchains/Common/Source")
_WORKSPACE_REFERENCE = _WORKSPACE / "reference/redmcsb-20210206/Toolchains/Common/Source"


def find_source_root(required: Iterable[str] = ()) -> Optional[Path]:
    """Return a verified source root, or ``None`` when no real reference exists."""
    configured = os.environ.get("FIRESTAFF_REDMCSB_SOURCE")
    candidates = []
    if configured:
        candidates.append(Path(configured).expanduser())
    candidates.extend((_WORKSPACE_REFERENCE, _LEGACY))
    names = tuple(required)
    for candidate in candidates:
        if candidate.is_dir() and all((candidate / name).is_file() for name in names):
            return candidate.resolve()
    return None


def source_root(required: Iterable[str] = ()) -> Path:
    """Return a verified source root or raise a diagnostic suitable for a gate."""
    root = find_source_root(required)
    if root is not None:
        return root
    suffix = ""
    names = tuple(required)
    if names:
        suffix = " containing " + ", ".join(names)
    raise FileNotFoundError(
        "ReDMCSB Common/Source reference unavailable" + suffix +
        "; set FIRESTAFF_REDMCSB_SOURCE to an operator-maintained checkout")
