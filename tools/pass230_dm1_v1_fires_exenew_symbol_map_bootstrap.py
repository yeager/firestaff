#!/usr/bin/env python3
"""Pass230: bootstrap the DM1 PC34 FIRES runtime-image map from FIRES.EXENEW evidence.

This pass intentionally does not invent ReDMCSB function addresses. It narrows
pass208/pass210 from "no decompressed runtime image" to the exact remaining
symbol-map gap: FIRES.MAP/TLINK public symbols or debugger-observed CS:IP hits.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
OUT_DIR = ROOT / "parity-evidence/verification/pass230_dm1_v1_fires_exenew_symbol_map_bootstrap"
REPORT = ROOT / "parity-evidence/pass230_dm1_v1_fires_exenew_symbol_map_bootstrap.md"
PASS208 = ROOT / "parity-evidence/verification/pass208_dm1_v1_fires_loader_csip_source_map_gate/manifest.json"
PASS210 = ROOT / "parity-evidence/verification/pass210_dm1_v1_original_runtime_binding_guard/manifest.json"
PASS229 = ROOT / "parity-evidence/verification/pass229_unlzexe_fires_runtime_unblock/manifest.json"
RUNTIME_IMAGE = ROOT / "data/original_runtime/dm1_pc34_i34e_runtime_image.v1.json"
SYMBOL_MAP = ROOT / "data/original_runtime/dm1_pc34_i34e_symbol_map.v1.json"

SEAMS = [
    {
        "id": "command_accepted",
        "file": "COMMAND.C",
        "line_range": [2045, 2156],
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "needles": ["L1160_i_Command = G0432_as_CommandQueue", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"],
        "line_refs": [
            "COMMAND.C:2075-2085 locks/emptiness-checks the queue",
            "COMMAND.C:2095 reads accepted command id from G0432_as_CommandQueue",
            "COMMAND.C:2118-2126 reads X/Y, decrements queued count, advances first index, unlocks queue",
            "COMMAND.C:2150-2156 dispatches turn or movement handlers",
        ],
        "breakpoint_anchor": "after COMMAND.C:2126 queue unlock or before COMMAND.C:2150 turn/move dispatch",
        "observable": "queue dequeue plus turn/step/click dispatch after accepted command id is read",
    },
    {
        "id": "turn_or_step_state_applied",
        "file": "CLIKMENU.C",
        "line_range": [142, 328],
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "needles": ["F0284_CHAMPION_SetPartyDirection", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE"],
        "line_refs": [
            "CLIKMENU.C:171-173 removes/adds party sensor state around F0284_CHAMPION_SetPartyDirection",
            "CLIKMENU.C:264-269 derives move destination from G0308_i_PartyDirection and movement-arrow deltas",
            "CLIKMENU.C:325-328 commits legal movement through F0267_MOVE_GetMoveResult_CPSCE",
        ],
        "breakpoint_anchor": "after CLIKMENU.C:173 for turns; after CLIKMENU.C:328 for steps",
        "observable": "party direction or destination state after turn/step handler",
    },
    {
        "id": "party_coordinates_committed",
        "file": "MOVESENS.C",
        "line_range": [316, 556],
        "function": "F0267_MOVE_GetMoveResult_CPSCE",
        "needles": ["G0306_i_PartyMapX = P0560_i_DestinationMapX;", "G0307_i_PartyMapY = P0561_i_DestinationMapY;"],
        "line_refs": [
            "MOVESENS.C:438-443 enters destination path and writes G0306_i_PartyMapX/G0307_i_PartyMapY",
            "MOVESENS.C:493-506 rewrites party X/Y/direction again for teleporter destinations",
            "MOVESENS.C:556 redraws during fall using G0308_i_PartyDirection and destination X/Y",
        ],
        "breakpoint_anchor": "memory write watchpoint on G0306_i_PartyMapX/G0307_i_PartyMapY, source-anchored at MOVESENS.C:442-443",
        "observable": "party X/Y global writes for successful move result",
    },
    {
        "id": "draw_uses_mutated_tuple",
        "file": "GAMELOOP.C",
        "line_range": [35, 91],
        "function": "F0002_MAIN_GameLoop_CPSDF",
        "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);"],
        "line_refs": [
            "GAMELOOP.C:55-63 begins each main-loop iteration and handles new map movement result",
            "GAMELOOP.C:80-90 gates drawing while not resting/in inventory and passes G0308/G0306/G0307 to F0128",
        ],
        "breakpoint_anchor": "GAMELOOP.C:90 call to F0128_DUNGEONVIEW_Draw_CPSF with the mutated direction/X/Y tuple",
        "observable": "post-command draw call consumes direction/X/Y tuple",
    },
    {
        "id": "viewport_buffer_composed",
        "file": "DUNVIEW.C",
        "line_range": [8318, 8610],
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "needles": ["G0076_B_UseFlippedWallAndFootprintsBitmaps", "F0127_DUNGEONVIEW_DrawSquareD0C", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"],
        "line_refs": [
            "DUNVIEW.C:8336-8356 prepares floor/ceiling/clickable/temporary viewport buffers",
            "DUNVIEW.C:8357-8368 derives flip state from map X/Y plus direction",
            "DUNVIEW.C:8466-8542 walks relative squares and draws D4..D0 into G0296_puc_Bitmap_Viewport",
            "DUNVIEW.C:8608-8611 requests viewport presentation for I34E/I34M/PC-family builds",
        ],
        "breakpoint_anchor": "before DUNVIEW.C:8608-8611 after buffer composition and before F0097_DUNGEONVIEW_DrawViewport",
        "observable": "dungeon view path composes viewport buffer from current party tuple and requests present",
    },
    {
        "id": "viewport_present",
        "file": "DRAWVIEW.C",
        "line_range": [709, 858],
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "needles": ["F0638_GetZone(C007_ZONE_VIEWPORT", "VIDRV_09_BlitViewPort"],
        "line_refs": [
            "DRAWVIEW.C:709-713 enters F0097_DUNGEONVIEW_DrawViewport",
            "DRAWVIEW.C:821-839 resolves requested dungeon/inventory palette state",
            "DRAWVIEW.C:849-857 resolves C007_ZONE_VIEWPORT and calls VIDRV_09_BlitViewPort for I34E/I34M/P31J",
        ],
        "breakpoint_anchor": "DRAWVIEW.C:857 PC-family VIDRV_09_BlitViewPort call with G0296_puc_Bitmap_Viewport",
        "observable": "C007 viewport-zone blit presents the composed viewport bitmap",
    },
]


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text())


def parse_hex_csip(value: str | None) -> dict[str, int | None]:
    if not value or ":" not in value:
        return {"cs": None, "ip": None}
    cs, ip = value.split(":", 1)
    return {"cs": int(cs, 16), "ip": int(ip, 16)}


def source_window(seam: dict[str, Any]) -> tuple[bool, list[str]]:
    path = SOURCE_ROOT / seam["file"]
    start, end = seam["line_range"]
    text = path.read_text(encoding="utf-8", errors="replace").splitlines()
    window = "\n".join(text[start - 1:end])
    missing = []
    function_markers = [part.strip() for part in seam["function"].split("/")]
    for marker in function_markers:
        if marker and marker not in window:
            missing.append(f"function marker {marker!r}")
    for needle in seam["needles"]:
        if needle not in window:
            missing.append(f"needle {needle!r}")
    return not missing, missing


def first_existing(paths: list[Path]) -> Path | None:
    for p in paths:
        if p.exists():
            return p
    return None


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    pass208 = load_json(PASS208)
    pass210 = load_json(PASS210)
    pass229 = load_json(PASS229)
    runtime = load_json(RUNTIME_IMAGE)
    symbol_map = load_json(SYMBOL_MAP)

    decomp = pass229.get("decompressed") or runtime.get("decompressed_fires") or {}
    packed = pass229.get("source_original") or runtime.get("packed_fires") or {}
    exenew_candidates = [ROOT / "parity-evidence/verification/pass229_unlzexe_fires_runtime_unblock/FIRES.EXENEW"]
    if decomp.get("path"):
        exenew_candidates.append(Path(str(decomp["path"])))
    local_exenew = first_existing(exenew_candidates)

    binary_checks: dict[str, Any] = {"present": bool(local_exenew), "path": str(local_exenew) if local_exenew else None}
    if local_exenew:
        actual_size = local_exenew.stat().st_size
        actual_sha = sha256(local_exenew)
        binary_checks.update({
            "size": actual_size,
            "sha256": actual_sha,
            "expected_size": decomp.get("size"),
            "expected_sha256": decomp.get("sha256"),
            "size_ok": actual_size == decomp.get("size"),
            "sha256_ok": actual_sha == decomp.get("sha256"),
            "first_64_matches_runtime_fixture": local_exenew.read_bytes()[:64].hex() == decomp.get("first_64"),
        })
    image_size = int(decomp.get("size", 0)) - int(decomp.get("header_bytes", 0))
    entry = parse_hex_csip(decomp.get("entry_cs_ip"))
    source_audit = []
    for seam in SEAMS:
        ok, missing = source_window(seam)
        source_audit.append({
            "id": seam["id"],
            "citation": f"{seam['file']}:{seam['line_range'][0]}-{seam['line_range'][1]}",
            "function": seam["function"],
            "observable": seam["observable"],
            "breakpoint_anchor": seam["breakpoint_anchor"],
            "line_refs": seam["line_refs"],
            "ok": ok,
            "missing": missing,
        })

    unresolved_symbols = []
    for entry_doc in symbol_map.get("entries", []):
        if entry_doc.get("confidence") != "verified_runtime_hit" or not entry_doc.get("runtime_cs_ip"):
            unresolved_symbols.append({
                "id": entry_doc.get("id"),
                "function": entry_doc.get("function"),
                "source_citation": entry_doc.get("source_citation"),
                "runtime_cs_ip": entry_doc.get("runtime_cs_ip"),
                "confidence": entry_doc.get("confidence"),
            })

    classification = "blocked/source-symbol-map-and-debugger-hits-required"
    exact_remaining_blocker = (
        "FIRES.EXENEW now supplies the decompressed runtime image layout, but no N2-local FIRES.MAP/public-symbol "
        "table or debugger-observed source seam CS:IP/global addresses bind ReDMCSB functions to the loaded image."
    )
    runtime_formula = {
        "schema": "dm1_pc34_i34e_runtime_address_formula.v1",
        "module": "FIRES",
        "packed_fires_sha256": packed.get("sha256"),
        "decompressed_fires_sha256": decomp.get("sha256"),
        "decompressed_evidence_path": str(local_exenew) if local_exenew else None,
        "decompressed_binary_present_on_n2": bool(local_exenew),
        "decompressed_binary_checks": binary_checks,
        "mz_header_bytes": decomp.get("header_bytes"),
        "loaded_image_bytes_excluding_mz_header": image_size,
        "relocations": decomp.get("relocations"),
        "entry_relative_cs_ip": decomp.get("entry_cs_ip"),
        "entry_relative_cs": entry["cs"],
        "entry_ip": entry["ip"],
        "load_base_rule": "DOS loads the EXE image immediately after the MZ header at program_load_segment = PSP + 0x10; TLINK/MAP segment:offsets are relative to that loaded image, not the packed LZEXE file bytes.",
        "runtime_cs_formula": "runtime_cs = program_load_segment + map_segment_or_relative_cs",
        "runtime_ip_formula": "runtime_ip = map_offset_or_relative_ip",
        "linear_formula": "linear_pc = (runtime_cs << 4) + runtime_ip",
        "non_promotable_inputs": ["packed FIRES compressed loader entry 1665:000e", "static compressed-file offsets", "source citations without FIRES.MAP/debugger CS:IP binding"],
    }
    symbol_gap = {
        "schema": "pass230_symbol_gap.v1",
        "classification": classification,
        "exact_remaining_blocker": exact_remaining_blocker,
        "supersedes_blocker_fragment": {
            "pass208_old_blocker": pass208.get("exact_remaining_blocker"),
            "pass210_classification": pass210.get("classification"),
            "pass229_status": pass229.get("status"),
        },
        "unresolved_symbol_entries": unresolved_symbols,
        "required_to_promote": [
            "PSP/program_load_segment from the actual FIRES run",
            "FIRES.MAP/TLINK public symbols or verified function/global offsets for each source seam",
            "debugger/hook hits proving command_accepted -> movement_applied -> viewport_present order",
        ],
    }
    manifest = {
        "schema": "pass230_dm1_v1_fires_exenew_symbol_map_bootstrap.v1",
        "classification": classification,
        "exact_remaining_blocker": exact_remaining_blocker,
        "n2_only": True,
        "inputs": {"pass208": str(PASS208), "pass210": str(PASS210), "pass229": str(PASS229), "runtime_image": str(RUNTIME_IMAGE), "symbol_map": str(SYMBOL_MAP), "redmcsb_source_root": str(SOURCE_ROOT)},
        "runtime_formula": runtime_formula,
        "source_audit": source_audit,
        "symbol_gap": symbol_gap,
        "non_claims": ["does not commit or require FIRES.EXENEW binary payload in git", "does not claim ReDMCSB functions are mapped to runtime CS:IP", "does not claim any debugger breakpoint/watchpoint was hit"],
    }
    (OUT_DIR / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    (OUT_DIR / "runtime_address_formula.json").write_text(json.dumps(runtime_formula, indent=2, sort_keys=True) + "\n")
    (OUT_DIR / "symbol_map_gap.json").write_text(json.dumps(symbol_gap, indent=2, sort_keys=True) + "\n")
    (OUT_DIR / "breakpoint_promotion_contract.md").write_text(
        "# Pass230 breakpoint promotion contract\n\n"
        "Use `FIRES.EXENEW` only as the decompressed image/layout input. Promote no event until a map/debugger source binding supplies each CS:IP/global address.\n\n"
        + "\n".join(f"- `{s['id']}` — `{s['citation']}` `{s['function']}`: {s['observable']} Breakpoint anchor: {s['breakpoint_anchor']}." for s in source_audit)
        + "\n\nRequired formula: `runtime_cs = (PSP + 0x10) + map_segment`, `runtime_ip = map_offset`.\n"
    )

    lines = [
        "# Pass230 — DM1 V1 FIRES.EXENEW symbol-map bootstrap",
        "",
        f"Classification: `{classification}`",
        f"Exact remaining blocker: {exact_remaining_blocker}",
        "",
        "## What is now unblocked",
        f"- Decompressed FIRES image evidence: sha256 `{decomp.get('sha256')}`, size `{decomp.get('size')}`, header bytes `{decomp.get('header_bytes')}`, relocations `{decomp.get('relocations')}`.",
        f"- Loaded image bytes excluding MZ header: `{image_size}`.",
        f"- Entry relative CS:IP from FIRES.EXENEW: `{decomp.get('entry_cs_ip')}`.",
        "- Runtime-base formula is now explicit: `program_load_segment = PSP + 0x10`; map segment:offsets bind as `runtime_cs = program_load_segment + map_segment`, `runtime_ip = map_offset`.",
        "",
        "## ReDMCSB source seams re-audited",
    ]
    for seam in source_audit:
        lines.append(f"- {'PASS' if seam['ok'] else 'FAIL'} `{seam['id']}` — `{seam['citation']}` `{seam['function']}`; observable: {seam['observable']}; breakpoint anchor: {seam['breakpoint_anchor']}")
        for ref in seam["line_refs"]:
            lines.append(f"  - {ref}")
    lines += [
        "",
        "## Still blocked",
        "- No FIRES.MAP/TLINK public-symbol table or debugger-observed CS:IP/global addresses exist for the seams above.",
        "- The packed loader entry and static compressed offsets remain non-promotable.",
        "",
        "## Artifacts",
        f"- `{OUT_DIR / 'manifest.json'}`",
        f"- `{OUT_DIR / 'runtime_address_formula.json'}`",
        f"- `{OUT_DIR / 'symbol_map_gap.json'}`",
        f"- `{OUT_DIR / 'breakpoint_promotion_contract.md'}`",
    ]
    REPORT.write_text("\n".join(lines) + "\n")
    print(json.dumps({"classification": classification, "report": str(REPORT)}, indent=2, sort_keys=True))
    binary_ok = bool(binary_checks.get("present")) and binary_checks.get("size_ok") and binary_checks.get("sha256_ok") and binary_checks.get("first_64_matches_runtime_fixture")
    return 0 if all(s["ok"] for s in source_audit) and decomp.get("sha256") else 1


if __name__ == "__main__":
    raise SystemExit(main())
