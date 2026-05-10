#!/usr/bin/env python3
"""Emit the pass304 original PC34 viewport capture blocker manifest.

This is intentionally metadata-only: it does not dump original pixels and does
not claim comparator parity.  It turns the pass127/pass300 render-plan seam into
a deterministic capture contract for the next original-DOSBox run.
"""
from __future__ import annotations

import csv
import hashlib
import json
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
OUT = REPO / "parity-evidence/verification/pass304_dm1_v1_original_viewport_capture_blocker_manifest.json"
RENDER_PLAN = REPO / "parity-evidence/verification/dm1_v1_viewport_wall_render_plan_gate.json"
GRAPHICS_INDEX = REPO / "parity-evidence/verification/pass302_dm1_graphics_dat_index_manifest.json"
WALL_GRAPHICS_INDEX = REPO / "parity-evidence/verification/pass305_dm1_wall_graphics_93_107_manifest.json"
SOURCE_ROOT = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DUNVIEW = SOURCE_ROOT / "DUNVIEW.C"
GAMELOOP = SOURCE_ROOT / "GAMELOOP.C"
DRAWVIEW = SOURCE_ROOT / "DRAWVIEW.C"
COMMAND = SOURCE_ROOT / "COMMAND.C"
CLIKMENU = SOURCE_ROOT / "CLIKMENU.C"
CHAMPION = SOURCE_ROOT / "CHAMPION.C"
MOVESENS = SOURCE_ROOT / "MOVESENS.C"
PASS308 = REPO / "parity-evidence/verification/pass308_original_capture_execution_manifest.json"
PASS312 = REPO / "parity-evidence/verification/pass312_dm1_v1_original_runtime_state_oracle.json"
ASSET_PATHS = {
    "GRAPHICS.DAT": (Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT"),
    "DUNGEON.DAT": (Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/DUNGEON.DAT"),
    "TITLE": (Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1/TITLE"),
    "DM.EXE": (Path.home() / ".openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34/DM.EXE"),
}

SNAPSHOT_CAPTURE = {
    "start_south": {
        "captureBatch": "A",
        "routeFromFreshGameplay": [],
        "requiredShotLabel": "start_south",
        "reason": "initial pass127 tuple before movement input",
    },
    "turn_right_west": {
        "captureBatch": "A",
        "routeFromFreshGameplay": ["right"],
        "requiredShotLabel": "turn_right_west",
        "reason": "COMMAND.C turn-right dispatch mutates direction 2/SOUTH -> 3/WEST",
    },
    "move_forward_west": {
        "captureBatch": "A",
        "routeFromFreshGameplay": ["right", "up"],
        "requiredShotLabel": "move_forward_west",
        "reason": "after west-facing step, tuple is mapX=0,mapY=3,dir=3/WEST",
    },
    "turn_left_east": {
        "captureBatch": "B",
        "routeFromFreshGameplay": ["left"],
        "requiredShotLabel": "turn_left_east",
        "reason": "branch from the fresh start tuple; cannot be reached after batch A without reset/state restore",
    },
    "blocked_forward_south_wall": {
        "captureBatch": "C",
        "routeFromFreshGameplay": ["up"],
        "requiredShotLabel": "blocked_forward_south_wall",
        "reason": "blocked step from start keeps mapX=1,mapY=3,dir=2/SOUTH while advancing the command tick",
    },
}


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def public_path(path: Path) -> str:
    text = str(path)
    replacements = {
        str(Path.home() / ".openclaw/data/firestaff-original-games/DM"): "<firestaff-original-games>",
        str(Path.home() / ".openclaw/data/firestaff-redmcsb-source"): "<redmcsb-source>",
    }
    for prefix, label in replacements.items():
        if text == prefix:
            return label
        if text.startswith(prefix + "/"):
            return label + text[len(prefix):]
    return text


def file_record(path: Path) -> dict[str, object]:
    target = Path(path)
    resolved = target.resolve() if target.exists() else target
    return {
        "path": public_path(target),
        "resolvedPath": public_path(resolved),
        "exists": target.exists(),
        "bytes": resolved.stat().st_size if resolved.exists() else None,
        "sha256": sha256(resolved) if resolved.exists() else None,
    }


def load_existing_original_crops() -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    for manifest in sorted((REPO / "verification-screens").glob("**/original_viewport_224x136_manifest.tsv")):
        label_manifest = manifest.with_name("original_viewport_shot_labels.tsv")
        labels: dict[str, str] = {}
        if label_manifest.exists():
            with label_manifest.open(newline="") as f:
                for row in csv.DictReader(f, delimiter="	"):
                    labels[row.get("filename", "")] = row.get("route_label", "")
        with manifest.open(newline="") as f:
            manifest_lines = [line for line in f if line.strip() and not line.startswith("#")]
            for row in csv.DictReader(manifest_lines, delimiter="	"):
                filename = row.get("filename")
                if not filename:
                    continue
                rows.append({
                    "manifest": str(manifest.relative_to(REPO)),
                    "filename": filename,
                    "routeLabel": labels.get(filename, ""),
                    "width": int(row["width"]),
                    "height": int(row["height"]),
                    "sha256": row["sha256"],
                })
    return rows


def command_for_batch(batch: str, route_tokens_after_entry: list[str], labels: list[str]) -> str:
    # Keep exactly six shots because the current capture script validates that shape.
    # Repeated final labels are explicitly non-promotable padding until the script accepts
    # arbitrary shot counts or a state-reset marker. Linux/DOSBox 0.74 needs the
    # explicit title->entrance->gameplay prelude and keypad movement tokens; plain
    # cursor arrows remain title/menu or no-op and must not be promoted.
    events = [
        "wait:7000", "enter", "wait:1500",
        "click:260,50", "wait:1500",
        "click:276,140", "wait:3000",
        "shot:" + labels[0],
    ]
    for token, label in zip(route_tokens_after_entry, labels[1:]):
        events.extend([token, "wait:1200", "shot:" + label])
    while len([t for t in events if t.startswith("shot")]) < 6:
        events.extend(["wait:600", "shot:padding_not_for_promotion"])
    route = " ".join(events)
    return (
        "OUT_DIR=$PWD/verification-screens/pass304-original-pc34-wall-comparator-batch-" + batch + " \\\n"
        "DM1_ORIGINAL_STAGE_DIR=<firestaff-original-games>/_extracted/dm-pc34/DungeonMasterPC34 \\\n"
        "DM1_ORIGINAL_PROGRAM=\"DM -vv -sn -pk\" \\\n"
        "DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \\\n"
        "WAIT_BEFORE_INPUT_MS=5000 \\\n"
        "NEW_FILE_TIMEOUT_MS=6000 \\\n"
        "DM1_ORIGINAL_ROUTE_EVENTS=\"" + route + "\" \\\n"
        "DOSBOX=/usr/bin/dosbox xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run"
    )


def main() -> int:
    render = json.loads(RENDER_PLAN.read_text())
    graphics_index = json.loads(GRAPHICS_INDEX.read_text())
    wall_graphics_index = json.loads(WALL_GRAPHICS_INDEX.read_text())
    snapshots = []
    missing_capture_contract_snapshots = []
    needed_wall_indices: set[int] = set()
    for snap in render["comparedSnapshots"]:
        events = snap["renderEvents"]
        wall_events = [e for e in events if e.get("event") == "draw_wall_bitmap"]
        for e in wall_events:
            pr = e.get("pixelRegion") or {}
            idx = pr.get("wallSetGraphicIndex")
            if isinstance(idx, int):
                needed_wall_indices.add(idx)
        spec = SNAPSHOT_CAPTURE.get(snap["name"])
        if spec is None:
            missing_capture_contract_snapshots.append({
                "name": snap["name"],
                "party": snap["party"],
                "renderEventCount": len(events),
                "wallBitmapEventCount": len(wall_events),
                "reason": "current render-plan snapshot has no original-DOSBox capture batch/label contract in SNAPSHOT_CAPTURE",
            })
            spec = {
                "captureBatch": None,
                "routeFromFreshGameplay": None,
                "requiredShotLabel": None,
                "reason": "BLOCKED: missing original capture batch/label contract",
            }
        snapshots.append({
            "name": snap["name"],
            "party": snap["party"],
            "captureBatch": spec["captureBatch"],
            "routeFromFreshGameplay": spec["routeFromFreshGameplay"],
            "requiredShotLabel": spec["requiredShotLabel"],
            "reason": spec["reason"],
            "renderEventCount": len(events),
            "wallBitmapEventCount": len(wall_events),
            "pixelRegionsReady": all(e.get("pixelRegion") for e in wall_events),
        })

    pass308 = json.loads(PASS308.read_text())
    pass312 = json.loads(PASS312.read_text())
    existing = load_existing_original_crops()
    required_labels = {s["requiredShotLabel"] for s in snapshots if s["requiredShotLabel"]}
    exact_label_hits = [r for r in existing if r["routeLabel"] in required_labels]
    candidate_hashes = sorted({r["sha256"] for r in existing})
    labels_found = {str(r["routeLabel"]) for r in exact_label_hits}
    route_label_coverage = bool(required_labels) and not missing_capture_contract_snapshots and required_labels <= labels_found
    capture_coverage = pass308.get("coverage", {})
    state_oracle = pass312.get("promotionDecision", {})
    state_oracle_ok = state_oracle.get("partyTupleF0128StateOracle") is True
    capture_ok = route_label_coverage and capture_coverage.get("requiredLabelCoverage") is True and capture_coverage.get("requiredPromotionRowsGameplayOrWallCloseup") is True
    entry_manifested_graphics = {int(r["graphicIndex"]) for r in graphics_index["records"]}
    wall_manifested_graphics = {int(r["graphicIndex"]) for r in wall_graphics_index["records"]}
    manifested_graphics = entry_manifested_graphics | wall_manifested_graphics
    missing_wall_indices = sorted(needed_wall_indices - manifested_graphics)
    if missing_capture_contract_snapshots:
        status = "BLOCKED_ORIGINAL_PC34_CAPTURE_CONTRACT_DRIFT"
    elif not capture_ok:
        status = "BLOCKED_ORIGINAL_PC34_VIEWPORT_CAPTURE_NOT_ROUTE_PROVEN"
    elif not state_oracle_ok:
        status = "BLOCKED_ORIGINAL_PC34_STATE_ORACLE_REQUIRED"
    elif missing_wall_indices:
        status = "BLOCKED_GRAPHICS_DAT_WALL_DECODE_RECORDS_REQUIRED"
    else:
        status = "PASS_ORIGINAL_PC34_VIEWPORT_CAPTURE_PROMOTION_READY"
    audit_reason = (
        "required pass304 route labels have captured crop hashes and pass308 capture coverage is true; pass312 binds the original runtime command/party-tuple/F0128 state oracle. Duplicate hashes remain semantic evidence only until comparator use."
        if capture_ok and state_oracle_ok else
        ("required pass304 route labels have captured crop hashes, but the original runtime party tuple/F0128 state is not source-bound yet; keep hashes as evidence only" if route_label_coverage else
        "tracked original viewport manifests do not contain every route label for the pass127/pass304 required snapshot names; crops cannot be bound to the required party tuple/F0128 state.")
    )

    manifest = {
        "pass": "pass304_dm1_v1_original_viewport_capture_blocker_manifest",
        "status": status,
        "notClaimed": [
            "pixel parity",
            "decoded full GRAPHICS.DAT bitmap byte dump publication",
        ],
        "sourceInputs": {
            "renderPlan": str(RENDER_PLAN.relative_to(REPO)),
            "renderPlanStatus": render["status"],
            "entryGraphicsIndexManifest": str(GRAPHICS_INDEX.relative_to(REPO)),
            "wallGraphicsIndexManifest": str(WALL_GRAPHICS_INDEX.relative_to(REPO)),
            "wallGraphicsIndexStatus": wall_graphics_index.get("status"),
            "pass127StateTables": [
                "parity-evidence/pass127_turn_viewport_orientation_probe.md",
                "parity-evidence/pass127_viewport_world_f0115_row_mapping.md",
            ],
            "redmcsbAnchors": {
                "DUNVIEW.C": file_record(DUNVIEW),
                "GAMELOOP.C": file_record(GAMELOOP),
                "DRAWVIEW.C": file_record(DRAWVIEW),
                "COMMAND.C": file_record(COMMAND),
                "CLIKMENU.C": file_record(CLIKMENU),
                "CHAMPION.C": file_record(CHAMPION),
                "MOVESENS.C": file_record(MOVESENS),
            },
        },
        "requiredOriginalCrop": {
            "fullFrame": {"width": 320, "height": 200},
            "viewportCrop": {"x": 0, "y": 33, "width": 224, "height": 136},
            "source": "scripts/dosbox_dm1_original_viewport_reference_capture.sh normalize_existing_raw_images crop=(0,33,224,169)",
            "requiredFormat": "PPM and PNG; PPM SHA-256 is comparator/hash source of truth",
        },
        "requiredRouteStates": snapshots,
        "missingOriginalCaptureContractSnapshots": missing_capture_contract_snapshots,
        "requiredOriginalAssets": {name: file_record(path) for name, path in ASSET_PATHS.items()},
        "requiredDecodedGraphicsDatRecords": {
            "manifestedEntryGraphicIndices": sorted(entry_manifested_graphics),
            "manifestedWallGraphicIndices": sorted(wall_manifested_graphics),
            "allManifestedGraphicIndices": sorted(manifested_graphics),
            "wallSetGraphicIndicesFromCurrentRenderPlan": sorted(needed_wall_indices),
            "missingWallSetGraphicIndices": missing_wall_indices,
            "blocker": ("decoded bitmap/palette byte manifests are still missing for wall-set graphic indices " + ",".join(map(str, missing_wall_indices)) + "; pass302 proves entry indices and pass305 proves bounded wall-set decode records.") if missing_wall_indices else "all wall-set graphic indices used by pass304 snapshots have decoded bitmap/palette byte records via pass305.",
        },
        "stateOracleSupport": {
            "manifest": "parity-evidence/verification/pass312_dm1_v1_original_runtime_state_oracle.json",
            "status": pass312.get("status"),
            "partyTupleF0128StateOracle": state_oracle_ok,
        },
        "captureExecutionSupport": {
            "manifest": "parity-evidence/verification/pass308_original_capture_execution_manifest.json",
            "status": pass308.get("status"),
            "coverage": capture_coverage,
        },
        "existingOriginalCropAudit": {
            "usableForPromotion": capture_ok and state_oracle_ok,
            "fullComparatorPromotionBlockedByDecodedGraphics": bool(missing_wall_indices),
            "routeLabelCoverage": route_label_coverage,
            "reason": audit_reason,
            "manifestsScanned": sorted({r["manifest"] for r in existing}),
            "uniqueCropSha256": candidate_hashes,
            "requiredLabelHits": exact_label_hits,
        },
        "nextCaptureCommands": {
            "knownBlocker": "Current script requires exactly six shots and has no reset/state-restore marker. The pass127 comparator set branches from the fresh start tuple, so exact promotion needs three independent batches; route capture now uses a proven title->entrance->gameplay prelude plus keypad tokens, but party-tuple promotion still needs a source-bound runtime state oracle.",
            "batchA_start_right_forward": command_for_batch("A", ["kp6", "kp8"], ["start_south", "turn_right_west", "move_forward_west"]),
            "batchB_start_left": command_for_batch("B", ["kp4"], ["start_south", "turn_left_east"]),
            "batchC_start_blocked_forward": command_for_batch("C", ["kp8"], ["start_south", "blocked_forward_south_wall"]),
        },
        "promotionCondition": "Original capture labels and party tuple/F0128 state oracle are source/runtime-bound by pass308/pass312; final comparator promotion also requires missingOriginalCaptureContractSnapshots to be empty, requiredDecodedGraphicsDatRecords.missingWallSetGraphicIndices to be empty, and matched original-vs-Firestaff viewport comparator to pass.",
    }
    OUT.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    print(f"wrote {OUT.relative_to(REPO)}")
    print(manifest["status"])
    print("route label coverage:", route_label_coverage)
    print("required states:", len(snapshots))
    print("wall graphics:", ",".join(map(str, sorted(needed_wall_indices))))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
