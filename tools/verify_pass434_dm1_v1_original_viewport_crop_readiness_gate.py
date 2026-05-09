#!/usr/bin/env python3
"""Pass434 gate: DM1 V1 original viewport crop/source-lock readiness.

Evidence/readiness gate only. It verifies N2-local ReDMCSB source anchors,
original capture/crop tooling, and existing 224x136 original viewport crop
manifests remain discoverable and executable. It does not launch DOSBox or
claim pixel parity.
"""
from __future__ import annotations

import json
import os
import shutil
import struct
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass434_dm1_v1_original_viewport_crop_readiness_gate"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
ORIGINAL_DM = Path.home() / ".openclaw/data/firestaff-original-games/DM"

SOURCE_ANCHORS: list[dict[str, Any]] = [
    {
        "id": "movement_route_binds_original_frames_to_party_tuple",
        "file": "CLIKMENU.C",
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "lines": "142-174,180-347",
        "needles": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0284_CHAMPION_SetPartyDirection",
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks",
        ],
        "claim": "accepted turns/steps mutate the party tuple before any original viewport crop can be promoted",
    },
    {
        "id": "f0128_composes_g0296_for_known_tuple",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "lines": "8318-8611",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "G0296_puc_Bitmap_Viewport",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
            "F0127_DUNGEONVIEW_DrawSquareD0C",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)",
        ],
        "claim": "the 224x136 crop is meaningful only after F0128 draws G0296 for direction/X/Y",
    },
    {
        "id": "f0097_presents_pc34_viewport",
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "lines": "709-858",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "F0638_GetZone(C007_ZONE_VIEWPORT",
            "G0296_puc_Bitmap_Viewport",
            "VIDRV_09_BlitViewPort",
        ],
        "claim": "capture readiness is locked to the PC34 viewport-present seam, not setup echo/menu text",
    },
]

REQUIRED_FILES = [
    "scripts/dosbox_dm1_original_viewport_reference_capture.sh",
    "tools/pass86_original_viewport_crop_manifest.py",
    "tools/verify_pass376_dm1_v1_original_artifact_command_manifest.py",
    "tools/verify_pass377_dm1_v1_paired_diff_artifact_blocker.py",
    "verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv",
    "verification-screens/pass376-original-dm1-viewports/pass86_original_viewport_crop_manifest.json",
]

EXPECTED_CROPS = [
    "01_ingame_start_original_viewport_224x136",
    "02_ingame_turn_right_original_viewport_224x136",
    "03_ingame_move_forward_original_viewport_224x136",
    "04_ingame_spell_panel_original_viewport_224x136",
    "05_ingame_after_cast_original_viewport_224x136",
    "06_ingame_inventory_panel_original_viewport_224x136",
]


def run(cmd: list[str]) -> dict[str, Any]:
    proc = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False)
    return {"cmd": cmd, "returncode": proc.returncode, "output_tail": proc.stdout[-4000:]}


def norm(text: str) -> str:
    return " ".join(text.split())


def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            start, end = [int(x) for x in part.split("-", 1)]
        else:
            start = end = int(part)
        chunks.append("\n".join(lines[start - 1:end]))
    return "\n".join(chunks)


def audit_sources() -> list[dict[str, Any]]:
    rows = []
    for anchor in SOURCE_ANCHORS:
        path = REDMCSB / anchor["file"]
        text = source_window(path, anchor["lines"]) if path.exists() else ""
        missing = [needle for needle in anchor["needles"] if norm(needle) not in norm(text)]
        rows.append({**anchor, "path": str(path), "exists": path.exists(), "ok": path.exists() and not missing, "missing": missing})
    return rows


def png_dims(path: Path) -> tuple[int, int] | None:
    try:
        data = path.read_bytes()[:24]
    except OSError:
        return None
    if data[:8] != b"\x89PNG\r\n\x1a\n" or data[12:16] != b"IHDR":
        return None
    return struct.unpack(">II", data[16:24])


def parse_crop_manifest(path: Path) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    if not path.exists():
        return rows
    for line in path.read_text(encoding="utf-8").splitlines():
        if not line or line.startswith("#"):
            continue
        parts = line.split("\t")
        if len(parts) != 6:
            rows.append({"raw": line, "parse_error": True})
            continue
        kind, filename, width, height, size, sha = parts
        rows.append({"kind": kind, "filename": filename, "width": int(width), "height": int(height), "bytes": int(size), "sha256": sha})
    return rows


def crop_artifacts() -> dict[str, Any]:
    base = ROOT / "verification-screens/pass376-original-dm1-viewports"
    manifest = base / "original_viewport_224x136_manifest.tsv"
    rows = parse_crop_manifest(manifest)
    files = []
    for stem in EXPECTED_CROPS:
        png = base / "viewport_224x136" / f"{stem}.png"
        ppm = base / "viewport_224x136" / f"{stem}.ppm"
        dims = png_dims(png) if png.exists() else None
        files.append({
            "stem": stem,
            "png": str(png.relative_to(ROOT)),
            "png_exists": png.exists(),
            "png_dims": list(dims) if dims else None,
            "ppm": str(ppm.relative_to(ROOT)),
            "ppm_exists": ppm.exists(),
            "manifest_row": any(r.get("filename") == ppm.name for r in rows),
        })
    return {
        "manifest": str(manifest.relative_to(ROOT)),
        "manifest_exists": manifest.exists(),
        "rows": rows,
        "row_count": len(rows),
        "rows_all_224x136": len(rows) == 6 and all(r.get("kind") == "original_viewport_224x136" and r.get("width") == 224 and r.get("height") == 136 and len(str(r.get("sha256", ""))) == 64 for r in rows),
        "files": files,
        "files_all_present": all(f["png_exists"] and f["ppm_exists"] and f["manifest_row"] and f["png_dims"] == [224, 136] for f in files),
    }


def prior_manifest_probe() -> dict[str, Any]:
    path = ROOT / "parity-evidence/verification/pass376_dm1_v1_original_artifact_command_manifest/manifest.json"
    if not path.exists():
        return {"path": str(path.relative_to(ROOT)), "exists": False, "status": None, "ok": False}
    data = json.loads(path.read_text(encoding="utf-8"))
    return {"path": str(path.relative_to(ROOT)), "exists": True, "status": data.get("status"), "ok": data.get("status") == "BLOCKED_PASS376_ORIGINAL_FRAMES_CROPS_NARROWED"}


def tooling_probe() -> dict[str, Any]:
    files = [{"path": rel, "exists": (ROOT / rel).exists()} for rel in REQUIRED_FILES]
    capture_script = ROOT / "scripts/dosbox_dm1_original_viewport_reference_capture.sh"
    capture_text = capture_script.read_text(encoding="utf-8") if capture_script.exists() else ""
    pass86 = run(["python3", "tools/pass86_original_viewport_crop_manifest.py", "--self-test"])
    return {
        "required_files": files,
        "capture_script_crop_lock": {
            "path": "scripts/dosbox_dm1_original_viewport_reference_capture.sh",
            "has_normalize_only": "--normalize-only" in capture_text,
            "has_pillow_crop_0_33_224_169": "crop = im.crop((0, 33, 224, 169))" in capture_text,
            "has_imagemagick_crop": "-crop 224x136+0+33" in capture_text,
            "has_manifest_geometry_check": "expected exactly 6 normalized viewport PPM crops" in capture_text and "(width, height) != (224, 136)" in capture_text,
        },
        "pass86_self_test": pass86,
        "pass376_prior_manifest": prior_manifest_probe(),
        "dosbox_available": shutil.which("dosbox") is not None,
        "xvfb_run_available": shutil.which("xvfb-run") is not None,
        "dosbox_required_for_this_gate": False,
        "original_dm_root_exists": ORIGINAL_DM.exists(),
        "dm1_pc34_stage_exists": (ORIGINAL_DM / "_extracted/dm-pc34/DungeonMasterPC34/DM.EXE").exists(),
    }


def write_report(data: dict[str, Any]) -> None:
    lines = [
        "# Pass434 — DM1 V1 original viewport crop readiness gate",
        "",
        f"Status: `{data['status']}`",
        "",
        "This gate keeps the original viewport crop/source-lock prerequisites executable without claiming pixel parity or requiring a live DOSBox run.",
        "",
        "## ReDMCSB source audit",
        "",
    ]
    for row in data["source_audit"]:
        lines.append(f"- `{row['file']}:{row['lines']}` `{row['function']}` — ok=`{row['ok']}`; {row['claim']}")
    crop = data["crop_artifacts"]
    tooling = data["tooling"]
    lines.extend([
        "",
        "## Existing crop artifacts",
        "",
        f"- manifest: `{crop['manifest']}`; rows=`{crop['row_count']}`; rows_all_224x136=`{crop['rows_all_224x136']}`",
        f"- six PNG/PPM crop pairs present and manifest-bound: `{crop['files_all_present']}`",
        "",
        "## Executable tooling",
        "",
        f"- pass86 self-test return code: `{tooling['pass86_self_test']['returncode']}`",
        f"- pass376 prior manifest status: `{tooling['pass376_prior_manifest']['status']}`; ok=`{tooling['pass376_prior_manifest']['ok']}`",
        f"- capture script crop locks: `{tooling['capture_script_crop_lock']}`",
        f"- DOSBox available: `{tooling['dosbox_available']}`; xvfb-run available: `{tooling['xvfb_run_available']}`; required for this gate: `False`",
        "",
        "## Blocker honesty",
        "",
        "- Existing pass376 crops are mechanical/review inputs only until the original route is semantically clean.",
        "- This gate does not launch DOSBox and does not claim original-vs-Firestaff pixel parity.",
        "- If DOSBox/live emulator is unavailable, promotion remains blocked at the capture/promotion step, not at this readiness gate.",
        "",
        f"Manifest: `{MANIFEST.relative_to(ROOT)}`",
    ])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    sources = audit_sources()
    crops = crop_artifacts()
    tooling = tooling_probe()
    required_ok = all(f["exists"] for f in tooling["required_files"])
    capture_ok = all(tooling["capture_script_crop_lock"][k] for k in ["has_normalize_only", "has_pillow_crop_0_33_224_169", "has_imagemagick_crop", "has_manifest_geometry_check"])
    ok = all(row["ok"] for row in sources) and required_ok and capture_ok and tooling["pass86_self_test"]["returncode"] == 0 and tooling["pass376_prior_manifest"]["ok"] and crops["rows_all_224x136"] and crops["files_all_present"]
    data = {
        "schema": f"{PASS}.v1",
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "status": "PASS_PASS434_ORIGINAL_VIEWPORT_CROP_READINESS" if ok else "FAIL_PASS434_ORIGINAL_VIEWPORT_CROP_READINESS",
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"])["output_tail"].strip(),
        "head": run(["git", "rev-parse", "HEAD"])["output_tail"].strip(),
        "source_root": str(REDMCSB),
        "source_audit": sources,
        "crop_artifacts": crops,
        "tooling": tooling,
        "not_claimed": ["DOSBox/live original runtime capture", "semantic original route promotion", "original-vs-Firestaff pixel parity", "HUD/overlay parity"],
    }
    MANIFEST.write_text(json.dumps(data, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(data)
    print(json.dumps({"status": data["status"], "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT))}, indent=2))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
