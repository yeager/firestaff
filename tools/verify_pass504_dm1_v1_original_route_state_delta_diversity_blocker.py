#!/usr/bin/env python3
"""Pass504: DM1 V1 original route-state diversity/post-command delta blocker."""
from __future__ import annotations

import json
import os
from collections import Counter
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass504_dm1_v1_original_route_state_delta_diversity_blocker"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
STATUS = "BLOCKED_PASS504_ORIGINAL_ROUTE_STATE_DELTA_DIVERSITY_NOT_PROVEN"

INPUTS = {
    "pass435": ROOT / "parity-evidence/verification/pass435_dm1_v1_semantic_original_route_readiness_gate/manifest.json",
    "pass498": ROOT / "parity-evidence/verification/pass498_dm1_v1_original_post_command_state_delta_boundary/manifest.json",
    "classifier": ROOT / "verification-screens/pass376-original-route/pass80_original_frame_classifier.json",
    "crop_manifest": ROOT / "verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv",
}
EXPECTED_SEQUENCE = ["dungeon_gameplay", "dungeon_gameplay", "dungeon_gameplay", "spell_panel", "dungeon_gameplay", "inventory"]

SOURCE_LOCKS = [
    {
        "id": "outer_loop_draws_then_waits_for_command_delta",
        "file": "GAMELOOP.C",
        "lines": "90,164,215-219",
        "function": "F0002_MAIN_GameLoop_CPSDF",
        "needles": [
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
            "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
        ],
        "meaning": "A useful screenshot must be after F0380 has changed state and the next outer-loop F0128 redraw consumes that new party tuple.",
    },
    {
        "id": "f0380_pops_queue_and_dispatches_turn_or_move",
        "file": "COMMAND.C",
        "lines": "2045-2156",
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "needles": [
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command",
            "G2153_i_QueuedCommandsCount--",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "meaning": "Route labels are not evidence unless the original run proves this queue pop/load and the matching F0365/F0366 branch.",
    },
    {
        "id": "f0365_turn_sets_stop_wait_and_mutates_direction",
        "file": "CLIKMENU.C",
        "lines": "142-174",
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "needles": ["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0284_CHAMPION_SetPartyDirection"],
        "meaning": "A turn state delta must show the stop-wait write and the party-direction mutation.",
    },
    {
        "id": "f0366_move_sets_stop_wait_and_can_cancel_on_block",
        "file": "CLIKMENU.C",
        "lines": "180-323",
        "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "needles": ["void F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "G0321_B_StopWaitingForPlayerInput = C0_FALSE;"],
        "meaning": "A movement capture must distinguish accepted movement from source-visible blocked/no-op movement.",
    },
    {
        "id": "f0128_builds_viewport_from_direction_x_y",
        "file": "DUNVIEW.C",
        "lines": "8318-8610",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "P0183_i_Direction", "P0184_i_MapX", "P0185_i_MapY", "F0127_DUNGEONVIEW_DrawSquareD0C", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"],
        "meaning": "The frame diversity check is only meaningful if the frame came from this updated direction/X/Y composition.",
    },
    {
        "id": "f0097_pc34_present_boundary",
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "F0638_GetZone(C007_ZONE_VIEWPORT", "VIDRV_09_BlitViewPort"],
        "meaning": "A crop belongs after the PC34 viewport present boundary, not after setup/menu echo.",
    },
]


def norm(text: str) -> str:
    return " ".join(text.split())


def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            start, end = [int(value) for value in part.split("-", 1)]
        else:
            start = end = int(part)
        chunks.append("\n".join(lines[start - 1:end]))
    return "\n".join(chunks)


def source_audit() -> list[dict[str, Any]]:
    rows = []
    for lock in SOURCE_LOCKS:
        path = REDMCSB / lock["file"]
        text = source_window(path, lock["lines"]) if path.exists() else ""
        missing = [needle for needle in lock["needles"] if norm(needle) not in norm(text)]
        rows.append({**lock, "path": str(path), "exists": path.exists(), "ok": path.exists() and not missing, "missing": missing})
    return rows


def rel(path: Path) -> str:
    return str(path.relative_to(ROOT))


def read_json(path: Path) -> dict[str, Any]:
    if not path.exists():
        return {"exists": False, "path": rel(path)}
    return {"exists": True, "path": rel(path), **json.loads(path.read_text(encoding="utf-8"))}


def read_crop_rows(path: Path) -> list[dict[str, Any]]:
    if not path.exists():
        return []
    rows = []
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("#"):
            continue
        kind, filename, width, height, size, sha = line.split("\t")
        rows.append({"kind": kind, "filename": filename, "width": int(width), "height": int(height), "bytes": int(size), "sha256": sha})
    return rows


def duplicate_index_groups(values: list[str | None]) -> dict[str, list[int]]:
    groups: dict[str, list[int]] = {}
    for index, value in enumerate(values, start=1):
        if value:
            groups.setdefault(value, []).append(index)
    return {value: indices for value, indices in groups.items() if len(indices) > 1}


def classify_current_artifacts(classifier: dict[str, Any], crop_rows: list[dict[str, Any]]) -> dict[str, Any]:
    captures = classifier.get("captures") or []
    classes = [row.get("classification") for row in captures]
    raw_hashes = [row.get("sha256") for row in captures]
    crop_hashes = [row.get("sha256") for row in crop_rows]
    route_rows = []
    for index in range(max(len(EXPECTED_SEQUENCE), len(captures), len(crop_rows))):
        capture = captures[index] if index < len(captures) else {}
        crop = crop_rows[index] if index < len(crop_rows) else {}
        expected = EXPECTED_SEQUENCE[index] if index < len(EXPECTED_SEQUENCE) else None
        route_rows.append({
            "index": index + 1,
            "expectedClass": expected,
            "actualClass": capture.get("classification"),
            "classMatches": capture.get("classification") == expected,
            "rawFile": capture.get("file"),
            "rawSha256": capture.get("sha256"),
            "cropFile": crop.get("filename"),
            "cropSha256": crop.get("sha256"),
        })
    return {
        "expectedSequence": EXPECTED_SEQUENCE,
        "actualClasses": classes,
        "classSequenceMatches": classes == EXPECTED_SEQUENCE,
        "rawCaptureCount": len(captures),
        "cropCount": len(crop_rows),
        "cropRowsAll224x136": len(crop_rows) == 6 and all(row["kind"] == "original_viewport_224x136" and row["width"] == 224 and row["height"] == 136 for row in crop_rows),
        "rawDuplicateSha256Counts": {sha: count for sha, count in Counter(raw_hashes).items() if sha and count > 1},
        "cropDuplicateSha256Counts": {sha: count for sha, count in Counter(crop_hashes).items() if sha and count > 1},
        "rawDuplicateRouteIndices": duplicate_index_groups(raw_hashes),
        "cropDuplicateRouteIndices": duplicate_index_groups(crop_hashes),
        "routeRows": route_rows,
    }


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    pass435 = read_json(INPUTS["pass435"])
    pass498 = read_json(INPUTS["pass498"])
    classifier = read_json(INPUTS["classifier"])
    crop_rows = read_crop_rows(INPUTS["crop_manifest"])
    artifact_state = classify_current_artifacts(classifier, crop_rows)
    source = source_audit()

    required_next_evidence = [
        "capture route begins from a verified post-entry gameplay state, not an entrance/setup echo",
        "each command label is backed by original F0380 queue pop/load/decrement evidence",
        "turn commands prove F0365 and direction mutation; movement commands prove F0366 or source-visible blocked/no-op handling",
        "G0321 stop-wait and game-time tick let the wait loop exit before the sampled frame",
        "the sampled frame is after the later F0128 and F0097/VIDRV present boundary for the same command",
        "six raw 320x200 frames and six 224x136 viewport crops are class-matching and non-duplicate by hash",
    ]

    blocker_reasons: list[str] = []
    if not all(row["ok"] for row in source):
        blocker_reasons.append("ReDMCSB source audit failed")
    if pass435.get("status") != "BLOCKED_PASS435_SEMANTIC_ORIGINAL_ROUTE_NOT_READY":
        blocker_reasons.append("pass435 did not report the expected semantic-original-route blocker")
    if pass498.get("status") != "PASS498_ORIGINAL_CAPTURE_BLOCKER_NARROWED_TO_POST_COMMAND_STATE_DELTA":
        blocker_reasons.append("pass498 did not narrow the blocker to post-command state delta")
    if not artifact_state["classSequenceMatches"]:
        blocker_reasons.append("current route classes do not match the expected six-state semantic sequence")
    if artifact_state["rawDuplicateSha256Counts"]:
        blocker_reasons.append("current raw route captures contain duplicate frame hashes")
    if artifact_state["cropDuplicateSha256Counts"]:
        blocker_reasons.append("current viewport crops contain duplicate hashes")
    if not artifact_state["cropRowsAll224x136"]:
        blocker_reasons.append("current crop manifest is not exactly six 224x136 viewport crops")

    remaining = (
        "Original route-state diversity is still blocked at the post-command state-delta proof boundary. "
        "The current pass376/pass435 artifacts are useful as quarantined regression inputs, but they do not prove "
        "that distinct raw/cropped states came after source-visible F0380 -> F0365/F0366 -> G0321 -> F0128/F0097 transitions."
    )
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "generatedBy": "tools/verify_pass504_dm1_v1_original_route_state_delta_diversity_blocker.py",
        "status": STATUS,
        "ok": True,
        "repo": str(ROOT),
        "sourceRoot": str(REDMCSB),
        "sourceAudit": source,
        "inputs": {"pass435": pass435.get("path"), "pass498": pass498.get("path"), "classifier": classifier.get("path"), "cropManifest": rel(INPUTS["crop_manifest"])},
        "observed": {"pass435Status": pass435.get("status"), "pass435Blockers": pass435.get("blockers"), "pass498Status": pass498.get("status"), "pass498NarrowedBlocker": pass498.get("narrowedBlocker"), "artifacts": artifact_state},
        "blockerReasons": blocker_reasons,
        "remainingBlocker": remaining,
        "requiredNextEvidence": required_next_evidence,
        "nonClaims": ["no DOSBox/original runtime was launched by pass504", "no original-vs-Firestaff pixel parity promotion", "no claim that movement or viewport source handlers are broken"],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = ["# Pass504 - DM1 V1 original route-state diversity blocker", "", f"Status: {STATUS}", "", "## Decision", "", remaining, "", "## ReDMCSB source audit", ""]
    for row in source:
        lines.append(f"- {row['file']}:{row['lines']} {row['function']} - ok={row['ok']}; {row['meaning']}")
    lines.extend(["", "## Current evidence", "", f"- pass435 status: {pass435.get('status')}", f"- pass498 status: {pass498.get('status')}", f"- class sequence matches expected: {artifact_state['classSequenceMatches']}", f"- raw duplicate route indices: {artifact_state['rawDuplicateRouteIndices']}", f"- crop duplicate route indices: {artifact_state['cropDuplicateRouteIndices']}", f"- crop rows all 224x136: {artifact_state['cropRowsAll224x136']}", "", "## Route matrix", "", "| # | expected | actual | class ok | raw sha | crop sha |", "|---|---|---|---|---|---|"])
    for row in artifact_state["routeRows"]:
        raw = (row.get("rawSha256") or "")[:12]
        crop = (row.get("cropSha256") or "")[:12]
        lines.append(f"| {row['index']} | {row.get('expectedClass')} | {row.get('actualClass')} | {row.get('classMatches')} | {raw} | {crop} |")
    lines.extend(["", "## Remaining blocker", ""])
    lines.extend(f"- {reason}" for reason in blocker_reasons)
    lines.extend(["", "## Required next evidence", ""])
    lines.extend(f"- {item}" for item in required_next_evidence)
    lines.extend(["", f"Manifest: {rel(MANIFEST)}"])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(json.dumps({"status": STATUS, "manifest": rel(MANIFEST), "report": rel(REPORT), "blockerReasons": blocker_reasons}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
