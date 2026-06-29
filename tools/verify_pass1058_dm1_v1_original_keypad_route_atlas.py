#!/usr/bin/env python3
"""Pass1058: DM1 V1 original keypad route atlas.

This is an evidence gate, not a creature-capture closure. It locks the
original DOSBox keypad behaviour that the next creature-route attempt must use
and preserves the failed first corrected route as a documented blocker.
"""
from __future__ import annotations

import csv
import json
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS = "pass1058_dm1_v1_original_keypad_route_atlas"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

CORRECTED_ROUTE_TO_STAIR_KEYS = [
    "Keypad-8", "Keypad-8", "Keypad-8", "Keypad-8", "Keypad-4",
    "Keypad-2", "Keypad-2", "Keypad-2", "Keypad-4", "Keypad-8",
    "Keypad-8", "Keypad-8", "Keypad-8", "Keypad-8", "Keypad-4",
    "Keypad-8", "Keypad-8", "Keypad-4", "Keypad-8", "Keypad-4",
    "Keypad-2", "Keypad-2", "Keypad-4", "Keypad-2", "Keypad-4",
    "Keypad-8", "Keypad-4", "Keypad-8", "Keypad-8", "Keypad-8",
    "Keypad-8", "Keypad-8", "Keypad-4", "Keypad-8", "Keypad-8",
    "Keypad-8", "Keypad-8", "Keypad-8", "Keypad-4", "Keypad-2",
    "Keypad-2", "Keypad-2", "Keypad-2", "Keypad-2", "Keypad-2",
    "Keypad-4", "Keypad-2",
]

CORRECTED_ROUTE_STAIR_TO_DOOR_KEYS = [
    "Keypad-8", "Keypad-4", "Keypad-8", "Keypad-8", "Keypad-8",
    "Keypad-4",
]

DOOR_PROBE_ACTIONS = [
    "Enter",
    "Space",
    "click:112,70",
    "click:112,120",
    "Keypad-8",
]


def load_tsv(name: str) -> list[dict[str, str]]:
    path = VERIFY_DIR / name
    if not path.exists():
        raise AssertionError(f"missing {path.relative_to(ROOT)}")
    with path.open("r", encoding="utf-8", newline="") as f:
        return list(csv.DictReader(f, delimiter="\t"))


def load_json(name: str) -> dict[str, Any]:
    path = VERIFY_DIR / name
    if not path.exists():
        raise AssertionError(f"missing {path.relative_to(ROOT)}")
    return json.loads(path.read_text(encoding="utf-8"))


def labels(name: str) -> list[str]:
    return [row["route_label"] for row in load_tsv(name)]


def shas(name: str) -> list[str]:
    return [row["sha256"] for row in load_tsv(name)]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def check_route_token_receipt() -> dict[str, Any]:
    require(len(CORRECTED_ROUTE_TO_STAIR_KEYS) == 47, "route-to-stair key count drifted")
    require(len(CORRECTED_ROUTE_STAIR_TO_DOOR_KEYS) == 6, "stair-to-door key count drifted")
    require(
        CORRECTED_ROUTE_TO_STAIR_KEYS[0] == "Keypad-8"
        and CORRECTED_ROUTE_TO_STAIR_KEYS[-1] == "Keypad-2",
        "route-to-stair boundary tokens drifted",
    )
    require(
        CORRECTED_ROUTE_STAIR_TO_DOOR_KEYS == [
            "Keypad-8", "Keypad-4", "Keypad-8", "Keypad-8", "Keypad-8", "Keypad-4",
        ],
        "stair-to-door sequence drifted",
    )
    require(
        DOOR_PROBE_ACTIONS == ["Enter", "Space", "click:112,70", "click:112,120", "Keypad-8"],
        "door-probe action sequence drifted",
    )
    return {
        "ok": True,
        "source": "redacted from local route-token logs; no proprietary frames embedded",
        "start_pose_route": {
            "to_stair_entry_keys": CORRECTED_ROUTE_TO_STAIR_KEYS,
            "stair_entry_to_creature_door_keys": CORRECTED_ROUTE_STAIR_TO_DOOR_KEYS,
            "total_key_count": (
                len(CORRECTED_ROUTE_TO_STAIR_KEYS)
                + len(CORRECTED_ROUTE_STAIR_TO_DOOR_KEYS)
            ),
        },
        "door_probe_actions": DOOR_PROBE_ACTIONS,
        "post_dungeon_runner_note": (
            "These tokens preserve the pass1058 start-pose route. The newer "
            "dosbox_capture_session.py --post-dungeon-route hook starts after "
            "the pass1073 live start/first-movement proof, so operators must "
            "align the starting pose before replay rather than pasting this "
            "sequence blindly."
        ),
    }


def check_key_atlas() -> dict[str, Any]:
    rows = load_tsv("key_atlas_raw_manifest.tsv")
    labs = labels("key_atlas_shot_labels.tsv")
    hashes = shas("key_atlas_raw_manifest.tsv")
    classifier = load_json("key_atlas_classifier.json")

    require(len(rows) == 5, "key atlas must have 5 raw captures")
    require(
        labs == ["start", "after_kp5", "after_kp8", "after_forward_click", "after_kp2"],
        f"unexpected key atlas labels: {labs}",
    )
    require(hashes[0] == hashes[4], "kp2 must return to the start frame")
    require(
        hashes[1] == hashes[2] == hashes[3],
        "kp5, kp8, and forward-click must reach the same forward frame",
    )
    require(hashes[0] != hashes[1], "forward frame must differ from start")
    require(
        classifier.get("class_counts") == {"dungeon_gameplay": 5},
        f"unexpected key atlas classes: {classifier.get('class_counts')}",
    )

    return {
        "labels": labs,
        "raw_sha256": hashes,
        "class_counts": classifier.get("class_counts"),
        "conclusion": "kp5/kp8/forward-click move forward; kp2 moves backward",
        "ok": True,
    }


def check_turn_atlas() -> dict[str, Any]:
    rows = load_tsv("turn_atlas_raw_manifest.tsv")
    labs = labels("turn_atlas_shot_labels.tsv")
    hashes = shas("turn_atlas_raw_manifest.tsv")
    classifier = load_json("turn_atlas_classifier.json")

    require(len(rows) == 5, "turn atlas must have 5 raw captures")
    require(
        labs == ["start", "after_kp4", "after_kp6", "after_kp6_again", "after_kp4_again"],
        f"unexpected turn atlas labels: {labs}",
    )
    require(
        hashes[0] == hashes[2] == hashes[4],
        "kp6 must undo kp4, and kp4 must undo the second kp6",
    )
    require(hashes[1] == hashes[3], "kp4 and second kp6 wall views must match")
    require(hashes[0] != hashes[1], "turn wall frame must differ from start")
    require(
        classifier.get("class_counts") == {"dungeon_gameplay": 3, "wall_closeup": 2},
        f"unexpected turn atlas classes: {classifier.get('class_counts')}",
    )

    return {
        "labels": labs,
        "raw_sha256": hashes,
        "class_counts": classifier.get("class_counts"),
        "conclusion": "kp4 is right-turn from the start pose; kp6 is left-turn/back",
        "ok": True,
    }


def check_creature_route() -> dict[str, Any]:
    rows = load_tsv("creature_route_raw_manifest.tsv")
    labs = labels("creature_route_shot_labels.tsv")
    hashes = shas("creature_route_raw_manifest.tsv")
    classifier = load_json("creature_route_classifier.json")

    require(len(rows) == 4, "corrected creature route must have 4 raw captures")
    require(
        labs == ["start", "stair_entry", "creature_door_closed", "creature_after_door_click"],
        f"unexpected corrected creature route labels: {labs}",
    )
    require(len(set(hashes)) == 3, "corrected creature route must reach 3 distinct states")
    require(
        hashes[2] == hashes[3],
        "door-click test must leave the corrected route door frame unchanged",
    )
    require(
        classifier.get("class_counts") == {"dungeon_gameplay": 1, "wall_closeup": 3},
        f"unexpected corrected creature route classes: {classifier.get('class_counts')}",
    )

    return {
        "labels": labs,
        "raw_sha256": hashes,
        "class_counts": classifier.get("class_counts"),
        "conclusion": "corrected route reaches new states, but the first target remains behind an inert door",
        "ok": True,
    }


def check_door_probe() -> dict[str, Any]:
    rows = load_tsv("door_probe_raw_manifest.tsv")
    labs = labels("door_probe_shot_labels.tsv")
    hashes = shas("door_probe_raw_manifest.tsv")

    require(len(rows) == 8, "door probe must have 8 raw captures")
    require(
        labs
        == [
            "start",
            "stair_entry",
            "creature_door_closed",
            "after_enter",
            "after_space",
            "after_click_high",
        ],
        f"unexpected door probe labels: {labs}",
    )
    require(
        len(labs) == 6,
        "door probe must keep the six labeled viewport crops that were promoted",
    )
    require(
        len(set(hashes[2:])) == 1,
        "door probe actions after the door frame must all leave the raw frame unchanged",
    )
    require(
        hashes[6] == hashes[2] and hashes[7] == hashes[2],
        "the two raw-only door-probe tail frames must remain on the inert door hash",
    )

    return {
        "labels": labs,
        "raw_sha256": hashes,
        "stable_door_sha256": hashes[2],
        "raw_only_tail_row_count": len(rows) - len(labs),
        "raw_only_tail_rows": [
            {
                "raw_index": rows[6]["index"],
                "expected_action": "click:112,120",
                "sha256": hashes[6],
                "matches_stable_door": hashes[6] == hashes[2],
            },
            {
                "raw_index": rows[7]["index"],
                "expected_action": "Keypad-8",
                "sha256": hashes[7],
                "matches_stable_door": hashes[7] == hashes[2],
            },
        ],
        "conclusion": "enter/space/two clicks/forward do not open this first target door",
        "ok": True,
    }


def main() -> int:
    try:
        result = {
            "schema": f"firestaff.parity.{PASS}.v1",
            "status": "PASS",
            "generated_at": datetime.now(timezone.utc).isoformat(),
            "report": str(REPORT.relative_to(ROOT)),
            "key_atlas": check_key_atlas(),
            "turn_atlas": check_turn_atlas(),
            "route_token_receipt": check_route_token_receipt(),
            "corrected_creature_route": check_creature_route(),
            "door_probe": check_door_probe(),
            "honesty": (
                "This gate locks original keypad/route evidence only. It does "
                "not claim a paired original creature screenshot."
            ),
        }
    except Exception as exc:
        print(f"FAIL {PASS}: {exc}", file=sys.stderr)
        return 1

    MANIFEST.write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"PASS {PASS}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
