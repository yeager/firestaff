#!/usr/bin/env python3
"""Pass609: tighten the DM1 V1 same-viewport capture contract.

This is an evidence gate only. It keeps the pass505 blocker precise by proving
that the current original-route attempt is not promotable unless every semantic
route label has a unique, explicitly paired original crop and Firestaff crop
bound to the same source-stop chain.
"""
from __future__ import annotations

import json
import os
import re
import subprocess
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
DATA = Path.home() / ".openclaw/data"
EXTERNAL_DATA = Path("/Volumes/Extern-disk/openclaw-data/firestaff")

def first_existing(env_name: str, candidates: list[Path]) -> Path:
    env = os.environ.get(env_name)
    if env:
        return Path(env)
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return candidates[0]

RED = first_existing("FIRESTAFF_REDMCSB_SOURCE", [
    DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
    EXTERNAL_DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
    Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
])
PASS = "pass609_dm1_v1_same_viewport_capture_contract"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
PASS505_MANIFEST = ROOT / "parity-evidence/verification/pass505_dm1_v1_same_viewport_capture_blocker/manifest.json"
STATUS = "PASS609_DM1_V1_SAME_VIEWPORT_CAPTURE_CONTRACT_LOCKED"

SOURCE_LOCKS = [
    {
        "id": "game_loop_waits_for_command_then_next_f0128",
        "file": "GAMELOOP.C",
        "lines": "90,164,215-219",
        "needles": [
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
        ],
        "claim": "A labeled original crop must be taken after the command wait loop allows the next F0128 draw.",
    },
    {
        "id": "f0380_dequeues_real_turn_or_move_command",
        "file": "COMMAND.C",
        "lines": "2045-2156",
        "needles": [
            "L1160_i_Command = G0432_as_CommandQueue[G0433_i_CommandQueueFirstIndex].Command;",
            "G2153_i_QueuedCommandsCount--;",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
        ],
        "claim": "The route label must be tied to an actual dequeued command, not only to a host-side screenshot name.",
    },
    {
        "id": "turn_and_move_handlers_mutate_state_before_viewport",
        "file": "CLIKMENU.C",
        "lines": "142-347",
        "needles": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0284_CHAMPION_SetPartyDirection",
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0267_MOVE_GetMoveResult_CPSCE",
        ],
        "claim": "The compared tuple must reflect the original turn/move state mutation for that label.",
    },
    {
        "id": "f0128_uses_tuple_then_calls_present",
        "file": "DUNVIEW.C",
        "lines": "8318-8610",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "claim": "The viewport crop must bind to the same direction/X/Y tuple used for composition.",
    },
    {
        "id": "f0097_presents_g0296_viewport",
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "F0638_GetZone(C007_ZONE_VIEWPORT, L2414_ai_XYZ);",
            "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",
        ],
        "claim": "The crop must be at or after the source present boundary for G0296.",
    },
]

FUTURE_CAPTURE_CONTRACT = [
    "one route label maps to exactly one original viewport crop filename and one Firestaff crop filename",
    "each route label records map, X, Y, direction, wall/door state, light/palette, and viewport crop hash",
    "each original crop has source-bound stops: F0380 pop -> F0365/F0366 mutation -> later F0128 tuple -> F0097/VIDRV present",
    "each paired Firestaff crop records the same map/X/Y/direction/wall-door tuple and reaches the local F0128-to-present path",
    "duplicate raw or viewport-crop hashes across semantic labels remain blockers unless the manifest proves the labels intentionally share the same tuple",
]


def read_text(path: Path) -> str:
    encoding = "latin-1" if path.suffix.upper() in {".C", ".H"} else "utf-8"
    return path.read_text(encoding=encoding, errors="replace")


def source_window(path: Path, spec: str) -> str:
    lines = read_text(path).splitlines()
    out: list[str] = []
    for part in spec.split(","):
        if "-" in part:
            first, last = (int(piece) for piece in part.split("-", 1))
        else:
            first = last = int(part)
        out.extend(lines[first - 1:last])
    return "\n".join(out)


def compact(text: str) -> str:
    return " ".join(text.split())


def audit_source(lock: dict[str, Any]) -> dict[str, Any]:
    body = compact(source_window(RED / lock["file"], lock["lines"]))
    missing = [needle for needle in lock["needles"] if compact(needle) not in body]
    return {
        "id": lock["id"],
        "file": lock["file"],
        "lines": lock["lines"],
        "ok": not missing,
        "claim": lock["claim"],
        "missing": missing,
    }


def run_pass505() -> dict[str, Any]:
    proc = subprocess.run(
        [sys.executable, "tools/verify_pass505_dm1_v1_same_viewport_capture_blocker.py"],
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        timeout=120,
    )
    return {
        "command": [sys.executable, "tools/verify_pass505_dm1_v1_same_viewport_capture_blocker.py"],
        "returncode": proc.returncode,
        "passed": proc.returncode == 0,
        "outputTail": "\n".join(proc.stdout.strip().splitlines()[-8:]),
    }


def slug(value: str) -> str:
    value = re.sub(r"^\d+_", "", value)
    value = re.sub(r"_original_viewport_224x136\.ppm$", "", value)
    value = re.sub(r"[^a-z0-9]+", "_", value.lower()).strip("_")
    return value


def audit_attempt(pass505: dict[str, Any]) -> dict[str, Any]:
    attempt = pass505.get("freshOriginalAttempt", {})
    derived = pass505.get("derived", {})
    labels = list(attempt.get("routeLabels", []))
    crops = list(attempt.get("cropFilenames", []))
    label_slugs = [slug(label) for label in labels]
    crop_slugs = [slug(crop) for crop in crops]
    paired_by_name = label_slugs == crop_slugs
    duplicate_raw = any(count > 1 for count in attempt.get("rawSha256Counts", {}).values())
    duplicate_crops = any(count > 1 for count in attempt.get("viewportCropSha256Counts", {}).values())
    return {
        "routeLabelCount": len(labels),
        "cropFilenameCount": len(crops),
        "labelSlugs": label_slugs,
        "cropSlugs": crop_slugs,
        "labelsAndCropsSameCount": len(labels) == len(crops),
        "labelsAndCropsPairedByName": paired_by_name,
        "duplicateRawHashes": duplicate_raw,
        "duplicateViewportCropHashes": duplicate_crops,
        "sourceBoundStopsPresent": derived.get("sourceBoundStopsPresent") is True,
        "pairedFirestaffCapturePresent": derived.get("pairedFirestaffCapturePresent") is True,
        "promotable": paired_by_name
        and not duplicate_raw
        and not duplicate_crops
        and derived.get("sourceBoundStopsPresent") is True
        and derived.get("pairedFirestaffCapturePresent") is True,
    }


def main() -> int:
    source = [audit_source(lock) for lock in SOURCE_LOCKS]
    pass505_run = run_pass505()
    pass505 = json.loads(PASS505_MANIFEST.read_text(encoding="utf-8")) if PASS505_MANIFEST.exists() else {}
    attempt = audit_attempt(pass505)
    expected_blocker = (
        pass505.get("ok") is True
        and pass505.get("status") == "PASS505_SAME_VIEWPORT_CAPTURE_BLOCKER_LOCKED"
        and attempt["labelsAndCropsSameCount"]
        and not attempt["labelsAndCropsPairedByName"]
        and attempt["duplicateRawHashes"]
        and attempt["duplicateViewportCropHashes"]
        and not attempt["sourceBoundStopsPresent"]
        and not attempt["pairedFirestaffCapturePresent"]
        and not attempt["promotable"]
    )
    problems = [f"source lock failed: {row['id']}" for row in source if not row["ok"]]
    if not pass505_run["passed"]:
        problems.append("pass505 prerequisite failed")
    if not expected_blocker:
        problems.append("pass505 same-viewport blocker predicates drifted")

    payload = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS if not problems else "FAIL_PASS609_DM1_V1_SAME_VIEWPORT_CAPTURE_CONTRACT",
        "ok": not problems,
        "sourceRoot": str(RED),
        "sourceAudit": source,
        "pass505Prerequisite": pass505_run,
        "pass505Manifest": str(PASS505_MANIFEST.relative_to(ROOT)),
        "currentAttemptAudit": attempt,
        "futureCaptureContract": FUTURE_CAPTURE_CONTRACT,
        "decision": "The current pass505 attempt remains non-promotable: labels and crop filenames are not one-to-one semantic pairs, hashes collapse to duplicate states, source-bound original stops are absent, and no paired Firestaff frame exists.",
        "nonClaims": [
            "no new original capture was performed",
            "no original-vs-Firestaff pixel parity is promoted",
            "no renderer behavior change",
            "no TODO.md update",
            "no DANNESBURK use",
        ],
        "problems": problems,
    }
    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass609 - DM1 V1 same-viewport capture contract",
        "",
        f"Status: {payload['status']}",
        "",
        "Decision: " + payload["decision"],
        "",
        "Primary source locks:",
    ]
    for row in source:
        lines.append(f"- {row['file']}:{row['lines']} {row['id']} ok={row['ok']} - {row['claim']}")
    lines += ["", "Current attempt audit:"]
    for key in [
        "routeLabelCount",
        "cropFilenameCount",
        "labelsAndCropsPairedByName",
        "duplicateRawHashes",
        "duplicateViewportCropHashes",
        "sourceBoundStopsPresent",
        "pairedFirestaffCapturePresent",
        "promotable",
    ]:
        lines.append(f"- {key}: {attempt[key]}")
    lines += ["", "Future capture contract:"]
    lines.extend(f"- {item}" for item in FUTURE_CAPTURE_CONTRACT)
    lines += ["", "Non-claims:"]
    lines.extend(f"- {item}" for item in payload["nonClaims"])
    if problems:
        lines += ["", "Problems:"]
        lines.extend(f"- {problem}" for problem in problems)
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(payload["status"])
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
