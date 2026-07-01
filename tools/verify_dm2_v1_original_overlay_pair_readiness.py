#!/usr/bin/env python3
"""Verify DM2 original-overlay original<->Firestaff pair readiness.

This is a bounded evidence gate for the OPEN DM2 original-overlay row. It does
not launch DOSBox, does not require proprietary captures in git, and does not
claim pixel parity. Instead it checks whether an operator-local H2313 original
capture attempt and a Firestaff-side pairing manifest are internally coherent
enough for a future same-state pixel comparison.

Default CTest behavior is skip-safe:
- missing original captures => OPEN_BOUNDED_NO_ORIGINAL_CAPTURE, exit 0;
- missing Firestaff pairing manifest after a valid original capture =>
  OPEN_BOUNDED_FIRESTAFF_PAIR_MISSING, exit 0;
- malformed present artifacts => FAIL_*, exit 1;
- at least one valid dungeon_gameplay original+Firestaff 224x136 pair =>
  PAIR_READY, exit 0.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS_ID = "dm2_v1_original_overlay_pair_readiness"
DEFAULT_ORIGINAL_DIR = ROOT / "verification-screens/passH2313-dm2-original-overlays"
DEFAULT_PAIR_MANIFEST = ROOT / "parity-evidence/dm2_v1_original_overlay_pair_manifest.json"
DEFAULT_VERIFY_DIR = ROOT / "parity-evidence/verification" / PASS_ID
DEFAULT_OUT_MANIFEST = DEFAULT_VERIFY_DIR / "manifest.json"
DEFAULT_OUT_REPORT = ROOT / "parity-evidence" / f"{PASS_ID}.md"

ORIGINAL_LABELS = "dm2_original_overlay_shot_labels.tsv"
ORIGINAL_CROPS = "dm2_viewport_224x136_manifest.tsv"
ORIGINAL_HEALTH = "dm2_raw_frame_health.json"
ORIGINAL_CROP_DIR = "viewport_224x136"
EXPECTED_W = 224
EXPECTED_H = 136


def display(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(ROOT))
    except ValueError:
        return str(path)


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def read_tsv(path: Path) -> list[dict[str, str]]:
    lines = path.read_text(encoding="utf-8").splitlines()
    if not lines:
        raise ValueError(f"{display(path)} is empty")
    header = lines[0].split("\t")
    rows: list[dict[str, str]] = []
    for line_no, line in enumerate(lines[1:], 2):
        if not line.strip():
            continue
        values = line.split("\t")
        if len(values) != len(header):
            raise ValueError(
                f"{display(path)}:{line_no} has {len(values)} fields, expected {len(header)}"
            )
        rows.append(dict(zip(header, values, strict=True)))
    return rows


def ppm_dims(path: Path) -> tuple[int, int] | None:
    try:
        data = path.read_bytes()
    except OSError:
        return None
    tokens: list[bytes] = []
    i = 0
    n = len(data)
    while len(tokens) < 4 and i < n:
        while i < n and data[i] in b" \t\r\n":
            i += 1
        if i < n and data[i] == ord("#"):
            while i < n and data[i] not in b"\r\n":
                i += 1
            continue
        start = i
        while i < n and data[i] not in b" \t\r\n":
            i += 1
        if start < i:
            tokens.append(data[start:i])
    if len(tokens) < 4 or tokens[0] != b"P6" or tokens[3] != b"255":
        return None
    while i < n and data[i] in b" \t\r\n":
        i += 1
    width = int(tokens[1])
    height = int(tokens[2])
    if len(data) - i != width * height * 3:
        return None
    return width, height


def png_dims(path: Path) -> tuple[int, int] | None:
    try:
        data = path.read_bytes()[:24]
    except OSError:
        return None
    if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
        return None
    return struct.unpack(">II", data[16:24])


def image_dims(path: Path) -> tuple[int, int] | None:
    if path.suffix.lower() == ".ppm":
        return ppm_dims(path)
    if path.suffix.lower() == ".png":
        return png_dims(path)
    return ppm_dims(path) or png_dims(path)


def resolve_repo_path(value: str, base: Path) -> Path:
    path = Path(value)
    if path.is_absolute():
        return path
    candidate = base / path
    if candidate.exists():
        return candidate
    return ROOT / path


def load_original_attempt(original_dir: Path) -> dict[str, Any]:
    labels_path = original_dir / ORIGINAL_LABELS
    crop_manifest_path = original_dir / ORIGINAL_CROPS
    health_path = original_dir / ORIGINAL_HEALTH
    crop_dir = original_dir / ORIGINAL_CROP_DIR
    result: dict[str, Any] = {
        "attempt_dir": display(original_dir),
        "labels_manifest": display(labels_path),
        "crop_manifest": display(crop_manifest_path),
        "health_manifest": display(health_path),
        "present": False,
        "ok": False,
        "rows": [],
        "problems": [],
    }
    if not labels_path.exists() and not crop_manifest_path.exists() and not health_path.exists():
        result["problems"].append("operator-local H2313 original capture manifests are absent")
        return result

    result["present"] = True
    if not labels_path.exists():
        result["problems"].append(f"missing {ORIGINAL_LABELS}")
    if not crop_manifest_path.exists():
        result["problems"].append(f"missing {ORIGINAL_CROPS}")
    if not health_path.exists():
        result["problems"].append(f"missing {ORIGINAL_HEALTH}")
    if result["problems"]:
        return result

    try:
        health = json.loads(health_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        result["problems"].append(f"{ORIGINAL_HEALTH} is not valid JSON: {exc}")
        return result
    result["raw_health_pass"] = bool(health.get("pass"))
    result["capture_count"] = health.get("captureCount")
    if not health.get("pass"):
        result["problems"].append(f"{ORIGINAL_HEALTH} pass=false")

    try:
        labels = read_tsv(labels_path)
        crops = read_tsv(crop_manifest_path)
    except ValueError as exc:
        result["problems"].append(str(exc))
        return result

    crop_by_name = {row.get("filename", ""): row for row in crops}
    seen_labels: set[str] = set()
    for label_row in labels:
        filename = label_row.get("filename", "")
        route_label = label_row.get("route_label", "")
        crop_row = crop_by_name.get(filename)
        path = crop_dir / filename
        dims = image_dims(path) if path.exists() else None
        actual_sha = sha256(path) if path.exists() else None
        expected_sha = crop_row.get("sha256") if crop_row else None
        row_problems: list[str] = []
        if not route_label:
            row_problems.append("route_label is empty; row cannot be promoted as same-state evidence")
        elif route_label in seen_labels:
            row_problems.append(f"duplicate route_label {route_label}")
        seen_labels.add(route_label)
        if crop_row is None:
            row_problems.append(f"{filename} is missing from {ORIGINAL_CROPS}")
        if not path.exists():
            row_problems.append(f"{display(path)} is missing")
        if dims != (EXPECTED_W, EXPECTED_H):
            row_problems.append(f"{filename} dimensions are {dims}, expected 224x136")
        if expected_sha and actual_sha != expected_sha:
            row_problems.append(f"{filename} sha256 does not match crop manifest")
        result["rows"].append(
            {
                "index": label_row.get("index"),
                "filename": filename,
                "route_label": route_label,
                "route_token": label_row.get("route_token"),
                "path": display(path),
                "exists": path.exists(),
                "dims": list(dims) if dims else None,
                "sha256": actual_sha,
                "manifest_sha256": expected_sha,
                "ok": not row_problems,
                "problems": row_problems,
            }
        )
    result["ok"] = not result["problems"] and all(row["ok"] for row in result["rows"])
    return result


def normalize_semantic_state(row: dict[str, Any]) -> str:
    for key in ("semantic_state", "classifier_state", "state", "class"):
        value = row.get(key)
        if isinstance(value, str) and value:
            return value
    return ""


def load_pair_manifest(path: Path, original: dict[str, Any]) -> dict[str, Any]:
    result: dict[str, Any] = {
        "manifest": display(path),
        "present": path.exists(),
        "ok": False,
        "rows": [],
        "problems": [],
        "dungeon_gameplay_pair_count": 0,
    }
    if not path.exists():
        result["problems"].append("Firestaff pairing manifest is absent")
        return result
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        result["problems"].append(f"{display(path)} is not valid JSON: {exc}")
        return result
    rows = data.get("rows")
    if not isinstance(rows, list):
        result["problems"].append("pair manifest must contain a rows array")
        return result
    result["schema"] = data.get("schema")
    result["route_id"] = data.get("route_id")
    original_by_label = {
        row["route_label"]: row
        for row in original.get("rows", [])
        if row.get("route_label") and row.get("ok")
    }
    for idx, row in enumerate(rows, 1):
        if not isinstance(row, dict):
            result["rows"].append({"index": idx, "ok": False, "problems": ["row is not an object"]})
            continue
        route_label = str(row.get("route_label", ""))
        semantic_state = normalize_semantic_state(row)
        firestaff_value = row.get("firestaff_crop") or row.get("firestaff_path")
        row_problems: list[str] = []
        original_row = original_by_label.get(route_label)
        if not route_label:
            row_problems.append("route_label is required")
        if original_row is None:
            row_problems.append(f"route_label {route_label!r} does not match an OK original crop")
        if semantic_state != "dungeon_gameplay":
            row_problems.append("semantic_state must be dungeon_gameplay before promotion")
        if not isinstance(firestaff_value, str) or not firestaff_value:
            row_problems.append("firestaff_crop or firestaff_path is required")
            firestaff_path = ROOT / "__missing_firestaff_crop__"
        else:
            firestaff_path = resolve_repo_path(firestaff_value, path.parent)
        dims = image_dims(firestaff_path) if firestaff_path.exists() else None
        actual_sha = sha256(firestaff_path) if firestaff_path.exists() else None
        expected_sha = row.get("firestaff_sha256")
        original_sha_claim = row.get("original_sha256")
        if not firestaff_path.exists():
            row_problems.append(f"Firestaff crop is missing: {display(firestaff_path)}")
        if dims != (EXPECTED_W, EXPECTED_H):
            row_problems.append(f"Firestaff crop dimensions are {dims}, expected 224x136")
        if expected_sha and actual_sha != expected_sha:
            row_problems.append("Firestaff crop sha256 does not match pair manifest")
        if original_row and original_sha_claim and original_sha_claim != original_row.get("sha256"):
            row_problems.append("original_sha256 does not match the H2313 original crop manifest")
        ok = not row_problems
        if ok and semantic_state == "dungeon_gameplay":
            result["dungeon_gameplay_pair_count"] += 1
        result["rows"].append(
            {
                "index": idx,
                "route_label": route_label,
                "semantic_state": semantic_state,
                "original_filename": original_row.get("filename") if original_row else None,
                "original_sha256": original_row.get("sha256") if original_row else None,
                "firestaff_path": display(firestaff_path),
                "firestaff_dims": list(dims) if dims else None,
                "firestaff_sha256": actual_sha,
                "ok": ok,
                "problems": row_problems,
            }
        )
    result["ok"] = bool(result["rows"]) and all(row["ok"] for row in result["rows"])
    return result


def decide_status(original: dict[str, Any], pairs: dict[str, Any]) -> tuple[str, bool]:
    if not original["present"]:
        return "OPEN_BOUNDED_NO_ORIGINAL_CAPTURE", True
    if not original["ok"]:
        return "FAIL_ORIGINAL_CAPTURE_INCOMPLETE", False
    if not pairs["present"]:
        return "OPEN_BOUNDED_FIRESTAFF_PAIR_MISSING", True
    if not pairs["ok"]:
        return "FAIL_PAIR_MANIFEST", False
    if pairs["dungeon_gameplay_pair_count"] <= 0:
        return "OPEN_BOUNDED_NO_DUNGEON_GAMEPLAY_PAIR", True
    return "PAIR_READY", True


def write_outputs(result: dict[str, Any], out_manifest: Path, out_report: Path) -> None:
    out_manifest.parent.mkdir(parents=True, exist_ok=True)
    out_report.parent.mkdir(parents=True, exist_ok=True)
    out_manifest.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    original = result["original"]
    pairs = result["pairs"]
    lines = [
        "# DM2 V1 original-overlay pair readiness",
        "",
        f"Status: `{result['status']}`",
        "",
        "This gate checks whether operator-local H2313 original viewport crops",
        "and Firestaff-side 224x136 viewport crops are ready to be compared as",
        "same-state DM2 original-overlay evidence. It is a readiness gate only;",
        "it does not claim pixel parity.",
        "",
        "## Inputs",
        "",
        f"- Original attempt: `{original['attempt_dir']}`",
        f"- Firestaff pair manifest: `{pairs['manifest']}`",
        "",
        "## Result",
        "",
        f"- Original manifests present: `{original['present']}`",
        f"- Original crop rows OK: `{original['ok']}`",
        f"- Firestaff pair manifest present: `{pairs['present']}`",
        f"- Firestaff pair rows OK: `{pairs['ok']}`",
        f"- Dungeon gameplay pairs: `{pairs['dungeon_gameplay_pair_count']}`",
        "",
        "## Boundary",
        "",
        "- Missing operator-local captures keep the gate OPEN-BOUNDED and passing.",
        "- Present but malformed captures or pair manifests fail the gate.",
        "- Promotion still requires reviewed original bytes plus same-route Firestaff bytes.",
        "",
    ]
    out_report.write_text("\n".join(lines), encoding="utf-8")


def main(argv: list[str]) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--original-dir", type=Path, default=DEFAULT_ORIGINAL_DIR)
    parser.add_argument("--firestaff-pair-manifest", type=Path, default=DEFAULT_PAIR_MANIFEST)
    parser.add_argument("--out-manifest", type=Path, default=DEFAULT_OUT_MANIFEST)
    parser.add_argument("--out-report", type=Path, default=DEFAULT_OUT_REPORT)
    args = parser.parse_args(argv)

    original = load_original_attempt(args.original_dir)
    pairs = load_pair_manifest(args.firestaff_pair_manifest, original)
    status, ok = decide_status(original, pairs)
    result = {
        "schema": "firestaff.dm2_v1_original_overlay_pair_readiness.v1",
        "status": status,
        "ok": ok,
        "scope": (
            "DM2 original-overlay pair readiness. Requires H2313 original 224x136 "
            "viewport crops and a Firestaff pair manifest with at least one "
            "dungeon_gameplay same-label 224x136 Firestaff crop before promotion."
        ),
        "honesty": (
            "Readiness/provenance gate only. No original-vs-Firestaff pixel parity "
            "claim is made by this verifier."
        ),
        "original": original,
        "pairs": pairs,
    }
    write_outputs(result, args.out_manifest, args.out_report)
    print(f"{status} dm2 original-overlay pair readiness")
    print(f"manifest={display(args.out_manifest)}")
    if ok:
        return 0
    print(json.dumps(result, indent=2)[:4000])
    return 1


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
