#!/usr/bin/env python3
"""Verify pass361 DM1 V1 viewport occlusion/redraw ordering gate."""
from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
MANIFEST = ROOT / "parity-evidence/verification/pass361_dm1_v1_viewport_occlusion_redraw_order_gate/manifest.json"
EVIDENCE = ROOT / "parity-evidence/pass361_dm1_v1_viewport_occlusion_redraw_order_gate.md"
EXPECTED_STATUS = "PASS_DM1_V1_VIEWPORT_OCCLUSION_REDRAW_ORDER_GATE"


def fail(message: str) -> int:
    print(f"status=FAIL_PASS361_VIEWPORT_OCCLUSION_REDRAW_ORDER_GATE reason={message}")
    return 1


def read_text(path: Path, encoding: str = "utf-8") -> str:
    return path.read_text(encoding=encoding)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def require_all(blob: str, needles: list[str], context: str) -> None:
    compact = " ".join(blob.split())
    for needle in needles:
        require(" ".join(needle.split()) in compact, f"{context} missing {needle}")


def source_block(file_name: str, start: int, end: int) -> str:
    path = SOURCE_ROOT / file_name
    require(path.exists(), f"missing ReDMCSB source {path}")
    lines = read_text(path, "latin-1").splitlines()
    require(end <= len(lines), f"{file_name} shorter than expected")
    return "\n".join(lines[start - 1:end])


def function_body(path: Path, function_name: str) -> str:
    text = read_text(path)
    marker = f"static void {function_name}("
    start = text.find(marker)
    require(start >= 0, f"missing {function_name}")
    brace = text.find("{", start)
    require(brace >= 0, f"missing body for {function_name}")
    depth = 0
    for pos in range(brace, len(text)):
        ch = text[pos]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError(f"unterminated body for {function_name}")


def assert_order(blob: str, labels: list[tuple[str, str]]) -> None:
    offset = -1
    for label, needle in labels:
        pos = blob.find(needle, offset + 1)
        require(pos >= 0, f"missing ordered marker {label}: {needle}")
        require(pos > offset, f"marker out of order {label}")
        offset = pos


def run_gate(cmd: list[str]) -> str:
    return subprocess.run(cmd, cwd=ROOT, check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True).stdout


def main() -> int:
    try:
        manifest = json.loads(read_text(MANIFEST))
        evidence = read_text(EVIDENCE)
        require(manifest.get("status") == EXPECTED_STATUS, "manifest status mismatch")
        require("raw source dumps" not in evidence.lower(), "evidence should cite, not dump source")
        for anchor in manifest.get("source_anchors", []):
            marker = anchor["file"] + ":" + anchor["lines"]
            require(marker in evidence, f"evidence missing anchor {marker}")

        require_all(source_block("DUNVIEW.C", 8318, 8618), [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling(",
            "F0116_DUNGEONVIEW_DrawSquareD3L(",
            "F0121_DUNGEONVIEW_DrawSquareD2C(",
            "F0124_DUNGEONVIEW_DrawSquareD1C(",
            "F0127_DUNGEONVIEW_DrawSquareD0C(",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ], "DUNVIEW.C:8318-8618")
        require_all(source_block("DUNVIEW.C", 8445, 8542), [
            "F0100_DUNGEONVIEW_DrawWallSetBitmap(G0697_puc_Bitmap_WallSet_Wall_D3L2",
            "F0116_DUNGEONVIEW_DrawSquareD3L(P0183_i_Direction",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction",
            "F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction",
            "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction",
        ], "DUNVIEW.C:8445-8542")
        require_all(source_block("DUNVIEW.C", 6400, 6835), [
            "F0100_DUNGEONVIEW_DrawWallSetBitmap(G0698_puc_Bitmap_WallSet_Wall_D3LCR",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
            "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
            "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
            "return;",
        ], "DUNVIEW.C:6400-6835")
        require_all(source_block("DUNVIEW.C", 7244, 7937), [
            "STATICFUNCTION void F0121_DUNGEONVIEW_DrawSquareD2C(",
            "STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C(",
            "F0100_DUNGEONVIEW_DrawWallSetBitmap(G0700_puc_Bitmap_WallSet_Wall_D1LCR",
            "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
            "return;",
        ], "DUNVIEW.C:7244-7937")
        require_all(source_block("DRAWVIEW.C", 709, 722), [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "M526_WaitVerticalBlank();",
        ], "DRAWVIEW.C:709-722")

        body = function_body(ROOT / "src/engine/m11_game_view.c", "m11_draw_viewport")
        require("if (state->showDebugHUD)" in body, "debug-only procedural corridor guard missing")
        require("nearer side layers" in body and "farther center doors/buttons/items cannot" in body,
                "near-side occlusion replay rationale missing")
        assert_order(body, [
            ("viewport clear", "m11_fill_rect(framebuffer, framebufferWidth, framebufferHeight,"),
            ("source background", "m11_draw_viewport_background(state,"),
            ("floor pits", "m11_draw_dm1_floor_pits(state,"),
            ("side walls", "m11_draw_dm1_side_walls(state,"),
            ("front walls", "m11_draw_dm1_front_walls(state,"),
            ("center doors", "m11_draw_dm1_center_doors(state,"),
            ("blocking center replay", "int blockingCenterDepth = m11_dm1_nearest_blocking_center_depth_index(cells);"),
            ("side contents", "m11_draw_dm1_side_contents(state,"),
            ("center contents fallback", "m11_draw_wall_contents(framebuffer,"),
            ("debug procedural renderer", "if (state->showDebugHUD)"),
        ])
        replay = re.search(r"if \(blockingCenterDepth > 0\).*?m11_draw_dm1_side_destroyed_door_masks\(", body, re.S)
        require(replay, "blocking center replay does not redraw nearer side occluder stack")

        run_gate([sys.executable, "tools/verify_pass359_dm1_v1_viewport_wall_draw_order_occlusion_sweep.py"])
        print(f"status={EXPECTED_STATUS}")
        print("sourceAnchors=%u" % len(manifest.get("source_anchors", [])))
        print("pass359ChainOk=1")
        print("normalViewportRedrawOrderOk=1")
        return 0
    except (AssertionError, OSError, json.JSONDecodeError, subprocess.CalledProcessError) as exc:
        return fail(str(exc))


if __name__ == "__main__":
    sys.exit(main())
