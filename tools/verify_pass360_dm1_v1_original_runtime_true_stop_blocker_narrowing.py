#!/usr/bin/env python3
"""Pass360 verifier: narrow the DM1 V1 original FIRES true-stop blocker.

Evidence-only pass.  The ReDMCSB source audit remains primary; prior runtime
artifacts are reconciled under the strict pass324/p359 rule so setup/BPLIST text
cannot be promoted as an original runtime stop.
"""
from __future__ import annotations

import json
import re
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
STATUS = "BLOCKED_PASS360_ORIGINAL_RUNTIME_TRUE_STOP_BLOCKER_NARROWED"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {"id":"mainloop_draw_consumes_party_tuple","file":"GAMELOOP.C","lines":"80-90","function":"F0002_MAIN_GameLoop_CPSDF","needles":["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)"],"decision":"F0128 is the source redraw seam that consumes the current party tuple."},
    {"id":"mainloop_input_then_queue_dispatch","file":"GAMELOOP.C","lines":"164-219","function":"F0002_MAIN_GameLoop_CPSDF","needles":["F0361_COMMAND_ProcessKeyPress","F0380_COMMAND_ProcessQueue_CPSC","G0321_B_StopWaitingForPlayerInput"],"decision":"Keyboard input is drained before F0380 queue dispatch during the wait loop."},
    {"id":"queue_dequeue_dispatch","file":"COMMAND.C","lines":"2045-2156","function":"F0380_COMMAND_ProcessQueue_CPSC","needles":["F0365_COMMAND_ProcessTypes1To2_TurnParty","F0366_COMMAND_ProcessTypes3To6_MoveParty","G2153_i_QueuedCommandsCount"],"decision":"F0380 remains the source movement command dequeue/dispatch seam."},
    {"id":"turn_step_mutate_party_tuple","file":"CLIKMENU.C","lines":"142-174,180-347","function":"F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty","needles":["F0284_CHAMPION_SetPartyDirection","F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement","G0310_i_DisabledMovementTicks"],"decision":"Accepted turns/steps mutate direction/coordinates and apply movement cooldown."},
    {"id":"f0128_composes_then_calls_f0097","file":"DUNVIEW.C","lines":"8336-8611","function":"F0128_DUNGEONVIEW_Draw_CPSF","needles":["G0296_puc_Bitmap_Viewport","F0127_DUNGEONVIEW_DrawSquareD0C","F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)"],"decision":"F0128 composes viewport content and calls F0097 for presentation."},
    {"id":"f0097_pc34_vidrv9_present","file":"DRAWVIEW.C","lines":"709-858","function":"F0097_DUNGEONVIEW_DrawViewport","needles":["void F0097_DUNGEONVIEW_DrawViewport","F0638_GetZone(C007_ZONE_VIEWPORT","VIDRV_09_BlitViewPort"],"decision":"F0097 is the PC34 viewport-present seam through video-driver slot 9."},
    {"id":"vidrv_slot9_blit_binding","file":"VIDEODRV.C","lines":"941-957,3566-3582","function":"G8133_ac_Code1DispatchTable / F8161_VIDRV_09_BlitViewPort","needles":["F8161_VIDRV_09_BlitViewPort","G8177_c_ViewportColorIndexOffset","F8151_VIDRV_02_Blit"],"decision":"VIDRV slot 9 is the source viewport blit implementation."},
]

PRIOR = {
    "pass315": {"path":"parity-evidence/verification/pass315_dm1_v1_f0128_runtime_hit_verifier/manifest.json", "expected":"F0128_RUNTIME_HIT_VERIFIED_F0380_REMAINS_BLOCKED", "role":"legacy F0128 claim to reclassify under strict parser"},
    "pass324": {"path":"parity-evidence/verification/pass324_dm1_v1_debugger_code_stop_control_primitive/manifest.json", "expected":"PASS_DEBUGGER_CODE_STOP_CONTROL_PRIMITIVE_FOUND", "role":"owned-PTY strict control primitive"},
    "pass326": {"path":"parity-evidence/verification/pass326_dm1_v1_direct_pty_f0128_code_stop/manifest.json", "expected":"BLOCKED_PASS326_DIRECT_PTY_F0128_CODE_STOP_NOT_PROVEN", "role":"strict F0128 target stop attempt"},
    "pass328": {"path":"parity-evidence/verification/pass328_dm1_v1_direct_pty_postload_route_timing/manifest.json", "expected":"BLOCKED_PASS328_BREAKPOINT_ARMING_TIMING", "role":"post-load arming timing attempt"},
    "pass329": {"path":"parity-evidence/verification/pass329_dm1_v1_direct_pty_breakpoint_arming_timing/manifest.json", "expected":"BLOCKED_PASS329_CODE_STOP_TRANSITION_NOT_EMITTED", "role":"breakpoint retention / arming timing attempt"},
    "pass330": {"path":"parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/manifest.json", "expected":"BLOCKED_PASS330_CPU_NEVER_REACHES_F0128_UNDER_ROUTE", "role":"route-to-target stop transition investigation"},
    "pass357": {"path":"parity-evidence/verification/pass357_dm1_v1_original_runtime_true_stop_control_blocker/manifest.json", "expected":"BLOCKED_PASS357_ORIGINAL_RUNTIME_TRUE_STOP_CONTROL_REQUIRED", "role":"prior consolidated true-stop blocker"},
    "pass359": {"path":"parity-evidence/verification/pass359_dm1_v1_movement_route_runtime_blocker_followup/manifest.json", "expected":"PASS_DM1_V1_MOVEMENT_ROUTE_RUNTIME_BLOCKER_CLASSIFIED", "role":"movement route retired / original true-stop active classification"},
}

TARGETS = {
    "F0128_DUNGEONVIEW_Draw_CPSF": "23AD:40FE",
    "F0097_DUNGEONVIEW_DrawViewport": "2809:1E31",
    "VIDRV_09_BlitViewPort_indirect_call_candidate": "2809:1EFF",
}

NEXT_CONTROLLED_COMMAND = "python3 tools/pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py --seconds 75 && python3 tools/verify_pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py && python3 tools/verify_pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing.py"


def run(cmd: list[str]) -> str:
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
    return p.stdout.strip()


def norm(s: str) -> str:
    return " ".join(s.split())


def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        start, end = [int(x) for x in part.split("-", 1)] if "-" in part else (int(part), int(part))
        chunks.append("\n".join(lines[start - 1:end]))
    return "\n".join(chunks)


def audit_sources() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for spec in SOURCE_LOCKS:
        path = REDMCSB / spec["file"]
        text = source_window(path, spec["lines"]) if path.exists() else ""
        missing = [n for n in spec["needles"] if norm(n) not in norm(text)]
        row = {k: v for k, v in spec.items() if k != "needles"}
        row.update({"path": str(path), "ok": path.exists() and not missing, "missingCount": len(missing)})
        rows.append(row)
    return rows


def load_json(rel: str) -> dict[str, Any]:
    path = ROOT / rel
    if not path.exists():
        return {"_missing": True}
    return json.loads(path.read_text(encoding="utf-8"))


def prior_status(data: dict[str, Any]) -> str | None:
    if data.get("status"):
        return data.get("status")
    # Some old manifests kept the status inside verifier_result.
    vr = data.get("verifier_result", {})
    if vr.get("returncode") == 0 and data.get("expected_status"):
        return data.get("expected_status")
    return None


def direct_hit_flags(data: dict[str, Any]) -> dict[str, Any]:
    blob = data.get("runtimeProbe") or data.get("runtime_probe") or {}
    strategies = blob.get("strategies") if isinstance(blob, dict) else None
    candidates = strategies if isinstance(strategies, list) else [blob]
    out = {
        "ran": any(bool(c.get("ran")) for c in candidates if isinstance(c, dict)) or bool(blob.get("ran")) if isinstance(blob, dict) else False,
        "strictF0128Stop": False,
        "strictF0097OrVidrvAfterF0128": False,
        "breakpointRetainedPostRoute": False,
        "sawRunning": False,
        "postRoutePauseCodeAddr": None,
    }
    for c in candidates:
        if not isinstance(c, dict):
            continue
        hits = c.get("directHits", {})
        out["strictF0128Stop"] = out["strictF0128Stop"] or bool(hits.get("f0128_23AD_40FE"))
        out["strictF0097OrVidrvAfterF0128"] = out["strictF0097OrVidrvAfterF0128"] or bool(hits.get("f0097_2809_1EFF_after_f0128") or hits.get("f0097_after_f0128"))
        out["breakpointRetainedPostRoute"] = out["breakpointRetainedPostRoute"] or bool(c.get("breakpointRetainedPostRoute"))
        out["sawRunning"] = out["sawRunning"] or bool(c.get("sawRunning"))
        if c.get("postRoutePauseCodeAddr"):
            out["postRoutePauseCodeAddr"] = c.get("postRoutePauseCodeAddr")
    if isinstance(blob, dict):
        out["strictF0128Stop"] = out["strictF0128Stop"] or bool(blob.get("f0128_target_stops"))
        out["sawRunning"] = out["sawRunning"] or bool(blob.get("sawRunning"))
    return out


def summarize_latest_pass330_attempt(priors: dict[str, Any]) -> dict[str, Any]:
    data = load_json("parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/manifest.json")
    runtime = data.get("runtimeProbe", {}) if isinstance(data, dict) else {}
    strategies = runtime.get("strategies", []) if isinstance(runtime, dict) else []
    first = strategies[0] if strategies and isinstance(strategies[0], dict) else {}
    return {
        "sourceManifest": "parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/manifest.json",
        "status": data.get("status"),
        "boundedSecondsPerStrategy": runtime.get("boundedSecondsPerStrategy"),
        "ran": runtime.get("ran"),
        "strategy": first.get("strategy"),
        "method": first.get("method"),
        "directHits": first.get("directHits", {}),
        "breakpointRetainedPostRoute": first.get("breakpointRetainedPostRoute"),
        "postRoutePauseCodeAddr": first.get("postRoutePauseCodeAddr"),
        "blocker": data.get("blocker") or first.get("blocker"),
        "strictReconciliationOk": priors.get("pass330", {}).get("ok", False),
        "nextCommand": NEXT_CONTROLLED_COMMAND,
    }


def audit_priors() -> dict[str, Any]:
    rows: dict[str, Any] = {}
    for name, spec in PRIOR.items():
        data = load_json(spec["path"])
        status = prior_status(data)
        row = {"path": spec["path"], "exists": not data.get("_missing"), "status": status, "expected": spec["expected"], "role": spec["role"], "ok": (not data.get("_missing")) and status == spec["expected"]}
        if name in {"pass326", "pass328", "pass329", "pass330"}:
            row["runtimeFlags"] = direct_hit_flags(data)
        if name == "pass315":
            ev = data.get("runtime_evidence", {})
            row["strictReclassification"] = {
                "legacyClaimPresent": status == spec["expected"],
                "legacyHitSignature": ev.get("hit_signature_required"),
                "sourcePassStatus": load_json("parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json").get("status"),
                "acceptedAsStrictTrueStopNow": False,
                "reason": "legacy evidence records a BP/list signature from a source pass that remained blocked; pass324/pass359 require a post-running code-view transition instead",
            }
        rows[name] = row
    return rows


def build_manifest() -> dict[str, Any]:
    sources = audit_sources()
    priors = audit_priors()
    strict_attempts = [priors[p]["runtimeFlags"] for p in ["pass326", "pass328", "pass329", "pass330"] if "runtimeFlags" in priors[p]]
    any_f0128 = any(x["strictF0128Stop"] for x in strict_attempts)
    any_present_after = any(x["strictF0097OrVidrvAfterF0128"] for x in strict_attempts)
    ok = all(x["ok"] for x in sources) and all(x["ok"] for x in priors.values()) and not any_f0128 and not any_present_after
    latest_pass330 = summarize_latest_pass330_attempt(priors)
    return {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": STATUS if ok else "FAIL_PASS360_TRUE_STOP_BLOCKER_NARROWING",
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"]),
        "head": run(["git", "rev-parse", "HEAD"]),
        "sourceRoot": str(REDMCSB),
        "sourceAudit": sources,
        "priorArtifactAudit": priors,
        "candidateRuntimeTargets": TARGETS,
        "latestControlledRuntimeAttempt": latest_pass330,
        "blockerNarrowing": {
            "notBlockedBy": [
                "Firestaff M11 movement route (pass349/pass351/pass352/pass358/pass359)",
                "owned-PTY debugger control primitive (pass324)",
                "BPLIST/setup parser ambiguity (strictly rejected here)",
                "breakpoint retention alone (pass329/pass330 retain F0128 but still do not stop at it)",
            ],
            "stillBlockedBy": "No bounded controlled original FIRES run produces a strict post-running code stop at F0128, followed by F0097 or VIDRV slot 9, at the current candidate CS:IP targets.",
            "nextUnblocker": "Re-establish the live FIRES CS:IP map or an equivalent source-bound runtime locator for F0128/F0097/VIDRV, then rerun a single owned-PTY sequence that records F0128 -> F0097/VIDRV after controlled movement input.",
            "nextCommand": NEXT_CONTROLLED_COMMAND,
            "promoteViewportComparator": False,
        },
        "notClaimed": [
            "new original FIRES F0128/F0097/VIDRV true stop",
            "original-vs-Firestaff pixel parity",
            "runtime proof from static offsets alone",
            "runtime proof from BPLIST/setup text",
        ],
    }


def write_report(m: dict[str, Any]) -> None:
    lines = [
        "# Pass360 — DM1 V1 original runtime true-stop blocker narrowing",
        "",
        "Status: `" + m["status"] + "`",
        "",
        "## Verdict",
        "",
        "The remaining blocker is now narrower: movement routing and debugger control are not the active problem. The unresolved item is a source-bound original FIRES true-stop sequence at the candidate F0128 target, followed by F0097 or VIDRV slot 9 in the same bounded run.",
        "",
        "## ReDMCSB source audit",
        "",
    ]
    for row in m["sourceAudit"]:
        lines.append("- `{}:{}` — `{}` — {} ok=`{}`".format(row["file"], row["lines"], row["function"], row["decision"], row["ok"]))
    lines += ["", "## Prior runtime reconciliation", ""]
    for name, row in m["priorArtifactAudit"].items():
        lines.append("- `{}`: `{}` (expected `{}`) — {} ok=`{}`".format(name, row["status"], row["expected"], row["role"], row["ok"]))
    latest = m["latestControlledRuntimeAttempt"]
    lines += ["", "## Latest controlled runtime attempt", ""]
    lines.append("- Pass330 status: `{}`; ran=`{}`; bounded seconds=`{}`".format(latest["status"], latest["ran"], latest["boundedSecondsPerStrategy"]))
    lines.append("- Direct hits: `{}`; retained post-route=`{}`; post-route pause code=`{}`".format(latest["directHits"], latest["breakpointRetainedPostRoute"], latest["postRoutePauseCodeAddr"]))
    lines.append("- Blocker: `{}`".format(latest["blocker"]))
    lines.append("- Exact next command: `{}`".format(latest["nextCommand"]))
    b = m["blockerNarrowing"]
    lines += ["", "## Blocker narrowing", "", "Not blocked by:"]
    lines += ["- " + x for x in b["notBlockedBy"]]
    lines += ["", "Still blocked by: `" + b["stillBlockedBy"] + "`", "", "Next unblocker: `" + b["nextUnblocker"] + "`", "", "Next command: `" + b["nextCommand"] + "`", "", "## Non-claims", ""]
    lines += ["- " + x for x in m["notClaimed"]]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    m = build_manifest()
    MANIFEST.write_text(json.dumps(m, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(m)
    print(json.dumps({"status": m["status"], "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT))}, indent=2))
    return 0 if m["status"] == STATUS else 1


if __name__ == "__main__":
    raise SystemExit(main())
