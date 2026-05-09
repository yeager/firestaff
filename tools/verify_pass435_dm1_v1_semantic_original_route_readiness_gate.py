#!/usr/bin/env python3
"""Pass435 gate: DM1 V1 semantic original-route readiness.

This is a no-DOSBox promotion gate/report. It ties the pass434 original crop
readiness result to the stricter pass385 runtime-dispatch proof and the existing
classifier/crop manifests. A blocked result is the expected successful outcome
until the semantic original route proves F0365/F0366 command dispatch and six
non-duplicate, classified route states.
"""
from __future__ import annotations

import json
import os
from collections import Counter
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass435_dm1_v1_semantic_original_route_readiness_gate"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
EXPECTED_BLOCKED = "BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY"
READY = "PASS435_SEMANTIC_ORIGINAL_ROUTE_READY"

SOURCE_ANCHORS: list[dict[str, Any]] = [
    {
        "id": "f0365_turn_party_acceptance",
        "file": "CLIKMENU.C",
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "lines": "142-174",
        "needles": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE",
            "F0284_CHAMPION_SetPartyDirection",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval",
        ],
        "claim": "turn commands must be observed reaching F0365, where stop-wait is set and party direction mutates",
    },
    {
        "id": "f0366_move_party_acceptance",
        "file": "CLIKMENU.C",
        "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "lines": "180-347",
        "needles": [
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks",
        ],
        "claim": "move commands must be observed reaching F0366, where target square, move result, and movement cooldown are committed",
    },
    {
        "id": "f0128_viewport_tuple_composition",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "lines": "8318-8611",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "G0296_puc_Bitmap_Viewport",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
            "F0127_DUNGEONVIEW_DrawSquareD0C",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)",
        ],
        "claim": "viewport crops are promotable only when F0128 composes G0296 for a known direction/X/Y tuple",
    },
    {
        "id": "f0097_pc34_viewport_present",
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "lines": "709-858",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "G0296_puc_Bitmap_Viewport",
            "F0638_GetZone(C007_ZONE_VIEWPORT",
            "VIDRV_09_BlitViewPort",
        ],
        "claim": "the capture seam must be the PC34 viewport present path, not setup/menu echo",
    },
]

INPUTS = {
    "pass434_crop_readiness": "parity-evidence/verification/pass434_dm1_v1_original_viewport_crop_readiness_gate/manifest.json",
    "pass385_runtime_semantic_route": "parity-evidence/verification/pass385_dm1_v1_corrected_loader_delta_semantic_route/manifest.json",
    "pass378_semantic_blocker": "parity-evidence/verification/pass378_dm1_v1_original_route_semantic_clean_blocker/manifest.json",
    "pass376_classifier": "verification-screens/pass376-original-route/pass80_original_frame_classifier.json",
    "pass376_crop_manifest": "verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv",
}
EXPECTED_SEQUENCE = ["dungeon_gameplay", "dungeon_gameplay", "dungeon_gameplay", "spell_panel", "dungeon_gameplay", "inventory"]


def norm(text: str) -> str:
    return " ".join(text.split())


def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            start, end = [int(x) for x in part.split("-", 1)]
        else:
            start = end = int(part)
        chunks.append("\n".join(lines[start - 1:end]))
    return "\n".join(chunks)


def audit_sources() -> list[dict[str, Any]]:
    rows = []
    for anchor in SOURCE_ANCHORS:
        path = REDMCSB / anchor["file"]
        text = source_window(path, anchor["lines"]) if path.exists() else ""
        missing = [needle for needle in anchor["needles"] if norm(needle) not in norm(text)]
        rows.append({**anchor, "path": str(path), "exists": path.exists(), "ok": path.exists() and not missing, "missing": missing})
    return rows


def load_json(rel: str) -> dict[str, Any]:
    path = ROOT / rel
    if not path.exists():
        return {"exists": False, "path": rel}
    return {"exists": True, "path": rel, **json.loads(path.read_text(encoding="utf-8"))}


def summarize_classifier(rel: str) -> dict[str, Any]:
    data = load_json(rel)
    if not data.get("exists"):
        return data
    captures = data.get("captures", [])
    classes = [c.get("classification") for c in captures]
    hashes = [c.get("sha256") for c in captures if c.get("sha256")]
    return {
        "exists": True,
        "path": rel,
        "pass": data.get("pass"),
        "capture_count": data.get("capture_count"),
        "classes": classes,
        "expected_sequence": EXPECTED_SEQUENCE,
        "sequence_ok": classes == EXPECTED_SEQUENCE,
        "class_counts": data.get("class_counts") or dict(Counter(classes)),
        "duplicate_sha256_counts": data.get("duplicate_sha256_counts") or {h: n for h, n in Counter(hashes).items() if n > 1},
        "problems": data.get("problems", []),
    }


def summarize_crop_manifest(rel: str) -> dict[str, Any]:
    path = ROOT / rel
    if not path.exists():
        return {"exists": False, "path": rel}
    rows = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("#"):
            continue
        kind, filename, width, height, size, sha = line.split("\t")
        rows.append({"kind": kind, "filename": filename, "width": int(width), "height": int(height), "bytes": int(size), "sha256": sha})
    hashes = [r["sha256"] for r in rows]
    return {
        "exists": True,
        "path": rel,
        "row_count": len(rows),
        "rows_all_224x136": len(rows) == 6 and all(r["kind"] == "original_viewport_224x136" and r["width"] == 224 and r["height"] == 136 for r in rows),
        "duplicate_sha256_counts": {h: n for h, n in Counter(hashes).items() if n > 1},
        "rows": rows,
    }


def blocker_list(data: dict[str, Any]) -> list[str]:
    blockers: list[str] = []
    if not all(row["ok"] for row in data["source_audit"]):
        blockers.append("ReDMCSB source anchor audit failed")

    pass434 = data["inputs"]["pass434_crop_readiness"]
    if pass434.get("status") != "PASS_PASS434_ORIGINAL_VIEWPORT_CROP_READINESS":
        blockers.append("pass434 crop/source readiness is not green")

    pred = (data["inputs"]["pass385_runtime_semantic_route"].get("proofPredicates") or {})
    if pred.get("f0380Hit") is not True:
        blockers.append("runtime does not prove F0380 command queue hit")
    if pred.get("f0365OrF0366Hit") is not True:
        blockers.append("runtime still does not prove F0365/F0366 command dispatch")
    if pred.get("g0321StopWaitWriteObserved") is not True:
        blockers.append("runtime does not prove G0321 stop-wait write")
    if pred.get("nextF0128AfterStopWaitObserved") is not True:
        blockers.append("runtime does not prove later F0128 viewport draw after stop-wait")

    classifier = data["classifier"]
    if classifier.get("pass") is not True:
        blockers.append("pass376 raw route classifier is not green")
    if classifier.get("sequence_ok") is not True:
        blockers.append("pass376 raw route classes do not match the semantic promotion sequence")
    if classifier.get("duplicate_sha256_counts"):
        blockers.append("pass376 raw route repeats screenshot hashes")

    crops = data["crop_manifest"]
    if crops.get("rows_all_224x136") is not True:
        blockers.append("pass376 crop manifest is not exactly six 224x136 viewport crops")
    if crops.get("duplicate_sha256_counts"):
        blockers.append("pass376 viewport crops repeat hashes, so labels are not semantically distinct")

    return blockers


def write_report(data: dict[str, Any]) -> None:
    lines = [
        "# Pass435 — DM1 V1 semantic original-route readiness gate",
        "",
        f"Status: `{data['status']}`",
        "",
        "## Verdict",
        "",
    ]
    if data["blockers"]:
        lines.append("Original viewport crop tooling is ready, but semantic original-route promotion remains blocked.")
    else:
        lines.append("Semantic original-route promotion predicates are ready for the next paired-diff stage.")
    lines.extend([
        "",
        "## ReDMCSB WIP20210206 source audit",
        "",
    ])
    for row in data["source_audit"]:
        lines.append(f"- `{row['file']}:{row['lines']}` `{row['function']}` — ok=`{row['ok']}`; {row['claim']}")
    pred = data["inputs"]["pass385_runtime_semantic_route"].get("proofPredicates") or {}
    lines.extend([
        "",
        "## Runtime semantic proof carried forward",
        "",
        f"- pass385 status: `{data['inputs']['pass385_runtime_semantic_route'].get('status')}`",
        f"- F0380 command queue hit: `{pred.get('f0380Hit')}`",
        f"- F0365/F0366 command dispatch hit: `{pred.get('f0365OrF0366Hit')}`",
        f"- G0321 stop-wait write observed: `{pred.get('g0321StopWaitWriteObserved')}`",
        f"- later F0128 after stop-wait observed: `{pred.get('nextF0128AfterStopWaitObserved')}`",
        "",
        "## Artifact semantics",
        "",
        f"- pass434 readiness: `{data['inputs']['pass434_crop_readiness'].get('status')}`",
        f"- raw classifier sequence ok: `{data['classifier'].get('sequence_ok')}`; classes: `{data['classifier'].get('classes')}`",
        f"- raw duplicate hashes: `{data['classifier'].get('duplicate_sha256_counts')}`",
        f"- crop manifest rows_all_224x136: `{data['crop_manifest'].get('rows_all_224x136')}`",
        f"- crop duplicate hashes: `{data['crop_manifest'].get('duplicate_sha256_counts')}`",
        "",
        "## Blockers",
        "",
    ])
    if data["blockers"]:
        lines.extend(f"- {b}" for b in data["blockers"])
    else:
        lines.append("- none")
    lines.extend([
        "",
        "## Promotion rule",
        "",
        data["promotion_rule"],
        "",
        "This gate does not launch DOSBox and does not claim pixel parity.",
        "",
        f"Manifest: `{MANIFEST.relative_to(ROOT)}`",
    ])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    data: dict[str, Any] = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "repo": str(ROOT),
        "sourceRoot": str(REDMCSB),
        "source_audit": audit_sources(),
        "inputs": {name: load_json(rel) for name, rel in INPUTS.items() if name not in {"pass376_classifier", "pass376_crop_manifest"}},
        "classifier": summarize_classifier(INPUTS["pass376_classifier"]),
        "crop_manifest": summarize_crop_manifest(INPUTS["pass376_crop_manifest"]),
        "promotion_rule": "Promote only when pass434 readiness is green, a bounded post-load runtime proves F0380 plus F0365/F0366 command dispatch plus G0321 stop-wait write plus a later F0128 viewport draw, and six raw/cropped route states are non-duplicate and match dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,spell_panel,dungeon_gameplay,inventory.",
        "not_claimed": ["DOSBox/live original capture during this gate", "original-vs-Firestaff pixel parity", "semantic route promotion while F0365/F0366 is unproven"],
    }
    data["blockers"] = blocker_list(data)
    data["status"] = EXPECTED_BLOCKED if data["blockers"] else READY
    MANIFEST.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(data)
    print(json.dumps({"status": data["status"], "blockers": data["blockers"], "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT))}, indent=2))
    return 0 if data["status"] == EXPECTED_BLOCKED else 1


if __name__ == "__main__":
    raise SystemExit(main())
