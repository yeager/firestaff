#!/usr/bin/env python3
"""Summarize executed pass304 original PC34 capture batches without committing pixels."""
from __future__ import annotations
import argparse, csv, json
from pathlib import Path
from typing import Any
PASS = "pass308_original_capture_execution_manifest"
ROOT = Path(__file__).resolve().parents[1]
BATCHES = {"A": ["start_south", "turn_right_west", "move_forward_west"], "B": ["start_south", "turn_left_east"], "C": ["start_south", "blocked_forward_south_wall"]}
OUT_JSON = ROOT / "parity-evidence/verification/pass308_original_capture_execution_manifest.json"
OUT_MD = ROOT / "parity-evidence/pass308_original_capture_execution_manifest.md"
def read_tsv(path: Path) -> list[dict[str, str]]:
    if not path.exists(): return []
    with path.open(newline="", encoding="utf-8") as f: return list(csv.DictReader(f, delimiter="\t"))
def batch_record(batch: str, required: list[str]) -> dict[str, Any]:
    d = ROOT / f"verification-screens/pass304-original-pc34-wall-comparator-batch-{batch}"
    labels = read_tsv(d / "original_viewport_shot_labels.tsv")
    crops = read_tsv(d / "original_viewport_224x136_manifest.tsv")
    classifier_path = d / "pass80_original_frame_classifier.json"
    classifier = json.loads(classifier_path.read_text()) if classifier_path.exists() else {}
    crop_by_file = {r.get("filename", ""): r for r in crops}
    captures = classifier.get("captures", [])
    rows = []
    for i, lab in enumerate(labels):
        crop = crop_by_file.get(lab.get("filename", ""), {})
        cls = captures[i].get("classification") if i < len(captures) else None
        rows.append({"index": lab.get("index"), "routeLabel": lab.get("route_label"), "filename": lab.get("filename"), "classification": cls, "cropSha256": crop.get("sha256"), "promotableLabel": lab.get("route_label") in required})
    required_rows = [r for r in rows if r["routeLabel"] in required]
    return {"batch": batch, "dir": str(d.relative_to(ROOT)), "requiredPromotionLabels": required, "labelsPresent": [r["routeLabel"] for r in rows], "requiredLabelsPresent": sorted({r["routeLabel"] for r in required_rows}), "requiredLabelCoverage": sorted({r["routeLabel"] for r in required_rows}) == sorted(required), "classifierClassCounts": classifier.get("class_counts", {}), "classifierProblems": classifier.get("problems", []), "promotionRows": required_rows}
def build() -> dict[str, Any]:
    batches = [batch_record(b, labels) for b, labels in BATCHES.items()]
    coverage_ok = all(b["requiredLabelCoverage"] for b in batches)
    gameplay_ok = all(row.get("classification") in {"dungeon_gameplay", "wall_closeup"} for b in batches for row in b["promotionRows"])
    return {"pass": PASS, "status": "PASS_CAPTURE_EXECUTED_STATE_ORACLE_PENDING" if coverage_ok and gameplay_ok else "BLOCKED_CAPTURE_EXECUTION_INCOMPLETE", "scope": "Metadata-only execution record for pass304 original PC34 DOSBox batches; records hashes/classes but not screenshots and not pixel parity.", "sourceInputs": {"pass304": "parity-evidence/verification/pass304_dm1_v1_original_viewport_capture_blocker_manifest.json", "pass307": "parity-evidence/verification/pass307_original_capture_batch_plan_manifest.json", "captureDirs": [b["dir"] for b in batches]}, "coverage": {"requiredLabelCoverage": coverage_ok, "requiredPromotionRowsGameplayOrWallCloseup": gameplay_ok}, "batches": batches, "remainingBlockers": ["party tuple/F0128 state is not source-bound for the original runtime yet", "duplicate viewport hashes are preserved as evidence and must not be interpreted as pixel parity", "no screenshots/PPM/PNG are promoted to tracked release artifacts by this pass"], "notClaimed": ["pixel parity", "complete state-oracle proof", "bitmap-byte publication"]}
def render_md(m: dict[str, Any]) -> str:
    lines = [f"# {PASS}", "", f"- status: `{m['status']}`", "- screenshots/pixels: not tracked", "", "| batch | labels covered | classes |", "|---|---|---|"]
    for b in m["batches"]: lines.append(f"| `{b['batch']}` | `{b['requiredLabelsPresent']}` | `{b['classifierClassCounts']}` |")
    lines += ["", "Remaining blockers:"] + [f"- {x}" for x in m["remainingBlockers"]]
    lines += ["", "Not claimed:"] + [f"- {x}" for x in m["notClaimed"]]
    return "\n".join(lines) + "\n"
def main() -> int:
    ap = argparse.ArgumentParser(); ap.add_argument("--write", action="store_true"); args = ap.parse_args(); m = build()
    if args.write:
        OUT_JSON.parent.mkdir(parents=True, exist_ok=True); OUT_JSON.write_text(json.dumps(m, indent=2, sort_keys=True) + "\n", encoding="utf-8"); OUT_MD.write_text(render_md(m), encoding="utf-8")
    print(f"{PASS}: {m['status']} coverage={m['coverage']}")
    return 0 if m["status"].startswith("PASS_") else 1
if __name__ == "__main__": raise SystemExit(main())
