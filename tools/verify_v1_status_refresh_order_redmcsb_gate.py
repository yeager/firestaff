#!/usr/bin/env python3
"""Verify V1 champion status refresh draw order against ReDMCSB.

This is a source-order gate only.  It checks the ReDMCSB changed-status path
(F0260 -> F0293 -> F0292) and the Firestaff V1 top-row renderer order; it does
not claim original-runtime pixel parity.
"""
from __future__ import annotations

import json
import os
from pathlib import Path
import re
import sys
from typing import Iterable

ROOT = Path(__file__).resolve().parents[1]
DEFAULT_REDMCSB_ROOT = Path(
    "/home/trv2/.openclaw/data/firestaff-redmcsb-source/"
    "ReDMCSB_WIP20210206/Toolchains/Common/Source"
)
REDMCSB_ROOT = Path(os.environ.get("REDMCSB_SOURCE_ROOT", DEFAULT_REDMCSB_ROOT))
FIRESTAFF_SRC = ROOT / "m11_game_view.c"
OUT_JSON = ROOT / "parity-evidence/verification/v1_status_refresh_order_redmcsb_gate.json"

SOURCE_RANGES = [
    {"file": "TIMELINE.C", "function": "F0260_TIMELINE_RefreshAllChampionStatusBoxes", "start": 1817, "end": 1830},
    {"file": "CHAMDRAW.C", "function": "F0292_CHAMPION_DrawState", "start": 771, "end": 815},
    {"file": "CHAMDRAW.C", "function": "F0292_CHAMPION_DrawState", "start": 843, "end": 940},
    {"file": "CHAMDRAW.C", "function": "F0292_CHAMPION_DrawState", "start": 1080, "end": 1110},
    {"file": "CHAMDRAW.C", "function": "F0293_CHAMPION_DrawAllChampionStates", "start": 1117, "end": 1139},
    {"file": "m11_game_view.c", "function": "m11_draw_party_panel", "start": 17818, "end": 18045},
]


def read_text(path: Path) -> str:
    if not path.is_file():
        raise AssertionError(f"missing source file: {path}")
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_function(text: str, name: str, next_name: str | None = None) -> tuple[int, int, str]:
    prefix = r"(?m)^(?:static\s+)?(?:STATICFUNCTION\s+)?(?:void|int16_t|BOOLEAN|int)\s+"
    pat = re.compile(prefix + re.escape(name) + r"\s*\(")
    match = pat.search(text)
    if not match:
        raise AssertionError(f"missing function declaration for {name}")
    if next_name:
        next_pat = re.compile(prefix + re.escape(next_name) + r"\s*\(")
        next_match = next_pat.search(text, match.end())
        if not next_match:
            raise AssertionError(f"missing following function declaration for {next_name}")
        end = next_match.start()
    else:
        end = min(len(text), match.start() + 24000)
    return match.start(), end, text[match.start():end]


def require_in_order(body: str, markers: Iterable[tuple[str, str]], label: str) -> list[dict[str, int | str]]:
    found: list[dict[str, int | str]] = []
    cursor = -1
    for name, needle in markers:
        pos = body.find(needle, cursor + 1)
        if pos < 0:
            raise AssertionError(f"{label}: missing {name} marker {needle!r}")
        found.append({"name": name, "offset": pos, "needle": needle})
        cursor = pos
    return found


def require_excerpt(path: Path, rel: str, start: int, end: int, needles: list[str]) -> None:
    lines = read_text(path).splitlines()
    if len(lines) < end:
        raise AssertionError(f"{path} has only {len(lines)} lines, need {end}")
    excerpt = "\n".join(lines[start - 1:end])
    missing = [needle for needle in needles if needle not in excerpt]
    if missing:
        raise AssertionError(f"{rel}:{start}-{end} missing {missing}")
    print(f"sourceRange={path}:{start}-{end} status=ok")


def verify_redmcsb() -> list[dict[str, int | str]]:
    timeline_path = REDMCSB_ROOT / "TIMELINE.C"
    timeline = read_text(timeline_path)
    f260_start, _f260_end, f260 = find_function(timeline, "F0260_TIMELINE_RefreshAllChampionStatusBoxes", "F0261_TIMELINE_Process_CPSEF")
    f260_markers = require_in_order(
        f260,
        [
            ("loop over party champions", "L0679_ui_ChampionIndex < G0305_ui_PartyChampionCount"),
            ("set changed status-box bit", "MASK0x1000_STATUS_BOX"),
            ("draw all champion states", "F0293_CHAMPION_DrawAllChampionStates()"),
        ],
        "ReDMCSB F0260 changed-status refresh",
    )

    cham_path = REDMCSB_ROOT / "CHAMDRAW.C"
    cham = read_text(cham_path)
    f292_start, _f292_end, f292 = find_function(cham, "F0292_CHAMPION_DrawState", "F0293_CHAMPION_DrawAllChampionStates")
    f292_markers = require_in_order(
        f292,
        [
            ("status-box dirty gate", "MASK0x1000_STATUS_BOX"),
            ("alive background fill", "M524_FillScreenBox(L0871_ai_Box, C12_COLOR_DARKEST_GRAY)"),
            ("fire shield queued", "C038_GRAPHIC_BORDER_PARTY_FIRESHIELD"),
            ("spell shield queued", "C039_GRAPHIC_BORDER_PARTY_SPELLSHIELD"),
            ("party/champion shield queued", "C037_GRAPHIC_BORDER_PARTY_SHIELD"),
            ("shield border blit", "M520_F0021_MAIN_BlitToScreen"),
            ("name/title bit scheduled", "MASK0x0080_NAME_TITLE | MASK0x0100_STATISTICS"),
            ("name/title draw gate", "MASK0x0080_NAME_TITLE"),
            ("statistics draw gate", "MASK0x0100_STATISTICS"),
            ("wounds/hand draw gate", "MASK0x2000_WOUNDS"),
            ("action-hand draw gate", "MASK0x8000_ACTION_HAND"),
            ("dirty bits cleared", "M009_CLEAR(L0865_ps_Champion->Attributes"),
        ],
        "ReDMCSB F0292 status-box overdraw order",
    )
    f293_start, _f293_end, f293 = find_function(cham, "F0293_CHAMPION_DrawAllChampionStates")
    f293_markers = require_in_order(
        f293,
        [
            ("loop over party champions", "L0873_ui_ChampionIndex < G0305_ui_PartyChampionCount"),
            ("draw one champion state", "F0292_CHAMPION_DrawState(L0873_ui_ChampionIndex)"),
        ],
        "ReDMCSB F0293 all-state loop",
    )

    require_excerpt(timeline_path, "TIMELINE.C", 1817, 1830, ["MASK0x1000_STATUS_BOX", "F0293_CHAMPION_DrawAllChampionStates"])
    require_excerpt(cham_path, "CHAMDRAW.C", 771, 815, ["MASK0x1000_STATUS_BOX", "C038_GRAPHIC_BORDER_PARTY_FIRESHIELD", "C037_GRAPHIC_BORDER_PARTY_SHIELD", "M520_F0021_MAIN_BlitToScreen"])
    require_excerpt(cham_path, "CHAMDRAW.C", 843, 940, ["MASK0x0080_NAME_TITLE", "MASK0x0100_STATISTICS", "MASK0x2000_WOUNDS"])
    require_excerpt(cham_path, "CHAMDRAW.C", 1080, 1110, ["MASK0x8000_ACTION_HAND", "M009_CLEAR"])
    require_excerpt(cham_path, "CHAMDRAW.C", 1117, 1139, ["F0293_CHAMPION_DrawAllChampionStates", "F0292_CHAMPION_DrawState"])

    def line_entries(src_text: str, src_start: int, markers: list[dict[str, int | str]], file: str, function: str) -> list[dict[str, int | str]]:
        return [
            {"file": file, "function": function, "marker": str(m["name"]), "line": line_no(src_text, src_start + int(m["offset"])), "needle": str(m["needle"])}
            for m in markers
        ]

    return (
        line_entries(timeline, f260_start, f260_markers, "TIMELINE.C", "F0260_TIMELINE_RefreshAllChampionStatusBoxes")
        + line_entries(cham, f292_start, f292_markers, "CHAMDRAW.C", "F0292_CHAMPION_DrawState")
        + line_entries(cham, f293_start, f293_markers, "CHAMDRAW.C", "F0293_CHAMPION_DrawAllChampionStates")
    )


def verify_firestaff() -> list[dict[str, int | str]]:
    text = read_text(FIRESTAFF_SRC)
    start, _end, body = find_function(text, "m11_draw_party_panel")
    markers = require_in_order(
        body,
        [
            ("status-box background comment", "V1 source status-box background"),
            ("alive status fill", "M11_GameView_GetV1StatusBoxFillColor"),
            ("source-order shield border block", "before top-row\n             * text, bars, and hand slots"),
            ("shield border blit", "M11_GameView_GetV1StatusShieldBorderZone"),
            ("name clear/text block", "V1 champion name/title status text"),
            ("bar graph block", "Pass 43: champion HP/stamina/mana bar graphs"),
            ("status hand slots block", "V1 status-box hand slots"),
            ("poison/damage later overlays", "GRAPHICS.DAT-backed POISONED label"),
        ],
        "Firestaff m11_draw_party_panel V1 status refresh order",
    )
    require_excerpt(FIRESTAFF_SRC, "m11_game_view.c", 17818, 18045, [
        "V1 source status-box background",
        "before top-row",
        "V1 champion name/title status text",
        "Pass 43: champion HP/stamina/mana bar graphs",
        "V1 status-box hand slots",
    ])
    return [
        {"file": "m11_game_view.c", "function": "m11_draw_party_panel", "marker": str(m["name"]), "line": line_no(text, start + int(m["offset"])), "needle": str(m["needle"])}
        for m in markers
    ]


def main() -> int:
    print("probe=v1_status_refresh_order_redmcsb_gate")
    print(f"sourceRoot={REDMCSB_ROOT}")
    redmcsb_markers = verify_redmcsb()
    firestaff_markers = verify_firestaff()
    result = {
        "schema": "v1_status_refresh_order_redmcsb_gate.v1",
        "pass": True,
        "scope": "DM1/V1 champion top-row HUD/status changed-status refresh source-order gate only.",
        "sourceRanges": SOURCE_RANGES,
        "redmcsbMarkers": redmcsb_markers,
        "firestaffMarkers": firestaff_markers,
        "contract": [
            "F0260 sets MASK0x1000_STATUS_BOX for each party champion, then calls F0293.",
            "F0293 walks party champions and calls F0292 for each.",
            "F0292 alive status-box refresh fills background, draws queued shield borders, schedules name/stat/wounds/action refresh bits, then draws name, stats, hand slots, and action hand before clearing dirty bits.",
            "Firestaff V1 m11_draw_party_panel keeps shield border overdraw before name, bar, and hand slot drawing."
        ],
        "nonClaims": [
            "No original-runtime screenshot/pixel parity claim.",
            "No inventory chest/action-hand refresh claim.",
            "No DM2 file-open, V2 scaffolding, or V2 asset claim."
        ],
    }
    OUT_JSON.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"evidence={OUT_JSON.relative_to(ROOT)} status=ok")
    print("v1StatusRefreshOrderRedmcsbGateOk=1")
    for marker in redmcsb_markers + firestaff_markers:
        print(f"- {marker['file']} {marker['function']} {marker['marker']}: line {marker['line']}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError, json.JSONDecodeError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        raise SystemExit(1)
