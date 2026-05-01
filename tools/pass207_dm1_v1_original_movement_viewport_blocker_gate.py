#!/usr/bin/env python3
"""Pass207: focused DM1 V1 original movement/viewport blocker gate.

This is the narrow follow-up to pass206.  It does not try to salvage or rerun
DOSBox captures.  It records the ReDMCSB movement -> viewport seam that an
original-faithful evidence capture must satisfy, then audits the current N2
attempt and emits an explicit blocker when that attempt is not promotable.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
PASS206_MANIFEST = ROOT / "parity-evidence/verification/pass206_dm1_v1_original_runner_minimal_gate/manifest.json"
OUT_DIR = ROOT / "parity-evidence/verification/pass207_dm1_v1_original_movement_viewport_blocker_gate"
REPORT = ROOT / "parity-evidence/pass207_dm1_v1_original_movement_viewport_blocker_gate.md"

SOURCE_CHECKS: list[dict[str, Any]] = [
    {
        "id": "title-launch-draws-original-front-door",
        "file": "TITLE.C",
        "function": "F0437_STARTEND_DrawTitle",
        "ranges": [(12, 40)],
        "needles": [
            "void F0437_STARTEND_DrawTitle",
            "unsigned char* L1384_puc_Bitmap_Title",
            "unsigned char* L1385_puc_Bitmap_LogicalScreenBase",
            "G0578_B_UseByteBoxCoordinates = C0_FALSE;",
        ],
        "claim": "Original route capture starts at the ReDMCSB title/front-door path, before any dungeon movement evidence is promotable.",
    },
    {
        "id": "entrance-wait-processes-input-queue-before-load",
        "file": "ENTRANCE.C",
        "function": "F0441_STARTEND_ProcessEntrance",
        "ranges": [(850, 883), (906, 943)],
        "needles": [
            "F0439_STARTEND_DrawEntrance();",
            "F0357_COMMAND_DiscardAllInput();",
            "G0298_B_NewGame = C099_MODE_WAITING_ON_ENTRANCE;",
            "M526_WaitVerticalBlank();",
            "F0361_COMMAND_ProcessKeyPress(F0540_INPUT_Crawcin());",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "F0022_MAIN_Delay(20);",
            "M522_MOUSE_HidePointer();",
            "F0438_STARTEND_OpenEntranceDoors();",
        ],
        "claim": "A launch/capture route must cross the source entrance input-wait loop and post-enter delay before dungeon-entry frames are evidence.",
    },
    {
        "id": "entrance-door-animation-vblank-seam",
        "file": "ENTRANCE.C",
        "function": "F0579_ENTRANCE_InitializeBitPlanes / F0580_ENTRANCE_DrawDoorAnimationStep / F0581_ENTRANCE_BlitDoors",
        "ranges": [(1095, 1147)],
        "needles": [
            "void F0579_ENTRANCE_InitializeBitPlanes",
            "G1123_puc_TargetCompositeBitmapUpperHalfPlane0",
            "void F0580_ENTRANCE_DrawDoorAnimationStep",
            "M526_WaitVerticalBlank();",
            "F0581_ENTRANCE_BlitDoors();",
            "void F0581_ENTRANCE_BlitDoors",
        ],
        "claim": "Entrance animation capture boundaries are tied to the original bitplane/vblank door-blit seam, not arbitrary screenshots.",
    },
    {
        "id": "vblank-wait-is-explicit-capture-timing-seam",
        "file": "VBLANK.C",
        "function": "F0693_WaitVerticalBlank",
        "ranges": [(626, 646)],
        "needles": [
            "void F0693_WaitVerticalBlank",
            "LDA        #C1_TRUE",
            "BNE        L0901B9",
        ],
        "claim": "Original-faithful capture timing must respect the source vblank wait seam used by entrance, highlighting, and viewport presentation.",
    },
    {
        "id": "command-discard-keypress-queue-primes-route",
        "file": "COMMAND.C",
        "function": "F0357_COMMAND_DiscardAllInput / F0361_COMMAND_ProcessKeyPress",
        "ranges": [(1305, 1325), (1709, 1812)],
        "needles": [
            "void F0357_COMMAND_DiscardAllInput",
            "while (M527_IsCharacterInKeyboardBuffer())",
            "M528_GetCharacterInKeyboardBuffer();",
            "void F0361_COMMAND_ProcessKeyPress",
            "G0435_B_CommandQueueLocked = C1_TRUE;",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G0435_B_CommandQueueLocked = C0_FALSE;",
            "F0360_COMMAND_ProcessPendingClick();",
        ],
        "claim": "Route keypress/click evidence must begin from the source discard/input queue path before movement dispatch is evaluated.",
    },
    {
        "id": "click-highlight-vblank-feedback-does-not-equal-movement",
        "file": "CLIKMENU.C",
        "function": "F0665_F0362_sub / F0363_COMMAND_HighlightBoxDisable",
        "ranges": [(8, 35), (83, 95)],
        "needles": [
            "STATICFUNCTION void F0665_F0362_sub",
            "F0698_InvertBox",
            "G0341_B_HighlightBoxEnabled = C1_TRUE;",
            "F0693_WaitVerticalBlank();",
            "void F0363_COMMAND_HighlightBoxDisable",
            "if (G0341_B_HighlightBoxEnabled)",
        ],
        "claim": "Highlight/vblank feedback can appear in captures; it is not by itself proof that the movement handler or viewport redraw seam was reached.",
    },
    {
        "id": "command-dispatch-reaches-turn-step-handlers",
        "file": "COMMAND.C",
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "ranges": [(2045, 2156)],
        "needles": [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "if ((L1160_i_Command == C002_COMMAND_TURN_RIGHT) || (L1160_i_Command == C001_COMMAND_TURN_LEFT))",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT))",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "claim": "Movement/turn shots are only original-faithful after the command queue reaches the source turn/step handlers.",
    },
    {
        "id": "turn-handler-mutates-direction-and-stops-wait",
        "file": "CLIKMENU.C",
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "ranges": [(142, 179)],
        "needles": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval",
            "F0284_CHAMPION_SetPartyDirection",
        ],
        "claim": "Turn evidence must be captured after source direction mutation and input-wait release.",
    },
    {
        "id": "step-handler-resolves-destination-and-cooldown",
        "file": "CLIKMENU.C",
        "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "ranges": [(180, 347)],
        "needles": [
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "L1116_i_SquareType == C00_ELEMENT_WALL",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        ],
        "claim": "Forward/side movement evidence must pass source legality, move-result, cooldown, and wait-release logic.",
    },
    {
        "id": "game-loop-redraws-before-next-input-wait",
        "file": "GAMELOOP.C",
        "function": "F0002_MAIN_GameLoop_CPSDF",
        "ranges": [(35, 97), (215, 219)],
        "needles": [
            "STATICFUNCTION void F0002_MAIN_GameLoop_CPSDF",
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
        ],
        "claim": "The next viewport reference must come from the game-loop redraw using the mutated party state.",
    },
    {
        "id": "viewport-draw-uses-direction-and-map-coordinates",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "ranges": [(8318, 8616)],
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "claim": "The comparable viewport is the source dungeon-view draw from current direction/X/Y, not a transient full-screen frame.",
    },
    {
        "id": "viewport-present-requests-vblank-blit",
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "ranges": [(709, 724), (840, 858)],
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "G0324_B_DrawViewportRequested = C1_TRUE",
            "M526_WaitVerticalBlank();",
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT",
        ],
        "claim": "A promotable capture must observe the presented 224x136 viewport after the source draw-request/vblank seam.",
    },
]


def read_text(path: Path) -> str:
    if not path.is_file():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding="latin-1", errors="replace")


def compact(text: str) -> str:
    return " ".join(text.split())


def source_block(file: str, ranges: list[tuple[int, int]]) -> str:
    lines = read_text(REDMCSB / file).splitlines()
    chunks: list[str] = []
    for start, end in ranges:
        chunks.extend(lines[start - 1:end])
    return "\n".join(chunks)


def audit_source() -> list[dict[str, Any]]:
    checks: list[dict[str, Any]] = []
    for check in SOURCE_CHECKS:
        text = compact(source_block(check["file"], check["ranges"]))
        missing = [needle for needle in check["needles"] if compact(needle) not in text]
        checks.append({
            "id": check["id"],
            "function": check["function"],
            "citations": [f"{check["file"]}:{a}-{b}" for a, b in check["ranges"]],
            "claim": check["claim"],
            "ok": not missing,
            "missing": missing,
        })
    return checks


def load_pass206() -> dict[str, Any]:
    if not PASS206_MANIFEST.is_file():
        return {"exists": False, "path": str(PASS206_MANIFEST), "status": "BLOCKED_MISSING_PASS206_MANIFEST"}
    doc = json.loads(PASS206_MANIFEST.read_text(encoding="utf-8"))
    attempt = doc.get("existing_attempt_audit", {})
    runner = doc.get("runner_prerequisites", {})
    return {
        "exists": True,
        "path": str(PASS206_MANIFEST),
        "pass206_status": doc.get("status"),
        "attempt_status": attempt.get("status"),
        "attempt_dir": attempt.get("attempt_dir"),
        "capture_count": attempt.get("capture_count"),
        "dimensions_seen": attempt.get("dimensions_seen"),
        "class_counts": attempt.get("class_counts"),
        "duplicate_sha256_counts_gt1": attempt.get("duplicate_sha256_counts_gt1"),
        "mismatches": attempt.get("mismatches") or [],
        "viewport_crop_ppm_count": attempt.get("viewport_crop_ppm_count"),
        "missing_tools": runner.get("missing_tools") or [],
        "canonical_files_ok": {k: v.get("ok") for k, v in (runner.get("canonical_files") or {}).items()},
        "route_events": runner.get("route_events"),
    }


def decide_status(source: list[dict[str, Any]], audit: dict[str, Any]) -> str:
    if any(not item["ok"] for item in source):
        return "FAIL_REDMCSB_SOURCE_AUDIT"
    if not audit.get("exists"):
        return "BLOCKED_MISSING_PASS206_MANIFEST"
    if audit.get("missing_tools") or not all(audit.get("canonical_files_ok", {}).values()):
        return "BLOCKED_ORIGINAL_RUNNER_PREREQUISITES"
    if audit.get("attempt_status") != "PASS_SEMANTIC_ROUTE_READY":
        return "BLOCKED_MOVEMENT_VIEWPORT_ROUTE_NOT_PROMOTABLE"
    return "PASS_MOVEMENT_VIEWPORT_ROUTE_PROMOTABLE"


def write_report(manifest: dict[str, Any], report: Path) -> None:
    audit = manifest["pass206_attempt_audit"]
    lines = [
        "# Pass207 — DM1 V1 original movement/viewport blocker gate",
        "",
        f"Status: `{manifest["status"]}`",
        "",
        "Scope: N2-only focused follow-up to pass206. This gate does **not** rerun DOSBox or salvage broad captures; it records the exact ReDMCSB movement→viewport seam and explains whether the current original-runner attempt can be promoted.",
        "",
        "## ReDMCSB source seam audit",
        "",
    ]
    for item in manifest["redmcsb_source_audit"]:
        mark = "PASS" if item["ok"] else "FAIL"
        lines.append("- {} `{}` — `{}` at {}: {}".format(mark, item["id"], item["function"], ", ".join(item["citations"]), item["claim"]))
    lines += [
        "",
        "## Current N2 original-runner attempt",
        "",
        f"- pass206 manifest: `{audit.get("path")}`",
        f"- pass206 status: `{audit.get("pass206_status")}`",
        f"- attempt status: `{audit.get("attempt_status")}`",
        f"- attempt dir: `{audit.get("attempt_dir")}`",
        f"- capture count / dimensions: `{audit.get("capture_count")}` / `{audit.get("dimensions_seen")}`",
        f"- viewport crop PPM count: `{audit.get("viewport_crop_ppm_count")}`",
        f"- class counts: `{audit.get("class_counts")}`",
        f"- duplicate SHA counts >1: `{audit.get("duplicate_sha256_counts_gt1")}`",
        f"- missing tools: `{audit.get("missing_tools")}`",
        f"- canonical files ok: `{audit.get("canonical_files_ok")}`",
        "",
        "## Blocker decision",
        "",
    ]
    mismatches = audit.get("mismatches") or []
    if manifest["status"] == "BLOCKED_MOVEMENT_VIEWPORT_ROUTE_NOT_PROMOTABLE":
        lines.append("The current attempt is **not promotable** as original-faithful movement/viewport evidence. It has the right local runner prerequisites, but the captured sequence does not remain semantically aligned after movement: classifier mismatches and repeated wall-closeup frames mean the shots cannot be tied to the ReDMCSB post-command redraw seam above.")
        lines.append("")
        if mismatches:
            lines.append("Mismatches:")
            for m in mismatches:
                lines.append(f"- shot {m.get("index")}: `{m.get("classification")}` expected `{m.get("expected")}` (`{m.get("file")}`)")
        else:
            lines.append("Mismatches: none recorded, but pass206 did not report `PASS_SEMANTIC_ROUTE_READY`.")
    elif manifest["status"].startswith("PASS"):
        lines.append("The current attempt is promotable by this narrow semantic gate; pixel parity still needs a separate gate.")
    else:
        lines.append("The gate is blocked before semantic promotion because a source/prerequisite artifact is missing or drifted.")
    lines += [
        "",
        "Non-claims: no DANNESBURK use, no push, no new capture route, no original-vs-Firestaff pixel parity claim.",
        "",
    ]
    report.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out-dir", type=Path, default=OUT_DIR)
    parser.add_argument("--report", type=Path, default=REPORT)
    args = parser.parse_args()
    source = audit_source()
    attempt = load_pass206()
    status = decide_status(source, attempt)
    manifest = {
        "schema": "pass207_dm1_v1_original_movement_viewport_blocker_gate.v1",
        "status": status,
        "worker": "N2 / firestaff-worker / trv2@192.168.3.121",
        "repo": str(ROOT),
        "redmcsb_source_root": str(REDMCSB),
        "forbidden_hosts": ["DANNESBURK", "192.168.2.126"],
        "redmcsb_source_audit": source,
        "pass206_attempt_audit": attempt,
    }
    args.out_dir.mkdir(parents=True, exist_ok=True)
    manifest_path = args.out_dir / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest, args.report)
    print(json.dumps({
        "status": status,
        "manifest": str(manifest_path),
        "report": str(args.report),
        "source_checks": len(source),
        "mismatch_count": len(attempt.get("mismatches") or []),
        "missing_tools": attempt.get("missing_tools"),
    }, indent=2, sort_keys=True))
    return 0 if status in {"PASS_MOVEMENT_VIEWPORT_ROUTE_PROMOTABLE", "BLOCKED_MOVEMENT_VIEWPORT_ROUTE_NOT_PROMOTABLE"} else 1


if __name__ == "__main__":
    raise SystemExit(main())
