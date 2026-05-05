#!/usr/bin/env python3
"""Pass209: audit the delayed-click original DM1 V1 route attempt.

This gate is intentionally source-first and read-only. It promotes neither
ignored DOSBox screenshots nor Firestaff pixels; it records whether the latest
N2 delayed-click route supplies movement/viewport/wall evidence usable for a
future original-vs-Firestaff overlay.
"""
from __future__ import annotations

import argparse
import json
import re
from collections import Counter
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
REDMCSB = Path("/home/trv2/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/Toolchains/Common/Source")
ATTEMPT = ROOT / "verification-screens/pass209-delayed-click-zone-route"
OUT_DIR = ROOT / "parity-evidence/verification/pass209_dm1_v1_original_delayed_click_route_gate"
REPORT = ROOT / "parity-evidence/pass209_dm1_v1_original_delayed_click_route_gate.md"
ROUTE_EVENTS = (
    "wait:7000 enter wait:1500 one wait:1500 click:276,140 wait:1500 one wait:1500 "
    "shot:party_hud kp5 wait:700 shot kp5 wait:700 shot f1 wait:700 "
    "shot:spell_panel one wait:700 shot i wait:700 shot:inventory_panel"
)

SOURCE_CHECKS: list[dict[str, Any]] = [
    {
        "id": "relative-movement-vector-source",
        "file": "DUNGEON.C",
        "function": "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
        "ranges": [(35, 44), (1371, 1391)],
        "needles": [
            "int16_t G0233_ai_Graphic559_DirectionToStepEastCount",
            "int16_t G0234_ai_Graphic559_DirectionToStepNorthCount",
            "void F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "P0253_i_Direction += 1, P0253_i_Direction &= 3; /* Simulate turning right */",
            "*P0256_pi_MapX += G0233_ai_Graphic559_DirectionToStepEastCount[P0253_i_Direction] * P0254_i_StepsForwardCount",
            "*P0257_pi_MapY += G0234_ai_Graphic559_DirectionToStepNorthCount[P0253_i_Direction] * P0254_i_StepsForwardCount",
        ],
        "claim": "Movement captures must be interpreted through the source relative direction-to-map-vector transform.",
    },
    {
        "id": "command-dispatch-turn-step-boundary",
        "file": "COMMAND.C",
        "function": "F0380_COMMAND_ProcessQueue_CPSC",
        "ranges": [(2045, 2156)],
        "needles": [
            "void F0380_COMMAND_ProcessQueue_CPSC",
            "F0365_COMMAND_ProcessTypes1To2_TurnParty(L1160_i_Command);",
            "F0366_COMMAND_ProcessTypes3To6_MoveParty(L1160_i_Command);",
            "L1160_i_Command >= C003_COMMAND_MOVE_FORWARD",
            "L1160_i_Command <= C006_COMMAND_MOVE_LEFT",
        ],
        "claim": "A key/click route is movement evidence only after the original queue dispatch reaches the turn/step handlers.",
    },
    {
        "id": "turn-handler-mutates-direction",
        "file": "CLIKMENU.C",
        "function": "F0365_COMMAND_ProcessTypes1To2_TurnParty",
        "ranges": [(142, 179)],
        "needles": [
            "void F0365_COMMAND_ProcessTypes1To2_TurnParty",
            "F0284_CHAMPION_SetPartyDirection",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        ],
        "claim": "Turn shots must happen after source direction mutation and input-wait release.",
    },
    {
        "id": "step-handler-resolves-walls-and-cooldown",
        "file": "CLIKMENU.C",
        "function": "F0366_COMMAND_ProcessTypes3To6_MoveParty",
        "ranges": [(180, 347)],
        "needles": [
            "void F0366_COMMAND_ProcessTypes3To6_MoveParty",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "L1116_i_SquareType == C00_ELEMENT_WALL",
            "F0267_MOVE_GetMoveResult_CPSCE",
            "G0310_i_DisabledMovementTicks = AL1115_ui_Ticks;",
            "G0321_B_StopWaitingForPlayerInput = C1_TRUE;",
        ],
        "claim": "Forward/side shots must pass original wall/door legality and movement cooldown logic before a viewport delta is expected.",
    },
    {
        "id": "game-loop-redraw-from-current-party-state",
        "file": "GAMELOOP.C",
        "function": "F0002_MAIN_GameLoop_CPSDF",
        "ranges": [(90, 90), (150, 155), (215, 219)],
        "needles": [
            "F0128_DUNGEONVIEW_Draw_CPSF(G0308_i_PartyDirection, G0306_i_PartyMapX, G0307_i_PartyMapY);",
            "G0310_i_DisabledMovementTicks--",
            "F0380_COMMAND_ProcessQueue_CPSC();",
            "while (!G0321_B_StopWaitingForPlayerInput || !G0301_B_GameTimeTicking);",
        ],
        "claim": "Promotable route frames must be sampled after the original game loop redraws from current party direction/X/Y.",
    },
    {
        "id": "viewport-draw-square-order-and-present-request",
        "file": "DUNVIEW.C",
        "function": "F0128_DUNGEONVIEW_Draw_CPSF",
        "ranges": [(8318, 8616)],
        "needles": [
            "void F0128_DUNGEONVIEW_Draw_CPSF",
            "F0098_DUNGEONVIEW_DrawFloorAndCeiling();",
            "F0150_DUNGEON_UpdateMapCoordinatesAfterRelativeMovement",
            "F0122_DUNGEONVIEW_DrawSquareD1L",
            "F0127_DUNGEONVIEW_DrawSquareD0C",
            "F0097_DUNGEONVIEW_DrawViewport(C1_VIEWPORT_DUNGEON_VIEW);",
        ],
        "claim": "Viewport/wall comparisons are source-ordered dungeon-view draws ending in a viewport presentation request.",
    },
    {
        "id": "floor-ceiling-and-wall-target-buffer",
        "file": "DUNVIEW.C",
        "function": "F0098_DUNGEONVIEW_DrawFloorAndCeiling / F0100_DUNGEONVIEW_DrawWallSetBitmap",
        "ranges": [(2962, 3003), (3048, 3110)],
        "needles": [
            "F0007_MAIN_CopyBytes(G0085_puc_Bitmap_Ceiling, G0296_puc_Bitmap_Viewport",
            "F0007_MAIN_CopyBytes(G0084_puc_Bitmap_Floor",
            "void F0100_DUNGEONVIEW_DrawWallSetBitmap",
            "F0132_VIDEO_Blit(P0105_puc_Bitmap, G0296_puc_Bitmap_Viewport",
            "void F0102_DUNGEONVIEW_DrawDoorBitmap",
        ],
        "claim": "Wall evidence must be the composited 224x136 viewport buffer, not standalone wallset assets.",
    },
    {
        "id": "presented-viewport-zone",
        "file": "DRAWVIEW.C",
        "function": "F0097_DUNGEONVIEW_DrawViewport",
        "ranges": [(709, 724), (840, 858)],
        "needles": [
            "void F0097_DUNGEONVIEW_DrawViewport",
            "G0324_B_DrawViewportRequested = C1_TRUE;",
            "M526_WaitVerticalBlank();",
            "F0021_MAIN_BlitToScreen(G0296_puc_Bitmap_Viewport, C007_ZONE_VIEWPORT",
        ],
        "claim": "The comparable original anchor is the presented viewport zone after draw-request/vblank, not an arbitrary full-screen tick.",
    },
]


def read_latin(path: Path) -> str:
    if not path.is_file():
        raise AssertionError(f"missing required file: {path}")
    return path.read_text(encoding="latin-1", errors="replace")


def compact(text: str) -> str:
    return " ".join(text.split())


def source_excerpt(file: str, ranges: list[tuple[int, int]]) -> str:
    lines = read_latin(REDMCSB / file).splitlines()
    out: list[str] = []
    for start, end in ranges:
        out.extend(lines[start - 1 : end])
    return "\n".join(out)


def audit_sources() -> list[dict[str, Any]]:
    results: list[dict[str, Any]] = []
    for spec in SOURCE_CHECKS:
        text = compact(source_excerpt(spec["file"], spec["ranges"]))
        missing = [needle for needle in spec["needles"] if compact(needle) not in text]
        results.append({
            "id": spec["id"],
            "function": spec["function"],
            "citations": [f"{spec['file']}:{a}-{b}" for a, b in spec["ranges"]],
            "claim": spec["claim"],
            "ok": not missing,
            "missing": missing,
        })
    return results


def read_tsv(path: Path) -> list[dict[str, str]]:
    if not path.is_file():
        return []
    rows = [line for line in path.read_text(encoding="utf-8", errors="replace").splitlines() if line.strip()]
    if not rows:
        return []
    header = rows[0].split("\t")
    out: list[dict[str, str]] = []
    for line in rows[1:]:
        fields = line.split("\t")
        out.append({header[i]: fields[i] if i < len(fields) else "" for i in range(len(header))})
    return out


def classifier_summary(path: Path) -> dict[str, Any]:
    if not path.is_file():
        return {"exists": False, "path": str(path)}
    data = json.loads(path.read_text(encoding="utf-8"))
    captures = data.get("captures") or []
    mismatches = [
        {
            "file": row.get("file"),
            "classification": row.get("classification"),
            "expected_class": row.get("expected_class"),
            "expected_match": row.get("expected_match"),
            "sha256": row.get("sha256"),
        }
        for row in captures
        if row.get("expected_match") is False
    ]
    return {
        "exists": True,
        "path": str(path),
        "pass": bool(data.get("pass")),
        "attempt_dir": data.get("attempt_dir"),
        "capture_count": data.get("capture_count"),
        "class_counts": data.get("class_counts"),
        "unique_raw_sha256": sorted({row.get("sha256") for row in captures if row.get("sha256")}),
        "mismatches": mismatches,
    }


def validate_route_shape(route: str) -> dict[str, Any]:
    tokens = route.split()
    shots = [t for t in tokens if t.lower() in {"shot", "capture", "screenshot"} or t.lower().startswith("shot:")]
    clicks = [t for t in tokens if t.lower().startswith("click:")]
    labels = [t.split(":", 1)[1] for t in shots if t.lower().startswith("shot:")]
    invalid: list[str] = []
    for token in tokens:
        low = token.lower()
        if low.startswith("wait:") and not re.fullmatch(r"wait:[0-9]+", low):
            invalid.append(token)
        if low.startswith("click:") and not re.fullmatch(r"click:([0-9]{1,3}),([0-9]{1,3})", low):
            invalid.append(token)
    return {"tokens": len(tokens), "shot_count": len(shots), "clicks": clicks, "labels": labels, "invalid_tokens": invalid}


def materialized_count(rows: list[dict[str, str]], field: str, base: Path) -> int:
    count = 0
    for row in rows:
        name = row.get(field, "")
        if not name:
            continue
        p = Path(name)
        if not p.is_absolute():
            p = base / p
        if p.is_file():
            count += 1
    return count


def attempt_audit() -> dict[str, Any]:
    raw = read_tsv(ATTEMPT / "raw_manifest.tsv")
    viewports = read_tsv(ATTEMPT / "original_viewport_224x136_manifest.tsv")
    labels = read_tsv(ATTEMPT / "original_viewport_shot_labels.tsv")
    raw_sha = Counter(row.get("sha256") for row in raw if row.get("sha256"))
    viewport_sha = Counter(row.get("sha256") for row in viewports if row.get("sha256"))
    cls = classifier_summary(ATTEMPT / "pass80_original_frame_classifier.json")
    return {
        "attempt_dir": str(ATTEMPT),
        "route_events": ROUTE_EVENTS,
        "route_shape": validate_route_shape(ROUTE_EVENTS),
        "raw_count": len(raw),
        "raw_dimensions": sorted({f"{row.get('width')}x{row.get('height')}" for row in raw if row.get("width") and row.get("height")}),
        "raw_sha256_counts": dict(raw_sha),
        "raw_mtime_iso": [row.get("mtime_iso") for row in raw],
        "viewport_count": len(viewports),
        "viewport_dimensions": sorted({f"{row.get('width')}x{row.get('height')}" for row in viewports if row.get("width") and row.get("height")}),
        "viewport_sha256_counts": dict(viewport_sha),
        "labels": labels,
        "materialized_raw_png_count": materialized_count(raw, "path", ATTEMPT),
        "materialized_viewport_ppm_count": materialized_count(viewports, "filename", ATTEMPT / "viewport_224x136"),
        "classifier": cls,
    }


def decide_status(source: list[dict[str, Any]], attempt: dict[str, Any]) -> str:
    if any(not row["ok"] for row in source):
        return "FAIL_REDMCSB_SOURCE_AUDIT"
    if attempt["raw_count"] != 6 or attempt["viewport_count"] != 6:
        return "BLOCKED_MISSING_ROUTE_ARTIFACT_MANIFEST"
    if len(attempt["raw_sha256_counts"]) == 1 and len(attempt["viewport_sha256_counts"]) == 1:
        return "BLOCKED_STATIC_IDENTICAL_FRAMES_AFTER_DELAYED_ROUTE"
    if not (attempt.get("classifier") or {}).get("pass"):
        return "BLOCKED_CLASSIFIER_ROUTE_NOT_PROMOTABLE"
    return "PASS_ROUTE_PROMOTABLE_FOR_PIXEL_OVERLAY"


def write_report(manifest: dict[str, Any], report: Path) -> None:
    a = manifest["attempt_audit"]
    c = a.get("classifier") or {}
    lines = [
        "# Pass209 — DM1 V1 original delayed-click movement/viewport gate",
        "",
        f"Status: `{manifest['status']}`",
        "",
        "Scope: N2-only delayed-click follow-up for original movement/viewport/wall evidence. This gate records the source anchors and route diagnostics; it does not claim original pixel parity.",
        "",
        "## ReDMCSB source audit before capture promotion",
        "",
    ]
    for row in manifest["redmcsb_source_audit"]:
        mark = "PASS" if row["ok"] else "FAIL"
        lines.append(f"- {mark} `{row['id']}` — `{row['function']}` at {', '.join(row['citations'])}: {row['claim']}")
    lines += [
        "",
        "## Reproducible route attempted on N2",
        "",
        "```sh",
        "OUT_DIR=$PWD/verification-screens/pass209-delayed-click-zone-route \\",
        "DM1_ORIGINAL_STAGE_DIR=/home/trv2/.openclaw/data/firestaff-original-games/DM/_extracted/dm-pc34/DungeonMasterPC34 \\",
        "DOSBOX=/usr/bin/dosbox \\",
        "DM1_ORIGINAL_PROGRAM='DM -vv -sn -pk' \\",
        "DM1_ROUTE_SKIP_STARTUP_SELECTOR=1 \\",
        f"DM1_ORIGINAL_ROUTE_EVENTS='{a['route_events']}' \\",
        "xvfb-run -a scripts/dosbox_dm1_original_viewport_reference_capture.sh --run",
        "```",
        "",
        f"- route shape: `{a['route_shape']}`",
        f"- labels manifest rows: `{len(a['labels'])}` from `verification-screens/pass209-delayed-click-zone-route/original_viewport_shot_labels.tsv`",
        "",
        "## Delayed-click route diagnostics",
        "",
        f"- raw frames: `{a['raw_count']}`; dimensions: `{a['raw_dimensions']}`; materialized ignored PNGs on N2: `{a['materialized_raw_png_count']}`",
        f"- raw SHA counts: `{a['raw_sha256_counts']}`",
        f"- raw mtimes: `{a['raw_mtime_iso']}`",
        f"- viewport crops: `{a['viewport_count']}`; dimensions: `{a['viewport_dimensions']}`; materialized ignored PPMs on N2: `{a['materialized_viewport_ppm_count']}`",
        f"- viewport SHA counts: `{a['viewport_sha256_counts']}`",
        f"- classifier: pass=`{c.get('pass')}`, class_counts=`{c.get('class_counts')}`, unique_raw_sha256=`{c.get('unique_raw_sha256')}`",
        "",
        "## Blocker decision",
        "",
    ]
    if manifest["status"] == "BLOCKED_STATIC_IDENTICAL_FRAMES_AFTER_DELAYED_ROUTE":
        lines.append("The delayed-click route is **not promotable** for movement/viewport/wall parity. All six 320x200 raw frames have the same SHA-256, and all six normalized 224x136 viewport crops have the same SHA-256. That proves the route reached a stable dungeon-looking frame, but it does not prove original turn/step/menu dispatch or post-command viewport redraw.")
    elif manifest["status"].startswith("BLOCKED"):
        lines.append("The route remains blocked before pixel overlay promotion; see manifest diagnostics for the missing/drifted artifact.")
    elif manifest["status"].startswith("PASS"):
        lines.append("The route is promotable by this narrow semantic gate; pixel comparison still requires a separate overlay gate.")
    else:
        lines.append("The ReDMCSB source audit failed; do not use this route as evidence.")
    if c.get("mismatches"):
        lines += ["", "Classifier mismatches:"]
        for m in c["mismatches"]:
            lines.append(f"- `{m.get('file')}`: `{m.get('classification')}` expected `{m.get('expected_class')}` sha256=`{m.get('sha256')}`")
    lines += [
        "",
        "Original anchors preserved: raw frame SHA `48ed3743ab6ac9de41689af6c1d3169a8fe00863b4552c1ed813e71c98286397`; viewport crop SHA `701689e73fc0b3f4aa027182a9c1f5059ae90279d164dd42329c7b96092c5d4c`.",
        "",
        "Non-claims: no DANNESBURK/192.168.2.126 use, no push, no original-vs-Firestaff pixel parity claim.",
        "",
    ]
    report.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out-dir", type=Path, default=OUT_DIR)
    ap.add_argument("--report", type=Path, default=REPORT)
    args = ap.parse_args()
    source = audit_sources()
    attempt = attempt_audit()
    status = decide_status(source, attempt)
    manifest = {
        "schema": "pass209_dm1_v1_original_delayed_click_route_gate.v1",
        "status": status,
        "worker": "N2 / firestaff-worker / trv2@192.168.3.121",
        "repo": str(ROOT),
        "redmcsb_source_root": str(REDMCSB),
        "redmcsb_source_audit": source,
        "attempt_audit": attempt,
    }
    args.out_dir.mkdir(parents=True, exist_ok=True)
    manifest_path = args.out_dir / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    write_report(manifest, args.report)
    print(json.dumps({
        "status": status,
        "manifest": str(manifest_path),
        "report": str(args.report),
        "raw_sha256_counts": attempt["raw_sha256_counts"],
        "viewport_sha256_counts": attempt["viewport_sha256_counts"],
    }, indent=2, sort_keys=True))
    return 0 if status in {"PASS_ROUTE_PROMOTABLE_FOR_PIXEL_OVERLAY", "BLOCKED_STATIC_IDENTICAL_FRAMES_AFTER_DELAYED_ROUTE", "BLOCKED_CLASSIFIER_ROUTE_NOT_PROMOTABLE"} else 1


if __name__ == "__main__":
    raise SystemExit(main())
