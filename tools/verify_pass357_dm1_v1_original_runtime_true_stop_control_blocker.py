#!/usr/bin/env python3
"""Pass357 verifier: preserve the DM1 V1 original-runtime true-stop blocker.

This pass is intentionally a blocker artifact, not a runtime probe. It audits
ReDMCSB source order, checks prior debugger-control artifacts, and records why
Firestaff-side viewport movement evidence must not promote the original-vs-
Firestaff viewport comparator until a strict original FIRES true-stop sequence
exists.
"""
from __future__ import annotations

import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path

PASS = "pass357_dm1_v1_original_runtime_true_stop_control_blocker"
ROOT = Path(__file__).resolve().parents[1]
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
EXPECTED_STATUS = "BLOCKED_PASS357_ORIGINAL_RUNTIME_TRUE_STOP_CONTROL_REQUIRED"

SOURCE_LOCKS = [
    {"id":"mainloop_draw_consumes_party_tuple","file":"GAMELOOP.C","function":"F0002_MAIN_GameLoop_CPSDF","lines":"80-90","needles":["if (!G0300_B_PartyIsResting)","F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)"],"decision":"F0128 is the original source seam that consumes direction/mapX/mapY for a viewport redraw."},
    {"id":"mainloop_input_then_f0380","file":"GAMELOOP.C","function":"F0002_MAIN_GameLoop_CPSDF","lines":"164-219","needles":["F0361_COMMAND_ProcessKeyPress","F0380_COMMAND_ProcessQueue_CPSC","G0321_B_StopWaitingForPlayerInput"],"decision":"Input is drained before F0380 command queue processing in the wait loop."},
    {"id":"f0380_dequeue_dispatch","file":"COMMAND.C","function":"F0380_COMMAND_ProcessQueue_CPSC","lines":"2045-2156","needles":["L1160_i_Command = G0432_as_CommandQueue","L1161_i_CommandX","L1162_i_CommandY","F0365_COMMAND_ProcessTypes1To2_TurnParty","F0366_COMMAND_ProcessTypes3To6_MoveParty"],"decision":"F0380 is the canonical original dequeue/dispatch seam for turn/move commands."},
    {"id":"f0128_compose_then_f0097","file":"DUNVIEW.C","function":"F0128_DUNGEONVIEW_Draw_CPSF","lines":"8336-8611","needles":["G0296_puc_Bitmap_Viewport","F0127_DUNGEONVIEW_DrawSquareD0C","F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)"],"decision":"F0128 composes viewport/walls into G0296 and then calls F0097 for presentation."},
    {"id":"f0097_viewport_present_pc34","file":"DRAWVIEW.C","function":"F0097_DUNGEONVIEW_DrawViewport","lines":"709-858","needles":["void F0097_DUNGEONVIEW_DrawViewport","F0638_GetZone(C007_ZONE_VIEWPORT","(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box)"],"decision":"For the PC34/I34E path, F0097 resolves C007 and invokes video-driver slot 9 with G0296."},
    {"id":"vidrv_slot9_binding","file":"VIDEODRV.C","function":"G8133_ac_Code1DispatchTable / F8161_VIDRV_09_BlitViewPort","lines":"941-957,3566-3582","needles":["F8161_VIDRV_09_BlitViewPort","G8177_c_ViewportColorIndexOffset = 0x10","F8151_VIDRV_02_Blit","G8177_c_ViewportColorIndexOffset = 0"],"decision":"Slot 9 is the PC VGA viewport blit implementation; a live VIDRV stop after F0128 would be the presentation proof."},
]

PRIOR_MANIFESTS = {
    "pass318": "parity-evidence/verification/pass318_dm1_v1_f0097_after_f0128_offset_window_probe/manifest.json",
    "pass320": "parity-evidence/verification/pass320_dm1_v1_f0097_vidrv_window_after_f0128_sequence_probe/manifest.json",
    "pass321": "parity-evidence/verification/pass321_dm1_v1_debugger_code_stop_control_sequence_probe/manifest.json",
    "pass328": "parity-evidence/verification/pass328_dm1_v1_direct_pty_postload_route_timing/manifest.json",
    "pass350": "parity-evidence/verification/pass350_dm1_v1_touch_live_dispatch_gate/manifest.json",
}
EXPECTED_PRIOR = {
    "pass318": "BLOCKED_F0097_AFTER_F0128_WINDOW_NO_HIT",
    "pass320": "BLOCKED_F0128_GATE_NOT_RECAPTURED_STRICT_STOP_FILTER",
    "pass321": "BLOCKED_PASS321_MISSING_DEBUGGER_CODE_STOP_CONTROL_PRIMITIVE",
    "pass328": "BLOCKED_PASS328_BREAKPOINT_ARMING_TIMING",
    "pass350": "PASS_DM1_V1_TOUCH_LIVE_DISPATCH_GATE",
}


def run(cmd: list[str]) -> dict:
    p = subprocess.run(cmd, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    return {"cmd": cmd, "returncode": p.returncode, "outputTail": p.stdout[-2000:]}


def read_json(rel: str) -> dict:
    path = ROOT / rel
    if not path.exists():
        return {"exists": False, "path": rel}
    return {"exists": True, "path": rel, "json": json.loads(path.read_text(encoding="utf-8"))}


def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        start, end = [int(x) for x in part.split("-")]
        chunks.append("\n".join(lines[start - 1:end]))
    return "\n".join(chunks)


def audit_sources() -> list[dict]:
    audited = []
    for lock in SOURCE_LOCKS:
        path = REDMCSB / lock["file"]
        text = source_window(path, lock["lines"]) if path.exists() else ""
        audited.append({**lock, "path": str(path), "ok": path.exists() and all(n in text for n in lock["needles"])})
    return audited


def audit_priors() -> dict:
    rows = {}
    for name, rel in PRIOR_MANIFESTS.items():
        row = read_json(rel)
        status = row.get("json", {}).get("status") if row.get("exists") else None
        row["status"] = status
        row["expectedStatus"] = EXPECTED_PRIOR[name]
        row["ok"] = row.get("exists") and status == EXPECTED_PRIOR[name]
        rows[name] = row
    return rows


def build_manifest() -> dict:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    source = audit_sources()
    prior = audit_priors()
    source_ok = all(r["ok"] for r in source)
    prior_ok = all(r["ok"] for r in prior.values())
    strict_sequence_available = False
    status = EXPECTED_STATUS if source_ok and prior_ok and not strict_sequence_available else "FAIL_PASS357_TRUE_STOP_BLOCKER_GUARD"
    return {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "repo": str(ROOT),
        "head": run(["git", "rev-parse", "HEAD"])["outputTail"].strip(),
        "sourceRoot": str(REDMCSB),
        "sourceAudit": source,
        "priorArtifactAudit": {k: {kk: vv for kk, vv in v.items() if kk != "json"} for k, v in prior.items()},
        "trueStopPredicate": {
            "requiredOrder": [
                "original FIRES true code stop at F0128_DUNGEONVIEW_Draw_CPSF after controlled movement/turn input",
                "same bounded run continues and reaches F0097_DUNGEONVIEW_DrawViewport or VIDRV_09_BlitViewPort after that F0128 stop",
                "transcript/parser marks stops from runtime code lines only, never BP command echoes, BPLIST output, setup collisions, or stale tmux pane text",
            ],
            "currentlyAvailable": strict_sequence_available,
            "knownCandidateAddresses": {
                "F0380_COMMAND_ProcessQueue_CPSC": "22F4:0699",
                "F0128_DUNGEONVIEW_Draw_CPSF": "23AD:40FE",
                "F0097_DUNGEONVIEW_DrawViewport": "2809:1E31",
                "VIDRV_09_BlitViewPort_indirect_call_candidate": "2809:1EFF",
            },
        },
        "originalVsFirestaffComparatorDecision": {
            "firestaffSideLiveDispatch": prior["pass350"]["status"],
            "originalRuntimeViewportPresent": "missing strict F0128 -> F0097/VIDRV true-stop sequence",
            "promoteViewportComparator": False,
            "reason": "Firestaff-side live dispatch is proven, but the comparator needs a source-bound original runtime presentation stop before wall/viewport captures can be promoted as original-vs-Firestaff parity evidence.",
        },
        "nextUnblocker": "Implement a reliable debugger control path that can arm/retain breakpoints after post-load readiness, stop at 23AD:40FE from runtime execution, continue, and then stop at 2809:1E31 or 2809:1EFF in the same controlled original FIRES route.",
        "notClaimed": ["new DOSBox/original FIRES runtime hit", "post-F0128 F0097/VIDRV true stop", "original-vs-Firestaff pixel parity", "bitmap/screenshot promotion"],
    }


def write_report(manifest: dict) -> None:
    lines = [
        "# Pass357 — DM1 V1 original-runtime true-stop/control blocker",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "## Verdict",
        "",
        "The ReDMCSB order is source-locked, and Firestaff-side live dispatch is already proven by pass350, but the original-runtime side still lacks the strict true-stop sequence needed to promote `viewport_present` or any original-vs-Firestaff viewport/walls comparator.",
        "",
        "## ReDMCSB source audit",
        "",
    ]
    for row in manifest["sourceAudit"]:
        lines.append(f"- `{row['file']}:{row['lines']}` — `{row['function']}` — {row['decision']} ok=`{row['ok']}`")
    lines += ["", "## Prior debugger/control evidence", ""]
    for name, row in manifest["priorArtifactAudit"].items():
        lines.append(f"- `{name}`: `{row.get('status')}` (expected `{row.get('expectedStatus')}`) ok=`{row.get('ok')}`")
    pred = manifest["trueStopPredicate"]
    lines += [
        "",
        "## Promotion rule",
        "",
        "Do not promote the original viewport comparator from screenshots, BPLIST/setup echoes, or static offsets. Promote only after one bounded original FIRES route records:",
    ]
    for item in pred["requiredOrder"]:
        lines.append(f"- {item}")
    lines += [
        "",
        "## Comparator decision",
        "",
        f"- Firestaff-side live dispatch: `{manifest['originalVsFirestaffComparatorDecision']['firestaffSideLiveDispatch']}`",
        f"- Original runtime viewport-present: `{manifest['originalVsFirestaffComparatorDecision']['originalRuntimeViewportPresent']}`",
        "- Promote viewport comparator: `False`",
        "",
        "## Next unblocker",
        "",
        manifest["nextUnblocker"],
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    manifest = build_manifest()
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(f"status={manifest['status']}")
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    return 0 if manifest["status"] == EXPECTED_STATUS else 1


if __name__ == "__main__":
    raise SystemExit(main())
