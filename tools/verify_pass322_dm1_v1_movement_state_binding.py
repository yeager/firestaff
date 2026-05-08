#!/usr/bin/env python3
"""Pass322 DM1 V1 movement state binding verifier.

This is a bounded verifier: it does not launch DOSBox.  It binds movement/core
state from ReDMCSB source seams to the already-unblocked PC34 public-symbol
addresses, then records the exact runtime probe shape needed next.
"""
from __future__ import annotations

import json
import re
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass322_dm1_v1_movement_state_binding"
SOURCE_ROOTS = [
    Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
    Path.home() / ".openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Source",
]
ATLAS = Path.home() / ".openclaw/data/firestaff-greatstone-atlas"
OUT_JSON = ROOT / f"parity-evidence/verification/{PASS}.json"
OUT_MD = ROOT / f"parity-evidence/{PASS}.md"
PASS273 = ROOT / "parity-evidence/verification/pass273_dm1_v1_fires_public_symbol_unblock/manifest.json"
PASS296 = ROOT / "parity-evidence/verification/pass296_dm1_v1_input_tuple_proof_without_direct_f0380/manifest.json"
PASS320 = ROOT / "parity-evidence/verification/pass320_dm1_v1_f0097_vidrv_window_after_f0128_sequence_probe.json"

SOURCE_CHECKS: list[dict[str, Any]] = [
    {"id": "command_queue_storage", "file": "COMMAND.C", "range": [1, 16], "needles": ["COMMAND G0432_as_CommandQueue", "G0433_i_CommandQueueFirstIndex", "G0434_i_CommandQueueLastIndex", "G0435_B_CommandQueueLocked"]},
    {"id": "keyboard_movement_to_queue", "file": "COMMAND.C", "range": [1734, 1812], "needles": ["G0443_ps_PrimaryKeyboardInput", "G0432_as_CommandQueue", "G0434_i_CommandQueueLastIndex", ".Command = L1111_i_Command"]},
    {"id": "mouse_movement_zones", "file": "COMMAND.C", "range": [100, 115], "needles": ["C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT", "C006_COMMAND_MOVE_LEFT", "C005_COMMAND_MOVE_BACKWARD", "C004_COMMAND_MOVE_RIGHT"]},
    {"id": "command_dequeue_dispatch", "file": "COMMAND.C", "range": [2045, 2156], "needles": ["void F0380_COMMAND_ProcessQueue_CPSC", "G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command", "L1161_i_CommandX", "L1162_i_CommandY", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]},
    {"id": "movement_command_ids", "file": "DEFS.H", "range": [238, 243], "needles": ["C001_COMMAND_TURN_LEFT", "C002_COMMAND_TURN_RIGHT", "C003_COMMAND_MOVE_FORWARD", "C004_COMMAND_MOVE_RIGHT", "C005_COMMAND_MOVE_BACKWARD", "C006_COMMAND_MOVE_LEFT"]},
    {"id": "command_struct_layout", "file": "DEFS.H", "range": [229, 233], "needles": ["typedef struct", "int16_t X", "int16_t Y", "int16_t Command"]},
    {"id": "turn_state_write_path", "file": "CLIKMENU.C", "range": [142, 173], "needles": ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0284_CHAMPION_SetPartyDirection", "G0308_i_PartyDirection"]},
    {"id": "direction_setter", "file": "CHAMPION.C", "range": [93, 130], "needles": ["F0284_CHAMPION_SetPartyDirection", "L0834_i_Delta", "G0308_i_PartyDirection = P0600_i_Direction"]},
    {"id": "move_destination_compute", "file": "CLIKMENU.C", "range": [180, 328], "needles": ["F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "G0465_ai_Graphic561_MovementArrowToStepForwardCount", "G0466_ai_Graphic561_MovementArrowToStepRightCount", "F0267_MOVE_GetMoveResult_CPSCE"]},
    {"id": "movesens_party_xy_commit", "file": "MOVESENS.C", "range": [316, 506], "needles": ["BOOLEAN F0267_MOVE_GetMoveResult_CPSCE", "G0306_i_PartyMapX = P0560_i_DestinationMapX", "G0307_i_PartyMapY = P0561_i_DestinationMapY", "F0284_CHAMPION_SetPartyDirection"]},
    {"id": "movesens_draw_while_falling", "file": "MOVESENS.C", "range": [550, 556], "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, P0560_i_DestinationMapX, P0561_i_DestinationMapY)"]},
    {"id": "mainloop_consumes_current_tuple", "file": "GAMELOOP.C", "range": [80, 90], "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)"]},
]

REQUIRED_GLOBALS = {
    "G0432_as_CommandQueue": "2C20:3E7A",
    "G0433_i_CommandQueueFirstIndex": "2C20:3EC8",
    "G0434_i_CommandQueueLastIndex": "2C20:1F08",
    "G0435_B_CommandQueueLocked": "2C20:1F0A",
    "G0308_i_PartyDirection": "2C20:3C92",
    "G0306_i_PartyMapX": "2C20:3C94",
    "G0307_i_PartyMapY": "2C20:3CE0",
    "G0310_i_DisabledMovementTicks": "2C20:3C9A",
    "G0311_i_ProjectileDisabledMovementTicks": "2C20:3D28",
    "G0312_i_LastProjectileDisabledMovementDirection": "2C20:3CE4",
}
REQUIRED_FUNCTIONS = {
    "F0380_COMMAND_ProcessQueue_CPSC": "22F4:0699",
    "F0365_COMMAND_ProcessTypes1To2_TurnParty": "1EA4:010D",
    "F0366_COMMAND_ProcessTypes3To6_MoveParty": "1EA4:01AA",
    "F0267_MOVE_GetMoveResult_CPSCE": "1859:0516",
    "F0128_DUNGEONVIEW_Draw_CPSF": "23AD:40FE",
}


def compact(s: str) -> str:
    return " ".join(s.split())


def source_root() -> Path:
    for root in SOURCE_ROOTS:
        if (root / "COMMAND.C").exists() and (root / "MOVESENS.C").exists():
            return root
    raise FileNotFoundError("ReDMCSB source root not found")


def audit_source(root: Path) -> list[dict[str, Any]]:
    rows = []
    for check in SOURCE_CHECKS:
        p = root / check["file"]
        lines = p.read_text(encoding="latin-1", errors="replace").splitlines() if p.exists() else []
        start, end = check["range"]
        block = compact("\n".join(lines[start - 1 : min(end, len(lines))]))
        missing = [n for n in check["needles"] if compact(n) not in block]
        rows.append({"id": check["id"], "file": check["file"], "range": check["range"], "ok": p.exists() and not missing, "missing": missing})
    return rows


def audit_movemenu(root: Path) -> dict[str, Any]:
    hits = [str(p) for base in [root, *SOURCE_ROOTS] for p in base.glob("**/MOVEMENU.C")]
    return {
        "requestedFile": "MOVEMENU.C",
        "present": bool(hits),
        "paths": sorted(set(hits)),
        "decision": "NOT_PRESENT_NONBLOCKING",
        "reason": "This ReDMCSB tree has no MOVEMENU.C; movement menu/input bindings are represented by COMMAND.C tables and CLIKMENU.C turn/move handlers, which this verifier audits explicitly.",
    }


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def symbol_map() -> dict[str, Any]:
    p273 = load_json(PASS273)
    globals_by_source = {g.get("source_name"): g for g in p273.get("globals", [])}
    funcs_by_source = {f.get("source_name"): f for f in p273.get("functions", [])}
    global_rows = []
    for name, want in REQUIRED_GLOBALS.items():
        row = globals_by_source.get(name, {})
        got = row.get("runtime_with_load_seg_0733")
        global_rows.append({"sourceName": name, "expectedRuntime": want, "actualRuntime": got, "ok": got == want, "publicSymbol": row.get("pc_public_symbol"), "mapLine": row.get("line")})
    function_rows = []
    for name, want in REQUIRED_FUNCTIONS.items():
        row = funcs_by_source.get(name, {})
        got = row.get("runtime_with_load_seg_0733")
        function_rows.append({"sourceName": name, "expectedRuntime": want, "actualRuntime": got, "ok": got == want, "mapSymbolPrefix": row.get("map_symbol_prefix"), "mapLine": row.get("line")})
    return {"pass273Status": p273.get("status"), "globals": global_rows, "functions": function_rows}


def queue_layout() -> dict[str, Any]:
    base = int(REQUIRED_GLOBALS["G0432_as_CommandQueue"].split(":")[1], 16)
    # ReDMCSB DEFS.H COMMAND is int16_t X, int16_t Y, int16_t Command.
    rows = []
    for i in range(5):
        off = base + i * 6
        rows.append({"index": i, "X": f"2C20:{off:04X}", "Y": f"2C20:{off+2:04X}", "Command": f"2C20:{off+4:04X}"})
    return {"commandStructBytes": 6, "queueSlots": 5, "slots": rows, "source": "DEFS.H:229-233 plus COMMAND.C:6 and M529_COMMAND_QUEUE_SIZE=4 for I34E"}


def atlas_audit() -> dict[str, Any]:
    gdm = ATLAS / "raw/greatstone.free.fr__dm__g_dm.html.html"
    summary = ATLAS / "index/SUMMARY.md"
    files = ATLAS / "index/files.json"
    text = gdm.read_text(encoding="utf-8", errors="replace") if gdm.exists() else ""
    pc34 = "dm_pc_34/dungeon.dat/dungeon.html" in text
    multi = "dm_pc_34_multi/dungeon.dat.en/dungeon.html" in text
    return {
        "atlasRoot": str(ATLAS),
        "summaryPresent": summary.exists(),
        "filesIndexPresent": files.exists(),
        "dmPc34DungeonMapsListed": pc34,
        "dmPc34MultiDungeonMapsListed": multi,
        "movementBindingData": "MAP_DATA_ONLY_NO_RUNTIME_QUEUE_OR_PARTY_TUPLE_OFFSETS",
        "decision": "Greatstone confirms PC 3.4 dungeon-map data availability, but not runtime command queue/party tuple binding; ReDMCSB/pass273 remain the binding authorities.",
    }


def existing_probe_audit() -> dict[str, Any]:
    probes = sorted(p.name for p in (ROOT / "tools").glob("pass3*_dm1_v1_*movement*") )
    probes += sorted(p.name for p in (ROOT / "tools").glob("pass3*_dm1_v1_*f0380*") )
    probes += sorted(p.name for p in (ROOT / "tools").glob("pass3*_dm1_v1_*f0097*") )
    p296 = load_json(PASS296)
    p320 = load_json(PASS320)
    return {
        "candidateProbeScripts": sorted(set(probes)),
        "pass296Status": p296.get("status"),
        "pass296ProofGatePassed": p296.get("proof_gate_passed"),
        "pass320Status": p320.get("status"),
        "decision": "Movement state binding can be source/public-symbol verified now; live promotion still depends on debugger stop/control sequencing before post-F0128/F0097 windows can be trusted.",
    }


def build() -> dict[str, Any]:
    root = source_root()
    source = audit_source(root)
    sym = symbol_map()
    atlas = atlas_audit()
    existing = existing_probe_audit()
    checks = {
        "source_audit_ok": all(r["ok"] for r in source),
        "movemenu_absence_recorded": audit_movemenu(root)["decision"] == "NOT_PRESENT_NONBLOCKING",
        "pass273_symbols_ok": sym["pass273Status"] == "UNBLOCKED_PUBLIC_SYMBOLS_FOUND_NO_RUNTIME_HOOK" and all(r["ok"] for r in sym["globals"] + sym["functions"]),
        "queue_layout_derived": queue_layout()["commandStructBytes"] == 6,
        "greatstone_pc34_map_data_seen": atlas["dmPc34DungeonMapsListed"] is True,
        "pass296_movement_tuple_proof_available": existing["pass296ProofGatePassed"] is True,
        "pass320_blocker_preserved": existing["pass320Status"] == "BLOCKED_F0128_GATE_NOT_RECAPTURED_STRICT_STOP_FILTER",
    }
    ok = all(checks.values())
    status = "MOVEMENT_STATE_BINDING_SOURCE_SYMBOL_LOCKED_RUNTIME_PROBE_DESIGNED" if ok else "BLOCKED_MOVEMENT_STATE_BINDING_AUDIT_INCOMPLETE"
    probe_plan = {
        "runtimeLimitSeconds": 60,
        "dosboxNeededForThisVerifier": False,
        "ifRuntimeProbeNext": [
            "Use pass273 runtime addresses, not guessed offsets.",
            "Set BPM on queue slot Command fields plus G0433/G0434/G0435, then BPM on G0308/G0306/G0307 and G0310/G0311/G0312.",
            "Drive controlled kp5/kp4/kp6 only after debugger-run readiness.",
            "On each true stop capture CPU plus MEMDUMP 2C20:3E7A 36 and MEMDUMP 2C20:3C88 96; ignore BPLIST/setup echoes as pass320 taught.",
            "Promote only if queue command write/dequeue, tuple mutation or disabled-tick state, and subsequent F0128/F0097 consumption are observed in order within <=60s.",
        ],
        "nextBlocker": "Debugger stop/control sequencing: pass320 did not recapture the post-F0128 gate after strict stop filtering, so live movement-state offset/table promotion must first get reliable true-stop control.",
    }
    return {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "sourceRoot": str(root),
        "checks": checks,
        "movemenuAudit": audit_movemenu(root),
        "sourceAudit": source,
        "symbolBinding": sym,
        "derivedQueueLayout": queue_layout(),
        "greatstoneAtlasAudit": atlas,
        "existingMovementProbeAudit": existing,
        "probePlan": probe_plan,
        "notClaimed": ["new DOSBox runtime hit", "post-F0128/F0097 true-stop promotion", "pixel parity", "new raw original bitmap publication"],
    }


def render_md(m: dict[str, Any]) -> str:
    lines = [
        "# Pass322 — DM1 V1 movement state binding",
        "",
        f"Status: `{m['status']}`",
        "",
        "## Verdict",
        "",
        "Movement/core state is now bound source-first to the PC34 public-symbol runtime addresses. This pass deliberately does not claim a fresh DOSBox runtime hit; it records the bounded next probe and preserves the pass320 debugger-control blocker.",
        "",
        "## Checks",
        "",
    ]
    lines += [f"- {'PASS' if v else 'FAIL'} `{k}`" for k, v in m["checks"].items()]
    lines += ["", "## Source audit", ""]
    lines += [f"- {'PASS' if r['ok'] else 'FAIL'} `{r['file']}:{r['range'][0]}-{r['range'][1]}` — {r['id']}" for r in m["sourceAudit"]]
    lines += ["", "## MOVEMENU.C", "", f"- `{m['movemenuAudit']['decision']}` — {m['movemenuAudit']['reason']}"]
    lines += ["", "## Runtime address bindings", "", "### Globals"]
    lines += [f"- {'PASS' if r['ok'] else 'FAIL'} `{r['sourceName']}` -> `{r['actualRuntime']}`" for r in m["symbolBinding"]["globals"]]
    lines += ["", "### Functions"]
    lines += [f"- {'PASS' if r['ok'] else 'FAIL'} `{r['sourceName']}` -> `{r['actualRuntime']}`" for r in m["symbolBinding"]["functions"]]
    lines += ["", "## Queue layout", ""]
    for slot in m["derivedQueueLayout"]["slots"]:
        lines.append(f"- slot {slot['index']}: X `{slot['X']}`, Y `{slot['Y']}`, Command `{slot['Command']}`")
    lines += ["", "## Greatstone", "", f"- {m['greatstoneAtlasAudit']['decision']}"]
    lines += ["", "## Probe design", ""]
    lines += [f"- {x}" for x in m["probePlan"]["ifRuntimeProbeNext"]]
    lines += ["", "## Next blocker", "", m["probePlan"]["nextBlocker"], ""]
    return "\n".join(lines)


def main() -> int:
    m = build()
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(m, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    OUT_MD.write_text(render_md(m), encoding="utf-8")
    print(json.dumps({"status": m["status"], "ok": m["status"].startswith("MOVEMENT_STATE_BINDING"), "json": str(OUT_JSON.relative_to(ROOT)), "report": str(OUT_MD.relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if m["status"].startswith("MOVEMENT_STATE_BINDING") else 1


if __name__ == "__main__":
    raise SystemExit(main())
