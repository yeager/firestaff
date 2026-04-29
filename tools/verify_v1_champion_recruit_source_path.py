#!/usr/bin/env python3
"""Verify the DM1/V1 champion recruitment source path lock.

This gate documents the route blocker narrowed by pass162: clicking the
Resurrect/Reincarnate panel is not the source entry point for party creation.
The original first requires a C127 wall champion portrait sensor to call
F0280_CHAMPION_AddCandidateChampionToParty; only then does C160/C161 finalize
that candidate through F0282.
"""
from __future__ import annotations

import json
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
SRC = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS162 = REPO / "parity-evidence/verification/pass162_original_party_route_unblock/manifest.json"
OUT = REPO / "parity-evidence/verification/pass163_champion_recruit_source_path"

CHECKS = [
    {
        "id": "SRC_RECRUIT_001",
        "file": "MOVESENS.C",
        "needles": ["case C127_SENSOR_WALL_CHAMPION_PORTRAIT", "F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData)"],
        "claim": "The source candidate-add transition is triggered by wall sensor C127, not by the panel button.",
    },
    {
        "id": "SRC_RECRUIT_002",
        "file": "REVIVE.C",
        "needles": ["G0299_ui_CandidateChampionOrdinal = L0799_ui_PreviousPartyChampionCount + 1", "if (++G0305_ui_PartyChampionCount == 1)"],
        "claim": "F0280 marks a candidate and increments party count before the resurrect/reincarnate decision panel is finalized.",
    },
    {
        "id": "SRC_RECRUIT_003",
        "file": "COMMAND.C",
        "needles": ["case M568_PANEL_RESURRECT_REINCARNATE", "F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel(L1157_ui_Command)"],
        "claim": "Panel clicks dispatch to F0282 only when the panel content is already Resurrect/Reincarnate.",
    },
    {
        "id": "SRC_RECRUIT_004",
        "file": "REVIVE.C",
        "needles": ["void F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel", "L0826_ps_Champion = &M516_CHAMPIONS[L0822_ui_ChampionIndex = G0305_ui_PartyChampionCount - 1]", "G0299_ui_CandidateChampionOrdinal = M000_INDEX_TO_ORDINAL(CM1_CHAMPION_NONE)"],
        "claim": "F0282 assumes F0280 already inserted a candidate at party_count-1, then clears candidate state after Resurrect/Reincarnate/Cancel.",
    },
    {
        "id": "SRC_RECRUIT_005",
        "file": "COMMAND.C",
        "needles": ["{ C160_COMMAND_CLICK_IN_PANEL_RESURRECT", "{ C161_COMMAND_CLICK_IN_PANEL_REINCARNATE", "104, 158,  86, 142", "163, 217,  86, 142"],
        "claim": "The old y=165 route was outside the original C160/C161 boxes; pass162 corrected this but still hit static-no-party, proving the missing step is earlier C127/F0280 candidate creation.",
    },
]


def line_hits(path: Path, needles: list[str]) -> list[dict[str, object]]:
    text = path.read_text(errors="replace").splitlines()
    hits = []
    for needle in needles:
        for i, line in enumerate(text, 1):
            if needle in line:
                hits.append({"needle": needle, "line": i, "text": line.strip()})
                break
        else:
            raise AssertionError(f"missing {needle!r} in {path}")
    return hits


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    checks = []
    for c in CHECKS:
        path = SRC / c["file"]
        hits = line_hits(path, c["needles"])
        checks.append({**c, "source": str(path), "hits": hits, "status": "PASS"})

    pass162_summary = None
    if PASS162.exists():
        data = json.loads(PASS162.read_text())
        pass162_summary = {
            "completed": data.get("completed"),
            "buckets": data.get("buckets"),
            "all_static_blocked": data.get("buckets", {}).get("blocked/static-no-party") == data.get("completed"),
            "scenarios": [
                {"name": r.get("name"), "classification": r.get("classification"), "reason": r.get("reason")}
                for r in data.get("results", [])
            ],
        }
        if not pass162_summary["all_static_blocked"]:
            raise AssertionError("pass162 no longer shows all scenarios static-blocked; update pass163 interpretation")

    result = {"schema": "pass163_champion_recruit_source_path.v1", "checks": checks, "pass162_summary": pass162_summary}
    (OUT / "source_path.json").write_text(json.dumps(result, indent=2) + "\n")
    lines = [
        "# Pass 163 — champion recruit source path lock",
        "",
        "This pass narrows Lane A after pass162: corrected Resurrect/Reincarnate box clicks still collapse to `48ed3743ab6a` because source party creation starts earlier, at the wall champion portrait sensor.",
        "",
        "## Source path",
        "",
        "1. `MOVESENS.C` handles `C127_SENSOR_WALL_CHAMPION_PORTRAIT` and calls `F0280_CHAMPION_AddCandidateChampionToParty(L0758_ui_SensorData)`.",
        "2. `REVIVE.C:F0280` sets `G0299_ui_CandidateChampionOrdinal` and increments `G0305_ui_PartyChampionCount`.",
        "3. Only after that does `COMMAND.C` route `M568_PANEL_RESURRECT_REINCARNATE` clicks to `REVIVE.C:F0282`.",
        "4. `F0282` assumes candidate state already exists at `G0305_ui_PartyChampionCount - 1` and finalizes/cancels it.",
        "",
        "## Checks",
        "",
    ]
    for c in checks:
        locs = ", ".join(f"{h['line']}" for h in c["hits"])
        lines.append(f"- {c['id']} PASS — `{c['file']}` lines {locs}: {c['claim']}")
    lines += [
        "",
        "## Pass162 linkage",
        "",
        f"- pass162 completed: {pass162_summary.get('completed') if pass162_summary else 'missing'}",
        f"- pass162 buckets: {pass162_summary.get('buckets') if pass162_summary else 'missing'}",
        "- Interpretation: next route work must find a reproducible original input path that visibly triggers the C127/F0280 candidate transition before clicking C160/C161. More Resurrect/Reincarnate panel coordinate permutations are now low value unless preceded by proof of candidate state.",
    ]
    (OUT / "README.md").write_text("\n".join(lines) + "\n")
    print(f"PASS wrote {OUT}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
