#!/usr/bin/env python3
"""Pass175 DM1 V1 C080 original-runtime debugger/address gate.

Source-first gate for the remaining original DM PC question after the Firestaff
C080 implementation blocker was retired: can N2 bind the stock original runtime
tightly enough to prove F0359 -> F0380 -> F0377 -> F0280 for screen click
(111,82), or is the remaining blocker still debugger/address-map binding?

This script does not use DANNESBURK, does not try new coordinates, and does not
retire anything on visual no-delta. It validates the required ReDMCSB source
anchors, probes available debugger/address tools against the N2-local original
DM.EXE, and emits a concrete breakpoint/address-map runbook plus exact blocker.
"""
from __future__ import annotations

import json
import os
import shutil
import subprocess
from pathlib import Path
from typing import Any

REPO = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
ORIGINAL_ROOT = Path("/home/trv2/.openclaw/data/firestaff-original-games/DM")
DM_STAGE = ORIGINAL_ROOT / "_extracted/dm-pc34/DungeonMasterPC34"
DM_EXE = DM_STAGE / "DM.EXE"
OUT = REPO / "parity-evidence/verification/pass175_c080_runtime_debugger_gate"

SOURCE_ANCHORS: list[dict[str, Any]] = [
    {
        "file": "COMMAND.C",
        "lines": "1379-1435",
        "symbol": "F0358_COMMAND_GetCommandFromMouseInput_CPSC",
        "point": "F0358 scans MOUSE_INPUT rectangles/zones and returns the matching command for the click/button state.",
        "must": ["F0358_COMMAND_GetCommandFromMouseInput_CPSC", "F0638_GetZone", "P0721_ps_MouseInput->Command"],
    },
    {
        "file": "COMMAND.C",
        "lines": "1452-1662",
        "symbol": "F0359_COMMAND_ProcessClick_CPSC",
        "point": "F0359 is the original click queue writer: records pending clicks, resolves primary/secondary mouse inputs via F0358, then queues command/X/Y in G0432_as_CommandQueue.",
        "must": ["void F0359_COMMAND_ProcessClick_CPSC", "G0436_B_PendingClickPresent", "F0358_COMMAND_GetCommandFromMouseInput_CPSC", "G0432_as_CommandQueue", ".Command = L1109_i_Command", ".X = P0725_i_X", ".Y = P0726_i_Y"],
    },
    {
        "file": "COMMAND.C",
        "lines": "2045-2127,2322-2324",
        "symbol": "F0380_COMMAND_ProcessQueue_CPSC",
        "point": "F0380 locks/dequeues command/X/Y from G0432_as_CommandQueue, processes pending clicks, then dispatches C080 to F0377.",
        "must": ["void F0380_COMMAND_ProcessQueue_CPSC", "L1160_i_Command = G0432_as_CommandQueue", "L1161_i_CommandX", "L1162_i_CommandY", "F0377_COMMAND_ProcessType80_ClickInDungeonView"],
    },
    {
        "file": "CLIKVIEW.C",
        "lines": "311-350,406-439",
        "symbol": "F0377_COMMAND_ProcessType80_ClickInDungeonView",
        "point": "F0377 is the C080 handler; PC builds subtract viewport origin and empty-hand C05 wall-ornament hits call F0372.",
        "must": ["void F0377_COMMAND_ProcessType80_ClickInDungeonView", "P0752_i_X -= G2067_i_ViewportScreenX", "C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT", "F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor"],
    },
    {
        "file": "DUNVIEW.C",
        "lines": "3913-3930,4210-4215,8343-8349",
        "symbol": "C05 clickable portrait/door-button storage",
        "point": "DUNVIEW stores front-wall champion portrait and D1C door-button clickability in C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT; startup clears the clickable storage.",
        "must": ["G0289_i_DungeonView_ChampionPortraitOrdinal", "G2210_aai_XYZ_DungeonViewClickable[C05_VIEW_CELL_DOOR_BUTTON_OR_WALL_ORNAMENT]", "F0008_MAIN_ClearBytes", "F0010_MAIN_WriteSpacedWords"],
    },
    {
        "file": "MOVESENS.C",
        "lines": "1392,1501-1503",
        "symbol": "C127_SENSOR_WALL_CHAMPION_PORTRAIT",
        "point": "MOVESENS identifies wall champion portrait sensors and calls F0280 with the sensor data/portrait index.",
        "must": ["C127_SENSOR_WALL_CHAMPION_PORTRAIT", "F0280_CHAMPION_AddCandidateChampionToParty"],
    },
    {
        "file": "REVIVE.C",
        "lines": "63-150,260-275,600-625",
        "symbol": "F0280_CHAMPION_AddCandidateChampionToParty",
        "point": "F0280 is the candidate transition reached from C127; it checks empty hand/party count and sets up the candidate champion path.",
        "must": ["void F0280_CHAMPION_AddCandidateChampionToParty", "if (!G0415_ui_LeaderEmptyHanded)", "G0305_ui_PartyChampionCount", "G0299_ui_CandidateChampionOrdinal"],
    },
]

BREAKPOINTS = [
    {
        "gate": "enqueue",
        "symbol": "F0359_COMMAND_ProcessClick_CPSC",
        "source": "COMMAND.C:1452-1662",
        "expect": "for screen click (111,82), P0725/P0726=111/82, L1109_i_Command=80, queue write G0432_as_CommandQueue[last]={Command:80,X:111,Y:82}",
        "blocker_if_missing": "host/emulator mouse translation or active mouse-input table blocks before original queue",
    },
    {
        "gate": "dequeue",
        "symbol": "F0380_COMMAND_ProcessQueue_CPSC",
        "source": "COMMAND.C:2045-2127",
        "expect": "L1160_i_Command=80 and L1161/L1162=111/82 after reading G0432_as_CommandQueue[first]",
        "blocker_if_missing": "queued command is dropped/overwritten/timed differently before C080 dispatch",
    },
    {
        "gate": "C080 handler",
        "symbol": "F0377_COMMAND_ProcessType80_ClickInDungeonView",
        "source": "COMMAND.C:2322-2324 + CLIKVIEW.C:311-439",
        "expect": "F0377 entered, PC viewport origin subtracted, normalized point hits C05 front-wall ornament/portrait clickable cell",
        "blocker_if_missing": "C080 dispatch or screen-to-viewport coordinate binding differs from source-locked assumption",
    },
    {
        "gate": "candidate transition",
        "symbol": "F0280_CHAMPION_AddCandidateChampionToParty",
        "source": "DUNVIEW.C:3913-3930 + MOVESENS.C:1501-1503 + REVIVE.C:63-150,260-275",
        "expect": "front-wall C127_SENSOR_WALL_CHAMPION_PORTRAIT reaches F0280 and sets G0299_ui_CandidateChampionOrdinal",
        "blocker_if_missing": "front-wall C05 hit-state/sensor face/data blocks after F0377 but before candidate transition",
    },
]


def lines_for(spec: str, file: str) -> str:
    src = (SOURCE_ROOT / file).read_text(errors="replace").splitlines()
    out: list[str] = []
    for part in spec.split(","):
        part = part.strip()
        if "-" in part:
            start, end = [int(x) for x in part.split("-", 1)]
        else:
            start = end = int(part)
        for no in range(start, min(end, len(src)) + 1):
            out.append(f"{file}:{no}: {src[no - 1]}")
    return "\n".join(out)


def run(argv: list[str], timeout: int = 12, env: dict[str, str] | None = None) -> dict[str, Any]:
    try:
        p = subprocess.run(argv, cwd=REPO, env={**os.environ, **(env or {})}, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout, check=False)
        return {"argv": argv, "returncode": p.returncode, "output": p.stdout.splitlines()[:80]}
    except Exception as exc:
        return {"argv": argv, "error": repr(exc), "output": []}


def audit_sources() -> list[dict[str, Any]]:
    audited = []
    for anchor in SOURCE_ANCHORS:
        excerpt = lines_for(anchor["lines"], anchor["file"])
        missing = [needle for needle in anchor["must"] if needle not in excerpt]
        audited.append({**anchor, "path": str(SOURCE_ROOT / anchor["file"]), "ok": not missing, "missing": missing, "excerpt": excerpt})
    return audited


def write_debugger_artifacts() -> dict[str, str]:
    OUT.mkdir(parents=True, exist_ok=True)
    gdb = OUT / "stock_dm_symbol_gate.gdb"
    gdb.write_text(f"""set pagination off
set confirm off
file {DM_EXE}
break F0359_COMMAND_ProcessClick_CPSC
break F0380_COMMAND_ProcessQueue_CPSC
break F0377_COMMAND_ProcessType80_ClickInDungeonView
break F0280_CHAMPION_AddCandidateChampionToParty
info files
info breakpoints
""")
    dbconf = OUT / "dosbox-debug-pass175.conf"
    dbconf.write_text(f"""[sdl]
fullscreen=false
output=surface
[dosbox]
machine=svga_paradise
memsize=4
[cpu]
core=normal
cycles=3000
[mixer]
nosound=true
[autoexec]
mount c "{DM_STAGE}"
c:
DM -vv -sn
""")
    runbook = OUT / "breakpoint_runbook.md"
    runbook.write_text("# Pass175 C080 runtime debugger runbook\n\n" + "\n".join(
        f"## {bp['gate']}\n- symbol: `{bp['symbol']}`\n- source: `{bp['source']}`\n- expect: {bp['expect']}\n- if missing: {bp['blocker_if_missing']}\n" for bp in BREAKPOINTS
    ))
    return {"gdb_script": str(gdb), "dosbox_debug_conf": str(dbconf), "runbook": str(runbook)}


def classify(tools: dict[str, str | None], commands: dict[str, Any]) -> tuple[str, str]:
    gdb_text = "\n".join(commands.get("gdb_stock_dm_symbol_gate", {}).get("output", []))
    if "No symbol table is loaded" in gdb_text or "not in executable format" in gdb_text:
        return (
            "blocked/address-map-required",
            "native gdb can open/probe only far enough to show the stock DOS DM.EXE has no ReDMCSB symbol binding; need a DOS real-mode/source-symbol bridge or address map before F0359/F0380/F0377/F0280 can be proven",
        )
    if not tools.get("dosbox-debug") and not tools.get("dosbox-x"):
        return ("blocked/debugger-emulator-missing", "no debugger-capable DOSBox binary found on N2")
    return (
        "blocked/address-map-required",
        "debugger binaries exist, but no N2-local symbol/address map maps ReDMCSB functions to stock DM.EXE runtime addresses yet; emitted runbook is the next executable gate",
    )


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    artifacts = write_debugger_artifacts()
    source_audit = audit_sources()
    tools = {name: shutil.which(name) for name in ("python3", "file", "gdb", "dosbox", "dosbox-debug", "dosbox-x", "objdump", "strings")}
    commands = {
        "dm_exe_file": run([tools.get("file") or "file", str(DM_EXE)]),
        "gdb_version": run([tools.get("gdb") or "gdb", "--version"]),
        "gdb_stock_dm_symbol_gate": run([tools.get("gdb") or "gdb", "--batch", "-x", artifacts["gdb_script"]]),
        "dosbox_debug_version": run([tools.get("dosbox-debug") or "dosbox-debug", "-version"], env={"TERM": "xterm"}) if tools.get("dosbox-debug") else None,
        "dosbox_x_version": run([tools.get("dosbox-x") or "dosbox-x", "-version"]) if tools.get("dosbox-x") else None,
    }
    classification, blocker = classify(tools, commands)
    if any(not x["ok"] for x in source_audit):
        classification = "blocked/source-anchor-mismatch"
        blocker = "required ReDMCSB source anchors no longer match exact files/lines; do not proceed to runtime claims"
    manifest = {
        "schema": "pass175_c080_runtime_debugger_gate.v1",
        "classification": classification,
        "exact_remaining_blocker": blocker,
        "click": {"screen_x": 111, "screen_y": 82, "button": "left"},
        "repo": str(REPO),
        "source_root": str(SOURCE_ROOT),
        "allowed_original_roots": [str(ORIGINAL_ROOT), str(ORIGINAL_ROOT / "_extracted")],
        "forbidden_roots_note": "DANNESBURK/192.168.2.126 not used.",
        "dm_exe": str(DM_EXE),
        "tools": tools,
        "artifacts": artifacts,
        "source_audit": source_audit,
        "breakpoint_gates": BREAKPOINTS,
        "commands": commands,
        "non_claims": [
            "does not prove original runtime reached F0359/F0380/F0377/F0280",
            "does not retire pass175 on visual no-delta",
            "does not try alternate coordinates",
            "does not use DANNESBURK or non-N2 references",
        ],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    readme = [
        "# Pass175 C080 runtime debugger/address gate",
        "",
        f"Classification: `{classification}`",
        f"Exact remaining blocker: {blocker}",
        "",
        "## Source anchors",
    ]
    for item in source_audit:
        readme.append(f"- {'PASS' if item['ok'] else 'FAIL'} `{item['file']}:{item['lines']}` `{item['symbol']}` — {item['point']}")
    readme += ["", "## Breakpoint gates"]
    for bp in BREAKPOINTS:
        readme.append(f"- `{bp['symbol']}` ({bp['source']}): {bp['expect']}")
    readme += ["", "## Tool probes"]
    for name, result in commands.items():
        if not result:
            readme.append(f"- `{name}`: unavailable")
        else:
            readme.append(f"- `{name}` rc={result.get('returncode', 'n/a')}: `{' '.join(result['argv'])}`")
            for line in result.get("output", [])[:10]:
                if line.strip():
                    readme.append(f"  - {line}")
    readme += ["", "## Non-claims"] + [f"- {x}" for x in manifest["non_claims"]] + [""]
    (OUT / "README.md").write_text("\n".join(readme))
    print(json.dumps({"classification": classification, "exact_remaining_blocker": blocker, "out": str(OUT), "source_failures": [f"{x['file']}:{x['lines']}" for x in source_audit if not x['ok']]}, indent=2))
    return 0 if classification != "blocked/source-anchor-mismatch" else 2


if __name__ == "__main__":
    raise SystemExit(main())
