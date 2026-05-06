#!/usr/bin/env python3
"""Pass208 DM1 V1 FIRES loader CS:IP/source-map gate.

This is a source-first, N2-only gate for the original-runtime blocker left by
pass175/pass179/pass207. It does not claim a live debugger hit. It narrows the
next debugger/address-map work by binding:

* the stock PC34/I34E FIRES LZEXE loader entry CS:IP from the MZ header,
* the available ReDMCSB I34E comparison FIRES binary and link order,
* exact ReDMCSB source seams for command_accepted, movement_applied,
  party_coordinates_committed, and viewport_present.
"""
from __future__ import annotations

import hashlib
import json
import struct
from pathlib import Path
from typing import Any

REPO = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
IBM_SOURCE = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/IBM PC/Source")
REDMCSB_ROOT = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206")
ORIGINAL_FIRES = Path("/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/DungeonMasterPC34/FIRES")
REDMCSB_FIRES = REDMCSB_ROOT / "Reference/ReDMCSB/I34E/FIRES"
ORIGINAL_REDMCSB_REF_FIRES = REDMCSB_ROOT / "Reference/Original/I34E/FIRES"
LINK_FILE = IBM_SOURCE / "I34E.LNK"
OUT = REPO / "parity-evidence/verification/pass208_dm1_v1_fires_loader_csip_source_map_gate"
REPORT = REPO / "parity-evidence/pass208_dm1_v1_fires_loader_csip_source_map_gate.md"

SOURCE_SEAMS: list[dict[str, Any]] = [
    {"seam": "command_accepted", "file": "COMMAND.C", "lines": "2095-2127,2150-2156,2322-2324", "symbols": ["F0380_COMMAND_ProcessQueue_CPSC", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0377_COMMAND_ProcessType80_ClickInDungeonView"], "must": ["L1160_i_Command = G0432_as_CommandQueue", "L1161_i_CommandX", "F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0377_COMMAND_ProcessType80_ClickInDungeonView"], "break_when": "queued command is read from G0432_as_CommandQueue and before C001/C002/C003-C006/C080 dispatch"},
    {"seam": "movement_applied", "file": "CLIKMENU.C", "lines": "256-270,317-329,345-347", "symbols": ["F0366_COMMAND_ProcessTypes3To6_MoveParty", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE"], "must": ["AL1118_ui_MovementArrowIndex", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "L1117_B_MovementBlocked", "F0267_MOVE_GetMoveResult_CPSCE", "G0310_i_DisabledMovementTicks"], "break_when": "after destination X/Y are computed and before/after the allowed F0267_MOVE_GetMoveResult_CPSCE call"},
    {"seam": "party_coordinates_committed", "file": "MOVESENS.C", "lines": "438-444,493-496,573-578", "symbols": ["F0267_MOVE_GetMoveResult_CPSCE", "G0306_i_PartyMapX", "G0307_i_PartyMapY"], "must": ["P0557_T_Thing == C0xFFFF_THING_PARTY", "G0306_i_PartyMapX = P0560_i_DestinationMapX", "G0307_i_PartyMapY = P0561_i_DestinationMapY"], "break_when": "party thing commits destination coordinates, including teleporter/pit chained commits"},
    {"seam": "viewport_present", "file": "GAMELOOP.C", "lines": "83-91,164-168,215-219", "symbols": ["F0128_DUNGEONVIEW_Draw_CPSF", "F0380_COMMAND_ProcessQueue_CPSC"], "must": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY)", "F0361_COMMAND_ProcessKeyPress", "F0380_COMMAND_ProcessQueue_CPSC"], "break_when": "main loop redraws using the current party direction/X/Y after command processing cadence"},
    {"seam": "viewport_present_blit", "file": "DRAWVIEW.C", "lines": "849-858", "symbols": ["F0097_DUNGEONVIEW_DrawViewport", "VIDRV_09_BlitViewPort"], "must": ["F0638_GetZone(C007_ZONE_VIEWPORT", "VIDRV_09_BlitViewPort", "G0296_puc_Bitmap_Viewport"], "break_when": "PC34 route resolves C007_ZONE_VIEWPORT and blits G0296_puc_Bitmap_Viewport to the video driver"},
    {"seam": "viewport_buffer_composed", "file": "DUNVIEW.C", "lines": "8608-8616", "symbols": ["F0097_DUNGEONVIEW_DrawViewport", "F0098_DUNGEONVIEW_DrawFloorAndCeiling"], "must": ["F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)", "F0098_DUNGEONVIEW_DrawFloorAndCeiling"], "break_when": "DUNVIEW has requested viewport draw and precomputed the next floor/ceiling base"},
]

MZ_FIELDS = ["e_magic", "e_cblp", "e_cp", "e_crlc", "e_cparhdr", "e_minalloc", "e_maxalloc", "e_ss", "e_sp", "e_csum", "e_ip", "e_cs", "e_lfarlc", "e_ovno"]


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def parse_mz(path: Path) -> dict[str, Any]:
    b = path.read_bytes()
    values = dict(zip(MZ_FIELDS, struct.unpack_from("<14H", b, 0)))
    if values["e_magic"] != 0x5A4D:
        raise ValueError(f"{path} is not MZ")
    image_file_size = (values["e_cp"] - 1) * 512 + (values["e_cblp"] or 512)
    header_bytes = values["e_cparhdr"] * 16
    entry_linear = values["e_cs"] * 16 + values["e_ip"]
    stack_linear = values["e_ss"] * 16 + values["e_sp"]
    return {"path": str(path), "exists": path.exists(), "size": len(b), "sha256": sha256(path), "mz": {k: {"hex": f"0x{v:04x}", "dec": v} for k, v in values.items()}, "image_file_size_from_header": image_file_size, "header_bytes": header_bytes, "load_image_bytes": max(0, image_file_size - header_bytes), "lz_signature_at_reloc_table": b[values["e_lfarlc"]:values["e_lfarlc"] + 4].decode("ascii", errors="replace"), "compressed_loader_entry": {"relative_cs_ip": f"{values['e_cs']:04x}:{values['e_ip']:04x}", "relative_linear_offset_from_load_image_base": f"0x{entry_linear:05x}", "dosbox_break_expression_once_psp_known": f"bp (psp_segment + 0x10 + 0x{values['e_cs']:04x}):0x{values['e_ip']:04x}"}, "compressed_loader_stack": {"relative_ss_sp": f"{values['e_ss']:04x}:{values['e_sp']:04x}", "relative_linear_offset_from_load_image_base": f"0x{stack_linear:05x}"}, "first_64_hex": b[:64].hex()}


def lines_for(file: str, spec: str) -> str:
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


def audit_source_seams() -> list[dict[str, Any]]:
    audited: list[dict[str, Any]] = []
    for seam in SOURCE_SEAMS:
        excerpt = lines_for(seam["file"], seam["lines"])
        missing = [needle for needle in seam["must"] if needle not in excerpt]
        audited.append({**seam, "path": str(SOURCE_ROOT / seam["file"]), "ok": not missing, "missing": missing, "excerpt": excerpt})
    return audited


def link_order() -> list[str]:
    return [line.rstrip("+").strip() for line in LINK_FILE.read_text(errors="replace").splitlines() if line.strip()]


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    mz_original = parse_mz(ORIGINAL_FIRES)
    mz_redmcsb = parse_mz(REDMCSB_FIRES)
    mz_original_ref = parse_mz(ORIGINAL_REDMCSB_REF_FIRES)
    seams = audit_source_seams()
    maps_found = [str(p) for p in REDMCSB_ROOT.rglob("*.MAP")]
    classification = "blocked/decompressed-runtime-address-map-required"
    blocker = "Stock FIRES is LZEXE v0.91; this pass proves the compressed loader entry CS:IP and source seams, but no decompressed FIRES memory dump or TLINK FIRES.MAP artifact is present on N2, so source symbols cannot yet be converted to stock runtime CS:IP breakpoints."
    if any(not x["ok"] for x in seams):
        classification = "blocked/source-seam-mismatch"
        blocker = "One or more required ReDMCSB source seams no longer match exact source excerpts."
    manifest = {"schema": "pass208_dm1_v1_fires_loader_csip_source_map_gate.v1", "classification": classification, "exact_remaining_blocker": blocker, "source_root": str(SOURCE_ROOT), "ibm_source_root": str(IBM_SOURCE), "n2_only": True, "stock_original_fires": mz_original, "redmcsb_i34e_fires": mz_redmcsb, "redmcsb_original_i34e_fires_reference": mz_original_ref, "stock_original_matches_redmcsb_original_reference": mz_original["sha256"] == mz_original_ref["sha256"], "redmcsb_comparison_binary_available": REDMCSB_FIRES.exists(), "i34e_link_file": str(LINK_FILE), "i34e_link_order": link_order(), "map_artifacts_found": maps_found, "expected_map_if_rebuilt": r"\\BUILD\\I34E\\FIRES.MAP from Toolchains/IBM PC/Source/MKII.BAT TLINK line", "source_seams": seams, "non_claims": ["does not claim a decompressed original FIRES runtime image base", "does not claim stock FIRES source symbols are mapped to runtime CS:IP", "does not claim debugger breakpoints were hit", "does not use DANNESBURK or non-N2 references"]}
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    (OUT / "source_seams.json").write_text(json.dumps(seams, indent=2, sort_keys=True) + "\n")
    runbook = OUT / "next_debugger_breakpoints.md"
    runbook.write_text("# Pass208 next debugger breakpoint seams\n\n" + "\n".join(f"## {s['seam']}\n- source: `{s['file']}:{s['lines']}`\n- symbols: " + ", ".join(f"`{x}`" for x in s["symbols"]) + f"\n- breakpoint intent: {s['break_when']}\n" for s in seams) + f"\n## Loader entry\n- stock FIRES compressed loader entry: `{mz_original['compressed_loader_entry']['relative_cs_ip']}` relative to DOS load-image base (`PSP+0x10`).\n- DOSBox expression once PSP is known: `{mz_original['compressed_loader_entry']['dosbox_break_expression_once_psp_known']}`.\n")
    report_lines = ["# Pass208 DM1 V1 FIRES loader CS:IP/source-map gate", "", f"Classification: `{classification}`", f"Exact remaining blocker: {blocker}", "", "## Loader facts", f"- Stock original FIRES: `{ORIGINAL_FIRES}` size `{mz_original['size']}` sha256 `{mz_original['sha256']}`.", f"- ReDMCSB bundled original I34E FIRES matches stock: `{manifest['stock_original_matches_redmcsb_original_reference']}`.", f"- ReDMCSB rebuilt/reference I34E FIRES is available: `{REDMCSB_FIRES}` size `{mz_redmcsb['size']}` sha256 `{mz_redmcsb['sha256']}`.", f"- Stock MZ/LZEXE signature at relocation table: `{mz_original['lz_signature_at_reloc_table']}`.", f"- Stock compressed loader entry CS:IP: `{mz_original['compressed_loader_entry']['relative_cs_ip']}` relative to DOS load-image base (`PSP+0x10`); linear offset `{mz_original['compressed_loader_entry']['relative_linear_offset_from_load_image_base']}`.", f"- Stack at loader entry: `{mz_original['compressed_loader_stack']['relative_ss_sp']}`.", "", "## Source seams verified"]
    for s in seams:
        status = "PASS" if s["ok"] else "FAIL"
        report_lines.append(f"- {status} `{s['seam']}` — `{s['file']}:{s['lines']}`; symbols: " + ", ".join(f"`{x}`" for x in s["symbols"]))
    report_lines += ["", "## Link/map evidence", f"- I34E link order file: `{LINK_FILE}` ({len(manifest['i34e_link_order'])} objects).", "- First objects: " + ", ".join(f"`{x}`" for x in manifest["i34e_link_order"][:8]) + ".", f"- Existing `*.MAP` artifacts under ReDMCSB tree: `{len(maps_found)}`.", "- Expected map if the PC toolchain build is run: `\\BUILD\\I34E\\FIRES.MAP` (from `MKII.BAT` TLINK line for `@\\SOURCE\\I34E.LNK`).", "", "## Next exact unblocker", "1. Produce either a decompressed stock FIRES memory dump or an I34E `FIRES.MAP` from the ReDMCSB/Turbo C toolchain.", "2. Bind map/source symbols to runtime by using the MZ load base (`PSP+0x10`) and the LZEXE post-decompression transfer point, not the compressed loader entry alone.", "3. Set debugger breakpoints for `command_accepted`, `movement_applied`, `party_coordinates_committed`, and `viewport_present` using `parity-evidence/verification/pass208_dm1_v1_fires_loader_csip_source_map_gate/next_debugger_breakpoints.md`.", "", "## Non-claims"]
    report_lines += [f"- {x}" for x in manifest["non_claims"]]
    REPORT.write_text("\n".join(report_lines) + "\n")
    print(classification)
    print(blocker)
    print(REPORT)
    print(OUT / "manifest.json")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
