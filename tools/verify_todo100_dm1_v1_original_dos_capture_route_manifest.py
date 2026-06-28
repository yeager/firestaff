#!/usr/bin/env python3
"""TODO100: DM1 V1 original DOS capture route manifest.

This is a skip-safe route-manifest verifier for the next original PC 3.4
DOSBox capture attempt.  It does not launch DOSBox, does not require original
game files, and does not promote screenshots.  The contract is one narrow,
deterministic Hall of Champions route that can later be paired with Firestaff
viewport hashes:

  start_south (1,3,SOUTH) -> forward_south (1,4,SOUTH) ->
  WUUF south_return (1,5,SOUTH), with the final frame expected to expose the
  ordinal-13 champion portrait viewport rectangle.
"""
from __future__ import annotations

import json
import os
import re
import sys
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS = "todo100_dm1_v1_original_dos_capture_route_manifest"
STATUS = "TODO100_DM1_V1_ORIGINAL_DOS_CAPTURE_ROUTE_MANIFEST_READY"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
CAPTURE_SCRIPT = ROOT / "scripts" / "dosbox_dm1_original_viewport_reference_capture.sh"
WUUF_PROBE = ROOT / "probes" / "m11" / "firestaff_dm1_v1_champion_mirror_portrait_13_south_return_portrait_rect_position_runtime_probe.c"
CAPTURE_GAP_DOC = ROOT / "docs" / "parity" / "DM1_V1_CAPTURE_GAP_EVIDENCE.md"

ROUTE_LABELS = [
    "hoc_start_south_1_3",
    "hoc_forward_south_1_4",
    "hoc_wuuf_south_return_1_5",
]

ROUTE_EVENTS = (
    "wait:9000 enter enter wait:1800 "
    "shot:hoc_start_south_1_3 "
    "kp8 wait:1200 shot:hoc_forward_south_1_4 "
    "kp8 wait:1200 shot:hoc_wuuf_south_return_1_5"
)

OPERATOR_ENV = {
    "DM1_ORIGINAL_PROGRAM": "DM -vv -sn -pk",
    "DM1_ROUTE_SKIP_STARTUP_SELECTOR": "1",
    "DM1_ORIGINAL_EXPECTED_SHOTS": str(len(ROUTE_LABELS)),
    "WAIT_BEFORE_INPUT_MS": "5000",
    "NEW_FILE_TIMEOUT_MS": "6000",
}

PC34_HASHES = {
    "DUNGEON.DAT_sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "GRAPHICS.DAT_sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "TITLE_sha256": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
}

EXPECTED_FRAMES: list[dict[str, Any]] = [
    {
        "index": 1,
        "label": "hoc_start_south_1_3",
        "expected_tuple": {"map": 0, "x": 1, "y": 3, "dir": "SOUTH", "dir_code": 2},
        "input_since_previous": [],
        "expected_crop": "01_hoc_start_south_1_3_original_viewport_224x136.ppm",
        "pairing_role": "baseline Hall of Champions start viewport",
    },
    {
        "index": 2,
        "label": "hoc_forward_south_1_4",
        "expected_tuple": {"map": 0, "x": 1, "y": 4, "dir": "SOUTH", "dir_code": 2},
        "input_since_previous": [{"route_token": "kp8", "command": "C003_COMMAND_MOVE_FORWARD"}],
        "expected_crop": "02_hoc_forward_south_1_4_original_viewport_224x136.ppm",
        "pairing_role": "one legal forward step toward the WUUF south_return pose",
    },
    {
        "index": 3,
        "label": "hoc_wuuf_south_return_1_5",
        "expected_tuple": {"map": 0, "x": 1, "y": 5, "dir": "SOUTH", "dir_code": 2},
        "input_since_previous": [{"route_token": "kp8", "command": "C003_COMMAND_MOVE_FORWARD"}],
        "expected_crop": "03_hoc_wuuf_south_return_1_5_original_viewport_224x136.ppm",
        "pairing_role": "target frame for future original-vs-Firestaff viewport hash pairing",
        "front_mirror": {
            "expected_ordinal": 13,
            "name": "WUUF",
            "title": "THE BIKA",
            "viewport_portrait_rect": [96, 35, 32, 29],
            "framebuffer_portrait_rect": [96, 68, 32, 29],
            "c026_source_rect": [160, 29, 32, 29],
        },
    },
]

SOURCE_ANCHORS = [
    "COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC",
    "CLIKMENU.C:180-347 F0366_COMMAND_ProcessTypes3To6_MoveParty",
    "DUNGEON.C:2573,2608-2612 C127 sensorData front-wall champion ordinal",
    "DUNVIEW.C:3913-3928 D1C champion portrait blit",
    "DUNVIEW.C:525 G0109 champion portrait viewport rectangle",
    "DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF redraw path",
]

NON_CLAIMS = [
    "No original assets, screenshots, DOSBox captures, or user-supplied game data are committed.",
    "No Firestaff-vs-original pixel parity or viewport-hash match is claimed.",
    "The route labels are metadata for a later operator-run capture and pairing pass.",
]


def expected_name(index: int, label: str) -> str:
    stem = re.sub(r"[^a-z0-9_-]+", "_", label.lower()).strip("_")
    return f"{index:02d}_{stem}_original_viewport_224x136.ppm"


def parse_route(route: str) -> dict[str, Any]:
    tokens = route.split()
    allowed = set("shot capture screenshot enter return esc escape space up down left right one two three four five six zero".split())
    allowed |= set("abcdefghijklmnopqrstuvwxyz")
    allowed |= set("0123456789")
    allowed |= {f"kp{i}" for i in range(10)}
    allowed |= {f"f{i}" for i in range(1, 5)}
    allowed |= {"kpenter"}
    labels: list[str] = []
    problems: list[str] = []
    for token in tokens:
        low = token.lower()
        if low in {"shot", "capture", "screenshot"}:
            labels.append("")
            problems.append("unlabeled shot token is forbidden for this manifest")
            continue
        if low.startswith("shot:"):
            label = low.split(":", 1)[1]
            if not re.fullmatch(r"[a-z0-9][a-z0-9_-]*", label):
                problems.append(f"invalid shot label: {token}")
            labels.append(label)
            continue
        if low.startswith("wait:"):
            if not re.fullmatch(r"wait:[0-9]+", low):
                problems.append(f"invalid wait token: {token}")
            continue
        if low.startswith("click:") or low.startswith("rclick:"):
            match = re.fullmatch(r"(?:r?click):([0-9]{1,3}),([0-9]{1,3})", low)
            if not match:
                problems.append(f"invalid click token: {token}")
                continue
            x, y = map(int, match.groups())
            if not (0 <= x < 320 and 0 <= y < 200):
                problems.append(f"click outside original 320x200 frame: {token}")
            continue
        if low not in allowed:
            problems.append(f"unknown route token: {token}")
    if labels != ROUTE_LABELS:
        problems.append(f"shot label order drifted: {labels}")
    return {
        "tokens": tokens,
        "token_count": len(tokens),
        "shot_labels": labels,
        "expected_labels": ROUTE_LABELS,
        "ok": not problems,
        "problems": problems,
    }


def audit_capture_script() -> dict[str, Any]:
    text = CAPTURE_SCRIPT.read_text(encoding="utf-8") if CAPTURE_SCRIPT.exists() else ""
    required = [
        "DM1_ORIGINAL_ROUTE_EVENTS",
        "DM1_ORIGINAL_EXPECTED_SHOTS",
        "shot:<label>",
        "kp0-kp9",
        "DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk'",
        "DM1_ROUTE_SKIP_STARTUP_SELECTOR=1",
        "original_viewport_shot_labels.tsv",
        "original_viewport_224x136_manifest.tsv",
        "original 320x200 frame",
    ]
    missing = [token for token in required if token not in text]
    return {
        "path": str(CAPTURE_SCRIPT.relative_to(ROOT)),
        "exists": CAPTURE_SCRIPT.exists(),
        "executable": os.access(CAPTURE_SCRIPT, os.X_OK),
        "required_tokens": required,
        "missing_tokens": missing,
        "ok": CAPTURE_SCRIPT.exists() and os.access(CAPTURE_SCRIPT, os.X_OK) and not missing,
    }


def audit_probe_anchor() -> dict[str, Any]:
    text = WUUF_PROBE.read_text(encoding="utf-8") if WUUF_PROBE.exists() else ""
    required = [
        "ordinal = 13",
        "route   = south_return",
        "party at (1, 5) facing SOUTH",
        "PROBE_PORTRAIT_VX       = 96",
        "PROBE_PORTRAIT_VY       = 35",
        "PROBE_EXPECTED_ORDINAL_SOUTH = 13",
        "Honesty: this is Firestaff deterministic-runtime evidence",
    ]
    missing = [token for token in required if token not in text]
    return {
        "path": str(WUUF_PROBE.relative_to(ROOT)),
        "exists": WUUF_PROBE.exists(),
        "required_tokens": required,
        "missing_tokens": missing,
        "ok": WUUF_PROBE.exists() and not missing,
    }


def audit_gap_doc() -> dict[str, Any]:
    text = CAPTURE_GAP_DOC.read_text(encoding="utf-8") if CAPTURE_GAP_DOC.exists() else ""
    required = [
        "Original Capture Gap Evidence",
        "Viewport content is still not",
        "`MATCHED`",
        "same-state viewport parity is still missing",
    ]
    missing = [token for token in required if token not in text]
    return {
        "path": str(CAPTURE_GAP_DOC.relative_to(ROOT)),
        "exists": CAPTURE_GAP_DOC.exists(),
        "required_tokens": required,
        "missing_tokens": missing,
        "ok": CAPTURE_GAP_DOC.exists() and not missing,
    }


def build_manifest() -> dict[str, Any]:
    route = parse_route(ROUTE_EVENTS)
    crop_names_ok = all(frame["expected_crop"] == expected_name(frame["index"], frame["label"]) for frame in EXPECTED_FRAMES)
    script = audit_capture_script()
    probe = audit_probe_anchor()
    gap_doc = audit_gap_doc()
    ok = route["ok"] and crop_names_ok and script["ok"] and probe["ok"] and gap_doc["ok"]
    return {
        "schema": "firestaff.parity.todo100_dm1_v1_original_dos_capture_route_manifest.v1",
        "status": STATUS if ok else "FAIL",
        "route_id": "dm1_pc34_hoc_wuuf_south_return_v1",
        "scope": "skip-safe route manifest only; no original runtime capture",
        "operator_command": {
            "description": "Operator-owned live run; do not run in CI and do not commit generated raw/crop frames.",
            "env": {**OPERATOR_ENV, "DM1_ORIGINAL_ROUTE_EVENTS": ROUTE_EVENTS, "OUT_DIR": "verification-screens/todo100-dm1-original-hoc-wuuf-south-return"},
            "argv": ["scripts/dosbox_dm1_original_viewport_reference_capture.sh", "--run"],
        },
        "route_events": ROUTE_EVENTS,
        "route_parse": route,
        "expected_frames": EXPECTED_FRAMES,
        "crop_names_ok": crop_names_ok,
        "pc34_hashes": PC34_HASHES,
        "source_anchors": SOURCE_ANCHORS,
        "capture_script": script,
        "firestaff_pairing_anchor": probe,
        "gap_doc_anchor": gap_doc,
        "non_claims": NON_CLAIMS,
        "ok": ok,
    }


def write_outputs(manifest: dict[str, Any]) -> None:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# TODO100 DM1 V1 original DOS capture route manifest",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "This gate defines one deterministic original PC 3.4 DOSBox capture route for a future same-state viewport pairing. It is intentionally skip-safe: it validates route metadata and local tooling only.",
        "",
        "## Route",
        "",
        f"- route id: `{manifest['route_id']}`",
        f"- route events: `{manifest['route_events']}`",
        f"- expected shots: `{manifest['operator_command']['env']['DM1_ORIGINAL_EXPECTED_SHOTS']}`",
        "- target: WUUF / THE BIKA ordinal 13, Hall of Champions south_return pose `(map 0, x=1, y=5, SOUTH)`",
        "",
        "## Expected Frames",
        "",
        "| # | Label | Tuple | Crop | Pairing role |",
        "|---:|---|---|---|---|",
    ]
    for frame in manifest["expected_frames"]:
        tup = frame["expected_tuple"]
        tuple_text = f"m{tup['map']} x{tup['x']} y{tup['y']} {tup['dir']}"
        lines.append(f"| {frame['index']} | `{frame['label']}` | `{tuple_text}` | `{frame['expected_crop']}` | {frame['pairing_role']} |")
    lines.extend([
        "",
        "## Anchors",
        "",
        f"- capture script: `{manifest['capture_script']['path']}` ok={manifest['capture_script']['ok']}",
        f"- Firestaff pairing probe: `{manifest['firestaff_pairing_anchor']['path']}` ok={manifest['firestaff_pairing_anchor']['ok']}",
        f"- gap document: `{manifest['gap_doc_anchor']['path']}` ok={manifest['gap_doc_anchor']['ok']}",
        "",
        "## Source Anchors",
        "",
    ])
    lines.extend(f"- {anchor}" for anchor in manifest["source_anchors"])
    lines.extend([
        "",
        "## Non-Claims",
        "",
    ])
    lines.extend(f"- {claim}" for claim in manifest["non_claims"])
    lines.append("")
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    manifest = build_manifest()
    write_outputs(manifest)
    if manifest["ok"]:
        print(f"PASS {PASS}")
        return 0
    print(f"FAIL {PASS}")
    print(json.dumps({k: manifest[k] for k in ("route_parse", "capture_script", "firestaff_pairing_anchor", "gap_doc_anchor")}, indent=2, sort_keys=True))
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
