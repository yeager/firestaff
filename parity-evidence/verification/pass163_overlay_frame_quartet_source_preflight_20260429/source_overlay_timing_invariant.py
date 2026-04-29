#!/usr/bin/env python3
"""Source-first overlay timing/geometry invariant for DM1 V1 frame-quartet work.

This does not assert pixel parity. It verifies the ReDMCSB source anchors that
must be true before future original/Firestaff quartet frames can be accepted:
viewport crops are 224x136 at screen y=33, PC VGA viewport blits apply the
middle-screen palette offset, and title/entrance/gameplay handoffs are
vertical-blank/timer gated rather than arbitrary screenshot cadence.
"""
from __future__ import annotations

import json
import re
from pathlib import Path

SRC = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
REPO = Path("/home/trv2/work/firestaff")
OUT = REPO / "parity-evidence/verification/pass163_overlay_frame_quartet_source_preflight_20260429/source_overlay_timing_invariant.json"

CHECKS = [
    {
        "name": "title_zoom_waits_and_bug071",
        "file": "TITLE.C",
        "range": [201, 251],
        "patterns": [
            r"for \(L1380_i_Counter = 0; L1380_i_Counter < 18; L1380_i_Counter\+\+\)",
            r"M526_WaitVerticalBlank\(\)",
            r"Delay\(25L\)",
            r"BUG0_71 Some timings are too short",
        ],
    },
    {
        "name": "entrance_door_steps_wait_vblank",
        "file": "ENTRANCE.C",
        "range": [147, 239],
        "patterns": [
            r"do \{",
            r"F0132_VIDEO_Blit\(G0296_puc_Bitmap_Viewport",
            r"M768_BOX_LEFT\(G0008_ai_Graphic562_Box_Entrance_OpeningDoorRight\) \+= 4",
            r"M526_WaitVerticalBlank\(\).*BUG0_71",
        ],
    },
    {
        "name": "entrance_wait_loop_to_open_doors",
        "file": "ENTRANCE.C",
        "range": [850, 943],
        "patterns": [
            r"G0298_B_NewGame = C099_MODE_WAITING_ON_ENTRANCE",
            r"M526_WaitVerticalBlank\(\)",
            r"F0380_COMMAND_ProcessQueue_CPSC\(\)",
            r"F0438_STARTEND_OpenEntranceDoors\(\)",
        ],
    },
    {
        "name": "gameplay_viewport_request_wait",
        "file": "DRAWVIEW.C",
        "range": [709, 723],
        "patterns": [
            r"void F0097_DUNGEONVIEW_DrawViewport",
            r"G0324_B_DrawViewportRequested = C1_TRUE",
            r"M526_WaitVerticalBlank\(\).*viewport is on screen when the function returns",
        ],
    },
    {
        "name": "pc_vga_blitviewport_224_palette_offset",
        "file": "VIDEODRV.C",
        "range": [3566, 3580],
        "patterns": [
            r"FUNC_DEF void F8161_VIDRV_09_BlitViewPort",
            r"G8177_c_ViewportColorIndexOffset = 0x10",
            r"F8151_VIDRV_02_Blit\(P2535_puc_Bitmap, NULL, P2536_pi_Box, 0, 0, 224, 320",
            r"G8177_c_ViewportColorIndexOffset = 0",
        ],
    },
    {
        "name": "amiga_viewport_origin_and_224x136_plane_copy",
        "file": "VIEWPORT.C",
        "range": [20, 96],
        "patterns": [
            r"M091_BITPLANE_SIZE\(224, 136\)",
            r"P0986_puc_Bitmap \+ M091_BITPLANE_SIZE\(320, 33\)",
            r"custom.bltdmod = M091_BITPLANE_SIZE\(320 - 224, 1\)",
            r"custom.bltsize = M092_BLITSIZE\(224 / 16, 136\)",
        ],
    },
]

FIRESTAFF_CHECKS = [
    {
        "name": "firestaff_viewport_constants_match_source",
        "file": REPO / "m11_game_view.c",
        "patterns": [
            r"M11_DM1_VIEWPORT_X = 0",
            r"M11_DM1_VIEWPORT_Y = 33",
            r"M11_DM1_VIEWPORT_W = 224",
            r"M11_DM1_VIEWPORT_H = 136",
        ],
    },
    {
        "name": "original_capture_script_crops_source_viewport_rect",
        "file": REPO / "scripts/dosbox_dm1_original_viewport_reference_capture.sh",
        "patterns": [r"-crop 224x136\+0\+33"],
    },
]


def line_window(path: Path, start: int, end: int) -> str:
    lines = path.read_text(errors="replace").splitlines()
    return "\n".join(f"{i+1}:{lines[i]}" for i in range(start - 1, min(end, len(lines))))


def check_source(item: dict) -> dict:
    path = SRC / item["file"]
    text = line_window(path, item["range"][0], item["range"][1])
    missing = [p for p in item["patterns"] if not re.search(p, text, re.S)]
    return {
        "name": item["name"],
        "path": str(path),
        "line_range": item["range"],
        "ok": not missing,
        "missing_patterns": missing,
    }


def check_file(item: dict) -> dict:
    path = item["file"]
    text = path.read_text(errors="replace")
    missing = [p for p in item["patterns"] if not re.search(p, text, re.S)]
    return {"name": item["name"], "path": str(path), "ok": not missing, "missing_patterns": missing}


def main() -> int:
    source_results = [check_source(c) for c in CHECKS]
    firestaff_results = [check_file(c) for c in FIRESTAFF_CHECKS]
    existing_blocker = REPO / "verification-m11/n2-original-overlay-unblock-20260429/pass94-fixed-clean/pass80_original_frame_classifier.md"
    blocker_text = existing_blocker.read_text(errors="replace") if existing_blocker.exists() else ""
    blocker_ok = "48ed3743ab6a" in blocker_text and "duplicate raw frames detected" in blocker_text
    result = {
        "pass": all(r["ok"] for r in source_results + firestaff_results) and blocker_ok,
        "scope": "source-anchored preflight invariant only; no final original/Firestaff pixel parity claim",
        "source_results": source_results,
        "firestaff_results": firestaff_results,
        "blocked_original_route_evidence": {
            "path": str(existing_blocker),
            "ok": blocker_ok,
            "reason": "existing classifier records static no-party hash 48ed3743ab6a / duplicate raw frames, so quartet acceptance remains blocked until Lane A party/control route is solved",
        },
        "invariant": {
            "viewport_rect": {"x": 0, "y": 33, "width": 224, "height": 136},
            "timing": "title zoom, entrance wait/open, and gameplay viewport blit are source-gated by WaitVerticalBlank/Delay, not by emulator screenshot cadence",
            "pc_vga_viewport_palette_offset": "VIDEODRV.C F8161 sets G8177_c_ViewportColorIndexOffset=0x10 only around viewport blit",
        },
    }
    OUT.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n")
    print(json.dumps(result, indent=2, sort_keys=True))
    return 0 if result["pass"] else 1

if __name__ == "__main__":
    raise SystemExit(main())
