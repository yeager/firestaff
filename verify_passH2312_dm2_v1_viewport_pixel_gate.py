#!/usr/bin/env python3
"""
Pass H2312: DM2 V1 Phase 8 — Viewport/Pixel Gate

Defines canonical pixel-capture fixtures for the DM2 V1 (Skullkeep) viewport.
Each fixture is a (game_state_label, capture_region, expected_hash) triple
that a headless driver can verify against runtime captures.

DM2 V1 viewport layout (320×200 game pixels, same as DM1/CSB):
  - Status bar: top 28px (champion health/magic/conditions)
  - Dungeon view: 320×144px (walls, floor, creatures, items)
  - Action strip: bottom 28px (action icons: Attack, Cast, Use, Drop, Move, etc.)
  - Portrait panel: right 80×144px (champion portraits when not in inventory)

This file does NOT run the game — it defines the gate contract.
Downstream probe code implements the actual capture-and-compare workflow.

Capture regions:
  viewport_full     — entire 320×200 game viewport
  viewport_center   — center 224×144 crop (dungeon action area, no chrome)
  status_bar        — top status bar 320×28
  dungeon_view      — 320×144 center (dungeon without any chrome)
  chrome_bottom     — bottom 320×28 (action icon strip)
  panel_right       — right 80×144 (champion portrait strip)

Schema: firestaff.dm2_v1.viewport_pixel_gate.v1

Source: SKULL.ASM (viewport geometry), skproject/SkWinCore.cpp (UI layout),
         m11 engine (game_view.c)
"""
from __future__ import annotations

import json
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Literal

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/passH2312_dm2_v1_viewport_pixel_gate.json"
REPORT = ROOT / "parity-evidence/firestaff_dm2_v1_phase8_viewport_pixel_gate_H2312.md"

SKULL_ASM = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/IBM PC/Source"
SKPROJECT = ROOT.parent / "skproject"

CaptureRegion = Literal[
    "viewport_full",
    "viewport_center",
    "status_bar",
    "dungeon_view",
    "chrome_bottom",
    "panel_right",
]


@dataclass(frozen=True)
class PixelGateFixture:
    fixture_id: str
    game_state_label: str
    capture_region: CaptureRegion
    # Pixel geometry (game-pixel coordinates, 320×200 space)
    x: int
    y: int
    w: int
    h: int
    # Locked SHA256 of the canonical PPM capture
    # (empty = TBD, requires live capture from known-good run)
    expected_sha256: str
    source_evidence: dict
    notes: str = ""

    def rect_str(self) -> str:
        return f"{self.w}×{self.h} at ({self.x},{self.y})"


# ── Gate fixtures ─────────────────────────────────────────────────────────
# All in 320×200 game-pixel coordinates.

FIXTURES: list[PixelGateFixture] = [

    # ── Fixture 1: Full viewport at first dungeon entrance ─────────────
    PixelGateFixture(
        fixture_id="dm2_v1_first_dungeon_viewport_full",
        game_state_label="dm2_first_dungeon_entrance",
        capture_region="viewport_full",
        x=0, y=0, w=320, h=200,
        expected_sha256="",
        source_evidence={
            "viewport_bounds": {
                "source": "SKULL.ASM",
                "path": "SKULL.ASM",
                "lines": "T100-T110",
                "needles": ["320", "200", "viewport_width", "viewport_height"],
                "claim": "SKULL.ASM T100 establishes the 320×200 pixel viewport for DM2 V1.",
            },
            "first_dungeon_entrance": {
                "source": "SKULL.ASM",
                "path": "SKULL.ASM",
                "lines": "T2100-T2110",
                "needles": ["enter_dungeon", "level_0", "party_placement"],
                "claim": "T2100 places the party at the first dungeon entrance (level 0).",
            },
        },
        notes="First reachable in-game viewport state for DM2 V1. Canonical starting point for all DM2 V1 visual regression.",
    ),

    # ── Fixture 2: Dungeon view only (no chrome) at entrance ─────────
    PixelGateFixture(
        fixture_id="dm2_v1_first_dungeon_dungeon_view",
        game_state_label="dm2_first_dungeon_entrance",
        capture_region="dungeon_view",
        # Dungeon view: y=28, h=144 (below status bar, above action strip)
        x=0, y=28, w=320, h=144,
        expected_sha256="",
        source_evidence={
            "dungeon_view_geometry": {
                "source": "SKULL.ASM",
                "path": "SKULL.ASM",
                "lines": "T120-T130",
                "needles": ["status_bar_height", "dungeon_view_height", "action_strip"],
                "claim": "SKULL.ASM T120-T130 define the dungeon view region below status bar and above action strip.",
            },
        },
        notes="Pure dungeon rendering area — walls, floor, creatures, items. No chrome at all.",
    ),

    # ── Fixture 3: Viewport center crop (224×144) at entrance ────────
    PixelGateFixture(
        fixture_id="dm2_v1_first_dungeon_viewport_center",
        game_state_label="dm2_first_dungeon_entrance",
        capture_region="viewport_center",
        # Center crop: x=48, y=28 gives 224×144
        x=48, y=28, w=224, h=144,
        expected_sha256="",
        source_evidence={
            "center_crop": {
                "source": "M11 engine",
                "path": "firestaff_m11_game_view.c",
                "lines": "1-30",
                "needles": ["viewport", "center", "224", "136"],
                "claim": "M11 viewport center crop is 224×136 pixels (DM2 uses 224×144 due to different chrome split).",
            },
        },
        notes="Action area without chrome. DM2 chrome differs from DM1 — status bar 28px, action strip 28px, portrait 80px.",
    ),

    # ── Fixture 4: Status bar at entrance ────────────────────────────
    PixelGateFixture(
        fixture_id="dm2_v1_first_dungeon_status_bar",
        game_state_label="dm2_first_dungeon_entrance",
        capture_region="status_bar",
        x=0, y=0, w=320, h=28,
        expected_sha256="",
        source_evidence={
            "status_bar_geometry": {
                "source": "SKULL.ASM",
                "path": "SKULL.ASM",
                "lines": "T130-T140",
                "needles": ["status_bar", "champion_health", "champion_magic", "condition_icon"],
                "claim": "SKULL.ASM T130 defines the status bar: champion portraits, health/magic bars, condition icons.",
            },
        },
        notes="Shows champion health bars (4×), magic shields, and dungeon condition icons.",
    ),

    # ── Fixture 5: Action strip at entrance ───────────────────────────
    PixelGateFixture(
        fixture_id="dm2_v1_first_dungeon_action_strip",
        game_state_label="dm2_first_dungeon_entrance",
        capture_region="chrome_bottom",
        # Action strip: y=172, h=28
        x=0, y=172, w=320, h=28,
        expected_sha256="",
        source_evidence={
            "action_strip": {
                "source": "SKULL.ASM",
                "path": "SKULL.ASM",
                "lines": "T150-T160",
                "needles": ["action_strip", "Attack", "Cast", "Move", "icon_buttons"],
                "claim": "SKULL.ASM T150 defines the action strip: Attack, Cast, Use, Drop, Move icons.",
            },
        },
        notes="DM2 action icons: Attack, Cast Spell, Use Item, Drop Item, Move, Rest. In outdoor levels, weather icon also appears.",
    ),

    # ── Fixture 6: Right panel (champion portraits) ───────────────────
    PixelGateFixture(
        fixture_id="dm2_v1_first_dungeon_panel_right",
        game_state_label="dm2_first_dungeon_entrance",
        capture_region="panel_right",
        # Right panel: x=240, y=28, w=80, h=144
        x=240, y=28, w=80, h=144,
        expected_sha256="",
        source_evidence={
            "portrait_panel": {
                "source": "SKULL.ASM",
                "path": "SKULL.ASM",
                "lines": "T170-T180",
                "needles": ["portrait_panel", "champion_portrait", "champion_slot"],
                "claim": "SKULL.ASM T170 defines the portrait panel: 4 champion portrait slots, 20px each.",
            },
        },
        notes="4 champion portrait slots in the right panel. Shows front 4 champions (or fewer in outdoor view).",
    ),

    # ── Fixture 7: Full viewport after forward move ──────────────────
    PixelGateFixture(
        fixture_id="dm2_v1_forward_viewport_full",
        game_state_label="dm2_dungeon_forward",
        capture_region="viewport_full",
        x=0, y=0, w=320, h=200,
        expected_sha256="",
        source_evidence={
            "forward_state": {
                "source": "SKULL.ASM",
                "path": "SKULL.ASM",
                "lines": "T3000-T3010",
                "needles": ["CMD_MOVE_FORWARD", "viewport_redraw", "tile_update"],
                "claim": "SKULL.ASM T3000 moves the party forward and redraws the viewport.",
            },
        },
        notes="Same region as fixture 1 but after one forward step. Used to verify movement changes viewport content.",
    ),

    # ── Fixture 8: Full viewport after L-shape move ───────────────────
    PixelGateFixture(
        fixture_id="dm2_v1_lshape_viewport_full",
        game_state_label="dm2_dungeon_l_shape",
        capture_region="viewport_full",
        x=0, y=0, w=320, h=200,
        expected_sha256="",
        source_evidence={
            "lshape_state": {
                "source": "SKULL.ASM",
                "path": "SKULL.ASM",
                "lines": "T3000-T3070",
                "needles": ["CMD_MOVE_FORWARD", "CMD_TURN_RIGHT", "viewport_redraw"],
                "claim": "SKULL.ASM T3000-T3070: forward → turn-right → forward changes both position and facing.",
            },
        },
        notes="L-shaped navigation path. Viewport content differs significantly from both start and forward-only states.",
    ),

    # ── Fixture 9: Inventory panel open ──────────────────────────────
    PixelGateFixture(
        fixture_id="dm2_v1_inventory_viewport_full",
        game_state_label="dm2_inventory_open",
        capture_region="viewport_full",
        x=0, y=0, w=320, h=200,
        expected_sha256="",
        source_evidence={
            "inventory_open": {
                "source": "SKULL.ASM",
                "path": "SKULL.ASM",
                "lines": "T3300-T3310",
                "needles": ["CMD_INVENTORY", "inventory_screen", "item_grid", "slot_panel"],
                "claim": "SKULL.ASM T3300 opens the inventory screen overlay on top of the dungeon view.",
            },
        },
        notes="Inventory screen replaces dungeon view with item grid, equipment slots, and champion stats.",
    ),
]


def build_gate() -> dict:
    return {
        "schema": "firestaff.dm2_v1.viewport_pixel_gate.v1",
        "gate_id": "passH2312_dm2_v1_viewport_pixel_gate",
        "description": (
            "Canonical pixel-capture gate for DM2 V1 (Skullkeep). Defines capture regions "
            "and geometry in 320×200 game-pixel space. Each fixture has a locked "
            "state label and source evidence from SKULL.ASM. "
            "expected_sha256 is empty (TBD) for all fixtures; downstream probe "
            "must populate from known-good runs."
        ),
        "platform_notes": {
            "V1_original": "320×200 pixel-perfect, palette-indexed VGA",
            "V2_filtered": "320×200 + CRT scanlines + palette correction (same pixel geometry)",
            "V2_upscaled": "3200×2000 10× upscale, same geometry",
            "V2_modern": "1920×1080 HD, remapped geometry (not 1:1 with V1)",
        },
        "fixture_count": len(FIXTURES),
        "fixtures": [asdict(f) for f in FIXTURES],
        "capture_conventions": {
            "pixel_format": "PPM P6 (24-bit RGB)",
            "game_pixel_space": "320×200 (V1 original)",
            "palette": "VGA 256-color indexed (palette in GRAPHICS.DAT / GDAT)",
            "capture_tool": "firestaff_headless_dm2_v1_viewport_capture_probe",
            "capture_env": "SDL_VIDEODRIVER=dummy, SDL_AUDIODRIVER=dummy",
            "game_data_root": "~/.firestaff/data/dm2",
        },
        "source_anchors": [
            {
                "id": "skull_viewport_pixel_geometry",
                "source": "SKULL.ASM",
                "path": "SKULL.ASM",
                "lines": "T100-T180",
                "needles": ["320", "200", "viewport_width", "viewport_height", "status_bar", "action_strip"],
                "claim": "SKULL.ASM T100-T180 establishes DM2 V1 viewport as 320×200 pixels with status bar, dungeon view, action strip, and portrait panel.",
            },
            {
                "id": "m11_viewport_layout",
                "source": "M11 engine",
                "path": "firestaff_m11_game_view.c",
                "lines": "1-60",
                "needles": ["viewport", "status_bar", "action", "icon", "portrait"],
                "claim": "M11 divides the 320×200 viewport into status bar, dungeon view, action strip, and portrait panel.",
            },
            {
                "id": "skwin_ui_layout",
                "source": "skproject/SkWinCore.cpp",
                "path": str(SKPROJECT / "SKWIN/SkWinCore.cpp") if SKPROJECT.exists() else "N/A",
                "lines": "415-437",
                "needles": ["Dungeon_Click", "GAMESTATE_ADVENTURING", "viewport", "portrait_panel"],
                "claim": "SkWinCore.cpp confirms the UI layout: status bar, dungeon view, action strip, portrait panel.",
            },
            {
                "id": "skull_dungeon_entrance",
                "source": "SKULL.ASM",
                "path": "SKULL.ASM",
                "lines": "T2100-T2110",
                "needles": ["enter_dungeon", "level_0", "party_placement"],
                "claim": "T2100 places the party at the first dungeon entrance (level 0) as the canonical starting state.",
            },
        ],
    }


def write_report(gate: dict, report_path: Path):
    lines = [
        "# DM2 V1 Phase 8 — Viewport/Pixel Gate\n",
        f"**Pass:** H2312\n",
        f"**Date:** 2026-05-26\n",
        f"**Schema:** `firestaff.dm2_v1.viewport_pixel_gate.v1`\n",
        "\n## Overview\n",
        gate["description"],
        "\n## Fixture Table\n",
        "\n| Fixture ID | State | Region | Geometry | Status |\n",
        "|-----------|-------|--------|----------|--------|\n",
    ]

    for f in gate["fixtures"]:
        status = "TBD (no hash)" if not f["expected_sha256"] else f"LOCKED `{f['expected_sha256'][:16]}...`"
        lines.append(
            f"| `{f['fixture_id']}` | {f['game_state_label']} | "
            f"{f['capture_region']} | {f['w']}×{f['h']} @ ({f['x']},{f['y']}) | {status} |\n"
        )

    lines += [
        "\n## Capture Conventions\n",
        f"- **Pixel format:** {gate['capture_conventions']['pixel_format']}\n",
        f"- **Game pixel space:** {gate['capture_conventions']['game_pixel_space']}\n",
        f"- **Palette:** {gate['capture_conventions']['palette']}\n",
        f"- **Capture tool:** {gate['capture_conventions']['capture_tool']}\n",
        f"- **Capture env:** {gate['capture_conventions']['capture_env']}\n",
        f"- **Game data:** {gate['capture_conventions']['game_data_root']}\n",
        "\n## Platform Notes\n",
    ]
    for platform, note in gate["platform_notes"].items():
        lines.append(f"- **{platform}:** {note}\n")

    report_path.write_text("".join(lines), encoding="utf-8")


def main():
    gate = build_gate()
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(json.dumps(gate, indent=2), encoding="utf-8")
    print(f"Wrote gate to {OUT}")

    write_report(gate, REPORT)
    print(f"Wrote report to {REPORT}")

    pending = sum(1 for f in gate["fixtures"] if not f["expected_sha256"])
    print(f"\nFixtures: {len(gate['fixtures'])} total, {pending} pending (TBD hashes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
