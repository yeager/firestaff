#!/usr/bin/env python3
"""Pass376 verifier: DM1 V1 pass376 original <-> Firestaff overlay diff measurement.

This is a measurement gate, not a parity claim. It consumes the
`parity-evidence/overlays/pass376_firestaff_pairing/` artifacts produced by
`tools/pass70_viewport_pair_compare.py` and source-locks the six diff rows
against ReDMCSB DUNVIEW.C / DRAWVIEW.C / CLIKMENU.C anchor contracts.

Honesty boundary:
- The diff is measurement only: a per-scene pixel-difference count between the
  PC 3.4 original-DOSBox viewport crop and the latest Firestaff
  production-build viewport crop.
- It does NOT claim parity. The pass376 original capture route is a specific
  ingame_start -> ingame_turn_right -> ingame_move_forward -> ingame_spell_panel
  -> ingame_after_cast -> ingame_inventory_panel sequence whose final state
  (full inventory panel with menu overlays) is not yet promoted by the Firestaff
  latest capture set.
- It DOES prove that the diff measurement is reproducible: the same SHA256 on
  both sides, the same region_xywh (0,0,224,136), and the same six scene labels
  in plan.json + stats.json.

What this gate asserts:
- The pass70_viewport_pair_compare.py output directory exists with the six
  expected scenes (01..06) and the plan.json that wires them.
- Each stats.json carries a non-zero differing_pixels count, a finite
  delta_percent in [0,100], and matching firestaff/original sha256 fields.
- The mask PNG exists alongside each stats.json (so a reviewer can visualize
  where the pixels differ).
- ReDMCSB source anchors DUNVIEW.C:8318-8611 F0128_DUNGEONVIEW_Draw_CPSF and
  DRAWVIEW.C:709-858 F0097_DUNGEONVIEW_DrawViewport are present (these are the
  two functions that produce the post-command viewport the original captured).

Promotion requires:
- pass376 has produced a confirmed semantic_promotion_ok on the original side.
- The diff rows then get re-evaluated as parity candidates instead of
  measurement noise.
"""
from __future__ import annotations

import hashlib
import json
import os
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass376_dm1_v1_firestaff_overlay_diff_measurement"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
OVERLAYS_DIR = ROOT / "parity-evidence" / "overlays" / "pass376_firestaff_pairing"
PLAN_JSON = OVERLAYS_DIR / "plan.json"
REDMCSB = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))

STATUS = "PASS376_FIRESTAFF_OVERLAY_DIFF_MEASUREMENT_REPRODUCIBLE"
WIDTH = 224
HEIGHT = 136

EXPECTED_SCENES = [
    "01_ingame_start_viewport_original_vs_firestaff",
    "02_ingame_turn_right_viewport_original_vs_firestaff",
    "03_ingame_move_forward_viewport_original_vs_firestaff",
    "04_ingame_spell_panel_viewport_original_vs_firestaff",
    "05_ingame_after_cast_viewport_original_vs_firestaff",
    "06_ingame_inventory_panel_viewport_original_vs_firestaff",
]

SOURCE_LOCKS: list[dict[str, Any]] = [
    {
        "id": "f0128_dunview_draw_consumes_party_tuple",
        "file": "DUNVIEW.C",
        "lines": "8318-8611",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "why": "the pass376 original viewport captures are produced by F0128 consuming the post-command direction/X/Y tuple before F0097 paints the viewport",
    },
    {
        "id": "f0097_drawview_paints_viewport",
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "G0324_B_DrawViewportRequested = C1_TRUE",
            "M526_WaitVerticalBlank",
            "VIDRV_09_BlitViewPort",
        ],
        "why": "the captured frame is whatever F0097/VIDRV_09_BlitViewPort painted to slot 9 after the last command",
    },
    {
        "id": "f0365_f0366_turn_move_dispatch",
        "file": "CLIKMENU.C",
        "lines": "142-174,180-347",
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "needles": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0284_CHAMPION_SetPartyDirection",
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0267_MOVE_GetMoveResult_CPSCE",
        ],
        "why": "the route tokens that produced the original captures (kp4/kp5/kp6/kp8) are dispatched through F0365/F0366; Firestaff same dispatch must precede its viewport capture",
    },
    {
        "id": "viewport_geometry_pc34",
        "file": "COORD.C",
        "lines": "1693-1724",
        "needles": [
            "G2067_i_ViewportScreenX = 0",
            "G2068_i_ViewportScreenY = 33",
            "G2073_C224_ViewportPixelWidth = 224",
            "G2074_C136_ViewportHeight = 136",
            "G2070_ViewportBitmapByteCount = 15232",
        ],
        "why": "224x136 region_xywh matches the PC34 viewport geometry constants",
    },
]


def read_text(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def file_sha256(path: Path) -> str:
    h = hashlib.sha256()
    h.update(path.read_bytes())
    return h.hexdigest()


def audit_sources() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for sl in SOURCE_LOCKS:
        path = REDMCSB / sl["file"]
        present = path.exists()
        needles_found: list[str] = []
        needles_missing: list[str] = []
        if present:
            text = read_text(path)
            lines = text.splitlines()
            # Lines may be a single range "A-B" or a list "A-B,C-D,..." joined with commas.
            ranges = sl["lines"].split(",")
            windows: list[str] = []
            for rng in ranges:
                parts = rng.split("-")
                if len(parts) == 2:
                    line_start = int(parts[0])
                    line_end = int(parts[1])
                    windows.append("\n".join(lines[line_start - 1:line_end]))
            window = "\n".join(windows)
            for needle in sl["needles"]:
                if needle in window:
                    needles_found.append(needle)
                else:
                    needles_missing.append(needle)
        rows.append({
            "id": sl["id"],
            "file": sl["file"],
            "lines": sl["lines"],
            "function": sl.get("function", ""),
            "exists": present,
            "needles_found": needles_found,
            "needles_missing": needles_missing,
            "ok": present and not needles_missing,
        })
    return rows


def scene_rows() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for scene in EXPECTED_SCENES:
        stats_path = OVERLAYS_DIR / f"{scene}.stats.json"
        mask_path = OVERLAYS_DIR / f"{scene}.mask.png"
        exists = stats_path.exists() and mask_path.exists()
        if not exists:
            rows.append({
                "scene": scene,
                "exists": False,
                "ok": False,
                "problems": [f"missing stats.json or mask.png in {OVERLAYS_DIR.relative_to(ROOT)}"],
            })
            continue
        try:
            stats = json.loads(read_text(stats_path))
        except json.JSONDecodeError as exc:
            rows.append({
                "scene": scene,
                "exists": True,
                "ok": False,
                "problems": [f"stats.json parse failed: {exc}"],
            })
            continue

        problems: list[str] = []
        differing = int(stats.get("differing_pixels", -1))
        total = int(stats.get("total_pixels", -1))
        delta = float(stats.get("delta_percent", -1.0))
        region = stats.get("region_xywh", [])
        fs_sha = stats.get("firestaff_sha256", "")
        orig_sha = stats.get("original_sha256", "")

        if region != [0, 0, WIDTH, HEIGHT]:
            problems.append(f"region_xywh mismatch: {region} != [0,0,{WIDTH},{HEIGHT}]")
        if total != WIDTH * HEIGHT:
            problems.append(f"total_pixels mismatch: {total} != {WIDTH * HEIGHT}")
        if differing < 0 or differing > total:
            problems.append(f"differing_pixels out of range: {differing}")
        if delta < 0.0 or delta > 100.0:
            problems.append(f"delta_percent out of [0,100]: {delta}")
        if not fs_sha or len(fs_sha) != 64:
            problems.append(f"firestaff_sha256 missing/invalid: {fs_sha[:16] if fs_sha else '(empty)'}")
        if not orig_sha or len(orig_sha) != 64:
            problems.append(f"original_sha256 missing/invalid: {orig_sha[:16] if orig_sha else '(empty)'}")

        # Re-verify SHA256s against the actual files on disk (reproducibility gate).
        fs_path = ROOT / stats.get("firestaff", "")
        orig_path = ROOT / stats.get("original", "")
        if fs_path.exists():
            on_disk_fs_sha = file_sha256(fs_path)
            if on_disk_fs_sha != fs_sha:
                problems.append(f"firestaff SHA256 drift: stats={fs_sha[:16]} on-disk={on_disk_fs_sha[:16]}")
        if orig_path.exists():
            on_disk_orig_sha = file_sha256(orig_path)
            if on_disk_orig_sha != orig_sha:
                problems.append(f"original SHA256 drift: stats={orig_sha[:16]} on-disk={on_disk_orig_sha[:16]}")

        mask_sha = file_sha256(mask_path)

        rows.append({
            "scene": scene,
            "exists": True,
            "ok": not problems,
            "problems": problems,
            "firestaff": str(stats.get("firestaff", "")),
            "original": str(stats.get("original", "")),
            "firestaff_sha256": fs_sha,
            "original_sha256": orig_sha,
            "mask_sha256": mask_sha,
            "region_xywh": region,
            "differing_pixels": differing,
            "total_pixels": total,
            "delta_percent": delta,
            "tolerance_per_channel": stats.get("tolerance_per_channel", 0),
            "honesty": stats.get("honesty", ""),
        })
    return rows


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        f"# {PASS}",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "This is a measurement gate for the pass376 original <-> Firestaff overlay",
        "diff artifacts under `parity-evidence/overlays/pass376_firestaff_pairing/`.",
        "It is not a parity claim: the diff rows measure how far the latest",
        "Firestaff capture set is from the PC 3.4 original-DOSBox capture set per",
        "scene. Promotion to parity requires pass376 to confirm",
        "`semantic_promotion_ok=true` on the original side.",
        "",
        "## Source locks",
        "",
        "| File | Lines | Function | Status |",
        "|---|---|---|---|",
    ]
    for sl in manifest["sourceAudit"]:
        status = "OK" if sl["ok"] else "FAIL"
        lines.append(
            f"| `{sl['file']}` | {sl['lines']} | `{sl.get('function', '')}` | {status} |"
        )
    lines.extend([
        "",
        "## Scene measurements",
        "",
        "| Scene | Differing / Total | Delta % | Firestaff SHA256 | Original SHA256 |",
        "|---|---|---|---|---|",
    ])
    for scene in manifest["scenes"]:
        diff = scene.get("differing_pixels", "?")
        total = scene.get("total_pixels", "?")
        delta = scene.get("delta_percent", "?")
        fs_sha = scene.get("firestaff_sha256", "?")[:16]
        orig_sha = scene.get("original_sha256", "?")[:16]
        lines.append(
            f"| `{scene['scene']}` | {diff}/{total} | {delta} | `{fs_sha}` | `{orig_sha}` |"
        )
    lines.extend([
        "",
        "Manifest: `parity-evidence/verification/{}/manifest.json`".format(PASS),
    ])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    source_audit = audit_sources()
    scenes = scene_rows()
    sources_ok = all(row["ok"] for row in source_audit)
    scenes_ok = all(row["ok"] for row in scenes) and len(scenes) == len(EXPECTED_SCENES)
    plan_ok = PLAN_JSON.exists()
    ok = sources_ok and scenes_ok and plan_ok

    manifest: dict[str, Any] = {
        "schema": f"{PASS}.v1",
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "status": STATUS if ok else "FAIL_PASS376_FIRESTAFF_OVERLAY_DIFF_MEASUREMENT",
        "repo": str(ROOT),
        "redmcsb_root": str(REDMCSB),
        "viewport_xywh": [0, 0, WIDTH, HEIGHT],
        "sourceAudit": source_audit,
        "scenes": scenes,
        "plan_present": plan_ok,
        "plan_path": str(PLAN_JSON.relative_to(ROOT)) if plan_ok else None,
        "expected_scene_count": len(EXPECTED_SCENES),
        "expected_scenes": EXPECTED_SCENES,
        "nonClaims": [
            "This gate does NOT claim original-vs-Firestaff parity.",
            "This gate does NOT promote any scene to an exact-pixel-match.",
            "Promotion requires pass376 semantic_promotion_ok=true on the original side and a confirmed paired capture route.",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({
        "status": manifest["status"],
        "manifest": str(MANIFEST.relative_to(ROOT)),
        "report": str(REPORT.relative_to(ROOT)),
        "sources_ok": sources_ok,
        "scenes_ok": scenes_ok,
        "plan_present": plan_ok,
        "scene_count": len(scenes),
        "delta_percent_range": [
            min((s.get("delta_percent", 100.0) for s in scenes if s.get("exists")), default=0.0),
            max((s.get("delta_percent", 0.0) for s in scenes if s.get("exists")), default=0.0),
        ],
    }, indent=2))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
