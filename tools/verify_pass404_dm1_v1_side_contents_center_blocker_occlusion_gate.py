#!/usr/bin/env python3
"""Verify pass404 DM1 V1 side contents obey nearest center blocker.

Firestaff renders several DM1 viewport primitive classes in split passes.
ReDMCSB DUNVIEW.C F0128 is square-ordered instead: D3L/D3R before D3C,
then D2L/D2R before D2C, then D1L/D1R before D1C. CSBWin's viewport
port keeps the same relative-cell order in DrawViewport's userCellNum loop.
Once Firestaff has already drawn the nearest blocking center wall/door,
late side contents and side explosions must not repaint same-depth or farther
side cells over it.
"""
from __future__ import annotations

from pathlib import Path
from datetime import datetime, timezone
import json
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass404_dm1_v1_side_contents_center_blocker_occlusion_gate"
OUT_DIR = ROOT / "parity-evidence" / "verification" / PASS
MANIFEST = OUT_DIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
SRC = ROOT / "m11_game_view.c"
REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source/DUNVIEW.C"
CSBWIN = Path.home() / ".openclaw/data/firestaff-csbwin-source/CSBWin/Viewport.cpp"


def line_no(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_function(text: str, name: str) -> tuple[int, str]:
    m = re.search(r"\b(?:static\s+)?(?:int|void)\s+" + re.escape(name) + r"\s*\(", text)
    if not m:
        raise AssertionError(f"missing function {name}")
    brace = text.find("{", m.end())
    if brace < 0:
        raise AssertionError(f"missing body for {name}")
    depth = 0
    for i in range(brace, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return m.start(), text[m.start():i + 1]
    raise AssertionError(f"unterminated body for {name}")


def require_order(body: str, needles: list[str], label: str) -> None:
    pos = -1
    for needle in needles:
        at = body.find(needle)
        if at < 0:
            raise AssertionError(f"{label}: missing {needle!r}")
        if at <= pos:
            raise AssertionError(f"{label}: {needle!r} is out of order")
        pos = at


def require_redmcsb_order() -> str:
    text = REDMCSB.read_text(encoding="latin-1")
    m = re.search(r"\bvoid\s+F0128_DUNGEONVIEW_Draw_CPSF\s*\(", text)
    if not m:
        raise AssertionError("missing function F0128_DUNGEONVIEW_Draw_CPSF")
    start = m.start()
    # This translated C file has conditional sections with preprocessor braces;
    # source-audit the bounded F0128 region by exact monotonic anchors.
    end_marker = text.find("\nvoid F0129_", start)
    if end_marker < 0:
        end_marker = len(text)
    body = text[start:end_marker]
    anchors = [
        "F0116_DUNGEONVIEW_DrawSquareD3L",
        "F0117_DUNGEONVIEW_DrawSquareD3R",
        "F0118_DUNGEONVIEW_DrawSquareD3C_CPSF",
        "F0119_DUNGEONVIEW_DrawSquareD2L",
        "F0120_DUNGEONVIEW_DrawSquareD2R_CPSF",
        "F0121_DUNGEONVIEW_DrawSquareD2C",
        "F0122_DUNGEONVIEW_DrawSquareD1L",
        "F0123_DUNGEONVIEW_DrawSquareD1R",
        "F0124_DUNGEONVIEW_DrawSquareD1C",
    ]
    require_order(body, anchors, "ReDMCSB F0128 side-before-center draw order")
    first = text.find(anchors[0], start)
    last = text.find(anchors[-1], start)
    return f"DUNVIEW.C:{line_no(text, first)}-{line_no(text, last)}"


def require_csbwin_order() -> str:
    text = CSBWIN.read_text(encoding="latin-1")
    _, body = find_function(text, "DrawViewport")
    anchors = [
        "for (userCellNum = 0; userCellNum < 21; userCellNum++)",
        "Interpret(pCode, startOffset, roomData, userCellNum, facing",
    ]
    require_order(body, anchors, "CSBWin DrawViewport relative-cell loop")
    names = ["RF3L1", "RF3R1", "RF3", "RF2L1", "RF2R1", "RF2", "RF1L1", "RF1R1", "RF1"]
    values: dict[str, int] = {}
    positions: list[int] = []
    for name in names:
        m = re.search(r"^#define\s+" + name + r"\s+(\d+)\s*$", text, re.MULTILINE)
        if not m:
            raise AssertionError(f"CSBWin missing {name} relative-cell define")
        values[name] = int(m.group(1))
        positions.append(m.start())
    for depth in ("3", "2", "1"):
        left = values[f"RF{depth}L1"]
        right = values[f"RF{depth}R1"]
        center = values[f"RF{depth}"]
        if not (left < center and right < center):
            raise AssertionError(f"CSBWin RF{depth} side indices do not precede center index")
    if not (values["RF3"] < values["RF2L1"] < values["RF2"] < values["RF1L1"] < values["RF1"]):
        raise AssertionError("CSBWin near/far relative-cell order is not monotonic")
    first = min(positions)
    last = max(positions)
    loop = text.find(anchors[0])
    return f"Viewport.cpp:{line_no(text, first)}-{line_no(text, last)} and DrawViewport loop at {line_no(text, loop)}"


def run(cmd: list[str]) -> str:
    p = subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
    if p.returncode != 0:
        raise AssertionError(f"command failed {' '.join(cmd)}: {p.stdout[-1000:]}")
    return p.stdout.strip()


def main() -> int:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    redmcsb_anchor = require_redmcsb_order()
    csbwin_anchor = require_csbwin_order()
    text = SRC.read_text(encoding="utf-8")
    ok: list[str] = []

    start, body = find_function(text, "m11_draw_dm1_side_contents")
    require_order(body, [
        "blockingCenterDepth = m11_dm1_nearest_blocking_center_depth_index(cells);",
        "if (blockingCenterDepth >= 0 && depth >= blockingCenterDepth)",
        "break;",
        "m11_draw_item_sprite",
    ], "side contents center-blocker occlusion")
    ok.append(f"side contents gate before item/creature/projectile draws: m11_game_view.c:{line_no(text, start)}")

    start, body = find_function(text, "m11_draw_dm1_deferred_explosion_pass")
    require_order(body, [
        "blockingCenterDepth = m11_dm1_nearest_blocking_center_depth_index(cells);",
        "if (blockingCenterDepth >= 0 && depth >= blockingCenterDepth)",
        "m11_draw_dm1_deferred_side_explosion",
    ], "side explosion center-blocker occlusion")
    ok.append(f"side explosion gate before deferred side explosion draw: m11_game_view.c:{line_no(text, start)}")

    status = "PASS404_DM1_V1_SIDE_CONTENTS_CENTER_BLOCKER_OCCLUSION_PROVEN"
    manifest = {
        "schema": f"{PASS}.v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "branch": run(["git", "branch", "--show-current"]),
        "head": run(["git", "rev-parse", "HEAD"]),
        "sourceAudit": {
            "ReDMCSB": redmcsb_anchor,
            "CSBWinCorroboration": csbwin_anchor,
        },
        "firestaffGuards": ok,
        "closedBlocker": "side contents / side explosions no longer repaint same-depth-or-farther cells past the nearest non-open center wall/door",
        "notClaimed": ["original pixel parity", "new original DOS runtime capture", "full creature/ornament coordinate parity"],
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n")
    lines = [
        "# Pass404 — DM1 V1 side contents center-blocker occlusion gate",
        "",
        f"Status: `{status}`",
        "",
        "## ReDMCSB-first source audit",
        f"- ReDMCSB square draw order: `{redmcsb_anchor}`",
        f"- CSBWin corroboration: `{csbwin_anchor}`",
        "",
        "## Firestaff guards",
    ]
    lines += [f"- `{line}`" for line in ok]
    lines += [
        "",
        "## Verdict",
        "- Closed blocker: side contents and deferred side explosions are bounded by the nearest non-open center wall/door before drawing item/creature/projectile/explosion primitives.",
        "- Scope guard: source/order closure only; no original pixel-parity or DOS runtime capture claim.",
        "",
        f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
    ]
    REPORT.write_text("\n".join(lines) + "\n")
    print("PASS pass404-dm1-v1-side-contents-center-blocker-occlusion-gate")
    print(f"- ReDMCSB square draw order: {redmcsb_anchor}")
    print(f"- CSBWin relative-cell corroboration: {csbwin_anchor}")
    for line in ok:
        print(f"- {line}")
    print(f"- report: {REPORT.relative_to(ROOT)}")
    print(f"- manifest: {MANIFEST.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (AssertionError, OSError) as exc:
        print(f"FAIL pass404-dm1-v1-side-contents-center-blocker-occlusion-gate: {exc}")
        raise SystemExit(1)
