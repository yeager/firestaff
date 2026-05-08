#!/usr/bin/env python3
"""Verify pass402 DM1 V1 side-content far-to-near ordering lock."""
from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_SOURCE = Path(
    "~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
).expanduser()


def read_slice(path: Path, line_range: str) -> str:
    start_s, end_s = line_range.split("-", 1)
    start = int(start_s)
    end = int(end_s)
    lines = path.read_text(errors="replace").splitlines()
    return "\n".join(lines[start - 1:end])


def ordered_missing(text: str, needles: list[str]) -> list[str]:
    pos = -1
    missing: list[str] = []
    for needle in needles:
        idx = text.find(needle, pos + 1)
        if idx < 0:
            missing.append(needle)
        else:
            pos = idx
    return missing


def local_function(text: str, name: str) -> str:
    m = re.search(rf"^static void {re.escape(name)}\s*\(", text, re.M)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[m.start():i + 1]
    raise AssertionError(f"unterminated function {name}")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--source", type=Path, default=DEFAULT_SOURCE)
    ap.add_argument("--json", action="store_true")
    args = ap.parse_args()

    checks: list[dict[str, Any]] = []
    ok = True

    f0128 = read_slice(args.source / "DUNVIEW.C", "8318-8542")
    missing = ordered_missing(f0128, [
        "F0116_DUNGEONVIEW_DrawSquareD3L",
        "F0117_DUNGEONVIEW_DrawSquareD3R",
        "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
        "F0119_DUNGEONVIEW_DrawSquareD2L",
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
        "F0121_DUNGEONVIEW_DrawSquareD2C",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        "F0124_DUNGEONVIEW_DrawSquareD1C",
        "F0125_DUNGEONVIEW_DrawSquareD0L",
        "F0126_DUNGEONVIEW_DrawSquareD0R",
        "F0127_DUNGEONVIEW_DrawSquareD0C",
    ])
    passed = not missing
    ok = ok and passed
    checks.append({
        "id": "redmcsb-f0128-side-squares-far-to-near",
        "passed": passed,
        "source": "DUNVIEW.C:8318-8542",
        "missing": missing,
        "why": "ReDMCSB replays side/center squares from far D3 through near D0, so same-lane content from farther squares must be painted before nearer content.",
    })

    f0115 = read_slice(args.source / "DUNVIEW.C", "4547-4910")
    missing = [n for n in [
        "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(",
        "P0146_ui_OrderedViewCellOrdinals >>= 4;",
        "P0141_T_Thing = L0146_T_FirstThingToDraw;",
        "if ((AL0127_i_ThingType = M012_TYPE(P0141_T_Thing)) == C04_THING_TYPE_GROUP)",
    ] if n not in f0115]
    passed = not missing
    ok = ok and passed
    checks.append({
        "id": "redmcsb-f0115-content-handoff",
        "passed": passed,
        "source": "DUNVIEW.C:4547-4910",
        "missing": missing,
        "why": "Side contents are not a free overlay pass; visible squares hand objects/creatures/projectiles to F0115 while that square is replayed.",
    })

    local = local_function((ROOT / "m11_game_view.c").read_text(errors="replace"), "m11_draw_dm1_side_contents")
    local_needles = [
        "for (depth = 2; depth >= 0; --depth)",
        "m11_dm1_center_line_clear_before_depth(cells, depth)",
        "m11_dm1_side_lane_clear_before_depth(cells, depth, sideIndex)",
        "m11_draw_item_sprite",
        "m11_draw_creature_sprite_ex",
        "m11_draw_projectile_sprite",
    ]
    missing = [n for n in local_needles if n not in local]
    passed = not missing
    ok = ok and passed
    checks.append({
        "id": "firestaff-side-content-far-to-near-and-occlusion-guards",
        "passed": passed,
        "source": "m11_game_view.c:m11_draw_dm1_side_contents",
        "missing": missing,
        "why": "Firestaff side content now paints D3 before D2 before D1 and retains center/side blocker guards, preventing far side items/creatures/projectiles from overdrawing nearer side content.",
    })

    payload = {"gate": "pass402_dm1_v1_side_content_far_to_near", "passed": ok, "source_root": str(args.source), "checks": checks}
    if args.json:
        print(json.dumps(payload, indent=2, sort_keys=True))
    else:
        for c in checks:
            print(("PASS" if c["passed"] else "FAIL"), c["id"], c["source"])
            print(" ", c["why"])
            for m in c["missing"]:
                print("  missing/order:", m)
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
