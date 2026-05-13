#!/usr/bin/env python3
"""Pass504: lock the DM1 V1 viewport wall render-plan promotion boundary.

This gate sits after pass502/pass503. It does not promote pixel parity. It proves
that the current comparator-ready render-plan artifact is still ordered by the
ReDMCSB F0128 far-to-near sequence and that the remaining blocker is the exact
same-viewport original/Firestaff runtime capture described by pass502.
"""
from __future__ import annotations

import hashlib
import json
import subprocess
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS502_REPORT = ROOT / "parity-evidence/pass502_dm1_v1_viewport_wall_occlusion_audit.md"
PASS503_MANIFEST = ROOT / "parity-evidence/verification/pass503_dm1_v1_viewport_wall_draw_order_evidence/manifest.json"
RENDER_PLAN_MANIFEST = ROOT / "parity-evidence/verification/dm1_v1_viewport_wall_render_plan_gate.json"
MANIFEST = ROOT / "parity-evidence/verification/pass504_dm1_v1_viewport_wall_render_plan_promotion_boundary/manifest.json"
REPORT = ROOT / "parity-evidence/pass504_dm1_v1_viewport_wall_render_plan_promotion_boundary.md"

EXPECTED_STATUS = "PASS504_DM1_V1_VIEWPORT_WALL_RENDER_PLAN_PROMOTION_BOUNDARY"
PASS503_STATUS = "PASS_PASS503_DM1_V1_VIEWPORT_WALL_DRAW_ORDER_EVIDENCE"
RENDER_PLAN_STATUS = "PASS_RENDER_PLAN_COMPARATOR_SEAM"

DRAW_ORDER = [
    "D4L",
    "D4R",
    "D4C",
    "D3L2",
    "D3R2",
    "D3L",
    "D3R",
    "D3C",
    "D2L2",
    "D2R2",
    "D2L",
    "D2R",
    "D2C",
    "D1L",
    "D1R",
    "D1C",
    "D0L",
    "D0R",
    "D0C",
]
ORDER_RANK = {row: index for index, row in enumerate(DRAW_ORDER)}

REQUIRED_PASS503_SOURCE_IDS = {
    "f0128-far-to-near-square-replay",
    "f0115-cell-layering-contract",
    "wall-blit-routes-into-viewport-buffer",
    "d3-wall-return-and-alcove-exception",
    "d2-d1-d0-wall-return-contract",
    "front-door-two-pass-occlusion",
    "viewport-present-boundary",
}

REQUIRED_PASS502_BLOCKER_TEXT = [
    "Required next evidence before parity promotion:",
    "Capture one canonical DM1 V1 original viewport frame",
    "Capture the matching Firestaff frame",
    "Record the Firestaff runtime path reaching the F0128-to-F0097 present boundary",
    "Attach a pixel/crop comparison manifest",
]


def run_prerequisite(cmd: list[str]) -> dict[str, Any]:
    result = subprocess.run(cmd, cwd=ROOT, text=True, capture_output=True, timeout=240)
    return {
        "cmd": cmd,
        "returncode": result.returncode,
        "stdoutTail": result.stdout[-2000:],
        "stderrTail": result.stderr[-2000:],
    }


def read_json(path: Path) -> dict[str, Any]:
    if not path.exists():
        raise AssertionError(f"missing required manifest: {path.relative_to(ROOT)}")
    return json.loads(path.read_text(encoding="utf-8"))


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def require(condition: bool, message: str, failures: list[str]) -> None:
    if not condition:
        failures.append(message)


def validate_pass502(failures: list[str]) -> dict[str, Any]:
    text = PASS502_REPORT.read_text(encoding="utf-8", errors="replace")
    missing = [needle for needle in REQUIRED_PASS502_BLOCKER_TEXT if needle not in text]
    require(not missing, f"pass502 blocker report missing {missing!r}", failures)
    return {
        "file": str(PASS502_REPORT.relative_to(ROOT)),
        "sha256": sha256(PASS502_REPORT),
        "missingRequiredText": missing,
        "promotionRule": "same-viewport original and Firestaff capture remains required before pixel parity",
    }


def validate_pass503(manifest: dict[str, Any], failures: list[str]) -> dict[str, Any]:
    require(manifest.get("status") == PASS503_STATUS, "pass503 manifest status is not pass", failures)
    require(manifest.get("ok") is True, "pass503 manifest ok flag is not true", failures)
    source_checks = manifest.get("sourceChecks", [])
    by_id = {row.get("id"): row for row in source_checks}
    missing_ids = sorted(REQUIRED_PASS503_SOURCE_IDS - set(by_id))
    require(not missing_ids, f"pass503 missing source ids {missing_ids!r}", failures)
    failed_ids = sorted(row.get("id") for row in source_checks if not row.get("ok"))
    require(not failed_ids, f"pass503 has failed source checks {failed_ids!r}", failures)
    return {
        "file": str(PASS503_MANIFEST.relative_to(ROOT)),
        "sha256": sha256(PASS503_MANIFEST),
        "status": manifest.get("status"),
        "sourceCheckCount": len(source_checks),
        "requiredSourceIdsPresent": not missing_ids,
    }


def validate_d4_event(event: dict[str, Any], failures: list[str], snapshot: str) -> None:
    row = event.get("row")
    require(event.get("event") == "draw_d4_f0115_object_stack", f"{snapshot}:{row} D4 event has wrong type", failures)
    require(event.get("localWallSpecRequired") is False, f"{snapshot}:{row} D4 event unexpectedly requires a wall spec", failures)
    require(event.get("pixelRegion") is None, f"{snapshot}:{row} D4 event unexpectedly has a wall pixel region", failures)
    require(event.get("cellOrder") == "C0x0001_CELL_ORDER_BACKLEFT", f"{snapshot}:{row} D4 cell order drifted", failures)
    require(str(event.get("source", "")).startswith("DUNVIEW.C:84"), f"{snapshot}:{row} D4 source citation drifted", failures)


def validate_wall_event(event: dict[str, Any], failures: list[str], snapshot: str) -> None:
    row = event.get("row")
    require(event.get("event") == "draw_wall_bitmap", f"{snapshot}:{row} wall event has wrong type", failures)
    require(str(event.get("source", "")).startswith("DUNVIEW.C:"), f"{snapshot}:{row} missing DUNVIEW source citation", failures)
    region = event.get("pixelRegion")
    require(isinstance(region, dict), f"{snapshot}:{row} missing pixel region", failures)
    if not isinstance(region, dict):
        return
    require(region.get("kind") == "expected_wall_bitmap_region", f"{snapshot}:{row} wrong pixel region kind", failures)
    require(region.get("layout") == 696, f"{snapshot}:{row} wrong layout id", failures)
    require(region.get("selectedWall") == event.get("selectedWall"), f"{snapshot}:{row} selected wall mismatch", failures)
    viewport = region.get("viewport", {})
    require(viewport.get("width") == 224 and viewport.get("height") == 136, f"{snapshot}:{row} viewport dimensions drifted", failures)
    rect = region.get("rect", {})
    require(0 <= int(rect.get("x", -1)) < 224, f"{snapshot}:{row} bad rect x", failures)
    require(0 <= int(rect.get("y", -1)) < 136, f"{snapshot}:{row} bad rect y", failures)
    require(0 < int(rect.get("width", 0)) <= 224 - int(rect.get("x", 224)), f"{snapshot}:{row} bad rect width", failures)
    require(0 < int(rect.get("height", 0)) <= 136 - int(rect.get("y", 136)), f"{snapshot}:{row} bad rect height", failures)
    graphic = int(region.get("wallSetGraphicIndex", -1))
    require(93 <= graphic <= 107, f"{snapshot}:{row} wall-set graphic index out of PC34 wall range", failures)


def validate_snapshot(snapshot: dict[str, Any], failures: list[str]) -> dict[str, Any]:
    name = str(snapshot.get("name"))
    events = snapshot.get("renderEvents", [])
    require(bool(events), f"{name} has no render events", failures)
    rows = [event.get("row") for event in events]
    unknown_rows = [row for row in rows if row not in ORDER_RANK]
    require(not unknown_rows, f"{name} has unknown rows {unknown_rows!r}", failures)
    ordered_rows = [row for row in rows if row in ORDER_RANK]
    rank_sequence = [ORDER_RANK[row] for row in ordered_rows]
    require(rank_sequence == sorted(rank_sequence), f"{name} render events are not F0128 ordered: {ordered_rows!r}", failures)
    d4_rows = []
    wall_rows = []
    for event in events:
        row = event.get("row")
        if row not in ORDER_RANK:
            continue
        if str(row).startswith("D4"):
            d4_rows.append(row)
            validate_d4_event(event, failures, name)
        else:
            wall_rows.append(row)
            validate_wall_event(event, failures, name)
    require(snapshot.get("d4ObjectRows", []) == d4_rows, f"{name} d4ObjectRows does not match render events", failures)
    expected_rows = set(snapshot.get("expectedWallRows", []))
    event_rows = set(rows)
    require(expected_rows == event_rows, f"{name} expectedWallRows does not match render event rows", failures)
    return {
        "name": name,
        "party": snapshot.get("party"),
        "eventCount": len(events),
        "d4ObjectRows": d4_rows,
        "wallRows": wall_rows,
        "orderedRows": ordered_rows,
    }


def validate_render_plan(manifest: dict[str, Any], failures: list[str]) -> dict[str, Any]:
    require(manifest.get("status") == RENDER_PLAN_STATUS, "render-plan manifest status is not pass", failures)
    require(manifest.get("unsupportedWallRows") == [], "render-plan has unsupported wall rows", failures)
    boundary = manifest.get("boundary", {})
    require(boundary.get("remainingBlocker") is None, "render-plan has an internal remaining blocker", failures)
    require("not claim" not in str(boundary.get("nowProduced", "")).lower(), "render-plan nowProduced text is malformed", failures)
    not_claimed = str(boundary.get("notClaimed", ""))
    require("original pixel parity" in not_claimed, "render-plan non-claim missing original pixel parity", failures)
    d4_audit = manifest.get("d4SourceAudit", {})
    d4_predicate = str(d4_audit.get("predicate", ""))
    require("D4L/D4R/D4C" in d4_predicate, "D4 source audit row set drifted", failures)
    require("wall bitmap/zone draw call" in d4_predicate, "D4 source audit predicate drifted", failures)
    snapshots = [validate_snapshot(snapshot, failures) for snapshot in manifest.get("comparedSnapshots", [])]
    require(len(snapshots) >= 5, "render-plan covers fewer than five pass127 snapshots", failures)
    return {
        "file": str(RENDER_PLAN_MANIFEST.relative_to(ROOT)),
        "sha256": sha256(RENDER_PLAN_MANIFEST),
        "status": manifest.get("status"),
        "unsupportedWallRows": manifest.get("unsupportedWallRows"),
        "snapshotSummaries": snapshots,
        "nonClaim": not_claimed,
    }


def main() -> int:
    prerequisite_runs = [
        run_prerequisite([sys.executable, "tools/verify_pass503_dm1_v1_viewport_wall_draw_order_evidence.py"]),
        run_prerequisite([sys.executable, "tools/verify_dm1_v1_viewport_wall_render_plan_gate.py"]),
    ]

    failures: list[str] = []
    for run in prerequisite_runs:
        require(run["returncode"] == 0, f"prerequisite failed: {run['cmd']}", failures)

    pass503 = read_json(PASS503_MANIFEST)
    render_plan = read_json(RENDER_PLAN_MANIFEST)

    pass502_result = validate_pass502(failures)
    pass503_result = validate_pass503(pass503, failures)
    render_result = validate_render_plan(render_plan, failures)

    status = EXPECTED_STATUS if not failures else "FAIL_PASS504_DM1_V1_VIEWPORT_WALL_RENDER_PLAN_PROMOTION_BOUNDARY"
    manifest = {
        "schema": "firestaff.parity.pass504_dm1_v1_viewport_wall_render_plan_promotion_boundary.v1",
        "status": status,
        "ok": not failures,
        "prerequisites": prerequisite_runs,
        "pass502Blocker": pass502_result,
        "pass503SourceLocks": pass503_result,
        "renderPlanBoundary": render_result,
        "promotionBlocker": "Do not claim pixel parity until one exact original DM1 V1 viewport frame and matching Firestaff frame are captured for the same map/x/y/direction/wall-door state and tied to the F0128-to-F0097 present boundary.",
        "nonClaims": [
            "no original-vs-Firestaff pixel parity promotion",
            "no new runtime capture",
            "no movement-core edits",
            "no pass435 capture-route edits",
        ],
        "failures": failures,
    }

    MANIFEST.parent.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass504 - DM1 V1 viewport wall render-plan promotion boundary",
        "",
        f"Status: {status}",
        "",
        "## What is locked",
        "",
        "- pass502 still defines the exact promotion blocker: same-viewport original and Firestaff runtime capture.",
        "- pass503 source locks are present and green for F0128 draw order, F0115 layering, wall/door blits, wall returns, front-door two-pass occlusion, and F0097 present.",
        "- The render-plan artifact has no unsupported wall rows and every snapshot's render events follow the ReDMCSB F0128 order.",
        "- D4 rows remain F0115 object-stack events, not invented wall bitmap rows.",
        "",
        "## Snapshot coverage",
        "",
    ]
    for snapshot in render_result["snapshotSummaries"]:
        lines.append(
            f"- {snapshot['name']}: events={snapshot['eventCount']} "
            f"d4={snapshot['d4ObjectRows']} walls={snapshot['wallRows']}"
        )
    lines += [
        "",
        "## Promotion blocker",
        "",
        manifest["promotionBlocker"],
        "",
        "## Non-claims",
        "",
    ]
    lines.extend(f"- {item}" for item in manifest["nonClaims"])
    if failures:
        lines += ["", "## Failures", ""]
        lines.extend(f"- {failure}" for failure in failures)
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(status)
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
