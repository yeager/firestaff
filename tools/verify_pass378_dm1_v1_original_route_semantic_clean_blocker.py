#!/usr/bin/env python3
"""Pass378 verifier for the remaining DM1 V1 original-route semantic blocker.

This pass intentionally does not promote original-side parity. It ties the
current pass376/pass377 artifact state to ReDMCSB movement/viewport anchors,
records a bounded source-portrait six-shot retry, and emits the exact failing
commands needed for the next worker.
"""
from __future__ import annotations

import json
import os
from collections import Counter
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass378_dm1_v1_original_route_semantic_clean_blocker"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
STATUS = "BLOCKED_PASS378_ORIGINAL_ROUTE_NOT_SEMANTICALLY_CLEAN"

SOURCE_ANCHORS: list[dict[str, Any]] = [
    {
        "id": "clikmenu_turn_move_acceptance",
        "file": "CLIKMENU.C",
        "lines": "142-174,180-347",
        "needles": [
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0284_CHAMPION_SetPartyDirection",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0267_MOVE_GetMoveResult_CPSCE",
        ],
        "why": "A promotable route must prove source turn/move acceptance, not merely repeated static screenshots.",
    },
    {
        "id": "dunview_viewport_composition",
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
        "why": "The original crop is meaningful only when bound to F0128 composing a known direction/X/Y viewport buffer.",
    },
    {
        "id": "drawview_viewport_presentation",
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "G0324_B_DrawViewportRequested = C1_TRUE",
            "M526_WaitVerticalBlank",
            "VIDRV_09_BlitViewPort",
        ],
        "why": "Promotion still needs a post-composition presentation seam, not BPLIST/setup echo text.",
    },
    {
        "id": "portrait_candidate_source_route",
        "file": "MOVESENS.C",
        "lines": "1392-1502",
        "needles": [
            "C127_SENSOR_WALL_CHAMPION_PORTRAIT",
            "F0280_CHAMPION_AddCandidateChampionToParty",
        ],
        "why": "The bounded retry uses the prior ReDMCSB champion-portrait route hypothesis to try to escape no-party static gameplay.",
    },
]

PRIOR_MANIFESTS = {
    "pass376_artifact_command": "parity-evidence/verification/pass376_dm1_v1_original_artifact_command_manifest/manifest.json",
    "pass377_paired_diff": "parity-evidence/verification/pass377_dm1_v1_paired_diff_artifact_blocker/manifest.json",
    "pass162_party_route": "parity-evidence/verification/pass162_original_party_route_unblock/manifest.json",
    "pass166_portrait_route": "parity-evidence/verification/pass166_source_portrait_click_route_probe/manifest.json",
}

CURRENT_PASS376_CLASSIFIER = "verification-screens/pass376-original-route/pass80_original_frame_classifier.json"
RETRY_DIR = "verification-screens/pass378-source-portrait-sixshot-retry"
RETRY_CLASSIFIER = f"{RETRY_DIR}/pass80_original_frame_classifier.json"
RETRY_COMMAND = (
    "OUT=$PWD/verification-screens/pass378-source-portrait-sixshot-retry; rm -rf \"$OUT\"; "
    "OUT_DIR=\"$OUT\" "
    "DM1_ORIGINAL_STAGE_DIR=$HOME/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 "
    "DOSBOX=/usr/bin/dosbox DM1_ORIGINAL_PROGRAM='DM -vv -sn' DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 "
    "WAIT_BEFORE_INPUT_MS=3000 NEW_FILE_TIMEOUT_MS=6000 "
    "DM1_ORIGINAL_ROUTE_EVENTS=\"wait:7000 enter wait:2500 shot:party_hud click:111,82 wait:1200 shot:portrait_candidate click:130,115 wait:1200 shot:resurrect_choice enter wait:1500 shot:after_confirm f1 wait:1200 shot:spell_panel f4 wait:1200 shot:inventory_panel\" "
    "xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run"
)
CLASSIFIER_COMMAND = (
    "python3 tools/pass80_original_frame_classifier.py "
    "verification-screens/pass378-source-portrait-sixshot-retry --expected pass77 --fail-on-duplicates"
)
PASS86_DRY_COMMAND = (
    "python3 tools/pass86_original_viewport_crop_manifest.py "
    "verification-screens/pass378-source-portrait-sixshot-retry "
    "--out-dir verification-screens/pass378-source-portrait-sixshot-viewports --dry-run"
)
NEXT_COMMAND = (
    "python3 tools/pass162_original_party_route_unblock.py && "
    "python3 tools/pass166_source_portrait_click_route_probe.py && "
    "python3 tools/verify_pass378_dm1_v1_original_route_semantic_clean_blocker.py"
)


def norm(text: str) -> str:
    return " ".join(text.split())


def source_window(path: Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    chunks: list[str] = []
    for part in spec.split(","):
        start, end = [int(x) for x in part.split("-", 1)] if "-" in part else (int(part), int(part))
        chunks.append("\n".join(lines[start - 1:end]))
    return "\n".join(chunks)


def audit_sources() -> list[dict[str, Any]]:
    out = []
    for anchor in SOURCE_ANCHORS:
        path = REDMCSB / anchor["file"]
        text = source_window(path, anchor["lines"]) if path.exists() else ""
        missing = [needle for needle in anchor["needles"] if norm(needle) not in norm(text)]
        out.append({**anchor, "path": str(path), "ok": path.exists() and not missing, "missing_markers": missing})
    return out


def load_json(rel: str) -> dict[str, Any]:
    path = ROOT / rel
    if not path.exists():
        return {"exists": False, "path": rel}
    data = json.loads(path.read_text(encoding="utf-8"))
    return {"exists": True, "path": rel, **data}


def summarize_classifier(rel: str) -> dict[str, Any]:
    data = load_json(rel)
    if not data.get("exists"):
        return data
    captures = data.get("captures", [])
    classes = [c.get("classification") for c in captures]
    hashes = [c.get("sha256") for c in captures]
    return {
        "exists": True,
        "path": rel,
        "pass": data.get("pass"),
        "capture_count": data.get("capture_count"),
        "class_counts": data.get("class_counts") or dict(Counter(classes)),
        "duplicate_sha256_counts": data.get("duplicate_sha256_counts") or {h: n for h, n in Counter(hashes).items() if h and n > 1},
        "problems": data.get("problems", []),
        "warnings": data.get("warnings", []),
        "captures": [
            {"file": c.get("file"), "classification": c.get("classification"), "sha12": str(c.get("sha256", ""))[:12], "reason": c.get("reason")}
            for c in captures
        ],
    }


def summarize_prior() -> dict[str, Any]:
    out = {}
    for name, rel in PRIOR_MANIFESTS.items():
        data = load_json(rel)
        item: dict[str, Any] = {"exists": data.get("exists"), "path": rel, "status": data.get("status")}
        if name in {"pass162_party_route", "pass166_portrait_route"} and data.get("exists"):
            item["buckets"] = data.get("buckets")
            item["errors"] = data.get("errors")
            item["result_summaries"] = [
                {
                    "name": r.get("name"),
                    "classification": r.get("classification"),
                    "reason": r.get("reason"),
                    "classes": (r.get("route_evidence") or {}).get("classes"),
                    "unique_hashes": (r.get("route_evidence") or {}).get("unique_hashes"),
                    "static_hits": (r.get("route_evidence") or {}).get("static_hits"),
                    "control_count": (r.get("route_evidence") or {}).get("control_count"),
                }
                for r in data.get("results", [])
            ]
        else:
            item["blocker"] = data.get("blocker") or data.get("blockers") or data.get("activeBlocker")
        out[name] = item
    return out


def write_report(m: dict[str, Any]) -> None:
    lines = [
        "# Pass378 — DM1 V1 original route semantic-clean blocker",
        "",
        f"Status: `{m['status']}`",
        "",
        "## Verdict",
        "",
        "The current pass376 original raw route and the bounded source-portrait retry do not yield six semantically clean, distinct party/HUD/viewport states. The blocker is original-side route semantics, not Firestaff pairing mechanics.",
        "",
        "## Current pass376 classifier",
        "",
        f"- pass: `{m['currentPass376Classifier'].get('pass')}`",
        f"- class counts: `{m['currentPass376Classifier'].get('class_counts')}`",
        f"- duplicate hashes: `{m['currentPass376Classifier'].get('duplicate_sha256_counts')}`",
        "",
        "## Source-portrait retry",
        "",
        f"- artifact dir: `{RETRY_DIR}`",
        f"- pass80 pass: `{m['sourcePortraitRetry'].get('pass')}`",
        f"- class counts: `{m['sourcePortraitRetry'].get('class_counts')}`",
        f"- duplicate hashes: `{m['sourcePortraitRetry'].get('duplicate_sha256_counts')}`",
        "",
        "Problems:",
    ]
    for p in m["sourcePortraitRetry"].get("problems", []):
        lines.append(f"- {p}")
    lines += [
        "",
        "## Failing commands preserved",
        "",
        "```bash",
        RETRY_COMMAND,
        CLASSIFIER_COMMAND,
        PASS86_DRY_COMMAND,
        "```",
        "",
        "## Next concrete command",
        "",
        "```bash",
        NEXT_COMMAND,
        "```",
        "",
        f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
    ]
    REPORT.write_text("\n".join(lines).rstrip() + "\n", encoding="utf-8")


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    sources = audit_sources()
    current = summarize_classifier(CURRENT_PASS376_CLASSIFIER)
    retry = summarize_classifier(RETRY_CLASSIFIER)
    prior = summarize_prior()
    blockers = []
    if current.get("duplicate_sha256_counts"):
        blockers.append("current pass376 route repeats raw frame hashes")
    if current.get("pass") is not True:
        blockers.append("current pass376 route fails pass80/pass86 expected semantic sequence")
    if retry.get("pass") is not True:
        blockers.append("source-portrait retry fails pass80/pass86 expected semantic sequence")
    if retry.get("duplicate_sha256_counts"):
        blockers.append("source-portrait retry still repeats raw frame hashes")
    if any(not s["ok"] for s in sources):
        blockers.append("ReDMCSB source anchor audit failed")
    manifest: dict[str, Any] = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": STATUS if blockers else "PASS378_ORIGINAL_ROUTE_SEMANTICALLY_CLEAN",
        "sourceRoot": str(REDMCSB),
        "sourceAnchors": sources,
        "priorManifests": prior,
        "currentPass376Classifier": current,
        "sourcePortraitRetry": retry,
        "failedCommands": [RETRY_COMMAND, CLASSIFIER_COMMAND, PASS86_DRY_COMMAND],
        "nextConcreteCommand": NEXT_COMMAND,
        "blockers": blockers,
        "promotionRule": "Promote only when six raw 320x200 frames classify as dungeon_gameplay,dungeon_gameplay,dungeon_gameplay,spell_panel,dungeon_gameplay,inventory with no duplicate raw hashes, then pass86 succeeds without --allow-mismatch.",
        "notClaimed": ["pixel parity", "semantically matched original runtime route", "strict F0128->F0097 original true-stop promotion"],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": manifest["status"], "blockers": blockers, "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT))}, indent=2))
    return 0 if manifest["status"] == STATUS else 1


if __name__ == "__main__":
    raise SystemExit(main())
