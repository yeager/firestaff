#!/usr/bin/env python3
from __future__ import annotations

import json
import os
import pathlib
import subprocess
from typing import Any

ROOT = pathlib.Path(__file__).resolve().parents[1]
PASS = "pass508_dm1_v1_movement_remaining_gap_after_pass374"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / (PASS + ".md")
REDMCSB = pathlib.Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", str(pathlib.Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")))
COMPLETION = ROOT / "parity-evidence/verification/firestaff_completion_matrix.json"
COMPLETION_DOC = ROOT / "docs/parity/COMPLETION_MATRIX.md"
PARITY_DOC = ROOT / "docs/parity/PARITY_MATRIX_DM1_V1.md"
PASS373 = ROOT / "parity-evidence/verification/pass373_dm1_v1_launcher_viewport_redraw_wall_occlusion_path/manifest.json"
PASS374 = ROOT / "parity-evidence/verification/pass374_dm1_v1_completion_viewport_wall_credit/manifest.json"
PASS207 = ROOT / "parity-evidence/verification/pass207_dm1_v1_original_movement_viewport_blocker_gate/manifest.json"
EXPECTED_STATUS = "BLOCKED_PASS508_DM1_V1_MOVEMENT_REMAINING_ORIGINAL_OVERLAY_GAP_PROVED"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {"id": "f0380-dequeues-and-dispatches-route-keys", "file": "COMMAND.C", "lines": "2045-2156", "function": "F0380_COMMAND_ProcessQueue_CPSC", "claim": "original route proof must show keyboard/click queue reaching the turn/move dispatch boundary", "markers": ["void F0380_COMMAND_ProcessQueue_CPSC(", "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT)", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"]},
    {"id": "f0365-turn-mutates-direction-before-redraw", "file": "CLIKMENU.C", "lines": "142-179", "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty", "claim": "turn overlay proof must bind original capture to source direction mutation", "markers": ["void F0365_COMMAND_ProcessTypes1To2_TurnParty(", "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX", "F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;"]},
    {"id": "f0366-step-legality-move-result-and-cooldown", "file": "CLIKMENU.C", "lines": "180-347", "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty", "claim": "step overlay proof must bind original capture to destination legality, F0267 movement, stamina, cooldown, and input-wait release", "markers": ["void F0366_COMMAND_ProcessTypes3To6_MoveParty(", "F0325_CHAMPION_DecrementStamina", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE", "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;"]},
    {"id": "f0267-commits-party-tuple-and-scent-timing", "file": "MOVESENS.C", "lines": "738-818", "function": "F0267_MOVE_GetMoveResult_CPSCE", "claim": "movement parity must observe committed original party tuple and timing side effects", "markers": ["G0397_i_MoveResultMapX = P0560_i_DestinationMapX;", "G0398_i_MoveResultMapY = P0561_i_DestinationMapY;", "G0399_ui_MoveResultMapIndex = L0715_ui_MapIndexDestination;", "F0317_CHAMPION_AddScentStrength", "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;"]},
    {"id": "f0002-redraws-viewport-from-mutated-party-state", "file": "GAMELOOP.C", "lines": "35-97,215-219", "function": "F0002_MAIN_GameLoop_CPSDF", "claim": "original overlay comparison must use post-command viewport redraw from mutated source party state", "markers": ["STATICFUNCTION void F0002_MAIN_GameLoop_CPSDF(", "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);", "F0380_COMMAND_ProcessQueue_CPSC();", "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"]},
]

def compact(value: str) -> str:
    return " ".join(value.split())

def read_text(path: pathlib.Path, encoding: str = "utf-8") -> str:
    if not path.is_file():
        raise AssertionError("missing required file: " + str(path))
    return path.read_text(encoding=encoding, errors="replace")

def source_excerpt(file_name: str, ranges: str) -> str:
    lines = read_text(REDMCSB / file_name, "latin-1").splitlines()
    chunks: list[str] = []
    for span in ranges.split(","):
        start_s, end_s = span.split("-", 1)
        start, end = int(start_s), int(end_s)
        chunks.extend(lines[start - 1:end])
    return "\n".join(chunks)

def audit_source() -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for lock in SOURCE_LOCKS:
        excerpt = compact(source_excerpt(lock["file"], lock["lines"]))
        missing = [marker for marker in lock["markers"] if compact(marker) not in excerpt]
        item = dict(lock)
        item["ok"] = not missing
        item["missingMarkers"] = missing
        item.pop("markers", None)
        results.append(item)
    return results

def load_json(path: pathlib.Path) -> dict[str, Any]:
    return json.loads(read_text(path))

def pass_status(path: pathlib.Path) -> str | None:
    return load_json(path).get("status") if path.is_file() else None

def run(cmd: list[str]) -> dict[str, Any]:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120)
    return {"cmd": cmd, "returncode": proc.returncode, "outputTail": proc.stdout[-4000:]}

def completion_dm1() -> dict[str, Any]:
    for row in load_json(COMPLETION)["rows"]:
        if row["target"] == "DM1 V1":
            return row
    raise AssertionError("missing DM1 V1 completion row")

def audit_current_evidence() -> list[dict[str, Any]]:
    dm1 = completion_dm1()
    core_score, core_note = dm1["scores"]["core_input_movement"]
    viewport_score, _viewport_note = dm1["scores"]["viewport_ui_render"]
    original_score, original_note = dm1["scores"]["original_overlay_regression"]
    parity_text = read_text(PARITY_DOC)
    completion_doc = read_text(COMPLETION_DOC)
    checks = [
        {"id": "pass373-live-route-wall-redraw-green", "ok": pass_status(PASS373) == "PASS373_LAUNCHER_VIEWPORT_REDRAW_WALL_OCCLUSION_PATH_PROVED", "path": str(PASS373.relative_to(ROOT)), "status": pass_status(PASS373)},
        {"id": "pass374-completion-credit-green", "ok": pass_status(PASS374) == "PASS374_DM1_V1_VIEWPORT_WALL_COMPLETION_CREDIT_PROVED", "path": str(PASS374.relative_to(ROOT)), "status": pass_status(PASS374)},
        {"id": "completion-matrix-current-after-pass374", "ok": dm1["completionPercent"] >= 58 and dm1["points"] >= 58 and core_score == 13 and viewport_score >= 12 and original_score == 0, "observed": {"completionPercent": dm1["completionPercent"], "points": dm1["points"], "core_input_movement": core_score, "viewport_ui_render": viewport_score, "original_overlay_regression": original_score}},
        {"id": "completion-notes-name-next-gap", "ok": "original DOS keyboard-buffer transcript/overlay proof" in core_note and "Representative original runtime overlay parity is not yet proven" in original_note, "coreNote": core_note, "originalOverlayNote": original_note},
        {"id": "docs-carry-narrowed-nonclaim", "ok": "Representative original runtime movement/HUD/viewport overlay parity missing" in completion_doc and "full original-behavior/pixel parity is not claimed" in parity_text},
    ]
    status = pass_status(PASS207)
    checks.append({"id": "prior-original-route-blocker-consulted", "ok": (status is None) or status in {"BLOCKED_MOVEMENT_VIEWPORT_ROUTE_NOT_PROMOTABLE", "SUPERSEDED_BY_PASS304_PASS308_STATE_ORACLE_PENDING", "PASS_MOVEMENT_VIEWPORT_ROUTE_PROMOTABLE"}, "path": str(PASS207.relative_to(ROOT)), "status": status or "not_present_in_this_worktree"})
    return checks

def write_report(manifest: dict[str, Any]) -> None:
    lines = ["# Pass508 - DM1 V1 movement remaining gap after pass373/pass374", "", "Status: " + manifest["status"], "", "Scope: movement/forflyttning evidence only. This pass consumes pass373/pass374 and proves the next remaining gap; it does not promote pixel parity.", "", "## ReDMCSB source audit first", ""]
    for item in manifest["redmcsbSourceAudit"]:
        state = "PASS" if item["ok"] else "FAIL"
        lines.append("- {} {}:{} - {}: {}.".format(state, item["file"], item["lines"], item["function"], item["claim"]))
    lines += ["", "## Current completion evidence consumed", ""]
    for item in manifest["currentEvidenceChecks"]:
        state = "PASS" if item["ok"] else "FAIL"
        lines.append("- {} {}".format(state, item["id"]))
    lines += ["", "## Remaining movement parity gap", "", "The next remaining gap is not Firestaff's live movement route: pass373/pass374 already credit that route into source-locked wall/door/occlusion redraw. The remaining movement gap is original-backed proof: a DOS PC/I34E keyboard-buffer or route transcript that reaches F0380, F0365/F0366, F0267, then the post-command F0128 viewport redraw, plus representative movement/HUD/viewport overlay captures tied to that tuple.", "", "Promotion requirements:", "", "- materialized original runtime frames or trace records, not ignored/absent capture assets", "- command-specific route labels for turn and step commands", "- post-vblank viewport frame hashes/crops that differ where the command changes direction or position", "- party tuple evidence: direction, map index, X, Y before and after source movement", "- explicit non-claim boundary for Firestaff-only source-equivalent tests until those original artifacts exist", "", "Missing tools/artifacts if blocked: no required executable is missing in this worktree; the missing item is original runtime evidence (DOS PC/I34E keyboard-buffer/F0380 transcript and representative original movement/HUD/viewport overlay captures).", ""]
    REPORT.write_text("\n".join(lines), encoding="utf-8")

def main() -> int:
    source = audit_source()
    evidence = audit_current_evidence()
    gates = [run(["python3", "tools/verify_firestaff_completion_matrix.py"]), run(["python3", "tools/firestaff_completion_status.py"])]
    status = EXPECTED_STATUS if all(i["ok"] for i in source) and all(i["ok"] for i in evidence) and all(g["returncode"] == 0 for g in gates) else "FAIL_PASS508_DM1_V1_MOVEMENT_REMAINING_GAP_AUDIT"
    manifest = {"schema": PASS + ".v1", "status": status, "repo": str(ROOT), "branch": run(["git", "branch", "--show-current"])["outputTail"].strip(), "redmcsbSourceRoot": str(REDMCSB), "afterEvidence": ["pass373", "pass374"], "remainingGap": {"id": "original_runtime_movement_overlay_and_keyboard_buffer_transcript", "missingArtifacts": ["DOS PC/I34E keyboard-buffer/F0380 route transcript for representative movement keys", "representative original movement/HUD/viewport overlay captures tied to pre/post party tuple"], "missingTools": [], "notClaimed": ["pixel-perfect movement/HUD/viewport parity", "original overlay regression", "binary-level direct F0380 body proof"]}, "redmcsbSourceAudit": source, "currentEvidenceChecks": evidence, "gates": gates}
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": status, "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT)), "sourceChecks": len(source), "currentEvidenceChecks": len(evidence), "missingTools": []}, indent=2, sort_keys=True))
    return 0 if status == EXPECTED_STATUS else 1

if __name__ == "__main__":
    raise SystemExit(main())
