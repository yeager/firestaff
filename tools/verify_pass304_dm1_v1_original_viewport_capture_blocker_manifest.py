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
DUNVIEW = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Source/DUNVIEW.C")
GAMELOOP = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Source/GAMELOOP.C")
DRAWVIEW = (Path.home() / ".openclaw/data/firestaff-redmcsb-source/Toolchains/Common/Source/DRAWVIEW.C")
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


def file_record(path: Path) -> dict[str, object]:
    target = Path(path)
    resolved = target.resolve() if target.exists() else target
    return {
        "path": str(target),
        "resolvedPath": str(resolved),
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
                for row in csv.DictReader(f, delimiter="\t"):
                    labels[row.get("filename", "")] = row.get("route_label", "")
        with manifest.open(newline="") as f:
            for row in csv.DictReader(f, delimiter="\t"):
                rows.append({
                    "manifest": str(manifest.relative_to(REPO)),
                    "filename": row["filename"],
                    "routeLabel": labels.get(row["filename"], ""),
                    "width": int(row["width"]),
                    "height": int(row["height"]),
                    "sha256": row["sha256"],
                })
    return rows


def command_for_batch(batch: str, route_tokens_after_entry: list[str], labels: list[str]) -> str:
    # Keep exactly six shots because the current capture script validates that shape.
    # Repeated final labels are explicitly non-promotable padding until the script accepts
    # arbitrary shot counts or a state-reset marker.
    events = ["wait:7000", "shot:" + labels[0]]
    for token, label in zip(route_tokens_after_entry, labels[1:]):
        events.extend([token, "wait:600", "shot:" + label])
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
    snapshots = []
    needed_wall_indices: set[int] = set()
    for snap in render["comparedSnapshots"]:
        events = snap["renderEvents"]
        wall_events = [e for e in events if e.get("event") == "draw_wall_bitmap"]
        for e in wall_events:
            pr = e.get("pixelRegion") or {}
            idx = pr.get("wallSetGraphicIndex")
            if isinstance(idx, int):
                needed_wall_indices.add(idx)
        spec = SNAPSHOT_CAPTURE[snap["name"]]
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

    existing = load_existing_original_crops()
    required_labels = {s["requiredShotLabel"] for s in snapshots}
    exact_label_hits = [r for r in existing if r["routeLabel"] in required_labels]
    candidate_hashes = sorted({r["sha256"] for r in existing})

    manifest = {
        "pass": "pass304_dm1_v1_original_viewport_capture_blocker_manifest",
        "status": "BLOCKED_ORIGINAL_PC34_VIEWPORT_CAPTURE_NOT_ROUTE_PROVEN",
        "notClaimed": [
            "pixel parity",
            "existing original crop equivalence to pass127 route states",
            "decoded full GRAPHICS.DAT bitmap byte dump publication",
        ],
        "sourceInputs": {
            "renderPlan": str(RENDER_PLAN.relative_to(REPO)),
            "renderPlanStatus": render["status"],
            "pass127StateTables": [
                "parity-evidence/pass127_turn_viewport_orientation_probe.md",
                "parity-evidence/pass127_viewport_world_f0115_row_mapping.md",
            ],
            "redmcsbAnchors": {
                "DUNVIEW.C": file_record(DUNVIEW),
                "GAMELOOP.C": file_record(GAMELOOP),
                "DRAWVIEW.C": file_record(DRAWVIEW),
            },
        },
        "requiredOriginalCrop": {
            "fullFrame": {"width": 320, "height": 200},
            "viewportCrop": {"x": 0, "y": 33, "width": 224, "height": 136},
            "source": "scripts/dosbox_dm1_original_viewport_reference_capture.sh normalize_existing_raw_images crop=(0,33,224,169)",
            "requiredFormat": "PPM and PNG; PPM SHA-256 is comparator/hash source of truth",
        },
        "requiredRouteStates": snapshots,
        "requiredOriginalAssets": {name: file_record(path) for name, path in ASSET_PATHS.items()},
        "requiredDecodedGraphicsDatRecords": {
            "alreadyManifestedForEntryOnly": [r["graphicIndex"] for r in graphics_index["records"]],
            "wallSetGraphicIndicesFromCurrentRenderPlan": sorted(needed_wall_indices),
            "blocker": "pass302 proves only 78/79/107; wall comparator promotion needs decoded bitmap/palette byte manifests for every listed wall-set graphic index used by pass304 snapshots, without broad dumps.",
        },
        "existingOriginalCropAudit": {
            "usableForPromotion": False,
            "reason": "tracked original viewport manifests do not contain route labels for the pass127/pass304 required snapshot names; several crops are duplicate no-party/diagnostic hashes, so they cannot be bound to the required party tuple/F0128 state.",
            "manifestsScanned": sorted({r["manifest"] for r in existing}),
            "uniqueCropSha256": candidate_hashes,
            "requiredLabelHits": exact_label_hits,
        },
        "nextCaptureCommands": {
            "knownBlocker": "Current script requires exactly six shots and has no reset/state-restore marker. The pass127 comparator set branches from the fresh start tuple, so exact promotion needs three independent batches or a script enhancement that supports multiple fresh-run batches with per-shot labels.",
            "batchA_start_right_forward": command_for_batch("A", ["right", "up"], ["start_south", "turn_right_west", "move_forward_west"]),
            "batchB_start_left": command_for_batch("B", ["left"], ["start_south", "turn_left_east"]),
            "batchC_start_blocked_forward": command_for_batch("C", ["up"], ["start_south", "blocked_forward_south_wall"]),
        },
        "promotionCondition": "Promote only after each requiredRouteStates row has an original PC34 crop hash whose route label and party tuple are proven, and requiredDecodedGraphicsDatRecords.wallSetGraphicIndicesFromCurrentRenderPlan have deterministic compressed/packed/unpacked/palette byte hashes.",
    }
    OUT.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    print(f"wrote {OUT.relative_to(REPO)}")
    print(manifest["status"])
    print("required states:", len(snapshots))
    print("wall graphics:", ",".join(map(str, sorted(needed_wall_indices))))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
