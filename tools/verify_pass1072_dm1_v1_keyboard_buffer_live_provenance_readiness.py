#!/usr/bin/env python3
"""Pass1072: DM1 V1 original keyboard-buffer live-provenance readiness.

This is an evidence automation gate, not a parity promotion gate. It
fingerprints the existing pass513 deterministic transcript and records whether
any row has debugger-observed original PC/I34E keyboard-buffer provenance.

The intended outcome today is a BLOCKED readiness status with ok=true:
source-filled transcript rows are useful automation, but they are not live
M527/M528/F0361/F0380 observations and must not close the original keyboard
buffer evidence row by themselves.
"""
from __future__ import annotations

import hashlib
import json
import os
import shutil
from datetime import datetime, timezone
from pathlib import Path
from typing import Any


ROOT = Path(__file__).resolve().parents[1]
PASS = "pass1072_dm1_v1_keyboard_buffer_live_provenance_readiness"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"

TRANSCRIPT = ROOT / "verification-screens/pass513-dm1-v1-promoted-transcript/promoted_transcript.json"
PASS513_SCOUT = ROOT / "verification-screens/pass1052-dm1-original-route-24h-turncycle/pass513_i34e_route_key_transcript_scaffold.json"
PASS513_REPORT = ROOT / "parity-evidence/pass513_dm1_v1_i34e_route_key_transcript_contract.md"
PASS513_VERIFIER = ROOT / "tools/verify_pass513_dm1_v1_i34e_route_key_transcript_contract.py"
DEBUGGER_HARNESS = ROOT / "tools/run_dosbox_debug_pty.py"

RED = Path(
    os.environ.get(
        "FIRESTAFF_REDMCSB_SOURCE",
        str(Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
    )
)

STATUS_BLOCKED = "BLOCKED_ORIGINAL_I34E_KEYBOARD_BUFFER_LIVE_DEBUGGER_OBSERVATION_MISSING"
STATUS_READY = "PASS1072_ORIGINAL_I34E_KEYBOARD_BUFFER_LIVE_PROVENANCE_READY"
STATUS_FAIL = "FAIL_PASS1072_DM1_V1_KEYBOARD_BUFFER_LIVE_PROVENANCE_READINESS"

REQUIRED_TOOLS = ["dosbox-debug", "Xvfb", "xdotool"]

SOURCE_LOCKS: list[dict[str, Any]] = [
    {
        "file": "IO2.C",
        "lines": "27-61",
        "id": "m528_f0540_keyboard_buffer_read",
        "needles": [
            "IODRV_00_GetKeyboardInput",
            "switch (L2944_ui_ - 0x1248)",
            "return L2944_ui_;",
        ],
    },
    {
        "file": "COMMAND.C",
        "lines": "636-685",
        "id": "i34e_keyboard_table_codes",
        "needles": [
            "G0459_as_Graphic561_SecondaryKeyboardInput_Movement",
            "{ C001_COMMAND_TURN_LEFT,     0x004B }",
            "{ C003_COMMAND_MOVE_FORWARD,  0x004C }",
            "{ C002_COMMAND_TURN_RIGHT,    0x004D }",
        ],
    },
    {
        "file": "COMMAND.C",
        "lines": "1734-1812",
        "id": "f0361_queue_write_count_delta",
        "needles": [
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex",
            "G2153_i_QueuedCommandsCount++;",
        ],
    },
    {
        "file": "COMMAND.C",
        "lines": "2075-2156",
        "id": "f0380_queue_pop_dispatch",
        "needles": [
            "G2153_i_QueuedCommandsCount--;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        ],
    },
    {
        "file": "DUNVIEW.C",
        "lines": "8318-8611",
        "id": "f0128_redraw_tuple",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
    },
    {
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "id": "f0097_viewport_present",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "G0296_puc_Bitmap_Viewport",
            "VIDRV_09_BlitViewPort",
        ],
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


def sha256(path: Path) -> str | None:
    if not path.is_file():
        return None
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def load_json(path: Path) -> dict[str, Any]:
    if not path.exists():
        return {}
    return json.loads(path.read_text(encoding="utf-8"))


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


def audit_tools() -> dict[str, Any]:
    tool_rows = [
        {"name": name, "path": shutil.which(name), "available": shutil.which(name) is not None}
        for name in REQUIRED_TOOLS
    ]
    harness_text = read_text(DEBUGGER_HARNESS) if DEBUGGER_HARNESS.exists() else ""
    harness_needles = [
        "dosbox-debug",
        "Xvfb",
        "xdotool",
        "missing tools:",
        "runtime_probe",
    ]
    missing_harness_needles = [needle for needle in harness_needles if needle not in harness_text]
    return {
        "requiredTools": tool_rows,
        "allRequiredToolsAvailable": all(row["available"] for row in tool_rows),
        "debuggerHarness": str(DEBUGGER_HARNESS.relative_to(ROOT)),
        "debuggerHarnessExists": DEBUGGER_HARNESS.exists(),
        "debuggerHarnessOk": DEBUGGER_HARNESS.exists() and not missing_harness_needles,
        "debuggerHarnessMissingNeedles": missing_harness_needles,
    }


def is_live_debugger_row(row: dict[str, Any]) -> bool:
    provenance_fields = [
        str(row.get("runtimeProvenance", "")),
        str(row.get("provenance", "")),
        str(row.get("inputSource", "")),
        str(row.get("promotionEvidence", "")),
    ]
    joined = " ".join(provenance_fields).lower()
    if row.get("debuggerObserved") is True or row.get("liveDebuggerObserved") is True:
        return True
    if "not a live" in joined or "source-locked deterministic" in joined:
        return False
    return (
        "live_i34e_debugger" in joined
        or "original pc/i34e debugger" in joined
        or "debugger-observed original pc/i34e" in joined
    )


def audit_transcript() -> dict[str, Any]:
    payload = load_json(TRANSCRIPT)
    rows = payload.get("rows", []) if isinstance(payload, dict) else []
    row_audits: list[dict[str, Any]] = []
    for index, row in enumerate(rows):
        if not isinstance(row, dict):
            row_audits.append({"index": index, "ok": False, "problem": "row is not an object"})
            continue
        capture_path = ROOT / str(row.get("capturePath", ""))
        raw_capture_path = ROOT / str(row.get("rawCapturePath", ""))
        row_audits.append({
            "index": index,
            "sampleIndex": row.get("sampleIndex"),
            "routeShotLabel": row.get("routeShotLabel"),
            "inputSource": row.get("inputSource"),
            "promotionEvidence": row.get("promotionEvidence"),
            "liveDebuggerObserved": is_live_debugger_row(row),
            "capturePath": str(capture_path.relative_to(ROOT)) if capture_path.exists() else str(row.get("capturePath", "")),
            "captureSha256": sha256(capture_path),
            "captureSha256Expected": row.get("captureSha256"),
            "rawCapturePath": str(raw_capture_path.relative_to(ROOT)) if raw_capture_path.exists() else str(row.get("rawCapturePath", "")),
            "rawCaptureSha256": sha256(raw_capture_path),
            "rawCaptureSha256Expected": row.get("rawCaptureSha256"),
            "captureOk": capture_path.exists() and sha256(capture_path) == row.get("captureSha256"),
            "rawCaptureOk": raw_capture_path.exists() and sha256(raw_capture_path) == row.get("rawCaptureSha256"),
        })
    non_claims = " ".join(str(item) for item in payload.get("nonClaims", []))
    promotion_note = str(payload.get("promotionNote", ""))
    explicit_non_live = "not a live" in (non_claims + " " + promotion_note).lower()
    return {
        "path": str(TRANSCRIPT.relative_to(ROOT)),
        "exists": TRANSCRIPT.exists(),
        "sha256": sha256(TRANSCRIPT),
        "schema": payload.get("schema"),
        "status": payload.get("status"),
        "rowCount": len(rows),
        "rows": row_audits,
        "allCapturesOk": bool(row_audits) and all(row.get("captureOk") and row.get("rawCaptureOk") for row in row_audits),
        "liveDebuggerRowCount": sum(1 for row in row_audits if row.get("liveDebuggerObserved")),
        "explicitNonLiveBoundary": explicit_non_live,
        "nonClaims": payload.get("nonClaims", []),
        "promotionNote": promotion_note,
    }


def audit_pass513_acceptance(transcript_audit: dict[str, Any]) -> dict[str, Any]:
    problems: list[str] = []
    if not PASS513_VERIFIER.exists():
        problems.append("pass513 verifier missing")
    if not PASS513_REPORT.exists():
        problems.append("pass513 report missing")
    if not PASS513_SCOUT.exists():
        problems.append("pass513 scaffold missing")
    if transcript_audit.get("status") != "PROMOTED_TRANSCRIPT_DETERMINISTIC_SOURCE_LOCKED":
        problems.append("deterministic transcript status drifted")
    if transcript_audit.get("liveDebuggerRowCount") == 0 and not transcript_audit.get("explicitNonLiveBoundary"):
        problems.append("deterministic transcript lacks explicit non-live boundary")
    if not transcript_audit.get("allCapturesOk"):
        problems.append("transcript capture hashes drifted")
    return {
        "pass513Verifier": str(PASS513_VERIFIER.relative_to(ROOT)),
        "pass513Report": str(PASS513_REPORT.relative_to(ROOT)),
        "pass513Scaffold": str(PASS513_SCOUT.relative_to(ROOT)),
        "deterministicTranscriptAcceptedAsReadiness": not problems,
        "problems": problems,
    }


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# Pass1072 - DM1 V1 keyboard-buffer live provenance readiness",
        "",
        f"Status: {manifest['status']}",
        "",
        "This gate fingerprints the pass513 deterministic transcript and keeps the original keyboard-buffer evidence row honest: source-filled rows are readiness evidence, not live M527/M528/F0361/F0380 debugger observations.",
        "",
        "## Transcript",
        "",
        f"- path: `{manifest['transcript']['path']}`",
        f"- sha256: `{manifest['transcript']['sha256']}`",
        f"- row count: `{manifest['transcript']['rowCount']}`",
        f"- live debugger rows: `{manifest['transcript']['liveDebuggerRowCount']}`",
        f"- capture hashes OK: `{manifest['transcript']['allCapturesOk']}`",
        f"- explicit non-live boundary: `{manifest['transcript']['explicitNonLiveBoundary']}`",
        "",
        "## Host Prerequisites",
        "",
    ]
    for row in manifest["toolAudit"]["requiredTools"]:
        state = "available" if row["available"] else "missing"
        lines.append(f"- {row['name']}: {state}")
    lines += [
        "",
        "## Decision",
        "",
        manifest["decision"],
        "",
        "## Non-claims",
        "",
    ]
    lines.extend(f"- {item}" for item in manifest["nonClaims"])
    if manifest["problems"]:
        lines += ["", "## Problems", ""]
        lines.extend(f"- {item}" for item in manifest["problems"])
    lines += ["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`"]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    source_audit = audit_sources()
    tool_audit = audit_tools()
    transcript_audit = audit_transcript()
    pass513 = audit_pass513_acceptance(transcript_audit)

    problems: list[str] = []
    problems.extend(f"source audit failed: {row['file']}:{row['lines']}" for row in source_audit if not row["ok"])
    if not tool_audit["debuggerHarnessOk"]:
        problems.append("debugger harness drifted or is missing required prerequisite checks")
    problems.extend(pass513["problems"])

    live_rows = int(transcript_audit.get("liveDebuggerRowCount") or 0)
    status = STATUS_FAIL if problems else (STATUS_READY if live_rows > 0 else STATUS_BLOCKED)
    ok = not problems
    decision = (
        "A live original PC/I34E keyboard-buffer transcript row is present; the row can be considered for promotion."
        if live_rows > 0 and ok
        else "The pass513 transcript is source-filled and reproducible, but it has zero debugger-observed original PC/I34E keyboard-buffer rows. The B1 keyboard-buffer evidence row remains PARTIAL until a live dosbox-debug run records M527/M528, F0361, F0380, F0128, and F0097 for the same route."
    )

    manifest = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "ok": ok,
        "sourceRoot": str(RED),
        "sourceAudit": source_audit,
        "toolAudit": tool_audit,
        "transcript": transcript_audit,
        "pass513Acceptance": pass513,
        "gapRowMovement": {
            "row": "B1 Original DOSBox/FIRES keyboard buffer transcript for I34E route keys",
            "from": "PARTIAL",
            "to": "PARTIAL",
            "reason": "deterministic source-filled transcript and capture hashes are now fingerprinted; live original debugger provenance is still missing",
        },
        "decision": decision,
        "nonClaims": [
            "no original DOSBox/FIRES runtime was launched by this verifier",
            "no source-filled transcript row is treated as live debugger observation",
            "no original-vs-Firestaff pixel parity is promoted",
            "no gameplay, renderer, or input behavior is changed",
            "no push, tag, package, or release action",
        ],
        "problems": problems,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    print(json.dumps({
        "status": status,
        "ok": ok,
        "manifest": str(MANIFEST.relative_to(ROOT)),
        "report": str(REPORT.relative_to(ROOT)),
        "liveDebuggerRows": live_rows,
        "missingTools": [row["name"] for row in tool_audit["requiredTools"] if not row["available"]],
        "problems": problems,
    }, indent=2, sort_keys=True))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
