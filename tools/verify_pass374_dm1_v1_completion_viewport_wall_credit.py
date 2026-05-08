#!/usr/bin/env python3
"""Pass374 verifier: credit pass373 live viewport wall/occlusion proof in the completion matrix."""
from __future__ import annotations

import json
import os
import pathlib
import subprocess
import sys
from datetime import datetime, timezone
from typing import Any

ROOT = pathlib.Path(__file__).resolve().parents[1]
PASS = "pass374_dm1_v1_completion_viewport_wall_credit"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
REDMCSB = pathlib.Path(os.environ.get(
    "FIRESTAFF_REDMCSB_SOURCE",
    str(pathlib.Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"),
))
PASS373 = ROOT / "parity-evidence/verification/pass373_dm1_v1_launcher_viewport_redraw_wall_occlusion_path/manifest.json"
MATRIX = ROOT / "parity-evidence/verification/firestaff_completion_matrix.json"
DOC = ROOT / "docs/parity/COMPLETION_MATRIX.md"
EXPECTED_STATUS = "PASS374_DM1_V1_VIEWPORT_WALL_COMPLETION_CREDIT_PROVED"

SOURCE_LOCKS: list[dict[str, Any]] = [
    {
        "file": "DUNVIEW.C",
        "lines": "2962-3003",
        "function": "F0098_DUNGEONVIEW_DrawFloorAndCeiling",
        "claim": "floor/ceiling base is copied into viewport buffers before wall replay",
        "markers": [
            "void F0098_DUNGEONVIEW_DrawFloorAndCeiling(",
            "F0674_F0128_sub(G2109_Ceiling, G0296_puc_Bitmap_Viewport);",
            "F0674_F0128_sub(G2108_Floor, G0087_puc_Bitmap_ViewportFloorArea);",
        ],
    },
    {
        "file": "DUNVIEW.C",
        "lines": "3048-3082",
        "function": "F0100/F0101/F0102 viewport blitters",
        "claim": "transparent wall, opaque wall, and door bitmap routines target the viewport bitmap",
        "markers": [
            "void F0100_DUNGEONVIEW_DrawWallSetBitmap(",
            "void F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency(",
            "void F0102_DUNGEONVIEW_DrawDoorBitmap(",
            "G0296_puc_Bitmap_Viewport",
        ],
    },
    {
        "file": "DUNVIEW.C",
        "lines": "4547-4910",
        "function": "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        "claim": "contents are replayed by ordered cell nibbles rather than by a flat depth sort",
        "markers": [
            "STATICFUNCTION void F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF(",
            "P0146_ui_OrderedViewCellOrdinals >>= 4;",
            "L0139_i_Cell = M021_NORMALIZE(AL0126_i_ViewCell + P0142_i_Direction);",
            "P0141_T_Thing = L0146_T_FirstThingToDraw;",
        ],
    },
    {
        "file": "DUNVIEW.C",
        "lines": "6400-6835",
        "function": "F0116/F0117/F0118 D3 side and center square draws",
        "claim": "D3 wall/door branches draw wall layers, door passes, and return/occlude as source order dictates",
        "markers": [
            "F0100_DUNGEONVIEW_DrawWallSetBitmap(G0698_puc_Bitmap_WallSet_Wall_D3LCR",
            "C0x0218_CELL_ORDER_DOORPASS1_BACKLEFT_BACKRIGHT",
            "C0x0349_CELL_ORDER_DOORPASS2_FRONTLEFT_FRONTRIGHT",
            "return;",
        ],
    },
    {
        "file": "DUNVIEW.C",
        "lines": "7244-7937",
        "function": "F0121/F0124 D2C and D1C center square draws",
        "claim": "center wall/door branches layer opaque wall/door graphics before ordered contents",
        "markers": [
            "STATICFUNCTION void F0121_DUNGEONVIEW_DrawSquareD2C(",
            "STATICFUNCTION void F0124_DUNGEONVIEW_DrawSquareD1C(",
            "F0101_DUNGEONVIEW_DrawWallSetBitmapWithoutTransparency",
            "F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF",
        ],
    },
    {
        "file": "DUNVIEW.C",
        "lines": "8318-8618",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "claim": "main viewport pass redraws floor/ceiling, replays visible squares far-to-near, then requests viewport presentation",
        "markers": [
            "void F0128_DUNGEONVIEW_Draw_CPSF(",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0116_DUNGEONVIEW_DrawSquareD3L(",
            "F0121_DUNGEONVIEW_DrawSquareD2C(",
            "F0124_DUNGEONVIEW_DrawSquareD1C(",
            "F0127_DUNGEONVIEW_DrawSquareD0C(",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
    },
    {
        "file": "DRAWVIEW.C",
        "lines": "709-722",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "claim": "viewport redraw request is presented and synchronized after composition",
        "markers": [
            "void F0097_DUNGEONVIEW_DrawViewport(",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "M526_WaitVerticalBlank();",
        ],
    },
]


def compact(text: str) -> str:
    return " ".join(text.split())


def read_lines(path: pathlib.Path, spec: str) -> str:
    lines = path.read_text(encoding="latin-1", errors="replace").splitlines()
    start_s, end_s = spec.split("-", 1)
    return "\n".join(lines[int(start_s) - 1:int(end_s)])


def verify_source_locks() -> list[dict[str, Any]]:
    rows = []
    for item in SOURCE_LOCKS:
        path = REDMCSB / item["file"]
        text = read_lines(path, item["lines"]) if path.exists() else ""
        missing = [m for m in item["markers"] if compact(m) not in compact(text)]
        rows.append({**item, "path": str(path), "ok": path.exists() and not missing, "missingMarkers": missing})
    return rows


def run(cmd: list[str], timeout: int = 120) -> dict[str, Any]:
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=timeout)
    return {"cmd": cmd, "returncode": p.returncode, "outputTail": p.stdout[-4000:]}


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    checks: list[dict[str, Any]] = []
    source_locks = verify_source_locks()
    checks.extend({"kind": "redmcsb_source_lock", "ok": r["ok"], "file": r["file"], "lines": r["lines"], "function": r["function"], "missingMarkers": r["missingMarkers"]} for r in source_locks)

    pass373 = json.loads(PASS373.read_text(encoding="utf-8")) if PASS373.exists() else {}
    checks.append({
        "kind": "pass373_prerequisite",
        "ok": pass373.get("status") == "PASS373_LAUNCHER_VIEWPORT_REDRAW_WALL_OCCLUSION_PATH_PROVED",
        "manifest": str(PASS373.relative_to(ROOT)),
        "status": pass373.get("status"),
        "scope": pass373.get("scope"),
    })

    matrix = json.loads(MATRIX.read_text(encoding="utf-8"))
    dm1 = next(r for r in matrix["rows"] if r["target"] == "DM1 V1")
    viewport_score, viewport_note = dm1["scores"]["viewport_ui_render"]
    checks.append({
        "kind": "completion_matrix_credit",
        "ok": dm1["completionPercent"] == 56 and dm1["points"] == 56 and viewport_score == 11 and "pass373" in viewport_note,
        "observed": {"completionPercent": dm1["completionPercent"], "points": dm1["points"], "viewport_ui_render": viewport_score, "note": viewport_note},
    })

    doc = DOC.read_text(encoding="utf-8")
    checks.append({"kind": "completion_doc_credit", "ok": "| DM1 V1 | 56% | 56/100 |" in doc and "| `viewport_ui_render` | 11/20 |" in doc})
    r = run([sys.executable, "tools/verify_firestaff_completion_matrix.py"])
    checks.append({"kind": "firestaff_completion_matrix_verifier", "ok": r["returncode"] == 0, "result": r})
    r = run([sys.executable, "tools/firestaff_completion_status.py"])
    checks.append({"kind": "firestaff_completion_status_cli", "ok": r["returncode"] == 0 and "DM1 V1 | completionPercent=56%" in r["outputTail"], "result": r})

    ok = all(c.get("ok") for c in checks)
    status = EXPECTED_STATUS if ok else "BLOCKED_PASS374_DM1_V1_VIEWPORT_WALL_COMPLETION_CREDIT"
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"])["outputTail"].strip(),
        "head": run(["git", "rev-parse", "HEAD"])["outputTail"].strip(),
        "sourceRoot": str(REDMCSB),
        "pass373Manifest": str(PASS373.relative_to(ROOT)),
        "completionImpact": "DM1 V1 verified completion increases from 55/100 to 56/100 by crediting one additional viewport_ui_render point for the pass373 live launcher->movement->viewport redraw path into source-locked wall/door/occlusion rendering.",
        "notClaimed": ["pixel-perfect viewport parity", "representative original overlay regression", "DOS keyboard-buffer true-stop proof"],
        "sourceLocks": source_locks,
        "checks": checks,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = [
        "# Pass374 — DM1 V1 viewport/wall completion credit",
        "",
        f"Status: `{status}`",
        "",
        "## ReDMCSB source audit first",
        "",
    ]
    for item in SOURCE_LOCKS:
        lines.append("- `{file}:{lines}` — `{function}`: {claim}.".format(**item))
    lines += [
        "",
        "## Landable update",
        "",
        "This pass credits pass373 in the conservative completion matrix: DM1 V1 moves from `55/100` to `56/100`; `viewport_ui_render` moves from `10/20` to `11/20`.",
        "",
        "The credit is narrow: live Firestaff launcher movement now reaches the source-locked wall/door/occlusion redraw stack. It does not claim pixel parity or original overlay regression.",
        "",
        "## Gates",
        "",
    ]
    for c in checks:
        lines.append("- `{}` ok={}".format(c["kind"], c.get("ok")))
    lines += ["", f"Manifest: `{MANIFEST.relative_to(ROOT)}`"]
    REPORT.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "manifest": str(MANIFEST.relative_to(ROOT)), "report": str(REPORT.relative_to(ROOT))}, indent=2))
    return 0 if ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
