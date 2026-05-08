#!/usr/bin/env python3
"""Pass377 verifier for DM1 V1 paired Firestaff/original viewport diff artifacts.

This is a blocker-narrowing pass for pass376 blocker #4.  It validates that
Firestaff-side viewport artifacts are available and builds the pairing/diff
manifest, but it refuses to claim or run paired diffs until blockers #1-#3
(original true-stop transcript, labelled original frames, original viewport
crops) have produced a promotable original side.
"""
from __future__ import annotations

import json
import os
import struct
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass377_dm1_v1_paired_diff_artifact_blocker"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
PLAN = ROOT / "parity-evidence" / "overlays" / "pass377" / "pass377_pairing_plan.json"
REDMCSB = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
STATUS = "BLOCKED_PASS377_PAIRED_DIFF_ARTIFACTS_ORIGINAL_SIDE_MISSING_FIRESTAFF_SIDE_READY"
WIDTH = 224
HEIGHT = 136

SOURCE_ANCHORS = [
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
        "why": "paired diff inputs must be 224x136 viewport crops from the PC34 viewport at x=0,y=33",
    },
    {
        "id": "viewport_composition_buffer",
        "file": "DUNVIEW.C",
        "lines": "2962-3002,3048-3092",
        "needles": [
            "void F0098_DUNGEONVIEW_DrawFloorAndCeiling",
            "G0296_puc_Bitmap_Viewport",
            "C112_BYTE_WIDTH_VIEWPORT",
            "C136_HEIGHT_VIEWPORT",
        ],
        "why": "Firestaff/original crops must compare the composed G0296 viewport buffer, not arbitrary full-frame pixels",
    },
    {
        "id": "viewport_presentation_seam",
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport",
            "VIDRV_09_BlitViewPort",
        ],
        "why": "original-side captures are promotable only after the source presentation seam",
    },
    {
        "id": "pc34_vidrv_slot9",
        "file": "VIDEODRV.C",
        "lines": "3566-3582",
        "needles": [
            "FUNC_DEF void F8161_VIDRV_09_BlitViewPort",
            "G8177_c_ViewportColorIndexOffset = 0x10",
            "F8151_VIDRV_02_Blit",
            "224, 320",
        ],
        "why": "PC34 slot 9 presents the viewport with the expected 224-wide source and 320-wide screen stride",
    },
]

PRIOR_DEPENDENCIES = {
    "blocker_1_original_true_stop_transcript": {
        "path": "parity-evidence/verification/pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing/manifest.json",
        "expected_status": "BLOCKED_PASS360_ORIGINAL_RUNTIME_TRUE_STOP_BLOCKER_NARROWED",
        "role": "strict original FIRES F0128 -> F0097/VIDRV true-stop is still required before original frames are promotable",
    },
    "blocker_2_labelled_original_full_frames": {
        "path": "parity-evidence/verification/pass376_dm1_v1_original_artifact_command_manifest/manifest.json",
        "expected_status": "BLOCKED_PASS376_ORIGINAL_FRAMES_CROPS_NARROWED",
        "role": "labels/raw original 320x200 frames and 224x136 crops exist, but duplicate hashes/pass86 mismatches still block semantic promotion",
    },
    "blocker_3_original_viewport_crops": {
        "path": "parity-evidence/verification/pass376_dm1_v1_original_overlay_parity_plan/manifest.json",
        "expected_status": "BLOCKED_PASS376_ORIGINAL_OVERLAY_RUNTIME_ARTIFACTS_MISSING",
        "role": "224x136 original crops are missing until pass86 succeeds on labelled original frames",
    },
    "firestaff_route_source_locked": {
        "path": "parity-evidence/verification/pass372_dm1_v1_movement_runtime_route/manifest.json",
        "expected_status": "PASS372_DM1_V1_MOVEMENT_RUNTIME_ROUTE_SOURCE_LOCKED",
        "role": "Firestaff movement/capture side is not the active route blocker",
    },
}


def run(cmd: list[str], check: bool = False) -> subprocess.CompletedProcess[str]:
    return subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=check)


def norm(text: str) -> str:
    return " ".join(text.split())


def source_window(path: Path, spec: str) -> str:
    data = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        start, end = [int(x) for x in part.split("-", 1)] if "-" in part else (int(part), int(part))
        chunks.append("\n".join(data[start - 1:end]))
    return "\n".join(chunks)


def audit_sources() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for anchor in SOURCE_ANCHORS:
        path = REDMCSB / anchor["file"]
        text = source_window(path, anchor["lines"]) if path.exists() else ""
        missing = [needle for needle in anchor["needles"] if norm(needle) not in norm(text)]
        rows.append({**anchor, "path": str(path), "ok": path.exists() and not missing, "missing_markers": missing})
    return rows


def load_dependencies() -> dict[str, Any]:
    out: dict[str, Any] = {}
    for name, dep in PRIOR_DEPENDENCIES.items():
        path = ROOT / dep["path"]
        data = json.loads(path.read_text(encoding="utf-8")) if path.exists() else {}
        status = data.get("status")
        route_probe = data.get("route_artifact_probe", {}) if isinstance(data, dict) else {}
        out[name] = {
            **dep,
            "exists": path.exists(),
            "status": status,
            "ok": path.exists() and status == dep["expected_status"],
            "active_blocker": data.get("activeBlocker") or data.get("blocker") or data.get("blockers"),
            "semantic_promotion_ok": route_probe.get("semantic_promotion_ok"),
            "duplicate_sha256_counts": route_probe.get("duplicate_sha256_counts"),
        }
    return out


def png_dims(path: Path) -> tuple[int, int] | None:
    with path.open("rb") as f:
        header = f.read(24)
    if len(header) >= 24 and header[:8] == b"\x89PNG\r\n\x1a\n" and header[12:16] == b"IHDR":
        return struct.unpack(">II", header[16:24])
    return None


def firestaff_artifact_rows(plan: dict[str, Any]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    manifest_path = ROOT / "verification-screens" / "capture_manifest_sha256.tsv"
    manifest_text = manifest_path.read_text(encoding="utf-8") if manifest_path.exists() else ""
    for pair in plan.get("pairings", []):
        png_rel = pair["firestaff_png"]
        ppm_rel = pair["firestaff_ppm"]
        png = ROOT / png_rel
        ppm_manifested = Path(ppm_rel).name in manifest_text and "viewport_224x136\t" in manifest_text
        dims = png_dims(png) if png.exists() else None
        rows.append({
            "index": pair["index"],
            "scene": pair["scene"],
            "png": png_rel,
            "png_exists": png.exists(),
            "png_dims": list(dims) if dims else None,
            "png_dims_ok": dims == (WIDTH, HEIGHT),
            "ppm_manifest_row": ppm_rel,
            "ppm_manifested_as_224x136": ppm_manifested,
            "ok": png.exists() and dims == (WIDTH, HEIGHT) and ppm_manifested,
        })
    return rows


def build_pairing_plan() -> tuple[dict[str, Any], str, int]:
    cmd = [
        "python3", "tools/pass70_viewport_pair_compare.py",
        "--firestaff-dir", "verification-screens",
        "--original-dir", "verification-screens/pass376-original-dm1-viewports/viewport_224x136",
        "--original-manifest", "verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv",
        "--out-dir", "parity-evidence/overlays/pass377",
        "--plan-json", str(PLAN.relative_to(ROOT)),
    ]
    proc = run(cmd)
    plan = json.loads(PLAN.read_text(encoding="utf-8")) if PLAN.exists() else {}
    return plan, proc.stdout, proc.returncode


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# Pass377 — DM1 V1 paired diff artifact blocker narrowing",
        "",
        "Status: `{}`".format(manifest["status"]),
        "",
        "## Decision",
        "",
        "Blocker #4 is narrowed to original-side absence only: all six Firestaff viewport inputs are present and dimension-checked, and the pass70 pairing plan is materialized. Paired diffs are deliberately not run because blockers #1-#3 are still active.",
        "",
        "## Source audit",
        "",
    ]
    for row in manifest["source_audit"]:
        lines.append("- `{}:{}` — {} ok=`{}`".format(row["file"], row["lines"], row["why"], row["ok"]))
    lines.extend(["", "## Dependency blockers", ""])
    for name, row in manifest["dependencies"].items():
        lines.append("- `{}` `{}` — {} ok=`{}`".format(name, row["status"], row["role"], row["ok"]))
    lines.extend(["", "## Firestaff-side artifacts", ""])
    for row in manifest["firestaff_artifacts"]:
        lines.append("- `{}` `{}` dims=`{}` manifest_ppm=`{}` ok=`{}`".format(row["scene"], row["png"], row["png_dims"], row["ppm_manifested_as_224x136"], row["ok"]))
    lines.extend([
        "",
        "## Pairing/diff plan",
        "",
        "- plan: `{}`".format(manifest["pairing_plan_path"]),
        "- original-side blockers from pass70: `{}`".format(manifest["pairing_plan_blockers"]),
        "- diffs run: `False` (requires promotable original pair; no parity claim)",
        "",
        "## Non-claims",
        "",
        "- No original-vs-Firestaff pixel parity is claimed.",
        "- No paired diff masks/stats are claimed because the original pair is missing.",
        "- No original true-stop transcript or semantically matched original frame is claimed.",
        "",
        "Manifest: `parity-evidence/verification/{}/manifest.json`".format(PASS),
    ])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    PLAN.parent.mkdir(parents=True, exist_ok=True)
    sources = audit_sources()
    deps = load_dependencies()
    plan, plan_stdout, plan_rc = build_pairing_plan()
    fs_rows = firestaff_artifact_rows(plan)
    source_ok = all(row["ok"] for row in sources)
    deps_ok = all(row["ok"] for row in deps.values())
    firestaff_ok = len(fs_rows) == 6 and all(row["ok"] for row in fs_rows)
    original_semantic_ready = deps.get("blocker_2_labelled_original_full_frames", {}).get("semantic_promotion_ok") is True
    original_not_promotable = not original_semantic_ready
    ok = source_ok and deps_ok and firestaff_ok and original_not_promotable and plan_rc == 0
    manifest: dict[str, Any] = {
        "schema": f"{PASS}.v1",
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "status": STATUS if ok else "FAIL_PASS377_PAIRED_DIFF_ARTIFACT_BLOCKER_AUDIT",
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"]).stdout.strip(),
        "head": run(["git", "rev-parse", "HEAD"]).stdout.strip(),
        "source_root": str(REDMCSB),
        "source_audit": sources,
        "dependencies": deps,
        "pairing_plan_path": str(PLAN.relative_to(ROOT)),
        "pairing_plan_schema": plan.get("schema"),
        "pairing_plan_blockers": plan.get("blockers", []),
        "pairing_plan_stdout": plan_stdout,
        "firestaff_artifacts": fs_rows,
        "blocker_narrowing": {
            "blocker_4": "paired Firestaff/original diff artifacts",
            "firestaff_side_ready": firestaff_ok,
            "original_side_ready": original_semantic_ready,
            "depends_on_blockers_1_to_3": [
                "blocker_1_original_true_stop_transcript",
                "blocker_2_labelled_original_full_frames",
                "blocker_3_original_viewport_crops",
            ],
            "diffs_run": False,
            "parity_claimed": False,
        },
        "not_claimed": [
            "original-vs-Firestaff pixel parity",
            "paired diff masks/stats",
            "new original FIRES F0128/F0097/VIDRV true stop",
            "semantically matched original gameplay capture",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": manifest["status"], "manifest": str(MANIFEST.relative_to(ROOT)), "plan": str(PLAN.relative_to(ROOT)), "firestaff_side_ready": firestaff_ok}, indent=2))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
