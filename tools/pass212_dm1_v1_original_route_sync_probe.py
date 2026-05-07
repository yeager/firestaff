#!/usr/bin/env python3
"""Pass212 offline route/capture synchronization probe for DM1 original movement evidence.

This probe is intentionally repo-friendly: it consumes manifests/logs/classifier
JSON from an original DOSBox capture attempt, but it does not require or emit raw
PNG/PPM image payloads.  Its job is to decide whether the attempt is promotable
movement/viewport evidence, or to preserve an exact synchronization blocker that
future N2 workers can retry without guessing.
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
from collections import defaultdict
from pathlib import Path
from typing import Any

REPO = Path(__file__).resolve().parent.parent
DEFAULT_ATTEMPT = REPO / "verification-screens/pass210-n2-original-movement-route-fresh"
DEFAULT_OUT_DIR = REPO / "parity-evidence/verification/pass212_dm1_v1_original_route_sync_probe"
PASS211_MANIFEST = REPO / "parity-evidence/verification/pass211_dm1_v1_original_movement_fresh_blocker/manifest.json"
PASS211_README = REPO / "parity-evidence/verification/pass211_dm1_v1_original_movement_fresh_blocker/README.md"
EXPECTED_LABELS = ["start", "turn_left", "turn_right", "forward", "turn_left_2", "post_redraw"]
MOVEMENT_LABELS = {"turn_left", "turn_right", "forward", "turn_left_2", "post_redraw"}
GAMEPLAY = "dungeon_gameplay"

SOURCE_SEAM = [
    {
        "file": "COMMAND.C",
        "symbols": ["F0361_COMMAND_ProcessKeyPress", "F0380_COMMAND_ProcessQueue_CPSC"],
        "claim": "input commands must be queued/dequeued before movement evidence is captured",
    },
    {
        "file": "CLIKMENU.C",
        "symbols": ["F0365_COMMAND_ProcessTypes1To2_TurnParty", "F0366_COMMAND_ProcessTypes3To6_MoveParty"],
        "claim": "turn and movement handlers mutate party direction/position before redraw",
    },
    {
        "file": "MOVESENS.C",
        "symbols": ["F0267_MOVE_GetMoveResult_CPSCE"],
        "claim": "movement legality/sensor processing gates post-command position changes",
    },
    {
        "file": "GAMELOOP.C",
        "symbols": ["F0002_MAIN_GameLoop_CPSDF"],
        "claim": "the main loop owns command processing and redraw cadence",
    },
    {
        "file": "DUNVIEW.C",
        "symbols": ["F0128_DUNGEONVIEW_Draw_CPSF"],
        "claim": "viewport cells are derived from party map/direction at draw time",
    },
    {
        "file": "DRAWVIEW.C",
        "symbols": ["F0097_DUNGEONVIEW_DrawViewport", "M526_WaitVerticalBlank", "VIDRV_09_BlitViewPort"],
        "claim": "the promotable seam is the presented 224x136 viewport after vblank/blit",
    },
]

RETRY_ROUTE = (
    "wait:9000 enter wait:1800 one wait:1800 click:276,140 wait:2200 one wait:2500 "
    "shot:readiness_preflight wait:700 kp4 wait:900 shot:turn_left_after_vblank "
    "wait:700 kp6 wait:900 shot:turn_right_after_vblank wait:700 kp8 wait:1200 "
    "shot:forward_after_vblank wait:700 kp4 wait:900 shot:turn_left_2_after_vblank "
    "wait:700 kp6 wait:1200 shot:post_redraw_after_vblank"
)


def rel(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO))
    except ValueError:
        return str(path)


def read_tsv(path: Path) -> list[dict[str, str]]:
    if not path.exists():
        return []
    with path.open(newline="") as f:
        return list(csv.DictReader(f, delimiter="\t"))


def load_json(path: Path) -> Any | None:
    if not path.exists():
        return None
    return json.loads(path.read_text())


def file_digest(path: Path) -> dict[str, Any]:
    if not path.exists():
        return {"path": rel(path), "exists": False}
    data = path.read_bytes()
    return {
        "path": rel(path),
        "exists": True,
        "size_bytes": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
    }


def classifier_rows(attempt: Path) -> list[dict[str, Any]]:
    data = load_json(attempt / "pass80_original_frame_classifier.json")
    if not data:
        return []
    return list(data.get("captures") or [])


def pass211_fallback_rows() -> tuple[list[dict[str, str]], list[dict[str, str]], list[dict[str, str]], list[dict[str, Any]], list[str]]:
    """Return manifest-like rows from the committed pass211 blocker manifest.

    The actual pass210 raw/crop files are intentionally not committed.  The
    pass211 repo manifest preserves their sha12 prefixes and the README
    preserves route/classifier binding, which is enough for this sync probe.
    """
    notes: list[str] = []
    manifest = load_json(PASS211_MANIFEST) or {}
    artifacts = list(manifest.get("artifacts") or [])
    if not artifacts:
        return [], [], [], [], notes
    by_name = {a.get("name"): a for a in artifacts}
    route_classes = {
        "start": "entrance_menu",
        "turn_left": "wall_closeup",
        "turn_right": "dungeon_gameplay",
        "forward": "dungeon_gameplay",
        "turn_left_2": "wall_closeup",
        "post_redraw": "dungeon_gameplay",
    }
    crop_names = [
        "01_ingame_start_original_viewport_224x136.ppm",
        "02_ingame_turn_right_original_viewport_224x136.ppm",
        "03_ingame_move_forward_original_viewport_224x136.ppm",
        "04_ingame_spell_panel_original_viewport_224x136.ppm",
        "05_ingame_after_cast_original_viewport_224x136.ppm",
        "06_ingame_inventory_panel_original_viewport_224x136.ppm",
    ]
    labels: list[dict[str, str]] = []
    raw_manifest: list[dict[str, str]] = []
    crop_manifest: list[dict[str, str]] = []
    captures: list[dict[str, Any]] = []
    for idx, route_label in enumerate(EXPECTED_LABELS, 1):
        raw_name = f"image{idx:04d}-raw.png"
        raw = by_name.get(raw_name, {})
        crop_name = crop_names[idx - 1]
        crop = by_name.get(crop_name, {})
        labels.append({"index": f"{idx:02d}", "filename": crop_name, "route_label": route_label, "route_token": f"shot:{route_label}"})
        raw_manifest.append({"index": f"{idx - 1:02d}", "path": str(Path("verification-screens/pass210-n2-original-movement-route-fresh") / raw_name), "sha256": raw.get("sha256_prefix", "")})
        crop_manifest.append({"kind": "original_viewport_224x136", "filename": crop_name, "sha256": crop.get("sha256_prefix", "")})
        captures.append({"file": raw_name, "classification": route_classes[route_label], "sha256": raw.get("sha256_prefix", ""), "reason": "from committed pass211 blocker shot binding; raw payload intentionally untracked"})
    notes.append("using committed pass211 blocker manifest fallback because the raw attempt directory is absent/incomplete")
    return labels, raw_manifest, crop_manifest, captures, notes


def classify_attempt(attempt: Path) -> dict[str, Any]:
    labels = read_tsv(attempt / "original_viewport_shot_labels.tsv")
    raw_manifest = read_tsv(attempt / "raw_manifest.tsv")
    crop_manifest = read_tsv(attempt / "original_viewport_224x136_manifest.tsv")
    captures = classifier_rows(attempt)
    fallback_notes: list[str] = []
    if not (labels and raw_manifest and crop_manifest and captures) and PASS211_MANIFEST.exists():
        labels, raw_manifest, crop_manifest, captures, fallback_notes = pass211_fallback_rows()

    by_file = {Path(str(row.get("file", ""))).name: row for row in captures}
    raw_by_idx = {str(i + 1): row for i, row in enumerate(raw_manifest)}
    crop_by_name = {row.get("filename", ""): row for row in crop_manifest}

    rows: list[dict[str, Any]] = []
    problems: list[str] = []
    warnings: list[str] = []
    raw_groups: dict[str, list[int]] = defaultdict(list)
    crop_groups: dict[str, list[int]] = defaultdict(list)

    if len(labels) != 6:
        problems.append(f"expected 6 shot-label rows, found {len(labels)}")
    if len(raw_manifest) != 6:
        problems.append(f"expected 6 raw-manifest rows, found {len(raw_manifest)}")
    if len(crop_manifest) != 6:
        problems.append(f"expected 6 viewport crop rows, found {len(crop_manifest)}")
    if len(captures) != 6:
        problems.append(f"expected 6 classifier rows, found {len(captures)}")

    for idx in range(1, 7):
        label_row = labels[idx - 1] if idx <= len(labels) else {}
        raw_row = raw_manifest[idx - 1] if idx <= len(raw_manifest) else {}
        route_label = label_row.get("route_label") or ""
        raw_name = Path(raw_row.get("path", f"image{idx:04d}-raw.png")).name
        capture = by_file.get(raw_name) or (captures[idx - 1] if idx <= len(captures) else {})
        raw_sha = (raw_row.get("sha256") or capture.get("sha256") or "")
        crop_name = label_row.get("filename") or ""
        crop_sha = (crop_by_name.get(crop_name) or {}).get("sha256", "")
        classification = capture.get("classification") or "missing_classifier"
        rows.append(
            {
                "shot": idx,
                "raw_frame": raw_name,
                "route_label": route_label,
                "expected_label": EXPECTED_LABELS[idx - 1],
                "classification": classification,
                "raw_sha256_prefix": raw_sha[:12] if raw_sha else "",
                "viewport_sha256_prefix": crop_sha[:12] if crop_sha else "",
                "classifier_reason": capture.get("reason", ""),
            }
        )
        if raw_sha:
            raw_groups[raw_sha].append(idx)
        if crop_sha:
            crop_groups[crop_sha].append(idx)

    if rows and rows[0]["classification"] != GAMEPLAY:
        problems.append(
            f"readiness shot is {rows[0]['classification']!r}, not {GAMEPLAY!r}; party-control gameplay was not proven before movement capture"
        )
    for row in rows[1:]:
        if row["route_label"] in MOVEMENT_LABELS and row["classification"] != GAMEPLAY:
            problems.append(
                f"movement shot {row['shot']} ({row['route_label']}) classified {row['classification']!r}, not {GAMEPLAY!r}"
            )

    raw_dups = {sha: shots for sha, shots in raw_groups.items() if len(shots) > 1}
    crop_dups = {sha: shots for sha, shots in crop_groups.items() if len(shots) > 1}
    if raw_dups:
        problems.append("duplicate raw-frame SHA groups show command/post-redraw collapse: " + ", ".join(f"{sha[:12]}={shots}" for sha, shots in raw_dups.items()))
    if crop_dups:
        problems.append("duplicate viewport-crop SHA groups show non-distinct presented viewports: " + ", ".join(f"{sha[:12]}={shots}" for sha, shots in crop_dups.items()))

    expected_seen = [row["route_label"] for row in rows]
    if expected_seen != EXPECTED_LABELS:
        warnings.append(f"route labels differ from pass211 movement contract: seen {expected_seen}, expected {EXPECTED_LABELS}")
    warnings.extend(fallback_notes)

    all_gameplay = all(row["classification"] == GAMEPLAY for row in rows)
    distinct_raw = len(raw_groups) == 6
    distinct_crops = len(crop_groups) == 6
    status = "PROMOTABLE_MOVEMENT_VIEWPORT_SYNC" if all_gameplay and distinct_raw and distinct_crops and not problems else "BLOCKED_ROUTE_CAPTURE_SYNC"

    return {
        "schema": "pass212_dm1_v1_original_route_sync_probe.v1",
        "status": status,
        "attempt_dir": rel(attempt),
        "source_seam": SOURCE_SEAM,
        "shots": rows,
        "raw_duplicate_groups": {sha[:12]: shots for sha, shots in raw_dups.items()},
        "viewport_duplicate_groups": {sha[:12]: shots for sha, shots in crop_dups.items()},
        "problems": problems,
        "warnings": warnings,
        "retry_contract": {
            "purpose": "separate gameplay-readiness from post-command redraw capture; keep raw PNG/PPM payloads out of git",
            "route_events": RETRY_ROUTE,
            "expected_classifier": [GAMEPLAY] * 6,
            "promotion_rule": "all six raw frames classify dungeon_gameplay and both raw/crop SHA groups are non-duplicate",
            "recommended_command": "OUT_DIR=$PWD/verification-screens/pass212-n2-original-movement-route-sync-retry DM1_ORIGINAL_STAGE_DIR=<firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34 DOSBOX=/usr/bin/dosbox DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 WAIT_BEFORE_INPUT_MS=3000 NEW_FILE_TIMEOUT_MS=6000 DM1_ORIGINAL_ROUTE_EVENTS=\"" + RETRY_ROUTE + "\" xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run",
        },
        "audited_inputs": [
            file_digest(attempt / "original_viewport_shot_labels.tsv"),
            file_digest(attempt / "raw_manifest.tsv"),
            file_digest(attempt / "original_viewport_224x136_manifest.tsv"),
            file_digest(attempt / "pass80_original_frame_classifier.json"),
            file_digest(attempt / "original-viewpoint-route-keys.log"),
            file_digest(attempt / "dosbox-original-viewports.log"),
            file_digest(PASS211_MANIFEST),
            file_digest(PASS211_README),
        ],
        "honesty": "No raw screenshots or viewport crops are emitted by this probe; hashes/manifests only.",
    }


def write_markdown(path: Path, result: dict[str, Any]) -> None:
    lines = [
        "# Pass212 DM1 V1 original route/capture synchronization probe",
        "",
        f"Status: `{result['status']}`",
        "",
        f"Attempt audited: `{result['attempt_dir']}`",
        "",
        "This is a manifest-only synchronization probe. It preserves why the original movement/viewport route is or is not promotable without committing raw DOSBox images or viewport crops.",
        "",
        "## ReDMCSB seam audited",
        "",
    ]
    for seam in result["source_seam"]:
        lines.append(f"- `{seam['file']}`: {', '.join('`' + s + '`' for s in seam['symbols'])} — {seam['claim']}.")
    lines.extend([
        "",
        "## Shot audit",
        "",
        "| shot | route label | classification | raw sha12 | viewport sha12 | reason |",
        "|---:|---|---|---|---|---|",
    ])
    for row in result["shots"]:
        lines.append(f"| {row['shot']} | `{row['route_label']}` | `{row['classification']}` | `{row['raw_sha256_prefix']}` | `{row['viewport_sha256_prefix']}` | {row['classifier_reason']} |")
    if result.get("problems"):
        lines.extend(["", "## Blockers", ""])
        for problem in result["problems"]:
            lines.append(f"- {problem}")
    if result.get("warnings"):
        lines.extend(["", "## Warnings", ""])
        for warning in result["warnings"]:
            lines.append(f"- {warning}")
    lines.extend([
        "",
        "## Retry contract",
        "",
        "The next N2 route should use readiness/post-vblank waits and promote only if all six frames classify as `dungeon_gameplay` and both raw/crop SHA groups are unique.",
        "",
        "```sh",
        result["retry_contract"]["recommended_command"],
        "```",
        "",
        f"Honesty: {result['honesty']}",
        "",
    ])
    path.write_text("\n".join(lines))


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--attempt", type=Path, default=DEFAULT_ATTEMPT)
    ap.add_argument("--out-dir", type=Path, default=DEFAULT_OUT_DIR)
    args = ap.parse_args()

    result = classify_attempt(args.attempt)
    args.out_dir.mkdir(parents=True, exist_ok=True)
    (args.out_dir / "manifest.json").write_text(json.dumps(result, indent=2) + "\n")
    write_markdown(args.out_dir / "README.md", result)
    print(f"{result['status']} wrote {rel(args.out_dir / 'manifest.json')}")
    # Return success for both promotable and blocker states: the gate verifies
    # honest synchronization accounting, not parity promotion.
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
