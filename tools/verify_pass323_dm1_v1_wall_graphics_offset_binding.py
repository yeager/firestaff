#!/usr/bin/env python3
"""Pass323 DM1 V1 wall graphics offset binding verifier.

Bounded verifier: no DOSBox launch and no raw bitmap publication. It audits the
ReDMCSB wall/tile presentation seams, cross-checks the Greatstone atlas and the
existing pass305 GRAPHICS.DAT wall manifest, then emits the exact offset binding
that a future live probe must promote after the pass318/pass320 debugger-control
blocker is solved.
"""
from __future__ import annotations

import json
import re
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass323_dm1_v1_wall_graphics_offset_binding"
SOURCE_ROOTS = [
    Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
    Path.home() / ".openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Source",
]
ATLAS = Path.home() / ".openclaw/data/firestaff-greatstone-atlas"
OUT_JSON = ROOT / f"parity-evidence/verification/{PASS}.json"
OUT_MD = ROOT / f"parity-evidence/{PASS}.md"
PASS305 = ROOT / "parity-evidence/verification/pass305_dm1_wall_graphics_93_107_manifest.json"
PASS318 = ROOT / "parity-evidence/verification/pass318_dm1_v1_f0097_after_f0128_offset_window_probe.json"
PASS320 = ROOT / "parity-evidence/verification/pass320_dm1_v1_f0097_vidrv_window_after_f0128_sequence_probe.json"

WALL_SLOT_NAMES = ["D0R", "D0L", "D1R", "D1L", "D1C", "D2R2", "D2L2", "D2R", "D2L", "D2C", "D3R2", "D3L2", "D3R", "D3L", "D3C"]
WALL_NEGATIVE_BITMAP_IDS = [-17, -16, -15, -14, -13, -9, -8, -12, -11, -10, -4, -3, -7, -6, -5]
WALL_GRAPHICS_INDICES = list(range(93, 108))

SOURCE_CHECKS: list[dict[str, Any]] = [
    {"id": "drawview_pc_viewport_present", "file": "DRAWVIEW.C", "range": [709, 858], "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "F0638_GetZone(C007_ZONE_VIEWPORT", "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box)"]},
    {"id": "videodrv_slot9_viewport_blit", "file": "VIDEODRV.C", "range": [941, 957], "needles": ["F8161_VIDRV_09_BlitViewPort", "/*  9 */"]},
    {"id": "videodrv_vga_color_offset_scope", "file": "VIDEODRV.C", "range": [3566, 3582], "needles": ["void F8161_VIDRV_09_BlitViewPort", "G8177_c_ViewportColorIndexOffset = 0x10", "F8151_VIDRV_02_Blit", "G8177_c_ViewportColorIndexOffset = 0"]},
    {"id": "tiledefs_substitute_view_square_wall_ordinals", "file": "DEFS.H", "range": [3423, 3437], "needles": ["C00_WALL_D0R", "C14_WALL_D3C"]},
    {"id": "tiledefs_substitute_zone_wall_ordinals", "file": "DEFS.H", "range": [4042, 4057], "needles": ["C702_ZONE_WALL_D3L2", "C717_ZONE_WALL_D0R"]},
    {"id": "wall_graphics_constants", "file": "DEFS.H", "range": [2359, 2373], "needles": ["M661_GRAPHIC_WALLSET_0_D0R", "C107_GRAPHIC_WALLSET_0_D3C"]},
    {"id": "wallset_negative_bitmap_slots", "file": "DUNVIEW.C", "range": [183, 200], "needles": ["int16_t G2107_WallSet[15]", "-17", "-5 }; /* Wall D3C */"]},
    {"id": "wall_frames_and_offsets", "file": "DUNVIEW.C", "range": [577, 594], "needles": ["G0161_auc_Graphic558_Box_WallBitmap_D3LCR", "G0162_auc_Graphic558_Box_WallBitmap_D2LCR", "G0163_aauc_Graphic558_Frame_Walls", "/* { X1, X2, Y1, Y2, ByteWidth, Height, X, Y } */"]},
    {"id": "wallset_load_loop", "file": "DUNVIEW.C", "range": [2214, 2217], "needles": ["L0070_i_WallSetLastGraphicIndex < 15", "AP0099_i_GraphicIndex++", "F0631_GetBitmapPointer(G2107_WallSet[L0070_i_WallSetLastGraphicIndex])"]},
    {"id": "wall_bitmap_draw_helpers", "file": "DUNVIEW.C", "range": [3048, 3074], "needles": ["void F0100_DUNGEONVIEW_DrawWallSetBitmap", "G0296_puc_Bitmap_Viewport", "P0106_puc_Frame[C6_X]", "P0106_puc_Frame[C7_Y]"]},
    {"id": "wall_center_draw_sites", "file": "DUNVIEW.C", "range": [6699, 6714], "needles": ["G0698_puc_Bitmap_WallSet_Wall_D3LCR", "G2107_WallSet[C14_WALL_D3C]", "G0076_B_UseFlippedWallAndFootprintsBitmaps"]},
    {"id": "wall_d2_draw_sites", "file": "DUNVIEW.C", "range": [7291, 7306], "needles": ["G0699_puc_Bitmap_WallSet_Wall_D2LCR", "G2107_WallSet[C09_WALL_D2C]", "G0076_B_UseFlippedWallAndFootprintsBitmaps"]},
]


def compact(s: str) -> str:
    return " ".join(s.split())


def source_root() -> Path:
    for root in SOURCE_ROOTS:
        if (root / "DUNVIEW.C").exists() and (root / "DRAWVIEW.C").exists() and (root / "VIDEODRV.C").exists():
            return root
    raise FileNotFoundError("ReDMCSB source root not found")


def audit_source(root: Path) -> list[dict[str, Any]]:
    rows = []
    for check in SOURCE_CHECKS:
        p = root / check["file"]
        lines = p.read_text(encoding="latin-1", errors="replace").splitlines() if p.exists() else []
        start, end = check["range"]
        block = compact("\n".join(lines[start - 1:min(end, len(lines))]))
        missing = [n for n in check["needles"] if compact(n) not in block]
        rows.append({"id": check["id"], "file": check["file"], "range": check["range"], "ok": p.exists() and not missing, "missing": missing})
    return rows


def audit_tiledefs(root: Path) -> dict[str, Any]:
    paths = sorted(str(p) for base in [root, *SOURCE_ROOTS] for p in base.glob("**/TILEDEFS.C"))
    return {"requestedFile": "TILEDEFS.C", "present": bool(paths), "paths": sorted(set(paths)), "decision": "NOT_PRESENT_NONBLOCKING_DEFS_H_DUNVIEW_ARE_TILEDEF_AUTHORITY", "reason": "This ReDMCSB tree has no TILEDEFS.C; PC wall/tile ordinals and zones are in DEFS.H, while wall-set bitmap tables/load/draw offsets are in DUNVIEW.C."}


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def wall_slot_bindings(pass305: dict[str, Any]) -> list[dict[str, Any]]:
    records = {int(r["graphicIndex"]): r for r in pass305.get("records", [])}
    rows = []
    for ordinal, (name, neg_id, gfx) in enumerate(zip(WALL_SLOT_NAMES, WALL_NEGATIVE_BITMAP_IDS, WALL_GRAPHICS_INDICES)):
        rec = records.get(gfx, {})
        rows.append({"ordinal": ordinal, "wallName": name, "g2107NegativeBitmapId": neg_id, "graphicsDatIndex": gfx, "graphicsDatRole": rec.get("role"), "width": rec.get("width"), "height": rec.get("height"), "compressedRecordSha256": rec.get("compressedRecordSha256"), "unpackedPixelSha256": rec.get("decode", {}).get("unpackedPixelSha256"), "ok": bool(rec) and rec.get("width") is not None and rec.get("height") is not None})
    return rows


def frame_offsets(root: Path) -> list[dict[str, Any]]:
    text = (root / "DUNVIEW.C").read_text(encoding="latin-1", errors="replace")
    m = re.search(r"G0163_aauc_Graphic558_Frame_Walls\[12\]\[8\]\s*=\s*\{(?P<body>.*?)\};", text, re.S)
    if not m:
        return []
    rows = []
    comments = ["D3C", "D3L", "D3R", "D2C", "D2L", "D2R", "D1C", "D1L", "D1R", "D0C", "D0L", "D0R"]
    for line in re.findall(r"\{\s*([^}]+?)\s*\}", m.group("body")):
        nums = [int(x) for x in re.findall(r"-?\d+", line)[:8]]
        if len(nums) == 8:
            i = len(rows)
            rows.append({"frameIndex": i, "viewSquare": comments[i] if i < len(comments) else f"index_{i}", "x1": nums[0], "x2": nums[1], "y1": nums[2], "y2": nums[3], "byteWidth": nums[4], "height": nums[5], "sourceX": nums[6], "sourceY": nums[7]})
    return rows


def atlas_audit() -> dict[str, Any]:
    gdm = ATLAS / "raw/greatstone.free.fr__dm__g_dm.html.html"
    text = gdm.read_text(encoding="utf-8", errors="replace") if gdm.exists() else ""
    graphics_links = sorted(set(re.findall(r"db_data/[^']*graphics\.dat/graphics\.dat\.html", text)))
    pc34 = "db_data/dm_pc_34/graphics.dat/graphics.dat.html" in graphics_links
    pc34_multi = "db_data/dm_pc_34_multi/graphics.dat/graphics.dat.html" in graphics_links
    return {"atlasRoot": str(ATLAS), "summaryPresent": (ATLAS / "index/SUMMARY.md").exists(), "keywordIndexPresent": (ATLAS / "index/keyword_hits.json").exists(), "dmPagePresent": gdm.exists(), "graphicsDatLinkCount": len(graphics_links), "pc34GraphicsDatListed": pc34, "pc34MultiGraphicsDatListed": pc34_multi, "pc34GraphicsLinks": [x for x in graphics_links if "dm_pc_34" in x], "decision": "Greatstone lists PC 3.4 graphics.dat/dungeon graphics pages, but the local crawl does not contain the per-entry graphics.dat HTML payload; pass305 canonical GRAPHICS.DAT decode remains the byte-level wall tile authority."}


def existing_probe_audit() -> dict[str, Any]:
    p318 = load_json(PASS318)
    p320 = load_json(PASS320)
    exact = p320.get("exactVidrvCandidate") or p320.get("exact_vidrv_candidate") or p320.get("runtime", {}).get("exactVidrvCandidate")
    if exact is None:
        txt = (ROOT / "parity-evidence/pass320_dm1_v1_f0097_vidrv_window_after_f0128_sequence_probe.md").read_text(encoding="utf-8")
        m = re.search(r"Exact VIDRV candidate: `([^`]+)`", txt)
        exact = m.group(1) if m else None
    return {"pass318Status": p318.get("status"), "pass320Status": p320.get("status"), "pass320ExactVidrvCandidate": exact, "postF0128WindowPromotable": False, "decision": "Preserve pass318/pass320 blocker: exact VIDRV call candidate is source/planned only until strict F0128 and F0097/VIDRV true stops are recaptured."}


def build() -> dict[str, Any]:
    root = source_root()
    p305 = load_json(PASS305)
    source = audit_source(root)
    tiles = audit_tiledefs(root)
    walls = wall_slot_bindings(p305)
    frames = frame_offsets(root)
    atlas = atlas_audit()
    probes = existing_probe_audit()
    checks = {
        "source_audit_ok": all(r["ok"] for r in source),
        "tiledefs_absence_recorded": tiles["decision"].startswith("NOT_PRESENT"),
        "pass305_wall_records_ok": p305.get("status") == "passed" and all(r["ok"] for r in walls) and [r["graphicsDatIndex"] for r in walls] == WALL_GRAPHICS_INDICES,
        "g2107_wall_order_ok": [r["g2107NegativeBitmapId"] for r in walls] == WALL_NEGATIVE_BITMAP_IDS,
        "frame_offsets_extracted": len(frames) == 12 and frames[0]["viewSquare"] == "D3C" and frames[-1]["viewSquare"] == "D0R",
        "greatstone_pc34_graphics_listed": atlas["pc34GraphicsDatListed"] is True and atlas["pc34MultiGraphicsDatListed"] is True,
        "pass318_blocker_preserved": str(probes["pass318Status"]).startswith("BLOCKED"),
        "pass320_blocker_preserved": probes["pass320Status"] == "BLOCKED_F0128_GATE_NOT_RECAPTURED_STRICT_STOP_FILTER",
        "vidrv_candidate_bound": probes["pass320ExactVidrvCandidate"] == "2809:1EFF",
    }
    ok = all(checks.values())
    return {"schema": f"{PASS}.v1", "timestampUtc": datetime.now(timezone.utc).isoformat(), "status": "WALL_GRAPHICS_OFFSET_BINDING_SOURCE_DATA_LOCKED_RUNTIME_BLOCKED" if ok else "BLOCKED_WALL_GRAPHICS_OFFSET_BINDING_AUDIT_INCOMPLETE", "sourceRoot": str(root), "checks": checks, "sourceAudit": source, "tiledefsAudit": tiles, "greatstoneAtlasAudit": atlas, "wallSlotBindings": walls, "frameOffsetBindings": frames, "runtimeOffsetBinding": {"f0097Entry": "2809:1E31", "vidrvSlot9CallCandidate": probes["pass320ExactVidrvCandidate"], "sourceAnchor": "DRAWVIEW.C:857 PC-family (*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box)", "promotionRule": "Promote only after strict true-stop recapture of F0128 followed by F0097/VIDRV call-window stop; do not use BPLIST/setup echoes."}, "existingProbeAudit": probes, "probePlan": {"dosboxNeededForThisVerifier": False, "ownVerifier": "python3 tools/verify_pass323_dm1_v1_wall_graphics_offset_binding.py", "nextRuntimeProbeShape": ["Retain pass320 strict-stop filtering.", "Arm F0128 23AD:40FE, F0097 2809:1E31, and the VIDRV call candidate 2809:1EFF after debugger readiness.", "After the route reaches a true post-F0128 stop, capture G0296 viewport pointer, L2413 viewport box, G0076 flip flag, and wall draw-site sequence around G2107 ordinals used in DUNVIEW.C.", "Promote each wall tile only when runtime draw-site ordinal -> G2107 negative bitmap id -> GRAPHICS.DAT index 93..107 -> frame offset is observed in order."], "nextBlocker": "Debugger stop/control sequencing: pass320 did not recapture the strict F0128 gate, so wall tile offset bindings are source/data locked but not live-runtime promoted."}, "notClaimed": ["new DOSBox runtime hit", "pixel parity", "new raw bitmap publication", "TILEDEFS.C existence"]}


def render_md(m: dict[str, Any]) -> str:
    lines = ["# Pass323 — DM1 V1 wall graphics offset binding", "", f"Status: `{m['status']}`", "", "## Verdict", "", "Wall graphics offsets are source/data locked: ReDMCSB binds PC wall ordinals to `G2107_WallSet`, pass305 binds those slots to GRAPHICS.DAT indices `93..107`, and DUNVIEW frame tables bind viewport source/destination offsets. Runtime promotion remains blocked by the pass320 strict-stop issue.", "", "## Checks", ""]
    lines += [f"- {'PASS' if v else 'FAIL'} `{k}`" for k, v in m["checks"].items()]
    lines += ["", "## Source audit", ""]
    lines += [f"- {'PASS' if r['ok'] else 'FAIL'} `{r['file']}:{r['range'][0]}-{r['range'][1]}` — {r['id']}" for r in m["sourceAudit"]]
    lines += ["", "## TILEDEFS.C", "", f"- `{m['tiledefsAudit']['decision']}` — {m['tiledefsAudit']['reason']}", "", "## Wall slot bindings", ""]
    for r in m["wallSlotBindings"]:
        lines.append(f"- {r['ordinal']:02d} `{r['wallName']}`: G2107 `{r['g2107NegativeBitmapId']}` -> GRAPHICS.DAT `{r['graphicsDatIndex']}` ({r['width']}x{r['height']})")
    lines += ["", "## Frame offsets", ""]
    for r in m["frameOffsetBindings"]:
        lines.append(f"- `{r['viewSquare']}`: dst x `{r['x1']}..{r['x2']}`, y `{r['y1']}..{r['y2']}`, byteWidth `{r['byteWidth']}`, height `{r['height']}`, src `{r['sourceX']},{r['sourceY']}`")
    lines += ["", "## Greatstone", "", f"- {m['greatstoneAtlasAudit']['decision']}", "", "## Runtime offset binding", ""]
    rb = m["runtimeOffsetBinding"]
    lines += [f"- F0097 entry: `{rb['f0097Entry']}`", f"- VIDRV slot-9 call candidate: `{rb['vidrvSlot9CallCandidate']}`", f"- Promotion rule: {rb['promotionRule']}", "", "## Next blocker", "", m["probePlan"]["nextBlocker"], ""]
    return "\n".join(lines)


def main() -> int:
    m = build()
    OUT_JSON.parent.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(m, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    OUT_MD.write_text(render_md(m), encoding="utf-8")
    ok = m["status"].startswith("WALL_GRAPHICS_OFFSET_BINDING")
    print(json.dumps({"status": m["status"], "ok": ok, "json": str(OUT_JSON.relative_to(ROOT)), "report": str(OUT_MD.relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
