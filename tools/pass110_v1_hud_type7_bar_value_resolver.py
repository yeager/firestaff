#!/usr/bin/env python3
"""Resolve DM1 V1 champion HUD type-7 bar value zones from source records.

This is evidence-only.  Pass 107 proved the C183..C206 chain is present;
this pass independently recomputes the top-left rectangles and sample filled
pixels for the type=7 HP/stamina/mana records instead of trusting the overlay
JSON alone.  It still does not claim original-runtime screenshot parity.
"""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any

REPO = Path(__file__).resolve().parent.parent
ZONES = REPO / "zones_h_reconstruction.json"
PASS83 = REPO / "parity-evidence" / "overlays" / "pass83" / "pass83_champion_hud_zone_overlay_stats.json"
OUT_JSON = REPO / "parity-evidence" / "overlays" / "pass110_v1_hud_type7_bar_value_resolver.json"
OUT_MD = REPO / "parity-evidence" / "pass110_v1_hud_type7_bar_value_resolver.md"

SLOT_XS = [0, 69, 138, 207]
STATS = [
    ("hp", 195, 100, 100),
    ("stamina", 199, 50, 100),
    ("mana", 203, 1, 100),
]


def rec(records: dict[str, Any], zone_id: int) -> dict[str, int]:
    value = records.get(str(zone_id))
    if not isinstance(value, dict):
        raise AssertionError(f"missing C{zone_id}")
    return {"type": int(value["type"]), "parent": int(value["parent"]), "d1": int(value["d1"]), "d2": int(value["d2"])}


def overlay_map() -> dict[str, list[int]]:
    data = json.loads(PASS83.read_text())
    if data.get("schema") != "pass83_champion_hud_zone_overlay_probe.v3" or data.get("pass") is not True:
        raise AssertionError("pass83 champion HUD overlay is not the expected passing v3 schema")
    return {z["name"]: z["xywh"] for z in data["zones"]}


def filled_rect(container: list[int], current: int, maximum: int) -> list[int]:
    """Mirror CHAMDRAW/F0287-style bottom-anchored proportional fill.

    Non-zero current values get at least one pixel of fill; zero fills nothing.
    """
    x, y, w, h = container
    if maximum <= 0 or current <= 0:
        fill_h = 0
    else:
        fill_h = (current * h) // maximum
        if fill_h <= 0:
            fill_h = 1
        if fill_h > h:
            fill_h = h
    return [x, y + h - fill_h, w, fill_h]


def main() -> int:
    src = json.loads(ZONES.read_text())
    records = src["records"]
    overlays = overlay_map()
    problems: list[str] = []
    resolved: list[dict[str, Any]] = []

    for slot, slot_x in enumerate(SLOT_XS):
        status = rec(records, 151 + slot)
        graph_template = rec(records, 183 + slot)
        graph_anchor = rec(records, 187 + slot)
        bar_template = rec(records, 191 + slot)

        expected_status = {"type": 1, "parent": 150, "d1": slot_x, "d2": 0}
        if status != expected_status:
            problems.append(f"C{151+slot} drift: {status} != {expected_status}")
        if graph_template != {"type": 9, "parent": 151 + slot, "d1": 24, "d2": 29}:
            problems.append(f"C{183+slot} drift: {graph_template}")
        if graph_anchor != {"type": 1, "parent": 183 + slot, "d1": 43, "d2": 0}:
            problems.append(f"C{187+slot} drift: {graph_anchor}")
        if bar_template != {"type": 9, "parent": 187 + slot, "d1": 4, "d2": 25}:
            problems.append(f"C{191+slot} drift: {bar_template}")

        anchor_x = slot_x + graph_anchor["d1"]
        anchor_y = status["d2"] + graph_anchor["d2"]
        region_h = graph_template["d2"]
        bar_w = bar_template["d1"]
        bar_h = bar_template["d2"]
        top_y = anchor_y + region_h - bar_h

        for stat_name, base_zone, sample_current, sample_maximum in STATS:
            zone = base_zone + slot
            value = rec(records, zone)
            expected_parent = 191 + slot
            if value["type"] != 7 or value["parent"] != expected_parent:
                problems.append(f"C{zone} type/parent drift: {value}")
            if value["d2"] != 26:
                problems.append(f"C{zone} bottom-anchor d2 drift: {value['d2']} != 26")
            left_x = anchor_x + value["d1"] - (bar_w // 2)
            rect = [left_x, top_y, bar_w, bar_h]
            overlay_key = f"C{zone}_{stat_name}_bar_slot{slot}"
            overlay_rect = overlays.get(overlay_key)
            if overlay_rect != rect:
                problems.append(f"{overlay_key} overlay drift: {overlay_rect} != {rect}")
            resolved.append({
                "slot": slot,
                "stat": stat_name,
                "zone": zone,
                "source_record": value,
                "computed_xywh": rect,
                "pass83_xywh": overlay_rect,
                "sample_current": sample_current,
                "sample_maximum": sample_maximum,
                "sample_filled_xywh": filled_rect(rect, sample_current, sample_maximum),
            })

    result = {
        "schema": "pass110_v1_hud_type7_bar_value_resolver.v1",
        "honesty": "Resolves layout-696 type=7 champion HUD bar value records to rectangles and sample fills; does not claim original-runtime screenshot parity.",
        "sources": {
            "zones": str(ZONES.relative_to(REPO)),
            "pass83_overlay_stats": str(PASS83.relative_to(REPO)),
        },
        "algorithm": {
            "bar_left_x": "slot_x + C187.d1 + C19x.d1 - (C191.d1 // 2)",
            "bar_top_y": "C151.d2 + C187.d2 + C183.d2 - C191.d2",
            "bar_size": "C191.d1 x C191.d2 == 4x25",
            "fill": "bottom-anchored proportional fill; non-zero current gets at least 1px",
        },
        "resolved_bars": resolved,
        "problems": problems,
        "pass": not problems,
    }
    OUT_JSON.write_text(json.dumps(result, indent=2) + "\n")

    lines = [
        "# Pass 110 — V1 HUD type-7 bar-value resolver",
        "",
        "Scope: DM1 PC 3.4 V1 champion top-row/status panel evidence only.",
        "",
        "This pass closes the Pass 107 honesty gap by independently resolving the layout-696 `type=7` HP/stamina/mana value zones into the same 4×25 rectangles used by the active champion HUD overlay.",
        "",
        "## Result",
        "",
        f"- resolver JSON: `{OUT_JSON.relative_to(REPO)}`",
        f"- source zones: `{ZONES.relative_to(REPO)}`",
        f"- overlay checked: `{PASS83.relative_to(REPO)}`",
        f"- resolved bars: `{len(resolved)}`",
        f"- pass: `{result['pass']}`",
        f"- problems: `{len(problems)}`",
        "",
        "## Locked geometry",
        "",
        "- slot origins remain `0/69/138/207` from `C151..C154`.",
        "- each bar region remains `C183..C186` → `C187..C190` → `C191..C194`.",
        "- each `type=7` value zone (`C195..C206`) resolves to a 4×25 bottom-flush container at x offsets `46/53/60` within each 67×29 status box.",
        "- sample fill math locks full HP (`25px`), half stamina (`12px`), and min-1px mana behavior for non-zero values.",
        "",
        "## Honesty boundary",
        "",
        "This is source-chain + overlay-geometry evidence, not original DOS runtime pixel parity. Original-runtime top-row overlay comparison remains blocked on a stable original gameplay reference frame.",
        "",
        "## Gate",
        "",
        "```sh",
        "python3 tools/pass110_v1_hud_type7_bar_value_resolver.py",
        "python3 -m json.tool parity-evidence/overlays/pass110_v1_hud_type7_bar_value_resolver.json >/dev/null",
        "git diff --check",
        "```",
        "",
    ]
    OUT_MD.write_text("\n".join(lines))
    print(json.dumps({"pass": result["pass"], "problems": problems, "json": str(OUT_JSON.relative_to(REPO)), "markdown": str(OUT_MD.relative_to(REPO))}, indent=2))
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
