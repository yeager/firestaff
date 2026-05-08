#!/usr/bin/env python3
"""Verify pass362 DM1 V1 viewport/walls source-lock landable metadata."""
from __future__ import annotations

import hashlib
import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CANONICAL_DM1 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
MANIFEST = ROOT / "parity-evidence/verification/pass362_dm1_v1_viewport_walls_source_lock_landable/manifest.json"
EVIDENCE = ROOT / "parity-evidence/pass362_dm1_v1_viewport_walls_source_lock_landable.md"
EXPECTED_STATUS = "PASS_DM1_V1_VIEWPORT_WALLS_SOURCE_LOCK_LANDABLE"


def fail(message: str) -> int:
    print(f"status=FAIL_PASS362_VIEWPORT_WALLS_SOURCE_LOCK reason={message}")
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


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def run_gate(cmd: list[str]) -> str:
    return subprocess.run(cmd, cwd=ROOT, check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True).stdout


def run_ctest_if_present(name: str) -> str:
    build = ROOT / "build"
    if not (build / "CTestTestfile.cmake").exists():
        return f"SKIP {name}: no build/CTestTestfile.cmake"
    return subprocess.run(["ctest", "--test-dir", str(build), "-R", f"^{name}$", "--output-on-failure"],
                          cwd=ROOT, check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True).stdout


def main() -> int:
    try:
        manifest = json.loads(read_text(MANIFEST))
        evidence = read_text(EVIDENCE)
        require(manifest.get("status") == EXPECTED_STATUS, "manifest status mismatch")
        require("pixel parity claim" not in evidence.lower().replace("no pixel parity claim", ""),
                "evidence must not claim pixel parity")
        require("No renderer behavior changes." in evidence, "scope guard missing")

        for anchor in manifest.get("source_anchors", []):
            marker = anchor["file"] + ":" + anchor["lines"]
            require(marker in evidence, f"evidence missing source anchor {marker}")

        require_all(source_block("DUNVIEW.C", 8318, 8618), [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0116_DUNGEONVIEW_DrawSquareD3L(",
            "F0121_DUNGEONVIEW_DrawSquareD2C(",
            "F0124_DUNGEONVIEW_DrawSquareD1C(",
            "F0127_DUNGEONVIEW_DrawSquareD0C(",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ], "DUNVIEW.C:8318-8618")
        require_all(source_block("DUNVIEW.C", 6213, 6356), [
            "C14_VIEW_SQUARE_D3L2",
            "C15_VIEW_SQUARE_D3R2",
            "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
            "C0x0128_CELL_ORDER_DOORPASS1_BACKRIGHT_BACKLEFT",
        ], "DUNVIEW.C:6213-6356")
        require_all(source_block("DUNVIEW.C", 6361, 6835), [
            "STATICFUNCTION void F0116_DUNGEONVIEW_DrawSquareD3L(",
            "STATICFUNCTION void F0117_DUNGEONVIEW_DrawSquareD3R(",
            "STATICFUNCTION void F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(",
            "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
            "return;",
        ], "DUNVIEW.C:6361-6835")
        require_all(source_block("DUNVIEW.C", 7244, 7937), [
            "STATICFUNCTION void F0121_DUNGEONVIEW_DrawSquareD2C(",
            "STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C(",
            "F0100_DUNGEONVIEW_DrawWallSetBitmap(G0700_puc_Bitmap_WallSet_Wall_D1LCR",
            "C0x0000_CELL_ORDER_ALCOVE",
        ], "DUNVIEW.C:7244-7937")
        require_all(source_block("DUNVIEW.C", 7960, 8308), [
            "STATICFUNCTION void F0125_DUNGEONVIEW_DrawSquareD0L(",
            "STATICFUNCTION void F0126_DUNGEONVIEW_DrawSquareD0R(",
            "STATICFUNCTION void F0127_DUNGEONVIEW_DrawSquareD0C(",
            "C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT",
        ], "DUNVIEW.C:7960-8308")
        require_all(source_block("DRAWVIEW.C", 709, 722), [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "M526_WaitVerticalBlank();",
        ], "DRAWVIEW.C:709-722")

        asset_hashes = {}
        for asset_name in ("GRAPHICS.DAT", "DUNGEON.DAT"):
            asset = CANONICAL_DM1 / asset_name
            require(asset.exists(), f"missing canonical DM1 asset {asset}")
            asset_hashes[asset_name] = sha256(asset)

        out361 = run_gate([sys.executable, "tools/verify_pass361_dm1_v1_viewport_occlusion_redraw_order_gate.py"])
        out_meta = run_gate([sys.executable, "tools/verify_dm1_v1_viewport_3d_occlusion_metadata_gate.py"])
        out_draw = run_ctest_if_present("dm1_v1_viewport_draw_order_probe")
        out_walls = run_ctest_if_present("firestaff_dm1_v1_walls_occlusion_blockers_probe")

        print(f"status={EXPECTED_STATUS}")
        print("canonicalAssets=" + ",".join(f"{k}:{v}" for k, v in asset_hashes.items()))
        print("pass361Ok=" + ("PASS_DM1_V1_VIEWPORT_OCCLUSION_REDRAW_ORDER_GATE" in out361 and "1" or "0"))
        print("metadataGateOk=" + ("PASS dm1-v1-viewport-3d-occlusion-metadata-gate" in out_meta and "1" or "0"))
        print("drawOrderProbe=" + out_draw.strip().splitlines()[-1])
        print("wallsOcclusionProbe=" + out_walls.strip().splitlines()[-1])
        return 0
    except (AssertionError, OSError, json.JSONDecodeError, subprocess.CalledProcessError) as exc:
        return fail(str(exc))


if __name__ == "__main__":
    sys.exit(main())
