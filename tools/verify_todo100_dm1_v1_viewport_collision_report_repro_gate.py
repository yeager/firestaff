#!/usr/bin/env python3
"""TODO100 DM1 V1 viewport/collision report reproducibility gate.

This is a skip-safe capture scaffold for unmanifested viewport/collision
reports. It deliberately does not promote the report to fixed: until a capture
manifest or paired original PC 3.4 evidence is installed, the gate records
BUG_OPEN_CAPTURE_MANIFEST_MISSING and verifies the public TODO/docs still say so.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "todo100_dm1_v1_viewport_collision_report_repro_gate"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
TODO = ROOT / "TODO.md"
GAP_DOC = ROOT / "docs/parity/DM1_V1_CAPTURE_GAP_EVIDENCE.md"
PARITY_MATRIX = ROOT / "docs/parity/PARITY_MATRIX_DM1_V1.md"
RUNBOOK = ROOT / "docs/parity/DM1_V1_ORIGINAL_CAPTURE_RUNBOOK.md"
FIRESTAFF_CAPTURE_PROBE = ROOT / "probes/m11/firestaff_m11_wall_collision_capture_probe.c"

BUG_ROW = (
    "Viewport/collision reports without capture manifests must stay as bugs "
    "until paired original PC 3.4 evidence or a reproducible local probe exists."
)

REQUIRED_GAP_SNIPPETS = [
    "Full original collision transcript | MISSING MISSING",
    "No paired original DOS transcript yet covers wall, door, fakewall",
    "collision cannot be marked globally `MATCHED`",
]

REQUIRED_MATRIX_SNIPPETS = [
    "Keep new wall/collision reports tied to exact route, map/x/y/direction, capture manifests, and post-present viewport evidence.",
]

REQUIRED_RUNBOOK_SNIPPETS = [
    "docs/parity/tools/dosbox_capture_manifest_writer.py",
    "--manifest-out capture_manifest.tsv",
    "verification-screens/capture_manifest_sha256.tsv",
]

REQUIRED_PROBE_SNIPPETS = [
    "firestaff.dm1_v1_wall_collision_runtime_capture.v1",
    "Firestaff deterministic runtime capture with exact state coordinates",
    "03_blocked_west_wall_1_3",
]


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def require_contains(text: str, needle: str, label: str) -> None:
    if needle not in text:
        raise AssertionError(f"{label}: missing {needle!r}")


def candidate_capture_dir() -> Path | None:
    raw = os.environ.get("FIRESTAFF_TODO100_DM1_CAPTURE_DIR", "").strip()
    if not raw:
        return None
    return Path(raw).expanduser()


def inspect_candidate_capture() -> dict[str, Any]:
    cap_dir = candidate_capture_dir()
    if cap_dir is None:
        return {
            "mode": "NO_CAPTURE_DIR_CONFIGURED",
            "path": None,
            "manifest_present": False,
            "classification": "SKIP_SAFE_SCAFFOLD_ONLY",
        }

    manifest = cap_dir / "capture_manifest.tsv"
    sidecar = cap_dir / "capture_manifest.sidecar.json"
    rows: list[str] = []
    headers: list[str] = []
    if manifest.exists():
        lines = manifest.read_text(encoding="utf-8", errors="replace").splitlines()
        data_lines = [line for line in lines if line.strip() and not line.startswith("#")]
        if data_lines:
            headers = data_lines[0].split("\t")
            rows = data_lines[1:]

    required_headers = {"capture_id", "classification", "sha256", "width", "height"}
    missing_headers = sorted(required_headers - set(headers))
    viewport_rows = [
        row for row in rows
        if "collision" in row.lower() or "viewport" in row.lower() or "wall" in row.lower()
    ]
    return {
        "mode": "OPERATOR_CAPTURE_DIR",
        "path": str(cap_dir),
        "manifest_path": str(manifest),
        "manifest_present": manifest.exists(),
        "sidecar_present": sidecar.exists(),
        "headers": headers,
        "missing_required_headers": missing_headers,
        "row_count": len(rows),
        "viewport_collision_candidate_rows": len(viewport_rows),
        "classification": (
            "CANDIDATE_MANIFEST_PRESENT_NOT_PROMOTED"
            if manifest.exists() and not missing_headers and viewport_rows
            else "SKIP_SAFE_SCAFFOLD_ONLY"
        ),
    }


def build_result() -> dict[str, Any]:
    todo = read(TODO)
    gap_doc = read(GAP_DOC)
    parity_matrix = read(PARITY_MATRIX)
    runbook = read(RUNBOOK)
    probe = read(FIRESTAFF_CAPTURE_PROBE)

    require_contains(todo, BUG_ROW, "TODO bug row")
    for snippet in REQUIRED_GAP_SNIPPETS:
        require_contains(gap_doc, snippet, "DM1 capture gap evidence")
    for snippet in REQUIRED_MATRIX_SNIPPETS:
        require_contains(parity_matrix, snippet, "DM1 V1 parity matrix")
    for snippet in REQUIRED_RUNBOOK_SNIPPETS:
        require_contains(runbook, snippet, "DM1 original capture runbook")
    for snippet in REQUIRED_PROBE_SNIPPETS:
        require_contains(probe, snippet, "Firestaff wall/collision capture probe")

    candidate = inspect_candidate_capture()
    promoted = candidate["classification"] == "CANDIDATE_MANIFEST_PRESENT_NOT_PROMOTED"
    status = (
        "BUG_OPEN_CANDIDATE_MANIFEST_PRESENT_REVIEW_REQUIRED"
        if promoted
        else "BUG_OPEN_CAPTURE_MANIFEST_MISSING"
    )
    return {
        "schema": "firestaff.todo100_dm1_v1_viewport_collision_report_repro_gate.v1",
        "status": status,
        "honesty": (
            "This gate is a reproducible skip-safe scaffold only. The viewport/"
            "collision report remains a bug until paired original PC 3.4 evidence "
            "or a reviewed local repro manifest exists."
        ),
        "bug_row_retained": True,
        "candidate_capture": candidate,
        "existing_partial_evidence": {
            "firestaff_runtime_capture_probe": str(FIRESTAFF_CAPTURE_PROBE.relative_to(ROOT)),
            "pass1055_closed_door_original_gate": "partial closed-door stasis only",
            "gap_document": str(GAP_DOC.relative_to(ROOT)),
            "parity_matrix": str(PARITY_MATRIX.relative_to(ROOT)),
            "runbook": str(RUNBOOK.relative_to(ROOT)),
        },
        "required_for_promotion": [
            "exact route tokens",
            "map/x/y/direction before and after the reported command",
            "original PC 3.4 DUNGEON.DAT and GRAPHICS.DAT SHA256 receipts",
            "post-present 320x200 raw frame and 224x136 viewport crop",
            "capture_manifest.tsv generated by docs/parity/tools/dosbox_capture_manifest_writer.py",
            "Firestaff local repro output from the identical state",
        ],
        "non_claims": [
            "no full wall/door/fakewall collision parity claim",
            "no original-vs-Firestaff pixel diff",
            "no bug closure",
        ],
    }


def write_outputs(result: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# TODO100 DM1 V1 viewport/collision report repro gate",
        "",
        f"Status: `{result['status']}`",
        "",
        "This is a skip-safe capture scaffold for a viewport/collision report that",
        "does not yet have a capture manifest. The report stays a bug until the",
        "operator supplies paired original PC 3.4 evidence or a reviewed local repro",
        "manifest.",
        "",
        "## Current Gate",
        "",
        f"- Bug row retained in `TODO.md`: `{result['bug_row_retained']}`",
        f"- Candidate capture mode: `{result['candidate_capture']['mode']}`",
        f"- Candidate manifest present: `{result['candidate_capture']['manifest_present']}`",
        "",
        "## Promotion Contract",
        "",
    ]
    lines.extend(f"- {item}" for item in result["required_for_promotion"])
    lines.extend([
        "",
        "## Non-Claims",
        "",
    ])
    lines.extend(f"- {item}" for item in result["non_claims"])
    lines.extend([
        "",
        f"Machine-readable manifest: `{MANIFEST.relative_to(ROOT)}`",
        "",
    ])
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    result = build_result()
    write_outputs(result)
    print(f"PASS {PASS} status={result['status']}")
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
