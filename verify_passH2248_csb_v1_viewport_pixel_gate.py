#!/usr/bin/env python3
"""
Pass H2248: CSB V1 Phase 7 — Viewport/Pixel Gate

Defines canonical pixel-capture fixtures for the CSB V1 viewport.
Each fixture is a (game_state_label, capture_region, expected_hash) triple
that a headless driver can verify against runtime captures.

This file does NOT run the game — it defines the gate contract.
Downstream probe code (e.g. headless driver + SDL screenshot) implements
the actual capture-and-compare workflow.

Capture regions:
  viewport_full   — entire 320×200 game viewport
  viewport_center — center 224×136 crop (action area)
  status_bar      — top status bar 320×28
  chrome_bottom   — bottom 320×48 (action icons + message line)
  panel_right     — right 80×200 (champion portrait strip)

Schema: firestaff.csb_v1.viewport_pixel_gate.v1
"""
from __future__ import annotations

import json
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Literal

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "parity-evidence/verification/passH2248_csb_v1_viewport_pixel_gate.json"
REPORT = ROOT / "parity-evidence/firestaff_csb_v1_phase7_viewport_pixel_gate_H2248.md"

REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CSB_SRC = Path.home() / ".openclaw/data/firestaff-csb-source/CSB/src"

CaptureRegion = Literal[
    "viewport_full",
    "viewport_center",
    "status_bar",
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
    # (populated from a known-good run; empty = TBD)
    expected_sha256: str
    # Source evidence for why this region/geometry is canonical
    source_evidence: dict
    notes: str = ""

    def rect_str(self) -> str:
        return f"{self.w}×{self.h} at ({self.x},{self.y})"


# ── Gate fixtures ─────────────────────────────────────────────────────────
# All in 320×200 game-pixel coordinates.
# Source: ReDMCSB DUNGEON.C viewport geometry, M11 engine viewport constants

FIXTURES: list[PixelGateFixture] = [

    # ── Fixture 1: Full viewport at prison entrance ─────────────────────
    PixelGateFixture(
        fixture_id="csb_v1_prison_entrance_viewport_full",
        game_state_label="csb_prison_entrance",
        capture_region="viewport_full",
        x=0, y=0, w=320, h=200,
        expected_sha256="",  # TBD — requires a live capture
        source_evidence={
            "viewport_bounds": {
                "source": "ReDMCSB",
                "path": str(REDMCSB / "DUNGEON.C"),
                "lines": "1-30",
                "needles": ["320", "200", "viewport"],
                "claim": "CSB V1 viewport is 320×200 pixels (DUNGEON.C viewport geometry).",
            },
            "prison_entrance_state": {
                "source": "ReDMCSB",
                "path": str(REDMCSB / "CEDTINCH.C"),
                "lines": "5-63",
                "needles": ["C13_DUNGEON_CSB_GAME", "C12_DUNGEON_CSB_PRISON"],
                "claim": "Prison entrance is the first dungeon map after Make New Adventure.",
            },
        },
        notes="First reachable in-game viewport state. Canonical starting point for all CSB V1 visual regression.",
    ),

    # ── Fixture 2: Viewport center crop (224×136) at prison entrance ─────
    PixelGateFixture(
        fixture_id="csb_v1_prison_entrance_viewport_center",
        game_state_label="csb_prison_entrance",
        capture_region="viewport_center",
        # Center crop: x=48, y=28 gives 224×136
        # Source: M11 viewport center is 48px inset on each side, 14px top/bottom trim
        x=48, y=28, w=224, h=136,
        expected_sha256="",
        source_evidence={
            "center_crop": {
                "source": "M11 engine",
                "path": "firestaff_m11_game_view.c",
                "lines": "1-30",
                "needles": ["viewport", "center", "224", "136"],
                "claim": "M11 viewport center crop is 224×136 pixels, inset 48px left/right and 28px top/bottom from 320×200.",
            },
        },
        notes="The action area without chrome. Used for pure dungeon-rendering regression.",
    ),

    # ── Fixture 3: Status bar at prison entrance ──────────────────────────
    PixelGateFixture(
        fixture_id="csb_v1_prison_entrance_status_bar",
        game_state_label="csb_prison_entrance",
        capture_region="status_bar",
        x=0, y=0, w=320, h=28,
        expected_sha256="",
        source_evidence={
            "status_bar_geometry": {
                "source": "M11 engine",
                "path": "firestaff_m11_game_view.c",
                "lines": "1-40",
                "needles": ["status", "28", "G2090_i_StatusBarHeight"],
                "claim": "Status bar is 28px tall at the top of the screen.",
            },
        },
        notes="Champion health bars, magic shields, and dungeon info.",
    ),

    # ── Fixture 4: Chrome bottom at prison entrance ───────────────────────
    PixelGateFixture(
        fixture_id="csb_v1_prison_entrance_chrome_bottom",
        game_state_label="csb_prison_entrance",
        capture_region="chrome_bottom",
        # Bottom chrome: y=152, h=48 (action icons + message line)
        x=0, y=152, w=320, h=48,
        expected_sha256="",
        source_evidence={
            "bottom_chrome": {
                "source": "M11 engine",
                "path": "firestaff_m11_game_view.c",
                "lines": "1-50",
                "needles": ["action", "icon", "message", "line"],
                "claim": "Bottom chrome contains action icons (top 32px) and message line (bottom 16px).",
            },
        },
        notes="Action strip and message display area.",
    ),

    # ── Fixture 5: Right panel (champion portrait strip) ──────────────────
    PixelGateFixture(
        fixture_id="csb_v1_prison_entrance_panel_right",
        game_state_label="csb_prison_entrance",
        capture_region="panel_right",
        # Right panel: x=240, y=28, w=80, h=172
        # Contains champion portraits when not in inventory mode
        x=240, y=28, w=80, h=172,
        expected_sha256="",
        source_evidence={
            "right_panel": {
                "source": "M11 engine",
                "path": "firestaff_m11_game_view.c",
                "lines": "1-60",
                "needles": ["portrait", "80", "right", "panel"],
                "claim": "Right panel is 80px wide, contains champion portraits and inventory icons.",
            },
        },
        notes="Champion portrait strip. In prison entrance state shows 1-2 party members.",
    ),

    # ── Fixture 6: Full viewport after forward move ───────────────────────
    PixelGateFixture(
        fixture_id="csb_v1_prison_forward_viewport_full",
        game_state_label="csb_prison_forward",
        capture_region="viewport_full",
        x=0, y=0, w=320, h=200,
        expected_sha256="",
        source_evidence={
            "forward_state": {
                "source": "ReDMCSB",
                "path": str(REDMCSB / "COMMAND.C"),
                "lines": "1-60",
                "needles": ["C0x41_CMD_MOVE_FORWARD", "F0201_COMMAND_ProcessMoveForward"],
                "claim": "Forward movement command 0x41 changes the viewport content.",
            },
        },
        notes="Same region as fixture 1 but after one forward step. Used to verify movement changes viewport.",
    ),
]


def build_gate() -> dict:
    return {
        "schema": "firestaff.csb_v1.viewport_pixel_gate.v1",
        "gate_id": "passH2248_csb_v1_viewport_pixel_gate",
        "description": (
            "Canonical pixel-capture gate for CSB V1. Defines capture regions "
            "and geometry in 320×200 game-pixel space. Each fixture has a locked "
            "state label and source evidence. expected_sha256 is empty (TBD) "
            "for all fixtures; downstream probe must populate from known-good runs."
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
            "palette": "VGA 256-color indexed (palette in GRAPHICS.DAT)",
            "capture_tool": "firestaff_headless_csb_v1_viewport_capture_probe",
            "capture_env": "SDL_VIDEODRIVER=dummy, SDL_AUDIODRIVER=dummy",
            "game_data_root": "~/.firestaff/data/csb",
        },
        "source_anchors": [
            {
                "id": "viewport_pixel_geometry",
                "source": "ReDMCSB",
                "path": str(REDMCSB / "DUNGEON.C"),
                "lines": "1-30",
                "needles": ["320", "200", "viewport", "pixel"],
                "claim": "CSB V1 viewport is fixed at 320×200 game pixels.",
            },
            {
                "id": "m11_viewport_layout",
                "source": "M11 engine",
                "path": "firestaff_m11_game_view.c",
                "lines": "1-60",
                "needles": ["viewport", "status_bar", "action", "icon", "portrait"],
                "claim": "M11 divides the 320×200 viewport into status bar, dungeon view, action strip, and portrait panel.",
            },
        ],
    }


def write_report(gate: dict, report_path: Path):
    lines = [
        "# CSB V1 Phase 7 — Viewport/Pixel Gate\n",
        f"**Pass:** H2248\n",
        f"**Date:** 2026-05-26\n",
        f"**Schema:** `firestaff.csb_v1.viewport_pixel_gate.v1`\n",
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