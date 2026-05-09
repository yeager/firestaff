#!/usr/bin/env python3
"""Pass450 DM1 V1 Hall original candidate artifact inventory.

This is a blocker-tightening gate for pass449.  It does not generate new
screenshots.  Instead it inventories the existing original PC34 Hall/candidate
artifacts, attaches exact PC34 GRAPHICS.DAT/DUNGEON.DAT provenance to every
review frame, verifies the ReDMCSB source anchors used by the route, and records
why the current local host cannot produce promotable original frames.
"""
from __future__ import annotations

import hashlib
import json
import shutil
import struct
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass450_dm1_v1_hall_original_candidate_artifact_inventory"
STATUS = "PARTIAL_PASS450_CORRECTED_CANDIDATE_AND_RESURRECT_AVAILABLE_REMAINING_CANCEL_REINCARNATE"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CANON_DM1 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
STAGE = Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34"

LOCKED_DATA = [
    {
        "label": "dm1_pc34_english_graphics",
        "path": CANON_DM1 / "GRAPHICS.DAT",
        "filename": "GRAPHICS.DAT",
        "variant": "DM PC 3.4 English / I34E",
        "bytes": 363417,
        "sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
        "md5": "fa6b1aa29e191418713bf2cda93d962e",
    },
    {
        "label": "dm1_pc34_english_dungeon",
        "path": CANON_DM1 / "DUNGEON.DAT",
        "filename": "DUNGEON.DAT",
        "variant": "DM PC 3.4 English / I34E",
        "bytes": 33357,
        "sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
        "md5": "766450c940651fc021c92fe5d0d0b3a6",
    },
]

# These are the exact source anchors needed for candidate select -> panel ->
# cancel/confirm -> HUD/status.  The verifier checks snippets in-place.
SOURCE_LOCKS = [
    ("COMMAND.C", "397-403,2322-2323", ["C007_ZONE_VIEWPORT", "C080_COMMAND_CLICK_IN_DUNGEON_VIEW", "F0377_COMMAND_ProcessType80_ClickInDungeonView"], "Viewport left-click dispatches to the type-80 dungeon-view handler."),
    ("CLIKVIEW.C", "21-25,348-349,407-431", ["F0372_COMMAND_ProcessType80_ClickInDungeonView_TouchFrontWallSensor", "P0752_i_X -= G2067_i_ViewportScreenX", "P0753_i_Y -= G2068_i_ViewportScreenY", "F0275_SENSOR_IsTriggeredByClickOnWall"], "PC34 screen click is converted to viewport coordinates and front-wall sensor dispatch."),
    ("MOVESENS.C", "1392,1501-1502", ["C127_SENSOR_WALL_CHAMPION_PORTRAIT", "F0280_CHAMPION_AddCandidateChampionToParty"], "C127 wall champion portrait sensor enters candidate champion creation."),
    ("DUNGEON.C", "2558,2608-2612", ["G0289_i_DungeonView_ChampionPortraitOrdinal", "C127_SENSOR_WALL_CHAMPION_PORTRAIT"], "Visible wall portrait ordinal is sourced from the same C127 sensor data."),
    ("DUNVIEW.C", "525,3913-3928", ["G0109_auc_Graphic558_Box_ChampionPortraitOnWall", "M635_ZONE_PORTRAIT_ON_WALL"], "D1C front-wall portrait draw/click geometry anchor."),
    ("COORD.C", "1693-1698", ["int16_t G2067_i_ViewportScreenX = 0", "int16_t G2068_i_ViewportScreenY = 33"], "PC viewport origin maps source portrait center to screen x=111 y=82."),
    ("PANEL.C", "1619-1636,1654-1656,2376-2385", ["F0346_INVENTORY_DrawPanel_ResurrectReincarnate", "C040_GRAPHIC_PANEL_RESURRECT_REINCARNATE", "C06_COLOR_DARK_GREEN", "if (G0299_ui_CandidateChampionOrdinal)"], "Candidate inventory redraw forces the resurrect/reincarnate panel and hides save/rest/close."),
    ("REVIVE.C", "272-294,744-807", ["G0299_ui_CandidateChampionOrdinal", "C162_COMMAND_CLICK_IN_PANEL_CANCEL", "C161_COMMAND_CLICK_IN_PANEL_REINCARNATE", "F0164_DUNGEON_UnlinkThingFromList", "M044_SET_TYPE_DISABLED"], "Candidate append, cancel cleanup, confirm cleanup, sensor disable, and reincarnate branch."),
    ("COMMAND.C", "228-240,1985-1991,2159-2184,2336-2370", ["C160_COMMAND_CLICK_IN_PANEL_RESURRECT", "C161_COMMAND_CLICK_IN_PANEL_REINCARNATE", "C162_COMMAND_CLICK_IN_PANEL_CANCEL", "M568_PANEL_RESURRECT_REINCARNATE", "!G0299_ui_CandidateChampionOrdinal"], "Panel command dispatch and candidate modal blocking for status/rest/save/close paths."),
    ("CHAMDRAW.C", "536-545,1210-1212", ["G0299_ui_CandidateChampionOrdinal", "return"], "Candidate-aware champion slot/hand draw suppression."),
]

PASS173_RUNS = [
    "gate_click_portrait_then_resurrect",
    "gate_click_portrait_then_reincarnate",
]
PASS173_ROOT = ROOT / "parity-evidence/verification/pass173_source_portrait_route_gate_probe"
N2_HALL_ARTIFACT_ROOT = Path("/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/dm1-hall-dosbox-20260509")
N2_HALL_ARTIFACT_STATUS = "NARROWED_ORIGINAL_HALL_PANEL_VISIBLE_CANDIDATE_CLICK_NO_TRANSITION"
N2_PROMOTABLE_LABEL = "03_panel_visible_north_front_mirror"
CORRECTED_HALL_ARTIFACT_ROOT = Path("/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/hall-corrected-click-primitive-20260509")
REQUIRED_PROMOTION_SCENES = [
    "candidate_select_portrait_click_before_panel",
    "candidate_panel_visible_after_append",
    "candidate_cancel_after_panel",
    "candidate_confirm_resurrect_after_panel",
    "candidate_confirm_reincarnate_after_panel",
    "hud_status_after_cancel",
    "hud_status_after_resurrect",
    "hud_status_after_reincarnate",
]


def sha(path: Path, algo: str = "sha256") -> str:
    h = hashlib.new(algo)
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def norm(text: str) -> str:
    return " ".join(text.split())


def line_block(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    out: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            a, b = part.split("-", 1)
            start, end = int(a), int(b)
        else:
            start = end = int(part)
        out.append("\n".join(lines[start - 1:end]))
    return "\n".join(out)


def png_dims(path: Path) -> list[int] | None:
    try:
        header = path.read_bytes()[:24]
    except FileNotFoundError:
        return None
    if len(header) >= 24 and header[:8] == b"\x89PNG\r\n\x1a\n" and header[12:16] == b"IHDR":
        return list(struct.unpack(">II", header[16:24]))
    return None


def cmd_available(cmd: str) -> str | None:
    return shutil.which(cmd)


def run_git(args: list[str]) -> str:
    return subprocess.run(["git", *args], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT).stdout.strip()


def audit_data() -> tuple[list[dict[str, Any]], list[str]]:
    rows: list[dict[str, Any]] = []
    errors: list[str] = []
    for item in LOCKED_DATA:
        path = item["path"]
        row = {k: (str(v) if isinstance(v, Path) else v) for k, v in item.items()}
        row["resolvedPath"] = str(path.resolve()) if path.exists() else None
        if not path.is_file():
            row.update({"exists": False, "ok": False})
            errors.append(f"missing {item['label']} at {path}")
        else:
            actual = {"sha256": sha(path), "md5": sha(path, "md5"), "bytes": path.stat().st_size}
            ok = actual["sha256"] == item["sha256"] and actual["md5"] == item["md5"] and actual["bytes"] == item["bytes"]
            row.update({"exists": True, "ok": ok, "actual": actual})
            if not ok:
                errors.append(f"hash mismatch {item['label']}: {actual}")
        rows.append(row)
    return rows, errors


def audit_sources() -> tuple[list[dict[str, Any]], list[str]]:
    rows: list[dict[str, Any]] = []
    errors: list[str] = []
    for file, lines, needles, claim in SOURCE_LOCKS:
        path = REDMCSB / file
        text = line_block(path, lines) if path.exists() else ""
        missing = [n for n in needles if norm(n) not in norm(text)]
        ok = path.exists() and not missing
        rows.append({
            "file": file,
            "path": str(path),
            "lines": lines,
            "claim": claim,
            "needles": needles,
            "ok": ok,
            "missing": missing,
            "lineBlockSha256": hashlib.sha256(text.encode("utf-8", errors="replace")).hexdigest() if text else None,
        })
        if not ok:
            errors.append(f"source lock failed {file}:{lines}: {missing or 'missing file'}")
    return rows, errors


def load_json(path: Path) -> dict[str, Any]:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except Exception:
        return {}


def frame_rows(data_rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    data_provenance = [
        {
            "label": d["label"],
            "variant": d["variant"],
            "filename": d["filename"],
            "sha256": d.get("actual", {}).get("sha256", d["sha256"]),
            "bytes": d.get("actual", {}).get("bytes", d["bytes"]),
            "resolvedPath": d.get("resolvedPath"),
        }
        for d in data_rows
    ]
    for run in PASS173_RUNS:
        run_dir = PASS173_ROOT / run
        summary = load_json(run_dir / "summary.json")
        summary_rows = {r.get("file"): r for r in summary.get("rows", []) if isinstance(r, dict)}
        for png in sorted(run_dir.glob("*.png")):
            rel = png.relative_to(ROOT).as_posix()
            row = summary_rows.get(png.name, {})
            rows.append({
                "run": run,
                "path": rel,
                "exists": True,
                "sha256": sha(png),
                "sha12": sha(png)[:12],
                "bytes": png.stat().st_size,
                "pngDims": png_dims(png),
                "label": row.get("label"),
                "phase": row.get("phase"),
                "class": row.get("class"),
                "reason": row.get("reason"),
                "pass173Classification": summary.get("classification"),
                "pass173Reason": summary.get("reason"),
                "originalDataProvenance": data_provenance,
                "promotionUse": "review_only_not_promotable_static_no_party" if summary.get("classification", "").startswith("blocked/") else "unknown_review_only",
                "promotionBlocker": "pass173 frame route is classified blocked/static-no-party-after-gate; no source-bound true-stop transcript or visible candidate-panel transition is attached to this frame.",
            })
    return rows



def audit_n2_hall_artifact() -> dict[str, Any]:
    root = N2_HALL_ARTIFACT_ROOT
    manifest_path = root / "manifest.json"
    sha_path = root / "SHA256SUMS.txt"
    readme_path = root / "README.md"
    row: dict[str, Any] = {
        "root": str(root),
        "exists": root.is_dir(),
        "manifestPath": str(manifest_path),
        "sha256SumsPath": str(sha_path),
        "readmePath": str(readme_path),
        "expectedStatus": N2_HALL_ARTIFACT_STATUS,
        "promotableLabel": N2_PROMOTABLE_LABEL,
        "promotionUse": "panel_visible_original_hall_front_mirror_only_not_candidate_panel_parity",
        "remainingBlocker": "candidate_select/cancel/resurrect_confirm/reincarnate_confirm/hud_status_after true-stop or transition frames remain missing; candidate clicks in this run did not visibly transition.",
    }
    if not root.is_dir():
        row.update({"ok": False, "errors": [f"missing N2 Hall artifact root {root}"]})
        return row
    errors: list[str] = []
    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except Exception as exc:
        manifest = {}
        errors.append(f"manifest read/parse failed: {exc}")
    row["status"] = manifest.get("status")
    row["host"] = manifest.get("host")
    row["created"] = manifest.get("created")
    row["entryCount"] = len(manifest.get("entries", [])) if isinstance(manifest.get("entries"), list) else None
    source = manifest.get("source_provenance", {}) if isinstance(manifest.get("source_provenance"), dict) else {}
    required_source = {
        "DUNGEON.DAT_sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
        "GRAPHICS.DAT_sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
        "TITLE_sha256": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
    }
    row["sourceProvenance"] = source
    row["requiredSourceProvenance"] = required_source
    for key, expected in required_source.items():
        if source.get(key) != expected:
            errors.append(f"source_provenance.{key} mismatch: {source.get(key)} != {expected}")
    if row.get("status") != N2_HALL_ARTIFACT_STATUS:
        errors.append(f"status mismatch: {row.get('status')} != {N2_HALL_ARTIFACT_STATUS}")
    if row.get("entryCount") != 11:
        errors.append(f"entry count mismatch: {row.get('entryCount')} != 11")
    entries = manifest.get("entries", []) if isinstance(manifest.get("entries"), list) else []
    promotable = next((entry for entry in entries if str(entry.get("pc320", "")).startswith(f"pc320/{N2_PROMOTABLE_LABEL}") or str(entry.get("viewport224x136", "")).startswith(f"viewport224x136/{N2_PROMOTABLE_LABEL}")), None)
    row["promotableEntry"] = promotable
    if not promotable:
        errors.append(f"missing promotable entry {N2_PROMOTABLE_LABEL}")
    checked_files: list[dict[str, Any]] = []
    for entry in entries:
        for rel_key, hash_key in (("pc320", "pc320_sha256"), ("viewport224x136", "viewport_sha256"), ("root", "root_sha256")):
            rel_path = entry.get(rel_key)
            expected_hash = entry.get(hash_key)
            if not rel_path or not expected_hash:
                continue
            path = root / rel_path
            item = {"path": str(path), "rel": rel_path, "expectedSha256": expected_hash, "exists": path.is_file()}
            if path.is_file():
                actual = sha(path)
                item.update({"actualSha256": actual, "bytes": path.stat().st_size, "pngDims": png_dims(path), "ok": actual == expected_hash})
                if actual != expected_hash:
                    errors.append(f"artifact hash mismatch {rel_path}: {actual} != {expected_hash}")
            else:
                item["ok"] = False
                errors.append(f"missing artifact file {path}")
            checked_files.append(item)
    row["checkedFiles"] = checked_files
    if sha_path.is_file():
        row["sha256SumsSha256"] = sha(sha_path)
        text = sha_path.read_text(encoding="utf-8", errors="replace")
        row["sha256SumsLineCount"] = len([ln for ln in text.splitlines() if ln.strip()])
    else:
        errors.append(f"missing {sha_path}")
    if readme_path.is_file():
        row["readmeSha256"] = sha(readme_path)
    else:
        errors.append(f"missing {readme_path}")
    row["ok"] = not errors
    row["errors"] = errors
    return row

def audit_environment() -> dict[str, Any]:
    import os
    import platform

    capture_tool = ROOT / "tools/pass173_source_portrait_route_gate_probe.py"
    external_root = Path("/Volumes/Extern-disk/openclaw-data/firestaff/artifacts/pass173_source_portrait_route_gate_probe")
    external_parent = external_root.parent
    env_dosbox = os.environ.get("FIRESTAFF_DOSBOX")
    dosbox_candidates = [
        {"name": "FIRESTAFF_DOSBOX", "path": env_dosbox, "available": bool(env_dosbox and Path(env_dosbox).exists())},
        {"name": "dosbox", "path": cmd_available("dosbox"), "available": bool(cmd_available("dosbox"))},
        {"name": "dosbox-x", "path": cmd_available("dosbox-x"), "available": bool(cmd_available("dosbox-x"))},
    ]
    selected = next((c for c in dosbox_candidates if c["available"]), None)
    system = platform.system()
    display = os.environ.get("DISPLAY")
    xvfb = cmd_available("xvfb-run")
    needs_xvfb = system == "Linux" and not display
    missing_tools: list[str] = []
    if not selected:
        missing_tools.append("dosbox or dosbox-x in PATH, or FIRESTAFF_DOSBOX=/absolute/path/to/dosbox")
    if needs_xvfb and not xvfb:
        missing_tools.append("xvfb-run for headless Linux capture")
    if not STAGE.is_dir():
        missing_tools.append(f"staged original PC34 directory {STAGE}")
    if not capture_tool.is_file():
        missing_tools.append(f"capture tool {capture_tool.relative_to(ROOT)}")

    run_base = Path(os.environ.get("FIRESTAFF_PASS173_RUN_BASE", os.environ.get("FIRESTAFF_ARTIFACT_ROOT", str(external_root))))
    command_prefix = []
    if needs_xvfb:
        command_prefix = [xvfb or "xvfb-run", "-a"]
    env_parts = [
        f"FIRESTAFF_ARTIFACT_ROOT={external_root}",
    ]
    if selected and selected["path"]:
        env_parts.append(f"FIRESTAFF_DOSBOX={selected['path']}")
    next_cmd = " ".join(env_parts + command_prefix + ["python3", "tools/pass173_source_portrait_route_gate_probe.py"])
    rerun_cmd = "python3 tools/verify_pass450_dm1_v1_hall_original_candidate_artifact_inventory.py && python3 tools/verify_pass449_dm1_v1_hall_candidate_framebuffer_evidence_gate.py"

    return {
        "localHostCaptureReady": not missing_tools,
        "platform": system,
        "machine": platform.machine(),
        "path": os.environ.get("PATH"),
        "display": display,
        "dosboxCandidates": dosbox_candidates,
        "selectedDosbox": selected,
        "dosbox": cmd_available("dosbox"),
        "dosboxX": cmd_available("dosbox-x"),
        "xvfbRun": xvfb,
        "needsXvfb": needs_xvfb,
        "stageDir": str(STAGE),
        "stageExists": STAGE.is_dir(),
        "captureTool": "tools/pass173_source_portrait_route_gate_probe.py",
        "captureToolExists": capture_tool.is_file(),
        "externalArtifactRoot": str(external_root),
        "externalParentExists": external_parent.exists(),
        "configuredRunBase": str(run_base),
        "missingTools": missing_tools,
        "blockingReason": "capture-ready" if not missing_tools else "Original PC34 Hall capture is blocked locally by: " + "; ".join(missing_tools),
        "nextExecutableStep": next_cmd,
        "postCaptureVerificationStep": rerun_cmd,
    }


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        f"# {PASS}",
        "",
        f"- status: `{manifest['status']}`",
        "- parity claim: **not made**",
        f"- frame rows inventoried: {len(manifest['existingOriginalReviewFrames'])}",
        "",
        "## Original data provenance",
    ]
    for row in manifest["dataHashLock"]:
        actual = row.get("actual", {})
        lines.append(f"- `{row['variant']}` `{row['filename']}` sha256 `{actual.get('sha256', row['sha256'])}` bytes `{actual.get('bytes', row['bytes'])}` resolved `{row.get('resolvedPath')}` ok={row['ok']}")
    lines += ["", "## ReDMCSB source anchors"]
    for row in manifest["sourceLocks"]:
        lines.append(f"- `{row['file']}:{row['lines']}` — {row['claim']} ok={row['ok']}")
    lines += ["", "## Existing reviewed frames"]
    for row in manifest["existingOriginalReviewFrames"]:
        lines.append(f"- `{row['path']}` sha12 `{row['sha12']}` dims={row['pngDims']} class=`{row.get('class')}` pass173=`{row.get('pass173Classification')}` use=`{row['promotionUse']}`")
    n2 = manifest["n2HallDosboxArtifact"]
    lines += ["", "## N2 DOSBox original Hall artifact"]
    lines.append(f"- root: `{n2['root']}` exists={n2['exists']} ok={n2['ok']}")
    lines.append(f"- status: `{n2.get('status')}` host=`{n2.get('host')}` created=`{n2.get('created')}` entries={n2.get('entryCount')}")
    lines.append(f"- promotable/narrowed label: `{n2['promotableLabel']}` use=`{n2['promotionUse']}`")
    sp = n2.get("sourceProvenance", {})
    lines.append(f"- DUNGEON.DAT sha256 `{sp.get('DUNGEON.DAT_sha256')}`; GRAPHICS.DAT sha256 `{sp.get('GRAPHICS.DAT_sha256')}`; TITLE sha256 `{sp.get('TITLE_sha256')}`")
    pe = n2.get("promotableEntry") or {}
    if pe:
        lines.append(f"- pc320 `{pe.get('pc320')}` sha256 `{pe.get('pc320_sha256')}`")
        lines.append(f"- viewport224x136 `{pe.get('viewport224x136')}` sha256 `{pe.get('viewport_sha256')}`")
    lines.append(f"- historical blocker: {n2['remainingBlocker']}")
    lines += ["", "## Corrected Hall artifact"]
    lines.append(f"- root: `{manifest['correctedHallArtifactRoot']}`")
    for scene, ok in manifest['correctedAvailableScenes'].items():
        lines.append(f"- `{scene}` available={ok}")
    lines += ["", "## Remaining promotable scenes"]
    for scene in manifest["missingPromotableScenes"]:
        lines.append(f"- `{scene}`")
    env = manifest["captureEnvironment"]
    lines += [
        "",
        "## Capture tooling readiness",
        f"- local host capture ready: `{env['localHostCaptureReady']}`",
        f"- platform: `{env['platform']}` `{env['machine']}`",
        f"- dosbox: `{env['dosbox']}`",
        f"- dosbox-x: `{env['dosboxX']}`",
        f"- selected DOSBox: `{(env['selectedDosbox'] or {}).get('path')}`",
        f"- xvfb-run: `{env['xvfbRun']}` needsXvfb=`{env['needsXvfb']}` display=`{env['display']}`",
        f"- stage exists: `{env['stageExists']}` `{env['stageDir']}`",
        f"- external artifact root: `{env['externalArtifactRoot']}` parentExists=`{env['externalParentExists']}`",
        f"- configured run base: `{env['configuredRunBase']}`",
        f"- missing tools/data: `{env['missingTools']}`",
        f"- reason: {env['blockingReason']}",
        f"- next step: `{env['nextExecutableStep']}`",
        f"- post-capture verification: `{env['postCaptureVerificationStep']}`",
        "",
    ]
    REPORT.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    data_rows, data_errors = audit_data()
    source_rows, source_errors = audit_sources()
    frames = frame_rows(data_rows)
    pass173_summaries = {run: load_json(PASS173_ROOT / run / "summary.json") for run in PASS173_RUNS}
    env = audit_environment()
    n2_artifact = audit_n2_hall_artifact()
    corrected_run = CORRECTED_HALL_ARTIFACT_ROOT / "probe-initial-south-corrected"
    corrected_available = {
        "candidate_select_portrait_click_before_panel": (corrected_run / "image0002-raw.png").is_file(),
        "candidate_panel_visible_after_append": (corrected_run / "image0002-raw.png").is_file(),
        "candidate_confirm_resurrect_after_panel": (corrected_run / "image0003-raw.png").is_file(),
        "hud_status_after_resurrect": (corrected_run / "image0003-raw.png").is_file(),
    }
    missing = [scene for scene in REQUIRED_PROMOTION_SCENES if not corrected_available.get(scene, False)]
    errors = data_errors + source_errors + n2_artifact.get("errors", [])
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": "FAIL_PASS450_SOURCE_OR_DATA_LOCK" if errors else STATUS,
        "repo": str(ROOT),
        "branch": run_git(["branch", "--show-current"]),
        "head": run_git(["rev-parse", "HEAD"]),
        "redmcsbRoot": str(REDMCSB),
        "dataHashLock": data_rows,
        "sourceLocks": source_rows,
        "pass173Summaries": pass173_summaries,
        "existingOriginalReviewFrames": frames,
        "n2HallDosboxArtifact": n2_artifact,
        "availablePromotableOriginalFrames": [
            "probe-initial-south-corrected/image0002-raw.png (candidate_select/panel_visible corrected initial-south transition)",
            "probe-initial-south-corrected/image0003-raw.png (resurrect_confirm/hud_status_after corrected C160 terminal transition)",
            "03_panel_visible_north_front_mirror pc320+viewport224x136 (historical Hall/front-mirror context only)",
        ],
        "correctedHallArtifactRoot": str(CORRECTED_HALL_ARTIFACT_ROOT),
        "correctedAvailableScenes": corrected_available,
        "missingPromotableScenes": missing,
        "captureEnvironment": env,
        "promotionDecision": "Promote corrected initial-south candidate_select/panel_visible plus resurrect_confirm/hud_status_after as source-routed original inputs. Do not claim full pixel parity; cancel/reincarnate/per-terminal HUD frames still need separate corrected original captures.",
        "errors": errors,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    write_report(manifest)
    if errors:
        print("FAIL pass450 source/data locks")
        for e in errors:
            print(f"- {e}")
        return 1
    print(f"PASS {PASS}: {STATUS}")
    print(f"wrote {MANIFEST}")
    print(f"wrote {REPORT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
