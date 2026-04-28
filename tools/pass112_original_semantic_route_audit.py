#!/usr/bin/env python3
"""Pass 112 original semantic-route audit for DM1 V1 overlay readiness.

This joins the labeled original-route shot manifest produced by
scripts/dosbox_dm1_original_viewport_reference_capture.sh with the pass80 raw
frame classifier output.  It is deliberately stricter than the older readiness
checks: a route is not overlay-ready merely because six 320x200 screenshots
exist.  The requested route labels and measured frame classes must also match
the Firestaff six-shot fixture before any pixel overlay can be trusted.
"""
from __future__ import annotations

import argparse
import json
import tempfile
from pathlib import Path
from typing import Iterable

REPO = Path(__file__).resolve().parent.parent
DEFAULT_ATTEMPT_DIR = REPO / "verification-screens" / "pass70-original-dm1-viewports"
LABEL_MANIFEST_NAME = "original_viewport_shot_labels.tsv"
CLASSIFIER_JSON_NAME = "pass80_original_frame_classifier.json"

EXPECTED_ROUTE_LABELS = ["party_hud", "", "", "spell_panel", "", "inventory_panel"]
EXPECTED_FRAME_CLASSES = ["dungeon_gameplay", "dungeon_gameplay", "dungeon_gameplay", "spell_panel", "dungeon_gameplay", "inventory"]


def display(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO))
    except ValueError:
        return str(path)


def read_label_manifest(path: Path) -> tuple[list[dict[str, object]], list[str]]:
    problems: list[str] = []
    rows: list[dict[str, object]] = []
    if not path.exists():
        return rows, [f"missing shot-label manifest: {display(path)}"]
    lines = path.read_text().splitlines()
    if not lines:
        return rows, [f"empty shot-label manifest: {display(path)}"]
    header = lines[0].split("\t")
    if header != ["index", "filename", "route_label", "route_token"]:
        problems.append(f"unexpected shot-label manifest header: {header!r}")
    for line_no, line in enumerate(lines[1:], 2):
        parts = line.split("\t")
        if len(parts) != 4:
            problems.append(f"{path.name}:{line_no}: malformed row: {line!r}")
            continue
        idx_text, filename, route_label, route_token = parts
        try:
            idx = int(idx_text)
        except ValueError:
            problems.append(f"{path.name}:{line_no}: non-numeric index: {idx_text!r}")
            idx = -1
        rows.append({"index": idx, "filename": filename, "route_label": route_label, "route_token": route_token})
    return rows, problems


def read_classifier(path: Path) -> tuple[dict[str, object] | None, list[str]]:
    if not path.exists():
        return None, [f"missing pass80 classifier JSON: {display(path)}"]
    try:
        data = json.loads(path.read_text())
    except json.JSONDecodeError as exc:
        return None, [f"invalid classifier JSON {display(path)}: {exc}"]
    if data.get("schema") != "pass80_original_frame_classifier.v1":
        return data, [f"unexpected classifier schema: {data.get('schema')!r}"]
    return data, []


def audit(attempt_dir: Path, label_manifest: Path, classifier_json: Path) -> dict[str, object]:
    label_rows, label_problems = read_label_manifest(label_manifest)
    classifier, classifier_problems = read_classifier(classifier_json)
    problems: list[str] = []
    warnings: list[str] = []
    problems.extend(label_problems)
    problems.extend(classifier_problems)

    labels = [str(row.get("route_label", "")) for row in label_rows]
    if label_rows and len(label_rows) != len(EXPECTED_ROUTE_LABELS):
        problems.append(f"expected {len(EXPECTED_ROUTE_LABELS)} shot-label rows, found {len(label_rows)}")
    if label_rows and labels != EXPECTED_ROUTE_LABELS:
        problems.append(f"route labels do not match expected six-shot fixture: {labels!r}")

    captures = []
    classes: list[str] = []
    if classifier is not None:
        raw_captures = classifier.get("captures", [])
        if not isinstance(raw_captures, list):
            problems.append("classifier captures field is not a list")
            raw_captures = []
        captures = raw_captures
        classes = [str(row.get("classification", "")) for row in raw_captures if isinstance(row, dict)]
        if len(classes) != len(EXPECTED_FRAME_CLASSES):
            problems.append(f"expected {len(EXPECTED_FRAME_CLASSES)} classified raw frames, found {len(classes)}")
        elif classes != EXPECTED_FRAME_CLASSES:
            problems.append(f"frame classes do not match expected semantic route: {classes!r}")
        if classifier.get("problems"):
            problems.append("pass80 classifier reported problems: " + "; ".join(map(str, classifier.get("problems", []))))
        if classifier.get("warnings"):
            warnings.extend(map(str, classifier.get("warnings", [])))
        if classifier.get("duplicate_sha256_counts"):
            problems.append("duplicate raw frame hashes remain; route did not advance through unique visual states")
        dims = classifier.get("dimensions_seen", {})
        if dims != {"320x200": len(EXPECTED_FRAME_CLASSES)}:
            problems.append(f"classified frames are not exactly six raw 320x200 captures: {dims!r}")

    joined_rows: list[dict[str, object]] = []
    for idx in range(max(len(label_rows), len(captures))):
        label_row = label_rows[idx] if idx < len(label_rows) else {}
        capture = captures[idx] if idx < len(captures) and isinstance(captures[idx], dict) else {}
        joined_rows.append({
            "index": idx + 1,
            "route_label": label_row.get("route_label", ""),
            "route_token": label_row.get("route_token", ""),
            "filename": label_row.get("filename", ""),
            "classification": capture.get("classification", ""),
            "expected_route_label": EXPECTED_ROUTE_LABELS[idx] if idx < len(EXPECTED_ROUTE_LABELS) else None,
            "expected_classification": EXPECTED_FRAME_CLASSES[idx] if idx < len(EXPECTED_FRAME_CLASSES) else None,
            "sha256": capture.get("sha256", ""),
        })

    return {
        "schema": "pass112_original_semantic_route_audit.v1",
        "attempt_dir": display(attempt_dir),
        "label_manifest": display(label_manifest),
        "classifier_json": display(classifier_json),
        "honesty": "Semantic route audit only. Passing this gate makes original-vs-Firestaff overlay inputs eligible for pixel comparison; it does not claim pixel parity.",
        "expected_route_labels": EXPECTED_ROUTE_LABELS,
        "expected_frame_classes": EXPECTED_FRAME_CLASSES,
        "rows": joined_rows,
        "warnings": warnings,
        "problems": problems,
        "semantic_route_ready_for_overlay": not problems,
    }


def write_markdown(path: Path, result: dict[str, object]) -> None:
    lines = [
        "# Pass 112 — original semantic-route audit",
        "",
        "This joins route-shot labels with pass80 raw-frame classifications before original frames are allowed into overlay comparison.",
        "",
        f"- attempt dir: `{result['attempt_dir']}`",
        f"- semantic route ready: `{str(result['semantic_route_ready_for_overlay']).lower()}`",
        f"- honesty: {result['honesty']}",
        "",
    ]
    if result.get("problems"):
        lines.extend(["## Problems", ""])
        for problem in result["problems"]:
            lines.append(f"- {problem}")
        lines.append("")
    if result.get("warnings"):
        lines.extend(["## Warnings", ""])
        for warning in result["warnings"]:
            lines.append(f"- {warning}")
        lines.append("")
    lines.extend([
        "## Six-shot semantic checkpoints",
        "",
        "| # | route label | expected label | classification | expected class | sha256 |",
        "|---|-------------|----------------|----------------|----------------|--------|",
    ])
    for row in result.get("rows", []):
        lines.append(
            f"| {row['index']} | `{row.get('route_label', '')}` | `{row.get('expected_route_label', '')}` | "
            f"`{row.get('classification', '')}` | `{row.get('expected_classification', '')}` | `{str(row.get('sha256', ''))[:12]}` |"
        )
    lines.append("")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines).rstrip() + "\n")


def run_self_test() -> int:
    with tempfile.TemporaryDirectory() as td:
        d = Path(td)
        labels = d / LABEL_MANIFEST_NAME
        classifier = d / CLASSIFIER_JSON_NAME
        labels.write_text(
            "index\tfilename\troute_label\troute_token\n" +
            "\n".join(
                f"{i:02d}\timage{i:04d}-raw.png\t{label}\tshot{(':' + label) if label else ''}"
                for i, label in enumerate(EXPECTED_ROUTE_LABELS, 1)
            ) + "\n"
        )
        classifier.write_text(json.dumps({
            "schema": "pass80_original_frame_classifier.v1",
            "dimensions_seen": {"320x200": 6},
            "captures": [{"classification": cls, "sha256": f"hash{i}"} for i, cls in enumerate(EXPECTED_FRAME_CLASSES, 1)],
            "warnings": [],
            "problems": [],
            "duplicate_sha256_counts": {},
        }))
        ok = audit(d, labels, classifier)
        classifier.write_text(json.dumps({
            "schema": "pass80_original_frame_classifier.v1",
            "dimensions_seen": {"320x200": 6},
            "captures": [{"classification": "entrance_menu", "sha256": "same"}] * 6,
            "warnings": [],
            "problems": [],
            "duplicate_sha256_counts": {"same": 6},
        }))
        bad = audit(d, labels, classifier)
    passed = ok["semantic_route_ready_for_overlay"] is True and bad["semantic_route_ready_for_overlay"] is False
    print(json.dumps({"pass": passed, "positive_ready": ok["semantic_route_ready_for_overlay"], "negative_ready": bad["semantic_route_ready_for_overlay"], "negative_problem_count": len(bad["problems"])}, indent=2))
    return 0 if passed else 1


def main(argv: Iterable[str] | None = None) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("attempt_dir", nargs="?", type=Path, default=DEFAULT_ATTEMPT_DIR)
    ap.add_argument("--label-manifest", type=Path, default=None)
    ap.add_argument("--classifier-json", type=Path, default=None)
    ap.add_argument("--out-json", type=Path, default=None)
    ap.add_argument("--out-md", type=Path, default=None)
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args(argv)

    if args.self_test:
        return run_self_test()

    attempt_dir = args.attempt_dir
    label_manifest = args.label_manifest or (attempt_dir / LABEL_MANIFEST_NAME)
    classifier_json = args.classifier_json or (attempt_dir / CLASSIFIER_JSON_NAME)
    result = audit(attempt_dir, label_manifest, classifier_json)
    out_json = args.out_json or (REPO / "parity-evidence" / "pass112_original_semantic_route_audit.json")
    out_md = args.out_md or out_json.with_suffix(".md")
    out_json.parent.mkdir(parents=True, exist_ok=True)
    out_json.write_text(json.dumps(result, indent=2) + "\n")
    write_markdown(out_md, result)
    print(json.dumps({"semantic_route_ready_for_overlay": result["semantic_route_ready_for_overlay"], "problems": result["problems"], "json": display(out_json), "markdown": display(out_md)}, indent=2))
    return 0 if result["semantic_route_ready_for_overlay"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
