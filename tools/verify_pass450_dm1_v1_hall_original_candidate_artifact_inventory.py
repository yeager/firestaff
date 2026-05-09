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
STATUS = "BLOCKED_PASS450_ORIGINAL_PC34_HALL_CANDIDATE_FRAMES_NOT_PROMOTABLE"
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


def audit_environment() -> dict[str, Any]:
    return {
        "localHostCaptureReady": False,
        "dosbox": cmd_available("dosbox"),
        "xvfbRun": cmd_available("xvfb-run"),
        "display": __import__("os").environ.get("DISPLAY"),
        "stageDir": str(STAGE),
        "stageExists": STAGE.is_dir(),
        "captureTool": "tools/pass173_source_portrait_route_gate_probe.py",
        "captureToolExists": (ROOT / "tools/pass173_source_portrait_route_gate_probe.py").is_file(),
        "blockingReason": "Local macOS worktree has no dosbox/xvfb-run in PATH, and pass173 currently hardcodes DOSBOX=/usr/bin/dosbox. Generating new original PC34 frames here would be non-deterministic/unavailable.",
        "nextExecutableStep": "On N2/Linux with /usr/bin/dosbox and xvfb-run available, run: xvfb-run -a python3 tools/pass173_source_portrait_route_gate_probe.py ; then rerun python3 tools/verify_pass450_dm1_v1_hall_original_candidate_artifact_inventory.py and promote only if pass173 no longer reports blocked/static-no-party-after-gate and all required pass449 scenes have labelled original fullframes/crops plus a true-stop transcript.",
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
    lines += ["", "## Missing promotable scenes"]
    for scene in manifest["missingPromotableScenes"]:
        lines.append(f"- `{scene}`")
    env = manifest["captureEnvironment"]
    lines += [
        "",
        "## Capture tooling blocker",
        f"- dosbox: `{env['dosbox']}`",
        f"- xvfb-run: `{env['xvfbRun']}`",
        f"- stage exists: `{env['stageExists']}` `{env['stageDir']}`",
        f"- reason: {env['blockingReason']}",
        f"- next step: `{env['nextExecutableStep']}`",
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
    missing = REQUIRED_PROMOTION_SCENES[:]
    errors = data_errors + source_errors
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
        "missingPromotableScenes": missing,
        "captureEnvironment": env,
        "promotionDecision": "Do not promote current original PC34 Hall frames/crops. They are static no-party review clues and lack source-bound true-stop/semantic labels for candidate panel visibility, cancel, resurrect confirm, reincarnate confirm, and HUD/status outcomes.",
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
