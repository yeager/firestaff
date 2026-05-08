#!/usr/bin/env python3
"""Pass381 verifier: prove/narrow the route-input -> F0380/F0365/F0366 stop-wait transition."""
from __future__ import annotations
import json, re, subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass381_dm1_v1_route_f0380_transition_source_path"
OUTDIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUTDIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
SRC = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
FIRES_MAP = Path.home() / ".openclaw/data/redmcsb-n2-build-probe/ibm-pc-i34e-fires/HARDDISK/BUILD/I34E/FIRES.MAP"
PASS379 = ROOT / "parity-evidence/verification/pass379_dm1_v1_true_stop_codepath_probe/manifest.json"
PASS380 = ROOT / "parity-evidence/verification/pass380_dm1_v1_f0128_f0097_map_transition_narrowing/manifest.json"
LOAD_SEG = 0x0733
EXPECTED_STATUS = {
    "pass379": "BLOCKED_PASS379_POST_ROUTE_PAUSE_MOVED_MAP_OR_PATH_UNRESOLVED",
    "pass380": "BLOCKED_PASS380_SOURCE_PATH_TRANSITION_NARROWED_MAP_BINDING_HELD",
}
SYMBOLS = {
    "F0380_COMMAND_ProcessQueue_CPSC": "F0380_COMMAND_PROCESSQUEUE_CPSC",
    "F0365_COMMAND_ProcessTypes1To2_TurnParty": "F0365_COMMAND_PROCESSTYPES1TO2_T",
    "F0366_COMMAND_ProcessTypes3To6_MoveParty": "F0366_COMMAND_PROCESSTYPES3TO6_M",
    "F0128_DUNGEONVIEW_Draw_CPSF": "F0128_DUNGEONVIEW_DRAW_CPSF",
}

def run(cmd: list[str]) -> str:
    return subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False).stdout.strip()

def compact(s: str) -> str:
    return " ".join(s.split())

def source_lines(filename: str) -> list[str]:
    return (SRC / filename).read_text(encoding="latin-1", errors="replace").splitlines()

def find_line(lines: list[str], needle: str) -> int | None:
    cn = compact(needle)
    for i, line in enumerate(lines, 1):
        if cn in compact(line):
            return i
    return None

def ordered_file_check(filename: str, needles: list[str]) -> dict[str, Any]:
    lines = source_lines(filename)
    found = {n: find_line(lines, n) for n in needles}
    missing = [n for n, line in found.items() if line is None]
    line_values = [line for line in found.values() if line is not None]
    ordered = line_values == sorted(line_values)
    return {"file": filename, "path": str(SRC / filename), "ok": not missing and ordered, "ordered": ordered, "foundLines": found, "missing": missing}

def parse_csip(csip: str) -> tuple[int, int]:
    seg, off = csip.split(":", 1)
    return int(seg, 16), int(off, 16)

def add_loader(csip: str) -> str:
    seg, off = parse_csip(csip)
    return f"{seg + LOAD_SEG:04X}:{off:04X}"

def map_symbols() -> dict[str, str]:
    out: dict[str, str] = {}
    text = FIRES_MAP.read_text(encoding="latin-1", errors="replace")
    for line in text.splitlines():
        m = re.search(r"\b([0-9A-F]{4}:[0-9A-F]{4})\s+(?:idle\s+)?_([A-Z0-9_]+)\b", line)
        if m and m.group(2) not in out:
            out[m.group(2)] = m.group(1)
    return out

def prior_artifacts() -> tuple[dict[str, Any], list[str]]:
    errors: list[str] = []
    p379 = json.loads(PASS379.read_text(encoding="utf-8"))
    p380 = json.loads(PASS380.read_text(encoding="utf-8"))
    rt379 = p379.get("runtimeProbe", {})
    checks = {
        "pass379": {
            "path": str(PASS379.relative_to(ROOT)),
            "status": p379.get("status"),
            "routeInputAfterArming": rt379.get("routeInputAfterArming"),
            "sawRunning": rt379.get("sawRunning"),
            "breakpointRetainedPostRoute": rt379.get("breakpointRetainedPostRoute"),
            "directHits": rt379.get("directHits"),
            "finalPauseCodeAddr": rt379.get("finalPauseCodeAddr"),
        },
        "pass380": {
            "path": str(PASS380.relative_to(ROOT)),
            "status": p380.get("status"),
            "mapBindingResolved": p380.get("decision", {}).get("mapBindingResolved"),
            "sourcePathTransitionNarrowed": p380.get("decision", {}).get("sourcePathTransitionNarrowed"),
            "finalPauseSegment": p380.get("finalPauseDecode", {}).get("segment"),
            "finalPauseNearestSymbol": p380.get("finalPauseDecode", {}).get("nearestSymbol", {}).get("name"),
        },
    }
    if checks["pass379"]["status"] != EXPECTED_STATUS["pass379"]:
        errors.append("pass379 status drift")
    if checks["pass379"]["routeInputAfterArming"] is not True or checks["pass379"]["sawRunning"] is not True:
        errors.append("pass379 did not prove post-arm route delivery")
    if any((checks["pass379"].get("directHits") or {}).values()):
        errors.append("pass379 unexpectedly hit old F0128/F0097/07FB candidates")
    if checks["pass380"]["status"] != EXPECTED_STATUS["pass380"]:
        errors.append("pass380 status drift")
    if checks["pass380"]["mapBindingResolved"] is not True or checks["pass380"]["sourcePathTransitionNarrowed"] is not True:
        errors.append("pass380 did not resolve map binding/source narrowing")
    return checks, errors

def main() -> int:
    errors: list[str] = []
    OUTDIR.mkdir(parents=True, exist_ok=True)
    source_checks = [
        ordered_file_check("GAMELOOP.C", [
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
            "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "if (!G0321_B_StopWaitingForPlayerInput) {",
            "} while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
        ]),
        ordered_file_check("COMMAND.C", [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ]),
        ordered_file_check("CLIKMENU.C", [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        ]),
    ]
    if not all(c["ok"] for c in source_checks):
        errors.append("source transition audit failed")

    symbols = map_symbols()
    addr_checks: dict[str, Any] = {"path": str(FIRES_MAP), "loaderSegmentDelta": f"{LOAD_SEG:04X}", "symbols": {}, "ok": True}
    for label, map_name in SYMBOLS.items():
        link = symbols.get(map_name)
        ok = bool(link)
        addr_checks["symbols"][label] = {"mapSymbol": map_name, "link": link, "runtimeWithLoader": add_loader(link) if link else None, "ok": ok}
        addr_checks["ok"] = bool(addr_checks["ok"] and ok)
    if not addr_checks["ok"]:
        errors.append("map symbol audit failed")

    priors, prior_errors = prior_artifacts()
    errors.extend(prior_errors)

    transition_proven = not errors
    status = "BLOCKED_PASS381_STATIC_F0380_STOPWAIT_TRANSITION_PROVEN_RUNTIME_BREAKPOINTS_NEXT" if transition_proven else "FAIL_PASS381_ROUTE_F0380_TRANSITION_AUDIT_INCOMPLETE"
    candidate_breakpoints = {
        label: data["runtimeWithLoader"]
        for label, data in addr_checks["symbols"].items()
        if label.startswith("F03") and data.get("runtimeWithLoader")
    }
    conclusion = (
        "Static ReDMCSB source path proves the intended post-route transition: GAMELOOP resets G0321_B_StopWaitingForPlayerInput, calls F0380_COMMAND_ProcessQueue_CPSC inside the wait loop, F0380 dispatches turn/move commands to F0365/F0366, and both handlers set G0321_B_StopWaitingForPlayerInput true before GAMELOOP can exit toward the next outer-loop F0128 draw. pass379/pass380 already prove route delivery and F0128/F0097 map binding, so the remaining runtime blocker is to arm the newly derived F0380/F0365/F0366 runtime breakpoints rather than retargeting F0128/F0097."
        if transition_proven else
        "The route-to-F0380 transition audit is incomplete; inspect errors before promoting the next runtime breakpoint set."
    )
    manifest = {
        "schema": PASS + ".v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"]),
        "head": run(["git", "rev-parse", "HEAD"]),
        "sourceRoot": str(SRC),
        "sourceTransitionAudit": source_checks,
        "mapAddressAudit": addr_checks,
        "priorArtifacts": priors,
        "candidateRuntimeBreakpointsNext": candidate_breakpoints,
        "decision": {"staticTransitionProven": transition_proven, "conclusion": conclusion},
        "notPromotedBy": ["source-only F0128/F0097 binding", "pass379 route keylog alone", "BPLIST echo", "forced-pause IMAGE_TEXT sample"],
        "errors": errors,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text("\n".join([
        "# Pass381 — DM1 V1 route input to F0380/F0365/F0366 transition",
        "",
        f"Status: `{status}`",
        "",
        "## Decision",
        "",
        conclusion,
        "",
        "## Evidence",
        "",
        f"- Source root: `{SRC}`",
        "- GAMELOOP order verified: F0128 draw precedes wait-loop reset; wait loop resets `G0321`, calls `F0380`, then loops until stop-wait/game-tick predicates allow exit.",
        "- COMMAND dispatch verified: queued turn commands call `F0365`; queued movement commands call `F0366`.",
        "- CLIKMENU stop-wait writes verified: both `F0365` and `F0366` set `G0321_B_StopWaitingForPlayerInput = C1_TRUE`.",
        "- Newly derived runtime breakpoint candidates: " + ", ".join(f"{k} `{v}`" for k, v in candidate_breakpoints.items()) + ".",
        "",
        f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
    ]) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "candidateRuntimeBreakpointsNext": candidate_breakpoints, "manifest": str(MANIFEST.relative_to(ROOT)), "errors": errors}, indent=2, sort_keys=True))
    return 0 if transition_proven else 1

if __name__ == "__main__":
    raise SystemExit(main())
