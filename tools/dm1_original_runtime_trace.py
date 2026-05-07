#!/usr/bin/env python3
"""DM1 PC34 original runtime trace adapter scaffold.

This tool is intentionally conservative: it validates the runtime-image fixture
and symbol map, then refuses to emit promoted trace events until every requested
seam has a verified runtime CS:IP/global binding.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

REPO = Path(__file__).resolve().parents[1]
DEFAULT_RUNTIME = REPO / "data/original_runtime/dm1_pc34_i34e_runtime_image.v1.json"
DEFAULT_SYMBOL_MAP = REPO / "data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json"

REQUIRED_EVENTS = [
    "command_accepted",
    "turn_or_step_state_applied",
    "party_coordinates_committed",
    "draw_uses_mutated_tuple",
    "viewport_buffer_composed",
    "viewport_present",
]


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def validate_runtime_image(runtime: dict[str, Any], runtime_json: Path) -> list[dict[str, Any]]:
    issues: list[dict[str, Any]] = []
    dec = runtime.get("decompressed_fires", {})
    raw_path = dec.get("path")
    if not raw_path:
        return [{"id": "decompressed_fires", "reason": "missing_path"}]
    path = Path(raw_path)
    if not path.is_absolute():
        path = runtime_json.parent.parent.parent / path
    if not path.exists():
        return [{"id": "decompressed_fires", "reason": "file_missing", "path": str(path)}]
    expected_size = dec.get("size")
    actual_size = path.stat().st_size
    if expected_size is not None and actual_size != expected_size:
        issues.append({"id": "decompressed_fires", "reason": "size_mismatch", "path": str(path), "expected": expected_size, "actual": actual_size})
    expected_sha = dec.get("sha256")
    actual_sha = sha256(path)
    if expected_sha and actual_sha != expected_sha:
        issues.append({"id": "decompressed_fires", "reason": "sha256_mismatch", "path": str(path), "expected": expected_sha, "actual": actual_sha})
    return issues


def unresolved_entries(symbol_map: dict[str, Any]) -> list[dict[str, Any]]:
    unresolved = []
    by_id = {entry.get("id"): entry for entry in symbol_map.get("entries", [])}
    for event_id in REQUIRED_EVENTS:
        entry = by_id.get(event_id)
        if not entry:
            unresolved.append({"id": event_id, "reason": "missing_symbol_map_entry"})
            continue
        if entry.get("confidence") != "verified_runtime_hit" or not entry.get("runtime_cs_ip"):
            unresolved.append({
                "id": event_id,
                "reason": "runtime_hit_required",
                "confidence": entry.get("confidence"),
                "source_citation": entry.get("source_citation"),
                "hook_kind": entry.get("hook_kind"),
            })
    return unresolved


def main() -> int:
    ap = argparse.ArgumentParser(description="Validate or run the DM1 original runtime trace adapter scaffold")
    ap.add_argument("--runtime-image", type=Path, default=DEFAULT_RUNTIME)
    ap.add_argument("--symbol-map", type=Path, default=DEFAULT_SYMBOL_MAP)
    ap.add_argument("--route", default="turn_left_once")
    ap.add_argument("--out", type=Path, help="Optional trace output path; written only after symbols are verified")
    ap.add_argument("--check-only", action="store_true")
    ap.add_argument("--expect-blocked", action="store_true", help="Return success when unresolved runtime-hit bindings are the expected blocker")
    args = ap.parse_args()

    runtime = load_json(args.runtime_image)
    symbol_map = load_json(args.symbol_map)
    runtime_issues = validate_runtime_image(runtime, args.runtime_image)
    missing = runtime_issues + unresolved_entries(symbol_map)
    status = "BLOCKED_RUNTIME_IMAGE_REQUIRED" if runtime_issues else ("BLOCKED_RUNTIME_HITS_REQUIRED" if missing else "READY_FOR_DEBUGGER_TRACE")
    result = {
        "schema": "dm1_original_runtime_trace.check.v1",
        "status": status,
        "runtime_image": str(args.runtime_image),
        "symbol_map": str(args.symbol_map),
        "route": args.route,
        "decompressed_fires_sha256": runtime.get("decompressed_fires", {}).get("sha256"),
        "unresolved": missing,
        "promotion_rule": "emit trace events only after verified_runtime_hit entries exist for command accepted -> movement applied -> viewport buffer composed -> viewport present",
    }
    print(json.dumps(result, indent=2, sort_keys=True))
    if args.out and not missing:
        args.out.write_text("")
    if missing and args.expect_blocked:
        return 0
    return 2 if missing else 0


if __name__ == "__main__":
    raise SystemExit(main())
