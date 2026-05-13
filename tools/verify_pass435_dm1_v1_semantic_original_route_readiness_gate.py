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
        "id": "f0380_queue_pop_dispatch",
        "file": "COMMAND.C",
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "lines": "2045-2156",
        "needles": [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command",
            "G2153_i_QueuedCommandsCount--",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "claim": "queued input must be observed through F0380 pop/load before turn commands branch to F0365 or movement commands branch to F0366",
    },
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
        "id": "input_entrance_click_origin",
        "file": "INPUT.C",
        "function": "F0536_INPUT_Initialize",
        "lines": "197-204",
        "needles": [
            "G1038_i_MouseX = 250 * 2; /* Coordinates of Resume button on entrance screen */",
            "G1038_i_MouseX = 250 * 2; /* Coordinates of Enter button on entrance screen */",
            "F0073_MOUSE_BuildPointerScreenArea(G1038_i_MouseX >> 1, G1039_i_MouseY >> 1);",
        ],
        "claim": "the original PC34 entrance route starts from source-defined mouse coordinates, so host click coordinates must be treated as route evidence until runtime state confirms gameplay",
    },
    {
        "id": "startup_mouse_keyboard_tables",
        "file": "STARTUP2.C",
        "function": "F0462_START_StartGame_CPSEF",
        "lines": "1179-1183",
        "needles": [
            "G0441_ps_PrimaryMouseInput = G0447_as_Graphic561_PrimaryMouseInput_Interface;",
            "G0442_ps_SecondaryMouseInput = G0448_as_Graphic561_SecondaryMouseInput_Movement;",
            "G0443_ps_PrimaryKeyboardInput = G0458_as_Graphic561_PrimaryKeyboardInput_Interface;",
            "G0444_ps_SecondaryKeyboardInput = G0459_as_Graphic561_SecondaryKeyboardInput_Movement;",
        ],
        "claim": "post-load capture shots are meaningful only after the original installs interface and movement input tables for both mouse and keyboard routes",
    },
    {
        "id": "startup_entrance_load_start_then_discard",
        "file": "STARTUP2.C",
        "function": "F0435/F0441/F0462 startup flow",
        "lines": "1441-1457,1507-1531",
        "needles": [
            "F0441_STARTEND_ProcessEntrance();",
            "while (F0435_STARTEND_LoadGame() != C01_LOAD_GAME_SUCCESS)",
            "F0477_MEMORY_OpenGraphicsDat_CPSDF();",
            "F0462_START_StartGame_CPSEF();",
            "F0357_COMMAND_DiscardAllInput();",
        ],
        "claim": "the reusable capture route must pass entrance/load/start and discard stale setup input before semantic shots are promotable",
    },
    {
        "id": "dungeon_clickable_wall_or_door_button",
        "file": "CLIKVIEW.C",
        "function": "F0377_COMMAND_ProcessType80_ClickInDungeonView",
        "lines": "367-385,407-431",
        "needles": [
            "F0376_COMMAND_IsPointInBox(G0291_aauc_DungeonViewClickableBoxes[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT], P0752_i_X, P0753_i_Y - 33)",
            "F0268_SENSOR_AddEvent(C10_EVENT_DOOR, L1155_i_MapX, L1156_i_MapY, 0, C02_EFFECT_TOGGLE, G0313_ul_GameTime + 1);",
            "for (AL1150_ui_ViewCell = C00_VIEW_CELL_FRONT_LEFT; AL1150_ui_ViewCell < C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT + 1; AL1150_ui_ViewCell++)",
            "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor();",
        ],
        "claim": "dungeon-view mouse captures are semantic only when clicks resolve through source clickable boxes and resulting door/sensor actions, not by filename labels",
    },
    {
        "id": "dunview_clickable_box_materialization",
        "file": "DUNVIEW.C",
        "function": "F0107/F0110 dungeon clickable materialization",
        "lines": "3722-3725,4163-4212",
        "needles": [
            "F0007_MAIN_CopyBytes(AL0090_puc_CoordinateSet, G0291_aauc_DungeonViewClickableBoxes[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT], sizeof(G0291_aauc_DungeonViewClickableBoxes[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT]));",
            "L0111_puc_CoordinateSet = G0208_aaauc_Graphic558_DoorButtonCoordinateSets",
            "F0791_DUNGEONVIEW_DrawBitmapXX(AL0112_puc_Bitmap, G0296_puc_Bitmap_Viewport",
            "F0007_MAIN_CopyBytes((char*)G2032_ai_XYZ, (char*)G2210_aai_XYZ_DungeonViewClickable[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT]",
        ],
        "claim": "viewport click hitboxes used by original capture are materialized while drawing G0296, so crop labels must be tied to the same draw/update boundary",
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
    "pass391_queued_command_dispatch": "parity-evidence/verification/pass391_dm1_v1_queued_command_dispatch/manifest.json",
    "pass378_semantic_blocker": "parity-evidence/verification/pass378_dm1_v1_original_route_semantic_clean_blocker/manifest.json",
    "pass376_classifier": "verification-screens/pass376-original-route/pass80_original_frame_classifier.json",
    "pass376_crop_manifest": "verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv",
    "pass376_route_labels": "verification-screens/pass376-original-route/original_viewport_shot_labels.tsv",
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
        "captures": captures,
    }



def summarize_route_labels(rel: str) -> dict[str, Any]:
    path = ROOT / rel
    if not path.exists():
        return {"exists": False, "path": rel, "rows": []}
    rows: list[dict[str, Any]] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("index\t"):
            continue
        parts = line.split("\t")
        if len(parts) >= 4:
            rows.append({"index": parts[0], "filename": parts[1], "route_label": parts[2], "route_token": parts[3]})
    return {"exists": True, "path": rel, "rows": rows}


def build_route_rows(labels: dict[str, Any], classifier: dict[str, Any], crops: dict[str, Any]) -> list[dict[str, Any]]:
    label_rows = labels.get("rows") or []
    captures = classifier.get("captures") or []
    crop_rows = crops.get("rows") or []
    out: list[dict[str, Any]] = []
    for idx in range(max(len(label_rows), len(captures), len(crop_rows), len(EXPECTED_SEQUENCE))):
        label = label_rows[idx] if idx < len(label_rows) else {}
        cap = captures[idx] if idx < len(captures) else {}
        crop = crop_rows[idx] if idx < len(crop_rows) else {}
        expected = EXPECTED_SEQUENCE[idx] if idx < len(EXPECTED_SEQUENCE) else None
        out.append({
            "index": label.get("index") or f"{idx + 1:02d}",
            "route_label": label.get("route_label"),
            "route_token": label.get("route_token"),
            "raw_file": cap.get("file"),
            "expected_class": expected,
            "actual_class": cap.get("classification"),
            "class_match": cap.get("classification") == expected,
            "raw_sha256": cap.get("sha256"),
            "crop_file": crop.get("filename"),
            "crop_sha256": crop.get("sha256"),
        })
    return out


def duplicate_groups(rows: list[dict[str, Any]], key: str) -> dict[str, list[str]]:
    seen: dict[str, list[str]] = {}
    for row in rows:
        value = row.get(key)
        if value:
            seen.setdefault(value, []).append(str(row.get("index")))
    return {value: indices for value, indices in seen.items() if len(indices) > 1}


def unblock_commands() -> dict[str, Any]:
    # Keep this route aligned with EXPECTED_SEQUENCE. The previous contract used
    # only movement keypad events while requiring spell_panel and inventory
    # classes, which made every rerun non-promotable before it started.
    route_capture = (
        "OUT_DIR=$PWD/verification-screens/pass376-original-route "
        "DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 "
        "DOSBOX=/usr/bin/dosbox DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' "
        "DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 WAIT_BEFORE_INPUT_MS=3000 NEW_FILE_TIMEOUT_MS=6000 "
        "DM1_ORIGINAL_ROUTE_EVENTS=\"wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:2200 one wait:2500 "
        "shot:party_hud wait:700 kp4 wait:900 shot:turn_left_after_vblank wait:700 kp6 wait:900 "
        "shot:turn_right_after_vblank wait:700 f1 wait:1200 shot:spell_panel wait:700 kp6 wait:1200 "
        "shot:post_spell_redraw wait:700 f4 wait:1200 shot:inventory_panel\" "
        "xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run"
    )
    return {
        "semantic_route_capture": route_capture,
        "crop_manifest_strict": "python3 tools/pass86_original_viewport_crop_manifest.py verification-screens/pass376-original-route --out-dir verification-screens/pass376-original-dm1-viewports",
        "readiness_gate": "python3 tools/verify_pass435_dm1_v1_semantic_original_route_readiness_gate.py",
        "route_contract": {
            "expected_classes": EXPECTED_SEQUENCE,
            "expected_labels": [
                "party_hud",
                "turn_left_after_vblank",
                "turn_right_after_vblank",
                "spell_panel",
                "post_spell_redraw",
                "inventory_panel",
            ],
            "required_driver_tokens": ["kp4", "kp6", "f1", "f4"],
            "reason": "the capture command must drive both movement and UI states because the readiness gate requires spell_panel and inventory classifications",
        },
        "promotion_requires": [
            "six raw 320x200 frames classified as dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,spell_panel,dungeon_gameplay,inventory",
            "no duplicate raw frame hashes",
            "six 224x136 viewport crops with no duplicate crop hashes",
            "pass434 remains green and runtime evidence still proves F0380 -> F0365/F0366 -> G0321 -> later F0128",
        ],
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


def quarantine_pass376_artifacts(classifier: dict[str, Any], crops: dict[str, Any]) -> dict[str, Any]:
    raw_dups = classifier.get("duplicate_sha256_counts") or {}
    crop_dups = crops.get("duplicate_sha256_counts") or {}
    reasons: list[str] = []
    if classifier.get("pass") is not True:
        reasons.append("raw classifier did not pass")
    if classifier.get("sequence_ok") is not True:
        reasons.append("raw classifier sequence does not match semantic promotion sequence")
    if raw_dups:
        reasons.append("raw route repeats screenshot hashes")
    if crop_dups:
        reasons.append("viewport crops repeat hashes")
    quarantined = bool(reasons)
    return {
        "quarantined": quarantined,
        "artifact_set": "pass376-original-route/pass376-original-dm1-viewports",
        "reason": "; ".join(reasons) if reasons else None,
        "raw_path": classifier.get("path"),
        "crop_path": crops.get("path"),
        "raw_classes": classifier.get("classes"),
        "expected_sequence": classifier.get("expected_sequence"),
        "raw_duplicate_sha256_counts": raw_dups,
        "crop_duplicate_sha256_counts": crop_dups,
        "decision": (
            "quarantine as non-promotable historical evidence; do not use for semantic original-route readiness or pixel parity until a replacement six-state route is captured"
            if quarantined else
            "not quarantined"
        ),
    }


def command_contract_audit(next_unblock: dict[str, Any]) -> dict[str, Any]:
    command = next_unblock.get("semantic_route_capture", "")
    contract = next_unblock.get("route_contract", {})
    labels = []
    if "DM1_ORIGINAL_ROUTE_EVENTS=\"" in command:
        route = command.split("DM1_ORIGINAL_ROUTE_EVENTS=\"", 1)[1].split("\"", 1)[0]
        tokens = route.split()
        labels = [tok.split(":", 1)[1] for tok in tokens if tok.startswith("shot:")]
    else:
        route = ""
        tokens = []
    missing_labels = [label for label in contract.get("expected_labels", []) if label not in labels]
    missing_tokens = [tok for tok in contract.get("required_driver_tokens", []) if tok not in tokens]
    return {
        "ok": not missing_labels and not missing_tokens and len(labels) == 6,
        "route": route,
        "shot_labels": labels,
        "missing_labels": missing_labels,
        "missing_required_driver_tokens": missing_tokens,
        "expected_classes": contract.get("expected_classes", []),
        "expected_labels": contract.get("expected_labels", []),
    }


def blocker_list(data: dict[str, Any]) -> list[str]:
    blockers: list[str] = []
    if not all(row["ok"] for row in data["source_audit"]):
        blockers.append("ReDMCSB source anchor audit failed")

    pass434 = data["inputs"]["pass434_crop_readiness"]
    if pass434.get("status") != "PASS_PASS434_ORIGINAL_VIEWPORT_CROP_READINESS":
        blockers.append("pass434 crop/source readiness is not green")

    pred = (data["inputs"]["pass385_runtime_semantic_route"].get("proofPredicates") or {})
    dispatch_pred = (data["inputs"].get("pass391_queued_command_dispatch", {}).get("proofPredicates") or {})
    pass391_status = data["inputs"].get("pass391_queued_command_dispatch", {}).get("status")
    pass391_dispatch_ok = (
        pass391_status == "PASS391_KEYBOARD_QUEUE_TO_F0380_DISPATCH_PROVEN"
        and dispatch_pred.get("f0365OrF0366DispatchObserved") is True
        and dispatch_pred.get("g2153DecrementPopLoadObserved") is True
        and dispatch_pred.get("consumerBreakpointsArmedAfterIncrement") is True
    )
    if not (pred.get("f0380Hit") is True or dispatch_pred.get("f0380PopLoadAfterQueueWriteObserved") is True):
        blockers.append("runtime does not prove F0380 command queue hit/pop-load")
    if not pass391_dispatch_ok:
        blockers.append("runtime still does not prove F0365/F0366 command dispatch")
    if pred.get("g0321StopWaitWriteObserved") is not True:
        blockers.append("runtime does not prove G0321 stop-wait write")
    if pred.get("nextF0128AfterStopWaitObserved") is not True:
        blockers.append("runtime does not prove later F0128 viewport draw after stop-wait")

    classifier = data["classifier"]
    crops = data["crop_manifest"]
    quarantine = data.get("pass376_quarantine") or {}
    if quarantine.get("quarantined"):
        blockers.append("pass376 original-route artifacts are quarantined as non-promotable duplicate/non-semantic evidence")
    else:
        if classifier.get("pass") is not True:
            blockers.append("pass376 raw route classifier is not green")
        if classifier.get("sequence_ok") is not True:
            blockers.append("pass376 raw route classes do not match the semantic promotion sequence")
        if classifier.get("duplicate_sha256_counts"):
            blockers.append("pass376 raw route repeats screenshot hashes")

    if crops.get("rows_all_224x136") is not True:
        blockers.append("pass376 crop manifest is not exactly six 224x136 viewport crops")
    if data.get("next_unblock_contract", {}).get("ok") is not True:
        blockers.append("pass435 next unblock command contract is internally inconsistent")

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
    dispatch_pred = data["inputs"].get("pass391_queued_command_dispatch", {}).get("proofPredicates") or {}
    pass391_status = data["inputs"].get("pass391_queued_command_dispatch", {}).get("status")
    lines.extend([
        "",
        "## Runtime semantic proof carried forward",
        "",
        f"- pass385 status: `{data['inputs']['pass385_runtime_semantic_route'].get('status')}`",
        f"- pass385 F0380 command queue hit: `{pred.get('f0380Hit')}`",
        f"- pass385 F0365/F0366 command dispatch hit: `{pred.get('f0365OrF0366Hit')}`",
        f"- pass391 status: `{pass391_status}`",
        f"- pass391 F0380 pop/load after queue write: `{dispatch_pred.get('f0380PopLoadAfterQueueWriteObserved')}`",
        f"- pass391 F0365/F0366 command dispatch observed: `{dispatch_pred.get('f0365OrF0366DispatchObserved')}`",
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
        "## Current route label/class/hash matrix",
        "",
        "| # | route label | expected | actual | class ok | raw sha | crop sha |",
        "|---|---|---|---|---|---|---|",
    ])
    for route in data.get("route_rows", []):
        raw_sha = (route.get("raw_sha256") or "")[:12]
        crop_sha = (route.get("crop_sha256") or "")[:12]
        lines.append(f"| {route.get('index')} | `{route.get('route_label')}` | `{route.get('expected_class')}` | `{route.get('actual_class')}` | `{route.get('class_match')}` | `{raw_sha}` | `{crop_sha}` |")
    lines.extend([
        "",
        "## Duplicate hash groups",
        "",
        f"- raw frame duplicates by route index: `{data.get('raw_duplicate_route_indices')}`",
        f"- viewport crop duplicates by route index: `{data.get('crop_duplicate_route_indices')}`",
        "",
        "## Pass376 artifact quarantine",
        "",
        f"- quarantined: `{data.get('pass376_quarantine', {}).get('quarantined')}`",
        f"- reason: `{data.get('pass376_quarantine', {}).get('reason')}`",
        f"- decision: {data.get('pass376_quarantine', {}).get('decision')}",
        "",
        "## Next unblock command contract",
        "",
        "Run the route capture, then strict crop manifest, then this gate again. These are actionability checks only; they do not claim pixel parity.",
        "",
        "```bash",
        data["next_unblock"]["semantic_route_capture"],
        data["next_unblock"]["crop_manifest_strict"],
        data["next_unblock"]["readiness_gate"],
        "```",
        "",
        "Promotion requires:",
    ])
    lines.extend(f"- {item}" for item in data["next_unblock"]["promotion_requires"])
    lines.extend([
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
        "schema": f"{PASS}.v2",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "repo": str(ROOT),
        "sourceRoot": str(REDMCSB),
        "source_audit": audit_sources(),
        "inputs": {name: load_json(rel) for name, rel in INPUTS.items() if name not in {"pass376_classifier", "pass376_crop_manifest", "pass376_route_labels"}},
        "classifier": summarize_classifier(INPUTS["pass376_classifier"]),
        "crop_manifest": summarize_crop_manifest(INPUTS["pass376_crop_manifest"]),
        "route_labels": summarize_route_labels(INPUTS["pass376_route_labels"]),
        "next_unblock": unblock_commands(),
        "promotion_rule": "Promote only when pass434 readiness is green, bounded original-runtime evidence proves F0380 pop/load plus F0365/F0366 command dispatch plus G0321 stop-wait write plus a later F0128 viewport draw, and six raw/cropped route states are non-duplicate and match dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,spell_panel,dungeon_gameplay,inventory.",
        "not_claimed": ["DOSBox/live original capture during this gate", "original-vs-Firestaff pixel parity", "semantic route promotion while pass376 artifacts are quarantined"],
    }
    data["pass376_quarantine"] = quarantine_pass376_artifacts(data["classifier"], data["crop_manifest"])
    data["next_unblock_contract"] = command_contract_audit(data["next_unblock"])
    data["route_rows"] = build_route_rows(data["route_labels"], data["classifier"], data["crop_manifest"])
    data["raw_duplicate_route_indices"] = duplicate_groups(data["route_rows"], "raw_sha256")
    data["crop_duplicate_route_indices"] = duplicate_groups(data["route_rows"], "crop_sha256")
    data["blockers"] = blocker_list(data)
    data["status"] = EXPECTED_BLOCKED if data["blockers"] else READY
    MANIFEST.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(data)
    print(json.dumps({"status": data["status"], "blockers": data["blockers"], "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT))}, indent=2))
    return 0 if data["status"] == EXPECTED_BLOCKED else 1


if __name__ == "__main__":
    raise SystemExit(main())
