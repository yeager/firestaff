#!/usr/bin/env python3
from __future__ import annotations

import json
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass558_dm1_v1_pass514_io_blocker_classifier"
OUT = ROOT / "parity-evidence" / "verification" / PASS
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"

PASS514_MANIFEST = ROOT / "parity-evidence/verification/pass514_dm1_v1_i34e_runtime_transcript_capture_path/manifest.json"

EXTERNAL_EVIDENCE = {
    "pass550": [
        ROOT / "parity-evidence/verification/pass558_external_evidence/pass550_pass514_manifest.json",
        Path("/home/trv2/work/firestaff-worktrees/pass550-dm1v1-pass514-capture-handoff-followup-20260515-codex/parity-evidence/verification/pass514_dm1_v1_i34e_runtime_transcript_capture_path/manifest.json"),
    ],
    "pass556": [
        ROOT / "parity-evidence/verification/pass558_external_evidence/pass556_manifest.json",
        Path("/home/trv2/work/firestaff-worktrees/pass556-dm1v1-pass555-io2-runtime-pointer-binding-20260515-codex/parity-evidence/verification/pass556_dm1_v1_pass555_io2_runtime_pointer_binding/manifest.json"),
    ],
    "pass557": [
        ROOT / "parity-evidence/verification/pass558_external_evidence/pass557_manifest.json",
        Path("/home/trv2/work/firestaff-worktrees/pass557-dm1v1-pass556-host-emulator-keyboard-delivery-20260515-codex/parity-evidence/verification/pass557_dm1_v1_pass556_host_emulator_keyboard_delivery_probe/manifest.json"),
    ],
}

SOURCE_SPECS = [
    {
        "id": "game_loop_presence_read_before_f0380",
        "file": "GAMELOOP.C",
        "needles": [
            "while (M527_IsCharacterInKeyboardBuffer())",
            "F0361_COMMAND_ProcessKeyPress(M528_GetCharacterInKeyboardBuffer());",
            "F0380_COMMAND_ProcessQueue_CPSC();",
        ],
    },
    {
        "id": "defs_i34e_keyboard_macros_to_io2",
        "file": "DEFS.H",
        "needles": [
            "#define M527_IsCharacterInKeyboardBuffer()                               F0539_INPUT_Cconis()",
            "#define M528_GetCharacterInKeyboardBuffer()                              F0540_INPUT_Crawcin()",
            "int16_t (*IODRV_00_GetKeyboardInput)();",
            "BOOLEAN (*IODRV_01_IsKeyboardInputPresent)();",
            "#define C254_DM_IO_INTERRUPT    254",
        ],
    },
    {
        "id": "io2_presence_and_read_driver_slots",
        "file": "IO2.C",
        "needles": [
            "int16_t F0540_INPUT_Crawcin(",
            "L2944_ui_ = (*(G2162_IODriver->IODRV_00_GetKeyboardInput))();",
            "BOOLEAN F0539_INPUT_Cconis(",
            "return (*(G2162_IODriver->IODRV_01_IsKeyboardInputPresent))();",
        ],
    },
    {
        "id": "ibmio_keyboard_irq_buffer_and_presence",
        "file": "IBMIO.C",
        "needles": [
            "asm     in      al, 60h        /* 8042 keyboard controller data register */",
            "G8083_[G8077_] = P3278_i_;",
            "while (!G8082_);",
            "if (G8082_) {",
            "setvect(C254_DM_IO_INTERRUPT, (void interrupt (*)())&G8101_apc_IOInterruptVector);",
        ],
    },
    {
        "id": "f0361_enqueue_target",
        "file": "COMMAND.C",
        "needles": [
            "void F0361_COMMAND_ProcessKeyPress",
            "if ((L1112_ps_KeyboardInput = G0443_ps_PrimaryKeyboardInput) == NULL)",
            "G0432_as_CommandQueue[G0434_i_CommandQueueLastIndex = L1110_i_CommandQueueIndex].Command = L1111_i_Command;",
            "G2153_i_QueuedCommandsCount++;",
        ],
    },
    {
        "id": "f0380_empty_or_pop_dispatch",
        "file": "COMMAND.C",
        "needles": [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "if (G2153_i_QueuedCommandsCount == 0)",
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G2153_i_QueuedCommandsCount--;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
    },
]


def run(cmd: list[str]) -> str:
    return subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False).stdout.strip()


def read_json(path: Path) -> dict[str, Any] | None:
    if not path.exists():
        return None
    return json.loads(path.read_text(encoding="utf-8"))


def find_line(path: Path, needle: str) -> int | None:
    if not path.exists():
        return None
    compact = " ".join(needle.split())
    for line_no, line in enumerate(path.read_text(encoding="latin-1", errors="replace").splitlines(), 1):
        if compact in " ".join(line.split()):
            return line_no
    return None


def source_audit() -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for spec in SOURCE_SPECS:
        path = RED / spec["file"]
        hits = {needle: find_line(path, needle) for needle in spec["needles"]}
        present = [line for line in hits.values() if line is not None]
        rows.append(
            {
                "id": spec["id"],
                "file": spec["file"],
                "path": str(path),
                "ok": path.exists() and len(present) == len(hits),
                "lineRange": [min(present), max(present)] if present else None,
                "lineHits": hits,
                "missing": [needle for needle, line in hits.items() if line is None],
            }
        )
    return rows


def external_audit() -> dict[str, Any]:
    rows: dict[str, Any] = {}
    for name, paths in EXTERNAL_EVIDENCE.items():
        path = next((candidate for candidate in paths if candidate.exists()), paths[0])
        payload = read_json(path)
        row: dict[str, Any] = {"path": str(path), "present": payload is not None}
        if payload:
            row["status"] = payload.get("status")
            row["proofPredicates"] = payload.get("proofPredicates") or payload.get("predicateSummary")
            row["runtimeBinding"] = payload.get("runtimeBinding")
            row["decision"] = payload.get("decision") or payload.get("blocker")
        rows[name] = row
    return rows


def classify(source: list[dict[str, Any]], pass514: dict[str, Any], external: dict[str, Any]) -> tuple[str, str, dict[str, bool]]:
    pass550 = external.get("pass550", {})
    effective_pass514 = pass550 if pass550.get("present") else pass514
    p514 = effective_pass514.get("proofPredicates") or {}
    p556 = external.get("pass556", {}).get("proofPredicates") or {}
    p557 = external.get("pass557", {}).get("proofPredicates") or {}

    predicates = {
        "sourceAuditOk": all(row["ok"] for row in source),
        "pass514ManifestPresent": bool(pass514),
        "pass514InputDeliveredButEmptyF0380": effective_pass514.get("status") == "BLOCKED_PASS514_KEYBOARD_INPUT_DELIVERED_BUT_NO_F0361_ENQUEUE_BEFORE_EMPTY_F0380",
        "pass514RouteInputAfterArmingSucceeded": bool(p514.get("routeInputAfterArmingSucceeded")),
        "pass514F0380Reached": bool(p514.get("f0380ReachedInN2DebuggerPath")),
        "pass514QueueCountWriteMissing": not bool(p514.get("queueCountWriteObserved")),
        "pass556ManifestPresent": external.get("pass556", {}).get("present", False),
        "pass556IoDriverSlotsResolved": bool(p556.get("ioDriverSlotsResolved")),
        "pass556NoIo2ReadOrPresenceAfterRoute": not bool(p556.get("io2PresenceHitAfterRouteInput")) and not bool(p556.get("io2ReadHitAfterRouteInput")),
        "pass556F0380ReachedAfterRouteInput": bool(p556.get("f0380ReachedAfterRouteInput")),
        "pass557ManifestPresent": external.get("pass557", {}).get("present", False),
        "pass557HostRouteSucceeded": bool(p557.get("hostRouteSucceeded")),
        "pass557RulesOutHostRouteGeneration": bool(p557.get("pass556RouteInputAfterArmingSucceeded")) and bool(p557.get("hostRouteSucceeded")),
    }

    if not predicates["sourceAuditOk"]:
        return "FAIL_PASS558_REDMCSB_SOURCE_AUDIT_FAILED", "ReDMCSB source audit failed", predicates
    if not predicates["pass514ManifestPresent"]:
        return "BLOCKED_PASS558_PASS514_MANIFEST_MISSING", "pass514 manifest missing in this checkout", predicates
    if (
        predicates["pass514InputDeliveredButEmptyF0380"]
        and predicates["pass556IoDriverSlotsResolved"]
        and predicates["pass556NoIo2ReadOrPresenceAfterRoute"]
        and predicates["pass557RulesOutHostRouteGeneration"]
    ):
        decision = (
            "Classified current DM1 V1 original capture unblocker as DOSBox window/event-queue to guest "
            "keyboard interrupt/IO2 dispatch boundary: ReDMCSB requires GAMELOOP M527/M528 to call IO2 "
            "presence/read before F0361 can enqueue, pass514 reaches F0380 with no enqueue/count write, "
            "pass556 resolves the IO2 driver slots but observes no IO2 read/presence after route input, "
            "and pass557 rules out host route-key generation as the first failing boundary."
        )
        return "BLOCKED_PASS558_DOSBOX_EVENT_TO_GUEST_IO2_DISPATCH_BOUNDARY_CLASSIFIED", decision, predicates
    return "BLOCKED_PASS558_PASS514_IO_CLASSIFICATION_INCOMPLETE", "pass514/pass556/pass557 evidence chain incomplete", predicates


def write_report(manifest: dict[str, Any]) -> None:
    lines = [
        "# Pass558 - DM1 V1 pass514/pass556 IO blocker classifier",
        "",
        f"Status: {manifest['status']}",
        "",
        "## Decision",
        "",
        manifest["decision"],
        "",
        "## ReDMCSB source anchors",
        "",
    ]
    for row in manifest["sourceAudit"]:
        line_range = row.get("lineRange")
        if line_range:
            location = f"{row['file']}:{line_range[0]}-{line_range[1]}"
        else:
            location = f"{row['file']}:missing"
        lines.append(f"- {location} {row['id']}")
    lines.extend(
        [
            "",
            "## Evidence chain",
            "",
            f"- pass514 status: {manifest['pass514Status']}",
            f"- pass550 status: {manifest['externalEvidence']['pass550'].get('status')}",
            f"- pass556 status: {manifest['externalEvidence']['pass556'].get('status')}",
            f"- pass557 status: {manifest['externalEvidence']['pass557'].get('status')}",
            "",
            "## Evidence",
            "",
            f"- Manifest: parity-evidence/verification/{PASS}/manifest.json",
        ]
    )
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    source = source_audit()
    pass514 = read_json(PASS514_MANIFEST) or {}
    external = external_audit()
    status, decision, predicates = classify(source, pass514, external)
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "decision": decision,
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"]),
        "head": run(["git", "rev-parse", "HEAD"]),
        "sourceRoot": str(RED),
        "sourceAudit": source,
        "pass514Manifest": str(PASS514_MANIFEST.relative_to(ROOT)),
        "pass514Status": pass514.get("status"),
        "proofPredicates": predicates,
        "externalEvidence": external,
        "nextBoundary": "DOSBox window/event queue -> guest keyboard IRQ/int09 -> IBMIO buffer -> IO2 presence/read",
        "nonClaims": [
            "no new original runtime capture",
            "no movement parity promotion",
            "no Firestaff behavior change",
            "no push",
        ],
    }
    (OUT / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest)
    if not status.startswith("BLOCKED_PASS558_DOSBOX_EVENT_TO_GUEST_IO2_DISPATCH_BOUNDARY_CLASSIFIED"):
        print(json.dumps(manifest, indent=2, sort_keys=True))
        return 1
    print(f"{status}: {decision}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
