#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]

def first_existing(env_name: str, candidates: list[Path]) -> Path:
    env = os.environ.get(env_name)
    if env:
        return Path(env)
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return candidates[0]

DATA = Path.home() / ".openclaw/data"
N2_DATA = Path("/home/trv2/.openclaw/data")
EXTERNAL_DATA = Path("/Volumes/Extern-disk/openclaw-data/firestaff")
RED = first_existing("FIRESTAFF_REDMCSB_SOURCE", [
    DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
    Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
])
GREATSTONE = first_existing("FIRESTAFF_GREATSTONE_ATLAS", [
    DATA / "firestaff-greatstone-atlas",
    Path("/home/trv2/.openclaw/data/firestaff-greatstone-atlas"),
])
CSBWIN = first_existing("FIRESTAFF_CSBWIN_SOURCE", [
    DATA / "firestaff-csbwin-source/CSBWin",
    Path("/home/trv2/.openclaw/data/firestaff-csbwin-source/CSBWin"),
])
CSB = first_existing("FIRESTAFF_CSB_SOURCE", [
    DATA / "firestaff-csb-source/CSB",
    Path("/home/trv2/.openclaw/data/firestaff-csb-source/CSB"),
])
DM1 = first_existing("FIRESTAFF_DM1_CANONICAL", [
    DATA / "firestaff-original-games/DM/_canonical/dm1",
    Path("/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1"),
])
OUT = ROOT / "parity-evidence/verification/pass512_dm1_v1_viewport_wall_clip_source_audit/manifest.json"
REPORT = ROOT / "parity-evidence/pass512_dm1_v1_viewport_wall_clip_source_audit.md"
DM1_HASHES = {"GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e", "DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"}

def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1048576), b""): h.update(chunk)
    return h.hexdigest()

def read(path: Path) -> str:
    if not path.exists(): raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding="utf-8", errors="replace")

def display_path(path: Path) -> str:
    path = path.resolve()
    for base, prefix in ((ROOT, ""), (DATA, "$OPENCLAW_DATA/"), (EXTERNAL_DATA, "$OPENCLAW_DATA/"), (N2_DATA, "$OPENCLAW_DATA/")):
        try:
            rel = path.relative_to(base.resolve())
        except ValueError:
            continue
        return f"{prefix}{rel.as_posix()}"
    return str(path)

def slice_lines(text: str, span: str) -> tuple[int, str]:
    start, end = (int(part) for part in span.split("-", 1))
    return start, "\n".join(text.splitlines()[start - 1:end])

def line_no(text: str, pos: int) -> int: return text.count("\n", 0, pos) + 1

def require_ordered(text: str, needles: list[str], label: str, base: int = 1) -> list[dict[str, object]]:
    hits = []; cursor = 0
    for needle in needles:
        pos = text.find(needle, cursor)
        if pos < 0: raise AssertionError(f"{label}: missing {needle!r}")
        hits.append({"line": base + line_no(text, pos) - 1, "needle": needle})
        cursor = pos + len(needle)
    return hits

def source_window(ident: str, path: Path, span: str, needles: list[str]) -> dict[str, object]:
    full = read(path); base, excerpt = slice_lines(full, span)
    return {"id": ident, "file": display_path(path), "lines": span, "sha256": sha256(path), "hits": require_ordered(excerpt, needles, ident, base)}

def whole_file(ident: str, path: Path, needles: list[str]) -> dict[str, object]:
    full = read(path)
    return {"id": ident, "file": display_path(path), "sha256": sha256(path), "hits": require_ordered(full, needles, ident)}

def main() -> int:
    redmcsb = [
        source_window("wall_frame_source_offsets", RED / "DUNVIEW.C", "436-440", ["unsigned char G0161_auc_Graphic558_Box_WallBitmap_D3LCR[4];", "unsigned char G0162_auc_Graphic558_Box_WallBitmap_D2LCR[4];", "unsigned char G0711_auc_Graphic558_Frame_Wall_D3L2[8];", "unsigned char G0712_auc_Graphic558_Frame_Wall_D3R2[8];", "unsigned char G0163_aauc_Graphic558_Frame_Walls[12][8];"]),
        source_window("transparent_and_opaque_wall_blit_routes", RED / "DUNVIEW.C", "3048-3076", ["void F0100_DUNGEONVIEW_DrawWallSetBitmap(", "P0106_puc_Frame[C6_X]", "P0106_puc_Frame[C7_Y]", "C10_COLOR_FLESH", "void F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency(", "CM1_COLOR_NO_TRANSPARENCY"]),
        source_window("f0791_source_row_clip_and_flip_adjustment", RED / "DUNVIEW.C", "3394-3470", ["STATICFUNCTION void F0791_DUNGEONVIEW_DrawBitmapXX(", "F0635_(P0101_puc_Bitmap_Source, G2032_ai_XYZ, P2081_i_ZoneIndex, &L2447_i_Width, &L2448_i_Height)", "if ((L2449_i_ > L2450_i_) && M007_GET(P2082_i_Flip, MASK0x0001_FLIP_HORIZONTAL))", "L2447_i_Width += L2449_i_;", "if ((L2449_i_ > L2450_i_) && M007_GET(P2082_i_Flip, MASK0x0002_FLIP_VERTICAL))", "L2448_i_Height += L2449_i_;", "F0132_VIDEO_Blit(P0101_puc_Bitmap_Source, P0102_puc_Bitmap_Destination, G2032_ai_XYZ"]),
        source_window("far_to_near_wall_square_replay", RED / "DUNVIEW.C", "8446-8542", ["F0153_DUNGEON_GetRelativeSquareType(P0183_i_Direction, 3, -2", "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF", "F0116_DUNGEONVIEW_DrawSquareD3L", "F0121_DUNGEONVIEW_DrawSquareD2C", "F0124_DUNGEONVIEW_DrawSquareD1C", "F0127_DUNGEONVIEW_DrawSquareD0C"]),
    ]
    firestaff = [
        source_window("local_clip_gate_contract", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "719-775", ["DM1_ViewportBlitClipGate dm1_viewport_3d_resolve_wall_blit_clip_gate", "gate.source_lines = \"DUNVIEW.C:3053-3058,3198-3204; COORD.C:2390-2409; IMAGE3.C:866-889\";", "int src_x = frame->blit_x;", "int src_y = frame->blit_y;", "if (dst_x < 0) { src_x -= dst_x; width += dst_x; dst_x = 0; }", "if (src_x + width > source_width) width = source_width - src_x;", "gate.src_x = (int16_t)src_x;", "gate.height = (int16_t)height;"]),
        source_window("local_transparent_wall_rows_use_clip_gate", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "466-500", ["void dm1_viewport_3d_draw_wall(", "dm1_viewport_3d_resolve_wall_blit_clip_gate", "const uint8_t *src_row = wall_bitmap + (gate.src_y + y) * bw + gate.src_x;", "uint8_t *dst_row = vp + (gate.dst_y + y) * vp_stride + gate.dst_x;", "if (pixel != COLOR_TRANSPARENT)"]),
        source_window("local_opaque_wall_rows_use_clip_gate", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "500-530", ["void dm1_viewport_3d_draw_wall_opaque(", "dm1_viewport_3d_resolve_wall_blit_clip_gate", "const uint8_t *src_row = wall_bitmap + (gate.src_y + y) * bw + gate.src_x;", "memcpy(dst_row, src_row, (size_t)gate.width);"]),
        source_window("local_clip_tests_cover_source_and_viewport_occlusion", ROOT / "tests/test_dm1_v1_viewport_3d_pc34_compat.c", "735-870", ["static void test_wall_source_row_clip_occlusion_gate(void)", "check_int(\"wall_clip_gate.151713.src_x\", gate.src_x, 4);", "check_int(\"wall_clip_gate.occluded_source_row\", gate.visible ? 1 : 0, 0);", "static void test_wall_draw_uses_clip_gate_source_offsets(void)", "check_int(\"wall_clip_draw.source_offset_next\"", "check_int(\"wall_clip_draw.opaque_copies_transparent_color\""]),
    ]
    secondary = [whole_file("greatstone_pc34_context_index", GREATSTONE / "index/pages.json", ["Swoosh Construction Kit", "graphics.dat", "dungeon.dat"]), whole_file("csbwin_viewport_script_lineage", CSBWIN / "Viewport.cpp", ["StdDrawRoomObjects", "roomSTONE", "StdDrawDoor", "DrawOrder349"]), whole_file("csb_viewport_script_lineage", CSB / "src/Viewport.cpp", ["StdDrawRoomObjects", "roomSTONE", "StdDrawDoor", "DrawOrder349"])]
    anchors = []
    for name, expected in DM1_HASHES.items():
        path = DM1 / name; actual = sha256(path)
        if actual != expected: raise AssertionError(f"{name} hash mismatch: {actual} != {expected}")
        anchors.append({"name": name, "path": display_path(path), "sha256": actual, "bytes": path.stat().st_size})
    manifest = {"schema": "pass512_dm1_v1_viewport_wall_clip_source_audit.v1", "status": "passed", "redmcsbPrimaryChecks": redmcsb, "firestaffChecks": firestaff, "secondaryReferenceChecks": secondary, "dm1CanonicalAnchors": anchors, "claim": "DM1 V1 wall drawing keeps ReDMCSB source-offset/source-row clipping represented in the local wall clip gate and wall-row blitters.", "nonClaims": ["No new original runtime screenshot was captured.", "No pixel-parity promotion is claimed.", "CSBWin/CSB are lineage references only; ReDMCSB remains primary."]}
    OUT.parent.mkdir(parents=True, exist_ok=True); OUT.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = ["# Pass512 DM1 V1 viewport wall clip source audit", "", "Status: passed", "", "## Primary ReDMCSB evidence"]
    lines.extend(f"- {Path(item['file']).name}:{item['lines']} {item['id']}" for item in redmcsb); lines.extend(["", "## Firestaff evidence"]); lines.extend(f"- {Path(item['file']).name}:{item['lines']} {item['id']}" for item in firestaff); lines.extend(["", "## Secondary references"]); lines.extend(f"- {item['file']} {item['id']}" for item in secondary); lines.extend(["", "## DM1 canonical anchors"]); lines.extend(f"- {item['name']} sha256 {item['sha256']} bytes {item['bytes']}" for item in anchors); lines.extend(["", "## Non-claims"]); lines.extend(f"- {item}" for item in manifest["nonClaims"])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("PASS pass512_dm1_v1_viewport_wall_clip_source_audit"); print(f"- wrote {OUT.relative_to(ROOT)}"); print(f"- wrote {REPORT.relative_to(ROOT)}")
    for item in redmcsb: print(f"- ReDMCSB {Path(item['file']).name}:{item['lines']} {item['id']}")
    for item in firestaff: print(f"- Firestaff {Path(item['file']).name}:{item['lines']} {item['id']}")
    return 0

if __name__ == "__main__":
    try: raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr); raise SystemExit(1)
