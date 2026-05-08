#!/usr/bin/env python3
"""Convert pass304 nextCaptureCommands into deterministic batch plan + label/hash manifest.

This is metadata only: it records the exact PC34 original-capture shell batches,
route labels, expected viewport dimensions, and hashes needed before screenshot
capture can be promoted. It does not run DOSBox, dump bitmaps, or claim pixel parity.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path
from typing import Any

PASS = "pass307_original_capture_batch_plan_manifest"
PASS304_JSON = Path("parity-evidence/verification/pass304_dm1_v1_original_viewport_capture_blocker_manifest.json")
PASS306_JSON = Path("parity-evidence/verification/pass306_dm1_wall_pixel_region_graphics_bridge.json")
PASS300_JSON = Path("parity-evidence/verification/dm1_v1_viewport_wall_render_plan_gate.json")
OUT_JSON = Path("parity-evidence/verification/pass307_original_capture_batch_plan_manifest.json")
OUT_MD = Path("parity-evidence/pass307_original_capture_batch_plan_manifest.md")
EXPECTED_VIEWPORT = {"width": 224, "height": 136}
SHOT_RE = re.compile(r"shot:([A-Za-z0-9_\-]+)")


def load_json(path: Path) -> Any:
    if not path.exists():
        raise FileNotFoundError(path)
    return json.loads(path.read_text(encoding="utf-8"))


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def command_records(raw_commands: Any) -> tuple[list[dict[str, Any]], list[str]]:
    if not isinstance(raw_commands, dict):
        return [], ["pass304 nextCaptureCommands is not a mapping"]

    records: list[dict[str, Any]] = []
    blockers: list[str] = []
    for label in sorted(raw_commands):
        command = raw_commands[label]
        if label == "knownBlocker":
            blockers.append(str(command))
            continue
        if not isinstance(command, str) or not command.strip():
            blockers.append(f"{label}: capture command is empty/non-string")
            continue
        shot_labels = SHOT_RE.findall(command)
        non_padding_shots = [s for s in shot_labels if not s.startswith("padding_")]
        records.append(
            {
                "batchIndex": len(records),
                "label": label,
                "commandHash": sha256_text(command),
                "labelHash": sha256_text(f"{label}|{EXPECTED_VIEWPORT['width']}x{EXPECTED_VIEWPORT['height']}|{','.join(non_padding_shots)}"),
                "expectedViewportCrop": EXPECTED_VIEWPORT,
                "shotLabels": shot_labels,
                "promotionShotLabels": non_padding_shots,
                "paddingShotCount": len(shot_labels) - len(non_padding_shots),
                "hasSixShotScriptContract": len(shot_labels) == 6,
                "usesOriginalPc34Stage": "DungeonMasterPC34" in command,
                "usesDeterministicRouteEvents": "DM1_ORIGINAL_ROUTE_EVENTS" in command,
                "usesDosboxXvfbRun": "DOSBOX=/usr/bin/dosbox xvfb-run" in command,
            }
        )
    return records, blockers


def build_manifest() -> dict[str, Any]:
    p304 = load_json(PASS304_JSON)
    p306 = load_json(PASS306_JSON)
    p300 = load_json(PASS300_JSON)
    records, known_blockers = command_records(p304.get("nextCaptureCommands"))
    required_routes = p304.get("requiredRouteStates", [])
    promotion_labels = sorted({label for rec in records for label in rec["promotionShotLabels"]})
    ok = bool(records) and all(
        rec["hasSixShotScriptContract"]
        and rec["usesOriginalPc34Stage"]
        and rec["usesDeterministicRouteEvents"]
        and rec["usesDosboxXvfbRun"]
        for rec in records
    )
    return {
        "pass": PASS,
        "status": "passed" if ok else "blocked",
        "scope": "Deterministic metadata batch plan for pass304 PC34 original viewport capture commands; no runtime capture and no pixel parity claim.",
        "sourceInputs": {
            "pass304BlockerManifest": str(PASS304_JSON),
            "pass306Bridge": str(PASS306_JSON),
            "pass300WallRenderPlan": str(PASS300_JSON),
        },
        "inputStatus": {
            "pass304": p304.get("status"),
            "pass306": p306.get("status"),
            "pass300": p300.get("status"),
        },
        "coverage": {
            "totalBatches": len(records),
            "promotionShotLabels": promotion_labels,
            "requiredRouteStates": required_routes,
            "allBatchesKeepSixShotContract": all(r["hasSixShotScriptContract"] for r in records),
            "allBatchesUseOriginalPc34Stage": all(r["usesOriginalPc34Stage"] for r in records),
            "allBatchesUseDeterministicRouteEvents": all(r["usesDeterministicRouteEvents"] for r in records),
            "allBatchesUseDosboxXvfbRun": all(r["usesDosboxXvfbRun"] for r in records),
        },
        "batchPlans": records,
        "knownBlockers": known_blockers,
        "remainingBlockers": [
            "execute the three DOSBox/Xvfb capture batches and keep screenshots outside the repo unless explicitly promoted as small metadata/evidence",
            "verify captured 224x136 viewport crops against pass300/pass306 wall-region metadata",
            "do not promote padding shots or bitmap parity claims from this metadata-only manifest",
        ],
        "notClaimed": [
            "original screenshot capture succeeded",
            "pixel parity",
            "bitmap bytes or screenshots in repository",
        ],
    }


def render_md(manifest: dict[str, Any]) -> str:
    lines = [
        "# pass307 original capture batch plan manifest",
        "",
        "Metadata-only deterministic batch plan for pass304 PC34 original viewport capture commands.",
        "",
        f"- status: `{manifest['status']}`",
        f"- batches: `{manifest['coverage']['totalBatches']}`",
        f"- promotion shots: `{manifest['coverage']['promotionShotLabels']}`",
        "",
        "| batch | promotion shots | command sha256 | guards |",
        "|---|---|---|---|",
    ]
    for rec in manifest["batchPlans"]:
        guards = []
        for key, label in [
            ("hasSixShotScriptContract", "six-shot"),
            ("usesOriginalPc34Stage", "PC34"),
            ("usesDeterministicRouteEvents", "route-events"),
            ("usesDosboxXvfbRun", "dosbox-xvfb"),
        ]:
            guards.append(f"{label}={'yes' if rec[key] else 'no'}")
        lines.append(f"| `{rec['label']}` | `{rec['promotionShotLabels']}` | `{rec['commandHash']}` | {', '.join(guards)} |")
    lines.extend(["", "Remaining blockers:"])
    lines.extend(f"- {b}" for b in manifest["remainingBlockers"])
    lines.extend(["", "Not claimed:"])
    lines.extend(f"- {c}" for c in manifest["notClaimed"])
    return "\n".join(lines) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--write", action="store_true")
    args = ap.parse_args()
    manifest = build_manifest()
    if args.write:
        OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
        OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
        OUT_MD.write_text(render_md(manifest), encoding="utf-8")
    print(f"{PASS}: {manifest['status']} batches={manifest['coverage']['totalBatches']} shots={manifest['coverage']['promotionShotLabels']}")
    return 0 if manifest["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main())
