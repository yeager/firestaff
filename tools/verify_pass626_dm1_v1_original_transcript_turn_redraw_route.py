#!/usr/bin/env python3
"""Pass626: source-lock the target original transcript turn -> redraw route.

This is a no-runtime gate. It narrows the pass625 target row to the exact
ReDMCSB command queue, turn-state mutation, next redraw, and viewport-present
boundary that an original PC/I34E transcript must prove before pixel parity can
be promoted.
"""
from __future__ import annotations

import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
PASS = "pass626_dm1_v1_original_transcript_turn_redraw_route"
STATUS = "PASS626_DM1_V1_ORIGINAL_TRANSCRIPT_TURN_REDRAW_ROUTE_LOCKED"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / f"{PASS}.md"
PASS625_JSON = ROOT / "parity-evidence/verification/pass625_dm1_v1_original_transcript_row_preflight/manifest.json"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {
        "id": "f0380_dequeues_c002_and_dispatches_turn_handler",
        "file": "COMMAND.C",
        "lines": "2045-2156",
        "needles": [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G2153_i_QueuedCommandsCount--;",
            "if ((L1160_i_Command == C002_COMMAND_TURN_RIGHT) || (L1160_i_Command == C001_COMMAND_TURN_LEFT))",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
        ],
        "claim": "the target row's C002 queue pop must dispatch through F0380 into F0365, not directly to a viewport crop",
    },
    {
        "id": "f0365_turn_right_sets_wait_stop_and_new_direction",
        "file": "CLIKMENU.C",
        "lines": "142-173",
        "needles": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
            "P0734_i_Command == C002_COMMAND_TURN_RIGHT",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, C1_TRUE, C0_FALSE);",
            "F0284_CHAMPION_SetPartyDirection(M021_NORMALIZE(G0308_i_PartyDirection + ((P0734_i_Command == C002_COMMAND_TURN_RIGHT) ? 1 : 3)));",
            "F0276_SENSOR_ProcessThingAdditionOrRemoval(G0306_i_PartyMapX, G0307_i_PartyMapY, C0xFFFF_THING_PARTY, C1_TRUE, C1_TRUE);",
        ],
        "claim": "C002 turn-right advances direction by +1, stops the wait loop, and brackets the direction change with party sensor removal/addition",
    },
    {
        "id": "f0284_commits_party_direction_before_redraw",
        "file": "CHAMPION.C",
        "lines": "117-130",
        "needles": [
            "if (P0600_i_Direction == G0308_i_PartyDirection)",
            "L0834_i_Delta = P0600_i_Direction - G0308_i_PartyDirection",
            "L0835_ps_Champion->Cell = M021_NORMALIZE(L0835_ps_Champion->Cell + L0834_i_Delta);",
            "L0835_ps_Champion->Direction = M021_NORMALIZE(L0835_ps_Champion->Direction + L0834_i_Delta);",
            "G0308_i_PartyDirection = P0600_i_Direction;",
            "F0296_CHAMPION_DrawChangedObjectIcons();",
        ],
        "claim": "F0365's computed direction is committed to G0308 before the next game-loop F0128 draw consumes it",
    },
    {
        "id": "main_loop_dispatches_then_next_iteration_redraws_current_tuple",
        "file": "GAMELOOP.C",
        "lines": "90-219",
        "needles": [
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
            "G0321_B_StopWaitingForPlayerInput = C0_FALSE;",
            "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "if (!G0321_B_StopWaitingForPlayerInput)",
            "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
        ],
        "claim": "after F0365 sets the wait-stop flag, the bounded wait loop exits and the next iteration draws the updated G0308/G0306/G0307 tuple",
    },
    {
        "id": "f0128_consumes_target_tuple_and_calls_present",
        "file": "DUNVIEW.C",
        "lines": "8318-8610",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "claim": "the post-turn tuple is the tuple consumed by F0128 before viewport present",
    },
    {
        "id": "i34e_f0097_blits_composed_viewport_to_screen",
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "F0638_GetZone(C007_ZONE_VIEWPORT, L2414_ai_XYZ);",
            "M768_BOX_LEFT(L2413_ai_Box) = M704_ZONE_LEFT(L2414_ai_XYZ);",
            "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",
        ],
        "claim": "the crop boundary remains the PC/I34E VIDRV_09_BlitViewPort present call, not a pre-present buffer snapshot",
    },
]

EXPECTED_TARGET = {
    "routeLabel": "02_turn_right_west_1_3",
    "inputToken": "M12_MENU_INPUT_RIGHT",
    "sourceCommandId": 2,
    "sourceCommandName": "C002_COMMAND_TURN_RIGHT",
    "partyBefore": {"map": 0, "x": 1, "y": 3, "direction": 2},
    "partyAfter": {"map": 0, "x": 1, "y": 3, "direction": 3},
    "dispatchHandler": "F0365_COMMAND_ProcessTypes1To2_TurnParty",
    "redrawFunction": "F0128_DUNGEONVIEW_Draw_CPSF",
    "presentBoundary": "F0097_DUNGEONVIEW_DrawViewport -> VIDRV_09_BlitViewPort",
}

REQUIRED_CAPTURE_EVENTS = [
    "queued C002 with queue index/count evidence",
    "F0380 dequeued C002 from the same queue slot",
    "F0365 received C002 and called F0284 with normalized direction 3",
    "post-dispatch party tuple is map=0 x=1 y=3 direction=3",
    "next F0128 consumed direction=3 x=1 y=3",
    "F0097 reached the PC/I34E VIDRV_09_BlitViewPort boundary before crop hash capture",
]


def read_text(path: Path) -> str:
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def compact(text: str) -> str:
    return " ".join(text.split())


def source_window(path: Path, spec: str) -> str:
    lines = read_text(path).splitlines()
    out: list[str] = []
    for part in spec.split(","):
        first_s, last_s = part.split("-", 1) if "-" in part else (part, part)
        first, last = int(first_s), int(last_s)
        out.extend(lines[first - 1:last])
    return "\n".join(out)


def audit_sources() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for lock in SOURCE_LOCKS:
        path = RED / lock["file"]
        if not path.exists():
            rows.append({**lock, "ok": False, "missing": [f"missing source file: {path}"]})
            continue
        body = compact(source_window(path, lock["lines"]))
        missing = [needle for needle in lock["needles"] if compact(needle) not in body]
        rows.append({"id": lock["id"], "file": lock["file"], "lines": lock["lines"], "claim": lock["claim"], "ok": not missing, "missing": missing})
    return rows


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8")) if path.exists() else {}


def audit_pass625_target() -> dict[str, Any]:
    data = load_json(PASS625_JSON)
    target = data.get("targetRowAudit", {}).get("target", {})
    problems: list[str] = []
    if data.get("status") != "PASS625_DM1_V1_ORIGINAL_TRANSCRIPT_ROW_PREFLIGHT_LOCKED":
        problems.append("pass625 status is not locked")
    for key in ["routeLabel", "inputToken", "sourceCommandId", "sourceCommandName", "partyBefore", "partyAfter"]:
        if target.get(key) != EXPECTED_TARGET[key]:
            problems.append(f"pass625 target {key} drifted: {target.get(key)!r}")
    before = target.get("partyBefore", {})
    after = target.get("partyAfter", {})
    expected_direction = (before.get("direction", -1) + 1) % 4
    if after.get("direction") != expected_direction:
        problems.append("C002 turn-right direction arithmetic drifted")
    if after.get("x") != before.get("x") or after.get("y") != before.get("y") or after.get("map") != before.get("map"):
        problems.append("turn-right target row moved position instead of direction only")
    return {"path": str(PASS625_JSON.relative_to(ROOT)), "status": data.get("status"), "target": target, "expected": EXPECTED_TARGET, "ok": not problems, "problems": problems}


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# Pass626 - DM1 V1 original transcript turn/redraw route",
        "",
        f"Status: {manifest['status']}",
        "",
        "This gate source-locks the pass625 target row from queue pop through C002 turn-right state mutation, next redraw, and PC/I34E viewport present. It does not run DOSBox and does not promote original-vs-Firestaff pixel parity.",
        "",
        "## ReDMCSB source evidence",
    ]
    for row in manifest["sourceAudit"]:
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['file']}:{row['lines']} {row['id']} - {row['claim']}")
    t = manifest["targetRowAudit"]["target"]
    lines += [
        "",
        "## Locked target row",
        f"- label={t.get('routeLabel')} input={t.get('inputToken')} command={t.get('sourceCommandId')} {t.get('sourceCommandName')}",
        f"- partyBefore={t.get('partyBefore')} partyAfter={t.get('partyAfter')}",
        f"- pass625 status={manifest['targetRowAudit']['status']} ok={manifest['targetRowAudit']['ok']}",
        "",
        "## Required original runtime transcript events",
    ]
    lines.extend(f"- {item}" for item in manifest["requiredCaptureEvents"])
    lines += ["", "## Decision", "", manifest["decision"], "", "## Non-claims"]
    lines.extend(f"- {item}" for item in manifest["nonClaims"])
    if manifest["problems"]:
        lines += ["", "## Problems"]
        lines.extend(f"- {item}" for item in manifest["problems"])
    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    source = audit_sources()
    target = audit_pass625_target()
    problems: list[str] = []
    problems.extend(f"source audit failed: {row['id']}" for row in source if not row["ok"])
    problems.extend(target["problems"])
    status = STATUS if not problems else "FAIL_PASS626_DM1_V1_ORIGINAL_TRANSCRIPT_TURN_REDRAW_ROUTE"
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "ok": not problems,
        "sourceRoot": str(RED),
        "sourceAudit": source,
        "targetRowAudit": target,
        "requiredCaptureEvents": REQUIRED_CAPTURE_EVENTS,
        "decision": "The pass625 target row is now narrowed to a source-backed C002 turn-right route: original capture still must provide runtime proof for queue write/pop, F0380->F0365, F0284 direction commit to 3 at map 0 x 1 y 3, the following F0128 tuple, and the F0097/VIDRV present boundary before any original-vs-Firestaff pixel claim is valid.",
        "nonClaims": [
            "no original DOS runtime capture was run",
            "no original-vs-Firestaff pixel parity is promoted",
            "no renderer, movement, or input behavior is changed",
            "no non-N2 original asset path is used",
            "no push, tag, package, or release action",
        ],
        "problems": problems,
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": status, "manifest": str(OUT_JSON.relative_to(ROOT)), "report": str(OUT_MD.relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
