#!/usr/bin/env python3
"""Pass622: lock the next DM1 V1 viewport/wall capture-closure gap.

This is an evidence/blocker gate. It does not try to promote pixel parity.
It proves that after pass609 the next wall/viewport closure step is still the
same missing artifact: a source-bound original command/state/redraw/present
transcript paired with a Firestaff viewport frame for the same tuple.
"""
from __future__ import annotations

import json
import os
import re
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


RED = first_existing(
    "FIRESTAFF_REDMCSB_SOURCE",
    [
        DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
        EXTERNAL_DATA / "firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source",
        Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
    ],
)

PASS = "pass622_dm1_v1_viewport_wall_capture_closure_gap"
STATUS = "BLOCKED_PASS622_DM1_V1_VIEWPORT_WALL_CAPTURE_CLOSURE_GAP_LOCKED"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
PASS608_MANIFEST = ROOT / "parity-evidence/verification/pass608_dm1_v1_same_viewport_capture_blocker/manifest.json"
PASS609_REPORT = ROOT / "parity-evidence/pass609_dm1_v1_same_viewport_capture_contract.md"
PASS609_TOOL = ROOT / "tools/verify_pass609_dm1_v1_same_viewport_capture_contract.py"

SOURCE_LOCKS = [
    {
        "id": "wall_and_door_blits_target_g0296_viewport",
        "file": "DUNVIEW.C",
        "lines": "3048-3107",
        "needles": [
            "void F0100_DUNGEONVIEW_DrawWallSetBitmap",
            "void F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency",
            "void F0102_DUNGEONVIEW_DrawDoorBitmap",
            "void F0103_DUNGEONVIEW_DrawDoorFrameBitmapFlippedHorizontally",
            "G0296_puc_Bitmap_Viewport",
        ],
        "claim": "wall and door evidence must be sampled from the composed viewport bitmap, not isolated wall assets",
    },
    {
        "id": "current_square_draws_after_d1_and_d0_side_squares",
        "file": "DUNVIEW.C",
        "lines": "8533-8542",
        "needles": [
            "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0125_DUNGEONVIEW_DrawSquareD0L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0126_DUNGEONVIEW_DrawSquareD0R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
        ],
        "claim": "D0C/current-square closure must be paired to the same F0128 order that already source-locks side and center walls",
    },
    {
        "id": "f0128_consumes_tuple_then_requests_present",
        "file": "DUNVIEW.C",
        "lines": "8318-8610",
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "P0183_i_Direction",
            "P0184_i_MapX",
            "P0185_i_MapY",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "claim": "the accepted crop must bind to the same direction/X/Y tuple consumed by F0128",
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
        "claim": "original captures are promotable only at or after the source viewport-present boundary",
    },
]

PROMOTION_REQUIRES = [
    "one original runtime transcript row with F0380 dequeue/count delta for the sampled route label",
    "matching F0365/F0366 state mutation or source-visible blocked/no-op proof for that row",
    "later F0128 direction/X/Y tuple and F0097/VIDRV present boundary for the same original frame",
    "one Firestaff viewport frame with the same map/X/Y/direction/wall-door tuple and reproducible viewport hash",
    "no duplicate viewport crop hash across labels unless the manifest proves the labels share the same semantic tuple",
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
        if "-" in part:
            first, last = (int(piece) for piece in part.split("-", 1))
        else:
            first = last = int(part)
        out.extend(lines[first - 1 : last])
    return "\n".join(out)


def audit_source(lock: dict[str, Any]) -> dict[str, Any]:
    path = RED / lock["file"]
    if not path.exists():
        return {**lock, "ok": False, "missing": [f"missing source file: {path}"]}
    body = compact(source_window(path, lock["lines"]))
    missing = [needle for needle in lock["needles"] if compact(needle) not in body]
    return {
        "id": lock["id"],
        "file": lock["file"],
        "lines": lock["lines"],
        "ok": not missing,
        "claim": lock["claim"],
        "missing": missing,
    }


def load_json(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8")) if path.exists() else {}


def audit_pass608(manifest: dict[str, Any]) -> dict[str, Any]:
    runtime = manifest.get("runtimeTranscript", {})
    firestaff = manifest.get("firestaffEvidence", {})
    original = manifest.get("freshOriginalDiagnostic", {})
    duplicate_crop_groups = original.get("cropDuplicateRouteIndices", {})
    duplicate_raw_groups = original.get("rawDuplicateRouteIndices", {})
    blockers = list(manifest.get("blockers", []))
    return {
        "manifest": str(PASS608_MANIFEST.relative_to(ROOT)),
        "exists": bool(manifest),
        "ok": manifest.get("ok") is True,
        "status": manifest.get("status"),
        "expectedStatus": "BLOCKED_PASS608_DM1_V1_SAME_VIEWPORT_CAPTURE_NOT_PROMOTABLE",
        "runtimeTranscriptProvided": runtime.get("provided") is True,
        "runtimeTranscriptOk": runtime.get("ok") is True,
        "runtimeTranscriptStatus": runtime.get("status"),
        "firestaffStateCount": len(firestaff.get("states", [])),
        "firestaffViewportHashCount": len(firestaff.get("viewportHashes", [])),
        "freshOriginalClassificationCount": len(original.get("classifications", [])),
        "duplicateOriginalCropGroups": duplicate_crop_groups,
        "duplicateOriginalRawGroups": duplicate_raw_groups,
        "hasExpectedTranscriptBlocker": any("transcript row" in blocker or "command/state/redraw" in blocker for blocker in blockers),
        "decision": manifest.get("decision"),
    }


def audit_pass609_report() -> dict[str, Any]:
    text = read_text(PASS609_REPORT) if PASS609_REPORT.exists() else ""
    source_rows = re.findall(r"^- ([A-Z0-9_.]+):(.*?) .* ok=True", text, flags=re.MULTILINE)
    return {
        "report": str(PASS609_REPORT.relative_to(ROOT)),
        "tool": str(PASS609_TOOL.relative_to(ROOT)),
        "reportExists": PASS609_REPORT.exists(),
        "toolExists": PASS609_TOOL.exists(),
        "statusLocked": "Status: PASS609_DM1_V1_SAME_VIEWPORT_CAPTURE_CONTRACT_LOCKED" in text,
        "currentAttemptNonPromotable": "promotable: False" in text,
        "sourceLockRowCount": len(source_rows),
        "hasFutureCaptureContract": "Future capture contract:" in text,
    }


def main() -> int:
    source = [audit_source(lock) for lock in SOURCE_LOCKS]
    pass608 = audit_pass608(load_json(PASS608_MANIFEST))
    pass609 = audit_pass609_report()

    problems: list[str] = []
    problems.extend(f"source lock failed: {row['id']}" for row in source if not row["ok"])
    if not pass608["exists"] or not pass608["ok"]:
        problems.append("pass608 same-viewport blocker manifest is absent or failed")
    if pass608["status"] != pass608["expectedStatus"]:
        problems.append("pass608 blocker status drifted")
    if pass608["runtimeTranscriptOk"] or pass608["runtimeTranscriptProvided"]:
        problems.append("runtime transcript is now present; replace this blocker with a promotion verifier")
    if pass608["firestaffStateCount"] < 6 or pass608["firestaffViewportHashCount"] < 6:
        problems.append("Firestaff same-viewport fixture evidence regressed")
    if not pass608["duplicateOriginalCropGroups"]:
        problems.append("fresh original diagnostic no longer records duplicate crop blockers")
    if not pass608["hasExpectedTranscriptBlocker"]:
        problems.append("pass608 no longer names the missing command/state/redraw transcript blocker")
    if not all([pass609["reportExists"], pass609["toolExists"], pass609["statusLocked"], pass609["currentAttemptNonPromotable"], pass609["hasFutureCaptureContract"]]):
        problems.append("pass609 same-viewport capture contract is missing or drifted")

    payload = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS if not problems else "FAIL_PASS622_DM1_V1_VIEWPORT_WALL_CAPTURE_CLOSURE_GAP",
        "ok": not problems,
        "sourceRoot": str(RED),
        "sourceAudit": source,
        "pass608SameViewportBlocker": pass608,
        "pass609Contract": pass609,
        "decision": "The next viewport/wall closure gap after pass609 is still capture-backed, not renderer-backed: Firestaff has deterministic same-viewport fixture rows, but the original side has no source-bound command/state/redraw/present transcript and the latest crops are duplicate/non-promotable.",
        "promotionRequires": PROMOTION_REQUIRES,
        "blocker": "missing original runtime transcript row paired to a Firestaff viewport frame for the same map/X/Y/direction/wall-door tuple",
        "nonClaims": [
            "no new original DOSBox capture was run",
            "no original-vs-Firestaff pixel parity is promoted",
            "no renderer behavior change",
            "no V2/CSB/DM2 scope",
            "no DANNESBURK use",
        ],
        "problems": problems,
    }

    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(payload, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass622 - DM1 V1 viewport wall capture-closure gap",
        "",
        f"Status: {payload['status']}",
        "",
        "Decision: " + payload["decision"],
        "",
        "Primary source locks:",
    ]
    for row in source:
        lines.append(f"- {row['file']}:{row['lines']} {row['id']} ok={row['ok']} - {row['claim']}")
    lines += ["", "Pass608 same-viewport blocker:"]
    lines.append(f"- status: {pass608['status']}")
    lines.append(f"- runtimeTranscriptProvided: {pass608['runtimeTranscriptProvided']}")
    lines.append(f"- runtimeTranscriptOk: {pass608['runtimeTranscriptOk']}")
    lines.append(f"- firestaffStateCount: {pass608['firestaffStateCount']}")
    lines.append(f"- firestaffViewportHashCount: {pass608['firestaffViewportHashCount']}")
    lines.append(f"- duplicateOriginalCropGroups: {len(pass608['duplicateOriginalCropGroups'])}")
    lines += ["", "Pass609 contract:"]
    lines.append(f"- statusLocked: {pass609['statusLocked']}")
    lines.append(f"- currentAttemptNonPromotable: {pass609['currentAttemptNonPromotable']}")
    lines.append(f"- sourceLockRowCount: {pass609['sourceLockRowCount']}")
    lines += ["", "Promotion requires:"]
    lines.extend(f"- {item}" for item in PROMOTION_REQUIRES)
    lines += ["", "Blocker:", f"- {payload['blocker']}", "", "Non-claims:"]
    lines.extend(f"- {item}" for item in payload["nonClaims"])
    if problems:
        lines += ["", "Problems:"]
        lines.extend(f"- {problem}" for problem in problems)
    lines.append("")
    lines.append(f"Manifest: {MANIFEST.relative_to(ROOT)}")
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(payload["status"])
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
