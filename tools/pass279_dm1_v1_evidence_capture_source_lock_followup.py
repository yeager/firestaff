#!/usr/bin/env python3
"""Pass279: DM1 V1 evidence/capture source-lock follow-up.

Text-only gate. It verifies that the current N2 evidence state is narrowed to
runtime-hook proof, not source ambiguity: ReDMCSB route seams are present, public
FIRES symbols are available, the original capture route is unblocked, and the
remaining blocker is the missing F0380 runtime hit in the noise-reduced debugger
attempt.
"""
from __future__ import annotations
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
OUT_DIR = ROOT / "parity-evidence/verification/pass279_dm1_v1_evidence_capture_source_lock_followup"
REPORT = ROOT / "parity-evidence/pass279_dm1_v1_evidence_capture_source_lock_followup.md"
PASS249_REPORT = ROOT / "parity-evidence/dm1_v1_original_runtime_route_unblock_20260506.md"
PASS273 = ROOT / "parity-evidence/verification/pass273_dm1_v1_fires_public_symbol_unblock/manifest.json"
PASS278 = ROOT / "parity-evidence/verification/pass278_dm1_v1_f0380_f0128_noise_reduced_runtime_proof/manifest.json"

SOURCE_CHECKS: list[dict[str, Any]] = [
    {"id":"input_table_to_queue","file":"COMMAND.C","function":"F0361_COMMAND_ProcessKeyPress / F0359_COMMAND_ProcessClick_CPSC","range":[1641,1812],"needles":["F0359_COMMAND_ProcessClick_CPSC", "F0361_COMMAND_ProcessKeyPress", "G0432_as_CommandQueue", "F0360_COMMAND_ProcessPendingClick"]},
    {"id":"queue_dequeue_dispatch","file":"COMMAND.C","function":"F0380_COMMAND_ProcessQueue_CPSC","range":[2045,2160],"needles":["void F0380_COMMAND_ProcessQueue_CPSC", "G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"]},
    {"id":"turn_mutates_direction","file":"CLIKMENU.C","function":"F0365_COMMAND_ProcessTypes1To2_TurnParty","range":[142,179],"needles":["void F0365_COMMAND_ProcessTypes1To2_TurnParty", "G0321_B_StopWaitingForPlayerInput = C1_TRUE", "F0284_CHAMPION_SetPartyDirection"]},
    {"id":"move_mutates_coordinates","file":"CLIKMENU.C","function":"F0366_COMMAND_ProcessTypes3To6_MoveParty","range":[180,347],"needles":["void F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE", "G0310_i_DisabledMovementTicks"]},
    {"id":"game_loop_consumes_tuple","file":"GAMELOOP.C","function":"F0002_MAIN_GameLoop_CPSDF","range":[55,95],"needles":["F0261_TIMELINE_Process_CPSEF();", "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);"]},
    {"id":"viewport_composes_buffer","file":"DUNVIEW.C","function":"F0128_DUNGEONVIEW_Draw_CPSF","range":[8318,8611],"needles":["void F0128_DUNGEONVIEW_Draw_CPSF", "G0296_puc_Bitmap_Viewport", "F0127_DUNGEONVIEW_DrawSquareD0C", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"]},
    {"id":"viewport_present_blit","file":"DRAWVIEW.C","function":"F0097_DUNGEONVIEW_DrawViewport","range":[709,858],"needles":["void F0097_DUNGEONVIEW_DrawViewport", "G0324_B_DrawViewportRequested = C1_TRUE", "F0638_GetZone(C007_ZONE_VIEWPORT", "VIDRV_09_BlitViewPort"]},
]

EXPECTED_FUNCTIONS = {
    "F0380_COMMAND_ProcessQueue_CPSC": "22F4:0699",
    "F0365_COMMAND_ProcessTypes1To2_TurnParty": "1EA4:010D",
    "F0366_COMMAND_ProcessTypes3To6_MoveParty": "1EA4:01AA",
    "F0267_MOVE_GetMoveResult_CPSCE": "1859:0516",
    "F0128_DUNGEONVIEW_Draw_CPSF": "23AD:40FE",
    "F0097_DUNGEONVIEW_DrawViewport": "2809:1E31",
}
EXPECTED_GLOBALS = {
    "G0432_as_CommandQueue": "2C20:3E7A",
    "G0308_i_PartyDirection": "2C20:3C92",
    "G0306_i_PartyMapX": "2C20:3C94",
    "G0307_i_PartyMapY": "2C20:3CE0",
    "G0296_puc_Bitmap_Viewport": "2C20:3D2A",
}

def find_line(lines: list[str], needle: str, start: int, end: int) -> int | None:
    compact = " ".join(needle.split())
    for idx in range(start - 1, min(end, len(lines))):
        if compact in " ".join(lines[idx].split()):
            return idx + 1
    return None

def source_audit() -> list[dict[str, Any]]:
    rows = []
    for chk in SOURCE_CHECKS:
        p = SOURCE_ROOT / chk["file"]
        start, end = chk["range"]
        lines = p.read_text(encoding="latin-1", errors="replace").splitlines() if p.exists() else []
        found, missing = {}, []
        for needle in chk["needles"]:
            line = find_line(lines, needle, start, end)
            (missing if line is None else found).__class__
            if line is None:
                missing.append(needle)
            else:
                found[needle] = line
        rows.append({**chk, "source_path": str(p), "needle_lines": found, "missing_needles": missing, "ok": p.exists() and not missing})
    return rows

def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))

def function_map(pass273: dict[str, Any]) -> dict[str, str]:
    out = {}
    for row in pass273.get("functions", []):
        name = row.get("source_name")
        addr = row.get("runtime_with_load_seg_0733")
        if name and addr:
            out[name] = addr
    return out

def global_map(pass273: dict[str, Any]) -> dict[str, str]:
    out = {}
    for row in pass273.get("globals", []):
        name = row.get("source") or row.get("source_name")
        addr = row.get("runtime_with_load_seg_0733") or row.get("runtime")
        if name and addr:
            out[name] = addr
    return out

def report_lines(manifest: dict[str, Any]) -> list[str]:
    lines = [
        "# Pass279 — DM1 V1 evidence/capture source-lock follow-up",
        "",
        "Status: `{}`".format(manifest["status"]),
        "",
        "## Source-lock result",
        "",
    ]
    for row in manifest["source_audit"]:
        mark = "PASS" if row["ok"] else "FAIL"
        lines.append(
            "- {} `{}` — `{}:{}-{}` `{}`".format(
                mark, row["id"], row["file"], row["range"][0], row["range"][1], row["function"]
            )
        )
        for needle, line in row["needle_lines"].items():
            lines.append("  - line {}: `{}`".format(line, needle))
    lines += [
        "",
        "## Evidence state",
        "",
        "- Original DOS capture route is unblocked by pass249: launch reaches live dungeon gameplay frames, controlled `kp5` changes viewport hashes, and 224x136 crops exist outside the repo.",
        "- Public-symbol/source-map blocker is unblocked by pass273: FIRES.MAP/DM.MAP were found and map source functions/globals to runtime addresses without committing original binaries.",
        "- Runtime hook proof remains blocked by pass278: controlled queue writes, party tuple mutation, and F0128 hit are present, but no proven F0380 dequeue BP hit at `22F4:0699`.",
        "",
        "## Runtime addresses locked for the next attempt",
        "",
    ]
    for name, addr in manifest["runtime_bindings"]["functions"].items():
        lines.append("- `{}` — `{}`".format(name, addr))
    lines += ["", "## Runtime globals locked for watches", ""]
    for name, addr in manifest["runtime_bindings"]["globals"].items():
        lines.append("- `{}` — `{}`".format(name, addr))
    lines += [
        "",
        "## Exact remaining blocker",
        "",
        manifest["remaining_blocker"],
        "",
        "Manifest: `{}`".format(OUT_DIR / "manifest.json"),
        "",
    ]
    return lines

def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    audit = source_audit()
    p273, p278 = load_json(PASS273), load_json(PASS278)
    fbind, gbind = function_map(p273), global_map(p273)
    function_ok = {k: fbind.get(k) == v for k, v in EXPECTED_FUNCTIONS.items()}
    global_ok = {k: gbind.get(k) == v for k, v in EXPECTED_GLOBALS.items()}
    capture_report = PASS249_REPORT.read_text(encoding="utf-8") if PASS249_REPORT.exists() else ""
    capture_ok = all(s in capture_report for s in ["PASS route/capture proof", "dungeon_gameplay", "224x136 viewport crops"])
    predicates = p278.get("proof_predicates", {})
    blocker_ok = (p273.get("status") == "UNBLOCKED_PUBLIC_SYMBOLS_FOUND_NO_RUNTIME_HOOK" and p278.get("status") == "BLOCKED_NO_PROVEN_RUNTIME_HOOK" and predicates.get("g0432_write_seen") is True and predicates.get("party_tuple_mutation_seen") is True and predicates.get("f0128_draw_hit_seen") is True and predicates.get("f0380_dequeue_hit_seen") is False)
    status = "PASS_EVIDENCE_CAPTURE_SOURCE_LOCKED_F0380_RUNTIME_HIT_REMAINS" if all(r["ok"] for r in audit) and all(function_ok.values()) and all(global_ok.values()) and capture_ok and blocker_ok else "FAIL_EVIDENCE_CAPTURE_SOURCE_LOCK_FOLLOWUP"
    manifest = {"schema":"pass279_dm1_v1_evidence_capture_source_lock_followup.v1", "status":status, "source_root":str(SOURCE_ROOT), "source_audit":audit, "inputs":{"pass249_report":str(PASS249_REPORT), "pass273_manifest":str(PASS273), "pass278_manifest":str(PASS278)}, "runtime_bindings":{"functions":{k:fbind.get(k) for k in EXPECTED_FUNCTIONS}, "globals":{k:gbind.get(k) for k in EXPECTED_GLOBALS}, "function_checks":function_ok, "global_checks":global_ok}, "capture_route_unblocked":capture_ok, "proof_predicates":predicates, "remaining_blocker":"Need a stock DM1 PC34 dosbox-debug transcript proving a controlled key/click command is dequeued by `F0380_COMMAND_ProcessQueue_CPSC` at `22F4:0699`, then mutates `G0308` or `G0306/G0307`, and is consumed by `F0128_DUNGEONVIEW_Draw_CPSF` at `23AD:40FE`. Source route, public symbols, capture route, and F0128 hit are no longer the blocker."}
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text("\n".join(report_lines(manifest)), encoding="utf-8")
    print(json.dumps({"status":status, "manifest":str(OUT_DIR / "manifest.json"), "report":str(REPORT)}, indent=2, sort_keys=True))
    return 0 if status.startswith("PASS_") else 1
if __name__ == "__main__":
    raise SystemExit(main())
