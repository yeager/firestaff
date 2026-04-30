#!/usr/bin/env python3
"""Source-lock the original overlay/capture route before pixel parity claims.

This verifier is intentionally read-only.  It ties the original DM1 viewport/HUD
capture route to ReDMCSB-owned screen/viewport paths, then reports whether an
existing original route attempt is semantically usable for overlay parity.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
DEFAULT_REDMCSB = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
DEFAULT_ATTEMPT = REPO / "verification-screens/pass112-n2-stable-hud-route"

SOURCE_CHECKS = [
    {
        "id": "viewport-screen-origin",
        "file": "COORD.C",
        "start": 1693,
        "end": 1698,
        "needles": ["G2067_i_ViewportScreenX = 0", "G2068_i_ViewportScreenY = 33"],
        "claim": "DM1 viewport screen anchor is x=0, y=33 for the PC34-era route.",
    },
    {
        "id": "viewport-buffer-compose",
        "file": "DUNVIEW.C",
        "start": 2969,
        "end": 3000,
        "needles": ["G0296_puc_Bitmap_Viewport", "G0085_puc_Bitmap_Ceiling", "G0087_puc_Bitmap_ViewportFloorArea", "G2073_C224_ViewportPixelWidth", "G2074_C136_ViewportHeight"],
        "claim": "Dungeon view is composed into G0296_puc_Bitmap_Viewport with the 224x136 viewport dimensions.",
    },
    {
        "id": "viewport-flush-to-screen",
        "file": "DRAWVIEW.C",
        "start": 840,
        "end": 858,
        "needles": ["F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT", "VIDRV_09_BlitViewPort"],
        "claim": "The composed viewport buffer is flushed to the screen viewport zone; platform paths are explicit.",
    },
    {
        "id": "pc34-vblank-viewport-screen-copy",
        "file": "BASE.C",
        "start": 961,
        "end": 987,
        "needles": [
            "G0324_B_DrawViewportRequested",
            "G0296_puc_Bitmap_Viewport",
            "G0348_Bitmap_Screen",
            "#5280",
            "#135",
            "40 + 40 + 32 = 112 bytes = 224 pixels",
            "C160_BYTE_WIDTH_SCREEN",
        ],
        "claim": "On the Atari/PC34-era vertical blank path, a viewport draw request copies 136 lines from G0296_puc_Bitmap_Viewport into G0348_Bitmap_Screen at offset 5280 (y=33).",
    },
    {
        "id": "generic-viewport-blit-helper",
        "file": "BASE.C",
        "start": 1246,
        "end": 1288,
        "needles": ["void F0020_MAIN_BlitToViewport", "G0296_puc_Bitmap_Viewport", "C112_BYTE_WIDTH_VIEWPORT", "G2073_C224_ViewportPixelWidth", "G2074_C136_ViewportHeight"],
        "claim": "Viewport overlays/panels use the shared helper that writes to G0296_puc_Bitmap_Viewport.",
    },
    {
        "id": "generic-screen-blit-helper",
        "file": "BASE.C",
        "start": 1394,
        "end": 1424,
        "needles": ["void F0021_MAIN_BlitToScreen", "G0348_Bitmap_Screen", "C160_BYTE_WIDTH_SCREEN"],
        "claim": "HUD/screen regions use the shared helper that writes to G0348_Bitmap_Screen.",
    },
    {
        "id": "screen-blit-fallback-helper",
        "file": "FILLBOX.C",
        "start": 814,
        "end": 822,
        "needles": ["void F0021_MAIN_BlitToScreen", "G0348_Bitmap_Screen", "F0132_VIDEO_Blit"],
        "claim": "The FILLBOX variant confirms the same screen blit helper route on SU1E/AU media.",
    },
    {
        "id": "champion-status-hud",
        "file": "CHAMDRAW.C",
        "start": 771,
        "end": 822,
        "needles": ["MASK0x1000_STATUS_BOX", "C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS", "C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION", "F0354_INVENTORY_DrawStatusBoxPortrait"],
        "claim": "Champion status HUD redraws are source-owned by CHAMDRAW status-box paths.",
    },
    {
        "id": "champion-icon-hud",
        "file": "CHAMDRAW.C",
        "start": 1019,
        "end": 1052,
        "needles": ["C028_GRAPHIC_CHAMPION_ICONS", "G0348_Bitmap_Screen", "C113_ZONE_CHAMPION_ICON_TOP_LEFT", "F0021_MAIN_BlitToScreen"],
        "claim": "Top champion icons/names are screen-HUD draws, not viewport-crop content.",
    },
    {
        "id": "inventory-overlay-viewport",
        "file": "PANEL.C",
        "start": 2375,
        "end": 2389,
        "needles": ["F0488_MEMORY_ExpandGraphicToBitmap(C017_GRAPHIC_INVENTORY, G0296_puc_Bitmap_Viewport", "C112_BYTE_WIDTH_VIEWPORT", "C136_HEIGHT_VIEWPORT"],
        "claim": "Inventory panel is an overlay composed into the viewport buffer before capture comparison.",
    },
    {
        "id": "timeline-hud-refresh-events",
        "file": "TIMELINE.C",
        "start": 1817,
        "end": 1829,
        "needles": ["F0260_TIMELINE_RefreshAllChampionStatusBoxes", "MASK0x1000_STATUS_BOX", "F0293_CHAMPION_DrawAllChampionStates"],
        "claim": "Timed HUD refreshes route back into champion redraw state rather than a separate capture-only path.",
    },
]

ROUTE_TOOL_CHECKS = [
    ("scripts/dosbox_dm1_original_viewport_reference_capture.sh", ["DM1_ORIGINAL_ROUTE_EVENTS", "shot:<label>", "--normalize-only"]),
    ("tools/pass80_original_frame_classifier.py", ["wall_closeup", "inventory", "spell_panel"]),
    ("tools/pass112_original_semantic_route_audit.py", ["expected_route_labels", "semantic_route_ready_for_overlay"]),
]


def display(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(REPO))
    except ValueError:
        return str(path)


def read_source(root: Path, rel: str) -> list[str]:
    path = root / rel
    if not path.exists():
        raise AssertionError(f"missing source file: {path}")
    return path.read_text(errors="replace").splitlines()


def check_sources(root: Path) -> bool:
    ok = True
    print("section=redmcsb_source_lock")
    print(f"redmcsbSource={root}")
    for check in SOURCE_CHECKS:
        try:
            lines = read_source(root, check["file"])
            excerpt = "\n".join(lines[check["start"] - 1 : check["end"]])
            missing = [needle for needle in check["needles"] if needle not in excerpt]
        except AssertionError as exc:
            missing = [str(exc)]
        status = "ok" if not missing else "missing:" + ";".join(missing)
        print("sourceRange={}:{}-{} id={} status={}".format(check["file"], check["start"], check["end"], check["id"], status))
        print("sourceClaim={}".format(check["claim"]))
        ok = ok and not missing
    print(f"redmcsbSourceLockOk={1 if ok else 0}")
    return ok


def check_route_tools() -> bool:
    ok = True
    print("section=route_tool_lock")
    for rel, needles in ROUTE_TOOL_CHECKS:
        path = REPO / rel
        if not path.exists():
            print(f"routeTool={rel} status=missing")
            ok = False
            continue
        text = path.read_text(errors="replace")
        missing = [needle for needle in needles if needle not in text]
        status = "ok" if not missing else "missing:" + ",".join(missing)
        print(f"routeTool={rel} status={status}")
        ok = ok and not missing
    print(f"routeToolLockOk={1 if ok else 0}")
    return ok


def check_attempt(attempt_dir: Path) -> bool:
    classifier = attempt_dir / "pass80_original_frame_classifier.json"
    labels = attempt_dir / "original_viewport_shot_labels.tsv"
    ok = True
    print("section=attempt_semantic_gate")
    print(f"attemptDir={display(attempt_dir)}")
    if not labels.exists():
        print(f"labelManifest={display(labels)} status=missing")
        ok = False
    else:
        print(f"labelManifest={display(labels)} status=present")
    if not classifier.exists():
        print(f"classifierJson={display(classifier)} status=missing")
        print("semanticReadyForOverlay=0")
        return False
    data = json.loads(classifier.read_text())
    passed = bool(data.get("pass"))
    print(f"classifierJson={display(classifier)} status=present pass={1 if passed else 0}")
    print("captureCount={}".format(data.get("capture_count")))
    for row in data.get("captures", []):
        print(
            "capture="
            + "{} class={} expected={} match={}".format(
                Path(row.get("file", "")).name,
                row.get("classification"),
                row.get("expected_class"),
                row.get("expected_match"),
            )
        )
    for problem in data.get("problems", []):
        print(f"attemptProblem={problem}")
    duplicate_counts = data.get("duplicate_sha256_counts") or {}
    if duplicate_counts:
        print(f"duplicateSha256Counts={json.dumps(duplicate_counts, sort_keys=True)}")
    ok = ok and passed
    print(f"semanticReadyForOverlay={1 if ok else 0}")
    return ok


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--redmcsb-source", type=Path, default=Path(os.environ.get("REDMCSB_SOURCE_ROOT", DEFAULT_REDMCSB)))
    parser.add_argument("--attempt-dir", type=Path, default=DEFAULT_ATTEMPT)
    args = parser.parse_args()

    print("probe=original_overlay_capture_source_lock")
    source_ok = check_sources(args.redmcsb_source)
    tool_ok = check_route_tools()
    attempt_ok = check_attempt(args.attempt_dir)
    print("honesty=source/tool lock only; semanticReadyForOverlay must be 1 before original-vs-Firestaff pixel parity claims")
    print(f"overallReadyForOverlayParity={1 if (source_ok and tool_ok and attempt_ok) else 0}")
    return 0 if source_ok and tool_ok else 1


if __name__ == "__main__":
    raise SystemExit(main())
