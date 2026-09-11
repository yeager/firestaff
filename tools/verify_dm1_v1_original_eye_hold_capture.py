#!/usr/bin/env python3
"""Audit an operator-local original DM1 inventory Eye-hold capture sequence.

The proprietary frames and game data deliberately stay outside the repository.
This tool only verifies a supplied three-frame observation:

    inventory -> Eye held -> Eye released

It checks capture geometry, the recorded route tokens and the observable panel
redraw.  It is not a Firestaff comparator and must never be interpreted as an
original-vs-Firestaff pixel-parity result.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
from typing import Any

from PIL import Image, ImageChops


EXPECTED = (
    ("inventory", "image0001-raw.png"),
    ("eye_hold", "image0002-raw.png"),
    ("eye_release", "image0003-raw.png"),
)
PANEL_BOX = (80, 0, 224, 169)
RIGHT_UI_BOX = (224, 0, 320, 200)


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(65536), b""):
            digest.update(chunk)
    return digest.hexdigest()


def changed_pixels(left: Image.Image, right: Image.Image, box: tuple[int, int, int, int]) -> int:
    delta = ImageChops.difference(left, right).crop(box)
    return sum(pixel != (0, 0, 0) for pixel in delta.get_flattened_data())


def load_labels(capture_dir: Path) -> list[dict[str, str]]:
    labels = capture_dir / "original_viewport_shot_labels.tsv"
    rows = labels.read_text(encoding="utf-8").splitlines()
    if not rows or rows[0] != "index\tfilename\troute_label\troute_token":
        raise ValueError("missing or invalid original_viewport_shot_labels.tsv header")
    result = []
    for row in rows[1:]:
        fields = row.split("\t")
        if len(fields) != 4:
            raise ValueError(f"invalid labels row: {row!r}")
        result.append(dict(zip(("index", "filename", "label", "token"), fields)))
    return result


def verify(capture_dir: Path) -> dict[str, Any]:
    problems: list[str] = []
    try:
        labels = load_labels(capture_dir)
    except (OSError, ValueError) as error:
        labels = []
        problems.append(str(error))

    expected_labels = [label for label, _ in EXPECTED]
    if [row.get("label") for row in labels] != expected_labels:
        problems.append("route labels must be inventory, eye_hold, eye_release in that order")
    if [row.get("token") for row in labels] != [f"shot:{label}" for label in expected_labels]:
        problems.append("route labels must retain their matching shot tokens")

    plan_path = capture_dir / "original_viewport_route_plan.json"
    try:
        plan = json.loads(plan_path.read_text(encoding="utf-8"))
        token_names = [str(row.get("token")) for row in plan.get("tokens", [])]
        required_order = ("shot:inventory", "press:16,53", "shot:eye_hold", "release", "shot:eye_release")
        position = -1
        for token in required_order:
            try:
                position = token_names.index(token, position + 1)
            except ValueError:
                problems.append(f"route plan lacks ordered token {token!r}")
                break
    except (OSError, json.JSONDecodeError) as error:
        problems.append(f"invalid route plan: {error}")

    images: list[Image.Image] = []
    frame_rows: list[dict[str, Any]] = []
    for _, filename in EXPECTED:
        path = capture_dir / filename
        if not path.is_file():
            problems.append(f"missing raw frame {filename}")
            continue
        with Image.open(path) as source:
            image = source.convert("RGB")
        if image.size != (320, 200):
            problems.append(f"{filename} has geometry {image.size}, expected 320x200")
        images.append(image)
        frame_rows.append({"file": filename, "sha256": sha256(path), "width": image.width, "height": image.height})

    observations: dict[str, Any] = {}
    if len(images) == 3:
        before, held, released = images
        panel_hold = changed_pixels(before, held, PANEL_BOX)
        panel_release = changed_pixels(held, released, PANEL_BOX)
        panel_restored = changed_pixels(before, released, PANEL_BOX)
        right_hold = changed_pixels(before, held, RIGHT_UI_BOX)
        right_release = changed_pixels(held, released, RIGHT_UI_BOX)
        observations = {
            "panelBox": list(PANEL_BOX),
            "rightUiBox": list(RIGHT_UI_BOX),
            "panelChangedBeforeToHeld": panel_hold,
            "panelChangedHeldToReleased": panel_release,
            "panelChangedBeforeToReleased": panel_restored,
            "rightUiChangedBeforeToHeld": right_hold,
            "rightUiChangedHeldToReleased": right_release,
        }
        if panel_hold < 512 or panel_release < 512:
            problems.append("Eye hold did not produce a substantial central-panel redraw")
        if panel_restored != 0:
            problems.append("central panel was not restored after Eye release")
        if right_hold != 0 or right_release != 0:
            problems.append("Eye observation unexpectedly changed the right UI")
        if len({row["sha256"] for row in frame_rows}) != 3:
            problems.append("raw frame hashes must all be distinct")

    return {
        "schema": "firestaff.local-evidence.dm1-v1-eye-hold.v1",
        "status": "passed" if not problems else "failed",
        "capture": {"frameCount": len(frame_rows), "frames": frame_rows},
        "route": {"expectedLabels": expected_labels, "requiredTransition": "inventory -> Eye held -> Eye released"},
        "observations": observations,
        "problems": problems,
        "nonClaims": [
            "The supplied frames remain operator-local and are not published by this tool.",
            "This verifies an observed original-runtime panel transition only.",
            "This is not original-vs-Firestaff pixel parity.",
            "This does not prove an original debugger trace or close unrelated HoC capture gaps.",
        ],
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("capture_dir", type=Path, help="operator-local three-frame capture directory")
    parser.add_argument("--report", type=Path, help="optional local JSON receipt destination")
    args = parser.parse_args()
    result = verify(args.capture_dir)
    rendered = json.dumps(result, indent=2, sort_keys=True) + "\n"
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(rendered, encoding="utf-8")
    print(rendered, end="")
    return 0 if result["status"] == "passed" else 1


if __name__ == "__main__":
    raise SystemExit(main())
