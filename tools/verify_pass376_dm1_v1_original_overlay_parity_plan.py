#!/usr/bin/env python3
"""Pass376 verifier: identify the remaining runtime artifacts for DM1 V1 original overlay parity.

Evidence/blocker pass only.  It starts from ReDMCSB DUNVIEW/DRAWVIEW/CLIKMENU
anchors plus pass360/pass372 and writes the exact artifact contract needed before
movement/HUD/viewport original-vs-Firestaff overlay parity can be promoted.
"""
from __future__ import annotations

import json
import os
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass376_dm1_v1_original_overlay_parity_plan"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path(os.environ.get("FIRESTAFF_REDMCSB_SOURCE", str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")))
STATUS = "BLOCKED_PASS376_ORIGINAL_OVERLAY_RUNTIME_ARTIFACTS_MISSING"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {
        "id": "movement_mutates_party_tuple",
        "file": "CLIKMENU.C",
        "lines": "142-174,180-347",
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty / F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "needles": [
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0284_CHAMPION_SetPartyDirection",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks",
        ],
        "decision": "Representative movement parity must be tied to accepted source turns/steps, not screenshots with unknown party tuple.",
    },
    {
        "id": "viewport_composed_from_tuple",
        "file": "DUNVIEW.C",
        "lines": "8318-8611",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "G0296_puc_Bitmap_Viewport",
            "F0127_DUNGEONVIEW_DrawSquareD0C",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW)",
        ],
        "decision": "The viewport crop to compare is only promotable after F0128 composes G0296 for the known direction/X/Y tuple.",
    },
    {
        "id": "viewport_presented_by_pc34_blit",
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "F0638_GetZone(C007_ZONE_VIEWPORT",
            "VIDRV_09_BlitViewPort",
            "G0296_puc_Bitmap_Viewport",
        ],
        "decision": "The original-side viewport artifact must be captured after the PC34 present seam, not at setup/BPLIST time.",
    },
]

PRIOR_MANIFESTS = {
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

NEXT_COMMANDS = [
    {
        "id": "strict_original_true_stop_sequence",
        "command": "python3 tools/pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py --seconds 75 && python3 tools/verify_pass330_dm1_v1_direct_pty_code_stop_transition_investigation.py && python3 tools/verify_pass360_dm1_v1_original_runtime_true_stop_blocker_narrowing.py",
        "must_produce": [
            "parity-evidence/verification/pass330_dm1_v1_direct_pty_code_stop_transition_investigation/manifest.json with strict F0128 stop after running",
            "same bounded transcript showing F0097_DUNGEONVIEW_DrawViewport or VIDRV_09_BlitViewPort after F0128",
        ],
    },
    {
        "id": "semantic_original_runtime_capture",
        "command": "OUT_DIR=$PWD/verification-screens/pass376-original-route DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 DOSBOX=/usr/bin/dosbox DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 WAIT_BEFORE_INPUT_MS=3000 NEW_FILE_TIMEOUT_MS=6000 DM1_ORIGINAL_ROUTE_EVENTS=\"wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:2200 one wait:2500 shot:readiness_preflight wait:700 kp4 wait:900 shot:turn_left_after_vblank wait:700 kp6 wait:900 shot:turn_right_after_vblank wait:700 kp8 wait:1200 shot:forward_after_vblank wait:700 kp4 wait:900 shot:turn_left_2_after_vblank wait:700 kp6 wait:1200 shot:post_redraw_after_vblank\" xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run",
        "must_produce": [
            "verification-screens/pass376-original-route/imageNNNN-raw.png frames with 320x200 gameplay dimensions",
            "route labels proving party-control-ready movement/HUD/viewport states, not menu/title/no-party duplicates",
        ],
    },
    {
        "id": "original_viewport_crop_manifest",
        "command": "python3 tools/pass86_original_viewport_crop_manifest.py verification-screens/pass376-original-route --out-dir verification-screens/pass376-original-dm1-viewports",
        "must_produce": [
            "verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv",
            "six 224x136 original viewport crops for pass70-compatible pairing",
        ],
    },
    {
        "id": "paired_firestaff_overlay_compare",
        "command": "python3 tools/pass70_viewport_pair_compare.py --firestaff-dir verification-screens --original-dir verification-screens/pass376-original-dm1-viewports/viewport_224x136 --original-manifest verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv --out-dir parity-evidence/overlays/pass376 --plan-json parity-evidence/overlays/pass376/pass376_pairing_plan.json --run-diff",
        "must_produce": [
            "parity-evidence/overlays/pass376/pass376_pairing_plan.json",
            "per-scene viewport diff masks/stats; these are eligibility artifacts, not parity claims until reviewed",
        ],
    },
]

MISSING_RUNTIME_ARTIFACTS = [
    {
        "id": "original_true_stop_transcript",
        "blocks": "source-bound original runtime state identity",
        "missing": "A bounded FIRES debugger transcript proving F0128_DUNGEONVIEW_Draw_CPSF then F0097_DUNGEONVIEW_DrawViewport or VIDRV_09_BlitViewPort after controlled movement input.",
    },
    {
        "id": "semantic_original_full_frames",
        "blocks": "movement/HUD/viewport overlay eligibility",
        "missing": "320x200 original PC34 gameplay frames classified as party-control-ready movement/HUD/viewport states, with route labels and hashes.",
    },
    {
        "id": "pass70_original_viewport_crops",
        "blocks": "viewport crop comparator",
        "missing": "224x136 original viewport crops and manifest derived from semantically matched original frames.",
    },
    {
        "id": "paired_firestaff_capture_and_diff",
        "blocks": "representative original-vs-Firestaff overlay review",
        "missing": "Same-scene Firestaff captures paired with the original crops plus diff masks/stats under parity-evidence/overlays/pass376.",
    },
]


def run(cmd: list[str]) -> str:
    return subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False).stdout.strip()


def norm(s: str) -> str:
    return " ".join(s.split())


def source_window(path: Path, spec: str) -> str:
    data = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            a, b = [int(x) for x in part.split("-", 1)]
        else:
            a = b = int(part)
        chunks.append("\n".join(data[a - 1:b]))
    return "\n".join(chunks)


def audit_sources() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for spec in SOURCE_LOCKS:
        path = REDMCSB / spec["file"]
        text = source_window(path, spec["lines"]) if path.exists() else ""
        missing = [needle for needle in spec["needles"] if norm(needle) not in norm(text)]
        rows.append({
            "id": spec["id"],
            "file": spec["file"],
            "path": str(path),
            "lines": spec["lines"],
            "function": spec["function"],
            "decision": spec["decision"],
            "ok": path.exists() and not missing,
            "missingMarkers": missing,
        })
    return rows


def load_prior() -> dict[str, Any]:
    out: dict[str, Any] = {}
    for name, spec in PRIOR_MANIFESTS.items():
        path = ROOT / spec["path"]
        data = json.loads(path.read_text(encoding="utf-8")) if path.exists() else {}
        status = data.get("status")
        out[name] = {
            "path": spec["path"],
            "exists": path.exists(),
            "status": status,
            "expected": spec["expected"],
            "role": spec["role"],
            "ok": path.exists() and status == spec["expected"],
            "notClaimed": data.get("notClaimed") or data.get("not_claimed"),
            "activeBlocker": data.get("activeBlocker") or data.get("blocker") or data.get("classification", {}).get("activeBlocker"),
        }
    return out


def artifact_probe() -> list[dict[str, Any]]:
    paths = [
        ("original_route_dir", ROOT / "verification-screens/pass376-original-route"),
        ("original_crop_manifest", ROOT / "verification-screens/pass376-original-dm1-viewports/original_viewport_224x136_manifest.tsv"),
        ("pairing_plan", ROOT / "parity-evidence/overlays/pass376/pass376_pairing_plan.json"),
    ]
    return [{"id": ident, "path": str(path.relative_to(ROOT)), "exists": path.exists()} for ident, path in paths]


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    sources = audit_sources()
    priors = load_prior()
    artifact_state = artifact_probe()
    ok = all(row["ok"] for row in sources) and all(row["ok"] for row in priors.values())
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": STATUS if ok else "FAIL_PASS376_SOURCE_OR_PRIOR_AUDIT",
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"]),
        "head": run(["git", "rev-parse", "HEAD"]),
        "sourceRoot": str(REDMCSB),
        "sourceAudit": sources,
        "priorEvidence": priors,
        "missingRuntimeArtifacts": MISSING_RUNTIME_ARTIFACTS,
        "artifactProbe": artifact_state,
        "exactNextCommands": NEXT_COMMANDS,
        "notClaimed": [
            "original-vs-Firestaff pixel parity",
            "new original FIRES F0128/F0097/VIDRV true stop",
            "semantically matched original gameplay capture",
            "HUD/viewport overlay parity",
        ],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass376 — DM1 V1 original overlay parity artifact plan",
        "",
        "Status: `%s`" % manifest["status"],
        "",
        "## Decision",
        "",
        "Do not claim representative movement/HUD/viewport overlay parity yet. Pass372 closes the Firestaff movement route, but pass360 still leaves the original FIRES true-stop/capture side unproven.",
        "",
        "## ReDMCSB source audit",
        "",
    ]
    for row in sources:
        lines.append("- `%s:%s` — `%s` — %s ok=`%s`" % (row["file"], row["lines"], row["function"], row["decision"], row["ok"]))
    lines += ["", "## Existing evidence", ""]
    for name, row in priors.items():
        lines.append("- `%s` `%s` — %s ok=`%s`" % (name, row["status"], row["role"], row["ok"]))
    lines += ["", "## Exact missing runtime artifacts", ""]
    for item in MISSING_RUNTIME_ARTIFACTS:
        lines.append("- `%s` — blocks: %s; missing: %s" % (item["id"], item["blocks"], item["missing"]))
    lines += ["", "## Exact next commands and required artifacts", ""]
    for idx, cmd in enumerate(NEXT_COMMANDS, 1):
        lines.append("%d. `%s`" % (idx, cmd["id"]))
        lines.append("")
        lines.append("   ```bash")
        lines.append("   %s" % cmd["command"])
        lines.append("   ```")
        for artifact in cmd["must_produce"]:
            lines.append(f"   - must produce: {artifact}")
        lines.append("")
    lines += [
        "## Non-claims",
        "",
        "- No original-vs-Firestaff pixel parity is claimed.",
        "- No new FIRES true-stop transcript is claimed.",
        "- No semantically matched original runtime capture is claimed.",
        "",
        f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
    ]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(json.dumps({"status": manifest["status"], "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT))}, indent=2))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
