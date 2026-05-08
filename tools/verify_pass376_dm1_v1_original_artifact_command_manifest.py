#!/usr/bin/env python3
"""Pass376 verifier-backed artifact-command manifest for DM1 V1 original overlay parity.

This pass does not claim parity. It records the exact source anchors, prior
blocker evidence, commands, and output paths required to produce the four
missing artifact classes before representative original-vs-Firestaff overlay
review can start.
"""
from __future__ import annotations

import json
import os
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass376_dm1_v1_original_artifact_command_manifest"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
STATUS = "BLOCKED_PASS376_ORIGINAL_ARTIFACT_COMMAND_MANIFEST_READY"

SOURCE_ANCHORS: list[dict[str, Any]] = [
    {
        "id": "accepted_movement_mutates_party_tuple",
        "file": "CLIKMENU.C",
        "lines": "142-174,180-347",
        "needles": [
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0284_CHAMPION_SetPartyDirection",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks",
        ],
        "why": "Original frames must be tied to accepted source turns/steps, not unknown screenshots.",
    },
    {
        "id": "f0128_composes_known_viewport_tuple",
        "file": "DUNVIEW.C",
        "lines": "8318-8611",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "G0296_puc_Bitmap_Viewport",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)",
        ],
        "why": "The 224x136 viewport crop is promotable only after F0128 composes G0296 for a known direction/X/Y tuple.",
    },
    {
        "id": "f0097_presents_pc34_viewport",
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "F0638_GetZone(C007_ZONE_VIEWPORT",
            "VIDRV_09_BlitViewPort",
            "G0296_puc_Bitmap_Viewport",
        ],
        "why": "Original-side capture must occur after the PC34 viewport-present seam, not setup/BPLIST echo text.",
    },
]

PRIOR_EVIDENCE = {
    "pass360": {
        "path": "parity-evidence/verification/pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing/manifest.json",
        "expected": "BLOCKED_PASS360_ORIGINAL_RUNTIME_TRUE_STOP_BLOCKER_NARROWED",
        "role": "strict original FIRES F0128 -> F0097/VIDRV true-stop blocker",
    },
    "pass372": {
        "path": "parity-evidence/verification/pass372_dm1_v1_movement_runtime_route/manifest.json",
        "expected": "PASS372_DM1_V1_MOVEMENT_RUNTIME_ROUTE_SOURCE_LOCKED",
        "role": "Firestaff M11 movement route source-locked; not the active blocker",
    },
}

ARTIFACT_COMMANDS: list[dict[str, Any]] = [
    {
        "id": "strict_f0128_to_f0097_vidrv_true_stop_transcript",
        "artifact_class": "original_true_stop_transcript",
        "working_directory": ".",
        "command": "python3 tools/pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py --seconds 75 && python3 tools/verify_pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py && python3 tools/verify_pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing.py",
        "required_outputs": [
            "parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/manifest.json",
            "parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/pre_arm_before_route.clean.txt",
            "parity-evidence/verification/pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing/manifest.json",
        ],
        "promotion_rule": "Promote only if the bounded transcript contains a strict post-Running stop at F0128_DUNGEONVIEW_Draw_CPSF followed by F0097_DUNGEONVIEW_DrawViewport or VIDRV_09_BlitViewPort; setup echo/BPLIST text is not evidence.",
    },
    {
        "id": "labelled_original_320x200_frames",
        "artifact_class": "semantic_original_full_frames",
        "working_directory": ".",
        "command": "OUT_DIR=$PWD/verification-screens/pass376-original-route DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 DOSBOX=/usr/bin/dosbox DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 WAIT_BEFORE_INPUT_MS=3000 NEW_FILE_TIMEOUT_MS=6000 DM1_ORIGINAL_ROUTE_EVENTS=\"wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:2200 one wait:2500 shot:readiness_preflight wait:700 kp4 wait:900 shot:turn_left_after_vblank wait:700 kp6 wait:900 shot:turn_right_after_vblank wait:700 kp8 wait:1200 shot:forward_after_vblank wait:700 kp4 wait:900 shot:turn_left_2_after_vblank wait:700 kp6 wait:1200 shot:post_redraw_after_vblank\" xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run",
        "required_outputs": [
            "verification-screens/pass376-original-route/image0001-raw.png through image0006-raw.png (320x200)",
            "verification-screens/pass376-original-route/original_viewport_shot_labels.tsv",
            "verification-screens/pass376-original-route/raw_manifest.tsv",
        ],
        "promotion_rule": "Promote only if pass80/pass86 classification shows gameplay/party-control-ready movement/HUD/viewport states, not menu/title/no-party duplicates.",
    },
    {
        "id": "original_224x136_viewport_crops",
        "artifact_class": "pass70_original_viewport_crops",
        "working_directory": ".",
        "command": "python3 tools/pass86_original_viewport_crop_manifest.py verification-screens/pass376-original-route --out-dir verification-screens/pass376-original-dm1-viewports",
        "required_outputs": [
            "verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv",
            "verification-screens/pass376-original-dm1-viewports/viewport_224x136/01_ingame_start_original_viewport_224x136.png",
            "verification-screens/pass376-original-dm1-viewports/viewport_224x136/02_ingame_turn_right_original_viewport_224x136.png",
            "verification-screens/pass376-original-dm1-viewports/viewport_224x136/03_ingame_move_forward_original_viewport_224x136.png",
            "verification-screens/pass376-original-dm1-viewports/viewport_224x136/04_ingame_spell_panel_original_viewport_224x136.png",
            "verification-screens/pass376-original-dm1-viewports/viewport_224x136/05_ingame_after_cast_original_viewport_224x136.png",
            "verification-screens/pass376-original-dm1-viewports/viewport_224x136/06_ingame_inventory_panel_original_viewport_224x136.png",
        ],
        "promotion_rule": "Promote only when all six crops are exactly 224x136 and manifest rows include kind/filename/width/height/bytes/sha256.",
    },
    {
        "id": "paired_firestaff_original_diff_artifacts",
        "artifact_class": "paired_firestaff_capture_and_diff",
        "working_directory": ".",
        "command": "python3 tools/pass70_viewport_pair_compare.py --firestaff-dir verification-screens --original-dir verification-screens/pass376-original-dm1-viewports/viewport_224x136 --original-manifest verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv --out-dir parity-evidence/overlays/pass376 --plan-json parity-evidence/overlays/pass376/pass376_pairing_plan.json --run-diff",
        "required_outputs": [
            "parity-evidence/overlays/pass376/pass376_pairing_plan.json",
            "parity-evidence/overlays/pass376/*_viewport_original_vs_firestaff.mask.png",
            "parity-evidence/overlays/pass376/*_viewport_original_vs_firestaff.stats.json",
        ],
        "promotion_rule": "Diff artifacts are eligibility/review inputs only; they do not by themselves claim pixel parity.",
    },
]


def run(cmd: list[str]) -> str:
    return subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False).stdout.strip()


def norm(text: str) -> str:
    return " ".join(text.split())


def source_window(path: Path, spec: str) -> str:
    data = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            start, end = [int(x) for x in part.split("-", 1)]
        else:
            start = end = int(part)
        chunks.append("\n".join(data[start - 1:end]))
    return "\n".join(chunks)


def audit_sources() -> list[dict[str, Any]]:
    rows = []
    for anchor in SOURCE_ANCHORS:
        path = REDMCSB / anchor["file"]
        text = source_window(path, anchor["lines"]) if path.exists() else ""
        missing = [needle for needle in anchor["needles"] if norm(needle) not in norm(text)]
        rows.append({**anchor, "path": str(path), "ok": path.exists() and not missing, "missing_markers": missing})
    return rows


def load_prior() -> dict[str, Any]:
    out: dict[str, Any] = {}
    for name, spec in PRIOR_EVIDENCE.items():
        path = ROOT / spec["path"]
        data = json.loads(path.read_text(encoding="utf-8")) if path.exists() else {}
        status = data.get("status")
        out[name] = {
            **spec,
            "exists": path.exists(),
            "status": status,
            "ok": path.exists() and status == spec["expected"],
            "active_blocker": data.get("activeBlocker") or data.get("blocker") or data.get("classification", {}).get("activeBlocker"),
            "not_claimed": data.get("notClaimed") or data.get("not_claimed"),
        }
    return out


def artifact_probe() -> list[dict[str, Any]]:
    paths = [p for cmd in ARTIFACT_COMMANDS for p in cmd["required_outputs"]]
    rows = []
    for raw in paths:
        if "*" in raw or " through " in raw or " (" in raw:
            rows.append({"path": raw, "exists": False, "probeable": False})
            continue
        path = ROOT / raw
        rows.append({"path": raw, "exists": path.exists(), "probeable": True})
    return rows


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# Pass376 — DM1 V1 original artifact command manifest",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "## Decision",
        "",
        "The overlay blocker is now an explicit artifact-command contract. No parity is claimed until the original true-stop transcript, labelled original frames, viewport crops, and paired diff artifacts all exist and pass their promotion rules.",
        "",
        "## Source anchors",
        "",
    ]
    for row in manifest["source_audit"]:
        lines.append(f"- `{row['file']}:{row['lines']}` — {row['why']} ok=`{row['ok']}`")
    lines.extend(["", "## Prior evidence", ""])
    for name, row in manifest["prior_evidence"].items():
        lines.append(f"- `{name}` `{row['status']}` — {row['role']} ok=`{row['ok']}`")
    lines.extend(["", "## Exact commands and artifact paths", ""])
    for idx, cmd in enumerate(manifest["artifact_commands"], 1):
        lines.append(f"{idx}. `{cmd['id']}` / `{cmd['artifact_class']}`")
        lines.append("")
        lines.append("   ```bash")
        lines.append(f"   {cmd['command']}")
        lines.append("   ```")
        lines.append("")
        lines.append("   Required outputs:")
        for output in cmd["required_outputs"]:
            lines.append(f"   - `{output}`")
        lines.append(f"   - promotion rule: {cmd['promotion_rule']}")
        lines.append("")
    lines.extend([
        "## Blockers",
        "",
        "- The pass360 strict FIRES true-stop blocker remains active until the first command proves F0128 -> F0097/VIDRV in a bounded owned-PTY run.",
        "- Existing pass94-era original frames/crops are historical inputs only; pass376 needs fresh labelled artifacts tied to this route contract before pairing.",
        "",
        "## Non-claims",
        "",
        "- No original-vs-Firestaff pixel parity is claimed.",
        "- No new original FIRES F0128/F0097/VIDRV true stop is claimed.",
        "- No semantically matched original runtime capture is claimed.",
        "- No HUD/viewport overlay parity is claimed.",
        "",
        f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
    ])
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    sources = audit_sources()
    priors = load_prior()
    ok = all(row["ok"] for row in sources) and all(row["ok"] for row in priors.values())
    manifest: dict[str, Any] = {
        "schema": f"{PASS}.v1",
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "status": STATUS if ok else "FAIL_PASS376_SOURCE_OR_PRIOR_AUDIT",
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"]),
        "head": run(["git", "rev-parse", "HEAD"]),
        "source_root": str(REDMCSB),
        "source_audit": sources,
        "prior_evidence": priors,
        "artifact_commands": ARTIFACT_COMMANDS,
        "artifact_probe": artifact_probe(),
        "blockers": [
            "pass360 still blocks strict original FIRES F0128 -> F0097/VIDRV true-stop promotion",
            "labelled original 320x200 gameplay frames for this route are not tracked",
            "224x136 original crops for this route are not tracked",
            "paired Firestaff/original diff artifacts for this route are not tracked",
        ],
        "not_claimed": [
            "original-vs-Firestaff pixel parity",
            "new original FIRES F0128/F0097/VIDRV true stop",
            "semantically matched original gameplay capture",
            "HUD/viewport overlay parity",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": manifest["status"], "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT))}, indent=2))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
