#!/usr/bin/env python3
"""Pass497: preserve the next original-capture blocker after pass487/pass492."""
from __future__ import annotations

import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS487 = ROOT / "parity-evidence/verification/pass487_dm1_v1_original_click_capture_blocker/manifest.json"
VERIFY_DIR = ROOT / "parity-evidence/verification/pass497_dm1_v1_original_capture_next_blocker"
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence/pass497_dm1_v1_original_capture_next_blocker.md"
STATUS = "PASS497_ORIGINAL_CAPTURE_NEXT_BLOCKER_LOCKED"

SOURCE_LOCKS = [
    {"id": "entrance-click-enters-load-dungeon", "refs": ["COMMAND.C:63-72", "ENTRANCE.C:739-747", "ENTRANCE.C:850-883", "COMMAND.C:2428-2456", "ENTRANCE.C:939-944"], "meaning": "entrance primary mouse table maps the enter box to C200; the entrance loop processes the command queue; C200 sets G0298_B_NewGame=C001_MODE_LOAD_DUNGEON and the doors open", "needles": [("COMMAND.C", "C200_COMMAND_ENTRANCE_ENTER_DUNGEON"), ("ENTRANCE.C", "G0298_B_NewGame = C099_MODE_WAITING_ON_ENTRANCE"), ("COMMAND.C", "G0298_B_NewGame = C001_MODE_LOAD_DUNGEON"), ("ENTRANCE.C", "F0438_STARTEND_OpenEntranceDoors")]},
    {"id": "movement-click-primitives-are-real-pc34-zones", "refs": ["COMMAND.C:106-114"], "meaning": "PC34 movement mouse zones are fixed: turn left/right, forward/back/strafe, and dungeon-view click", "needles": [("COMMAND.C", "C001_COMMAND_TURN_LEFT"), ("COMMAND.C", "C003_COMMAND_MOVE_FORWARD"), ("COMMAND.C", "C005_COMMAND_MOVE_BACKWARD"), ("COMMAND.C", "C080_COMMAND_CLICK_IN_DUNGEON_VIEW")]},
    {"id": "queued-commands-must-hit-f0365-or-f0366-before-state-delta", "refs": ["COMMAND.C:2045-2156", "CLIKMENU.C:142-174", "CLIKMENU.C:180-347"], "meaning": "F0380 drains the command queue and dispatches turns to F0365 and moves to F0366; F0365 changes party direction and F0366 computes movement and calls F0267", "needles": [("COMMAND.C", "F0365_COMMAND_ProcessTypes1To2_TurnParty"), ("COMMAND.C", "F0366_COMMAND_ProcessTypes3To6_MoveParty"), ("CLIKMENU.C", "F0284_CHAMPION_SetPartyDirection"), ("CLIKMENU.C", "F0267_MOVE_GetMoveResult_CPSCE")]},
    {"id": "movement-state-delta-is-source-visible", "refs": ["DUNGEON.C:1371-1392", "MOVESENS.C:316-843"], "meaning": "relative movement updates map coordinates; F0267 writes G0306/G0307 on successful party movement/teleporter/pit transitions and can draw intermediate viewport frames while falling", "needles": [("DUNGEON.C", "G0233_ai_Graphic559_DirectionToStepEastCount"), ("MOVESENS.C", "G0306_i_PartyMapX = P0560_i_DestinationMapX"), ("MOVESENS.C", "G0307_i_PartyMapY = P0561_i_DestinationMapY"), ("MOVESENS.C", "F0128_DUNGEONVIEW_Draw_CPSF")]},
    {"id": "viewport-redraw-is-presentable-after-state-delta", "refs": ["DUNVIEW.C:2962-3000", "DUNVIEW.C:8318-8618", "DRAWVIEW.C:709-858"], "meaning": "viewport draw rebuilds floor/ceiling and wall layers into G0296, then F0097 requests/presents the viewport; repeated identical 48ed frames are capture/route-state failure, not overlay evidence", "needles": [("DUNVIEW.C", "F0098_DUNGEONVIEW_DrawFloorAndCeiling"), ("DUNVIEW.C", "F0128_DUNGEONVIEW_Draw_CPSF"), ("DRAWVIEW.C", "G0296_puc_Bitmap_Viewport"), ("DRAWVIEW.C", "VIDRV_09_BlitViewPort")]},
]


def source_has(file: str, needle: str) -> bool:
    path = RED / file
    return path.exists() and needle in path.read_text(encoding="latin-1", errors="replace")


def main() -> int:
    if not PASS487.exists():
        raise SystemExit(f"missing pass487 manifest: {PASS487}")
    p487 = json.loads(PASS487.read_text(encoding="utf-8"))
    blockers = p487.get("blockerFindings", {})
    rows = []
    problems: list[str] = []
    for lock in SOURCE_LOCKS:
        missing = [f"{file}:{needle}" for file, needle in lock["needles"] if not source_has(file, needle)]
        rows.append({**lock, "ok": not missing, "missing": missing})
        problems += [f"source lock failed {m}" for m in missing]

    required = {
        "pass487_ok": p487.get("ok") is True,
        "entrance_first_frame": blockers.get("firstFrameStillEntranceMenu") is True,
        "post_entry_static_hash": blockers.get("postEntryGameplayHashRepeated") is True,
        "post_entry_region_repeat": blockers.get("postEntryRegionStatsRepeated") is True,
        "route_label_drift_present": len(blockers.get("filenameLabelDrift", [])) == 5,
    }
    problems += [name for name, ok in required.items() if not ok]
    next_blocker = (
        "Capture/replay must prove a source-visible post-command state delta before overlay promotion: "
        "after entering gameplay, each movement/turn click needs a capture tied to the exact command "
        "completion/redraw boundary with either a new raw hash/region fingerprint or debugger/runtime proof "
        "that F0365/F0366 and the F0128/F0097 present path ran for that shot. The current pass487/pass492 "
        "evidence reaches gameplay but repeats the 48ed static no-state-delta frame five times and has "
        "filename/route-label drift, so those frames remain blocker evidence only."
    )
    payload = {
        "status": STATUS,
        "ok": not problems,
        "pass487Manifest": str(PASS487.relative_to(ROOT)),
        "pass487Status": p487.get("status"),
        "pass487Decision": p487.get("decision"),
        "observed": {
            "classes": p487.get("classes"),
            "duplicateSha256Counts": p487.get("duplicateSha256Counts"),
            "firstFrameSha256": blockers.get("firstFrameSha256"),
            "postEntryGameplaySha256": blockers.get("postEntryGameplaySha256"),
            "filenameLabelDriftRows": len(blockers.get("filenameLabelDrift", [])),
        },
        "sourceLocks": rows,
        "required": required,
        "nextBlocker": next_blocker,
        "nonClaims": ["no original-vs-Firestaff pixel parity", "no promotion of pass487/pass492 frames as overlay references", "no claim that movement processors are broken; the blocker is capture/route-state proof"],
        "problems": problems,
    }
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    lines = [
        "# Pass497 — DM1 V1 original capture next blocker",
        "",
        f"Status: `{STATUS}`",
        "",
        "This pass does not unblock original overlay parity. It locks the next blocker after the pass487/pass492 click evidence so static frames cannot be promoted by accident.",
        "",
        "## Observed pass487/pass492 state",
        f"- classes: `{', '.join(p487.get('classes', []))}`",
        f"- duplicate hashes: `{p487.get('duplicateSha256Counts')}`",
        f"- first frame SHA: `{blockers.get('firstFrameSha256')}`",
        f"- post-entry static SHA: `{blockers.get('postEntryGameplaySha256')}`",
        f"- filename/route-label drift rows: `{len(blockers.get('filenameLabelDrift', []))}`",
        "- interpretation: source-locked click primitives reach gameplay, but the post-entry captures are still static/no-state-delta blocker frames.",
        "",
        "## ReDMCSB source audit",
    ]
    for row in rows:
        lines.append(f"- `{row['id']}` ok={row['ok']} refs={', '.join(row['refs'])}: {row['meaning']}")
    lines += [
        "",
        "## Precise next blocker",
        next_blocker,
        "",
        "## Gate",
        "- `python3 tools/verify_pass497_dm1_v1_original_capture_next_blocker.py`",
        "",
        "## Non-claims",
        "No original-vs-Firestaff pixel parity, no overlay-reference promotion, and no claim that F0365/F0366 are broken.",
    ]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    if problems:
        print("FAIL")
        for problem in problems:
            print(problem)
        return 1
    print(f"PASS {STATUS}")
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
