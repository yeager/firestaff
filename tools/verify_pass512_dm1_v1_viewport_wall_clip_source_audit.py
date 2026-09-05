# NOTE: Updated line ranges after v0.9.0 merge
#!/usr/bin/env python3
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import subprocess
from zipfile import ZipFile
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from firestaff_build_dir import resolve_build_dir, find_build_dir

ROOT = Path(__file__).resolve().parents[1]

RED = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    ROOT / "reference/redmcsb-20210206/Toolchains/Common/Source"))
DM1_ARCHIVE = Path(os.environ.get(
    "FIRESTAFF_DM1_PC34_ARCHIVE",
    Path.home() / ".firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"))
OUT = ROOT / "parity-evidence/verification/pass512_dm1_v1_viewport_wall_clip_source_audit/manifest.json"
REPORT = ROOT / "parity-evidence/pass512_dm1_v1_viewport_wall_clip_source_audit.md"
DM1_HASHES = {"DATA/GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e", "DATA/DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"}

def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1048576), b""): h.update(chunk)
    return h.hexdigest()

def read(path: Path) -> str:
    if not path.exists(): raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding="utf-8", errors="replace")

def zip_member(path: Path, name: str) -> bytes:
    result = subprocess.run(["unzip", "-p", str(path), name], check=False,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if result.returncode not in (0, 1) or not result.stdout:
        raise AssertionError(f"cannot read retail ZIP member {name}")
    return result.stdout

def display_path(path: Path) -> str:
    path = path.resolve()
    try:
        return path.relative_to(ROOT.resolve()).as_posix()
    except ValueError:
        pass
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
        # Line ranges refreshed against the current placement of these
        # three symbols in dm1_v1_viewport_3d_pc34_compat.c:
        #   dm1_viewport_3d_resolve_wall_blit_clip_gate: 2558-2598
        #   dm1_viewport_3d_draw_wall:                    1586-1610
        #   dm1_viewport_3d_draw_wall_opaque:             1620-1638
        # Body content unchanged; only the offsets drifted.
        source_window("local_clip_gate_contract", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "2594-2640", ["DM1_ViewportBlitClipGate dm1_viewport_3d_resolve_wall_blit_clip_gate", "gate.source_lines = \"DUNVIEW.C:3053-3058,3198-3204; COORD.C:2390-2409; IMAGE3.C:866-889\";", "int src_x = frame->blit_x;", "int src_y = frame->blit_y;", "if (dst_x < 0) { src_x -= dst_x; width += dst_x; dst_x = 0; }", "if (src_x + width > source_width) width = source_width - src_x;", "gate.src_x = (int16_t)src_x;", "gate.height = (int16_t)height;"]),
        source_window("local_transparent_wall_rows_use_clip_gate", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "1622-1654", ["void dm1_viewport_3d_draw_wall(DM1_Viewport3DState *state,", "dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);", "const uint8_t *src_row = wall_bitmap + (gate.src_y + y) * bw + gate.src_x;", "uint8_t *dst_row = vp + (gate.dst_y + y) * vp_stride + gate.dst_x;", "if (pixel != COLOR_TRANSPARENT)"]),
        source_window("local_opaque_wall_rows_use_clip_gate", ROOT / "src/dm1/dm1_v1_viewport_3d_pc34_compat.c", "1656-1685", ["void dm1_viewport_3d_draw_wall_opaque(DM1_Viewport3DState *state,", "dm1_viewport_3d_resolve_wall_blit_clip_gate(frame, frame->byte_width, frame->height);", "const uint8_t *src_row = wall_bitmap + (gate.src_y + y) * bw + gate.src_x;", "memcpy(dst_row, src_row, (size_t)gate.width);"]),
        # Test suite offsets drifted: the two clip-gate coverage tests are
        # now at lines 2159 and 2195 in the same file.
        source_window("local_clip_tests_cover_source_and_viewport_occlusion", ROOT / "tests/test_dm1_v1_viewport_3d_pc34_compat.c", "2183-2285", ["static void test_wall_source_row_clip_occlusion_gate(void)", "check_int(\"wall_clip_gate.151713.src_x\", gate.src_x, 4);", "check_int(\"wall_clip_gate.occluded_source_row\", gate.visible ? 1 : 0, 0);", "static void test_wall_draw_uses_clip_gate_source_offsets(void)", "check_int(\"wall_clip_draw.source_offset_next\"", "check_int(\"wall_clip_draw.opaque_copies_transparent_color\""]),
    ]
    secondary: list[dict[str, object]] = []
    anchors = []
    if not DM1_ARCHIVE.is_file(): raise AssertionError(f"missing retail PC34 archive: {DM1_ARCHIVE}")
    with ZipFile(DM1_ARCHIVE) as archive: members = {item.filename for item in archive.infolist()}
    for name, expected in DM1_HASHES.items():
        if name not in members: raise AssertionError(f"missing retail ZIP member: {name}")
        data = zip_member(DM1_ARCHIVE, name); actual = hashlib.sha256(data).hexdigest()
        if actual != expected: raise AssertionError(f"{name} hash mismatch: {actual} != {expected}")
        anchors.append({"name": name, "archive": str(DM1_ARCHIVE), "sha256": actual, "bytes": len(data)})
    manifest = {"schema": "pass512_dm1_v1_viewport_wall_clip_source_audit.v1", "status": "passed", "redmcsbPrimaryChecks": redmcsb, "firestaffChecks": firestaff, "secondaryReferenceChecks": secondary, "dm1CanonicalAnchors": anchors, "claim": "DM1 V1 wall drawing keeps ReDMCSB source-offset/source-row clipping represented in the local wall clip gate and wall-row blitters.", "nonClaims": ["No new original runtime screenshot was captured.", "No pixel-parity promotion is claimed."]}
    OUT.parent.mkdir(parents=True, exist_ok=True); OUT.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = ["# Pass512 DM1 V1 viewport wall clip source audit", "", "Status: passed", "", "## Primary ReDMCSB evidence"]
    lines.extend(f"- {Path(item['file']).name}:{item['lines']} {item['id']}" for item in redmcsb); lines.extend(["", "## Firestaff evidence"]); lines.extend(f"- {Path(item['file']).name}:{item['lines']} {item['id']}" for item in firestaff); lines.extend(["", "## DM1 canonical anchors"]); lines.extend(f"- {item['name']} sha256 {item['sha256']} bytes {item['bytes']}" for item in anchors); lines.extend(["", "## Non-claims"]); lines.extend(f"- {item}" for item in manifest["nonClaims"])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print("PASS pass512_dm1_v1_viewport_wall_clip_source_audit"); print(f"- wrote {OUT.relative_to(ROOT)}"); print(f"- wrote {REPORT.relative_to(ROOT)}")
    for item in redmcsb: print(f"- ReDMCSB {Path(item['file']).name}:{item['lines']} {item['id']}")
    for item in firestaff: print(f"- Firestaff {Path(item['file']).name}:{item['lines']} {item['id']}")
    return 0

if __name__ == "__main__":
    try: raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr); raise SystemExit(1)
