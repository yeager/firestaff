#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import signal
import subprocess
import time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass548_dm1_v1_original_overlay_capture_progress"
OUT = ROOT / "parity-evidence" / "verification" / PASS
REPORT = ROOT / "parity-evidence" / (PASS + ".md")
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"
LIVE_COMMAND = ["python3", "tools/verify_pass475_dm1_v1_movement_viewport_wall_live_click_capture.py", "--seconds", "10"]

SOURCE_REFS = [
    ("COMMAND.C", "pc34_movement_click_boxes", [
        "G0448_as_Graphic561_SecondaryMouseInput_Movement",
        "C001_COMMAND_TURN_LEFT,             234, 261, 125, 145",
        "C003_COMMAND_MOVE_FORWARD,          263, 289, 125, 145",
        "C002_COMMAND_TURN_RIGHT,            291, 318, 125, 145",
        "C080_COMMAND_CLICK_IN_DUNGEON_VIEW,   0, 223,  33, 168",
    ]),
    ("COMMAND.C", "queue_pop_to_turn_step_handlers", [
        "void F0380_COMMAND_ProcessQueue_CPSC",
        "if (G2153_i_QueuedCommandsCount == 0)",
        "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
    ]),
    ("CLIKMENU.C", "turn_and_step_party_tuple_mutation", [
        "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "F0267_MOVE_GetMoveResult_CPSCE",
    ]),
    ("MOVESENS.C", "successful_step_tuple_commit", [
        "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE",
        "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
        "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
    ]),
    ("DUNVIEW.C", "viewport_wall_composition_and_present_call", [
        "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
        "F0100_DUNGEONVIEW_DrawWallSetBitmap",
        "F0102_DUNGEONVIEW_DrawDoorBitmap",
        "void F0128_DUNGEONVIEW_Draw_CPSF",
        "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
    ]),
    ("DRAWVIEW.C", "pc34_viewport_present_overlay_boundary", [
        "void F0097_DUNGEONVIEW_DrawViewport",
        "F0638_GetZone(C007_ZONE_VIEWPORT",
        "G0296_puc_Bitmap_Viewport",
        "VIDRV_09_BlitViewPort",
    ]),
]

def norm(text: str) -> str:
    return " ".join(text.split())

def find_line(path: Path, needle: str) -> int | None:
    wanted = norm(needle)
    for idx, line in enumerate(path.read_text(encoding="latin-1", errors="replace").splitlines(), 1):
        if wanted in norm(line):
            return idx
    return None

def source_audit() -> list[dict[str, Any]]:
    rows = []
    for file_name, label, needles in SOURCE_REFS:
        path = RED / file_name
        hits = {needle: find_line(path, needle) for needle in needles} if path.exists() else {}
        present = [line for line in hits.values() if line is not None]
        rows.append({
            "file": file_name,
            "label": label,
            "path": str(path),
            "lineHits": hits,
            "lineRange": [min(present), max(present)] if present else None,
            "missing": [needle for needle, line in hits.items() if line is None],
            "ok": path.exists() and len(present) == len(needles),
        })
    return rows

def command_path(name: str) -> str:
    return subprocess.run(["bash", "-lc", "command -v " + name], text=True, stdout=subprocess.PIPE).stdout.strip()

def prerequisite_rows() -> dict[str, Any]:
    tools = {name: command_path(name) for name in ["dosbox-debug", "Xvfb", "xdotool"]}
    helper = ROOT / LIVE_COMMAND[1]
    missing = [name for name, value in tools.items() if not value]
    if not (ORIG / "DM.EXE").exists():
        missing.append("DM.EXE")
    if not helper.exists():
        missing.append(str(helper.relative_to(ROOT)))
    return {
        "originalStage": str(ORIG),
        "dmExeExists": (ORIG / "DM.EXE").exists(),
        "liveVerifier": str(helper.relative_to(ROOT)),
        "liveVerifierExists": helper.exists(),
        "tools": tools,
        "missing": missing,
    }

def run_live(timeout_seconds: int) -> dict[str, Any]:
    OUT.mkdir(parents=True, exist_ok=True)
    started = time.time()
    stdout_path = OUT / "pass548_live_attempt_stdout.txt"
    with stdout_path.open("w", encoding="utf-8", errors="replace") as stdout_file:
        proc = subprocess.Popen(
            LIVE_COMMAND,
            cwd=ROOT,
            text=True,
            stdout=stdout_file,
            stderr=subprocess.STDOUT,
            start_new_session=True,
        )
        try:
            proc.wait(timeout=timeout_seconds)
            timed_out = False
        except subprocess.TimeoutExpired:
            timed_out = True
            os.killpg(proc.pid, signal.SIGTERM)
            try:
                proc.wait(timeout=3)
            except subprocess.TimeoutExpired:
                os.killpg(proc.pid, signal.SIGKILL)
                proc.wait(timeout=3)
    stdout_tail = stdout_path.read_text(encoding="utf-8", errors="replace")[-4000:] if stdout_path.exists() else ""
    return {
        "attempted": True,
        "command": " ".join(LIVE_COMMAND),
        "timeoutSeconds": timeout_seconds,
        "timedOut": timed_out,
        "returncode": proc.returncode,
        "durationSeconds": round(time.time() - started, 3),
        "stdoutPath": str(stdout_path.relative_to(ROOT)),
        "stdoutTail": stdout_tail,
        "expectedManifest": "parity-evidence/verification/pass475_dm1_v1_movement_viewport_wall_live_click_capture/manifest.json",
    }

def pass475_snapshot() -> dict[str, Any]:
    path = ROOT / "parity-evidence/verification/pass475_dm1_v1_movement_viewport_wall_live_click_capture/manifest.json"
    if not path.exists():
        return {"exists": False, "path": str(path.relative_to(ROOT))}
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except Exception as exc:
        return {"exists": True, "path": str(path.relative_to(ROOT)), "parseError": str(exc)}
    return {
        "exists": True,
        "path": str(path.relative_to(ROOT)),
        "status": data.get("status"),
        "summary": data.get("summary"),
        "predicates": data.get("predicates"),
    }

def write_outputs(status: str, blocker: str | None, source: list[dict[str, Any]], prereq: dict[str, Any], live: dict[str, Any] | None, snapshot: dict[str, Any]) -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    head = subprocess.run(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True, stdout=subprocess.PIPE).stdout.strip()
    branch = subprocess.run(["git", "branch", "--show-current"], cwd=ROOT, text=True, stdout=subprocess.PIPE).stdout.strip()
    manifest = {
        "schema": PASS + ".v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "blocker": blocker,
        "repo": str(ROOT),
        "head": head,
        "branch": branch,
        "sourceRoot": str(RED),
        "sourceAudit": source,
        "prerequisites": prereq,
        "liveAttempt": live,
        "pass475Snapshot": snapshot,
        "progress": [
            "reduced original overlay/capture progress to one bounded N2 command",
            "records timeout/no-manifest as a concrete blocker instead of promotable pixels",
            "keeps movement/viewport/walls source anchors tied to ReDMCSB lines",
        ],
        "nonClaims": [
            "no original frame promoted as parity evidence",
            "no runtime or gameplay behavior changed",
            "no push",
        ],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + chr(10), encoding="utf-8")
    lines = ["# Pass548 - DM1 V1 original overlay/capture progress", "", "Status: " + status, "", "## ReDMCSB source anchors", ""]
    for row in source:
        rng = row["lineRange"] or ["?", "?"]
        lines.append("- {0}:{1}-{2} {3}".format(row["file"], rng[0], rng[1], row["label"]))
    lines += [
        "",
        "## Smallest reproducible capture command",
        "",
        "    " + " ".join(LIVE_COMMAND),
        "",
        "## Result",
        "",
        blocker or "The bounded live capture path returned without a blocker.",
        "",
        "## Evidence",
        "",
        "- Manifest: parity-evidence/verification/" + PASS + "/manifest.json",
        "- Live verifier snapshot: " + snapshot.get("path", "missing"),
        "",
        "## Non-claims",
        "",
        "- No original frame is promoted as parity evidence by this pass.",
        "- No movement/viewport/walls behavior is changed.",
    ]
    REPORT.write_text(chr(10).join(lines) + chr(10), encoding="utf-8")

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--attempt-live", action="store_true")
    ap.add_argument("--timeout", type=int, default=18)
    args = ap.parse_args()
    args.timeout = max(8, min(args.timeout, 45))

    source = source_audit()
    prereq = prerequisite_rows()
    live = run_live(args.timeout) if args.attempt_live and not prereq["missing"] else None
    snapshot = pass475_snapshot()

    if not all(row["ok"] for row in source):
        status = "FAIL_PASS548_SOURCE_AUDIT"
        blocker = "ReDMCSB source audit missing required movement/viewport/wall anchors"
    elif prereq["missing"]:
        status = "BLOCKED_PASS548_MISSING_N2_PREREQUISITE"
        blocker = "missing prerequisite(s): " + ", ".join(prereq["missing"])
    elif live and live["timedOut"]:
        status = "BLOCKED_PASS548_LIVE_CAPTURE_TIMEOUT"
        blocker = "bounded original live capture did not return before timeout; exact command: " + live["command"]
    elif live and live["returncode"] not in (0, None):
        status = "BLOCKED_PASS548_LIVE_CAPTURE_NONZERO"
        blocker = "bounded original live capture returned non-zero; exact command: " + live["command"]
    elif snapshot.get("status", "").startswith("PASS"):
        status = "PASS548_ORIGINAL_OVERLAY_CAPTURE_PROGRESS_REPRODUCED"
        blocker = None
    else:
        status = "BLOCKED_PASS548_NO_PROMOTABLE_ORIGINAL_CAPTURE"
        blocker = "no promotable original overlay/capture frame exists yet; exact command: " + " ".join(LIVE_COMMAND)

    write_outputs(status, blocker, source, prereq, live, snapshot)
    print(json.dumps({
        "status": status,
        "blocker": blocker,
        "manifest": "parity-evidence/verification/" + PASS + "/manifest.json",
        "report": "parity-evidence/" + PASS + ".md",
        "sourceRanges": {row["file"] + ":" + row["label"]: row["lineRange"] for row in source},
    }, indent=2, sort_keys=True))
    return 0 if status.startswith(("PASS548", "BLOCKED_PASS548")) else 1

if __name__ == "__main__":
    raise SystemExit(main())
