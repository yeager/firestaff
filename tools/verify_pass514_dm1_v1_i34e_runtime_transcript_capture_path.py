#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, shutil, subprocess, time
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass514_dm1_v1_i34e_runtime_transcript_capture_path"
OUT = ROOT / "parity-evidence" / "verification" / PASS
REPORT = ROOT / "parity-evidence" / (PASS + ".md")
RED = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
ORIG = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"

ADDR = {
    "F0361_COMMAND_ProcessKeyPress": "22F7:0407",
    "F0380_COMMAND_ProcessQueue_CPSC": "22F7:0699",
    "F0365_COMMAND_ProcessTypes1To2_TurnParty": "1EA7:010D",
    "F0366_COMMAND_ProcessTypes3To6_MoveParty": "1EA7:01AA",
    "F0128_DUNGEONVIEW_Draw_CPSF": "23B0:40FE",
    "F0097_DUNGEONVIEW_DrawViewport_entry": "280C:1E31",
    "F0097_VIDRV_09_BlitViewPort_indirect_call": "280C:1EFF",
    "G2153_i_QueuedCommandsCount": "2C23:3E78",
    "G0432_as_CommandQueue": "2C23:3E7A",
    "G0433_i_CommandQueueFirstIndex": "2C23:3EC8",
    "G0434_i_CommandQueueLastIndex": "2C23:1F08",
    "G0309_i_PartyMapIndex": "2C23:3C8A",
    "G0308_i_PartyDirection": "2C23:3C92",
    "G0306_i_PartyMapX": "2C23:3C94",
    "G0307_i_PartyMapY": "2C23:3CE0",
}

CAPTURE_COMMAND = "python3 tools/verify_pass514_dm1_v1_i34e_runtime_transcript_capture_path.py --run-capture --seconds 45"
RUNTIME_HELPERS = [
    "tools/verify_pass385_dm1_v1_corrected_loader_delta_semantic_route.py",
    "tools/verify_pass386_dm1_v1_keyboard_vs_click_command_dispatch.py",
    "tools/verify_pass388_dm1_v1_queue_producer_runtime.py",
]


def run(cmd: list[str], **kw: Any) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, **kw)


def find_line(path: Path, needle: str) -> int | None:
    if not path.exists():
        return None
    compact = " ".join(needle.split())
    for i, line in enumerate(path.read_text(encoding="latin-1", errors="replace").splitlines(), 1):
        if compact in " ".join(line.split()):
            return i
    return None


def source_audit() -> list[dict[str, Any]]:
    specs = [
        ("COMMAND.C", "f0361_keyboard_queue_write", [
            "void F0361_COMMAND_ProcessKeyPress",
            "if ((L1112_ps_KeyboardInput = G0443_ps_PrimaryKeyboardInput) == NULL)",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G2153_i_QueuedCommandsCount++;",
        ]),
        ("COMMAND.C", "f0380_pop_count_dispatch", [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "if (G2153_i_QueuedCommandsCount == 0)",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G2153_i_QueuedCommandsCount--;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ]),
        ("GAMELOOP.C", "keyboard_then_f0380_loop_order", [
            "while (M527_IsCharacterInKeyboardBuffer())",
            "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
            "F0380_COMMAND_ProcessQueue_CPSC();",
        ]),
        ("CLIKMENU.C", "turn_step_party_tuple_handlers", [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0267_MOVE_GetMoveResult_CPSCE",
        ]),
        ("MOVESENS.C", "successful_step_party_tuple_commit", [
            "BOOLEAN F0267_MOVE_GetMoveResult_CPSCE",
            "G0306_i_PartyMapX = P0560_i_DestinationMapX;",
            "G0307_i_PartyMapY = P0561_i_DestinationMapY;",
        ]),
        ("DUNVIEW.C", "f0128_to_f0097_present_boundary", [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ]),
        ("DRAWVIEW.C", "pc34_f0097_viewport_present", [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "VIDRV_09_BlitViewPort",
        ]),
    ]
    rows = []
    for file_name, section, needles in specs:
        path = RED / file_name
        hits = {needle: find_line(path, needle) for needle in needles}
        present = [line for line in hits.values() if line is not None]
        rows.append({
            "file": file_name,
            "section": section,
            "path": str(path),
            "ok": path.exists() and len(present) == len(hits),
            "lineHits": hits,
            "lineRange": [min(present), max(present)] if present else None,
            "missing": [needle for needle, line in hits.items() if line is None],
        })
    return rows


def prerequisites() -> dict[str, Any]:
    tools = {name: shutil.which(name) for name in ["dosbox-debug", "Xvfb", "xdotool"]}
    helper_rows = {rel: (ROOT / rel).exists() for rel in RUNTIME_HELPERS}
    return {
        "tools": tools,
        "missingTools": [name for name, path in tools.items() if not path],
        "runtimeHelpers": helper_rows,
        "missingHelpers": [rel for rel, exists in helper_rows.items() if not exists],
        "originalStage": str(ORIG),
        "originalStageExists": ORIG.exists(),
        "dmExeExists": (ORIG / "DM.EXE").exists(),
    }


def prior_runtime_evidence() -> dict[str, Any]:
    rows: dict[str, Any] = {}
    for name, rel in {
        "pass387_f0361_queue_write": "parity-evidence/verification/pass387_keyboard_f0361_queue_write/manifest.json",
        "pass388_queue_producer_runtime": "parity-evidence/verification/pass388_dm1_v1_queue_producer_runtime/manifest.json",
    }.items():
        path = ROOT / rel
        rows[name] = {"path": rel, "status": "missing"}
        if path.exists():
            payload = json.loads(path.read_text(encoding="utf-8"))
            rows[name] = {"path": rel, "status": payload.get("status"), "proofPredicates": payload.get("proofPredicates")}
    return rows


def run_capture(seconds: int) -> dict[str, Any]:
    # Keep the first landable pass narrow: launch the already-proven producer
    # debugger path and save the exact outcome as a blocker or proof artifact.
    cmd = ["python3", "tools/verify_pass388_dm1_v1_queue_producer_runtime.py", "--only", "keyboard", "--seconds", str(seconds)]
    started = time.time()
    proc = run(cmd, cwd=ROOT, timeout=seconds + 80)
    manifest_path = ROOT / "parity-evidence/verification/pass388_dm1_v1_queue_producer_runtime/manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8")) if manifest_path.exists() else None
    return {
        "ran": True,
        "command": " ".join(cmd),
        "returncode": proc.returncode,
        "durationSeconds": round(time.time() - started, 3),
        "stdoutTail": proc.stdout[-4000:],
        "pass388Manifest": str(manifest_path.relative_to(ROOT)),
        "pass388Status": manifest.get("status") if isinstance(manifest, dict) else None,
        "pass388Predicates": manifest.get("proofPredicates") if isinstance(manifest, dict) else None,
    }


def summarize(source: list[dict[str, Any]], prereq: dict[str, Any], capture: dict[str, Any] | None) -> tuple[str, str | None, dict[str, Any]]:
    prior = prior_runtime_evidence()
    p387_status = prior.get("pass387_f0361_queue_write", {}).get("status")
    p388_status = (capture or {}).get("pass388Status") or prior.get("pass388_queue_producer_runtime", {}).get("status")
    p388_pred = (capture or {}).get("pass388Predicates") or prior.get("pass388_queue_producer_runtime", {}).get("proofPredicates") or {}
    predicates = {
        "sourceAuditOk": all(row["ok"] for row in source),
        "prerequisitesOk": not prereq["missingTools"] and not prereq["missingHelpers"] and prereq["dmExeExists"],
        "f0361QueueCountPathAlreadyProven": p387_status == "PASS387_KEYBOARD_F0361_QUEUE_WRITE_PROVEN",
        "f0380ReachedInN2DebuggerPath": bool(p388_pred.get("f0380Reached")),
        "f0380QueuePositiveBeforePop": bool(p388_pred.get("f0380QueueCountPositiveImmediatelyBefore")),
        "dispatchReached": bool(p388_pred.get("dispatchReached")),
        "captureAttemptedThisRun": bool(capture),
    }
    if not predicates["sourceAuditOk"]:
        return "FAIL_PASS514_SOURCE_AUDIT_FAILED", "source audit failed", predicates
    if not predicates["prerequisitesOk"]:
        missing = prereq["missingTools"] + prereq["missingHelpers"]
        return "BLOCKED_PASS514_MISSING_N2_DEBUGGER_PREREQUISITE", "missing prerequisite: " + ", ".join(missing), predicates
    if predicates["f0361QueueCountPathAlreadyProven"] and predicates["f0380ReachedInN2DebuggerPath"] and not predicates["f0380QueuePositiveBeforePop"]:
        blocker = "smallest N2 debugger path is wired, but unified capture is blocked because the pass388 keyboard route reaches F0380 with G2153 sampled as zero; exact command: " + CAPTURE_COMMAND
        return "BLOCKED_PASS514_F0380_REACHED_WITH_EMPTY_QUEUE", blocker, predicates
    blocker = "unified original PC/I34E transcript incomplete; exact command: " + CAPTURE_COMMAND
    return "BLOCKED_PASS514_UNIFIED_RUNTIME_TRANSCRIPT_INCOMPLETE", blocker, predicates


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--run-capture", action="store_true")
    ap.add_argument("--seconds", type=int, default=45)
    args = ap.parse_args()
    args.seconds = max(10, min(args.seconds, 90))
    OUT.mkdir(parents=True, exist_ok=True)
    source = source_audit()
    prereq = prerequisites()
    capture = run_capture(args.seconds) if args.run_capture else None
    status, blocker, predicates = summarize(source, prereq, capture)
    manifest = {
        "schema": PASS + ".v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"], cwd=ROOT).stdout.strip(),
        "head": run(["git", "rev-parse", "HEAD"], cwd=ROOT).stdout.strip(),
        "sourceRoot": str(RED),
        "addresses": ADDR,
        "sourceAudit": source,
        "prerequisites": prereq,
        "priorRuntimeEvidence": prior_runtime_evidence(),
        "captureCommand": CAPTURE_COMMAND,
        "runtimeCaptureAttempt": capture,
        "proofPredicates": predicates,
        "blocker": blocker,
        "requiredUnifiedFields": [
            "F0361 queue slot/count",
            "F0380 pop/count",
            "party before/after map/x/y/dir",
            "F0128 direction/x/y tuple",
            "F0097 present boundary",
        ],
        "nonClaims": ["no movement behavior changed", "no pass513 scaffold promoted", "no push"],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text("\n".join([
        "# Pass514 - DM1 V1 PC/I34E runtime transcript capture path",
        "",
        "Status: " + status,
        "",
        "## Source anchors",
        "",
        *["- " + row["file"] + " " + str(row["lineRange"]) + ": " + row["section"] for row in source],
        "",
        "## Capture path",
        "",
        "- N2-local original stage: ~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34",
        "- Debugger stack: dosbox-debug + Xvfb + xdotool",
        "- Exact command: " + CAPTURE_COMMAND,
        "",
        "## Decision",
        "",
        blocker or "Unified runtime capture path observed the required boundary categories.",
        "",
        "## Evidence",
        "",
        "- Manifest: parity-evidence/verification/" + PASS + "/manifest.json",
    ]) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "blocker": blocker, "predicates": predicates, "manifest": "parity-evidence/verification/" + PASS + "/manifest.json"}, indent=2, sort_keys=True))
    return 0 if status.startswith("BLOCKED_PASS514") or status.startswith("PASS514") else 1


if __name__ == "__main__":
    raise SystemExit(main())
