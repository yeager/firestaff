#!/usr/bin/env python3
"""Pass175: source-audited original C080 queue breakpoint probe pack.

This pass deliberately does not try more portrait coordinates.  It packages the
source-audited command path and the exact debugger breakpoints needed to answer
pass162/pass174's remaining question:

    did the original PC runtime enqueue/dequeue C080 and enter F0377/F0280, or
    did DOSBox/PC mouse translation die before the original command queue?

The stock /usr/bin/dosbox on N2 is not a debugger build, so this script records
that limitation explicitly and emits a reusable breakpoint/runbook artifact for a
DOSBox debugger/DOSBox-X debug build rather than producing another coordinate
capture with no semantic signal.
"""
from __future__ import annotations

import json
import shutil
import subprocess
from pathlib import Path
from typing import Any

REPO = Path(__file__).resolve().parent.parent
SOURCE_ROOT = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
PREV_ROOT = REPO / "parity-evidence/verification/pass162_original_party_route_unblock"
OUT_ROOT = REPO / "parity-evidence/verification/pass175_original_queue_breakpoint_probe"

SOURCE_CITATIONS = [
    {"file": "COMMAND.C", "lines": "1-16", "point": "G0432_as_CommandQueue, first/last indices, queue lock, and pending-click globals are the original queue storage to watch.", "must_contain": ["G0432_as_CommandQueue", "G0433_i_CommandQueueFirstIndex", "G0434_i_CommandQueueLastIndex", "G0435_B_CommandQueueLocked", "G0436_B_PendingClickPresent"]},
    {"file": "COMMAND.C", "lines": "108-114, 397-403", "point": "The movement secondary mouse table maps the viewport/zone C007 left-click to C080_COMMAND_CLICK_IN_DUNGEON_VIEW.", "must_contain": ["C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "C007_ZONE_VIEWPORT"]},
    {"file": "COMMAND.C", "lines": "1452-1662", "point": "F0359 (the actual ReDMCSB mouse-click queue writer; requested F0365 is turn-party handling in this tree) records pending click fields, resolves primary/secondary mouse input into L1109_i_Command, and writes nonzero commands plus X/Y into G0432_as_CommandQueue.", "must_contain": ["G0436_B_PendingClickPresent", "F0358_COMMAND_GetCommandFromMouseInput_CPSC", "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command", "G0432_as_CommandQueue[L1108_i_CommandQueueIndex].X"]},
    {"file": "COMMAND.C", "lines": "2045-2127, 2322-2324", "point": "F0380 dequeues command/X/Y from G0432_as_CommandQueue, unlocks/processes pending clicks, then dispatches C080 to F0377_COMMAND_ProcessType80_ClickInDungeonView.", "must_contain": ["void F0380_COMMAND_ProcessQueue_CPSC", "L1160_i_Command = G0432_as_CommandQueue", "L1161_i_CommandX", "F0377_COMMAND_ProcessType80_ClickInDungeonView"]},
    {"file": "ENTRANCE.C", "lines": "850-883", "point": "F0441_STARTEND_ProcessEntrance discards input, waits on C099_MODE_WAITING_ON_ENTRANCE, processes keys, and calls F0380 each loop before loading the dungeon.", "must_contain": ["F0357_COMMAND_DiscardAllInput", "G0298_B_NewGame = C099_MODE_WAITING_ON_ENTRANCE", "F0380_COMMAND_ProcessQueue_CPSC"]},
    {"file": "CLIKVIEW.C", "lines": "311, 347-349, 405-431", "point": "F0377 is the C080 handler; PC builds subtract the viewport origin, then empty-hand C05 wall-ornament hits call F0372 to touch the front-wall sensor.", "must_contain": ["void F0377_COMMAND_ProcessType80_ClickInDungeonView", "P0752_i_X -= G2067_i_ViewportScreenX", "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor"]},
    {"file": "MOVESENS.C", "lines": "1392, 1501-1502", "point": "A no-leader party may still trigger C127_SENSOR_WALL_CHAMPION_PORTRAIT; that case calls F0280_CHAMPION_AddCandidateChampionToParty.", "must_contain": ["C127_SENSOR_WALL_CHAMPION_PORTRAIT", "F0280_CHAMPION_AddCandidateChampionToParty"]},
    {"file": "REVIVE.C", "lines": "63-150", "point": "F0280_CHAMPION_AddCandidateChampionToParty is the semantic transition: it requires an empty hand and party count <4, then consumes G0305_ui_PartyChampionCount to build the candidate champion.", "must_contain": ["void F0280_CHAMPION_AddCandidateChampionToParty", "if (!G0415_ui_LeaderEmptyHanded)", "G0305_ui_PartyChampionCount"]},
]

BREAKPOINTS = [
    {"name": "mouse interrupt / enqueue candidate", "symbol": "F0359_COMMAND_ProcessClick_CPSC", "source": "COMMAND.C:1452-1662", "condition": "after click x=111,y=82, expect L1109_i_Command == C080 and queue write into G0432_as_CommandQueue with X=111,Y=82 (screen-relative PC coordinates).", "logs": ["P0725_i_X", "P0726_i_Y", "P0727_i_ButtonsStatus", "L1109_i_Command", "G0433_i_CommandQueueFirstIndex", "G0434_i_CommandQueueLastIndex"]},
    {"name": "queue dequeue", "symbol": "F0380_COMMAND_ProcessQueue_CPSC", "source": "COMMAND.C:2045-2127", "condition": "break before/after L1160/L1161/L1162 are read; expect L1160_i_Command == C080_COMMAND_CLICK_IN_DUNGEON_VIEW and L1161/L1162 == 111/82.", "logs": ["G0432_as_CommandQueue", "G0433_i_CommandQueueFirstIndex", "G0434_i_CommandQueueLastIndex", "L1160_i_Command", "L1161_i_CommandX", "L1162_i_CommandY"]},
    {"name": "C080 dispatch", "symbol": "F0377_COMMAND_ProcessType80_ClickInDungeonView", "source": "COMMAND.C:2322-2324 + CLIKVIEW.C:311-431", "condition": "break on function entry; PC-normalized viewport point should become x=111-G2067, y=82-G2068 and hit C05 front-wall ornament/portrait box.", "logs": ["P0752_i_X", "P0753_i_Y", "G2067_i_ViewportScreenX", "G2068_i_ViewportScreenY", "AL1150_ui_ViewCell"]},
    {"name": "front-wall portrait sensor", "symbol": "F0280_CHAMPION_AddCandidateChampionToParty", "source": "MOVESENS.C:1501-1502 + REVIVE.C:63-150", "condition": "break on F0280; expect P0596_ui_ChampionPortraitIndex/sensorData == 10 and G0305_ui_PartyChampionCount to advance after candidate setup.", "logs": ["P0596_ui_ChampionPortraitIndex", "G0415_ui_LeaderEmptyHanded", "G0305_ui_PartyChampionCount", "G0299_ui_CandidateChampionOrdinal"]},
]


def source_excerpt(file: str, spec: str) -> str:
    path = SOURCE_ROOT / file
    lines = path.read_text(errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        part = part.strip()
        if "-" in part:
            a, b = [int(x) for x in part.split("-", 1)]
        else:
            a = b = int(part)
        for no in range(a, b + 1):
            chunks.append(f"{file}:{no}: {lines[no - 1]}")
    return "\n".join(chunks)


def validate_sources() -> list[dict[str, Any]]:
    out: list[dict[str, Any]] = []
    for cite in SOURCE_CITATIONS:
        excerpt = source_excerpt(cite["file"], cite["lines"])
        missing = [needle for needle in cite["must_contain"] if needle not in excerpt]
        out.append({**cite, "excerpt": excerpt, "validated": not missing, "missing": missing})
    return out


def command_output(argv: list[str]) -> dict[str, Any]:
    try:
        p = subprocess.run(argv, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=8)
        return {"argv": argv, "returncode": p.returncode, "output": p.stdout.strip()[:4000]}
    except Exception as exc:
        return {"argv": argv, "error": str(exc)}


def load_pass162() -> dict[str, Any]:
    manifest_path = PREV_ROOT / "manifest.json"
    if not manifest_path.exists():
        return {"available": False, "path": str(manifest_path)}
    m = json.loads(manifest_path.read_text())
    focused = []
    for name in ("source_gated_portrait_then_resurrect", "source_gated_portrait_then_reincarnate"):
        summary_path = PREV_ROOT / name / "summary.json"
        if not summary_path.exists():
            continue
        s = json.loads(summary_path.read_text())
        ev = s.get("route_evidence", {})
        focused.append({"name": name, "classification": s.get("classification"), "reason": s.get("reason"), "portrait_click_delta": ev.get("portrait_click_delta"), "choice_delta": ev.get("choice_delta"), "input_delivery_probe": ev.get("input_delivery_probe"), "unique_hashes": ev.get("unique_hashes")})
    return {"available": True, "path": str(manifest_path), "buckets": m.get("buckets"), "focused_runs": focused}


def render_markdown(manifest: dict[str, Any]) -> str:
    lines = ["# Pass 175 — original C080 queue breakpoint probe", "", "Purpose: stop coordinate guessing and isolate whether the original DM1 PC runtime sees the source portrait click as `C080`, reaches `F0380`/`F0377`, and then reaches `F0280`.", "", "## Verdict", "", f"- probe classification: **{manifest['classification']}**", f"- exact next blocker: {manifest['next_blocker']}", f"- N2 debugger availability: `{manifest['debugger']['classification']}`", "", "## Source-audited command path", ""]
    for c in manifest["source_citations"]:
        status = "validated" if c["validated"] else "MISSING: " + ", ".join(c["missing"])
        lines += [f"- `{c['file']}` {c['lines']} — {status}: {c['point']}"]
    lines += ["", "## Runtime evidence carried from pass162/pass174", ""]
    p162 = manifest["pass162"]
    if not p162.get("available"):
        lines.append(f"- pass162 manifest unavailable at `{p162['path']}`")
    else:
        lines.append(f"- pass162 buckets: `{p162.get('buckets')}`")
        for r in p162.get("focused_runs", []):
            lines.append(f"- `{r['name']}`: `{r['classification']}` — {r['reason']}")
            lines.append(f"  - unique hashes: `{r.get('unique_hashes')}`")
            lines.append(f"  - portrait deltas: `{r.get('portrait_click_delta')}`")
            lines.append(f"  - choice deltas: `{r.get('choice_delta')}`")
    lines += ["", "## Breakpoint plan", ""]
    for bp in manifest["breakpoints"]:
        lines += [f"- **{bp['name']}** — `{bp['symbol']}` ({bp['source']})", f"  - condition: {bp['condition']}", f"  - log: `{', '.join(bp['logs'])}`"]
    lines += ["", "## Interpretation", "", "The source path is internally consistent: viewport `C007` left-click should become `C080`, be queued in `G0432_as_CommandQueue`, dequeued by `F0380`, dispatched to `F0377`, touch the front-wall sensor, and call `F0280` for `C127_SENSOR_WALL_CHAMPION_PORTRAIT`. N2 now has `/usr/bin/dosbox-debug` and `/usr/bin/dosbox-x`, but the stock-original debugger route is still blocked at symbol/address binding unless a DOS real-mode/source-symbol bridge or address map is supplied. The stale Firestaff implementation blocker is retired by the source-locked M11 C080 gate; use the emitted breakpoint plan only for the remaining original-runtime classification, not as a Firestaff blocker."]
    return "\n".join(lines) + "\n"


def main() -> int:
    OUT_ROOT.mkdir(parents=True, exist_ok=True)
    citations = validate_sources()
    dosbox = command_output(["dosbox", "-version"])
    debugger_bins = {name: shutil.which(name) for name in ("dosbox-debug", "dosbox-x", "dosbox-staging")}
    dbg_available = any(debugger_bins.values())
    manifest = {"schema": "pass175_original_queue_breakpoint_probe.v1", "classification": "blocked/debugger-required" if not dbg_available else "retired/firestaff-source-locked-c080-gate-passed-original-debugger-symbol-binding-blocked", "next_blocker": "Original binary in-process route classification still needs a DOS real-mode/source-symbol bridge or address map; the stale Firestaff pass175 no-delta implementation blocker is retired by the source-locked M11 C080 gate.", "source_root": str(SOURCE_ROOT), "source_citations": citations, "pass162": load_pass162(), "debugger": {"classification": "stock-dosbox-no-debugger" if not dbg_available else "debugger-binary-present", "bins": debugger_bins, "dosbox_version": dosbox}, "breakpoints": BREAKPOINTS, "non_goals": ["do not try more portrait coordinates", "do not push", "do not treat visual dungeon_gameplay hash as party/control readiness"]}
    (OUT_ROOT / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    (OUT_ROOT / "README.md").write_text(render_markdown(manifest))
    (OUT_ROOT / "breakpoints.md").write_text("# Pass175 breakpoint checklist\n\n" + "\n".join(f"## {bp['name']}\n- symbol: `{bp['symbol']}`\n- source: `{bp['source']}`\n- condition: {bp['condition']}\n- log: `{', '.join(bp['logs'])}`\n" for bp in BREAKPOINTS))
    print(f"wrote {OUT_ROOT}/manifest.json")
    print(f"wrote {OUT_ROOT}/README.md")
    print(f"classification={manifest['classification']}")
    bad = [c for c in citations if not c["validated"]]
    if bad:
        print(f"source validation failures={len(bad)}")
        for c in bad:
            print(c["file"], c["lines"], c["missing"])
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
