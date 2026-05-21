#!/usr/bin/env python3
"""Pass624: bind the next original transcript row preflight to one viewport crop row.

This is a no-runtime gate. It does not promote original-vs-Firestaff parity.
It locks the exact N2-local original asset set, ReDMCSB source route, and
single-row transcript schema required before the pass622 blocker can move.
"""
from __future__ import annotations

import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
CANON_DM1 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
PASS = "pass625_dm1_v1_original_transcript_row_preflight"
STATUS = "PASS625_DM1_V1_ORIGINAL_TRANSCRIPT_ROW_PREFLIGHT_LOCKED"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / f"{PASS}.md"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {"id": "i34e_entrance_enter_mouse_command", "file": "COMMAND.C", "lines": "341-352", "needles": ["G0445_as_Graphic561_PrimaryMouseInput_Entrance", "C200_COMMAND_ENTRANCE_ENTER_DUNGEON", "CM1_SCREEN_RELATIVE", "C407_ZONE_ENTRANCE_ENTER", "MASK0x0002_MOUSE_LEFT_BUTTON"], "claim": "the original PC/I34E entrance click must resolve to C200 before gameplay capture labels are accepted"},
    {"id": "i34e_entrance_enter_keyboard_command", "file": "COMMAND.C", "lines": "551-571", "needles": ["G2195_as_PrimaryKeyboardInput_Entrance", "C200_COMMAND_ENTRANCE_ENTER_DUNGEON", "0x001C", "C216_COMMAND_QUIT"], "claim": "the original PC/I34E keyboard entrance route has a source command id distinct from later movement commands"},
    {"id": "i34e_movement_mouse_commands", "file": "COMMAND.C", "lines": "396-405", "needles": ["C001_COMMAND_TURN_LEFT", "C003_COMMAND_MOVE_FORWARD", "C002_COMMAND_TURN_RIGHT", "C006_COMMAND_MOVE_LEFT", "C005_COMMAND_MOVE_BACKWARD", "C004_COMMAND_MOVE_RIGHT", "C080_COMMAND_CLICK_IN_DUNGEON_VIEW"], "claim": "post-entry movement labels must use the source movement command ids consumed by F0380"},
    {"id": "mouse_queue_write_with_coordinates", "file": "COMMAND.C", "lines": "1452-1661", "needles": ["void F0359_COMMAND_ProcessClick_CPSC", "L1109_i_Command = F0358_COMMAND_GetCommandFromMouseInput_CPSC", "G2153_i_QueuedCommandsCount++", "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1108_i_CommandQueueIndex].Command = L1109_i_Command", "G0432_as_CommandQueue[L1108_i_CommandQueueIndex].X = P0725_i_X", "G0432_as_CommandQueue[L1108_i_CommandQueueIndex].Y = P0726_i_Y"], "claim": "a mouse-driven transcript row must record the G0432 command/x/y write and queued-count delta"},
    {"id": "keyboard_queue_write", "file": "COMMAND.C", "lines": "1709-1813", "needles": ["void F0361_COMMAND_ProcessKeyPress", "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command", "G2153_i_QueuedCommandsCount++", "F0360_COMMAND_ProcessPendingClick();"], "claim": "a keyboard-driven transcript row must record the same queue boundary before F0380 dispatch"},
    {"id": "queue_pop_dispatch_turn_or_step", "file": "COMMAND.C", "lines": "2045-2156", "needles": ["void F0380_COMMAND_ProcessQueue_CPSC", "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;", "G2153_i_QueuedCommandsCount--;", "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);", "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);"], "claim": "the transcript row must bind the queue pop to the exact turn/step handler"},
    {"id": "game_loop_redraw_after_dispatch", "file": "GAMELOOP.C", "lines": "90-219", "needles": ["F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);", "G0321_B_StopWaitingForPlayerInput = C0_FALSE;", "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());", "F0380_COMMAND_ProcessQueue_CPSC();", "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);"], "claim": "a promotable crop must be sampled after the bounded command wait loop reaches the next F0128 draw"},
    {"id": "viewport_tuple_consumed_by_f0128", "file": "DUNVIEW.C", "lines": "8318-8610", "needles": ["void F0128_DUNGEONVIEW_Draw_CPSF", "P0183_i_Direction", "P0184_i_MapX", "P0185_i_MapY", "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);"], "claim": "the transcript row must include the direction/x/y tuple consumed by F0128"},
    {"id": "pc_viewport_present_boundary", "file": "DRAWVIEW.C", "lines": "709-858", "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "F0638_GetZone(C007_ZONE_VIEWPORT, L2414_ai_XYZ);", "M768_BOX_LEFT(L2413_ai_Box) = M704_ZONE_LEFT(L2414_ai_XYZ);", "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);"], "claim": "the original screenshot/crop must be taken at or after the viewport-present boundary"},
]

ASSET_LOCKS = [
    {"relative": "TITLE", "bytes": 12002, "sha256": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745"},
    {"relative": "GRAPHICS.DAT", "bytes": 363417, "sha256": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e"},
    {"relative": "DUNGEON.DAT", "bytes": 33357, "sha256": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85"},
    {"relative": "DungeonMasterPC34/DM.EXE", "bytes": 11471, "sha256": "4c79b43276f1eb3191d496ba71f8e4c03380d252193561bc6bba6017ef554db4"},
    {"relative": "DungeonMasterPC34/VGA", "bytes": 4503, "sha256": "4d9815e777e135bf69e3575fea533128b6073ae8c6b5282c24529c606f95af3b"},
    {"relative": "DungeonMasterPC34/SELECTOR", "bytes": 15474, "sha256": "1f32014376b90bd958d5c6bff7c67cb6378b47de4416d7206ea7e27bfc3c07c4"},
    {"relative": "Dungeon-Master_DOS_EN.zip", "bytes": 896553, "sha256": "aeb5a47f3b753206e474185f2c08b5e884dc8ddf4bd5cb82e2f28f9b7617f275"},
]

GATES = [
    ("pass608_same_viewport_blocker", ROOT / "parity-evidence/verification/pass608_dm1_v1_same_viewport_capture_blocker/manifest.json", "BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE"),
    ("pass622_viewport_wall_capture_gap", ROOT / "parity-evidence/verification/pass622_dm1_v1_viewport_wall_capture_closure_gap/manifest.json", "BLOCKED_PASS622_DM1_V1_VIEWPORT_WALL_CAPTURE_CLOSURE_GAP_LOCKED"),
    ("pass623_input_capture_bridge", ROOT / "parity-evidence/verification/pass623_dm1_v1_input_capture_readiness_bridge/manifest.json", "PASS623_DM1_V1_INPUT_CAPTURE_READINESS_BRIDGE_LOCKED"),
]

TARGET_ROW = {
    "routeLabel": "02_turn_right_west_1_3",
    "inputToken": "M12_MENU_INPUT_RIGHT",
    "sourceCommandId": 2,
    "sourceCommandName": "C002_COMMAND_TURN_RIGHT",
    "partyBefore": {"map": 0, "x": 1, "y": 3, "direction": 2},
    "partyAfter": {"map": 0, "x": 1, "y": 3, "direction": 3},
    "f0128Tuple": {"map": 0, "x": 1, "y": 3, "direction": 3},
    "firestaffViewportSha256": "1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81",
}

REQUIRED_TRANSCRIPT_FIELDS = [
    "runId", "routeLabel", "originalAssetSet.sha256.GRAPHICS.DAT", "originalAssetSet.sha256.DUNGEON.DAT", "originalFrame.path", "originalFrame.rawSha256", "originalFrame.cropSha256", "originalFrame.width", "originalFrame.height", "input.source", "input.token", "input.sourceCommandId", "commandQueue.sourceFunction", "commandQueue.command", "commandQueue.countBefore", "commandQueue.countAfter", "commandQueue.firstIndexBefore", "commandQueue.firstIndexAfter", "dispatch.sourceFunction", "dispatch.handler", "partyBefore.mapIndex", "partyBefore.mapX", "partyBefore.mapY", "partyBefore.direction", "partyAfter.mapIndex", "partyAfter.mapX", "partyAfter.mapY", "partyAfter.direction", "redraw.sourceFunction", "redraw.mapX", "redraw.mapY", "redraw.direction", "present.sourceFunction", "present.viewportPresented", "present.boundary", "firestaffFrame.mapIndex", "firestaffFrame.mapX", "firestaffFrame.mapY", "firestaffFrame.direction", "firestaffFrame.viewportSha256",
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


def sha256(path: Path) -> str | None:
    if not path.exists() or not path.is_file():
        return None
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


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


def audit_assets() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for lock in ASSET_LOCKS:
        path = CANON_DM1 / lock["relative"]
        actual_sha = sha256(path)
        actual_bytes = path.stat().st_size if path.exists() and path.is_file() else None
        rows.append({"relative": lock["relative"], "path": str(path), "realpath": str(path.resolve()) if path.exists() else None, "exists": path.exists(), "bytes": actual_bytes, "expectedBytes": lock["bytes"], "sha256": actual_sha, "expectedSha256": lock["sha256"], "ok": path.exists() and actual_bytes == lock["bytes"] and actual_sha == lock["sha256"]})
    return rows


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8")) if path.exists() else {}


def audit_gates() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for gate_id, path, expected in GATES:
        data = load_json(path)
        status = data.get("status")
        row: dict[str, Any] = {"id": gate_id, "path": str(path.relative_to(ROOT)), "expected": expected, "observed": status, "ok": bool(data) and status == expected}
        if gate_id == "pass608_same_viewport_blocker":
            runtime = data.get("runtimeTranscript", {})
            original = data.get("freshOriginalDiagnostic", {})
            row.update({"runtimeTranscriptProvided": runtime.get("provided") is True, "runtimeTranscriptOk": runtime.get("ok") is True, "duplicateCropGroups": original.get("cropDuplicateRouteIndices", {})})
        if gate_id == "pass622_viewport_wall_capture_gap":
            row["blocker"] = data.get("blocker")
            row["manifestOk"] = data.get("ok") is True
            row["ok"] = row["ok"] and data.get("ok") is True
        rows.append(row)
    return rows


def audit_target_row() -> dict[str, Any]:
    data = load_json(ROOT / "parity-evidence/verification/pass623_dm1_v1_input_capture_readiness_bridge/manifest.json")
    row = next((r for r in data.get("canonicalInputCaptureRows", []) if r.get("label") == TARGET_ROW["routeLabel"]), {})
    problems: list[str] = []
    if not row:
        problems.append("pass623 target row missing")
    else:
        observed = row.get("observed") or {}
        if row.get("commandIds") != [TARGET_ROW["sourceCommandId"]]:
            problems.append("target command id drifted")
        if row.get("inputTokens") != [TARGET_ROW["inputToken"]]:
            problems.append("target input token drifted")
        if row.get("postTuple") != TARGET_ROW["partyAfter"]:
            problems.append("target post tuple drifted")
        if observed.get("sha") != TARGET_ROW["firestaffViewportSha256"]:
            problems.append("target Firestaff viewport hash drifted")
    return {"target": TARGET_ROW, "pass623Row": row, "ok": not problems, "problems": problems}


def transcript_template() -> dict[str, Any]:
    return {
        "runId": "<original-runtime-run-id>",
        "routeLabel": TARGET_ROW["routeLabel"],
        "originalAssetSet": {"sha256": {item["relative"]: item["sha256"] for item in ASSET_LOCKS if item["relative"] in {"GRAPHICS.DAT", "DUNGEON.DAT", "TITLE"}}},
        "originalFrame": {"path": "<raw-320x200-frame.png>", "rawSha256": "<sha256>", "cropSha256": "<sha256>", "width": 320, "height": 200},
        "input": {"source": "original PC/I34E", "token": TARGET_ROW["inputToken"], "sourceCommandId": TARGET_ROW["sourceCommandId"], "sourceCommandName": TARGET_ROW["sourceCommandName"]},
        "commandQueue": {"sourceFunction": "F0359_COMMAND_ProcessClick_CPSC or F0361_COMMAND_ProcessKeyPress", "command": TARGET_ROW["sourceCommandId"], "countBefore": "<int>", "countAfter": "<int>", "firstIndexBefore": "<int>", "firstIndexAfter": "<int>"},
        "dispatch": {"sourceFunction": "F0380_COMMAND_ProcessQueue_CPSC", "handler": "F0365_COMMAND_ProcessTypes1To2_TurnParty"},
        "partyBefore": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 2},
        "partyAfter": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 3},
        "redraw": {"sourceFunction": "F0128_DUNGEONVIEW_Draw_CPSF", "mapX": 1, "mapY": 3, "direction": 3},
        "present": {"sourceFunction": "F0097_DUNGEONVIEW_DrawViewport", "viewportPresented": True, "boundary": "VIDRV_09_BlitViewPort"},
        "firestaffFrame": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 3, "viewportSha256": TARGET_ROW["firestaffViewportSha256"]},
    }


def flatten_keys(obj: Any, prefix: str = "") -> set[str]:
    if isinstance(obj, dict):
        keys: set[str] = set()
        for key, value in obj.items():
            child = f"{prefix}.{key}" if prefix else key
            keys.add(child)
            keys.update(flatten_keys(value, child))
        return keys
    return set()


def audit_transcript_template(template: dict[str, Any]) -> dict[str, Any]:
    flat = flatten_keys(template)
    missing = [field for field in REQUIRED_TRANSCRIPT_FIELDS if field not in flat]
    return {"requiredFields": REQUIRED_TRANSCRIPT_FIELDS, "template": template, "ok": not missing, "missing": missing}


def write_report(manifest: dict[str, Any]) -> None:
    lines = ["# Pass624 - DM1 V1 original transcript row preflight", "", f"Status: {manifest['status']}", "", "This gate narrows the pass622 blocker to one original runtime transcript row for `02_turn_right_west_1_3`. It does not run DOSBox and does not promote original-vs-Firestaff parity.", "", "## Source evidence"]
    for row in manifest["sourceAudit"]:
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['file']}:{row['lines']} {row['id']} - {row['claim']}")
    lines += ["", "## N2 original asset locks"]
    for row in manifest["originalAssetLocks"]:
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['relative']} bytes={row['bytes']} sha256={row['sha256']}")
    lines += ["", "## Target transcript row"]
    t = manifest["targetRowAudit"]["target"]
    lines.append(f"- label={t['routeLabel']} input={t['inputToken']} command={t['sourceCommandId']} {t['sourceCommandName']}")
    lines.append(f"- partyAfter={t['partyAfter']} firestaffViewportSha256={t['firestaffViewportSha256']}")
    lines.append(f"- pass623 row ok={manifest['targetRowAudit']['ok']}")
    lines += ["", "## Consumed gates"]
    for row in manifest["gateStatusChecks"]:
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['id']} observed={row['observed']}")
    lines += ["", "## Required original transcript fields"]
    lines.extend(f"- {field}" for field in manifest["transcriptTemplateAudit"]["requiredFields"])
    lines += ["", "## Decision", "", manifest["decision"], "", "## Non-claims"]
    lines.extend(f"- {item}" for item in manifest["nonClaims"])
    if manifest["problems"]:
        lines += ["", "## Problems"]
        lines.extend(f"- {item}" for item in manifest["problems"])
    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    source = audit_sources()
    assets = audit_assets()
    gates = audit_gates()
    target = audit_target_row()
    template_audit = audit_transcript_template(transcript_template())
    problems: list[str] = []
    problems.extend(f"source audit failed: {row['id']}" for row in source if not row["ok"])
    problems.extend(f"asset lock failed: {row['relative']}" for row in assets if not row["ok"])
    problems.extend(f"gate status drifted: {row['id']}" for row in gates if not row["ok"])
    problems.extend(target["problems"])
    problems.extend(f"transcript template missing field: {field}" for field in template_audit["missing"])
    pass608 = next((row for row in gates if row["id"] == "pass608_same_viewport_blocker"), {})
    if pass608.get("runtimeTranscriptProvided") or pass608.get("runtimeTranscriptOk"):
        problems.append("pass608 now has a runtime transcript; replace this preflight with a promotion verifier")
    status = STATUS if not problems else "FAIL_PASS625_DM1_V1_ORIGINAL_TRANSCRIPT_ROW_PREFLIGHT"
    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "ok": not problems,
        "sourceRoot": str(RED),
        "canonicalOriginalRoot": str(CANON_DM1),
        "sourceAudit": source,
        "originalAssetLocks": assets,
        "gateStatusChecks": gates,
        "targetRowAudit": target,
        "transcriptTemplateAudit": template_audit,
        "decision": "The next original capture attempt has a machine-checked, source-backed target row: one original PC/I34E transcript for 02_turn_right_west_1_3 must prove queue write/pop, F0380->F0365 dispatch, party tuple 0/1/3/3, F0128 redraw, and F0097/VIDRV viewport present before its crop can be paired with the locked Firestaff viewport hash.",
        "nonClaims": ["no original DOS runtime capture was run", "no original-vs-Firestaff pixel parity is promoted", "no renderer, movement, or input behavior is changed", "no non-N2 original asset path is used", "no push, tag, package, or release action"],
        "problems": problems,
    }
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({"status": status, "manifest": str(OUT_JSON.relative_to(ROOT)), "report": str(OUT_MD.relative_to(ROOT))}, indent=2, sort_keys=True))
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
