#!/usr/bin/env python3
"""Report verified Firestaff completion status without inventing percentages.

The output is intentionally an exact eight-row matrix, one row for each tracked
family/version pair. A completion percent is printed only when a repository
artifact contains a verified completionPercent for that row. Otherwise the row is
`okänd` and `ej verifierad`, with concrete evidence counts so the report remains
useful without guessing.
"""
from __future__ import annotations

import argparse
import json
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable

ROOT = Path(__file__).resolve().parents[1]


@dataclass(frozen=True)
class Target:
    name: str
    module_globs: tuple[str, ...]
    test_globs: tuple[str, ...]
    tool_needles: tuple[str, ...]
    manifest_globs: tuple[str, ...] = ()
    matrix_artifacts: tuple[Path, ...] = ()


TARGETS: tuple[Target, ...] = (
    Target(
        "DM1 V1",
        ("dm1_v1_*.c", "m11_v1_*.c"),
        ("test_dm1_v1*.c", "test_m11_v1*.c"),
        ("dm1_v1", "v1_", "m11_v1"),
    ),
    Target(
        "DM1 V2",
        ("dm1_v2_*_pc34.c",),
        ("test_dm1_v2*.c",),
        ("dm1_v2", "v2_"),
        ("assets-v2/manifests/firestaff-v2-*.manifest.json",),
        (Path("parity-evidence/verification/dm1_v2_completion_matrix.json"),),
    ),
    Target(
        "CSB V1",
        ("csb_v1_*.c",),
        ("test_csb_v1*.c", "test_csb*.c"),
        ("csb",),
    ),
    Target(
        "CSB V2",
        ("csb_v2_*.c",),
        ("test_csb_v2*.c",),
        ("csb_v2",),
    ),
    Target(
        "DM2 V1",
        ("dm2_v1_*.c",),
        ("test_dm2_v1*.c", "test_dm2*.c"),
        ("dm2",),
    ),
    Target(
        "DM2 V2",
        ("dm2_v2_*.c",),
        ("test_dm2_v2*.c",),
        ("dm2_v2",),
    ),
    Target(
        "DM Nexus V1",
        ("dm_nexus_v1_*.c", "nexus_v1_*.c"),
        ("test_dm_nexus_v1*.c", "test_nexus_v1*.c"),
        ("nexus",),
    ),
    Target(
        "DM Nexus V2",
        ("dm_nexus_v2_*.c", "nexus_v2_*.c"),
        ("test_dm_nexus_v2*.c", "test_nexus_v2*.c"),
        ("nexus_v2",),
    ),
)


def unique_paths(patterns: Iterable[str], base: Path = ROOT) -> list[Path]:
    paths: set[Path] = set()
    for pattern in patterns:
        paths.update(base.glob(pattern))
    return sorted(paths, key=lambda p: str(p.relative_to(ROOT)))


def tool_count(needles: Iterable[str]) -> int:
    tools_dir = ROOT / "tools"
    if not tools_dir.is_dir():
        return 0
    lowered = tuple(n.lower() for n in needles)
    return sum(1 for p in tools_dir.iterdir() if p.is_file() and any(n in p.name.lower() for n in lowered))


def load_json(path: Path) -> Any | None:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return None


def artifact_status(paths: Iterable[Path]) -> tuple[str, list[str]]:
    statuses: list[str] = []
    evidence: list[str] = []
    for rel in paths:
        path = ROOT / rel
        data = load_json(path)
        if not isinstance(data, dict):
            continue
        status = data.get("status")
        if isinstance(status, str):
            statuses.append(status)
            evidence.append(f"matrix={status}")
        for key in ("moduleCount", "testCount", "manifestCount"):
            value = data.get(key)
            if isinstance(value, int):
                evidence.append(f"{key}={value}")
    if any(s.lower() in {"failed", "error"} for s in statuses):
        return "ej verifierad", evidence
    if any(s.lower() in {"passed", "pass", "ok"} for s in statuses):
        return "ej verifierad", evidence
    return "ej verifierad", evidence


def verified_marker(value: Any) -> bool:
    if value is True:
        return True
    if isinstance(value, str) and value.lower() in {"verified", "passed", "pass", "ok"}:
        return True
    return False


def iter_dicts(value: Any) -> Iterable[dict[str, Any]]:
    if isinstance(value, dict):
        yield value
        for child in value.values():
            yield from iter_dicts(child)
    elif isinstance(value, list):
        for child in value:
            yield from iter_dicts(child)


def row_matches(row: dict[str, Any], target_name: str) -> bool:
    haystack = " ".join(str(v) for v in row.values() if isinstance(v, (str, int, float))).lower()
    return all(part.lower() in haystack for part in target_name.split())


def find_verified_completion_percent(target: Target) -> str | None:
    roots = [ROOT / "parity-evidence", ROOT / "verification-m11", ROOT / "verification-m12", ROOT / "verification-m13", ROOT / "data"]
    for root in roots:
        if not root.exists():
            continue
        for path in root.rglob("*.json"):
            data = load_json(path)
            if data is None:
                continue
            for row in iter_dicts(data):
                if "completionPercent" not in row:
                    continue
                if not row_matches(row, target.name):
                    continue
                markers = (row.get("verified"), row.get("verification"), row.get("status"), row.get("matrixStatus"))
                if not any(verified_marker(marker) for marker in markers):
                    continue
                value = row["completionPercent"]
                if isinstance(value, (int, float)):
                    return f"{value:g}%"
                if isinstance(value, str) and value.strip():
                    return value.strip()
    return None


def build_row(target: Target) -> str:
    modules = unique_paths(target.module_globs)
    tests = unique_paths(target.test_globs)
    manifests = unique_paths(target.manifest_globs)
    tools = tool_count(target.tool_needles)
    percent = find_verified_completion_percent(target)
    status, artifact_evidence = artifact_status(target.matrix_artifacts)

    completion = percent if percent is not None else "okänd"
    if percent is not None:
        status = "verifierad"

    evidence = [f"moduleCount={len(modules)}", f"tests={len(tests)}", f"tools={tools}"]
    if target.manifest_globs:
        evidence.append(f"manifests={len(manifests)}")
    evidence.extend(artifact_evidence)
    # Keep rows compact and deterministic while preserving all requested evidence.
    evidence_text = "; ".join(dict.fromkeys(evidence))
    return f"{target.name} | completionPercent={completion} | status={status} | evidens: {evidence_text}"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=ROOT, help="accepted for compatibility; script uses its own repository root")
    args = parser.parse_args()
    if args.repo.resolve() != ROOT.resolve():
        parser.error(f"--repo must be this checkout: {ROOT}")

    for target in TARGETS:
        print(build_row(target))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
