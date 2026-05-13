#!/usr/bin/env python3
"""Pass504 DM1 V1 viewport present/capture contract source lock."""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
RED = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DM1 = Path.home() / ".openclaw/data/firestaff-original-games/DM/_canonical/dm1"
PASS = "pass504_dm1_v1_viewport_present_capture_contract"
VERIFY_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = VERIFY_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
STATUS = "PASS504_DM1_V1_VIEWPORT_PRESENT_CAPTURE_CONTRACT_LOCKED"

EXPECTED_DM1_HASHES = {
    "DUNGEON.DAT": "d90b6b1c38fd17e41d63682f8afe5ca3341565b5f5ddae5545f0ce78754bdd85",
    "GRAPHICS.DAT": "2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e",
    "TITLE": "adc7f1916eeef343849f23c047977d307495b29793b796a54aa427ba71dd3745",
}

SOURCE_LOCKS = [
    {
        "id": "f0128_composes_wall_occlusion_then_calls_present",
        "file": "DUNVIEW.C",
        "lines": "8318-8610",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "claim": "Composition reaches the F0097 present boundary after far-to-near wall/content replay.",
        "ordered": [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(F0162_DUNGEON_GetSquareFirstObject(L0224_i_MapX, L0225_i_MapY), P0183_i_Direction, L0224_i_MapX, L0225_i_MapY, M598_VIEW_SQUARE_D4L",
            "F0116_DUNGEONVIEW_DrawSquareD3L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0117_DUNGEONVIEW_DrawSquareD3R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0119_DUNGEONVIEW_DrawSquareD2L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0121_DUNGEONVIEW_DrawSquareD2C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0122_DUNGEONVIEW_DrawSquareD1L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0123_DUNGEONVIEW_DrawSquareD1R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0124_DUNGEONVIEW_DrawSquareD1C(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0125_DUNGEONVIEW_DrawSquareD0L(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0126_DUNGEONVIEW_DrawSquareD0R(P0183_i_Direction, L0224_i_MapX, L0225_i_MapY);",
            "F0127_DUNGEONVIEW_DrawSquareD0C(P0183_i_Direction, P0184_i_MapX, P0185_i_MapY);",
            "F0097_DUNGEONVIEW_DrawViewport(G0309_i_PartyMapIndex != C255_MAP_INDEX_ENTRANCE);",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
    },
    {
        "id": "f0097_pc34_present_blits_g0296_viewport",
        "file": "DRAWVIEW.C",
        "lines": "709-858",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "claim": "PC34 present boundary blits G0296_puc_Bitmap_Viewport through C007_ZONE_VIEWPORT.",
        "ordered": [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "F0638_GetZone(C007_ZONE_VIEWPORT, L2414_ai_XYZ);",
            "M768_BOX_LEFT(L2413_ai_Box) = M704_ZONE_LEFT(L2414_ai_XYZ);",
            "(*(G2156_VideoDriver->VIDRV_09_BlitViewPort))(G0296_puc_Bitmap_Viewport, L2413_ai_Box);",
        ],
    },
]

LOCAL_LOCKS = [
    {
        "id": "pass502_precise_blocker",
        "path": ROOT / "parity-evidence/pass502_dm1_v1_viewport_wall_occlusion_audit.md",
        "needles": [
            "Status: precise blocker documented; no new pixel-parity promotion.",
            "F0128_DUNGEONVIEW_Draw_CPSF",
            "F0097_DUNGEONVIEW_DrawViewport",
            "Required next evidence before parity promotion:",
        ],
    },
    {
        "id": "pass499_promotion_predicate",
        "path": ROOT / "tools/verify_pass499_dm1_v1_wall_occlusion_runtime_evidence_gate.py",
        "needles": [
            "runtime/capture lane reaches the F0128 to F0097 present boundary for the same viewport",
            "no original-vs-Firestaff pixel parity",
            "no promotion of static repeated screenshots",
        ],
    },
]

CAPTURE_CONTRACT = [
    "hash-lock canonical PC34 DUNGEON.DAT, GRAPHICS.DAT, and TITLE before choosing the frame",
    "record map, x, y, facing, wall/door state, light/palette state, and viewport crop for the original frame",
    "record the matching Firestaff state and prove it reaches F0128_DUNGEONVIEW_Draw_CPSF",
    "prove the same frame reaches F0097_DUNGEONVIEW_DrawViewport before comparing pixels",
    "reject static duplicate screenshots and source-only evidence as parity promotion inputs",
]


def read(path: Path) -> str:
    if not path.exists():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding="latin-1", errors="replace")


def file_sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def slice_lines(text: str, span: str) -> str:
    start_s, end_s = span.split("-", 1)
    start, end = int(start_s), int(end_s)
    lines = text.splitlines()
    return "\n".join(lines[start - 1 : end])


def sha256_text(text: str) -> str:
    return hashlib.sha256(text.encode("utf-8")).hexdigest()


def ordered_positions(excerpt: str, needles: list[str]) -> tuple[list[int], list[str]]:
    positions = []
    missing = []
    cursor = 0
    for needle in needles:
        found = excerpt.find(needle, cursor)
        if found < 0:
            missing.append(needle)
            positions.append(-1)
        else:
            positions.append(excerpt[:found].count("\n") + 1)
            cursor = found + len(needle)
    return positions, missing


def audit_source(lock: dict[str, Any]) -> dict[str, Any]:
    excerpt = slice_lines(read(RED / lock["file"]), lock["lines"])
    positions, missing = ordered_positions(excerpt, lock["ordered"])
    return {
        "id": lock["id"],
        "file": lock["file"],
        "lines": lock["lines"],
        "function": lock["function"],
        "claim": lock["claim"],
        "ok": not missing,
        "missing": missing,
        "positionsInSlice": positions,
        "sliceSha256": sha256_text(excerpt),
    }


def audit_local(lock: dict[str, Any]) -> dict[str, Any]:
    text = read(lock["path"])
    missing = [needle for needle in lock["needles"] if needle not in text]
    return {
        "id": lock["id"],
        "path": str(lock["path"].relative_to(ROOT)),
        "ok": not missing,
        "missing": missing,
    }


def audit_original_data() -> list[dict[str, Any]]:
    rows = []
    for name, expected in EXPECTED_DM1_HASHES.items():
        path = DM1 / name
        actual = file_sha256(path)
        rows.append(
            {
                "name": name,
                "path": str(path),
                "resolvedPath": str(path.resolve()),
                "sha256": actual,
                "expectedSha256": expected,
                "ok": actual == expected,
            }
        )
    return rows


def main() -> int:
    problems = []
    source = [audit_source(lock) for lock in SOURCE_LOCKS]
    local = [audit_local(lock) for lock in LOCAL_LOCKS]
    original = audit_original_data()

    problems.extend(f"source lock failed: {row['id']}" for row in source if not row["ok"])
    problems.extend(f"local lock failed: {row['id']}" for row in local if not row["ok"])
    problems.extend(f"original data hash changed: {row['name']}" for row in original if not row["ok"])

    payload = {
        "schema": f"firestaff.parity.{PASS}.v1",
        "status": STATUS if not problems else "FAIL_PASS504_DM1_V1_VIEWPORT_PRESENT_CAPTURE_CONTRACT",
        "ok": not problems,
        "sourceRoot": str(RED),
        "canonicalDm1Root": str(DM1),
        "sourceLocks": source,
        "localLocks": local,
        "originalData": original,
        "captureContract": CAPTURE_CONTRACT,
        "decision": "The next viewport/walls parity promotion must be a same-frame capture contract tied to ReDMCSB F0128 composition and F0097 present, with canonical PC34 data hashes recorded.",
        "nonClaims": [
            "no new original capture was performed",
            "no original-vs-Firestaff pixel parity is promoted",
            "no movement-core files are touched",
        ],
        "problems": problems,
    }

    VERIFY_DIR.mkdir(parents=True, exist_ok=True)
    MANIFEST.write_text(json.dumps(payload, indent=2, sort_keys=True)  + "\n", encoding="utf-8")

    lines = [
        "# Pass504 - DM1 V1 viewport present/capture contract",
        "",
        f"Status: {payload['status']}",
        "",
        "## Decision",
        payload["decision"],
        "",
        "## ReDMCSB source locks",
    ]
    for row in source:
        lines.append(f"- {row['file']}:{row['lines']} / {row['function']} - ok={row['ok']}; {row['claim']}")
    lines += ["", "## Canonical DM1 PC34 data"]
    for row in original:
        lines.append(f"- {row['name']} - ok={row['ok']}; sha256 {row['sha256']}")
    lines += ["", "## Capture contract"]
    lines.extend(f"- {item}" for item in CAPTURE_CONTRACT)
    lines += ["", "## Non-claims"]
    lines.extend(f"- {item}" for item in payload["nonClaims"])
    if problems:
        lines += ["", "## Problems"]
        lines.extend(f"- {problem}" for problem in problems)
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(payload["status"])
    print(f"manifest={MANIFEST.relative_to(ROOT)}")
    print(f"report={REPORT.relative_to(ROOT)}")
    return 0 if not problems else 1


if __name__ == "__main__":
    raise SystemExit(main())
