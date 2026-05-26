#!/usr/bin/env python3
"""
Pass H2248: CSB V1 Phase 7 — Source-Evidence Manifest

Collects and cross-references all source evidence used in the CSB V1
Phase 7 verification suite artifacts. This is the master evidence index
for the CSB V1 Phase 7 pass.

Schema: firestaff.csb_v1.source_evidence_manifest.v1
"""
from __future__ import annotations

import json
from dataclasses import asdict, dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/passH2248_csb_v1_source_evidence_manifest.json"
REPORT = ROOT / "parity-evidence/firestaff_csb_v1_phase7_source_evidence_manifest_H2248.md"

REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
REDMCSB_PC = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/IBM PC/Source"
CSB_SRC = Path.home() / ".openclaw/data/firestaff-csb-source/CSB/src"
CSBWIN = Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin"

# ── All source anchors across all Phase 7 artifacts ───────────────────────

ALL_ANCHORS: list[dict] = [
    # ── Asset manifest ──
    {
        "artifact": "passH2248_csb_v1_canonical_asset_manifest",
        "id": "redmcsb_csb_dungeon_ids",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "DEFS.H"),
        "lines": "519-523",
        "needles": ["C12_DUNGEON_CSB_PRISON", "C13_DUNGEON_CSB_GAME", "Value used in CSB MINI.DAT"],
        "claim": "CSB prison/game dungeon IDs and association with MINI.DAT.",
        "phase": "P2",
    },
    {
        "artifact": "passH2248_csb_v1_canonical_asset_manifest",
        "id": "redmcsb_save_header_format",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "SAVEHEAD.C"),
        "lines": "14-63",
        "needles": ["F0417_SAVEUTIL_GetChecksumAndObfuscate", "C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX", "128"],
        "claim": "CSB save header uses key index C29, not the DM key index C10.",
        "phase": "P6",
    },
    {
        "artifact": "passH2248_csb_v1_canonical_asset_manifest",
        "id": "csb_lineage_payload_contract",
        "source": "CSB lineage",
        "path": str(CSB_SRC / "README") if CSB_SRC.exists() else "N/A",
        "lines": "14-21",
        "needles": ["dungeon.dat", "hcsb.dat", "hcsb.hct", "mini.dat", "graphics.dat", "config.txt"],
        "claim": "CSB runtime requires the full payload set: GRAPHICS.DAT, DUNGEON.DAT, HCSB.DAT, HCSB.HTC, MINI.DAT, CONFIG.TXT.",
        "phase": "P1",
    },
    {
        "artifact": "passH2248_csb_v1_canonical_asset_manifest",
        "id": "csb_lineage_graphics_open",
        "source": "CSB lineage",
        "path": str(CSB_SRC / "Graphics.cpp") if CSB_SRC.exists() else "N/A",
        "lines": "1814-1915",
        "needles": ["OpenCSBgraphicsFile", "graphics.dat", "Cannot find 'graphics.dat'", "CSBgraphics.dat"],
        "claim": "CSB graphics open route is separate from DM; optional CSBgraphics.dat boundary.",
        "phase": "P3",
    },
    {
        "artifact": "passH2248_csb_v1_canonical_asset_manifest",
        "id": "csb_lineage_csbgame_save",
        "source": "CSB lineage",
        "path": str(CSB_SRC / "Chaos.cpp") if CSB_SRC.exists() else "N/A",
        "lines": "507-623",
        "needles": ["CSBGAME", "csbgame.dat", "csbgame.bak", "SAVING NEW ADVENTURE", "MINI.DAT"],
        "claim": "CSB Utility / Make New Adventure route uses CSBGAME slots and MINI.DAT support.",
        "phase": "P6",
    },
    {
        "artifact": "passH2248_csb_v1_canonical_asset_manifest",
        "id": "csb_lineage_disk_type",
        "source": "CSB lineage",
        "path": str(CSB_SRC / "Reqdisk.cpp") if CSB_SRC.exists() else "N/A",
        "lines": "1-30",
        "needles": ["GetDiskType_CPSB", "C0x02_SAVE_HEADER_FORMAT_CHAOS_STRIKES_BACK"],
        "claim": "CSB disk type detection returns a format namespace distinct from DM.",
        "phase": "P1",
    },

    # ── Dungeon parser probe ──
    {
        "artifact": "firestaff_csb_v1_dungeon_parser_probe.c",
        "id": "csbwin_dungeon_init",
        "source": "CSBWin",
        "path": str(CSBWIN / "CSBCode.cpp") if CSBWIN.exists() else "N/A",
        "lines": "318-480",
        "needles": ["DBank::Initialize", "TAG00332a", "LoadDungeon"],
        "claim": "CSBWin DBank::Initialize parses the dungeon header at offset 4 with level descriptors.",
        "phase": "P2",
    },
    {
        "artifact": "firestaff_csb_v1_dungeon_parser_probe.c",
        "id": "redmcsb_dungeon_format",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "DUNGEON.C"),
        "lines": "1-100",
        "needles": ["F0148_DUNGEON_GetSquareFirstThingType", "F0151_DUNGEON_GetSquare", "column-major"],
        "claim": "ReDMCSB DUNGEON.C F0148-F0170 defines the shared dungeon.dat format; column-major indexing.",
        "phase": "P2",
    },

    # ── Input scripts ──
    {
        "artifact": "passH2248_csb_v1_deterministic_input_scripts",
        "id": "redmcsb_cedtinch_new_adventure",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "CEDTINCH.C"),
        "lines": "5-63",
        "needles": ["F7086_IsReadyToMakeNewAdventure", "C13_DUNGEON_CSB_GAME", "C12_DUNGEON_CSB_PRISON"],
        "claim": "New Adventure readiness is gated on valid CSB game dungeon source (C13), not just catalog match.",
        "phase": "P6",
    },
    {
        "artifact": "passH2248_csb_v1_deterministic_input_scripts",
        "id": "csbwin_adventuring_mode",
        "source": "CSBWin",
        "path": str(CSBWIN / "Game/readme.txt") if CSBWIN.exists() else "N/A",
        "lines": "1-30",
        "needles": ["Enter the dungeon", "choose prison", "Make New Adventure", "GAMESTATE_EnterPrison"],
        "claim": "CSBWin confirms the workflow: prison entry precedes Make New Adventure and adventuring mode.",
        "phase": "P6",
    },
    {
        "artifact": "passH2248_csb_v1_deterministic_input_scripts",
        "id": "redmcsb_command_forward",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "COMMAND.C"),
        "lines": "1-80",
        "needles": ["C0x41_CMD_MOVE_FORWARD", "F0201_COMMAND_ProcessMoveForward", "C0x43_CMD_TURN_RIGHT"],
        "claim": "Forward movement (0x41) and turn-right (0x43) are documented in COMMAND.C.",
        "phase": "P5",
    },
    {
        "artifact": "passH2248_csb_v1_deterministic_input_scripts",
        "id": "redmcsb_movesens_collision",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "MOVESENS.C"),
        "lines": "1-50",
        "needles": ["F0230_MOVE_CheckBlocked", "C0x01_SQUARE_WALL"],
        "claim": "Forward movement collision check is in MOVESENS.C F0230.",
        "phase": "P5",
    },

    # ── Viewport pixel gate ──
    {
        "artifact": "passH2248_csb_v1_viewport_pixel_gate",
        "id": "redmcsb_viewport_geometry",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "DUNGEON.C"),
        "lines": "1-30",
        "needles": ["320", "200", "viewport", "pixel"],
        "claim": "CSB V1 viewport is fixed at 320×200 game pixels.",
        "phase": "P3",
    },
    {
        "artifact": "passH2248_csb_v1_viewport_pixel_gate",
        "id": "m11_viewport_layout",
        "source": "M11 engine",
        "path": "firestaff_m11_game_view.c",
        "lines": "1-60",
        "needles": ["viewport", "status_bar", "action", "icon", "portrait"],
        "claim": "M11 divides the 320×200 viewport into status bar, dungeon view, action strip, and portrait panel.",
        "phase": "P3",
    },
    {
        "artifact": "passH2248_csb_v1_viewport_pixel_gate",
        "id": "redmcsb_prison_state",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "CEDTINCH.C"),
        "lines": "5-63",
        "needles": ["C13_DUNGEON_CSB_GAME", "C12_DUNGEON_CSB_PRISON"],
        "claim": "Prison entrance is dungeon C12, the first state after Make New Adventure.",
        "phase": "P2",
    },

    # ── Save/load round-trip ──
    {
        "artifact": "passH2248_csb_v1_save_load_round_trip",
        "id": "redmcsb_save_header_obfuscate",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "SAVEHEAD.C"),
        "lines": "1-80",
        "needles": ["F0417_SAVEUTIL_GetChecksumAndObfuscate", "C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX"],
        "claim": "F0417 obfuscates 128 uint16_t words using a 32-entry key table (XOR), then computes checksum.",
        "phase": "P6",
    },
    {
        "artifact": "passH2248_csb_v1_save_load_round_trip",
        "id": "redmcsb_header_read_checksum",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "SAVEHEAD.C"),
        "lines": "14-63",
        "needles": ["F0429_STARTEND_IsReadSaveHeaderSuccessful", "L1321_i_ExpectedChecksum", "L1322_i_Checksum"],
        "claim": "F0429 computes checksum over first 256 bytes (interleaved +/-/XOR), then deobfuscates last 256 and sums to verify.",
        "phase": "P6",
    },
    {
        "artifact": "passH2248_csb_v1_save_load_round_trip",
        "id": "redmcsb_csb_dungeon_id",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "DEFS.H"),
        "lines": "519-523",
        "needles": ["C29_CSB_SAVE_HEADER_DECRYPTION_KEY_INDEX", "C13_DUNGEON_CSB_GAME", "C12_DUNGEON_CSB_PRISON"],
        "claim": "CSB save header uses key index C29; dungeon IDs C12/C13 identify CSB prison/game.",
        "phase": "P6",
    },
    {
        "artifact": "passH2248_csb_v1_save_load_round_trip",
        "id": "redmcsb_save_load_funcs",
        "source": "ReDMCSB",
        "path": str(REDMCSB / "LOADSAVE.C"),
        "lines": "1-50",
        "needles": ["F0435_STARTEND_LoadGame", "F0433_STARTEND_SaveGame", "CSBGAME"],
        "claim": "F0433/F0435 are the primary save/load functions; CSBGAME is the CSB save namespace.",
        "phase": "P6",
    },
]

# Group by source file
from collections import defaultdict
by_source: dict[str, list[dict]] = defaultdict(list)
for anchor in ALL_ANCHORS:
    key = f"{anchor['source']}::{anchor['path'].split('/')[-1]}"
    by_source[key].append(anchor)

# Phase coverage
phase_map = {"P1": 1, "P2": 2, "P3": 3, "P4": 4, "P5": 5, "P6": 6, "P7": 7}
phases_covered = sorted(set(a["phase"] for a in ALL_ANCHORS), key=lambda p: phase_map.get(p, 0))


def build_manifest() -> dict:
    return {
        "schema": "firestaff.csb_v1.source_evidence_manifest.v1",
        "manifest_id": "passH2248_csb_v1_source_evidence_manifest",
        "description": (
            "Master source-evidence index for CSB V1 Phase 7 verification suite. "
            "Collects all source anchors cited in: canonical asset manifest, "
            "dungeon parser probe, deterministic input scripts, viewport/pixel gate, "
            "and save/load round-trip verification."
        ),
        "total_anchors": len(ALL_ANCHORS),
        "phases_covered": phases_covered,
        "phases_missing": [f"P{p}" for p in range(1, 8) if f"P{p}" not in phases_covered],
        "source_file_count": len(by_source),
        "by_source": {k: v for k, v in sorted(by_source.items())},
        "all_anchors": ALL_ANCHORS,
        "source_paths": {
            "ReDMCSB_common": str(REDMCSB),
            "ReDMCSB_ibm_pc": str(REDMCSB_PC),
            "CSB_lineage": str(CSB_SRC) if CSB_SRC.exists() else "unavailable",
            "CSBWin": str(CSBWIN) if CSBWIN.exists() else "unavailable",
        },
        "artifacts": [
            "verify_passH2248_csb_v1_canonical_asset_manifest.py",
            "probes/firestaff_csb_v1_dungeon_parser_probe.c",
            "verify_passH2248_csb_v1_deterministic_input_scripts.py",
            "verify_passH2248_csb_v1_viewport_pixel_gate.py",
            "verify_passH2248_csb_v1_save_load_round_trip.py",
        ],
    }


def write_report(manifest: dict, report_path: Path):
    lines = [
        "# CSB V1 Phase 7 — Source-Evidence Manifest\n",
        f"**Pass:** H2248\n",
        f"**Date:** 2026-05-26\n",
        f"**Schema:** `firestaff.csb_v1.source_evidence_manifest.v1`\n",
        "\n## Overview\n",
        manifest["description"],
        f"\n- Total anchors: {manifest['total_anchors']}\n",
        f"- Unique source files: {manifest['source_file_count']}\n",
        f"- Phases covered: {', '.join(manifest['phases_covered'])}\n",
        f"- Phases missing: {', '.join(manifest['phases_missing']) or 'none'}\n",
        "\n## Source Paths\n",
    ]
    for name, path in manifest["source_paths"].items():
        lines.append(f"- **{name}:** `{path}`\n")

    lines += [
        "\n## Source Files\n",
    ]
    for key, anchors in sorted(manifest["by_source"].items()):
        source, filename = key.split("::", 1)
        lines += [
            f"### `{filename}` ({source})\n",
            f"**{len(anchors)} anchors**\n",
            "\n| Anchor ID | Artifact | Phase | Claim |\n",
            "|-----------|----------|-------|-------|\n",
        ]
        for a in anchors:
            lines.append(
                f"| `{a['id']}` | {a['artifact']} | {a['phase']} | {a['claim'][:80]}... |\n"
            )
        lines.append("\n")

    lines += [
        "\n## All Anchors (by ID)\n",
    ]
    for a in ALL_ANCHORS:
        lines += [
            f"### `{a['id']}`\n",
            f"- Artifact: {a['artifact']}\n",
            f"- Source: {a['source']}\n",
            f"- File: `{a['path']}`\n",
            f"- Lines: {a['lines']}\n",
            f"- Phase: {a['phase']}\n",
            f"- Claim: {a['claim']}\n",
            f"- Needles: `{'`, `'.join(a['needles'])}`\n",
            "\n",
        ]

    report_path.write_text("".join(lines), encoding="utf-8")


def main():
    manifest = build_manifest()
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(f"Wrote manifest to {OUT}")

    write_report(manifest, REPORT)
    print(f"Wrote report to {REPORT}")

    print(f"\nTotal anchors: {manifest['total_anchors']}")
    print(f"Source files: {manifest['source_file_count']}")
    print(f"Phases covered: {', '.join(manifest['phases_covered'])}")
    if manifest["phases_missing"]:
        print(f"Phases missing: {', '.join(manifest['phases_missing'])}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())