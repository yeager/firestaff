#!/usr/bin/env python3
"""Pass 113 original party-state probe for DM1 PC 3.4 routing.

This is a narrow follow-up gate for the pass112 unblocker. It does not try to
prove pixel parity. It answers a smaller question: did an original-route capture
reach a state suitable for party/HUD, spell, or inventory control comparison, or
is it still just direct-start/no-party dungeon imagery?
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Iterable

REPO = Path(__file__).resolve().parent.parent
LABEL_MANIFEST_NAME = "original_viewport_shot_labels.tsv"
CLASSIFIER_JSON_NAME = "pass80_original_frame_classifier.json"

CONTROL_CLASSES = {"spell_panel", "inventory"}
UNSAFE_CLASSES = {"title_or_menu", "entrance_menu", "wall_closeup", "non_graphics_blocker", "graphics_320x200_unclassified"}
# Pass151/pass153 proved this hash is a static no-party dungeon placeholder:
# helper keys, xdotool keys/types, and panel clicks do not yield party control.
STATIC_NO_PARTY_DUNGEON_HASHES = {"48ed3743ab6a"}


def display(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO))
    except ValueError:
        return str(path)


def read_labels(path: Path) -> tuple[list[dict[str, str]], list[str]]:
    if not path.exists():
        return [], [f"missing shot-label manifest: {display(path)}"]
    rows: list[dict[str, str]] = []
    problems: list[str] = []
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
        rows.append({"index": parts[0], "filename": parts[1], "route_label": parts[2], "route_token": parts[3]})
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


def _region_value(capture: dict[str, object], region: str, key: str, default: float = 0.0) -> float:
    regions = capture.get("regions", {})
    if not isinstance(regions, dict):
        return default
    region_data = regions.get(region, {})
    if not isinstance(region_data, dict):
        return default
    value = region_data.get(key, default)
    return float(value) if isinstance(value, (int, float)) else default


def audit(attempt_dir: Path, label_manifest: Path, classifier_json: Path) -> dict[str, object]:
    label_rows, label_problems = read_labels(label_manifest)
    classifier, classifier_problems = read_classifier(classifier_json)
    problems = label_problems + classifier_problems
    warnings: list[str] = []
    captures: list[dict[str, object]] = []
    duplicate_sha256_counts: dict[str, object] = {}
    if classifier is not None:
        raw = classifier.get("captures", [])
        if isinstance(raw, list):
            captures = [row for row in raw if isinstance(row, dict)]
        else:
            problems.append("classifier captures field is not a list")
        problems.extend(map(str, classifier.get("problems", []) or []))
        warnings.extend(map(str, classifier.get("warnings", []) or []))
        raw_dupes = classifier.get("duplicate_sha256_counts", {}) or {}
        if isinstance(raw_dupes, dict):
            duplicate_sha256_counts = raw_dupes

    classes = [str(row.get("classification", "")) for row in captures]
    labels = [row.get("route_label", "") for row in label_rows]
    capture_sha12s = [str(row.get("sha256", ""))[:12] for row in captures]

    unsafe_seen = sorted({cls for cls in classes if cls in UNSAFE_CLASSES})
    control_seen = sorted({cls for cls in classes if cls in CONTROL_CLASSES})
    static_no_party_seen = sorted({sha for sha in capture_sha12s if sha in STATIC_NO_PARTY_DUNGEON_HASHES})
    blank_right_column_frames = 0
    for row in captures:
        if _region_value(row, "right_column", "nonblack_ratio") < 0.15 and _region_value(row, "spell_area", "nonblack_ratio") < 0.05:
            blank_right_column_frames += 1

    all_gameplay = bool(classes) and all(cls == "dungeon_gameplay" for cls in classes)
    direct_start_no_party_signature = (
        all_gameplay
        and not control_seen
        and blank_right_column_frames >= max(1, len(captures) - 1)
        and (bool(duplicate_sha256_counts) or bool(static_no_party_seen))
    )
    party_control_ready = (
        bool(control_seen)
        and not unsafe_seen
        and not duplicate_sha256_counts
        and not static_no_party_seen
        and len(captures) == 6
    )

    if direct_start_no_party_signature:
        problems.append("direct-start/no-party signature: static no-party dungeon viewport reached (including known 48ed placeholder), but party/HUD/spell/inventory control was not proven")
    elif not party_control_ready:
        problems.append("party-control state not proven: capture lacks a clean six-frame spell/inventory/HUD-ready sequence")

    rows: list[dict[str, object]] = []
    for idx, capture in enumerate(captures):
        label = labels[idx] if idx < len(labels) else ""
        rows.append({
            "index": idx + 1,
            "route_label": label,
            "classification": capture.get("classification", ""),
            "sha256": capture.get("sha256", ""),
            "right_column_nonblack_ratio": round(_region_value(capture, "right_column", "nonblack_ratio"), 4),
            "spell_area_nonblack_ratio": round(_region_value(capture, "spell_area", "nonblack_ratio"), 4),
        })

    return {
        "schema": "pass113_original_party_state_probe.v1",
        "attempt_dir": display(attempt_dir),
        "label_manifest": display(label_manifest),
        "classifier_json": display(classifier_json),
        "honesty": "Party-state semantic probe only. It can block unsafe original captures; it does not claim pixel parity or champion identity.",
        "capture_count": len(captures),
        "classes": classes,
        "labels": labels,
        "duplicate_sha256_counts": duplicate_sha256_counts,
        "static_no_party_hashes_seen": static_no_party_seen,
        "unsafe_classes_seen": unsafe_seen,
        "control_classes_seen": control_seen,
        "blank_right_column_frames": blank_right_column_frames,
        "direct_start_no_party_signature": direct_start_no_party_signature,
        "party_control_ready": party_control_ready,
        "rows": rows,
        "warnings": warnings,
        "problems": problems,
    }


def write_markdown(path: Path, result: dict[str, object]) -> None:
    lines = [
        "# Pass 113 — original party-state probe",
        "",
        "This pass113 gate classifies whether an original DM1 PC 3.4 capture is actually usable for party/HUD, spell, or inventory comparison after direct-start/recruitment attempts.",
        "",
        f"- attempt dir: `{result['attempt_dir']}`",
        f"- party control ready: `{str(result['party_control_ready']).lower()}`",
        f"- direct-start/no-party signature: `{str(result['direct_start_no_party_signature']).lower()}`",
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
        "## Captures",
        "",
        "| # | route label | classification | right-column nonblack | spell-area nonblack | sha256 |",
        "|---|-------------|----------------|-----------------------|---------------------|--------|",
    ])
    for row in result.get("rows", []):
        lines.append(
            f"| {row['index']} | `{row.get('route_label', '')}` | `{row.get('classification', '')}` | "
            f"{row.get('right_column_nonblack_ratio', 0):.4f} | {row.get('spell_area_nonblack_ratio', 0):.4f} | `{str(row.get('sha256', ''))[:12]}` |"
        )
    lines.append("")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("\n".join(lines).rstrip() + "\n")


def run_self_test() -> int:
    ready = {
        "schema": "pass80_original_frame_classifier.v1",
        "captures": [
            {"classification": cls, "sha256": f"h{i}", "regions": {"right_column": {"nonblack_ratio": 0.05}, "spell_area": {"nonblack_ratio": 0.6 if cls == "spell_panel" else 0.0}}}
            for i, cls in enumerate(["dungeon_gameplay", "dungeon_gameplay", "dungeon_gameplay", "spell_panel", "dungeon_gameplay", "inventory"], 1)
        ],
        "problems": [],
        "warnings": [],
        "duplicate_sha256_counts": {},
    }
    blank = {
        "schema": "pass80_original_frame_classifier.v1",
        "captures": [
            {"classification": "dungeon_gameplay", "sha256": "same" if i else "h0", "regions": {"right_column": {"nonblack_ratio": 0.01}, "spell_area": {"nonblack_ratio": 0.0}}}
            for i in range(6)
        ],
        "problems": ["duplicate raw frames detected: 1 unique sha256 value(s) repeat"],
        "warnings": [],
        "duplicate_sha256_counts": {"same": 5},
    }
    import tempfile
    with tempfile.TemporaryDirectory() as td:
        d = Path(td)
        labels = d / LABEL_MANIFEST_NAME
        labels.write_text("index\tfilename\troute_label\troute_token\n" + "\n".join(f"{i:02d}\timage{i:04d}-raw.png\tprobe{i}\tshot:probe{i}" for i in range(1, 7)) + "\n")
        cj = d / CLASSIFIER_JSON_NAME
        cj.write_text(json.dumps(ready))
        a = audit(d, labels, cj)
        cj.write_text(json.dumps(blank))
        b = audit(d, labels, cj)
    static48ed = {
        "schema": "pass80_original_frame_classifier.v1",
        "captures": [
            {"classification": "dungeon_gameplay", "sha256": "48ed3743ab6a" + str(i).zfill(52), "regions": {"right_column": {"nonblack_ratio": 0.01}, "spell_area": {"nonblack_ratio": 0.0}}}
            for i in range(6)
        ],
        "problems": [],
        "warnings": [],
        "duplicate_sha256_counts": {},
    }
    with tempfile.TemporaryDirectory() as td:
        d = Path(td)
        labels = d / LABEL_MANIFEST_NAME
        labels.write_text("index\tfilename\troute_label\troute_token\n" + "\n".join(f"{i:02d}\timage{i:04d}-raw.png\tprobe{i}\tshot:probe{i}" for i in range(1, 7)) + "\n")
        cj = d / CLASSIFIER_JSON_NAME
        cj.write_text(json.dumps(static48ed))
        c = audit(d, labels, cj)
    passed = (
        a["party_control_ready"] is True
        and b["direct_start_no_party_signature"] is True
        and b["party_control_ready"] is False
        and c["direct_start_no_party_signature"] is True
        and c["party_control_ready"] is False
        and c["static_no_party_hashes_seen"] == ["48ed3743ab6a"]
    )
    print(json.dumps({"pass": passed, "ready_party_control": a["party_control_ready"], "blank_no_party_signature": b["direct_start_no_party_signature"], "static48ed_blocked": c["direct_start_no_party_signature"], "static48ed_ready": c["party_control_ready"], "static_no_party_hashes_seen": c["static_no_party_hashes_seen"]}, indent=2))
    return 0 if passed else 1


def main(argv: Iterable[str] | None = None) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("attempt_dir", type=Path, nargs="?", default=REPO / "verification-screens" / "pass112-n2-diagnostic-direct")
    ap.add_argument("--label-manifest", type=Path, default=None)
    ap.add_argument("--classifier-json", type=Path, default=None)
    ap.add_argument("--out-json", type=Path, default=None)
    ap.add_argument("--out-md", type=Path, default=None)
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args(argv)
    if args.self_test:
        return run_self_test()
    label_manifest = args.label_manifest or args.attempt_dir / LABEL_MANIFEST_NAME
    classifier_json = args.classifier_json or args.attempt_dir / CLASSIFIER_JSON_NAME
    result = audit(args.attempt_dir, label_manifest, classifier_json)
    out_json = args.out_json or REPO / "parity-evidence" / "pass113_original_party_state_probe.json"
    out_md = args.out_md or out_json.with_suffix(".md")
    out_json.parent.mkdir(parents=True, exist_ok=True)
    out_json.write_text(json.dumps(result, indent=2) + "\n")
    write_markdown(out_md, result)
    print(json.dumps({"party_control_ready": result["party_control_ready"], "direct_start_no_party_signature": result["direct_start_no_party_signature"], "problems": result["problems"], "json": display(out_json), "markdown": display(out_md)}, indent=2))
    return 0 if result["party_control_ready"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
