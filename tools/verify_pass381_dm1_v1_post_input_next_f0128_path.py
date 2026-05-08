#!/usr/bin/env python3
"""Pass381 verifier: source-lock the post-input wait-loop path to the next F0128 draw.

This is an evidence-only narrowing pass. It does not promote an original runtime
viewport hit. It explains why pass379 retained the F0128/F0097 breakpoints yet
never stopped there: pass379 proves debugger control plus delivered host key
input, but it does not prove a semantic command dequeue, G0321 wait-loop exit,
or the outer-loop wrap that reaches the next F0128 draw.
"""
from __future__ import annotations

import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass381_dm1_v1_post_input_next_f0128_path"
OUTDIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUTDIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
SRC = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

PRIORS = {
    "pass330": ROOT / "parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/manifest.json",
    "pass360": ROOT / "parity-evidence/verification/pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing/manifest.json",
    "pass377": ROOT / "parity-evidence/verification/pass377_dm1_v1_postload_f0128_f0097_true_stop_route/manifest.json",
    "pass379": ROOT / "parity-evidence/verification/pass379_dm1_v1_true_stop_codepath_probe/manifest.json",
    "pass380": ROOT / "parity-evidence/verification/pass380_dm1_v1_f0128_f0097_map_transition_narrowing/manifest.json",
}

EXPECTED_PRIOR_STATUSES = {
    "pass330": {"BLOCKED_PASS330_CPU_NEVER_REACHES_F0128_UNDER_ROUTE"},
    "pass360": {"BLOCKED_PASS360_ORIGINAL_RUNTIME_TRUE_STOP_BLOCKER_NARROWED"},
    "pass377": {"BLOCKED_PASS377_POSTLOAD_F0128_TRUE_STOP_NOT_RECAPTURED", "PASS377_POSTLOAD_F0128_TRUE_STOP_ONLY_F0097_BLOCKED", "PASS377_POSTLOAD_F0128_F0097_TRUE_STOP_SEQUENCE_PROVEN"},
    "pass379": {"BLOCKED_PASS379_POST_ROUTE_PAUSE_MOVED_MAP_OR_PATH_UNRESOLVED"},
    "pass380": {"BLOCKED_PASS380_SOURCE_PATH_TRANSITION_NARROWED_MAP_BINDING_HELD"},
}

SOURCE_LOCKS: list[dict[str, Any]] = [
    {"id": "outer_loop_draw_before_input_wait", "file": "GAMELOOP.C", "lines": "80-92", "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);"], "decision": "F0128 is reached in the outer game-loop draw section before the later input wait/dequeue loop."},
    {"id": "wait_loop_resets_and_dequeues_until_stop_and_tick", "file": "GAMELOOP.C", "lines": "160-220", "needles": ["G0321_B_StopWaitingForPlayerInput = C0_FALSE;", "while (M527_IsCharacterInKeyboardBuffer())", "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());", "F0380_COMMAND_ProcessQueue_CPSC();", "if (!G0321_B_StopWaitingForPlayerInput)", "} while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"], "decision": "Host keys must become queued game commands; then F0380 must set G0321 true and game-time ticking must be true before the loop can exit."},
    {"id": "movement_dequeue_dispatches_turn_or_step", "file": "COMMAND.C", "lines": "2045-2156", "needles": ["L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;", "F0360_COMMAND_ProcessPendingClick();", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"], "decision": "The path from route key input to movement draw goes through F0380 queue dequeue and movement dispatch, not directly to F0128."},
    {"id": "accepted_turn_sets_stop_flag", "file": "CLIKMENU.C", "lines": "142-174", "needles": ["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0284_CHAMPION_SetPartyDirection"], "decision": "A semantic turn command sets G0321 true before changing party direction."},
    {"id": "accepted_step_sets_stop_flag", "file": "CLIKMENU.C", "lines": "180-270", "needles": ["void F0366_COMMAND_ProcessTypes3To6_MoveParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;", "F0362_COMMAND_HighlightBoxEnable"], "decision": "A semantic move command also sets G0321 true before movement processing continues."},
    {"id": "f0128_calls_f0097_viewport_present", "file": "DUNVIEW.C", "lines": "8318-8612", "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"], "decision": "Once the outer-loop draw path reaches F0128, F0097 is the source viewport-present call site."},
]


def run(cmd: list[str]) -> str:
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
    return p.stdout.strip()


def norm(text: str) -> str:
    return " ".join(text.split())


def find_line(path: Path, needle: str) -> int | None:
    cn = norm(needle)
    for i, line in enumerate(path.read_text(encoding="latin-1", errors="replace").splitlines(), 1):
        if cn in norm(line):
            return i
    return None


def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        lo, hi = [int(x) for x in part.split("-", 1)] if "-" in part else (int(part), int(part))
        chunks.append("\n".join(lines[lo - 1:hi]))
    return "\n".join(chunks)


def audit_sources() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for lock in SOURCE_LOCKS:
        path = SRC / lock["file"]
        text = source_window(path, lock["lines"]) if path.exists() else ""
        missing = [n for n in lock["needles"] if norm(n) not in norm(text)]
        lines = {n: find_line(path, n) for n in lock["needles"]} if path.exists() else {}
        row = {k: v for k, v in lock.items() if k != "needles"}
        row.update({"path": str(path), "ok": path.exists() and not missing, "lineHits": lines, "missing": missing})
        rows.append(row)
    return rows


def load_json(path: Path) -> dict[str, Any]:
    if not path.exists():
        return {"_missing": True}
    return json.loads(path.read_text(encoding="utf-8"))


def prior_audit() -> dict[str, Any]:
    out: dict[str, Any] = {}
    for name, path in PRIORS.items():
        data = load_json(path)
        status = data.get("status")
        out[name] = {"path": str(path.relative_to(ROOT)), "exists": not data.get("_missing"), "status": status, "expectedAny": sorted(EXPECTED_PRIOR_STATUSES[name]), "ok": (not data.get("_missing")) and status in EXPECTED_PRIOR_STATUSES[name]}
    return out


def pass379_reason(pass379: dict[str, Any], pass380: dict[str, Any]) -> dict[str, Any]:
    rt = pass379.get("runtimeProbe", {})
    direct = rt.get("directHits", {})
    keylog_path = ROOT / rt.get("routeKeylog", "")
    keylog = json.loads(keylog_path.read_text()) if keylog_path.exists() else []
    movement_keys = [r.get("route_item") for r in keylog if str(r.get("route_item", "")).startswith("kp")]
    final_decode = pass380.get("finalPauseDecode", {})
    return {
        "routeInputAfterArming": rt.get("routeInputAfterArming"),
        "breakpointRetainedPostRoute": rt.get("breakpointRetainedPostRoute"),
        "sawRunning": rt.get("sawRunning"),
        "movementKeyEventsAfterArming": movement_keys,
        "directHits": direct,
        "anyF0128OrF0097Hit": bool(direct.get("f0128_23AD_40FE") or direct.get("f0097_2809_1EFF") or direct.get("f0097_entry_2809_1E31")),
        "finalPauseCodeAddr": rt.get("finalPauseCodeAddr"),
        "finalPauseDecode": final_decode,
        "explainsNoHit": rt.get("routeInputAfterArming") is True and rt.get("breakpointRetainedPostRoute") is True and not bool(direct.get("f0128_23AD_40FE") or direct.get("f0097_2809_1EFF") or direct.get("f0097_entry_2809_1E31")) and final_decode.get("segment") == "IMAGE_TEXT" and final_decode.get("isF0128OrF0097") is False,
    }


def build_control_flow() -> dict[str, Any]:
    game = SRC / "GAMELOOP.C"
    cli = SRC / "CLIKMENU.C"
    dun = SRC / "DUNVIEW.C"
    return {
        "sourceOrder": [
            {"step": 1, "file": "GAMELOOP.C", "line": find_line(game, "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);"), "meaning": "outer-loop viewport draw"},
            {"step": 2, "file": "GAMELOOP.C", "line": find_line(game, "G0321_B_StopWaitingForPlayerInput = C0_FALSE;"), "meaning": "input wait flag reset later in the same outer iteration"},
            {"step": 3, "file": "GAMELOOP.C", "line": find_line(game, "F0380_COMMAND_ProcessQueue_CPSC();"), "meaning": "queue processing inside wait loop"},
            {"step": 4, "file": "CLIKMENU.C", "line": find_line(cli, "G0321_B_StopWaitingForPlayerInput = C1_TRUE;"), "meaning": "accepted turn/move handlers set stop flag"},
            {"step": 5, "file": "GAMELOOP.C", "line": find_line(game, "} while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"), "meaning": "wait loop exits only after stop flag and game-time tick"},
            {"step": 6, "file": "GAMELOOP.C", "line": find_line(game, "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);"), "meaning": "next F0128 occurs only when the outer loop wraps to its next draw section"},
            {"step": 7, "file": "DUNVIEW.C", "line": find_line(dun, "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"), "meaning": "F0128 eventually presents through F0097"},
        ],
        "criticalPredicate": "route key delivery is insufficient; evidence must prove F0380 dequeued a movement/turn command, F0365/F0366 set G0321 true, G0301_B_GameTimeTicking allowed the wait loop to exit, and the outer loop wrapped to F0128.",
    }


def main() -> int:
    OUTDIR.mkdir(parents=True, exist_ok=True)
    errors: list[str] = []
    source_rows = audit_sources()
    if not all(r["ok"] for r in source_rows):
        errors.append("source audit failed")
    priors = prior_audit()
    if not all(r["ok"] for r in priors.values()):
        errors.append("prior status drift")
    reason = pass379_reason(load_json(PRIORS["pass379"]), load_json(PRIORS["pass380"]))
    if not reason["explainsNoHit"]:
        errors.append("pass379/pass380 predicates no longer explain no-hit state")
    control = build_control_flow()
    source_ok = all(r["ok"] for r in source_rows)
    transition_explained = source_ok and reason["explainsNoHit"] and all(r["ok"] for r in priors.values())
    status = "BLOCKED_PASS381_NEXT_F0128_REQUIRES_WAIT_LOOP_EXIT_NOT_PROVED_BY_PASS379" if transition_explained else "FAIL_PASS381_NEXT_F0128_PATH_UNRESOLVED"
    if errors:
        status = "FAIL_PASS381_NEXT_F0128_PATH_UNRESOLVED"
    conclusion = ("ReDMCSB source puts the next viewport draw on the outer-loop wrap: route keys must first become queued commands, F0380 must dispatch F0365/F0366, those handlers set G0321_B_StopWaitingForPlayerInput, and the wait loop exits only when G0321 and G0301_B_GameTimeTicking are both true. pass379 retained F0128/F0097 breakpoints and delivered host route keys, but it did not prove F0380 dequeue/G0321 wait-loop exit; after 75s it still had no F0128/F0097 hit and a forced pause decoded to IMAGE_TEXT near F0683. So pass379 should be read as a semantic route/wait-loop-exit blocker, not a reason to retarget F0128/F0097 addresses." if transition_explained else "The source/prior predicates did not all hold; inspect errors before using this narrowing.")
    manifest = {"schema": PASS + ".v1", "timestampUtc": datetime.now(timezone.utc).isoformat(), "status": status, "repo": str(ROOT), "branch": run(["git", "branch", "--show-current"]), "head": run(["git", "rev-parse", "HEAD"]), "sourceRoot": str(SRC), "sourceAudit": source_rows, "controlFlow": control, "priorAudit": priors, "pass379NoHitExplanation": reason, "decision": {"nextF0128PathLocated": source_ok, "pass379NoHitExplained": transition_explained, "conclusion": conclusion}, "notPromotedBy": ["host route keylog alone", "BPLIST retention alone", "BP command echo", "source-only address binding", "forced-pause IMAGE_TEXT sample"], "errors": errors}
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text("\n".join(["# Pass381 — DM1 V1 post-input next F0128 path", "", f"Status: `{status}`", "", "## Decision", "", conclusion, "", "## Source path", "", "1. `GAMELOOP.C` draws the viewport with `F0128_DUNGEONVIEW_Draw_CPSF(...)` before the input wait loop.", "2. The same outer iteration later resets `G0321_B_StopWaitingForPlayerInput = C0_FALSE` and loops through keyboard drain + `F0380_COMMAND_ProcessQueue_CPSC()`.", "3. `COMMAND.C` dispatches queued turn/step commands to `F0365`/`F0366`; `CLIKMENU.C` sets `G0321_B_StopWaitingForPlayerInput = C1_TRUE` in those accepted movement handlers.", "4. `GAMELOOP.C` exits the wait loop only when `G0321_B_StopWaitingForPlayerInput` and `G0301_B_GameTimeTicking` are both true; the next F0128 is on the following outer-loop draw, where F0128 calls F0097.", "", "## Why pass379 did not stop at F0128/F0097", "", "pass379 proves debugger control, retained breakpoints, and delivered host route-key events. It does **not** prove that those keys became a semantic queued movement command, that F0380 dispatched it, or that G0321/tick let the input wait loop exit. Its final forced pause decoded to IMAGE_TEXT near F0683, while F0128/F0097 direct-hit flags stayed false.", "", f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`"]) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "manifest": str(MANIFEST.relative_to(ROOT)), "errors": errors}, indent=2, sort_keys=True))
    return 0 if status.startswith("BLOCKED_PASS381_") else 1


if __name__ == "__main__":
    raise SystemExit(main())
