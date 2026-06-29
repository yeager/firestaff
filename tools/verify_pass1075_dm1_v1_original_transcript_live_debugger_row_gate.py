#!/usr/bin/env python3
"""Pass1075: validate a live-debugger I34E transcript row before promotion.

This is a narrow guard for the remaining DM1 V1 original-capture blocker.  The
pass623/pass625/pass626 gates already lock the C002 turn-right target row and
the ReDMCSB queue -> redraw -> present route, while pass1072 keeps the current
source-filled transcript from being mistaken for live keyboard-buffer evidence.

This verifier adds the missing reusable acceptance rule for the future row: a
candidate row must be live debugger-observed, must name the source functions
seen at runtime, and must match the pass625/pass626 C002 target tuple before
it can be treated as a real I34E runtime transcript row.  With no candidate row
provided it exits PASS with a BLOCKED status, so CI can keep the rule locked
without needing private original frame bytes.
"""
from __future__ import annotations

import json
import os
import re
import tempfile
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
RED = Path(
    os.environ.get(
        "FIRESTAFF_REDMCSB_SOURCE",
        str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
    )
)
PASS = "pass1075_dm1_v1_original_transcript_live_debugger_row_gate"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
OUT_JSON = OUT_DIR / "manifest.json"
OUT_MD = ROOT / "parity-evidence" / f"{PASS}.md"
PASS625_JSON = ROOT / "parity-evidence/verification/pass625_dm1_v1_original_transcript_row_preflight/manifest.json"
PASS626_JSON = ROOT / "parity-evidence/verification/pass626_dm1_v1_original_transcript_turn_redraw_route/manifest.json"

STATUS_BLOCKED = "BLOCKED_PASS1075_DM1_V1_ORIGINAL_I34E_LIVE_DEBUGGER_ROW_MISSING"
STATUS_READY = "PASS1075_DM1_V1_ORIGINAL_I34E_LIVE_DEBUGGER_ROW_ACCEPTED"
STATUS_FAIL = "FAIL_PASS1075_DM1_V1_ORIGINAL_TRANSCRIPT_LIVE_DEBUGGER_ROW"

EXPECTED_TARGET: dict[str, Any] = {
    "routeLabel": "02_turn_right_west_1_3",
    "inputToken": "M12_MENU_INPUT_RIGHT",
    "sourceCommandId": 2,
    "sourceCommandName": "C002_COMMAND_TURN_RIGHT",
    "partyBefore": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 2},
    "partyAfter": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 3},
    "firestaffViewportSha256": "1e71ed8799806ff0594943c52a0a99a12c3f6f441888a750f7f6be0f7c2c6d81",
}

CANONICAL_ASSET_SHA256 = {
    "GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
}

REQUIRED_DEBUGGER_FUNCTIONS = [
    "M528_GetCharacterInKeyboardBuffer",
    "F0361_COMMAND_ProcessKeyPress",
    "F0380_COMMAND_ProcessQueue_CPSC",
    "F0365_COMMAND_ProcessTypes1To2_TurnParty",
    "F0284_CHAMPION_SetPartyDirection",
    "F0128_DUNGEONVIEW_Draw_CPSF",
    "F0097_DUNGEONVIEW_DrawViewport",
]

SOURCE_LOCKS = [
    {
        "id": "m528_keyboard_buffer_read",
        "file": "IO2.C",
        "lines": "27-61",
        "needles": ["IODRV_00_GetKeyboardInput", "switch (L2944_ui_ - 0x1248)", "return L2944_ui_;"],
    },
    {
        "id": "f0361_keyboard_queue_write",
        "file": "COMMAND.C",
        "lines": "1709-1813",
        "needles": [
            "void F0361_COMMAND_ProcessKeyPress",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex",
            "G2153_i_QueuedCommandsCount++;",
        ],
    },
    {
        "id": "f0380_queue_pop_dispatch",
        "file": "COMMAND.C",
        "lines": "2045-2156",
        "needles": [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "G2153_i_QueuedCommandsCount--;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        ],
    },
    {
        "id": "f0365_turn_right_direction_commit",
        "file": "CLIKMENU.C",
        "lines": "142-173",
        "needles": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "P0734_i_Command == C002_COMMAND_TURN_RIGHT",
            "F0284_CHAMPION_SetPartyDirection",
        ],
    },
    {
        "id": "f0128_tuple_redraw",
        "file": "DUNVIEW.C",
        "lines": "8318-8610",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
    },
    {
        "id": "f0097_pc_i34e_viewport_present",
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "needles": ["void F0097_DUNGEONVIEW_DrawViewport", "G0296_puc_Bitmap_Viewport", "VIDRV_09_BlitViewPort"],
    },
]


def read_text(path: Path, encoding: str = "utf-8") -> str:
    return path.read_text(encoding=encoding, errors="replace")


def compact(text: str) -> str:
    return " ".join(text.split())


def source_window(path: Path, spec: str) -> str:
    lines = read_text(path, "latin-1").splitlines()
    out: list[str] = []
    for part in spec.split(","):
        first_s, last_s = part.split("-", 1) if "-" in part else (part, part)
        first, last = int(first_s), int(last_s)
        out.extend(lines[first - 1:last])
    return "\n".join(out)


def load_json(path: Path) -> dict[str, Any]:
    if not path.exists():
        return {}
    return json.loads(read_text(path))


def get_nested(obj: dict[str, Any], dotted: str) -> Any:
    cur: Any = obj
    for part in dotted.split("."):
        if not isinstance(cur, dict) or part not in cur:
            return None
        cur = cur[part]
    return cur


def set_nested(obj: dict[str, Any], dotted: str, value: Any) -> None:
    cur = obj
    parts = dotted.split(".")
    for part in parts[:-1]:
        cur = cur.setdefault(part, {})
    cur[parts[-1]] = value


def normalize_party_tuple(value: dict[str, Any] | None) -> dict[str, Any]:
    value = value or {}
    return {
        "mapIndex": value.get("mapIndex", value.get("map")),
        "mapX": value.get("mapX", value.get("x")),
        "mapY": value.get("mapY", value.get("y")),
        "direction": value.get("direction"),
    }


def audit_sources() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for lock in SOURCE_LOCKS:
        path = RED / lock["file"]
        if not path.exists():
            rows.append({**lock, "path": str(path), "ok": False, "missing": [f"missing source file: {path}"]})
            continue
        body = compact(source_window(path, lock["lines"]))
        missing = [needle for needle in lock["needles"] if compact(needle) not in body]
        rows.append({
            "id": lock["id"],
            "file": lock["file"],
            "lines": lock["lines"],
            "path": str(path),
            "ok": not missing,
            "missing": missing,
        })
    return rows


def audit_upstream_gates() -> list[dict[str, Any]]:
    checks = [
        (PASS625_JSON, "PASS625_DM1_V1_ORIGINAL_TRANSCRIPT_ROW_PREFLIGHT_LOCKED"),
        (PASS626_JSON, "PASS626_DM1_V1_ORIGINAL_TRANSCRIPT_TURN_REDRAW_ROUTE_LOCKED"),
    ]
    rows = []
    for path, expected in checks:
        data = load_json(path)
        status = data.get("status")
        rows.append({
            "path": str(path.relative_to(ROOT)),
            "expected": expected,
            "observed": status,
            "ok": status == expected,
        })
    return rows


def provenance_text(row: dict[str, Any]) -> str:
    values = [
        row.get("runtimeProvenance"),
        row.get("provenance"),
        row.get("promotionEvidence"),
        get_nested(row, "input.source"),
        get_nested(row, "debugger.provenance"),
    ]
    return " ".join(str(value) for value in values if value is not None).lower()


def observed_functions(row: dict[str, Any]) -> set[str]:
    found: set[str] = set()
    for key in ("debuggerObservedFunctions", "observedFunctions", "runtimeFunctions"):
        value = row.get(key)
        if isinstance(value, list):
            found.update(str(item) for item in value)
    value = get_nested(row, "debugger.observedFunctions")
    if isinstance(value, list):
        found.update(str(item) for item in value)
    return found


def is_live_debugger_row(row: dict[str, Any]) -> bool:
    text = provenance_text(row)
    if "not a live" in text or "source-filled" in text or "source-locked deterministic" in text:
        return False
    if row.get("liveDebuggerObserved") is True or row.get("debuggerObserved") is True:
        return True
    return (
        "live_i34e_debugger" in text
        or "debugger-observed original pc/i34e" in text
        or "original pc/i34e debugger" in text
    )


def require_equal(problems: list[str], label: str, observed: Any, expected: Any) -> None:
    if observed != expected:
        problems.append(f"{label} expected {expected!r} observed {observed!r}")


def validate_candidate_row(row: dict[str, Any]) -> dict[str, Any]:
    problems: list[str] = []
    require_equal(problems, "routeLabel", row.get("routeLabel", row.get("label")), EXPECTED_TARGET["routeLabel"])
    require_equal(problems, "input.token", get_nested(row, "input.token"), EXPECTED_TARGET["inputToken"])
    require_equal(problems, "input.sourceCommandId", get_nested(row, "input.sourceCommandId"), EXPECTED_TARGET["sourceCommandId"])
    require_equal(problems, "input.sourceCommandName", get_nested(row, "input.sourceCommandName"), EXPECTED_TARGET["sourceCommandName"])
    require_equal(problems, "commandQueue.command", get_nested(row, "commandQueue.command"), EXPECTED_TARGET["sourceCommandId"])
    require_equal(problems, "dispatch.sourceFunction", get_nested(row, "dispatch.sourceFunction"), "F0380_COMMAND_ProcessQueue_CPSC")
    require_equal(problems, "dispatch.handler", get_nested(row, "dispatch.handler"), "F0365_COMMAND_ProcessTypes1To2_TurnParty")
    require_equal(problems, "redraw.sourceFunction", get_nested(row, "redraw.sourceFunction"), "F0128_DUNGEONVIEW_Draw_CPSF")
    require_equal(problems, "present.sourceFunction", get_nested(row, "present.sourceFunction"), "F0097_DUNGEONVIEW_DrawViewport")
    require_equal(problems, "present.viewportPresented", get_nested(row, "present.viewportPresented"), True)
    require_equal(problems, "present.boundary", get_nested(row, "present.boundary"), "VIDRV_09_BlitViewPort")
    require_equal(problems, "partyBefore", normalize_party_tuple(get_nested(row, "partyBefore")), EXPECTED_TARGET["partyBefore"])
    require_equal(problems, "partyAfter", normalize_party_tuple(get_nested(row, "partyAfter")), EXPECTED_TARGET["partyAfter"])
    require_equal(problems, "redraw tuple", normalize_party_tuple({
        "mapIndex": get_nested(row, "partyAfter.mapIndex"),
        "mapX": get_nested(row, "redraw.mapX"),
        "mapY": get_nested(row, "redraw.mapY"),
        "direction": get_nested(row, "redraw.direction"),
    }), EXPECTED_TARGET["partyAfter"])
    require_equal(problems, "firestaff viewport sha", get_nested(row, "firestaffFrame.viewportSha256"), EXPECTED_TARGET["firestaffViewportSha256"])
    asset_sha = get_nested(row, "originalAssetSet.sha256") or {}
    for name, expected_sha in CANONICAL_ASSET_SHA256.items():
        observed_sha = asset_sha.get(name) if isinstance(asset_sha, dict) else None
        require_equal(problems, f"originalAssetSet.sha256.{name}", observed_sha, expected_sha)
    if get_nested(row, "originalFrame.width") != 320 or get_nested(row, "originalFrame.height") != 200:
        problems.append("originalFrame must be a 320x200 PC/I34E frame")
    for dotted in ("originalFrame.rawSha256", "originalFrame.cropSha256"):
        value = get_nested(row, dotted)
        if not isinstance(value, str) or not re.fullmatch(r"[0-9a-f]{64}", value):
            problems.append(f"{dotted} must be a lowercase 64-hex sha256")
    if not is_live_debugger_row(row):
        problems.append("row is not marked as a live original PC/I34E debugger observation")
    functions = observed_functions(row)
    missing_functions = [name for name in REQUIRED_DEBUGGER_FUNCTIONS if name not in functions]
    if missing_functions:
        problems.append("missing debugger-observed functions: " + ", ".join(missing_functions))
    return {
        "ok": not problems,
        "routeLabel": row.get("routeLabel", row.get("label")),
        "liveDebuggerObserved": is_live_debugger_row(row),
        "observedFunctions": sorted(functions),
        "problems": problems,
    }


def candidate_rows_from_payload(payload: Any) -> list[dict[str, Any]]:
    if isinstance(payload, dict):
        for key in ("rows", "transcriptRows", "frameBindings"):
            rows = payload.get(key)
            if isinstance(rows, list):
                return [row for row in rows if isinstance(row, dict)]
        return [payload]
    if isinstance(payload, list):
        return [row for row in payload if isinstance(row, dict)]
    return []


def build_valid_fixture() -> dict[str, Any]:
    row: dict[str, Any] = {
        "runId": "live-i34e-debugger-fixture",
        "routeLabel": EXPECTED_TARGET["routeLabel"],
        "runtimeProvenance": "live_i34e_debugger",
        "liveDebuggerObserved": True,
        "debuggerObservedFunctions": REQUIRED_DEBUGGER_FUNCTIONS[:],
        "originalAssetSet": {"sha256": CANONICAL_ASSET_SHA256.copy()},
        "originalFrame": {
            "path": "operator-local/02_turn_right_west_1_3.png",
            "rawSha256": "a" * 64,
            "cropSha256": "b" * 64,
            "width": 320,
            "height": 200,
        },
        "input": {
            "source": "live_i34e_debugger",
            "token": EXPECTED_TARGET["inputToken"],
            "sourceCommandId": EXPECTED_TARGET["sourceCommandId"],
            "sourceCommandName": EXPECTED_TARGET["sourceCommandName"],
        },
        "commandQueue": {
            "sourceFunction": "F0361_COMMAND_ProcessKeyPress",
            "command": EXPECTED_TARGET["sourceCommandId"],
            "countBefore": 0,
            "countAfter": 1,
            "firstIndexBefore": 0,
            "firstIndexAfter": 0,
        },
        "dispatch": {"sourceFunction": "F0380_COMMAND_ProcessQueue_CPSC", "handler": "F0365_COMMAND_ProcessTypes1To2_TurnParty"},
        "partyBefore": EXPECTED_TARGET["partyBefore"].copy(),
        "partyAfter": EXPECTED_TARGET["partyAfter"].copy(),
        "redraw": {"sourceFunction": "F0128_DUNGEONVIEW_Draw_CPSF", "mapX": 1, "mapY": 3, "direction": 3},
        "present": {"sourceFunction": "F0097_DUNGEONVIEW_DrawViewport", "viewportPresented": True, "boundary": "VIDRV_09_BlitViewPort"},
        "firestaffFrame": {"mapIndex": 0, "mapX": 1, "mapY": 3, "direction": 3, "viewportSha256": EXPECTED_TARGET["firestaffViewportSha256"]},
    }
    return row


def run_selftest() -> dict[str, Any]:
    cases: list[dict[str, Any]] = []
    valid = build_valid_fixture()
    cases.append({"name": "valid_live_debugger_row", "expectOk": True, "row": valid})

    deterministic = json.loads(json.dumps(valid))
    deterministic["liveDebuggerObserved"] = False
    deterministic["runtimeProvenance"] = "source-filled deterministic, not a live debugger observation"
    cases.append({"name": "reject_source_filled_row", "expectOk": False, "row": deterministic})

    missing_function = json.loads(json.dumps(valid))
    missing_function["debuggerObservedFunctions"].remove("F0361_COMMAND_ProcessKeyPress")
    cases.append({"name": "reject_missing_f0361_observation", "expectOk": False, "row": missing_function})

    wrong_tuple = json.loads(json.dumps(valid))
    set_nested(wrong_tuple, "partyAfter.direction", 2)
    cases.append({"name": "reject_wrong_post_turn_tuple", "expectOk": False, "row": wrong_tuple})

    wrong_hash = json.loads(json.dumps(valid))
    set_nested(wrong_hash, "firestaffFrame.viewportSha256", "0" * 64)
    cases.append({"name": "reject_unknown_firestaff_target_hash", "expectOk": False, "row": wrong_hash})

    rows: list[dict[str, Any]] = []
    for case in cases:
        audit = validate_candidate_row(case["row"])
        rows.append({
            "name": case["name"],
            "expectOk": case["expectOk"],
            "observedOk": audit["ok"],
            "ok": audit["ok"] is case["expectOk"],
            "problems": audit["problems"],
        })

    with tempfile.TemporaryDirectory(prefix="pass1075-row-") as tmp:
        path = Path(tmp) / "candidate.json"
        path.write_text(json.dumps({"rows": [valid]}, indent=2) + "\n", encoding="utf-8")
        loaded = candidate_rows_from_payload(load_json(path))
        file_audit = validate_candidate_row(loaded[0]) if loaded else {"ok": False, "problems": ["no row loaded"]}
        rows.append({
            "name": "loads_rows_payload",
            "expectOk": True,
            "observedOk": file_audit["ok"],
            "ok": file_audit["ok"] is True,
            "problems": file_audit["problems"],
        })

    return {"ok": all(row["ok"] for row in rows), "cases": rows}


def load_candidate_from_env() -> dict[str, Any]:
    raw = os.environ.get("FIRESTAFF_DM1_V1_I34E_LIVE_TRANSCRIPT_ROW", "").strip()
    if not raw:
        return {"provided": False, "path": None, "rows": [], "audits": []}
    path = Path(raw)
    payload = load_json(path)
    rows = candidate_rows_from_payload(payload)
    audits = [validate_candidate_row(row) for row in rows]
    return {"provided": True, "path": str(path), "rowCount": len(rows), "rows": rows, "audits": audits}


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# Pass1075 - DM1 V1 live debugger transcript row gate",
        "",
        f"Status: {manifest['status']}",
        "",
        "This gate validates the future original PC/I34E transcript row for the pass625/pass626 C002 turn-right target. It does not run DOSBox and does not promote pixel parity without a supplied live-debugger row.",
        "",
        "## Source audit",
    ]
    for row in manifest["sourceAudit"]:
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['file']}:{row['lines']} {row['id']}")
    lines += ["", "## Upstream gates"]
    for row in manifest["upstreamGates"]:
        lines.append(f"- {'PASS' if row['ok'] else 'FAIL'} {row['path']} observed={row['observed']}")
    lines += [
        "",
        "## Candidate",
        f"- provided: {manifest['candidate']['provided']}",
        f"- row count: {manifest['candidate'].get('rowCount', 0)}",
        "",
        "## Decision",
        "",
        manifest["decision"],
        "",
        "## Non-claims",
    ]
    lines.extend(f"- {item}" for item in manifest["nonClaims"])
    if manifest["problems"]:
        lines += ["", "## Problems"]
        lines.extend(f"- {item}" for item in manifest["problems"])
    OUT_MD.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    source_audit = audit_sources()
    upstream = audit_upstream_gates()
    selftest = run_selftest()
    candidate = load_candidate_from_env()

    problems: list[str] = []
    problems.extend(f"source audit failed: {row['id']}" for row in source_audit if not row["ok"])
    problems.extend(f"upstream gate drifted: {row['path']}" for row in upstream if not row["ok"])
    if not selftest["ok"]:
        problems.append("self-test cases failed")
    if candidate["provided"]:
        if not candidate.get("audits"):
            problems.append("candidate file provided but no candidate rows were found")
        problems.extend(
            f"candidate row {index} invalid: " + "; ".join(audit["problems"])
            for index, audit in enumerate(candidate.get("audits", []))
            if not audit["ok"]
        )

    candidate_ready = candidate["provided"] and bool(candidate.get("audits")) and all(audit["ok"] for audit in candidate["audits"])
    status = STATUS_FAIL if problems else (STATUS_READY if candidate_ready else STATUS_BLOCKED)
    decision = (
        "A supplied candidate row satisfies the live-debugger C002 turn/right transcript rule and can be considered by the original-capture promotion lane."
        if candidate_ready and not problems
        else "No live original PC/I34E debugger row is supplied. The rule is locked, but the original transcript blocker remains open until an operator-local row proves M528/F0361/F0380/F0365/F0284/F0128/F0097 at runtime for 02_turn_right_west_1_3."
    )

    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "ok": not problems,
        "sourceRoot": str(RED),
        "expectedTarget": EXPECTED_TARGET,
        "requiredDebuggerFunctions": REQUIRED_DEBUGGER_FUNCTIONS,
        "sourceAudit": source_audit,
        "upstreamGates": upstream,
        "selfTest": selftest,
        "candidate": {
            "provided": candidate["provided"],
            "path": candidate.get("path"),
            "rowCount": candidate.get("rowCount", 0),
            "audits": candidate.get("audits", []),
        },
        "decision": decision,
        "nonClaims": [
            "no DOSBox, dosbox-debug, FIRES, or original runtime was launched by this verifier",
            "no operator-local original frame bytes are written to the repository",
            "no source-filled deterministic row is accepted as live debugger evidence",
            "no original-vs-Firestaff pixel parity is promoted",
            "no gameplay, renderer, input, or asset-loading behavior is changed",
        ],
        "problems": problems,
    }
    OUT_JSON.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({
        "status": status,
        "ok": not problems,
        "manifest": str(OUT_JSON.relative_to(ROOT)),
        "report": str(OUT_MD.relative_to(ROOT)),
        "candidateProvided": candidate["provided"],
        "problems": problems,
    }, indent=2, sort_keys=True))
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
