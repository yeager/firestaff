#!/usr/bin/env python3
"""Pass220 DM1 V1 original post-command readiness oracle.

This gate is intentionally conservative. It does not bless a raw DOSBox
screenshot merely because pass80 calls it dungeon_gameplay. A movement capture
is ready only when the route is in party-control gameplay and each command shot
has a fresh post-command/post-redraw observable instead of the previous frame.
"""
from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
DEFAULT_ATTEMPT = ROOT / "verification-screens/pass212-n2-state-aware-movement-probe"
DEFAULT_OUT_DIR = ROOT / "parity-evidence/verification/pass220_dm1_v1_original_readiness_oracle"
DEFAULT_REPORT = ROOT / "parity-evidence/pass220_dm1_v1_original_readiness_oracle.md"

SOURCE_CHECKS: list[dict[str, Any]] = [
    {"id": "command-queue-dispatches-turn-step", "file": "COMMAND.C", "function": "F0380_COMMAND_ProcessQueue_CPSC", "ranges": [(2045, 2156)], "needles": ["void F0380_COMMAND_ProcessQueue_CPSC", "if ((L1160_i_Command == C002_COMMAND_TURN_RIGHT) || (L1160_i_Command == C001_COMMAND_TURN_LEFT))", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT))", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"], "claim": "Movement captures must be after queued input reaches source turn/step dispatch, not after xdotool delivery alone."},
    {"id": "turn-handler-mutates-direction-and-releases-wait", "file": "CLIKMENU.C", "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty", "ranges": [(142, 179)], "needles": ["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0276_SENSOR_ProcessThingAdditionOrRemoval", "F0284_CHAMPION_SetPartyDirection"], "claim": "Turn shots need a post-turn state mutation and source wait-loop release observable."},
    {"id": "step-handler-resolves-map-move-and-cooldown", "file": "CLIKMENU.C", "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty", "ranges": [(180, 347)], "needles": ["void F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE", "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;"], "claim": "Step shots need source destination resolution, move-result handling, cooldown, and wait-loop release."},
    {"id": "move-result-updates-party-map-time-and-sensors", "file": "MOVESENS.C", "function": "F0267_MOVE_GetMoveResult_CPSCE", "ranges": [(442, 818), (1553, 1793)], "needles": ["G0306_i_PartyMapX = P0560_i_DestinationMapX;", "G0307_i_PartyMapY = P0561_i_DestinationMapY;", "G0362_l_LastPartyMovementTime = G0313_ul_GameTime;", "F0276_SENSOR_ProcessThingAdditionOrRemoval"], "claim": "A move command is state-ready only once source movement can mutate party X/Y/time and process sensors."},
    {"id": "game-loop-redraws-mutated-party-state-before-wait", "file": "GAMELOOP.C", "function": "F0002_MAIN_GameLoop_CPSDF", "ranges": [(35, 97), (215, 219)], "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);", "F0380_COMMAND_ProcessQueue_CPSC();", "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"], "claim": "Screenshot must be after the loop redraws from current direction/X/Y, not during pre-command wait/menu churn."},
    {"id": "dungeon-view-draw-uses-direction-and-map-coordinates", "file": "DUNVIEW.C", "function": "F0128_DUNGEONVIEW_Draw_CPSF", "ranges": [(8318, 8616)], "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "P0183_i_Direction", "P0184_i_MapX", "P0185_i_MapY", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"], "claim": "Post-redraw means source dungeon view draw consumed direction/X/Y and requested viewport presentation."},
    {"id": "viewport-present-vblank-blit-seam", "file": "DRAWVIEW.C", "function": "F0097_DUNGEONVIEW_DrawViewport", "ranges": [(709, 724), (840, 858)], "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "G0324_B_DrawViewportRequested = C1_TRUE", "M526_WaitVerticalBlank();", "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT"], "claim": "A raw frame is promotable only after the viewport request/vblank blit seam can have presented it."},
]

COMMAND_LABELS = {
    "move_up_after_gate": "Up", "turn_right_after_gate": "Right", "move_left_after_gate": "Left",
    "turn_left_after_gate": "Left", "turn_left_2": "Left", "turn_left": "Left",
    "turn_right": "Right", "forward": "Up",
}


def compact(text: str) -> str:
    return " ".join(text.split())


def read_source(file: str) -> list[str]:
    path = REDMCSB / file
    if not path.is_file():
        raise AssertionError(f"missing ReDMCSB source file: {path}")
    return path.read_text(encoding="latin-1", errors="replace").splitlines()


def source_block(file: str, ranges: list[tuple[int, int]]) -> str:
    lines = read_source(file)
    chunks: list[str] = []
    for start, end in ranges:
        chunks.extend(lines[start - 1:end])
    return "\n".join(chunks)


def audit_source() -> list[dict[str, Any]]:
    out: list[dict[str, Any]] = []
    for check in SOURCE_CHECKS:
        text = compact(source_block(check["file"], check["ranges"]))
        missing = [needle for needle in check["needles"] if compact(needle) not in text]
        out.append({"id": check["id"], "file": check["file"], "function": check["function"], "citations": [f"{check['file']}:{a}-{b}" for a, b in check["ranges"]], "claim": check["claim"], "ok": not missing, "missing": missing})
    return out


def load_manifest(attempt_dir: Path) -> list[dict[str, str]]:
    path = attempt_dir / "original_viewport_shot_labels.tsv"
    if not path.is_file():
        return []
    with path.open(newline="", encoding="utf-8", errors="replace") as f:
        return list(csv.DictReader(f, delimiter="\t"))


def load_driver_log(attempt_dir: Path) -> list[str]:
    path = attempt_dir / "pass118_driver.log"
    return path.read_text(encoding="utf-8", errors="replace").splitlines() if path.is_file() else []


def key_seen_before_capture(log_lines: list[str], label: str, key: str) -> bool:
    key_line = f"key {key}"
    capture_token = f"capture {label} "
    last_key_index = -1
    for idx, line in enumerate(log_lines):
        if line == key_line:
            last_key_index = idx
        if capture_token in line:
            return last_key_index >= 0 and last_key_index < idx
    return False


def load_classifier(attempt_dir: Path) -> dict[str, Any]:
    for path in [attempt_dir / "pass80_movement_six_class_gate.json", attempt_dir / "pass80_original_frame_classifier.json"]:
        if path.is_file():
            doc = json.loads(path.read_text(encoding="utf-8"))
            doc["_path"] = str(path)
            return doc
    return {"_path": None, "pass": False, "problems": ["missing pass80 classifier JSON"], "captures": []}


def audit_attempt(attempt_dir: Path) -> dict[str, Any]:
    classifier = load_classifier(attempt_dir)
    manifest_rows = load_manifest(attempt_dir)
    log_lines = load_driver_log(attempt_dir)
    labels_by_file = {row.get("filename", ""): row.get("route_label", "") for row in manifest_rows}
    captures = classifier.get("captures") or []
    shot_rows: list[dict[str, Any]] = []
    blockers: list[str] = []
    prior_sha: str | None = None
    party_control_seen = False
    command_observable_count = 0

    if not attempt_dir.is_dir():
        blockers.append(f"attempt directory missing: {attempt_dir}")
    if not captures:
        blockers.append("no classifier captures found")

    for index, cap in enumerate(captures, 1):
        file_name = Path(str(cap.get("file", ""))).name
        label = labels_by_file.get(file_name, "")
        sha = str(cap.get("sha256", ""))
        cls = cap.get("classification")
        is_gameplay = cls == "dungeon_gameplay"
        is_fresh = bool(sha) and sha != prior_sha
        required_key = COMMAND_LABELS.get(label)
        has_key = key_seen_before_capture(log_lines, label, required_key) if required_key else False
        if label == "gate_confirmed_gameplay" and is_gameplay:
            party_control_seen = True
        if required_key:
            command_observable_count += 1
            if not party_control_seen:
                blockers.append(f"shot {index} `{label}` occurs before a gameplay gate confirmation")
            if not has_key:
                blockers.append(f"shot {index} `{label}` has no logged `{required_key}` command before capture")
            if not is_gameplay:
                blockers.append(f"shot {index} `{label}` classified `{cls}`, not dungeon_gameplay")
            if not is_fresh:
                blockers.append(f"shot {index} `{label}` repeats the previous raw SHA; no post-command redraw/state observable")
        shot_rows.append({"index": index, "file": file_name, "route_label": label, "classification": cls, "sha12": sha[:12], "required_key": required_key, "key_logged_before_capture": has_key if required_key else None, "fresh_vs_previous": None if prior_sha is None else is_fresh, "party_control_gate_already_seen": party_control_seen})
        prior_sha = sha

    if not party_control_seen:
        blockers.append("no `gate_confirmed_gameplay` party-control gate frame was observed")
    if command_observable_count == 0:
        blockers.append("no movement/turn command-labelled shots were found")
    for problem in classifier.get("problems") or []:
        if "duplicate raw frames" in str(problem):
            blockers.append("pass80 duplicate-frame gate failed; repeated raw SHA values cannot prove post-command redraw")
        elif "classified" in str(problem):
            blockers.append(f"pass80 classifier problem: {problem}")

    status = "PASS_POST_COMMAND_READINESS_ORACLE" if not blockers else "BLOCKED_MISSING_POST_COMMAND_REDRAW_OBSERVABLE"
    return {"attempt_dir": str(attempt_dir), "classifier_json": classifier.get("_path"), "classifier_pass": classifier.get("pass"), "class_counts": classifier.get("class_counts"), "duplicate_sha256_counts": classifier.get("duplicate_sha256_counts"), "manifest_rows": len(manifest_rows), "driver_log_exists": bool(log_lines), "status": status, "blockers": blockers, "shots": shot_rows}


def decide_status(source: list[dict[str, Any]], attempt: dict[str, Any]) -> str:
    if any(not item["ok"] for item in source):
        return "FAIL_REDMCSB_SOURCE_AUDIT"
    return attempt["status"]


def write_report(manifest: dict[str, Any], report: Path) -> None:
    attempt = manifest["attempt_audit"]
    lines = ["# Pass220 — DM1 V1 original post-command readiness oracle", "", f"Status: `{manifest['status']}`", "", "Scope: N2 original-runner movement capture readiness. This is a strict oracle/gate, not a new PNG/PPM capture and not a pixel-parity claim.", "", "## ReDMCSB readiness seam", ""]
    for item in manifest["redmcsb_source_audit"]:
        mark = "PASS" if item["ok"] else "FAIL"
        lines.append(f"- {mark} `{item['id']}` — `{item['function']}` at {', '.join(item['citations'])}: {item['claim']}")
    lines += ["", "## Oracle rule", "", "A screenshot is party-control/post-command ready only when all of these hold:", "", "1. a prior `gate_confirmed_gameplay` frame has been classified as `dungeon_gameplay`;", "2. the shot label is bound to a movement/turn command in the driver log before capture;", "3. the shot itself is classified as `dungeon_gameplay`;", "4. the raw SHA differs from the previous shot, proving at least one fresh presented frame after the command;", "5. pass80 duplicate-frame strictness is clean for the movement sequence.", "", "This visual oracle is intentionally weaker than a future memory oracle for `G0308_i_PartyDirection`, `G0306_i_PartyMapX`, `G0307_i_PartyMapY`, `G0321_B_StopWaitingForPlayerInput`, and `G0305_ui_PartyChampionCount`, but it blocks the known pass211/pass212 false positives.", "", "## Attempt audit", "", f"- attempt dir: `{attempt['attempt_dir']}`", f"- classifier JSON: `{attempt['classifier_json']}`", f"- classifier pass: `{attempt['classifier_pass']}`", f"- class counts: `{attempt['class_counts']}`", f"- duplicate SHA counts: `{attempt['duplicate_sha256_counts']}`", f"- manifest rows: `{attempt['manifest_rows']}`", f"- driver log exists: `{attempt['driver_log_exists']}`", "", "| # | label | class | sha12 | key | key before capture | fresh vs previous |", "|---|-------|-------|-------|-----|--------------------|-------------------|"]
    for shot in attempt["shots"]:
        lines.append(f"| {shot['index']} | `{shot['route_label']}` | `{shot['classification']}` | `{shot['sha12']}` | `{shot['required_key'] or ''}` | `{shot['key_logged_before_capture']}` | `{shot['fresh_vs_previous']}` |")
    lines += ["", "## Decision", ""]
    if attempt["blockers"]:
        lines.append("Blocked: the current captures are gameplay-classified but do not expose post-command redraw/state readiness.")
        lines.append("")
        for blocker in attempt["blockers"]:
            lines.append(f"- {blocker}")
    else:
        lines.append("PASS: the route has a fresh gameplay frame after every logged movement/turn command.")
    lines += ["", "Non-claims: no DANNESBURK use, no push, no PNG/PPM committed, no original-vs-Firestaff pixel parity claim.", ""]
    report.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--attempt-dir", type=Path, default=DEFAULT_ATTEMPT)
    parser.add_argument("--out-dir", type=Path, default=DEFAULT_OUT_DIR)
    parser.add_argument("--report", type=Path, default=DEFAULT_REPORT)
    args = parser.parse_args()
    source = audit_source()
    attempt = audit_attempt(args.attempt_dir)
    status = decide_status(source, attempt)
    manifest = {"schema": "pass220_dm1_v1_original_readiness_oracle.v1", "status": status, "repo": str(ROOT), "redmcsb_source_root": str(REDMCSB), "forbidden_hosts": ["DANNESBURK", "192.168.2.126"], "redmcsb_source_audit": source, "attempt_audit": attempt}
    args.out_dir.mkdir(parents=True, exist_ok=True)
    manifest_path = args.out_dir / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest, args.report)
    print(json.dumps({"status": status, "manifest": str(manifest_path), "report": str(args.report), "source_checks": len(source), "blocker_count": len(attempt["blockers"])}, indent=2, sort_keys=True))
    return 0 if status in {"PASS_POST_COMMAND_READINESS_ORACLE", "BLOCKED_MISSING_POST_COMMAND_REDRAW_OBSERVABLE"} else 1


if __name__ == "__main__":
    raise SystemExit(main())
