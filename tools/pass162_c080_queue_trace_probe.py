#!/usr/bin/env python3
"""Pass162 C080 queue trace probe planner for original DM PC lane.

This is intentionally source-first and non-invasive: it audits the ReDMCSB
command path, verifies the N2-only original-game inputs are present, and emits a
small breakpoint/probe plan that narrows the next original-runtime question to:
mouse translation before the original queue, C080 enqueue/dequeue/dispatch, or
front-wall hit-state before F0280.
"""
from __future__ import annotations

import argparse
import json
import shutil
import subprocess
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Iterable

REPO = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
ORIGINAL_DM = Path("/home/trv2/.openclaw/data/firestaff-original-games/DM")
EXTRACTED_DM = ORIGINAL_DM / "_extracted"
PASS162_SUMMARY = REPO / "parity-evidence/verification/pass162_original_party_route_unblock/source_gated_portrait_then_resurrect/summary.json"
OUT_DIR = REPO / "parity-evidence/verification/pass162_c080_queue_trace"


@dataclass(frozen=True)
class Citation:
    file: str
    lines: str
    symbol: str
    point: str
    must_contain: tuple[str, ...]


CITATIONS: tuple[Citation, ...] = (
    Citation("DEFS.H", "305", "C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "C080 is command ordinal 80.", ("#define C080_COMMAND_CLICK_IN_DUNGEON_VIEW",)),
    Citation("DEFS.H", "3752", "C007_ZONE_VIEWPORT", "C007 is the viewport zone used by the PC movement secondary mouse table.", ("#define C007_ZONE_VIEWPORT",)),
    Citation("COMMAND.C", "1-16", "G0432_as_CommandQueue", "Original command queue storage, first/last indices, queue lock, and pending-click fields.", ("G0432_as_CommandQueue", "G0433_i_CommandQueueFirstIndex", "G0434_i_CommandQueueLastIndex", "G0435_B_CommandQueueLocked", "G0436_B_PendingClickPresent")),
    Citation("COMMAND.C", "106-114", "C007 -> C080 mouse route", "PC secondary movement mouse input maps viewport left-click box 0..223,33..168 to C080.", ("G0448_as_Graphic561_SecondaryMouseInput_Movement", "C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "0, 223,  33, 168")),
    Citation("COMMAND.C", "397-403", "C007 -> C080 zone route", "Zone-based movement table maps C007_ZONE_VIEWPORT left-click to C080.", ("C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "C007_ZONE_VIEWPORT")),
    Citation("COMMAND.C", "1452-1662", "F0359_COMMAND_ProcessClick_CPSC", "Actual mouse-click queue writer: derives command from primary/secondary mouse tables and writes nonzero command plus X/Y into G0432_as_CommandQueue.", ("void F0359_COMMAND_ProcessClick_CPSC", "F0358_COMMAND_GetCommandFromMouseInput_CPSC", "G0432_as_CommandQueue", ".Command = L1109_i_Command", ".X = P0725_i_X", ".Y = P0726_i_Y")),
    Citation("CLIKMENU.C", "142-174", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "Required audit symbol: in this ReDMCSB tree F0365 is turn-party handling, not the C080 mouse queue writer; it is dispatched only for C001/C002 in F0380.", ("void F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0284_CHAMPION_SetPartyDirection")),
    Citation("COMMAND.C", "2045-2127", "F0380_COMMAND_ProcessQueue_CPSC dequeue", "F0380 locks/dequeues command, X, Y from G0432_as_CommandQueue and unlocks before dispatch.", ("void F0380_COMMAND_ProcessQueue_CPSC", "L1160_i_Command = G0432_as_CommandQueue", "L1161_i_CommandX", "L1162_i_CommandY", "G0435_B_CommandQueueLocked = C0_FALSE")),
    Citation("COMMAND.C", "2150-2152", "F0380 -> F0365 dispatch", "F0380 dispatches only C001/C002 turn commands to F0365.", ("F0365_COMMAND_ProcessTypes1To2_TurnParty",)),
    Citation("COMMAND.C", "2322-2324", "F0380 -> F0377 dispatch", "F0380 dispatches C080 to F0377 with dequeued X/Y.", ("if (L1160_i_Command == C080_COMMAND_CLICK_IN_DUNGEON_VIEW)", "F0377_COMMAND_ProcessType80_ClickInDungeonView")),
    Citation("CLIKVIEW.C", "311-350", "F0377_COMMAND_ProcessType80_ClickInDungeonView", "C080 handler; PC builds normalize screen coordinates by subtracting viewport origin before hit testing.", ("void F0377_COMMAND_ProcessType80_ClickInDungeonView", "P0752_i_X -= G2067_i_ViewportScreenX", "P0753_i_Y -= G2068_i_ViewportScreenY")),
    Citation("CLIKVIEW.C", "406-439", "F0377 empty-hand front-wall hit", "Empty-hand C05 door-button/wall-ornament hit calls F0372, otherwise object cells call grab/drop paths.", ("G0415_ui_LeaderEmptyHanded", "C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT", "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor")),
    Citation("CLIKVIEW.C", "5-27", "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor", "F0372 computes the square in front of the party and invokes F0275 on the wall face opposite party direction.", ("STATICFUNCTION void F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor", "F0275_SENSOR_IsTriggeredByClickOnWall", "M018_OPPOSITE(G0308_i_PartyDirection)")),
    Citation("MOVESENS.C", "1501-1503", "C127_SENSOR_WALL_CHAMPION_PORTRAIT -> F0280", "A clicked champion portrait wall sensor calls F0280 with sensorData/portrait index.", ("case C127_SENSOR_WALL_CHAMPION_PORTRAIT", "F0280_CHAMPION_AddCandidateChampionToParty")),
    Citation("REVIVE.C", "63-88", "F0280_CHAMPION_AddCandidateChampionToParty", "Candidate champion entry point reached after C127 portrait sensor processing.", ("void F0280_CHAMPION_AddCandidateChampionToParty", "P0596_ui_ChampionPortraitIndex")),
)

BREAKPOINTS = (
    {"gate": "mouse translation / queue write", "source": "COMMAND.C:1452-1662 F0359_COMMAND_ProcessClick_CPSC (not F0365 in this tree)", "probe": "break on F0359 entry and after L1109_i_Command is assigned/written", "expect": "after x=111,y=82 left click, P0725/P0726 are 111/82, L1109_i_Command == 80, G0432_as_CommandQueue[last].Command == 80 with X=111,Y=82", "if_missing": "host/DOSBox mouse translation or active mouse-input table is blocking before the original queue"},
    {"gate": "queue dequeue", "source": "COMMAND.C:2045-2127 F0380_COMMAND_ProcessQueue_CPSC", "probe": "break when L1160/L1161/L1162 are loaded from G0432_as_CommandQueue", "expect": "L1160_i_Command == 80 and L1161/L1162 == 111/82", "if_missing": "queue overwrite/drop/BUG0_73 collision or wrong timing before dispatch"},
    {"gate": "C080 dispatch / viewport normalization", "source": "COMMAND.C:2322-2324 + CLIKVIEW.C:311-350", "probe": "break on F0377 entry and after PC coordinate normalization", "expect": "F0377 is entered; normalized point remains inside C05 wall ornament/portrait hit zone for the source-locked front wall", "if_missing": "C080 is not dispatched or screen-to-viewport translation is different than the visual click assumption"},
    {"gate": "front-wall sensor hit-state", "source": "CLIKVIEW.C:406-439, CLIKVIEW.C:5-27, MOVESENS.C:1501-1503, REVIVE.C:63-88", "probe": "break on F0372 and F0280; log G0306/G0307/G0308, forward square, wall face, sensor type/data", "expect": "pose map0 x=1 y=3 dir=South touches front square x=1 y=4 opposite face and reaches F0280(sensorData=10)", "if_missing": "front-wall hit zone/state/sensor face is blocking after F0377 but before F0280"},
)

DEBUG_NOTES = """# pass162_c080_queue_trace breakpoint notes
# Original PC DM is DOS real-mode code under DOSBox/DOSBox-X, so these are
# source-symbol gates, not direct native gdb commands unless a symbolized/debug
# build or emulator bridge maps ReDMCSB symbols to addresses.
#
# Stop at these gates in order:
# 1. COMMAND.C:F0359_COMMAND_ProcessClick_CPSC entry and post L1109 assignment/write
# 2. COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC after L1160/L1161/L1162 dequeue
# 3. COMMAND.C:2322 C080 branch and CLIKVIEW.C:F0377 entry
# 4. CLIKVIEW.C:F0372 entry and MOVESENS.C:1501/REVIVE.C:F0280
"""


def read_lines(path: Path, line_range: str) -> str:
    if "-" in line_range:
        start, end = [int(part) for part in line_range.split("-", 1)]
    else:
        start = end = int(line_range)
    lines = path.read_text(errors="replace").splitlines()
    return "\n".join(f"{idx + 1}: {lines[idx]}" for idx in range(start - 1, min(end, len(lines))))


def audit_citations() -> list[dict[str, object]]:
    audited: list[dict[str, object]] = []
    for citation in CITATIONS:
        path = SOURCE_ROOT / citation.file
        excerpt = read_lines(path, citation.lines)
        missing = [token for token in citation.must_contain if token not in excerpt]
        audited.append({**asdict(citation), "path": str(path), "ok": not missing, "missing": missing, "excerpt": excerpt})
    return audited


def run_cmd(args: Iterable[str]) -> dict[str, object]:
    try:
        proc = subprocess.run(list(args), text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=10, check=False)
        return {"args": list(args), "returncode": proc.returncode, "output": proc.stdout.strip().splitlines()[:8]}
    except Exception as exc:
        return {"args": list(args), "error": repr(exc)}


def build_manifest(dry_run: bool) -> dict[str, object]:
    citations = audit_citations()
    bins = {name: shutil.which(name) for name in ("dosbox", "dosbox-x", "gdb", "python3", "xdotool", "xwd")}
    pass162 = None
    if PASS162_SUMMARY.exists():
        pass162 = json.loads(PASS162_SUMMARY.read_text())
    manifest = {
        "schema": "pass162_c080_queue_trace.v1",
        "classification": "ready/probe-plan-emitted" if bins.get("dosbox-x") else "blocked/missing-debugger-capable-emulator",
        "dry_run": dry_run,
        "source_root": str(SOURCE_ROOT),
        "allowed_original_roots": [str(ORIGINAL_DM), str(EXTRACTED_DM)],
        "forbidden_roots_note": "DANNESBURK not used by this script.",
        "pass162_context": {"summary_path": str(PASS162_SUMMARY), "loaded": pass162 is not None, "classification": pass162.get("classification") if isinstance(pass162, dict) else None, "reason": pass162.get("reason") if isinstance(pass162, dict) else None},
        "source_audit": citations,
        "symbol_note": "Hard-rule symbol F0365 is audited, but in this ReDMCSB tree F0365 is turn-party; the actual mouse queue writer for C080 is F0359_COMMAND_ProcessClick_CPSC at COMMAND.C:1452-1662.",
        "probe_gates": BREAKPOINTS,
        "tool_probe": {"bins": bins, "dosbox_version": run_cmd([bins["dosbox"], "-version"]) if bins.get("dosbox") else None, "dosbox_x_version": run_cmd([bins["dosbox-x"], "-version"]) if bins.get("dosbox-x") else None},
        "next_step": "Run the emitted gate list in DOSBox-X/debugger against the source-locked pass162 pose; classify first missing gate instead of trying more coordinates.",
        "non_claims": ["does not prove the stock original binary reached C080/F0377/F0280", "does not use DANNESBURK", "does not claim x=111,y=82 is wrong; it narrows where to instrument before changing coordinates"],
    }
    if any(not item["ok"] for item in citations):
        manifest["classification"] = "blocked/source-audit-token-mismatch"
    return manifest


def write_outputs(manifest: dict[str, object]) -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    (OUT_DIR / "breakpoint_notes.txt").write_text(DEBUG_NOTES)
    lines = ["# pass162 C080 queue trace", "", "Purpose: stop portrait coordinate guessing and isolate whether pass162's x=111,y=82 source-locked portrait click reaches the original queue/dispatch path or blocks in mouse translation/hit-state before F0280.", "", f"Classification: `{manifest['classification']}`", "", "## Source-audited path"]
    for item in manifest["source_audit"]:
        status = "PASS" if item["ok"] else "FAIL"
        lines.append(f"- {status} `{item['file']}:{item['lines']}` `{item['symbol']}` — {item['point']}")
    lines += ["", "## Narrow probe gates"]
    for idx, bp in enumerate(BREAKPOINTS, 1):
        lines.append(f"{idx}. **{bp['gate']}** — {bp['source']}; expect: {bp['expect']}; if missing: {bp['if_missing']}")
    bins = manifest["tool_probe"]["bins"]
    lines += ["", "## Tool status", f"- dosbox: `{bins.get('dosbox')}`", f"- dosbox-x: `{bins.get('dosbox-x')}`", f"- gdb: `{bins.get('gdb')}`", "", "## Non-claims"]
    lines += [f"- {claim}" for claim in manifest["non_claims"]]
    lines += ["", f"Next step: {manifest['next_step']}", ""]
    (OUT_DIR / "README.md").write_text("\n".join(lines))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--dry-run", action="store_true", help="audit and print classification without writing artifacts")
    args = parser.parse_args()
    manifest = build_manifest(dry_run=args.dry_run)
    if not args.dry_run:
        write_outputs(manifest)
    print(json.dumps({"classification": manifest["classification"], "out_dir": str(OUT_DIR), "citation_failures": [item for item in manifest["source_audit"] if not item["ok"]]}, indent=2))
    return 0 if manifest["classification"] != "blocked/source-audit-token-mismatch" else 2


if __name__ == "__main__":
    raise SystemExit(main())
