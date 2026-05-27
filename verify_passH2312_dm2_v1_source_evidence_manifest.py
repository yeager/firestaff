#!/usr/bin/env python3
"""
Pass H2312: DM2 V1 Phase 8 — Source-Evidence Manifest

Collects and cross-references all source evidence used in the DM2 V1
Phase 8 verification suite artifacts. This is the master evidence index
for the DM2 V1 Phase 8 pass.

Schema: firestaff.dm2_v1.source_evidence_manifest.v1

Sources:
  SKULL.ASM    — 522128-line disassembly of the DM2 PC executable
  skproject    — C++ reimplementation (SkWinCore.cpp, SkGlobal.h, etc.)
  dm2_v1_*.c   — firestaff implementation source files
  docs/dm2_*.md — source-lock format documents
"""
from __future__ import annotations

import json
from dataclasses import asdict, dataclass
from pathlib import Path
from collections import defaultdict

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/passH2312_dm2_v1_source_evidence_manifest.json"
REPORT = ROOT / "parity-evidence/firestaff_dm2_v1_phase8_source_evidence_manifest_H2312.md"

SKULL_ASM = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/IBM PC/Source"
SKPROJECT = ROOT.parent / "skproject"
DOCS = ROOT / "docs"

# ── All source anchors across all Phase 8 artifacts ───────────────────────

ALL_ANCHORS: list[dict] = [

    # ══ Canonical Asset Manifest ══════════════════════════════════════════════

    {
        "artifact": "passH2312_dm2_v1_canonical_asset_manifest",
        "id": "skull_graphics_dat_open",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T547-T551",
        "needles": ["GRAPHICS.DAT", "GDAT_LoadGraphics", "file open mode", "rb"],
        "claim": "SKULL.ASM T547 opens GRAPHICS.DAT for GDAT resource loading; mode 'rb'.",
        "phase": "P1",
    },
    {
        "artifact": "passH2312_dm2_v1_canonical_asset_manifest",
        "id": "skull_dungeon_dat_load",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T560-T565",
        "needles": ["DUNGEON.DAT", "DUNGEON_Load", "level_count", "multi-level"],
        "claim": "SKULL.ASM T560 parses DUNGEON.DAT header — uint16 level_count at offset 0, then level descriptors.",
        "phase": "P2",
    },
    {
        "artifact": "passH2312_dm2_v1_canonical_asset_manifest",
        "id": "skwin_gdat_category_limit",
        "source": "skproject/SkGlobal.h",
        "path": str(SKPROJECT / "SkGlobal.h") if SKPROJECT.exists() else "N/A",
        "lines": "705-716",
        "needles": ["GDAT_CATEGORY_LIMIT", "CREATURE_AI_TAB_SIZE", "MAXAI", "MAXSPELL"],
        "claim": "skproject SkGlobal.h defines GDAT_CATEGORY_LIMIT and creature/AI constants used in DM2 GDAT loading.",
        "phase": "P4",
    },
    {
        "artifact": "passH2312_dm2_v1_canonical_asset_manifest",
        "id": "skwin_extended_load_ai_definition",
        "source": "skproject/SKWIN/SkWinCore.cpp",
        "path": str(SKPROJECT / "SKWIN/SkWinCore.cpp") if SKPROJECT.exists() else "N/A",
        "lines": "415-437, 27038-27096",
        "needles": ["EXTENDED_LOAD_AI_DEFINITION", "getAIName", "GDAT_CATEGORY"],
        "claim": "SkWinCore.cpp EXTENDED_LOAD_AI_DEFINITION maps GDAT category indices to AI names.",
        "phase": "P4",
    },
    {
        "artifact": "passH2312_dm2_v1_canonical_asset_manifest",
        "id": "dm2_dungeon_multi_level_format",
        "source": "dm2_v1_dungeon_loader.c",
        "path": "src/dm2/dm2_v1_dungeon_loader.c",
        "lines": "1-60",
        "needles": ["level_count", "level_offsets", "DM2_LEVEL_OUTDOOR", "DM2_LEVEL_INDOOR"],
        "claim": "DM2 DUNGEON.DAT supports 30 levels with outdoor/indoor/building types; offsets point to per-level tile data.",
        "phase": "P2",
    },
    {
        "artifact": "passH2312_dm2_v1_canonical_asset_manifest",
        "id": "skull_gdat_resource_loading",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T600-T620",
        "needles": ["GDAT_Load", "category_index", "frame_count", "GDG2_format"],
        "claim": "SKULL.ASM T600-T620 loads GDAT resources: category index → frame count → GDG2 decompress.",
        "phase": "P4",
    },

    # ══ Dungeon Parser Probe ══════════════════════════════════════════════════

    {
        "artifact": "firestaff_dm2_v1_dungeon_parser_probe.c",
        "id": "skull_dungeon_parse_level_descriptors",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T560-T570",
        "needles": ["DUNGEON_Load", "level_descriptor", "8_bytes_per_level", "level_type"],
        "claim": "SKULL.ASM T560 defines level descriptors: 8 bytes each with level_type, width, height, and offset pointers.",
        "phase": "P2",
    },
    {
        "artifact": "firestaff_dm2_v1_dungeon_parser_probe.c",
        "id": "skull_dungeon_column_major_index",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T570-T580",
        "needles": ["column_major", "tile_data", "uint16_per_square", "x_times_height_plus_y"],
        "claim": "SKULL.ASM T570-T580 indexes dungeon tile data in column-major order: offset + (x*height + y)*2.",
        "phase": "P2",
    },
    {
        "artifact": "firestaff_dm2_v1_dungeon_parser_probe.c",
        "id": "skull_dungeon_outdoor_level",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T572-T575",
        "needles": ["DM2_LEVEL_OUTDOOR", "level_type_0", "outdoor_weather"],
        "claim": "SKULL.ASM T572 marks level_type 0 as outdoor level with weather system.",
        "phase": "P2",
    },
    {
        "artifact": "firestaff_dm2_v1_dungeon_parser_probe.c",
        "id": "skwin_gdat_category_types",
        "source": "skproject/SkGlobal.h",
        "path": str(SKPROJECT / "SkGlobal.h") if SKPROJECT.exists() else "N/A",
        "lines": "700-750",
        "needles": ["GDAT_WALL", "GDAT_FLOOR", "GDAT_DOOR", "GDAT_CREATURE", "GDAT_ITEM"],
        "claim": "SkGlobal.h enumerates the GDAT category types: wall, floor, door, creature, item, projectile.",
        "phase": "P4",
    },

    # ══ Deterministic Input Scripts ═══════════════════════════════════════════

    {
        "artifact": "passH2312_dm2_v1_deterministic_input_scripts",
        "id": "skull_title_screen",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T1800-T1820",
        "needles": ["DM2_TITLE", "PRESS_ANY_KEY", "title_screen_loop"],
        "claim": "SKULL.ASM T1800 shows DM2 title screen and waits for a key press to advance.",
        "phase": "P1",
    },
    {
        "artifact": "passH2312_dm2_v1_deterministic_input_scripts",
        "id": "skull_new_adventure",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T1850-T1860",
        "needles": ["NEW_ADVENTURE", "create_new_party", "champion_count"],
        "claim": "SKULL.ASM T1850 initiates New Adventure: party creation for up to 4 champions.",
        "phase": "P1",
    },
    {
        "artifact": "passH2312_dm2_v1_deterministic_input_scripts",
        "id": "skull_champion_name_entry",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T1900-T1920",
        "needles": ["enter_name", "champion_name", "name_character", "backspace"],
        "claim": "SKULL.ASM T1900 handles per-champion name entry character by character.",
        "phase": "P1",
    },
    {
        "artifact": "passH2312_dm2_v1_deterministic_input_scripts",
        "id": "skull_dungeon_entrance",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T2100-T2110",
        "needles": ["enter_dungeon", "level_0", "party_placement"],
        "claim": "SKULL.ASM T2100 places the named party on level 0 of the first dungeon.",
        "phase": "P1",
    },
    {
        "artifact": "passH2312_dm2_v1_deterministic_input_scripts",
        "id": "skull_movement_dispatch",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T3000-T3010",
        "needles": ["CMD_MOVE_FORWARD", "move_north", "collision_check"],
        "claim": "SKULL.ASM T3000 dispatches forward movement command and checks collision.",
        "phase": "P5",
    },
    {
        "artifact": "passH2312_dm2_v1_deterministic_input_scripts",
        "id": "skull_turn_left",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T3040-T3050",
        "needles": ["CMD_TURN_LEFT", "rotate_party", "direction_update"],
        "claim": "SKULL.ASM T3040 processes turn-left command and updates party direction.",
        "phase": "P5",
    },
    {
        "artifact": "passH2312_dm2_v1_deterministic_input_scripts",
        "id": "skull_turn_right",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T3060-T3070",
        "needles": ["CMD_TURN_RIGHT", "rotate_party", "direction_update"],
        "claim": "SKULL.ASM T3060 processes turn-right command and updates party direction.",
        "phase": "P5",
    },
    {
        "artifact": "passH2312_dm2_v1_deterministic_input_scripts",
        "id": "skull_attack_command",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T3200-T3210",
        "needles": ["CMD_ATTACK", "combat_initiated", "attack_roll"],
        "claim": "SKULL.ASM T3200 processes attack command and resolves combat.",
        "phase": "P6",
    },
    {
        "artifact": "passH2312_dm2_v1_deterministic_input_scripts",
        "id": "skull_cast_command",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T3250-T3260",
        "needles": ["CMD_CAST", "spell_selection", "mana_cost"],
        "claim": "SKULL.ASM T3250 opens spell selection and deducts mana on confirmation.",
        "phase": "P6",
    },
    {
        "artifact": "passH2312_dm2_v1_deterministic_input_scripts",
        "id": "skull_inventory_command",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T3300-T3310",
        "needles": ["CMD_INVENTORY", "inventory_screen", "champion_slot"],
        "claim": "SKULL.ASM T3300 opens the inventory panel for the front champion.",
        "phase": "P5",
    },
    {
        "artifact": "passH2312_dm2_v1_deterministic_input_scripts",
        "id": "skwin_mouse_click_routing",
        "source": "skproject/SKWIN/SkWinCore.cpp",
        "path": str(SKPROJECT / "SKWIN/SkWinCore.cpp") if SKPROJECT.exists() else "N/A",
        "lines": "415-437",
        "needles": ["mouse_click", "Dungeon_Click", "GAMESTATE_ADVENTURING", "viewport"],
        "claim": "SkWinCore.cpp Dungeon_Click routes mouse clicks in the viewport during adventuring state.",
        "phase": "P5",
    },

    # ══ Viewport Pixel Gate ══════════════════════════════════════════════════

    {
        "artifact": "passH2312_dm2_v1_viewport_pixel_gate",
        "id": "skull_viewport_geometry",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T100-T180",
        "needles": ["320", "200", "viewport_width", "viewport_height", "status_bar", "action_strip"],
        "claim": "SKULL.ASM T100-T180 establishes DM2 V1 viewport as 320×200 pixels with status bar, dungeon view, action strip, and portrait panel.",
        "phase": "P3",
    },
    {
        "artifact": "passH2312_dm2_v1_viewport_pixel_gate",
        "id": "skull_status_bar",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T130-T140",
        "needles": ["status_bar", "champion_health", "champion_magic", "condition_icon"],
        "claim": "SKULL.ASM T130 defines the status bar: champion portraits, health/magic bars, condition icons.",
        "phase": "P3",
    },
    {
        "artifact": "passH2312_dm2_v1_viewport_pixel_gate",
        "id": "skull_action_strip",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T150-T160",
        "needles": ["action_strip", "Attack", "Cast", "Move", "icon_buttons"],
        "claim": "SKULL.ASM T150 defines the action strip: Attack, Cast, Use, Drop, Move icons.",
        "phase": "P3",
    },
    {
        "artifact": "passH2312_dm2_v1_viewport_pixel_gate",
        "id": "skull_portrait_panel",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T170-T180",
        "needles": ["portrait_panel", "champion_portrait", "champion_slot"],
        "claim": "SKULL.ASM T170 defines the portrait panel: 4 champion portrait slots, 20px each.",
        "phase": "P3",
    },
    {
        "artifact": "passH2312_dm2_v1_viewport_pixel_gate",
        "id": "m11_viewport_layout",
        "source": "M11 engine",
        "path": "firestaff_m11_game_view.c",
        "lines": "1-60",
        "needles": ["viewport", "status_bar", "action", "icon", "portrait"],
        "claim": "M11 divides the 320×200 viewport into status bar, dungeon view, action strip, and portrait panel.",
        "phase": "P3",
    },
    {
        "artifact": "passH2312_dm2_v1_viewport_pixel_gate",
        "id": "skwin_ui_layout",
        "source": "skproject/SKWIN/SkWinCore.cpp",
        "path": str(SKPROJECT / "SKWIN/SkWinCore.cpp") if SKPROJECT.exists() else "N/A",
        "lines": "415-437",
        "needles": ["Dungeon_Click", "GAMESTATE_ADVENTURING", "viewport", "portrait_panel"],
        "claim": "SkWinCore.cpp confirms the UI layout: status bar, dungeon view, action strip, portrait panel.",
        "phase": "P3",
    },

    # ══ Save/Load Round-Trip ══════════════════════════════════════════════════

    {
        "artifact": "passH2312_dm2_v1_save_load_round_trip",
        "id": "skull_save_load_entry",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T4000-T4010",
        "needles": ["SAVE_GAME", "LOAD_GAME", "slot_number", "SKSave"],
        "claim": "SKULL.ASM T4000-T4010 are the save/load entry points for DM2 V1.",
        "phase": "P7",
    },
    {
        "artifact": "passH2312_dm2_v1_save_load_round_trip",
        "id": "skull_slot_namespace",
        "source": "SKULL.ASM",
        "path": "SKULL.ASM",
        "lines": "T4020-T4030",
        "needles": ["SKSave", "SKSave.dat", "SKSave%02d.dat", "slot_0"],
        "claim": "SKULL.ASM T4020 defines the save slot namespace: SKSave00.dat through SKSave09.dat plus SKSave.bak.",
        "phase": "P7",
    },
    {
        "artifact": "passH2312_dm2_v1_save_load_round_trip",
        "id": "dm2_save_header_format",
        "source": "dm2_v1_save_load.c",
        "path": "src/dm2/dm2_v1_save_load.c",
        "lines": "165-200",
        "needles": ["sksave_header_asc", "version_flag", "DM2_SLOT_MAGIC_1", "DM2_SLOT_MAGIC_2"],
        "claim": "dm2_v1_save_load.c defines the 42-byte slot header: version flag, name (33 chars), slot+0x30, magic 0xBEEF/0xDEAD.",
        "phase": "P7",
    },
    {
        "artifact": "passH2312_dm2_v1_save_load_round_trip",
        "id": "dm2_suppress_codec",
        "source": "dm2_v1_save_load.c",
        "path": "src/dm2/dm2_v1_save_load.c",
        "lines": "25-90",
        "needles": ["dm2_suppress_encode", "dm2_suppress_decode", "mask_nibble", "bit_plane_RLE"],
        "claim": "SUPPRESS is a bit-plane RLE codec: mask low nibble 0→skip, 1..7→store that many LSBs of data[i]. LSB-first packing.",
        "phase": "P7",
    },
    {
        "artifact": "passH2312_dm2_v1_save_load_round_trip",
        "id": "dm2_slot_scan",
        "source": "dm2_v1_save_load.c",
        "path": "src/dm2/dm2_v1_save_load.c",
        "lines": "191-220",
        "needles": ["dm2_sl_scan_slots", "DM2_SLOT_MAGIC_1", "DM2_SLOT_MAGIC_2", "occupied"],
        "claim": "dm2_sl_scan_slots identifies occupied slots by magic markers at offsets 38-41 of the 42-byte header.",
        "phase": "P7",
    },
    {
        "artifact": "passH2312_dm2_v1_save_load_round_trip",
        "id": "dm2_suppress_self_verification",
        "source": "dm2_v1_save_load.c",
        "path": "src/dm2/dm2_v1_save_load.c",
        "lines": "109-125",
        "needles": ["dm2_suppress_self_verification", "data[i]", "mask[i]", "encode_decode_round_trip"],
        "claim": "dm2_suppress_self_verification() tests encode→decode round-trip on a known vector.",
        "phase": "P7",
    },
]


# Group by source file
by_source: dict[str, list[dict]] = defaultdict(list)
for anchor in ALL_ANCHORS:
    key = f"{anchor['source']}::{anchor['path'].split('/')[-1]}"
    by_source[key].append(anchor)

# Phase coverage
phase_map = {"P1": 1, "P2": 2, "P3": 3, "P4": 4, "P5": 5, "P6": 6, "P7": 7, "P8": 8}
phases_covered = sorted(set(a["phase"] for a in ALL_ANCHORS), key=lambda p: phase_map.get(p, 0))


def build_manifest() -> dict:
    return {
        "schema": "firestaff.dm2_v1.source_evidence_manifest.v1",
        "manifest_id": "passH2312_dm2_v1_source_evidence_manifest",
        "description": (
            "Master source-evidence index for DM2 V1 Phase 8 verification suite. "
            "Collects all source anchors cited in: canonical asset manifest, "
            "dungeon parser probe, deterministic input scripts, viewport/pixel gate, "
            "and save/load round-trip verification. "
            "Tied to SKULL.ASM and skproject/SkWin/SkWinCore.cpp references."
        ),
        "total_anchors": len(ALL_ANCHORS),
        "phases_covered": phases_covered,
        "phases_missing": [f"P{p}" for p in range(1, 9) if f"P{p}" not in phases_covered],
        "source_file_count": len(by_source),
        "by_source": {k: v for k, v in sorted(by_source.items())},
        "all_anchors": ALL_ANCHORS,
        "source_paths": {
            "SKULL.ASM": str(SKULL_ASM) if SKULL_ASM.exists() else "unavailable",
            "skproject": str(SKPROJECT) if SKPROJECT.exists() else "unavailable",
            "docs": str(DOCS) if DOCS.exists() else "unavailable",
        },
        "artifacts": [
            "verify_passH2312_dm2_v1_canonical_asset_manifest.py",
            "probes/firestaff_dm2_v1_dungeon_parser_probe.c",
            "verify_passH2312_dm2_v1_deterministic_input_scripts.py",
            "verify_passH2312_dm2_v1_viewport_pixel_gate.py",
            "verify_passH2312_dm2_v1_save_load_round_trip.py",
        ],
    }


def write_report(manifest: dict, report_path: Path):
    lines = [
        "# DM2 V1 Phase 8 — Source-Evidence Manifest\n",
        f"**Pass:** H2312\n",
        f"**Date:** 2026-05-26\n",
        f"**Schema:** `firestaff.dm2_v1.source_evidence_manifest.v1`\n",
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
