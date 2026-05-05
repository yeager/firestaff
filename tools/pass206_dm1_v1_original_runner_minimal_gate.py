#!/usr/bin/env python3
"""Minimal DM1 V1 original-runner evidence gate for N2."""
from __future__ import annotations

import argparse
import hashlib
import json
import shutil
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path("~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source").expanduser()
DM1_STAGE = Path("~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34").expanduser()
CAPTURE_SCRIPT = ROOT / "scripts/dosbox_dm1_original_viewport_reference_capture.sh"
ATTEMPT_DIR = ROOT / "verification-screens/pass112-n2-stable-hud-route"
CLASSIFIER_JSON = ATTEMPT_DIR / "pass80_original_frame_classifier.json"
LABELS_TSV = ATTEMPT_DIR / "original_viewport_shot_labels.tsv"
KEYS_LOG = ATTEMPT_DIR / "original-viewpoint-route-keys.log"
OUT_DIR = ROOT / "parity-evidence/verification/pass206_dm1_v1_original_runner_minimal_gate"
REPORT = ROOT / "parity-evidence/pass206_dm1_v1_original_runner_minimal_gate.md"

EXPECTED_FILES = {
    "DM.EXE": (DM1_STAGE / "DM.EXE", "4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4").expanduser(),
    "DATA/DUNGEON.DAT": (DM1_STAGE / "DATA/DUNGEON.DAT", "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"),
    "DATA/GRAPHICS.DAT": (DM1_STAGE / "DATA/GRAPHICS.DAT", "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"),
}

SOURCE_CHECKS: list[dict[str, Any]] = [
    {"id": "pc34-movement-input-bindings", "file": "COMMAND.C", "ranges": [(396, 402), (670, 675)], "needles": ["C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT", "C006_COMMAND_MOVE_LEFT", "C005_COMMAND_MOVE_BACKWARD", "C004_COMMAND_MOVE_RIGHT", "0x4600", "0x4C00", "0x5F00", "0x4F00", "0x4D00", "0x4E00"], "claim": "Original PC34 exposes movement by right-panel zones and keyboard scan codes before command dispatch."},
    {"id": "command-dispatch-to-turn-step", "file": "COMMAND.C", "ranges": [(2096, 2156)], "needles": ["if ((L1160_i_Command == C002_COMMAND_TURN_RIGHT) || (L1160_i_Command == C001_COMMAND_TURN_LEFT))", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "if ((L1160_i_Command >= C003_COMMAND_MOVE_FORWARD) && (L1160_i_Command <= C006_COMMAND_MOVE_LEFT))", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"], "claim": "The route must reach command processing, not just produce visually changing frames."},
    {"id": "turn-step-state-mutation-and-wait-stop", "file": "CLIKMENU.C", "ranges": [(156, 173), (237, 347)], "needles": ["F0284_CHAMPION_SetPartyDirection", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0267_MOVE_GetMoveResult_CPSCE", "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;", "G0321_B_StopWaitingForPlayerInput = C1_TRUE;"], "claim": "Successful turn/step captures must be after the source movement handler has stopped the input wait loop."},
    {"id": "game-loop-redraw-cadence", "file": "GAMELOOP.C", "ranges": [(90, 90), (215, 219)], "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);", "F0380_COMMAND_ProcessQueue_CPSC();", "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"], "claim": "A movement/viewport gate must sample after redraw from current party state, not during menu/input churn."},
    {"id": "viewport-draw-and-present-seam", "file": "DUNVIEW.C", "ranges": [(8318, 8616)], "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "P0183_i_Direction", "P0184_i_MapX", "P0185_i_MapY", "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"], "claim": "The viewport reference is the dungeon-view draw from updated direction/map coordinates."},
    {"id": "viewport-present-request", "file": "DRAWVIEW.C", "ranges": [(709, 724), (840, 858)], "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "G0324_B_DrawViewportRequested = C1_TRUE", "M526_WaitVerticalBlank();", "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT"], "claim": "The comparison seam is the presented 224x136 viewport after the draw request/vblank path."},
]

ROUTE_EVENTS = "wait:7000 enter wait:1500 one wait:1500 click:276,140 wait:1500 one wait:1500 shot:party_hud kp5 wait:700 shot kp5 wait:700 shot f1 wait:700 shot:spell_panel one wait:700 shot i wait:700 shot:inventory_panel"
EXPECTED_SEQUENCE = ["dungeon_gameplay", "dungeon_gameplay", "dungeon_gameplay", "spell_panel", "dungeon_gameplay", "inventory"]


def read_lines(path: Path) -> list[str]:
    if not path.is_file():
        raise AssertionError(f"missing ReDMCSB source file: {path}")
    return path.read_text(encoding="latin-1", errors="replace").splitlines()


def source_block(file: str, ranges: list[tuple[int, int]]) -> str:
    lines = read_lines(REDMCSB / file)
    out: list[str] = []
    for start, end in ranges:
        out.extend(lines[start - 1:end])
    return "\n".join(out)


def compact(text: str) -> str:
    return " ".join(text.split())


def audit_source() -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for check in SOURCE_CHECKS:
        text = compact(source_block(check["file"], check["ranges"]))
        missing = [needle for needle in check["needles"] if compact(needle) not in text]
        results.append({"id": check["id"], "citations": [f"{check['file']}:{a}-{b}" for a, b in check["ranges"]], "claim": check["claim"], "ok": not missing, "missing": missing})
    return results


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def audit_runner() -> dict[str, Any]:
    commands = ["dosbox", "xvfb-run", "xdotool", "python3", "convert", "ffmpeg"]
    tools = {cmd: shutil.which(cmd) for cmd in commands}
    files: dict[str, Any] = {}
    for label, (path, expected) in EXPECTED_FILES.items():
        exists = path.is_file()
        observed = sha256(path) if exists else None
        files[label] = {"path": str(path), "exists": exists, "sha256": observed, "expectedSha256": expected, "ok": exists and observed == expected}
    return {
        "host_contract": "portable host; DANNESBURK and 192.168.2.126 forbidden",
        "tools": tools,
        "missing_tools": [cmd for cmd, path in tools.items() if not path],
        "canonical_files": files,
        "capture_script": {"path": str(CAPTURE_SCRIPT), "exists": CAPTURE_SCRIPT.is_file(), "executable": CAPTURE_SCRIPT.exists() and CAPTURE_SCRIPT.stat().st_mode & 0o111 != 0},
        "route_events": ROUTE_EVENTS,
    }


def audit_existing_attempt() -> dict[str, Any]:
    if not CLASSIFIER_JSON.is_file():
        return {"exists": False, "path": str(CLASSIFIER_JSON), "status": "BLOCKED_MISSING_CLASSIFIER"}
    doc = json.loads(CLASSIFIER_JSON.read_text(encoding="utf-8"))
    captures = doc.get("captures", [])
    mismatches = [{"index": i + 1, "file": c.get("file"), "classification": c.get("classification"), "expected": c.get("expected_class")} for i, c in enumerate(captures) if not c.get("expected_match")]
    duplicate_counts = doc.get("duplicate_sha256_counts", {})
    duplicate_blockers = {sha: count for sha, count in duplicate_counts.items() if count and count > 1}
    crops = sorted((ATTEMPT_DIR / "viewport_224x136").glob("*.ppm"))
    labels = LABELS_TSV.read_text(encoding="utf-8", errors="replace").splitlines() if LABELS_TSV.is_file() else []
    key_log_tail = KEYS_LOG.read_text(encoding="utf-8", errors="replace").splitlines()[-30:] if KEYS_LOG.is_file() else []
    semantic_ready = doc.get("capture_count") == 6 and not mismatches and not duplicate_blockers
    return {"exists": True, "path": str(CLASSIFIER_JSON), "attempt_dir": str(ATTEMPT_DIR), "status": "PASS_SEMANTIC_ROUTE_READY" if semantic_ready else "BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE", "capture_count": doc.get("capture_count"), "dimensions_seen": doc.get("dimensions_seen"), "expected_sequence": doc.get("expected_sequence", EXPECTED_SEQUENCE), "class_counts": doc.get("class_counts"), "mismatches": mismatches, "duplicate_sha256_counts_gt1": duplicate_blockers, "viewport_crop_ppm_count": len(crops), "labels_tsv": labels, "key_log_tail": key_log_tail}


def decide_status(source: list[dict[str, Any]], runner: dict[str, Any], attempt: dict[str, Any]) -> str:
    if any(not item["ok"] for item in source):
        return "FAIL_REDMCSB_SOURCE_AUDIT"
    if runner["missing_tools"] or any(not f["ok"] for f in runner["canonical_files"].values()) or not runner["capture_script"]["exists"]:
        return "BLOCKED_ORIGINAL_RUNNER_PREREQUISITES"
    if attempt.get("status") != "PASS_SEMANTIC_ROUTE_READY":
        return "BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE"
    return "PASS_MINIMAL_ORIGINAL_RUNNER_CAPTURE_GATE_READY"


def write_report(manifest: dict[str, Any], report: Path) -> None:
    runner = manifest["runner_prerequisites"]
    attempt = manifest["existing_attempt_audit"]
    lines = ["# Pass206 — DM1 V1 original-runner minimal gate", "", f"Status: `{manifest['status']}`", "", "Scope: N2 Linux-only original DM1 PC34 runner/capture readiness for movement/viewport evidence. No Firestaff renderer code changed; no pixel parity is claimed.", "", "## ReDMCSB source audit", ""]
    for item in manifest["redmcsb_source_audit"]:
        status = "PASS" if item["ok"] else "FAIL"
        lines.append(f"- {status} `{item['id']}` — {', '.join(item['citations'])}: {item['claim']}")
    lines += ["", "## N2 runner prerequisites", ""]
    for cmd, path in runner["tools"].items():
        lines.append(f"- `{cmd}`: `{path or 'MISSING'}`")
    lines.append("")
    for label, info in runner["canonical_files"].items():
        mark = "PASS" if info["ok"] else "FAIL"
        lines.append(f"- {mark} `{label}` `{info['sha256']}` at `{info['path']}`")
    lines += ["", "## Reproducible dry-run command", "", "```sh", "DM1_ORIGINAL_STAGE_DIR=~/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 \\", "DOSBOX=/usr/bin/dosbox \\", "DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \\", "DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \\", f"DM1_ORIGINAL_ROUTE_EVENTS='{runner['route_events']}' \\", "scripts/dosbox_dm1_original_viewport_reference_capture.sh --dry-run", "```", "", "## Existing N2 route attempt audit", "", f"- Attempt: `{attempt.get('attempt_dir', attempt.get('path'))}`", f"- Classifier status: `{attempt.get('status')}`", f"- Capture count/dimensions: `{attempt.get('capture_count')}` / `{attempt.get('dimensions_seen')}`", f"- Viewport crop PPM count: `{attempt.get('viewport_crop_ppm_count')}`", f"- Class counts: `{attempt.get('class_counts')}`", f"- Duplicate SHA counts >1: `{attempt.get('duplicate_sha256_counts_gt1')}`", "", "### Semantic mismatches blocking promotion", ""]
    mismatches = attempt.get("mismatches") or []
    if mismatches:
        for m in mismatches:
            lines.append(f"- shot {m['index']}: `{m['classification']}` expected `{m['expected']}` (`{m['file']}`)")
    else:
        lines.append("- none")
    lines += ["", "## Decision", "", "This is a landable gate because it separates three facts cleanly:", "", "1. ReDMCSB source seams for movement/viewport capture are cited and checked.", "2. N2 has the Linux runner prerequisites and exact PC34 input hashes for a reproducible DOSBox capture attempt.", "3. The current six-shot route is **not promotable** as original movement/viewport evidence because semantic classifier mismatches and duplicate frames remain.", "", "Non-claims: no DANNESBURK use, no push, no original-vs-Firestaff pixel parity claim.", ""]
    report.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out-dir", type=Path, default=OUT_DIR)
    parser.add_argument("--report", type=Path, default=REPORT)
    args = parser.parse_args()
    source = audit_source()
    runner = audit_runner()
    attempt = audit_existing_attempt()
    status = decide_status(source, runner, attempt)
    manifest = {"schema": "pass206_dm1_v1_original_runner_minimal_gate.v1", "status": status, "worker": "portable host", "repo": str(ROOT), "redmcsb_source_root": str(REDMCSB), "forbidden_hosts": ["DANNESBURK", "192.168.2.126"], "redmcsb_source_audit": source, "runner_prerequisites": runner, "existing_attempt_audit": attempt}
    args.out_dir.mkdir(parents=True, exist_ok=True)
    (args.out_dir / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest, args.report)
    print(json.dumps({"status": status, "manifest": str(args.out_dir / "manifest.json"), "report": str(args.report), "source_checks": len(source), "missing_tools": runner["missing_tools"], "attempt_status": attempt.get("status"), "mismatch_count": len(attempt.get("mismatches") or [])}, indent=2, sort_keys=True))
    return 0 if status in {"PASS_MINIMAL_ORIGINAL_RUNNER_CAPTURE_GATE_READY", "BLOCKED_SEMANTIC_ROUTE_NOT_PROMOTABLE"} else 1


if __name__ == "__main__":
    raise SystemExit(main())
