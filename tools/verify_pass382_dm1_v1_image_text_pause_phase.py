#!/usr/bin/env python3
"""Pass382 verifier: classify the pass379 forced pause in IMAGE_TEXT near F0683.

The pause is a forced sample, not a debugger stop. This pass only decides what
that sample means: expected render/blit activity after route input, or evidence
that the runtime has left the useful DM1 game-loop/render phase.
"""
from __future__ import annotations

import json
import re
import subprocess
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
PASS = "pass382_dm1_v1_image_text_pause_phase"
OUTDIR = ROOT / "parity-evidence/verification" / PASS
MANIFEST = OUTDIR / "manifest.json"
REPORT = ROOT / "parity-evidence" / f"{PASS}.md"
SRC = Path.home() / ".openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source"
FIRES_MAP = Path.home() / ".openclaw/data/redmcsb-n2-build-probe/ibm-pc-i34e-fires/HARDDISK/BUILD/I34E/FIRES.MAP"
P379 = ROOT / "parity-evidence/verification/pass379_dm1_v1_true_stop_codepath_probe/manifest.json"
P380 = ROOT / "parity-evidence/verification/pass380_dm1_v1_f0128_f0097_map_transition_narrowing/manifest.json"
P381 = ROOT / "parity-evidence/verification/pass381_dm1_v1_route_f0380_transition_source_path/manifest.json"

EXPECTED = {
    "pass379": "BLOCKED_PASS379_POST_ROUTE_PAUSE_MOVED_MAP_OR_PATH_UNRESOLVED",
    "pass380": "BLOCKED_PASS380_SOURCE_PATH_TRANSITION_NARROWED_MAP_BINDING_HELD",
    "pass381": "BLOCKED_PASS381_STATIC_F0380_STOPWAIT_TRANSITION_PROVEN_RUNTIME_BREAKPOINTS_NEXT",
}

SOURCE_LOCKS = [
    {
        "id": "image_segment_is_image_c",
        "file": "IMAGE.C",
        "needles": ["#include \"IMAGE3.C\"", "#include \"DRAWVIEW.C\"", "#include \"DRAWMSGA.C\""],
        "meaning": "IMAGE_TEXT is the IMAGE.C code segment containing the generic image/viewport/message blit helpers, not TEXT.C string logic.",
    },
    {
        "id": "f0683_is_pixel_line_blitter",
        "file": "IMAGE3.C",
        "needles": [
            "FUNC_DEF void F0683_CopyPixelLineToScreenWithTransparencyFlippedHorizontally",
            "P2332_c_TransparentColor",
            "G2160_puc_Bitmap_Destination",
            "G2159_puc_Bitmap_Source",
        ],
        "meaning": "F0683 copies one transparent, horizontally flipped pixel line to the destination bitmap/screen.",
    },
    {
        "id": "f0684_reaches_f0683_for_flipped_transparent_blits",
        "file": "IMAGE3.C",
        "needles": [
            "FUNC_DEF void F0684_Blit",
            "case MASK0x0001_FLIP_HORIZONTAL:",
            "F0683_CopyPixelLineToScreenWithTransparencyFlippedHorizontally(L2511_i_SourcePixelIndex, L2512_i_DestinationPixelIndex, L2509_ui_Width, P2340_i_TransparentColor);",
            "case (MASK0x0002_FLIP_VERTICAL | MASK0x0001_FLIP_HORIZONTAL):",
        ],
        "meaning": "The nearest symbol is the low-level worker selected by F0684_Blit for transparent horizontally flipped blits.",
    },
    {
        "id": "dunview_has_flipped_transparent_viewport_blits",
        "file": "DUNVIEW.C",
        "needles": [
            "F0132_VIDEO_Blit(L2439_puc_Bitmap, G0296_puc_Bitmap_Viewport, L2468_ai_XYZ, L2440_s_Struct2.X + L2440_s_Struct2.Width, L2440_s_Struct2.Y + L2440_s_Struct2.Height, M100_PIXEL_WIDTH(L2439_puc_Bitmap), G2073_C224_ViewportPixelWidth, C10_COLOR_FLESH, MASK0x0001_FLIP_HORIZONTAL);",
            "F0132_VIDEO_Blit(L2443_puc_Bitmap, G0296_puc_Bitmap_Viewport, L2445_ai_XYZ, L2444_s_Struct2.X + L2444_s_Struct2.Width, L2444_s_Struct2.Y + L2444_s_Struct2.Height, M100_PIXEL_WIDTH(L2443_puc_Bitmap), G2073_C224_ViewportPixelWidth, CM1_COLOR_NO_TRANSPARENCY, P2075_i_Flip);",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "meaning": "DUNVIEW can legitimately reach transparent/flipped blits while drawing/presenting the dungeon viewport.",
    },
    {
        "id": "drawmsga_uses_same_image_segment_for_message_redraw",
        "file": "DRAWMSGA.C",
        "needles": ["F0132_VIDEO_Blit(G0356_puc_Bitmap_MessageAreaNewRow", "G0348_Bitmap_Screen"],
        "meaning": "The same IMAGE_TEXT segment also serves message-area redraws, so a forced sample there is generic UI/render work.",
    },
]

MAP_SYMBOLS = {
    "F0683_COPYPIXELLINETOSCREENWITHT": "F0683_COPYPIXELLINETOSCREENWITHT",
    "F0684_BLIT": "F0684_BLIT",
    "F0132_VIDEO_BLIT": "F0132_VIDEO_BLIT",
    "F0128_DUNGEONVIEW_DRAW_CPSF": "F0128_DUNGEONVIEW_DRAW_CPSF",
    "F0097_DUNGEONVIEW_DRAWVIEWPORT": "F0097_DUNGEONVIEW_DRAWVIEWPORT",
}


def run(cmd: list[str]) -> str:
    return subprocess.run(cmd, cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, check=False).stdout.strip()


def compact(s: str) -> str:
    return " ".join(s.split())


def find_line(path: Path, needle: str) -> int | None:
    want = compact(needle)
    for i, line in enumerate(path.read_text(encoding="latin-1", errors="replace").splitlines(), 1):
        if want in compact(line):
            return i
    return None


def audit_sources() -> list[dict[str, Any]]:
    rows = []
    for lock in SOURCE_LOCKS:
        path = SRC / lock["file"]
        hits = {needle: find_line(path, needle) for needle in lock["needles"]} if path.exists() else {}
        missing = [needle for needle, line in hits.items() if line is None]
        rows.append({k: v for k, v in lock.items() if k != "needles"} | {"path": str(path), "ok": path.exists() and not missing, "lineHits": hits, "missing": missing})
    return rows


def load(path: Path) -> dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def map_audit() -> dict[str, Any]:
    out: dict[str, Any] = {"path": str(FIRES_MAP), "ok": True, "segmentRows": [], "symbols": {}}
    text = FIRES_MAP.read_text(encoding="latin-1", errors="replace")
    for line in text.splitlines():
        if "IMAGE_TEXT" in line:
            out["segmentRows"].append(line.strip())
        m = re.search(r"\b([0-9A-F]{4}:[0-9A-F]{4})\s+(?:idle\s+)?_([A-Z0-9_]+)\b", line)
        if m:
            name = m.group(2)
            for label, sym in MAP_SYMBOLS.items():
                if name == sym and label not in out["symbols"]:
                    out["symbols"][label] = {"link": m.group(1), "row": line.strip()}
    for label in MAP_SYMBOLS:
        out["symbols"].setdefault(label, {"link": None, "row": None})
        out["symbols"][label]["ok"] = out["symbols"][label]["link"] is not None
        out["ok"] = bool(out["ok"] and out["symbols"][label]["ok"])
    out["imageTextMentionsImageC"] = any("M=IMAGE.C" in row for row in out["segmentRows"])
    out["ok"] = bool(out["ok"] and out["imageTextMentionsImageC"])
    return out


def prior_audit() -> tuple[dict[str, Any], list[str]]:
    errors = []
    p379, p380, p381 = load(P379), load(P380), load(P381)
    rt = p379.get("runtimeProbe", {})
    decode = p380.get("finalPauseDecode", {})
    priors = {
        "pass379": {"path": str(P379.relative_to(ROOT)), "status": p379.get("status"), "finalPauseCodeAddr": rt.get("finalPauseCodeAddr"), "directHits": rt.get("directHits"), "routeInputAfterArming": rt.get("routeInputAfterArming"), "breakpointRetainedPostRoute": rt.get("breakpointRetainedPostRoute")},
        "pass380": {"path": str(P380.relative_to(ROOT)), "status": p380.get("status"), "finalPauseDecode": decode},
        "pass381": {"path": str(P381.relative_to(ROOT)), "status": p381.get("status"), "candidateRuntimeBreakpointsNext": p381.get("candidateRuntimeBreakpointsNext")},
    }
    for key, status in EXPECTED.items():
        if priors[key]["status"] != status:
            errors.append(f"{key} status drift")
    hits = rt.get("directHits") or {}
    if hits.get("f0128_23AD_40FE") or hits.get("f0097_2809_1EFF") or hits.get("f0097_entry_2809_1E31"):
        errors.append("pass379 unexpectedly hit F0128/F0097")
    if rt.get("finalPauseCodeAddr") != "280C:14B5":
        errors.append("pass379 final pause moved")
    if decode.get("segment") != "IMAGE_TEXT" or decode.get("nearestSymbol", {}).get("name") != "F0683_COPYPIXELLINETOSCREENWITHT" or decode.get("isF0128OrF0097") is not False:
        errors.append("pass380 final pause decode drift")
    return priors, errors


def main() -> int:
    OUTDIR.mkdir(parents=True, exist_ok=True)
    errors: list[str] = []
    src_rows = audit_sources()
    if not all(row["ok"] for row in src_rows):
        errors.append("source render-path audit failed")
    maps = map_audit()
    if not maps["ok"]:
        errors.append("FIRES.MAP IMAGE_TEXT/symbol audit failed")
    priors, prior_errors = prior_audit()
    errors.extend(prior_errors)

    ok = not errors
    status = "BLOCKED_PASS382_IMAGE_TEXT_PAUSE_EXPECTED_RENDER_HELPER_NOT_WRONG_PHASE" if ok else "FAIL_PASS382_IMAGE_TEXT_PAUSE_PHASE_UNRESOLVED"
    conclusion = (
        "The forced pause at 280C:14B5 decodes into the IMAGE.C/IMAGE3.C blit segment near F0683, a transparent horizontally-flipped pixel-line worker reached by F0684/F0132 blits. ReDMCSB shows this is normal render/UI machinery used by DUNVIEW viewport drawing and message-area redraws after input. It is therefore expected render-helper activity, not evidence that the runtime is in a wrong phase. Because pass379 still had no direct F0128/F0097/07FB stop, the sample does not prove the next outer-loop viewport draw; the blocker remains proving F0380/F0365/F0366 dequeue, G0321/tick wait-loop exit, then the next F0128/F0097 hit."
        if ok else
        "The IMAGE_TEXT pause classification could not be locked; inspect errors before using this as evidence."
    )
    manifest = {
        "schema": PASS + ".v1",
        "timestampUtc": datetime.now(timezone.utc).isoformat(),
        "status": status,
        "repo": str(ROOT),
        "branch": run(["git", "branch", "--show-current"]),
        "head": run(["git", "rev-parse", "HEAD"]),
        "sourceRoot": str(SRC),
        "mapAudit": maps,
        "sourceAudit": src_rows,
        "priorArtifacts": priors,
        "decision": {"imageTextPauseIsExpectedRenderHelper": ok, "wrongRuntimePhase": False if ok else None, "provesNextF0128": False, "conclusion": conclusion},
        "nextRuntimeBreakpoints": priors.get("pass381", {}).get("candidateRuntimeBreakpointsNext", {}),
        "notPromotedBy": ["forced pause sample", "IMAGE_TEXT segment name", "route keylog alone", "BPLIST retention alone"],
        "errors": errors,
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    REPORT.write_text("\n".join([
        "# Pass382 — DM1 V1 IMAGE_TEXT forced-pause phase classification",
        "",
        f"Status: `{status}`",
        "",
        "## Decision",
        "",
        conclusion,
        "",
        "## Evidence",
        "",
        "- pass379 forced pause stayed at `280C:14B5` with no F0128/F0097/07FB direct hit.",
        "- pass380 decodes that sample into `IMAGE_TEXT`, nearest `F0683_COPYPIXELLINETOSCREENWITHT`, and explicitly not F0128/F0097.",
        "- FIRES.MAP binds `IMAGE_TEXT` to `IMAGE.C`; ReDMCSB includes `IMAGE3.C` there.",
        "- `F0683` is a transparent horizontally-flipped pixel-line blitter reached from `F0684_Blit` / `F0132_VIDEO_Blit`.",
        "- DUNVIEW and DRAWMSGA both call the same blit machinery, so a forced pause there after input is normal render/UI activity, not a phase escape.",
        "",
        "## Consequence",
        "",
        "Do not retarget F0128/F0097 from this sample. The next useful runtime pass should arm the pass381 F0380/F0365/F0366 candidates, then prove G0321/tick wait-loop exit before expecting the next F0128/F0097 stop.",
        "",
        f"Manifest: `parity-evidence/verification/{PASS}/manifest.json`",
    ]) + "\n", encoding="utf-8")
    print(json.dumps({"status": status, "manifest": str(MANIFEST.relative_to(ROOT)), "errors": errors}, indent=2, sort_keys=True))
    return 0 if ok else 1

if __name__ == "__main__":
    raise SystemExit(main())
