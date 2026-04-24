#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
INVENTORY_PATH = ROOT / "assets-v2/creatures/wave1/creature_family_inventory.json"
INDEX_PATH = ROOT / "assets-v2/creatures/wave1/WAVE1_CREATURE_FAMILY_INDEX.md"
AGGREGATE_MANIFEST_PATH = ROOT / "assets-v2/manifests/firestaff-v2-wave1-creatures.manifest.json"


def load_inventory() -> list[dict[str, str]]:
    entries = json.loads(INVENTORY_PATH.read_text())
    entries.sort(key=lambda item: item["slug"])
    return entries


def run_family_generator(entry: dict[str, str]) -> None:
    cmd = [
        "python3",
        str(ROOT / "tools/generate_v2_creature_family.py"),
        "--source",
        entry["source"],
        "--slug",
        entry["slug"],
        "--display-name",
        entry["displayName"],
        "--family-dir",
        f"assets-v2/creatures/wave1/{entry['slug']}-family",
        "--origin",
        entry["origin"],
    ]
    subprocess.run(cmd, cwd=ROOT, check=True)


def load_family_manifest(slug: str) -> dict:
    path = ROOT / "assets-v2/manifests" / f"firestaff-v2-wave1-{slug}-family.manifest.json"
    return json.loads(path.read_text())


def write_aggregate_manifest(entries: list[dict[str, str]]) -> None:
    assets = []
    for entry in entries:
        assets.extend(load_family_manifest(entry["slug"])["assets"])
    manifest = {
        "manifestVersion": "1.0.0",
        "packId": "firestaff-v2-wave1-creatures",
        "description": "Full currently discoverable Firestaff V2 Wave 1 creature-family pack for offline prep, upscale, and export workflow coverage.",
        "targetPolicy": {
            "masterResolution": "4k-first",
            "derivedResolutions": ["1080p"],
            "layoutSkeleton": "dm1-depth-ladder",
        },
        "assets": assets,
    }
    AGGREGATE_MANIFEST_PATH.write_text(json.dumps(manifest, indent=2) + "\n")


def write_index(entries: list[dict[str, str]]) -> None:
    lines = [
        "# Firestaff V2 Wave 1 creature family index",
        "",
        "This index tracks the full currently discoverable V2 creature-family source set covered by the bounded Wave 1 prep/upscale/export workflow.",
        "",
        "## Coverage",
        "",
        f"- families in scope: **{len(entries)}**",
        f"- generated asset outputs: **{len(entries) * 3}** front-view stills (`near`, `mid`, `far`)",
        f"- resolution outputs: **{len(entries) * 6}** image files across 4K masters and 1080p derivatives",
        "- timing rule: smoother V2 animation is allowed later, but perceived gameplay timing and travel/attack speed must stay aligned to the original",
        "",
        "## Family inventory",
        "",
        "| Family | Source | Coverage | Notes |",
        "|---|---|---|---|",
    ]
    for entry in entries:
        source = entry["source"]
        notes = "card art source" if "/creatures/" in source else "reference sprite source"
        lines.append(
            f"| {entry['displayName']} | `{source}` | front near/mid/far → 4K + 1080p | {notes} |"
        )
    lines.extend(
        [
            "",
            "## Regeneration",
            "",
            "```bash",
            "python3 tools/generate_v2_creature_wave1_pack.py",
            "```",
            "",
            "The pack generator reads `creature_family_inventory.json`, regenerates every family directory/manifest pair, then refreshes the aggregate manifest in `assets-v2/manifests/`.",
        ]
    )
    INDEX_PATH.write_text("\n".join(lines) + "\n")


def main() -> None:
    entries = load_inventory()
    for entry in entries:
        run_family_generator(entry)
    write_aggregate_manifest(entries)
    write_index(entries)
    print(f"generated {len(entries)} creature families")


if __name__ == "__main__":
    main()
