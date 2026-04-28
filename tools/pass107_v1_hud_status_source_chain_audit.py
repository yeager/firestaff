#!/usr/bin/env python3
"""Audit V1 champion HUD/status overlay zones against layout-696 source records.

Evidence only: this checks that the pass83 top-row HUD overlay is still backed by
`zones_h_reconstruction.json` records for C151..C166, C187..C206, and C207..C218.
It intentionally does not claim original-runtime pixel parity.
"""
from __future__ import annotations

import json
from pathlib import Path
from typing import Any

REPO = Path(__file__).resolve().parent.parent
ZONES = REPO / "zones_h_reconstruction.json"
PASS83 = REPO / "parity-evidence" / "overlays" / "pass83" / "pass83_champion_hud_zone_overlay_stats.json"
OUT_JSON = REPO / "parity-evidence" / "overlays" / "pass107_v1_hud_status_source_chain_audit.json"
OUT_MD = REPO / "parity-evidence" / "pass107_v1_hud_status_source_chain_audit.md"


def rec(records: dict[str, Any], zone_id: int) -> dict[str, int]:
    value = records.get(str(zone_id))
    if not isinstance(value, dict):
        raise AssertionError(f"missing C{zone_id}")
    return {"type": int(value["type"]), "parent": int(value["parent"]), "d1": int(value["d1"]), "d2": int(value["d2"])}


def expect(records: dict[str, Any], zone_id: int, expected: dict[str, int], problems: list[str]) -> dict[str, int]:
    got = rec(records, zone_id)
    if got != expected:
        problems.append(f"C{zone_id} {got} != {expected}")
    return got


def load_pass83_zones() -> tuple[dict[str, list[int]], dict[str, Any]]:
    data = json.loads(PASS83.read_text())
    zones = {z["name"]: z["xywh"] for z in data["zones"]}
    if data.get("schema") != "pass83_champion_hud_zone_overlay_probe.v3":
        raise AssertionError(f"unexpected pass83 schema: {data.get('schema')}")
    if data.get("pass") is not True:
        raise AssertionError("pass83 stats are not passing")
    return zones, data


def main() -> int:
    src = json.loads(ZONES.read_text())
    records = src["records"]
    pass83, pass83_data = load_pass83_zones()
    problems: list[str] = []

    checked: dict[str, Any] = {
        "layout_provenance": src.get("$provenance", {}),
        "status_template": expect(records, 150, {"type": 9, "parent": 0, "d1": 67, "d2": 29}, problems),
        "status_slots": [],
        "name_zones": [],
        "bar_graph_chains": [],
        "hand_zones": [],
        "pass83_overlay_zones": {},
    }

    for slot, x in enumerate((0, 69, 138, 207)):
        checked["status_slots"].append({
            "zone": 151 + slot,
            "record": expect(records, 151 + slot, {"type": 1, "parent": 150, "d1": x, "d2": 0}, problems),
            "pass83_xywh": pass83.get(f"C{151 + slot}_status_box_slot{slot}"),
        })
        if pass83.get(f"C{151 + slot}_status_box_slot{slot}") != [x, 0, 67, 29]:
            problems.append(f"pass83 C{151 + slot} slot{slot} xywh drifted")

        clear_template = 155 + slot
        clear_zone = 159 + slot
        text_zone = 163 + slot
        checked["name_zones"].append({
            "slot": slot,
            "clear_template_zone": clear_template,
            "clear_template_record": expect(records, clear_template, {"type": 9, "parent": 151 + slot, "d1": 43, "d2": 7}, problems),
            "clear_zone": clear_zone,
            "clear_record": expect(records, clear_zone, {"type": 1, "parent": clear_template, "d1": 0, "d2": 0}, problems),
            "text_zone": text_zone,
            "text_record": expect(records, text_zone, {"type": 18, "parent": clear_zone, "d1": 1, "d2": 0}, problems),
            "pass83_clear_xywh": pass83.get(f"C{clear_zone}_name_clear_slot{slot}"),
            "pass83_text_xywh": pass83.get(f"C{text_zone}_name_text_slot{slot}"),
        })

        graph_template = 183 + slot
        graph_anchor = 187 + slot
        bar_template = 191 + slot
        checked["bar_graph_chains"].append({
            "slot": slot,
            "graph_template_zone": graph_template,
            "graph_template_record": expect(records, graph_template, {"type": 9, "parent": 151 + slot, "d1": 24, "d2": 29}, problems),
            "graph_anchor_zone": graph_anchor,
            "graph_anchor_record": expect(records, graph_anchor, {"type": 1, "parent": graph_template, "d1": 43, "d2": 0}, problems),
            "bar_template_zone": bar_template,
            "bar_template_record": expect(records, bar_template, {"type": 9, "parent": graph_anchor, "d1": 4, "d2": 25}, problems),
            "value_zones": [195 + slot, 199 + slot, 203 + slot],
            "value_records": [
                expect(records, 195 + slot, {"type": 7, "parent": bar_template, "d1": 5, "d2": 26}, problems),
                expect(records, 199 + slot, {"type": 7, "parent": bar_template, "d1": 12, "d2": 26}, problems),
                expect(records, 203 + slot, {"type": 7, "parent": bar_template, "d1": 19, "d2": 26}, problems),
            ],
            "pass83_value_xywh": [
                pass83.get(f"C{195 + slot}_hp_bar_slot{slot}"),
                pass83.get(f"C{199 + slot}_stamina_bar_slot{slot}"),
                pass83.get(f"C{203 + slot}_mana_bar_slot{slot}"),
            ],
        })

        hand_template = 207 + slot
        ready_zone = 211 + slot * 2
        action_zone = 212 + slot * 2
        checked["hand_zones"].append({
            "slot": slot,
            "hand_template_zone": hand_template,
            "hand_template_record": expect(records, hand_template, {"type": 9, "parent": 151 + slot, "d1": 16, "d2": 16}, problems),
            "ready_zone": ready_zone,
            "ready_record": expect(records, ready_zone, {"type": 1, "parent": hand_template, "d1": 4, "d2": 10}, problems),
            "action_zone": action_zone,
            "action_record": expect(records, action_zone, {"type": 1, "parent": hand_template, "d1": 24, "d2": 10}, problems),
            "pass83_ready_xywh": pass83.get(f"C{ready_zone}_ready_hand_slot{slot}"),
            "pass83_action_xywh": pass83.get(f"C{action_zone}_action_hand_slot{slot}"),
        })

    checked["pass83_overlay_zones"] = {
        "schema": pass83_data.get("schema"),
        "geometry_acceptance": pass83_data.get("geometry_acceptance"),
    }
    result = {
        "schema": "pass107_v1_hud_status_source_chain_audit.v1",
        "honesty": "Source-chain audit only. It ties pass83 HUD/status overlay zones to layout-696 records; it does not promote original-runtime pixel parity.",
        "source": str(ZONES.relative_to(REPO)),
        "pass83_stats": str(PASS83.relative_to(REPO)),
        "checked": checked,
        "remaining_hud_blocker": "Independent Python re-resolution of type=7 proportional bar-value records is still not implemented here; bar pixel-fill parity remains covered by M11 C probes and pass83 crops, not by original runtime screenshots.",
        "problems": problems,
        "pass": not problems,
    }
    OUT_JSON.write_text(json.dumps(result, indent=2) + "\n")

    lines = [
        "# Pass 107 — V1 HUD/status source-chain audit",
        "",
        "Scope: DM1 PC 3.4 V1 champion top row / HUD status panel evidence only.",
        "",
        "This pass adds a bounded source-chain audit between `zones_h_reconstruction.json` and the existing pass83 champion HUD overlay. It avoids another raw log dump: the JSON records the exact layout-696 records checked and the pass83 overlay zones they support.",
        "",
        "## Result",
        "",
        f"- audit JSON: `{OUT_JSON.relative_to(REPO)}`",
        f"- source: `{ZONES.relative_to(REPO)}`",
        f"- pass83 stats: `{PASS83.relative_to(REPO)}`",
        f"- pass: `{result['pass']}`",
        f"- problems: `{len(problems)}`",
        "",
        "## Locked source chains",
        "",
        "- `C150` status-box template is `67x29`; `C151..C154` place slots at x `0/69/138/207`.",
        "- `C155..C166` name clear/text chain is present under each status slot; text is clipped with the source `+1` x offset.",
        "- `C183..C206` bar-graph/value chain is present for all four slots and three stats (`C195..C206`).",
        "- `C207..C218` ready/action hand source chain is present for all four slots.",
        "- Pass83 remains schema `pass83_champion_hud_zone_overlay_probe.v3` and passing.",
        "",
        "## Remaining HUD-specific honesty boundary",
        "",
        result["remaining_hud_blocker"],
        "",
        "## Gate",
        "",
        "```sh",
        "python3 tools/pass107_v1_hud_status_source_chain_audit.py",
        "python3 -m json.tool parity-evidence/overlays/pass107_v1_hud_status_source_chain_audit.json >/dev/null",
        "git diff --check",
        "```",
        "",
    ]
    OUT_MD.write_text("\n".join(lines))
    print(json.dumps({"pass": result["pass"], "problems": problems, "json": str(OUT_JSON.relative_to(REPO)), "markdown": str(OUT_MD.relative_to(REPO))}, indent=2))
    return 0 if result["pass"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
